#!/usr/bin/env bash
# Testes do compilador EC1 completo (Atividade 06 — geração de código).
# Requer: as (GNU Assembler) e ld disponíveis no PATH.
set -u
cd "$(dirname "$0")/.."

BIN=./ec1
echo "==> Compilando o compilador..."
g++ -std=c++17 -Wall -Wextra src/*.cpp -o "$BIN" \
    || { echo "Falha na compilacao"; exit 1; }
echo

passou=0
falhou=0

# regressão: análise e interpretação (Atividade 05)
echo "===== Regressao: analise e interpretacao (Atividade 05) ====="
validos_05=(
  "tests/sin/v1.ec1|42|42"
  "tests/sin/v2.ec1|(6 * 7)|42"
  "tests/sin/v3.ec1|(3 + (4 + (11 + 7)))|25"
  "tests/sin/v4.ec1|(33 + (912 * 11))|10065"
  "tests/sin/v5.ec1|((427 / 7) + (11 * (231 + 5)))|2657"
  "tests/sin/v6.ec1|(100 - (50 + 25))|25"
  "tests/sin/v7.ec1|((1 + 2) * (3 + 4))|21"
  "tests/sin/v8.ec1|(0 + 0)|0"
  "tests/sin/v9.ec1|(10 - 20)|-10"
)
for caso in "${validos_05[@]}"; do
  IFS='|' read -r arq arvore valor <<< "$caso"
  saida=$("$BIN" "$arq" 2>/dev/null)
  rc=$?
  if [ $rc -eq 0 ] \
     && printf '%s\n' "$saida" | grep -qF "Arvore (linear): $arvore" \
     && printf '%s\n' "$saida" | grep -qF "Valor: $valor"; then
    echo "  [PASS] $arq  ->  $arvore  =  $valor"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  (esperado: $arvore = $valor, rc=$rc)"
    falhou=$((falhou+1))
  fi
done
echo

# geração de código: compila → monta → linka → executa → compara
# arquivo | valor esperado
echo "===== Geracao de codigo (Atividade 06) ====="
codegen=(
  "tests/cod/c1.ec1|42"
  "tests/cod/c2.ec1|42"
  "tests/cod/c3.ec1|25"
  "tests/cod/c4.ec1|10065"
  "tests/cod/c5.ec1|2657"
  "tests/cod/c6.ec1|4"
  "tests/cod/c7.ec1|49"
  "tests/cod/c8.ec1|10"
  "tests/cod/c9.ec1|-3"
  "tests/cod/c10.ec1|45"
  "tests/cod/c11.ec1|36"
  "tests/cod/c12.ec1|35"
)

for caso in "${codegen[@]}"; do
  IFS='|' read -r arq esperado <<< "$caso"
  base="${arq%.ec1}"

  # gera o assembly
  "$BIN" --compilar "$arq" 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "  [FAIL] $arq  (erro na geracao de assembly)"
    falhou=$((falhou+1)); continue
  fi

  # monta ( -I src para o .include "runtime.s" resolver src/runtime.s )
  as --64 -I src -o "${base}.o" "${base}.s" 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "  [FAIL] $arq  (erro na montagem)"
    falhou=$((falhou+1)); continue
  fi

  # linka
  ld -o "${base}" "${base}.o" 2>/dev/null
  if [ $? -ne 0 ]; then
    echo "  [FAIL] $arq  (erro no link)"
    falhou=$((falhou+1)); continue
  fi

  # executa e compara
  resultado=$("${base}" 2>/dev/null)
  if [ "$resultado" = "$esperado" ]; then
    echo "  [PASS] $arq  ->  $resultado"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  (esperado: $esperado, obtido: $resultado)"
    falhou=$((falhou+1))
  fi
done
echo

# erros sintáticos devem ser rejeitados também no modo --compilar
echo "===== Erros de sintaxe (devem ser detectados) ====="
erros=(
  "tests/sin/e1.ec1|operando direito ausente"
  "tests/sin/e2.ec1|parentese de fechamento ausente"
  "tests/sin/e3.ec1|tokens sobrando apos a expressao"
  "tests/sin/e4.ec1|parenteses sem expressao"
  "tests/sin/e5.ec1|operador ausente"
  "tests/sin/e6.ec1|caractere invalido (!)"
  "tests/sin/e7.ec1|letras no lugar de numero"
)
for caso in "${erros[@]}"; do
  IFS='|' read -r arq desc <<< "$caso"
  saida=$("$BIN" --compilar "$arq" 2>&1)
  rc=$?
  if [ $rc -ne 0 ] && printf '%s\n' "$saida" | grep -qi "erro"; then
    echo "  [PASS] $arq  ($desc)"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  ($desc) nao detectado (rc=$rc)"
    falhou=$((falhou+1))
  fi
done
echo

echo "===== Resumo: $passou passaram, $falhou falharam ====="
[ $falhou -eq 0 ]
