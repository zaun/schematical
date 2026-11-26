#!/bin/sh
# Rebuilds the scenario files backend/tests/avr/<name>.json from their
# sources here (<name>.asm and <name>.meta.json, the latter written by
# make_meta.py), with avr8js's assembler (plan-62). A program without a
# settings file becomes backend/tests/avr/<name>.hex, firmware the engine
# tests load. Needs node and python3.
#
#   sh backend/tests/avr/src/build.sh
#
# Afterwards regenerate the golden lines:
#   <build>/avr_periph_test --print-golden   (the `run` lines of ../golden.txt)
set -e
here=$(cd "$(dirname "$0")" && pwd)
oracle="$here/../../../tools/avr8js-oracle/oracle.js"
python3 "$here/make_meta.py"
for source in "$here"/*.asm; do
    name=$(basename "$source" .asm)
    if [ -f "$here/$name.meta.json" ]; then
        node "$oracle" build "$source" "$here/$name.meta.json" > "$here/../$name.json"
        echo "built $name.json"
    else
        # No settings file: firmware for the engine tests (mcu_test and others).
        node "$oracle" asm "$source" > "$here/../$name.hex"
        echo "built $name.hex"
    fi
done
