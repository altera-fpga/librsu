/*
 * Copyright (c) 2024, Intel Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * HAL layer for RSU file operations
 */
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "RSU_OSAL_types.h"
#include "hal/RSU_plat_file.h"
#include <utils/RSU_logging.h>

#include "ff_sddisk.h"
#include "FreeRTOS.h"
#include "ff_sys.h"
#include "ff_stdio.h"
#include "ff_headers.h"

#define MOUNT_POINT         "/sdcard"
#define RSU_MOUNT_DEV_SD    -1
FF_Disk_t *pxRsuDisk;
FF_FILE *pxRsuFile;

struct fileCntxt
{
    RSU_OSAL_CHAR fileName[60];
    BaseType_t xIsCntxtOpen;

};

struct fileCntxt CurrFileCntxt = {
    .xIsCntxtOpen = 0,
};

static RSU_OSAL_FILE *plat_filesys_open(RSU_OSAL_CHAR *filename,
        RSU_filesys_flags_t flag)
{
    FF_Error_t xError;

    if (!filename)
    {
        RSU_LOG_ERR("invalid argument");
        return NULL;
    }
    strcpy(CurrFileCntxt.fileName, filename);
    if (flag == RSU_FILE_READ)
    {
        pxRsuFile = FF_Open(pxRsuDisk->pxIOManager,
                filename, FF_MODE_READ,
                &xError);
    }
    else if (flag == RSU_FILE_WRITE)
    {
        pxRsuFile = FF_Open(pxRsuDisk->pxIOManager,
                filename, FF_MODE_CREATE | FF_MODE_WRITE | FF_MODE_TRUNCATE,
                &xError);
    }
    else if (flag == RSU_FILE_APPEND)
    {
        pxRsuFile = FF_Open(pxRsuDisk->pxIOManager,
                filename, FF_MODE_APPEND | FF_MODE_CREATE | FF_MODE_WRITE,
                &xError);
    }
    else
    {
        RSU_LOG_ERR("invalid argument in flag");
        return NULL;
    }

    if (xError != FF_ERR_NONE)
    {
        RSU_LOG_ERR("opening the file failed");
        return NULL;
    }
    CurrFileCntxt.xIsCntxtOpen = 1;

    return pxRsuFile;
}

static RSU_OSAL_INT plat_filesys_close(RSU_OSAL_FILE *file)
{
    FF_Error_t xError;
    if (!file)
    {
        return -EINVAL;
    }

    xError = FF_Close(file);
    if (xError != FF_ERR_NONE)
    {
        RSU_LOG_ERR("error in closing the file");
        return -1;
    }
    CurrFileCntxt.xIsCntxtOpen = 0;
    return 0;
}

static RSU_OSAL_INT plat_filesys_write(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len,
        RSU_OSAL_FILE *file)
{
    if (!buf || (len <= 0) || !file)
    {
        return -EINVAL;
    }

    int32_t bytesWritten = FF_Write(file, 1, len, buf);
    if (bytesWritten < 0)
    {
        RSU_LOG_ERR("error in writing to file");
        return -1;
    }
    return bytesWritten;
}

static RSU_OSAL_INT plat_filesys_read(RSU_OSAL_VOID *buf, RSU_OSAL_SIZE len,
        RSU_OSAL_FILE *file)
{

    if (!buf || (len <= 0) || !file)
    {
        return -EINVAL;
    }

    int32_t bytesRead = FF_Read(file, 1, len, buf);
    if (bytesRead < 0)
    {
        RSU_LOG_ERR("error in reading the file ");
        return -1;
    }
    return bytesRead;
}

static RSU_OSAL_INT plat_filesys_fgets(RSU_OSAL_CHAR *str, RSU_OSAL_SIZE len,
        RSU_OSAL_FILE *file)
{
    if ((str == NULL) || (len == 0) || (file == NULL))
    {
        return -EINVAL;
    }

    int32_t lRet;
    lRet = FF_GetLine(file, str, (uint32_t)len);

    if ((lRet == 0) && (str == NULL))
    {
        return 1;
    }
    else if (lRet != 0)
    {
        return lRet;
    }
    else
    {
        return 0;
    }
}

static RSU_OSAL_INT plat_filesys_fseek(RSU_OSAL_OFFSET offset,
        RSU_filesys_whence_t whence,
        RSU_OSAL_FILE *file)
{
    if (!file)
    {
        return -EINVAL;
    }

    FF_Error_t xError;

    if (whence == RSU_SEEK_SET)
    {
        xError = FF_Seek(file, offset, FF_SEEK_SET);
    }
    else if (whence == RSU_SEEK_CUR)
    {
        xError = FF_Seek(file, offset, FF_SEEK_CUR);
    }
    else if (whence == RSU_SEEK_END)
    {
        xError = FF_Seek(file, offset, FF_SEEK_END);
    }
    else
    {
        RSU_LOG_ERR("invalid whence %u\n", whence);
        return -EINVAL;
    }

    if (xError != FF_ERR_NONE)
    {
        return -1;
    }
    return 0;
}

static RSU_OSAL_INT plat_filesys_ftruncate(RSU_OSAL_OFFSET length,
        RSU_OSAL_FILE *file)
{
    if (!file)
    {
        return -EINVAL;
    }

    if (CurrFileCntxt.xIsCntxtOpen)
    {
        RSU_OSAL_FILE *tempStream = ff_truncate(CurrFileCntxt.fileName, length);

        if (tempStream == NULL)
        {
            return -1;
        }
    }
    return 0;
}

static RSU_OSAL_INT plat_filesys_terminate(RSU_OSAL_VOID)
{
    FF_Error_t xError = FF_Unmount(pxRsuDisk);
    if (xError != FF_ERR_NONE)
    {
        RSU_LOG_ERR("Failed to unmount filesystem");
        FF_SDDiskDelete(pxRsuDisk);
        return -1;
    }

    FF_SDDiskDelete(pxRsuDisk);
    return 0;
}

RSU_OSAL_INT plat_filesys_init(struct filesys_ll_intf *filesys_intf)
{
    FF_Error_t xError;
    if (!filesys_intf)
    {
        return -EINVAL;
    }

    pxRsuDisk = FF_SDDiskInit(MOUNT_POINT, RSU_MOUNT_DEV_SD);
    if (pxRsuDisk == NULL)
    {
        RSU_LOG_ERR("Failed to initialize disk");
        return -1;
    }

    xError = FF_Mount(pxRsuDisk, 0);
    if (xError != FF_ERR_NONE)
    {
        RSU_LOG_ERR("Failed to mount filesystem");
        FF_SDDiskDelete(pxRsuDisk);
        return -1;
    }

    filesys_intf->open = plat_filesys_open;
    filesys_intf->read = plat_filesys_read;
    filesys_intf->fgets = plat_filesys_fgets;
    filesys_intf->write = plat_filesys_write;
    filesys_intf->fseek = plat_filesys_fseek;
    filesys_intf->ftruncate = plat_filesys_ftruncate;
    filesys_intf->close = plat_filesys_close;
    filesys_intf->terminate = plat_filesys_terminate;
    return 0;
}

