/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU_misc.h>
#include <string.h>

static RSU_OSAL_CHAR *reserved_names[] = {
	"BOOT_INFO",
	"FACTORY_IMAGE",
	"SPT",
	"SPT0",
	"SPT1",
	"CPB",
	"CPB0",
	"CPB1",
	"" /* Terminating table entry */
};

RSU_OSAL_VOID swap_bits(RSU_OSAL_CHAR *data, RSU_OSAL_INT len)
{
	RSU_OSAL_INT x, y;
	RSU_OSAL_CHAR tmp;

	for (x = 0; x < len; x++) {
		tmp = 0;
		for (y = 0; y < 8; y++) {
			tmp <<= 1;
			if (data[x] & 1) {
				tmp |= 1;
			}
			data[x] >>= 1;
		}
		data[x] = tmp;
	}
}

RSU_OSAL_U32 swap_endian32(RSU_OSAL_U32 val)
{
	RSU_OSAL_U32 rtn;
	RSU_OSAL_CHAR *from = (RSU_OSAL_CHAR *)&val;
	RSU_OSAL_CHAR *to = (RSU_OSAL_CHAR *)&rtn;
	RSU_OSAL_INT x;

	for (x = 0; x < 4; x++) {
		to[x] = from[3 - x];
	}

	return rtn;
}

RSU_OSAL_BOOL librsu_misc_is_rsvd_name(RSU_OSAL_CHAR *name)
{
	RSU_OSAL_U32 x;

	for (x = 0; reserved_names[x][0] != '\0'; x++) {
		if (strcmp(name, reserved_names[x]) == 0) {
			return 1;
		}
	}

	return false;
}

RSU_OSAL_BOOL librsu_misc_is_slot(struct librsu_hl_intf *intf, RSU_OSAL_INT part_num)
{
	if (intf->partition.readonly(part_num) || intf->partition.reserved(part_num) ||
	    librsu_misc_is_rsvd_name(intf->partition.name(part_num))) {
		return false;
	}

	return true;
}

RSU_OSAL_INT librsu_misc_slot2part(struct librsu_hl_intf *intf, RSU_OSAL_INT slot)
{
	RSU_OSAL_INT partitions;
	RSU_OSAL_INT x, cnt = 0;

	partitions = intf->partition.count();

	for (x = 0; x < partitions; x++) {
		if (librsu_misc_is_slot(intf, x)) {
			if (slot == cnt) {
				return x;
			}
			cnt++;
		}
	}

	return -EINVAL;
}

RSU_OSAL_VOID SAFE_STRCPY(RSU_OSAL_CHAR *dst, RSU_OSAL_INT dsz, RSU_OSAL_CHAR *src,
			  RSU_OSAL_INT ssz)
{
	RSU_OSAL_INT len;

	if (!dst || dsz <= 0) {
		return;
	}

	if (!src || ssz <= 0) {
		dst[0] = '\0';
		return;
	}

	/*
	 * calculate len to be smaller of source string len or destination
	 * buffer size, then copy len bytes and terminate string.
	 */

	len = strnlen(src, ssz);

	if (len >= dsz) {
		len = dsz - 1;
	}

	rsu_memcpy(dst, src, len);
	dst[len] = '\0';
}
