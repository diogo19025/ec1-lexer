# ec1-lexer

Compilador para a linguagem **EC** (Expressões Constantes), em C++, para a
disciplina de Construção de Compiladores. O projeto evolui a cada atividade;
cada atividade corresponde a um commit no histórico do git. A partir da
Atividade 08 a linguagem de entrada passa a ser a **EV** (Expressões com
Variáveis), que estende a EC2 com declaração e uso de variáveis, e a partir
da Atividade 09 passa a ser a **Cmd**, que adiciona comandos (atribuição,
`if`/`else`, `while`, `return`) sobre a base da EV.

| Atividade | Linguagem | O que foi adicionado |
|---|---|---|
| 04 | EC1 | Análise léxica (`token.*`, `lexer.*`) |
| 05 | EC1 | Análise sintática, AST e interpretador (`ast.*`, `parser.*`) |
| 06 | EC1 | Gerador de código assembly x86-64 (`codegen.*`, `runtime.s`) |
| 07 | EC2 | **Precedência e associatividade** — expressões sem parênteses obrigatórios (`parser.*`) |
| 08 | EV | **Variáveis** — declarações, tabela de símbolos e geração de código (`ast.*`, `parser.*`, `semantica.*`, `codegen.*`) |
| 09 | Cmd | **Comandos** — atribuição, `if`/`else`, `while`, `return`, operadores relacionais (`<`, `>`, `==`) e verificação semântica dos comandos (`token.*`, `ast.*`, `parser.*`, `semantica.*`) |

## Estrutura do projeto

```
.
├── Makefile                    # build e testes
├── src/                        # código-fonte do compilador
│   ├── token.h  / token.cpp    # tipos de token e classe Token
│   ├── lexer.h  / lexer.cpp    # análise léxica
│   ├── ast.h    / ast.cpp      # AST: Exp, Const, Var, OpBin, Decl, Programa,
│   │                           # e os comandos Atribuicao, Retorno, Bloco, If, While
│   ├── parser.h / parser.cpp   # analisador descendente recursivo (EV e Cmd)
│   ├── semantica.h / .cpp      # verificação de variáveis declaradas (tabela de
│   │                           # símbolos), tanto na forma EV quanto nos comandos da Cmd
│   ├── codegen.h / codegen.cpp # gerador de código assembly x86-64 (Exp e Programa/EV)
│   ├── runtime.s               # sub-rotinas imprime_num e sair
│   └── main.cpp                # ponto de entrada
├── scripts/
│   ├── run_tests.sh              # testes de análise/interpretação (Atividade 05)
│   ├── run_tests_ec2.sh          # testes de precedência/associatividade (Atividade 07)
│   ├── run_tests_ativ06.sh       # testes de geração de código (Atividade 06)
│   ├── run_tests_ev.sh           # testes de geração de código com variáveis (Atividade 08)
│   └── run_tests_semantica_cmd.sh # testes de análise semântica dos comandos (Atividade 09)
└── tests/
    ├── lex/  test1..15.ec1     # análise léxica (Atividade 04)
    ├── sin/  v1..9, e1..7.ec1  # análise sintática válida e com erro (Atividade 05)
    ├── cod/  c1..12.ec1        # geração de código (Atividade 06)
    ├── ec2/  *.ec1             # expressões sem parênteses (Atividade 07)
    ├── ev/   v*.ec1, e*.ec1    # geração de código com variáveis (Atividade 08)
    ├── cmd/  v*.ec1, e*.ec1    # atribuição, condição e repetição da linguagem Cmd (Atividade 09)
    ├── lexer_ev_test.cpp       # testes léxicos da linguagem EV (Atividade 08)
    ├── parser_ev_test.cpp      # testes de parser/semântica da EV (Atividade 08)
    ├── parser_cmd_test.cpp     # testes de parser dos comandos da linguagem Cmd (Atividade 09)
    └── semantica_cmd_test.cpp  # testes de análise semântica dos comandos (Atividade 09)
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

### Modo análise/interpretação (Atividades 04, 05, 07, 08 e 09)

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

Programas com variáveis (EV) ou com comandos (Cmd) são reconhecidos e a
árvore é impressa normalmente, junto com a análise semântica (verificação de
variáveis não declaradas). A *interpretação* direta (sem gerar assembly),
porém, só existe para expressões sem variáveis — para programas EV ou Cmd,
use o modo `--compilar` (a geração de código para a linguagem Cmd é
responsabilidade de uma etapa posterior à análise semântica implementada
aqui).

### Modo compilador (Atividades 06 e 08)

Gera `<arquivo>.s` com o assembly completo pronto para ser montado — funciona
tanto para expressões simples (EC1/EC2) quanto para programas com variáveis
(EV). A geração de código para comandos (`if`, `while`, `Cmd`) ainda não está
implementada.

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
variáveis](#geração-de-código-para-variáveis) acima.

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
  um deles com `./ec1 <arquivo>` (modo de análise, sem `--compilar`, já que a
  geração de código para a linguagem Cmd é uma etapa posterior) e confere se
  o resultado (aceito ou erro semântico) é o esperado.
