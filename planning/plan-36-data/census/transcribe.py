"""plan-36-B Phase 2 (§ 3.3): transcribe datasheet pinout figures from the PDF's word boxes.

usage: python3 -I transcribe.py <transcribe-units.tsv> <out transcribed-auto.tsv> [--only <pdf substring>]

For each AVR unit (PDF, geometry, pins) of transcribe-units.tsv: every page
whose text has "Pinout" / "Pin Configuration" / "Pin-out" is read with
`pdftotext -bbox` (the word boxes, not the reflowed text). Pin-number words
(1..N) are clustered into figures (numbers within 50 pt of one another),
and each number is paired with the nearest pad name aligned with it: on its
row (left / right sides) or in its column (top / bottom sides), on the side
away from the figure's centre. A pad name is a word roles.py knows (a port
`P[A-R][0-9]`, VCC, GND, ...; `DNC` / `NC`), i.e. the datasheet's primary
name outside the parentheses (B § 3.1).

Numbers link into one figure when within 50 pt of each other (centres). A figure is kept when it numbers 1..N exactly once and every number found a
name, and its layout matches the unit's geometry (dual: two columns of N/2;
quad: four sides; plcc: four sides). The first such figure in page order is
the unit's transcription; the others with the same pins are reported as
`agree` / `DIFFER` (a datasheet that draws one package twice must draw it the
same way). Grid (BGA) units are not read here (transcribed by hand).

Output rows: `pdf page figure devices pinout designator name role`, the
transcribed.tsv format; `figure` is `auto:<page>:<first caption line above>`.
"""
import csv
import os
import re
import subprocess
import sys
from collections import defaultdict
from html import unescape

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pkgcheck as P  # noqa: E402
import roles as R  # noqa: E402

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
WORD = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">([^<]*)</word>')
PAGE = re.compile(r"<page ")
DEBUG = "--debug" in sys.argv


def page_words(pdf, page):
    out = subprocess.run(["pdftotext", "-q", "-f", str(page), "-l", str(page), "-bbox", pdf, "-"],
                         capture_output=True, text=True, timeout=120).stdout
    words = []
    for m in WORD.finditer(out):
        x0, y0, x1, y1 = map(float, m.groups()[:4])
        words.append((x0, y0, x1, y1, unescape(m.group(5))))
    return words


def pin_pages(pdf):
    txt = subprocess.run(["pdftotext", "-q", "-layout", pdf, "-"], capture_output=True, text=True, timeout=600).stdout
    pages = txt.split("\f")
    return [i + 1 for i, p in enumerate(pages) if re.search(r"Pinout|Pin[- ]?out|Pin\s+Configuration", p, re.I)], pages


def pad_name(w):
    """The pad name a figure word gives, or None: a footnote mark is dropped
    (`NC*`), and of a slash pair (`RESET/PG5`) the port name is kept, the
    ATDF's spelling (B § Decisions: bare pad names)."""
    w = re.sub(r"(\*+|\(\d\))$", "", w).upper()  # some figures print `nc`, `UVcc`, `Reset`
    if "(" in w and not w.startswith("("):
        w = re.sub(r"\([^)]*\)?", "", w)  # `PF1(ADC1)`: the name glued to its alternate function
    if w in ("DNC", "NC") or R.role(w, "avr") is not None:
        return w
    parts = w.split("/")
    if len(parts) > 1:
        ports = [x for x in parts if R.PORT.match(x)]
        if ports:
            return ports[0]
        known = [x for x in parts if R.role(x, "avr") is not None]
        if known:
            return known[0]
    return None


def is_name(w):
    return pad_name(w) is not None


def cx(w):
    return (w[0] + w[2]) / 2


def cy(w):
    return (w[1] + w[3]) / 2


def clusters(nums, gap=50.0, rows=True):
    """Number words linked when their centres are within `gap` pt (neighbours
    on a side are ~10-20 pt apart) or on one baseline within 130 pt (the two
    columns of a dual figure, 40-90 pt apart); separate figures are further
    apart."""
    parent = list(range(len(nums)))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i
    for i in range(len(nums)):
        for j in range(i + 1, len(nums)):
            a, b = nums[i], nums[j]
            dx, dy = abs(cx(a) - cx(b)), abs(cy(a) - cy(b))
            # near neighbours (a side, a corner), or the two ends of one row of
            # a dual figure (same baseline, up to 130 pt apart across the body)
            if dx * dx + dy * dy < gap * gap or (rows and dy < 3 and dx < 130):
                parent[find(i)] = find(j)
    groups = defaultdict(list)
    for i, w in enumerate(nums):
        groups[find(i)].append(w)
    return list(groups.values())


