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

/* Specific functions for Visual C */

#include <icuid/icuid_types.h>

#if _WIN64
extern void __run_cpuid(uint32_t *regs);
#else

int cpuid_is_supported(void)
{
    int rv;
    __asm {
        pushfd
        pop eax
        mov ecx, eax
        xor eax, 0x200000
        push eax
        popfd
        pushfd
        pop eax
        xor eax, ecx
        mov rv, eax
    }
    return (rv != 0);
}

static void __run_cpuid(uint32_t *regs)
{
    __asm {
        push ebx
        push ecx
        push edx
        push edi
        mov edi, regs

        mov eax, [edi]
        mov ebx, [edi+4]
        mov ecx, [edi+8]
        mov edx, [edi+12]

        cpuid

        mov [edi],    eax
        mov [edi+4],  ebx
        mov [edi+8],  ecx
        mov [edi+12], edx

        pop edi
        pop edx
        pop ecx
        pop ebx
    }
}

uint64_t icuid_xgetbv(const uint32_t xcr)
{
    __asm {
        mov ecx, xcr
        _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0 /* xgetbv */
        shl edx, 32
        or  edx, eax
        mov eax, edx
    }
}
#endif /* !_WIN64 */

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
