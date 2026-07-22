#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <cstddef>
#include <ostream>

enum class TokenType {
    LITERAL,
    IDENTIFICADOR,
    IGUAL,          // = (declaracao ou atribuicao)
    IGUALDADE,      // ==
    PONTO_VIRGULA,
    PAREN_ESQ,
    PAREN_DIR,
    CHAVE_ESQ,
    CHAVE_DIR,
    MENOR,
    MAIOR,
    IF,
    ELSE,
    WHILE,
    RETURN,
    SOMA,
    SUB,
    MULT,
    DIV,
    FIM,
    INVALIDO
};

class Token {
private:
    TokenType tipo;
    std::string lexema;
    std::size_t posicao;

public:
    Token(TokenType tipo, std::string lexema, std::size_t posicao);

    // Getters
    TokenType get_tipo() const;
    const std::string& get_lexema() const;
    std::size_t get_posicao() const;
};

// Sobrecarga do operador << para printar diretamente
std::ostream& operator<<(std::ostream& os, const Token& token);

// Converter tipo ENUM pra STRING
std::string token_type_to_string(TokenType tipo);

#endif // TOKEN_H
