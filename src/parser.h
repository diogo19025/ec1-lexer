#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

class ErroSintatico : public std::runtime_error {
public:
    explicit ErroSintatico(const std::string& msg);
};

class Parser {
private:
    std::vector<Token> tokens;
    std::size_t pos;

    const Token& atual() const;
    Token avancar();

    Token consumir(TokenType tipo, const std::string& contexto);

    // um não-terminal por nível de precedência (gramática EC2)
    std::unique_ptr<Exp> analisaExpA();   // <exp_a>: adição e subtração
    std::unique_ptr<Exp> analisaExpM();   // <exp_m>: multiplicação e divisão
    std::unique_ptr<Exp> analisaPrim();   // <prim>:  <num> | '(' <exp_a> ')'

public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<Exp> analisar();
};

#endif
