#!/usr/bin/env python3
"""Validate a converted datasheet against its source PDF, without an agent.

check.py says whether a file follows the format. This script says whether
its content matches the PDF it came from. It reads the PDF's text layer
with pdftotext (and OCRs pages that have none, with tesseract), then runs
these checks:

  numbers     Every number and hex value in the file must appear in the PDF.
  rows        Every table row must have a match in the PDF: the row's
              values must sit on the same PDF line (or within two lines) as
              the row's words. In spec tables a miss is an error.
  terms       Identifiers (register, bit, pin and symbol names, anything
              with a digit or inner capitals) must appear in the PDF.
  prose       Each sentence, list item, table note and long table cell is
              matched to the closest PDF sentence. A weak match is reported
              for review; so is a sentence whose negation ("not", "no") or
              obligation ("must", "shall") differs from that PDF sentence.
  notes       Every table note in the PDF ("[1] ...", "Notes: 1. ...") on
              a converted page must be carried into the file.
  pages       Converted PDF pages where most numbers are missing from the
              file are reported (dropped content).
  registers   In a register map with Bit 7 .. Bit 0 and Reset columns,
              fixed 0/1 bits must agree with the reset value.
  waveforms   Edges must join defined nodes; a node must sit on a
              transition; a data bus must not be driven while its active-low
              output enable has been high for two columns.
  visual      Waveforms, curves and figures can't be checked from text;
              each is listed with the PDF page it most likely came from.

Errors are facts that don't match; warnings need a person to look.
The exit status is 1 when any file has errors.

Usage: python3 -I validate.py [--quiet] [--info] [--max N] [--json]
                              [--ocr auto|off|force] [--cache DIR] files...
"""

import argparse
import collections
import hashlib
import json
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unicodedata
from decimal import Decimal, InvalidOperation

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
DATASHEETS_DIR = os.path.join(REPO, "datasheets")

PROSE_MIN = 0.6        # weighted share of a sentence's words found in one PDF passage
PROSE_LOW = 0.35       # below this a sentence is "not found", above it "reworded"
ROW_TEXT_MIN = 0.5     # share of a row's words found next to its values
NOTE_MIN = 0.5         # share of a PDF note's words found in one place in the file
PAGE_MIN = 0.4         # share of a page's numbers found in the file
SKIP_SECTIONS = {"conversion notes"}
SKIP_BLOCKS = {"pinout", "package", "ordering"}
VALUE_COLS = {"min", "typ", "nom", "max", "value"}

STOP = set("""a an and are as at be been by can for from has have if in into is it its
of on or that the their then there these this to was were which while will with
when where who also any each per than other used use using only both either all
same such so up out over under after before between""".split())
NEG = {"no", "not", "never", "none", "nothing", "cannot", "without", "neither", "nor"}
OBLIGE = {"must", "shall", "required", "requires", "mandatory"}
UNITS = {"ns", "us", "μs", "ms", "s", "ps", "hz", "khz", "mhz", "ghz", "v", "mv", "μv", "kv",
         "a", "ma", "μa", "na", "pa", "w", "mw", "μw", "ω", "kω", "mω", "pf", "nf", "μf", "uf",
         "mm", "mil", "c", "k", "kb", "mb", "dbm", "db", "bit", "bits", "byte", "bytes", "x"}

TOKEN_RE = re.compile(r"(?<![\w.])\.\d+|[^\W_]+(?:[.,]\d+)*")
NUM_RE = re.compile(r"^\d+(?:\.\d+)?$")
NUM_UNIT_RE = re.compile(r"^(\d+(?:\.\d+)?)([^\W\d_]{1,6})$")
NAME_NUM_RE = re.compile(r"^([^\W\d_]{2,})x?(\d*\.\d+)$")     # VDDx0.8
HEX_RE = re.compile(r"^0x([0-9a-f]+)$")
HEX_H_RE = re.compile(r"^([0-9a-f]+)h$")
BARE_HEX_RE = re.compile(r"^[0-9a-f]{1,8}$")
MARK_RE = re.compile(r"(?<![\w\\])\[(\d+)\]")
ISSUE_RE = re.compile(r"\b(Source conflict|Source error|Unconfirmed)\b")
DASHES = {ord(c): "-" for c in "‐‑‒–—−"}
QUOTES = {ord(c): "'" for c in "‘’‛′"}
QUOTES.update({ord(c): '"' for c in "“”"})


# ---------------------------------------------------------------- text

def norm(s):
    s = unicodedata.normalize("NFKC", s).translate(DASHES).translate(QUOTES).lower()
    s = s.replace("n't", " not").replace("cannot", "can not")
    return s


def canon_num(t):
    try:
        return format(Decimal(t).normalize(), "f")
    except InvalidOperation:
        return t


def canon_hex(h):
    return "0x" + (h.lstrip("0") or "0")


