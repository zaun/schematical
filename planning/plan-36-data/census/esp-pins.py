"""plan-36-B Phase 2 (§ 3.3): the ESP32 module "Pin Definitions" tables.

usage: python3 -I esp-pins.py <transcribe-units.tsv> <out.tsv> [--dump]

For each module unit, reads the module datasheet's pin table (pdftotext
-layout): rows `Name  No.  Type  Function`, Type one of P / I / O / I/O / -,
continued across pages. Footnote marks run into names (`IO163` is IO16 with
note 3, B § 3.3): a trailing digit is dropped when it is one of the table's
footnote numbers and the name without it is a valid pad name (an IO number of
at most 48, or a name from the fixed list), never otherwise. Roles come from
roles.py (`esp32`: GND -> ground, the Type column for the rest).

The exposed GND pad(s) are added by esp-pads.tsv (`device first last name`,
from each datasheet's pin layout / land pattern), numbered after the table's
pins (WROOM-32E: 39); E draws the matching pads.
"""
import csv
import os
import re
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import roles as R  # noqa: E402

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
ROW = re.compile(r"^\s{0,12}([A-Z0-9_+\-/]+?)\s+(\d{1,3})\s+(I/O|I|O|P|-)(?:\s|$)")
FIXED = {"GND", "3V3", "EN", "NC", "TXD0", "RXD0", "U0TXD", "U0RXD", "SENSOR_VP", "SENSOR_VN", "CHIP_PU",
         "I34", "I35", "I36", "I37", "I38", "I39", "EPAD", "RXD", "TXD"}


def valid(name):
    """A module pad name: an IO / GPIO number up to 48, or a fixed name."""
    m = re.fullmatch(r"(?:IO|GPIO)(\d+)", name)
    if m:
        return int(m.group(1)) <= 48
    return name in FIXED


def expand(text):
    """`1, 2, 11, 14, 36-53` -> [1, 2, 11, 14, 36, ..., 53]."""
    out = []
    for part in re.findall(r"\d+\s*[-–]\s*\d+|\d+", text):
        if re.search(r"[-–]", part):
            a, b = map(int, re.split(r"\s*[-–]\s*", part))
            out.extend(range(a, b + 1))
        else:
            out.append(int(part))
    return out


def table(pdf):
    """[(number, name, type)] of the module's pin table, read by column: the
    header `Name  No.  Type  Function` gives the columns; a name with no number
    on its own line takes the number fragments of the lines around it (newer
    tables centre `1, 2, 11, 14, / GND / 36-53` on the name). Ends at the
    table's first footnote (`1 P: power supply ...`)."""
    txt = subprocess.run(["pdftotext", "-q", "-layout", pdf, "-"], capture_output=True, text=True).stdout
    starts = [m.start() for m in re.finditer(r"Table [\d\-.]*:?\s*Pin (Definitions|Description)", txt)]
    starts = [x for x in starts if "...." not in txt[x:x + 200].split("\n")[0]]
    if not starts:
        return [], set()
    lines = txt[starts[0]:].split("\n")
    recs = []        # [line index, name, type, numbers, own-line numbers?]
    frags = []       # (line index, numbers)
    notes = set()
    T = r"(I/O/T|I/O|I|O|P|—|-)"
    NUMS = r"((?:\d+\s*(?:[,∼~–-]\s*)?)+)"
    seen_header = False
    for i, line in enumerate(lines[:500]):
        if re.search(r"Name\s+No\.\s+Type", line):
            seen_header = True
            continue
        if not seen_header:
            continue
        if re.match(r"^\s{0,8}(1|a)\s+P:\s", line):
            break
        n = re.match(r"^\s{0,8}(\d)\s+[A-Z]", line)
        if n:
            notes.add(n.group(1))
        # a footnote mark may follow the name: `IO26b`, `IO47 c`, `IO26 2` (B § 3.3)
        name = r"^\s{0,12}([A-Z0-9][A-Z0-9_+\-]*?)(?:[a-e]| [a-e2-9])?\s+"
        m = re.match(name + NUMS + r"\s+" + T + r"(\s|$)", line)
        if m and not m.group(1).isdigit():
            # own-line numbers ending in a comma continue on the lines around
            # (`1, 2, 30,` / `GND  42, 43,  P` / `46-65`), so the record still takes fragments
            recs.append([i, m.group(1), "-" if m.group(3) == "—" else m.group(3), expand(m.group(2).replace("∼", "-").replace("~", "-")),
                         not m.group(2).rstrip().endswith(",")])
            continue
        m = re.match(name + T + r"(\s|$)", line)
        if m and not m.group(1).isdigit():
            recs.append([i, m.group(1), "-" if m.group(2) == "—" else m.group(2), [], False])
            continue
        m = re.match(r"^\s*" + NUMS + r"\s*$", line)
        if m:
            text = m.group(1).replace("∼", "-").replace("~", "-")
            if frags and frags[-1][2].rstrip().endswith("-") and i - frags[-1][0] <= 2:
                # a range split over two lines (`1, 2, 27, 38 ~` / `55`)
                j, _, prev = frags.pop()
                text = prev + text
            frags.append((i, expand(text), text))
    frags = [(i, nums) for i, nums, _ in frags]
    for i, nums in frags:
        free = [r for r in recs if not r[4]]
        if free:
            min(free, key=lambda r: abs(r[0] - i))[3].extend(nums)
    rows = []
    for _, name, t, nums, _own in recs:
        for k in nums:
            rows.append((k, name, t))
    rows.sort()
    return rows, notes


def clean(name, notes):
    if valid(name):
        return name
    for k in (1, 2):
        if len(name) > k and name[-k:].isdigit() and all(d in notes for d in name[-k:]) and valid(name[:-k]):
            return name[:-k]
    return name


def main(argv):
    units_path, out_path = argv[0], argv[1]
    here = os.path.dirname(os.path.abspath(units_path))
    pads = {}
    pads_path = os.path.join(here, "esp-pads.tsv")
    if os.path.exists(pads_path):
        for r in csv.DictReader(open(pads_path), delimiter="\t"):
            pads[r["device"]] = r
    out = open(out_path, "w")
    out.write("pdf\tpage\tfigure\tdevices\tpinout\tdesignator\tname\trole\n")
    for u in csv.DictReader(open(units_path), delimiter="\t"):
        if u["geometry"] != "module":
            continue
        rows, notes = table(os.path.join(ROOT, u["pdf"]))
        nums = [r[0] for r in rows]
        ok = nums == list(range(1, len(nums) + 1))
        # EPAD (the newer tables' name for the exposed pad) is ground: it is a GND pad
        names = [(n, "GND" if clean(nm, notes) == "EPAD" else clean(nm, notes), t) for n, nm, t in rows]
        if "--dump" in argv:
            print(u["devices"], len(rows), "1..N" if ok else f"NUMBERS {nums}", "notes", sorted(notes))
            print("   ", " ".join(f"{n}:{nm}" + ("" if valid(nm) else "?") for n, nm, t in names))
        if not ok:
            continue
        extra = []
        p = pads.get(u["devices"])
        if p:
            extra = [(k, p["name"], "P") for k in range(int(p["first"]), int(p["last"]) + 1)]
        allp = names + extra
        n = len(allp)
        for k, nm, t in allp:
            role = R.role(nm, "esp32", t)
            out.write(f"{u['pdf']}\t\tmodule-table: Pin Definitions table{' + ' + p['src'] if p else ''}\t"
                      f"{u['devices']}\tmodule{n}\t{k}\t{nm}\t{role}\n")


if __name__ == "__main__":
    main(sys.argv[1:])
