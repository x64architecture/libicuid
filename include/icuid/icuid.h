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

#ifndef __LIBICUID_H__
#define __LIBICUID_H__

#include <icuid/icuid_err.h>
#include <icuid/icuid_limits.h>
#include <icuid/icuid_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @mainpage libicuid
 *
 * libicuid is a library that provides a C interface for the CPUID opcode
 * written by Kurt Cancemi and is licensed under the ISC License.
 *
 * Brief intro to some of the available functions
 * <p>
 * To check for the CPUID instruction, use \ref cpuid_is_supported <br>
 * To run the CPUID instruction, use \ref cpuid <br>
 * To get the raw CPUID info needed for CPU identification, use
 *   \ref cpuid_get_raw_data <br>
 * To decode that raw info use \ref icuid_identify <br>
 * </p>
 */

/**
 * @brief Run the cpuid instruction
 * @param [in] eax - passed in to the EAX register when executing cpuid
 * @param [out] regs - result of cpuid. regs[0] = EAX, regs[1] = EBX, regs[2] = ecx, ...
 * @note cpuid will be executed with the input eax into the EAX register and all of the
         other registers (EBX, ECX, EDX) will be set to 0.
 */
void cpuid(uint32_t eax, uint32_t *regs);

/**
 * @brief Run the cpuid instruction with custom register values
 * @param [in] regs - regs[0] (EAX), regs[1] (EBX), regs[2] (ECX), regs[3] (EDX)
 * @param [out] regs - result of cpuid. regs[0] = EAX, regs[1] = EBX, regs[2] = ecx, ...
 * @note cpuid will be executed with the input regs copied into their respective registers
         and the original input registers will be overwritten with the output of cpuid
 */
void cpuid_ext(uint32_t *regs);

/**
 * @brief Check if the cpuid instruction is supported
 * @retval ICUID_OK if the cpuid instruction is supported.
 * @retval ICUID_NO_CPUID if the cpuid instruction is unsupported.
 */
int cpuid_is_supported(void);

/**
 * @brief Run the xgetbv instruction
 * @param [in] xcr - passed in to the ECX register when executing xgetbv (can only be 0 atow)
 * @returns 64-bits of the extended control register (XCR) specified by |xcr|
 * @warning This function will cause a crash if an unsupported XCR is used. For example
            icuid_xgetbv(0) can't be used if data->features[CPU_FEATURE_OSXSAVE] is not set
 */
uint64_t icuid_xgetbv(const uint32_t xcr);

/**
 * @brief CPU feature bits
 *
 * Usage:
 * @code
 * cpuid_data_t data;
 * int ret;
 * ...
 * ret = icuid_identify(NULL, &data)
 * if (ret == ICUID_OK) {
 *     if (data->flags[CPU_FEATURE_AVX2] && data->xfeatures[XFEATURE_AVX]) {
 *         // The CPU has AVX2 and AVX (YMM) registers are supported by the OS
 *     } else {
 *         // AVX2 unsupported
 *     }
 * } else {
 *     // Error getting cpu info
 * }
 * @endcode
 */
