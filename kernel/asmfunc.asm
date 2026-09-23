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