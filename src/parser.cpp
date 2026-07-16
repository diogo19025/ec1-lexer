#include "parser.h"

ErroSintatico::ErroSintatico(const std::string& msg)
    : std::runtime_error(msg) {}

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), pos(0) {}

const Token& Parser::atual() const {
    return tokens[pos];
}

Token Parser::avancar() {
    Token t = tokens[pos];
    if (pos + 1 < tokens.size())
        ++pos;
    return t;
}

Token Parser::consumir(TokenType tipo, const std::string& contexto) {
    if (atual().get_tipo() != tipo) {
        throw ErroSintatico(
            "esperava " + token_type_to_string(tipo) + " " + contexto +
            ", mas encontrou " + token_type_to_string(atual().get_tipo()) +
            " (\"" + atual().get_lexema() + "\") na posicao " +
            std::to_string(atual().get_posicao()));
    }
    return avancar();
}

// <exp_a> ::= <exp_m> (('+' | '-') <exp_m>)*
// Operadores associativos à esquerda: o resultado parcial vira o operando
// esquerdo do próximo operador (constrói a árvore da esquerda para a direita).
std::unique_ptr<Exp> Parser::analisaExpA() {
    std::unique_ptr<Exp> esq = analisaExpM();

    while (atual().get_tipo() == TokenType::SOMA ||
           atual().get_tipo() == TokenType::SUB) {
        Operador op = (atual().get_tipo() == TokenType::SOMA)
                          ? Operador::SOMA : Operador::SUB;
        avancar();                                   // consome o operador
        std::unique_ptr<Exp> dir = analisaExpM();
        esq = std::make_unique<OpBin>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// <exp_m> ::= <prim> (('*' | '/') <prim>)*
std::unique_ptr<Exp> Parser::analisaExpM() {
    std::unique_ptr<Exp> esq = analisaPrim();

    while (atual().get_tipo() == TokenType::MULT ||
           atual().get_tipo() == TokenType::DIV) {
        Operador op = (atual().get_tipo() == TokenType::MULT)
                          ? Operador::MULT : Operador::DIV;
        avancar();                                   // consome o operador
        std::unique_ptr<Exp> dir = analisaPrim();
        esq = std::make_unique<OpBin>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// <prim> ::= <num> | '(' <exp_a> ')'
std::unique_ptr<Exp> Parser::analisaPrim() {
    const Token& tok = atual();

    if (tok.get_tipo() == TokenType::LITERAL) {
        avancar();
        try {
            return std::make_unique<Const>(std::stoll(tok.get_lexema()));
        } catch (const std::out_of_range&) {
            throw ErroSintatico(
                "constante inteira grande demais para 64 bits: \"" +
                tok.get_lexema() + "\" na posicao " +
                std::to_string(tok.get_posicao()));
        }
    }

    if (tok.get_tipo() == TokenType::PAREN_ESQ) {
        avancar();
        std::unique_ptr<Exp> interna = analisaExpA();
        consumir(TokenType::PAREN_DIR, "para fechar a expressao");
        return interna;
    }

    throw ErroSintatico(
        "esperava uma expressao (numero ou '('), mas encontrou " +
        token_type_to_string(tok.get_tipo()) + " (\"" + tok.get_lexema() +
        "\") na posicao " + std::to_string(tok.get_posicao()));
}

std::unique_ptr<Exp> Parser::analisar() {
    std::unique_ptr<Exp> arvore = analisaExpA();
    consumir(TokenType::FIM, "ao final do programa");
    return arvore;
}
