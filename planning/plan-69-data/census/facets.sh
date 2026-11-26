#!/bin/zsh
# plans 69-74 (planning, 2026-11-25): one DigiKey category page through Safari (AGENTS.md: stock in Safari),
# filtered In Stock (-2 = 5) + Active (1989 = 0) + optional extra filters, saved as the page's own data
# (productCount, every facet with its options' productCount, the first 100 products).
#   census/facets.sh <out.json> <slug/id, e.g. x/710> ['{"578":[325925]}']
# Needs lz-string: mkdir /tmp/lz && cd /tmp/lz && npm i lz-string; then LZ=/tmp/lz/node_modules census/facets.sh ...
# Show the counts: python3 -I census/show.py <out.json> Manufacturer Protocol
HERE=${0:A:h}; ROOT=${HERE:h:h:h}; cd "$ROOT"
U=$(NODE_PATH=${LZ:-/tmp/lz/node_modules} node "$HERE/url.js" "$2" "$3")
JS='var d=JSON.parse(document.getElementById("__NEXT_DATA__").textContent).props.pageProps.envelope.data; JSON.stringify({n:d.productCount,f:d.filters.concat(d.commonFilters||[]).map(g=>({id:g.key,name:g.label,o:(g.options||[]).map(o=>[o.text,o.value,o.productCount])})),p:d.products})'
for t in 1 2 3 4; do scripts/safari-digikey.sh "$U" 10 "$JS" > "$1" 2>/dev/null; python3 -I -c "import json,sys; d=json.load(open(sys.argv[1])); sys.exit(0 if d['n'] is not None else 1)" "$1" 2>/dev/null && exit 0; sleep 3; done; echo "FAILED $2 $3"; exit 1
