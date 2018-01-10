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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <icuid/icuid.h>

#include "features.h"
#include "internal.h"
#include "intel.h"
#include "match.h"

typedef enum {
    Lnone, /*!< Indicates error */
    L1I,   /*!< L1 Instruction Cache */
    L1D,   /*!< L1 Data Cache */
    L2,    /*!< L2 Cache */
    L3,    /*!< L3 Cache */
    L4,    /*!< L4 Cache */
} cache_type_t;

static void set_cache_info(cpuid_data_t *data, const cache_type_t cache,
                           const uint32_t size, const uint32_t associativity,
                           const uint32_t linesize)
{
    
    switch (cache) {
        case L1I:
            data->l1_instruction_cache = size;
            break;
        case L1D:
            data->l1_data_cache = size;
            data->l1_associativity = associativity;
            data->l1_cacheline = linesize;
            break;
        case L2:
            data->l2_cache = size;
            data->l2_associativity = associativity;
            data->l2_cacheline = linesize;
            break;
        case L3:
            data->l3_cache = size;
            data->l3_associativity = associativity;
            data->l3_cacheline = linesize;
            break;
        case L4:
            data->l4_cache = size;
            data->l4_associativity = associativity;
            data->l4_cacheline = linesize;
            break;
        default:
            break;
    }
}

/**
 * Intel Deterministic Cache Method
 */
static void get_intel_deterministic_cacheinfo(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    uint32_t idx;
    uint32_t associativity, partitions, linesize, sets, size, level, cache_type;
    cache_type_t type;
    for (idx = 0; idx < raw->max_intel_dc_level; idx++) {
        type = Lnone;
        cache_type = raw->intel_dc[idx][eax] & 0x1F;
        if (cache_type == 0) /* Check validity */
            break;
        level = (raw->intel_dc[idx][eax] >> 5) & 0x7;
        switch (level) {
            case 1:
                if (cache_type == 1)
                    type = L1D;
                else if (cache_type == 2)
                    type = L1I;
                break;
            case 2:
                if (cache_type == 3)
                    type = L2;
                break;
            case 3:
                if (cache_type == 3)
                    type = L3;
                break;
            case 4:
                if (cache_type == 3)
                    type = L4;
                break;
            default:
                break;
        }
        associativity = (raw->intel_dc[idx][ebx] >> 22) + 1;
        partitions = ((raw->intel_dc[idx][ebx] >> 12) & 0x3FF) + 1;
        linesize = (raw->intel_dc[idx][ebx] & 0xFFF) + 1;
        sets = raw->intel_dc[idx][ecx] + 1;
        size = (linesize * sets * associativity * partitions) >> 10; /* Size in KB */
        set_cache_info(data, type, size, associativity, linesize);
    }
}

/**
 * Read Extended Topology Information
 */
static int read_intel_extended_topology(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    uint32_t i;
    uint32_t smt = 0, cores = 0, level_type;
    for (i = 0; i < raw->max_intel_et_level; i++) {
        level_type = (raw->intel_et[i][ecx] >> 8) & 0xFF;
        switch (level_type) {
            case THREAD:
                smt = raw->intel_et[i][ebx] & 0xFFFF;
                break;
            case CORE:
                cores = raw->intel_et[i][ebx] & 0xFFFF;
                break;
            default:
                break;
        }
    }
    if (!smt || !cores)
        return 0;
    data->cores = cores / smt;
    data->logical_cpus = cores;
    return 1;
}

/**
 * Get the number of cores
 */
static void get_intel_number_cores(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    uint32_t cores = 0, logical_cpus = 0;
    if (data->cpuid_max_basic >= 0xB) {
        if (read_intel_extended_topology(raw, data))
            return;
    }

    if (data->cpuid_max_basic >= 1) {
        logical_cpus = (raw->cpuid[1][ebx] >> 16) & 0xFF;
        if (raw->cpuid[0][0] >= 4)
            cores = 1 + ((raw->cpuid[4][eax] >> 26) & 0x3F);
    }
    if (data->flags[CPU_FEATURE_HT]) {
        if (cores > 1) {
            data->cores = cores;
            data->logical_cpus = logical_cpus;
        } else {
            data->cores = 1;
            data->logical_cpus = (logical_cpus >= 2 ? logical_cpus : 1);
            if (data->logical_cpus == 1)
                data->flags[CPU_FEATURE_HT] = 0;
        }
    } else {
        data->cores = data->logical_cpus = 1;
    }
}

