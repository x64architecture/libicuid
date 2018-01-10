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
            cores = 1 + (raw->cpuid_ext[8][ecx] & 0xFF);
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
    /* 486 */
    { 4, 3,    NA, NA, NA, "Am486 DX2WT"                   },
    { 4, 7,    NA, NA, NA, "Am486 DX2WB"                   },
    { 4, 8,    NA, NA, NA, "Am486 DX4WT"                   },
    { 4, 9,    NA, NA, NA, "Am486 DX4WB"                   },
    { 4, 10,   NA, NA, NA, "Elan SC4xx"                    },
    { 4, 14,   NA, NA, NA, "Am5x86WT"                      },
    { 4, 15,   NA, NA, NA, "Am5x86WB"                      },

    /* K5 */
    { 5, NA,   NA, NA, NA, "Unknown K5"                    },
    { 5, 0,    NA, NA, NA, "K5"                            },
    { 5, 1,    NA, NA, NA, "K5"                            },
    { 5, 2,    NA, NA, NA, "K5"                            },
    { 5, 3,    NA, NA, NA, "K5"                            },

    /* K6 */
    { 5, 6,    NA, NA, NA, "K6"                            },
    { 5, 7,    NA, NA, NA, "K6"                            },
    { 5, 8,    NA, NA, NA, "K6-2"                          },
    { 5, 9,    NA, NA, NA, "K6-III"                        },
    { 5, 13,   NA, NA, NA, "K6-2+"                         },

    /* K7 */
    { 6, 1,    NA, NA, NA, "K7"                            },
    { 6, 2,    NA, NA, NA, "K7"                            },
    { 6, 3,    NA, NA, NA, "K7"                            },
    { 6, 4,    NA, NA, NA, "K7"                            },
    { 6, 6,    NA, NA, NA, "K7"                            },
    { 6, 7,    NA, NA, NA, "K7"                            },
    { 6, 8,    NA, NA, NA, "K7"                            },
    { 6, 10,   NA, NA, NA, "K7"                            },

    /* K8/K9 */
    { 15, NA,  NA, 15, NA, "Unknown K8"                    },
    { 15, NA,  NA, 16, NA, "Unknown K9"                    },
    { 15, 4,   NA, NA, NA, "K8"                            },
    { 15, 5,   NA, NA, NA, "K8"                            },
    { 15, 7,   NA, NA, NA, "K8 (Athon 64)"                 },
    { 15, 8,   NA, NA, NA, "K8"                            },
    { 15, 11,  NA, NA, NA, "K8 (Athlon 64)"                },
    { 15, 12,  NA, NA, NA, "K8"                            },
    { 15, 14,  NA, NA, NA, "K8"                            },
    { 15, 15,  NA, NA, NA, "K8"                            },
    { 15, 20,  NA, NA, NA, "K8"                            },
    { 15, 21,  NA, NA, NA, "K8"                            },
    { 15, 23,  NA, NA, NA, "K8"                            },
    { 15, 27,  NA, NA, NA, "K8 (Athlon 64)"                },
    { 15, 28,  NA, NA, NA, "K8"                            },
    { 15, 31,  NA, NA, NA, "K8"                            },
    { 15, 33,  NA, NA, NA, "K8 (rev. E)"                   },
    { 15, 35,  NA, NA, NA, "Opteron K8 (rev. E)"           },
    { 15, 36,  NA, NA, NA, "K8 (rev. E)"                   },
    { 15, 37,  NA, NA, NA, "K8 (rev. E)"                   },
    { 15, 43,  NA, NA, NA, "Athlon 64 X2 K8 (rev. E)"      },
    { 15, 44,  NA, NA, NA, "K8 (rev. E)"                   },
    { 15, 47,  NA, NA, NA, "K8 (rev. E)"                   },
    { 15, 65,  NA, NA, NA, "Opteron K8 (rev. F+)"          },
    { 15, 67,  NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 72,  NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 75,  NA, NA, NA, "Athlon 64 X2 K8 (rev. F+)"     },
    { 15, 76,  NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 79,  NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 93,  NA, NA, NA, "Opteron K8 (rev. F+)"          },
    { 15, 95,  NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 104, NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 107, NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 108, NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 111, NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 124, NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 127, NA, NA, NA, "K8 (rev. F+)"                  },
    { 15, 193, NA, NA, NA, "Athlon 64 FX DC K8 (rev. F+)"  },

    /* K10 */
    { 10, NA,  NA, NA, NA, "Unknown K10"                   },
    { 10, 4,   NA, NA, NA, "K10"                           },
    { 10, 5,   NA, NA, NA, "K10"                           },
    { 10, 6,   NA, NA, NA, "K10"                           },
    { 10, 8,   NA, NA, NA, "K10"                           },
    { 10, 9,   NA, NA, NA, "K10"                           },
    { 10, 10,  NA, NA, NA, "K10"                           },

    { 11, 3,   NA, NA, NA, "K8 (rev. E+)"                  },

    { 12, 1,   NA, NA, NA, "K10"                           },

    { 14, NA,  NA, NA, NA, "Unknown Bobcat"                },
    { 14, 1,   NA, NA, NA, "Bobcat"                        },
    { 14, 2,   NA, NA, NA, "Bobcat"                        },

    { 15, 1,   NA, NA, NA, "Bulldozer"                     },
    { 15, 2,   NA, NA, NA, "Piledriver"                    },
    { 15, 16,  NA, NA, NA, "Piledriver"                    },
    { 15, 19,  NA, NA, NA, "Piledriver"                    },
    { 15, 48,  NA, NA, NA, "Steamroller"                   },

    { 16, 0,   NA, NA, NA, "Jaguar"                        },
    { 16, 30,  NA, NA, NA, "Jaguar (Puma)"                 },

    { 17, NA,  NA, NA, NA, "Zen"                           },
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
