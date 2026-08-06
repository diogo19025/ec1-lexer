# ec1-lexer

Compilador para a linguagem **EC** (Expressões Constantes), em C++, para a
disciplina de Construção de Compiladores. O projeto evolui a cada atividade;
cada atividade corresponde a um commit no histórico do git. A partir da
Atividade 08 a linguagem de entrada passa a ser a **EV** (Expressões com
Variáveis), que estende a EC2 com declaração e uso de variáveis, a partir
da Atividade 09 passa a ser a **Cmd**, que adiciona comandos (atribuição,
`if`/`else`, `while`, `return`) sobre a base da EV, e a partir da
Atividade 10 passa a ser a **Fun**, que adiciona funções com parâmetros,
variáveis locais e recursão sobre a base da Cmd.

| Atividade | Linguagem | O que foi adicionado |
|---|---|---|
| 04 | EC1 | Análise léxica (`token.*`, `lexer.*`) |
| 05 | EC1 | Análise sintática, AST e interpretador (`ast.*`, `parser.*`) |
| 06 | EC1 | Gerador de código assembly x86-64 (`codegen.*`, `runtime.s`) |
| 07 | EC2 | **Precedência e associatividade** — expressões sem parênteses obrigatórios (`parser.*`) |
| 08 | EV | **Variáveis** — declarações, tabela de símbolos e geração de código (`ast.*`, `parser.*`, `semantica.*`, `codegen.*`) |
| 09 | Cmd | **Comandos** — atribuição, `if`/`else`, `while`, `return`, operadores relacionais (`<`, `>`, `==`), verificação semântica e geração de código dos comandos (`token.*`, `ast.*`, `parser.*`, `semantica.*`, `codegen.*`) |
| 10 | Fun | **Funções** — declaração de funções com parâmetros e variáveis locais, chamadas (inclusive recursivas), escopos, tabela de símbolos por função e geração de código com pilha de chamadas (`token.*`, `ast.*`, `parser.*`, `semantica.*`, `codegen.*`) |

## Estrutura do projeto

```
.
├── Makefile                    # build e testes
├── src/                        # código-fonte do compilador
│   ├── token.h  / token.cpp    # tipos de token e classe Token
│   ├── lexer.h  / lexer.cpp    # análise léxica
│   ├── ast.h    / ast.cpp      # AST: Exp, Const, Var, ChamadaFuncao, OpBin, Decl, Funcao,
│   │                           # Programa, e os comandos Atribuicao, Retorno, Bloco, If, While
│   ├── parser.h / parser.cpp   # analisador descendente recursivo (EV, Cmd e Fun)
│   ├── semantica.h / .cpp      # verificação de variáveis, funções e escopos (tabela de
│   │                           # símbolos global e local por função), nas formas EV/Cmd/Fun
│   ├── codegen.h / codegen.cpp # gerador de código assembly x86-64 (Exp, Programa/EV,
│   │                           # comandos/Cmd e funções/Fun: pilha de chamadas, prólogo/
│   │                           # epílogo, deslocamentos de parâmetros e locais)
│   ├── runtime.s               # sub-rotinas imprime_num e sair
│   └── main.cpp                # ponto de entrada
├── scripts/
│   ├── run_tests.sh                # testes de análise/interpretação (Atividade 05)
│   ├── run_tests_ec2.sh            # testes de precedência/associatividade (Atividade 07)
│   ├── run_tests_ativ06.sh         # testes de geração de código (Atividade 06)
│   ├── run_tests_ev.sh             # testes de geração de código com variáveis (Atividade 08)
│   ├── run_tests_semantica_cmd.sh  # testes de análise semântica dos comandos (Atividade 09)
│   └── run_tests_fun.sh            # testes ponta a ponta da linguagem Fun (Atividade 10)
└── tests/
    ├── lex/  test1..15.ec1     # análise léxica (Atividade 04)
    ├── sin/  v1..9, e1..7.ec1  # análise sintática válida e com erro (Atividade 05)
    ├── cod/  c1..12.ec1        # geração de código (Atividade 06)
    ├── ec2/  *.ec1             # expressões sem parênteses (Atividade 07)
    ├── ev/   v*.ec1, e*.ec1    # geração de código com variáveis (Atividade 08)
    ├── cmd/  v*.ec1, e*.ec1    # atribuição, condição e repetição da linguagem Cmd (Atividade 09)
    ├── fun/  v*.ec1, e*.ec1    # testes ponta a ponta de funções da linguagem Fun (Atividade 10)
    ├── lexer_ev_test.cpp       # testes léxicos da linguagem EV (Atividade 08)
    ├── parser_ev_test.cpp      # testes de parser/semântica da EV (Atividade 08)
    ├── parser_cmd_test.cpp     # testes de parser dos comandos da linguagem Cmd (Atividade 09)
    ├── semantica_cmd_test.cpp  # testes de análise semântica dos comandos (Atividade 09)
    ├── lexer_fun_test.cpp      # testes léxicos da linguagem Fun (Atividade 10)
    ├── parser_fun_test.cpp     # testes de parser/AST da linguagem Fun (Atividade 10)
    ├── semantica_fun_test.cpp  # testes de análise semântica das funções (Atividade 10)
    └── codegen_fun_test.cpp    # testes unitários de geração de código das funções (Atividade 10)
```

