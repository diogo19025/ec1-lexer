// Testes de analise semantica da linguagem Fun (Atividade 10 - parte 2).
// Cobre: funcao chamada sem ter sido declarada, nome chamado que nao e uma
// funcao, quantidade de argumentos diferente da quantidade de parametros,
// escopo (resolucao local antes de global, sombreamento de globais por
// parametros e variaveis locais, e o isolamento entre os escopos de funcoes
// diferentes) e recursao direta.

#include "lexer.h"
#include "parser.h"
#include "semantica.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

static int falhas = 0;

static void checar(bool condicao, const std::string& descricao) {
    if (condicao) {
        std::cout << "[OK]    " << descricao << "\n";
    } else {
        std::cout << "[FALHA] " << descricao << "\n";
        ++falhas;
    }
}

// tokeniza e faz o parse completo de um programa; o proprio Parser::analisar
// ja executa a analise semantica e lanca ErroSemantico/ErroSintatico
static std::unique_ptr<Programa> parse(const std::string& fonte) {
    Lexer lexer(fonte);
    std::vector<Token> tokens;
    while (true) {
        Token t = lexer.proximo_token();
        tokens.push_back(t);
        if (t.get_tipo() == TokenType::FIM)
            break;
    }
    Parser parser(std::move(tokens));
    return parser.analisar();
}

