"""plan-27-B/C: merge the per-maker readers' files into the plan's data files.

usage: python3 -I merge-work.py <letter B|C> <slug>...

For each work/<slug>/ (BRIEF.md):
  cannot-draw.tsv-> candidates.tsv: those add / pending rows become `skip`,
                    with the file's reason ("cannot draw a footprint: …",
                    "not in the datasheet's ordering table: …")
  checks.tsv     -> candidates.tsv: a `check;` row becomes `out` (reason
                    `check; circuit|lit|mounting`) or keeps its decision with
                    circuit, mounting, actuator, rating filled
  schemes.tsv    -> appended to schemes.tsv (rows already there are kept once)
  footprints.json-> footprints-<letter>.json; a footprint identical to one
                    already in footprints-*.json or Parts/footprints.json
                    (every pad within 0.05 mm, same numbering, body within
                    0.1 mm; plan-27-B section 4.2) is not added: its id is
                    replaced by the existing one in the merged spec rows, and
                    the existing object's note gains "shared with <series>"
  specs.tsv      -> replaces the skeleton rows of the same (maker, series,
                    mounting) in specs.tsv
Run scheme-check.py afterwards (it applies the schemes to candidates.tsv).
Idempotent: a second run with the same files changes nothing.
"""
import csv
import glob
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
PARTS_FP = os.path.join(ROOT, "schematical", "schematical", "Parts", "footprints.json")
CAND = os.path.join(DATA, "candidates.tsv")
SCHEMES = os.path.join(DATA, "schemes.tsv")
SPECS = os.path.join(DATA, "specs.tsv")
SPEC_COLS = ["maker", "series", "mounting", "mpn_regex", "terminals", "pins", "footprint", "note"]


def read(path):
    if not os.path.exists(path):
        return [], None
    with open(path) as f:
        r = csv.DictReader(f, delimiter="\t")
        rows = list(r)
        return rows, r.fieldnames


def write(path, rows, cols):
    with open(path, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r.get(c, "") for c in cols) + "\n")


def pads(fp):
    """[(pin, x, y, w, h, drill)] for dual (pins/perSide/slots) and explicit styles."""
    if fp["style"] == "explicit":
        return sorted((str(p["pin"]), p["x"], p["y"], p["width"], p["height"], p.get("drill", 0)) for p in fp["pads"])
    if fp["style"] != "dual":
        return None
    per = fp.get("perSide", fp["pins"] // 2)
    slots = fp.get("slots", list(range(1, fp["pins"] + 1)))
    out = []
    for i, s in enumerate(slots):
        if s <= per:
            x, y = -fp["span"] / 2, (s - 1 - (per - 1) / 2) * fp["pitch"]
        else:
            x, y = fp["span"] / 2, ((2 * per - s) - (per - 1) / 2) * fp["pitch"]
        out.append((str(i + 1), x, y, fp["pad"]["width"], fp["pad"]["height"], fp.get("drill", 0)))
    return sorted(out)


def same(a, b):
    pa, pb = pads(a), pads(b)
    if pa is None or pb is None or len(pa) != len(pb):
        return False
    for x, y in zip(pa, pb):
        if x[0] != y[0] or any(abs(u - v) > 0.05 + 1e-9 for u, v in zip(x[1:], y[1:])):
            return False
    return (abs(a["body"]["width"] - b["body"]["width"]) <= 0.1 + 1e-9
            and abs(a["body"]["height"] - b["body"]["height"]) <= 0.1 + 1e-9)


def main(letter, slugs):
    target = os.path.join(DATA, f"footprints-{letter}.json")
    mine = json.load(open(target))["footprints"] if os.path.exists(target) else []
    others = []
    for p in sorted(glob.glob(os.path.join(DATA, "footprints-*.json"))):
        if os.path.abspath(p) != os.path.abspath(target):
            others += json.load(open(p))["footprints"]
    others += json.load(open(PARTS_FP))["footprints"]
    cand, cand_cols = read(CAND)
    by_mpn = {}
    for r in cand:
        by_mpn.setdefault(r["mpn"], []).append(r)
    schemes, _ = read(SCHEMES)
    specs, _ = read(SPECS)
    renamed = {}
    for slug in slugs:
        work = os.path.join(DATA, "work", slug)
        # checks
        checks, _ = read(os.path.join(work, "checks.tsv"))
        for c in checks:
            for r in by_mpn.get(c["mpn"], []):
                if not r["reason"].startswith("check"):
                    continue
                if c["decision"] == "out":
                    r["decision"], r["reason"] = "out", "check; " + c["reason"]
                else:
                    for k in ("circuit", "mounting", "actuator", "rating"):
                        r[k] = c[k]
        # parts the reader could not draw: skip, with the reader's reason
        cannot, _ = read(os.path.join(work, "cannot-draw.tsv"))
        for c in cannot:
            for r in by_mpn.get(c["mpn"], []):
                if r["decision"] in ("add", "pending"):
                    r["decision"] = "skip"
                    r["reason"] = c["reason"]
        # schemes
        rows, _ = read(os.path.join(work, "schemes.tsv"))
        have = {(s["maker"], s["series"], s["regex"]) for s in schemes}
        for s in rows:
            if (s["maker"], s["series"], s["regex"]) not in have:
                schemes.append(s)
        # footprints
        fps = json.load(open(os.path.join(work, "footprints.json")))["footprints"]
        for fp in fps:
            existing = next((o for o in mine + others if o["id"] == fp["id"]), None)
            if existing is not None and {k: v for k, v in existing.items() if k != "note"} == {k: v for k, v in fp.items() if k != "note"}:
                continue
            twin = next((o for o in mine + others if o["id"] != fp["id"] and same(o, fp)), None)
            if twin is not None:
                renamed[fp["id"]] = twin["id"]
                if twin in mine and f"shared with {fp['id']}" not in twin.get("note", ""):
                    twin["note"] = twin.get("note", "") + f"; shared with {fp['id']} ({slug})"
                continue
            if existing is not None:
                raise SystemExit(f"footprint id {fp['id']} ({slug}) is taken by a different land pattern")
            mine.append(fp)
        # specs
        rows, _ = read(os.path.join(work, "specs.tsv"))
        keys = {(s["maker"], s["series"], s["mounting"]) for s in rows}
        specs = [s for s in specs if (s["maker"], s["series"], s["mounting"]) not in keys]
        for s in rows:
            s = dict(s)
            s["footprint"] = renamed.get(s["footprint"], s["footprint"])
            specs.append(s)
    # drop skeleton rows whose group gained real rows; keep the rest sorted
    filled = {(s["maker"], s["series"], s["mounting"]) for s in specs if s["terminals"]}
    specs = [s for s in specs if s["terminals"] or (s["maker"], s["series"], s["mounting"]) not in filled]
    specs.sort(key=lambda s: (s["maker"], s["series"], s["mounting"], s["mpn_regex"]))
    write(CAND, cand, cand_cols)
    write(SCHEMES, schemes, ["maker", "series", "regex", "source"])
    write(SPECS, specs, SPEC_COLS)
    with open(target, "w") as out:
        out.write('{"footprints":[\n')
        out.write(",\n".join(json.dumps(o, ensure_ascii=False, separators=(",", ":")) for o in mine))
        out.write("\n]}\n")
    print(f"{len(mine)} footprints in {os.path.basename(target)}; {len(renamed)} shared with an existing one; "
          f"{len(schemes)} scheme rows; {len(specs)} spec rows")
    for a, b in sorted(renamed.items()):
        print(f"  {a} -> {b}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2:]))
