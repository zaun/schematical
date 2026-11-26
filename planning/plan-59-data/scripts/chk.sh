#!/bin/zsh
# chk.sh <patch>…: applies each patch (in order) to a throwaway detached worktree of the base commit.
S=${0:A:h}
cd /Users/justinzaun/Development/logic
rm -rf $S/chk; git worktree prune
git worktree add --detach $S/chk 2d10a029 > /dev/null 2>&1
cd $S/chk
rc=0
for patch in "$@"; do
  git apply --check $patch; r=$?
  echo "git apply --check ${patch:t} -> exit $r"
  [ $r = 0 ] && git apply $patch || rc=1
done
cd /Users/justinzaun/Development/logic
git worktree remove --force $S/chk
git worktree list | grep -c chk
exit $rc
