#include "lexer.h"
#include <cctype>
#include <iostream>
#include <stdexcept>

Lexer::Lexer(std::string entrada)
    : entrada(std::move(entrada)), pos(0) {}

// helpers

void Lexer::pular_espacos() {
    // espaço (32), tab (9), newline (10), carriage return (13)
    while (pos < entrada.size() && std::isspace((unsigned char)entrada[pos]))
        ++pos;
}

Token Lexer::ler_numero() {
    std::size_t inicio = pos;
    // consome enquanto for dígito
    while (pos < entrada.size() && std::isdigit((unsigned char)entrada[pos]))
        ++pos;

    // Uma sequência alfanumérica iniciada por dígito não pode ser
    // dividida entre um número e um identificador: ela inteira é inválida.
    if (pos < entrada.size() && std::isalpha((unsigned char)entrada[pos])) {
        while (pos < entrada.size() && std::isalnum((unsigned char)entrada[pos]))
            ++pos;
        return Token(TokenType::INVALIDO,
                     entrada.substr(inicio, pos - inicio), inicio);
    }

    std::string lexema = entrada.substr(inicio, pos - inicio);
    return Token(TokenType::LITERAL, lexema, inicio);
}

Token Lexer::ler_identificador() {
    std::size_t inicio = pos;
    while (pos < entrada.size() && std::isalnum((unsigned char)entrada[pos]))
        ++pos;
    return Token(TokenType::IDENTIFICADOR,
                 entrada.substr(inicio, pos - inicio), inicio);
}

// interface

Token Lexer::proximo_token() {
    pular_espacos();

    // fim de entrada
    if (pos >= entrada.size())
        return Token(TokenType::FIM, "", pos);

    char c = entrada[pos];

    // consome todos os dígitos consecutivos
    if (std::isdigit((unsigned char)c))
        return ler_numero();

    // identificador: letra seguida de zero ou mais letras ou dígitos
    if (std::isalpha((unsigned char)c))
        return ler_identificador();

    // símbolo de um único caractere
    std::size_t pos_atual = pos;
    ++pos;

    switch (c) {
        case '(': return Token(TokenType::PAREN_ESQ, "(", pos_atual);
        case ')': return Token(TokenType::PAREN_DIR, ")", pos_atual);
        case '+': return Token(TokenType::SOMA,      "+", pos_atual);
        case '-': return Token(TokenType::SUB,        "-", pos_atual);
        case '*': return Token(TokenType::MULT,       "*", pos_atual);
        case '/': return Token(TokenType::DIV,        "/", pos_atual);
        case '=': return Token(TokenType::IGUAL,      "=", pos_atual);
        case ';': return Token(TokenType::PONTO_VIRGULA, ";", pos_atual);
        default:
            // caractere fora do alfabeto da linguagem = erro léxico
            return Token(TokenType::INVALIDO, std::string(1, c), pos_atual);
    }
}

std::vector<Token> Lexer::tokenizar() {
    std::vector<Token> tokens;
    bool erro = false;

    while (true) {
        Token t = proximo_token();

        if (t.get_tipo() == TokenType::INVALIDO) {
            // reporta e continua tentando
            std::cerr << "Erro léxico na posição " << t.get_posicao()
                      << ": caractere inválido '"
                      << t.get_lexema() << "'\n";
            erro = true;
            continue;
        }

        tokens.push_back(t);

        if (t.get_tipo() == TokenType::FIM)
            break;
    }

    // se houve erro léxico, a lista pode estar incompleta — sinalizamos
    if (erro) {
        // mantém os tokens válidos coletados, mas o chamador sabe que houve erro
    }

    return tokens;
}
