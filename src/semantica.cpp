#include "semantica.h"
#include <string>
#include <unordered_map>

ErroSemantico::ErroSemantico(const std::string& msg)
    : std::runtime_error(msg) {}

// ---------------------------------------------------------------------------
// Simbolos e tabelas de simbolos (Atividade 10, linguagem Fun)
// ---------------------------------------------------------------------------

// Cada nome declarado vira um simbolo com um tipo. Ate a Atividade 09 bastava
// saber se um nome existia (um conjunto de nomes), porque tudo era variavel
// global; com funcoes, parametros e variaveis locais o mesmo nome pode
// significar coisas diferentes dependendo de onde aparece, e o tipo do
// simbolo e o que permite recusar usos incoerentes (chamar uma variavel,
// usar uma funcao como valor).
enum class TipoSimbolo {
    VARIAVEL_GLOBAL,
    FUNCAO,
    PARAMETRO,
    VARIAVEL_LOCAL
};

struct Simbolo {
    TipoSimbolo tipo;
    // quantidade de parametros formais; so faz sentido quando tipo == FUNCAO,
    // e e o que permite conferir a aridade de cada chamada
    std::size_t qtd_parametros;
};

using TabelaSimbolos = std::unordered_map<std::string, Simbolo>;

// Contexto de resolucao de nomes: a tabela global (variaveis globais e
// funcoes) e, quando estamos dentro de uma funcao ou do main, a tabela local
// daquele corpo (parametros e variaveis locais). 'locais' e nulo enquanto
// analisamos as declaracoes globais, onde nao existe escopo local.
struct Escopo {
    const TabelaSimbolos* globais;
    const TabelaSimbolos* locais;
};

// Procura um nome primeiro no escopo local e depois no global; devolve nulo
// se ele nao foi declarado em nenhum dos dois. E essa ordem que faz um
// parametro ou variavel local esconder uma global de mesmo nome: como o
// escopo local e consultado primeiro, a global nunca chega a ser alcancada.
static const Simbolo* resolver(const Escopo& escopo, const std::string& nome) {
    if (escopo.locais != nullptr) {
        auto local = escopo.locais->find(nome);
        if (local != escopo.locais->end())
            return &local->second;
    }

    auto global = escopo.globais->find(nome);
    if (global != escopo.globais->end())
        return &global->second;

    return nullptr;
}

// ---------------------------------------------------------------------------
// Verificacao das expressoes
// ---------------------------------------------------------------------------

// percorre recursivamente uma expressao verificando se todo nome usado nela
// ja foi declarado e se esta sendo usado de acordo com o seu tipo de simbolo.
static void verificar_exp(const Exp& exp, const Escopo& escopo) {
    if (const auto* var = dynamic_cast<const Var*>(&exp)) {
        const Simbolo* simbolo = resolver(escopo, var->get_nome());
        if (simbolo == nullptr) {
            throw ErroSemantico(
                "variavel '" + var->get_nome() +
                "' usada antes de ser declarada");
        }
        // o nome existe, mas e o de uma funcao: uma funcao so pode aparecer
        // numa chamada, nunca como valor de uma expressao
        if (simbolo->tipo == TipoSimbolo::FUNCAO) {
            throw ErroSemantico(
                "'" + var->get_nome() +
                "' e uma funcao, nao uma variavel");
        }
        return;
    }

    if (const auto* chamada = dynamic_cast<const ChamadaFuncao*>(&exp)) {
        const std::string& nome = chamada->get_nome();
        const Simbolo* simbolo = resolver(escopo, nome);

        if (simbolo == nullptr)
            throw ErroSemantico("funcao '" + nome + "' nao foi declarada");

        // o nome existe, mas nao e de uma funcao (e uma variavel global, um
        // parametro ou uma variavel local): nao pode ser chamado
        if (simbolo->tipo != TipoSimbolo::FUNCAO)
            throw ErroSemantico("'" + nome + "' nao e uma funcao");

        // a chamada precisa passar exatamente um argumento por parametro
        // formal declarado na funcao
        if (chamada->get_argumentos().size() != simbolo->qtd_parametros) {
            throw ErroSemantico(
                "funcao '" + nome + "' espera " +
                std::to_string(simbolo->qtd_parametros) +
                " argumento(s), mas recebeu " +
                std::to_string(chamada->get_argumentos().size()));
        }

        // cada argumento e uma expressao qualquer, verificada no mesmo escopo
        // de quem faz a chamada
        for (const auto& argumento : chamada->get_argumentos())
            verificar_exp(*argumento, escopo);
        return;
    }

    if (const auto* op = dynamic_cast<const OpBin*>(&exp)) {
        verificar_exp(op->get_esq(), escopo);
        verificar_exp(op->get_dir(), escopo);
        return;
    }

    // Const e um valor literal, nao referencia nenhum nome
}

// ---------------------------------------------------------------------------
// Verificacao dos comandos
// ---------------------------------------------------------------------------

// declaracao antecipada: um Bloco contem uma lista de comandos, e um
// comando (If, While) pode conter blocos, entao as duas funcoes a seguir
// sao mutuamente recursivas.
static void verificar_bloco(const Bloco& bloco, const Escopo& escopo);

