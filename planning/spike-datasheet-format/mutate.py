#!/usr/bin/env python3
"""Test the checkers: plant errors in converted datasheets and count how many
check.py and validate.py catch.

For every file given, and every kind of mistake below, the script makes
copies of the file with one planted mistake each, runs both checkers on the
copy, and compares the result with the unchanged file's. A mistake is

  caught   the copy has an error the original does not have
  missed   the checkers say the same as before

Nothing in the repository is changed; the copies live in a temporary folder.

Kinds of planted mistake:

  value        one digit of a spec-table value changed (3.4 -> 3.5)
  value-swap   two rows of a spec table exchange a value in one column
  sign         a spec-table value's sign flipped (-18 -> 18)
  column       a value moved to another Min/Typ/Max column of its row
  unit         a row's unit changed (mA -> µA)
  row-drop     a spec-table row deleted
  cond         a test condition's number changed to another row's
  symbol       two rows of a spec table exchange their symbols
  ftable       an H/L flipped in a function table
  note-drop    a table note deleted with its markers
  negation     "not" removed from a sentence or note
  prose-num    a number in prose changed
  fig          a figure parameter changed (CL: 50 pF -> 15 pF)
  ord-qty      an ordering line's packing quantity changed
  ord-temp     an ordering line's temperature range changed
  ord-mark     an ordering line's marking changed
  ord-part     an ordering line moved to another claimed part
  fm-pages     source.pages changed
  fm-doc       source.document's revision letter changed
  fm-date      source.date changed
  claim-drop   a claimed part removed from "parts"

Usage: python3 -I mutate.py [--per N] [--seed N] [--kinds a,b,...]
                            [--missed] [--min PCT] files...

--per N     mutants per kind and file (default 2)
--missed    list every missed mutant
--min PCT   exit status 1 when any kind's catch rate is below PCT
"""

import argparse
import collections
import importlib.util
import json
import os
import random
import re
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))


def load(name):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HERE, name + ".py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


check = load("check")
check.HOLD_TO_CATALOG = True
validate = load("validate")

VALUE_COLS = {"min", "typ", "nom", "max", "value"}
SPEC_SECTIONS = {"Absolute maximum ratings", "Recommended operating conditions", "Thermal information",
                 "Electrical characteristics", "Switching characteristics"}
UNIT_SWAP = {"mA": "µA", "µA": "mA", "nA": "µA", "A": "mA", "ns": "µs", "ps": "ns", "µs": "ms", "ms": "µs",
             "V": "mV", "mV": "V", "pF": "nF", "nF": "pF", "µF": "nF", "°C": "K", "°C/W": "°C/mW",
             "MHz": "kHz", "kHz": "MHz", "Hz": "kHz", "Ω": "kΩ", "kΩ": "Ω", "mW": "W", "W": "mW", "V/ns": "V/µs",
             "ns/V": "µs/V", "µW": "mW", "pC": "nC", "%": "‰"}
NUM_CELL = re.compile(r"^([<>≤≥]?\s?)([±+-]?)(\d+(?:\.\d+)?)((?:\s*\[\d+\])*)$")


# ---------------------------------------------------------------- parsing

class Table:
    def __init__(self, start, lines, attrs, section):
        self.attrs, self.section = attrs, section
        self.header, self.rows, self.notes = [], [], []     # (line index, cells)
        delim = False
        k = start
        while k < len(lines) and lines[k].strip() and lines[k].strip() != "```":
            cells = validate.split_row(lines[k])
            if cells is None:
                break
            if all(re.fullmatch(r":?-+:?", c) for c in cells):
                delim = True
            elif delim:
                self.rows.append((k, cells))
            else:
                self.header.append((k, cells))
            k += 1
        while k < len(lines) and lines[k].strip() != "```":
            if re.match(r"^\[\d+\]\s", lines[k].strip()):
                self.notes.append(k)
            k += 1
        self.names = []
        width = len(self.header[-1][1]) if self.header else 0
        for c in range(width):
            name = ""
            for _, cells in reversed(self.header):
                v = cells[c] if c < len(cells) else ""
                if v == "^":
                    continue
                cc = c
                while v == "<" and cc > 0:
                    cc -= 1
                    v = cells[cc]
                name = v
                break
            self.names.append(re.sub(r"\[\d+\]|[~*]", "", name).strip().lower())
        self.spec = section in SPEC_SECTIONS and "unit" in self.names
        if "values" in attrs:
            named = VALUE_COLS | {"symbol", "parameter", "conditions", "test conditions", "from", "to", "unit"}
            self.value_cols = [c for c, n in enumerate(self.names) if c > 0 and n not in named]
        else:
            self.value_cols = [c for c, n in enumerate(self.names) if n in VALUE_COLS]
        if self.spec and not self.value_cols and section == "Thermal information":
            self.value_cols = [c for c, n in enumerate(self.names)
                               if c > 0 and n not in ("symbol", "parameter", "unit")]


