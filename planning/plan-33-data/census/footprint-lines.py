"""plan-33-B Phase 4: the add rows' footprint groups -> fpgroups.tsv and footprints-needed.txt.

usage: python3 -I footprint-lines.py

One fpgroups.tsv row per (maker, type, DigiKey package, size, lead spacing,
size code, datasheet) of the `add` rows, with the rows' count.  `letter` is
C for chips (0402-1206 MLCC; molded tantalum, tantalum polymer and niobium
oxide chips; the 2917 aluminum polymer chip) and D for the rest (cans,
leaded MLCC, discs, axials, dipped tantalums).  `height` is DigiKey's seated
height (or thickness) of the group's first row.  The last four columns
(footprint part_package pinout note) are left for plan-33-C and -D; a re-run
keeps what they wrote.  footprints-needed.txt lists the same groups, one per
line, C first.
"""
import collections
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
COLS = ["letter", "maker", "type", "package", "size", "spacing", "size_code", "height", "pdf", "rows",
        "footprint", "part_package", "pinout", "note"]
SHORT = {"KEMET": "KEMET", "Cal-Chip Electronics": "Cal-Chip", "KYOCERA AVX": "AVX"}


def letter(r):
    p = r["package"]
    if r["type"] == "ceramic":
        return "C" if p[:4] in ("0402", "0603", "0805", "1206") else "D"
    if "Metric" in p:
        return "C"
    return "D"


def main():
    paths = {p["url"]: p["path"] for p in csv.DictReader(
        open(os.path.join(ROOT, "datasheets", "capacitor", "datasheets.tsv")), delimiter="\t")}
    rows = [r for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv"), newline="", encoding="utf-8"),
                                      delimiter="\t") if r["decision"] == "add"]
    out_path = os.path.join(DATA, "fpgroups.tsv")
    old = {}
    if os.path.exists(out_path):
        for g in csv.DictReader(open(out_path, newline="", encoding="utf-8"), delimiter="\t"):
            old[tuple(g[c] for c in COLS[1:7] + ["pdf"])] = g
    groups = collections.OrderedDict()
    for r in sorted(rows, key=lambda r: (letter(r), r["maker"], r["type"], r["package"], r["size"], r["spacing"])):
        key = (SHORT[r["maker"]], r["type"], r["package"], r["size"], r["spacing"], r["size_code"],
               paths[r["datasheet_url"]])
        g = groups.get(key)
        if g is None:
            g = {"letter": letter(r), "maker": key[0], "type": key[1], "package": key[2], "size": key[3],
                 "spacing": key[4], "size_code": key[5], "height": r["height"], "pdf": key[6], "rows": 0,
                 "footprint": "", "part_package": "", "pinout": "", "note": ""}
            prev = old.get(key[:6] + (key[6],))
            if prev:
                for c in ("footprint", "part_package", "pinout", "note"):
                    g[c] = prev[c]
            groups[key] = g
        g["rows"] += 1
    with open(out_path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, COLS, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(groups.values())
    with open(os.path.join(DATA, "footprints-needed.txt"), "w") as f:
        f.write("# plan-33-B Phase 4: one line per footprint group of the add rows (fpgroups.tsv);\n")
        f.write("# letter, rows, maker, type, DigiKey package, size, lead spacing, size code, datasheet.\n")
        for g in groups.values():
            f.write(f"{g['letter']} {g['rows']:4}  {g['maker']} | {g['type']} | {g['package']} | {g['size']} | "
                    f"{g['spacing']} | {g['size_code']} | {os.path.basename(g['pdf'])}\n")
    by = collections.Counter(g["letter"] for g in groups.values())
    print(f"{len(groups)} groups (C {by['C']}, D {by['D']}), {sum(g['rows'] for g in groups.values())} rows")
    return 0


if __name__ == "__main__":
    main()
