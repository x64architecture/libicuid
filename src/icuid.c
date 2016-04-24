/*
 * Copyright (c) 2015 - 2016, Kurt Cancemi (kurt@x64architecture.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <icuid/icuid.h>

#include <internal/stdcompat.h>

#include "features.h"
#include "intel.h"
#include "amd.h"

int cpuid_get_raw_data(cpuid_raw_data_t *raw)
{
    int i;

    if (!cpuid_is_supported())
        return ICUID_NO_CPUID;

    for (i = 0; i < MAX_CPUID_LEVEL; i++)
        cpuid(i, raw->cpuid[i]);
    for (i = 0; i < MAX_EXT_CPUID_LEVEL; i++)
        cpuid(0x80000000 + i, raw->cpuid_ext[i]);
    for (i = 0; i < MAX_INTEL_DC_LEVEL; i++) {
        memset(raw->intel_dc[i], 0, sizeof(raw->intel_dc[i]));
        raw->intel_dc[i][0] = 4;
        raw->intel_dc[i][2] = i;
        cpuid_ext(raw->intel_dc[i]);
    }
    for (i = 0; i < MAX_INTEL_ET_LEVEL; i++) {
        memset(raw->intel_et[i], 0, sizeof(raw->intel_et[i]));
        raw->intel_et[i][0] = 11;
        raw->intel_et[i][2] = i;
        cpuid_ext(raw->intel_et[i]);
    }

    return ICUID_OK;
}

static int parse_line(char *line, const char *token, uint32_t regs[][4],
                      const uint32_t limit)
{
    int i;
    char buf[64];
    char *p, *pp, *ex;
    size_t tokenlen = strlen(token);
    unsigned long level, reg;

    /* Check if our token matches the line */
    if (line[tokenlen] != '[')
        return 2;
    if (strncmp(line, token, tokenlen) != 0)
        return 2;

    /* Since strtok modifies the original buffer we need to make a copy */
    (void) strlcpy(buf, line + tokenlen + 1, sizeof(buf));

    /* Separate the text between the brackets into separate strings */
    p = strtok(buf, "[ ]");
    if (p == NULL)
        return 0;

    errno = 0;
    level = strtoul(p, &ex, 10);
    if (ex == p) {
        return 0;
    } else if (level == ULONG_MAX && errno == ERANGE) {
        return 0;
    } else if (level > limit) {
        return 0;
    } else if ((level == 0 && errno == EINVAL)) {
        return 0;
    }

    /* Get the text after the equals sign */
    p = strstr(line, "=");
    if (p == NULL)
        return 0;
    p += 1;

    /* Separate the text with spaces into separate strings */
    pp = strtok(p, " ");
    if (pp == NULL)
        return 0;

    for (i = 0; i < 4 && pp != NULL; i++) {
        errno = 0;
        reg = strtoul(pp, &ex, 16);
        if (ex == pp) {
            return 0;
        } else if (reg == ULONG_MAX && errno == ERANGE) {
            return 0;
        } else if (reg > 0xFFFFFFFF /* 2^32-1 */) {
            return 0;
        } else if ((reg == 0 && errno == EINVAL)) {
            return 0;
        }
        regs[level][i] = reg;
        pp = strtok(NULL, " "); /* Get next string */
    }

    return 1;
}

int cpuid_serialize_raw_data(cpuid_raw_data_t *raw, const char *file)
{
    char line[64];
    FILE *fp;

    if (raw == NULL || file == NULL)
        return ICUID_PASSED_NULL;

    if (strcmp(file, "") == 0)
        fp = stdin;
    else
        fp = fopen(file, "rt");

    if (fp == NULL)
        return ICUID_ERROR_OPEN;

    memset(raw, 0, sizeof(cpuid_raw_data_t));

    while (fgets(line, sizeof(line), fp)) {

        if (line[0] == '#')
            continue;

        if (!parse_line(line, "cpuid", raw->cpuid, MAX_CPUID_LEVEL))
            goto parse_err;
        if (!parse_line(line, "cpuid_ext", raw->cpuid_ext, MAX_EXT_CPUID_LEVEL))
            goto parse_err;
        if (!parse_line(line, "intel_dc", raw->intel_dc, MAX_INTEL_DC_LEVEL))
            goto parse_err;
        if (!parse_line(line, "intel_et", raw->intel_et, MAX_INTEL_ET_LEVEL))
            goto parse_err;
    }

    fclose(fp);

    return ICUID_OK;

parse_err:
    fclose(fp);
    return ICUID_ERROR_PARSING;
}

int cpuid_deserialize_raw_data(cpuid_raw_data_t *raw, const char *file)
{
    int ret = -1;
    int i;
    FILE *fp;

    if (raw == NULL || file == NULL)
        return ICUID_PASSED_NULL;

    if (strcmp(file, "") == 0)
        fp = stdout;
    else
        fp = fopen(file, "wt");

    if (fp == NULL)
        return ICUID_ERROR_OPEN;

    ret = cpuid_get_raw_data(raw);
    if (ret != ICUID_OK) {
        fclose(fp);
        return ret;
    }

    for (i = 0; i < MAX_CPUID_LEVEL; i++)
        fprintf(fp, "cpuid[%u]=%08x %08x %08x %08x\n", i,
                raw->cpuid[i][0], raw->cpuid[i][1],
                raw->cpuid[i][2], raw->cpuid[i][3]);
    for (i = 0; i < MAX_EXT_CPUID_LEVEL; i++)
        fprintf(fp, "cpuid_ext[%u]=%08x %08x %08x %08x\n", i,
                raw->cpuid_ext[i][0], raw->cpuid_ext[i][1],
                raw->cpuid_ext[i][2], raw->cpuid_ext[i][3]);
    for (i = 0; i < MAX_INTEL_DC_LEVEL; i++)
        fprintf(fp, "intel_dc[%u]=%08x %08x %08x %08x\n", i,
                raw->intel_dc[i][0], raw->intel_dc[i][1],
                raw->intel_dc[i][2], raw->intel_dc[i][3]);
    for (i = 0; i < MAX_INTEL_ET_LEVEL; i++)
        fprintf(fp, "intel_et[%u]=%08x %08x %08x %08x\n", i,
                raw->intel_et[i][0], raw->intel_et[i][1],
                raw->intel_et[i][2], raw->intel_et[i][3]);

    fclose(fp);

    return ICUID_OK;
}

