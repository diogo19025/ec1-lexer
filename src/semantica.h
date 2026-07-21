#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"
#include <stdexcept>
#include <string>

// erro semantico: uso de uma variavel que nao foi declarada antes
// (na propria declaracao, em declaracoes anteriores, ou na expressao final)
class ErroSemantico : public std::runtime_error {
public:
    explicit ErroSemantico(const std::string& msg);
};

// percorre a arvore do programa e verifica, na ordem em que aparecem, se
// toda variavel usada em cada declaracao ja foi declarada anteriormente,
// e se toda variavel usada na expressao final foi declarada em algum ponto
// do programa. Usa uma tabela de simbolos (conjunto de nomes declarados)
// que vai crescendo a medida que as declaracoes sao processadas.
//
// lanca ErroSemantico assim que encontra a primeira variavel nao declarada.
void verificar_variaveis(const Programa& programa);

#endif
