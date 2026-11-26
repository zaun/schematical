#!/bin/sh
# Names defined both in liblogic.a and in a test bench object (each bench
# object is compiled once per program, so bench names are deduplicated first).
cd /tmp/p68-build || exit 1
nm -g --defined-only liblogic.a 2>/dev/null | awk '$2 ~ /^[TDSB]$/ {print $3}' | sort | uniq -d > /tmp/p68-lib-dups.txt
nm -g --defined-only liblogic.a 2>/dev/null | awk '$2 ~ /^[TDSB]$/ {print $3}' | sort -u > /tmp/p68-lib.txt
find CMakeFiles -name '*bench*.c.o' -exec nm -g --defined-only {} \; 2>/dev/null | awk '$2 ~ /^[TDSB]$/ {print $3}' | sort -u > /tmp/p68-bench.txt
echo "in the library twice:"; cat /tmp/p68-lib-dups.txt
echo "library and bench:"; comm -12 /tmp/p68-lib.txt /tmp/p68-bench.txt
