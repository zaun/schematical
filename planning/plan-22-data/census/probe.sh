#!/bin/bash
for n in "$@"; do ( c=$(curl -s -o /dev/null -w '%{http_code}:%{content_type}' -L "https://www.ti.com/lit/ds/symlink/$n.pdf"); echo "$n $c" ) & done; wait
