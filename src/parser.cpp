#include "parser.h"
#include "semantica.h"

ErroSintatico::ErroSintatico(const std::string& msg)
    : std::runtime_error(msg) {}

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), pos(0), modo_fun(false) {}

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

// <vardecl> ::= 'var' <ident> '=' <exp> ';'
Decl Parser::analisaVarDecl() {
    consumir(TokenType::VAR, "no inicio da declaracao de variavel");
    Token nomeTok = consumir(TokenType::IDENTIFICADOR,
                              "como nome da variavel");
    consumir(TokenType::IGUAL, "apos o nome da variavel");
    std::unique_ptr<Exp> valor = analisaExp();
    consumir(TokenType::PONTO_VIRGULA, "ao final da declaracao");
    return Decl(nomeTok.get_lexema(), std::move(valor), true);
}

std::vector<std::string> Parser::analisaParametrosFormais() {
    std::vector<std::string> parametros;
    if (atual().get_tipo() != TokenType::PAREN_DIR) {
        while (true) {
            Token parametro = consumir(
                TokenType::IDENTIFICADOR,
                "como parametro formal da funcao");
            parametros.push_back(parametro.get_lexema());
            if (atual().get_tipo() != TokenType::VIRGULA)
                break;
            avancar();
        }
    }
    consumir(TokenType::PAREN_DIR, "para fechar a lista de parametros");
    return parametros;
}

std::vector<std::unique_ptr<Exp>> Parser::analisaArgumentos() {
    std::vector<std::unique_ptr<Exp>> argumentos;
    if (atual().get_tipo() != TokenType::PAREN_DIR) {
        while (true) {
            argumentos.push_back(analisaExp());
            if (atual().get_tipo() != TokenType::VIRGULA)
                break;
            avancar();
        }
    }
    consumir(TokenType::PAREN_DIR, "para fechar a chamada de funcao");
    return argumentos;
}