// percorre um comando verificando os nomes usados nele. O escopo e o mesmo
// para os dois ramos de um 'if' e para o corpo de um 'while': nenhum comando
// declara nomes novos (as declaracoes ficam no topo do programa, e as locais
// no topo do corpo da funcao/main), entao nao ha necessidade de copiar as
// tabelas para cada ramo.
static void verificar_cmd(const Cmd& cmd, const Escopo& escopo) {
    if (const auto* atrib = dynamic_cast<const Atribuicao*>(&cmd)) {
        // a atribuicao so e valida se a variavel ja foi declarada antes;
        // atribuir a uma variavel nao declarada e um erro semantico (nem a
        // linguagem Cmd nem a Fun permitem declarar variaveis dentro do
        // corpo, so no topo)
        const Simbolo* simbolo = resolver(escopo, atrib->get_nome());
        if (simbolo == nullptr) {
            throw ErroSemantico(
                "atribuicao a variavel '" + atrib->get_nome() +
                "' que nao foi declarada");
        }
        if (simbolo->tipo == TipoSimbolo::FUNCAO) {
            throw ErroSemantico(
                "'" + atrib->get_nome() +
                "' e uma funcao, nao pode receber atribuicao");
        }
        verificar_exp(atrib->get_valor(), escopo);
        return;
    }

    if (const auto* ret = dynamic_cast<const Retorno*>(&cmd)) {
        verificar_exp(ret->get_valor(), escopo);
        return;
    }

    if (const auto* bloco = dynamic_cast<const Bloco*>(&cmd)) {
        verificar_bloco(*bloco, escopo);
        return;
    }

    if (const auto* no_if = dynamic_cast<const If*>(&cmd)) {
        verificar_exp(no_if->get_condicao(), escopo);
        verificar_bloco(no_if->get_entao(), escopo);
        if (no_if->tem_senao())
            verificar_bloco(no_if->get_senao(), escopo);
        return;
    }

    if (const auto* no_while = dynamic_cast<const While*>(&cmd)) {
        verificar_exp(no_while->get_condicao(), escopo);
        verificar_bloco(no_while->get_corpo(), escopo);
        return;
    }
}

static void verificar_bloco(const Bloco& bloco, const Escopo& escopo) {
    for (const auto& cmd : bloco.get_comandos())
        verificar_cmd(*cmd, escopo);
}

// ---------------------------------------------------------------------------
// Verificacao das funcoes
// ---------------------------------------------------------------------------

// Monta a tabela local de um corpo (funcao ou main) e verifica os comandos
// dele. Os parametros entram primeiro; depois, as variaveis locais sao
// processadas na ordem em que foram declaradas, entao a expressao de
// inicializacao de uma local so pode usar os parametros, as locais
// declaradas antes dela e o que ja existe no escopo global.
static void verificar_corpo(const std::vector<std::string>& parametros,
                            const std::vector<Decl>& variaveis_locais,
                            const Bloco& corpo,
                            const TabelaSimbolos& globais) {
    TabelaSimbolos locais;
    for (const std::string& parametro : parametros)
        locais[parametro] = {TipoSimbolo::PARAMETRO, 0};

    // a tabela local e consultada por ponteiro, entao continua crescendo
    // corretamente a cada variavel local registrada abaixo
    Escopo escopo{&globais, &locais};

    for (const Decl& local : variaveis_locais) {
        verificar_exp(local.get_valor(), escopo);
        locais[local.get_nome()] = {TipoSimbolo::VARIAVEL_LOCAL, 0};
    }

    verificar_bloco(corpo, escopo);
}

// ---------------------------------------------------------------------------
// Ponto de entrada
// ---------------------------------------------------------------------------

void verificar_variaveis(const Programa& programa) {
    TabelaSimbolos globais;

    if (programa.eh_fun()) {
        // percorre as declaracoes de topo na ordem em que aparecem no
        // programa: cada uma so enxerga o que foi declarado antes dela.
        for (const DeclaracaoTopo& topo : programa.get_ordem_declaracoes()) {
            if (topo.tipo == TipoDeclaracaoTopo::VARIAVEL) {
                const Decl& decl = programa.get_decls()[topo.indice];
                verificar_exp(decl.get_valor(), Escopo{&globais, nullptr});
                globais[decl.get_nome()] = {TipoSimbolo::VARIAVEL_GLOBAL, 0};
                continue;
            }

            const Funcao& funcao = programa.get_funcoes()[topo.indice];
            // a funcao e registrada na tabela global ANTES de seu corpo ser
            // analisado: e isso que permite a recursao direta, porque quando
            // o corpo de f for percorrido o nome f ja estara declarado.
            globais[funcao.get_nome()] =
                {TipoSimbolo::FUNCAO, funcao.get_parametros().size()};
            verificar_corpo(funcao.get_parametros(),
                            funcao.get_variaveis_locais(),
                            funcao.get_corpo(), globais);
        }
    } else {
        // formas EV e Cmd: as declaracoes de topo sao todas de variavel, e a
        // expressao de cada uma so pode usar variaveis declaradas antes dela;
        // depois de verificada, a propria variavel entra na tabela.
        for (const Decl& decl : programa.get_decls()) {
            verificar_exp(decl.get_valor(), Escopo{&globais, nullptr});
            globais[decl.get_nome()] = {TipoSimbolo::VARIAVEL_GLOBAL, 0};
        }
    }

    // a partir daqui o programa tem uma das tres formas possiveis:
    //  - forma Fun (Atividade 10): bloco main, com locais proprias
    //  - forma Cmd (Atividade 09): corpo de comandos entre chaves
    //  - forma EV  (Atividade 08): expressao final ('=' <exp>)
    if (programa.tem_corpo()) {
        // o main tem um escopo local como o de uma funcao, so que sem
        // parametros; nas formas EV e Cmd a lista de locais e vazia, e a
        // tabela local resultante nao esconde nada.
        verificar_corpo({}, programa.get_locais_main(),
                        programa.get_corpo(), globais);
        return;
    }

    // forma EV: a expressao final so pode usar variaveis declaradas em
    // algum ponto do programa.
    verificar_exp(programa.get_exp(), Escopo{&globais, nullptr});
}
