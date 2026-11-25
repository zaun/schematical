#!/bin/zsh
# Runs a read-only or patch-export query against a spike worktree: wt.sh G|H stat|status|patch <out>
W=/Users/justinzaun/Development/logic/.claude/worktrees/spike-59-$1
cd $W || exit 1
case $2 in
  stat) git add -N . ; git diff --stat 2d10a029 | tail -1 ;;
  files) git add -N . ; git diff --stat 2d10a029 ;;
  status) git status --short ;;
  patch) git add -N . ; git diff 2d10a029 > $3; wc -l $3 ;;
  head) git rev-parse HEAD ;;
esac
