#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <vector>

class Lexer {
private:
    std::string entrada;   // texto completo lido do arquivo
    std::size_t pos;       // posição atual

    // pula espaços em branco (espaço, tab, \n, \r)
    void pular_espacos();

    // lê todos os dígitos a partir de pos e devolve um Token LITERAL
    Token ler_numero();

public:
    explicit Lexer(std::string entrada);

    // devolve o próximo token da entrada
    Token proximo_token();

    // consome toda a entrada de uma vez e devolve a lista de tokens
    std::vector<Token> tokenizar();
};

#endif