typedef enum {
    /* cpuid 0x00000001, ecx */
    CPU_FEATURE_PNI = 0,       /*!< PNI (SSE3) Instructions Supported */
    CPU_FEATURE_PCLMULDQ,      /*!< PCLMULDQ Instruction Supported */
    CPU_FEATURE_DTS64,         /*!< 64-bit Debug Store Supported */
    CPU_FEATURE_MONITOR,       /*!< MONITOR / MWAIT Supported */
    CPU_FEATURE_DS_CPL,        /*!< CPL Qualified Debug Store */
    CPU_FEATURE_VMX,           /*!< Virtualization Technology Supported */
    CPU_FEATURE_SMX,           /*!< Safer Mode Exceptions */
    CPU_FEATURE_EST,           /*!< Enhanced SpeedStep */
    CPU_FEATURE_TM2,           /*!< Thermal Monitor 2 */
    CPU_FEATURE_SSSE3,         /*!< SSSE3 Instructions Supported */
    CPU_FEATURE_CID,           /*!< Context ID Supported */
    CPU_FEATURE_SDBG,          /*!< Silicon Debug Supported */
    CPU_FEATURE_FMA,           /*!< The FMA Instruction Set */
    CPU_FEATURE_CX16,          /*!< CMPXCHG16B Instruction Supported */
    CPU_FEATURE_XTPR,          /*!< Send Task Priority Messages Disable */
    CPU_FEATURE_PDCM,          /*!< Performance Capabilities MSR Supported */
    /* 16 Reserved */
    CPU_FEATURE_PCID,          /*!< Process Context Identifiers Supported */
    CPU_FEATURE_DCA,           /*!< Direct Cache Access Supported */
    CPU_FEATURE_SSE4_1,        /*!< SSE 4.1 Instructions Supported */
    CPU_FEATURE_SSE4_2,        /*!< SSE 4.2 Instructions Supported */
    CPU_FEATURE_X2APIC,        /*!< x2APIC Support */
    CPU_FEATURE_MOVBE,         /*!< MOVBE Instruction Supported */
    CPU_FEATURE_POPCNT,        /*!< POPCNT Instruction Supported */
    CPU_FEATURE_TSC_DEADLINE,  /*!< APIC Supports One-Shot Operation Using A TSC Deadline Value */
    CPU_FEATURE_AES,           /*!< AES Instructions Supported */
    CPU_FEATURE_XSAVE,         /*!< XSAVE/XRSTOR/XSETBV/XGETBV Instructions Supported */
    CPU_FEATURE_OSXSAVE,       /*!< Indicates The OS Enabled Support For XSAVE */
    CPU_FEATURE_AVX,           /*!< Advanced Vector Extensions Supported */
    CPU_FEATURE_F16C,          /*!< 16-bit FP convert Instruction Support */
    CPU_FEATURE_RDRAND,        /*!< RDRAND Instruction */
    CPU_FEATURE_HYPERVISOR,    /*!< Running On a Hypervisor */

    /* cpuid 0x00000001, edx */
    CPU_FEATURE_FPU,           /*!< Floating Point Unit On-Chip */
    CPU_FEATURE_VME,           /*!< Virtual Mode Extension */
    CPU_FEATURE_DE,            /*!< Debugging Extension */
    CPU_FEATURE_PSE,           /*!< Page Size Extension */
    CPU_FEATURE_TSC,           /*!< Time Stamp Counter */
    CPU_FEATURE_MSR,           /*!< Model Specific Registers, RDMSR/WRMSR Supported */
    CPU_FEATURE_PAE,           /*!< Physical Address Extension */
    CPU_FEATURE_MCE,           /*!< Machine-Check Exception */
    CPU_FEATURE_CX8,           /*!< CMPXCHG8 Instruction Supported */
    CPU_FEATURE_APIC,          /*!< On-chip APIC Support */
    /* 10 Reserved */
    CPU_FEATURE_SEP,           /*!< Fast System Call (SYSENTER/SYSEXIT) Instructions Supported */
    CPU_FEATURE_MTRR,          /*!< Memory Type Range Registers */
    CPU_FEATURE_PGE,           /*!< Page Global Enable */
    CPU_FEATURE_MCA,           /*!< Machine-Check Architecture */
    CPU_FEATURE_CMOV,          /*!< Conditional Move (CMOVxx) Instructions Supported */
    CPU_FEATURE_PAT,           /*!< Page Attribute Table */
    CPU_FEATURE_PSE36,         /*!< 36-bit Page Size Extension */
    CPU_FEATURE_PN,            /*!< Processor Serial # Implemented (Intel P3 only) */
    CPU_FEATURE_CLFLUSH,       /*!< CLFLUSH Instruction Supported */
    CPU_FEATURE_DTS,           /*!< Debug Store Supported */
    CPU_FEATURE_ACPI,          /*!< ACPI support (Power States) */
    CPU_FEATURE_MMX,           /*!< MMX Instruction Set Supported */
    CPU_FEATURE_FXSR,          /*!< FXSAVE/FXRSTOR Supported */
    CPU_FEATURE_SSE,           /*!< Streaming SIMD Extensions (SSE) Supported */
    CPU_FEATURE_SSE2,          /*!< Streaming SIMD Extensions 2 (SSE2) Instructions Supported */
    CPU_FEATURE_SS,            /*!< Self-Snoop */
    CPU_FEATURE_HT,            /*!< Hyper-Threading Supported By CPU */
    CPU_FEATURE_TM,            /*!< Thermal Monitor */
    CPU_FEATURE_IA64,          /*!< IA64 Supported (Itanium only) */
    CPU_FEATURE_PBE,           /*!< Pending-Break Enable */

    /* cpuid 0x00000007, ebx */
    CPU_FEATURE_FSGSBASE,      /*!< Access To Base Of %fs And %gs */
    CPU_FEATURE_TSC_ADJUST,    /*!< TSC Adjustment MSR 0x3b */
    CPU_FEATURE_SGX,           /*!< Software Guard Extensions */
    CPU_FEATURE_BMI1,          /*!< BMI1 Instructions */
    CPU_FEATURE_HLE,           /*!< Transactional Synchronization Extensions */
    CPU_FEATURE_AVX2,          /*!< AVX2 Instructions */
    /* 6 Reserved */
    CPU_FEATURE_SMEP,          /*!< Supervisor-Mode Execution Prevention */
    CPU_FEATURE_BMI2,          /*!< BMI2 Instructions */
    CPU_FEATURE_ERMS,          /*!< Enhanced REP MOVSB/STOSB Instructions */
    CPU_FEATURE_INVPCID,       /*!< INVPCID Instruction */
    CPU_FEATURE_RTM,           /*!< Restricted Transactional Memory */
    CPU_FEATURE_CQM,           /*!< Cache QoS Monitoring */
    /* 13 Reserved */
    CPU_FEATURE_MPX,           /*!< Intel MPX Extensions */
    /* 15 Reserved */
    CPU_FEATURE_AVX512F,       /*!< AVX-512 Foundation */
    CPU_FEATURE_AVX512DQ,      /*!< AVX-512 Doubleword and Quadword */
    CPU_FEATURE_RDSEED,        /*!< Intel RDSEED Instruction */
    CPU_FEATURE_ADX,           /*!< Intel ADX Extensions */
    CPU_FEATURE_SMAP,          /*!< Supervisor Mode Access Prevention */
    /* 21 Reserved */
    CPU_FEATURE_PCOMMIT,       /*!< PCOMMIT Instruction */
    CPU_FEATURE_CLFLUSHOPT,    /*!< CLFLUSHOPT Instruction */
    CPU_FEATURE_CLWB,          /*!< CLWB Instruction */
    CPU_FEATURE_AVX512PF,      /*!< AVX-512 Prefetch Instructions */
    CPU_FEATURE_AVX512ER,      /*!< AVX-512 Exponential and Reciprocal Instructions */
    CPU_FEATURE_AVX512CD,      /*!< AVX-512 Conflict Detection Instructions */
    CPU_FEATURE_SHA,           /*!< Intel SHA Extensions */
    CPU_FEATURE_AVX512BW,      /*!< AVX-512 Byte and Word Instructions */
    CPU_FEATURE_AVX512VL,      /*!< AVX-512 Vector Length Instructions */

    /* cpuid 0x80000001, ecx */
    CPU_FEATURE_LAHF_LM,       /*!< LAHF/SAHF Supported In 64-bit Mode */
    CPU_FEATURE_CMP_LEGACY,    /*!< Core Multi-Processing Legacy Mode (AMD Only) */
    CPU_FEATURE_SVM,           /*!< AMD Secure Virtual Machine (AMD Only) */
    CPU_FEATURE_EXTAPIC,       /*!< Extended APIC space (AMD Only) */
    CPU_FEATURE_CR8_LEGACY,    /*!< CR8 in 32-bit mode (AMD Only) */
    CPU_FEATURE_ABM,           /*!< Advanced Bit Manipulation (AMD Only) */
    CPU_FEATURE_SSE4A,         /*!< SSE 4A (AMD Only) */
    CPU_FEATURE_MISALIGNSSE,   /*!< Misaligned SSE Supported (AMD Only) */
    CPU_FEATURE_3DNOWPREFETCH, /*!< PREFETCH/PREFETCHW Support (AMD Only) */
    CPU_FEATURE_OSVW,          /*!< OS Visible Workaround (AMD Only) */
    CPU_FEATURE_IBS,           /*!< Instruction-Based Sampling (AMD Only) */
    CPU_FEATURE_XOP,           /*!< The XOP Instruction Set (AMD Only) */
    CPU_FEATURE_SKINIT,        /*!< SKINIT/STGI Supported (AMD Only) */
    CPU_FEATURE_WDT,           /*!< Watchdog Timer Support (AMD Only) */
    /* 14 Reserved */
    CPU_FEATURE_LWP,           /*!< Light Weight Profiling (AMD Only) */
    CPU_FEATURE_FMA4,          /*!< The FMA4 Instruction Set (AMD Only) */
    CPU_FEATURE_TCE,           /*!< Translation Cache Extension (AMD Only) */
    /* 18 Reserved */
    CPU_FEATURE_NODEID_MSR,    /*!< NodeId MSR (AMD Only) */
    /* 20 Reserved */
    CPU_FEATURE_TBM,           /*!< Trailing bit manipulation Instruction support (AMD Only) */
    CPU_FEATURE_TOPOEXT,       /*!< Topology extension CPUID leafs (AMD Only) */
    CPU_FEATURE_PERFCTR_CORE,  /*!< Core Performance Counter Extensions (AMD Only) */
    CPU_FEATURE_PERFCTR_NB,    /*!< NB Core Performance Counter Extensions (AMD Only) */
    /* 25 Reserved */
    CPU_FEATURE_BPEXT,         /*!< Data Breakpoint Extension (AMD Only) */
    /* 27 Reserved */
    CPU_FEATURE_PERFCTR_L2,    /*!< L2 Performance Counter Extensions (AMD Only) */
    CPU_FEATURE_MONITORX,      /*!< MONITORX / MWAITX Supported (AMD Only) */

    /* cpuid 0x80000001, edx */
    /* 0-10 Reserved */
    CPU_FEATURE_SYSCALL,       /*!< SYSCALL/SYSRET Instructions Supported */
    /* 12-19 Reserved */
    CPU_FEATURE_NX,            /*!< No-Execute Bit Supported */
    /* 21 Reserved */
    CPU_FEATURE_MMXEXT,        /*!< AMD MMX-Extended Instructions Supported */
    /* 23-24 Reserved */
    CPU_FEATURE_FXSR_OPT,      /*!< FXSAVE and FXRSTOR Instructions (AMD Only) */
    CPU_FEATURE_PDPE1GB,       /*!< Gibibyte Pages Supported */
    CPU_FEATURE_RDTSCP,        /*!< RDTSCP Instruction Supported */
    /* 28 Reserved */
    CPU_FEATURE_LM,            /*!< Long Mode (x86_64/EM64T) Supported */
    CPU_FEATURE_3DNOWEXT,      /*!< AMD 3DNow! Extended Instructions Supported */
    CPU_FEATURE_3DNOW,         /*!< AMD 3DNow! Instructions Supported */

    /* cpuid 0x80000007, edx */
    CPU_FEATURE_TS,            /*!< Temperature Sensor (AMD Only) */
    CPU_FEATURE_FID,           /*!< Frequency ID Control (AMD Only) */
    CPU_FEATURE_VID,           /*!< Voltage ID Control (AMD Only) */
    CPU_FEATURE_TTP,           /*!< THERMTRIP (AMD Only) */
    CPU_FEATURE_TM_AMD,        /*!< AMD Specified Hardware Thermal Control */
    CPU_FEATURE_STC,           /*!< Software Thermal Control (AMD Only) */
    CPU_FEATURE_100MHZSTEPS,   /*!< 100 MHz Multiplier Control (AMD Only) */
    CPU_FEATURE_HWPSTATE,      /*!< Hardware P-state Control (AMD Only) */
    CPU_FEATURE_CONSTANT_TSC,  /*!< TSC ticks At A Constant Rate */
    CPU_FEATURE_CPB,           /*!< Core Performance Boost (AMD Only) */
    CPU_FEATURE_APERFMPERF,    /*!< MPERF/APERF MSRs support (AMD Only) */
    CPU_FEATURE_PFI,           /*!< Processor Feedback Interface Support (AMD Only) */
    CPU_FEATURE_PA,            /*!< Processor Accumulator (AMD Only) */
    /* 13-31 Reserved */

    /* cpuid 0x80000008, ebx */
    CPU_FEATURE_CLZERO,        /*!< CLZERO Instruction Support (AMD Only) */
    CPU_FEATURE_IRPERF,        /*!< Instructions Retired Count (AMD Only) */
    /* 2-31 Reserved */

    /* cpuid 0x8000001F, eax */
    CPU_FEATURE_SME,           /*!< Secure Memory Encryption Support (AMD Only) */
    /* 1-31 Reserved */

    NUM_CPU_FEATURES,
} cpuid_feature_t;

