#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <ostream>

// gera as instruções assembly para a expressão; resultado em %rax
void gerar_codigo(const Exp& exp, std::ostream& os);

// envolve o código gerado no modelo completo do arquivo assembly
void gerar_assembly_completo(const Exp& exp, std::ostream& os);

#endif
