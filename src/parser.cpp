#include "parser.h"
#include "semantica.h"

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

// <decl> ::= <ident> '=' <exp> ';'
Decl Parser::analisaDecl() {
    Token nomeTok = consumir(TokenType::IDENTIFICADOR,
                              "como nome de variavel no inicio da declaracao");
    consumir(TokenType::IGUAL, "apos o nome da variavel na declaracao");
    std::unique_ptr<Exp> valor = analisaExp();
    consumir(TokenType::PONTO_VIRGULA, "ao final da declaracao");
    return Decl(nomeTok.get_lexema(), std::move(valor));
}

// <exp> ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
// Nível de menor precedência: as comparações agrupam depois de + - * /.
std::unique_ptr<Exp> Parser::analisaExp() {
    std::unique_ptr<Exp> esq = analisaExpA();

    while (atual().get_tipo() == TokenType::MENOR ||
           atual().get_tipo() == TokenType::MAIOR ||
           atual().get_tipo() == TokenType::IGUALDADE) {
        Operador op;
        switch (atual().get_tipo()) {
            case TokenType::MENOR: op = Operador::MENOR;     break;
            case TokenType::MAIOR: op = Operador::MAIOR;     break;
            default:               op = Operador::IGUALDADE; break;
        }
        avancar();                                   // consome o operador
        std::unique_ptr<Exp> dir = analisaExpA();
        esq = std::make_unique<OpBin>(op, std::move(esq), std::move(dir));
    }
    return esq;
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

// <prim> ::= <num> | <ident> | '(' <exp> ')'
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

    // referencia a uma variavel: nao verificamos aqui se ela foi
    // declarada, isso e responsabilidade da analise semantica
    if (tok.get_tipo() == TokenType::IDENTIFICADOR) {
        avancar();
        return std::make_unique<Var>(tok.get_lexema());
    }

    if (tok.get_tipo() == TokenType::PAREN_ESQ) {
        avancar();
        std::unique_ptr<Exp> interna = analisaExp();
        consumir(TokenType::PAREN_DIR, "para fechar a expressao");
        return interna;
    }

    throw ErroSintatico(
        "esperava uma expressao (numero, variavel ou '('), mas encontrou " +
        token_type_to_string(tok.get_tipo()) + " (\"" + tok.get_lexema() +
        "\") na posicao " + std::to_string(tok.get_posicao()));
}

// <bloco> ::= '{' <cmd>* '}'
std::unique_ptr<Bloco> Parser::analisaBloco() {
    consumir(TokenType::CHAVE_ESQ, "para abrir o bloco de comandos");

    std::vector<std::unique_ptr<Cmd>> comandos;
    while (atual().get_tipo() != TokenType::CHAVE_DIR &&
           atual().get_tipo() != TokenType::FIM)
        comandos.push_back(analisaCmd());

    consumir(TokenType::CHAVE_DIR, "para fechar o bloco de comandos");
    return std::make_unique<Bloco>(std::move(comandos));
}

// <cmd> ::= <atrib> | <if> | <while> | <retorno> | <bloco>
// o primeiro token decide qual comando reconhecer
std::unique_ptr<Cmd> Parser::analisaCmd() {
    switch (atual().get_tipo()) {
        case TokenType::IDENTIFICADOR: return analisaAtribuicao();
        case TokenType::IF:            return analisaIf();
        case TokenType::WHILE:         return analisaWhile();
        case TokenType::RETURN:        return analisaRetorno();
        case TokenType::CHAVE_ESQ:     return analisaBloco();
        default:
            throw ErroSintatico(
                "esperava um comando (atribuicao, if, while, return ou "
                "bloco), mas encontrou " +
                token_type_to_string(atual().get_tipo()) + " (\"" +
                atual().get_lexema() + "\") na posicao " +
                std::to_string(atual().get_posicao()));
    }
}

// <atrib> ::= <ident> '=' <exp> ';'
std::unique_ptr<Cmd> Parser::analisaAtribuicao() {
    Token nomeTok = consumir(TokenType::IDENTIFICADOR,
                              "como nome de variavel na atribuicao");
    consumir(TokenType::IGUAL, "apos o nome da variavel na atribuicao");
    std::unique_ptr<Exp> valor = analisaExp();
    consumir(TokenType::PONTO_VIRGULA, "ao final da atribuicao");
    return std::make_unique<Atribuicao>(nomeTok.get_lexema(), std::move(valor));
}

// <if> ::= 'if' '(' <exp> ')' <bloco> ('else' <bloco>)?
std::unique_ptr<Cmd> Parser::analisaIf() {
    consumir(TokenType::IF, "no inicio do comando if");
    consumir(TokenType::PAREN_ESQ, "apos o if");
    std::unique_ptr<Exp> condicao = analisaExp();
    consumir(TokenType::PAREN_DIR, "para fechar a condicao do if");
    std::unique_ptr<Bloco> entao = analisaBloco();

    std::unique_ptr<Bloco> senao;
    if (atual().get_tipo() == TokenType::ELSE) {
        avancar();                                   // consome o 'else'
        senao = analisaBloco();
    }
    return std::make_unique<If>(std::move(condicao), std::move(entao),
                                std::move(senao));
}

// <while> ::= 'while' '(' <exp> ')' <bloco>
std::unique_ptr<Cmd> Parser::analisaWhile() {
    consumir(TokenType::WHILE, "no inicio do comando while");
    consumir(TokenType::PAREN_ESQ, "apos o while");
    std::unique_ptr<Exp> condicao = analisaExp();
    consumir(TokenType::PAREN_DIR, "para fechar a condicao do while");
    std::unique_ptr<Bloco> corpo = analisaBloco();
    return std::make_unique<While>(std::move(condicao), std::move(corpo));
}

// <retorno> ::= 'return' <exp> ';'
std::unique_ptr<Cmd> Parser::analisaRetorno() {
    consumir(TokenType::RETURN, "no inicio do comando return");
    std::unique_ptr<Exp> valor = analisaExp();
    consumir(TokenType::PONTO_VIRGULA, "ao final do return");
    return std::make_unique<Retorno>(std::move(valor));
}

// <programa> ::= <decl>* ( '=' <exp> | <bloco> )
//
// olha o proximo token: enquanto for um identificador, reconhece mais uma
// declaracao; depois, um '{' abre o corpo de comandos (linguagem Cmd) e um
// '=' inicia a expressao final (forma EV das atividades anteriores).
std::unique_ptr<Programa> Parser::analisar() {
    std::vector<Decl> decls;

    while (atual().get_tipo() == TokenType::IDENTIFICADOR)
        decls.push_back(analisaDecl());

    if (atual().get_tipo() == TokenType::CHAVE_ESQ) {
        std::unique_ptr<Bloco> corpo = analisaBloco();
        consumir(TokenType::FIM, "ao final do programa");

        // a verificacao das variaveis usadas dentro dos comandos (if,
        // while, atribuicoes, return) e da parte 3 da Atividade 09
        return std::make_unique<Programa>(std::move(decls), std::move(corpo));
    }

    consumir(TokenType::IGUAL, "para iniciar a expressao final do programa");
    std::unique_ptr<Exp> expFinal = analisaExp();
    consumir(TokenType::FIM, "ao final do programa");

    auto programa = std::make_unique<Programa>(std::move(decls), std::move(expFinal));

    // analise semantica: verifica se toda variavel usada (nas declaracoes
    // e na expressao final) foi declarada antes de seu uso. Lanca
    // ErroSemantico em caso de variavel nao declarada.
    verificar_variaveis(*programa);

    return programa;
}
