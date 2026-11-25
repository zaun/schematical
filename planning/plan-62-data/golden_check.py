#!/usr/bin/env python3
"""Checks that plan-62-G moved the goldens for the 126 simulated parts only.

    python3 planning/plan-62-data/golden_check.py [base]

Compares backend/tests/golden/parts.txt and traces.txt in the working tree
with the same files at commit `base` (HEAD by default). It fails unless:

  - no line was added or removed;
  - every changed line names one of the parts of the 18 devices that
    set_models.py gave a "model" (read from mcu.json);
  - each changed parts.txt line differs only by the removed `|unsimulated`
    and the added ` model=<layout>` after `ram=<n>`.
"""

import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..", "..")
base = sys.argv[1] if len(sys.argv) > 1 else "HEAD"

devices = json.load(open(os.path.join(ROOT, "schematical/schematical/Parts/mcu.json")))["devices"]
model_of = {}
for device in devices:
    if "model" in device:
        for part in device["parts"]:
            model_of[part["name"]] = device["model"]


def old(path):
    return subprocess.run(["git", "-C", ROOT, "show", "%s:%s" % (base, path)], check=True,
                          capture_output=True, text=True).stdout.split("\n")


def new(path):
    return open(os.path.join(ROOT, path)).read().split("\n")


failed = False
counts = {}
for name in ("parts.txt", "traces.txt"):
    path = "backend/tests/golden/" + name
    before, after = old(path), new(path)
    if len(before) != len(after):
        print("FAIL: %s has %d lines, had %d" % (name, len(after), len(before)))
        failed = True
        continue
    changed = 0
    for a, b in zip(before, after):
        if a == b:
            continue
        changed += 1
        # parts.txt separates fields with `|`, traces.txt with a tab.
        part = re.split(r"[|\t]", b)[0]
        if part != re.split(r"[|\t]", a)[0] or part not in model_of:
            print("FAIL: %s: a line of %s changed, and it is not one of the %d parts" %
                  (name, part, len(model_of)))
            failed = True
            continue
        if name == "parts.txt":
            want = re.sub(r"\|unsimulated$", "", a)
            want = re.sub(r"( ram=\d+)", r"\1 model=" + model_of[part], want, count=1)
            if want != b:
                print("FAIL: parts.txt: %s changed by more than |unsimulated and model=" % part)
                failed = True
    counts[name] = changed

if "parts.txt" in counts and counts["parts.txt"] != len(model_of):
    print("FAIL: %d parts.txt lines changed; %d parts have a model" % (counts["parts.txt"], len(model_of)))
    failed = True
if failed:
    sys.exit(1)
print("%d parts.txt lines, %d traces.txt lines, all within the %d parts" %
      (counts["parts.txt"], counts["traces.txt"], len(model_of)))
