#
# runtime.s — sub-rotinas de suporte para o compilador EC1
# Alvo: Linux x86-64, GNU Assembler (gas), sintaxe AT&T.
#

# encerra o processo com código 0
sair:
    mov  $60, %rax       # syscall: exit
    xor  %rdi, %rdi      # código de saída 0
    syscall

# imprime o inteiro com sinal em %rax seguido de newline
imprime_num:
    push %rbp
    mov  %rsp, %rbp
    push %rbx
    push %r12
    push %r13

    mov  %rax, %r12      # guarda valor a imprimir
    xor  %r13, %r13      # r13 = 1 se negativo, 0 se não

    # buffer de 24 bytes; dígitos são escritos de trás para frente
    sub  $24, %rsp
    lea  23(%rsp), %rbx  # rbx aponta para o fim do buffer
    movb $'\n', (%rbx)
    dec  %rbx

    # caso especial: zero
    test %r12, %r12
    jnz  .pn_nao_zero
    movb $'0', (%rbx)
    dec  %rbx
    jmp  .pn_sinal

.pn_nao_zero:
    jge  .pn_loop
    neg  %r12            # torna positivo para extrair dígitos
    mov  $1, %r13        # marca como negativo

    # extrai dígitos do menos significativo para o mais significativo
.pn_loop:
    mov  %r12, %rax
    xor  %rdx, %rdx
    mov  $10, %rcx
    div  %rcx            # rax = quociente, rdx = dígito (0–9)
    mov  %rax, %r12
    add  $'0', %rdx
    mov  %dl, (%rbx)
    dec  %rbx
    test %r12, %r12
    jnz  .pn_loop

.pn_sinal:
    test %r13, %r13
    jz   .pn_escreve
    movb $'-', (%rbx)
    dec  %rbx

.pn_escreve:
    inc  %rbx            # primeiro caractere do número
    lea  23(%rsp), %rdx
    sub  %rbx, %rdx
    inc  %rdx            # comprimento total (inclui '\n')

    mov  $1, %rax        # syscall: write
    mov  $1, %rdi        # fd: stdout
    mov  %rbx, %rsi      # buffer
    syscall

    add  $24, %rsp
    pop  %r13
    pop  %r12
    pop  %rbx
    pop  %rbp
    ret
