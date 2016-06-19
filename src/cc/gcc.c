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

/* Specific functions for gcc/clang */

#include <icuid/icuid_types.h>

#if defined(__x86_64) || defined(__x86_64__)
#define ICUID_X86_64
#elif defined(__x86) || defined(__i386) || defined(__i386__)
#define ICUID_X86
#endif

int cpuid_is_supported(void)
{
#if defined(ICUID_X86_64)
    /* CPUID is always supported in x86_64 */
    return 1;
#elif defined(ICUID_X86)
    int rv;
    __asm__ (
        "pushf\n"
        "pop %%eax\n"
        "mov %%eax, %%ecx\n"
        "xor $0x200000, %%eax\n"
        "push %%eax\n"
        "popf\n"
        "pushf\n"
        "pop %%eax\n"
        "xor %%ecx, %%eax\n"
        "mov %%eax, %0"
        : "=m"(rv)
        : : "memory");
    return (rv != 0);
#else
    #error "Unsupported CPU architecture"
#endif
}

void icuid_cpuid(uint32_t eax, uint32_t *regs)
{
    __asm__ (
        "xor %%ecx, %%ecx\n"
        "cpuid"
        : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
        : "a"(eax)
    );
}

void icuid_cpuid_ext(uint32_t *regs)
{
    __asm__ (
        "cpuid"
        : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
        : "a"(regs[0]), "c"(regs[2])
    );
}

uint64_t icuid_xgetbv(const uint32_t xcr)
{
    uint32_t eax, edx;

    __asm__ (
        ".byte 0x0f, 0x01, 0xd0" /* xgetbv */
        : "=a"(eax), "=d"(edx) : "c"(xcr)
    );

    return (((uint64_t)edx) << 32) | eax;
}
