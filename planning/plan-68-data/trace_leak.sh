#!/bin/sh
# usage: trace_leak.sh <test> <bench>   prints the live backend allocations after each call in main
W=/Users/justinzaun/Development/logic/.claude/worktrees/P-68/backend
T=$1; B=$2
mkdir -p /tmp/p68-trace
python3 - "$W/tests/$T.c" /tmp/p68-trace/$T.c <<'EOF'
import re, sys
s = open(sys.argv[1]).read()
i = s.index('\nint main(')
head, main = s[:i], s[i:]
main = re.sub(r'^(    )([a-z_0-9]+)\(([^;]*)\);$', lambda m: m.group(0) + f'\n    fprintf(stderr, "LIVE %ld after {m.group(2)}\\n", logic_alloc_live());', main, flags=re.M)
open(sys.argv[2], 'w').write('#include "logic_alloc.h"\n#include <stdio.h>\n' + head + main)
EOF
cc -std=c11 -I$W/include -I$W/src -I$W/src/engine -I$W/src/library -I$W/src/models -I$W/src/third_party/cjson -I$W/tests/support \
  -DPARTS_DIR="\"$W/../schematical/schematical/Parts\"" -DGOLDEN_DIR="\"$W/tests/golden\"" \
  -o /tmp/p68-trace/$T /tmp/p68-trace/$T.c $W/tests/support/$B.c /tmp/p68-build/liblogic_test_support.a /tmp/p68-build/liblogic.a -lm 2>&1 | head -5
cd /tmp && /tmp/p68-trace/$T 2>&1 | grep LIVE | awk '{if ($2 != prev) print; prev = $2}'
