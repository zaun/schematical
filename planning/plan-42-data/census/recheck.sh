#!/bin/zsh
# plan-42 census: the deeper re-listings of the pieces select.py --report marks
# NEEDS DEEPER LISTING. Each lists the piece's type value again with
# MinimumQuantityAvailable = the pick's floor for that (category, maker) at the
# time of the run (a part with less stock cannot be picked: the floor only rises
# as rows are added), split by Output Type then Package / Case (41,16).
# Output: <category>-<maker id>-re.jsonl / .err (select.py reads it first and
# drops the base file's pieces).
# usage: recheck.sh <raw dir>
raw=${1:?usage: recheck.sh <raw dir>}
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
run() {  # <category> <maker id> <floor>
  local f="$raw/$1-$2-re"
  [[ -e $f.done ]] && return
  node "$here/top.js" $1 $2 $f.jsonl 41,16 '' $3 2> $f.err && touch $f.done || { echo "recheck.sh: $1-$2 failed" >&2; exit 1; }
}
# The planning-time floors (2026-11-15, select.py --report):
run 699 296 35427
run 699 505 2334
echo "recheck.sh: done"