class File:
    def __init__(self, path):
        with open(path, encoding="utf-8") as f:
            self.lines = f.read().split("\n")
        self.fm_end = self.lines.index("---", 1)
        self.tables, self.blocks, self.prose = [], [], []      # blocks: (kind, attrs, first body line, end line, section)
        section, i = "", self.fm_end + 1
        while i < len(self.lines):
            line = self.lines[i]
            if line.startswith("```") and len(line.strip()) > 3:
                kind, attrs = validate.parse_info(line[3:])
                j = i + 1
                while j < len(self.lines) and self.lines[j].strip() != "```":
                    j += 1
                self.blocks.append((kind, attrs, i + 1, j, section))
                if kind == "table":
                    self.tables.append(Table(i + 1, self.lines, attrs, section))
                i = j + 1
                continue
            if line.startswith("## "):
                section = line[3:].strip()
            elif line.strip() and not line.startswith("#") and section != "Conversion notes":
                self.prose.append((i, section))
            i += 1


def set_cell(line, col, new):
    """The table line with cell `col` replaced."""
    parts = line.strip()[1:-1].split("|")
    parts[col] = " %s " % new
    return "|" + "|".join(parts) + "|"


def bump(num):
    """A different number of the same shape: one digit changed."""
    digits = [k for k, ch in enumerate(num) if ch.isdigit()]
    k = digits[-1] if len(digits) == 1 or num[digits[-1]] != "0" else digits[0]
    d = int(num[k])
    return num[:k] + str(d + 1 if d < 9 else d - 1) + num[k + 1:]


# ---------------------------------------------------------------- mutations
# Each returns a list of (description, {line index: new text or None to delete}).

def value_cells(f):
    for t in f.tables:
        if not t.spec:
            continue
        for ln, cells in t.rows:
            for c in t.value_cols:
                if c < len(cells) and NUM_CELL.match(cells[c]):
                    yield t, ln, cells, c


def m_value(f):
    out = []
    for t, ln, cells, c in value_cells(f):
        m = NUM_CELL.match(cells[c])
        new = m.group(1) + m.group(2) + bump(m.group(3)) + m.group(4)
        out.append(("%s -> %s (%s)" % (cells[c], new, t.names[c]), {ln: set_cell(f.lines[ln], c, new)}))
    return out


def m_value_swap(f):
    out = []
    for t in f.tables:
        if not t.spec:
            continue
        for c in t.value_cols:
            rows = [(ln, cells) for ln, cells in t.rows if c < len(cells) and NUM_CELL.match(cells[c])]
            for (l1, c1), (l2, c2) in zip(rows, rows[1:]):
                if c1[c] != c2[c]:
                    out.append(("rows exchange %s and %s (%s)" % (c1[c], c2[c], t.names[c]),
                                {l1: set_cell(f.lines[l1], c, c2[c]), l2: set_cell(f.lines[l2], c, c1[c])}))
    return out


def m_sign(f):
    out = []
    for t, ln, cells, c in value_cells(f):
        m = NUM_CELL.match(cells[c])
        if m.group(2) in ("±", "+") or float(m.group(3)) == 0:
            continue
        new = m.group(1) + ("" if m.group(2) == "-" else "-") + m.group(3) + m.group(4)
        out.append(("%s -> %s (%s)" % (cells[c], new, t.names[c]), {ln: set_cell(f.lines[ln], c, new)}))
    return out


def m_column(f):
    out = []
    for t in f.tables:
        if not t.spec or "values" in t.attrs or len(t.value_cols) < 2:
            continue
        for ln, cells in t.rows:
            for c in t.value_cols:
                if not NUM_CELL.match(cells[c]):
                    continue
                for d in t.value_cols:
                    if d != c and cells[d] == "" and abs(t.value_cols.index(d) - t.value_cols.index(c)) == 1:
                        line = set_cell(set_cell(f.lines[ln], c, ""), d, cells[c])
                        out.append(("%s moved from %s to %s" % (cells[c], t.names[c], t.names[d]), {ln: line}))
    return out


def m_unit(f):
    out = []
    for t in f.tables:
        if not t.spec or "unit" not in t.names:
            continue
        c = t.names.index("unit")
        for ln, cells in t.rows:
            if c < len(cells) and cells[c] in UNIT_SWAP:
                new = UNIT_SWAP[cells[c]]
                out.append(("unit %s -> %s" % (cells[c], new), {ln: set_cell(f.lines[ln], c, new)}))
    return out


