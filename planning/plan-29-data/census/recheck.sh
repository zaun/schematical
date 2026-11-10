#!/bin/zsh
# plan-29 census, step 1b (plan-29-A section 5): re-list a piece that the
# 300-result cap cut short, one keyword search per part-number prefix.
#   usage: recheck.sh <raw-dir> <category> <maker id> <prefix>...
# Each search goes to <raw-dir>/<category>-<maker id>-kw-<prefix>.jsonl
# (build.py reads every *.jsonl and drops duplicates), with its summary in
# the matching .err; a prefix whose .done exists is skipped.
# Keywords are fuzzy (scripts/digikey.js help), so a search may return parts
# of other families too; that is harmless, they are the same in-stock parts.
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
RAW="$1"; c="$2"; m="$3"; shift 3
mkdir -p "$RAW"
cd "$(dirname "$0")/../../.." || exit 1
for p in "$@"; do
  out="$RAW/$c-$m-kw-$p"
  [ -s "$out.done" ] && continue
  node scripts/digikey.js --category "$c" --manufacturer "$m" --in-stock \
    --status 0 --status 4 --limit 5000 --json "$p" > "$out.jsonl" 2> "$out.err" &&
    tail -1 "$out.err" > "$out.done"
  echo "$c $m $p: $(tail -1 "$out.err")"
  grep -i 'only the first 300' "$out.err"
done
