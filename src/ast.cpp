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

// AcessoArray

AcessoArray::AcessoArray(std::string nome, std::unique_ptr<Exp> indice)
    : nome(std::move(nome)), indice(std::move(indice)) {}

const std::string& AcessoArray::get_nome() const { return nome; }
const Exp& AcessoArray::get_indice() const { return *indice; }

long long AcessoArray::avaliar() const {
    throw std::runtime_error(
        "array '" + nome + "' nao pode ser avaliado sem um ambiente de valores");
}

std::string AcessoArray::imprimir() const {
    return nome + "[" + indice->imprimir() + "]";
}

void AcessoArray::imprimir_arvore(std::ostream& os, int nivel) const {
    const std::string indent(static_cast<std::size_t>(nivel) * 2, ' ');
    os << indent << "acesso-array " << nome << "\n";
    indice->imprimir_arvore(os, nivel + 1);
}

// ChamadaFuncao

ChamadaFuncao::ChamadaFuncao(
    std::string nome,
    std::vector<std::unique_ptr<Exp>> argumentos)
    : nome(std::move(nome)), argumentos(std::move(argumentos)) {}

const std::string& ChamadaFuncao::get_nome() const { return nome; }

const std::vector<std::unique_ptr<Exp>>&
ChamadaFuncao::get_argumentos() const {
    return argumentos;
}

long long ChamadaFuncao::avaliar() const {
    throw std::runtime_error(
        "chamada da funcao '" + nome +
        "' nao pode ser avaliada sem um ambiente de funcoes");
}

std::string ChamadaFuncao::imprimir() const {
    std::string saida = nome + "(";
    for (std::size_t i = 0; i < argumentos.size(); ++i) {
        if (i != 0)
            saida += ", ";
        saida += argumentos[i]->imprimir();
    }
    return saida + ")";
}

void ChamadaFuncao::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ')
       << "chamada " << nome << "\n";
    for (const auto& argumento : argumentos)
        argumento->imprimir_arvore(os, nivel + 1);
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

Decl::Decl(std::string nome, std::unique_ptr<Exp> valor,
           bool usa_palavra_var)
    : nome(std::move(nome)),
      valor(std::move(valor)),
      usa_palavra_var(usa_palavra_var),
      array(false),
      tamanho_array(0) {}

Decl::Decl(std::string nome, std::size_t tamanho_array)
    : nome(std::move(nome)),
      valor(nullptr),
      usa_palavra_var(true),
      array(true),
      tamanho_array(tamanho_array) {}

const std::string& Decl::get_nome()  const { return nome; }
const Exp&         Decl::get_valor() const { return *valor; }
bool               Decl::usa_var()   const { return usa_palavra_var; }
bool               Decl::eh_array()  const { return array; }
std::size_t        Decl::get_tamanho_array() const { return tamanho_array; }

std::string Decl::imprimir() const {
    if (array)
        return "var " + nome + "[" + std::to_string(tamanho_array) + "];";
    return std::string(usa_palavra_var ? "var " : "") +
           nome + " = " + valor->imprimir() + ";";
}