## Gramática (EC2)

A partir da Atividade 07, os parênteses deixam de ser obrigatórios. A precedência
e a associatividade à esquerda são codificadas na gramática, com um não-terminal
por nível de precedência:

```
<exp_a> ::= <exp_m> (('+' | '-') <exp_m>)*     # adição e subtração
<exp_m> ::= <prim>  (('*' | '/') <prim>)*      # multiplicação e divisão
<prim>  ::= <num> | '(' <exp_a> ')'            # constante ou subexpressão
```

Assim, `7 + 5 * 3` é analisado como `(7 + (5 * 3))` e `10 - 8 - 2` como
`((10 - 8) - 2)`. Expressões totalmente parentizadas da EC1 continuam válidas e
produzem exatamente a mesma árvore.

## Gramática (EV) — Atividade 08

A partir da Atividade 08, a linguagem passa a se chamar **EV** (Expressões com
Variáveis) e um programa é uma sequência de zero ou mais declarações de
variável, seguida obrigatoriamente de uma expressão final marcada com `=`:

```
<programa> ::= <decl>* <result>
<decl>     ::= <ident> '=' <exp> ';'
<result>   ::= '=' <exp>
<ident>    ::= <letra><letra_digito>*
```

Exemplo — perímetro de um retângulo:

```
l = 30;
c = 40;
= l + l + c + c
```

Uma variável só pode ser usada depois de ter sido declarada (nas declarações
seguintes ou na expressão final); o contrário é um **erro semântico**,
detectado por `verificar_variaveis` (`src/semantica.*`) logo após a análise
sintática, usando uma tabela de símbolos:

```
$ ./ec1 --compilar tests/ev/e1_var_nao_declarada_final.ec1
Erro semantico: variavel 'x' usada antes de ser declarada
```

Note que, como a expressão final agora exige o `=` no início, entradas das
atividades anteriores (que eram só a expressão, sem `=`) precisaram ganhar
esse prefixo nos arquivos de teste (`tests/sin`, `tests/ec2`, `tests/cod`).

### Geração de código para variáveis

Cada variável declarada vira um símbolo de 8 bytes (inteiro de 64 bits) na
seção `.bss`, usando a diretiva `.lcomm`. O código de cada declaração calcula
o valor da expressão (deixando o resultado em `%rax`) e copia esse valor para
a variável com `mov %rax, <nome>`; uma referência à variável em uma expressão
gera `mov <nome>, %rax`:

