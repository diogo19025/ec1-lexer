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

    // gramatica EV (Expressoes com Variaveis):
    //   <programa> ::= <decl>* <result>
    //   <decl>     ::= <ident> '=' <exp> ';'
    //   <result>   ::= '=' <exp>
    //   <exp>      ::= <exp_m> (('+' | '-') <exp_m>)*
    //   <exp_m>    ::= <prim> (('*' | '/') <prim>)*
    //   <prim>     ::= <num> | <ident> | '(' <exp> ')'
    Decl analisaDecl();                  // <decl>
    std::unique_ptr<Exp> analisaExp();   // <exp>:   adicao e subtracao
    std::unique_ptr<Exp> analisaExpM();  // <exp_m>: multiplicacao e divisao
    std::unique_ptr<Exp> analisaPrim();  // <prim>:  <num> | <ident> | '(' <exp> ')'

public:
    explicit Parser(std::vector<Token> tokens);

    // analisa o programa completo (declaracoes + expressao final).
    // apos montar a arvore, executa a analise semantica (verificacao de
    // variaveis declaradas) sobre ela antes de devolve-la.
    // lanca ErroSintatico em caso de erro de sintaxe, ou ErroSemantico
    // (definido em semantica.h) em caso de variavel usada sem ter sido
    // declarada antes.
    std::unique_ptr<Programa> analisar();
};

#endif
