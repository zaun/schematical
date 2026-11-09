#!/bin/bash
mkdir -p /tmp/p24/dk
cd /tmp/p24/dk || exit 1
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
DK=/Users/justinzaun/Development/logic/.claude/worktrees/P-24/scripts/digikey.js
cat=$1; shift
for k in "$@"; do
  node $DK --category $cat --manufacturer 296 --in-stock --status 0 --json "$k" > "kwj-$cat-$k.jsonl" 2> "kwj-$cat-$k.err"
  echo "$k: $(tail -1 "kwj-$cat-$k.err")"
done
