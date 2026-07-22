#include "ast.h"
#include <stdexcept>

std::string operador_para_string(Operador op) {
    switch (op) {
        case Operador::SOMA: return "+";
        case Operador::SUB:  return "-";
        case Operador::MULT: return "*";
        case Operador::DIV:  return "/";
        case Operador::MENOR:     return "<";
        case Operador::MAIOR:     return ">";
        case Operador::IGUALDADE: return "==";
        default:             return "?";
    }
}

// Const

Const::Const(long long valor) : valor(valor) {}

long long Const::get_valor() const { return valor; }

long long Const::avaliar() const { return valor; }

std::string Const::imprimir() const { return std::to_string(valor); }

void Const::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << valor << "\n";
}

// Var

Var::Var(std::string nome) : nome(std::move(nome)) {}

const std::string& Var::get_nome() const { return nome; }

long long Var::avaliar() const {
    throw std::runtime_error("variavel '" + nome
        + "' nao pode ser avaliada sem um ambiente de valores");
}

std::string Var::imprimir() const { return nome; }

void Var::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << nome << "\n";
}

// OpBin

OpBin::OpBin(Operador op, std::unique_ptr<Exp> esq, std::unique_ptr<Exp> dir)
    : op(op), esq(std::move(esq)), dir(std::move(dir)) {}

Operador   OpBin::get_op()  const { return op; }
const Exp& OpBin::get_esq() const { return *esq; }
const Exp& OpBin::get_dir() const { return *dir; }

long long OpBin::avaliar() const {
    long long a = esq->avaliar();
    long long b = dir->avaliar();
    switch (op) {
        case Operador::SOMA: return a + b;
        case Operador::SUB:  return a - b;
        case Operador::MULT: return a * b;
        case Operador::DIV:
            if (b == 0)
                throw std::runtime_error("divisao por zero durante a interpretacao");
            return a / b;
        // comparacoes nao tem tipo booleano na linguagem: o resultado e o
        // inteiro 1 (verdadeiro) ou 0 (falso)
        case Operador::MENOR:     return (a < b)  ? 1 : 0;
        case Operador::MAIOR:     return (a > b)  ? 1 : 0;
        case Operador::IGUALDADE: return (a == b) ? 1 : 0;
    }
    throw std::runtime_error("operador desconhecido");
}

std::string OpBin::imprimir() const {
    return "(" + esq->imprimir() + " " + operador_para_string(op)
         + " " + dir->imprimir() + ")";
}

void OpBin::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ')
       << operador_para_string(op) << "\n";
    esq->imprimir_arvore(os, nivel + 1);
    dir->imprimir_arvore(os, nivel + 1);
}

// Decl

Decl::Decl(std::string nome, std::unique_ptr<Exp> valor)
    : nome(std::move(nome)), valor(std::move(valor)) {}

const std::string& Decl::get_nome()  const { return nome; }
const Exp&         Decl::get_valor() const { return *valor; }

std::string Decl::imprimir() const {
    return nome + " = " + valor->imprimir() + ";";
}

void Decl::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "=\n";
    os << std::string(static_cast<std::size_t>(nivel + 1) * 2, ' ') << nome << "\n";
    valor->imprimir_arvore(os, nivel + 1);
}

// Atribuicao

Atribuicao::Atribuicao(std::string nome, std::unique_ptr<Exp> valor)
    : nome(std::move(nome)), valor(std::move(valor)) {}

const std::string& Atribuicao::get_nome()  const { return nome; }
const Exp&         Atribuicao::get_valor() const { return *valor; }

std::string Atribuicao::imprimir() const {
    return nome + " = " + valor->imprimir() + ";";
}

void Atribuicao::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "=\n";
    os << std::string(static_cast<std::size_t>(nivel + 1) * 2, ' ') << nome << "\n";
    valor->imprimir_arvore(os, nivel + 1);
}

// Retorno

Retorno::Retorno(std::unique_ptr<Exp> valor) : valor(std::move(valor)) {}

const Exp& Retorno::get_valor() const { return *valor; }

std::string Retorno::imprimir() const {
    return "return " + valor->imprimir() + ";";
}

void Retorno::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "return\n";
    valor->imprimir_arvore(os, nivel + 1);
}

// Bloco

Bloco::Bloco(std::vector<std::unique_ptr<Cmd>> comandos)
    : comandos(std::move(comandos)) {}

const std::vector<std::unique_ptr<Cmd>>& Bloco::get_comandos() const {
    return comandos;
}

std::string Bloco::imprimir() const {
    std::string saida = "{";
    for (const auto& cmd : comandos)
        saida += " " + cmd->imprimir();
    saida += " }";
    return saida;
}

void Bloco::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "bloco\n";
    for (const auto& cmd : comandos)
        cmd->imprimir_arvore(os, nivel + 1);
}

// If

If::If(std::unique_ptr<Exp> condicao,
       std::unique_ptr<Bloco> entao,
       std::unique_ptr<Bloco> senao)
    : condicao(std::move(condicao)),
      entao(std::move(entao)),
      senao(std::move(senao)) {}

const Exp&   If::get_condicao() const { return *condicao; }
const Bloco& If::get_entao()    const { return *entao; }
bool         If::tem_senao()    const { return senao != nullptr; }
const Bloco& If::get_senao()    const { return *senao; }

std::string If::imprimir() const {
    std::string saida = "if (" + condicao->imprimir() + ") " + entao->imprimir();
    if (senao)
        saida += " else " + senao->imprimir();
    return saida;
}

void If::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "if\n";
    condicao->imprimir_arvore(os, nivel + 1);
    os << std::string(static_cast<std::size_t>(nivel + 1) * 2, ' ') << "entao\n";
    entao->imprimir_arvore(os, nivel + 2);
    if (senao) {
        os << std::string(static_cast<std::size_t>(nivel + 1) * 2, ' ') << "senao\n";
        senao->imprimir_arvore(os, nivel + 2);
    }
}

// While

While::While(std::unique_ptr<Exp> condicao, std::unique_ptr<Bloco> corpo)
    : condicao(std::move(condicao)), corpo(std::move(corpo)) {}

const Exp&   While::get_condicao() const { return *condicao; }
const Bloco& While::get_corpo()    const { return *corpo; }

std::string While::imprimir() const {
    return "while (" + condicao->imprimir() + ") " + corpo->imprimir();
}

void While::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "while\n";
    condicao->imprimir_arvore(os, nivel + 1);
    corpo->imprimir_arvore(os, nivel + 1);
}

// Programa

Programa::Programa(std::vector<Decl> decls, std::unique_ptr<Exp> exp)
    : decls(std::move(decls)), exp(std::move(exp)) {}

const std::vector<Decl>& Programa::get_decls() const { return decls; }
const Exp&               Programa::get_exp()   const { return *exp; }

std::string Programa::imprimir() const {
    std::string saida;
    for (const Decl& d : decls)
        saida += d.imprimir() + "\n";
    saida += exp->imprimir();
    return saida;
}

void Programa::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "programa\n";
    for (const Decl& d : decls)
        d.imprimir_arvore(os, nivel + 1);
    exp->imprimir_arvore(os, nivel + 1);
}