```
$ ./ec1 --compilar tests/ev/v1_perimetro.ec1
Assembly gerado: tests/ev/v1_perimetro.s

$ as --64 -I src -o tests/ev/v1_perimetro.o tests/ev/v1_perimetro.s
$ ld -o tests/ev/v1_perimetro tests/ev/v1_perimetro.o
$ ./tests/ev/v1_perimetro
140
```

## Gramática (Cmd) — Atividade 09

A partir da Atividade 09 a linguagem passa a se chamar **Cmd**, que adiciona
comandos sobre a base da EV. Um programa continua começando com zero ou mais
declarações de variável, mas em vez de terminar só com uma expressão final,
pode terminar com um **corpo de comandos entre chaves**:

```
<programa> ::= <decl>* ( '=' <exp> | <bloco> )
<decl>     ::= <ident> '=' <exp> ';'
<bloco>    ::= '{' <cmd>* '}'
<cmd>      ::= <atrib> | <if> | <while> | <retorno> | <bloco>
<atrib>    ::= <ident> '=' <exp> ';'
<if>       ::= 'if' '(' <exp> ')' <bloco> ('else' <bloco>)?
<while>    ::= 'while' '(' <exp> ')' <bloco>
<retorno>  ::= 'return' <exp> ';'
<exp>      ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
<exp_a>    ::= <exp_m> (('+' | '-') <exp_m>)*
<exp_m>    ::= <prim> (('*' | '/') <prim>)*
<prim>     ::= <num> | <ident> | '(' <exp> ')'
```

A forma antiga da EV (`'=' <exp>`) continua válida — os dois formatos de
`<programa>` coexistem, e o parser escolhe um dos dois de acordo com o
próximo token depois das declarações (`{` inicia um bloco de comandos, `=`
inicia a expressão final antiga).

Os operadores relacionais `<`, `>` e `==` têm a **menor precedência** entre
todos os operadores: em `a + 1 < b * 2`, os dois lados da comparação são
agrupados antes dela (`(a + 1) < (b * 2)`). O resultado de uma comparação é
`1` (verdadeiro) ou `0` (falso), e pode ser usado normalmente em outras
expressões.

Exemplo — maior de dois números:

```
a = 7;
b = 12;
{
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
```

Exemplo — soma de 1 a `n` usando `while`:

```
n = 10;
i = 1;
soma = 0;
{
    while (i < n + 1) {
        soma = soma + i;
        i = i + 1;
    }
    return soma;
}
```

### Análise semântica dos comandos

Assim como na EV, toda variável usada precisa ter sido declarada antes. Na
linguagem Cmd isso se estende aos comandos do corpo do programa:

- uma **atribuição** (`<ident> '=' <exp> ';'` dentro do corpo) só é válida se
  a variável do lado esquerdo já foi declarada antes — diferente de uma
  `<decl>`, uma atribuição **não declara** uma variável nova; atribuir a uma
  variável nunca declarada é um erro semântico;
- toda variável usada dentro da condição de um `if` ou `while`, ou dentro do
  valor de um `return`, também precisa ter sido declarada antes;
- a verificação percorre recursivamente os comandos dentro dos dois ramos de
  um `if` (inclusive quando só um dos ramos usa a variável inválida) e do
  corpo de um `while`, além de blocos aninhados dentro de outros blocos.

Essa verificação é feita por `verificar_variaveis` (`src/semantica.*`), a
mesma função usada para a EV, e roda automaticamente dentro de
`Parser::analisar()` assim que a árvore do programa é construída:

```
$ ./ec1 tests/cmd/e1_atribuicao_nao_declarada.ec1
...
Erro semantico: atribuicao a variavel 'x' que nao foi declarada
```

### Geração de código dos comandos

O modo `--compilar` também gera assembly para programas na forma `Cmd`
(corpo de comandos entre chaves), implementado em `src/codegen.*`:

