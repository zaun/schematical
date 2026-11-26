#!/usr/bin/env python3
"""Writes the scenarios' settings files, <name>.meta.json (plan-62).

A scenario is a program (<name>.asm) plus these settings: the layout, the
memory sizes, how many instructions to run, and the stimuli. A stimulus is
[cycle, kind, ...], applied before the first instruction that starts at or
after that cycle:

    [cycle, "pin", port letter, bit, value]    the outside drives a pin
    [cycle, "rx", usart index, byte]           a byte arrives at a USART
    [cycle, "adc", channel, millivolts]        an ADC input's voltage

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

meta("usart", 20000,
     [[20500, "rx", 0, 0x41]] + [[22000 + 1000 * i, "rx", 0, 0x42 + i] for i in range(4)] +
     [[27000, "rx", 0, 0x46]])
meta("spi", 6000, [])
meta("twi", 8000, [])
meta("adc", 14000,
     [[0, "adc", channel, millivolts] for channel, millivolts in
      enumerate([0, 500, 1000, 2500, 3300, 4999, 5000, 6000])] +
     [[3000, "adc", 0, 2500], [3000, "adc", 3, 1100], [6000, "adc", 3, 600]])
meta("eeprom", 170000, [])
meta("watchdog", 9000, [])

meta("m48_vectors", 2500,
     toggles("D", 2, 211, 6000) + [[300, "rx", 0, 0x61], [1500, "rx", 0, 0x62]],
     flash=4096, sram=512, eeprom=256)
meta("m168pb_porte", 400,
     [[20, "pin", "E", 1, 1], [60, "pin", "E", 3, 1], [100, "pin", "E", 1, 0], [200, "pin", "E", 0, 1]],
     layout="atmega168pb", flash=16384, sram=1024, eeprom=512)
meta("m328pb_extras", 4000,
     toggles("E", 0, 301, 9000) + toggles("E", 2, 517, 9000) + toggles("E", 3, 733, 9000) +
     [[400, "rx", 1, 0x41], [2000, "rx", 1, 0x42]],
     layout="atmega328pb")

# The ATtiny25/45/85. "sram" is the chip's SRAM; it starts at 0x60.
TINY85 = dict(layout="attiny85", flash=8192, sram=512, eeprom=512)
meta("t85_gpio", 1500,
     toggles("B", 2, 37, 8000) + toggles("B", 3, 53, 8000) + toggles("B", 4, 71, 8000) +
     toggles("B", 5, 97, 8000), **TINY85)
meta("t85_timer0", 20000, toggles("B", 2, 19, 60000), **TINY85)
meta("t85_timer1", 60000, [], **TINY85)
meta("t85_periph", 60000,
     [[0, "adc", channel, millivolts] for channel, millivolts in enumerate([300, 1100, 2500, 2400])] +
     [[400, "adc", 1, 4000]], **TINY85)
meta("t25_small", 4000, [], layout="attiny85", flash=2048, sram=128, eeprom=128)
