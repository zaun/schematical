#!/bin/bash
# kw.sh <category> <keyword>...  : in-stock Active TI keyword listings into /tmp/p24/dk/kw-<cat>-<kw>.tsv
mkdir -p /tmp/p24/dk
cd /tmp/p24/dk || exit 1
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
DK=/Users/justinzaun/Development/logic/.claude/worktrees/P-24/scripts/digikey.js
cat=$1; shift
for k in "$@"; do
  node $DK --category $cat --manufacturer 296 --in-stock --status 0 --list "$k" > "kw-$cat-$k.tsv" 2> "kw-$cat-$k.err"
  echo "$k: $(tail -1 "kw-$cat-$k.err")"
done
