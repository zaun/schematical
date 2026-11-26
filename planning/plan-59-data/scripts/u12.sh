#!/bin/zsh
# Probes U1, U2 and U3 on the 6502 document (document 0). usage: u12.sh G|H
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
mkdir -p $C/fx
cp $S/base/examples/6502.schematical $C/fx/6502.schematical
$S/launch.sh $W $C/fx/6502.schematical
short() { grep '^parts\|^>' | grep -v '^> sleep\|^> state' | sed -E 's/^parts=([0-9]+) labels=([0-9]+) lastLabel=([^ ]*) boards=([^ ]*) [^ ]* /   labels=\2 last=\3 \4 /' }
echo "== U1: one edit"
$S/sp.sh "state 0" "edit 0 pcbmove" "state 0" | short
echo "== U2: undo back to clean; edit, save, edit, undo; redo"
$S/sp.sh "docundo 0" "state 0" "edit 0 label" "save 0" "sleep 400" "state 0" "edit 0 pcbmove" "state 0" "docundo 0" "state 0" "docredo 0" "state 0" "docundo 0" "state 0" | short
echo "== U3: schematic edit, PCB edit, schematic edit; undo x3; redo x3"
$S/sp.sh "edit 0 label" "state 0" "edit 0 pcbmove" "state 0" "edit 0 label" "state 0" \
  "docundo 0" "state 0" "docundo 0" "state 0" "docundo 0" "state 0" \
  "docredo 0" "state 0" "docredo 0" "state 0" "docredo 0" "state 0" | short
echo "== U3: coalescing: typing 6 characters is one step; a 5-frame drag is one step"
$S/sp.sh "edit 0 rename ABCDEF" "state 0" "docundo 0" "state 0" "docredo 0" "state 0" "edit 0 endcoalescing" "edit 0 pcbdrag" "state 0" "docundo 0" "state 0" | short
