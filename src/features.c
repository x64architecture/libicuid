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

#include <stdio.h>
#include <stdlib.h>

#include <icuid/icuid.h>

#include "internal.h"
#include "features.h"

void set_feature_bits(cpuid_data_t *data, const cpuid_feature_map_t *feature,
                      const unsigned int array_size, const uint32_t reg)
{
    unsigned int i;
    for (i = 0; i < array_size; i++) {
        if (reg & (1 << feature[i].bit))
            data->flags[feature[i].feature] = 1;
    }
}

/* Map feature to string */
const char *cpu_feature_str(cpuid_feature_t feature)
{
    switch(feature) {
        case CPU_FEATURE_FPU: return "fpu";
        case CPU_FEATURE_VME: return "vme";
        case CPU_FEATURE_DE: return "de";
        case CPU_FEATURE_PSE: return "pse";
        case CPU_FEATURE_TSC: return "tsc";
        case CPU_FEATURE_MSR: return "msr";
        case CPU_FEATURE_PAE: return "pae";
        case CPU_FEATURE_MCE: return "mce";
        case CPU_FEATURE_CX8: return "cx8";
        case CPU_FEATURE_APIC: return "apic";
        case CPU_FEATURE_MTRR: return "mtrr";
        case CPU_FEATURE_SEP: return "sep";
        case CPU_FEATURE_PGE: return "pge";
        case CPU_FEATURE_MCA: return "mca";
        case CPU_FEATURE_CMOV: return "cmov";
        case CPU_FEATURE_PAT: return "pat";
        case CPU_FEATURE_PSE36: return "pse36";
        case CPU_FEATURE_PN: return "pn";
        case CPU_FEATURE_CLFLUSH: return "clflush";
        case CPU_FEATURE_DTS: return "dts";
        case CPU_FEATURE_ACPI: return "acpi";
        case CPU_FEATURE_MMX: return "mmx";
        case CPU_FEATURE_FXSR: return "fxsr";
        case CPU_FEATURE_PDPE1GB: return "pdpe1gb";
        case CPU_FEATURE_SSE: return "sse";
        case CPU_FEATURE_SSE2: return "sse2";
        case CPU_FEATURE_SS: return "ss";
        case CPU_FEATURE_HT: return "ht";
        case CPU_FEATURE_TM: return "tm";
        case CPU_FEATURE_IA64: return "ia64";
        case CPU_FEATURE_PBE: return "pbe";
        case CPU_FEATURE_PNI: return "pni";
        case CPU_FEATURE_PCLMULDQ: return "pclmuldq";
        case CPU_FEATURE_DTS64: return "dts64";
        case CPU_FEATURE_MONITOR: return "monitor";
        case CPU_FEATURE_DS_CPL: return "ds_cpl";
        case CPU_FEATURE_VMX: return "vmx";
        case CPU_FEATURE_SMX: return "smx";
        case CPU_FEATURE_EST: return "est";
        case CPU_FEATURE_TM2: return "tm2";
        case CPU_FEATURE_SSSE3: return "ssse3";
        case CPU_FEATURE_CID: return "cid";
        case CPU_FEATURE_SDBG: return "sdbg";
        case CPU_FEATURE_CX16: return "cx16";
        case CPU_FEATURE_XTPR: return "xtpr";
        case CPU_FEATURE_PDCM: return "pdcm";
        case CPU_FEATURE_PCID: return "pcid";
        case CPU_FEATURE_DCA: return "dca";
        case CPU_FEATURE_SSE4_1: return "sse4.1";
        case CPU_FEATURE_SSE4_2: return "sse4.2";
        case CPU_FEATURE_SYSCALL: return "syscall";
        case CPU_FEATURE_X2APIC: return "x2apic";
        case CPU_FEATURE_MOVBE: return "movbe";
        case CPU_FEATURE_POPCNT: return "popcnt";
        case CPU_FEATURE_TSC_DEADLINE: return "tsc_deadline_timer";
        case CPU_FEATURE_AES: return "aes";
        case CPU_FEATURE_XSAVE: return "xsave";
        case CPU_FEATURE_OSXSAVE: return "osxsave";
        case CPU_FEATURE_AVX: return "avx";
        case CPU_FEATURE_MMXEXT: return "mmxext";
        case CPU_FEATURE_3DNOW: return "3dnow";
        case CPU_FEATURE_3DNOWEXT: return "3dnowext";
        case CPU_FEATURE_NX: return "nx";
        case CPU_FEATURE_FXSR_OPT: return "fxsr_opt";
        case CPU_FEATURE_RDTSCP: return "rdtscp";
        case CPU_FEATURE_LM: return "lm";
        case CPU_FEATURE_LAHF_LM: return "lahf_lm";
        case CPU_FEATURE_CMP_LEGACY: return "cmp_legacy";
        case CPU_FEATURE_SVM: return "svm";
        case CPU_FEATURE_SSE4A: return "sse4a";
        case CPU_FEATURE_MISALIGNSSE: return "misalignsse";
        case CPU_FEATURE_ABM: return "abm";
        case CPU_FEATURE_3DNOWPREFETCH: return "3dnowprefetch";
        case CPU_FEATURE_OSVW: return "osvw";
        case CPU_FEATURE_IBS: return "ibs";
        case CPU_FEATURE_SKINIT: return "skinit";
        case CPU_FEATURE_WDT: return "wdt";
        case CPU_FEATURE_TS: return "ts";
        case CPU_FEATURE_FID: return "fid";
        case CPU_FEATURE_VID: return "vid";
        case CPU_FEATURE_TTP: return "ttp";
        case CPU_FEATURE_TM_AMD: return "tm_amd";
        case CPU_FEATURE_STC: return "stc";
        case CPU_FEATURE_100MHZSTEPS: return "100mhzsteps";
        case CPU_FEATURE_HWPSTATE: return "hwpstate";
        case CPU_FEATURE_CONSTANT_TSC: return "constant_tsc";
        case CPU_FEATURE_XOP: return "xop";
        case CPU_FEATURE_FMA: return "fma";
        case CPU_FEATURE_FMA4: return "fma4";
        case CPU_FEATURE_TBM: return "tbm";
        case CPU_FEATURE_F16C: return "f16c";
        case CPU_FEATURE_RDRAND: return "rdrand";
        case CPU_FEATURE_CPB: return "cpb";
        case CPU_FEATURE_APERFMPERF: return "aperfmperf";
        case CPU_FEATURE_PFI: return "pfi";
        case CPU_FEATURE_PA: return "pa";
        case CPU_FEATURE_AVX2: return "avx2";
        case CPU_FEATURE_BMI1: return "bmi1";
        case CPU_FEATURE_BMI2: return "bmi2";
        case CPU_FEATURE_HYPERVISOR: return "hypervisor";
        case CPU_FEATURE_FSGSBASE: return "fsgsbase";
        case CPU_FEATURE_HLE: return "hle";
        case CPU_FEATURE_SMEP: return "smep";
        case CPU_FEATURE_ERMS: return "erms";
        case CPU_FEATURE_INVPCID: return "invpcid";
        case CPU_FEATURE_MPX: return "mpx";
        case CPU_FEATURE_RDSEED: return "rdseed";
        case CPU_FEATURE_ADX: return "adx";
        case CPU_FEATURE_SMAP: return "smap";
        case CPU_FEATURE_SHA: return "sha";
        case CPU_FEATURE_CLZERO: return "clzero";
        case CPU_FEATURE_IRPERF: return "irperf";
        case CPU_FEATURE_EXTAPIC: return "extapic";
        case CPU_FEATURE_CR8_LEGACY: return "cr8_legacy";
        case CPU_FEATURE_LWP: return "lwp";
        case CPU_FEATURE_TCE: return "tce";
        case CPU_FEATURE_NODEID_MSR: return "nodeid_msr";
        case CPU_FEATURE_TOPOEXT: return "topoext";
        case CPU_FEATURE_PERFCTR_CORE: return "perfctr_core";
        case CPU_FEATURE_PERFCTR_NB: return "perfctr_nb";
        case CPU_FEATURE_BPEXT: return "bpext";
        case CPU_FEATURE_PERFCTR_L2: return "perfctr_l2";
        case CPU_FEATURE_MONITORX: return "monitorx";
        case CPU_FEATURE_TSC_ADJUST: return "tsc_adjust";
        case CPU_FEATURE_RTM: return "rtm";
        case CPU_FEATURE_CQM: return "cqm";
        case CPU_FEATURE_AVX512F: return "avx512f";
        case CPU_FEATURE_AVX512DQ: return "avx512dq";
        case CPU_FEATURE_PCOMMIT: return "pcommit";
        case CPU_FEATURE_CLFLUSHOPT: return "clflushopt";
        case CPU_FEATURE_CLWB: return "clwb";
        case CPU_FEATURE_AVX512PF: return "avx512pf";
        case CPU_FEATURE_AVX512ER: return "avx512er";
        case CPU_FEATURE_AVX512CD: return "avx512cd";
        case CPU_FEATURE_AVX512BW: return "avx512bw";
        case CPU_FEATURE_AVX512VL: return "avx512vl";
        case CPU_FEATURE_SGX: return "sgx";
        case CPU_FEATURE_SME: return "sme";
        default:
            return "";
    }
}

