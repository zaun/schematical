#!/usr/bin/env python3
"""Checker for the spike datasheet format (format.md, same folder).

Usage: python3 -I check.py [--quiet] FILE.md ...

Checks each file against format.md and the part catalog in
schematical/schematical/Parts/, then prints errors, warnings and a size
report. Exit status 1 if any file has an error.
"""

import fnmatch
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
PARTS_DIR = os.path.join(REPO, "schematical", "schematical", "Parts")
DATASHEETS_DIR = os.path.join(REPO, "datasheets")

CLASSES = ["logic", "memory", "controller", "oscillator", "display", "module", "connector", "switch",
           "microprocessor", "microcontroller"]

# (title, anchor, requirement per class in CLASSES order: R required, O optional, - not allowed)
SECTIONS = [
    ("Description", "description", "RRRRRRRRRR"),
    ("Pinout", "pinout", "RRRRRRRRRR"),
    ("Pin functions", "pin-functions", "RRRRRROORR"),
    ("Functional description", "functional-description", "ORROOOOORR"),
    ("Programming model", "programming-model", "--------RR"),
    ("Instruction set", "instruction-set", "--------RR"),
    ("Memory map", "memory-map", "-OO--O--OR"),
    ("Peripherals", "peripherals", "--------OR"),
    ("Function tables", "function-tables", "RROOOO-OOO"),
    ("Registers and commands", "registers", "-OR--O--OR"),
    ("Absolute maximum ratings", "absolute-maximum-ratings", "RRRRRROORR"),
    ("Recommended operating conditions", "recommended-operating-conditions", "RRROOO--RR"),
    ("Thermal information", "thermal-information", "OOOO----OO"),
    ("Electrical characteristics", "electrical-characteristics", "RRRRRRRRRR"),
    ("Switching characteristics", "switching-characteristics", "RRRO-O-ORR"),
    ("Timing diagrams", "timing-diagrams", "ORRO-O--RO"),
    ("Typical characteristics", "typical-characteristics", "OOOOOO--OO"),
    ("Parameter measurement information", "parameter-measurement", "OOOO----OO"),
    ("Application information", "application-information", "OOOOOOOOOO"),
    ("Package", "package", "RRRRRRRRRR"),
    ("Ordering information", "ordering", "RRRRRRRRRR"),
    ("Conversion notes", "conversion-notes", "RRRRRRRRRR"),
]
SECTION_INDEX = {t: i for i, (t, _, _) in enumerate(SECTIONS)}
SPEC_SECTIONS = {"Absolute maximum ratings", "Recommended operating conditions", "Thermal information",
                 "Electrical characteristics", "Switching characteristics"}
BLOCK_HOME = {"pins": "Pin functions", "pinout": "Pinout", "package": "Package", "ordering": "Ordering information"}
BLOCK_TYPES = {"table", "pins", "pinout", "wavedrom", "curve", "figure", "package", "ordering", "equation"}
FIGURE_LIBRARY = {"load-cmos-50pf", "waveforms-cmos-tpd-tt", "waveforms-cmos-tsu-th-tw",
                  "load-eeprom-ttl", "clamp-diodes"}
FRONT_KEYS = {"format", "version", "id", "title", "class", "source", "devices", "parts", "packages", "converted",
              "pending"}
SOURCE_KEYS = {"pdf", "vendor", "document", "date", "pages"}
VALUE_COLS = {"min", "typ", "nom", "max", "value"}
NAMED_COLS = {"symbol", "parameter", "conditions", "test conditions", "from", "to", "unit"} | VALUE_COLS
SIZE_WARN = 30 * 1024
SIZE_LIMIT = {"controller": 60 * 1024, "microprocessor": 140 * 1024, "microcontroller": 140 * 1024}
MAX_CURVES = {"microprocessor": 12, "microcontroller": 12}


class DupKey(ValueError):
    pass


def no_dups(pairs):
    seen = {}
    for k, v in pairs:
        if k in seen:
            raise DupKey('duplicate key "%s"' % k)
        seen[k] = v
    return seen


def load_json(text):
    return json.loads(text, object_pairs_hook=no_dups)


# ---------------------------------------------------------------- catalog

