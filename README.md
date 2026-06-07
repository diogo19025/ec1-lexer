# ec1-lexer

Analisador para a linguagem EC1 (Expressões Constantes 1), em C++, para a
disciplina de Construção de Compiladores. O projeto faz a análise **léxica**
(Atividade 04) e, sobre ela, a análise **sintática** e a **interpretação**
(Atividade 05): produz a árvore de sintaxe abstrata e calcula o valor da expressão.

## Arquivos

- `token.*`, `lexer.*` — análise léxica (Atividade 04).
- `ast.*` — árvore de sintaxe abstrata: `Exp` (base), `Const`, `OpBin`; interpretação (`avaliar`) e impressão (`imprimir`).
- `parser.*` — analisador sintático descendente recursivo (`analisaExp`, `analisaOperador`).
- `main.cpp` — lê o arquivo e executa lexer → parser → árvore → valor.

## Compilar

```bash
g++ -std=c++17 -Wall main.cpp lexer.cpp token.cpp parser.cpp ast.cpp -o ec1
```

No Windows, use `-o ec1.exe`.

## Executar

```bash
./ec1 <arquivo.ec1>
```

Exemplo:

```bash
$ ./ec1 tests/v4.ec1
<ParenEsq, "(", 0>
... (demais tokens) ...

Arvore (linear): (33 + (912 * 11))
Arvore (visual):
  +
    33
    *
      912
      11
Valor: 10065
```

O programa imprime os tokens (análise léxica), a árvore sintática e o valor
obtido pela interpretação. Erros léxicos e de sintaxe são reportados no
`stderr` e encerram o programa com código 1.

## Testes

Os arquivos `.ec1` ficam em `tests/`. Para rodar a bateria automática da
Atividade 05 (compila e confere árvore, valor e detecção de erros):

```bash
./run_tests.sh
```

### Testes de análise léxica (Atividade 04)

| Arquivo | Entrada | Descrição |
|---|---|---|
| `test1.ec1` | `(((10 + 20) * (3 - 4)) / 2)` | Expressão válida aninhada |
| `test2.ec1` | `1 + a * 2 @ 3` | Erros léxicos: `a` e `@` inválidos |
| `test3.ec1` | `999999` | Literal simples válido |
| `test4.ec1` | `### $$ !!!` | Todos os caracteres são inválidos |
| `test5.ec1` | `(3 + (4 + (11 + 7)))` | Soma aninhada válida |
| `test6.ec1` | `((427 / 7) + (11 * (231 + 5)))` | Expressão válida com todas as operações |
| `test7.ec1` | `(0 + 0)` | Operação com zeros |
| `test8.ec1` | `(99999999 * 88888888)` | Números grandes |
| `test9.ec1` | `((1 + 2) * (3 + 4))` | Multiplicação de somas |
| `test10.ec1` | `(100 - (50 + 25))` | Subtração com soma aninhada |
| `test11.ec1` | `(33 @ 5)` | Erro léxico: `@` inválido |
| `test12.ec1` | `(10 + abc)` | Erro léxico: palavra no lugar de número |
| `test13.ec1` | `(?)` | Erro léxico: `?` inválido |
| `test14.ec1` | `(5 + !3)` | Erro léxico: `!` inválido |
| `test15.ec1` | `( 42 )` | Espaços extras entre tokens |

### Testes de análise sintática e interpretação (Atividade 05)

Programas válidos (conferem a árvore produzida e o valor interpretado):

| Arquivo | Entrada | Árvore / Valor |
|---|---|---|
| `v1.ec1` | `42` | `42` = 42 |
| `v2.ec1` | `(6 * 7)` | `(6 * 7)` = 42 |
| `v3.ec1` | `(3 + (4 + (11 + 7)))` | = 25 |
| `v4.ec1` | `(33 + (912 * 11))` | = 10065 |
| `v5.ec1` | `((427 / 7) + (11 * (231 + 5)))` | = 2657 |
| `v6.ec1` | `(100 - (50 + 25))` | = 25 |
| `v7.ec1` | `((1 + 2) * (3 + 4))` | = 21 |
| `v8.ec1` | `(0 + 0)` | = 0 |
| `v9.ec1` | `(  10  -  20  )` | `(10 - 20)` = -10 |

Programas com erro de sintaxe (devem ser detectados e reportados):

| Arquivo | Entrada | Erro |
|---|---|---|
| `e1.ec1` | `(1 + )` | operando direito ausente |
| `e2.ec1` | `(1 + 2` | parêntese de fechamento ausente |
| `e3.ec1` | `1 + 2)` | tokens sobrando após a expressão |
| `e4.ec1` | `()` | parênteses sem expressão |
| `e5.ec1` | `(1 2 3)` | operador ausente |
| `e6.ec1` | `(5 + !3)` | caractere inválido `!` |
| `e7.ec1` | `(10 + abc)` | letras no lugar de número |