/* Set common features (shared between Intel & AMD) */
void set_common_features(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    const cpuid_feature_map_t regidmap_ecx01[] = {
        { 0, CPU_FEATURE_PNI },
        { 1, CPU_FEATURE_PCLMULDQ },
        { 3, CPU_FEATURE_MONITOR },
        { 9, CPU_FEATURE_SSSE3 },
        { 12, CPU_FEATURE_FMA },
        { 13, CPU_FEATURE_CX16 },
        { 19, CPU_FEATURE_SSE4_1 },
        { 20, CPU_FEATURE_SSE4_2 },
        { 22, CPU_FEATURE_MOVBE },
        { 23, CPU_FEATURE_POPCNT },
        { 25, CPU_FEATURE_AES },
        { 26, CPU_FEATURE_XSAVE },
        { 27, CPU_FEATURE_OSXSAVE },
        { 28, CPU_FEATURE_AVX },
        { 29, CPU_FEATURE_F16C },
        { 30, CPU_FEATURE_RDRAND },
        { 31, CPU_FEATURE_HYPERVISOR },
    };
    const cpuid_feature_map_t regidmap_edx01[] = {
        { 0, CPU_FEATURE_FPU },
        { 1, CPU_FEATURE_VME },
        { 2, CPU_FEATURE_DE },
        { 3, CPU_FEATURE_PSE },
        { 4, CPU_FEATURE_TSC },
        { 5, CPU_FEATURE_MSR },
        { 6, CPU_FEATURE_PAE },
        { 7, CPU_FEATURE_MCE },
        { 8, CPU_FEATURE_CX8 },
        { 9, CPU_FEATURE_APIC },
        { 11, CPU_FEATURE_SEP },
        { 12, CPU_FEATURE_MTRR },
        { 13, CPU_FEATURE_PGE },
        { 14, CPU_FEATURE_MCA },
        { 15, CPU_FEATURE_CMOV },
        { 16, CPU_FEATURE_PAT },
        { 17, CPU_FEATURE_PSE36 },
        { 19, CPU_FEATURE_CLFLUSH },
        { 23, CPU_FEATURE_MMX },
        { 24, CPU_FEATURE_FXSR },
        { 25, CPU_FEATURE_SSE },
        { 26, CPU_FEATURE_SSE2 },
        { 28, CPU_FEATURE_HT },
    };
    const cpuid_feature_map_t regidmap_ebx07[] = {
        { 0, CPU_FEATURE_FSGSBASE },
        { 3, CPU_FEATURE_BMI1 },
        { 5, CPU_FEATURE_AVX2 },
        { 7, CPU_FEATURE_SMEP },
        { 8, CPU_FEATURE_BMI2 },
    };
    const cpuid_feature_map_t regidmap_ecx81[] = {
        { 0, CPU_FEATURE_LAHF_LM },
    };
    const cpuid_feature_map_t regidmap_edx81[] = {
        { 11, CPU_FEATURE_SYSCALL },
        { 20, CPU_FEATURE_NX },
        { 26, CPU_FEATURE_PDPE1GB },
        { 27, CPU_FEATURE_RDTSCP },
        { 29, CPU_FEATURE_LM },
    };
    const cpuid_feature_map_t regidmap_edx87[] = {
        { 8, CPU_FEATURE_CONSTANT_TSC },
    };

    if (data->cpuid_max_basic >= 1) {
        set_feature_bits(data, regidmap_ecx01, NELEMS(regidmap_ecx01), raw->cpuid[1][2]);
        set_feature_bits(data, regidmap_edx01, NELEMS(regidmap_edx01), raw->cpuid[1][3]);
    }
    if (data->cpuid_max_basic >= 7)
        set_feature_bits(data, regidmap_ebx07, NELEMS(regidmap_ebx07), raw->cpuid[7][1]);
    if (data->cpuid_max_ext >= 0x80000001) {
        set_feature_bits(data, regidmap_ecx81, NELEMS(regidmap_ecx81), raw->cpuid_ext[1][2]);
        set_feature_bits(data, regidmap_edx81, NELEMS(regidmap_edx81), raw->cpuid_ext[1][3]);
    }
    if (data->cpuid_max_ext >= 0x80000007)
        set_feature_bits(data, regidmap_edx87, NELEMS(regidmap_edx87), raw->cpuid_ext[7][3]);
}

void set_common_xfeatures(cpuid_data_t *data, const uint64_t xcr0)
{
    unsigned int i;
    const struct {
        uint8_t bit;
        xfeature_t feature;
    } xfeatures_t[NUM_XFEATURES] = {
        { 0, XFEATURE_FP },
        { 1, XFEATURE_SSE },
        { 2, XFEATURE_AVX },
        { 3, XFEATURE_BNDREGS },
        { 4, XFEATURE_BNDCSR },
        { 5, XFEATURE_OPMASK },
        { 6, XFEATURE_ZMM_Hi256 },
        { 7, XFEATURE_Hi16_ZMM },
        { 8, XFEATURE_IA32_XSS },
        { 9, XFEATURE_PKRU },
    };
    for (i = 0; i < NUM_XFEATURES; i++) {
        if (xcr0 & (1ULL << xfeatures_t[i].bit))
            data->xfeatures[xfeatures_t[i].feature] = 1;
    }
}
