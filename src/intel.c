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
#include <internal/stdcompat.h>

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
} cache_type_t;

static void set_cache_info(cpuid_data_t *data, const cache_type_t cache,
                           const uint32_t size, const uint32_t associativity,
                           const uint32_t linesize)
{
    
    switch (cache) {
        case Lnone:
            break;
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
        default:
            break;
    }
}

/**
 * Intel Deterministic Cache Method
 */
static void get_intel_deterministic_cacheinfo(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    uint32_t ecx;
    uint32_t associativity, partitions, linesize, sets, size, level, cache_type;
    cache_type_t type = Lnone;
    for (ecx = 0; ecx < 4; ecx++) {
        cache_type = raw->intel_dc[ecx][0] & 0x1F;
        if (cache_type == 0) /* Check validity */
            break;
        level = (raw->intel_dc[ecx][0] >> 5) & 0x7;
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
            default:
                fprintf(stderr, "Error level/type\n");
        }
        associativity = (raw->intel_dc[ecx][1] >> 22) + 1;
        partitions = ((raw->intel_dc[ecx][1] >> 12) & 0x3FF) + 1;
        linesize = (raw->intel_dc[ecx][1] & 0xFFF) + 1;
        sets = raw->intel_dc[ecx][2] + 1;
        size = (linesize * sets * associativity * partitions) >> 10; /* Size in kB */
        set_cache_info(data, type, size, associativity, linesize);
    }
}

/**
 * Read Extended Topology Information
 */
