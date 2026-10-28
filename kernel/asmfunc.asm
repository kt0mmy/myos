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
    
global GetCR3 ; uint64_t GetCR3();
GetCR3:
    mov rax, cr3 
    ret

global SetCR3 ; void SetCR3(uint64_t value);
SetCR3:
    mov cr3, rdi
    ret

global SwitchContext ; void SwitchContext(void* next_ctx, void* current_ctx);
SwitchContext:
    ; RDI は第一引数
    ; RSI は第二引数
    mov [rsi + 0x40], rax
    mov [rsi + 0x48], rbx
    mov [rsi + 0x50], rcx
    mov [rsi + 0x58], rdx
    mov [rsi + 0x60], rdi
    mov [rsi + 0x68], rsi

    ; 戻り先はcall命令でスタックに積まれる
    ; つまり、rsp （アドレス）に戻り先、rsp+8に前のスタック位置
    lea rax, [rsp + 8]
    mov [rsi + 0x70], rax ; RSP

    mov [rsi + 0x78], rbp

    mov [rsi + 0x80], r8
    mov [rsi + 0x88], r9
    mov [rsi + 0x90], r10
    mov [rsi + 0x98], r11
    mov [rsi + 0xa0], r12
    mov [rsi + 0xa8], r13
    mov [rsi + 0xb0], r14
    mov [rsi + 0xb8], r15

    mov rax, cr3
    mov [rsi + 0x00], rax;

    mov rax, [rsp]
    mov [rsi + 0x08], rax

    pushfq ; RFLAGS をスタックに積む
    pop qword [rsi + 0x10]

    mov ax, cs
    mov [rsi + 0x20], rax
    mov bx, ss
    mov [rsi + 0x28], rbx
    mov cx, fs
    mov [rsi + 0x30], rcx
    mov dx, gs
    mov [rsi + 0x38], rdx

    fxsave [rsi + 0xc0]

    ; iret用のスタック仮構築
    push qword [rdi + 0x28] ; ss
    push qword [rdi + 0x70] ; RSP
    push qword [rdi + 0x10] ; RFLAGS
    push qword [rdi + 0x20] ; CS
    push qword [rdi + 0x08] ; RIP


    ; こんてきすとふっき
    fxrstor [rdi + 0xc0]    
    
    mov rax, [rdi + 0x00]
    mov cr3, rax
    
    mov rax, [rdi + 0x30]
    mov fs , ax

    mov rax, [rdi + 0x38]
    mov gs, ax

    mov rax, [rdi + 0x40]
    mov rbx, [rdi + 0x48]
    mov rcx, [rdi + 0x50]
    mov rdx, [rdi + 0x58]
    mov rsi, [rdi + 0x68]
    mov rbp, [rdi + 0x78]
    mov r8, [rdi + 0x80]
    mov r9, [rdi + 0x88]
    mov r10, [rdi + 0x90]
    mov r11, [rdi + 0x98]
    mov r12, [rdi + 0xa0]
    mov r13, [rdi + 0xa8]
    mov r14, [rdi + 0xb0]
    mov r15, [rdi + 0xb8]

    mov rdi, [rdi + 0x60]
    o64 iret ; 本来は割り込みの戻りで使われる。RIP,CS, RFLAGなどの情報も復元してくれる
