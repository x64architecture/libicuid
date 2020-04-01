/*
 * Copyright (c) 2015 - 2019, Kurt Cancemi (kurt@x64architecture.com)
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
            cores = (raw->cpuid_ext[8][ecx] >> 12) & 0xF; /* ApicIdCoreIdSize */
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

void read_amd_data(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    get_amd_number_cores(raw, data);
    get_amd_cache_info(raw, data);
    get_amd_codename(data);
}
