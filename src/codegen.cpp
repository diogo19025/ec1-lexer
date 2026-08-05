#include "codegen.h"
#include "ast.h"
#include <stdexcept>
#include <string>
#include <unordered_map>

// contador global de rótulos: cada if e cada while pega um número
// sequencial único, usado para montar rótulos como "ELSE_0", "FIM_IF_0",
// "WHILE_INICIO_1", "WHILE_FIM_1" etc. Como identificadores da linguagem
// Cmd só podem conter letras e dígitos (nunca '_'), rótulos com '_' nunca
// colidem com o nome de uma variável do programa do usuário.
static int contador_rotulos = 0;

static std::string novo_rotulo(const std::string& prefixo) {
    return prefixo + "_" + std::to_string(contador_rotulos++);
}

// mapa de nomes locais (parâmetros e variáveis locais de uma função ou do
// main) para o deslocamento correspondente em relação a %rbp, já formatado
// como operando de memória ("16(%rbp)", "-8(%rbp)" etc.). Um nome que não
// aparece aqui é uma variável global, guardada na seção .bss e acessada
// diretamente pelo nome.
using MapaLocal = std::unordered_map<std::string, std::string>;

static const MapaLocal MAPA_VAZIO;

// resolve como acessar um nome: o deslocamento local, se ele for parâmetro
// ou variável local da função/main atual, ou o próprio nome, se for global
static std::string operando(const std::string& nome, const MapaLocal& locais) {
    auto achado = locais.find(nome);
    if (achado != locais.end())
        return achado->second;
    return nome;
}

// percorre a árvore recursivamente emitindo instruções assembly. 'locais'
// contém os deslocamentos dos parâmetros e variáveis locais da função (ou
// do main) em que a expressão aparece; fora de qualquer função (formas EV
// e Cmd) ele vem vazio, e toda variável é global.
static void gerar_rec(const Exp& no, std::ostream& os,
                      const MapaLocal& locais = MAPA_VAZIO) {

    // constante inteira: mov direto para %rax
    if (const auto* c = dynamic_cast<const Const*>(&no)) {
        os << "    mov $" << c->get_valor() << ", %rax\n";
        return;
    }

    // uso de variável: se for parâmetro ou local, o valor está no frame da
    // função (endereço relativo a %rbp); se for global, na seção .bss
    if (const auto* var = dynamic_cast<const Var*>(&no)) {
        os << "    mov " << operando(var->get_nome(), locais) << ", %rax\n";
        return;
    }

    // chamada de função: calcula cada argumento e empilha, do último para
    // o primeiro, de modo que o primeiro argumento fique no topo da pilha
    // (mais perto do endereço de retorno) quando o "call" acontecer; a
    // função lê seus parâmetros a partir daí (ver gerar_funcao). Depois da
    // chamada, o resultado está em %rax e os argumentos são removidos da
    // pilha somando seu tamanho total a %rsp.
    if (const auto* chamada = dynamic_cast<const ChamadaFuncao*>(&no)) {
        const auto& argumentos = chamada->get_argumentos();
        for (auto it = argumentos.rbegin(); it != argumentos.rend(); ++it) {
            gerar_rec(**it, os, locais);
            os << "    push %rax\n";
        }
        os << "    call " << chamada->get_nome() << "\n";
        if (!argumentos.empty())
            os << "    add $" << (8 * argumentos.size()) << ", %rsp\n";
        return;
    }

    // operação binária: esquema de pilha
    // avalia direito primeiro, empilha; avalia esquerdo; desempilha direito em %rbx
    // assim a operação "esq OP dir" fica: %rax OP %rbx
    if (const auto* op = dynamic_cast<const OpBin*>(&no)) {
        gerar_rec(op->get_dir(), os, locais);  // direito → %rax
        os << "    push %rax\n";               // salva na pilha
        gerar_rec(op->get_esq(), os, locais);  // esquerdo → %rax
        os << "    pop %rbx\n";                // recupera direito em %rbx

        switch (op->get_op()) {
            case Operador::SOMA:
                os << "    add %rbx, %rax\n";
                break;
            case Operador::SUB:
                os << "    sub %rbx, %rax\n";
                break;
            case Operador::MULT:
                os << "    imul %rbx, %rax\n";
                break;
            case Operador::DIV:
                // idiv usa rdx:rax; cqo estende o sinal de rax para rdx
                os << "    mov %rbx, %rcx\n";
                os << "    cqo\n";
                os << "    idiv %rcx\n";
                break;
            // operadores relacionais: cmp calcula %rax - %rbx (dst - src)
            // e ajusta as flags; setX grava 0 ou 1 em %al de acordo com a
            // comparacao (esq OP dir), e movzbq estende esse byte para o
            // restante de %rax, que e o registrador de resultado
            case Operador::MENOR:
                os << "    cmp %rbx, %rax\n";
                os << "    setl %al\n";
                os << "    movzbq %al, %rax\n";
                break;
            case Operador::MAIOR:
                os << "    cmp %rbx, %rax\n";
                os << "    setg %al\n";
                os << "    movzbq %al, %rax\n";
                break;
            case Operador::IGUALDADE:
                os << "    cmp %rbx, %rax\n";
                os << "    sete %al\n";
                os << "    movzbq %al, %rax\n";
                break;
        }
        return;
    }
}

