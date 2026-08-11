#!/usr/bin/env bash
set -u
cd "$(dirname "$0")/.."

BIN=./ec1
passou=0
falhou=0

validos=(
  "tests/array/v1_global.ec1|60|array global com indice calculado"
  "tests/array/v2_local_funcao.ec1|14|array local dentro de funcao"
  "tests/array/v3_sombreamento.ec1|15|array local escondendo array global"
)

erros=(
  "tests/array/e1_indice_fora.ec1|indice literal fora dos limites"
  "tests/array/e2_escalar_indexado.ec1|variavel escalar usada como array"
  "tests/array/e3_array_sem_indice.ec1|array atribuido sem indice"
)

echo "===== Arrays: testes ponta a ponta ====="
for caso in "${validos[@]}"; do
  IFS='|' read -r arquivo esperado descricao <<< "$caso"
  base="${arquivo%.ec1}"

  if ! "$BIN" --compilar "$arquivo" >/dev/null 2>&1; then
    echo "  [FAIL] $arquivo (geracao de assembly: $descricao)"
    falhou=$((falhou + 1))
    continue
  fi
  if ! as --64 -I src -o "${base}.o" "${base}.s" 2>/dev/null; then
    echo "  [FAIL] $arquivo (montagem: $descricao)"
    falhou=$((falhou + 1))
    continue
  fi
  if ! ld -o "$base" "${base}.o" 2>/dev/null; then
    echo "  [FAIL] $arquivo (ligacao: $descricao)"
    falhou=$((falhou + 1))
    continue
  fi

  resultado=$("$base" 2>/dev/null)
  if [ "$resultado" = "$esperado" ]; then
    echo "  [PASS] $arquivo -> $resultado ($descricao)"
    passou=$((passou + 1))
  else
    echo "  [FAIL] $arquivo (esperado: $esperado, obtido: $resultado)"
    falhou=$((falhou + 1))
  fi
done

echo "===== Arrays: erros semanticos ====="
for caso in "${erros[@]}"; do
  IFS='|' read -r arquivo descricao <<< "$caso"
  saida=$("$BIN" --compilar "$arquivo" 2>&1)
  codigo=$?
  if [ "$codigo" -ne 0 ] && printf '%s\n' "$saida" | grep -qi "erro semantico"; then
    echo "  [PASS] $arquivo ($descricao)"
    passou=$((passou + 1))
  else
    echo "  [FAIL] $arquivo (erro nao detectado: $descricao)"
    falhou=$((falhou + 1))
  fi
done

rm -f tests/array/*.s tests/array/*.o
for caso in "${validos[@]}"; do
  IFS='|' read -r arquivo _ <<< "$caso"
  rm -f "${arquivo%.ec1}"
done

echo "===== Resumo arrays: $passou passaram, $falhou falharam ====="
[ "$falhou" -eq 0 ]
