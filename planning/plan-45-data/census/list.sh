#!/bin/zsh
# plan-45 census: lists every (category, maker, scope) piece with list.js.
# usage: list.sh <raw dir>
# One <piece>.jsonl and .err per piece; a piece whose .done exists is skipped, so a run
# cut by the daily quota goes on the next day.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
# Both keys: list.js uses the second (DIGIKEY_*2, its own quota) unless DK_OPT2=0 (census/dk.js).
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)2?=' ~/.zshrc)"
# piece = <category>-<maker id>-<scope>. Categories: 371 Terminal Blocks - Wire to Board,
# 370 Terminal Blocks - Headers, Plugs and Sockets, 314 Rectangular Connectors - Headers,
# Male Pins. Makers: 277 Phoenix Contact, 281 Weidmuller, 732 Wurth Elektronik (the three
# with most in-stock Active / Last Time Buy single-level terminal parts in scope whose
# DigiKey datasheet links are on the maker's own site, plan-45-A § 2), 455 JST Sales America.
pieces=(314-455-jst 371-277-t 371-281-t 371-732-t 370-277-t 370-281-t 370-732-t)
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