def m_row_drop(f):
    out = []
    for t in f.tables:
        if not t.spec:
            continue
        for k, (ln, cells) in enumerate(t.rows):
            nxt = t.rows[k + 1][1] if k + 1 < len(t.rows) else []
            if "^" in cells or "^" in nxt or "<" in cells or not any(NUM_CELL.match(c) for c in cells):
                continue
            if any(re.search(r"\[\d+\]", c) for c in cells):
                continue          # would orphan a note: check.py's business, not the point here
            out.append(("row dropped: %s" % validate.clip(f.lines[ln], 70), {ln: None}))
    return out


def m_cond(f):
    out = []
    for t in f.tables:
        if not t.spec:
            continue
        for c, name in enumerate(t.names):
            if name in VALUE_COLS or name in ("symbol", "parameter", "unit") or c in t.value_cols:
                continue
            nums = collections.OrderedDict()
            for ln, cells in t.rows:
                if c < len(cells):
                    for m in re.finditer(r"(?<![\w.~])\d+(?:\.\d+)?(?![\w.])", cells[c]):
                        nums.setdefault(m.group(0), None)
            if len(nums) < 2:
                continue
            for ln, cells in t.rows:
                m = re.search(r"(?<![\w.~])\d+(?:\.\d+)?(?![\w.])", cells[c]) if c < len(cells) else None
                if not m:
                    continue
                other = next(n for n in nums if n != m.group(0))
                new = cells[c][:m.start()] + other + cells[c][m.end():]
                out.append(("condition %s -> %s" % (cells[c], new), {ln: set_cell(f.lines[ln], c, new)}))
    return out


def m_symbol(f):
    out = []
    for t in f.tables:
        if not t.spec or "symbol" not in t.names:
            continue
        c = t.names.index("symbol")
        rows = [(ln, cells) for ln, cells in t.rows if cells[c] not in ("", "^", "<")]
        for (l1, c1), (l2, c2) in zip(rows, rows[1:]):
            if c1[c] != c2[c]:
                out.append(("symbols %s and %s exchanged" % (c1[c], c2[c]),
                            {l1: set_cell(f.lines[l1], c, c2[c]), l2: set_cell(f.lines[l2], c, c1[c])}))
    return out


def m_ftable(f):
    out = []
    for t in f.tables:
        if t.section != "Function tables":
            continue
        for ln, cells in t.rows:
            c = len(cells) - 1
            if cells[c] in ("H", "L"):
                new = "L" if cells[c] == "H" else "H"
                out.append(("function table output %s -> %s" % (cells[c], new), {ln: set_cell(f.lines[ln], c, new)}))
    return out


def m_note_drop(f):
    out = []
    for t in f.tables:
        for ln in t.notes:
            n = re.match(r"^\[(\d+)\]", f.lines[ln].strip()).group(1)
            if validate.ISSUE_RE.search(f.lines[ln]):
                continue          # the converter's own note, not a source note
            edits = {ln: None}
            for k in [x for x, _ in t.header + t.rows] + [t.header[0][0] - 1 if t.header else ln]:
                if "[%s]" % n in f.lines[k]:
                    edits[k] = re.sub(r" ?\[%s\]" % n, "", f.lines[k])
            out.append(("note dropped: %s" % validate.clip(f.lines[ln], 70), edits))
    return out


def in_remark(f, ln):
    """Whether a line is part of a paragraph the converter labelled as its own
    (Conversion note:, Source error:, …): nothing in the source says it."""
    k = ln
    while k >= 0 and f.lines[k].strip():
        if validate.ISSUE_RE.search(f.lines[k]):
            return True
        k -= 1
    return False


def m_note_cut(f):
    """A table note loses its last clause."""
    out = []
    for t in f.tables:
        for ln in t.notes:
            line = f.lines[ln]
            if validate.ISSUE_RE.search(line):
                continue
            m = re.search(r"^(\[\d+\] .{20,}?)(?:,| and) [^,]{12,}$", line.rstrip("."))
            if m:
                out.append(("note cut short: %s" % validate.clip(line, 70), {ln: m.group(1) + "."}))
    return out


def m_head_drop(f):
    """A table loses its first header row (a group row over the columns)."""
    out = []
    for t in f.tables:
        if len(t.header) >= 2:
            ln, cells = t.header[0]
            if any(c.strip() == "<" for c in cells):
                out.append(("header row dropped: %s" % validate.clip(f.lines[ln], 70), {ln: None}))
    return out