void gerar_codigo(const Exp& exp, std::ostream& os) {
    gerar_rec(exp, os);
}

// modelo de arquivo assembly definido na Atividade 02
void gerar_assembly_completo(const Exp& exp, std::ostream& os) {
    os << "#\n";
    os << "# Codigo gerado pelo compilador EC1\n";
    os << "#\n";
    os << "    .section .text\n";
    os << "    .globl _start\n";
    os << "_start:\n";
    gerar_codigo(exp, os);
    os << "    call imprime_num\n";
    os << "    call sair\n";
    os << "    .include \"runtime.s\"\n";
}

// gera o código de uma única declaração global: o código da expressão do
// lado direito (resultado em %rax), seguido de um mov para guardar o
// resultado na variável (reservada na seção .bss). Uma declaração global só
// pode usar outras globais, então nunca precisa de um mapa local.
static void gerar_decl(const Decl& decl, std::ostream& os) {
    os << "    # " << decl.get_nome() << " = " << decl.get_valor().imprimir() << ";\n";
    gerar_rec(decl.get_valor(), os);
    os << "    mov %rax, " << decl.get_nome() << "\n";
}

// declaracao antecipada: Cmd e Bloco sao mutuamente recursivos (um bloco
// contem comandos, e um comando If/While contem blocos)
static void gerar_bloco(const Bloco& bloco, std::ostream& os,
                        const MapaLocal& locais, const std::string& rotulo_retorno);

// rótulo fixo para onde todo 'return' do bloco main salta: o valor de
// retorno já está em %rax quando o salto acontece, e logo após esse rótulo
// (ao final de gerar_codigo(Programa&, ostream&)) o chamador
// (gerar_assembly_completo) emite as chamadas a imprime_num/sair, então o
// fluxo cai exatamente onde precisa, seja por um 'jmp' explícito ou por
// fall-through natural. Um 'return' dentro de uma função salta para um
// rótulo próprio daquela função (ver gerar_funcao), nunca para este.
static const char* ROTULO_FIM_PROGRAMA = "FIM_PROGRAMA";