class Catalog:
    def __init__(self):
        self.devices = {}      # "kind:id" -> device dict
        self.part_device = {}  # part name -> "kind:id"
        self.parts = {}        # part name -> part dict
        self.footprints = set()
        for name in sorted(os.listdir(PARTS_DIR)):
            if not name.endswith(".json"):
                continue
            with open(os.path.join(PARTS_DIR, name), encoding="utf-8") as f:
                data = json.load(f)
            if name == "footprints.json":
                self.footprints = {fp["id"] for fp in data["footprints"]}
                continue
            kind = data["kind"]
            for dev in data["devices"]:
                key = "%s:%s" % (kind, dev["id"])
                self.devices[key] = dev
                for p in dev["parts"]:
                    self.part_device[p["name"]] = key
                    self.parts[p["name"]] = p


# ---------------------------------------------------------------- helpers

ATTR_RE = re.compile(r'(\w[\w-]*)=("([^"]*)"|\S+)')


def parse_info(info):
    info = info.strip()
    kind, _, rest = info.partition(" ")
    attrs = {}
    for m in ATTR_RE.finditer(rest):
        attrs[m.group(1)] = m.group(3) if m.group(3) is not None else m.group(2)
    return kind, attrs


def split_row(line):
    s = line.strip()
    if not (s.startswith("|") and s.endswith("|")) or len(s) < 2:
        return None
    return [c.strip() for c in s[1:-1].split("|")]


DELIM_RE = re.compile(r"^:?-+:?$")
NOTE_RE = re.compile(r"^\[(\d+)\]\s+\S")
MARK_RE = re.compile(r"(?<![\w\\])\[(\d+)\]")
NUM_RE = re.compile(r"^[<>≤≥]?\s?[±+-]?\d+(\.\d+)?$")
RATIO_RE = re.compile(r"^\d+(\.\d+)?[/:]\d+(\.\d+)?$")
EXPR_RE = re.compile(r"^[A-Za-z0-9~ .+\-×()/]+$")
RAILS = {"GND", "VCC", "VDD", "VSS", "AVCC", "AREF"}


class Report:
    def __init__(self, path):
        self.path = path
        self.errors = []
        self.warnings = []

    def err(self, line, msg):
        self.errors.append((line, msg))

    def warn(self, line, msg):
        self.warnings.append((line, msg))


# ---------------------------------------------------------------- blocks