/* Must be called after the vendor string is obtained */
static void get_vendor(cpuid_data_t *data)
{
    int i;
    const struct {
        char vendor_str[VENDOR_STR_MAX];
        cpu_vendor_t vendor;
    } cpu_vendors[] = {
        { "GenuineIntel", VENDOR_INTEL },
        { "AuthenticAMD", VENDOR_AMD },
        { "CentaurHauls", VENDOR_CENTAUR },
        { "CyrixInstead", VENDOR_CYRIX },
        { "TransmetaCPU", VENDOR_TRANSMETA },
        { "GenuineTMx86", VENDOR_TRANSMETA },
        { "Geode by NSC", VENDOR_NSC },
        { "NexGenDriven", VENDOR_NEXGEN },
        { "RiseRiseRise", VENDOR_RISE },
        { "SiS SiS SiS ", VENDOR_SIS },
        { "UMC UMC UMC ", VENDOR_UMC },
        { "VIA VIA VIA ", VENDOR_VIA },
        { "KVMKVMKVMKVM", VENDOR_HV_KVM },
        { "Microsoft Hv", VENDOR_HV_HYPERV },
        { "VMwareVMware", VENDOR_HV_VMWARE },
        { "XenVMMXenVMM", VENDOR_HV_XEN },
    };
    for (i = 0; i < NUM_CPU_VENDORS; i++) {
        if (strcmp(data->vendor_str, cpu_vendors[i].vendor_str) == 0) {
            data->vendor = cpu_vendors[i].vendor;
            return;
        }
    }
}

int icuid_identify(cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    int ret;
    int i, j = 0;
    uint8_t ext_family, ext_model;
    char brandstr[BRAND_STR_MAX];
    cpuid_raw_data_t iraw;

    if (raw == NULL) {
        ret = cpuid_get_raw_data(&iraw);
        if (ret != ICUID_OK)
            return ret;
        raw = &iraw;
    }

    memset(data, 0, sizeof(cpuid_data_t));

    /* Get max cpuid level */
    data->cpuid_max_basic = raw->cpuid[0][0];
    /* Get max extended cpuid level */
    data->cpuid_max_ext = raw->cpuid_ext[0][0];

    /* Get vendor string */
    memcpy(data->vendor_str + 0, &raw->cpuid[0][1], 4);
    memcpy(data->vendor_str + 4, &raw->cpuid[0][3], 4);
    memcpy(data->vendor_str + 8, &raw->cpuid[0][2], 4);
    data->vendor_str[12] = '\0';

    /* Get vendor */
    get_vendor(data);

    if (data->cpuid_max_basic >= 1) {
        data->family = (raw->cpuid[1][0] >> 8) & 0xF;
        data->model = (raw->cpuid[1][0] >> 4) & 0xF;
        data->stepping = (raw->cpuid[1][0] >> 0) & 0xF;
        ext_model = (raw->cpuid[1][0] >> 16) & 0xF;
        ext_family = (raw->cpuid[1][0] >> 20) & 0xFF;
        data->signature = (raw->cpuid[1][0] >> 4);
        if (data->vendor == VENDOR_AMD && data->family < 0xF)
            data->ext_family = data->family;
        else
            data->ext_family |= data->family + ext_family;
        data->ext_model |= data->model + (ext_model << 4);
    }

    /* Get brand string */
    if (data->cpuid_max_ext >= 0x80000004) {
        for (i = 2; i <= 4; i++, j += 16) {
            memcpy(brandstr +  0 + j, &raw->cpuid_ext[i][0], 4);
            memcpy(brandstr +  4 + j, &raw->cpuid_ext[i][1], 4);
            memcpy(brandstr +  8 + j, &raw->cpuid_ext[i][2], 4);
            memcpy(brandstr + 12 + j, &raw->cpuid_ext[i][3], 4);
        }
        /*
         * Some brand strings have spaces prepended to them so we have
         * to remove them.
         */
        i = 0;
        while (brandstr[i] == ' ')
            i++;
        (void) strlcpy(data->brand_str, brandstr + i, sizeof(data->brand_str));
    }

    /* Populate data->flags */
    set_common_features(raw, data);

    /* Get addressing info */
    if (data->cpuid_max_ext >= 0x80000008) {
        data->physical_address_bits = raw->cpuid_ext[8][0] & 0xFF;
        data->virtual_address_bits = (raw->cpuid_ext[8][0] >> 8) & 0xFF;
    }

    if (data->flags[CPU_FEATURE_OSXSAVE]) {
        uint64_t xcr0 = icuid_xgetbv(0);
        set_common_xfeatures(data, xcr0);
    }

    /* Get vendor specific info */
    if (data->vendor == VENDOR_INTEL)
        read_intel_data(raw, data);
    else if (data->vendor == VENDOR_AMD)
        read_amd_data(raw, data);
    
    return ICUID_OK;
}
