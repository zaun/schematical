#!/bin/zsh
# plan-34 census: top.js for each (category, maker). usage: list.sh <raw dir>
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
here=${0:A:h}
raw=$1
for cat in 52 53 50; do
  for m in 2577:viking 13:yageo 541:dale; do
    id=${m%%:*}; slug=${m##*:}
    [[ -e $raw/$cat-$slug.done ]] && continue
    node $here/top.js $cat $id $raw/$cat-$slug.jsonl 2> $raw/$cat-$slug.err && touch $raw/$cat-$slug.done
    tail -1 $raw/$cat-$slug.err
  done
done
