#!/bin/zsh
# Probe A6 (lifetime). usage: a6.sh G|H
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
cp $S/base/examples/6502.schematical $C/fx/6502.schematical
$S/launch.sh $W $C/fx/6502.schematical; $S/sp.sh "only 6502" | tail -1
count() { heap SchematicalSpike59 2>/dev/null | grep -E " $1\$| schematical\.$1\$| $1 " | head -2 | tr -s ' ' | cut -c1-80 }
echo "== control: a Part Editor window (a plain WindowGroup scene) opened and closed"
$S/sp.sh "click File>New Part…" "sleep 800" > /dev/null
echo "   open:   $(count PartEditorModel)"
$S/sp.sh "closewindow New Part" "sleep 1500" | grep performClose
echo "   closed: $(count PartEditorModel)"
echo "== the document: a second (Untitled) document keeps the app running; run the simulation; close the 6502 window"
$S/sp.sh "new" "sleep 600" "docs" | grep '^\[' | cut -c1-70
$S/sp.sh "watch 0" "play 0" "child 0 memory" "child 0 bom" "sleep 600" "weak" | grep -v '^> w'
$S/sp.sh "closewindow 6502.schematical" "sleep 500" "weak" "sleep 3000" "weak" | grep -v 'sleep\|slept'
echo "   Simulation threads: $(sample SchematicalSpike59 1 2>/dev/null | grep -c 'Thread_.*Simulation')"
grep 'disappeared\|deinit' $C/spike59.log | cut -c16- | tail -4
