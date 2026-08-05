// Testes de geracao de codigo para funcoes da linguagem Fun (Atividade 10 -
// parte 3). Cobre: empilhamento de argumentos em ordem inversa e chamada
// (call/limpeza da pilha), rotulo/prologo/epilogo de funcao preservando
// %rbp, deslocamentos de parametros e variaveis locais em relacao a %rbp,
// diferenciacao entre acesso a variavel global e local, e funcoes sem
// parametros, sem variaveis locais, aninhadas e recursivas. Verifica o
// texto do assembly gerado (nao monta nem executa: isso e coberto pelos
// testes ponta a ponta da parte 4).

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <iostream>
#include <memory>
#include <sstream>
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

// gera o assembly completo de um programa Fun e devolve como string, para
// que os testes possam checar trechos esperados com find()
static std::string gerar(const std::string& fonte) {
    std::unique_ptr<Programa> programa = parse(fonte);
    std::ostringstream out;
    gerar_assembly_completo(*programa, out);
    return out.str();
}

static bool contem(const std::string& texto, const std::string& trecho) {
    return texto.find(trecho) != std::string::npos;
}

// -----------------------------------------------------------------------
// Chamada de funcao: argumentos em ordem inversa, call, limpeza da pilha
// -----------------------------------------------------------------------

static void teste_chamada_argumentos() {
    std::string s = gerar(
        "fun soma(x, y) { return x + y; }"
        "main { return soma(1, 2); }");

    // o segundo argumento (2) e calculado e empilhado antes do primeiro (1)
    std::size_t pos_push_2 = s.find("mov $2, %rax");
    std::size_t pos_push_1 = s.find("mov $1, %rax");
    checar(pos_push_2 != std::string::npos && pos_push_1 != std::string::npos &&
           pos_push_2 < pos_push_1,
           "segundo argumento e calculado/empilhado antes do primeiro (ordem inversa)");

    checar(contem(s, "call soma"), "chamada emite \"call soma\"");
    checar(contem(s, "add $16, %rsp"),
           "os dois argumentos (16 bytes) sao removidos da pilha apos a chamada");

    std::string s1 = gerar("fun f() { return 1; } main { return f(); }");
    checar(!contem(s1, "add $"),
           "chamada sem argumentos nao remove nada da pilha depois do call");
}

// -----------------------------------------------------------------------
// Rotulo, prologo e epilogo de funcao; preservacao de %rbp
// -----------------------------------------------------------------------

static void teste_prologo_epilogo() {
    std::string s = gerar("fun f(x) { return x; } main { return f(1); }");

    checar(contem(s, "f:\n    push %rbp\n    mov %rsp, %rbp"),
           "funcao comeca com rotulo, push %rbp e mov %rsp, %rbp");
    checar(contem(s, "mov %rbp, %rsp\n    pop %rbp\n    ret"),
           "epilogo desfaz o prologo (mov %rbp, %rsp e pop %rbp) antes do ret");
    checar(contem(s, "ret"), "funcao termina com ret");
}

// -----------------------------------------------------------------------
// Deslocamentos de parametros e variaveis locais em relacao a %rbp
// -----------------------------------------------------------------------

static void teste_deslocamentos() {
    std::string s = gerar(
        "fun f(a, b, c) {"
        "  var x = a;"
        "  var y = b;"
        "  return c;"
        "}"
        "main { return f(1, 2, 3); }");

    // parametros: 16(%rbp), 24(%rbp), 32(%rbp), na ordem declarada
    checar(contem(s, "mov 16(%rbp), %rax"),
           "primeiro parametro fica em 16(%rbp)");
    checar(contem(s, "mov 24(%rbp), %rax"),
           "segundo parametro fica em 24(%rbp)");
    checar(contem(s, "mov 32(%rbp), %rax"),
           "terceiro parametro fica em 32(%rbp)");

    // locais: -8(%rbp), -16(%rbp), na ordem declarada
    checar(contem(s, "mov %rax, -8(%rbp)"),
           "primeira variavel local fica em -8(%rbp)");
    checar(contem(s, "mov %rax, -16(%rbp)"),
           "segunda variavel local fica em -16(%rbp)");
    checar(contem(s, "sub $16, %rsp"),
           "espaco das duas variaveis locais (16 bytes) e reservado de uma vez");
}

// -----------------------------------------------------------------------
// Acesso a variavel global vs local (parametro/local esconde global)
// -----------------------------------------------------------------------

static void teste_global_vs_local() {
    std::string s = gerar(
        "var g = 10;"
        "fun f(g) { return g; }"
        "main { return f(1) + g; }");

    // dentro de f, o nome 'g' e o parametro (deslocamento), nao a global
    std::size_t pos_f = s.find("f:\n");
    std::size_t pos_return_f = s.find("return g;", pos_f);
    checar(pos_return_f != std::string::npos, "corpo da funcao f encontrado");
    std::size_t pos_mov_apos = s.find("mov ", pos_return_f);
    checar(pos_mov_apos != std::string::npos &&
           s.compare(pos_mov_apos, 9, "mov 16(%r") == 0,
           "dentro de f, 'g' (parametro) acessa 16(%rbp), nao a global");

    // no main, o 'g' apos a chamada e a global, acessada pelo nome
    checar(contem(s, "mov g, %rax"),
           "no main, 'g' (global) e acessado pelo nome, nao por deslocamento");
}

// -----------------------------------------------------------------------
// Funcao sem parametros e sem variaveis locais
// -----------------------------------------------------------------------

static void teste_sem_parametros_sem_locais() {
    std::string s = gerar("fun um() { return 1; } main { return um(); }");

    checar(contem(s, "um:\n    push %rbp\n    mov %rsp, %rbp\n    # return 1;"),
           "funcao sem parametros e sem locais nao reserva espaco extra na pilha");
    checar(!contem(s, "sub $0, %rsp"),
           "nao emite um \"sub $0\" inutil quando nao ha variaveis locais");
}

// -----------------------------------------------------------------------
// Funcoes aninhadas (uma chama a outra) e recursivas
// -----------------------------------------------------------------------

static void teste_aninhada_e_recursiva() {
    std::string s = gerar(
        "fun dobro(x) { return x + x; }"
        "fun quadruplo(x) { return dobro(dobro(x)); }"
        "main { return quadruplo(2); }");

    checar(contem(s, "call dobro") && contem(s, "call quadruplo"),
           "funcao aninhada chama a outra e e chamada pelo main");

    std::string r = gerar(
        "fun fat(n) {"
        "  var r = 1;"
        "  if n > 1 { r = n * fat(n - 1); }"
        "  return r;"
        "}"
        "main { return fat(5); }");

    checar(contem(r, "call fat"),
           "funcao recursiva chama a si mesma por nome, sem tratamento especial");
    checar(contem(r, "fat:\n    push %rbp"),
           "rotulo e prologo da funcao recursiva sao gerados normalmente");
}

int main() {
    teste_chamada_argumentos();
    teste_prologo_epilogo();
    teste_deslocamentos();
    teste_global_vs_local();
    teste_sem_parametros_sem_locais();
    teste_aninhada_e_recursiva();

    if (falhas == 0) {
        std::cout
            << "Todos os testes de geracao de codigo da linguagem Fun passaram.\n";
        return 0;
    }
    std::cout << falhas << " teste(s) falharam.\n";
    return 1;
}
