#!/bin/zsh
# esc.sh [key code] [modifier]: presses a key code (default 53, Escape) in the spike app,
# only while it is frontmost and the screen is unlocked. modifier: command, shift, option.
ioreg -n Root -d1 | grep -q '"CGSSessionScreenIsLocked"=Yes' && { echo LOCKED; exit 2; }
code=${1:-53}
using=""
[ -n "$2" ] && using="using {$2 down}"
osascript -e "tell application \"System Events\" to if frontmost of process \"SchematicalSpike59\" then key code $code $using" && echo "key code $code $2"
sleep 0.8
