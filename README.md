# ec1-lexer

Analisador léxico para a linguagem EC1 (Expressões Constantes 1), desenvolvido em C++ para a disciplina de Construção de Compiladores.

## Compilar

```bash
g++ -std=c++17 -Wall main.cpp lexer.cpp token.cpp -o lexer
```

No Windows:

```bash
g++ -std=c++17 -Wall main.cpp lexer.cpp token.cpp -o lexer.exe
```

## Executar

```bash
./lexer <arquivo.ec1>
```

Exemplo:

```bash
./lexer tests/test1.ec1
```

Erros léxicos são reportados no `stderr`. Para ver tudo junto:

```bash
./lexer tests/test11.ec1 2>&1
```

## Testes

Os arquivos de teste estão na pasta `tests/`. Cada arquivo `.ec1` contém um programa EC1 de entrada.

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

Para rodar todos:

```bash
./lexer tests/test1.ec1
./lexer tests/test2.ec1 2>&1
./lexer tests/test3.ec1
./lexer tests/test4.ec1 2>&1
./lexer tests/test5.ec1
./lexer tests/test6.ec1
./lexer tests/test7.ec1
./lexer tests/test8.ec1
./lexer tests/test9.ec1
./lexer tests/test10.ec1
./lexer tests/test11.ec1 2>&1
./lexer tests/test12.ec1 2>&1
./lexer tests/test13.ec1 2>&1
./lexer tests/test14.ec1 2>&1
./lexer tests/test15.ec1
```
