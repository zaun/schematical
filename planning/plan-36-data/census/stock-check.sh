#!/bin/bash
# plan-36-A § 3 rule 1: look up each in-scope row with no top-level stock figure
# by its own part number. One DigiKey call per part.
# usage: stock-check.sh <cand.tsv> <out.tsv>
# out: mpn found stock date  (found = 1 when a product with exactly that
# ManufacturerProductNumber has QuantityAvailable > 0 or a ProductVariations
# entry with QuantityAvailableforPackageType > 0)
here=$(cd "$(dirname "$0")" && pwd)
cand=$1; out=$2; tmp=/tmp/p36/stock; mkdir -p "$tmp" || exit 1
printf 'mpn\tfound\tstock\tdate\n' > "$out"
awk -F'\t' 'NR>1 && $5=="in" && $11=="" {print $1"\t"$2}' "$cand" | while IFS=$'\t' read -r mpn maker; do
  case "$maker" in
    "Microchip Technology") id=150 ;; "Espressif Systems") id=1904 ;; "Raspberry Pi") id=1690 ;; *) id= ;;
  esac
  f="$tmp/$(printf '%s' "$mpn" | tr '/()' '___').json"
  [ -s "$f" ] || /tmp/p36/dk.sh --manufacturer "$id" --in-stock --limit 5 --json "$mpn" > "$f" 2> "$f.err"
  python3 -I "$here/stock-parse.py" "$mpn" "$f" >> "$out"
done
tail -1 "$tmp"/*.err 2>/dev/null | grep 'left' | tail -1
