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

#include <icuid/icuid.h>

#include "features.h"
#include "internal.h"
#include "match.h"

/**
 * Get number of cores
 */
static void get_amd_number_cores(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    uint32_t logical_cpus = 0, cores = 0;
    
    if (data->cpuid_max_basic >= 1) {
        logical_cpus = (raw->cpuid[1][ebx] >> 16) & 0xFF;
        if (raw->cpuid_ext[0][0] >= 8) {
            cores = (raw->cpuid_ext[8][ecx] >> 12) & 0xF; // ApicIdCoreIdSize
        }
    }
    if (data->flags[CPU_FEATURE_HT]) {
        if (cores > 1) {
            data->cores = cores;
            data->logical_cpus = logical_cpus;
        } else {
            data->cores = 1;
            data->logical_cpus = (logical_cpus >= 2 ? logical_cpus : 2);
        }
    } else {
        data->cores = data->logical_cpus = 1;
    }
}

const match_codename_t codename_amd_t[] = {
    /* Zen */

    { 0x0F, 0x08, 0x01, 0x00,   NA, "Zen"             },
    { 0x0F, 0x08, 0x01, 0x01,   NA, "Raven Ridge"     },

    /* Zen+ */

    { 0x0F, 0x17, 0x08, 0x18,   NA, "Picasso"         },
    { 0x0F, 0x17, 0x08, 0x08,   NA, "Pinnacle Ridge"  },

    /* Zen 2 */

    { 0x0F, 0x17, 0x08, 0x71,   NA, "Matisse"         },
};

/**
 * Get codename
 * N.B. Not in spec so custom method is used
 * The codename will be selected based on data in the above table
 */
static void get_amd_codename(cpuid_data_t *data)
{
    if (data->cpuid_max_basic < 1)
        return;

    cpu_to_codename(data, codename_amd_t, NELEMS(codename_amd_t));
}

/**
 * Get Cache Info
 */
static void get_amd_cache_info(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    uint32_t l3_result;
    const uint32_t assoc_table[16] = {
        /* 0x0 - 0xF */
        0, 1, 2, 0, 4, 0, 8, 0, 16, 0, 32, 48, 64, 96, 128, 255
    };
    
    if (data->cpuid_max_ext >= 0x80000005) {
        data->l1_data_cache = (raw->cpuid_ext[5][ecx] >> 24) & 0xFF;
        data->l1_associativity = (raw->cpuid_ext[5][ecx] >> 16) & 0xFF;
        data->l1_cacheline = (raw->cpuid_ext[5][ecx]) & 0xFF;
        data->l1_instruction_cache = (raw->cpuid_ext[5][edx] >> 24) & 0xFF;
    }

    if (data->cpuid_max_ext < 0x80000006)
        return;

    data->l2_cache = (raw->cpuid_ext[6][ecx] >> 16) & 0xFFFF;
    data->l2_associativity = assoc_table[(raw->cpuid_ext[6][ecx] >> 12) & 0xF];
    data->l2_cacheline = (raw->cpuid_ext[6][ecx]) & 0xFF;
      
    l3_result = (raw->cpuid_ext[6][3] >> 18);
    if (l3_result > 0) {
        data->l3_cache = l3_result * 512; /* Size in KB */
        data->l3_associativity = assoc_table[(raw->cpuid_ext[6][edx] >> 12) & 0xF];
        data->l3_cacheline = (raw->cpuid_ext[6][edx]) & 0xFF;
    } else {
        data->l3_cache = 0;
    }
}

static void get_amd_features(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    const cpuid_feature_map_t regidmap_ecx81[] = {
        {  1, CPU_FEATURE_CMP_LEGACY },
        {  2, CPU_FEATURE_SVM },
        {  3, CPU_FEATURE_EXTAPIC },
        {  4, CPU_FEATURE_CR8_LEGACY },
        {  5, CPU_FEATURE_ABM },
        {  6, CPU_FEATURE_SSE4A },
        {  7, CPU_FEATURE_MISALIGNSSE },
        {  8, CPU_FEATURE_3DNOWPREFETCH },
        {  9, CPU_FEATURE_OSVW },
        { 10, CPU_FEATURE_IBS },
        { 11, CPU_FEATURE_XOP },
        { 12, CPU_FEATURE_SKINIT },
        { 13, CPU_FEATURE_WDT },
        { 15, CPU_FEATURE_LWP },
        { 16, CPU_FEATURE_FMA4 },
        { 17, CPU_FEATURE_TCE },
        { 19, CPU_FEATURE_NODEID_MSR },
        { 21, CPU_FEATURE_TBM },
        { 22, CPU_FEATURE_TOPOEXT },
        { 23, CPU_FEATURE_PERFCTR_CORE },
        { 24, CPU_FEATURE_PERFCTR_NB },
        { 26, CPU_FEATURE_BPEXT },
        { 28, CPU_FEATURE_PERFCTR_L2 },
        { 29, CPU_FEATURE_MONITORX },
    };
    const cpuid_feature_map_t regidmap_edx81[] = {
        { 22, CPU_FEATURE_MMXEXT },
        { 25, CPU_FEATURE_FXSR_OPT },
        { 30, CPU_FEATURE_3DNOWEXT },
        { 31, CPU_FEATURE_3DNOW },
    };
    const cpuid_feature_map_t regidmap_edx87[] = {
        {  0, CPU_FEATURE_TS },
        {  1, CPU_FEATURE_FID },
        {  2, CPU_FEATURE_VID },
        {  3, CPU_FEATURE_TTP },
        {  4, CPU_FEATURE_TM_AMD },
        {  5, CPU_FEATURE_STC },
        {  6, CPU_FEATURE_100MHZSTEPS },
        {  7, CPU_FEATURE_HWPSTATE },
        {  9, CPU_FEATURE_CPB },
        { 10, CPU_FEATURE_APERFMPERF },
        { 11, CPU_FEATURE_PFI },
        { 12, CPU_FEATURE_PA },
    };
    const cpuid_feature_map_t regidmap_ebx88[] = {
        {  0, CPU_FEATURE_CLZERO },
        {  1, CPU_FEATURE_IRPERF },
    };
    const cpuid_feature_map_t regidmap_eax_8000_1F[] = {
        {  0, CPU_FEATURE_SME },
    };

    if (data->cpuid_max_ext < 0x80000001)
        return;
    set_feature_bits(data, regidmap_edx81, NELEMS(regidmap_edx81), raw->cpuid_ext[1][edx]);
    set_feature_bits(data, regidmap_ecx81, NELEMS(regidmap_ecx81), raw->cpuid_ext[1][ecx]);

    if (data->cpuid_max_ext < 0x80000007)
        return;
    set_feature_bits(data, regidmap_edx87, NELEMS(regidmap_edx87), raw->cpuid_ext[7][edx]);

    if (data->cpuid_max_ext < 0x80000008)
        return;
    set_feature_bits(data, regidmap_ebx88, NELEMS(regidmap_ebx88), raw->cpuid_ext[8][ebx]);

    if (data->cpuid_max_ext < 0x8000001F)
        return;
    set_feature_bits(data, regidmap_eax_8000_1F, NELEMS(regidmap_eax_8000_1F), raw->cpuid_ext[31][eax]);
}

void read_amd_data(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    get_amd_features(raw, data);
    get_amd_number_cores(raw, data);
    get_amd_cache_info(raw, data);
    get_amd_codename(data);
}
