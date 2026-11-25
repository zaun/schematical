#!/bin/zsh
# plan-59-C phase 1: the H worktree (detached, base 2d10a029), G's patch as the starting copy, then H's changes.
set -e
S=${0:A:h}
H=/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-H
cd /Users/justinzaun/Development/logic
git worktree add --detach $H 2d10a029 > /dev/null 2>&1
cd $H
git apply /Users/justinzaun/Development/logic/.claude/worktrees/P-59/planning/plan-59-data/spike-G-app.patch
python3 $S/patch-H.py
git rev-parse HEAD
