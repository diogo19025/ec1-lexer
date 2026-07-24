#include "semantica.h"
#include <unordered_set>

ErroSemantico::ErroSemantico(const std::string& msg)
    : std::runtime_error(msg) {}

// tabela de simbolos: basta saber se uma variavel foi declarada ou nao,
// entao um conjunto de nomes basta. As declaracoes (<decl>, no topo do
// programa) sao as unicas que adicionam nomes a tabela; comandos do corpo
// (atribuicao, if, while, return) apenas consultam a tabela ja construida.
using TabelaSimbolos = std::unordered_set<std::string>;

// percorre recursivamente uma expressao verificando se toda variavel (Var)
// usada nela ja esta presente na tabela de simbolos.
static void verificar_exp(const Exp& exp, const TabelaSimbolos& simbolos) {
    if (const auto* var = dynamic_cast<const Var*>(&exp)) {
        if (simbolos.find(var->get_nome()) == simbolos.end()) {
            throw ErroSemantico(
                "variavel '" + var->get_nome() +
                "' usada antes de ser declarada");
        }
        return;
    }

    if (const auto* op = dynamic_cast<const OpBin*>(&exp)) {
        verificar_exp(op->get_esq(), simbolos);
        verificar_exp(op->get_dir(), simbolos);
        return;
    }

    // Const e um valor literal, nao referencia nenhuma variavel
}

// declaracao antecipada: um Bloco contem uma lista de comandos, e um
// comando (If, While) pode conter blocos, entao as duas funcoes a seguir
// sao mutuamente recursivas.
static void verificar_bloco(const Bloco& bloco, const TabelaSimbolos& simbolos);

// percorre um comando verificando as variaveis usadas nele. A tabela de
// simbolos e a mesma para os dois ramos de um 'if' e para o corpo de um
// 'while': como a linguagem Cmd nao permite declarar variaveis dentro do
// corpo do programa (apenas atribuir a variaveis ja declaradas), nenhum
// comando insere nomes novos na tabela, entao nao ha necessidade de copiar
// a tabela para cada ramo.
static void verificar_cmd(const Cmd& cmd, const TabelaSimbolos& simbolos) {
    if (const auto* atrib = dynamic_cast<const Atribuicao*>(&cmd)) {
        // a atribuicao so e valida se a variavel ja foi declarada antes;
        // atribuir a uma variavel nao declarada e um erro semantico (a
        // linguagem Cmd nao permite declarar variaveis dentro do corpo)
        if (simbolos.find(atrib->get_nome()) == simbolos.end()) {
            throw ErroSemantico(
                "atribuicao a variavel '" + atrib->get_nome() +
                "' que nao foi declarada");
        }
        verificar_exp(atrib->get_valor(), simbolos);
        return;
    }

    if (const auto* ret = dynamic_cast<const Retorno*>(&cmd)) {
        verificar_exp(ret->get_valor(), simbolos);
        return;
    }

    if (const auto* bloco = dynamic_cast<const Bloco*>(&cmd)) {
        verificar_bloco(*bloco, simbolos);
        return;
    }

    if (const auto* no_if = dynamic_cast<const If*>(&cmd)) {
        verificar_exp(no_if->get_condicao(), simbolos);
        verificar_bloco(no_if->get_entao(), simbolos);
        if (no_if->tem_senao())
            verificar_bloco(no_if->get_senao(), simbolos);
        return;
    }

    if (const auto* no_while = dynamic_cast<const While*>(&cmd)) {
        verificar_exp(no_while->get_condicao(), simbolos);
        verificar_bloco(no_while->get_corpo(), simbolos);
        return;
    }
}

static void verificar_bloco(const Bloco& bloco, const TabelaSimbolos& simbolos) {
    for (const auto& cmd : bloco.get_comandos())
        verificar_cmd(*cmd, simbolos);
}

void verificar_variaveis(const Programa& programa) {
    TabelaSimbolos simbolos;

    // processa as declaracoes na ordem em que aparecem: a expressao de
    // cada declaracao so pode usar variaveis ja declaradas antes dela;
    // apos verificar a expressao, a propria variavel declarada passa a
    // fazer parte da tabela de simbolos.
    for (const Decl& decl : programa.get_decls()) {
        verificar_exp(decl.get_valor(), simbolos);
        simbolos.insert(decl.get_nome());
    }

    // a partir daqui o programa tem uma das duas formas possiveis:
    //  - forma Cmd (Atividade 09): corpo de comandos entre chaves
    //  - forma EV  (Atividade 08): expressao final ('=' <exp>)
    if (programa.tem_corpo()) {
        verificar_bloco(programa.get_corpo(), simbolos);
        return;
    }

    // forma EV: a expressao final so pode usar variaveis declaradas em
    // algum ponto do programa.
    verificar_exp(programa.get_exp(), simbolos);
}
