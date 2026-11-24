#!/bin/zsh
# plan-54 census: lists every (category, maker) piece with list.js.
# usage: list.sh <raw dir>
# One <piece>.jsonl and .err per piece; a piece whose .done exists is skipped, so a run cut by the daily
# quota goes on the next day.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
# piece = <category>-<maker id>. Categories: 414 Memory Connectors - PC Card Sockets, 413 Memory
# Connectors - Inline Module Sockets, 303 Card Edge Connectors - Edgeboard Connectors. Makers (the three
# with most in-stock Active parts per family, a company's DigiKey labels counted together, plan-54-A § 2):
#   414: Same Sky 2223, Amphenol ICC 609 + 664, GCT 2072 (3M 19 and ATTEND 4827 are listed for the record:
#        3M's links are mostly DigiKey copies, ATTEND has no DigiKey stock; both are `maker` out, plan-54-A § 2)
#   413: TE Connectivity 17 + 994, Amphenol ICC 609, Molex 23
#   303: Samtec 612, TE Connectivity 17 + 994, Amphenol ICC 609 + 664
pieces=(414-2223 414-609 414-664 414-2072 414-19 414-4827 413-17 413-994 413-609 413-23 303-612 303-17 303-994 303-609 303-664)
# PIECES (space separated) replaces the default list above when set.
[[ -n $PIECES ]] && pieces=(${=PIECES})
for p in $pieces; do
  f="$raw/$p"
  [[ -e $f.done ]] && continue
  if node "$here/list.js" $p $f.jsonl 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $p failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#pieces} pieces done"
