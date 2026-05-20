#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <cstddef>

enum class TokenType {
    LITERAL,
    PAREN_ESQ,
    PAREN_DIR,
    SOMA,
    SUB,
    MULT,
    DIV,
    FIM,         // EOF
    INVALIDO
};

#endif // TOKEN_H