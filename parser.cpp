#include "parser.h"

ErroSintatico::ErroSintatico(const std::string& msg)
    : std::runtime_error(msg) {}

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), pos(0) {}

const Token& Parser::atual() const {
    // A lista sempre termina com um token FIM, entao pos e valida.
    return tokens[pos];
}

Token Parser::avancar() {
    Token t = tokens[pos];
    if (pos + 1 < tokens.size())
        ++pos;          // nunca avancamos alem do token FIM
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

// <expressao> ::= <literal> | '(' <expressao> <operador> <expressao> ')'
std::unique_ptr<Exp> Parser::analisaExp() {
    const Token& tok = atual();

    // Caso 1: literal inteiro -> no constante (expressao completa).
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

    // Caso 2: abre parentese -> operacao binaria (analise recursiva).
    if (tok.get_tipo() == TokenType::PAREN_ESQ) {
        avancar();                                  // consome '('
        std::unique_ptr<Exp> esq = analisaExp();    // operando esquerdo
        Operador op = analisaOperador();            // operador
        std::unique_ptr<Exp> dir = analisaExp();    // operando direito
        consumir(TokenType::PAREN_DIR, "para fechar a expressao");
        return std::make_unique<OpBin>(op, std::move(esq), std::move(dir));
    }

    // Caso 3: qualquer outra coisa -> erro sintatico.
    throw ErroSintatico(
        "esperava uma expressao (numero ou '('), mas encontrou " +
        token_type_to_string(tok.get_tipo()) + " (\"" + tok.get_lexema() +
        "\") na posicao " + std::to_string(tok.get_posicao()));
}

// <operador> ::= '+' | '-' | '*' | '/'
Operador Parser::analisaOperador() {
    const Token& tok = atual();
    switch (tok.get_tipo()) {
        case TokenType::SOMA: avancar(); return Operador::SOMA;
        case TokenType::SUB:  avancar(); return Operador::SUB;
        case TokenType::MULT: avancar(); return Operador::MULT;
        case TokenType::DIV:  avancar(); return Operador::DIV;
        default:
            throw ErroSintatico(
                "esperava um operador (+, -, *, /), mas encontrou " +
                token_type_to_string(tok.get_tipo()) + " (\"" + tok.get_lexema() +
                "\") na posicao " + std::to_string(tok.get_posicao()));
    }
}

// <programa> ::= <expressao>
std::unique_ptr<Exp> Parser::analisar() {
    std::unique_ptr<Exp> arvore = analisaExp();
    // Apos a expressao, so pode haver o fim da entrada.
    consumir(TokenType::FIM, "ao final do programa");
    return arvore;
}
