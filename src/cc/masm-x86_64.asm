
.code

cpuid_is_supported proc
    mov eax, 1
    ret
cpuid_is_supported endp

icuid_cpuid proc
    mov r8, rdx

    mov eax, ecx
    xor ecx, ecx

    push rbx

    cpuid

    mov   [r8], eax
    mov  4[r8], ebx
    mov  8[r8], ecx
    mov 12[r8], edx

    pop rbx
    ret
icuid_cpuid endp

icuid_cpuid_ext proc
    mov r8, rcx

    mov eax,   [r8]
    mov ecx,  8[r8]

    push rbx

    cpuid

    mov   [r8], eax
    mov  4[r8], ebx
    mov  8[r8], ecx
    mov 12[r8], edx

    pop rbx
    ret
icuid_cpuid_ext endp

icuid_xgetbv proc
    db 15, 1, 208 ; xgetbv
    shl rdx, 32
    or  rdx, rax
    mov rax, rdx
    ret

icuid_xgetbv endp

END
