// Testes do parser da linguagem Cmd (Atividade 09 - parte 2)
// Cobre: nos da AST de comandos (Atribuicao, Retorno, Bloco, If, While),
// corpo entre chaves, return e a precedencia dos operadores <, > e ==.
// Os testes semanticos (variaveis nao declaradas dentro de comandos) sao
// responsabilidade da parte 3.

#include "lexer.h"
#include "parser.h"
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

// tokeniza e faz o parse completo de um programa
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

// parse que deve falhar com ErroSintatico
static bool rejeita(const std::string& fonte) {
    try {
        parse(fonte);
    } catch (const ErroSintatico&) {
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------
// Precedencia dos operadores <, > e ==
// -----------------------------------------------------------------------

static void teste_precedencia_relacionais() {
    checar(parse("= 1 + 2 < 3 * 4")->get_exp().imprimir() == "((1 + 2) < (3 * 4))",
           "'<' agrupa depois de '+' e '*'");
    checar(parse("= 5 - 1 > 2")->get_exp().imprimir() == "((5 - 1) > 2)",
           "'>' agrupa depois de '-'");
    checar(parse("= 2 * 3 == 6")->get_exp().imprimir() == "((2 * 3) == 6)",
           "'==' agrupa depois de '*'");
    checar(parse("= 1 < 2 == 3 > 4")->get_exp().imprimir() == "(((1 < 2) == 3) > 4)",
           "relacionais sao associativos a esquerda");
    checar(parse("= (1 < 2) * 3")->get_exp().imprimir() == "((1 < 2) * 3)",
           "parenteses sobrepoem a precedencia da comparacao");
    checar(parse("= 1 < 2")->get_exp().avaliar() == 1,
           "comparacao verdadeira avalia para 1");
    checar(parse("= 3 == 4")->get_exp().avaliar() == 0,
           "comparacao falsa avalia para 0");
}

// -----------------------------------------------------------------------
// Comandos que devem ser aceitos
// -----------------------------------------------------------------------

static void teste_programa_com_corpo() {
    auto programa = parse("x = 1; { return x + 1; }");
    checar(programa->tem_corpo(), "programa com corpo entre chaves eh aceito");
    checar(programa->get_decls().size() == 1,
           "declaracoes antes do corpo sao reconhecidas");
    checar(programa->get_corpo().get_comandos().size() == 1,
           "corpo tem exatamente 1 comando");
    checar(programa->get_corpo().imprimir() == "{ return (x + 1); }",
           "comando return eh reconhecido dentro do corpo");
}

static void teste_forma_antiga_continua_valida() {
    auto programa = parse("x = 2; = x * 3");
    checar(!programa->tem_corpo(),
           "programa da forma antiga (expressao final) continua valido");
    checar(programa->get_exp().imprimir() == "(x * 3)",
           "expressao final da forma antiga eh construida normalmente");
}

static void teste_atribuicao() {
    auto programa = parse("x = 1; { x = x + 1; }");
    const auto& cmds = programa->get_corpo().get_comandos();
    const auto* atrib = dynamic_cast<const Atribuicao*>(cmds[0].get());
    checar(atrib != nullptr, "atribuicao vira um no Atribuicao");
    checar(atrib && atrib->get_nome() == "x",
           "atribuicao guarda o nome da variavel");
    checar(atrib && atrib->get_valor().imprimir() == "(x + 1)",
           "atribuicao guarda a expressao do lado direito");
}

static void teste_if_sem_else() {
    auto programa = parse("{ if (1 < 2) { return 1; } }");
    const auto* no_if = dynamic_cast<const If*>(
        programa->get_corpo().get_comandos()[0].get());
    checar(no_if != nullptr, "if vira um no If");
    checar(no_if && !no_if->tem_senao(), "if sem else nao tem ramo senao");
    checar(no_if && no_if->get_condicao().imprimir() == "(1 < 2)",
           "condicao do if eh reconhecida");
}

static void teste_if_com_else() {
    auto programa = parse("{ if (1 == 2) { return 1; } else { return 2; } }");
    const auto* no_if = dynamic_cast<const If*>(
        programa->get_corpo().get_comandos()[0].get());
    checar(no_if != nullptr && no_if->tem_senao(),
           "if com else tem os dois ramos");
    checar(no_if && no_if->imprimir() ==
               "if ((1 == 2)) { return 1; } else { return 2; }",
           "if/else eh impresso no formato da gramatica");
}

static void teste_while() {
    auto programa = parse("i = 0; { while (i < 10) { i = i + 1; } return i; }");
    const auto& cmds = programa->get_corpo().get_comandos();
    const auto* no_while = dynamic_cast<const While*>(cmds[0].get());
    checar(no_while != nullptr, "while vira um no While");
    checar(no_while && no_while->get_condicao().imprimir() == "(i < 10)",
           "condicao do while eh reconhecida");
    checar(no_while && no_while->get_corpo().get_comandos().size() == 1,
           "corpo do while tem os comandos reconhecidos");
}

static void teste_bloco_aninhado() {
    auto programa = parse("{ { x = 1; } return 0; }");
    const auto* interno = dynamic_cast<const Bloco*>(
        programa->get_corpo().get_comandos()[0].get());
    checar(interno != nullptr, "bloco aninhado vira um no Bloco");
    checar(programa->get_corpo().get_comandos().size() == 2,
           "bloco aninhado e return sao dois comandos do corpo");
}

// -----------------------------------------------------------------------
// Comandos que devem ser rejeitados
// -----------------------------------------------------------------------

static void teste_erros_sintaticos() {
    checar(rejeita("{ return 1;"),
           "corpo sem '}' de fechamento gera ErroSintatico");
    checar(rejeita("{ if 1 < 2 { return 1; } }"),
           "if sem parenteses na condicao gera ErroSintatico");
    checar(rejeita("{ if (1 < 2) return 1; }"),
           "corpo do if sem chaves gera ErroSintatico");
    checar(rejeita("{ while (1) x = 1; }"),
           "corpo do while sem chaves gera ErroSintatico");
    checar(rejeita("{ return 1 }"),
           "return sem ';' gera ErroSintatico");
    checar(rejeita("{ else { return 1; } }"),
           "else sem if gera ErroSintatico");
    checar(rejeita("{ x = ; }"),
           "atribuicao sem expressao gera ErroSintatico");
}

int main() {
    teste_precedencia_relacionais();
    teste_programa_com_corpo();
    teste_forma_antiga_continua_valida();
    teste_atribuicao();
    teste_if_sem_else();
    teste_if_com_else();
    teste_while();
    teste_bloco_aninhado();
    teste_erros_sintaticos();

    std::cout << "\n";
    if (falhas == 0) {
        std::cout << "Todos os testes do parser da linguagem Cmd passaram.\n";
        return 0;
    }
    std::cout << falhas << " teste(s) falharam.\n";
    return 1;
}