/**
 * @brief CPU vendor, as we determined from the Vendor String
 * @note HVs such as KVM don't usually report their own
 *       IDs; they usually report their hosts CPU vendor.
 */
typedef enum {
    VENDOR_UNKNOWN = 0, /*!< Unknown Vendor */
    VENDOR_INTEL,       /*!< Intel CPU */
    VENDOR_AMD,         /*!< AMD CPU */
    VENDOR_CYRIX,       /*!< Cyrix CPU */
    VENDOR_NEXGEN,      /*!< NexGen CPU */
    VENDOR_TRANSMETA,   /*!< Transmeta CPU */
    VENDOR_UMC,         /*!< UMC CPU */
    VENDOR_CENTAUR,     /*!< IDT CPU */
    VENDOR_RISE,        /*!< Rise CPU */
    VENDOR_SIS,         /*!< SiS CPU */
    VENDOR_NSC,         /*!< National Semiconductor CPU */
    VENDOR_VIA,         /*!< VIA CPU */
    VENDOR_HV_KVM,      /*!< KVM HV */
    VENDOR_HV_HYPERV,   /*!< Microsoft Hyper-V */
    VENDOR_HV_VMWARE,   /*!< VMware HV */
    VENDOR_HV_XEN,      /*!< Xen HV */
    NUM_CPU_VENDORS,
} cpu_vendor_t;

