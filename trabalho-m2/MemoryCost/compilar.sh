#!/usr/bin/env bash
set -euo pipefail

SRC="main.cpp"
TMP="build_main.cpp"
OUT="./main"

if [ ! -f "$SRC" ]; then
  echo "Arquivo $SRC não encontrado." >&2
  exit 1
fi

echo "Preparando cópia sanitizada de $SRC..."
# Remove include do precompiled header do Windows e aplica correções pontuais para compilar no Linux.
sed '/#include "stdafx.h"/d' "$SRC" > "$TMP"
sed -i 's/start = clock.now()/start = std::chrono::steady_clock::now()/g' "$TMP"
sed -i 's/auto end = clock.now()/auto end = std::chrono::steady_clock::now()/g' "$TMP"
sed -i '/std::chrono::steady_clock clock;/d' "$TMP"
sed -i 's/Timer operator=(const Timer*) = delete;/Timer& operator=(const Timer&) = delete;/g' "$TMP"

echo "Compilando $SRC..."
g++ -std=c++17 -O2 -Wall -Wextra "$TMP" -o "$OUT"

echo "Execução de $OUT:"
"$OUT"

# Remover arquivo temporário
rm -f "$TMP"