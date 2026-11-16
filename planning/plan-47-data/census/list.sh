#!/bin/zsh
# plan-47 census: lists every piece of the table below with list.js.
# usage: list.sh <raw dir>
# One <piece>.jsonl and .err per piece; a piece whose .done exists is
# skipped, so a run cut by the daily quota goes on the next day.
#
# Makers: the three companies with the most in-stock Active / Last Time Buy
# surface-mount and through-hole parts of each type (plan-47-A § 2); the
# DigiKey maker ids of one company are listed together.
#   inductor (71 Fixed Inductors): 118 Bourns, 490 Murata, 732 Würth
#   fuse (139 Fuses): 18 Littelfuse, 283 Eaton, 486 SCHURTER
#   ptc (150 PTC Resettable Fuses, Type Polymeric): 18 Littelfuse, 118 Bourns, 507 Bel Fuse
#   relay (189 signal, 188 power, 964 reed, 183 solid state, 962 automotive):
#     TE Connectivity 844,41,994,17; 5785 Franjobaim; 10 Panasonic
#   buzzer (157 Alarms, Buzzers, and Sirens): 2223 Same Sky, 668 PUI Audio, 458 Mallory Sonalert
# Parameter 69 Mounting Type: 409393 Surface Mount, 411897 Through Hole,
# 395953 Requires Holder. Parameter 16 Package / Case: 248782 5mm x 20mm,
# 201760 3AB, 3AG, 1/4" x 1-1/4". Parameter 183 Type: 388650 Polymeric.
# The pieces DigiKey holds no part for (a filtered search that matches
# nothing is refused) are not in the table: relay 964 and 183 for
# Franjobaim, relay 964 for Panasonic.
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
SMD="--param 69=409393"
THT="--param 69=411897"
PCB="--param 69=409393,411897"
CART="--param 69=395953 --param 16=248782,201760"
POLY="--param 183=388650"
pieces=(
  "inductor ind-bourns-smd 71 --maker 118 $SMD"
  "inductor ind-bourns-tht 71 --maker 118 $THT"
  "inductor ind-murata-smd 71 --maker 490 $SMD"
  "inductor ind-murata-tht 71 --maker 490 $THT"
  "inductor ind-wurth-smd 71 --maker 732 $SMD"
  "inductor ind-wurth-tht 71 --maker 732 $THT"
  "fuse fuse-littelfuse-smd 139 --maker 18 $SMD"
  "fuse fuse-littelfuse-tht 139 --maker 18 $THT"
  "fuse fuse-littelfuse-cart 139 --maker 18 $CART"
  "fuse fuse-eaton-smd 139 --maker 283 $SMD"
  "fuse fuse-eaton-tht 139 --maker 283 $THT"
  "fuse fuse-eaton-cart 139 --maker 283 $CART"
  "fuse fuse-schurter-smd 139 --maker 486 $SMD"
  "fuse fuse-schurter-tht 139 --maker 486 $THT"
  "fuse fuse-schurter-cart 139 --maker 486 $CART"
  "ptc ptc-littelfuse-smd 150 --maker 18 $SMD $POLY"
  "ptc ptc-littelfuse-tht 150 --maker 18 $THT $POLY"
  "ptc ptc-bourns-smd 150 --maker 118 $SMD $POLY"
  "ptc ptc-bourns-tht 150 --maker 118 $THT $POLY"
  "ptc ptc-belfuse-smd 150 --maker 507 $SMD $POLY"
  "ptc ptc-belfuse-tht 150 --maker 507 $THT $POLY"
  "relay relay-te-signal 189 --maker 844,41,994,17 $PCB"
  "relay relay-te-power 188 --maker 844,41,994,17 $PCB"
  "relay relay-te-reed 964 --maker 844,41,994,17 $PCB"
  "relay relay-te-ssr 183 --maker 844,41,994,17 $PCB"
  "relay relay-te-auto 962 --maker 844,41,994,17 $PCB"
  "relay relay-franjobaim-signal 189 --maker 5785 $PCB"
  "relay relay-franjobaim-power 188 --maker 5785 $PCB"
  "relay relay-franjobaim-auto 962 --maker 5785 $PCB"
  "relay relay-panasonic-signal 189 --maker 10 $PCB"
  "relay relay-panasonic-power 188 --maker 10 $PCB"
  "relay relay-panasonic-ssr 183 --maker 10 $PCB"
  "relay relay-panasonic-auto 962 --maker 10 $PCB"
  "buzzer buzzer-samesky 157 --maker 2223 $PCB"
  "buzzer buzzer-pui 157 --maker 668 $PCB"
  "buzzer buzzer-mallory 157 --maker 458 $PCB"
)
for p in $pieces; do
  a=(${=p})
  f="$raw/$a[2]"
  [[ -e $f.done ]] && continue
  if node "$here/list.js" $f.jsonl ${a[1]} ${a[2]} ${a[3,-1]} 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $a[2] failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#pieces} pieces done"
