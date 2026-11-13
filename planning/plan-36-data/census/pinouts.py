"""plan-36-B Phase 1-2: pinouts.tsv and pinout-map.tsv.

usage: python3 -I pinouts.py <candidates.tsv> <atdf-dir> [--transcribed transcribed.tsv]

For each `add` AVR row whose device has an ATDF file with pinouts: its
package code's (geometry, pins) row of package-codes.tsv picks the ATDF
pinout of the same geometry class (atdfmatch.geom, plus `plcc`) and pin count
(atdfmatch.count). Positions the ATDF leaves out are `NC` / nc (B § 3.1).
Pad names are trimmed; roles come from roles.py. atdf-exclude.tsv lists the
ATDF pinouts found wrong against the datasheet; those rows are transcribed. Several matching ATDF
pinouts must have one pad list (B § 2), else the row is reported.

--transcribed: transcribed.tsv (`pdf page figure devices pinout designator
name role`; devices comma-separated, B § 3.3) supplies the pinouts of the
rows the ATDF cannot fill, the ESP32 modules and the RP2040: a row takes the
pinout its own PDF transcribes for its device and its package code's
(geometry, pins).

Writes, beside candidates.tsv: pinouts.tsv (`device pinout designator name
role src`), pinout-map.tsv (`device code pins pinout src`), and the
`pinout_src` column of candidates.tsv (`atdf`, `transcribed`,
`module-table`). Prints counts and every row left without a pinout.
"""
import csv
import os
import re
import sys
import xml.etree.ElementTree as ET
from collections import OrderedDict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import atdfmatch as A  # noqa: E402
import roles as R  # noqa: E402

PREFIX = {"dual": "p", "quad": "quad", "drqfn": "drqfn", "plcc": "plcc", "grid": "bga", "module": "module"}


def load_codes(path):
    out = {}
    for r in csv.DictReader(open(path), delimiter="\t"):
        out[(r["code"], r["pins"])] = r
    return out


def pins_of(row):
    m = re.match(r"AVR\d+[A-Z]{2}(\d+)$", row["device"])
    if m:
        return m.group(1)
    m = re.match(r"(\d+)", row["dk_package"])
    if m:
        return m.group(1)
    return "6" if "SOT-23-6" in row["dk_package"] else ""


def proper(device):
    """The datasheet spelling of a device with no ATDF: ATTINY26 -> ATtiny26,
    ATMEGA8535 -> ATmega8535, ATXMEGA192C3 -> ATxmega192C3; AT90..., AVR...,
    ESP32... and RP2040 as they are."""
    for up, pr in (("ATXMEGA", "ATxmega"), ("ATMEGA", "ATmega"), ("ATTINY", "ATtiny")):
        if device.startswith(up):
            return pr + device[len(up):]
    return device


def atdf_geom(name):
    if re.search(r"PLCC|LCC", name, re.I):
        return "plcc"
    return A.geom(name)


def atdf_pinouts(path):
    root = ET.parse(path).getroot()
    dev = root.find("devices/device")
    pos = OrderedDict()
    for po in root.iter("pinout"):
        pos[po.get("name")] = [(p.get("position"), (p.get("pad") or "").strip()) for p in po.findall("pin")]
    return dev.get("name"), pos


def build(pins, n, numbered):
    """Pad list -> [(designator, name)], gaps filled with NC for a numbered pinout of n pins."""
    by = {d: name for d, name in pins}
    if numbered:
        return [(str(i), by.get(str(i), "NC")) for i in range(1, n + 1)]
    return sorted(pins, key=lambda x: (re.sub(r"\d", "", x[0]), int(re.sub(r"\D", "", x[0]) or 0)))


