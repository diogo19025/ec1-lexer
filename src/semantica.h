#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"
#include <stdexcept>
#include <string>

// erro semantico: uso de uma variavel que nao foi declarada antes
// (na propria declaracao, em declaracoes anteriores, na expressao final,
// ou em qualquer comando do corpo do programa da linguagem Cmd), ou, na
// linguagem Fun, um problema envolvendo funcoes: funcao nao declarada,
// nome que nao e uma funcao sendo chamado, ou numero errado de argumentos
class ErroSemantico : public std::runtime_error {
public:
    explicit ErroSemantico(const std::string& msg);
};

// percorre a arvore do programa e verifica, na ordem em que aparecem, se
// todo nome usado ja foi declarado anteriormente e se esta sendo usado de
// acordo com o que ele e. Cada nome declarado vira um simbolo com um tipo
// (variavel global, funcao, parametro ou variavel local) numa tabela de
// simbolos que cresce a medida que as declaracoes sao processadas.
//
// Funciona para as tres formas de <programa> aceitas pelo parser:
//
//   - forma EV (Atividade 08): <decl>* '=' <exp>
//     verifica as declaracoes e a expressao final.
//
//   - forma Cmd (Atividade 09): <decl>* <bloco>
//     verifica as declaracoes e, em seguida, percorre recursivamente os
//     comandos do corpo (atribuicao, if, while, return, blocos aninhados).
//     Uma atribuicao (<ident> '=' <exp> ';') a uma variavel que nao foi
//     declarada tambem e um erro semantico, assim como o uso da variavel
//     numa expressao antes de ser declarada. As duas ramificacoes de um
//     'if' e o corpo de um 'while' sao verificados com a MESMA tabela de
//     simbolos usada antes deles (uma variavel so passa a existir depois
//     de sua declaracao/atribuicao ser executada, e blocos condicionais
//     nao sao necessariamente executados).
//
//   - forma Fun (Atividade 10): (<vardecl> | <fundecl>)* 'main' <corpo>
//     ha uma tabela global, com as variaveis globais e as funcoes, e uma
//     tabela local para cada funcao (e para o main), com os parametros e
//     as variaveis locais. As declaracoes de topo sao processadas na ordem
//     em que aparecem no programa, entao cada uma so enxerga o que foi
//     declarado antes dela. Uma funcao e registrada na tabela global ANTES
//     de seu corpo ser analisado, o que permite recursao direta (o corpo
//     de f pode chamar f).
//
//     Um nome dentro de uma funcao e resolvido primeiro no escopo local e
//     depois no global, entao um parametro ou variavel local esconde uma
//     global de mesmo nome. Numa chamada, verifica-se que o nome chamado
//     foi declarado, que ele realmente e uma funcao, e que a quantidade de
//     argumentos e igual a quantidade de parametros da funcao.
//
// lanca ErroSemantico assim que encontra o primeiro problema.
void verificar_variaveis(const Programa& programa);

#endif
