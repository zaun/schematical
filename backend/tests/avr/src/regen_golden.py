#!/usr/bin/env python3
"""Rewrites the `run` lines of ../golden.txt from the oracle (plan-62).

    python3 backend/tests/avr/src/regen_golden.py <build dir>

Runs <build dir>/avr_periph_test --print-golden, which hashes the traces
avr8js itself prints (never the C side's), and replaces every `run` line of
golden.txt with the result. Review the diff: a line that moved without its
scenario having changed means avr8js and the old golden disagree.
"""

import os
import subprocess
import sys

here = os.path.dirname(os.path.abspath(__file__))
golden = os.path.join(here, "..", "golden.txt")
out = subprocess.run([os.path.join(sys.argv[1], "avr_periph_test"), "--print-golden"],
                     check=True, capture_output=True, text=True).stdout
kinds = {line.split()[0] for line in out.splitlines() if line.strip()}
kept = [line for line in open(golden).read().splitlines() if line.split(" ")[0] not in kinds]
open(golden, "w").write("\n".join(kept) + "\n" + out)
print(out, end="")