def main(argv):
    tsv, atdf_dir = argv[0], argv[1]
    here = os.path.dirname(os.path.abspath(tsv))
    codes = load_codes(os.path.join(here, "package-codes.tsv"))
    # ATDF pinouts the name check / figures proved wrong (B § 3.4): transcribed instead.
    exclude = {(r["device"], r["atdf_pinout"]) for r in csv.DictReader(open(os.path.join(here, "atdf-exclude.tsv")), delimiter="\t")}
    rows = list(csv.DictReader(open(tsv), delimiter="\t"))
    cols = list(rows[0].keys())
    cache = {}
    pinouts = OrderedDict()   # (device, pinout) -> [(designator, name, role)], src
    pmap = OrderedDict()      # (device, code, pins) -> (pinout, src)
    missing = []
    counts = {"atdf": 0, "transcribed": 0, "module-table": 0}
    for r in rows:
        if r["verdict"] != "add":
            continue
        r["pinout_src"] = ""
        if r["family"] in ("ESP32", "RP2040") or not r["atdf_device"]:
            continue
        f = os.path.join(atdf_dir, r["atdf_device"])
        if f not in cache:
            cache[f] = atdf_pinouts(f)
        dev, pos = cache[f]
        n = pins_of(r)
        c = codes.get((r["package_code"], n))
        if c is None or not pos:
            continue
        g, n = c["geometry"], int(c["pins"])
        cands = [k for k, v in pos.items() if atdf_geom(k) == g and A.count(k, v) == n and (dev, k) not in exclude]
        if not cands:
            continue
        lists = {tuple(sorted(pos[k])) for k in cands}
        if len(lists) > 1:
            missing.append(f"{r['mpn']}: several ATDF pinouts with different pads {cands}")
            continue
        numbered = all(d.isdigit() for d, _ in pos[cands[0]])
        name = f"{'wlcsp' if g == 'grid' and 'WLCSP' in r['dk_package'].upper() else PREFIX[g]}{n}"
        pins = build(pos[cands[0]], n, numbered)
        key = (dev, name)
        src = f"atdf:{os.path.basename(f)}:{cands[0]}"
        if key in pinouts and [p[:2] for p in pinouts[key][0]] != pins:
            missing.append(f"{r['mpn']}: {dev} {name} differs from an earlier {name} ({pinouts[key][1]} vs {src})")
            continue
        if key not in pinouts:
            pinouts[key] = ([(d, nm, R.role(nm, "avr") or "?") for d, nm in pins], src)
        pmap[(dev, r["package_code"], str(n))] = (name, pinouts[key][1])
        r["pinout_src"] = "atdf"
        counts["atdf"] += 1
    if "--transcribed" in argv:
        # transcribed.tsv rows are keyed by (PDF, device, pinout name); an add row
        # takes the pinout its own PDF transcribes for its device and its
        # package's (geometry, pins) -- module<N> for an ESP32 module, quad57 for
        # the RP2040 (56 pads and the exposed GND pad)
        tr = argv[argv.index("--transcribed") + 1]
        units = OrderedDict()
        for t in csv.DictReader(open(tr), delimiter="\t"):
            src = f"{'module-table' if t['figure'].startswith('module-table') else 'transcribed'}:" \
                  f"{os.path.basename(t['pdf'])}:{('p' + t['page'] + ':') if t['page'] else ''}{t['figure']}"
            units.setdefault((t["pdf"], t["devices"], t["pinout"]), ([], src))[0] \
                .append((t["designator"], t["name"], t["role"]))
        tpins = OrderedDict()
        for (pdf, devs, po), (pins, src) in units.items():
            for dev in devs.split(","):
                key = (pdf, dev.strip().upper(), po)
                if key in tpins and tpins[key][0] != pins:
                    missing.append(f"{dev} {po}: two transcriptions in {os.path.basename(pdf)} disagree")
                tpins.setdefault(key, (pins, src))
        for r in rows:
            if r["verdict"] != "add" or r["pinout_src"]:
                continue
            if r["family"] == "ESP32":
                keys = [k for k in tpins if k[0] == r["pdf"] and k[1] == r["device"].upper() and k[2].startswith("module")]
                kind = "module-table"
            elif r["family"] == "RP2040":
                keys = [k for k in tpins if k[0] == r["pdf"] and k[1] == "RP2040" and k[2] == "quad57"]
                kind = "transcribed"
            else:
                c = codes.get((r["package_code"], pins_of(r)))
                if c is None:
                    continue
                want = f"{PREFIX[c['geometry']]}{c['pins']}"
                keys = [k for k in tpins if k[0] == r["pdf"] and k[1] == r["device"].upper() and k[2] == want]
                kind = "transcribed"
            if not keys:
                continue
            pdf, _, name = keys[0]
            pins, src = tpins[keys[0]]
            dev = proper(r["device"])
            if (dev, name) in pinouts and [p[:2] for p in pinouts[(dev, name)][0]] != [p[:2] for p in pins]:
                missing.append(f"{r['mpn']}: {dev} {name} transcribed differently from an earlier {name}")
                continue
            pinouts.setdefault((dev, name), (pins, src))
            n = pins_of(r) if r["family"] not in ("ESP32", "RP2040") else ("56" if r["family"] == "RP2040" else "")
            pmap[(dev, r["package_code"], n)] = (name, pinouts[(dev, name)][1])
            r["pinout_src"] = kind
            counts[kind] += 1
    for r in rows:
        if r["verdict"] == "add" and not r["pinout_src"]:
            missing.append(f"{r['mpn']}: no pinout ({r['device']} {r['package_code']})")
    with open(os.path.join(here, "pinouts.tsv"), "w") as out:
        out.write("device\tpinout\tdesignator\tname\trole\tsrc\n")
        for (dev, name), (pins, src) in pinouts.items():
            for d, nm, role in pins:
                out.write(f"{dev}\t{name}\t{d}\t{nm}\t{role}\t{src}\n")
    with open(os.path.join(here, "pinout-map.tsv"), "w") as out:
        out.write("device\tcode\tpins\tpinout\tsrc\n")
        for (dev, code, n), (name, src) in pmap.items():
            out.write(f"{dev}\t{code}\t{n}\t{name}\t{src}\n")
    with open(tsv, "w") as out:
        out.write("\t".join(cols) + "\n")
        for r in rows:
            out.write("\t".join(r[c] for c in cols) + "\n")
    print(f"pinouts {len(pinouts)}, map lines {len(pmap)}; rows: " + ", ".join(f"{k} {v}" for k, v in counts.items()))
    print(f"rows without a pinout {sum(1 for m in missing if 'no pinout' in m)}; other problems {sum(1 for m in missing if 'no pinout' not in m)}")
    for m in missing:
        if "no pinout" not in m or "--all" in argv:
            print(" ", m)


if __name__ == "__main__":
    main(sys.argv[1:])
