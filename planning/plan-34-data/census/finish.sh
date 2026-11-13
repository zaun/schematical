#!/bin/zsh
# plan-34-A Phase 2 (added task) and the reconciliation after the stock
# check: run once stock-check.tsv covers every picked row from a cut piece.
#   usage: finish.sh            (from anywhere; paths are the worktree's)
# Steps: (1) the final pick with the stock check and the verdicts; stop if a
# newly picked row from a cut piece is unchecked (run stock-check.sh on
# /tmp/p34/pick.tsv, then this again). (2) A Phases 3-4 again on the
# stock-checked rows (fetch, textscan, sibling, place after clearing
# datasheets/resistor/, specs-skeleton merge) and their checks. (3) B:
# verdicts, the final pick into candidates.tsv, skipped-parts.md, prune,
# footprints-needed.txt. FINISH_DRY=1: skip the unchecked-rows stops and end
# before (6) (a rehearsal on a copy of the worktree). (4) C/D: fp-check per category (stop on a line with
# no footprint: draw it). (5) specs-fill. (6) F: gen-parts, goldens, ctest.
set -e
here=${0:A:h}
data=${here:h}
repo=${data:h:h}
raw=/tmp/p34/raw
st=$data/stock-check.tsv
py() { python3 -I "$@"; }
cd $repo
cp /tmp/p34/stock-check.tsv $st

echo "== (1) final pick"
py $here/select.py $raw /tmp/p34/pick.tsv --stock $st --verdicts $data/verdicts.tsv >/dev/null
py $here/cut-picked.py $raw /tmp/p34/pick.tsv $st > /tmp/p34/unchecked.txt
if [[ -s /tmp/p34/unchecked.txt && -z $FINISH_DRY ]]; then
  echo "unchecked picked rows: $(wc -l < /tmp/p34/unchecked.txt); run stock-check.sh $raw /tmp/p34/pick.tsv /tmp/p34/stock-check.tsv"
  exit 3
fi

echo "== (2) plan-34-A Phases 3-4 on the stock-checked rows"
py $here/select.py $raw /tmp/p34/cand.tsv --stock $st >/dev/null
py $here/fetch.py /tmp/p34/cand.tsv /tmp/p34/pdf
py $here/textscan.py /tmp/p34/cand.tsv /tmp/p34/pdf /tmp/p34/cand2.tsv
py $here/sibling.py /tmp/p34/cand2.tsv /tmp/p34/pdf /tmp/p34/cand3.tsv
rm -rf $repo/datasheets/resistor
cp /tmp/p34/cand3.tsv $data/candidates.tsv
py $here/place.py $data/candidates.tsv /tmp/p34/pdf
py $here/place.py --verify $data/candidates.tsv
py $here/specs-skeleton.py $data/candidates.tsv $data/specs.tsv
n1=$(awk -F'\t' 'NR>1 && $21!="" && $23!="automotive" && $23!="current-sense" {print $2"|"$18"|"$21}' $data/candidates.tsv | sort -u | wc -l)
n2=$(awk 'END{print NR-1}' $data/specs.tsv)
echo "spec groups $n1, specs.tsv rows $n2"; [[ $n1 -eq $n2 ]]
empty=$(awk -F'\t' 'NR>1 && ($4=="" || $5=="")' $data/specs.tsv | wc -l)
if [[ $empty -gt 0 ]]; then
  echo "specs.tsv rows without a scheme: $empty (plan-34-B Phase 1: write them)"; exit 4
fi

echo "== (3) plan-34-B"
py $here/verdicts.py $data/candidates.tsv $data/specs.tsv $data/verdicts.tsv
cp $data/candidates.tsv /tmp/p34/cand-a.tsv
py $here/select.py $raw $data/candidates.tsv --stock $st --verdicts $data/verdicts.tsv --report | grep -v ' -> complete'
py $here/cut-picked.py $raw $data/candidates.tsv $st > /tmp/p34/unchecked.txt
if [[ -s /tmp/p34/unchecked.txt && -z $FINISH_DRY ]]; then
  echo "unchecked picked rows after the verdicts: $(wc -l < /tmp/p34/unchecked.txt); stock-check them, then run again"; exit 3
fi
echo "candidate rows left: $(awk -F'\t' '$22=="candidate"' $data/candidates.tsv | wc -l)"
py $here/verdicts.py --write-skipped $data/candidates.tsv
py $here/verdicts.py --verify-skipped $data/candidates.tsv
py $here/place.py --prune $data/candidates.tsv | wc -l
py $here/place.py --verify-add $data/candidates.tsv
py $here/footprint-lines.py $data/candidates.tsv > $data/footprints-needed.txt

echo "== (4) plan-34-C/D footprints"
for c in chip through-hole array; do py $here/fp-check.py $c; done

echo "== (5) specs-fill"
py $here/specs-fill.py $data/candidates.tsv

[[ -n $FINISH_DRY ]] && { echo "dry run: stop before (6)"; exit 0; }
echo "== (6) plan-34-F"
# datasheets.json and footprints.json as they were before plan-34-F Phase 2
# (3860182), so an entry the final pick no longer uses does not stay.
for f in datasheets footprints; do
  git -C $repo show 3860182^:schematical/schematical/Parts/$f.json > schematical/schematical/Parts/$f.json
done
py $here/fp-merge.py schematical/schematical/Parts/footprints.json $data/footprints-C.json $data/footprints-D.json
py $here/gen-parts.py $data/candidates.tsv
cmake --build /tmp/logic-p34-wt -j8 > /dev/null
/tmp/logic-p34-wt/listing_check --print > backend/tests/golden/parts.txt
/tmp/logic-p34-wt/trace_check --print > backend/tests/golden/traces.txt
/tmp/logic-p34-wt/footprint_check --print > backend/tests/golden/footprints.txt
git diff --stat backend/tests/golden
ctest --test-dir /tmp/logic-p34-wt -j8 | tail -3
echo "resistor parts: $(grep -c '|resistor|' backend/tests/golden/parts.txt); add rows: $(awk -F'\t' '$22=="add"' $data/candidates.tsv | wc -l)"
