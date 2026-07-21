#include "semantica.h"
#include <unordered_set>

ErroSemantico::ErroSemantico(const std::string& msg)
    : std::runtime_error(msg) {}

// tabela de simbolos: para a linguagem EV so precisamos saber se uma
// variavel foi declarada ou nao, entao um conjunto de nomes basta.
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

    // por fim, a expressao final so pode usar variaveis declaradas em
    // algum ponto do programa.
    verificar_exp(programa.get_exp(), simbolos);
}
