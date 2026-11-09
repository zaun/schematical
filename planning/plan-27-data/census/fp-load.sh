#!/bin/zsh
# plan-27-B section 4.4: load ready footprint objects through footprint_check.
#   usage: fp-load.sh <footprints-X.json>...
# Copies backend/ and the Parts directory to /tmp/p27-fp, appends every object
# of the named files to the copy's footprints.json, builds footprint_check
# there and runs it.  Expected: only "extra footprint (not in the golden)" and
# "unused footprint" lines, for exactly the new ids; no load error.
HERE="${0:A:h}"
REPO="${HERE:h:h:h}"
DST=/tmp/p27-fp
rm -rf "$DST"
mkdir -p "$DST/schematical/schematical"
cp -R "$REPO/backend" "$DST/backend"
cp -R "$REPO/schematical/schematical/Parts" "$DST/schematical/schematical/Parts"
python3 -I "$HERE/fp-merge.py" "$DST/schematical/schematical/Parts/footprints.json" "$@" || exit 1
cmake -S "$DST/backend" -B "$DST/build" > "$DST/cmake.log" 2>&1 || { tail -20 "$DST/cmake.log"; exit 1; }
cmake --build "$DST/build" -j8 --target footprint_check > "$DST/build.log" 2>&1 || { tail -20 "$DST/build.log"; exit 1; }
"$DST/build/footprint_check" > "$DST/out.txt" 2>&1
"$DST/build/footprint_check" --print > "$DST/all.txt" 2>&1
python3 -I "$HERE/fp-merge.py" --expect "$DST/out.txt" "$DST/all.txt" "$REPO/backend/tests/golden/footprints.txt" "$@"
