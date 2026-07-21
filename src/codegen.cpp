#include "codegen.h"
#include "ast.h"

// percorre a árvore recursivamente emitindo instruções assembly
static void gerar_rec(const Exp& no, std::ostream& os) {

    // constante inteira: mov direto para %rax
    if (const auto* c = dynamic_cast<const Const*>(&no)) {
        os << "    mov $" << c->get_valor() << ", %rax\n";
        return;
    }

    // uso de variável: o valor já está guardado na memória (seção .bss),
    // basta trazê-lo para %rax
    if (const auto* var = dynamic_cast<const Var*>(&no)) {
        os << "    mov " << var->get_nome() << ", %rax\n";
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

// gera o código de uma única declaração: o código da expressão do lado
// direito (resultado em %rax), seguido de um mov para guardar o resultado
// na variável (reservada na seção .bss)
static void gerar_decl(const Decl& decl, std::ostream& os) {
    os << "    # " << decl.get_nome() << " = " << decl.get_valor().imprimir() << ";\n";
    gerar_rec(decl.get_valor(), os);
    os << "    mov %rax, " << decl.get_nome() << "\n";
}

void gerar_codigo(const Programa& programa, std::ostream& os) {
    // 1) código de cada declaração, na ordem em que aparecem no fonte
    for (const Decl& decl : programa.get_decls())
        gerar_decl(decl, os);

    // 2) código da expressão final (resultado em %rax)
    os << "    # expressao final\n";
    gerar_rec(programa.get_exp(), os);
}

// seção .bss: uma diretiva .lcomm por variável declarada, reservando 8 bytes
// (inteiro de 64 bits) para cada uma
static void gerar_bss(const Programa& programa, std::ostream& os) {
    os << "    .section .bss\n";
    for (const Decl& decl : programa.get_decls())
        os << "    .lcomm " << decl.get_nome() << ", 8\n";
}

void gerar_assembly_completo(const Programa& programa, std::ostream& os) {
    os << "#\n";
    os << "# Codigo gerado pelo compilador EV\n";
    os << "#\n";

    // a secao .bss so eh necessaria se houver variaveis declaradas
    if (!programa.get_decls().empty())
        gerar_bss(programa, os);

    os << "    .section .text\n";
    os << "    .globl _start\n";
    os << "_start:\n";
    gerar_codigo(programa, os);
    os << "    call imprime_num\n";
    os << "    call sair\n";
    os << "    .include \"runtime.s\"\n";
}
