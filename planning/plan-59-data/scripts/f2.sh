#!/bin/zsh
# Probe F2 (file URL while writing). usage: f2.sh G|H
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
P=/Users/justinzaun/Development/logic/.claude/worktrees/P-59/planning/plan-59-data
rm -rf $C/fx/A $C/fx/B $C/fx/B2 $C/fx/B3; mkdir -p $C/fx/A $C/fx/B $C/fx/B2 $C/fx/B3
cp $P/fixtures/eeprom-rel/design.schematical $P/fixtures/eeprom-rel/rom.bin $C/fx/A/
path() { echo "   stored in $1: $(grep -A1 '"memoryFile"' $C/fx/$1 | grep path | tr -d ' ')" }
$S/launch.sh $W $C/fx/A/design.schematical
echo "== autosave in place (A/design.schematical), after one edit"
$S/sp.sh "edit 0 label" "autosave 0" "sleep 600" | grep autosave; path A/design.schematical
echo "== Save As into B (B2 value: ../A/rom.bin)"
$S/sp.sh "saveas 0 $C/fx/B/design.schematical" "sleep 800" "mem 0" | grep -v sleep\\\|slept; path B/design.schematical
echo "== the next autosave after Save As (one more edit)"
$S/sp.sh "edit 0 label" "autosave 0" "sleep 600" | grep autosave; path B/design.schematical
echo "== Save To into B3 (a copy; the document stays at B)"
$S/sp.sh "saveto 0 $C/fx/B3/design.schematical" "sleep 800" | grep saveto; path B3/design.schematical
if [ $W = G ]; then
  echo "== with the re-save workaround on: Save As into B2, then the autosave it causes"
  $S/sp.sh "resave on" "saveas 0 $C/fx/B2/design.schematical" "sleep 800" | grep -v sleep\\\|slept; path B2/design.schematical
  $S/sp.sh "docs" | grep '^\[0\]' | sed -E 's/.*(edited=[a-z]+ unautosaved=[a-z]+).*/   document \1/'
  $S/sp.sh "autosave 0" "sleep 600" | grep autosave; path B2/design.schematical
fi
echo "== Duplicate (load file without a bookmark)"
$S/sp.sh "duplicate 0" "sleep 1200" "mem 1" | grep -v sleep\\\|slept | cut -c1-330
echo "== Duplicate (load file with an app-scoped bookmark, as the open panel gives)"
$S/sp.sh "close 1" "bookmark 0" "duplicate 0" "sleep 1200" "mem 1" "close 1" | grep -v sleep\\\|slept | cut -c1-330
grep -c 'bookmark' "$C/Library/Autosave Information/"*.schematical 2>/dev/null
echo "== log"
grep 'snapshot ms\|attach' $C/spike59.log | sed -E "s#$C/##g" | cut -c16-220
