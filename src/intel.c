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
            data->logical_cpus = (logical_cpus >= 2 ? logical_cpus : 2);
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
    int i, need_table = 1;

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

        /* RDRAND is in Ivy Bridge+ */
        if (data->flags[CPU_FEATURE_RDRAND])
            uarch = CORE_IVYBRIDGE3;
        /* FMA is in Haswell+ */
        if (data->flags[CPU_FEATURE_FMA])
            uarch = CORE_HASWELL3;
        /* ADX is in Broadwell+ */
        if (data->flags[CPU_FEATURE_ADX])
            uarch = CORE_BROADWELL3;
        /* MPX is in Skylake+ */
        if (data->flags[CPU_FEATURE_MPX])
            uarch = CORE_SKYLAKE3;

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

        switch (uarch) {
            case CORE_HASWELL5:
                if (strstr(bs, "4690K") != NULL)
                    uarch = CORE_DEVILSCANYON5;
                break;
            case CORE_HASWELL7:
                if (strstr(bs, "4790K") != NULL)
                    uarch = CORE_DEVILSCANYON7;
                break;
            default:
                break;
        }
    }
    if (strstr(bs, "Mobile") != NULL) {
        need_table = 0;
        if (strstr(bs, "Celeron"))
            uarch = MOBILE_CELERON;
        else if (strstr(bs, "Pentium") != NULL)
            uarch = MOBILE_PENTIUM;
    }
    if (need_table) {
        for (i = 0; i < (int)NELEMS(uarch_t); i++)
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
                uarch = MORE_THAN_QUADCORE;
                break;
        }
    }
    
    if (uarch == CORE_DUO && data->ext_model >= 23) {
        uarch = WOLFDALE;
    }
    if (uarch == PENTIUM_D && data->ext_model >= 23) {
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
    { NA, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown CPU uarch"       },
    
    /* i486 */
    {  4, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown i486"            },
    {  4,  0, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX-25/33"           },
    {  4,  1, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX-50"              },
    {  4,  2, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 SX"                 },
    {  4,  3, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX2"                },
    {  4,  4, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 SL"                 },
    {  4,  5, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 SX2"                },
    {  4,  7, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX2 WriteBack"      },
    {  4,  8, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX4"                },
    {  4,  9, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "i486 DX4 WriteBack"      },
    
    /* Pentium 1 - P5 based */
    {  5, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Pentium"         },
    {  5,  0, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium A-Step"          },
    {  5,  1, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.8u)"        },
    {  5,  2, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.35u)"       },
    {  5,  3, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium OverDrive"       },
    {  5,  4, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.35u)"       },
    {  5,  7, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 1 (0.35u)"       },
    {  5,  8, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium MMX (0.25u)"     },
    {  5,  9, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Clanton"                 },
    
    /* Pentium 2 / 3 / M - P6 based */
    {  6, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown P6"              },
    {  6,  0, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium Pro"             },
    {  6,  1, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium Pro"             },
    {  6,  3, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium II (Klamath)"    },
    {  6,  5, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium II (Deschutes)"  },
    {  6,  5, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile Pentium II (Tonga)"},
    {  6,  6, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium II (Dixon)"      },
    
    {  6,  3, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-II Xeon"               },
    {  6,  5, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-II Xeon"               },
    {  6,  6, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-II Xeon"               },
        
    {  6,  5, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "P-II Celeron (no L2)"    },
    {  6,  6, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "P-II Celeron (128K)"     },
    
    {  6,  7, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium III (Katmai)"    },
    {  6,  8, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium III (Coppermine)"},
    {  6, 10, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium III (Coppermine)"},
    {  6, 11, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium III (Tualatin)"  },
    
    {  6,  7, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-III Xeon"              },
    {  6,  8, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-III Xeon"              },
    {  6, 10, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-III Xeon"              },
    {  6, 11, NA, NA, NA,   1,    NA,    NA, XEON              ,     "P-III Xeon"              },
    
    {  6,  7, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "P-III Celeron"           },
    {  6,  8, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "P-III Celeron"           },
    {  6, 10, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "P-III Celeron"           },
    {  6, 11, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "P-III Celeron"           },
    
    /* Netburst based (Pentium 4 and later)
       classic P4s */
    { 15, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Pentium 4"       },
    { 15, NA, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Unknown P-4 Celeron"     },
    { 15, NA, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Unknown Xeon"            },
    
    { 15,  0, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 4 (Willamette)"  },
    { 15,  1, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 4 (Willamette)"  },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 4 (Northwood)"   },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 4 (Prescott)"    },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 4 (Prescott)"    },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium 4 (Cedar Mill)"  },
    { 15,  0, NA, 15, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile P-4 (Willamette)" },
    { 15,  1, NA, 15, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile P-4 (Willamette)" },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile P-4 (Northwood)"  },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile P-4 (Prescott)"   },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile P-4 (Prescott)"   },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, MOBILE_PENTIUM    ,     "Mobile P-4 (Cedar Mill)" },
    
    /* server CPUs */
    { 15,  0, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Xeon (Foster)"           },
    { 15,  1, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Xeon (Foster)"           },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Xeon (Prestonia)"        },
    { 15,  2, NA, 15, NA,   1,    NA,    NA, XEONMP            ,     "Xeon (Gallatin)"         },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Xeon (Nocona)"           },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Xeon (Nocona)"           },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEON_IRWIN        ,     "Xeon (Irwindale)"        },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEONMP            ,     "Xeon (Cranford)"         },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, XEON_POTOMAC      ,     "Xeon (Potomac)"          },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, XEON              ,     "Xeon (Dempsey)"          },
    
    /* Pentium Ds */
    { 15,  4,  4, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium D"               },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, PENTIUM_D         ,     "Pentium D"               },
    { 15,  4,  7, 15, NA,   1,    NA,    NA, NO_CODE           ,     "Pentium D"               },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, PENTIUM_D         ,     "Pentium D"               },

    /* Celeron and Celeron Ds */
    { 15,  1, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "P-4 Celeron (128K)"      }, 
    { 15,  2, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "P-4 Celeron (128K)"      },
    { 15,  3, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Celeron D"               },
    { 15,  4, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Celeron D"               },
    { 15,  6, NA, 15, NA,   1,    NA,    NA, CELERON           ,     "Celeron D"               },
    {  6, 55, NA, NA, NA,   2,    NA,    NA, CELERON           ,     "Celeron (Bay Trail-M)"   },
    {  6, 55, NA, NA, NA,   4,    NA,    NA, CELERON           ,     "Celeron (Bay Trail-D)"   },

    /* Intel Core - P6-based */
    
    {  6,  9, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Pentium M"          },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM_M  ,     "Unknown Pentium M"          },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, PENTIUM           ,     "Pentium M (Banias)"         },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM_M  ,     "Pentium M (Banias)"         },
    {  6,  9, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Celeron M"                  },
    {  6, 13, NA, NA, NA,   1,    NA,    NA, PENTIUM           ,     "Pentium M (Dothan)"         },
    {  6, 13, NA, NA, NA,   1,    NA,    NA, MOBILE_PENTIUM_M  ,     "Pentium M (Dothan)"         },
    {  6, 13, NA, NA, NA,   1,    NA,    NA, CELERON           ,     "Celeron M"                  },
    
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM              ,     "Unknown Atom"               },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_DIAMONDVILLE ,     "Atom (Diamondville)"        },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_SILVERTHORNE ,     "Atom (Silverthorne)"        },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_CEDARVIEW    ,     "Atom (Cedarview)"           },
    {  6,  6, NA, NA, NA,  NA,    NA,    NA, ATOM_CEDARVIEW    ,     "Atom (Cedarview)"           },
    {  6, 12, NA, NA, NA,  NA,    NA,    NA, ATOM_PINEVIEW     ,     "Atom (Pineview)"            },
    {  6, 55, NA, NA, NA,  NA,    NA,    NA, ATOM              ,     "Atom (Bay Trail)"           },
    {  6, 77, NA, NA, NA,  NA,    NA,    NA, NO_CODE           ,     "Atom (Avoton)"              },
    
    {  6, 14, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Yonah"              },
    {  6, 14, NA, NA, NA,   1,    NA,    NA, CORE_SOLO         ,     "Yonah (Core Solo)"          },
    {  6, 14, NA, NA, NA,   2,    NA,    NA, CORE_DUO          ,     "Yonah (Core Duo)"           },
    {  6, 14, NA, NA, NA,   1,    NA,    NA, MOBILE_CORE_SOLO  ,     "Yonah (Core Solo)"          },
    {  6, 14, NA, NA, NA,   2,    NA,    NA, MOBILE_CORE_DUO   ,     "Yonah (Core Duo)"           },
    {  6, 14, NA, NA, NA,   1,    NA,    NA, CORE_SOLO         ,     "Yonah (Core Solo)"          },
    
    {  6, 15, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Unknown Core 2"             },
    {  6, 15, NA, NA, NA,   2,  4096,    NA, CORE_DUO          ,     "Conroe (Core 2 Duo)"        },
    {  6, 15, NA, NA, NA,   2,  1024,    NA, CORE_DUO          ,     "Conroe (Core 2 Duo) 1024K"  },
    {  6, 15, NA, NA, NA,   2,   512,    NA, CORE_DUO          ,     "Conroe (Core 2 Duo) 512K"   },
    {  6, 15, NA, NA, NA,   4,    NA,    NA, QUAD_CORE         ,     "Kentsfield (Core 2 Quad)"   },
    {  6, 15, NA, NA, NA,   4,  4096,    NA, QUAD_CORE         ,     "Kentsfield (Core 2 Quad)"   },
    {  6, 15, NA, NA, NA, 400,    NA,    NA, MORE_THAN_QUADCORE,     "More than quad-core"        },
    {  6, 15, NA, NA, NA,   2,  2048,    NA, CORE_DUO          ,     "Allendale (Core 2 Duo)"     },
    {  6, 15, NA, NA, NA,   2,    NA,    NA, MOBILE_CORE_DUO   ,     "Merom (Core 2 Duo)"         },
    {  6, 15, NA, NA, NA,   2,  2048,    NA, MEROM             ,     "Merom (Core 2 Duo) 2048K"   },
    {  6, 15, NA, NA, NA,   2,  4096,    NA, MEROM             ,     "Merom (Core 2 Duo) 4096K"   },
    
    {  6, 15, NA, NA, 15,   1,    NA,    NA, CELERON           ,     "Conroe-L (Celeron)"         },
    {  6,  6, NA, NA, 22,   1,    NA,    NA, CELERON           ,     "Conroe-L (Celeron)"         },
    {  6, 15, NA, NA, 15,   2,    NA,    NA, CELERON           ,     "Conroe-L (Allendale)"       },
    {  6,  6, NA, NA, 22,   2,    NA,    NA, CELERON           ,     "Conroe-L (Allendale)"       },
    
    
    {  6,  6, NA, NA, 22,   1,    NA,    NA, NO_CODE           ,     "Unknown Core ?"             },
    {  6,  7, NA, NA, 23,   1,    NA,    NA, NO_CODE           ,     "Unknown Core ?"             },
    {  6,  6, NA, NA, 22, 400,    NA,    NA, MORE_THAN_QUADCORE,     "More than quad-core"        },
    {  6,  7, NA, NA, 23, 400,    NA,    NA, MORE_THAN_QUADCORE,     "More than quad-core"        },
    
    {  6,  7, NA, NA, 23,   1,    NA,    NA, CORE_SOLO         ,     "Unknown Core 45nm"          },
    {  6,  7, NA, NA, 23,   1,    NA,    NA, CORE_DUO          ,     "Unknown Core 45nm"          },
    {  6,  7, NA, NA, 23,   2,  1024,    NA, WOLFDALE          ,     "Celeron Wolfdale 1M"        },
    {  6,  7, NA, NA, 23,   2,  2048,    NA, WOLFDALE          ,     "Wolfdale (Core 2 Duo) 2M"   },
    {  6,  7, NA, NA, 23,   2,  3072,    NA, WOLFDALE          ,     "Wolfdale (Core 2 Duo) 3M"   },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, WOLFDALE          ,     "Wolfdale (Core 2 Duo) 6M"   },
    {  6,  7, NA, NA, 23,   1,    NA,    NA, MOBILE_CORE_DUO   ,     "Penryn (Core 2 Duo)"        },
    {  6,  7, NA, NA, 23,   2,  3072,    NA, PENRYN            ,     "Penryn (Core 2 Duo) 3M"     },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, PENRYN            ,     "Penryn (Core 2 Duo) 6M"     },
    {  6,  7, NA, NA, 23,   4,  2048,    NA, QUAD_CORE         ,     "Yorkfield (Core 2 Quad) 2M" },
    {  6,  7, NA, NA, 23,   4,  3072,    NA, QUAD_CORE         ,     "Yorkfield (Core 2 Quad) 3M" },
    {  6,  7, NA, NA, 23,   4,  6144,    NA, QUAD_CORE         ,     "Yorkfield (Core 2 Quad) 6M" },
    
    /* Xeons - Core based */
    {  6, 14, NA, NA, 14,   1,    NA,    NA, XEON              ,     "Xeon LV"                  },
    {  6, 15, NA, NA, 15,   2,  4096,    NA, XEON              ,     "Xeon (Woodcrest)"         },
    {  6, 15, NA, NA, 15,   2,  2048,    NA, XEON              ,     "Xeon (Conroe/2M)"         },
    {  6, 15, NA, NA, 15,   2,  4096,    NA, XEON              ,     "Xeon (Conroe/4M)"         },
    {  6, 15, NA, NA, 15,   4,  4096,    NA, XEON              ,     "Xeon (Kentsfield)"        },
    {  6, 15, NA, NA, 15,   4,  4096,    NA, XEON              ,     "Xeon (Clovertown)"        },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, XEON              ,     "Xeon (Wolfdale)"          },
    {  6,  7, NA, NA, 23,   2,  6144,    NA, XEON              ,     "Xeon (Wolfdale DP)"       },
    {  6,  7, NA, NA, 23,   4,  6144,    NA, XEON              ,     "Xeon (Harpertown)"        },
    {  6,  7, NA, NA, 23,   4,  3072,    NA, XEON              ,     "Xeon (Yorkfield/3M)"      },
    {  6,  7, NA, NA, 23,   4,  6144,    NA, XEON              ,     "Xeon (Yorkfield/6M)"      },

    /* Nehalem (45nm) */
    {  6, 10, NA, NA, 26,   4,    NA,    NA, XEON_GAINESTOWN   ,     "Gainestown (Xeon)"        },
    {  6, 10, NA, NA, 26,   4,    NA,  4096, XEON_GAINESTOWN   ,     "Gainestown 4M (Xeon)"     },
    {  6, 10, NA, NA, 26,   4,    NA,  8192, XEON_GAINESTOWN   ,     "Gainestown 8M (Xeon)"     },
    {  6, 10, NA, NA, 26,   4,    NA,    NA, XEON_I7           ,     "Bloomfield (Xeon)"        },
    {  6, 10, NA, NA, 26,   4,    NA,    NA, CORE_I7           ,     "Bloomfield (Core i7)"     },
    {  6, 10, NA, NA, 30,   4,    NA,    NA, CORE_I7           ,     "Lynnfield (Core i7)"      },
    {  6,  5, NA, NA, 37,   4,    NA,  8192, CORE_I5           ,     "Lynnfield (Core i5)"      },

    /* Westmere (32nm) */
    {  6,  5, NA, NA, 37,   2,    NA,    NA, NO_CODE           ,     "Unknown Core i3/i5"       },
    {  6, 12, NA, NA, 44,  NA,    NA,    NA, XEON_WESTMERE     ,     "Westmere (Xeon)"          },
    {  6, 12, NA, NA, 44,  NA,    NA, 12288, XEON_WESTMERE     ,     "Gulftown (Xeon)"          },
    {  6, 12, NA, NA, 44,   4,    NA, 12288, CORE_I7           ,     "Gulftown (Core i7)"       },
    {  6,  5, NA, NA, 37,   2,    NA,  4096, CORE_I5           ,     "Clarkdale (Core i5)"      },
    {  6,  5, NA, NA, 37,   2,    NA,  4096, CORE_I3           ,     "Clarkdale (Core i3)"      },
    {  6,  5, NA, NA, 37,   2,    NA,  4096, CORE_I7           ,     "Arrandale (Core i7)"      },
    {  6,  5, NA, NA, 37,   2,    NA,  3072, CORE_I5           ,     "Arrandale (Core i5)"      },
    {  6,  5, NA, NA, 37,   2,    NA,  3072, CORE_I3           ,     "Arrandale (Core i3)"      },

    /* Sandy Bridge (32nm) */
    {  6, 10, NA, NA, 42,  NA,    NA,    NA, NO_CODE           ,     "Unknown Sandy Bridge"     },
    {  6, 10, NA, NA, 42,  NA,    NA,    NA, XEON              ,     "Sandy Bridge (Xeon)"      },
    {  6, 10, NA, NA, 42,  NA,    NA,    NA, CORE_I7           ,     "Sandy Bridge (Core i7)"   },
    {  6, 10, NA, NA, 42,   4,    NA,    NA, CORE_I7           ,     "Sandy Bridge (Core i7)"   },
    {  6, 10, NA, NA, 42,   4,    NA,    NA, CORE_I5           ,     "Sandy Bridge (Core i5)"   },
    {  6, 10, NA, NA, 42,   2,    NA,    NA, CORE_I3           ,     "Sandy Bridge (Core i3)"   },
    {  6, 10, NA, NA, 42,   2,    NA,    NA, PENTIUM           ,     "Sandy Bridge (Pentium)"   },
    {  6, 10, NA, NA, 42,   1,    NA,    NA, CELERON           ,     "Sandy Bridge (Celeron)"   },
    {  6, 10, NA, NA, 42,   2,    NA,    NA, CELERON           ,     "Sandy Bridge (Celeron)"   },
    {  6, 13, NA, NA, 45,  NA,    NA,    NA, NO_CODE           ,     "Sandy Bridge-E"           },
    {  6, 13, NA, NA, 45,  NA,    NA,    NA, XEON              ,     "Sandy Bridge-E (Xeon)"    },

    /* Ivy Bridge (22nm) */
    {  6, 10, NA, NA, 58,  NA,    NA,    NA, XEON              ,     "Ivy Bridge (Xeon)"        },
    {  6, 10, NA, NA, 58,   4,    NA,    NA, CORE_IVYBRIDGE7   ,     "Ivy Bridge (Core i7)"     },
    {  6, 10, NA, NA, 58,   4,    NA,    NA, CORE_IVYBRIDGE5   ,     "Ivy Bridge (Core i5)"     },
    {  6, 10, NA, NA, 58,   2,    NA,    NA, CORE_IVYBRIDGE3   ,     "Ivy Bridge (Core i3)"     },
    {  6, 10, NA, NA, 58,   2,    NA,    NA, PENTIUM           ,     "Ivy Bridge (Pentium)"     },
    {  6, 10, NA, NA, 58,   1,    NA,    NA, CELERON           ,     "Ivy Bridge (Celeron)"     },
    {  6, 10, NA, NA, 58,   2,    NA,    NA, CELERON           ,     "Ivy Bridge (Celeron)"     },
    {  6, 14, NA, NA, 62,  NA,    NA,    NA, NO_CODE           ,     "Ivy Bridge-E"             },
    
    /* Haswell (22nm) */
    {  6, 12, NA, NA, 60,  NA,    NA,    NA, XEON              ,     "Haswell (Xeon)"           },
    {  6, 12, NA, NA, 60,   4,    NA,    NA, CORE_HASWELL7     ,     "Haswell (Core i7)"        },
    {  6,  5, NA, NA, 69,   4,    NA,    NA, CORE_HASWELL7     ,     "Haswell (Core i7)"        },
    {  6, 12, NA, NA, 60,   4,    NA,    NA, CORE_HASWELL5     ,     "Haswell (Core i5)"        },
    {  6,  5, NA, NA, 69,   4,    NA,    NA, CORE_HASWELL5     ,     "Haswell (Core i5)"        },
    {  6, 12, NA, NA, 60,   2,    NA,    NA, CORE_HASWELL3     ,     "Haswell (Core i3)"        },
    {  6,  5, NA, NA, 69,   2,    NA,    NA, CORE_HASWELL3     ,     "Haswell (Core i3)"        },
    {  6, 12, NA, NA, 60,   2,    NA,    NA, PENTIUM           ,     "Haswell (Pentium)"        },
    {  6, 12, NA, NA, 60,   2,    NA,    NA, CELERON           ,     "Haswell (Celeron)"        },
    {  6, 12, NA, NA, 60,   1,    NA,    NA, CELERON           ,     "Haswell (Celeron)"        },
    {  6, 15, NA, NA, 63,  NA,    NA,    NA, NO_CODE           ,     "Haswell-E"                },
    /* Devil's Canyon */
    {  6, 12, NA, NA, 60,   4,    NA,    NA, CORE_DEVILSCANYON5,     "Devil's Canyon (Core i5)" },
    {  6, 12, NA, NA, 60,   4,    NA,    NA, CORE_DEVILSCANYON7,     "Devil's Canyon (Core i7)" },

    /* Broadwell (14nm) */
    {  6,  7, NA, NA, 71,   4,    NA,    NA, CORE_BROADWELL7   ,     "Broadwell (Core i7)"      },
    {  6,  7, NA, NA, 71,   4,    NA,    NA, CORE_BROADWELL5   ,     "Broadwell (Core i5)"      },
    {  6, 13, NA, NA, 61,   4,    NA,    NA, CORE_BROADWELL7   ,     "Broadwell-U (Core i7)"    },
    {  6, 13, NA, NA, 61,   2,    NA,    NA, CORE_BROADWELL7   ,     "Broadwell-U (Core i7)"    },
    {  6, 13, NA, NA, 61,   2,    NA,    NA, CORE_BROADWELL5   ,     "Broadwell-U (Core i5)"    },
    {  6, 13, NA, NA, 61,   2,    NA,    NA, CORE_BROADWELL3   ,     "Broadwell-U (Core i3)"    },
    {  6, 13, NA, NA, 61,   2,    NA,    NA, PENTIUM           ,     "Broadwell-U (Pentium)"    },
    {  6, 13, NA, NA, 61,   2,    NA,    NA, CELERON           ,     "Broadwell-U (Celeron)"    },
    {  6, 13, NA, NA, 61,   2,    NA,    NA, NA                ,     "Broadwell-U (Core M)"     },
    
    /* Skylake (14nm) */
    {  6, 14, NA, NA, 94,   4,    NA,    NA, CORE_SKYLAKE7     ,     "Skylake (Core i7)"        },
    {  6, 14, NA, NA, 94,   4,    NA,    NA, CORE_SKYLAKE5     ,     "Skylake (Core i5)"        },
    {  6, 14, NA, NA, 94,   4,    NA,    NA, CORE_SKYLAKE3     ,     "Skylake (Core i3)"        },
    {  6, 14, NA, NA, 94,   4,    NA,    NA, PENTIUM           ,     "Skylake (Pentium)"        },

    /* Itanium */
    {  7, NA, NA, NA, NA,   1,    NA,    NA, NO_CODE           ,     "Itanium"                  },
    { 15, NA, NA, 16, NA,   1,    NA,    NA, NO_CODE           ,     "Itanium 2"                },
    
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
