/*
 * Copyright (c) 2024, Intel Corporation.
 *
 * SPDX-License-Identifier: MIT
 *
 * CRC32 implementation for RSU
 */

#include <arm_acle.h>
#include "RSU_crc32_def.h"

const unsigned int x2n_table[] =
{
    0x40000000, 0x20000000, 0x08000000, 0x00800000, 0x00008000,
    0xedb88320, 0xb1e6b092, 0xa06a2517, 0xed627dae, 0x88d14467,
    0xd7bbfe6a, 0xec447f11, 0x8e7ea170, 0x6427800e, 0x4d47bae0,
    0x09fe548f, 0x83852d0f, 0x30362f1a, 0x7b5a9cc3, 0x31fec169,
    0x9fec022a, 0x6c8dedc4, 0x15d6874d, 0x5fde7a4e, 0xbad90e37,
    0x2e4e5eef, 0x4eaba214, 0xa8a472c0, 0x429a969e, 0x148d302a,
    0xc40ba6d0, 0xc4e22c3c
};

unsigned int multmod(unsigned int op1, unsigned int op2)
{
    unsigned int modulo, poly;
    modulo = (unsigned int)1 << 31;
    poly = 0;
    while (1)
    {
        if (op1 & modulo)
        {
            poly ^= op2;
            if ((op1 & (modulo - 1)) == 0)
            {
                break;
            }
        }
        modulo >>= 1;
        op2 = op2 & 1 ? (op2 >> 1) ^ CRC_POLY : op2 >> 1;
    }
    return poly;
}

unsigned int x2nmodp(uint64_t n, unsigned int k)
{
    unsigned int poly;

    poly = (unsigned int )1 << 31;
    while (n > 0)
    {
        if (n & 1)
        {
            poly = multmod(x2n_table[k & 31], poly);
        }
        n >>= 1;
        k++;
    }
    return poly;
}

unsigned long int calculate_crc32(unsigned long int ulCrc, void *vData,
        unsigned long ulDataSize)
{
    char *buf = (char *)vData;
    unsigned int val;
    unsigned long int crc1, crc2;
    const unsigned long int  *word;
    unsigned long int val0, val1, val2;
    unsigned long int last1, last2, i, num;

    if (vData == NULL)
    {
        return 0;
    }

    ulCrc = (~ulCrc) & 0xffffffff;

    while (ulDataSize && ((unsigned long int)buf & 7) != 0)
    {
        ulDataSize--;
        val = *buf++;
        ulCrc = __crc32b(ulCrc, val);
    }
    word = (const unsigned long int *)buf;
    num = ulDataSize >> 3;
    ulDataSize &= 7;

    while (num >= (3 * CRC_BATCH_SZ))
    {
        crc1 = 0;
        crc2 = 0;
        for (int i = 0; i < CRC_BATCH_SZ; i++)
        {
            val0 = word[i];
            val1 = word[i + CRC_BATCH_SZ];
            val2 = word[i + 2 * CRC_BATCH_SZ];
            ulCrc = __crc32d(ulCrc, val0);
            crc1  = __crc32d(crc1, val1);
            crc2  = __crc32d(crc2, val2);

        }
        word += 3 * CRC_BATCH_SZ;
        num  -= 3 * CRC_BATCH_SZ;
        ulCrc = multmod(CRC_BATCH_ZEROES, ulCrc) ^ crc1;
        ulCrc = multmod(CRC_BATCH_ZEROES, ulCrc) ^ crc2;
    }
    last1 = num / 3;
    if (last1 >= CRC_BATCH_MIN_SZ)
    {
        last2 = last1 << 1;
        crc1 = 0;
        crc2 = 0;
        for (i = 0; i < last1; i++)
        {
            val0 = word[i];
            val1 = word[i + last1];
            val2 = word[i + last2];
            ulCrc = __crc32d(ulCrc, val0);
            crc1  = __crc32d(crc1, val1);
            crc2  = __crc32d(crc2, val2);
        }
        word += 3 * last1;
        num  -= 3 * last1;
        val = x2nmodp(last1, 6);
        ulCrc = multmod(val, ulCrc) ^ crc1;
        ulCrc = multmod(val, ulCrc) ^ crc2;
    }
    for (i = 0; i < num; i++)
    {
        val0 = word[i];
        ulCrc = __crc32d(ulCrc, val0);
    }
    word += num;

    buf = (char *)word;
    while (ulDataSize)
    {
        ulDataSize--;
        val = *buf++;
        ulCrc = __crc32b(ulCrc, val);
    }
    return ulCrc ^ 0xffffffff;
}

