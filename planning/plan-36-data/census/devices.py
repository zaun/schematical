"""plan-36-B Phase 3 (§ 3.5): devices.tsv, one row per device of the add rows.

usage: python3 -I devices.py <candidates.tsv> <atdf-dir>

Columns: device family core clock_hz flash ram eeprom vcc_min vcc_max src.
  device   the ATDF name (ATmega328P), else the datasheet spelling
           (pinouts.proper); ESP32 module names; RP2040
  core     ATDF architecture: AVR8 -> avr, AVR8L -> avrrc, AVR8X -> avrxt,
           AVR8_XMEGA -> avrxm; a device with no ATDF file takes its sibling's
           (`ATtiny102F` -> ATtiny102, `AT90PWM2` -> AT90PWM2B,
           `AVR32EB28` -> AVR16EB28); ESP32 / S2 / S3 -> xtensa-lx6 / -lx7 / -lx7,
           C3 / C6 -> riscv, RP2040 -> cortex-m0plus
  flash, ram, eeprom (bytes)   the ATDF FLASH segment, the internal SRAM
           segment (IRAM / INTERNAL_SRAM), the eeprom segment; no ATDF:
           DigiKey's Program Memory / RAM / EEPROM sizes (facts-check.py
           confirms them in the datasheet). ESP32: the SoC's SRAM (module
           datasheet), flash the smallest of its parts (G overrides per part).
  clock_hz, vcc_min, vcc_max   the most common DigiKey Speed / Voltage over
           the device's add parts (G writes a differing part's own values);
           ESP32 modules: 3.0-3.6 V, their SoC's top clock (datasheet).
"""
import csv
import glob
import os
import re
import sys
import xml.etree.ElementTree as ET
from collections import Counter, OrderedDict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pinouts as P  # noqa: E402

ARCH = {"AVR8": "avr", "AVR8L": "avrrc", "AVR8X": "avrxt", "AVR8_XMEGA": "avrxm"}
SIBLING = {"ATTINY102F": "ATtiny102", "ATTINY1634R": "ATtiny1634", "ATTINY828R": "ATtiny828",
           "AT90PWM2": "AT90PWM2B", "AVR32EB14": "AVR16EB14", "AVR32EB20": "AVR16EB20",
           "AVR32EB28": "AVR16EB28", "AVR32EB32": "AVR16EB32", "ATTINY28V": None}
# ESP32 SoC facts, from the module datasheets' "Overview" / CPU section:
# (core, top clock, internal SRAM bytes); the module supply is 3.0-3.6 V in each.
ESP = {"ESP32": ("xtensa-lx6", 240000000, 520 * 1024), "ESP32-S2": ("xtensa-lx7", 240000000, 320 * 1024),
       "ESP32-S3": ("xtensa-lx7", 240000000, 512 * 1024), "ESP32-C3": ("riscv", 160000000, 400 * 1024),
       "ESP32-C6": ("riscv", 160000000, 512 * 1024)}


def size(text):
    """DigiKey sizes: `32KB (16K x 16)` -> 32768, `2K x 8` -> 2048, `512 x 8` -> 512, `-` -> 0."""
    text = (text or "").strip()
    m = re.match(r"([\d.]+)\s*(K|M)?B\b", text)
    if m:
        return int(float(m.group(1)) * {"K": 1024, "M": 1024 * 1024, None: 1}[m.group(2)])
    m = re.match(r"([\d.]+)\s*(K)?\s*x\s*(\d+)", text)
    if m:
        return int(float(m.group(1)) * (1024 if m.group(2) else 1) * int(m.group(3)) / 8)
    return 0


def volts(text):
    m = re.match(r"([\d.]+)V\s*~\s*([\d.]+)V", text or "")
    return (float(m.group(1)), float(m.group(2))) if m else (None, None)


def hz(text):
    m = re.match(r"([\d.]+)\s*MHz", text or "")
    return int(float(m.group(1)) * 1e6) if m else None