const match_codename_t codename_intel_t[] = {
    /* i486 */
    { 0x04, 0x00, 0x00, NA, NA,   "i486 DX-25/33"         },
    { 0x04, 0x00, 0x01, NA, NA,    "i486 DX-50"            },
    { 0x04, 0x00, 0x02, NA, NA,    "i486 SX"               },
    { 0x04, 0x00, 0x03, NA, NA,    "i486 DX2"              },
    { 0x04, 0x00, 0x04, NA, NA,    "i486 SL"               },
    { 0x04, 0x00, 0x05, NA, NA,    "i486 SX2"              },
    { 0x04, 0x00, 0x07, NA, NA,    "i486 DX2 WriteBack"    },
    { 0x04, 0x00, 0x08, NA, NA,    "i486 DX4"              },
    { 0x04, 0x00, 0x09, NA, NA,    "i486 DX4 WriteBack"    },

    /* Pentium 1 - P5 based */
    { 0x05, 0x00, 0x00, NA, NA,    "Pentium A-Step"         },
    { 0x05, 0x00, 0x01, NA, NA,    "Pentium 1 (0.8u)"       },
    { 0x05, 0x00, 0x02, NA, NA,    "Pentium 1 (0.35u)"      },
    { 0x05, 0x00, 0x03, NA, NA,    "Pentium OverDrive"      },
    { 0x05, 0x00, 0x04, NA, NA,    "Pentium 1 (0.35u)"      },
    { 0x05, 0x00, 0x07, NA, NA,    "Pentium 1 (0.35u)"      },
    { 0x05, 0x00, 0x08, NA, NA,    "Pentium MMX (0.25u)"    },
    { 0x05, 0x00, 0x09, NA, NA,    "Clanton"                },

    /* Atom */
    /* Moorefield (22nm) */
    { 0x06, 0x00, 0x37, NA, NA,    "Moorefield"             },
    /* Cherry Trail (14nm) */
    { 0x06, 0x00, 0x35, NA, NA,    "Cherry Trail"           },

    /* 90nm */
    { 0x0F, 0x00, 0x03, NA, NA,    "Prescott"               },
    { 0x0F, 0x00, 0x04, NA, NA,    "Prescott"               },
    /* 65nm */
    { 0x0F, 0x00, 0x06, NA, NA,    "Presler"                },
    { 0x06, 0x00, 0x0F, NA, NA,    "Merom"                  },
    { 0x06, 0x00, 0x16, NA, NA,    "Merom"                  },
    /* 45nm */
    { 0x06, 0x1D, NA, NA, NA,    "Dunnington (MP)"        },
    { 0x06, 0x17, NA, NA, NA,    "Penryn"                 },
    { 0x06, 0x1A, NA, NA, NA,    "Nehalem"                },
    { 0x06, 0x1E, NA, NA, NA,    "Nehalem"                },
    { 0x06, 0x2E, NA, NA, NA,    "Nehalem EX"             },
    /* 32nm */
    { 0x06, 0x25, NA, NA, NA,    "Westmere"               },
    { 0x06, 0x2C, NA, NA, NA,    "Westmere"               },
    { 0x06, 0x2F, NA, NA, NA,    "Westmere EX"            },

    { 0x06, 0x2A, NA, NA, NA,    "Sandy Bridge"           },
    { 0x06, 0x2D, NA, NA, NA,    "Sandy Bridge-E[NP]"     },
    /* Ivy Bridge (22nm) */
    { 0x06, 0x3A, NA, NA, NA,    "Ivy Bridge"             },
    { 0x06, 0x2B, NA, NA, NA,    "Ivy Bridge LGA 2011"    },
    { 0x06, 0x3E, NA, NA, NA,    "Ivy Bridge E"           },

    /* Haswell (22nm) */
    { 0x06, 0x3C, NA, NA, NA,    "Haswell"                },
    { 0x06, 0x3F, NA, NA, NA,    "Haswell-E"              },
    { 0x06, 0x45, NA, NA, NA,    "Haswell-ULT"            },
    { 0x06, 0x46, NA, NA, NA,    "Crystal Well"           },

    /* Broadwell (14nm) */
    { 0x06, 0x3D, NA, NA, NA,    "Broadwell"              },
    { 0x06, 0x47, NA, NA, NA,    "Broadwell"              },
    { 0x06, 0x4F, NA, NA, NA,    "Broadwell"              },
    { 0x06, 0x56, NA, NA, NA,    "Broadwell"              },

    /* Skylake (14nm) */
    { 0x06, 0x4E, NA, NA, NA,    "Skylake"                },
    { 0x06, 0x5E, NA, NA, NA,    "Skylake"                },

    /* Knights Landing (14nm) */
    { 0x06, 0x57, NA, NA, NA,    "Knights Landing"        },

    /* Kaby Lake (14nm) */
    { 0x06, 0x8E, NA, NA, NA,    "Kaby Lake"              },

    /* Coffee Lake (14nm) */
    { 0x06, 0x9E, NA, NA, NA,    "Coffee Lake"            },
};

