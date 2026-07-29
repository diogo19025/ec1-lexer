#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <ostream>

// gera as instruções assembly para a expressão; resultado em %rax
// (referências a variáveis geram um "mov <nome>, %rax"; os operadores
// relacionais <, > e == geram 0 ou 1 em %rax via cmp/setX/movzbq)
void gerar_codigo(const Exp& exp, std::ostream& os);

// envolve o código gerado no modelo completo do arquivo assembly (EC1/EC2,
// sem variáveis)
void gerar_assembly_completo(const Exp& exp, std::ostream& os);

// gera o código de um programa completo, nas duas formas aceitas pelo
// parser (Atividades 08 e 09):
//   - forma EV: o código de cada declaração (na ordem em que aparecem),
//     seguido do código da expressão final. O resultado fica em %rax.
//   - forma Cmd: o código de cada declaração, seguido do código do corpo
//     de comandos (atribuição, if/else, while, return, blocos aninhados).
//     Cada "return" grava seu valor em %rax e desvia para um rótulo comum
//     de fim de programa; if/else e while usam rótulos com um contador
//     global, reiniciado no início desta função, para garantir nomes
//     únicos mesmo com aninhamento.
void gerar_codigo(const Programa& programa, std::ostream& os);

// envolve o código gerado no modelo completo do arquivo assembly: declara
// as variáveis na seção .bss (uma diretiva .lcomm por variável) e gera o
// código do programa (forma EV ou Cmd, veja gerar_codigo acima) na seção
// .text, seguido das chamadas a imprime_num/sair (o valor impresso é o que
// estiver em %rax nesse ponto — o valor de retorno na forma Cmd, ou o
// valor da expressão final na forma EV).
void gerar_assembly_completo(const Programa& programa, std::ostream& os);

#endif
