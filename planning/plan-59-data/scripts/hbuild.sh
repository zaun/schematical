#!/bin/zsh
# hbuild.sh G|H <main.swift> <out name>: builds a harness main against a frozen copy of a
# spike worktree's sources, so later edits in the worktree cannot disturb the 15-minute build.
S=${0:A:h}
W=$1; MAIN=$2; OUT=$3
SNAP=$S/snap-$W-$OUT
rm -rf $SNAP; mkdir -p $SNAP
rsync -a --exclude .git /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-$W/schematical \
  /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-$W/backend \
  /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-$W/scripts $SNAP/
cd $SNAP
APP=$S/dd-$W/Build/Products/Debug/SchematicalSpike59.app scripts/harness/build.sh $MAIN $S/$OUT > $S/$OUT.log 2>&1
echo "exit $?" >> $S/$OUT.log
grep -E ' error|^error|^exit' $S/$OUT.log | head
