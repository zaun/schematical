#!/bin/zsh
# plan-40 census: lists every (category, maker id) piece with top.js.
# usage: list.sh <raw dir>
# One <category>-<maker id>.jsonl and .err per piece; a piece whose .done
# exists is skipped, so a run cut by the daily quota goes on the next day.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
# category: 276 single BJT, 277 BJT arrays, 292 single pre-biased BJT,
# 293 pre-biased BJT arrays, 278 single FET/MOSFET, 289 MOSFET arrays.
# makers: 488 onsemi, 600039 Fairchild (onsemi), 1727 Nexperia, 31 Diodes,
# 448 Infineon, 21 Infineon Americas, 600047 International Rectifier
# (Infineon), 742 Vishay Siliconix, 600092 Siliconix (Vishay Siliconix).
pieces=()
for c in 276 277 292 293; do for m in 488 600039 1727 31; do pieces+=("$c $m"); done; done
for c in 278 289; do for m in 488 600039 448 21 600047 742 600092; do pieces+=("$c $m"); done; done
for p in $pieces; do
  c=${p% *}; m=${p#* }
  f="$raw/$c-$m"
  [[ -e $f.done ]] && continue
  if node "$here/top.js" $c $m $f.jsonl 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $c-$m failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#pieces} pieces done"
