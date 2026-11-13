"""plan-36-G: Parts/mcu.json and the datasheets.json entries from the census data.

usage: python3 -I gen-parts.py [--write] [--check-esp]

Inputs (this directory's parent): candidates.tsv (`add` rows), devices.tsv,
pinouts.tsv, pinout-map.tsv, package-codes.tsv, footprint-device.tsv,
footprint-mpn.tsv; datasheets/mcu/datasheets.tsv (the URLs).

  default      build everything and print counts and any problem (exit 1 on a problem)
  --write      also write schematical/schematical/Parts/mcu.json (the cpu.json layout:
               one line per key, pin and part) and insert the new datasheets.json
               entries in id order (an existing id is reused, never changed)
  --check-esp  check each ESP32 part's flash / PSRAM decoding against the line of
               its module datasheet's ordering table that names it

Devices (plan-36-G § 3.1): one per devices.tsv device. `id` its lower-case
name, `function` its datasheet spelling (the ATDF file's name, else
pinouts.proper()), `drawer` ["Microcontroller", family]. `core`, `clock`,
`flash`, `ram`, `eeprom` (omitted when 0) and `vcc` from devices.tsv.
Pinouts: those of the device's add rows (pinout-map.tsv, keyed like
pinouts.py: the ATDF device, else proper(device)). Pin layout (§ 3.2):
`group` from the pin name; ports split across the block's two sides, ESP32
/ RP2040 GPIOs at their median.
Parts (§ 3.3): manufacturer, datasheet (the PDF's name in lower case),
package (package-codes.tsv), pinout, footprint (footprint-mpn.tsv, then
footprint-device.tsv, then package-codes.tsv); `flags` automotive; `vcc` /
`clock` when DigiKey's values differ from the device's (AVR V / L grades;
not for ESP32 / RP2040, whose DigiKey speed is empty or the device's);
ESP32 `flash` / `psram` from the ordering code (N<n> / H<n> flash MB, R<n>
PSRAM MB).
"""
import csv
import json
import os
import re
import subprocess
import sys
from collections import OrderedDict, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.join(HERE, "..")
ROOT = os.path.abspath(os.path.join(DATA, "..", ".."))
sys.path.insert(0, HERE)
import pinouts as P  # noqa: E402

MB = 1024 * 1024
USB = {"D+", "D-", "DM", "DP", "USB_DP", "USB_DM", "VBUS", "UCAP"}
PROGRAM = {"UPDI", "PDI", "PDI_DATA", "RESET", "RUN", "SWCLK", "SWDIO", "EN"}
SIDED = {"input", "output", "inout"}


def tsv(name):
    return list(csv.DictReader(open(os.path.join(DATA, name)), delimiter="\t"))


def group(name, family):
    if re.fullmatch(r"P[A-Z]\d+", name):
        return "port-" + name[1].lower()
    if family in ("ESP32", "RP2040") and re.fullmatch(r"(?:IO|GPIO|I)\d+", name):
        return "gpio"
    if name in USB:
        return "usb"
    if name in PROGRAM:
        return "program"
    if re.match(r"XTAL|TOSC", name) or name in ("XIN", "XOUT"):
        return "clock"
    if name.startswith("QSPI_"):
        return "qspi"
    if re.match(r"SEG|COM|BIAS|CAP", name) and name != "UCAP" or name in ("LCDCAP", "VLCD"):
        return "lcd"
    return "default"