def check_table(rep, start, body, attrs, section):
    lines = body
    rows, notes, i = [], [], 0
    while i < len(lines) and lines[i].strip():
        cells = split_row(lines[i])
        if cells is None:
            rep.err(start + i, "table row must start and end with |")
            return
        rows.append((start + i, cells))
        i += 1
    while i < len(lines):
        s = lines[i].strip()
        if s:
            if NOTE_RE.match(s):
                notes.append((start + i, s))
            else:
                rep.err(start + i, "after a table only notes '[n] text' may follow")
        i += 1
    delim = [k for k, (_, c) in enumerate(rows) if all(DELIM_RE.match(x) for x in c)]
    if len(delim) != 1:
        rep.err(start, "table needs exactly one |---| delimiter row")
        return
    d = delim[0]
    if d == 0:
        rep.err(start, "table needs at least one header row")
        return
    width = len(rows[d][1])
    for ln, cells in rows:
        if len(cells) != width:
            rep.err(ln, "row has %d cells, delimiter has %d" % (len(cells), width))
            return
    header, body_rows = rows[:d], rows[d + 1:]
    if not body_rows:
        rep.err(start, "table has no body rows")
    for group in (header, body_rows):
        for k, (ln, cells) in enumerate(group):
            if cells[0] == "<":
                rep.err(ln, "a row's first cell can't be '<'")
            if k == 0 and "^" in cells:
                rep.err(ln, "'^' in the first %s row" % ("header" if group is header else "body"))
    # resolve column names from the header rows
    names = []
    for c in range(width):
        name = ""
        for _, cells in reversed(header):
            v = cells[c]
            if v == "^":
                continue
            if v == "<":
                cc = c
                while cc > 0 and cells[cc] == "<":
                    cc -= 1
                v = cells[cc]
            name = v
            break
        names.append(MARK_RE.sub("", name).strip().lower())
    # notes and markers
    used = set()
    for ln, cells in rows:
        for c in cells:
            used.update(MARK_RE.findall(c))
    used.update(MARK_RE.findall(attrs.get("caption", "")))
    used.update(MARK_RE.findall(attrs.get("conditions", "")))
    defined = {}
    for ln, s in notes:
        n = NOTE_RE.match(s).group(1)
        if n in defined:
            rep.err(ln, "note [%s] defined twice" % n)
        defined[n] = ln
    for n in sorted(used - set(defined)):
        rep.err(start, "marker [%s] has no note" % n)
    for n in sorted(set(defined) - used):
        rep.err(defined[n], "note [%s] has no marker" % n)
    # spec tables: every table in a spec section, and any other table with
    # a Unit column and a Min/Typ/Nom/Max/Value column (peripheral tables)
    if section in ("Peripherals", "Functional description") and "unit" in names and any(n in VALUE_COLS for n in names):
        section = "Electrical characteristics"
    if section in SPEC_SECTIONS:
        if "unit" not in names:
            rep.err(start, "spec table (%s) needs a Unit column" % section)
        value_cols = [c for c, n in enumerate(names) if n in VALUE_COLS]
        if "values" in attrs:
            if attrs["values"].lower() not in VALUE_COLS:
                rep.err(start, 'values= must be one of Min, Typ, Nom, Max, Value')
            value_cols = [c for c, n in enumerate(names) if c > 0 and n not in NAMED_COLS]
        elif not value_cols and section != "Thermal information":
            rep.err(start, "spec table needs a Min, Typ, Nom, Max or Value column (or values=)")
        if section == "Thermal information" and not value_cols:
            value_cols = [c for c, n in enumerate(names) if c > 0 and n not in NAMED_COLS]
        for ln, cells in body_rows:
            if all(x == "<" for x in cells[1:]):
                continue  # banner row
            for c in value_cols:
                v = re.sub(r"\s*\[\d+\]", "", cells[c]).strip()
                if v in ("", "^", "<"):
                    continue
                if NUM_RE.match(v) or RATIO_RE.match(v):
                    continue
                if EXPR_RE.match(v) and ("~" in v or "×" in v):
                    continue
                if v in RAILS:
                    continue
                rep.err(ln, 'value cell "%s" (column %s) is not a number, ratio or symbol expression'
                        % (cells[c], names[c] or c + 1))


def check_wavedrom(rep, start, text):
    try:
        w = load_json(text)
    except (ValueError, DupKey) as e:
        rep.err(start, "wavedrom: %s" % e)
        return
    if not isinstance(w, dict) or not ("signal" in w or "reg" in w):
        rep.err(start, "wavedrom: needs an object with 'signal' or 'reg'")
        return
    nodes = set()

    def walk(items):
        for s in items:
            if isinstance(s, list):
                walk([x for x in s if not isinstance(x, str)])
            elif isinstance(s, dict) and s:
                wave = s.get("wave", "")
                data = s.get("data")
                if isinstance(data, str):
                    data = data.split()
                slots = sum(1 for ch in wave if ch == "=" or ch in "23456789")
                if data is not None and len(data) != slots:
                    rep.err(start, 'wavedrom: "%s" has %d data cells but %d data labels'
                            % (s.get("name", "?"), slots, len(data)))
                if data is None and slots:
                    rep.warn(start, 'wavedrom: "%s" has data cells and no data labels' % s.get("name", "?"))
                for ch in s.get("node", ""):
                    if ch != "." and not ch.isupper():
                        rep.err(start, 'wavedrom: node "%s" must be an uppercase letter (lowercase ones are printed)' % ch)
                    if ch != ".":
                        if ch in nodes:
                            rep.err(start, 'wavedrom: node "%s" used twice' % ch)
                        nodes.add(ch)

    for k in sorted(set(w) - {"signal", "edge", "config", "reg"}):
        rep.err(start, 'wavedrom: key "%s" is not used by this format' % k)
    if set(w.get("config", {})) - {"hscale"}:
        rep.err(start, "wavedrom: config may only set hscale")
    if "signal" in w:
        walk(w["signal"])
        for e in w.get("edge", []):
            tok = e.split(" ", 1)[0]
            m = re.match(r"^([A-Z])([-~<>|]+)([A-Z])$", tok)
            if not m:
                rep.err(start, 'wavedrom: edge "%s" must be <node><arrow><node> label' % e)
                continue
            for n in (m.group(1), m.group(3)):
                if n not in nodes:
                    rep.err(start, 'wavedrom: edge "%s" names unknown node "%s"' % (e, n))


