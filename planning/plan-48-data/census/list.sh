#!/bin/zsh
# plan-48 census: lists every piece of the table below with list.js.
# usage: list.sh <raw dir>
# One <piece>.jsonl and .err per piece; a piece whose .done exists is
# skipped, so a run cut by the daily quota goes on the next day.
#
#   timer (689 Programmable Timers and Oscillators, parameter 183 Type:
#     243489 "555 Type, Timer/Oscillator (Single)", 243487 "… (Dual)"):
#     every maker in one piece (86 parts), the makers are ranked from it.
#   opamp (687 Instrumentation, Op Amps, Buffer Amps, parameter 161
#     Amplifier Type: 863824 Standard (General Purpose), 362007 J-FET,
#     331587 CMOS; parameter 69 Mounting Type: 409393 Surface Mount,
#     411897 Through Hole): 296 Texas Instruments; 505 Analog Devices and
#     175 Analog Devices/Maxim (one company); 497 STMicroelectronics.
#   opamp-legacy: one page of the labels DigiKey keeps for Rochester
#     Electronics stock (14 National Semiconductor, 600026 Harris
#     Corporation), to show their datasheet links (plan-48-A § 2).
raw=${1:?usage: list.sh <raw dir>}
mkdir -p "$raw"
here=${0:A:h}
eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
T555="--param 183=243487,243489"
OPA="--param 161=863824,362007,331587 --param 69=409393,411897"
pieces=(
  "300 timer timer-all 689 $T555"
  "300 opamp opamp-ti 687 --maker 296 $OPA"
  "300 opamp opamp-adi 687 --maker 505,175 $OPA"
  "300 opamp opamp-st 687 --maker 497 $OPA"
  "50 opamp opamp-legacy 687 --maker 14,600026 $OPA"
)
for p in $pieces; do
  a=(${=p})
  f="$raw/$a[3]"
  [[ -e $f.done ]] && continue
  if LIST_CAP=$a[1] node "$here/list.js" $f.jsonl ${a[2]} ${a[3]} ${a[4,-1]} 2> $f.err; then
    touch $f.done
  else
    echo "list.sh: $a[3] failed (see $f.err)" >&2
    exit 1
  fi
done
echo "list.sh: all ${#pieces} pieces done"
