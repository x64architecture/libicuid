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

int cpuid_is_supported(void)
{
#if defined(__x86_64)
    int rv;
    __asm__ volatile(
        "pushfq\n"
        "pop %%rax\n"
        "mov %%rax, %%rcx\n"
        "xor $0x200000, %%rax\n"
        "push %%rax\n"
        "popfq\n"
        "pushfq\n"
        "pop %%rax\n"
        "xor %%rcx, %%rax\n"
        "mov %%eax, %0\n"
        : "=m"(rv)
        : : "rax", "rcx", "memory");
    return (rv != 0);
#elif defined(__i386__)
    int rv;
    __asm__ volatile(
        "pushfd\n"
        "pop %%eax\n"
        "mov %%eax, %%ecx\n"
        "xor $0x200000, %%eax\n"
        "push %%eax\n"
        "popfd\n"
        "pushfd\n"
        "pop %%eax\n"
        "xor %%ecx, %%eax\n"
        "mov %%eax, %0"
        : "=m"(rv)
        : : "memory");
    return (rv != 0);
#endif
    return 0;
}

static void __run_cpuid(uint32_t *regs)
{
#if defined(__x86_64)
    __asm__ volatile(
        "mov %0, %%rdi\n"

        "push %%rbx\n"
        "push %%rcx\n"
        "push %%rdx\n"

        "mov   (%%rdi), %%eax\n"
        "mov  4(%%rdi), %%ebx\n"
        "mov  8(%%rdi), %%ecx\n"
        "mov 12(%%rdi), %%edx\n"

        "cpuid\n"

        "mov %%eax,   (%%rdi)\n"
        "mov %%ebx,  4(%%rdi)\n"
        "mov %%ecx,  8(%%rdi)\n"
        "mov %%edx, 12(%%rdi)\n"
        "pop %%rdx\n"
        "pop %%rcx\n"
        "pop %%rbx\n"
        :
        :"m"(regs)
        :"memory", "eax", "rsi", "rdi"
    );
#elif defined(__i386__)
    __asm__ volatile(
        "mov %0, %%edi\n"

        "push %%ebx\n"
        "push %%ecx\n"
        "push %%edx\n"

        "mov   (%%edi), %%eax\n"
        "mov  4(%%edi), %%ebx\n"
        "mov  8(%%edi), %%ecx\n"
        "mov 12(%%edi), %%edx\n"

        "cpuid\n"

        "mov %%eax,   (%%edi)\n"
        "mov %%ebx,  4(%%edi)\n"
        "mov %%ecx,  8(%%edi)\n"
        "mov %%edx, 12(%%edi)\n"
        "pop %%edx\n"
        "pop %%ecx\n"
        "pop %%ebx\n"
        :
        :"m"(regs)
        :"memory", "eax", "edi"
    );
#endif
}

void cpuid(uint32_t eax, uint32_t *regs)
{
    regs[0] = eax;
    regs[1] = regs[2] = regs[3] = 0;
    __run_cpuid(regs);
}

void cpuid_ext(uint32_t *regs)
{
    __run_cpuid(regs);
}

uint64_t icuid_xgetbv(const uint32_t xcr)
{
    uint32_t eax, edx;

    __asm__ volatile(
        ".byte 0x0f, 0x01, 0xd0" /* xgetbv */
        : "=a"(eax), "=d"(edx) : "c"(xcr)
    );

    return (((uint64_t)edx) << 32) | eax;
}
