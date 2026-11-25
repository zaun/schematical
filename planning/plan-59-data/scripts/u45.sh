#!/bin/zsh
# Probes U4 (text field undo) and U5 (simulation lock). usage: u45.sh G|H
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
cp $S/base/examples/6502.schematical $C/fx/6502.schematical
$S/launch.sh $W $C/fx/6502.schematical >/dev/null; $S/sp.sh "only 6502" >/dev/null
short() { grep -v 'sleep\|slept' | sed -E 's/^parts=([0-9]+) labels=([0-9]+) lastLabel=([^ ]*) boards=([^ ]*) [^ ]* /   labels=\2 last=\3 /' | cut -c1-235 }
echo "== U4: type in the net-label name field, undo twice with the field editing, once with it closed"
$S/sp.sh "edit 0 label" "edit 0 rename T12345" "edit 0 endcoalescing" "save 0" "sleep 500" "select 0" "sleep 400" "fields 0" "focus 0 1" "type 0 AB" "state 0" "chain 0 undo:" "responder 0" "state 0" "chain 0 undo:" "responder 0" "state 0" "unfocus 0" "chain 0 undo:" "state 0" | short
echo "== U5: undo while the simulation runs"
$S/sp.sh "edit 0 label" "state 0" "play 0" "chain 0 undo:" "sleep 300" "state 0" "stop 0" "chain 0 undo:" "state 0" | short
grep refused $C/spike59.log | cut -c16-
pkill -9 -x SchematicalSpike59
