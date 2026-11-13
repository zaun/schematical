"""plan-36-B Phase 2 (§ 3.3): the transcription units.

usage: python3 -I units.py <candidates.tsv> > transcribe-units.tsv

Every `add` row without `pinout_src` (after pinouts.py's ATDF pass), grouped
by (PDF, geometry, pins, package type): one datasheet figure usually covers
every device of that PDF in that package. Columns: `pdf geometry pins type
code devices rows name` (type: pkgcheck.ptype of the code's description).
"""
import csv
import os
import sys
from collections import OrderedDict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pinouts as P  # noqa: E402
import pkgcheck as K  # noqa: E402


def main(tsv):
    here = os.path.dirname(os.path.abspath(tsv))
    codes = P.load_codes(os.path.join(here, "package-codes.tsv"))
    units = OrderedDict()
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["verdict"] != "add" or r["pinout_src"] == "atdf":
            continue
        if r["family"] == "ESP32":
            g, n, t, name = "module", "", "", ""
        elif r["family"] == "RP2040":
            g, n, t, name = "quad", "56", "QFN", ""
        else:
            c = codes[(r["package_code"], P.pins_of(r))]
            g, n, name = c["geometry"], c["pins"], c["name"]
            t = K.ptype(name) or ""
        u = units.setdefault((r["pdf"], g, n, t), {"codes": set(), "devices": set(), "rows": 0, "names": set()})
        u["names"].add(name)
        u["codes"].add(r["package_code"])
        u["devices"].add(r["device"])
        u["rows"] += 1
    print("pdf\tgeometry\tpins\ttype\tcode\tdevices\trows\tname")
    for (pdf, g, n, t), u in sorted(units.items()):
        print(f"{pdf}\t{g}\t{n}\t{t}\t{','.join(sorted(u['codes']))}\t{','.join(sorted(u['devices']))}\t{u['rows']}\t{' | '.join(sorted(u['names']))}")
    print(f"{len(units)} units, {sum(u['rows'] for u in units.values())} rows", file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1])
