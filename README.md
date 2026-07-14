# ec1-lexer

Compilador para a linguagem **EC1** (Expressões Constantes 1), em C++, para a
disciplina de Construção de Compiladores. O projeto evolui a cada atividade;
cada atividade corresponde a um commit no histórico do git.

| Atividade | O que foi adicionado |
|---|---|
| 04 | Análise léxica (`token.*`, `lexer.*`) |
| 05 | Análise sintática, AST e interpretador (`ast.*`, `parser.*`) |
| 06 | Gerador de código assembly x86-64 (`codegen.*`, `runtime.s`) |

## Estrutura do projeto

```
.
├── Makefile                    # build e testes
├── src/                        # código-fonte do compilador
│   ├── token.h  / token.cpp    # tipos de token e classe Token
    ├── lexer.h  / lexer.cpp    # análise léxica
│   ├── ast.h    / ast.cpp      # AST: Exp, Const, OpBin
│   ├── parser.h / parser.cpp   # analisador descendente recursivo
    ├── codegen.h / codegen.cpp # gerador de código assembly x86-64
│   ├── runtime.s               # sub-rotinas imprime_num e sair
│   └── main.cpp                # ponto de entrada
├── scripts/
│   ├── run_tests.sh            # testes de análise/interpretação (Atividade 05)
│   └── run_tests_ativ06.sh     # testes de geração de código (Atividade 06)
└── tests/
    ├── lex/  test1..15.ec1     # análise léxica (Atividade 04)
    ├── sin/  v1..9, e1..7.ec1  # análise sintática válida e com erro (Atividade 05)
│   └── cod/  c1..12.ec1        # geração de código (Atividade 06)
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

### Modo análise/interpretação (Atividades 04 e 05)

Imprime os tokens, a árvore sintática e o valor interpretado:

```bash
./ec1 <arquivo.ec1>
```

## Testes

```bash
make test        # análise/interpretação (Atividade 05)
make test-cod    # geração de código (Atividade 06) — requer Linux com as/ld
```

`scripts/run_tests.sh` executa programas válidos (`sin/v1..v9.ec1`) e com erro de
sintaxe (`sin/e1..e7.ec1`). `scripts/run_tests_ativ06.sh` compila, gera assembly,
monta, linka, executa e compara o resultado de cada caso em `cod/`.
