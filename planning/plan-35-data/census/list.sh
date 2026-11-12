#!/bin/zsh
# plan-35 census listing (run 2026-11-11). usage: list.sh <outdir>
# Category 99 = "LCD, OLED Character and Numeric". Manufacturer ids:
# 2632 Focus LCDs, 757 Newhaven Display Intl, 3086 Midas Displays.
# Needs DIGIKEY_CLIENT_ID/SECRET from ~/.zshrc; 54 API calls in total.
set -e
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
repo=${0:A:h:h:h:h}
out=$1; mkdir -p $out
node $repo/scripts/digikey.js --category 99 --in-stock --limit 2000 --json > $out/cat99-instock.jsonl 2> $out/cat99-instock.err
for m in 2632 757 3086; do
  node $repo/scripts/digikey.js --category 99 --manufacturer $m --limit 3000 --json > $out/cat99-m$m-all.jsonl 2> $out/cat99-m$m-all.err
done
cat $out/*.err
