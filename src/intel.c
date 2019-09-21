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
    /* Atom */

    /* Bonnell (45nm) */
    { 0x06,   NA,   NA, 0x1C,   NA,    "Bonnell"                },
    { 0x06,   NA,   NA, 0x26,   NA,    "Lincroft"               },

    /* Saltwell (32nm) */
    { 0x06,   NA,   NA, 0x35,   NA,    "Cloverview"             },
    { 0x06,   NA,   NA, 0x36,   NA,    "Cedarview"              },
    { 0x06,   NA,   NA, 0x27,   NA,    "Penwell"                },

    /* Silvermont (22nm)*/
    { 0x06,   NA,   NA, 0x5D,   NA,    "SoFIA"                  },
    { 0x06,   NA,   NA, 0x5A,   NA,    "Anniedale"              },
    { 0x06,   NA,   NA, 0x4D,   NA,    "Silvermont"             },
    { 0x06,   NA,   NA, 0x4A,   NA,    "Tangier"                },
    { 0x06,   NA,   NA, 0x37,   NA,    "Bay Trail"              },

    /* Moorefield (22nm) */
    { 0x06,   NA,   NA, 0x37,   NA,    "Moorefield"             },

    /* Airmont (14nm) */
    { 0x06,   NA,   NA, 0x4C,   NA,    "Airmont"                },

    /* Goldmont (14nm) */
    { 0x06,   NA,   NA, 0x5C,   NA,    "Apollo Lake"            },
    { 0x06,   NA,   NA, 0x5F,   NA,    "Denverton"              },

    /* Goldmont Plus (14nm) */
    { 0x06,   NA,   NA, 0x7A,   NA,    "Gemini Lake"            },

    /* Tremont (10nm) */
    { 0x06,   NA,   NA, 0x86,   NA,    "Tremont"                },

    /* 90nm */
    { 0x0F,   NA, 0x03, 0x00,   NA,    "Prescott"               },
    { 0x0F,   NA, 0x04, 0x00,   NA,    "Prescott"               },

    /* 65nm */
    { 0x0F,   NA, 0x06, 0x00,   NA,    "Presler"                },
    { 0x06,   NA, 0x0F, 0x00,   NA,    "Merom"                  },
    { 0x06,   NA, 0x16, 0x00,   NA,    "Merom"                  },

    /* 45nm */
    { 0x06,   NA,   NA, 0x1D,   NA,    "Dunnington (MP)"        },
    { 0x06,   NA,   NA, 0x17,   NA,    "Penryn"                 },
    { 0x06,   NA,   NA, 0x1A,   NA,    "Nehalem"                },
    { 0x06,   NA,   NA, 0x1E,   NA,    "Clarksfield"            },
    { 0x06,   NA,   NA, 0x2E,   NA,    "Nehalem EX"             },

    /* 32nm */
    { 0x06,   NA,   NA, 0x25,   NA,    "Westmere"               },
    { 0x06,   NA,   NA, 0x2C,   NA,    "Westmere"               },
    { 0x06,   NA,   NA, 0x2F,   NA,    "Westmere EX"            },

    { 0x06,   NA,   NA, 0x2A,   NA,    "Sandy Bridge"           },
    { 0x06,   NA,   NA, 0x2D,   NA,    "Sandy Bridge-E[NP]"     },

    /* Ivy Bridge (22nm) */
    { 0x06,   NA,   NA, 0x3A,   NA,    "Ivy Bridge"             },
    { 0x06,   NA,   NA, 0x2B,   NA,    "Ivy Bridge LGA 2011"    },
    { 0x06,   NA,   NA, 0x3E,   NA,    "Ivy Bridge E"           },

    /* Haswell (22nm) */
    { 0x06,   NA,   NA, 0x3C,   NA,    "Haswell"                },
    { 0x06,   NA,   NA, 0x3F,   NA,    "Haswell-E"              },
    { 0x06,   NA,   NA, 0x45,   NA,    "Haswell-ULT"            },
    { 0x06,   NA,   NA, 0x46,   NA,    "Crystal Well"           },

    /* Broadwell (14nm) */
    { 0x06,   NA,   NA, 0x3D,   NA,    "Broadwell"              },
    { 0x06,   NA,   NA, 0x47,   NA,    "Broadwell"              },
    { 0x06,   NA,   NA, 0x4F,   NA,    "Broadwell"              },
    { 0x06,   NA,   NA, 0x56,   NA,    "Broadwell"              },

    /* Skylake (14nm) */
    { 0x06,   NA,   NA, 0x4E,   NA,    "Skylake"                },
    { 0x06,   NA,   NA, 0x5E,   NA,    "Skylake"                },

    /* Knights Landing (14nm) */
    { 0x06,   NA,   NA, 0x75,   NA,    "Knights Landing"        },
    { 0x06,   NA,   NA, 0x58,   NA,    "Knights Mill"           },

    /* Kaby Lake (14nm) */
    { 0x06,   NA,   NA, 0x8E,   NA,    "Kaby Lake"              },

    /* Coffee Lake (14nm) */
    { 0x06,   NA,   NA, 0x9E,   NA,    "Coffee Lake"            },

    /* Cannon Lake (14nm) */
    { 0x06,   NA,   NA, 0x66,   NA,    "Cannon Lake"            },

    /* Ice Lake (10nm) */
    { 0x06,   NA,   NA, 0x7D,   NA,    "Ice Lake"               },
    { 0x06,   NA,   NA, 0x7E,   NA,    "Ice Lake"               },
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

void read_intel_data(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    get_intel_number_cores(raw, data);
    get_intel_deterministic_cacheinfo(raw, data);
    get_intel_codename(data);
}