def tokens(s):
    """Normalized tokens. Numbers are canonical; '4.7mA' gives '4.7', 'ma'."""
    out = []
    for t in TOKEN_RE.findall(s):
        if t.startswith("."):
            t = "0" + t
        if re.fullmatch(r"\d+(?:,\d+)+(?:\.\d+)?", t):
            if re.fullmatch(r"\d{1,3}(?:,\d{3})+(?:\.\d+)?", t):
                out.append(canon_num(t.replace(",", "")))
            else:
                out += [canon_num(p) for p in t.split(",")]
            continue
        t = t.replace(",", ".") if re.fullmatch(r"\d+,\d+", t) else t
        if NUM_RE.match(t):
            out.append(canon_num(t))
            continue
        m = HEX_RE.match(t)
        if m:
            out.append(canon_hex(m.group(1)))
            continue
        if HEX_H_RE.match(t) and t[0].isdigit():
            out.append(t)          # 19h stays a hex value
            continue
        m = NUM_UNIT_RE.match(t) or NAME_NUM_RE.match(t)
        if m:
            a, b = m.group(1), m.group(2)
            out += [canon_num(a), b] if a[0].isdigit() else [a, canon_num(b)]
            continue
        out.append(t)
    return out


def is_num(t):
    return bool(NUM_RE.match(t)) or t.startswith("0x")


def content(toks):
    return [t for t in toks if t not in STOP and (len(t) > 1 or t.isdigit())]


def md_plain(s):
    s = re.sub(r"!\{([^}]*)\}", r"\1", s)
    s = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", s)
    s = MARK_RE.sub(" ", s)
    s = re.sub(r"\^([^\s^]+)\^", r" \1", s)
    s = s.replace("~", "").replace("**", "").replace("`", "").replace("\\", "")
    s = re.sub(r"(?<!\w)\*|\*(?!\w)", "", s)
    return s


def clip(s, n=110):
    s = re.sub(r"\s+", " ", s).strip()
    return s if len(s) <= n else s[:n - 1] + "…"


# ---------------------------------------------------------------- source PDF

def run(cmd):
    return subprocess.run(cmd, check=True, capture_output=True).stdout.decode("utf-8", "replace")


def ocr_page(pdf, page, tmp):
    prefix = os.path.join(tmp, "p")
    run(["pdftoppm", "-r", "300", "-gray", "-png", "-f", str(page), "-l", str(page), pdf, prefix])
    img = [f for f in os.listdir(tmp) if f.startswith("p") and f.endswith(".png")]
    if not img:
        return ""
    path = os.path.join(tmp, img[0])
    text = run(["tesseract", path, "stdout", "--psm", "4"])
    os.remove(path)
    return text


