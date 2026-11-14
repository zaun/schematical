#!/bin/zsh
# plan-34-A § 5 rule 2: look up each picked row from a cut piece by its own
# part number (one DigiKey call each) and append a row to <stock-check.tsv>:
# mpn, found (1 = a product with exactly that part number has stock above 0),
# stock, status, date. Part numbers already in the file are skipped, so a run
# the quota cuts goes on the next day.
# usage: stock-check.sh <raw dir> <candidates.tsv> <stock-check.tsv>
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
here=${0:A:h}
raw=$1 cand=$2 out=$3
dk=$here/../../../scripts/digikey.js
[[ -e $out ]] || print -r -- $'mpn\tfound\tstock\tstatus\tdate' > $out
python3 -I $here/cut-picked.py $raw $cand $out | while IFS=$'\t' read -r id mpn; do
  json=$(node $dk --manufacturer $id --in-stock --status 0 --status 4 --limit 5 --json "$mpn" 2> $raw/stock-check.last < /dev/null)
  left=$(sed -n 's/.* \([0-9][0-9]*\) left today.*/\1/p' $raw/stock-check.last | tail -1)
  if [[ -n $left && $left -lt ${STOCK_CHECK_RESERVE:-20} ]]; then
    print -r -- "stock-check: stopped with $left calls left today (reserve ${STOCK_CHECK_RESERVE:-20}); run again tomorrow"
    # this row's answer is still good: fall through and record it, then stop
    stop=1
  fi
  if grep -q 'rate limited\|429' $raw/stock-check.last; then
    cat $raw/stock-check.last; exit 1
  fi
  if [[ -z $json ]] && ! grep -q '^digikey:' $raw/stock-check.last; then
    print -r -- "stock-check: $mpn failed: $(cat $raw/stock-check.last)"; exit 1
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
        if p["ManufacturerProductNumber"] == mpn and p["QuantityAvailable"] > 0:
            hit = hit or p
st = hit["ProductStatus"]["Status"] if hit else ""
print("\t".join([mpn, "1" if hit and st in ("Active", "Last Time Buy") else "0",
                 str(hit["QuantityAvailable"]) if hit else "0", st,
                 datetime.date.today().isoformat()]))
' "$mpn" >> $out
  [[ -n $stop ]] && break
done
tail -1 $raw/stock-check.last 2>/dev/null
