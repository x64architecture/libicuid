
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
    mov QWORD PTR [rsp+24], r8  ; o_edx
    mov QWORD PTR [rsp+16], rdx ; o_eax
    mov DWORD PTR [rsp+8],  ecx ; xcr
    db 15, 1, 208 ; xgetbv
    mov r10d, eax
    mov r11d, edx
    mov rax, QWORD PTR 16[rsp]
    mov DWORD PTR [rax], r10d
    mov rax, QWORD PTR 24[rsp]
    mov DWORD PTR [rax], r11d
    ret

icuid_xgetbv endp

END
