# ec1-lexer

Compilador para a linguagem **EC** (Expressões Constantes), em C++, para a
disciplina de Construção de Compiladores. O projeto evolui a cada atividade;
cada atividade corresponde a um commit no histórico do git. A partir da
Atividade 08 a linguagem de entrada passa a ser a **EV** (Expressões com
Variáveis), que estende a EC2 com declaração e uso de variáveis.

| Atividade | Linguagem | O que foi adicionado |
|---|---|---|
| 04 | EC1 | Análise léxica (`token.*`, `lexer.*`) |
| 05 | EC1 | Análise sintática, AST e interpretador (`ast.*`, `parser.*`) |
| 06 | EC1 | Gerador de código assembly x86-64 (`codegen.*`, `runtime.s`) |
| 07 | EC2 | **Precedência e associatividade** — expressões sem parênteses obrigatórios (`parser.*`) |
| 08 | EV | **Variáveis** — declarações, tabela de símbolos e geração de código (`ast.*`, `parser.*`, `semantica.*`, `codegen.*`) |

## Estrutura do projeto

```
.
├── Makefile                    # build e testes
├── src/                        # código-fonte do compilador
│   ├── token.h  / token.cpp    # tipos de token e classe Token
│   ├── lexer.h  / lexer.cpp    # análise léxica
│   ├── ast.h    / ast.cpp      # AST: Exp, Const, Var, OpBin, Decl, Programa
│   ├── parser.h / parser.cpp   # analisador descendente recursivo (EV)
│   ├── semantica.h / .cpp      # verificação de variáveis declaradas (tabela de símbolos)
│   ├── codegen.h / codegen.cpp # gerador de código assembly x86-64 (Exp e Programa)
│   ├── runtime.s               # sub-rotinas imprime_num e sair
│   └── main.cpp                # ponto de entrada
├── scripts/
│   ├── run_tests.sh            # testes de análise/interpretação (Atividade 05)
│   ├── run_tests_ec2.sh        # testes de precedência/associatividade (Atividade 07)
│   ├── run_tests_ativ06.sh     # testes de geração de código (Atividade 06)
│   └── run_tests_ev.sh         # testes de geração de código com variáveis (Atividade 08)
└── tests/
    ├── lex/  test1..15.ec1     # análise léxica (Atividade 04)
    ├── sin/  v1..9, e1..7.ec1  # análise sintática válida e com erro (Atividade 05)
    ├── cod/  c1..12.ec1        # geração de código (Atividade 06)
    ├── ec2/  *.ec1             # expressões sem parênteses (Atividade 07)
    ├── lexer_ev_test.cpp       # testes léxicos da linguagem EV (Atividade 08)
    ├── parser_ev_test.cpp      # testes de parser/semântica da EV (Atividade 08)
    └── ev/   v*.ec1, e*.ec1    # geração de código com variáveis (Atividade 08)
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

### Modo análise/interpretação (Atividades 04, 05, 07 e 08)

Imprime os tokens, a árvore sintática e (para programas sem variáveis) o valor
interpretado:

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

Programas com variáveis são reconhecidos e a árvore é impressa normalmente,
mas a *interpretação* direta (sem gerar assembly) não está implementada para
`Var` — use o modo `--compilar` para executar programas com variáveis.

### Modo compilador (Atividades 06 e 08)

Gera `<arquivo>.s` com o assembly completo pronto para ser montado — funciona
tanto para expressões simples (EC1/EC2) quanto para programas com variáveis
(EV):

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
make test        # analise/interpretacao (05) + precedencia (07) + lexico EV + parser/semantica EV + codegen EV
make test-ec2    # apenas Atividade 07
make test-sin    # apenas Atividade 05
make test-cod    # geracao de codigo EC1/EC2 (Atividade 06) — requer Linux com as/ld
make test-lex-ev    # analise lexica da linguagem EV (Atividade 08 — Pessoa 1)
make test-parser-ev # parser + analise semantica da EV (Atividade 08 — Pessoa 3)
make test-cod-ev    # geracao de codigo da EV com variaveis (Atividade 08 — Pessoa 4) — requer Linux com as/ld
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

O trabalho foi dividido em quatro partes, cada uma em uma branch própria:

| Parte | Responsável | O que foi feito |
|---|---|---|
| Lexer e tokens | Pessoa 1 | tokens `IDENTIFICADOR`, `IGUAL`, `PONTO_VIRGULA`; testes em `tests/lexer_ev_test.cpp` |
| AST | Pessoa 2 | nós `Var`, `Decl`, `Programa` em `src/ast.*`, com impressão linear e em árvore |
| Parser e semântica | Pessoa 3 | `Parser::analisar()` reconhece `<programa>` e chama `verificar_variaveis` (`src/semantica.*`), que detecta variáveis usadas antes de declaradas |
| Geração de código e integração | Pessoa 4 | seção `.bss` para as variáveis, código de cada declaração e da expressão final (`src/codegen.*`), integração no `main.cpp`, testes finais (`tests/ev/`, `scripts/run_tests_ev.sh`) e este README |

`scripts/run_tests_ev.sh` compila cada programa em `tests/ev/v*.ec1` com
`--compilar`, monta, linka e executa, comparando o resultado com o valor
esperado; e verifica que os programas em `tests/ev/e*.ec1` (que usam variáveis
não declaradas) são rejeitados com um erro semântico. Requer `as`/`ld` de um
ambiente Linux x86-64.
