bits 64
section .text

global IoOut32 ; void IoOut32(uint16_t addr, uint32_t data);
IoOut32:
    mov dx, di
    mov eax, esi
    out dx, eax
    ret

global IoIn32  ; uint32_t IoIn32(uint16_t addr);
IoIn32:
    mov dx, di
    in eax, dx
    ret

; 引数はRDI, RSI, RDX, RCX, R8, R9の順
global LoadIDT ; void LoadIDT(uint16_t limit, uint64_t offset);
LoadIDT:
    push rbp ; 呼び出し元のrbpをスタックにpush
    mov rbp, rsp  ; 今のスタックの先頭をスタックのベースとする
    sub rsp, 10 ; 10バイト確保
    mov [rsp], di ; limit 
    mov [rsp+2], rsi ; offset 
    lidt [rsp]
    mov rsp, rbp 
    pop rbp 
    ret

global GetCS
GetCS:
    xor eax, eax 
    mov ax, cs 
    ret

extern kernel_main_stack
extern KernelMainNewStack

global KernelMain
KernelMain:
    ; 引数はkernel_main から引き継ぎ（レジスタを変えていないので）
    mov rsp, kernel_main_stack + 1024 * 1024
    call KernelMainNewStack
.fin:
    hlt 
    jmp .fin

global LoadGDT: ; void LoadGDT(uint16_t limit, uint64_t offset)
LoadGDT:
    push rbp 
    mov rbp, rsp 
    sub rsp, 10
    mov [rsp], di ; limit 
    mov [rsp + 2], rsi ; offset 
    lgdt [rsp]
    mov rsp, rbp 
    pop rbp 
    ret

; Data Segment系のレジスタを設定(すべて使われないのでnullにする)
global SetDSAll: ; void SetDSAll(uint16_t value);
SetDSAll:
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di
    ret

global SetCSSS: ; void SetCSSS(uint16_t cs, uint16_t ss)
SetCSSS:
    push rbp
    mov rbp, rsp 
    mov ss, si 
    mov rax, .next
    push rdi ; far return で CS に設定
    push rax ; far return で RIP に設定
    o64 retf
.next:
    ; RIPに.nextが入っているので、ここから実行
    ; 通常の関数の return 
    mov rsp, rbp 
    pop rbp 
    ret
    
global SetCR3 ; void SetCR3(uint64_t value);
SetCR3:
    mov cr3, rdi
    ret