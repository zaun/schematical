#!/usr/bin/env python3
"""Makes the ATmega48/88/168/328 family simulated in mcu.json (plan-62-G).

    python3 planning/plan-62-data/set_models.py

For each of the 18 devices it replaces the line `"simulated": false,` of
that device with `"model": "<layout>",`. Nothing else in the file changes:
it is a line edit, not a JSON re-dump. Running it again changes nothing.
"""

import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
PATH = os.path.join(HERE, "..", "..", "schematical", "schematical", "Parts", "mcu.json")

BASE = ["atmega48", "atmega48a", "atmega48p", "atmega48pa", "atmega88", "atmega88a", "atmega88p",
        "atmega88pa", "atmega168", "atmega168a", "atmega168p", "atmega168pa", "atmega328",
        "atmega328p"]
MODELS = {name: "atmega328" for name in BASE}
MODELS.update({"atmega48pb": "atmega168pb", "atmega88pb": "atmega168pb",
               "atmega168pb": "atmega168pb", "atmega328pb": "atmega328pb"})

lines = open(PATH).read().split("\n")
device = None
changed = 0
for i, line in enumerate(lines):
    match = re.match(r'^      "id": "([^"]+)",$', line)
    if match:
        device = match.group(1)
    elif device in MODELS and line == '      "simulated": false,':
        lines[i] = '      "model": "%s",' % MODELS[device]
        changed += 1
open(PATH, "w").write("\n".join(lines))
print("%d devices changed" % changed)
