#include "ast.h"
#include <stdexcept>

std::string operador_para_string(Operador op) {
    switch (op) {
        case Operador::SOMA: return "+";
        case Operador::SUB:  return "-";
        case Operador::MULT: return "*";
        case Operador::DIV:  return "/";
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
