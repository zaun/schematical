#!/usr/bin/env python3
"""Writes the scenarios' settings files, <name>.meta.json (plan-62).

A scenario is a program (<name>.asm) plus these settings: the layout, the
memory sizes, how many instructions to run, and the stimuli. A stimulus is
[cycle, kind, ...], applied before the first instruction that starts at or
after that cycle:

    [cycle, "pin", port letter, bit, value]    the outside drives a pin

Run from this folder: python3 make_meta.py. Then rebuild each scenario file:

    node ../../../tools/avr8js-oracle/oracle.js build <name>.asm <name>.meta.json > ../<name>.json
"""

import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))


def meta(name, steps, stimuli, layout="atmega328", flash=32768, sram=2048, eeprom=1024):
    settings = {
        "layout": layout,
        "flash": flash,
        "sram": sram,
        "eeprom": eeprom,
        "steps": steps,
        "stimuli": sorted(stimuli, key=lambda s: s[0]),
    }
    with open(os.path.join(HERE, name + ".meta.json"), "w") as out:
        out.write(json.dumps(settings) + "\n")


def toggles(port, bit, period, until, start=0):
    """A pin that changes every `period` cycles, first to 1."""
    out = []
    value = 0
    cycle = start + period
    while cycle < until:
        value ^= 1
        out.append([cycle, "pin", port, bit, value])
        cycle += period
    return out


meta("gpio_out", 400, [
    [10, "pin", "D", 5, 1], [30, "pin", "B", 0, 1], [45, "pin", "B", 0, 0],
    [70, "pin", "C", 2, 1], [120, "pin", "B", 7, 1], [200, "pin", "D", 0, 1],
    [260, "pin", "C", 1, 1], [300, "pin", "B", 7, 0],
])

meta("gpio_int", 6000,
     toggles("D", 2, 37, 30000) + toggles("D", 3, 53, 30000) + toggles("B", 0, 71, 30000) +
     toggles("C", 1, 97, 30000) + toggles("D", 7, 113, 30000) + toggles("B", 1, 41, 30000))

meta("clkpr", 160, [])

meta("timer0", 60000, [])
meta("timer1", 45000, [])
meta("timer2", 56000, [])
meta("timer_ext", 2500, toggles("D", 4, 23, 30000) + toggles("D", 5, 31, 30000))
