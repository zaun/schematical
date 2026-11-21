#!/bin/zsh
# plan-42-B/C: re-run the pick after a verdict change (a new override, a scheme).
# Restores the placed PDFs of every download-set row from the download cache
# (the prune deleted those no `add` row used), re-reads dead flags, makes the
# verdicts, runs the pick with --verdicts, then prunes the PDFs again.
# usage: refresh.sh <raw dir> <pdf cache dir>
here=${0:A:h}
data=$here/..
raw=${1:-/tmp/p42/raw} pdf=${2:-/tmp/p42/pdf}
C=$data/candidates.tsv
set -e
python3 -I $here/textscan.py $C $pdf $C
python3 -I $here/sibling.py $C $pdf $C
python3 -I $here/place.py $C $pdf
python3 -I $here/dead-merge.py > /dev/null
python3 -I $here/specs-skeleton.py $C $data/specs.tsv
python3 -I $here/verdicts.py $C $data/specs.tsv $data/verdicts.tsv
python3 -I $here/select.py $raw /tmp/p42/cand-final.tsv --stock $data/stock-check.tsv --carry $C \
  --verdicts $data/verdicts.tsv --report | tail -16
cp /tmp/p42/cand-final.tsv $C
python3 -I $here/prune.py
