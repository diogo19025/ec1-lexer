#!/usr/bin/env bash
# Testes de analise semantica para a linguagem Cmd (Atividade 09 - parte 3).
# Roda o compilador no modo de analise (sem --compilar, ja que a geracao de
# codigo para Cmd e responsabilidade da parte 4) sobre cada programa em
# tests/cmd/, conferindo que os programas validos (v*.ec1) sao aceitos e que
# os programas com variavel nao declarada (e*.ec1) sao rejeitados com um
# erro semantico.
set -u
cd "$(dirname "$0")/.."

BIN=./ec1
echo "==> Compilando o compilador..."
g++ -std=c++17 -Wall -Wextra src/*.cpp -o "$BIN" \
    || { echo "Falha na compilacao"; exit 1; }
echo

passou=0
falhou=0

echo "===== Programas validos: atribuicao, condicao e repeticao (Atividade 09) ====="
for arq in tests/cmd/v*.ec1; do
  saida=$("$BIN" "$arq" 2>&1)
  rc=$?
  if [ $rc -eq 0 ]; then
    echo "  [PASS] $arq"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  (deveria ser aceito, mas foi rejeitado)"
    echo "         $saida" | tail -1
    falhou=$((falhou+1))
  fi
done
echo

echo "===== Erros semanticos: variavel nao declarada em comandos (devem ser rejeitados) ====="
for arq in tests/cmd/e*.ec1; do
  saida=$("$BIN" "$arq" 2>&1)
  rc=$?
  if [ $rc -ne 0 ] && printf '%s\n' "$saida" | grep -qi "erro semantico"; then
    echo "  [PASS] $arq"
    passou=$((passou+1))
  else
    echo "  [FAIL] $arq  (deveria gerar erro semantico, rc=$rc)"
    falhou=$((falhou+1))
  fi
done
echo

echo "===== Resumo semantica Cmd: $passou passaram, $falhou falharam ====="
[ $falhou -eq 0 ]
