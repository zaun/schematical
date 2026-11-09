#!/bin/zsh
# plan-29 census, step 1: list every in-stock Active / Last Time Buy part of the
# four diode categories for the three chosen makers.
# Raw DigiKey JSON (one line per page) goes to $RAW/<category>-<maker>.jsonl.
#   usage: list.sh <raw-dir>
# Needs DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET (read from ~/.zshrc).
# Categories: 280 Single Diodes, 286 Diode Arrays, 299 Bridge Rectifiers,
# 287 Single Zener Diodes, 295 Zener Diode Arrays.
# Makers (ids from the manufacturer facet of
#   scripts/digikey.js --category 280 --in-stock --pages 1 --json):
# 112 Vishay General Semiconductor - Diodes Division, 488 onsemi,
# 1801 Taiwan Semiconductor Corporation.
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
RAW="$1"
mkdir -p "$RAW"
cd "$(dirname "$0")/../../.." || exit 1
for c in 280 286 299 287 295; do
  for m in 112:vishay 488:onsemi 1801:tsc; do
    id="${m%%:*}"
    name="${m##*:}"
    out="$RAW/$c-$name"
    [ -s "$out.done" ] && continue
    node scripts/digikey.js --category "$c" --manufacturer "$id" --in-stock \
      --status 0 --status 4 --limit 20000 --json > "$out.jsonl" 2> "$out.err" &&
      tail -1 "$out.err" > "$out.done"
    echo "$c $name: $(tail -1 "$out.err")"
    grep -i 'cut short\|warn' "$out.err"
  done
done
