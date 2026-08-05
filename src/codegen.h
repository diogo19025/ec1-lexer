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

// gera o código de um programa completo, nas três formas aceitas pelo
// parser (Atividades 08, 09 e 10):
//   - forma EV: o código de cada declaração (na ordem em que aparecem),
//     seguido do código da expressão final. O resultado fica em %rax.
//   - forma Cmd: o código de cada declaração, seguido do código do corpo
//     de comandos (atribuição, if/else, while, return, blocos aninhados).
//     Cada "return" grava seu valor em %rax e desvia para um rótulo comum
//     de fim de programa; if/else e while usam rótulos com um contador
//     global, reiniciado no início desta função, para garantir nomes
//     únicos mesmo com aninhamento.
//   - forma Fun: o código de cada variável global, seguido do código do
//     bloco main (que tem variáveis locais como uma função, mas nenhum
//     parâmetro e nenhum prólogo/epílogo de chamada — o processo já começa
//     e termina ali). O código de cada função (rótulo, prólogo, corpo,
//     epílogo) é gerado à parte por gerar_assembly_completo, depois das
//     chamadas a imprime_num/sair, e só pode ser alcançado por um "call".
void gerar_codigo(const Programa& programa, std::ostream& os);

// envolve o código gerado no modelo completo do arquivo assembly: declara
// as variáveis globais na seção .bss (uma diretiva .lcomm por variável) e
// gera o código do programa (forma EV, Cmd ou Fun, veja gerar_codigo acima)
// na seção .text, seguido das chamadas a imprime_num/sair (o valor impresso
// é o que estiver em %rax nesse ponto — o valor de retorno do bloco main
// nas formas Cmd/Fun, ou o valor da expressão final na forma EV). Na forma
// Fun, o código de cada função é colocado logo depois, na mesma seção
// .text, antes do runtime.
void gerar_assembly_completo(const Programa& programa, std::ostream& os);

#endif
