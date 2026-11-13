#!/bin/zsh
curl -sIL -m 60 -o /dev/null -w "%{http_code}\t%{content_type}\t%header{content-length}\t$1\n" "$1"
