#!/bin/zsh
# usage: midas.sh candidates.tsv outdir -> part, own-site pdf url, sha256 match vs DigiKey copy
UA='Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.0 Safari/605.1.15'
awk -F'\t' '$1=="Midas Displays" && $4=="add"{print $2"\t"$14}' $1 | while IFS=$'\t' read pn dk; do
  slug=${(L)pn}
  page=$(curl -sL --max-time 30 -A "$UA" "https://midasdisplays.com/shop/lcd/lcd-character/$slug/")
  url=$(print -r -- "$page" | grep -o -i 'https://midasdisplays.com/wp-content/uploads/[^"'"'"' ]*\.pdf' | grep -i -F "$pn" | head -1)
  [[ -z $url ]] && url=$(print -r -- "$page" | grep -o -i 'https://midasdisplays.com/wp-content/uploads/[^"'"'"' ]*\.pdf' | head -1)
  f=$2/${url:t}; [[ -n $url ]] && curl -sL --max-time 60 -A "$UA" -o $f "$url"
  same=no; [[ -n $url ]] && cmp -s $f /tmp/p-lcd-pdf/${dk:t} && same=yes
  print -r -- "$pn	$url	$same	$([[ -n $url ]] && pdfinfo $f 2>/dev/null | awk '/^Pages/{print $2}')"
done
