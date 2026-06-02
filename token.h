#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <cstddef>
#include <ostream>

enum class TokenType {
    LITERAL,
    PAREN_ESQ,
    PAREN_DIR,
    SOMA,
    SUB,
    MULT,
    DIV,
    FIM,
    INVALIDO
};

struct Token {
    TokenType tipo;
    std::string lexema;
    std::size_t posicao;

    Token(TokenType tipo, std::string lexema, std::size_t posicao);
};

// Sobrecarga do operador << para printar diretamente
std::ostream& operator<<(std::ostream& os, const Token& token);

// Converter ENUM em STRING para usos externos
std::string token_type_to_string(TokenType tipo);

#endif // TOKEN_H