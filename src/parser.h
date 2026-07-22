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

    // gramatica dos comandos (Atividade 09, linguagem Cmd):
    //   <programa> ::= <decl>* ( '=' <exp> | <bloco> )
    //   <decl>     ::= <ident> '=' <exp> ';'
    //   <bloco>    ::= '{' <cmd>* '}'
    //   <cmd>      ::= <atrib> | <if> | <while> | <retorno> | <bloco>
    //   <atrib>    ::= <ident> '=' <exp> ';'
    //   <if>       ::= 'if' '(' <exp> ')' <bloco> ('else' <bloco>)?
    //   <while>    ::= 'while' '(' <exp> ')' <bloco>
    //   <retorno>  ::= 'return' <exp> ';'
    // a forma "'=' <exp>" e a expressao final da linguagem EV (atividades
    // anteriores), mantida para os programas antigos continuarem validos.
    std::unique_ptr<Bloco> analisaBloco();       // <bloco>
    std::unique_ptr<Cmd>   analisaCmd();         // <cmd>
    std::unique_ptr<Cmd>   analisaAtribuicao();  // <atrib>
    std::unique_ptr<Cmd>   analisaIf();          // <if>
    std::unique_ptr<Cmd>   analisaWhile();       // <while>
    std::unique_ptr<Cmd>   analisaRetorno();     // <retorno>

    // gramatica das expressoes (Atividade 09, linguagem Cmd):
    //   <exp>      ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
    //   <exp_a>    ::= <exp_m> (('+' | '-') <exp_m>)*
    //   <exp_m>    ::= <prim>  (('*' | '/') <prim>)*
    //   <prim>     ::= <num> | <ident> | '(' <exp> ')'
    // os operadores relacionais tem a menor precedencia: em "a + 1 < b * 2"
    // os dois lados sao agrupados antes da comparacao.
    Decl analisaDecl();                  // <decl> ::= <ident> '=' <exp> ';'
    std::unique_ptr<Exp> analisaExp();   // <exp>:   comparacoes (<, >, ==)
    std::unique_ptr<Exp> analisaExpA();  // <exp_a>: adicao e subtracao
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
