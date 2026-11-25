#!/bin/zsh
# A6 repeat: open 6502 beside an Untitled document, close its window as the close button does,
# and report whether the document's objects are freed 2.5 s later. usage: a6c.sh G|H <runs> [commands before closing…]
S=${0:A:h}
W=${1:-G}; N=${2:-4}; shift 2
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
export SPIKE_ID=$ID
C=~/Library/Containers/$ID/Data
for i in $(seq 1 $N); do
  $S/launch.sh $W $C/fx/6502.schematical > /dev/null; $S/sp.sh "only 6502" > /dev/null
  $S/sp.sh "new" "sleep 600" "watch 0" > /dev/null
  [ $# -gt 0 ] && $S/sp.sh "$@" "sleep 500" > /dev/null
  echo "run $i ($*): $($S/sp.sh "closewindow 6502.schematical" "sleep 2500" "weak" | grep 'alive\|freed' | tr '\n' ' ')"
done
