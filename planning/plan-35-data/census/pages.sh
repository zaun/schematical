#!/bin/zsh
# usage: pages.sh pdf regex -> pages whose text matches
n=$(pdfinfo "$1" | awk '/^Pages/{print $2}')
for p in $(seq 1 $n); do pdftotext -f $p -l $p "$1" - 2>/dev/null | grep -qiE "$2" && print -n "$p "; done; echo
