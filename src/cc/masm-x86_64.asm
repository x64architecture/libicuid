
.code

cpuid_is_supported proc
    push rdi
    pushfq
    pushfq
    pop rdi
    mov eax, edi
    xor eax, 200000h
    xor eax, edi
    popfq
    pop rdi
    test eax, eax
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

END