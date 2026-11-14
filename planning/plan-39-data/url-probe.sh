#!/bin/sh
# usage: awk -F'\t' 'NF==2{print $1" "$2}' no-local-pdf.tsv | xargs -P 8 -L 1 ./url-probe.sh | sort > url-probe.txt
# Prints "<http status> <datasheet id>" for the first KB of the URL (a browser User-Agent: some sites refuse curl's).
id="$1"; url="$2"
code=$(curl -s -o /dev/null -L -m 20 -A "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 Safari/605.1.15" -r 0-1023 -w '%{http_code}' "$url")
echo "$code $id"
