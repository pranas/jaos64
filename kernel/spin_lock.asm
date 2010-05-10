bits 64

global spin_lock

spin_lock:
    mov rax, 1

.loop:
    xchg rax, [rdi]
    test rax, rax
    jnz .loop
    ret