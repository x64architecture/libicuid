/*
 * Copyright (c) 2015 - 2019, Kurt Cancemi (kurt@x64architecture.com)
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

void icuid_cpuid(uint32_t level, uint32_t *regs)
{
    __asm {
        mov edi, regs

        mov eax, level
        xor ecx, ecx

        cpuid

        mov   [edi], eax
        mov  4[edi], ebx
        mov  8[edi], ecx
        mov 12[edi], edx
    }
}

void icuid_cpuid_ext(uint32_t *regs)
{
    __asm {
        mov edi, regs

        mov eax,   [edi]
        mov ecx,  8[edi]

        cpuid

        mov   [edi], eax
        mov  4[edi], ebx
        mov  8[edi], ecx
        mov 12[edi], edx
    }
}
