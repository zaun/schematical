#!/bin/zsh
# launch.sh G|H [file…]: kills the spike app, clears its log, opens it (with files), waits.
S=${0:A:h}
W=$1; shift
ID=jgz.schematical.spike59; [ $W = H ] && ID=jgz.schematical.spike59h
C=~/Library/Containers/$ID/Data
pkill -9 -x SchematicalSpike59; sleep 1
[ -n "$KEEP_STATE" ] || rm -rf "$C/Library/Saved Application State"
: > $C/spike59.log 2>/dev/null
open -a $S/dd-$W/Build/Products/Debug/SchematicalSpike59.app "$@"
sleep ${WAIT:-6}