/**
 * @brief XSAVE Features, used to determine if a particular
 *        feature is supported and enabled by the OS.
 */
typedef enum {
    XFEATURE_FP = 0,    /*!< x87 FPU State */
    XFEATURE_SSE,       /*!< SSE (XMM) State */
    XFEATURE_AVX,       /*!< AVX (YMM) State */
    XFEATURE_BNDREGS,   /*!< MPX: BND0-BND3 Regs State*/
    XFEATURE_BNDCSR,    /*!< MPX: Bounds configuration and status component State */
    XFEATURE_OPMASK,    /*!< AVX-512: Opmask Regs State */
    XFEATURE_ZMM_Hi256, /*!< AVX-512: ZMM0-ZMM15 Regs State */
    XFEATURE_Hi16_ZMM,  /*!< AVX-512: ZMM16-ZMM31 Regs State */
    XFEATURE_IA32_XSS,  /*!< Extended Supervisor State Mask (R/W) MSR State */
    XFEATURE_PKRU,      /*!< Protection Key Rights register for User pages State */
    NUM_XFEATURES,
} xfeature_t;

typedef struct {
    /**
     * Basic CPUID Information
     * Contains: results of cpuid when eax=[0-7]
     */
    uint32_t cpuid[MAX_CPUID_LEVEL][4];

    /**
     * Extended CPUID Information
     * Contains: results of cpuid when eax=[0x80000000-0x80000008]
     */
    uint32_t cpuid_ext[MAX_EXT_CPUID_LEVEL][4];

    /**
     * Intel Deterministic Cache
     * Contains: eax=4 and ecx=[0-3]
     */
    uint32_t intel_dc[MAX_INTEL_DC_LEVEL][4];

    /**
     * Intel Extended Topology
     * Contains: eax=11 and ecx=[0-3]
     */
    uint32_t intel_et[MAX_INTEL_ET_LEVEL][4];
} cpuid_raw_data_t;

