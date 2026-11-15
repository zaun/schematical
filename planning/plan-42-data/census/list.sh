#!/bin/zsh
# plan-42 census: lists every (category, maker id) piece with top.js.
# usage: list.sh <raw dir>
# One <category>-<maker id>.jsonl and .err per piece; a piece whose .done
# exists is skipped, so a run cut by the daily quota goes on the next day.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
# category: 699 Voltage Regulators - Linear, Low Drop Out (LDO) Regulators,
# 739 Voltage Regulators - DC DC Switching Regulators.
# makers: 296 Texas Instruments, 14 National Semiconductor and 600062 Burr
# Brown (TI); 505 Analog Devices Inc., 175 Analog Devices Inc./Maxim
# Integrated (ADI); 150 Microchip Technology, 576 Micrel Inc. (Microchip).
pieces=()
for c in 699 739; do for m in 296 14 600062 505 175 150 576; do pieces+=("$c $m"); done; done
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
