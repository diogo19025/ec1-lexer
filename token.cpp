#include "token.h"

Token::Token(TokenType tipo, std::string lexema, std::size_t posicao)
    : tipo(tipo), lexema(std::move(lexema)), posicao(posicao) {}

std::string token_type_to_string(TokenType tipo) {
    switch (tipo) {
        case TokenType::LITERAL: return "Numero";
        case TokenType::PAREN_ESQ: return "ParenEsq";
        case TokenType::PAREN_DIR: return "ParenDir";
        case TokenType::SOMA: return "Soma";
        case TokenType::SUB: return "Sub";
        case TokenType::MULT: return "Mult";
        case TokenType::DIV: return "Div";
        case TokenType::FIM: return "EOF";
        case TokenType::INVALIDO: return "Invalido";
        default: return "Desconhecido";
    }
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "<"
       << token_type_to_string(token.tipo)
       << ", \""
       << token.lexema
       << "\", "
       << token.posicao
       << ">";
    return os;
}