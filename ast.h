#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <ostream>

// Operadores binarios da linguagem EC1
enum class Operador { SOMA, SUB, MULT, DIV };

// Converte o operador para o simbolo correspondente ("+", "-", "*", "/")
std::string operador_para_string(Operador op);

// Classe-base abstrata para os nos da arvore de sintaxe abstrata.
// Nenhum objeto e criado diretamente com esta classe (ver enunciado, secao 5).
class Exp {
public:
    virtual ~Exp() = default;

    // Interpretacao por varredura da arvore: devolve o valor da expressao.
    virtual long long avaliar() const = 0;

    // Impressao "linear": reconstroi a expressao no formato (esq op dir).
    virtual std::string imprimir() const = 0;

    // Impressao "visual": arvore indentada (util para testes/depuracao).
    virtual void imprimir_arvore(std::ostream& os, int nivel = 0) const = 0;
};

// No de constante inteira: precisa apenas do valor (enunciado, secao 5).
class Const : public Exp {
private:
    long long valor;
public:
    explicit Const(long long valor);
    long long get_valor() const;

    long long avaliar() const override;
    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// No de operacao binaria: operador + operandos esquerdo e direito (que sao Exp).
class OpBin : public Exp {
private:
    Operador op;
    std::unique_ptr<Exp> esq;
    std::unique_ptr<Exp> dir;
public:
    OpBin(Operador op, std::unique_ptr<Exp> esq, std::unique_ptr<Exp> dir);

    long long avaliar() const override;
    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

#endif // AST_H
