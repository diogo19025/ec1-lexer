# ec1-lexer

Compilador para a linguagem **EC** (Expressões Constantes), em C++, para a
disciplina de Construção de Compiladores. O projeto evolui a cada atividade;
cada atividade corresponde a um commit no histórico do git.

| Atividade | Linguagem | O que foi adicionado |
|---|---|---|
| 04 | EC1 | Análise léxica (`token.*`, `lexer.*`) |
| 05 | EC1 | Análise sintática, AST e interpretador (`ast.*`, `parser.*`) |
| 06 | EC1 | Gerador de código assembly x86-64 (`codegen.*`, `runtime.s`) |
| 07 | EC2 | **Precedência e associatividade** — expressões sem parênteses obrigatórios (`parser.*`) |

## Estrutura do projeto

```
.
├── Makefile                    # build e testes
├── src/                        # código-fonte do compilador
│   ├── token.h  / token.cpp    # tipos de token e classe Token
    ├── lexer.h  / lexer.cpp    # análise léxica
│   ├── ast.h    / ast.cpp      # AST: Exp, Const, OpBin
│   ├── parser.h / parser.cpp   # analisador descendente recursivo (EC2)
    ├── codegen.h / codegen.cpp # gerador de código assembly x86-64
│   ├── runtime.s               # sub-rotinas imprime_num e sair
│   └── main.cpp                # ponto de entrada
├── scripts/
│   ├── run_tests.sh            # testes de análise/interpretação (Atividade 05)
│   ├── run_tests_ec2.sh        # testes de precedência/associatividade (Atividade 07)
│   └── run_tests_ativ06.sh     # testes de geração de código (Atividade 06)
└── tests/
    ├── lex/  test1..15.ec1     # análise léxica (Atividade 04)
    ├── sin/  v1..9, e1..7.ec1  # análise sintática válida e com erro (Atividade 05)
    ├── cod/  c1..12.ec1        # geração de código (Atividade 06)
    └── ec2/  *.ec1             # expressões sem parênteses (Atividade 07)
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

### Modo análise/interpretação (Atividades 04, 05 e 07)

Imprime os tokens, a árvore sintática e o valor interpretado:

```bash
./ec1 <arquivo.ec1>
```

Exemplo:

```bash
$ echo "7 + 5 * 3" > exemplo.ec1
$ ./ec1 exemplo.ec1
...
Arvore (linear): (7 + (5 * 3))
Valor: 22
```

### Modo compilador (Atividade 06)

Gera `<arquivo>.s` com o assembly completo pronto para ser montado:

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

## Testes

```bash
make test        # análise/interpretação (05) + precedência (07)
make test-ec2    # apenas Atividade 07
make test-sin    # apenas Atividade 05
make test-cod    # geração de código (Atividade 06) — requer Linux com as/ld
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