def atdf_facts(path):
    root = ET.parse(path).getroot()
    dev = root.find("devices/device")
    flash = ram = eeprom = 0
    for ms in root.iter("memory-segment"):
        t, name, sz = ms.get("type"), ms.get("name"), int(ms.get("size"), 0)
        if t == "flash" and name in ("FLASH", "PROGMEM", "APP_SECTION") and not flash:
            flash = sz
        elif t == "ram" and name in ("IRAM", "INTERNAL_SRAM") and not ram:
            ram = sz
        elif t == "eeprom" and not eeprom:
            eeprom = sz
    # XMEGA has no FLASH segment: APP_SECTION is the application flash the
    # datasheet names first (`128K + 8K` = APP_SECTION + BOOT_SECTION)
    return dev.get("name"), ARCH.get(dev.get("architecture"), "?"), flash, ram, eeprom


def main(tsv, atdf_dir):
    here = os.path.dirname(os.path.abspath(tsv))
    names = {os.path.basename(f)[:-5].upper(): f for f in glob.glob(os.path.join(atdf_dir, "*.atdf"))}
    groups = OrderedDict()
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["verdict"] == "add":
            groups.setdefault(r["device"].upper(), []).append(r)
    out = []
    for dev, rows in groups.items():
        fam = rows[0]["family"]
        vcc = Counter(volts(r["dk_vcc"]) for r in rows if volts(r["dk_vcc"])[0] is not None).most_common(1)
        clock = Counter(hz(r["dk_speed"]) for r in rows if hz(r["dk_speed"])).most_common(1)
        vmin, vmax = vcc[0][0] if vcc else (None, None)
        clk = clock[0][0] if clock else None
        if fam == "ESP32":
            soc = re.match(r"ESP32(-S2|-S3|-C3|-C6)?", dev).group(0)
            core, clk, ram = ESP[soc]
            flashes = [size(re.split(",", r["dk_flash"])[0]) for r in rows if "Flash" in r["dk_flash"]]
            out.append((dev, fam, core, clk, min(flashes) if flashes else 4 * 1024 * 1024, ram, 0, 3.0, 3.6,
                        f"module datasheet ({soc} SoC: core, clock, SRAM); flash: smallest part"))
            continue
        if fam == "RP2040":
            out.append(("RP2040", fam, "cortex-m0plus", clk or 133000000, 0, 270336, 0, vmin, vmax,
                        "rp2040-datasheet.pdf § 1 (264 KB SRAM, external QSPI flash); DigiKey clock / IOVDD"))
            continue
        path = names.get(dev)
        src = "atdf"
        if path is None and SIBLING.get(dev):
            path = names.get(SIBLING[dev].upper())
            src = f"atdf of sibling {SIBLING[dev]} (core); sizes DigiKey"
        if path:
            name, core, flash, ram, eeprom = atdf_facts(path)
            if src != "atdf":
                name = P.proper(dev)
                flash, ram, eeprom = size(rows[0]["dk_flash"]), size(rows[0]["dk_ram"]), size(rows[0]["dk_eeprom"])
            else:
                src = f"atdf:{os.path.basename(path)}"
        else:
            name, core = P.proper(dev), "avr"
            flash, ram, eeprom = size(rows[0]["dk_flash"]), size(rows[0]["dk_ram"]), size(rows[0]["dk_eeprom"])
            src = "no ATDF: classic AVR core (datasheet family); sizes DigiKey"
        out.append((name, fam, core, clk, flash, ram, eeprom, vmin, vmax, src))
    # facts-check.py's resolutions: a value read in the datasheet where DigiKey's differs
    cols = ["device", "family", "core", "clock_hz", "flash", "ram", "eeprom", "vcc_min", "vcc_max", "src"]
    for name in ("facts-overrides.tsv", "facts-overrides-hand.tsv"):
        p = os.path.join(here, name)
        if not os.path.exists(p):
            continue
        for o in csv.DictReader(open(p), delimiter="\t"):
            for i, row in enumerate(out):
                if row[0] == o["device"]:
                    row = list(row)
                    k = cols.index(o["field"])
                    row[k] = float(o["value"]) if o["field"].startswith("vcc") else int(o["value"])
                    row[-1] += f"; {o['field']} from the datasheet ({name})"
                    out[i] = tuple(row)
    with open(os.path.join(here, "devices.tsv"), "w") as f:
        f.write("device\tfamily\tcore\tclock_hz\tflash\tram\teeprom\tvcc_min\tvcc_max\tsrc\n")
        for row in out:
            f.write("\t".join("" if x is None else str(x) for x in row) + "\n")
    print(len(out), "devices;", sum(1 for r in out if r[3] is None or r[7] is None), "without clock or vcc")


if __name__ == "__main__":
    main(*sys.argv[1:3])
