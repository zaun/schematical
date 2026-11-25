#!/bin/zsh
# Probe F6 (sandbox bookmark after relaunch; another writer). usage: f6.sh G|H
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
P=/Users/justinzaun/Development/logic/.claude/worktrees/P-59/planning/plan-59-data
rm -rf $C/fx/F6; mkdir -p $C/fx/F6/moved
cp $P/fixtures/eeprom-rel/design.schematical $P/fixtures/eeprom-rel/rom.bin $C/fx/F6/
$S/launch.sh $W $C/fx/F6/design.schematical
echo "== give the load file an app-scoped bookmark, save, quit"
$S/sp.sh "bookmark 0" "save 0" "sleep 600" "close 0" | grep -v 'sleep\|slept'
sleep 2; pgrep -x SchematicalSpike59 > /dev/null && $S/sp.sh quit > /dev/null; sleep 2
echo "   bookmark in the saved file: $(grep -c '"bookmark"' $C/fx/F6/design.schematical)"
mv $C/fx/F6/rom.bin $C/fx/F6/moved/rom.bin
echo "== rom.bin moved to F6/moved/ (the stored path rom.bin no longer resolves); relaunch and open"
$S/launch.sh $W $C/fx/F6/design.schematical
$S/sp.sh "mem 0" | sed -E "s#$C/##g"
echo "== another writer changes the file; the document has no unsaved changes"
echo "" >> $C/fx/F6/design.schematical
sleep 3
$S/sp.sh "docs" "sheet 0" | grep '^\[0\]\|window\|sheet' | sed -E "s#$C/##g" | cut -c1-260
echo "== edit, another writer changes the file again, then File > Save (saveDocument:)"
$S/sp.sh "edit 0 label" > /dev/null
echo "" >> $C/fx/F6/design.schematical
sleep 1
$S/sp.sh "action 0 saveDocument:" "sleep 1500" "sheet 0" "windows" | grep -v 'sleep\|slept' | cut -c1-400
