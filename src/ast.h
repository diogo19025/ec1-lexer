#ifndef AST_H
#define AST_H

#include <string>
#include <memory>
#include <ostream>
#include <vector>

enum class Operador { SOMA, SUB, MULT, DIV, MENOR, MAIOR, IGUALDADE };

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

// declaração de variável: <ident> '=' <exp> ';'
// não é uma expressão (não tem valor), por isso não deriva de Exp
class Decl {
private:
    std::string nome;
    std::unique_ptr<Exp> valor;
public:
    Decl(std::string nome, std::unique_ptr<Exp> valor);

    const std::string& get_nome()  const;
    const Exp&         get_valor() const;

    // declaração como string no formato da gramática
    std::string imprimir() const;

    // imprime a subárvore da declaração com indentação visual
    void imprimir_arvore(std::ostream& os, int nivel = 0) const;
};

// ---------------------------------------------------------------------------
// Comandos (Atividade 09, linguagem Cmd)
// ---------------------------------------------------------------------------

// classe base abstrata para nós de comando: diferentemente de Exp, um
// comando não tem valor — ele produz um efeito (atribuir, desviar, retornar)
class Cmd {
public:
    virtual ~Cmd() = default;

    // comando como string no formato da gramática
    virtual std::string imprimir() const = 0;

    // imprime a subárvore do comando com indentação visual
    virtual void imprimir_arvore(std::ostream& os, int nivel = 0) const = 0;
};

// atribuição: <ident> '=' <exp> ';'
// sintaticamente igual à declaração, mas aparece dentro do corpo do
// programa; distinguir atribuição de declaração é papel da análise semântica
class Atribuicao : public Cmd {
private:
    std::string nome;
    std::unique_ptr<Exp> valor;
public:
    Atribuicao(std::string nome, std::unique_ptr<Exp> valor);

    const std::string& get_nome()  const;
    const Exp&         get_valor() const;

    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// retorno: 'return' <exp> ';' — encerra o programa com o valor da expressão
class Retorno : public Cmd {
private:
    std::unique_ptr<Exp> valor;
public:
    explicit Retorno(std::unique_ptr<Exp> valor);

    const Exp& get_valor() const;

    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// bloco de comandos: '{' <cmd>* '}'
class Bloco : public Cmd {
private:
    std::vector<std::unique_ptr<Cmd>> comandos;
public:
    explicit Bloco(std::vector<std::unique_ptr<Cmd>> comandos);

    const std::vector<std::unique_ptr<Cmd>>& get_comandos() const;

    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// condicional: 'if' '(' <exp> ')' <bloco> ('else' <bloco>)?
// o ramo else é opcional (nulo quando ausente)
class If : public Cmd {
private:
    std::unique_ptr<Exp>   condicao;
    std::unique_ptr<Bloco> entao;
    std::unique_ptr<Bloco> senao;   // pode ser nulo
public:
    If(std::unique_ptr<Exp> condicao,
       std::unique_ptr<Bloco> entao,
       std::unique_ptr<Bloco> senao);

    const Exp&   get_condicao() const;
    const Bloco& get_entao()    const;
    bool         tem_senao()    const;
    const Bloco& get_senao()    const;   // só pode ser chamado se tem_senao()

    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// repetição: 'while' '(' <exp> ')' <bloco>
class While : public Cmd {
private:
    std::unique_ptr<Exp>   condicao;
    std::unique_ptr<Bloco> corpo;
public:
    While(std::unique_ptr<Exp> condicao, std::unique_ptr<Bloco> corpo);

    const Exp&   get_condicao() const;
    const Bloco& get_corpo()    const;

    std::string imprimir() const override;
    void imprimir_arvore(std::ostream& os, int nivel) const override;
};

// programa: zero ou mais declarações seguidas da expressão final
class Programa {
private:
    std::vector<Decl> decls;
    std::unique_ptr<Exp> exp;
public:
    Programa(std::vector<Decl> decls, std::unique_ptr<Exp> exp);

    const std::vector<Decl>& get_decls() const;
    const Exp&               get_exp()   const;

    // programa como string, uma declaração por linha e a expressão ao final
    std::string imprimir() const;

    // imprime a árvore completa do programa com indentação visual
    void imprimir_arvore(std::ostream& os, int nivel = 0) const;
};

#endif
