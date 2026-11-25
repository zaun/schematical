#!/bin/zsh
# plan-55 census: lists every (family, maker label) piece with list.js.
# usage: list.sh <raw dir> [--opt2]
# One <piece>.jsonl and .err per piece; a piece whose .done exists is skipped, so a run cut by the daily
# quota goes on the next day. --opt2 uses the second DigiKey key (its own daily quota).
# The 14 pieces (plan-55-B § 3.1) are family-<maker id>, the three families of common.js:
#   ideal   (category 758): Texas Instruments 296, Analog Devices 505 (Analog Devices Inc.) + 175 (Maxim),
#           Monolithic Power Systems 1589
#   load    (category 726): Texas Instruments 296, onsemi 488 + 600039 (Fairchild), Microchip 150 + 576 (Micrel)
#   limited (category 726): Texas Instruments 296 + 14 (National), Diodes Incorporated 31, Microchip 150 + 576
raw=${1:?usage: list.sh <raw dir> [--opt2]}
opt2=${2:-}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)2?=' ~/.zshrc)"
pieces=(ideal-296 ideal-505 ideal-175 ideal-1589 load-296 load-488 load-600039 load-150 load-576 limited-296 limited-14 limited-31 limited-150 limited-576)
for p in $pieces; do
  f="$raw/$p"
  [[ -e $f.done ]] && continue
  if node "$here/list.js" $p $f.jsonl $opt2 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $p failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#pieces} pieces done"
