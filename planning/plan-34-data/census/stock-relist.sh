#!/bin/zsh
# plan-34-A § Corrections 7: the part-by-part stock check in bulk. Lists each
# (category, maker) again with top.js down to its category's floor
# (TOP_MIN_QTY): every picked row has stock at or above the floor, so each
# one is in the listing unless DigiKey no longer has it in stock (or a piece
# is cut at 300). stock-merge.py then confirms each picked row found there by
# its exact part number; stock-check.sh does the rest one call each.
#   usage: stock-relist.sh <relist dir> <floor chip> <floor through-hole> <floor array>
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
here=${0:A:h}
out=$1
mkdir -p $out
typeset -A floor
floor=(52 $2 53 $3 50 $4)
for cat in 52 53 50; do
  for m in 2577:viking 13:yageo 541:dale; do
    id=${m%%:*}; slug=${m##*:}
    [[ -e $out/$cat-$slug.done ]] && continue
    [[ -e $out/$cat-$slug.err ]] && cp $out/$cat-$slug.err $out/$cat-$slug.resume
    TOP_MIN_QTY=$floor[$cat] node $here/top.js $cat $id $out/$cat-$slug.jsonl 2> $out/$cat-$slug.err && touch $out/$cat-$slug.done
    tail -1 $out/$cat-$slug.err
    grep -q 'rate limited' $out/$cat-$slug.err && exit 1
  done
done
