#!/bin/zsh
# plan-42 census: the owner-named base part numbers (planning/todo.md: 78xx,
# LM317, LDOs, bucks) by keyword, so a named part below a cut piece's 300
# most-stocked is still listed. One <category>-<maker id>-named-<base> piece each.
# usage: named.sh <raw dir>
raw=${1:?usage: named.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
pieces=(
  "699 296 UA7805" "699 296 LM7805" "699 296 LM340" "699 296 LM7812" "699 296 UA7812"
  "699 296 LM7833" "699 296 LM317" "699 296 LM1117" "699 296 TLV1117"
  "739 296 LM2596" "739 296 LM2576" "699 150 MCP1700" "699 150 MCP1702"
)
for p in $pieces; do
  parts=(${=p})
  f="$raw/${parts[1]}-${parts[2]}-named-${parts[3]}"
  [[ -e $f.done ]] && continue
  if node "$here/top.js" ${parts[1]} ${parts[2]} $f.jsonl 41 ${parts[3]} 2> $f.err; then
    touch $f.done
  else
    echo "named.sh: ${parts[3]} failed (see $f.err)" >&2
    exit 1
  fi
done
echo "named.sh: all ${#pieces} pieces done"
