#!/bin/zsh
# Runs plan-61's design probes on the H build. Kills the app at the end.
S=${0:A:h}
export SPIKE_ID=jgz.schematical.spike59h SP_TRIES=60
C=~/Library/Containers/$SPIKE_ID/Data
APP=$S/dd-H/Build/Products/Debug/SchematicalSpike59.app
cp $S/base/examples/6502.schematical $C/fx/6502.schematical; cp $S/base/examples/nixie-clock.schematical $C/fx/nixie.schematical
echo "== plain launch, nothing to restore"
pkill -9 -x SchematicalSpike59; sleep 1; rm -f "$C/Library/Autosave Information/"*(N); : > $C/spike59.log
open -a $APP; sleep 5
$S/sp.sh docs | grep 'documents=\|^\[' | cut -c1-50
grep 'didFinish\|next main' $C/spike59.log | cut -c16-
echo "== title bar with the title shown"
$S/sp.sh "open $C/fx/6502.schematical" "sleep 1200" "docs" | grep 'documents=\|^\[' | cut -c1-50
I=$($S/sp.sh docs | grep '^\[' | grep -n 6502 | cut -d: -f1); I=$((I-1))
$S/sp.sh "edit $I pcbmove" "sleep 400" "titlebar $I" | tail -1
W=$($S/sp.sh windows | grep "'6502" | grep -o '#[0-9]*' | tr -d '#'); screencapture -x -l $W $S/p61-title.png
echo "== tab setting: default (tabs)"
$S/sp.sh "new" "sleep 700" "open $C/fx/nixie.schematical" "sleep 1000" "tabs" | grep tabGroup | cut -c1-60
echo "== tab setting: windows"
$S/sp.sh "default documentTabs windows" "only none" > /dev/null 2>&1; sleep 1
pkill -9 -x SchematicalSpike59; sleep 1; rm -f "$C/Library/Autosave Information/"*(N)
: > $C/spike59.log
open -a $APP $C/fx/6502.schematical; sleep 5
grep 'didFinish\|next main' $C/spike59.log | cut -c16-
$S/sp.sh "new" "sleep 700" "open $C/fx/nixie.schematical" "sleep 1000" "tabs" | grep tabGroup | cut -c1-60
$S/sp.sh "default documentTabs remove" > /dev/null
echo "== .schpart sent to the app"
echo '{}' > $C/fx/test.schpart
open -a $APP $C/fx/test.schpart; sleep 3
grep 'onOpenURL' $C/spike59.log | cut -c16-
$S/sp.sh windows | grep 'visible=true' | grep -v TUINS | sed -E "s/^#[0-9]+ [A-Za-z]+ //; s/ visible.*//" | tr '\n' ';'; echo
pkill -9 -x SchematicalSpike59