static int read_intel_extended_topology(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    int i;
    uint32_t smt = 0, cores = 0, level_type;
    for (i = 0; i < 4; i++) {
        level_type = (raw->intel_et[i][2] >> 8) & 0xFF;
        switch (level_type) {
            case INVALID:
                break;
            case THREAD:
                smt = raw->intel_et[i][1] & 0xFFFF;
                break;
            case CORE:
                cores = raw->intel_et[i][1] & 0xFFFF;
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
    if (data->cpuid_max_basic >= 11) {
        if (read_intel_extended_topology(raw, data))
            return;
    }

    if (data->cpuid_max_basic >= 1) {
        logical_cpus = (raw->cpuid[1][1] >> 16) & 0xFF;
        if (raw->cpuid[0][0] >= 4)
            cores = 1 + ((raw->cpuid[4][0] >> 26) & 0x3F);
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

static intel_uarch_t brand_string_method(const cpuid_data_t *data)
{
    intel_uarch_t uarch = NO_CODE;
    const char *bs = data->brand_str;
    char *p = NULL;
    unsigned int i, need_table = 1;

    const struct {
        intel_uarch_t uarch;
        const char *name;
    } uarch_t[] = {
        { ATOM_DIAMONDVILLE, "Atom(TM) CPU [N ][23]## " },
        { ATOM_SILVERTHORNE, "Atom(TM) CPU Z" },
        { ATOM_PINEVIEW,     "Atom(TM) CPU D" },
        { ATOM_CEDARVIEW,    "Atom(TM) CPU N####" },
        { ATOM,              "Atom(TM) CPU" },
        { CELERON,           "Celeron" },
        { CORE_SOLO,         "Genuine Intel(R) CPU" },
        { CORE_SOLO,         "Intel(R) Core(TM)" },
        { MOBILE_PENTIUM_M,  "Pentium(R) M" },
        { CORE_SOLO,         "Pentium(R) Dual  CPU" },
        { CORE_SOLO,         "Pentium(R) Dual-Core" },
        { PENTIUM_D,         "Pentium(R) D" },
        { PENTIUM,           "Pentium" },
        { XEONMP,            "Xeon MP" },
        { XEONMP,            "Xeon(TM) MP" },
        { XEON,              "Xeon" },
    };

    if ((p = strstr(bs, "Core(TM) i")) != NULL) {
        p += 10;
        need_table = 0;
        uarch = CORE_I3;

        switch (*p) {
            case '3':
                uarch = uarch + 0;
                break;
            case '5':
                uarch = uarch + 1;
                break;
            case '7':
                uarch = uarch + 2;
                break;
        }

        if (strstr(bs, "4690K") != NULL || strstr(bs, "4790K") != NULL)
            uarch = DEVILSCANYON;
    }
    if (strstr(bs, "Mobile") != NULL) {
        need_table = 0;
        if (strstr(bs, "Celeron"))
            uarch = MOBILE_CELERON;
        else if (strstr(bs, "Pentium") != NULL)
            uarch = MOBILE_PENTIUM;
    }
    if (need_table) {
        for (i = 0; i < NELEMS(uarch_t); i++)
            if (match_pattern(bs, uarch_t[i].name)) {
                uarch = uarch_t[i].uarch;
                break;
            }
    }
    if (uarch == CORE_SOLO) {
        if ((p = strstr(bs, "CPU")) != NULL) {
            p += 3;
            while (*p == ' ')
                p++;
            if (*p == 'T') {
                if (data->cores == 1)
                    uarch = MOBILE_CORE_SOLO;
                else
                    uarch = MOBILE_CORE_DUO;
            }
        }
    }
    if (uarch == CORE_SOLO) {
        switch (data->cores) {
            case 1:
                break;
            case 2:
                uarch = CORE_DUO;
                if (data->logical_cpus > 2)
                    uarch = DUAL_CORE_HT;
                break;
            case 4:
                uarch = QUAD_CORE;
                if (data->logical_cpus > 4)
                    uarch = QUAD_CORE_HT;
                break;
            default:
                uarch = NO_CODE;
                break;
        }
    }
    
    if ((uarch == CORE_DUO || uarch == PENTIUM_D) && data->ext_model >= 23) {
        uarch = WOLFDALE;
    }
    if (uarch == MOBILE_CORE_DUO && data->model != 14) {
        if (data->ext_model < 23) {
            uarch = MEROM;
        } else {
            uarch = PENRYN;
        }
    }
    if (uarch == XEON) {
        if (match_pattern(bs, "W35##") || match_pattern(bs, "[ELXW]75##"))
            uarch = XEON_I7;
        else if (match_pattern(bs, "[ELXW]55##"))
            uarch = XEON_GAINESTOWN;
        else if (match_pattern(bs, "[ELXW]56##"))
            uarch = XEON_WESTMERE;
        else if (data->l3_cache > 0 && data->family == 16)
            uarch = XEON_IRWIN;
    }
    if (uarch == XEONMP && data->l3_cache > 0)
        uarch = XEON_POTOMAC;

    return uarch;
}

const match_uarch_t uarch_intel_t[] = {
    { NA, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown CPU uarch"  },

    /* i486 */
    {  4, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown i486"       },
    {  4,  0, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX-25/33"      },
    {  4,  1, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX-50"         },
    {  4,  2, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 SX"            },
    {  4,  3, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX2"           },
    {  4,  4, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 SL"            },
    {  4,  5, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 SX2"           },
    {  4,  7, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX2 WriteBack" },
    {  4,  8, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX4"           },
    {  4,  9, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX4 WriteBack" },

    /* Pentium 1 - P5 based */
    {  5, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Pentium"    },
    {  5,  0, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium A-Step"     },
    {  5,  1, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.8u)"   },
    {  5,  2, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.35u)"  },
    {  5,  3, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium OverDrive"  },
    {  5,  4, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.35u)"  },
    {  5,  7, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.35u)"  },
    {  5,  8, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium MMX (0.25u)" },
    {  5,  9, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Clanton"            },

    /* Pentium 2 / 3 / M - P6 based */
    {  6, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown P6"         },
    {  6,  0, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium Pro"        },
    {  6,  1, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium Pro"        },
    {  6,  3, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Klamath"            },
    {  6,  5, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Deschutes"          },
    {  6,  5, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Tonga"              },
    {  6,  6, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Dixon"              },

    {  6,  3, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Klamath"            },
    {  6,  5, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Drake"              },
    {  6,  6, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Dixon"              },

    {  6,  5, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Covingtons"         },
    {  6,  6, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Mendocino"          },

    {  6,  7, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Katmai"             },
    {  6,  8, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Coppermine"         },
    {  6, 10, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Coppermine"         },
    {  6, 11, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Tualatin"           },

    {  6,  7, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Tanner"             },
    {  6,  8, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Cascades"           },
    {  6, 10, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Cascades"           },
    {  6, 11, NA, NA, NA,   1,    NA,    NA, XEON              ,     "Tualatin"           },

    {  6,  7, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Katmai"             },
    {  6,  8, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Coppermine-128"     },
    {  6, 10, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Coppermine-128"     },
    {  6, 11, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Tualatin-256"       },

    /* Netburst based (Pentium 4 and later) classic P4s */
    { 15, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Pentium 4"  },
    { 15, NA, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Unknown P-4 Celeron" },
    { 15, NA, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Unknown Xeon"       },
    
    { 15,  0, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Willamette"         },
    { 15,  1, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Willamette"         },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Northwood"          },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Prescott"           },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Prescott"           },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Cedar Mill"         },

    /* server CPUs */
    { 15,  0, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Foster"             },
    { 15,  1, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Foster"             },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Prestonia"          },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, XEONMP            ,     "Gallatin"           },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Nocona"             },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Nocona"             },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEON_IRWIN        ,     "Irwindale"          },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEONMP            ,     "Cranford"           },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEON_POTOMAC      ,     "Potomac"            },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Dempsey"            },
    { 15, NA, NA, NA,  6,  NA,    NA,    NA, NO_CODE           ,     "Tulsa"              },

    /* Pentium Ds */
    { 15,  4,  4, 15, NA,   1,    NA,    NA, NO_CODE           ,     "SmithField"         },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, PENTIUM_D         ,     "SmithField"         },
    { 15,  4,  7, 15, NA,   1,    NA,    NA, NO_CODE           ,     "SmithField"         },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, PENTIUM_D         ,     "Presler"            },

    /* Celeron and Celeron Ds */
    { 15,  1, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Willamette-128"     },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Northwood-128"      },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Prescott-256"       },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Prescott-256"       },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Cedar Mill-512"     },
    {  6, 55, NA, NA, NA,   2,    NA,    NA, CELERON           ,     "Bay Trail-M"        },
    {  6, 55, NA, NA, NA,   4,    NA,    NA, CELERON           ,     "Bay Trail-D"        },

    /* Intel Core - P6-based */

    {  6,  9, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Pentium M"  },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM_M  ,     "Unknown Pentium M"  },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, PENTIUM           ,     "Banias"             },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM_M  ,     "Banias"             },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Celeron M"          },
    {  6, 13, NA, NA, NA,   1,    NA,    NA, PENTIUM           ,     "Dothan"             },
    {  6, 13, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM_M  ,     "Dothan"             },
    {  6, 13, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Celeron M"          },
    
    /* Intel Atom */
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM              ,     "Unknown Atom"       },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_DIAMONDVILLE ,     "Diamondville"       },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_SILVERTHORNE ,     "Silverthorne"       },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_CEDARVIEW    ,     "Cedarview"          },
    {  6,  6, NA, NA, NA,  NA,    NA,    NA, ATOM_CEDARVIEW    ,     "Cedarview"          },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_PINEVIEW     ,     "Pineview"           },
    {  6, 55, NA, NA, NA,  NA,    NA,    NA, ATOM              ,     "Bay Trail"          },
    {  6, 77, NA, NA, NA,  NA,    NA,    NA, NO_CODE           ,     "Avoton"             },
    {  6, NA, NA, NA, 38,  NA,    NA,    NA, NO_CODE           ,     "Tunnel Creek"       },

    {  6, 14, NA, NA, NA,   NA,   NA,    NA, NO_CODE           ,     "Yonah"              },

    {  6, 15, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Core 2"     },
    {  6, 15, NA, NA, NA,   2,  4096,    NA, CORE_DUO          ,     "Conroe"             },
    {  6, 15, NA, NA, NA,   2,  1024,    NA, CORE_DUO          ,     "Conroe"             },
    {  6, 15, NA, NA, NA,   2,   512,    NA, CORE_DUO          ,     "Conroe"             },
    {  6, 15, NA, NA, NA,   4,    NA,    NA, QUAD_CORE         ,     "Kentsfield"         },
    {  6, 15, NA, NA, NA,   4,  4096,    NA, QUAD_CORE         ,     "Kentsfield"         },
    {  6, 15, NA, NA, NA,   2,  2048,    NA, CORE_DUO          ,     "Allendale"          },
    {  6, 15, NA, NA, NA,   2,    NA,    NA, MOBILE_CORE_DUO   ,     "Merom"              },
    {  6, 15, NA, NA, NA,   2,  2048,    NA, MEROM             ,     "Merom"              },
    {  6, 15, NA, NA, NA,   2,  4096,    NA, MEROM             ,     "Merom"              },

    {  6, 15, NA, NA, 15,   1,    NA,    NA, CELERON           ,     "Celeron"            },
    {  6,  6, NA, NA, 22,   1,    NA,    NA, CELERON           ,     "Celeron"            },
    {  6, 15, NA, NA, 15,   2,    NA,    NA, CELERON           ,     "Allendale"          },
    {  6,  6, NA, NA, 22,   2,    NA,    NA, CELERON           ,     "Allendale"          },

    {  6,  7, NA, NA, 23,   2,  1024,    NA, WOLFDALE          ,     "Wolfdale"           },
    {  6,  7, NA, NA, 23,   2,  2048,    NA, WOLFDALE          ,     "Wolfdale"           },
    {  6,  7, NA, NA, 23,   2,  3072,    NA, WOLFDALE          ,     "Wolfdale"           },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, WOLFDALE          ,     "Wolfdale"           },
    {  6,  7, NA, NA, 23,   1,    NA,    NA, MOBILE_CORE_DUO   ,     "Penryn"             },
    {  6,  7, NA, NA, 23,   2,    NA,    NA, PENRYN            ,     "Penryn"             },
    {  6,  7, NA, NA, 23,   2,  3072,    NA, PENRYN            ,     "Penryn"             },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, PENRYN            ,     "Penryn"             },
    {  6,  7, NA, NA, 23,   4,  2048,    NA, QUAD_CORE         ,     "Yorkfield"          },
    {  6,  7, NA, NA, 23,   4,  3072,    NA, QUAD_CORE         ,     "Yorkfield"          },
    {  6,  7, NA, NA, 23,   4,  6144,    NA, QUAD_CORE         ,     "Yorkfield"          },
    
    /* Xeons - Core based */
    {  6, 14, NA, NA, 14,   1,    NA,    NA, XEON              ,     "LV"                 },
    {  6, 15, NA, NA, 15,   2,  4096,    NA, XEON              ,     "Woodcrest"          },
    {  6, 15, NA, NA, 15,   2,  2048,    NA, XEON              ,     "Conroe"             },
    {  6, 15, NA, NA, 15,   2,  4096,    NA, XEON              ,     "Conroe"             },
    {  6, 15, NA, NA, 15,   4,  4096,    NA, XEON              ,     "Kentsfield"         },
    {  6, 15, NA, NA, 15,   4,  4096,    NA, XEON              ,     "Clovertown"         },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, XEON              ,     "Wolfdale"           },
    {  6,  7, NA, NA, 23,   4,  6144,    NA, XEON              ,     "Harpertown"         },
    {  6,  7, NA, NA, 23,   4,  3072,    NA, XEON              ,     "Yorkfield"          },
    {  6,  7, NA, NA, 23,   4,  6144,    NA, XEON              ,     "Yorkfield"          },

    /* Nehalem (45nm) */
    {  6, NA, NA, NA, 46,   NA,   NA,    NA, NO_CODE           ,     "Beckton"            },
    {  6, 10, NA, NA, 26,   NA,   NA,    NA, XEON_GAINESTOWN   ,     "Gainestown"         },
    {  6, 10, NA, NA, 26,   NA,   NA,  4096, XEON_GAINESTOWN   ,     "Gainestown"         },
    {  6, 10, NA, NA, 26,   NA,   NA,  8192, XEON_GAINESTOWN   ,     "Gainestown"         },
    {  6, 10, NA, NA, 26,   NA,   NA,    NA, XEON_I7           ,     "Bloomfield"         },
    {  6, 10, NA, NA, 26,   NA,   NA,    NA, CORE_I7           ,     "Bloomfield"         },
    {  6, 10, NA, NA, 30,   NA,   NA,    NA, CORE_I7           ,     "Lynnfield"          },
    {  6,  5, NA, NA, 37,   NA,   NA,  8192, CORE_I5           ,     "Lynnfield"          },

    /* Westmere (32nm) */
    {  6, 12, NA, NA, 44,   NA,   NA,    NA, NO_CODE           ,     "Westmere"           },
    {  6, NA, NA, NA, 47,   NA,   NA,    NA, NO_CODE           ,     "Westmere-EX"        },
    {  6, 12, NA, NA, 44,   NA,   NA, 12288, NO_CODE           ,     "Gulftown"           },
    {  6,  5, NA, NA, 37,   NA,   NA,  4096, NO_CODE           ,     "Clarkdale"          },
    {  6,  5, NA, NA, 37,   NA,   NA,  4096, NO_CODE           ,     "Arrandale"          },
    {  6,  5, NA, NA, 37,   NA,   NA,  3072, NO_CODE           ,     "Arrandale"          },

    /* Sandy Bridge (32nm) */
    {  6, 10, NA, NA, 42,   NA,   NA,    NA, NO_CODE           ,     "Sandy Bridge"       },
    {  6, 13, NA, NA, 45,   NA,   NA,    NA, NO_CODE           ,     "Sandy Bridge-E"     },

    /* Ivy Bridge (22nm) */
    {  6, 10, NA, NA, 58,   NA,   NA,    NA, NO_CODE           ,     "Ivy Bridge"         },
    {  6, 14, NA, NA, 62,   NA,   NA,    NA, NO_CODE           ,     "Ivy Bridge-E"       },
    
    /* Haswell (22nm) */
    {  6, 12, NA, NA, 60,   NA,   NA,    NA, NO_CODE           ,     "Haswell"            },
    {  6, 15, NA, NA, 63,   NA,   NA,    NA, NO_CODE           ,     "Haswell-E"          },
    {  6,  3, NA, NA, 60,   NA,   NA,    NA, NO_CODE           ,     "Haswell-DT"         },
    {  6,  5, NA, NA, 69,   NA,   NA,    NA, NO_CODE           ,     "Haswell-ULT"        },
    {  6, 12, NA, NA, 60,   NA,   NA,    NA, DEVILSCANYON      ,     "Devil's Canyon"     },
    {  6,  1, NA, NA, 70,   NA,   NA,    NA, NO_CODE           ,     "Crystal Well-DT"    },

    /* Broadwell (14nm) */
    {  6,  7, NA, NA, 71,   NA,   NA,    NA, NO_CODE           ,     "Broadwell"          },
    {  6, 13, NA, NA, 61,   NA,   NA,    NA, NO_CODE           ,     "Broadwell-U"        },
    
    /* Skylake (14nm) */
    {  6, 14, NA, NA, 78,   NA,   NA,    NA, NO_CODE           ,     "Skylake"            },
    {  6, 14, NA, NA, 94,   NA,   NA,    NA, NO_CODE           ,     "Skylake-DT"         },

    /* Itanium */
    {  7, NA, NA, NA, NA,    1,   NA,    NA, NO_CODE           ,     "Itanium"            },
    { 15, NA, NA, 16, NA,    1,   NA,    NA, NO_CODE           ,     "Itanium 2"          },
    { 20, NA, NA, NA,  2,   NA,   NA,    NA, NO_CODE           ,     "Tukwila"            },
    
};

/**
 * Get uarch (codename)
 * N.B. Not in spec so custom method is used
 * uarch will be selected based on *PROBABILITY*
 */
static void get_intel_uarch(cpuid_data_t *data)
{
    if (data->cpuid_max_basic < 1)
        return;

    match_cpu_uarch(data, uarch_intel_t, NELEMS(uarch_intel_t),
                    brand_string_method(data));
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
    set_feature_bits(data, regidmap_ecx01, NELEMS(regidmap_ecx01), raw->cpuid[1][2]);
    set_feature_bits(data, regidmap_edx01, NELEMS(regidmap_edx01), raw->cpuid[1][3]);
    if (data->cpuid_max_basic < 7)
        return;
    set_feature_bits(data, regidmap_ebx07, NELEMS(regidmap_ebx07), raw->cpuid[7][1]);
}

void read_intel_data(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    get_intel_features(raw, data);
    get_intel_number_cores(raw, data);
    get_intel_deterministic_cacheinfo(raw, data);
    get_intel_uarch(data);
}
