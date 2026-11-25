#!/bin/zsh
# plan-55-B Phase 2: the stock re-check, a second listing (the plan-44-A one-call-per-part check is not needed:
# no piece is cut at the 300-result cap, so a listing sees every row).
# usage: stock-check.sh <raw-recheck dir> <candidates.tsv> <stock-check.tsv> [--opt2]
# Lists the 14 pieces again into <raw-recheck dir> (the facets files of planning/plan-55-data/raw are copied
# first: they save the two facet calls), then writes stock-check.tsv with stock.py. Needs the daily quota
# (about 35 calls), on a day later than the first listing (plan-55-B Prerequisites).
recheck=${1:?usage: stock-check.sh <raw-recheck dir> <candidates.tsv> <stock-check.tsv> [--opt2]}
cand=${2:?} out=${3:?} opt2=${4:-}
here=${0:A:h}
mkdir -p "$recheck"
for f in $here/../raw/facets-*.json; do
  [[ -e $recheck/${f:t} ]] || cp $f $recheck/
done
zsh $here/list.sh $recheck $opt2 || exit 1
python3 -I $here/stock.py $recheck $cand > $out
echo "stock-check: $(( $(wc -l < $out) - 1 )) rows, $(awk -F'\t' 'NR>1 && $3==1' $out | wc -l | tr -d ' ') found again"
