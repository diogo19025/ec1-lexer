#!/usr/bin/env bash
# Testes do analisador sintático e interpretador EC1 (Atividade 05).
set -u
cd "$(dirname "$0")/.."

BIN=./ec1
echo "==> Compilando..."
g++ -std=c++17 -Wall -Wextra src/*.cpp -o "$BIN" \
    || { echo "Falha na compilacao"; exit 1; }
echo

passou=0
falhou=0

# arquivo | arvore esperada | valor esperado
validos=(
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

echo "===== Programas validos (arvore + valor) ====="
for caso in "${validos[@]}"; do
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

# arquivo | descrição do erro
erros=(
  "tests/sin/e1.ec1|operando direito ausente"
  "tests/sin/e2.ec1|parentese de fechamento ausente"
  "tests/sin/e3.ec1|tokens sobrando apos a expressao"
  "tests/sin/e4.ec1|parenteses sem expressao"
  "tests/sin/e5.ec1|operador ausente"
  "tests/sin/e6.ec1|caractere invalido (!)"
  "tests/sin/e7.ec1|letras no lugar de numero"
)

echo "===== Erros de sintaxe (devem ser detectados) ====="
for caso in "${erros[@]}"; do
  IFS='|' read -r arq desc <<< "$caso"
  saida=$("$BIN" "$arq" 2>&1)
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
