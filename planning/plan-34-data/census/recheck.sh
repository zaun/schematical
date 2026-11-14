#!/bin/zsh
# plan-34 § 5: re-list the pieces the first listing cut at 300 above the floor.
# usage: recheck.sh <raw dir>
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
here=${0:A:h}
raw=$1
run() { # <out stem> <cat> <maker> <args...>
  local stem=$1; shift
  [[ -e $raw/$stem.done ]] && return
  TOP_MIN_QTY=${MINQ:-1} node $here/top.js $1 $2 $raw/$stem.jsonl "${@:3}" 2> $raw/$stem.err && touch $raw/$stem.done
  tail -1 $raw/$stem.err
}
run 52-dale-re-1 52 541 '±1%'
run 52-yageo-re-1 52 13 '±1%' 39158 '0402 (1005 Metric)'
run 52-yageo-re-2 52 13 '±1%' 39246 '0603 (1608 Metric)'
run 53-yageo-re-1 53 13 '±1%' 317190 'Axial'
run 53-yageo-re-2 53 13 '±5%' 317190 'Axial'
# plan-34-A § Corrections 3: the RC_L / Moisture Resistant / 1/16W / ±100ppm
# piece of 52-yageo-re-1 (942 parts) no facet splits; listed down to a stock
# below the chip floor (81917 on 2026-11-13) instead.
MINQ=75000 run 52-yageo-re-3 52 13 '±1%' 39158 '0402 (1005 Metric)'
