"""plan-36-B Phase 3 (§ 3.5): confirm devices.tsv's clock and supply in each datasheet.

usage: python3 -I facts-check.py <devices.tsv> [--candidates candidates.tsv] [--write-overrides]

For each device, the datasheet its add rows use most is read with pdftotext.
  vcc_min / vcc_max must print as volts (`1.8`, `5.5`, `1.8V`, `1.8 - 5.5V`)
    on the first pages (title, features, `Operating Voltage`); the newer
    datasheets give the range only in Electrical Characteristics, so the
    rest of the text is searched next for an operating-voltage line holding
    both numbers (`VDD ... 1.8 ... 5.5 V`).
  clock must print as MHz on the first pages (`20 MHz`, `Speed Grade 0-20
    MHz`). DigiKey's Speed is the internal oscillator's for some tinyAVR
    1-series parts (16 MHz) where the datasheet says "running at up to
    20 MHz": --write-overrides then writes facts-overrides.tsv (`device
    field value evidence`) with the datasheet's figure, which devices.py
    applies.
The first pages are pages 1-3, or for a datasheet that opens with a long
table of contents (the RP2040's) the first 3 pages without dot leaders.
Prints each device that still disagrees and `<n> mismatches`.
"""
import csv
import os
import re
import subprocess
import sys
from collections import Counter

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))


def pages(pdf, cache={}):
    if pdf not in cache:
        txt = subprocess.run(["pdftotext", "-q", os.path.join(ROOT, pdf), "-"],
                             capture_output=True, text=True, timeout=600).stdout
        cache[pdf] = [re.sub(r"\s+", " ", p) for p in txt.split("\f")]
    return cache[pdf]


def first(ps):
    body = [p for p in ps if p.count(". . .") < 5 and p.count("....") < 5]
    return " ".join(body[:3])


def num(v):
    f = float(v)
    return f"{f:g}" if f != int(f) else f"{int(f)}"


def volts_in(text, v):
    alts = {num(v), f"{float(v):.1f}"}
    return any(re.search(r"(?<![\d.])" + re.escape(a) + r"\s*(V|-|–|~|to)", text) for a in alts)


def main(argv):
    path = argv[0]
    here = os.path.dirname(os.path.abspath(path))
    cand = argv[argv.index("--candidates") + 1] if "--candidates" in argv else os.path.join(here, "candidates.tsv")
    pdfs = {}
    for r in csv.DictReader(open(cand), delimiter="\t"):
        if r["verdict"] == "add":
            pdfs.setdefault(r["device"].upper(), Counter())[r["pdf"]] += 1
    bad = 0
    n = 0
    overrides = []
    for d in csv.DictReader(open(path), delimiter="\t"):
        c = pdfs.get(d["device"].upper())
        if not c:
            print("no add rows:", d["device"])
            bad += 1
            continue
        n += 1
        pdf = c.most_common(1)[0][0]
        ps = pages(pdf)
        head = first(ps)
        why = []
        if not (volts_in(head, d["vcc_min"]) and volts_in(head, d["vcc_max"])):
            # Electrical Characteristics: an operating-voltage line with both numbers
            lo, hi = re.escape(num(d["vcc_min"])), re.escape(num(d["vcc_max"]))
            rx = re.compile(r"(Operating Voltage|Supply Voltage|Power Supply|V ?DD|VCC|IOVDD)[^.]{0,160}?(?<![\d.])" + lo
                            + r"[^.]{0,80}?(?<![\d.])" + hi + r"\s*V", re.I)
            if not any(rx.search(p) for p in ps):
                why.append(f"vcc {d['vcc_min']}-{d['vcc_max']}")
        mhz = num(int(d["clock_hz"]) / 1e6)
        if not re.search(r"(?<![\d.])" + re.escape(mhz) + r"\s*MHz", head):
            m = re.search(r"(?:running at |up to |@ ?)(\d+)\s*MHz", head, re.I)
            if m and d["family"].startswith("AVR"):
                overrides.append((d["device"], "clock_hz", str(int(m.group(1)) * 1000000),
                                  f"{os.path.basename(pdf)} first pages: \"{m.group(0)}\" (DigiKey Speed {mhz} MHz)"))
            else:
                why.append(f"clock {mhz} MHz")
        if why:
            bad += 1
            print(f"{d['device']}: {', '.join(why)} not found in {os.path.basename(pdf)}")
    if "--write-overrides" in argv:
        with open(os.path.join(here, "facts-overrides.tsv"), "w") as f:
            f.write("device\tfield\tvalue\tevidence\n")
            for o in overrides:
                f.write("\t".join(o) + "\n")
        print(f"{len(overrides)} overrides written")
    elif overrides:
        bad += len(overrides)
        for o in overrides:
            print(f"{o[0]}: clock differs: {o[3]}")
    print(f"{n} devices, {bad} mismatches")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