// espera que o programa seja aceito (sem erro sintatico nem semantico)
static bool aceita(const std::string& fonte) {
    try {
        parse(fonte);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// espera que o programa seja rejeitado especificamente por ErroSemantico
// (nao por ErroSintatico, que seria um erro na parte 1)
static bool rejeita_semantico(const std::string& fonte) {
    try {
        parse(fonte);
    } catch (const ErroSemantico&) {
        return true;
    } catch (const std::exception&) {
        return false;
    }
    return false;
}

// -----------------------------------------------------------------------
// Funcao chamada sem ter sido declarada
// -----------------------------------------------------------------------

static void teste_funcao_inexistente() {
    checar(rejeita_semantico("main { return f(1); }"),
           "chamada a funcao que nao existe no programa e rejeitada");
    checar(rejeita_semantico("fun g() { return 1; } main { return f(); }"),
           "chamada a funcao com nome errado e rejeitada");
    checar(rejeita_semantico("fun f() { return g(); } main { return f(); }"),
           "chamada a funcao inexistente dentro do corpo de outra e rejeitada");
    checar(rejeita_semantico("fun f(x) { return x; } main { return f(g(1)); }"),
           "chamada inexistente usada como argumento e rejeitada");

    // uma funcao so passa a existir a partir da sua declaracao: como as
    // declaracoes de topo sao processadas na ordem do programa, chamar uma
    // funcao declarada mais adiante e um erro
    checar(rejeita_semantico("fun a() { return b(); }"
                             "fun b() { return 1; }"
                             "main { return a(); }"),
           "chamada a funcao declarada depois (para frente) e rejeitada");

    checar(aceita("fun b() { return 1; }"
                  "fun a() { return b(); }"
                  "main { return a(); }"),
           "chamada a funcao declarada antes e aceita");
}

// -----------------------------------------------------------------------
// Nome chamado que nao representa uma funcao (e o inverso)
// -----------------------------------------------------------------------

static void teste_nome_nao_e_funcao() {
    checar(rejeita_semantico("var v = 1; main { return v(1); }"),
           "chamar uma variavel global e rejeitado");
    checar(rejeita_semantico("fun f(x) { return x(1); } main { return f(2); }"),
           "chamar um parametro e rejeitado");
    checar(rejeita_semantico("fun f() { var a = 1; return a(); }"
                             "main { return f(); }"),
           "chamar uma variavel local e rejeitado");
    checar(rejeita_semantico("main { var a = 1; return a(); }"),
           "chamar uma variavel local do main e rejeitado");

    // o outro lado da mesma regra: o nome de uma funcao so pode aparecer
    // numa chamada, nunca como valor de uma expressao
    checar(rejeita_semantico("fun f() { return 1; } main { return f; }"),
           "usar o nome de uma funcao como variavel e rejeitado");
    checar(rejeita_semantico("fun f() { return 1; }"
                             "main { return f + 1; }"),
           "usar o nome de uma funcao dentro de uma expressao e rejeitado");
    checar(rejeita_semantico("fun f() { return 1; }"
                             "main { f = 1; return 0; }"),
           "atribuir ao nome de uma funcao e rejeitado");
}

// -----------------------------------------------------------------------
// Quantidade de argumentos da chamada
// -----------------------------------------------------------------------

static void teste_aridade() {
    checar(rejeita_semantico("fun f(x) { return x; } main { return f(); }"),
           "argumentos de menos e rejeitado");
    checar(rejeita_semantico("fun f(x) { return x; } main { return f(1, 2); }"),
           "argumentos de mais e rejeitado");
    checar(rejeita_semantico("fun f() { return 1; } main { return f(1); }"),
           "argumento passado a funcao sem parametros e rejeitado");
    checar(rejeita_semantico("fun f(x, y, z) { return x + y + z; }"
                             "main { return f(1, 2); }"),
           "aridade errada com varios parametros e rejeitada");
    checar(rejeita_semantico("fun um() { return 1; }"
                             "fun f(x) { return x; }"
                             "main { return f(um(1)); }"),
           "aridade errada numa chamada aninhada e rejeitada");
    checar(rejeita_semantico("fun f(n) { return f(n, 1); } main { return f(1); }"),
           "aridade errada numa chamada recursiva e rejeitada");

    checar(aceita("fun f() { return 1; } main { return f(); }"),
           "chamada sem argumentos a funcao sem parametros e aceita");
    checar(aceita("fun f(x, y, z) { return x + y + z; }"
                  "main { return f(1, 2, 3); }"),
           "chamada com a quantidade certa de argumentos e aceita");
    checar(aceita("fun zero() { return 0; }"
                  "fun soma(x, y) { return x + y; }"
                  "main { return soma(zero(), soma(2, 3)); }"),
           "chamadas aninhadas com aridades corretas sao aceitas");
}

// -----------------------------------------------------------------------
// Escopo: resolucao local antes de global, sombreamento e isolamento
// -----------------------------------------------------------------------

static void teste_escopo() {
    // do escopo local para o global
    checar(aceita("var g = 5; fun f() { return g; } main { return f(); }"),
           "funcao enxerga uma variavel global declarada antes dela");
    checar(aceita("var g = 5; fun f() { g = 6; return g; } main { return f(); }"),
           "funcao pode atribuir a uma variavel global");
    checar(aceita("fun f(x) { var d = x + x; return d; } main { return f(2); }"),
           "funcao enxerga os proprios parametros e variaveis locais");
    checar(rejeita_semantico("fun f() { return g; }"
                             "var g = 5;"
                             "main { return f(); }"),
           "funcao nao enxerga uma global declarada depois dela");

    // sombreamento: o escopo local e consultado primeiro
    checar(aceita("var g = 5; fun f(g) { return g; } main { return f(1); }"),
           "parametro esconde uma global de mesmo nome");
    checar(aceita("var g = 5; fun f() { var g = 1; return g; }"
                  "main { return f(); }"),
           "variavel local esconde uma global de mesmo nome");
    checar(aceita("var g = 5; main { var g = 1; return g; }"),
           "variavel local do main esconde uma global de mesmo nome");
    checar(aceita("var g = 5; fun f(g) { g = g + 1; return g; }"
                  "main { return f(1); }"),
           "atribuicao dentro da funcao atinge o parametro, nao a global");

    // isolamento entre escopos
    checar(rejeita_semantico("fun f(x) { return x; } main { return x; }"),
           "parametro de uma funcao nao vaza para o main");
    checar(rejeita_semantico("fun f() { var y = 1; return y; }"
                             "main { return y; }"),
           "variavel local de uma funcao nao vaza para o main");
    checar(rejeita_semantico("fun f(x) { return x; }"
                             "fun g() { return x; }"
                             "main { return g(); }"),
           "parametro de uma funcao nao vaza para outra funcao");
    checar(rejeita_semantico("fun f() { return m; }"
                             "main { var m = 1; return f(); }"),
           "variavel local do main nao e visivel dentro de uma funcao");

    // uma local so existe a partir da sua propria declaracao
    checar(rejeita_semantico("fun f() { var a = b; var b = 1; return a; }"
                             "main { return f(); }"),
           "variavel local usada antes de ser declarada e rejeitada");
    checar(aceita("fun f() { var a = 1; var b = a + 1; return b; }"
                  "main { return f(); }"),
           "variavel local pode usar outra declarada antes dela");

    // o escopo vale tambem dentro dos comandos aninhados
    checar(rejeita_semantico("fun f(x) { if x > 0 { return z; } return x; }"
                             "main { return f(1); }"),
           "nome nao declarado dentro de um if na funcao e rejeitado");
    checar(rejeita_semantico("fun f(x) { while x > 0 { x = z; } return x; }"
                             "main { return f(1); }"),
           "nome nao declarado dentro de um while na funcao e rejeitado");
    checar(rejeita_semantico("fun f() { z = 1; return 1; } main { return f(); }"),
           "atribuicao a nome nao declarado dentro da funcao e rejeitada");
}

// -----------------------------------------------------------------------
// Recursao direta
// -----------------------------------------------------------------------

static void teste_recursao() {
    checar(aceita("fun f(n) { return f(n - 1); } main { return f(3); }"),
           "funcao pode chamar a si mesma (registrada antes do corpo)");
    checar(aceita("fun fat(n) {"
                  "  var r = 1;"
                  "  if n > 1 { r = n * fat(n - 1); }"
                  "  return r;"
                  "}"
                  "main { return fat(5); }"),
           "fatorial recursivo e aceito");
    checar(aceita("fun fib(n) {"
                  "  var r = 0;"
                  "  if n < 2 { r = 1; }"
                  "  else { r = fib(n - 1) + fib(n - 2); }"
                  "  return r;"
                  "}"
                  "main { return fib(8); }"),
           "fibonacci com duas chamadas recursivas e aceito");
    checar(aceita("var g = 1;"
                  "fun f(n) { var r = g; if n > 0 { r = f(n - 1); } return r; }"
                  "main { return f(3); }"),
           "funcao recursiva que tambem usa uma global e aceita");
}

// -----------------------------------------------------------------------
// Regressao: as formas EV e Cmd continuam validas
// -----------------------------------------------------------------------

static void teste_regressao_ev_cmd() {
    checar(aceita("l = 30; c = 40; = l + l + c + c"),
           "forma EV (declaracoes + expressao final) continua aceita");
    checar(rejeita_semantico("= x"),
           "forma EV com variavel nao declarada continua rejeitada");
    checar(aceita("x = 1; { x = x + 1; return x; }"),
           "forma Cmd (declaracoes + bloco de comandos) continua aceita");
    checar(rejeita_semantico("x = 1; { y = 2; return y; }"),
           "forma Cmd com atribuicao a nao declarada continua rejeitada");
}

int main() {
    teste_funcao_inexistente();
    teste_nome_nao_e_funcao();
    teste_aridade();
    teste_escopo();
    teste_recursao();
    teste_regressao_ev_cmd();

    if (falhas == 0) {
        std::cout
            << "Todos os testes de analise semantica da linguagem Fun passaram.\n";
        return 0;
    }
    std::cout << falhas << " teste(s) falharam.\n";
    return 1;
}
