#!/bin/zsh
# plan-51 census: lists category 409 (IC Sockets) for each candidate maker with list.js.
# usage: list.sh <raw dir>
# One <maker id>.jsonl and .err per maker; a maker whose .done exists is skipped, so a run
# cut by the daily quota goes on the next day.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
# 54 Mill-Max, 1212 Preci-Dip, 43 Aries, 19 3M, 17 TE AMP, 123 Assmann WSW, 612 Samtec,
# 2057 Adam Tech, 609 Amphenol ICC, 52 On Shore, 1175 CNC Tech, 952 Harwin: the makers of
# category 409 with a DIP or PLCC socket in stock (plan-51-A § 2).
makers=(54 1212 43 19 17 123 612 2057 609 52 1175 952)
for m in $makers; do
  f="$raw/$m"
  [[ -e $f.done ]] && continue
  if node "$here/list.js" $m $f.jsonl 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $m failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#makers} makers done"
