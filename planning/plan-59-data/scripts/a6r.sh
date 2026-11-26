#!/bin/zsh
# Probe A6, second half: Revert To Last Saved with the simulation running. usage: a6r.sh G|H
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
cp $S/base/examples/6502.schematical $C/fx/6502.schematical
$S/launch.sh $W $C/fx/6502.schematical > /dev/null; $S/sp.sh "only 6502" > /dev/null
short() { grep '^parts\|^>\|model id\|sheet\|pressed\|running' | grep -v '^> sleep\|^> state\|^> docs' | sed -E 's/^parts=([0-9]+) labels=([0-9]+) lastLabel=([^ ]*) boards=([^ ]*) [^ ]* /   labels=\2 last=\3 /; s/ url=[^ ]*//' | cut -c1-210 }
$S/sp.sh "edit 0 label" "save 0" "sleep 500" "edit 0 label" "edit 0 label" "state 0" "docs" "play 0" | short
: > $C/spike59.log
$S/sp.sh "action 0 revertDocumentToSaved:" "sleep 1500" "sheet 0" | short
$S/sp.sh "sheet 0 Last Saved Version" "sleep 2500" "docs" "state 0" | short
echo "   Simulation threads: $(sample SchematicalSpike59 1 2>/dev/null | grep -c 'Thread_.*Simulation')"
cut -c16-150 $C/spike59.log | grep -v snapshot | sed -E "s#$C/##"
