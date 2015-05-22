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

#include <icuid/icuid.h>

#include "internal.h"
#include "features.h"
#include "intel.h"

typedef enum {
    Lnone, /*!< Indicates error */
    L1I,   /*!< L1 Instruction Cache */
    L1D,   /*!< L1 Data Cache */
    L2,    /*!< L2 Cache */
    L3,    /*!< L3 Cache */
} cache_type_t;

static void set_cache_info(cache_type_t cache, uint32_t size, uint32_t associativity,
                           uint32_t linesize, cpuid_data_t *data)
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
static void get_intel_deterministic_cacheinfo(cpuid_raw_data_t *raw, cpuid_data_t *data)
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
        set_cache_info(type, size, associativity, linesize, data);
    }
}

/**
 * Read Extended Topology Information
 */
static int read_intel_extended_topology(cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    unsigned int i;
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
static void get_intel_number_cores(cpuid_raw_data_t *raw, cpuid_data_t *data)
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

/**
 * Get codename
 */
static void get_intel_procinfo(cpuid_data_t *data)
{
    if (data->cpuid_max_basic < 1)
        return;

    switch (data->signature) {
        case CPU_MODEL_HASWELL:
            data->codename = "Haswell";
            break;
        case CPU_MODEL_HASWELL_E:
            data->codename = "Haswell-E";
            break;
        case CPU_MODEL_HASWELL_ULT:
            data->codename = "Haswell-ULT";
            break;
        case CPU_MODEL_IVYBRIDGE:
            data->codename = "IvyBridge";
            break;
        case CPU_MODEL_IVYBRIDGE_EP:
            data->codename = "IvyBridge-EP";
            break;
        case CPU_MODEL_SANDYBRIDGE:
            data->codename = "SandyBridge";
            break;
        case CPU_MODEL_SANDYBRIDGE_E:
            data->codename = "SandyBridge-E";
            break;
        case CPU_MODEL_WESTMERE:
            data->codename = "Westmere";
            break;
        case CPU_MODEL_WESTMERE_EP:
            data->codename = "Westmere-EP";
            break;
        case CPU_MODEL_WESTMERE_EX:
            data->codename = "Westmere-EX";
            break;
        case CPU_MODEL_NEHALEM:
            data->codename = "Nehalem";
            break;
        case CPU_MODEL_NEHALEM_EP:
            data->codename = "Nehalem-EP";
            break;
        case CPU_MODEL_NEHALEM_EX:
            data->codename = "Nehalem-EX";
            break;
        case CPU_MODEL_PENRYN:
            data->codename = "Penryn";
            break;
        case CPU_MODEL_PENRYN_E:
            data->codename = "Penryn-E";
            break;
        case CPU_MODEL_MEROM:
            data->codename = "Merom";
            break;
        default:
            data->codename = "Unknown";
            break;
    }
}

/**
 * Load Intel specific features
 */
static void get_intel_features(cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    const cpuid_feature_map_t regidmap_edx1[] = {
        { 18, CPU_FEATURE_PN },
        { 21, CPU_FEATURE_DTS },
        { 22, CPU_FEATURE_ACPI },
        { 27, CPU_FEATURE_SS },
        { 29, CPU_FEATURE_TM },
        { 30, CPU_FEATURE_IA64 },
        { 31, CPU_FEATURE_PBE },
    };
    const cpuid_feature_map_t regidmap_ecx1[] = {
        {  1, CPU_FEATURE_PCLMULDQ },
        {  2, CPU_FEATURE_DTS64 },
        {  4, CPU_FEATURE_DS_CPL },
        {  5, CPU_FEATURE_VMX },
        {  6, CPU_FEATURE_SMX },
        {  7, CPU_FEATURE_EST },
        {  8, CPU_FEATURE_TM2 },
        { 10, CPU_FEATURE_CID },
        { 14, CPU_FEATURE_XTPR },
        { 15, CPU_FEATURE_PDCM },
        { 17, CPU_FEATURE_PCID },
        { 18, CPU_FEATURE_DCA },
        { 20, CPU_FEATURE_SSE4_2 },
        { 22, CPU_FEATURE_MOVBE },
        { 24, CPU_FEATURE_TSC_DEADLINE },
        { 25, CPU_FEATURE_AES },
        { 26, CPU_FEATURE_XSAVE },
        { 27, CPU_FEATURE_OSXSAVE },
        { 30, CPU_FEATURE_RDRAND },
    };
    const cpuid_feature_map_t regidmap_ecx07[] = {
        { 0, CPU_FEATURE_FSGSBASE },      
        { 4, CPU_FEATURE_HLE },
        { 7, CPU_FEATURE_SMEP },
        { 9, CPU_FEATURE_ERMS },
        { 10, CPU_FEATURE_INVPCID },
        { 14, CPU_FEATURE_MPX },
        { 18, CPU_FEATURE_RDSEED },
        { 19, CPU_FEATURE_ADX },
        { 20, CPU_FEATURE_SMAP },
        { 29, CPU_FEATURE_SHA },
    };
    if (data->cpuid_max_basic < 1)
        return;
    set_feature_bits(regidmap_edx1, NELEMS(regidmap_edx1), raw->cpuid[1][3], data);
    set_feature_bits(regidmap_ecx1, NELEMS(regidmap_ecx1), raw->cpuid[1][2], data);
    if (data->cpuid_max_basic >= 7)
        set_feature_bits(regidmap_ecx07, NELEMS(regidmap_ecx07), raw->cpuid[7][1], data);
}

void read_intel_data(cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    get_intel_features(raw, data);
    get_intel_number_cores(raw, data);
    get_intel_deterministic_cacheinfo(raw, data);
    get_intel_procinfo(data);
}
