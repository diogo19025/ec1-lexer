#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <ostream>

// gera as instruções assembly para a expressão; resultado em %rax
// (referências a variáveis geram um "mov <nome>, %rax")
void gerar_codigo(const Exp& exp, std::ostream& os);

// envolve o código gerado no modelo completo do arquivo assembly (EC1/EC2,
// sem variáveis)
void gerar_assembly_completo(const Exp& exp, std::ostream& os);

// gera o código de um programa completo da linguagem EV: o código de cada
// declaração (na ordem em que aparecem), seguido do código da expressão
// final. O resultado da expressão final fica em %rax.
void gerar_codigo(const Programa& programa, std::ostream& os);

// envolve o código gerado no modelo completo do arquivo assembly (EV):
// declara as variáveis na seção .bss (uma diretiva .lcomm por variável) e
// gera o código de declarações + expressão final na seção .text.
void gerar_assembly_completo(const Programa& programa, std::ostream& os);

#endif
