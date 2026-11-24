#!/bin/zsh
# plan-45-C: the footprint-relevant numbers of a Weidmuller per-item sheet and its dimensional drawing.
#   usage: wdm.sh <item>      prints the dimension lines and extracts the gray 800x800 raster of the "Drawings"
#                             page to /tmp/p45-img/wdm-<item>-<n>.png (the dimensional drawing is the one
#                             listed as `gray`; the product photo and the graph are the other images)
item=$1
pdf=${0:A:h:h:h:h}/datasheets/connector-terminal/weidmuller/$item.pdf
echo "== $item: $(pdfinfo $pdf | awk '/^Pages:/{print $2}') pages"
pdftotext -layout $pdf - | grep -i "^ *\(Depth\|Height\|Width\|Pitch in mm\|Number of poles\|Solder eyelet hole\|Solder pin dim\|Solder pin length\|Number of solder pins\|Conductor outlet\|Mounting onto\|Max. adjacent\|Height of lowest\|Pin series\)" | sed 's/   */  /g' | cut -c1-140
n=$(pdfinfo $pdf | awk '/^Pages:/{print $2}')
page=0
for p in $(seq 1 $n); do
  pdftotext -f $p -l $p -layout $pdf - | grep -q "Drawings" && pdftotext -f $p -l $p -layout $pdf - | grep -q "Dimensional drawing" && page=$p
done
echo "drawings page: $page"
mkdir -p /tmp/p45-img
if [[ $page -gt 0 ]]; then
  rm -f /tmp/p45-img/wdm-$item-*.png
  pdfimages -png -f $page -l $page $pdf /tmp/p45-img/wdm-$item
  pdfimages -list -f $page -l $page $pdf | tail -n +3
fi
