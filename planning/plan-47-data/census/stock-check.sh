#!/bin/zsh
# plan-47-A section 5: look up each download-set row (and each fuse clip) by
# its own part number (one DigiKey call each) and append a row to
# <stock-check.tsv>: mpn, found (1 = a product with exactly that part number
# has stock above 0 and status Active or Last Time Buy), stock, status, date.
# Part numbers already in the file are skipped, so a run the quota cuts goes
# on the next day. Stops when fewer than STOCK_CHECK_RESERVE (default 20)
# calls are left today. STOCK_CHECK_MAX caps the calls of one run.
# usage: stock-check.sh <raw dir> <clips.jsonl> <candidates.tsv> <stock-check.tsv>
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
here=${0:A:h}
raw=$1 clips=$2 cand=$3 out=$4
dk=$here/../../../scripts/digikey.js
last=${TMPDIR:-/tmp}/p47-stock-check.last
[[ -e $out ]] || print -r -- $'mpn\tfound\tstock\tstatus\tdate' > $out
n=0
python3 -I $here/to-check.py $raw $clips $cand $out | while IFS=$'\t' read -r id mpn; do
  json=$(node $dk --manufacturer $id --in-stock --status 0 --status 4 --limit 5 --json "$mpn" 2> $last < /dev/null)
  left=$(sed -n 's/.* \([0-9][0-9]*\) left today.*/\1/p' $last | tail -1)
  if grep -q 'rate limited\|429' $last; then
    cat $last; exit 1
  fi
  if [[ -z $json ]] && ! grep -q '^digikey:' $last; then
    print -r -- "stock-check: $mpn failed: $(cat $last)"; exit 1
  fi
  print -r -- "$json" | python3 -I -c '
import json, sys, datetime
mpn = sys.argv[1]
hit = None
for line in sys.stdin:
    line = line.strip()
    if not line:
        continue
    d = json.loads(line)
    for p in (d.get("ExactMatches") or []) + (d.get("Products") or []):
        if p["ManufacturerProductNumber"] == mpn and (p.get("QuantityAvailable") or 0) > 0:
            hit = hit or p
st = hit["ProductStatus"]["Status"] if hit else ""
print("\t".join([mpn, "1" if hit and st in ("Active", "Last Time Buy") else "0",
                 str(hit["QuantityAvailable"]) if hit else "0", st,
                 datetime.date.today().isoformat()]))
' "$mpn" >> $out
  n=$((n + 1))
  if [[ -n $left && $left -lt ${STOCK_CHECK_RESERVE:-20} ]]; then
    print -r -- "stock-check: stopped with $left calls left today (reserve ${STOCK_CHECK_RESERVE:-20}); run again tomorrow"
    break
  fi
  if [[ -n $STOCK_CHECK_MAX && $n -ge $STOCK_CHECK_MAX ]]; then
    print -r -- "stock-check: stopped after $n calls (STOCK_CHECK_MAX)"
    break
  fi
done
tail -1 $last 2>/dev/null