def m_negation(f):
    out = []
    notes = [ln for t in f.tables for ln in t.notes]
    for ln in notes + [ln for ln, _ in f.prose]:
        m = re.search(r"\b(is|are|does|do|may|must|should|can|will) not\b", f.lines[ln])
        if m and not validate.ISSUE_RE.search(f.lines[ln]) and not in_remark(f, ln):
            new = f.lines[ln][:m.start()] + m.group(1) + f.lines[ln][m.end():]
            out.append(("'not' removed: %s" % validate.clip(f.lines[ln][max(0, m.start() - 30):], 70), {ln: new}))
    return out


def m_prose_num(f):
    out = []
    for ln, section in f.prose:
        line = f.lines[ln]
        if section == "Ordering information" or line.startswith("|") or in_remark(f, ln):
            continue
        for m in re.finditer(r"(?<![\w.~{/-])\d+(?:\.\d+)?(?= (?:V|mA|µA|ns|pF|°C|Ω|kΩ|MHz|%)\b)", line):
            new = line[:m.start()] + bump(m.group(0)) + line[m.end():]
            out.append(("prose %s -> %s: %s" % (m.group(0), bump(m.group(0)), validate.clip(line, 50)), {ln: new}))
    return out


def m_fig(f):
    out = []
    for kind, attrs, a, b, _ in f.blocks:
        if kind != "figure":
            continue
        for ln in range(a, b):
            m = re.search(r":\s*[^\d]*(\d+(?:\.\d+)?)", f.lines[ln])
            if m:
                new = f.lines[ln][:m.start(1)] + bump(m.group(1)) + f.lines[ln][m.end(1):]
                out.append(("figure parameter %s -> %s" % (f.lines[ln].strip(), new.strip()), {ln: new}))
    return out


def ordering_lines(f):
    for kind, attrs, a, b, _ in f.blocks:
        if kind == "ordering":
            for ln in range(a, b):
                cells = [c.strip() for c in f.lines[ln].split("|")]
                if len(cells) == 4:
                    yield ln, cells


def m_ord_qty(f):
    out = []
    for ln, cells in ordering_lines(f):
        m = re.search(r"\d+", cells[1])
        if m:
            new = cells[1][:m.start()] + bump(m.group(0)) + cells[1][m.end():]
            out.append(("%s packing %s -> %s" % (cells[0], cells[1], new),
                        {ln: " | ".join([cells[0], new, cells[2], cells[3]])}))
    return out


def m_ord_temp(f):
    out = []
    for ln, cells in ordering_lines(f):
        m = re.search(r"\d+$", cells[2])
        if m:
            new = cells[2][:m.start()] + ("85" if m.group(0) != "85" else "125")
            out.append(("%s temperature %s -> %s" % (cells[0], cells[2], new),
                        {ln: " | ".join([cells[0], cells[1], new, cells[3]])}))
    return out


def m_ord_mark(f):
    out = []
    for ln, cells in ordering_lines(f):
        if len(cells[3]) >= 3:
            ch = cells[3][-1]
            new = cells[3][:-1] + ("8" if ch != "8" else "3")
            out.append(("%s marking %s -> %s" % (cells[0], cells[3], new),
                        {ln: " | ".join(cells[:3] + [new])}))
    return out


def m_ord_part(f):
    rows = list(ordering_lines(f))
    out = []
    for (l1, c1), (l2, c2) in zip(rows, rows[1:]):
        if c1[1:] != c2[1:]:
            out.append(("%s and %s exchange their ordering data" % (c1[0], c2[0]),
                        {l1: " | ".join([c1[0]] + c2[1:]), l2: " | ".join([c2[0]] + c1[1:])}))
    return out


def fm_edit(f, key_re, change):
    for ln in range(1, f.fm_end):
        m = re.search(key_re, f.lines[ln])
        if m:
            new = change(m.group(1))
            if new is not None and new != m.group(1):
                return [("%s -> %s" % (m.group(0), new), {ln: f.lines[ln][:m.start(1)] + new + f.lines[ln][m.end(1):]})]
    return []


def m_fm_pages(f):
    return fm_edit(f, r'"pages":\s*(\d+)', lambda v: str(int(v) + 1))


def m_fm_doc(f):
    return fm_edit(f, r'"document":\s*"([^"]+)"',
                   lambda v: v[:-1] + chr(ord(v[-1]) + 1) if v[-1].isalpha() and v[-1] not in "zZ" else v + "A")


def m_fm_date(f):
    return fm_edit(f, r'"date":\s*"(\d{4})', lambda v: str(int(v) - 1))


