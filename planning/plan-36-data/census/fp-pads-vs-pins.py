"""plan-36-D/E: every add row's footprint has a pad of each of its pinout's designators.

usage: python3 -I fp-pads-vs-pins.py <footprints-X.json>... <pinouts.tsv> [--all-rows]

Default (E's check): for each ESP32 add row, the footprint its MPN gets
(footprint-mpn.tsv, else package-codes.tsv) has exactly the pinout's
designators.
--all-rows: every add row of candidates.tsv (pinout via pinout-map.tsv,
footprint via footprint-mpn.tsv, footprint-device.tsv, then
package-codes.tsv): the
footprint's pin count equals the pinout's, and every pinout designator has a
pad (numbered footprints `1`..`pins`; grid `balls`; explicit pads; the
library's footprints.json too). Prints each mismatch and `<n> mismatches`.
"""
import csv
import json
import os
import re
import sys
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(HERE, "..")
ROOT = os.path.abspath(os.path.join(DATA, "..", ".."))
sys.path.insert(0, HERE)
import pinouts as P  # noqa: E402


def designators(f):
    if f["style"] == "grid":
        return set(f["balls"])
    if f["style"] == "explicit":
        out = set()
        for p in f["pads"]:
            if "ball" in p:
                out.add(p["ball"])
            elif p.get("pin", 0) > 0:
                out.add(str(p["pin"]))
        return out
    return {str(i) for i in range(1, f["pins"] + 1)}


def main(argv):
    files = [a for a in argv if a.endswith(".json")]
    pinouts_path = next(a for a in argv if a.endswith("pinouts.tsv"))
    fps = {f["id"]: f for f in json.load(open(os.path.join(ROOT, "schematical/schematical/Parts/footprints.json")))["footprints"]}
    for p in files:
        for f in json.load(open(p))["footprints"]:
            fps[f["id"]] = f
    pins = defaultdict(list)
    for r in csv.DictReader(open(pinouts_path), delimiter="\t"):
        pins[(r["device"], r["pinout"])].append(r["designator"])
    codes = {(r["code"], r["pins"]): r for r in csv.DictReader(open(os.path.join(DATA, "package-codes.tsv")), delimiter="\t")}
    dev_fp = {}
    dp = os.path.join(DATA, "footprint-device.tsv")
    if os.path.exists(dp):
        for r in csv.DictReader(open(dp), delimiter="\t"):
            dev_fp[(r["device"].upper(), r["code"], r["pins"])] = r["footprint"]
    mpn_fp = {}
    mp = os.path.join(DATA, "footprint-mpn.tsv")
    if os.path.exists(mp):
        for r in csv.DictReader(open(mp), delimiter="\t"):
            mpn_fp[r["mpn"]] = r["footprint"]
    pmap = {}
    for r in csv.DictReader(open(os.path.join(DATA, "pinout-map.tsv")), delimiter="\t"):
        pmap[(r["device"].upper(), r["code"], r["pins"])] = (r["device"], r["pinout"])
    bad = n = 0
    seen = set()
    for r in csv.DictReader(open(os.path.join(DATA, "candidates.tsv")), delimiter="\t"):
        if r["verdict"] != "add":
            continue
        if "--all-rows" not in argv and r["family"] != "ESP32":
            continue
        k = (r["package_code"], P.pins_of(r) if r["family"] not in ("ESP32", "RP2040") else ("" if r["family"] == "ESP32" else "56"))
        dev = r["atdf_device"][:-5].upper() if r["atdf_device"].endswith(".atdf") else r["device"].upper()
        po = None
        for key in ((dev, k[0], k[1]), (r["device"].upper(), k[0], k[1])):
            if key in pmap:
                po = pmap[key]
                break
        if po is None:
            cands = [v for kk, v in pmap.items() if kk[0] in (dev, r["device"].upper()) and kk[1] == k[0]]
            po = cands[0] if cands else None
        fid = mpn_fp.get(r["mpn"]) or dev_fp.get((r["device"].upper(), k[0], k[1])) or (codes.get(k) or {}).get("footprint")
        if (po, fid) in seen:
            continue
        seen.add((po, fid))
        n += 1
        if po is None or fid not in fps:
            print(f"{r['mpn']}: pinout {po} footprint {fid!r} missing")
            bad += 1
            continue
        want = set(pins[po])
        f = fps[fid]
        have = designators(f)
        exact = r["family"] == "ESP32"
        if f["pins"] != len(want) or not want <= have or (exact and want != have):
            print(f"{r['mpn']}: {po[0]} {po[1]} ({len(want)} pins) vs {fid} ({f['pins']} pins): missing {sorted(want - have)[:8]}")
            bad += 1
    print(f"{n} (pinout, footprint) pairs, {bad} mismatches")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
