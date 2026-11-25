#!/bin/zsh
# plan-59-D phase 4: remove the two spike worktrees and what can be removed of the spike apps' containers.
pkill -9 -x SchematicalSpike59
cd /Users/justinzaun/Development/logic
git worktree remove --force /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-G
git worktree remove --force /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-H
git worktree prune
echo "spike worktrees left: $(git worktree list | grep -c spike-59)"
for c in jgz.schematical.spike59 jgz.schematical.spike59h; do
  chflags -R nouchg ~/Library/Containers/$c/Data/fx 2>/dev/null
  rm -rf ~/Library/Containers/$c 2>&1 | tail -1
  [ -e ~/Library/Containers/$c ] && echo "$c: container remains ($(ls ~/Library/Containers/$c/Data 2>/dev/null | wc -l | tr -d ' ') entries in Data)" || echo "$c: container removed"
done
ls ~/Library/Containers | grep -c spike59
