#!/bin/zsh
# usage: dl.sh cand0.tsv outdir
while IFS=$'\t' read -r mfr pn url; do
  f="$2/${url:t}"
  [[ -s "$f" ]] && continue
  curl -sL --max-time 60 -A 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15' -o "$f" "$url"
  print -r -- "$(file -b "$f" | cut -c1-30)  ${url:t}"
done < "$1"