void Decl::imprimir_arvore(std::ostream& os, int nivel) const {
    if (array) {
        os << std::string(static_cast<std::size_t>(nivel) * 2, ' ')
           << "array " << nome << "[" << tamanho_array << "]\n";
        return;
    }
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

// AtribuicaoArray

AtribuicaoArray::AtribuicaoArray(std::string nome,
                                 std::unique_ptr<Exp> indice,
                                 std::unique_ptr<Exp> valor)
    : nome(std::move(nome)),
      indice(std::move(indice)),
      valor(std::move(valor)) {}

const std::string& AtribuicaoArray::get_nome() const { return nome; }
const Exp& AtribuicaoArray::get_indice() const { return *indice; }
const Exp& AtribuicaoArray::get_valor() const { return *valor; }

std::string AtribuicaoArray::imprimir() const {
    return nome + "[" + indice->imprimir() + "] = " + valor->imprimir() + ";";
}

void AtribuicaoArray::imprimir_arvore(std::ostream& os, int nivel) const {
    const std::string indent(static_cast<std::size_t>(nivel) * 2, ' ');
    os << indent << "atrib-array " << nome << "\n";
    os << indent << "  indice\n";
    indice->imprimir_arvore(os, nivel + 2);
    os << indent << "  valor\n";
    valor->imprimir_arvore(os, nivel + 2);
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

// Funcao

Funcao::Funcao(std::string nome,
               std::vector<std::string> parametros,
               std::vector<Decl> variaveis_locais,
               std::unique_ptr<Bloco> corpo)
    : nome(std::move(nome)),
      parametros(std::move(parametros)),
      variaveis_locais(std::move(variaveis_locais)),
      corpo(std::move(corpo)) {}

const std::string& Funcao::get_nome() const { return nome; }

const std::vector<std::string>& Funcao::get_parametros() const {
    return parametros;
}

const std::vector<Decl>& Funcao::get_variaveis_locais() const {
    return variaveis_locais;
}

const Bloco& Funcao::get_corpo() const { return *corpo; }

std::string Funcao::imprimir() const {
    std::string saida = "fun " + nome + "(";
    for (std::size_t i = 0; i < parametros.size(); ++i) {
        if (i != 0)
            saida += ", ";
        saida += parametros[i];
    }
    saida += ") {";
    for (const Decl& variavel : variaveis_locais)
        saida += " " + variavel.imprimir();
    for (const auto& comando : corpo->get_comandos())
        saida += " " + comando->imprimir();
    return saida + " }";
}

void Funcao::imprimir_arvore(std::ostream& os, int nivel) const {
    const std::string indent(static_cast<std::size_t>(nivel) * 2, ' ');
    os << indent << "funcao " << nome << "\n";
    os << indent << "  parametros\n";
    for (const std::string& parametro : parametros)
        os << indent << "    " << parametro << "\n";
    os << indent << "  variaveis locais\n";
    for (const Decl& variavel : variaveis_locais)
        variavel.imprimir_arvore(os, nivel + 2);
    corpo->imprimir_arvore(os, nivel + 1);
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
    : decls(std::move(decls)),
      exp(std::move(exp)),
      corpo(nullptr),
      forma_fun(false) {
    for (std::size_t i = 0; i < this->decls.size(); ++i)
        ordem_declaracoes.push_back({TipoDeclaracaoTopo::VARIAVEL, i});
}

Programa::Programa(std::vector<Decl> decls, std::unique_ptr<Bloco> corpo)
    : decls(std::move(decls)),
      exp(nullptr),
      corpo(std::move(corpo)),
      forma_fun(false) {
    for (std::size_t i = 0; i < this->decls.size(); ++i)
        ordem_declaracoes.push_back({TipoDeclaracaoTopo::VARIAVEL, i});
}

Programa::Programa(std::vector<Decl> decls,
                   std::vector<Funcao> funcoes,
                   std::vector<DeclaracaoTopo> ordem_declaracoes,
                   std::vector<Decl> locais_main,
                   std::unique_ptr<Bloco> corpo)
    : decls(std::move(decls)),
      funcoes(std::move(funcoes)),
      ordem_declaracoes(std::move(ordem_declaracoes)),
      locais_main(std::move(locais_main)),
      exp(nullptr),
      corpo(std::move(corpo)),
      forma_fun(true) {}

const std::vector<Decl>& Programa::get_decls() const { return decls; }
const std::vector<Funcao>& Programa::get_funcoes() const { return funcoes; }
const std::vector<DeclaracaoTopo>&
Programa::get_ordem_declaracoes() const {
    return ordem_declaracoes;
}
const std::vector<Decl>& Programa::get_locais_main() const {
    return locais_main;
}
const Exp&               Programa::get_exp()   const { return *exp; }
bool                     Programa::tem_corpo() const { return corpo != nullptr; }
const Bloco&             Programa::get_corpo() const { return *corpo; }
bool                     Programa::eh_fun()    const { return forma_fun; }

std::string Programa::imprimir() const {
    std::string saida;
    if (forma_fun) {
        for (const DeclaracaoTopo& declaracao : ordem_declaracoes) {
            if (declaracao.tipo == TipoDeclaracaoTopo::VARIAVEL)
                saida += decls[declaracao.indice].imprimir() + "\n";
            else
                saida += funcoes[declaracao.indice].imprimir() + "\n";
        }
        // o main imprime as proprias locais antes dos comandos, do mesmo
        // jeito que Funcao::imprimir faz com as locais da funcao
        saida += "main {";
        for (const Decl& variavel : locais_main)
            saida += " " + variavel.imprimir();
        for (const auto& comando : corpo->get_comandos())
            saida += " " + comando->imprimir();
        saida += " }";
        return saida;
    }

    for (const Decl& declaracao : decls)
        saida += declaracao.imprimir() + "\n";
    saida += corpo ? corpo->imprimir() : exp->imprimir();
    return saida;
}

void Programa::imprimir_arvore(std::ostream& os, int nivel) const {
    os << std::string(static_cast<std::size_t>(nivel) * 2, ' ') << "programa\n";
    if (forma_fun) {
        for (const DeclaracaoTopo& declaracao : ordem_declaracoes) {
            if (declaracao.tipo == TipoDeclaracaoTopo::VARIAVEL)
                decls[declaracao.indice].imprimir_arvore(os, nivel + 1);
            else
                funcoes[declaracao.indice].imprimir_arvore(os, nivel + 1);
        }
        os << std::string(static_cast<std::size_t>(nivel + 1) * 2, ' ')
           << "main\n";
        // so imprime o cabecalho quando o main tem locais, para nao poluir
        // a arvore dos programas que nao declaram nenhuma
        if (!locais_main.empty()) {
            os << std::string(static_cast<std::size_t>(nivel + 2) * 2, ' ')
               << "variaveis locais\n";
            for (const Decl& variavel : locais_main)
                variavel.imprimir_arvore(os, nivel + 3);
        }
        corpo->imprimir_arvore(os, nivel + 2);
        return;
    }

    for (const Decl& declaracao : decls)
        declaracao.imprimir_arvore(os, nivel + 1);
    if (corpo)
        corpo->imprimir_arvore(os, nivel + 1);
    else
        exp->imprimir_arvore(os, nivel + 1);
}
