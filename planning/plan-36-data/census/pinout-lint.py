"""plan-36-B § 3.4: structural lint of pinouts.tsv.

usage: python3 -I pinout-lint.py <pinouts.tsv>

pinouts.tsv: `device pinout designator name role src`, one row per pin.
Per (device, pinout):
  - a numbered pinout of N pins (designators all digits) lists 1..N exactly once;
  - a ball pinout lists each ball once, and every ball is a JEP95 name
    (row letters without I O Q S X Z, column 1-99);
  - the pinout name matches its geometry and count (p<N>, quad<N>, drqfn<N>,
    plcc<N>, bga<N>, wlcsp<N>, module<N>) with N = the pin count;
  - every role is one of input output inout power ground nc;
  - every name is in roles.py's table or a port (role() is not None for
    the row's kind), and the role equals the table's for AVR / RP2040;
  - at least one power and one ground pin (F § 3.3, the loader's rule).
Prints each problem and `<n> problems`.
"""
import csv
import os
import re
import sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import roles as R  # noqa: E402

ROLES = {"input", "output", "inout", "power", "ground", "nc"}
BALL = re.compile(r"^[A-HJ-NPRTUVWY]{1,2}[1-9][0-9]?$")


def kind_of(device):
    if device.startswith("ESP32"):
        return "esp32"
    if device == "RP2040":
        return "rp2040"
    return "avr"


def main(path):
    groups = defaultdict(list)
    for r in csv.DictReader(open(path), delimiter="\t"):
        groups[(r["device"], r["pinout"])].append(r)
    problems = []
    for (dev, po), pins in groups.items():
        des = [p["designator"] for p in pins]
        n = len(des)
        if len(set(des)) != n:
            problems.append(f"{dev} {po}: repeated designators")
        m = re.match(r"^(p|quad|drqfn|plcc|bga|wlcsp|module)(\d+)$", po)
        if not m:
            problems.append(f"{dev} {po}: pinout name")
        elif int(m.group(2)) != n:
            problems.append(f"{dev} {po}: name says {m.group(2)} pins, has {n}")
        if all(d.isdigit() for d in des):
            if sorted(int(d) for d in des) != list(range(1, n + 1)):
                problems.append(f"{dev} {po}: numbers are not 1..{n}")
        elif m and m.group(1) in ("bga", "wlcsp"):
            bad = [d for d in des if not BALL.match(d)]
            if bad:
                problems.append(f"{dev} {po}: not ball names {bad[:5]}")
        elif not (m and m.group(1) == "drqfn"):
            problems.append(f"{dev} {po}: mixed designators")
        k = kind_of(dev)
        for p in pins:
            if p["role"] not in ROLES:
                problems.append(f"{dev} {po} {p['designator']}: role {p['role']}")
            if k != "esp32":
                want = R.role(p["name"], k)
                if want is None:
                    problems.append(f"{dev} {po} {p['designator']}: name {p['name']!r} not in the role table")
                elif want != p["role"]:
                    problems.append(f"{dev} {po} {p['designator']}: {p['name']} is {p['role']}, table says {want}")
        rs = {p["role"] for p in pins}
        if "power" not in rs or "ground" not in rs:
            problems.append(f"{dev} {po}: needs a power and a ground pin")
    for p in problems:
        print(p)
    print(f"{len(groups)} pinouts, {len(problems)} problems")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
