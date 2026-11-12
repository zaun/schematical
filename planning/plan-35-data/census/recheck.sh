#!/bin/zsh
# plan-35-A: part-by-part stock and status re-check, one exact-part search per
# "add" row of candidates.tsv (68 API calls). usage: recheck.sh <candidates.tsv> <out.tsv>
# Writes: part <TAB> status <TAB> stock (empty stock = not listed in stock).
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
repo=${0:A:h:h:h:h}
: > $2
awk -F'\t' '$4=="add"{print $2}' $1 | while read -r pn; do
  node $repo/scripts/digikey.js --in-stock --limit 5 --json "$pn" 2>/dev/null |
    python3 -I -c 'import json,sys
pn=sys.argv[1]; hit=None
for l in sys.stdin:
    for p in json.loads(l).get("Products",[]):
        if p["ManufacturerProductNumber"]==pn: hit=p
print(pn+"\t"+(hit["ProductStatus"]["Status"] if hit else "-")+"\t"+(str(hit.get("QuantityAvailable",0)) if hit else ""))' "$pn" >> $2
done
awk -F'\t' '$3=="" || $3==0 || ($2!="Active" && $2!="Last Time Buy")' $2
