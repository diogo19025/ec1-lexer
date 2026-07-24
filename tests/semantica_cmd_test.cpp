// Testes de analise semantica da linguagem Cmd (Atividade 09 - parte 3).
// Cobre: verificacao de variaveis usadas em expressoes dentro de comandos,
// atribuicao a variavel nao declarada, e percurso dos comandos presentes
// dentro de if e while (incluindo os dois ramos do if e blocos aninhados).

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
// (nao por ErroSintatico, que seria um erro na parte 2)
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
// Programas validos: atribuicao, condicao (if/else) e repeticao (while)
// -----------------------------------------------------------------------

static void teste_atribuicao_valida() {
    checar(aceita("x = 1; { x = x + 1; return x; }"),
           "atribuicao a variavel previamente declarada eh aceita");
    checar(aceita("x = 1; y = 2; { x = y; y = x; return x + y; }"),
           "atribuicoes cruzadas entre variaveis ja declaradas sao aceitas");
}

static void teste_condicao_valida() {
    checar(aceita("x = 5; { if (x > 0) { return 1; } else { return 0; } }"),
           "if/else usando variavel declarada eh aceito");
    checar(aceita("x = 5; y = 10; { if (x < y) { x = y; } return x; }"),
           "atribuicao dentro do ramo 'entao' de um if eh aceita");
    checar(aceita("x = 1; { if (x == 1) { x = 2; } }"),
           "if sem else, com atribuicao no corpo, eh aceito");
}

static void teste_repeticao_valida() {
    checar(aceita("i = 0; soma = 0; "
                  "{ while (i < 10) { soma = soma + i; i = i + 1; } "
                  "return soma; }"),
           "while somando os elementos de 0 a 9 eh aceito");
    checar(aceita("i = 0; { while (i < 5) { i = i + 1; } return i; }"),
           "while simples com uma unica atribuicao no corpo eh aceito");
}

static void teste_programas_completos() {
    // maior de dois numeros
    checar(aceita("a = 7; b = 3; "
                  "{ if (a > b) { return a; } else { return b; } }"),
           "programa completo: maior de dois numeros");

    // fatorial iterativo
    checar(aceita("n = 5; fat = 1; i = 1; "
                  "{ while (i < n + 1) { fat = fat * i; i = i + 1; } "
                  "return fat; }"),
           "programa completo: fatorial iterativo com while");

    // blocos aninhados dentro de if/while
    checar(aceita("x = 0; { while (x < 3) { { x = x + 1; } } return x; }"),
           "bloco aninhado dentro do corpo de um while eh aceito");
}

// -----------------------------------------------------------------------
// Erros semanticos: variavel nao declarada, em cada contexto de uso
// -----------------------------------------------------------------------

static void teste_atribuicao_variavel_nao_declarada() {
    checar(rejeita_semantico("{ x = 1; return x; }"),
           "atribuicao a variavel nunca declarada gera ErroSemantico");
    checar(rejeita_semantico("x = 1; { y = 2; return x + y; }"),
           "atribuicao a variavel diferente da declarada gera ErroSemantico");
}

static void teste_expressao_variavel_nao_declarada() {
    checar(rejeita_semantico("{ return y; }"),
           "return de variavel nao declarada gera ErroSemantico");
    checar(rejeita_semantico("x = 1; { return x + z; }"),
           "uso de variavel nao declarada numa soma gera ErroSemantico");
}

static void teste_condicao_variavel_nao_declarada() {
    checar(rejeita_semantico("{ if (x > 0) { return 1; } }"),
           "condicao do if com variavel nao declarada gera ErroSemantico");
    checar(rejeita_semantico("x = 1; { if (x > 0) { return y; } }"),
           "uso de variavel nao declarada dentro do ramo 'entao' gera ErroSemantico");
    checar(rejeita_semantico("x = 1; { if (x > 0) { return x; } else { return y; } }"),
           "uso de variavel nao declarada dentro do ramo 'senao' gera ErroSemantico");
}

static void teste_repeticao_variavel_nao_declarada() {
    checar(rejeita_semantico("{ while (i < 10) { return 0; } }"),
           "condicao do while com variavel nao declarada gera ErroSemantico");
    checar(rejeita_semantico("i = 0; { while (i < 10) { i = j + 1; } }"),
           "atribuicao usando variavel nao declarada dentro do while gera ErroSemantico");
}

static void teste_bloco_aninhado_variavel_nao_declarada() {
    checar(rejeita_semantico("{ { return w; } }"),
           "variavel nao declarada dentro de bloco aninhado gera ErroSemantico");
}

int main() {
    teste_atribuicao_valida();
    teste_condicao_valida();
    teste_repeticao_valida();
    teste_programas_completos();

    teste_atribuicao_variavel_nao_declarada();
    teste_expressao_variavel_nao_declarada();
    teste_condicao_variavel_nao_declarada();
    teste_repeticao_variavel_nao_declarada();
    teste_bloco_aninhado_variavel_nao_declarada();

    std::cout << "\n";
    if (falhas == 0) {
        std::cout << "Todos os testes de analise semantica da linguagem Cmd passaram.\n";
        return 0;
    }
    std::cout << falhas << " teste(s) falharam.\n";
    return 1;
}
