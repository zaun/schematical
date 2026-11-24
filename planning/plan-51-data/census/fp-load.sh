#!/bin/zsh
# plan-51-D: loads a side file of footprints through the real loader (see fp-load.js).
# usage: fp-load.sh <side.json>      -> "<n> new footprints: load ok", or what is wrong (exit 1)
exec node "${0:A:h}/fp-load.js" "$@"