def load_pdf(pdf, ocr, cache):
    """Pages of (raw reading-order text, layout text, ocr flag), cached."""
    with open(pdf, "rb") as f:
        digest = hashlib.sha1(f.read()).hexdigest()
    path = os.path.join(cache, "%s-%s.json" % (digest, ocr))
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    raw = run(["pdftotext", "-enc", "UTF-8", pdf, "-"]).split("\f")
    lay = run(["pdftotext", "-enc", "UTF-8", "-layout", pdf, "-"]).split("\f")
    n = min(len(raw), len(lay))
    pages = [[raw[i], lay[i], False] for i in range(n)]
    while pages and not pages[-1][0].strip() and not pages[-1][1].strip():
        pages.pop()
    can_ocr = shutil.which("tesseract") and shutil.which("pdftoppm")
    if ocr != "off" and can_ocr:
        with tempfile.TemporaryDirectory() as tmp:
            for i, p in enumerate(pages):
                if ocr == "force" or len(re.sub(r"\s", "", p[0])) < 80:
                    text = ocr_page(pdf, i + 1, tmp)
                    if len(re.sub(r"\s", "", text)) > len(re.sub(r"\s", "", p[0])):
                        pages[i] = [text, text, True]
    os.makedirs(cache, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(pages, f)
    return pages


class Source:
    def __init__(self, pages):
        self.pages = pages
        self.ocr_pages = [i + 1 for i, p in enumerate(pages) if p[2]]
        self.vocab = set()
        self.page_nums = []
        # layout lines, for table rows and notes
        self.lines = []          # (page, text, tokens)
        self.line_post = collections.defaultdict(list)
        for pi, (_, lay, _) in enumerate(pages):
            nums = set()
            for text in lay.split("\n"):
                text = self.fix(text)
                toks = tokens(norm(text))
                # "1 0 0 1" bit codes as one token, and hex written $4D, 4Dh or bare
                toks += [b.replace(" ", "") for b in re.findall(r"(?<![\w.])[01](?: [01])+(?![\w.])", text)]
                toks += [canon_hex(h.lower()) for h in re.findall(r"(?:\$|\b)([0-9A-Fa-f]{1,4})[hH]?\b", text)
                         if re.search(r"[A-Fa-f]", h) or text.find("$" + h) >= 0]
                lid = len(self.lines)
                self.lines.append((pi + 1, text, toks))
                for t in set(toks):
                    self.line_post[t].append(lid)
                self.vocab.update(toks)
                nums.update(t for t in toks if is_num(t))
            self.page_nums.append(nums)
        # reading-order sentences, for prose
        self.sents = []          # (page, text, token set)
        self.sent_post = collections.defaultdict(list)
        for pi, (raw, _, _) in enumerate(pages):
            raw = self.fix(raw)
            raw = re.sub(r"(\w)-\n\s*(\w)", r"\1\2", raw)
            for para in re.split(r"\n\s*\n", raw):
                para = re.sub(r"\s+", " ", para)
                for s in re.split(r"(?<=[.!?;:])\s+(?=[A-Z(\[•])|\s+•\s+", para):
                    toks = set(content(tokens(norm(s))))
                    if not toks:
                        continue
                    sid = len(self.sents)
                    self.sents.append((pi + 1, s, toks))
                    for t in toks:
                        self.sent_post[t].append(sid)
                    self.vocab.update(toks)
        self.hexes = set()
        for t in list(self.vocab):
            m = HEX_H_RE.match(t)
            if m:
                self.hexes.add(canon_hex(m.group(1)))
            if BARE_HEX_RE.match(t):
                self.hexes.add(canon_hex(t))
            if t.startswith("0x"):
                self.hexes.add(t)
        for pi, (raw, lay, _) in enumerate(pages):
            for m in re.finditer(r"\$([0-9A-Fa-f]+)\b", lay):
                self.hexes.add(canon_hex(m.group(1).lower()))
            for m in re.finditer(r"(?<![\w.])([0-9A-Fa-f]{1,8})[hH]?(?![\w.])", lay):
                self.hexes.add(canon_hex(m.group(1).lower()))
        self.nsent = max(1, len(self.sents))

    @staticmethod
    def fix(text):
        # I²C with a lost superscript prints as "I C".
        return re.sub(r"\bI C\b", "I2C", text)

    def has(self, t):
        if t.startswith("0x"):
            return t in self.hexes or t in self.vocab
        return t in self.vocab

    def idf(self, t):
        return math.log(1 + self.nsent / (1 + len(self.sent_post.get(t, ()))))

    def best_passage(self, toks):
        """Closest run of up to three sentences: (score, sentence id)."""
        toks = set(toks)
        if not toks:
            return 0.0, None
        weight = {t: self.idf(t) for t in toks}
        total = sum(weight.values())
        known = sorted((t for t in toks if t in self.sent_post), key=lambda t: len(self.sent_post[t]))
        cand = set()
        for t in known[:4]:
            cand.update(self.sent_post[t][:600])
        best = (0.0, None)
        for sid in cand:
            win = set()
            for k in (sid - 1, sid, sid + 1):
                if 0 <= k < len(self.sents) and self.sents[k][0] == self.sents[sid][0]:
                    win |= self.sents[k][2]
            score = sum(weight[t] for t in toks & win) / total
            if score > best[0]:
                best = (score, sid)
        if best[1] is not None:
            # the single sentence of the run that carries the most of it
            sid = best[1]
            best = (best[0], max((k for k in (sid - 1, sid, sid + 1) if 0 <= k < len(self.sents)),
                                 key=lambda k: sum(weight.get(t, 0) for t in toks & self.sents[k][2])))
        return best

    def absorb(self, words, missing):
        """Take a row's missing values from every place in the PDF that holds
        the row's text (a row may join one table per device or mode)."""
        known = sorted({t for t in words if t in self.line_post}, key=lambda t: len(self.line_post[t]))
        if not known:
            return missing
        left = collections.Counter(missing)
        used = set()
        for lid in self.line_post[known[0]]:
            page = self.lines[lid][0]
            near = [k for k in range(lid - 5, lid + 6) if 0 <= k < len(self.lines) and self.lines[k][0] == page]
            wset = set().union(*(self.lines[k][2] for k in near))
            if sum(t in wset for t in words) / len(words) < ROW_TEXT_MIN:
                continue
            for k in range(lid - 2, lid + 3):
                if k in used or not (0 <= k < len(self.lines)) or self.lines[k][0] != page:
                    continue
                hit = False
                for t in self.lines[k][2]:
                    if left[t] > 0:
                        left[t] -= 1
                        hit = True
                if hit:
                    used.add(k)
        return list(left.elements())

    def best_row(self, toks, exclude=()):
        """Closest window of five layout lines: (text share, value share, missing values, line id)."""
        words = [t for t in toks if not is_num(t)]
        vals = [t for t in toks if is_num(t)]
        known = sorted({t for t in toks if t in self.line_post}, key=lambda t: len(self.line_post[t]))
        cand = set()
        rare_vals = [t for t in known if is_num(t)][:2]      # the values' own lines too
        for t in known[:3] + rare_vals:
            cand.update(self.line_post[t])
        best = (0.0, 0.0, vals, None, -1.0)
        for lid in cand:
            if lid in exclude:
                continue
            page = self.lines[lid][0]
            win, wset = collections.Counter(), set()
            for k in range(lid - 5, lid + 6):
                if 0 <= k < len(self.lines) and self.lines[k][0] == page:
                    wset.update(self.lines[k][2])        # words: a wider window
                    if abs(k - lid) <= 2:
                        win.update(self.lines[k][2])     # values: close by
            w_share = sum(t in wset for t in words) / len(words) if words else 1.0
            left = collections.Counter(win)
            missing = []
            for v in vals:
                if left[v] > 0:
                    left[v] -= 1
                elif not (v.startswith("0x") and (v[2:] in left or v[2:] + "h" in left)):
                    missing.append(v)
            v_share = 1 - len(missing) / len(vals) if vals else 1.0
            score = w_share + 2 * v_share
            if score > best[4]:
                best = (w_share, v_share, missing, lid, score)
        return best[:4]


# ---------------------------------------------------------------- converted file

def parse_info(info):
    kind, _, rest = info.strip().partition(" ")
    attrs = {}
    for m in re.finditer(r'(\w[\w-]*)=("([^"]*)"|\S+)', rest):
        attrs[m.group(1)] = m.group(3) if m.group(3) is not None else m.group(2)
    return kind, attrs


def split_row(line):
    s = line.strip()
    if not (s.startswith("|") and s.endswith("|")) or len(s) < 2:
        return None
    return [c.strip() for c in s[1:-1].split("|")]


class Doc:
    """The parts of a converted file that carry source facts."""

    def __init__(self, path):
        with open(path, encoding="utf-8") as f:
            self.text = f.read()
        lines = self.text.split("\n")
        end = lines.index("---", 1)
        self.fm = json.loads("\n".join(lines[1:end]))
        self.units = []      # (line, text) prose to match
        self.tables = []     # dicts: line, attrs, header, rows [(line, cells)], spec
        self.blocks = []     # (line, kind, attrs, body)
        self.terms = []      # (line, plain text) every text that carries facts
        self.converted_pages = None
        self.derived = set()     # numbers listed under "Derived" in Conversion notes
        self.in_derived = False
        section = ""
        para, para_line = [], 0
        i = end + 1

        def flush():
            if para and section.lower() not in SKIP_SECTIONS:
                self.add_text(para_line, " ".join(para))
            para.clear()

        while i < len(lines):
            line = lines[i]
            if line.startswith("```") and len(line.strip()) > 3:
                flush()
                kind, attrs = parse_info(line[3:])
                j = i + 1
                while j < len(lines) and lines[j].strip() != "```":
                    j += 1
                if section.lower() not in SKIP_SECTIONS:
                    self.add_block(i + 1, kind, attrs, lines[i + 1:j])
                i = j + 1
                continue
            if line.startswith("#"):
                flush()
                if line.startswith("## "):
                    section = line[3:].strip()
            elif not line.strip():
                flush()
            elif re.match(r"^\s*([-*]|\d+\.)\s+", line):
                flush()
                para_line = i + 1
                para.append(re.sub(r"^\s*([-*]|\d+\.)\s+", "", line))
            else:
                if not para:
                    para_line = i + 1
                para.append(line.strip())
            if section.lower() == "conversion notes":
                if re.match(r"^\s*[-*]\s+(Derived|From images)\b", line) or (self.in_derived and line.startswith("  ")):
                    self.in_derived = True
                    vals = re.sub(r"\b(p|pp|page|pages|figure|figures|fig|table|tables|section|chapter)\.?\s*[\d.–-]+(?:\s*(?:and|,)\s*[\d.–-]+)*",
                                  " ", md_plain(line), flags=re.I)
                    self.derived.update(t for t in tokens(norm(vals)) if is_num(t))
                else:
                    self.in_derived = False
                m = re.search(r"\b[Pp]ages? ([\d\s,–-]+(?:(?:and|plus)\s+[\d\s,–-]+)*)\s*converted", re.sub(r"\([^)]*\)", "", line))
                if m:
                    pages = self.converted_pages or set()
                    for a, b in re.findall(r"(\d+)(?:\s*[–-]\s*(\d+))?", m.group(1)):
                        pages.update(range(int(a), int(b or a) + 1))
                    self.converted_pages = pages
            i += 1
        flush()

    def add_text(self, line, text):
        plain = re.sub(r"(?<![\w.])\d{1,3}(?: \d{3})+(?:\.\d+)?(?![\d.])", lambda m: m.group(0).replace(" ", ""), md_plain(text))
        self.terms.append((line, plain))
        for s in re.split(r"(?<=[.!?;])\s+(?=[A-Z(!])", plain):
            if s.strip():
                self.units.append((line, s))

    def add_block(self, line, kind, attrs, body):
        if kind in SKIP_BLOCKS:
            return
        if kind == "table":
            self.add_table(line, attrs, body)
        elif kind == "pins":
            for k, b in enumerate(body):
                names, _, desc = b.partition(":")
                self.terms.append((line + 1 + k, md_plain(names)))
                if desc.strip():
                    self.add_text(line + 1 + k, desc.strip())
        elif kind == "equation":
            for k, b in enumerate(body):
                self.terms.append((line + 1 + k, md_plain(b)))
        self.blocks.append((line, kind, attrs, body))

    def add_table(self, line, attrs, body):
        rows, k = [], 0
        while k < len(body) and body[k].strip():
            cells = split_row(body[k])
            if cells is not None:
                rows.append((line + 1 + k, cells))
            k += 1
        delim = next((n for n, (_, c) in enumerate(rows) if all(re.fullmatch(r":?-+:?", x) for x in c)), None)
        if delim is None:
            return
        header = [c for _, c in rows[:delim]]
        names = [md_plain(re.sub(MARK_RE, "", c)).strip().lower() for c in header[-1]] if header else []
        allnames = {md_plain(c).strip().lower() for h in header for c in h}
        data, above = [], {}
        for ln, cells in rows[delim + 1:]:
            if len(cells) > 1 and all(c == "<" for c in cells[1:]):
                continue      # group heading row
            full = []
            for n, c in enumerate(cells):
                if c == "^":
                    c = above.get(n, "")
                above[n] = c
                full.append("" if c == "<" else c)
            data.append((ln, full))
            self.terms.append((ln, " ".join(md_plain(c) for c in full)))
            for c in full:
                if len(c.split()) >= 4:
                    self.units.append((ln, md_plain(c)))
        for n in body[k:]:
            if re.match(r"^\[\d+\]\s+\S", n.strip()):
                ln = line + 1 + body.index(n)
                self.add_text(ln, re.sub(r"^\[\d+\]\s+", "", n.strip()))
        for key in ("caption", "conditions"):
            if attrs.get(key):
                self.terms.append((line, md_plain(attrs[key])))
        spec = "unit" in allnames and bool(allnames & VALUE_COLS)
        self.tables.append({"line": line, "attrs": attrs, "names": names, "rows": data, "spec": spec})


# ---------------------------------------------------------------- checks

class Report:
    def __init__(self, path):
        self.path = path
        self.items = []      # (severity, check, line, message)
        self.stats = collections.OrderedDict()

    def add(self, sev, check, line, msg):
        self.items.append((sev, check, line, msg))

    def count(self, sev):
        return sum(1 for s, *_ in self.items if s == sev)


def check_numbers_terms(doc, src, rep):
    seen_num, seen_term = set(), set()
    n_num = n_num_ok = n_term = n_term_ok = 0
    for line, plain in doc.terms:
        for raw in TOKEN_RE.findall(unicodedata.normalize("NFKC", plain)):
            low = raw.lower()
            if re.fullmatch(r"0x[0-9a-fx]*x[0-9a-fx]*", low) and "x" in low[2:]:
                continue          # 0xXX: undefined bits
            for t in tokens(norm(raw)):
                if is_num(t):
                    n_num += 1
                    if t in doc.derived or src.has(t) or (t.startswith("0x") and src.has(canon_num(str(int(t[2:], 16))))):
                        n_num_ok += 1
                    elif (t, line) not in seen_num:
                        seen_num.add((t, line))
                        rep.add("error", "numbers", line, "%s is not in the source PDF" % raw)
                elif re.search(r"\d", t) or re.search(r"[a-z][A-Z]|[A-Z]{2}", raw):
                    if len(t) < 2:
                        continue
                    n_term += 1
                    if src.has(t):
                        n_term_ok += 1
                    elif t not in seen_term:
                        seen_term.add(t)
                        rep.add("warning", "terms", line, '"%s" is not in the source PDF' % raw)
    rep.stats["numbers found"] = (n_num_ok, n_num)
    rep.stats["terms found"] = (n_term_ok, n_term)


def check_rows(doc, src, rep):
    total = ok = 0
    for t in doc.tables:
        for line, cells in t["rows"]:
            toks = [x for x in tokens(norm(md_plain(" ".join(cells)))) if x not in STOP]
            if len(toks) < 2:
                continue
            total += 1
            w_share, v_share, missing, lid = src.best_row(toks)
            missing = [m for m in missing if src.has(m) and m not in doc.derived
                       and (t["spec"] or not (m.isdigit() and int(m) < 10))]
            # A row may join several source tables (one per device or per
            # mode): look for the rest of its values in other places that
            # also hold the row's text.
            words = [x for x in toks if not is_num(x)]
            if missing:
                missing = src.absorb(words, missing)
            used = set(range(lid - 2, lid + 3)) if lid is not None else set()
            for _ in range(16):
                if not missing:
                    break
                w2, v2, missing2, lid2 = src.best_row(words + missing, used)
                if lid2 is None:
                    break
                used.update(range(lid2 - 2, lid2 + 3))
                missing2 = [m for m in missing2 if src.has(m)]
                if w2 < ROW_TEXT_MIN or len(missing2) >= len(missing):
                    break
                missing = missing2
            near = ""
            if lid is not None:
                near = " Closest: p.%d \"%s\"" % (src.lines[lid][0], clip(src.lines[lid][1]))
            if missing:
                sev = "error" if t["spec"] else "warning"
                rep.add(sev, "rows", line, "values %s are not next to this row's text in the source.%s"
                        % (", ".join(missing), near))
            elif w_share < ROW_TEXT_MIN and len([x for x in toks if not is_num(x)]) >= 2:
                rep.add("info", "rows", line, "row text not found with its values (%.0f%%).%s"
                        % (100 * w_share, near))
            else:
                ok += 1
    rep.stats["rows matched"] = (ok, total)


def check_prose(doc, src, rep):
    total = ok = 0
    for line, text in doc.units:
        if ISSUE_RE.search(text):
            continue          # the converter's own note on a source problem
        toks = content(tokens(norm(text)))
        if len(set(toks)) < 3:
            continue
        total += 1
        score, sid = src.best_passage(toks)
        near = ""
        if sid is not None:
            near = " Closest: p.%d \"%s\"" % (src.sents[sid][0], clip(src.sents[sid][1], 140))
        if score < PROSE_MIN:
            sev, what = ("warning", "not found in") if score < PROSE_LOW else ("info", "reworded from")
            rep.add(sev, "prose", line, "%s the source (%.0f%%).%s Text: \"%s\""
                    % (what, 100 * score, near, clip(text, 90)))
            continue
        ok += 1
        mine = set(tokens(norm(text)))
        theirs = set(tokens(norm(src.sents[sid][1])))
        mc, tc = set(content(list(mine))), set(content(list(theirs)))
        if score < 0.75 or not tc or len(mc & tc) / len(tc) < 0.5:
            continue
        if bool(mine & NEG) != bool(theirs & NEG):
            which = "the source negates it" if theirs & NEG else "the source does not negate it"
            rep.add("warning", "prose", line, "negation differs: %s.%s" % (which, near))
        elif bool(mine & OBLIGE) != bool(theirs & OBLIGE):
            which = "the source makes it a requirement" if theirs & OBLIGE else "the source doesn't say must"
            rep.add("warning", "prose", line, "obligation differs: %s.%s" % (which, near))
    rep.stats["prose supported"] = (ok, total)


def source_notes(src, pages):
    """Table notes in the PDF: '[n] text' lines and numbered lines after 'Notes:'."""
    notes, cur, in_notes, gap = [], None, False, 0
    for lid, (page, text, _) in enumerate(src.lines):
        if pages and page not in pages:
            continue
        s = text.strip()
        m = (re.match(r"^\[(\d+)\]\s+(\S.{8,})", s)                                # [1] text
             or re.match(r"^notes?\s*(\d+)\s*:\s*(\S.{8,})", s, re.I)                 # Note 2: text
             or re.match(r"^notes?\s*:?\s+(\d+)[.)]\s+(\S.{8,})", s, re.I)            # Note: 1. text
             or re.match(r"^notes?\s*:\s+()([^\d\s].{8,})", s, re.I)                    # Note: text
             or (in_notes and re.match(r"^(\d+)[.)]\s+(\S.{8,})", s)))                   # 1. text after Notes:
        if m or re.match(r"^notes?\s*:?$", s, re.I):
            in_notes, gap = True, 0
        if not m and re.match(r"^notes?\s*:?$", s, re.I):
            continue
        indent = len(text) - len(text.lstrip())
        if m:
            cur = [page, m.group(2), indent]
            notes.append(cur)
            gap = 0
        elif not s:
            cur = None
            gap += 1
            if gap > 1:
                in_notes = False
        elif cur and indent > cur[2] and len(s) < 160:
            cur[1] += " " + s          # continuation, indented under its marker
        else:
            cur = None
    return [(p, t) for p, t, _ in notes]


def check_notes(doc, src, rep):
    md_units = [set(content(tokens(norm(t)))) for _, t in doc.terms]
    idf = src.idf
    total = ok = 0
    for page, text in source_notes(src, doc.converted_pages):
        if re.match(r"^\s*(see|refer to)\b", text, re.I) and len(text) < 120:
            continue          # a cross-reference, not a fact
        toks = set(content(tokens(norm(text))))
        if len(toks) < 3:
            continue
        total += 1
        weight = {t: idf(t) for t in toks}
        tot = sum(weight.values())
        best = max((sum(weight[t] for t in toks & u) / tot for u in md_units), default=0)
        if best < NOTE_MIN:
            rep.add("warning", "notes", 0, "source note on p.%d is not in the file: \"%s\"" % (page, clip(text, 120)))
        else:
            ok += 1
    rep.stats["source notes carried"] = (ok, total)


def check_pages(doc, src, rep):
    mine = set()
    for _, plain in doc.terms:
        mine.update(t for t in tokens(norm(plain)) if is_num(t))
    low = []
    for p in sorted(doc.converted_pages or range(1, len(src.pages) + 1)):
        if p > len(src.pages):
            continue
        nums = {n for n in src.page_nums[p - 1] if not (n.isdigit() and int(n) < 10)}
        if len(nums) >= 10:
            share = len(nums & mine) / len(nums)
            if share < PAGE_MIN:
                low.append("%d (%.0f%%)" % (p, 100 * share))
    if low:
        rep.add("warning", "pages", 0, "few of these pages' numbers are in the file: " + ", ".join(low))


def check_registers(doc, rep):
    for t in doc.tables:
        names = t["names"]
        bits = {n: names.index("bit %d" % n) for n in range(8) if "bit %d" % n in names}
        if len(bits) != 8 or "reset" not in names:
            continue
        rc = names.index("reset")
        for line, cells in t["rows"]:
            if rc >= len(cells):
                continue
            flagged = bool(MARK_RE.search(cells[rc]))      # the conflict has a note
            m = re.fullmatch(r"0x([0-9A-Fa-fXx]{1,2})", MARK_RE.sub("", cells[rc]).strip())
            if not m:
                continue
            digits = m.group(1).rjust(2, "0")
            for b, col in bits.items():
                cell = cells[col].strip() if col < len(cells) else ""
                if cell not in ("0", "1"):
                    continue
                d = digits[1 - b // 4]
                if d in "xX":
                    continue
                if str((int(d, 16) >> (b % 4)) & 1) != cell:
                    rep.add("info" if flagged else "error", "registers", line,
                            "bit %d is fixed at %s but reset %s says %d"
                            % (b, cell, cells[rc], 1 - int(cell)))


LEVELS = set("01hlHLzxud")
DATA = set("=23456789")


def columns(wave):
    """State per column, and whether the column starts a transition."""
    states, trans, prev = [], [], None
    for i, c in enumerate(wave):
        if c in ".|":
            states.append(prev)
            trans.append(c == "." and prev is not None and prev in "pnPN")   # clocks toggle every column
            continue
        changed = i > 0 and (c in DATA or c != prev)
        states.append(c)
        trans.append(changed)
        prev = c
    return states, trans


def flat_signals(sig):
    for s in sig:
        if isinstance(s, list):
            yield from flat_signals(s[1:])
        elif isinstance(s, dict) and ("wave" in s or "node" in s):
            yield s


def active_low(name, base):
    n = name.strip()
    core = re.sub(r"[#!_\s]|^n|b$", "", n, flags=re.I).lower()
    low = n.endswith("#") or n.startswith("!") or n.startswith("/") or n.lower().startswith("n") or n.endswith("B")
    return core == base and low


def check_waveform(rep, line, attrs, body, src):
    try:
        wd = json.loads("\n".join(body))
    except ValueError:
        return
    lanes = list(flat_signals(wd.get("signal", [])))
    sigs = [s for s in lanes if "wave" in s]
    nodes = {}
    for s in lanes:
        if "wave" not in s:          # a node-only lane marks free points
            nodes.update((c, "spacer") for c in s.get("node", "") if c != ".")
            continue
        states, trans = columns(s["wave"])
        for i, c in enumerate(s.get("node", "")):
            if c == ".":
                continue
            nodes[c] = s["name"]
            if i >= len(trans):
                rep.add("error", "waveforms", line, 'node %s on %s is past the end of the wave' % (c, s["name"]))
            elif not trans[i]:
                rep.add("warning", "waveforms", line, 'node %s on %s (column %d) is not on a transition'
                        % (c, s["name"], i))
    for e in wd.get("edge", []):
        m = re.match(r"^\s*([A-Za-z])\s*[-~|<>]+\s*([A-Za-z])\s*(.*)$", e)
        if not m:
            continue
        for n in m.group(1), m.group(2):
            if n not in nodes:
                rep.add("error", "waveforms", line, 'edge "%s" uses node %s, which no signal defines' % (e, n))
        label = m.group(3).strip()
        if label and not all(src.has(t) for t in tokens(norm(label)) if len(t) > 1):
            rep.add("warning", "waveforms", line, 'edge label "%s" is not in the source PDF' % label)
    # A data bus must not be driven while its output enable is off.
    oe = [s for s in sigs if active_low(s["name"], "oe") or active_low(s["name"], "g")]
    we = [s for s in sigs if active_low(s["name"], "we") or active_low(s["name"], "w")]
    buses = [s for s in sigs if "z" in s["wave"] and s not in oe]
    def labels(sig):
        """Data label shown in each column."""
        out, k, cur = [], 0, ""
        data = sig.get("data", [])
        data = data.split() if isinstance(data, str) else data
        for c in sig["wave"]:
            if c in DATA:
                cur = str(data[k]) if k < len(data) else ""
                k += 1
            elif c not in ".|":
                cur = ""
            out.append(cur)
        return out
    host = re.compile(r"addr|cmd|command|\bin\b|input|write", re.I)
    for o in oe:
        ost, _ = columns(o["wave"])
        wst = [columns(w["wave"])[0] for w in we]
        for b in buses:
            bst, _ = columns(b["wave"])
            blab = labels(b)
            for t in range(1, min(len(ost), len(bst))):
                writing = any(w[k] == "0" for w in wst for k in (t, t - 1, t - 2) if 0 <= k < len(w))
                if (ost[t] == "1" and ost[t - 1] == "1" and bst[t] not in ("z", "x", None) and not writing
                        and not host.search(blab[t])):
                    rep.add("error", "waveforms", line, "%s is driven at column %d while %s has been high since column %d"
                            % (b["name"], t, o["name"], t - 1))
                    break


def guess_page(src, text):
    score, sid = src.best_passage(content(tokens(norm(text))))
    return src.sents[sid][0] if sid is not None and score > 0.3 else None


def check_blocks(doc, src, rep):
    for line, kind, attrs, body in doc.blocks:
        if kind == "wavedrom":
            check_waveform(rep, line, attrs, body, src)
        if kind in ("wavedrom", "curve") or (kind == "figure" and "src" in attrs):
            page = guess_page(src, md_plain(attrs.get("caption", "")))
            rep.add("info", "visual", line, "%s %s: compare by eye with the source%s"
                    % (kind, attrs.get("id", ""), " (likely p.%d)" % page if page else ""))


def validate(path, ocr, cache):
    rep = Report(path)
    doc = Doc(path)
    pdf = os.path.join(DATASHEETS_DIR, doc.fm["source"]["pdf"])
    if not os.path.exists(pdf):
        rep.add("error", "source", 0, "source PDF %s not found" % doc.fm["source"]["pdf"])
        return rep
    src = Source(load_pdf(pdf, ocr, cache))
    if src.ocr_pages:
        rep.add("info", "source", 0, "OCR used for pages %s; expect misses there"
                % ", ".join(map(str, src.ocr_pages)))
    check_numbers_terms(doc, src, rep)
    check_rows(doc, src, rep)
    check_prose(doc, src, rep)
    check_notes(doc, src, rep)
    check_pages(doc, src, rep)
    check_registers(doc, rep)
    check_blocks(doc, src, rep)
    flags = ISSUE_RE.findall(doc.text)
    rep.stats["source issues flagged"] = (len(flags), len(flags))
    rep.items.sort(key=lambda x: ({"error": 0, "warning": 1, "info": 2}[x[0]], x[2]))
    return rep


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("files", nargs="+")
    ap.add_argument("--quiet", action="store_true", help="one summary line per file")
    ap.add_argument("--info", action="store_true", help="also list info items (visual checks)")
    ap.add_argument("--max", type=int, default=80, help="items shown per file and severity (0 = all)")
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    ap.add_argument("--ocr", choices=["auto", "off", "force"], default="auto")
    ap.add_argument("--cache", default=os.path.join(tempfile.gettempdir(), "dsvalidate"))
    args = ap.parse_args(argv)
    if not shutil.which("pdftotext"):
        sys.exit("validate.py needs pdftotext (poppler)")
    failed = False
    out = []
    for path in args.files:
        rep = validate(path, args.ocr, args.cache)
        failed |= rep.count("error") > 0
        if args.json:
            out.append({"file": path, "stats": rep.stats,
                        "items": [dict(zip(("severity", "check", "line", "message"), i)) for i in rep.items]})
            continue
        stats = ", ".join("%s %d" % (k, a) if k == "source issues flagged" else "%s %d/%d" % (k, a, b)
                          for k, (a, b) in rep.stats.items())
        print("%s: %d errors, %d warnings; %s" % (path, rep.count("error"), rep.count("warning"), stats))
        if args.quiet:
            continue
        shown = collections.Counter()
        for sev, check, line, msg in rep.items:
            if sev == "info" and not args.info:
                continue
            shown[sev] += 1
            if args.max and shown[sev] > args.max:
                continue
            print("  %s %s%s: %s" % (sev, check, " line %d" % line if line else "", msg))
        for sev, n in shown.items():
            if args.max and n > args.max:
                print("  ... %d more %ss (use --max 0)" % (n - args.max, sev))
    if args.json:
        print(json.dumps(out, indent=1, ensure_ascii=False))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