- **atribuição** (`<ident> '=' <exp> ';'` dentro do corpo) gera o código da
  expressão do lado direito (resultado em `%rax`) seguido de
  `mov %rax, <nome>`, exatamente como uma declaração — a diferença é que a
  variável já existe em `.bss` desde a declaração, a atribuição só
  sobrescreve o valor;
- os **operadores relacionais** `<`, `>` e `==` são traduzidos para
  `cmp` seguido de `setl` / `setg` / `sete` sobre `%al`, estendido para
  `%rax` com `movzbq`; o resultado é sempre `0` ou `1`, como na
  interpretação (`OpBin::avaliar`);
- **`if`/`else`** avalia a condição, compara o resultado com `0` (`cmp $0,
  %rax`) e usa `je` para desviar: se não houver `else`, desvia direto para
  o rótulo de fim do `if`; se houver, desvia para o rótulo do `else`, e o
  fim do bloco `então` pula (`jmp`) o `else` antes de cair no rótulo final;
- **`while`** funciona de forma parecida: um rótulo marca o início do
  laço (onde a condição é reavaliada a cada iteração), e `je` desvia para
  o rótulo de fim assim que a condição for falsa; o final do corpo do
  laço volta (`jmp`) para o rótulo de início;
- **`return`** gera o código da expressão (resultado em `%rax`) e desvia
  (`jmp`) para um rótulo fixo `FIM_PROGRAMA`, comum a todos os `return` do
  programa (inclusive os que estão dentro de `if`/`while` aninhados); logo
  depois desse rótulo o compilador emite as chamadas de `imprime_num` e
  `sair`, então o valor deixado em `%rax` pelo `return` (ou, na ausência de
  um `return` executado, o que quer que reste em `%rax` ao final do bloco)
  é o que acaba sendo impresso;
- os **rótulos** de cada `if` e `while` (`ELSE_N`, `FIM_IF_N`,
  `WHILE_INICIO_N`, `WHILE_FIM_N`) recebem um número sequencial de um
  contador global, reiniciado a cada chamada de `gerar_codigo(Programa&,
  ...)`, garantindo que sejam únicos mesmo com `if`/`while` aninhados ou
  repetidos no mesmo programa. Como identificadores da linguagem Cmd só
  podem conter letras e dígitos (nunca `_`), esses rótulos nunca colidem
  com o nome de uma variável do programa do usuário.

Exemplo — maior de dois números, compilado e executado:

```
$ ./ec1 --compilar tests/cmd/v2_maior_de_dois.ec1
Assembly gerado: tests/cmd/v2_maior_de_dois.s

$ as --64 -I src -o tests/cmd/v2_maior_de_dois.o tests/cmd/v2_maior_de_dois.s
$ ld -o tests/cmd/v2_maior_de_dois tests/cmd/v2_maior_de_dois.o
$ ./tests/cmd/v2_maior_de_dois
12
```

Exemplo — soma de 1 a `n` com `while` (veja a seção anterior para o
fonte): compilando, montando, linkando e executando da mesma forma, a
saída é `55` para `n = 10`.

## Gramática (Fun) — Atividade 10

A partir da Atividade 10 a linguagem passa a se chamar **Fun**, que
adiciona **funções** sobre a base da Cmd. Um programa passa a ser uma
sequência de declarações de topo — variáveis globais (`var`) e funções
(`fun`) — seguida obrigatoriamente de um bloco `main`:

```
<programa> ::= <topo>* 'main' <corpo>
<topo>     ::= <vardecl> | <funcao>
<vardecl>  ::= 'var' <ident> '=' <exp> ';'
<funcao>   ::= 'fun' <ident> '(' <params>? ')' <corpo>
<params>   ::= <ident> (',' <ident>)*
<corpo>    ::= '{' <vardecl>* <cmd>* '}'
<cmd>      ::= <atrib> | <if> | <while> | <retorno> | <bloco>
<atrib>    ::= <ident> '=' <exp> ';'
<exp>      ::= <exp_r> (('<' | '>' | '==') <exp_r>)*
<exp_r>    ::= <exp_a> (('+' | '-') <exp_a>)*
<exp_a>    ::= <prim> (('*' | '/') <prim>)*
<prim>     ::= <num> | <chamada> | <ident> | '(' <exp> ')'
<chamada>  ::= <ident> '(' <args>? ')'
<args>     ::= <exp> (',' <exp>)*
```