// gera o código de um único comando, percorrendo a árvore recursivamente.
// 'locais' resolve os nomes de parâmetros/variáveis locais da função (ou
// do main) atual; 'rotulo_retorno' é para onde um 'return' deste corpo
// deve desviar (o fim do main, ou o epílogo da função em questão).
static void gerar_cmd(const Cmd& cmd, std::ostream& os,
                      const MapaLocal& locais, const std::string& rotulo_retorno) {
    // atribuicao: <ident> '=' <exp> ';' — igual à declaração, mas a
    // variável já existe (garantido pela análise semântica), então basta
    // recalcular a expressão e sobrescrever o valor guardado (no frame,
    // se for parâmetro/local, ou na seção .bss, se for global)
    if (const auto* atrib = dynamic_cast<const Atribuicao*>(&cmd)) {
        os << "    # " << atrib->get_nome() << " = "
           << atrib->get_valor().imprimir() << ";\n";
        gerar_rec(atrib->get_valor(), os, locais);
        os << "    mov %rax, " << operando(atrib->get_nome(), locais) << "\n";
        return;
    }

    // return: calcula o valor e desvia para o rotulo_retorno deste corpo,
    // onde o valor em %rax sera usado (impresso e o processo encerrado, no
    // main; devolvido ao chamador via 'ret', numa função)
    if (const auto* ret = dynamic_cast<const Retorno*>(&cmd)) {
        os << "    # return " << ret->get_valor().imprimir() << ";\n";
        gerar_rec(ret->get_valor(), os, locais);
        os << "    jmp " << rotulo_retorno << "\n";
        return;
    }

    // bloco aninhado: apenas gera cada comando em sequencia
    if (const auto* bloco = dynamic_cast<const Bloco*>(&cmd)) {
        gerar_bloco(*bloco, os, locais, rotulo_retorno);
        return;
    }

    // if/else: avalia a condicao; se for falsa (== 0), pula o bloco 'entao'
    // (indo direto para o 'else', se houver, ou para o fim do if). O
    // rotulo tem um numero unico (contador_rotulos) para nao colidir com
    // os rotulos de outros if/while do mesmo programa.
    if (const auto* no_if = dynamic_cast<const If*>(&cmd)) {
        std::string rotulo_fim = novo_rotulo("FIM_IF");

        os << "    # if (" << no_if->get_condicao().imprimir() << ")\n";
        gerar_rec(no_if->get_condicao(), os, locais);
        os << "    cmp $0, %rax\n";

        if (no_if->tem_senao()) {
            std::string rotulo_else = novo_rotulo("ELSE");
            os << "    je " << rotulo_else << "\n";
            gerar_bloco(no_if->get_entao(), os, locais, rotulo_retorno);
            os << "    jmp " << rotulo_fim << "\n";
            os << rotulo_else << ":\n";
            gerar_bloco(no_if->get_senao(), os, locais, rotulo_retorno);
        } else {
            os << "    je " << rotulo_fim << "\n";
            gerar_bloco(no_if->get_entao(), os, locais, rotulo_retorno);
        }

        os << rotulo_fim << ":\n";
        return;
    }

    // while: reavalia a condicao a cada iteracao; sai do laco assim que
    // ela for falsa (== 0)
    if (const auto* no_while = dynamic_cast<const While*>(&cmd)) {
        std::string rotulo_inicio = novo_rotulo("WHILE_INICIO");
        std::string rotulo_fim    = novo_rotulo("WHILE_FIM");

        os << rotulo_inicio << ":\n";
        os << "    # while (" << no_while->get_condicao().imprimir() << ")\n";
        gerar_rec(no_while->get_condicao(), os, locais);
        os << "    cmp $0, %rax\n";
        os << "    je " << rotulo_fim << "\n";
        gerar_bloco(no_while->get_corpo(), os, locais, rotulo_retorno);
        os << "    jmp " << rotulo_inicio << "\n";
        os << rotulo_fim << ":\n";
        return;
    }
}

// gera o codigo de cada comando do bloco, na ordem em que aparecem
static void gerar_bloco(const Bloco& bloco, std::ostream& os,
                        const MapaLocal& locais, const std::string& rotulo_retorno) {
    for (const auto& cmd : bloco.get_comandos())
        gerar_cmd(*cmd, os, locais, rotulo_retorno);
}

