#!/bin/zsh
# plan-44 census: lists every (category, maker id) piece with list.js.
# usage: list.sh <raw dir>
# One <category>-<maker id>.jsonl and .err per piece; a piece whose .done
# exists is skipped, so a run cut by the daily quota goes on the next day.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
# category: 171 Crystals, 174 Resonators. makers: 535 Abracon LLC, 50 ECS
# Inc., 2388 Suntsu Electronics, Inc (the three with most in-stock Active /
# Last Time Buy parts in the two categories, plan-44-A § 2). Suntsu has no
# resonator.
pieces=("171 535" "171 50" "171 2388" "174 535" "174 50")
for p in $pieces; do
  c=${p% *}; m=${p#* }
  f="$raw/$c-$m"
  [[ -e $f.done ]] && continue
  if node "$here/list.js" $c $m $f.jsonl 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $c-$m failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#pieces} pieces done"
