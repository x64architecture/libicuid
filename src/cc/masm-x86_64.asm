
.code

cpuid_is_supported proc
    mov eax, 1
    ret
cpuid_is_supported endp

icuid_cpuid proc
    push rbx

    mov rax, rcx
    mov rdi, rdx

    cpuid

    mov   [rdi], eax
    mov  4[rdi], ebx
    mov  8[rdi], ecx
    mov 12[rdi], edx

    pop rbx
    ret
icuid_cpuid endp

icuid_cpuid_ext proc
    push rbx

    mov rdi, rcx

    mov eax,   [rdi]
    mov ebx,  4[rdi]
    mov ecx,  8[rdi]
    mov edx, 12[rdi]

    cpuid

    mov   [rdi], eax
    mov  4[rdi], ebx
    mov  8[rdi], ecx
    mov 12[rdi], edx

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
