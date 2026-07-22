#include "token.h"

// CLasse Token
Token::Token(TokenType tipo, std::string lexema, std::size_t posicao)
    : tipo(tipo), lexema(std::move(lexema)), posicao(posicao) {}

TokenType Token::get_tipo() const {
    return tipo;
}

const std::string& Token::get_lexema() const {
    return lexema;
}

std::size_t Token::get_posicao() const {
    return posicao;
}

// ENUM -> STRING
std::string token_type_to_string(TokenType tipo) {
    switch (tipo) {
        case TokenType::LITERAL: return "Numero";
        case TokenType::IDENTIFICADOR: return "Identificador";
        case TokenType::IGUAL: return "Igual";
        case TokenType::IGUALDADE: return "Igualdade";
        case TokenType::PONTO_VIRGULA: return "PontoVirgula";
        case TokenType::PAREN_ESQ: return "ParenEsq";
        case TokenType::PAREN_DIR: return "ParenDir";
        case TokenType::CHAVE_ESQ: return "ChaveEsq";
        case TokenType::CHAVE_DIR: return "ChaveDir";
        case TokenType::MENOR: return "Menor";
        case TokenType::MAIOR: return "Maior";
        case TokenType::IF: return "If";
        case TokenType::ELSE: return "Else";
        case TokenType::WHILE: return "While";
        case TokenType::RETURN: return "Return";
        case TokenType::SOMA: return "Soma";
        case TokenType::SUB: return "Sub";
        case TokenType::MULT: return "Mult";
        case TokenType::DIV: return "Div";
        case TokenType::FIM: return "EOF";
        case TokenType::INVALIDO: return "Invalido";
        default: return "Desconhecido";
    }
}

// Print do Token no formato <Tipo, "Lexema", Posicao> 
std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "<"
       << token_type_to_string(token.get_tipo())
       << ", \""
       << token.get_lexema()
       << "\", "
       << token.get_posicao()
       << ">";
    return os;
}