typedef struct {
    /** Contains the vendor string */
    char vendor_str[VENDOR_STR_MAX];
    /** Contains the brand string */
    char brand_str[BRAND_STR_MAX];

    /** Contains the vendor; ID'd by us */
    cpu_vendor_t vendor;

    /** Contains the CPU family */
    uint8_t family;
    /** Contains the CPU model */
    uint8_t model;
    /** Contains the CPU stepping */
    uint8_t stepping;
    /** Contains the CPU extended family */
    uint8_t ext_family;
    /** Contains the CPU extended model */
    uint8_t ext_model;
    /** Contains the CPU type */
    uint8_t type;
    /** Contains the CPU signature */
    uint32_t signature;
    /** Contains the CPU codename */
    const char *codename;

    /** Contains the max basic cpuid level */
    uint32_t cpuid_max_basic;
    /** Contains the max extended cpuid level */
    uint32_t cpuid_max_ext;
    /** Contains the feature flags */
    uint8_t flags[CPU_FLAGS_MAX];

    /** Number of cores on the current CPU */
    uint32_t cores;

    /**
     * Number of logical processors on the current CPU.
     * When Hyper-Threading is enabled the value will
     * typically be (cores * 2).
     */
    uint32_t logical_cpus;

    /**
     * L1 data cache size in kB.
     * If there is no L1 D cache, this will be 0.
     */
    uint32_t l1_data_cache;

    /**
     * L1 Instruction cache size in kB.
     * If there is no L1 I cache, this will be 0.
     */
    uint32_t l1_instruction_cache;

    /**
     * L2 data cache size in kB.
     * If there is no L2 cache, this will be 0.
     */
    uint32_t l2_cache;

    /**
     * L3 data cache size in kB.
     * If there is no L3 Cache, this will be 0.
     */
    uint32_t l3_cache;

    /** Cache associativity for the L1 cache. */
    uint32_t l1_associativity;

    /** Cache associativity for the L2 cache. */
    uint32_t l2_associativity;

    /** Cache associativity for the L3 cache. */
    uint32_t l3_associativity;

    /** Cache line size for the L1 cache. */
    uint32_t l1_cacheline;

    /** Cache line size for the L2 cache. */
    uint32_t l2_cacheline;

    /** Cache line size for the L3 cache. */
    uint32_t l3_cacheline;

    /** Physical Address bits. */
    uint32_t physical_address_bits;

    /** Virtual Address bits. */
    uint32_t virtual_address_bits;

    /** XSAVE features */
    uint8_t xfeatures[XFEATURE_FLAGS_MAX];
} cpuid_data_t;

