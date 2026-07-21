// Testes do parser e da analise semantica da linguagem EV (Atividade 08)
// Responsabilidade da Pessoa 3: parser (declaracoes + expressao final) e
// verificacao de variaveis nao declaradas.

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

// tokeniza e faz o parse completo (sintaxe + semantica) de um programa EV
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

// -----------------------------------------------------------------------
// Casos que devem ser aceitos
// -----------------------------------------------------------------------

static void teste_programa_sem_declaracoes() {
    auto programa = parse("= 1 + 2 * 3");
    checar(programa != nullptr, "programa sem declaracoes eh aceito");
    checar(programa->get_decls().empty(),
           "programa sem declaracoes nao tem nenhuma Decl");
    checar(programa->get_exp().imprimir() == "(1 + (2 * 3))",
           "expressao final e construida com a precedencia correta");
}

static void teste_programa_com_declaracoes() {
    auto programa = parse(
        "x = (7 + 4) * 12;\n"
        "y = x * 3 + 11;\n"
        "= (x * y) + (x * 11) + (y * 13)\n");

    checar(programa != nullptr, "programa com declaracoes eh aceito");
    checar(programa->get_decls().size() == 2,
           "programa tem exatamente 2 declaracoes");
    checar(programa->get_decls()[0].get_nome() == "x",
           "primeira declaracao e de 'x'");
    checar(programa->get_decls()[1].get_nome() == "y",
           "segunda declaracao e de 'y'");
}

static void teste_variavel_usada_apos_ser_declarada() {
    bool lancou = false;
    try {
        parse("x = 10; y = x + 1; = x * y");
    } catch (const ErroSemantico&) {
        lancou = true;
    }
    checar(!lancou,
           "variavel usada apos ser declarada nao gera erro semantico");
}

// -----------------------------------------------------------------------
// Casos que devem ser rejeitados
// -----------------------------------------------------------------------

static void teste_variavel_nao_declarada_na_expressao_final() {
    bool lancou = false;
    try {
        parse("x = 1; = x + z");
    } catch (const ErroSemantico&) {
        lancou = true;
    }
    checar(lancou,
           "uso de variavel nao declarada na expressao final gera ErroSemantico");
}

static void teste_variavel_usada_antes_de_ser_declarada() {
    // y e usada na declaracao de x, mas so e declarada depois
    bool lancou = false;
    try {
        parse("x = 7 + y; y = x * 11; = x * y");
    } catch (const ErroSemantico&) {
        lancou = true;
    }
    checar(lancou,
           "variavel usada antes de sua propria declaracao gera ErroSemantico");
}

static void teste_erro_sintatico_sem_igual_final() {
    bool lancou = false;
    try {
        parse("x = 1;\n");
    } catch (const ErroSintatico&) {
        lancou = true;
    }
    checar(lancou,
           "programa sem '=' antes da expressao final gera ErroSintatico");
}

static void teste_erro_sintatico_declaracao_sem_ponto_virgula() {
    bool lancou = false;
    try {
        parse("x = 1\n= x");
    } catch (const ErroSintatico&) {
        lancou = true;
    }
    checar(lancou, "declaracao sem ';' gera ErroSintatico");
}

int main() {
    teste_programa_sem_declaracoes();
    teste_programa_com_declaracoes();
    teste_variavel_usada_apos_ser_declarada();
    teste_variavel_nao_declarada_na_expressao_final();
    teste_variavel_usada_antes_de_ser_declarada();
    teste_erro_sintatico_sem_igual_final();
    teste_erro_sintatico_declaracao_sem_ponto_virgula();

    std::cout << "\n";
    if (falhas == 0) {
        std::cout << "Todos os testes do parser/semantica passaram.\n";
        return 0;
    }
    std::cout << falhas << " teste(s) falharam.\n";
    return 1;
}
