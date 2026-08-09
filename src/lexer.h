#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <vector>

class Lexer {
private:
    std::string entrada;   // texto completo lido do arquivo
    std::size_t pos;       // posição atual
    bool erro_lexico;      // true assim que um token INVALIDO é produzido

    // pula espaços em branco (espaço, tab, \n, \r)
    void pular_espacos();

    // lê um número; sequências alfanuméricas iniciadas por dígito são inválidas
    Token ler_numero();

    // lê uma letra seguida de zero ou mais letras ou dígitos
    Token ler_identificador();

public:
    explicit Lexer(std::string entrada);

    // devolve o próximo token da entrada
    Token proximo_token();

    // consome toda a entrada de uma vez e devolve a lista de tokens. Os
    // tokens inválidos são reportados em std::cerr e omitidos da lista (a
    // análise continua, para reportar todos os erros de uma vez), mas
    // houve_erro_lexico() passa a devolver true.
    std::vector<Token> tokenizar();

    // true se algum token inválido foi encontrado desde a construção do
    // Lexer. Como os tokens inválidos não aparecem na lista devolvida por
    // tokenizar(), é este sinalizador que permite ao chamador interromper a
    // compilação em vez de seguir analisando uma entrada incompleta.
    bool houve_erro_lexico() const;
};

#endif