/**
 * @brief Returns the short form of the CPU feature flag
 * @param feature [in] - the feature, whose short form is desired
 * @returns a (const char) string of the CPU feature flag; e.g. "avx"
 */
const char *cpu_feature_str(cpuid_feature_t feature);

/**
 * @brief Obtains the raw CPUID info from the CPU
 * @param raw [in] - a pointer to a cpuid_raw_data_t structure
 * @returns ICUID_OK if successful, and some other error code otherwise.
 *          The error message can be obtained by calling \ref icuid_errorstr.
 *
 */
int cpuid_get_raw_data(cpuid_raw_data_t *raw);

/**
 * @brief Writes the raw CPUID info to a file or stdout
 * @param raw [in] - a pointer to a cpuid_raw_data_t structure
 * @param file [in] - the path to the file, where the serialized raw data should
 *                    be written. If empty, stdout will be used instead.
 * @note This is primarily intended for debugging use. e.g. if someone has an
 *       error with the returned data they can send the raw data to us for us
 *       to fix. Backwards compatibility is *not* guaranteed nor is forwards
 *       compatibility.
 * @returns ICUID_OK if successful, and some other error code otherwise.
 *          The error message can be obtained by calling \ref icuid_errorstr.
 */
int cpuid_serialize_raw_data(cpuid_raw_data_t *raw, const char *file);

/**
 * @brief Reads the raw CPUID info to a file or stdin
 * @param raw [in] - a pointer to a cpuid_raw_data_t structure
 * @param file [in] - the path to the file, where the serialized raw data should
 *                    be read from. If empty, stdin will be used instead.
 * @note This is primarily intended for debugging use. e.g. if someone has an
 *       error with the returned data they can send the raw data to us for us
 *       to fix. Backwards compatibility is *not* guaranteed nor is forwards
 *       compatibility.
 * @returns ICUID_OK if successful, and some other error code otherwise.
 *          The error message can be obtained by calling \ref icuid_errorstr.
 */
int cpuid_deserialize_raw_data(cpuid_raw_data_t *raw, const char *file);

/**
 * @brief Identifies the CPU
 * @param raw [in] - a pointer to the raw CPUID data, which is obtained
 *              by cpuid_get_raw_data or by passing NULL, in which case
 *              the function calls cpuid_get_raw_data itself.
 * @param data [out] - the decoded CPU information
 * @note This function will not fail even if some info is not collected
 *       due to error or unsupported info.
 * @returns ICUID_OK if successful, and some other error code otherwise.
 *          The error message can be obtained by calling \ref icuid_errorstr.
 *
 */
int icuid_identify(cpuid_raw_data_t *raw, cpuid_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __LIBICUID_H__ */
