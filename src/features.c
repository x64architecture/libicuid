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

#define IS_AMD   (data->vendor == VENDOR_AMD)
#define IS_INTEL (data->vendor == VENDOR_INTEL)

#define VEND_INTEL  0
#define VEND_AMD    1
#define VEND_SHARED 2

typedef struct {
    uint8_t bit;
    cpuid_feature_t feature;
    uint8_t vendor;
} cpuid_feature_map_t;

static void set_feature_bits(cpuid_data_t *data,
                      const cpuid_feature_map_t *feature,
                      const unsigned int array_size, const uint32_t reg)
{
    unsigned int i;
    for (i = 0; i < array_size; i++) {
        if (!(reg & (1 << feature[i].bit)))
            continue;
        if (feature[i].vendor == VEND_SHARED)
            data->flags[feature[i].feature] = 1;
        else if (feature[i].vendor == VEND_INTEL && IS_INTEL)
            data->flags[feature[i].feature] = 1;
        else if (feature[i].vendor == VEND_AMD && IS_AMD)
            data->flags[feature[i].feature] = 1;
    }
}

/* Map feature to string */
const char *cpu_feature_str(cpuid_feature_t feature)
{
    switch (feature) {
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
        case CPU_FEATURE_SPEC_CTRL: return "spec_ctrl";
        case CPU_FEATURE_SEV: return "sev";
        case CPU_FEATURE_PAGEFLUSH: return "page_flush";
        case CPU_FEATURE_SEV_ES: return "sev_es";
        default:
            return "";
    }
}

