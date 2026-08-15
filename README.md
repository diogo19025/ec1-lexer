# Compilador EC1

Compilador didático escrito em C++17. Ele executa análise léxica, análise
sintática, construção da AST, análise semântica e geração de assembly x86-64.
A linguagem aceita expressões inteiras, variáveis, comandos, funções recursivas
e arrays fixos de inteiros.

## Integrantes do grupo

- Ivaldo Pureza Freire Junior — 20230012879
- Diogo Soares Alves Barreto de Carvalho — 20230012799
- Luis Henrique Fernandes de Carvalho — 20230102410
- Tiago Brito e Silva — 20230102439

## Requisitos

- `g++` com suporte a C++17;
- `make` e Bash para executar os alvos do Makefile;
- GNU `as` e `ld` em Linux x86-64 para montar e ligar o assembly gerado.

No Windows, a geração do arquivo `.s` funciona normalmente; a montagem e a
execução do binário x86-64 devem ser feitas no WSL.

## Compilação

```sh
make
```

O comando cria o executável `ec1`.

## Uso

Para exibir os tokens, a AST e o resultado da análise:

```sh
./ec1 programa.ec1
```

Para gerar `programa.s`:

```sh
./ec1 --compilar programa.ec1
```

Para montar, ligar e executar em Linux x86-64:

```sh
as --64 -I src -o programa.o programa.s
ld -o programa programa.o
./programa
```

## Linguagem

Um programa completo contém declarações globais e funções opcionais, seguidas
por um bloco `main` obrigatório:

```text
<programa>  ::= (<vardecl> | <funcao>)* 'main' <corpo>
<funcao>    ::= 'fun' <ident> '(' <params>? ')' <corpo>
<params>    ::= <ident> (',' <ident>)*
<corpo>     ::= '{' <vardecl>* <comando>* '}'

<vardecl>   ::= 'var' <ident> '=' <exp> ';'
              | 'var' <ident> '[' <numero> ']' ';'

<comando>   ::= <atribuicao> | <if> | <while> | <retorno> | <bloco>
<atribuicao>::= <ident> '=' <exp> ';'
              | <ident> '[' <exp> ']' '=' <exp> ';'
<if>        ::= 'if' ('(' <exp> ')' | <exp>) <bloco>
                ('else' <bloco>)?
<while>     ::= 'while' ('(' <exp> ')' | <exp>) <bloco>
<retorno>   ::= 'return' <exp> ';'
<bloco>     ::= '{' <comando>* '}'

<exp>       ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
<exp_a>     ::= <exp_m> (('+' | '-') <exp_m>)*
<exp_m>     ::= <prim> (('*' | '/') <prim>)*
<prim>      ::= <numero>
              | <ident>
              | <ident> '[' <exp> ']'
              | <ident> '(' <argumentos>? ')'
              | '(' <exp> ')'
<argumentos>::= <exp> (',' <exp>)*
```

Todos os valores escalares e elementos de arrays são inteiros com sinal de
64 bits. Comparações produzem `1` para verdadeiro e `0` para falso.

### Variáveis e escopos

Variáveis globais são visíveis depois de sua declaração. Parâmetros e variáveis
locais pertencem ao corpo da função ou do `main` e escondem uma global de mesmo
nome. Uma variável precisa ser declarada antes do uso e não pode ser declarada
duas vezes no mesmo escopo.

### Funções

Funções recebem valores inteiros, retornam o resultado em `return` e podem
chamar funções declaradas anteriormente. Recursão direta é aceita. A quantidade
de argumentos deve ser exatamente igual à quantidade de parâmetros.

### Arrays de inteiros

Arrays são declarados com tamanho fixo e positivo:

```text
var valores[8];
```

O primeiro índice é `0`. A leitura e a escrita aceitam qualquer expressão
inteira como índice:

```text
valores[i] = i * i;
return valores[i];
```

Arrays globais e locais começam preenchidos com zero. Arrays locais ocupam o
frame da função; arrays globais ficam na seção `.bss`. Um índice literal fora
do intervalo é rejeitado durante a análise semântica. Índices calculados não
recebem verificação de limites em tempo de execução.

Arrays não podem ser usados como um valor único, passados como parâmetro ou
inicializados por lista. Somente seus elementos podem ser lidos e alterados.

## Exemplo

```text
fun somaquadrados(n) {
    var valores[4];
    var i = 0;
    var soma = 0;
    while (i < n) {
        valores[i] = i * i;
        soma = soma + valores[i];
        i = i + 1;
    }
    return soma;
}

main {
    return somaquadrados(4);
}
```

O programa retorna e imprime `14`.

## Testes

```sh
make test             # suíte completa
make test-array       # lexer, AST, parser, semântica e codegen de arrays
make test-array-e2e   # gera, monta, liga e executa os programas com arrays
```

Os testes ponta a ponta exigem Linux x86-64 com GNU `as` e `ld`.

## Estrutura

```text
src/       implementação do compilador e runtime assembly
tests/     testes unitários e programas de entrada
scripts/   testes ponta a ponta e de regressão
Makefile   compilação e alvos de teste
```

## Limitações

- identificadores contêm apenas letras e dígitos e começam por uma letra;
- não há operador unário; valores negativos são escritos como `0 - valor`;
- não há strings, ponto flutuante, ponteiros, passagem por referência ou I/O
  além da impressão do valor final;
- o compilador exige ao menos um `return` no corpo de cada função e do `main`,
  mas não prova que todos os caminhos de execução retornam;
- índices calculados de arrays não são verificados em tempo de execução.
