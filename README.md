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
./lexer tests/test3.ec1 2>&1
```

## Testes

Os arquivos de teste estão na pasta `tests/`. Cada arquivo `.ec1` contém um programa EC1 de entrada.

| Arquivo | Descrição |
|---|---|
| `test1.ec1` | Expressão aritmética válida |
| `test2.ec1` | Erro léxico: caracteres inválidos (a, @) |
| `test3.ec1` | Expressão aritmética válida |
| `test4.ec1` | Erro léxico: caracteres inválidos |

Para rodar todos manualmente:

```bash
./lexer tests/test1.ec1
./lexer tests/test2.ec1
./lexer tests/test3.ec1
./lexer tests/test4.ec1 2>&1
```
