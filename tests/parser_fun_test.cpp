#include "lexer.h"
#include "parser.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

static int falhas = 0;

static void checar(bool condicao, const std::string& descricao) {
    if (condicao) {
        std::cout << "[OK]    " << descricao << "\n";
    } else {
        std::cerr << "[FALHA] " << descricao << "\n";
        ++falhas;
    }
}

static std::unique_ptr<Programa> parse(const std::string& fonte) {
    Lexer lexer(fonte);
    std::vector<Token> tokens = lexer.tokenizar();
    Parser parser(std::move(tokens));
    return parser.analisar();
}

static bool rejeita(const std::string& fonte) {
    try {
        parse(fonte);
    } catch (const ErroSintatico&) {
        return true;
    }
    return false;
}

static void teste_programa_fun_completo() {
    auto programa = parse(
        "var base = 10;"
        "fun soma(x, y) {"
        "  var total = x + y;"
        "  total = total + base;"
        "  return total;"
        "}"
        "main { return soma(2, 3); }");

    checar(programa->eh_fun(), "programa com main usa a forma Fun");
    checar(programa->get_decls().size() == 1,
           "declaracao de variavel global e reconhecida");
    checar(programa->get_decls()[0].usa_var(),
           "declaracao Fun preserva a palavra-chave var");
    checar(programa->get_funcoes().size() == 1,
           "declaracao de funcao e reconhecida");
    checar(programa->get_ordem_declaracoes().size() == 2 &&
           programa->get_ordem_declaracoes()[0].tipo ==
               TipoDeclaracaoTopo::VARIAVEL &&
           programa->get_ordem_declaracoes()[1].tipo ==
               TipoDeclaracaoTopo::FUNCAO,
           "ordem das declaracoes de topo e preservada");

    const Funcao& soma = programa->get_funcoes()[0];
    checar(soma.get_nome() == "soma", "nome da funcao e preservado");
    checar(soma.get_parametros().size() == 2 &&
           soma.get_parametros()[0] == "x" &&
           soma.get_parametros()[1] == "y",
           "parametros formais sao reconhecidos em ordem");
    checar(soma.get_variaveis_locais().size() == 1 &&
           soma.get_variaveis_locais()[0].get_nome() == "total",
           "variaveis locais ficam separadas dos comandos");
    checar(soma.get_corpo().get_comandos().size() == 2,
           "corpo contem comando e return final");

    const auto* retorno = dynamic_cast<const Retorno*>(
        programa->get_corpo().get_comandos().back().get());
    const auto* chamada = retorno
        ? dynamic_cast<const ChamadaFuncao*>(&retorno->get_valor())
        : nullptr;
    checar(chamada && chamada->get_nome() == "soma" &&
           chamada->get_argumentos().size() == 2,
           "chamada de funcao e distinguida de referencia a variavel");
}

static void teste_chamadas_aninhadas_e_vazias() {
    auto programa = parse(
        "fun zero() { return 0; }"
        "fun soma(x, y) { return x + y; }"
        "main { return soma(zero(), soma(2, 3)); }");

    const auto* retorno = dynamic_cast<const Retorno*>(
        programa->get_corpo().get_comandos().back().get());
    const auto* chamada = retorno
        ? dynamic_cast<const ChamadaFuncao*>(&retorno->get_valor())
        : nullptr;
    checar(chamada && chamada->get_argumentos().size() == 2,
           "listas de argumentos aceitam expressoes aninhadas");
    const auto* zero = chamada
        ? dynamic_cast<const ChamadaFuncao*>(
              chamada->get_argumentos()[0].get())
        : nullptr;
    checar(zero && zero->get_argumentos().empty(),
           "funcao sem parametros pode ser chamada");
}

static void teste_recursao_sintatica() {
    auto programa = parse(
        "fun fib(n) {"
        "  var res = 0;"
        "  if n < 2 { res = 1; }"
        "  else { res = fib(n - 1) + fib(n - 2); }"
        "  return res;"
        "}"
        "main { return fib(8); }");

    checar(programa->get_funcoes().size() == 1,
           "funcao recursiva e aceita sintaticamente");
    checar(programa->get_funcoes()[0].get_corpo().get_comandos().size() == 2,
           "if/else sem parenteses segue a gramatica Fun");
}

static void teste_erros_sintaticos() {
    checar(rejeita("fun f() { return 1; }"),
           "programa Fun sem main e rejeitado");
    checar(rejeita("main { }"),
           "main sem return final e rejeitado");
    checar(rejeita("fun f(x,) { return x; } main { return f(1); }"),
           "virgula final em parametros formais e rejeitada");
    checar(rejeita("fun f() { var x = 1; x = 2; } main { return 0; }"),
           "funcao sem return final e rejeitada");
    checar(rejeita("main { return f(1,); }"),
           "virgula final em argumentos reais e rejeitada");
}

int main() {
    teste_programa_fun_completo();
    teste_chamadas_aninhadas_e_vazias();
    teste_recursao_sintatica();
    teste_erros_sintaticos();

    if (falhas == 0) {
        std::cout << "Todos os testes do parser da linguagem Fun passaram.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << falhas << " teste(s) falharam.\n";
    return EXIT_FAILURE;
}
