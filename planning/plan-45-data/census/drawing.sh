#!/bin/zsh
# plan-45-C/D/E: show where a datasheet's drawings are, and render a region of a page enlarged for reading.
#   usage: drawing.sh <pdf>                         pages, dimension text lines, the pages that name a drawing
#          drawing.sh <pdf> <page> <dpi> [x y w h]  render <page> at <dpi> (optionally cropped, pixels at that
#                                                   dpi) to /tmp/p45-drawing-<name>-<page>.png and print the path
pdf=$1
name=${${pdf:t}%.pdf}
if [[ $# -eq 1 ]]; then
  echo "== $name: $(pdfinfo $pdf | awk '/^Pages:/{print $2}') pages, page size $(pdfinfo $pdf | awk '/^Page size:/{print $3 "x" $5}')"
  n=$(pdfinfo $pdf | awk '/^Pages:/{print $2}')
  for p in $(seq 1 $n); do
    t=$(pdftotext -f $p -l $p -layout $pdf - | grep -i -c "drilling diagram\|dimensional drawing\|drawings\|hole pattern\|PCB LAYOUT\|PC board layout\|Layout")
    [[ $t -gt 0 ]] && echo "page $p names a drawing ($t lines)"
  done
  pdftotext -layout $pdf - | grep -n -i "^ *\(Length\|Height\|Width\|Depth\|Pitch\|Dimension [a-z]\|Pin dimensions\|Pin spacing\|Hole diameter\|Pin length\)\b" | cut -c1-110 | head -14
else
  page=$2; dpi=$3
  out=/tmp/p45-drawing-$name-$page
  if [[ $# -ge 7 ]]; then
    pdftoppm -r $dpi -png -f $page -l $page -x $4 -y $5 -W $6 -H $7 $pdf $out
  else
    pdftoppm -r $dpi -png -f $page -l $page $pdf $out
  fi
  ls $out*.png
fi