def check_curve(rep, start, text):
    try:
        c = load_json(text)
    except (ValueError, DupKey) as e:
        rep.err(start, "curve: %s" % e)
        return
    if set(c) != {"x", "y", "series"}:
        rep.err(start, "curve: keys must be x, y, series")
        return
    for ax in ("x", "y"):
        a = c[ax]
        if not {"label", "unit", "range"} <= set(a) or set(a) - {"label", "unit", "range", "log"}:
            rep.err(start, "curve: %s needs label, unit, range (and optional log)" % ax)
            return
    (x0, x1), (y0, y1) = c["x"]["range"], c["y"]["range"]
    for s in c["series"]:
        pts = s.get("points", [])
        if not pts or len(pts) > 24:
            rep.err(start, 'curve: series "%s" needs 1-24 points' % s.get("label"))
        xs = [p[0] for p in pts]
        if xs != sorted(xs):
            rep.err(start, 'curve: series "%s" x values must ascend' % s.get("label"))
        for x, y in pts:
            if not (x0 <= x <= x1 and y0 <= y <= y1):
                rep.warn(start, 'curve: point (%s, %s) of "%s" is outside the axis ranges' % (x, y, s.get("label")))


# ---------------------------------------------------------------- file

def check_file(path, cat, claims):
    rep = Report(path)
    with open(path, encoding="utf-8") as f:
        text = f.read()
    size = len(text.encode("utf-8"))
    if "\t" in text:
        rep.err(0, "tabs are not allowed")
    lines = text.split("\n")
    if not lines or lines[0] != "---":
        rep.err(1, "file must start with a '---' line and JSON front matter")
        return rep, size, {}
    try:
        end = lines.index("---", 1)
    except ValueError:
        rep.err(1, "front matter has no closing '---'")
        return rep, size, {}
    try:
        fm = load_json("\n".join(lines[1:end]))
    except (ValueError, DupKey) as e:
        rep.err(2, "front matter: %s" % e)
        return rep, size, {}

    # ---- front matter
    for k in sorted(set(fm) - FRONT_KEYS):
        rep.err(2, 'front matter: unknown key "%s"' % k)
    for k in sorted(FRONT_KEYS - {"packages", "pending"} - set(fm)):
        rep.err(2, 'front matter: missing "%s"' % k)
    if fm.get("format") != "schematical-datasheet":
        rep.err(2, 'front matter: format must be "schematical-datasheet"')
    if fm.get("version") != 1:
        rep.err(2, "front matter: version must be 1")
    base = os.path.splitext(os.path.basename(path))[0]
    if fm.get("id") != base:
        rep.err(2, 'front matter: id "%s" must be the file name "%s"' % (fm.get("id"), base))
    cls = fm.get("class")
    if cls not in CLASSES:
        rep.err(2, 'front matter: class "%s" is not one of %s' % (cls, ", ".join(CLASSES)))
        cls = None
    src = fm.get("source", {})
    if not isinstance(src, dict) or set(src) != SOURCE_KEYS:
        rep.err(2, "front matter: source needs exactly %s" % ", ".join(sorted(SOURCE_KEYS)))
    else:
        pdf = os.path.join(DATASHEETS_DIR, src["pdf"])
        if not os.path.exists(pdf):
            rep.warn(2, "source pdf %s not found under datasheets/" % src["pdf"])
        elif os.path.splitext(os.path.basename(pdf))[0] != base:
            rep.err(2, "source pdf base name must equal the id")
        if not isinstance(src["pages"], int) or src["pages"] < 1:
            rep.err(2, "source.pages must be a positive integer")
        if src["date"] and not re.match(r"^\d{4}(-\d{2})?$", src["date"]):
            rep.err(2, "source.date must be YYYY or YYYY-MM")
    if not re.match(r"^\d{4}-\d{2}-\d{2}$", str(fm.get("converted", ""))):
        rep.err(2, "converted must be YYYY-MM-DD")
    devices = fm.get("devices", [])
    pending = "pending" in fm
    if pending:
        if not isinstance(fm["pending"], str) or not fm["pending"].strip():
            rep.err(2, "pending must be a non-empty reason")
        if devices or fm.get("parts"):
            rep.err(2, "a pending datasheet has empty devices and parts")
    elif not devices:
        rep.err(2, "devices must list at least one device")
    for d in devices:
        if d not in cat.devices:
            rep.err(2, 'unknown device "%s" (want "<kind>:<id>")' % d)
    devs = [cat.devices[d] for d in devices if d in cat.devices]
    dev_parts = {p["name"]: d for d in devices if d in cat.devices for p in cat.devices[d]["parts"]}
    claimed = []
    for p in fm.get("parts", []):
        if "*" in p or "?" in p:
            hits = sorted(n for n in dev_parts if fnmatch.fnmatchcase(n, p))
            if not hits:
                rep.err(2, 'parts pattern "%s" matches no part of the listed devices' % p)
            claimed += hits
        elif p not in cat.parts:
            rep.err(2, 'unknown part "%s"' % p)
        elif p not in dev_parts:
            rep.err(2, 'part "%s" belongs to %s, not to the listed devices' % (p, cat.part_device[p]))
        else:
            claimed.append(p)
    if len(set(claimed)) != len(claimed):
        rep.err(2, "a part is listed twice in parts")
    for p in claimed:
        claims.setdefault(p, []).append(path)
    pinout_names = {pn for d in devs for pn in d["pinouts"]}
    for pn in fm.get("packages", {}):
        if pn not in pinout_names:
            rep.err(2, 'packages: unknown pinout "%s"' % pn)
    used_pinouts = {cat.parts[p]["pinout"] for p in claimed}
    used_footprints = {cat.parts[p].get("footprint") for p in claimed} - {None}

    # ---- body
    body_start = end + 1
    h1 = 0
    sections = []          # (title, line)
    sec_bytes = {}
    current = None
    blocks = []            # (type, attrs, start line, body lines, section)
    i = body_start
    in_fence = None
    fence_lines = []
    while i < len(lines):
        line = lines[i]
        ln = i + 1
        if in_fence is not None:
            if line.strip() == "```":
                blocks.append(in_fence + (fence_lines, current))
                in_fence = None
            else:
                fence_lines.append(line)
        elif line.startswith("```"):
            kind, attrs = parse_info(line[3:])
            if kind not in BLOCK_TYPES:
                rep.err(ln, 'unknown block type "%s"' % kind)
            in_fence = (kind, attrs, ln + 1)
            fence_lines = []
        elif line.startswith("# "):
            h1 += 1
            if sections:
                rep.err(ln, "the # title must come before the sections")
        elif line.startswith("## "):
            title = line[3:].strip()
            if title not in SECTION_INDEX:
                rep.err(ln, 'unknown section "%s"' % title)
            else:
                if sections and SECTION_INDEX.get(sections[-1][0], -1) >= SECTION_INDEX[title]:
                    rep.err(ln, 'section "%s" is out of order or repeated' % title)
                sections.append((title, ln))
            current = title
        if current is not None:
            sec_bytes[current] = sec_bytes.get(current, 0) + len(line.encode("utf-8")) + 1
        i += 1
    if in_fence is not None:
        rep.err(in_fence[2], "unclosed ``` block")
    if h1 != 1:
        rep.err(body_start, "need exactly one '# ' title heading (found %d)" % h1)

    present = {t for t, _ in sections}
    if cls:
        ci = CLASSES.index(cls)
        for title, _, req in SECTIONS:
            r = req[ci]
            if r == "R" and title not in present:
                rep.err(0, 'class %s requires section "%s"' % (cls, title))
            if r == "-" and title in present:
                rep.err(0, 'class %s does not allow section "%s"' % (cls, title))

    # ---- anchors and links
    anchors = {}
    in_f = False
    for k, line in enumerate(lines[body_start:], body_start + 1):
        if line.startswith("```"):
            in_f = not in_f if line.strip() == "```" or not in_f else in_f
            if not in_f and line.strip() != "```":
                in_f = True
            continue
        if in_f:
            continue
        m = re.match(r"^(#{1,6}) (.+)$", line)
        if m:
            title = m.group(2).strip()
            if m.group(1) == "##" and title in SECTION_INDEX:
                slug = SECTIONS[SECTION_INDEX[title]][1]
            else:
                slug = re.sub(r"[^\w\- ]", "", title.lower()).replace(" ", "-")
            if slug in anchors:
                rep.warn(k, 'heading anchor "#%s" repeats line %d' % (slug, anchors[slug]))
            anchors.setdefault(slug, k)
    for kind, attrs, start, body, section in blocks:
        if "id" in attrs:
            if attrs["id"] in anchors:
                rep.err(start - 1, 'id "%s" repeats an anchor on line %d' % (attrs["id"], anchors[attrs["id"]]))
            anchors.setdefault(attrs["id"], start - 1)
    for k, line in enumerate(lines[body_start:], body_start + 1):
        for m in re.finditer(r"\]\(#([^)\s]+)\)", line):
            if m.group(1) not in anchors:
                rep.err(k, 'link to unknown anchor "#%s"' % m.group(1))

    # ---- inline markup (outside wavedrom and curve blocks)
    skip = set()
    for kind, attrs, start, body, section in blocks:
        if kind in ("wavedrom", "curve"):
            skip.update(range(start, start + len(body)))
    for k, line in enumerate(lines[body_start:], body_start + 1):
        if k in skip:
            continue
        for m in re.finditer(r"!\{", line):
            if "}" not in line[m.end():]:
                rep.err(k, "unclosed !{")
        if line.count("~") % 2:
            rep.warn(k, "odd number of ~ (unbalanced subscript?)")

    # ---- blocks
    pins_seen = set()
    pinout_blocks = 0
    package_fps = set()
    ordering_blocks = 0
    wavedrom_blocks = 0
    for kind, attrs, start, body, section in blocks:
        text_ = "\n".join(body)
        home = BLOCK_HOME.get(kind)
        if pending and kind in ("pinout", "package", "ordering"):
            rep.err(start - 1, "a pending datasheet uses tables, not %s blocks" % kind)
        if home and section != home:
            rep.err(start - 1, '%s block belongs in "%s", not "%s"' % (kind, home, section))
        if kind == "table":
            check_table(rep, start, body, attrs, section)
            for g in attrs.get("parts", "").split(","):
                g = g.strip()
                if g and not any(fnmatch.fnmatchcase(p, g) for p in claimed):
                    rep.err(start - 1, 'table parts= "%s" matches no claimed part' % g)
        elif kind == "pins":
            names_all = {}
            for pn in pinout_names:
                for d in devs:
                    for pin in d["pinouts"].get(pn, []):
                        names_all.setdefault(pin["name"], set()).add(pin["role"])
            for k, line in enumerate(body, start):
                if not line.strip():
                    continue
                left, sep, desc = line.partition(":")
                if not sep or not desc.strip():
                    rep.err(k, "pins line must be 'NAME[, NAME]: description'")
                    continue
                for n in [x.strip() for x in left.split(",")]:
                    if not pending and n not in names_all:
                        rep.err(k, 'pin "%s" is in no pinout of the listed devices' % n)
                    if n in pins_seen:
                        rep.err(k, 'pin "%s" described twice' % n)
                    pins_seen.add(n)
        elif kind == "pinout":
            pinout_blocks += 1
            pn = attrs.get("pinout")
            if pn is not None and pn not in pinout_names:
                rep.err(start - 1, 'pinout "%s" is not a pinout of the listed devices' % pn)
            if text_.strip():
                rep.err(start, "pinout block body must be empty")
        elif kind == "package":
            fp = attrs.get("footprint")
            if fp not in cat.footprints:
                rep.err(start - 1, 'unknown footprint "%s"' % fp)
            elif fp not in used_footprints:
                rep.err(start - 1, 'footprint "%s" is used by no claimed part' % fp)
            package_fps.add(fp)
            if text_.strip():
                rep.err(start, "package block body must be empty")
        elif kind == "ordering":
            ordering_blocks += 1
            for k, line in enumerate(body, start):
                if not line.strip():
                    continue
                cells = [c.strip() for c in line.split("|")]
                if len(cells) != 4:
                    rep.err(k, "ordering line must be 'part | packing | temp | marking'")
                elif cells[0] not in claimed:
                    rep.err(k, 'ordering: "%s" is not a claimed part' % cells[0])
        elif kind == "wavedrom":
            wavedrom_blocks += 1
            check_wavedrom(rep, start, text_)
        elif kind == "curve":
            check_curve(rep, start, text_)
        elif kind == "figure":
            if "ref" in attrs:
                if attrs["ref"] not in FIGURE_LIBRARY:
                    rep.err(start - 1, 'figure ref "%s" is not in the shared library' % attrs["ref"])
            elif "src" in attrs:
                sp = os.path.join(os.path.dirname(path), attrs["src"])
                if not attrs["src"].startswith(base + ".figures/"):
                    rep.err(start - 1, "figure src must be in %s.figures/" % base)
                elif not os.path.exists(sp):
                    rep.err(start - 1, "figure src %s not found" % attrs["src"])
                else:
                    size += os.path.getsize(sp)
            else:
                rep.err(start - 1, "figure needs ref= or src=")
        elif kind == "equation":
            if not text_.strip():
                rep.err(start, "empty equation")

    # ---- coverage
    if "Pinout" in present and not pinout_blocks and not pending:
        rep.err(0, "Pinout section has no pinout block")
    required_pins = set()
    for p in claimed:
        d = cat.devices[cat.part_device[p]]
        for pin in d["pinouts"][cat.parts[p]["pinout"]]:
            if pin["role"] not in ("nc", "absent"):
                required_pins.add(pin["name"])
    if "Pin functions" in present:
        for n in sorted(required_pins - pins_seen):
            rep.err(0, 'pin "%s" has no description' % n)
    for fp in sorted(used_footprints - package_fps):
        rep.err(0, 'footprint "%s" of a claimed part has no package block' % fp)
    if "Ordering information" in present and not ordering_blocks and not pending:
        rep.err(0, "Ordering information has no ordering block")
    if cls in ("memory", "controller", "microprocessor") and not wavedrom_blocks:
        rep.err(0, "class %s needs at least one wavedrom block" % cls)
    curves = sum(1 for b in blocks if b[0] == "curve")
    if cls in MAX_CURVES and curves > MAX_CURVES[cls]:
        rep.err(0, "%d curve blocks; class %s keeps at most %d" % (curves, cls, MAX_CURVES[cls]))
    if cls in SIZE_LIMIT:
        if size > SIZE_LIMIT[cls]:
            rep.err(0, "size %.1f KB is over the %d KB limit for class %s"
                    % (size / 1024, SIZE_LIMIT[cls] // 1024, cls))
    elif size > SIZE_WARN:
        rep.warn(0, "size %.1f KB is over the 30 KB target" % (size / 1024))
    info = {"size": size, "sections": sec_bytes, "claimed": len(claimed),
            "pinouts": sorted(used_pinouts)}
    return rep, size, info


def main(argv):
    quiet = "--quiet" in argv
    files = [a for a in argv if a != "--quiet"]
    if not files:
        print(__doc__.strip())
        return 2
    cat = Catalog()
    claims = {}
    failed = False
    results = []
    for f in files:
        rep, size, info = check_file(f, cat, claims)
        results.append((f, rep, size, info))
    for p, fs in sorted(claims.items()):
        if len(fs) > 1:
            for f, rep, _, _ in results:
                if f in fs:
                    rep.err(0, 'part "%s" is also claimed by %s' % (p, ", ".join(x for x in fs if x != f)))
    for f, rep, size, info in results:
        status = "FAIL" if rep.errors else "ok"
        failed |= bool(rep.errors)
        print("%s: %s, %.1f KB, %d parts" % (f, status, size / 1024, info.get("claimed", 0)))
        for ln, msg in rep.errors:
            print("  error   line %d: %s" % (ln, msg) if ln else "  error   %s" % msg)
        for ln, msg in rep.warnings:
            print("  warning line %d: %s" % (ln, msg) if ln else "  warning %s" % msg)
        if not quiet and info.get("sections"):
            for t, b in sorted(info["sections"].items(), key=lambda kv: -kv[1]):
                print("    %6.1f KB  %s" % (b / 1024, t))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
