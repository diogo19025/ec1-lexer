#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

// Excecao lancada quando a entrada nao respeita a gramatica de EC1.
class ErroSintatico : public std::runtime_error {
public:
    explicit ErroSintatico(const std::string& msg);
};

// Analisador sintatico descendente recursivo para a linguagem EC1.
// Consome a sequencia de tokens produzida pelo analisador lexico (Atividade 04)
// e produz a arvore de sintaxe abstrata.
class Parser {
private:
    std::vector<Token> tokens;
    std::size_t pos;

    const Token& atual() const;   // proximo token, sem consumir
    Token avancar();              // devolve o token atual e avanca a leitura

    // Equivalente ao verificaProxToken do enunciado: exige um tipo de token,
    // sinalizando erro sintatico caso o tipo nao bata.
    Token consumir(TokenType tipo, const std::string& contexto);

    std::unique_ptr<Exp> analisaExp();   // <expressao>
    Operador analisaOperador();          // <operador>

public:
    explicit Parser(std::vector<Token> tokens);

    // <programa> ::= <expressao>
    // Analisa o programa inteiro e devolve a arvore; lanca ErroSintatico em erro.
    std::unique_ptr<Exp> analisar();
};

#endif // PARSER_H