/**
 * Get codename
 * N.B. Not in spec so custom method is used
 * The codename will be selected based on data in the above table
 */
static void get_intel_codename(cpuid_data_t *data)
{
    if (data->cpuid_max_basic < 1)
        return;

    cpu_to_codename(data, codename_intel_t, NELEMS(codename_intel_t));
}

/**
 * Load Intel specific features
 */
static void get_intel_features(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    const cpuid_feature_map_t regidmap_ecx01[] = {
        {  2, CPU_FEATURE_DTS64 },
        {  4, CPU_FEATURE_DS_CPL },
        {  5, CPU_FEATURE_VMX },
        {  6, CPU_FEATURE_SMX },
        {  7, CPU_FEATURE_EST },
        {  8, CPU_FEATURE_TM2 },
        { 10, CPU_FEATURE_CID },
        { 11, CPU_FEATURE_SDBG },
        { 14, CPU_FEATURE_XTPR },
        { 15, CPU_FEATURE_PDCM },
        { 17, CPU_FEATURE_PCID },
        { 18, CPU_FEATURE_DCA },
        { 21, CPU_FEATURE_X2APIC },
        { 24, CPU_FEATURE_TSC_DEADLINE },
    };
    const cpuid_feature_map_t regidmap_edx01[] = {
        { 18, CPU_FEATURE_PN },
        { 21, CPU_FEATURE_DTS },
        { 22, CPU_FEATURE_ACPI },
        { 27, CPU_FEATURE_SS },
        { 29, CPU_FEATURE_TM },
        { 30, CPU_FEATURE_IA64 },
        { 31, CPU_FEATURE_PBE },
    };
    const cpuid_feature_map_t regidmap_ebx07[] = {
        {  1, CPU_FEATURE_TSC_ADJUST },
        {  2, CPU_FEATURE_SGX },
        {  4, CPU_FEATURE_HLE },
        {  9, CPU_FEATURE_ERMS },
        { 10, CPU_FEATURE_INVPCID },
        { 11, CPU_FEATURE_RTM },
        { 12, CPU_FEATURE_CQM },
        { 14, CPU_FEATURE_MPX },
        { 16, CPU_FEATURE_AVX512F },
        { 17, CPU_FEATURE_AVX512DQ },
        { 18, CPU_FEATURE_RDSEED },
        { 19, CPU_FEATURE_ADX },
        { 20, CPU_FEATURE_SMAP },
        { 22, CPU_FEATURE_PCOMMIT },
        { 23, CPU_FEATURE_CLFLUSHOPT },
        { 24, CPU_FEATURE_CLWB },
        { 26, CPU_FEATURE_AVX512PF },
        { 27, CPU_FEATURE_AVX512ER },
        { 28, CPU_FEATURE_AVX512CD },
        { 29, CPU_FEATURE_SHA },
        { 30, CPU_FEATURE_AVX512BW },
        { 31, CPU_FEATURE_AVX512VL },
    };
    if (data->cpuid_max_basic < 1)
        return;
    set_feature_bits(data, regidmap_ecx01, NELEMS(regidmap_ecx01), raw->cpuid[1][ecx]);
    set_feature_bits(data, regidmap_edx01, NELEMS(regidmap_edx01), raw->cpuid[1][edx]);
    if (data->cpuid_max_basic < 7)
        return;
    set_feature_bits(data, regidmap_ebx07, NELEMS(regidmap_ebx07), raw->cpuid[7][ebx]);
}

void read_intel_data(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    get_intel_features(raw, data);
    get_intel_number_cores(raw, data);
    get_intel_deterministic_cacheinfo(raw, data);
    get_intel_codename(data);
}
