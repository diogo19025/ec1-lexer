#!/usr/bin/env bash
# Testes de geração de código para a linguagem EV (Atividade 08).
# Compila cada programa .ec1 (com --compilar), monta, linka, executa e
# compara a saída com o valor esperado. Também verifica que programas com
# variáveis não declaradas são rejeitados pela análise semântica.
# Requer: as (GNU Assembler) e ld disponíveis no PATH (Linux x86-64).
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
  "tests/ev/v1_perimetro.ec1|140|perimetro de retangulo (2 variaveis)"
  "tests/ev/v2_guia_xy.ec1|60467|exemplo do guia: variavel usa outra variavel"
  "tests/ev/v3_tabela_simbolos.ec1|33577|exemplo da tabela de simbolos do guia"
  "tests/ev/v4_sem_declaracoes.ec1|7|programa sem nenhuma declaracao"
  "tests/ev/v5_uma_variavel.ec1|100|variavel usada duas vezes na mesma expressao"
  "tests/ev/v6_encadeado.ec1|10|declaracoes encadeadas (a,b,c,d)"
  "tests/ev/v7_subtracao_negativa.ec1|-3|resultado negativo (verifica sinal)"
)

echo "===== Geracao de codigo com variaveis (Atividade 08) ====="
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
  "tests/ev/e1_var_nao_declarada_final.ec1|variavel usada na expressao final sem ter sido declarada"
  "tests/ev/e2_var_nao_declarada_em_decl.ec1|variavel usada dentro de uma declaracao antes de ser declarada"
  "tests/ev/e3_guia_duas_nao_declaradas.ec1|exemplo do guia: duas referencias a variaveis nao declaradas"
)

echo "===== Erros semanticos: variavel nao declarada (devem ser rejeitados) ====="
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
rm -f tests/ev/*.s tests/ev/*.o
for caso in "${validos[@]}"; do
  IFS='|' read -r arq _ <<< "$caso"
  rm -f "${arq%.ec1}"
done

echo "===== Resumo EV: $passou passaram, $falhou falharam ====="
[ $falhou -eq 0 ]