// monta o mapa de deslocamentos de um corpo (função ou main): os
// parâmetros, na ordem em que foram declarados, ocupam 16(%rbp), 24(%rbp),
// ... acima do endereço de retorno — porque são empilhados do último para
// o primeiro (ver gerar_rec/ChamadaFuncao), o primeiro parâmetro fica mais
// perto do topo da pilha no momento do "call", e por isso no deslocamento
// mais baixo. As variáveis locais, na ordem em que aparecem, ocupam
// -8(%rbp), -16(%rbp), ... abaixo do %rbp salvo.
static MapaLocal montar_mapa_local(const std::vector<std::string>& parametros,
                                    const std::vector<Decl>& variaveis_locais) {
    MapaLocal locais;
    for (std::size_t i = 0; i < parametros.size(); ++i)
        locais[parametros[i]] =
            std::to_string(16 + 8 * i) + "(%rbp)";
    for (std::size_t i = 0; i < variaveis_locais.size(); ++i)
        locais[variaveis_locais[i].get_nome()] =
            std::to_string(-8 * static_cast<long long>(i + 1)) + "(%rbp)";
    return locais;
}

// gera o código das variáveis locais de um corpo: cada uma tem seu valor
// calculado e guardado no deslocamento reservado para ela no frame
static void gerar_locais(const std::vector<Decl>& variaveis_locais,
                         const MapaLocal& locais, std::ostream& os) {
    for (const Decl& local : variaveis_locais) {
        os << "    # var " << local.get_nome() << " = "
           << local.get_valor().imprimir() << ";\n";
        gerar_rec(local.get_valor(), os, locais);
        os << "    mov %rax, " << operando(local.get_nome(), locais) << "\n";
    }
}

// gera o código de uma função: rótulo, prólogo, corpo e epílogo.
//
// Prólogo: empilha o %rbp do chamador (preservando-o) e faz %rbp apontar
// para o topo da pilha nesse instante, o que fixa os parâmetros em
// deslocamentos positivos e o espaço das locais em deslocamentos negativos
// pelo resto da função; em seguida reserva com "sub" o espaço de todas as
// variáveis locais de uma vez (8 bytes cada).
//
// Epílogo: desfaz exatamente o inverso do prólogo ("mov %rbp, %rsp"
// descarta o espaço das locais devolvendo %rsp ao ponto salvo; "pop %rbp"
// restaura o %rbp do chamador) e retorna com "ret", que usa o endereço de
// retorno empilhado pelo "call" de quem chamou esta função. Todo 'return'
// do corpo desvia para o rótulo do epílogo, com o valor já em %rax.
static void gerar_funcao(const Funcao& funcao, std::ostream& os) {
    MapaLocal locais = montar_mapa_local(funcao.get_parametros(),
                                          funcao.get_variaveis_locais());
    std::string rotulo_fim = "FIM_" + funcao.get_nome();

    os << funcao.get_nome() << ":\n";
    os << "    push %rbp\n";
    os << "    mov %rsp, %rbp\n";
    if (!funcao.get_variaveis_locais().empty())
        os << "    sub $" << (8 * funcao.get_variaveis_locais().size())
           << ", %rsp\n";

    gerar_locais(funcao.get_variaveis_locais(), locais, os);
    gerar_bloco(funcao.get_corpo(), os, locais, rotulo_fim);

    os << rotulo_fim << ":\n";
    os << "    mov %rbp, %rsp\n";
    os << "    pop %rbp\n";
    os << "    ret\n";
}

// gera o código do bloco main: tem variáveis locais como uma função (que
// também escondem globais de mesmo nome), mas nenhum parâmetro e nenhum
// prólogo/epílogo de chamada — o processo já começa em _start e termina
// logo depois do rótulo ROTULO_FIM_PROGRAMA, com imprime_num/sair. Ainda
// assim, se houver locais, é preciso reservar espaço para elas no frame
// (mesmo esquema de deslocamentos negativos de uma função).
static void gerar_main(const Programa& programa, std::ostream& os) {
    const std::vector<Decl>& locais_decl = programa.get_locais_main();
    MapaLocal locais = montar_mapa_local({}, locais_decl);

    if (!locais_decl.empty()) {
        os << "    push %rbp\n";
        os << "    mov %rsp, %rbp\n";
        os << "    sub $" << (8 * locais_decl.size()) << ", %rsp\n";
    }

    gerar_locais(locais_decl, locais, os);
    gerar_bloco(programa.get_corpo(), os, locais, ROTULO_FIM_PROGRAMA);
    os << ROTULO_FIM_PROGRAMA << ":\n";
}

