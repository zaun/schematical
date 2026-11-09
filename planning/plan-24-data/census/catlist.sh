#!/bin/bash
# category listings, in-stock TI, per status; output in /tmp/p24/dk
mkdir -p /tmp/p24/dk
cd /tmp/p24/dk || exit 1
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
DK=/Users/justinzaun/Development/logic/.claude/worktrees/P-24/scripts/digikey.js
for cat in 700 701 690; do
  node $DK --category $cat --manufacturer 296 --in-stock --status 0 --list > list-$cat-active.tsv 2> list-$cat-active.err
  tail -3 list-$cat-active.err
  node $DK --category $cat --manufacturer 296 --in-stock --status "Last Time Buy" --list > list-$cat-ltb.tsv 2> list-$cat-ltb.err
  tail -3 list-$cat-ltb.err
done
wc -l list-*.tsv
