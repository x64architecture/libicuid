
.code

cpuid_is_supported proc
    pushfq
    pop rax
    mov rcx, rax
    xor rax, 200000h
    push rax
    popfq
    pushfq
    pop rax
    xor rax, rcx
    jnz cpuid_supported
    ret

cpuid_supported:
    xor eax, eax
    inc eax
    ret

cpuid_is_supported endp

__run_cpuid proc
    push rbx
    push rcx
    push rdx
    push rdi
    
    mov rdi, rcx

    mov eax, [rdi]
    mov ebx, [rdi+4]
    mov ecx, [rdi+8]
    mov edx, [rdi+12]
    
    cpuid
    
    mov [rdi],    eax
    mov [rdi+4],  ebx
    mov [rdi+8],  ecx
    mov [rdi+12], edx
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    ret

__run_cpuid endp

icuid_xgetbv proc
    mov DWORD PTR [rsp+8], ecx
    mov ecx, DWORD PTR 8[rsp]
    db 15, 1, 208 ; xgetbv
    shl rdx, 32
    or  rdx, rax
    mov rax, rdx
    ret

icuid_xgetbv endp

END