def figure_pins(words, n):
    nums = [w for w in words if re.fullmatch(r"\d{1,3}", w[4]) and 1 <= int(w[4]) <= n]
    names = [w for w in words if is_name(w[4])]
    found = []
    seen = set()
    groups = []
    # without, then with the across-the-body row links; then tighter gaps, which
    # split figures drawn close together (a PDIP beside a TQFP)
    for gap, rows in ((50, False), (50, True), (35, False), (35, True), (25, True), (25, False)):
        for g in clusters(nums, gap=gap, rows=rows):
            key = frozenset(id(w) for w in g)
            if key not in seen:
                seen.add(key)
                groups.append(g)
    for g in groups:
        got = {}
        for w in g:
            got.setdefault(int(w[4]), w)
        if sorted(got) != list(range(1, n + 1)) or len(g) != n:
            if DEBUG and len(g) > n // 2:
                print("   cluster of", len(g), "numbers; missing", sorted(set(range(1, n + 1)) - set(got))[:10],
                      "extra", len(g) - len(got))
            continue
        mx = sum(cx(w) for w in g) / n
        my = sum(cy(w) for w in g) / n
        pins = {}
        for k, w in got.items():
            best = None
            for nm in names:
                dx, dy = cx(nm) - cx(w), cy(nm) - cy(w)
                # the gap between the boxes' facing edges, so a long word
                # (`PB7(ADC4/PSCOUT01/SCK)`) pairs by its near end
                if abs(dy) < 4 and (dx * (cx(w) - mx) > 0):
                    d = nm[0] - w[2] if dx > 0 else w[0] - nm[2]
                elif abs(dx) < 4 and (dy * (cy(w) - my) > 0):
                    d = nm[1] - w[3] if dy > 0 else w[1] - nm[3]
                else:
                    continue
                if not -2 <= d < 60:
                    continue
                if best is None or d < best[0]:
                    best = (d, nm)
            if best is None:
                if DEBUG:
                    near = sorted(words, key=lambda v: abs(cx(v) - cx(w)) + abs(cy(v) - cy(w)))[1:5]
                    print("   no name for", k, "at", round(cx(w)), round(cy(w)), "near:", [v[4] for v in near])
                break
            pins[k] = best[1]
        if len(pins) != n:
            continue
        found.append((g, pins, (mx, my)))
    return found


def sides(g):
    """Counts of the figure's numbers on its left, right, top and bottom edges."""
    x0, x1 = min(cx(w) for w in g), max(cx(w) for w in g)
    y0, y1 = min(cy(w) for w in g), max(cy(w) for w in g)
    out = {"l": 0, "r": 0, "t": 0, "b": 0}
    for w in g:
        d = {"l": cx(w) - x0, "r": x1 - cx(w), "t": cy(w) - y0, "b": y1 - cy(w)}
        out[min(d, key=d.get)] += 1
    return out


def layout_ok(g, geometry, n):
    """dual: two columns of N/2 numbers; quad / plcc: N/4 on each side."""
    s = sides(g)
    if geometry == "dual":
        # two columns: split at the mean x; each column N/2 numbers, each
        # narrow (`1` and `10` centre a few pt apart)
        mx = sum(cx(w) for w in g) / len(g)
        left = [cx(w) for w in g if cx(w) < mx]
        right = [cx(w) for w in g if cx(w) >= mx]
        return (len(left) == len(right) == n // 2 and max(left) - min(left) < 15
                and max(right) - min(right) < 15)
    return all(abs(v - n / 4) <= 1 for v in s.values())


PKG_WORD = re.compile(r"PDIP|SPDIP|DIP|SOIC|SOIJ|SO\d|SSOP|TSSOP|TQFP|LQFP|QFP|QFN|MLF|VQFN|WQFN|UQFN|PLCC|DFN|SOT", re.I)


