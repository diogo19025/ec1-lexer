#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <ostream>

enum class Operador { SOMA, SUB, MULT, DIV };

// converte operador para o símbolo correspondente
std::string operador_para_string(Operador op);

// classe base abstrata para nós da árvore
class Exp {
public:
    virtual ~Exp() = default;

    // avalia e retorna o valor da expressão
    virtual long long avaliar() const = 0;

    // retorna a expressão como string no formato da gramática
    virtual std::string imprimir() const = 0;

    // imprime a árvore com indentação visual
    virtual void imprimir_arvore(std::ostream& os, int nivel = 0) const = 0;
};

// nó folha: constante inteira
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

// nó folha: uso de variável pelo nome
class Var : public Exp {
private:
    std::string nome;
public:
    explicit Var(std::string nome);
    const std::string& get_nome() const;

    // a avaliação de variáveis depende de um ambiente de valores,
    // que será introduzido em etapa posterior; por ora lança erro
    long long avaliar() const override;
    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// nó interno: operação binária (esq OP dir)
class OpBin : public Exp {
private:
    Operador op;
    std::unique_ptr<Exp> esq;
    std::unique_ptr<Exp> dir;
public:
    OpBin(Operador op, std::unique_ptr<Exp> esq, std::unique_ptr<Exp> dir);

    // getters usados pelo gerador de código
    Operador    get_op()  const;
    const Exp&  get_esq() const;
    const Exp&  get_dir() const;

    long long avaliar() const override;
    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

#endif