O `<corpo>` de uma função ou do `main` pode declarar variáveis locais com
`var` (só no início do bloco, antes dos comandos) e precisa conter pelo
menos um `return` em algum ponto — diretamente no corpo, ou dentro dos
ramos de um `if`/`else` ou do corpo de um `while` (uma função recursiva
escrita como `if (...) { return ...; } else { return ...; }` é uma forma
válida e comum de terminar o corpo). Uma chamada de função (`<ident> '('
<args>? ')'`) é diferenciada de uma simples referência a variável pelo
parêntese que a segue; o próprio parser já resolve essa ambiguidade léxica
olhando o próximo token.

Exemplo — soma de dois números com uma variável global usada dentro da
função:

```
var base = 10;

fun soma(x, y) {
    var total = x + y;
    total = total + base;
    return total;
}

main {
    return soma(2, 3);
}
```

Exemplo — fatorial recursivo:

```
fun fatorial(n) {
    if (n == 0) {
        return 1;
    } else {
        return n * fatorial(n - 1);
    }
}

main {
    return fatorial(5);
}
```

### Análise semântica das funções

Além da verificação de variáveis já feita para a EV e a Cmd, a linguagem
Fun introduz:

- uma **tabela de símbolos global** (variáveis globais e nomes de função) e
  uma **tabela local**, criada para cada função (e para o `main`), com seus
  parâmetros e variáveis locais;
- uma referência a um nome é resolvida primeiro no escopo local; se não
  estiver lá, cai para o escopo global — por isso um parâmetro ou variável
  local **esconde** uma global de mesmo nome dentro daquela função;
- uma função é registrada na tabela global **antes** de seu corpo ser
  analisado, o que permite chamadas recursivas (a função já "existe" para
  si mesma) e chamadas para frente (uma função pode chamar outra declarada
  depois dela no arquivo);
- toda chamada é verificada: o nome chamado precisa existir e ser
  realmente uma função (não uma variável), e a quantidade de argumentos
  passados precisa bater exatamente com a quantidade de parâmetros da
  função (nem a mais, nem a menos);
- atribuir a um nome que é função (em vez de variável) também é um erro
  semântico.

```
$ ./ec1 --compilar tests/fun/e2_aridade_menos_argumentos.ec1
Erro semantico: funcao 'soma' espera 2 argumento(s), mas recebeu 1
```

### Geração de código das funções

Cada função vira um bloco de assembly próprio na seção `.text`, alcançado
só por `call` (nunca por fall-through a partir do `main`), seguindo a
convenção clássica de pilha do x86-64:

- **chamada**: os argumentos são calculados e empilhados na **ordem
  inversa** (do último para o primeiro), de modo que o primeiro parâmetro
  fique mais perto do topo da pilha no momento do `call`; depois da
  chamada, os argumentos são removidos da pilha somando seu tamanho total
  a `%rsp`, e o resultado da função está em `%rax`;
- **prólogo**: `push %rbp` (preserva o `%rbp` do chamador) seguido de
  `mov %rsp, %rbp`, e depois um `sub` reservando de uma vez o espaço de
  todas as variáveis locais (8 bytes cada); os parâmetros ficam em
  deslocamentos positivos (`16(%rbp)`, `24(%rbp)`, ...) e as locais em
  deslocamentos negativos (`-8(%rbp)`, `-16(%rbp)`, ...) em relação a
  `%rbp`;
