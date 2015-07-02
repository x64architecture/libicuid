/*
 * Copyright (c) 2015, Kurt Cancemi (kurt@x64architecture.com)
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

#include <stdio.h>
#include <string.h>

#include <icuid/icuid.h>

#include "internal/stdcompat.h"
#include "internal.h"
#include "intel.h"
#include "amd.h"

int cpuid_get_raw_data(cpuid_raw_data_t *raw)
{
    unsigned int i;

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

/* This probably needs some work but it still gets the job done */
static int parse_line(char *line, const char *token, uint32_t regs[][4])
{
    char levelbuf[64];
    char valuebuf[64];
    size_t count = strlen(token);
    uint32_t level;
    uint32_t eax, ebx, ecx, edx;

    /* Check if our token matches the line */
    if (strncmp(line, token, count) != 0)
        return 2; /* Doesn't match */
    if (line[count] == '_')
        return 2; /* Doesn't match (cpuid) */
    /* Get level */
    (void) strlcpy(levelbuf, line+count, count);
    if (sscanf(levelbuf, "[%u]", &level) != 1)
        return 0;
    /* Get value */
    for (count = 0; line[count] != '='; count++);
    (void) strlcpy(valuebuf, line+count+1, sizeof(valuebuf));
    if (sscanf(valuebuf, "%x%x%x%x", &eax, &ebx, &ecx, &edx) != 4)
        return 0;
    regs[level][0] = eax;
    regs[level][1] = ebx;
    regs[level][2] = ecx;
    regs[level][3] = edx;
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

        if (parse_line(line, "cpuid", raw->cpuid) == 0)
            goto parse_err;
        if (parse_line(line, "cpuid_ext", raw->cpuid_ext) == 0)
            goto parse_err;
        if (parse_line(line, "intel_dc", raw->intel_dc) == 0)
            goto parse_err;
        if (parse_line(line, "intel_et", raw->intel_et) == 0)
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
    unsigned int i;
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
static void GetVendor(cpuid_data_t *data)
{
    unsigned int i;
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
        if (strcmp(data->vendor_str, cpu_vendors[i].vendor_str) == 0)
            data->vendor = cpu_vendors[i].vendor;
    }
}

int icuid_identify(cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    int ret;
    unsigned int i, j, k = 0;
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
    data->vendor = VENDOR_UNKNOWN;
    GetVendor(data);

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

    /* Get vendor specific info */
    if (data->vendor == VENDOR_INTEL)
        read_intel_data(raw, data);
    else if (data->vendor == VENDOR_AMD)
        read_amd_data(raw, data);

    /* Get brand string */
    if (data->cpuid_max_ext >= 0x80000004) {
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 4; j++) {
                memcpy(brandstr + k, &raw->cpuid_ext[2 + i][j], 4);
                k += 4;
            }
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
    
    return ICUID_OK;
}
