#include "codegen.h"
#include "ast.h"

// percorre a árvore recursivamente emitindo instruções assembly
static void gerar_rec(const Exp& no, std::ostream& os) {

    // constante inteira: mov direto para %rax
    if (const auto* c = dynamic_cast<const Const*>(&no)) {
        os << "    mov $" << c->get_valor() << ", %rax\n";
        return;
    }

    // operação binária: esquema de pilha
    // avalia direito primeiro, empilha; avalia esquerdo; desempilha direito em %rbx
    // assim a operação "esq OP dir" fica: %rax OP %rbx
    if (const auto* op = dynamic_cast<const OpBin*>(&no)) {
        gerar_rec(op->get_dir(), os);       // direito → %rax
        os << "    push %rax\n";            // salva na pilha
        gerar_rec(op->get_esq(), os);       // esquerdo → %rax
        os << "    pop %rbx\n";             // recupera direito em %rbx

        switch (op->get_op()) {
            case Operador::SOMA:
                os << "    add %rbx, %rax\n";
                break;
            case Operador::SUB:
                os << "    sub %rbx, %rax\n";
                break;
            case Operador::MULT:
                os << "    imul %rbx, %rax\n";
                break;
            case Operador::DIV:
                // idiv usa rdx:rax; cqo estende o sinal de rax para rdx
                os << "    mov %rbx, %rcx\n";
                os << "    cqo\n";
                os << "    idiv %rcx\n";
                break;
        }
        return;
    }
}

void gerar_codigo(const Exp& exp, std::ostream& os) {
    gerar_rec(exp, os);
}

// modelo de arquivo assembly definido na Atividade 02
void gerar_assembly_completo(const Exp& exp, std::ostream& os) {
    os << "#\n";
    os << "# Codigo gerado pelo compilador EC1\n";
    os << "#\n";
    os << "    .section .text\n";
    os << "    .globl _start\n";
    os << "_start:\n";
    gerar_codigo(exp, os);
    os << "    call imprime_num\n";
    os << "    call sair\n";
    os << "    .include \"runtime.s\"\n";
}
