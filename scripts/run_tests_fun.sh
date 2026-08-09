#!/usr/bin/env bash
# Testes ponta a ponta da linguagem Fun (Atividade 10 - integração).
# Compila cada programa .ec1 (com --compilar), monta, linka, executa e
# compara a saida com o valor esperado. Tambem verifica que programas com
# erro semantico de funcao (nao declarada, nao e funcao, aridade incorreta)
# sao corretamente rejeitados. Requer: as (GNU Assembler) e ld disponiveis
# no PATH (Linux x86-64).
set -u
cd "$(dirname "$0")/.."

BIN=./ec1
echo "==> Compilando o compilador..."
g++ -std=c++17 -Wall -Wextra src/*.cpp -o "$BIN" \
    || { echo "Falha na compilacao"; exit 1; }
echo

passou=0
falhou=0

# arquivo | valor esperado | o que demonstra
validos=(
  "tests/fun/v1_soma_simples.ec1|30|funcao com dois parametros, sem variaveis locais"
  "tests/fun/v2_sem_parametros.ec1|42|funcao sem parametros"
  "tests/fun/v3_sem_locais.ec1|42|funcao com parametro, sem variaveis locais"
  "tests/fun/v4_com_locais.ec1|15|funcao com variaveis locais"
  "tests/fun/v5_varios_parametros.ec1|10|funcao com varios parametros (4)"
  "tests/fun/v6_fatorial_recursivo.ec1|120|funcao recursiva (fatorial de 5, com if/else)"
  "tests/fun/v7_fibonacci_recursivo.ec1|55|funcao recursiva com duas chamadas (fibonacci de 10)"
  "tests/fun/v8_funcao_chama_outra.ec1|20|funcao que chama outra funcao"
  "tests/fun/v9_sombreamento_global.ec1|102|parametro de funcao esconde variavel global"
  "tests/fun/v10_var_local_esconde_global.ec1|101|variavel local de funcao esconde global e a atualiza"
  "tests/fun/v11_local_inicializada_com_global.ec1|16|expressao que inicializa uma local le a global que ela esconde"
)

echo "===== Testes ponta a ponta: linguagem Fun (Atividade 10) ====="
for caso in "${validos[@]}"; do
  IFS='|' read -r arq esperado desc <<< "$caso"
  base="${arq%.ec1}"

  "$BIN" --compilar "$arq" 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "  [FAIL] $arq  (erro na geracao de assembly)  ($desc)"
    falhou=$((falhou+1)); continue
  fi

  as --64 -I src -o "${base}.o" "${base}.s" 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "  [FAIL] $arq  (erro na montagem)  ($desc)"
    falhou=$((falhou+1)); continue
  fi

  ld -o "${base}" "${base}.o" 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "  [FAIL] $arq  (erro no link)  ($desc)"
    falhou=$((falhou+1)); continue
  fi

  resultado=$("${base}" 2>/dev/null)
  if [ "$resultado" = "$esperado" ]; then
    echo "  [PASS] $arq  ->  $resultado   ($desc)"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  (esperado: $esperado, obtido: $resultado)  ($desc)"
    falhou=$((falhou+1))
  fi
done
echo

# arquivo | descricao do erro semantico esperado
erros=(
  "tests/fun/e1_funcao_inexistente.ec1|chamada a funcao nunca declarada"
  "tests/fun/e2_aridade_menos_argumentos.ec1|chamada com argumentos a menos que os parametros"
  "tests/fun/e3_aridade_mais_argumentos.ec1|chamada com argumentos a mais que os parametros"
  "tests/fun/e4_variavel_como_funcao.ec1|tentativa de chamar uma variavel como se fosse funcao"
  "tests/fun/e5_atribuicao_ao_nome_funcao.ec1|tentativa de atribuir a um nome de funcao"
)

echo "===== Erros semanticos de funcao e aridade (devem ser rejeitados) ====="
for caso in "${erros[@]}"; do
  IFS='|' read -r arq desc <<< "$caso"
  saida=$("$BIN" --compilar "$arq" 2>&1)
  rc=$?
  if [ $rc -ne 0 ] && printf '%s\n' "$saida" | grep -qi "erro semantico"; then
    echo "  [PASS] $arq  ($desc)"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  ($desc) nao detectado (rc=$rc)"
    falhou=$((falhou+1))
  fi
done
echo

# limpa os artefatos gerados pelos testes (.s, .o, binarios)
rm -f tests/fun/*.s tests/fun/*.o
for caso in "${validos[@]}"; do
  IFS='|' read -r arq _ <<< "$caso"
  rm -f "${arq%.ec1}"
done

echo "===== Resumo Fun (ponta a ponta): $passou passaram, $falhou falharam ====="
[ $falhou -eq 0 ]
