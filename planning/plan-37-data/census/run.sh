#!/bin/zsh
# plan-37 census wrapper. usage: run.sh <rawdir> <cat|-> <maker|-> <package id|-> <label> [keywords]
# Credentials from ~/.zshrc; a piece with a <label>.done marker is skipped.
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
here=${0:A:h}
raw=$1; shift
f="$raw/$4.jsonl"; [ -e "$raw/$4.done" ] && { echo "skip $4"; exit 0; }
rm -f "$f"
node "$here/list.js" "$1" "$2" "$3" "$4" "$f" "${5:-}" 2> "$raw/$4.err" && touch "$raw/$4.done"
cat "$raw/$4.err"
