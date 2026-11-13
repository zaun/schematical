"""Helper of fp-load.sh (plan-37-B; copied from plan-35-data, itself from plan-29-B section 4.4).

usage: fp-merge.py <footprints.json> <footprints-X.json>...   append the objects (one line each)
       fp-merge.py --expect <check output> <check --print output> <golden> <footprints-X.json>...
           exit 0 when the printed footprints beyond the golden are exactly the new
           ids, none has overlapping pads, and the check run failed only with
           "extra footprint" / "unused footprint" lines for new ids.
"""
import json
import re
import sys


def objects(paths):
    out = []
    for p in paths:
        out.extend(json.load(open(p))["footprints"])
    return out


def merge(target, paths):
    text = open(target).read()
    have = set(re.findall(r'"id":"([^"]+)"', text))
    # Objects already moved into the library (plan-27-E and later) are
    # skipped; the rest must be new and distinct.
    new = [o for o in objects(paths) if o["id"] not in have]
    ids = [o["id"] for o in new]
    dup = sorted({i for i in ids if ids.count(i) > 1})
    if dup:
        print("duplicate ids:", ", ".join(dup))
        return 1
    if not new:
        print("0 footprints appended")
        return 0
    tail = "\n  ]\n}"
    end = text.rstrip().rfind("]")
    head = text[:end].rstrip()
    lines = ",\n".join("    " + json.dumps(o, ensure_ascii=False, separators=(",", ":")) for o in new)
    open(target, "w").write(head + ",\n" + lines + tail + "\n")
    print(len(new), "footprints appended")
    return 0


def overlaps(line):
    """Pairs of pads (by their bounding boxes, as footprint_check prints them:
    `pads=<pin>:<x>,<y>,<w>x<h>,<shape>;...`) whose areas overlap; touching is fine.
    The loader has no overlap check (plan-27-B Correction 1), so this is it."""
    m = re.search(r"\|pads=([^|]*)", line)
    if not m:
        return []
    pads = []
    for p in m.group(1).split(";"):
        pm = re.match(r"([^:]+):(-?[\d.]+),(-?[\d.]+),([\d.]+)x([\d.]+)", p)
        if pm:
            pads.append((pm.group(1), float(pm.group(2)), float(pm.group(3)), float(pm.group(4)), float(pm.group(5))))
    out = []
    for i in range(len(pads)):
        for j in range(i + 1, len(pads)):
            a, b = pads[i], pads[j]
            if (abs(a[1] - b[1]) < (a[3] + b[3]) / 2 - 1e-6 and abs(a[2] - b[2]) < (a[4] + b[4]) / 2 - 1e-6):
                out.append(f"{a[0]} and {b[0]}")
    return out


def expect(out_path, all_path, golden_path, paths):
    """footprint_check prints at most 10 extra/unused lines, so the new ids
    and their pads come from `footprint_check --print` (every footprint) minus
    the golden; the normal run's output may hold only extra/unused lines for
    new ids, its summary and the mismatch count."""
    golden = {l.split("|")[0] for l in open(golden_path).read().splitlines() if l}
    ids = {o["id"] for o in objects(paths)} - golden
    printed = {}
    for line in open(all_path).read().splitlines():
        if line:
            printed[line.split("|")[0]] = line
    other = []
    new = set(printed) - golden
    for i in sorted(new):
        for clash in overlaps(printed[i]):
            other.append(f"{i}: pads {clash} overlap")
    for line in open(out_path).read().splitlines():
        m = re.match(r"(?:extra footprint \(not in the golden\)|unused footprint): ([^|\s]+)", line)
        if m:
            if m.group(1) not in ids:
                other.append(line)
            continue
        if re.match(r"\d+ footprints compared, \d+ parts loaded$|FAIL: \d+ mismatches$", line):
            continue
        other.append(line)
    bad = False
    for line in other:
        print("other failure:", line)
        bad = True
    if new != ids:
        print(f"loaded ids differ: not in the files {sorted(new - ids)}; not loaded {sorted(ids - new)}")
        bad = True
    print(f"{len(ids)} new footprints: {'load ok' if not bad else 'FAILED'}")
    return 1 if bad else 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a[0] == "--expect":
        sys.exit(expect(a[1], a[2], a[3], a[4:]))
    sys.exit(merge(a[0], a[1:]))
