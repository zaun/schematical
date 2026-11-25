#!/bin/zsh
# Builds candidate G (or H with "H") and prints errors only.
S=${0:A:h}
W=${1:-G}
xcodebuild -project /Users/justinzaun/Development/logic/.claude/worktrees/spike-59-$W/schematical/schematical.xcodeproj \
  -scheme schematical -configuration Debug -derivedDataPath $S/dd-$W CODE_SIGNING_ALLOWED=YES build > $S/dd-$W.log 2>&1
echo "exit $?"
grep -E 'error:|BUILD' $S/dd-$W.log | sort -u | head -30
