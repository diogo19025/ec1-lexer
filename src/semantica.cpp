#include "semantica.h"
#include <limits>
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
    ARRAY_GLOBAL,
    FUNCAO,
    PARAMETRO,
    VARIAVEL_LOCAL,
    ARRAY_LOCAL
};

struct Simbolo {
    TipoSimbolo tipo;
    // quantidade de parametros formais; so faz sentido quando tipo == FUNCAO,
    // e e o que permite conferir a aridade de cada chamada
    std::size_t qtd_parametros = 0;
    // quantidade de elementos; so faz sentido para ARRAY_GLOBAL/ARRAY_LOCAL
    std::size_t tamanho_array = 0;
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

// Nomes que o gerador de código não pode usar porque já existem no assembly
// produzido: 'sair' é uma sub-rotina do runtime (runtime.s) e viraria um
// rótulo duplicado, e 'PROGRAMA' colidiria com FIM_PROGRAMA, o rótulo de
// retorno do bloco main. Os demais nomes internos ('_start', 'imprime_num',
// 'FIM_IF_0', ...) têm '_', que o lexer nunca aceita dentro de um
// identificador, então não há como um programa da linguagem produzi-los.
static bool nome_reservado(const std::string& nome) {
    return nome == "sair" || nome == "PROGRAMA";
}

static void verificar_nome_disponivel(const std::string& nome) {
    if (nome_reservado(nome))
        throw ErroSemantico(
            "'" + nome + "' e um nome reservado pelo gerador de codigo e "
            "nao pode nomear uma variavel global ou funcao");
}

// Registra um nome numa tabela de símbolos, recusando redeclarações no mesmo
// escopo. Sem esta verificação, dois nomes iguais chegam ao gerador de código
// e viram dois símbolos com o mesmo rótulo no assembly (".lcomm x" duas
// vezes, ou o rótulo de uma função sobre uma variável global): o compilador
// diz "Assembly gerado" e sai com sucesso, e o programa só quebra depois, na
// montagem, com uma mensagem do 'as' que não aponta para o arquivo-fonte.
static void declarar(TabelaSimbolos& tabela, const std::string& nome,
                     const Simbolo& simbolo, const std::string& escopo) {
    if (tabela.find(nome) != tabela.end())
        throw ErroSemantico(
            "'" + nome + "' ja foi declarado " + escopo);
    tabela[nome] = simbolo;
}

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

static bool simbolo_eh_array(const Simbolo& simbolo) {
    return simbolo.tipo == TipoSimbolo::ARRAY_GLOBAL ||
           simbolo.tipo == TipoSimbolo::ARRAY_LOCAL;
}

static void verificar_tamanho_array(const Decl& decl) {
    if (decl.get_tamanho_array() == 0)
        throw ErroSemantico(
            "array '" + decl.get_nome() + "' precisa ter tamanho positivo");
    if (decl.get_tamanho_array() >
        static_cast<std::size_t>(std::numeric_limits<long long>::max()) /
            sizeof(long long))
        throw ErroSemantico(
            "array '" + decl.get_nome() + "' e grande demais");
}

// ---------------------------------------------------------------------------
// Verificacao das expressoes
// ---------------------------------------------------------------------------

// percorre recursivamente uma expressao verificando se todo nome usado nela
// ja foi declarado e se esta sendo usado de acordo com o seu tipo de simbolo.
static void verificar_exp(const Exp& exp, const Escopo& escopo);

static void verificar_acesso_array(const std::string& nome,
                                   const Exp& indice,
                                   const Escopo& escopo) {
    const Simbolo* simbolo = resolver(escopo, nome);
    if (simbolo == nullptr)
        throw ErroSemantico(
            "array '" + nome + "' usado antes de ser declarado");
    if (!simbolo_eh_array(*simbolo))
        throw ErroSemantico("'" + nome + "' nao e um array");

    verificar_exp(indice, escopo);

    // Indices literais podem ser conferidos em compilacao. Indices calculados
    // continuam permitidos e sao resolvidos em tempo de execucao.
    if (const auto* literal = dynamic_cast<const Const*>(&indice)) {
        const long long valor = literal->get_valor();
        if (valor < 0 || static_cast<unsigned long long>(valor) >=
                             simbolo->tamanho_array) {
            throw ErroSemantico(
                "indice " + std::to_string(valor) + " fora dos limites do "
                "array '" + nome + "' (tamanho " +
                std::to_string(simbolo->tamanho_array) + ")");
        }
    }
}

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
        if (simbolo_eh_array(*simbolo)) {
            throw ErroSemantico(
                "array '" + var->get_nome() + "' precisa de um indice");
        }
        return;
    }

    if (const auto* acesso = dynamic_cast<const AcessoArray*>(&exp)) {
        verificar_acesso_array(acesso->get_nome(), acesso->get_indice(), escopo);
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
        if (simbolo_eh_array(*simbolo)) {
            throw ErroSemantico(
                "array '" + atrib->get_nome() + "' precisa de um indice");
        }
        verificar_exp(atrib->get_valor(), escopo);
        return;
    }

    if (const auto* atrib = dynamic_cast<const AtribuicaoArray*>(&cmd)) {
        verificar_acesso_array(atrib->get_nome(), atrib->get_indice(), escopo);
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
    // dois parametros com o mesmo nome, ou uma local repetindo o nome de um
    // parametro, dariam o mesmo deslocamento a dois lugares diferentes do
    // frame: um deles ficaria inacessivel, sem nenhum aviso
    for (const std::string& parametro : parametros)
        declarar(locais, parametro, {TipoSimbolo::PARAMETRO, 0},
                 "como parametro desta funcao");

    // a tabela local e consultada por ponteiro, entao continua crescendo
    // corretamente a cada variavel local registrada abaixo
    Escopo escopo{&globais, &locais};

    for (const Decl& local : variaveis_locais) {
        if (local.eh_array()) {
            verificar_tamanho_array(local);
            declarar(locais, local.get_nome(),
                     {TipoSimbolo::ARRAY_LOCAL, 0,
                      local.get_tamanho_array()},
                     "neste corpo");
        } else {
            verificar_exp(local.get_valor(), escopo);
            declarar(locais, local.get_nome(),
                     {TipoSimbolo::VARIAVEL_LOCAL, 0}, "neste corpo");
        }
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
                verificar_nome_disponivel(decl.get_nome());
                if (decl.eh_array()) {
                    verificar_tamanho_array(decl);
                    declarar(globais, decl.get_nome(),
                             {TipoSimbolo::ARRAY_GLOBAL, 0,
                              decl.get_tamanho_array()},
                             "no escopo global");
                } else {
                    verificar_exp(decl.get_valor(),
                                  Escopo{&globais, nullptr});
                    declarar(globais, decl.get_nome(),
                             {TipoSimbolo::VARIAVEL_GLOBAL, 0},
                             "no escopo global");
                }
                continue;
            }

            const Funcao& funcao = programa.get_funcoes()[topo.indice];
            // a funcao e registrada na tabela global ANTES de seu corpo ser
            // analisado: e isso que permite a recursao direta, porque quando
            // o corpo de f for percorrido o nome f ja estara declarado.
            verificar_nome_disponivel(funcao.get_nome());
            declarar(globais, funcao.get_nome(),
                     {TipoSimbolo::FUNCAO, funcao.get_parametros().size()},
                     "no escopo global");
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
            verificar_nome_disponivel(decl.get_nome());
            declarar(globais, decl.get_nome(),
                     {TipoSimbolo::VARIAVEL_GLOBAL, 0}, "no escopo global");
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