def label(words, g):
    """The package words of the nearest caption line above the figure (within 120 pt)."""
    top = min(w[1] for w in g)
    x0, x1 = min(w[0] for w in g) - 60, max(w[2] for w in g) + 60
    cands = [w for w in words if w[3] <= top and top - w[3] < 120 and x0 <= cx(w) <= x1 and PKG_WORD.search(w[4])]
    if not cands:
        return ""
    y = max(w[3] for w in cands)
    # the caption line (`32-Pin VQFN and TQFP` may start left of the figure),
    # but not the caption of a figure beside this one (`TQFP/MLF`  `PLCC`)
    fx = sum(cx(w) for w in g) / len(g)
    line = [w for w in words if abs(w[3] - y) < 4 and PKG_WORD.search(w[4]) and abs(cx(w) - fx) < 260]
    return " ".join(w[4] for w in sorted(line, key=lambda w: cx(w)))


def fits(lab, want):
    """Does a caption (`TQFP/MLF`, `PDIP`, `20-WQFN`) name the unit's package type?"""
    if not lab:
        return None
    L = lab.upper()
    words = {"QFP": r"QFP", "QFN": r"QFN|MLF|DFN", "DIP": r"DIP|SOIC|SOIJ|SO\d|SSOP|TSSOP",
             "SO": r"SOIC|SOIJ|SO\d|DIP|SSOP|TSSOP", "SSOP": r"SSOP|SOIC|DIP|TSSOP", "TSSOP": r"TSSOP|SSOP|SOIC|DIP",
             "LCC": r"PLCC|LCC"}.get(want)
    return bool(words and re.search(words, L))


def main(argv):
    units_path, out_path = argv[0], argv[1]
    only = argv[argv.index("--only") + 1] if "--only" in argv else None
    units = [u for u in csv.DictReader(open(units_path), delimiter="\t")
             if u["geometry"] in ("dual", "quad", "plcc") and not u["pdf"].startswith("datasheets/mcu/raspberry")
             and (only is None or only in u["pdf"])]
    out = open(out_path, "w")
    out.write("pdf\tpage\tfigure\tdevices\tpinout\tdesignator\tname\trole\n")
    cache = {}
    done = fail = 0
    written = {}
    for u in units:
        pdf = os.path.join(ROOT, u["pdf"])
        n = int(u["pins"])
        if pdf not in cache:
            cache[pdf] = pin_pages(pdf)
        pages, texts = cache[pdf]
        chosen, others = None, []
        want = u["type"] or None
        for p in pages:
            ws = page_words(pdf, p)
            for g, pins, centre in figure_pins(ws, n):
                if not layout_ok(g, u["geometry"], n):
                    continue
                if fits(label(ws, g), want) is False:
                    continue
                seq = [pad_name(pins[k][4]) for k in range(1, n + 1)]
                if chosen is None:
                    chosen = (p, seq)
                else:
                    others.append((p, seq))
        if chosen is None:
            print(f"FAIL {u['pdf']} {u['geometry']}{n} {u['type']} {u['devices']}")
            fail += 1
            continue
        p, seq = chosen
        agree = [("agree" if s == seq else "DIFFER") + f":p{q}" for q, s in others]
        pre = {"dual": "p", "quad": "quad", "plcc": "plcc"}[u["geometry"]]
        print(f"ok   {u['pdf']} {pre}{n} {u['type']} p{p} {u['devices']} {' '.join(agree)}")
        done += 1
        # two package types of one PDF (SOIC and TSSOP) usually share a figure
        # and a pinout name: write it once, and stop on a different reading
        key = (u["pdf"], u["devices"], f"{pre}{n}")
        if key in written:
            if written[key] != seq:
                print(f"DIFFER {u['pdf']} {pre}{n}: {u['type']} reads differently from an earlier unit")
            continue
        written[key] = seq
        for i, nm in enumerate(seq, 1):
            role = "nc" if nm in ("DNC", "NC") else R.role(nm, "avr")
            out.write(f"{u['pdf']}\t{p}\tauto:p{p}\t{u['devices']}\t{pre}{n}\t{i}\t{nm}\t{role}\n")
    print(f"{done} units transcribed, {fail} not found")


if __name__ == "__main__":
    main(sys.argv[1:])
