#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <ostream>

enum class Operador { SOMA, SUB, MULT, DIV };

std::string operador_para_string(Operador op);

class Exp {
public:
    virtual ~Exp() = default;

    virtual long long avaliar() const = 0;

    virtual std::string imprimir() const = 0;

    virtual void imprimir_arvore(std::ostream& os, int nivel = 0) const = 0;
};

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

#endif
