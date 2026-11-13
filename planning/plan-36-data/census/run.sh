#!/bin/zsh
# usage: run.sh <rawdir> <cat> <maker> <series|-> <label> [keywords]
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
raw=$1; shift
f="$raw/$4.jsonl"; [ -e "$raw/$4.done" ] && { echo "skip $4"; exit 0; }
rm -f "$f"
node /tmp/p36/census/list.js "$1" "$2" "$3" "$4" "$f" "${5:-}" 2> "$raw/$4.err" && touch "$raw/$4.done"
cat "$raw/$4.err"
