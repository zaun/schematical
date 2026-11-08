#!/bin/zsh
# plan-27 census, step 1: list every in-stock Active / Last Time Buy part of the
# Tactile Switches category (DigiKey category 197) for the ten chosen makers.
# Raw DigiKey JSON (one line per page) goes to $RAW/<maker-id>.jsonl.
#   usage: list.sh <raw-dir>
# Needs DIGIKEY_CLIENT_ID / DIGIKEY_CLIENT_SECRET (read from ~/.zshrc).
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
RAW="$1"
mkdir -p "$RAW"
cd "$(dirname "$0")/../../.." || exit 1
# id:name -- ids from the manufacturer facet of
#   scripts/digikey.js --category 197 --in-stock --active-only --pages 1 --json
for m in 108:ck 141:eswitch 2223:samesky 450:te-alco 39:omron 10:panasonic 4809:alps 732:wurth 360:nkk 679:apem; do
  id="${m%%:*}"
  name="${m##*:}"
  node scripts/digikey.js --category 197 --manufacturer "$id" --in-stock \
    --status 0 --status 4 --json > "$RAW/$id-$name.jsonl" 2> "$RAW/$id-$name.err"
  echo "$name: $(tail -1 "$RAW/$id-$name.err")"
done
