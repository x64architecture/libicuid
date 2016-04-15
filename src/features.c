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
                      const int num, const uint32_t reg)
{
    int i;
    for (i = 0; i < num; i++) {
        if (reg & (1 << feature[i].bit))
            data->flags[feature[i].feature] = 1;
    }
}

/* Map feature to name */
const char *cpu_feature_str(cpuid_feature_t feature)
{
    int i, n;
    const struct {
        cpuid_feature_t feature;
        const char *name;
    } featurestr[] = {
        { CPU_FEATURE_FPU, "fpu" },
        { CPU_FEATURE_VME, "vme" },
        { CPU_FEATURE_DE, "de" },
        { CPU_FEATURE_PSE, "pse" },
        { CPU_FEATURE_TSC, "tsc" },
        { CPU_FEATURE_MSR, "msr" },
        { CPU_FEATURE_PAE, "pae" },
        { CPU_FEATURE_MCE, "mce" },
        { CPU_FEATURE_CX8, "cx8" },
        { CPU_FEATURE_APIC, "apic" },
        { CPU_FEATURE_MTRR, "mtrr" },
        { CPU_FEATURE_SEP, "sep" },
        { CPU_FEATURE_PGE, "pge" },
        { CPU_FEATURE_MCA, "mca" },
        { CPU_FEATURE_CMOV, "cmov" },
        { CPU_FEATURE_PAT, "pat" },
        { CPU_FEATURE_PSE36, "pse36" },
        { CPU_FEATURE_PN, "pn" },
        { CPU_FEATURE_CLFLUSH, "clflush" },
        { CPU_FEATURE_DTS, "dts" },
        { CPU_FEATURE_ACPI, "acpi" },
        { CPU_FEATURE_MMX, "mmx" },
        { CPU_FEATURE_FXSR, "fxsr" },
        { CPU_FEATURE_PDPE1GB, "pdpe1gb" },
        { CPU_FEATURE_SSE, "sse" },
        { CPU_FEATURE_SSE2, "sse2" },
        { CPU_FEATURE_SS, "ss" },
        { CPU_FEATURE_HT, "ht" },
        { CPU_FEATURE_TM, "tm" },
        { CPU_FEATURE_IA64, "ia64" },
        { CPU_FEATURE_PBE, "pbe" },
        { CPU_FEATURE_PNI, "pni" },
        { CPU_FEATURE_PCLMULDQ, "pclmuldq" },
        { CPU_FEATURE_DTS64, "dts64" },
        { CPU_FEATURE_MONITOR, "monitor" },
        { CPU_FEATURE_DS_CPL, "ds_cpl" },
        { CPU_FEATURE_VMX, "vmx" },
        { CPU_FEATURE_SMX, "smx" },
        { CPU_FEATURE_EST, "est" },
        { CPU_FEATURE_TM2, "tm2" },
        { CPU_FEATURE_SSSE3, "ssse3" },
        { CPU_FEATURE_CID, "cid" },
        { CPU_FEATURE_SDBG, "sdbg" },
        { CPU_FEATURE_CX16, "cx16" },
        { CPU_FEATURE_XTPR, "xtpr" },
        { CPU_FEATURE_PDCM, "pdcm" },
        { CPU_FEATURE_PCID, "pcid" },
        { CPU_FEATURE_DCA, "dca" },
        { CPU_FEATURE_SSE4_1, "sse4.1" },
        { CPU_FEATURE_SSE4_2, "sse4.2" },
        { CPU_FEATURE_SYSCALL, "syscall" },
        { CPU_FEATURE_X2APIC, "x2apic" },
        { CPU_FEATURE_MOVBE, "movbe" },
        { CPU_FEATURE_POPCNT, "popcnt" },
        { CPU_FEATURE_TSC_DEADLINE, "tsc_deadline_timer" },
        { CPU_FEATURE_AES, "aes" },
        { CPU_FEATURE_XSAVE, "xsave" },
        { CPU_FEATURE_OSXSAVE, "osxsave" },
        { CPU_FEATURE_AVX, "avx" },
        { CPU_FEATURE_MMXEXT, "mmxext" },
        { CPU_FEATURE_3DNOW, "3dnow" },
        { CPU_FEATURE_3DNOWEXT, "3dnowext" },
        { CPU_FEATURE_NX, "nx" },
        { CPU_FEATURE_FXSR_OPT, "fxsr_opt" },
        { CPU_FEATURE_RDTSCP, "rdtscp" },
        { CPU_FEATURE_LM, "lm" },
        { CPU_FEATURE_LAHF_LM, "lahf_lm" },
        { CPU_FEATURE_CMP_LEGACY, "cmp_legacy" },
        { CPU_FEATURE_SVM, "svm" },
        { CPU_FEATURE_SSE4A, "sse4a" },
        { CPU_FEATURE_MISALIGNSSE, "misalignsse" },
        { CPU_FEATURE_ABM, "abm" },
        { CPU_FEATURE_3DNOWPREFETCH, "3dnowprefetch" },
        { CPU_FEATURE_OSVW, "osvw" },
        { CPU_FEATURE_IBS, "ibs" },
        { CPU_FEATURE_SKINIT, "skinit" },
        { CPU_FEATURE_WDT, "wdt" },
        { CPU_FEATURE_TS, "ts" },
        { CPU_FEATURE_FID, "fid" },
        { CPU_FEATURE_VID, "vid" },
        { CPU_FEATURE_TTP, "ttp" },
        { CPU_FEATURE_TM_AMD, "tm_amd" },
        { CPU_FEATURE_STC, "stc" },
        { CPU_FEATURE_100MHZSTEPS, "100mhzsteps" },
        { CPU_FEATURE_HWPSTATE, "hwpstate" },
        { CPU_FEATURE_CONSTANT_TSC, "constant_tsc" },
        { CPU_FEATURE_XOP, "xop" },
        { CPU_FEATURE_FMA, "fma" },
        { CPU_FEATURE_FMA4, "fma4" },
        { CPU_FEATURE_TBM, "tbm" },
        { CPU_FEATURE_F16C, "f16c" },
        { CPU_FEATURE_RDRAND, "rdrand" },
        { CPU_FEATURE_CPB, "cpb" },
        { CPU_FEATURE_APERFMPERF, "aperfmperf" },
        { CPU_FEATURE_PFI, "pfi" },
        { CPU_FEATURE_PA, "pa" },
        { CPU_FEATURE_AVX2, "avx2" },
        { CPU_FEATURE_BMI1, "bmi1" },
        { CPU_FEATURE_BMI2, "bmi2" },
        { CPU_FEATURE_HYPERVISOR, "hypervisor" },
        { CPU_FEATURE_FSGSBASE, "fsgsbase" },
        { CPU_FEATURE_HLE, "hle" },
        { CPU_FEATURE_SMEP, "smep" },
        { CPU_FEATURE_ERMS, "erms" },
        { CPU_FEATURE_INVPCID, "invpcid" },
        { CPU_FEATURE_MPX, "mpx" },
        { CPU_FEATURE_RDSEED, "rdseed" },
        { CPU_FEATURE_ADX, "adx" },
        { CPU_FEATURE_SMAP, "smap" },
        { CPU_FEATURE_SHA, "sha" },
        { CPU_FEATURE_CLZERO, "clzero" },
        { CPU_FEATURE_IRPERF, "irperf" },
    };
    n = NELEMS(featurestr);
    if (n != NUM_CPU_FEATURES) {
        fprintf(stderr, "Table needs to be updated. %s, %d\n", __FILE__, __LINE__);
        abort();
    }
    for (i = 0; i < NUM_CPU_FEATURES; i++) {
        if (featurestr[i].feature == feature)
            return featurestr[i].name;
    }
    return "";
}