def m_claim_drop(f):
    for ln in range(1, f.fm_end):
        m = re.search(r'"parts":\s*\[(.*)\]', f.lines[ln])
        if m:
            parts = re.findall(r'"[^"]+"', m.group(1))
            if len(parts) > 1 and not any("*" in p for p in parts):
                gone = parts[-1]
                new = f.lines[ln][:m.start(1)] + ", ".join(parts[:-1]) + f.lines[ln][m.end(1):]
                edits = {ln: new}
                for k, cells in ordering_lines(f):
                    if '"%s"' % cells[0] == gone:
                        edits[k] = None
                return [("part %s no longer claimed" % gone, edits)]
    return []


KINDS = collections.OrderedDict([
    ("value", m_value), ("value-swap", m_value_swap), ("sign", m_sign), ("column", m_column), ("unit", m_unit),
    ("row-drop", m_row_drop), ("cond", m_cond), ("symbol", m_symbol), ("ftable", m_ftable),
    ("note-drop", m_note_drop), ("note-cut", m_note_cut), ("head-drop", m_head_drop), ("negation", m_negation), ("prose-num", m_prose_num), ("fig", m_fig),
    ("ord-qty", m_ord_qty), ("ord-temp", m_ord_temp), ("ord-mark", m_ord_mark), ("ord-part", m_ord_part),
    ("fm-pages", m_fm_pages), ("fm-doc", m_fm_doc), ("fm-date", m_fm_date), ("claim-drop", m_claim_drop),
])


# ---------------------------------------------------------------- running

def findings(path, cat, cache):
    """What both checkers say about a file, without line numbers."""
    out = collections.Counter()
    rep, _, _ = check.check_file(path, cat, {})
    for _, msg in rep.errors:
        out[("error", "format", msg)] += 1
    for _, msg in rep.warnings:
        out[("warning", "format", msg)] += 1
    for sev, chk, _, msg in validate.validate(path, "auto", cache).items:
        if sev == "error":
            out[(sev, chk, re.sub(r" line \d+", "", msg))] += 1
    return out


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("files", nargs="+")
    ap.add_argument("--per", type=int, default=2)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--kinds", default="")
    ap.add_argument("--missed", action="store_true")
    ap.add_argument("--min", type=float, default=None)
    ap.add_argument("--cache", default=os.path.join(tempfile.gettempdir(), "dsvalidate"))
    args = ap.parse_args(argv)
    kinds = [k for k in args.kinds.split(",") if k] or list(KINDS)
    for k in kinds:
        if k not in KINDS:
            sys.exit('unknown kind "%s"; kinds: %s' % (k, ", ".join(KINDS)))
    cat = check.Catalog()
    rng = random.Random(args.seed)
    tally = {k: collections.Counter() for k in kinds}
    by = {k: collections.Counter() for k in kinds}
    missed, scanned = [], []
    with tempfile.TemporaryDirectory() as tmp:
        for path in args.files:
            f = File(path)
            base = findings(path, cat, args.cache)
            if validate.validate(path, "auto", args.cache).scanned:
                scanned.append(os.path.basename(path))
                continue          # a scanned source: the script leaves its tables and text to a person
            copy = os.path.join(tmp, os.path.basename(path))
            for kind in kinds:
                cands = KINDS[kind](f)
                rng.shuffle(cands)
                for what, edits in cands[:args.per]:
                    lines = [edits.get(k, line) for k, line in enumerate(f.lines)]
                    with open(copy, "w", encoding="utf-8") as out:
                        out.write("\n".join(line for line in lines if line is not None))
                    new = findings(copy, cat, args.cache) - base
                    sevs = {s for s, _, _ in new}
                    result = "error" if "error" in sevs else "missed"
                    tally[kind][result] += 1
                    for s, chk, _ in new:
                        if s == result:
                            by[kind][chk] += 1
                    if result == "missed":
                        missed.append("%s [%s] %s" % (os.path.basename(path), kind, what))
    print("%-11s %7s %7s %7s %6s  %s" % ("kind", "mutants", "caught", "missed", "rate", "caught by"))
    low = False
    for kind in kinds:
        t = tally[kind]
        n = sum(t.values())
        if not n:
            print("%-11s %7d" % (kind, 0))
            continue
        pct = 100.0 * t["error"] / n
        low |= args.min is not None and pct < args.min
        print("%-11s %7d %7d %7d %5.0f%%  %s" % (kind, n, t["error"], t["missed"], pct,
              ", ".join("%s %d" % kv for kv in by[kind].most_common(4))))
    if scanned:
        print("not tested (scanned sources, checked by eye): %s" % ", ".join(scanned))
    if args.missed:
        for m in missed:
            print("  missed: " + m)
    return 1 if low else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
