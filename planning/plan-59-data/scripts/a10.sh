#!/bin/zsh
# Probe A10 (restoration). usage: a10.sh G|H
# Two documents (6502 edited and on the PCB page, with a Bill of Materials window; an untitled
# document with one edit); quit; relaunch. Run once with the system default (windows are closed
# at quit) and once with NSQuitAlwaysKeepsWindows on in the app's own defaults.
S=${0:A:h}
W=${1:-G}
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
round() {
  cp $S/base/examples/6502.schematical $C/fx/6502.schematical
  $S/launch.sh $W $C/fx/6502.schematical > /dev/null; $S/sp.sh "only 6502" > /dev/null
  $S/sp.sh "default NSQuitAlwaysKeepsWindows $1" | tail -1
  $S/sp.sh "new" "sleep 700" "edit 1 label" "edit 0 label" "edit 0 label" "segment 0 3 1" "sleep 400" "child 0 bom" "sleep 500" > /dev/null
  echo "   before quit: $($S/sp.sh docs | grep -c '^\[') documents; 6502: $($S/sp.sh 'state 0' | grep -o 'labels=[0-9]*') page segment $($S/sp.sh 'controls 0' | grep Schematic | grep -o 'selected=[0-9-]*'); untitled: $($S/sp.sh 'state 1' | grep -o 'labels=[0-9]*'); windows: $($S/sp.sh windows | grep -c visible=true)"
  $S/sp.sh terminate > /dev/null 2>&1; sleep 3
  if pgrep -x SchematicalSpike59 > /dev/null; then
    echo "   quit is waiting: $($S/sp.sh windows | grep -o "sheet=[^ ]* '[^']*'" | grep -v none | head -2 | tr '\n' ' ') $($S/sp.sh 'sheet 1' | cut -c1-300)"
    return
  fi
  echo "   quit: no prompt; 6502 on disk has $(grep -o '"netLabels"' $C/fx/6502.schematical | wc -l | tr -d ' ') netLabels key, N-labels: $(grep -c '"name" : "N[0-9]"' $C/fx/6502.schematical)"
  KEEP_STATE=1 $S/launch.sh $W > /dev/null
  echo "   after relaunch:"
  $S/sp.sh docs windows | grep '^\[\|#' | sed -E "s#$C/##; s/ edited=.*//; s/ visible=.*//" | sed 's/^/      /'
  for i in 0 1; do
    $S/sp.sh "state $i" "controls $i" | grep 'labels=\|Schematic' | sed -E 's/ boards=.*//; s/^\[3\] segmented /page /' | tr '\n' ' ' | sed 's/^/      /'; echo
  done
}
echo "== system default (NSQuitAlwaysKeepsWindows unset: windows are closed when quitting)"
round remove
echo "== NSQuitAlwaysKeepsWindows = true for the spike app"
round true
$S/sp.sh "default NSQuitAlwaysKeepsWindows remove" > /dev/null
