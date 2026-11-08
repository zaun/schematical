#!/bin/bash
# dl.sh <keysfile> <outdir> : download ti.com/lit/ds/symlink/<key>.pdf for every key (col 1)
keys=$1; out=$2
mkdir -p "$out"
cut -f1 "$keys" | xargs -P 8 -I{} sh -c 'f="'"$out"'/{}.pdf"; [ -s "$f" ] || curl -sfL --max-time 600 -A "Mozilla/5.0" -o "$f" "https://www.ti.com/lit/ds/symlink/{}.pdf" || { rm -f "$f"; echo "FAIL {}"; }'
echo done
