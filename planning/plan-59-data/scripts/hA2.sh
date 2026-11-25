#!/bin/zsh
# H: launch behaviour, A1 (again), A8 blank-Untitled replacement, A3, A4, A11, A12, A13. Kills the app at the end.
S=${0:A:h}
export SPIKE_ID=jgz.schematical.spike59h SP_TRIES=60
C=~/Library/Containers/$SPIKE_ID/Data
cp $S/base/examples/6502.schematical $C/fx/6502.schematical
echo "== launch with no file and nothing to restore"
$S/launch.sh H >/dev/null
$S/sp.sh "only none" >/dev/null 2>&1; sleep 1; pkill -9 -x SchematicalSpike59; sleep 1
rm -f "$C/Library/Autosave Information/"*(N)
$S/launch.sh H >/dev/null
$S/sp.sh docs windows | grep 'documents=\|^\[\|^#' | grep -v TUINS | cut -c1-90
echo "== A8: open a file while the blank Untitled is there"
open -a $S/dd-H/Build/Products/Debug/SchematicalSpike59.app $C/fx/6502.schematical; sleep 3
$S/sp.sh docs | grep 'documents=\|^\[' | cut -c1-60
grep 'blank Untitled' $C/spike59.log | cut -c16-
echo "== A1 File menu"
$S/ax.sh items File
$S/sp.sh "menu File" | grep '\[' | sed -E 's/ -> .*//; s/ \(disabled\)//; s/^ +//' | tr '\n' ';'; echo
echo "== A12 versions, Revert To, lock"
$S/sp.sh "edit 0 label" "save 0" "sleep 600" "edit 0 label" "save 0" "sleep 600" "versions 0" | tail -1 | cut -c1-80
osascript -e 'tell application "System Events" to tell process "SchematicalSpike59" to get {name, enabled} of every menu item of menu 1 of menu item "Revert To" of menu "File" of menu bar 1'
$S/sp.sh "lockdoc 0" "sleep 400" "docs" "unlockdoc 0" | grep 'lock:\|^\[' | sed -E 's/^\[0\].*(locked=[a-z]*).*/\1/'
echo "== A4 printDocument: (the system's File > Print flow)"
$S/sp.sh "action 0 printDocument:" "sleep 2500" "windows" | grep "sheet=[A-Z]" | sed -E "s/^#[0-9]+ NSWindow //; s/ visible.*sheet=/ sheet=/; s/ frame.*//"
$S/esc.sh > /dev/null; sleep 0.5
echo "== A3 exports (document outside the container)"
pkill -9 -x SchematicalSpike59; rm -f $S/exp/*.kicad*(N) $S/exp/*.epro(N) $S/exp/copy.schematical(N)
cp $S/base/examples/6502.schematical $S/exp/6502.schematical
$S/launch.sh H $S/exp/6502.schematical >/dev/null; $S/sp.sh "only 6502" >/dev/null
$S/ax.sh click File Export "KiCad Project…"; sleep 2; $S/sp.sh "sheet 0" | cut -c1-150; $S/esc.sh 36 >/dev/null; sleep 2.5
$S/ax.sh click File Export "EasyEDA Pro Project…"; sleep 2; $S/sp.sh "sheet 0" | cut -c1-110; $S/esc.sh 36 >/dev/null; sleep 2.5
ls $S/exp | tr '\n' ' '; echo
echo "== A13 automation tools"
$S/sp.sh choosefolder >/dev/null; sleep 2; $S/esc.sh 36 >/dev/null; sleep 1
$S/sp.sh "folder" "tool list_windows" "tool new_window" "tool open {\"path\":\"AFU.schematical\"}" "sleep 600" "tool list_windows" "edit 0 label" "tool save" "tool save {\"path\":\"copy.schematical\"}" "tool list_windows" "tool focus_window" | grep -v 'sleep\|slept\|^> edit\|added label' | sed -E "s#$S/##g; s#/tmp/claude-501/[^ ]*/scratchpad/##g"
echo "== A11 close and quit"
$S/sp.sh "only none" >/dev/null 2>&1; sleep 1; pkill -9 -x SchematicalSpike59
cp $S/base/examples/6502.schematical $C/fx/6502.schematical
$S/launch.sh H $C/fx/6502.schematical >/dev/null; $S/sp.sh "only 6502" >/dev/null
$S/sp.sh "new" "sleep 600" "edit 0 label" "performclose 0" "sleep 1200" "docs" | grep 'documents=' | cut -c1-40
$S/sp.sh "open $C/fx/6502.schematical" "sleep 900" "docs" "state 1" | grep '^\[\|labels=' | sed -E 's/ url=.*//; s/ boards.*(edited=[a-z]* equalsSavedFile=[a-z]*).*/ \1/' | tr '\n' ';'; echo
I=$($S/sp.sh docs | grep '^\[' | grep -n Untitled | cut -d: -f1); I=$((I-1))
$S/sp.sh "edit $I label" "performclose $I" "sleep 1200" "sheet $I" | tail -1 | cut -c1-160
$S/sp.sh "sheet $I Cancel" "sleep 500" "docs" | grep 'cancelled\|documents=' | tr '\n' ' '; echo
J=$($S/sp.sh docs | grep '^\[' | grep -n 6502 | cut -d: -f1); J=$((J-1))
$S/sp.sh "edit $J label" "play $J" | tail -1
$S/ax.sh enabled File Save >/dev/null; $S/esc.sh 12 command >/dev/null; sleep 1.5
echo "   after ⌘Q: sheets: $($S/sp.sh windows | grep -o "'[^']*' visible=true.*sheet=NSSavePanel" | cut -c1-12)"
$S/esc.sh >/dev/null; sleep 1
pgrep -x SchematicalSpike59 >/dev/null && echo "   after Cancel: app still running; $($S/sp.sh docs | grep -o 'documents=[0-9]*\|playing=[a-z]*' | tr '\n' ' ')" || echo "   the app quit"
pkill -9 -x SchematicalSpike59
