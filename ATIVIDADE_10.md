# Atividade 10 - divisão em quatro partes

A atividade adiciona a linguagem Fun ao compilador da linguagem Cmd. A divisão abaixo
segue as dependências naturais do compilador e permite que cada pessoa trabalhe em uma
frente com critérios de conclusão claros.

## Pessoa 1 - Léxico, AST e análise sintática

- Adicionar os tokens `,`, `fun`, `var` e `main`.
- Representar na AST declarações de função, parâmetros formais, variáveis locais e
  chamadas de função.
- Reconhecer declarações globais e de função, o bloco `main`, listas de parâmetros
  formais e reais e o `return` final obrigatório.
- Diferenciar uma referência a variável de uma chamada de função pelo `(` após o
  identificador.
- Manter compatibilidade sintática com os programas das atividades anteriores.
- Criar testes unitários do lexer e do parser.

Status: implementada nesta branch.

## Pessoa 2 - Análise semântica e tabelas de símbolos

- Substituir a tabela simples de nomes por símbolos tipados: variável global, função,
  parâmetro ou variável local.
- Guardar, para cada função, a quantidade de parâmetros e sua tabela de símbolos local.
- Processar declarações na ordem do programa e registrar a função antes de analisar seu
  corpo, permitindo recursão direta.
- Validar função declarada, uso de um símbolo do tipo função e aridade das chamadas.
- Resolver variáveis primeiro no escopo local e depois no global, incluindo o
  sombreamento de globais por parâmetros ou variáveis locais.
- Criar testes para erros de nome, tipo de símbolo, aridade e escopo.

## Pessoa 3 - Geração de código para funções

- Gerar argumentos em ordem inversa, emitir `call` e remover os argumentos da pilha.
- Retornar o resultado de cada função em `%rax`.
- Gerar rótulo, prólogo e epílogo de função, preservando `%rbp`.
- Reservar o espaço das variáveis locais no frame e calcular os deslocamentos de
  parâmetros e variáveis locais em relação a `%rbp`.
- Diferenciar leituras e escritas de variáveis globais e locais.
- Cobrir funções sem parâmetros, sem variáveis locais, aninhadas e recursivas.

## Pessoa 4 - Integração, testes ponta a ponta e documentação

- Integrar as funções na seção `.text` antes ou depois de `_start`, mantendo as
  variáveis globais na seção `.bss` e o runtime no final do assembly.
- Criar programas de teste com recursão, chamadas entre funções, diferentes aridades e
  diferentes quantidades de variáveis locais.
- Adicionar um script que compile, monte, ligue e confira os resultados desses programas.
- Executar a regressão das linguagens anteriores e corrigir incompatibilidades de
  integração.
- Atualizar o README com a sintaxe Fun, limitações conhecidas e comandos para executar
  todos os testes.

## Como validar a Parte 1

```sh
make test-lexer-fun
make test-parser-fun
```

Para validar também a compatibilidade com as etapas anteriores:

```sh
make test-lex-ev
make test-parser-ev
make test-parser-cmd
make test-semantica-cmd
```