void set_cpuid_features(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    const cpuid_feature_map_t regidmap_ecx01[] = {
        { 0,  CPU_FEATURE_PNI,             VEND_SHARED },
        { 1,  CPU_FEATURE_PCLMULDQ,        VEND_SHARED },
        { 2,  CPU_FEATURE_DTS64,           VEND_INTEL  },
        { 3,  CPU_FEATURE_MONITOR,         VEND_SHARED },
        { 4,  CPU_FEATURE_DS_CPL,          VEND_INTEL  },
        { 5,  CPU_FEATURE_VMX,             VEND_INTEL  },
        { 6,  CPU_FEATURE_SMX,             VEND_INTEL  },
        { 7,  CPU_FEATURE_EST,             VEND_INTEL  },
        { 8,  CPU_FEATURE_TM2,             VEND_INTEL  },
        { 9,  CPU_FEATURE_SSSE3,           VEND_SHARED },
        { 10, CPU_FEATURE_CID,             VEND_INTEL  },
        { 11, CPU_FEATURE_SDBG,            VEND_INTEL  },
        { 12, CPU_FEATURE_FMA,             VEND_SHARED },
        { 13, CPU_FEATURE_CX16,            VEND_SHARED },
        { 14, CPU_FEATURE_XTPR,            VEND_INTEL  },
        { 15, CPU_FEATURE_PDCM,            VEND_INTEL  },
        { 17, CPU_FEATURE_PCID,            VEND_INTEL  },
        { 18, CPU_FEATURE_DCA,             VEND_INTEL  },
        { 19, CPU_FEATURE_SSE4_1,          VEND_SHARED },
        { 20, CPU_FEATURE_SSE4_2,          VEND_SHARED },
        { 21, CPU_FEATURE_X2APIC,          VEND_INTEL  },
        { 22, CPU_FEATURE_MOVBE,           VEND_SHARED },
        { 23, CPU_FEATURE_POPCNT,          VEND_SHARED },
        { 24, CPU_FEATURE_TSC_DEADLINE,    VEND_INTEL  },
        { 25, CPU_FEATURE_AES,             VEND_SHARED },
        { 26, CPU_FEATURE_XSAVE,           VEND_SHARED },
        { 27, CPU_FEATURE_OSXSAVE,         VEND_SHARED },
        { 28, CPU_FEATURE_AVX,             VEND_SHARED },
        { 29, CPU_FEATURE_F16C,            VEND_SHARED },
        { 30, CPU_FEATURE_RDRAND,          VEND_SHARED },
        { 31, CPU_FEATURE_HYPERVISOR,      VEND_SHARED },
    };
    const cpuid_feature_map_t regidmap_edx01[] = {
        { 0,  CPU_FEATURE_FPU,             VEND_SHARED },
        { 1,  CPU_FEATURE_VME,             VEND_SHARED },
        { 2,  CPU_FEATURE_DE,              VEND_SHARED },
        { 3,  CPU_FEATURE_PSE,             VEND_SHARED },
        { 4,  CPU_FEATURE_TSC,             VEND_SHARED },
        { 5,  CPU_FEATURE_MSR,             VEND_SHARED },
        { 6,  CPU_FEATURE_PAE,             VEND_SHARED },
        { 7,  CPU_FEATURE_MCE,             VEND_SHARED },
        { 8,  CPU_FEATURE_CX8,             VEND_SHARED },
        { 9,  CPU_FEATURE_APIC,            VEND_SHARED },
        { 11, CPU_FEATURE_SEP,             VEND_SHARED },
        { 12, CPU_FEATURE_MTRR,            VEND_SHARED },
        { 13, CPU_FEATURE_PGE,             VEND_SHARED },
        { 14, CPU_FEATURE_MCA,             VEND_SHARED },
        { 15, CPU_FEATURE_CMOV,            VEND_SHARED },
        { 16, CPU_FEATURE_PAT,             VEND_SHARED },
        { 17, CPU_FEATURE_PSE36,           VEND_SHARED },
        { 18, CPU_FEATURE_PN,              VEND_INTEL  },
        { 19, CPU_FEATURE_CLFLUSH,         VEND_SHARED },
        { 21, CPU_FEATURE_DTS,             VEND_INTEL  },
        { 22, CPU_FEATURE_ACPI,            VEND_INTEL  },
        { 23, CPU_FEATURE_MMX,             VEND_SHARED },
        { 24, CPU_FEATURE_FXSR,            VEND_SHARED },
        { 25, CPU_FEATURE_SSE,             VEND_SHARED },
        { 26, CPU_FEATURE_SSE2,            VEND_SHARED },
        { 27, CPU_FEATURE_SS,              VEND_INTEL  },
        { 28, CPU_FEATURE_HT,              VEND_SHARED },
        { 29, CPU_FEATURE_TM,              VEND_INTEL  },
        { 30, CPU_FEATURE_IA64,            VEND_INTEL  },
        { 31, CPU_FEATURE_PBE,             VEND_INTEL  },
    };
    const cpuid_feature_map_t regidmap_ebx07[] = {
        { 0,  CPU_FEATURE_FSGSBASE,        VEND_SHARED },
        { 1,  CPU_FEATURE_TSC_ADJUST,      VEND_INTEL  },
        { 2,  CPU_FEATURE_SGX,             VEND_INTEL  },
        { 3,  CPU_FEATURE_BMI1,            VEND_SHARED },
        { 4,  CPU_FEATURE_HLE,             VEND_INTEL  },
        { 5,  CPU_FEATURE_AVX2,            VEND_SHARED },
        { 7,  CPU_FEATURE_SMEP,            VEND_SHARED },
        { 8,  CPU_FEATURE_BMI2,            VEND_SHARED },
        { 9,  CPU_FEATURE_ERMS,            VEND_INTEL  },
        { 10, CPU_FEATURE_INVPCID,         VEND_INTEL  },
        { 11, CPU_FEATURE_RTM,             VEND_INTEL  },
        { 12, CPU_FEATURE_CQM,             VEND_INTEL  },
        { 14, CPU_FEATURE_MPX,             VEND_INTEL  },
        { 16, CPU_FEATURE_AVX512F,         VEND_INTEL  },
        { 17, CPU_FEATURE_AVX512DQ,        VEND_INTEL  },
        { 18, CPU_FEATURE_RDSEED,          VEND_SHARED },
        { 19, CPU_FEATURE_ADX,             VEND_SHARED },
        { 20, CPU_FEATURE_SMAP,            VEND_SHARED },
        { 22, CPU_FEATURE_PCOMMIT,         VEND_INTEL  },
        { 23, CPU_FEATURE_CLFLUSHOPT,      VEND_SHARED },
        { 24, CPU_FEATURE_CLWB,            VEND_INTEL  },
        { 26, CPU_FEATURE_AVX512PF,        VEND_INTEL  },
        { 27, CPU_FEATURE_AVX512ER,        VEND_INTEL  },
        { 28, CPU_FEATURE_AVX512CD,        VEND_INTEL  },
        { 29, CPU_FEATURE_SHA,             VEND_SHARED },
        { 30, CPU_FEATURE_AVX512BW,        VEND_INTEL  },
        { 31, CPU_FEATURE_AVX512VL,        VEND_INTEL  },
    };
    const cpuid_feature_map_t regidmap_edx07[] = {
        { 26, CPU_FEATURE_SPEC_CTRL,       VEND_SHARED },
    };
    const cpuid_feature_map_t regidmap_ecx81[] = {
        { 0,  CPU_FEATURE_LAHF_LM,         VEND_SHARED },
        { 1,  CPU_FEATURE_CMP_LEGACY,      VEND_AMD    },
        { 2,  CPU_FEATURE_SVM,             VEND_AMD    },
        { 3,  CPU_FEATURE_EXTAPIC,         VEND_AMD    },
        { 4,  CPU_FEATURE_CR8_LEGACY,      VEND_AMD    },
        { 5,  CPU_FEATURE_ABM,             VEND_AMD    },
        { 6,  CPU_FEATURE_SSE4A,           VEND_AMD    },
        { 7,  CPU_FEATURE_MISALIGNSSE,     VEND_AMD    },
        { 8,  CPU_FEATURE_3DNOWPREFETCH,   VEND_AMD    },
        { 9,  CPU_FEATURE_OSVW,            VEND_AMD    },
        { 10, CPU_FEATURE_IBS,             VEND_AMD    },
        { 11, CPU_FEATURE_XOP,             VEND_AMD    },
        { 12, CPU_FEATURE_SKINIT,          VEND_AMD    },
        { 13, CPU_FEATURE_WDT,             VEND_AMD    },
        { 15, CPU_FEATURE_LWP,             VEND_AMD    },
        { 16, CPU_FEATURE_FMA4,            VEND_AMD    },
        { 17, CPU_FEATURE_TCE,             VEND_AMD    },
        { 19, CPU_FEATURE_NODEID_MSR,      VEND_AMD    },
        { 21, CPU_FEATURE_TBM,             VEND_AMD    },
        { 22, CPU_FEATURE_TOPOEXT,         VEND_AMD    },
        { 23, CPU_FEATURE_PERFCTR_CORE,    VEND_AMD    },
        { 24, CPU_FEATURE_PERFCTR_NB,      VEND_AMD    },
        { 26, CPU_FEATURE_BPEXT,           VEND_AMD    },
        { 28, CPU_FEATURE_PERFCTR_L2,      VEND_AMD    },
        { 29, CPU_FEATURE_MONITORX,        VEND_AMD    },
    };
    const cpuid_feature_map_t regidmap_edx81[] = {
        { 11, CPU_FEATURE_SYSCALL,         VEND_SHARED },
        { 20, CPU_FEATURE_NX,              VEND_SHARED },
        { 22, CPU_FEATURE_MMXEXT,          VEND_AMD    },
        { 25, CPU_FEATURE_FXSR_OPT,        VEND_AMD    },
        { 26, CPU_FEATURE_PDPE1GB,         VEND_SHARED },
        { 27, CPU_FEATURE_RDTSCP,          VEND_SHARED },
        { 29, CPU_FEATURE_LM,              VEND_SHARED },
        { 30, CPU_FEATURE_3DNOWEXT,        VEND_AMD    },
        { 31, CPU_FEATURE_3DNOW,           VEND_AMD    },
    };
    const cpuid_feature_map_t regidmap_edx87[] = {
        { 0,  CPU_FEATURE_TS,              VEND_AMD    },
        { 1,  CPU_FEATURE_FID,             VEND_AMD    },
        { 2,  CPU_FEATURE_VID,             VEND_AMD    },
        { 3,  CPU_FEATURE_TTP,             VEND_AMD    },
        { 4,  CPU_FEATURE_TM_AMD,          VEND_AMD    },
        { 5,  CPU_FEATURE_STC,             VEND_AMD    },
        { 6,  CPU_FEATURE_100MHZSTEPS,     VEND_AMD    },
        { 7,  CPU_FEATURE_HWPSTATE,        VEND_AMD    },
        { 8,  CPU_FEATURE_CONSTANT_TSC,    VEND_SHARED },
        { 9,  CPU_FEATURE_CPB,             VEND_AMD    },
        { 10, CPU_FEATURE_APERFMPERF,      VEND_AMD    },
        { 11, CPU_FEATURE_PFI,             VEND_AMD    },
        { 12, CPU_FEATURE_PA,              VEND_AMD    },
    };
    const cpuid_feature_map_t regidmap_ebx88[] = {
        {  0, CPU_FEATURE_CLZERO,          VEND_AMD    },
        {  1, CPU_FEATURE_IRPERF,          VEND_AMD    },
    };
    const cpuid_feature_map_t regidmap_eax_8000_1F[] = {
        {  0, CPU_FEATURE_SME,             VEND_AMD    },
        {  1, CPU_FEATURE_SEV,             VEND_AMD    },
        {  2, CPU_FEATURE_PAGEFLUSH,       VEND_AMD    },
        {  3, CPU_FEATURE_SEV_ES,          VEND_AMD    },
    };

    if (data->cpuid_max_basic >= 1) {
        set_feature_bits(data, regidmap_ecx01, NELEMS(regidmap_ecx01), raw->cpuid[1][ecx]);
        set_feature_bits(data, regidmap_edx01, NELEMS(regidmap_edx01), raw->cpuid[1][edx]);
    }
    if (data->cpuid_max_basic >= 7) {
        set_feature_bits(data, regidmap_ebx07, NELEMS(regidmap_ebx07), raw->cpuid[7][ebx]);
        set_feature_bits(data, regidmap_edx07, NELEMS(regidmap_edx07), raw->cpuid[7][edx]);
    }
    if (data->cpuid_max_ext >= 0x80000001) {
        set_feature_bits(data, regidmap_ecx81, NELEMS(regidmap_ecx81), raw->cpuid_ext[1][ecx]);
        set_feature_bits(data, regidmap_edx81, NELEMS(regidmap_edx81), raw->cpuid_ext[1][edx]);
    }
    if (data->cpuid_max_ext >= 0x80000007)
        set_feature_bits(data, regidmap_edx87, NELEMS(regidmap_edx87), raw->cpuid_ext[7][edx]);
    if (data->cpuid_max_ext >= 0x80000008)
        set_feature_bits(data, regidmap_ebx88, NELEMS(regidmap_ebx88), raw->cpuid_ext[8][ebx]);
    if (data->cpuid_max_ext >= 0x8000001F)
        set_feature_bits(data, regidmap_eax_8000_1F, NELEMS(regidmap_eax_8000_1F), raw->cpuid_ext[31][eax]);
}

void set_cpuid_xfeatures(cpuid_data_t *data, const uint64_t xcr0)
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
