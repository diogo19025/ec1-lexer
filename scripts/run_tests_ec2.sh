#!/usr/bin/env bash
# Testes da Atividade 07 (EC2): precedência e associatividade de operadores.
# Verifica que expressões SEM parênteses obrigatórios são analisadas com a
# árvore correta (checando a árvore linear e o valor interpretado).
set -u
cd "$(dirname "$0")/.."

BIN=./ec1
echo "==> Compilando..."
g++ -std=c++17 -Wall -Wextra src/*.cpp -o "$BIN" \
    || { echo "Falha na compilacao"; exit 1; }
echo

passou=0
falhou=0

# arquivo | arvore esperada | valor esperado | o que prova
casos=(
  "tests/ec2/prec_mul_add.ec1|(7 + (5 * 3))|22|precedencia: * antes de +"
  "tests/ec2/prec_add_mul.ec1|((2 * 3) + 4)|10|precedencia: * antes de +"
  "tests/ec2/prec_sub_mul.ec1|(20 - (2 * 3))|14|precedencia: * antes de -"
  "tests/ec2/assoc_add.ec1|((7 + 5) + 3)|15|associatividade a esquerda (+)"
  "tests/ec2/assoc_sub.ec1|((10 - 8) - 2)|0|associatividade a esquerda (-)"
  "tests/ec2/assoc_div.ec1|((100 / 10) / 2)|5|associatividade a esquerda (/)"
  "tests/ec2/assoc_muldiv.ec1|((8 / 4) * 2)|4|mesma precedencia, esq->dir"
  "tests/ec2/mix_levels.ec1|((2 + (3 * 4)) - 5)|9|niveis de precedencia misturados"
  "tests/ec2/paren_over.ec1|(2 * (3 + 4))|14|parenteses sobrepoem a precedencia"
  "tests/ec2/paren_mix.ec1|(((1 + 2) * 3) + 4)|13|parenteses + precedencia"
  "tests/ec2/const.ec1|42|42|constante isolada (exp_a->exp_m->prim)"
)

echo "===== Expressoes sem parenteses obrigatorios (EC2) ====="
for caso in "${casos[@]}"; do
  IFS='|' read -r arq arvore valor desc <<< "$caso"
  entrada=$(tr -d '\r\n' < "$arq")
  saida=$("$BIN" "$arq" 2>/dev/null)
  rc=$?
  if [ $rc -eq 0 ] \
     && printf '%s\n' "$saida" | grep -qF "Arvore (linear): $arvore" \
     && printf '%s\n' "$saida" | grep -qF "Valor: $valor"; then
    echo "  [PASS] $entrada  ->  $arvore = $valor   ($desc)"
    passou=$((passou+1))
  else
    echo "  [FAIL] $entrada  (esperado: $arvore = $valor, rc=$rc)  ($desc)"
    falhou=$((falhou+1))
  fi
done
echo

echo "===== Resumo EC2: $passou passaram, $falhou falharam ====="
[ $falhou -eq 0 ]
