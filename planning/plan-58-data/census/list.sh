#!/bin/zsh
# plan-58-A: DigiKey listing of the slide-switch census through Safari (AGENTS.md: look stock up in Safari).
# usage: census/list.sh <raw-dir> [maker [circuit]]   (only that maker / circuit, e.g. NKK SPST: to redo one file)
# For each row of urls.tsv (maker, circuit, URL) loads the URL in a temporary tab of the Safari window that has a
# DigiKey page open (scripts/safari-digikey.sh explains the setup) and saves the page's own product data
# (__NEXT_DATA__ envelope: products, filters, productCount) to <raw-dir>/<maker>_<circuit>.json.
# A page that fails check.py (empty file, or another tab's page) is retried up to 5 times. Each page holds up to 100 products; every
# (maker, circuit) of the census has fewer. Prints "<maker> <circuit> count N rows M" per file.
RAW="$1"; ONLY="$2"; CIRC="$3"; HERE="${0:A:h}"; ROOT="${HERE:h:h:h}"
[ -n "$RAW" ] || { echo "usage: list.sh <raw-dir>"; exit 2; }
mkdir -p "$RAW"
JS='var d=JSON.parse(document.getElementById("__NEXT_DATA__").textContent).props.pageProps.envelope.data; JSON.stringify({n:d.productCount,f:d.filters.concat(d.commonFilters),p:d.products})'
while IFS=$'\t' read -r maker circuit url; do
  [ -z "$ONLY" -o "$ONLY" = "$maker" ] || continue
  [ -z "$CIRC" -o "$CIRC" = "$circuit" ] || continue
  out="$RAW/${maker}_${circuit}.json"
  ok=0
  for try in 1 2 3 4 5; do
    "$ROOT/scripts/safari-digikey.sh" "$url" 9 "$JS" > "$out" 2>/dev/null
    python3 -I "$HERE/check.py" "$out" "$maker" "$circuit" && { ok=1; break; }
    sleep 3
  done
  [ $ok = 1 ] || echo "$maker $circuit FAILED"
done < "$HERE/urls.tsv"