/* Set common features (shared between Intel & AMD) */
void set_common_features(const cpuid_raw_data_t *raw, cpuid_data_t *data)
{
    const cpuid_feature_map_t regidmap_edx1[] = {
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
        { 26, CPU_FEATURE_PDPE1GB },
        { 25, CPU_FEATURE_SSE },
        { 26, CPU_FEATURE_SSE2 },
        { 27, CPU_FEATURE_RDTSCP },
        { 28, CPU_FEATURE_HT },
    };
    const cpuid_feature_map_t regidmap_ecx1[] = {
        { 0, CPU_FEATURE_PNI },
        { 3, CPU_FEATURE_MONITOR },
        { 9, CPU_FEATURE_SSSE3 },
        { 12, CPU_FEATURE_FMA },
        { 13, CPU_FEATURE_CX16 },
        { 19, CPU_FEATURE_SSE4_1 },
        { 23, CPU_FEATURE_POPCNT },
        { 25, CPU_FEATURE_AES },
        { 26, CPU_FEATURE_XSAVE },
        { 27, CPU_FEATURE_OSXSAVE },
        { 28, CPU_FEATURE_AVX },
        { 29, CPU_FEATURE_F16C },
        { 31, CPU_FEATURE_HYPERVISOR },
    };
    const cpuid_feature_map_t regidmap_ebx7[] = {
        { 3, CPU_FEATURE_BMI1 },
        { 5, CPU_FEATURE_AVX2 },
        { 8, CPU_FEATURE_BMI2 },
    };
    const cpuid_feature_map_t regidmap_edx81[] = {
        { 5, CPU_FEATURE_ABM },
        { 20, CPU_FEATURE_NX },
        { 11, CPU_FEATURE_SYSCALL },
        { 29, CPU_FEATURE_LM },
    };
    const cpuid_feature_map_t regidmap_ecx81[] = {
        { 0, CPU_FEATURE_LAHF_LM },
    };
    const cpuid_feature_map_t regidmap_edx87[] = {
        { 8, CPU_FEATURE_CONSTANT_TSC },
    };

    if (data->cpuid_max_basic >= 1) {
        set_feature_bits(data, regidmap_edx1, NELEMS(regidmap_edx1), raw->cpuid[1][3]);
        set_feature_bits(data, regidmap_ecx1, NELEMS(regidmap_ecx1), raw->cpuid[1][2]);
    }
    if (data->cpuid_max_basic >= 7)
        set_feature_bits(data, regidmap_ebx7, NELEMS(regidmap_ebx7), raw->cpuid[7][1]);
    if (data->cpuid_max_ext >= 0x80000001) {
        set_feature_bits(data, regidmap_edx81, NELEMS(regidmap_edx81), raw->cpuid_ext[1][3]);
        set_feature_bits(data, regidmap_ecx81, NELEMS(regidmap_ecx81), raw->cpuid_ext[1][2]);
    }
    if (data->cpuid_max_ext >= 0x80000007)
        set_feature_bits(data, regidmap_edx87, NELEMS(regidmap_edx87), raw->cpuid_ext[7][3]);
}

void set_common_xfeatures(cpuid_data_t *data, const uint64_t xcr0)
{
    int i;
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
        if (xcr0 & (uint64_t)(1 << xfeatures_t[i].bit))
            data->xfeatures[xfeatures_t[i].feature] = 1;
    }
}