- **epílogo**: `mov %rbp, %rsp` seguido de `pop %rbp` desfazem exatamente o
  prólogo, e `ret` retorna usando o endereço de retorno empilhado pelo
  `call`; todo `return` do corpo desvia para o rótulo do epílogo, com o
  valor já calculado em `%rax`;
- uma referência a um nome dentro de uma função usa o deslocamento salvo
  no mapa local se for parâmetro/variável local, ou o próprio nome (seção
  `.bss`) se for global — a mesma resolução de escopo da análise semântica.

```
$ ./ec1 --compilar tests/fun/v6_fatorial_recursivo.ec1
Assembly gerado: tests/fun/v6_fatorial_recursivo.s

$ as --64 -I src -o tests/fun/v6_fatorial_recursivo.o tests/fun/v6_fatorial_recursivo.s
$ ld -o tests/fun/v6_fatorial_recursivo tests/fun/v6_fatorial_recursivo.o
$ ./tests/fun/v6_fatorial_recursivo
120
```

## Compilar

```bash
make            # gera o executável ./ec1  (usa src/*.cpp)
```

Ou manualmente:

```bash
g++ -std=c++17 -Wall -Wextra src/*.cpp -o ec1
```

No Windows (MinGW), o executável sai como `ec1.exe`.

## Executar

### Modo análise/interpretação (Atividades 04, 05, 07, 08, 09 e 10)

Imprime os tokens, a árvore sintática e, dependendo da forma do programa, o
valor interpretado ou um aviso de que a interpretação ainda não está
disponível:

```bash
./ec1 <arquivo.ec1>
```

Exemplo:

```bash
$ echo "= 7 + 5 * 3" > exemplo.ec1
$ ./ec1 exemplo.ec1
...
Arvore (linear): (7 + (5 * 3))
Valor: 22
```

Programas com variáveis (EV), comandos (Cmd) ou funções (Fun) são
reconhecidos e a árvore é impressa normalmente, junto com a análise
semântica (verificação de variáveis não declaradas, funções inexistentes ou
aridade incorreta). A *interpretação* direta (sem gerar assembly), porém,
só existe para expressões sem variáveis — para programas EV, Cmd ou Fun,
use o modo `--compilar`.

### Modo compilador (Atividades 06, 08, 09 e 10)

