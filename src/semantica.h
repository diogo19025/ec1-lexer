#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"
#include <stdexcept>
#include <string>

// erro semantico: uso de uma variavel que nao foi declarada antes
// (na propria declaracao, em declaracoes anteriores, na expressao final,
// ou em qualquer comando do corpo do programa da linguagem Cmd)
class ErroSemantico : public std::runtime_error {
public:
    explicit ErroSemantico(const std::string& msg);
};

// percorre a arvore do programa e verifica, na ordem em que aparecem, se
// toda variavel usada ja foi declarada anteriormente. Usa uma tabela de
// simbolos (conjunto de nomes declarados) que vai crescendo a medida que
// declaracoes e atribuicoes sao processadas.
//
// Funciona para as duas formas de <programa> aceitas pelo parser:
//   - forma EV (Atividade 08): <decl>* '=' <exp>
//     verifica as declaracoes e a expressao final.
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
// lanca ErroSemantico assim que encontra a primeira variavel nao declarada.
void verificar_variaveis(const Programa& programa);

#endif
