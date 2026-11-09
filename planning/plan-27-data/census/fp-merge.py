"""Helper of fp-load.sh (plan-27-B section 4.4).

usage: fp-merge.py <footprints.json> <footprints-X.json>...   append the objects (one line each)
       fp-merge.py --expect <footprint_check output> <footprints-X.json>...
           exit 0 when every failure line is "extra footprint (not in the golden)"
           or "unused footprint" for exactly the new ids, and nothing else failed.
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
    new = objects(paths)
    ids = [o["id"] for o in new]
    dup = sorted({i for i in ids if ids.count(i) > 1} | (set(ids) & have))
    if dup:
        print("duplicate ids:", ", ".join(dup))
        return 1
    tail = "\n  ]\n}"
    end = text.rstrip().rfind("]")
    head = text[:end].rstrip()
    lines = ",\n".join("    " + json.dumps(o, ensure_ascii=False, separators=(",", ":")) for o in new)
    open(target, "w").write(head + ",\n" + lines + tail + "\n")
    print(len(new), "footprints appended")
    return 0


def expect(out_path, paths):
    ids = {o["id"] for o in objects(paths)}
    text = open(out_path).read()
    extra, unused, other = set(), set(), []
    for line in text.splitlines():
        m = re.match(r"extra footprint \(not in the golden\): ([^|\s]+)", line)
        if m:
            extra.add(m.group(1))
            continue
        m = re.match(r"unused footprint: (\S+)", line)
        if m:
            unused.add(m.group(1))
            continue
        if re.match(r"\d+ footprints compared, \d+ parts loaded$|FAIL: \d+ mismatches$", line):
            continue
        other.append(line)
    bad = False
    for line in other:
        print("other failure:", line)
        bad = True
    for name, got in (("extra", extra), ("unused", unused)):
        if got != ids:
            print(f"{name} ids differ: not new {sorted(got - ids)}; new but not reported {sorted(ids - got)}")
            bad = True
    print(f"{len(ids)} new footprints: {'load ok' if not bad else 'FAILED'}")
    return 1 if bad else 0


if __name__ == "__main__":
    a = sys.argv[1:]
    if a[0] == "--expect":
        sys.exit(expect(a[1], a[2:]))
    sys.exit(merge(a[0], a[1:]))