Gera `<arquivo>.s` com o assembly completo pronto para ser montado — funciona
para expressões simples (EC1/EC2), para programas com variáveis (EV), para
programas com comandos (`if`, `while`, `return`, atribuição — linguagem
Cmd, Atividade 09; veja a seção [Geração de código dos
comandos](#geração-de-código-dos-comandos) acima) e para programas com
funções (`fun`, `var`, `main` — linguagem Fun, Atividade 10; veja a seção
[Geração de código das funções](#geração-de-código-das-funções) acima).

```bash
./ec1 --compilar <arquivo.ec1>
```

Exemplo completo — compilar, montar, linkar e executar (Linux x86-64):

```bash
$ ./ec1 --compilar tests/cod/c4.ec1
Assembly gerado: tests/cod/c4.s

$ as --64 -I src -o tests/cod/c4.o tests/cod/c4.s   # -I src acha o runtime.s
$ ld -o tests/cod/c4 tests/cod/c4.o
$ ./tests/cod/c4
10065
```

Exemplo com variáveis, veja também a seção [Geração de código para
variáveis](#geração-de-código-para-variáveis) acima. Exemplo com funções,
veja [Geração de código das funções](#geração-de-código-das-funções).

## Testes

```bash
make test        # roda toda a suite (05, 06, 07, 08 e a parte semantica da 09)
make test-ec2    # apenas Atividade 07
make test-sin    # apenas Atividade 05
make test-cod    # geracao de codigo EC1/EC2 (Atividade 06) — requer Linux com as/ld
make test-lex-ev    # analise lexica da linguagem EV (Atividade 08)
make test-parser-ev # parser + analise semantica da EV (Atividade 08)
make test-cod-ev    # geracao de codigo da EV com variaveis (Atividade 08) — requer Linux com as/ld
make test-parser-cmd    # parser dos comandos da linguagem Cmd (Atividade 09)
make test-semantica-cmd # testes unitarios da analise semantica dos comandos (Atividade 09)
make test-cmd           # programas .ec1 completos (atribuicao/condicao/repeticao) da linguagem Cmd (Atividade 09)
make test-lexer-fun     # testes lexicos da linguagem Fun (Atividade 10)
make test-parser-fun    # testes de parser/AST da linguagem Fun (Atividade 10)
make test-semantica-fun # testes unitarios da analise semantica das funcoes (Atividade 10)
make test-codegen-fun   # testes unitarios de geracao de codigo das funcoes (Atividade 10)
make test-fun           # testes ponta a ponta (compilar/montar/linkar/executar) da linguagem Fun (Atividade 10) — requer Linux com as/ld
```

### Atividade 07 — precedência e associatividade

`scripts/run_tests_ec2.sh` verifica, para cada expressão sem parênteses, que a
árvore linear e o valor interpretado estão corretos.

| Entrada | Árvore | Valor | O que demonstra |
|---|---|---|---|
| `7 + 5 * 3` | `(7 + (5 * 3))` | 22 | `*` tem precedência sobre `+` |
| `2 * 3 + 4` | `((2 * 3) + 4)` | 10 | `*` tem precedência sobre `+` |
| `20 - 2 * 3` | `(20 - (2 * 3))` | 14 | `*` tem precedência sobre `-` |
| `7 + 5 + 3` | `((7 + 5) + 3)` | 15 | associatividade à esquerda (`+`) |
| `10 - 8 - 2` | `((10 - 8) - 2)` | 0 | associatividade à esquerda (`-`) |
| `100 / 10 / 2` | `((100 / 10) / 2)` | 5 | associatividade à esquerda (`/`) |
| `8 / 4 * 2` | `((8 / 4) * 2)` | 4 | mesma precedência, esquerda→direita |
| `2 + 3 * 4 - 5` | `((2 + (3 * 4)) - 5)` | 9 | níveis de precedência misturados |
| `2 * (3 + 4)` | `(2 * (3 + 4))` | 14 | parênteses sobrepõem a precedência |
| `(1 + 2) * 3 + 4` | `(((1 + 2) * 3) + 4)` | 13 | parênteses + precedência |
| `42` | `42` | 42 | constante isolada |

### Atividade 06 — geração de código

`scripts/run_tests_ativ06.sh` compila o compilador, gera assembly para cada caso,
monta, linka, executa e compara o resultado. Requer `as` e `ld` de um ambiente
**Linux x86-64** (o assembly usa syscalls do Linux). Inclui regressão das
Atividades 04 e 05.

### Atividade 05 — análise sintática e interpretação

`scripts/run_tests.sh` executa programas válidos (`sin/v1..v9.ec1`) e com erro de
sintaxe (`sin/e1..e7.ec1`), que devem ser corretamente rejeitados.

### Atividade 08 — variáveis (EV)

`scripts/run_tests_ev.sh` compila cada programa em `tests/ev/v*.ec1` com
`--compilar`, monta, linka e executa, comparando o resultado com o valor
esperado; e verifica que os programas em `tests/ev/e*.ec1` (que usam variáveis
não declaradas) são rejeitados com um erro semântico. Requer `as`/`ld` de um
ambiente Linux x86-64.

### Atividade 09 — análise semântica dos comandos (Cmd)

- `tests/semantica_cmd_test.cpp` (rodado por `make test-semantica-cmd`) são
  testes unitários que chamam o parser diretamente sobre trechos de código,
  cobrindo: atribuição a variável declarada e não declarada, condições de
  `if`/`while` com e sem variável não declarada nos dois ramos do `if`,
  blocos aninhados e alguns programas completos (maior de dois números,
  fatorial iterativo).
- `tests/cmd/` contém programas `.ec1` completos exercitando atribuição,
  condição e repetição — `v1..v6` devem ser aceitos, `e1..e4` devem ser
  rejeitados com erro semântico (variável não declarada usada em atribuição,
  na condição de `if`/`while`, ou em algum dos ramos do `if`).
  `scripts/run_tests_semantica_cmd.sh` (rodado por `make test-cmd`) roda cada
  um deles com `./ec1 <arquivo>` (modo de análise, sem `--compilar`) e
  confere se o resultado (aceito ou erro semântico) é o esperado — esse
  script cobre apenas parser e análise semântica, não a geração de código.
- a geração de código dos comandos (`--compilar`, veja a seção [Geração de
  código dos comandos](#geração-de-código-dos-comandos)) pode ser validada
  manualmente sobre os mesmos programas de `tests/cmd/v*.ec1`: compilar,
  montar com `as`, linkar com `ld` e executar, comparando a saída com o
  valor esperado — o mesmo fluxo usado nos exemplos acima e no
  `scripts/run_tests_ev.sh` da Atividade 08.

### Atividade 10 — funções (Fun)

- `tests/lexer_fun_test.cpp`, `tests/parser_fun_test.cpp` e
  `tests/semantica_fun_test.cpp` (rodados por `make test-lexer-fun`,
  `make test-parser-fun` e `make test-semantica-fun`) são testes unitários
  do léxico, da AST/parser e da análise semântica das funções — cobrem,
  entre outras coisas, resolução de escopo (local antes de global,
  parâmetro/local escondendo global), registro antecipado da função (para
  permitir recursão), e chamadas com nome inexistente, aridade errada ou
  para algo que não é função.
- `tests/codegen_fun_test.cpp` (rodado por `make test-codegen-fun`) são
  testes unitários sobre o assembly gerado: ordem de empilhamento dos
  argumentos, prólogo/epílogo, deslocamentos de parâmetros e variáveis
  locais em relação a `%rbp`, e resolução de local vs. global dentro do
  corpo de uma função.
- `tests/fun/` contém 15 programas `.ec1` completos para os testes **ponta
  a ponta** (compilar → montar → linkar → executar), rodados por
  `scripts/run_tests_fun.sh` (`make test-fun`, requer `as`/`ld` de um
  ambiente Linux x86-64):
  - `v1..v10` (aceitos, com o valor de retorno comparado ao esperado):
    função com dois parâmetros, função sem parâmetros, função com
    parâmetro e sem variáveis locais, função com variáveis locais, função
    com vários parâmetros (4), função recursiva (fatorial, usando
    `if`/`else`), função recursiva com duas chamadas (fibonacci), função
    que chama outra função, parâmetro escondendo uma variável global, e
    variável local escondendo (e atualizando) uma global;
  - `e1..e5` (rejeitados com erro semântico): chamada a função nunca
    declarada, chamada com argumentos a menos ou a mais que os parâmetros
    da função, tentativa de chamar uma variável como se fosse função, e
    tentativa de atribuir a um nome de função.
- `make test` roda toda a suíte, incluindo a regressão completa das
  Atividades 05 a 09 (léxico, sintático, geração de código EC1/EC2/EV/Cmd),
  para garantir que a linguagem Fun não quebrou nada das etapas anteriores.

Durante a validação ponta a ponta desta atividade, uma função recursiva
escrita com `if (...) { return ...; } else { return ...; }` (o padrão mais
natural para recursão, como no fatorial) era rejeitada pelo parser, que só
aceitava um `return` literalmente como o último comando do corpo, sem olhar
dentro dos ramos de um `if`/`while`. Isso foi corrigido em `src/parser.cpp`
(`analisaCorpoFun`): o corpo de uma função/`main` agora aceita `return` em
qualquer nível de aninhamento, mas continua exigindo pelo menos um `return`
em algum ponto do corpo (um corpo sem nenhum `return`, em qualquer nível,
continua sendo um erro de sintaxe).