void gerar_codigo(const Programa& programa, std::ostream& os) {
    // zera o contador a cada geracao completa, para que rotulos sejam
    // sempre previsiveis (e reprodutiveis em testes) independentemente de
    // quantas vezes gerar_codigo/gerar_assembly_completo ja foram chamados
    // no processo
    contador_rotulos = 0;

    // 1) código de cada declaração global, na ordem em que aparecem no fonte
    for (const Decl& decl : programa.get_decls())
        gerar_decl(decl, os);

    // 2a) forma Fun: bloco main. O código de cada função (chamada por ele
    // ou por outra função) é gerado à parte, em gerar_funcoes, depois das
    // chamadas a imprime_num/sair, para nunca ser alcançado por
    // fall-through a partir daqui.
    if (programa.eh_fun()) {
        os << "    # bloco main\n";
        gerar_main(programa, os);
        return;
    }

    // 2b) forma Cmd: corpo de comandos entre chaves. O rotulo
    // FIM_PROGRAMA e o ponto de encontro de todos os 'return' (e do
    // fim natural do bloco, caso nenhum 'return' seja executado); logo
    // depois dele, gerar_assembly_completo emite as chamadas de
    // imprime_num/sair usando o valor que estiver em %rax nesse ponto.
    if (programa.tem_corpo()) {
        os << "    # corpo de comandos\n";
        gerar_bloco(programa.get_corpo(), os, MAPA_VAZIO, ROTULO_FIM_PROGRAMA);
        os << ROTULO_FIM_PROGRAMA << ":\n";
        return;
    }

    // 2c) forma EV: expressão final (resultado em %rax)
    os << "    # expressao final\n";
    gerar_rec(programa.get_exp(), os);
}

// gera o código de cada função declarada no programa (forma Fun), uma
// depois da outra; cada uma só pode ser alcançada por um "call" (nunca por
// fall-through), então a ordem entre elas não importa e chamadas para
// frente ou recursivas funcionam sem nenhum tratamento especial: o rótulo
// da função já existe em algum ponto do arquivo quando o "call" é montado.
static void gerar_funcoes(const Programa& programa, std::ostream& os) {
    for (const Funcao& funcao : programa.get_funcoes())
        gerar_funcao(funcao, os);
}

// seção .bss: uma diretiva .lcomm por variável global declarada, reservando
// 8 bytes (inteiro de 64 bits) para cada uma
static void gerar_bss(const Programa& programa, std::ostream& os) {
    os << "    .section .bss\n";
    for (const Decl& decl : programa.get_decls())
        os << "    .lcomm " << decl.get_nome() << ", 8\n";
}

void gerar_assembly_completo(const Programa& programa, std::ostream& os) {
    os << "#\n";
    os << "# Codigo gerado pelo compilador EV/Cmd/Fun\n";
    os << "#\n";

    // a secao .bss so eh necessaria se houver variaveis globais declaradas
    if (!programa.get_decls().empty())
        gerar_bss(programa, os);

    os << "    .section .text\n";
    os << "    .globl _start\n";
    os << "_start:\n";
    gerar_codigo(programa, os);
    os << "    call imprime_num\n";
    os << "    call sair\n";

    // forma Fun: o código de cada função vem depois das chamadas acima,
    // então nunca é alcançado por fall-through a partir do main
    if (programa.eh_fun())
        gerar_funcoes(programa, os);

    os << "    .include \"runtime.s\"\n";
}