Funcao Parser::analisaFuncao() {
    consumir(TokenType::FUN, "no inicio da declaracao de funcao");
    Token nome = consumir(TokenType::IDENTIFICADOR,
                          "como nome da funcao");
    consumir(TokenType::PAREN_ESQ, "apos o nome da funcao");
    std::vector<std::string> parametros = analisaParametrosFormais();

    std::vector<Decl> variaveis_locais;
    std::unique_ptr<Bloco> corpo = analisaCorpoFun(variaveis_locais);

    return Funcao(nome.get_lexema(), std::move(parametros),
                  std::move(variaveis_locais), std::move(corpo));
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

    if (tok.get_tipo() == TokenType::IDENTIFICADOR) {
        Token nome = avancar();
        if (atual().get_tipo() == TokenType::PAREN_ESQ) {
            avancar();
            return std::make_unique<ChamadaFuncao>(
                nome.get_lexema(), analisaArgumentos());
        }

        // referencia a uma variavel: a verificacao de declaracao pertence
        // a analise semantica.
        return std::make_unique<Var>(nome.get_lexema());
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

// verifica recursivamente se um comando contem um 'return' em algum nivel
// (diretamente, ou dentro de um bloco aninhado, de um dos ramos de um if,
// ou do corpo de um while)
static bool contem_return(const Cmd& cmd) {
    if (dynamic_cast<const Retorno*>(&cmd) != nullptr)
        return true;

    if (const auto* bloco = dynamic_cast<const Bloco*>(&cmd)) {
        for (const auto& c : bloco->get_comandos())
            if (contem_return(*c))
                return true;
        return false;
    }

    if (const auto* no_if = dynamic_cast<const If*>(&cmd)) {
        for (const auto& c : no_if->get_entao().get_comandos())
            if (contem_return(*c))
                return true;
        if (no_if->tem_senao())
            for (const auto& c : no_if->get_senao().get_comandos())
                if (contem_return(*c))
                    return true;
        return false;
    }

    if (const auto* no_while = dynamic_cast<const While*>(&cmd)) {
        for (const auto& c : no_while->get_corpo().get_comandos())
            if (contem_return(*c))
                return true;
        return false;
    }

    return false;  // Atribuicao nao contem return
}

// Corpo de funcao/main: '{' <vardecl>* <cmd>* '}'
// As declaracoes locais vem antes dos comandos e sao devolvidas em
// 'locais'. Os comandos seguem a mesma gramatica <cmd> de um bloco comum
// (atribuicao, if, while, return, bloco aninhado): um 'return' pode
// aparecer tanto diretamente no corpo quanto dentro dos ramos de um if/else
// ou de um while — por exemplo, uma funcao recursiva escrita com
// "if (...) { return ...; } else { return ...; }" e uma forma valida e
// comum de terminar o corpo, mesmo sem um 'return' textualmente no ultimo
// nivel. Ainda assim, o corpo precisa conter pelo menos um 'return' em
// algum ponto (em qualquer nivel de aninhamento) — um corpo sem nenhum
// 'return' e um erro de sintaxe, ja que toda funcao (e o bloco main) deve
// produzir um valor. Serve tanto ao corpo de uma funcao quanto ao bloco
// main, que declara locais do mesmo jeito.
std::unique_ptr<Bloco> Parser::analisaCorpoFun(std::vector<Decl>& locais) {
    consumir(TokenType::CHAVE_ESQ, "para abrir o bloco");

    while (atual().get_tipo() == TokenType::VAR)
        locais.push_back(analisaVarDecl());

    std::vector<std::unique_ptr<Cmd>> comandos;
    while (atual().get_tipo() != TokenType::CHAVE_DIR &&
           atual().get_tipo() != TokenType::FIM)
        comandos.push_back(analisaCmd());

    consumir(TokenType::CHAVE_DIR, "para fechar o bloco");

    bool tem_return = false;
    for (const auto& c : comandos)
        if (contem_return(*c))
            { tem_return = true; break; }
    if (!tem_return)
        throw ErroSintatico(
            "corpo de funcao/main precisa conter pelo menos um 'return'");

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
    bool tem_parenteses = atual().get_tipo() == TokenType::PAREN_ESQ;
    if (!modo_fun || tem_parenteses) {
        consumir(TokenType::PAREN_ESQ, "apos o if");
        tem_parenteses = true;
    }
    std::unique_ptr<Exp> condicao = analisaExp();
    if (tem_parenteses)
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
    bool tem_parenteses = atual().get_tipo() == TokenType::PAREN_ESQ;
    if (!modo_fun || tem_parenteses) {
        consumir(TokenType::PAREN_ESQ, "apos o while");
        tem_parenteses = true;
    }
    std::unique_ptr<Exp> condicao = analisaExp();
    if (tem_parenteses)
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

std::unique_ptr<Programa> Parser::analisaProgramaFun() {
    modo_fun = true;
    std::vector<Decl> variaveis_globais;
    std::vector<Funcao> funcoes;
    std::vector<DeclaracaoTopo> ordem_declaracoes;

    while (atual().get_tipo() == TokenType::VAR ||
           atual().get_tipo() == TokenType::FUN) {
        if (atual().get_tipo() == TokenType::VAR) {
            variaveis_globais.push_back(analisaVarDecl());
            ordem_declaracoes.push_back(
                {TipoDeclaracaoTopo::VARIAVEL,
                 variaveis_globais.size() - 1});
        } else {
            funcoes.push_back(analisaFuncao());
            ordem_declaracoes.push_back(
                {TipoDeclaracaoTopo::FUNCAO, funcoes.size() - 1});
        }
    }

    consumir(TokenType::MAIN,
             "depois das declaracoes globais e de funcoes");
    std::vector<Decl> locais_main;
    std::unique_ptr<Bloco> principal = analisaCorpoFun(locais_main);
    consumir(TokenType::FIM, "ao final do programa");

    auto programa = std::make_unique<Programa>(
        std::move(variaveis_globais), std::move(funcoes),
        std::move(ordem_declaracoes), std::move(locais_main),
        std::move(principal));

    // Nesta primeira parte, a verificacao existente continua cobrindo
    // variaveis globais e o bloco main. Escopos e chamadas de funcao sao
    // responsabilidade da Parte 2 da Atividade 10.
    verificar_variaveis(*programa);
    return programa;
}

// <programa> ::= <decl>* ( '=' <exp> | <bloco> )
//
// olha o proximo token: enquanto for um identificador, reconhece mais uma
// declaracao; depois, um '{' abre o corpo de comandos (linguagem Cmd) e um
// '=' inicia a expressao final (forma EV das atividades anteriores).
std::unique_ptr<Programa> Parser::analisar() {
    if (atual().get_tipo() == TokenType::VAR ||
        atual().get_tipo() == TokenType::FUN ||
        atual().get_tipo() == TokenType::MAIN)
        return analisaProgramaFun();

    std::vector<Decl> decls;

    while (atual().get_tipo() == TokenType::IDENTIFICADOR)
        decls.push_back(analisaDecl());

    if (atual().get_tipo() == TokenType::CHAVE_ESQ) {
        std::unique_ptr<Bloco> corpo = analisaBloco();
        consumir(TokenType::FIM, "ao final do programa");

        auto programa = std::make_unique<Programa>(std::move(decls), std::move(corpo));

        // analise semantica: verifica se toda variavel usada (nas
        // declaracoes e nos comandos do corpo — atribuicoes, condicoes de
        // if/while, return) foi declarada antes de seu uso, e que nenhuma
        // atribuicao referencia uma variavel nao declarada. Lanca
        // ErroSemantico em caso de erro.
        verificar_variaveis(*programa);

        return programa;
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
