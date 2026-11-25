#!/bin/zsh
# H: probes A1, A2, A5, A7, A8, A9 (part), A15 in one session. Kills the app at the end.
S=${0:A:h}
export SPIKE_ID=jgz.schematical.spike59h SP_TRIES=60
C=~/Library/Containers/$SPIKE_ID/Data
cp $S/base/examples/6502.schematical $C/fx/6502.schematical; cp $S/base/examples/nixie-clock.schematical $C/fx/nixie.schematical
$S/launch.sh H $C/fx/6502.schematical $C/fx/nixie.schematical >/dev/null
echo "== A9 two files in one open: $($S/sp.sh docs | grep -c '^\[') documents; recents: $($S/sp.sh recents | grep -c schematical)"
echo "== A1 File menu (System Events)"
$S/ax.sh items File
osascript -e 'tell application "System Events" to tell process "SchematicalSpike59" to get name of every menu item of menu 1 of menu item "Export" of menu "File" of menu bar 1'
osascript -e 'tell application "System Events" to tell process "SchematicalSpike59" to get name of every menu item of menu 1 of menu item "Open Recent" of menu "File" of menu bar 1'
for item in Save Duplicate "Rename…" "Move To…" "Print…" Close; do echo "   $item enabled: $($S/ax.sh enabled File $item)"; done
$S/sp.sh "menu File" | grep '\[' | sed -E 's/ -> .*//; s/ \(disabled\)//' | tr '\n' ';'; echo
echo "== A2 Export Image from each document"
$S/ax.sh click Window "6502.schematical" >/dev/null; sleep 0.6; $S/ax.sh click File Export "Image…"; sleep 1.3
$S/sp.sh windows | grep 'doc=true' | sed -E "s/^#[0-9]+ NSWindow //; s/ visible.*sheet=/ sheet=/; s/ frame.*//"
$S/esc.sh >/dev/null
$S/ax.sh click Window "nixie.schematical" >/dev/null; sleep 0.6; $S/ax.sh click File Export "Image…"; sleep 1.3
$S/sp.sh windows | grep 'doc=true' | sed -E "s/^#[0-9]+ NSWindow //; s/ visible.*sheet=/ sheet=/; s/ frame.*//"
$S/esc.sh >/dev/null
grep 'commands evaluated' $C/spike59.log | cut -c16- | uniq | tail -4
echo "== A7 title bar"
$S/sp.sh "edit 0 pcbmove" "titlebar 0" | tail -1
W=$($S/sp.sh windows | grep "'6502" | grep -o '#[0-9]*' | tr -d '#'); screencapture -x -l $W $S/a7-H.png
echo "== A8 tabs (system setting as is), open twice"
$S/sp.sh "tabs" "open $C/fx/nixie.schematical" "docs" | grep "tabGroup\|alreadyOpen\|documents=" | cut -c1-70
echo "== A5 child windows"
$S/sp.sh "child 0 memory" "child 0 cpu" "child 0 terminal" "child 0 bom" "child 0 drc" "child 0 preview" "child 1 bom" "child 1 drc" "child 1 preview" "sleep 1200" "windows" | grep '^#' | grep -v TUINS | sed -E 's/ visible=([a-z]*).*/ visible=\1/' | tr '\n' ';'; echo
grep 'child window resolved' $C/spike59.log | cut -c16-
$S/sp.sh "closewindow nixie.schematical" "sleep 900" "windows" | grep '^#' | grep -v TUINS | grep -c 'visible=true'
grep 'H child windows closed\|H document close' $C/spike59.log | cut -c16-
echo "== A15 other scenes"
$S/sp.sh "click File>User Library…" "sleep 500" "click File>New Part…" "sleep 700" "click Help>Schematic Help" "sleep 500" > /dev/null
osascript -e 'tell application "System Events" to tell process "SchematicalSpike59" to click menu item "Settings…" of menu 1 of menu bar item 2 of menu bar 1' > /dev/null 2>&1; sleep 1
$S/sp.sh windows | grep 'visible=true' | grep -v TUINS | sed -E "s/^#[0-9]+ [A-Za-z]+ //; s/ visible.*//" | tr '\n' ';'; echo
pkill -9 -x SchematicalSpike59
