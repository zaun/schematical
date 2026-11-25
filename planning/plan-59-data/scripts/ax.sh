#!/bin/zsh
# ax.sh click <Menu> <Item> [<Subitem>]   — clicks a menu item of the spike app through System Events
# ax.sh enabled <Menu> <Item> [<Subitem>] — reads whether it is enabled
# ax.sh items <Menu>                      — names of the menu's items
# ax.sh key <char> <modifiers…>           — a key press sent to the spike app (only while it is frontmost)
# Refuses to act unless the screen is unlocked and the spike app is frontmost.
P=SchematicalSpike59
if ioreg -n Root -d1 | grep -q '"CGSSessionScreenIsLocked"=Yes'; then echo "LOCKED"; exit 2; fi
front() { osascript -e "tell application \"System Events\" to get frontmost of process \"$P\"" 2>/dev/null }
if [ "$(front)" != "true" ]; then
  osascript -e "tell application \"System Events\" to set frontmost of process \"$P\" to true" > /dev/null 2>&1
  sleep 0.6
  [ "$(front)" = "true" ] || { echo "NOT FRONTMOST"; exit 3; }
fi
item="menu item \"$3\" of menu \"$2\" of menu bar 1"
[ -n "$4" ] && item="menu item \"$4\" of menu 1 of menu item \"$3\" of menu \"$2\" of menu bar 1"
case $1 in
  click) osascript -e "tell application \"System Events\" to tell process \"$P\" to click $item" > /dev/null 2>&1 && echo "clicked $2 > $3 ${4:+> $4}" || echo "could not click $2 > $3 $4" ;;
  enabled) osascript -e "tell application \"System Events\" to tell process \"$P\" to get enabled of $item" 2>&1 ;;
  items) osascript -e "tell application \"System Events\" to tell process \"$P\" to get name of every menu item of menu \"$2\" of menu bar 1" 2>&1 ;;
  key) shift; ch=$1; shift; mods=""; for m in "$@"; do mods="$mods${mods:+, }$m down"; done
       osascript -e "tell application \"System Events\" to keystroke \"$ch\" ${mods:+using {$mods}}" 2>&1 && echo "key $ch $*" ;;
esac
