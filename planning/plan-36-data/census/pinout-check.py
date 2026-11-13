"""plan-36-B Phase 3 (§ 3.4): the name check of every ATDF pinout used.

usage: python3 -I pinout-check.py <candidates.tsv> [--full full-checks.tsv]

For each (device, pinout) of pinouts.tsv whose src is the ATDF: every pin
name except NC must occur as a token in the text of its datasheet's
pin-configuration pages: the pages whose text has `Pin Configuration`,
`Pinout` or `Pin-out` and names the device (textscan.names, list titles
expanded). The PDFs tried are those of the add rows mapped to the pinout;
one passing PDF is enough. Writes pinout-checks.tsv (`device pinout pdf
pages result`): `ok`, or `missing: <names>`. A pinout the text cannot
check (an image figure, a figure that prints a pad's function) takes its
by-eye result from eye-checks.tsv. --full adds the ten full checks of
full-checks.tsv (`device pinout pdf pages result`) as they are.
"""
import csv
import os
import re
import subprocess
import sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import textscan as T  # noqa: E402

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))


def pages_of(pdf, cache={}):
    if pdf not in cache:
        txt = subprocess.run(["pdftotext", "-q", "-layout", os.path.join(ROOT, pdf), "-"],
                             capture_output=True, text=True, timeout=600).stdout
        cache[pdf] = [T.dashes(p) for p in txt.split("\f")]
    return cache[pdf]


# ATDF spellings of pads the figures print another way (same pad, same rail):
#   AGND -> GND (megaAVR 0 prints GND beside AVDD; AVR DD/SD print GND),
#   AVDD -> VDD (AVR DD/SD have one supply, which the ATDF calls AVDD),
#   REF -> AREF (ATmega164/324/644 family), DM / DP -> D- / D+ (ATmega16U4/32U4)
ALIAS = {"AGND": ("GND",), "AVDD": ("VDD",), "REF": ("AREF",), "DM": ("D-",), "DP": ("D+",)}


def present(name, toks):
    return name.upper() in toks or any(a in toks for a in ALIAS.get(name.upper(), ()))


def ocr(pdf, page, cache={}):
    """Tokens tesseract reads on one page (rendered at 300 dpi), for figures
    that are images. Cached under /tmp/p36/ocr."""
    key = (pdf, page)
    if key not in cache:
        d = "/tmp/p36/ocr"
        os.makedirs(d, exist_ok=True)
        stem = os.path.join(d, re.sub(r"[^A-Za-z0-9]", "_", os.path.basename(pdf)) + f"-{page}")
        txt = stem + ".txt"
        if not os.path.exists(txt):
            subprocess.run(["pdftoppm", "-r", "300", "-png", "-singlefile", "-f", str(page), "-l", str(page),
                            os.path.join(ROOT, pdf), stem], capture_output=True, timeout=300)
            subprocess.run(["tesseract", stem + ".png", stem, "--psm", "11"], capture_output=True, timeout=300)
        text = open(txt, errors="replace").read().upper() if os.path.exists(txt) else ""
        cache[key] = set(re.findall(r"[A-Z0-9_+\-]+", text))
    return cache[key]


def main(argv):
    tsv = argv[0]
    here = os.path.dirname(os.path.abspath(tsv))
    pins = defaultdict(list)
    src = {}
    for r in csv.DictReader(open(os.path.join(here, "pinouts.tsv")), delimiter="\t"):
        pins[(r["device"], r["pinout"])].append(r["name"])
        src[(r["device"], r["pinout"])] = r["src"]
    pmap = defaultdict(set)
    for r in csv.DictReader(open(os.path.join(here, "pinout-map.tsv")), delimiter="\t"):
        pmap[(r["device"].upper(), r["code"], r["pins"])].add(r["pinout"])
    pdfs = defaultdict(set)
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import pinouts as P  # noqa: E402
    for r in csv.DictReader(open(tsv), delimiter="\t"):
        if r["verdict"] != "add" or r["pinout_src"] != "atdf":
            continue
        dev = r["atdf_device"][:-5] if r["atdf_device"].endswith(".atdf") else r["device"]
        for po in pmap.get((dev.upper(), r["package_code"], P.pins_of(r)), ()):
            pdfs[(dev, po)].add(r["pdf"])
    eye = {}
    eye_path = os.path.join(here, "eye-checks.tsv")
    if os.path.exists(eye_path):
        for r in csv.DictReader(open(eye_path), delimiter="\t"):
            eye[(r["device"], r["pinout"])] = (r["pdf"], r["pages"], r["result"])
    out = []
    bad = 0
    for (dev, po), names in pins.items():
        if not src[(dev, po)].startswith("atdf"):
            continue
        want = sorted({n for n in names if n not in ("NC", "DNC")})
        best = None
        for pdf in sorted(pdfs.get((dev, po), ())):
            pages = pages_of(pdf)
            # pin-configuration pages, not the table of contents (dot leaders)
            pin_pages = [i for i, p in enumerate(pages[:40]) if re.search(r"Pin\s*Configuration|Pinout|Pin-out", p, re.I)
                         and p.count("......") < 5]
            # figures continue on the next pages (`Figure 5-3. 32-pin TQFP` after the 5.1 heading page)
            pin_pages = sorted({j for i in pin_pages for j in range(i, min(i + 4, len(pages)))
                                if pages[j].count("......") < 5})
            hits = [i for i in pin_pages if dev.upper() in T.names(pages[i].upper())]
            how = ""
            if not hits:
                # the figure page does not repeat the device name (the title page
                # lists the family); the PDF is the device's own datasheet
                hits, how = pin_pages[:4], " (pinout pages of the device's datasheet)"
            toks = set()
            for i in hits:
                toks |= set(re.findall(r"[A-Z0-9_+\-]+", pages[i].upper()))
            missing = [n for n in want if not present(n, toks)]
            if missing and hits:
                # the figure is an image (Atmel XMEGA): OCR the pages at 300 dpi
                for i in sorted(set(hits) | set(pin_pages[:8])):
                    toks |= ocr(pdf, i + 1)
                missing = [n for n in want if not present(n, toks)]
                how += " (OCR)"
            res = (pdf, ",".join(str(i + 1) for i in hits) + how, "ok" if hits and not missing else
                   ("missing: " + " ".join(missing) if hits else "no pin-configuration page"))
            if best is None or res[2] == "ok" or (best[2] != "ok" and len(res[2]) < len(best[2])):
                best = res
        if best is None:
            best = ("", "", "no add row maps to it")
        if best[2] != "ok" and (dev, po) in eye:
            best = eye[(dev, po)]  # read on the figure (eye-checks.tsv)
        bad += not best[2].startswith("ok")
        out.append((dev, po, best[0], best[1], best[2]))
    if "--full" in argv:
        for r in csv.DictReader(open(argv[argv.index("--full") + 1]), delimiter="\t"):
            out.append((r["device"], r["pinout"], r["pdf"], r["pages"], r["result"]))
    with open(os.path.join(here, "pinout-checks.tsv"), "w") as f:
        f.write("device\tpinout\tpdf\tpages\tresult\n")
        for row in out:
            f.write("\t".join(row) + "\n")
    print(f"{len(out)} checks, {bad} name checks not ok")
    for row in out:
        if row[4] != "ok" and not row[4].startswith("full ok"):
            print(" ", row[0], row[1], os.path.basename(row[2]), row[4][:120])


if __name__ == "__main__":
    main(sys.argv[1:])
