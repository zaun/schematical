#!/bin/zsh
# plan-40-A phase 1: the deeper re-listings of the pieces `select.py --report`
# marks NEEDS DEEPER LISTING. Each runs top.js on the whole (category, maker)
# piece with more split facets (74 Transistor Type, 606 FET Type, 16 Package /
# Case), so a piece above 300 is split to pieces of at most 300 (or 600, listed
# from both ends). The result is <category>-<maker id>-re.jsonl, which
# select.py reads before the base file. A piece whose .done exists is skipped,
# so a run cut by the daily quota goes on the next day.
# usage: recheck.sh <raw dir>
raw=${1:?usage: recheck.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
run() {  # <category> <maker id> <split facet ids>
  local f="$raw/$1-$2-re"
  [[ -e $f.done ]] && return 0
  if node "$here/top.js" $1 $2 $f.jsonl $3 2> $f.err; then
    touch $f.done
  else
    echo "recheck.sh: $1-$2 failed (see $f.err)" >&2
    exit 1
  fi
}
run 276 488 74,16     # onsemi single BJTs, 1543 parts
run 278 488 606,16    # onsemi single FETs, 2233 parts
run 278 742 606,16    # Vishay Siliconix single FETs, 1952 parts
run 278 448 606,16    # Infineon single FETs, 2173 parts (plan-40-B: skips lowered its floor below the cut)
echo "recheck.sh: done"