def sides(pins, family):
    """{index: "in" | "out"} per § 3.2."""
    out = {}
    ports = defaultdict(list)
    gpio = []
    for i, p in enumerate(pins):
        if p["role"] not in SIDED:
            continue
        g = p.get("group", "default")
        if g.startswith("port-"):
            ports[g].append(i)
        elif g == "gpio":
            gpio.append(i)
    total = sum(len(v) for v in ports.values())
    left = 0
    going_left = True
    for g in sorted(ports):
        if going_left and left + len(ports[g]) <= total / 2:
            left += len(ports[g])
            side = "in"
        else:
            going_left = False
            side = "out"
        for i in ports[g]:
            out[i] = side
    if gpio:
        nums = sorted(int(re.search(r"\d+", pins[i]["name"]).group()) for i in gpio)
        median = nums[len(nums) // 2] if len(nums) % 2 else (nums[len(nums) // 2 - 1] + nums[len(nums) // 2]) / 2
        for i in gpio:
            out[i] = "in" if int(re.search(r"\d+", pins[i]["name"]).group()) < median else "out"
    return out


def ball_key(d):
    m = re.fullmatch(r"([A-Z]+)(\d+)", d)
    return (len(m.group(1)), m.group(1), int(m.group(2)))


def volts(text):
    m = re.fullmatch(r"([\d.]+)V ~ ([\d.]+)V", text.strip())
    return (float(m.group(1)), float(m.group(2))) if m else None


def hz(text):
    m = re.fullmatch(r"([\d.]+)(k|M|G)Hz", text.strip())
    if not m:
        return None
    return int(round(float(m.group(1)) * {"k": 1e3, "M": 1e6, "G": 1e9}[m.group(2)]))


def num(v):
    v = float(v)
    return int(v) if v == int(v) else v


def esp_memory(mpn):
    """(flash bytes, psram bytes) from an ESP32 ordering code: -N8, -H4, -N4R2, -N16R8."""
    m = re.search(r"-(?:[NH])(\d+)(?:R(\d+))?[A-Z]?(?:-[A-Z]+)?$", mpn)
    if not m:
        return None
    return int(m.group(1)) * MB, (int(m.group(2)) * MB if m.group(2) else 0)


def describe(family, device, flash):
    size = f"{flash // 1024} KB" if flash >= 1024 else f"{flash} B"
    if family == "AVR XMEGA":
        return f"8/16-bit AVR XMEGA microcontroller, {size} flash"
    if family.startswith("AVR"):
        return f"8-bit AVR microcontroller, {size} flash"
    if family == "RP2040":
        return "Dual Cortex-M0+ microcontroller"
    soc = re.match(r"ESP32(-[A-Z]\d)?", device).group(0)
    radio = {"ESP32": "Wi-Fi + Bluetooth + Bluetooth LE", "ESP32-S2": "Wi-Fi",
             "ESP32-S3": "Wi-Fi + Bluetooth LE", "ESP32-C3": "Wi-Fi + Bluetooth LE",
             "ESP32-C6": "Wi-Fi 6 + Bluetooth LE + 802.15.4"}[soc]
    return f"{radio} module ({soc})"


def build():
    problems = []
    rows = [r for r in tsv("candidates.tsv") if r["verdict"] == "add"]
    devs = {r["device"].upper(): r for r in tsv("devices.tsv")}
    pins_by = defaultdict(list)
    src_by = {}
    for r in tsv("pinouts.tsv"):
        pins_by[(r["device"].upper(), r["pinout"])].append(r)
        src_by[(r["device"].upper(), r["pinout"])] = r["src"]
    pmap = {(r["device"].upper(), r["code"], r["pins"]): r for r in tsv("pinout-map.tsv")}
    codes = {(r["code"], r["pins"]): r for r in tsv("package-codes.tsv")}
    dev_fp = {(r["device"].upper(), r["code"], r["pins"]): r["footprint"] for r in tsv("footprint-device.tsv")}
    mpn_fp = {r["mpn"]: r["footprint"] for r in tsv("footprint-mpn.tsv")}
    urls = OrderedDict()
    for r in csv.DictReader(open(os.path.join(ROOT, "datasheets/mcu/datasheets.tsv")), delimiter="\t"):
        # the format wants https: ww1.microchip.com serves the same files over it
        urls.setdefault(r["path"], re.sub(r"^http://ww1\.microchip\.com/", "https://ww1.microchip.com/", r["url"]))

    devices = OrderedDict()
    sheets = {}
    for r in rows:
        d = devs.get(r["device"].upper())
        if d is None:
            problems.append(f"{r['mpn']}: device {r['device']} not in devices.tsv")
            continue
        n = P.pins_of(r) if r["family"] not in ("ESP32", "RP2040") else ("56" if r["family"] == "RP2040" else "")
        key_dev = r["atdf_device"][:-5] if r["atdf_device"].endswith(".atdf") else P.proper(r["device"])
        m = pmap.get((key_dev.upper(), r["package_code"], n)) or pmap.get((r["device"].upper(), r["package_code"], n))
        if m is None:
            problems.append(f"{r['mpn']}: no pinout-map line for ({key_dev}, {r['package_code']}, {n})")
            continue
        pkey = (m["device"].upper(), m["pinout"])
        c = codes.get((r["package_code"], n))
        fp = mpn_fp.get(r["mpn"]) or dev_fp.get((r["device"].upper(), r["package_code"], n)) or (c or {}).get("footprint")
        if not fp or not c or not c["package"]:
            problems.append(f"{r['mpn']}: footprint {fp!r} package {(c or {}).get('package')!r}")
            continue
        did = re.sub(r"[/ ]", "-", r["device"].lower())
        dv = devices.setdefault(r["device"], {
            "id": did, "family": r["family"], "name": P.proper(d["device"]), "facts": d,
            "pinouts": OrderedDict(), "srcs": OrderedDict(), "parts": []})
        po = m["pinout"]
        if po in dv["pinouts"] and dv["pinouts"][po][0] != pkey:
            a = [(p["designator"], p["name"]) for p in pins_by[dv["pinouts"][po][0]]]
            b = [(p["designator"], p["name"]) for p in pins_by[pkey]]
            if a != b:
                problems.append(f"{r['mpn']}: pinout {po} differs between {dv['pinouts'][po][0][0]} and {pkey[0]}")
                continue
        dv["pinouts"].setdefault(po, (pkey,))
        dv["srcs"].setdefault(po, src_by[pkey])
        pdf = r["pdf"]
        sid = os.path.basename(pdf)[:-4].lower()
        if pdf not in urls:
            problems.append(f"{r['mpn']}: {pdf} has no URL in datasheets.tsv")
            continue
        sheets[sid] = urls[pdf]
        part = OrderedDict([("name", r["mpn"]), ("manufacturer", r["maker"]), ("datasheet", sid),
                            ("package", c["package"]), ("pinout", po), ("footprint", fp)])
        if r["grade"] == "automotive":
            part["flags"] = ["automotive"]
        if r["family"] not in ("ESP32", "RP2040"):
            v = volts(r["dk_vcc"])
            if v and (v[0] != float(d["vcc_min"]) or v[1] != float(d["vcc_max"])):
                part["vcc"] = [num(v[0]), num(v[1])]
            h = hz(r["dk_speed"])
            if h and h != int(d["clock_hz"]):
                part["clock"] = h
        if r["family"] == "ESP32":
            mem = esp_memory(r["mpn"])
            if mem is None:
                problems.append(f"{r['mpn']}: no flash code")
                continue
            part["flash"] = mem[0]
            if mem[1]:
                part["psram"] = mem[1]
        dv["parts"].append(part)

    out = []
    for name in sorted(devices, key=lambda k: devices[k]["id"]):
        dv = devices[name]
        d = dv["facts"]
        obj = OrderedDict()
        obj["id"] = dv["id"]
        obj["function"] = dv["name"]
        obj["description"] = describe(dv["family"], dv["name"], int(d["flash"]))
        obj["drawer"] = ["Microcontroller", dv["family"]]
        obj["note"] = ("facts: " + d["src"] + "; pinouts: " +
                       "; ".join(f"{po} {s}" for po, s in dv["srcs"].items()))
        obj["simulated"] = False
        obj["core"] = d["core"]
        obj["clock"] = int(d["clock_hz"])
        obj["flash"] = int(d["flash"])
        obj["ram"] = int(d["ram"])
        if int(d["eeprom"]):
            obj["eeprom"] = int(d["eeprom"])
        obj["vcc"] = [num(d["vcc_min"]), num(d["vcc_max"])]
        pos = OrderedDict()
        for po in sorted(dv["pinouts"], key=lambda s: (re.sub(r"\d+", "", s), int(re.sub(r"\D", "", s) or 0))):
            src = pins_by[dv["pinouts"][po][0]]
            numbered = all(p["designator"].isdigit() for p in src)
            src = sorted(src, key=lambda p: int(p["designator"]) if numbered else ball_key(p["designator"]))
            pins = []
            for p in src:
                pin = OrderedDict()
                if numbered:
                    pin["number"] = int(p["designator"])
                else:
                    pin["ball"] = p["designator"]
                pin["name"] = p["name"]
                pin["role"] = p["role"]
                g = group(p["name"], dv["family"])
                if g != "default":
                    pin["group"] = g
                pins.append(pin)
            for i, side in sides(pins, dv["family"]).items():
                pins[i]["side"] = side
            if numbered and [p["number"] for p in pins] != list(range(1, len(pins) + 1)):
                problems.append(f"{dv['id']} {po}: numbers not 1..N")
            pos[po] = pins
        obj["pinouts"] = pos
        obj["parts"] = sorted(dv["parts"], key=lambda p: p["name"])
        out.append(obj)
    return out, sheets, problems, rows


def dumps_device(d):
    lines = ["    {"]
    keys = [k for k in d if k not in ("pinouts", "parts")]
    for k in keys:
        lines.append(f"      {json.dumps(k)}: {json.dumps(d[k], ensure_ascii=False)},")
    lines.append('      "pinouts": {')
    pos = list(d["pinouts"].items())
    for j, (po, pins) in enumerate(pos):
        lines.append(f"        {json.dumps(po)}: [")
        for i, p in enumerate(pins):
            lines.append("          " + json.dumps(p, separators=(",", ":"), ensure_ascii=False) + ("," if i < len(pins) - 1 else ""))
        lines.append("        ]" + ("," if j < len(pos) - 1 else ""))
    lines.append("      },")
    lines.append('      "parts": [')
    for i, p in enumerate(d["parts"]):
        lines.append("        " + json.dumps(p, separators=(",", ":"), ensure_ascii=False) + ("," if i < len(d["parts"]) - 1 else ""))
    lines.append("      ]")
    lines.append("    }")
    return "\n".join(lines)


def check_esp(rows):
    """Each ESP32 part's decoded flash / PSRAM against its ordering table: the
    table row naming the part (the comparison table, whose header names a
    Flash and maybe a PSRAM column), each value the nearest "<n> MB" in that
    column within 3 lines (the tables merge a value across the rows that
    share it). A part no table row names (ESP32-C3-MINI-1-N4-A, the AWS
    ExpressLink twin of -N4) is reported and read by hand."""
    bad = 0
    texts = {}
    for r in rows:
        if r["family"] != "ESP32":
            continue
        if r["pdf"] not in texts:
            texts[r["pdf"]] = subprocess.run(["pdftotext", "-layout", "-l", "8", os.path.join(ROOT, r["pdf"]), "-"],
                                             capture_output=True, text=True).stdout.splitlines()
        lines = texts[r["pdf"]]
        mem = esp_memory(r["mpn"])
        want = (mem[0] // MB, mem[1] // MB)
        hit = None
        for i, l in enumerate(lines):
            m = re.match(r"\s*" + re.escape(r["mpn"]) + r"(?![\w-])", l)
            if not m:
                continue
            hdr = next((j for j in range(i - 1, max(i - 25, -1), -1) if "Part Number" in lines[j]), None)
            if hdr is None:
                continue
            cols = {}
            for j in (hdr - 1, hdr, hdr + 1):
                for name in ("Flash", "PSRAM"):
                    k = lines[j].find(name) if 0 <= j < len(lines) else -1
                    if k >= 0 and name not in cols:
                        cols[name] = k
            if "Flash" not in cols:
                continue
            tokens = []
            for j in range(i - 3, i + 4):
                if 0 <= j < len(lines) and not re.match(r"\s*\d\s+[A-Z][a-z]", lines[j]):
                    for t in re.finditer(r"(\d+) MB|[—-](?=\s{2}|$)", lines[j]):
                        col = min(cols, key=lambda c: abs(cols[c] - t.start()))
                        tokens.append((abs(j - i), col, int(t.group(1)) if t.group(1) else 0))
            got = []
            for col in ("Flash", "PSRAM"):
                if col not in cols:
                    got.append(0)
                    continue
                c = sorted(t for t in tokens if t[1] == col)
                got.append(c[0][2] if c else None)
            hit = (got, re.sub(r"\s+", " ", l.strip())[:90])
            break
        if hit is None:
            print(f"?   {r['mpn']}: no table row names it; decoded {want} MB")
            bad += 1
            continue
        ok = hit[0] == list(want)
        print(f"{'ok ' if ok else 'BAD'} {r['mpn']}: decoded flash/PSRAM {list(want)} MB, table {hit[0]}: {hit[1]}")
        bad += not ok
    print(f"{bad} ESP32 parts not confirmed")
    return bad


def main(argv):
    devices, sheets, problems, rows = build()
    nparts = sum(len(d["parts"]) for d in devices)
    print(f"{len(devices)} devices, {nparts} parts of {len(rows)} add rows, {len(sheets)} datasheets")
    for p in problems:
        print("  PROBLEM", p)
    if "--check-esp" in argv:
        check_esp(rows)
    if problems or nparts != len(rows):
        return 1
    if "--write" in argv:
        path = os.path.join(ROOT, "schematical/schematical/Parts/mcu.json")
        with open(path, "w") as f:
            f.write('{\n  "format": "schematical-parts",\n  "version": 1,\n  "kind": "mcu",\n  "devices": [\n')
            f.write(",\n".join(dumps_device(d) for d in devices))
            f.write("\n  ]\n}\n")
        dpath = os.path.join(ROOT, "schematical/schematical/Parts/datasheets.json")
        ds = json.load(open(dpath))
        have = {d["id"] for d in ds["datasheets"]}
        added = [{"id": i, "url": u} for i, u in sheets.items() if i not in have]
        text = open(dpath).read()
        for e in sorted(added, key=lambda e: e["id"]):
            # insert before the first existing entry whose id sorts after it (the file is in id order)
            line = "    " + json.dumps(e, ensure_ascii=False)
            after = next((d["id"] for d in ds["datasheets"] if d["id"] > e["id"]), None)
            if after is None:
                raise SystemExit(f"{e['id']}: sorts last; insert by hand")
            anchor = re.search(r'\n(\s*)\{\s*"id":\s*' + re.escape(json.dumps(after)), text)
            text = text[:anchor.start()] + "\n" + anchor.group(1) + json.dumps(e, ensure_ascii=False) + "," + text[anchor.start():]
            ds["datasheets"].append(e)
            ds["datasheets"].sort(key=lambda d: d["id"])
        open(dpath, "w").write(text)
        print(f"wrote mcu.json; {len(added)} datasheets.json entries added, {len(sheets) - len(added)} reused")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
