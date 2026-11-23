#!/usr/bin/env python3
"""Validate a converted datasheet against its source PDF, without an agent.

check.py says whether a file follows the format. This script says whether
its content matches the PDF it came from. It reads the PDF's text with
pdftotext (and OCRs pages that have none, with tesseract), every word with
its position on the page (pdftotext -bbox-layout) and the list of pictures
(pdfimages), then runs these checks:

  numbers     Every number and hex value in the file is in the PDF.
  terms       Identifiers (names with a digit or inner capitals) are in the PDF.
  rows        A table row's values are next to the row's words in the PDF;
              a function-table row is a row of the PDF.
  cells       Each spec-table row is placed on the PDF line that holds its
              values and compared there: the header each value is under
              (Min/Typ/Max), the order, the sign, the unit, the symbol, the
              condition numbers, and values or rows the file lacks.
  coverage    Every row of a PDF table with Min/Typ/Max headers is in the file.
  ordering    Each ordering line against its row of the PDF's ordering table.
  catalog     The catalog's obsolete flags and pin counts against that table.
  frontmatter source.pages, source.document, source.date, packages.
  pages       The page ranges Conversion notes give.
  figures     A shared figure's parameters are printed in the PDF.
  prose       Each sentence is matched to the closest PDF passage: its words,
              its numbers, its negation ("not") and obligation ("must").
  notes       Every table note of the PDF on a converted page is in the file.
  cites       Each citation [[page::paragraph]] / [[page::Table 6-3]]: the page and
              paragraph exist, the label is printed there, the paragraph's
              sentences, a table's values, a pin block's names are there.
  registers   A register map's fixed bits agree with its reset value.
  waveforms   Edges join defined nodes on transitions; a bus is not driven
              while its output enable is off.

Everything the script can decide is an error. What it can't decide (a
waveform's shape, a curve, a scanned page) it lists as "to check by eye".
There are no warnings. format.md § Validator describes each check; mutate.py
tests them by planting mistakes. The exit status is 1 when any file has errors.

Usage: python3 -I validate.py [--quiet] [--info] [--max N] [--json]
                              [--ocr auto|off|force] [--cache DIR] files...
"""

import argparse
import collections
import fnmatch
import hashlib
import html
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
PROSE_ORDER = 0.8      # share of a sentence's words that follow the order of that passage
PROSE_LOW = 0.35       # below this a sentence is "not found", above it "reworded"
ROW_TEXT_MIN = 0.5     # share of a row's words found next to its values
PARAM_REACH = (3, 8)   # lines above and below a row within which its parameter name is printed
NOTE_FULL = 0.9        # share of a source note's words a file note has when it is kept whole
NOTE_MIN = 0.5         # share of a PDF note's words found in one place in the file
PAGE_MIN = 0.4         # share of a page's numbers found in the file
SKIP_SECTIONS = {"conversion notes"}
SKIP_BLOCKS = {"pinout", "package", "ordering"}
VALUE_COLS = {"min", "typ", "nom", "max", "value"}
NAMED_COLS = {"symbol", "parameter", "thermal metric", "conditions", "test conditions", "from", "to", "unit"} | VALUE_COLS

STOP = set("""a an and are as at be been by can for from has have if in into is it its
of on or that the their then there these this to was were which while will with
when where who also any each per than other used use using only both either all
same such so up out over under after before between""".split())
NEG = {"no", "not", "never", "none", "nothing", "cannot", "without", "neither"}      # not "nor": NOR is a gate
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
CITE_RE = re.compile(r"[ \t]*\[\[(\d+)::([^\[\]]+?)\]\]")
ISSUE_RE = re.compile(r"\b(Source conflict|Source error|Unconfirmed|Conversion note)\b")
DASHES = {ord(c): "-" for c in "‐‑‒–—−"}
QUOTES = {ord(c): "'" for c in "‘’‛′"}
QUOTES.update({ord(c): '"' for c in "“”"})


# ---------------------------------------------------------------- text

def norm(s):
    s = unicodedata.normalize("NFKC", s).translate(DASHES).translate(QUOTES).replace("∆", "Δ").lower()
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
        m = re.fullmatch(r"(to)(\d+(?:\.\d+)?)", t)
        if m:
            out += ["to", canon_num(m.group(2))]        # "to125°C": a lost space
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


ABBREV_RE = re.compile(r"(?:^|[\s(])(e\.g|i\.e|vs|fig|no|typ|min|max|approx|etc)\.$", re.I)


def md_sentences(text):
    """The sentences of a paragraph: split after . ! ? before a letter, not after an abbreviation."""
    out = []
    for piece in re.split(r"(?<=[.!?])\s+(?=[A-Za-z(!])|(?:(?<=[A-Za-z]{2}\.)|(?<=[)%]\.)|(?<= [VA]\.))\s+(?=[±\d])", text):
        if out and ABBREV_RE.search(out[-1]):
            out[-1] += " " + piece
        else:
            out.append(piece)
    return out


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


def running_lines(page_lines):
    """Lines printed on many pages (running headers and footers), digits blanked.
    `page_lines`: per page, its lines of text."""
    seen = collections.defaultdict(set)
    for pi, lines in enumerate(page_lines):
        for line in lines:
            line = line.strip()
            if line and len(line) <= 120:
                seen[re.sub(r"\d+", "#", line.lower())].add(pi)
    need = max(3, math.ceil(0.3 * len(page_lines)))
    return {k for k, v in seen.items() if len(v) >= need}


def is_prose(text):
    """A block of page text that is running prose: five words of three letters or more (not in
    capitals), and at least 30 % of its tokens are such words. A table cell, a pin list or a row of
    symbols and numbers is not."""
    tokens_ = text.split()
    words = sum(bool(re.fullmatch(r"[A-Za-z][A-Za-z'’-]{2,}[.,;:)]*", t)) and not t.rstrip(".,;:)").isupper() for t in tokens_)
    return words >= 5 and 10 * words >= 3 * len(tokens_)


BLOCK_RE = re.compile(r'<block [^>]*>(.*?)</block>', re.S)
LINE_RE = re.compile(r'<line [^>]*>(.*?)</line>', re.S)


def load_blocks(pdf, cache):
    """Per page, the paragraphs poppler's layout analysis finds, in reading
    order: each a list of its lines' text."""
    with open(pdf, "rb") as f:
        digest = hashlib.sha1(f.read()).hexdigest()
    path = os.path.join(cache, "%s-blocks.json" % digest)
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    out = run(["pdftotext", "-enc", "UTF-8", "-bbox-layout", pdf, "-"])
    pages = []
    for chunk in out.split("<page ")[1:]:
        blocks = []
        for blk in BLOCK_RE.findall(chunk):
            lines = [" ".join(html.unescape(w) for w in re.findall(r">([^<]*)</word>", ln)) for ln in LINE_RE.findall(blk)]
            if lines:
                blocks.append(lines)
        pages.append(blocks)
    os.makedirs(cache, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(pages, f)
    return pages


def numbered_blocks(pages, blocks, everything=False):
    """Per page, its paragraphs as (number, text) in reading order: a layout
    block (a page read by OCR: a blank-line separated block of its text) that
    is prose once the running header and footer lines are out. A citation
    [[page::n]] names one. With `everything`, every block is listed, the number
    None for one that is not a paragraph."""
    lines = []
    for pi, (raw, _, ocr) in enumerate(pages):
        if ocr or pi >= len(blocks):
            lines.append([x for x in raw.split("\n")])
        else:
            lines.append([x for b in blocks[pi] for x in b])
    running = running_lines(lines)
    out = []
    for pi, (raw, _, ocr) in enumerate(pages):
        if ocr or pi >= len(blocks):
            texts = [b.split("\n") for b in re.split(r"\n\s*\n", raw)]
        else:
            texts = blocks[pi]
        found, n = [], 0
        for b in texts:
            kept = " ".join(x for x in b if re.sub(r"\d+", "#", x.strip().lower()) not in running)
            if is_prose(kept):
                n += 1
                found.append((n, re.sub(r"\s+", " ", kept).strip()))
            elif everything and kept.strip():
                found.append((None, re.sub(r"\s+", " ", kept).strip()))
        out.append(found)
    return out


class Source:
    def __init__(self, pages, blocks=None):
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
                self.vocab.update(a + b for a, b in zip(toks, toks[1:]) if not is_num(a) and not is_num(b))
                self.vocab.update(a + b + c for a, b, c in zip(toks, toks[1:], toks[2:])
                                  if not (is_num(a) or is_num(b) or is_num(c)) and len(a + b + c) <= 12)
                nums.update(t for t in toks if is_num(t))
            self.page_nums.append(nums)
        # reading-order sentences, for prose
        self.sents = []          # (page, text, token set)
        self.sent_para = []      # per sentence: its paragraph number on the page (numbered_blocks), or None
        self.sent_post = collections.defaultdict(list)
        self.paras = numbered_blocks(pages, blocks or [])
        para_toks = [[(n, self.block_tokens(t)) for n, t in page] for page in self.paras]
        for pi, (raw, _, _) in enumerate(pages):
            raw = self.fix(raw)
            raw = re.sub(r"([a-z])-\n\s*([a-z])", r"\1\2", raw)
            for para in re.split(r"\n\s*\n", raw):
                para = re.sub(r"\s+", " ", para)
                for s in re.split(r"(?<=[.!?;:])\s+(?=[A-Z(\[•])|\s+•\s+|\s+(?=\(\d+\)\s+[A-Z])"
                                  r"|\s+(?=\d+(?:\.\d+)+\s+[A-Z][a-z])|(?<=[a-z])\s+(?=\d+(?:\.\d+)+\s+[A-Z])"
                                  r"|\s+(?=[†‡§¶]\s*[A-Z])", para):
                    toks = set(content(tokens(norm(s))))
                    if not toks:
                        continue
                    sid = len(self.sents)
                    self.sents.append((pi + 1, s, toks, content(tokens(norm(s)))))
                    self.sent_para.append(self.paragraph_of(para_toks[pi], toks))
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
        # "5.1 Absolute Maximum Ratings": a numbered heading, so that "see Section 5.1" can be
        # read as the name the section has
        self.sections = {}
        for _, text, _ in self.lines:
            m = re.match(r"^\s*(\d+(?:\.\d+)+)\s+([A-Z][A-Za-z][A-Za-z0-9 ,/()\-]{2,60}?)\s*(?:\.{3,}.*|\s\d+)?$", text)
            if m and m.group(1) not in self.sections:
                self.sections[m.group(1)] = content(tokens(norm(m.group(2))))
        self.micro = any("µ" in raw or "μ" in raw for raw, _, _ in pages)
        self.page_toks = [set() for _ in pages]
        for page, _, toks in self.lines:
            self.page_toks[page - 1].update(toks)

    @staticmethod
    def block_tokens(text):
        """A block's content words, and the joins of a one or two letter word with the next ("t PLH" is tPLH)."""
        toks = tokens(norm(text))
        both = set(content(toks))
        both.update(a + b for a, b in zip(toks, toks[1:]) if len(a) <= 2 and a.isalpha() and b.isalnum())
        return both

    @staticmethod
    def paragraph_of(page_paras, toks):
        """The number of the page's paragraph that holds most of a sentence's words
        (at least 70 %, the first when two hold as many), or None."""
        best, top = None, 0.7 - 1e-9
        for n, ptoks in page_paras:
            share = len(toks & ptoks) / len(toks)
            if share > top:
                best, top = n, share
        return best

    @staticmethod
    def fix(text):
        # I²C with a lost superscript prints as "I C".
        return re.sub(r"\bI C\b", "I2C", text).replace("#none#", "")

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

    def rivals(self, toks, sid):
        """Other places that hold as many of these words as the closest one:
        a sentence the source prints twice with one word changed (HBM, CDM)."""
        toks = set(toks)
        weight = {t: self.idf(t) for t in toks}
        total = sum(weight.values()) or 1.0
        known = sorted((t for t in toks if t in self.sent_post), key=lambda t: len(self.sent_post[t]))
        cand = set()
        for t in known[:4]:
            cand.update(self.sent_post[t][:600])
        scored = sorted(((sum(weight[t] for t in toks & self.sents[k][2]) / total, k) for k in cand), reverse=True)
        return [sid] + [k for sc, k in scored[:8] if sc >= PROSE_MIN / 2 and k != sid]

    def run(self, sid):
        """The words, in order, of a sentence and its two neighbours on the page,
        a "Section 5.1" followed by the words of that section's heading."""
        out = []
        for k in (sid - 1, sid, sid + 1):
            if 0 <= k < len(self.sents) and self.sents[k][0] == self.sents[sid][0]:
                seq = self.sents[k][3]
                for i, t in enumerate(seq):
                    out.append(t)
                    if t in self.sections and i and seq[i - 1].startswith("section"):
                        out += self.sections[t]
        return out

    def best_lines(self, toks):
        """Share of these words found within five layout lines of one place, and that line's id."""
        toks = set(toks)
        known = sorted((t for t in toks if t in self.line_post), key=lambda t: len(self.line_post[t]))
        best = (0.0, None)
        weight = {t: self.idf(t) for t in toks}
        total = sum(weight.values()) or 1.0
        for t in known[:3]:
            for lid in self.line_post[t][:400]:
                page = self.lines[lid][0]
                win = set()
                for k in range(lid - 2, lid + 3):
                    if 0 <= k < len(self.lines) and self.lines[k][0] == page:
                        win.update(self.lines[k][2])
                score = sum(weight[x] for x in toks & win) / total
                if score > best[0]:
                    best = (score, lid)
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
        self.row_pending, self.placed = [], set()
        self.terms = []      # (line, plain text) every text that carries facts
        self.converted_pages = None
        self.fences = []     # (line, kind, attrs, body, section) every fenced block
        self.prose = []      # (line, section, text) every line outside the blocks
        self.section = ""
        self.derived = set()     # numbers listed under "Derived" in Conversion notes
        self.known = set()       # every token listed there, and the catalog's pin names
        self.row_text = {}       # (line, cell text) -> its whole table row
        self.pin_units = set()   # the units that are pin descriptions (several source rows in one line)
        self.issue_nums = set()  # numbers quoted in a labelled remark (Source error, Unconfirmed, …)
        self.in_derived = False
        self.cites = []          # (line, page, ref, owner): owner ("para", first line) or ("fence", fence line)
        self.paras = []          # (first line, section) of every prose paragraph and list item
        self.unit_para = {}      # (line, sentence) -> first line of its paragraph
        self.where = {}          # (line, sentence) -> ("sent", source sentence id) | ("page", page) once matched
        section = ""
        para, para_line = [], 0
        i = end + 1
        after_fence = None       # (line index after a block's closing fence, the block's line)

        def flush():
            if para and section.lower() not in SKIP_SECTIONS:
                self.paras.append((para_line, section))
                self.add_text(para_line, " ".join(para), [len(p) + 1 for p in para], owner=para_line)
            para.clear()

        while i < len(lines):
            line = lines[i]
            if line.startswith("```") and len(line.strip()) > 3:
                flush()
                kind, attrs = parse_info(line[3:])
                j = i + 1
                while j < len(lines) and lines[j].strip() != "```":
                    j += 1
                self.section = section
                if section.lower() not in SKIP_SECTIONS:
                    self.add_block(i + 1, kind, attrs, lines[i + 1:j])
                self.fences.append((i + 1, kind, attrs, lines[i + 1:j], section))
                after_fence = (j + 1, i + 1)
                i = j + 1
                continue
            found = [(int(a), b.strip()) for a, b in CITE_RE.findall(line)]
            if found:
                line = CITE_RE.sub("", line).rstrip()
                if not line.strip() and after_fence and after_fence[0] == i:
                    self.cites += [(i + 1, a, b, ("fence", after_fence[1])) for a, b in found]
                    found = []
                    i += 1
                    continue
            if line.strip() and not line.startswith("#"):
                self.prose.append((i + 1, section, line))
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
            self.cites += [(i + 1, a, b, ("para", para_line)) for a, b in found]
            if section.lower() == "conversion notes":
                if re.match(r"^\s*[-*]\s+(Derived|From images)\b", line) or (self.in_derived and line.startswith("  ")):
                    self.in_derived = True
                    vals = re.sub(r"\b(p|pp|page|pages|figure|figures|fig|table|tables|section|chapter)\.?\s*[\d.–-]+(?:\s*(?:and|,)\s*[\d.–-]+)*",
                                  " ", md_plain(line), flags=re.I)
                    self.derived.update(t for t in tokens(norm(vals)) if is_num(t))
                    self.known.update(tokens(norm(vals)))
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

    def add_text(self, line, text, widths=None, owner=None):
        if ISSUE_RE.search(text):
            self.issue_nums.update(t for t in tokens(norm(md_plain(text))) if is_num(t))
        """`widths`: the length of each source line of `text` (joined with
        spaces), so every sentence gets the line it starts on."""
        group = lambda t: re.sub(r"(?<![\w.])\d{1,3}(?: \d{3})+(?:\.\d+)?(?![\d.])",
                                 lambda m: m.group(0).replace(" ", ""), t)
        self.terms.append((line, group(md_plain(text))))
        pos = 0
        for s in md_sentences(text):
            at, k, acc = line, 0, 0
            for k, w in enumerate(widths or ()):
                if pos < acc + w:
                    break
                acc += w
            at = line + k if widths else line
            pos += len(s) + 1
            if re.fullmatch(r"\s*(See|Refer to) \[[^\]]+\]\(#[^)]+\)\.?\s*", s):
                continue          # a link to another section of the file
            if s.strip():
                self.units.append((at, group(md_plain(s))))
                if owner is not None:
                    self.unit_para[(at, self.units[-1][1])] = owner

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
                    before = len(self.units)
                    self.add_text(line + 1 + k, desc.strip())
                    self.pin_units.update(self.units[before:])
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
        # notes that are the converter's (Source error: …): a cell carrying such a marker was corrected
        own_notes = {m.group(1) for n in body[k:] for m in [re.match(r"^\[(\d+)\]\s+(.*)", n.strip())]
                     if m and ISSUE_RE.search(m.group(2))}
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
            for n, c in enumerate(full):
                if cells[n] in ("^", "<") or set(MARK_RE.findall(c)) & own_notes:
                    continue      # said once on the row above; or corrected, with a labelled note
                if len(c.split()) >= 4:
                    plain = md_plain(MARK_RE.sub("", c))
                    self.units.append((ln, plain))
                    self.row_text[(ln, plain)] = md_plain(" ".join(full))
        for n in body[k:]:
            if re.match(r"^\[\d+\]\s+\S", n.strip()):
                ln = line + 1 + body.index(n)
                self.add_text(ln, re.sub(r"^\[\d+\]\s+", "", n.strip()))
        for key in ("caption", "conditions"):
            if attrs.get(key):
                self.terms.append((line, md_plain(attrs[key])))
        spec = "unit" in allnames and bool(allnames & VALUE_COLS)
        # column names through the header's merges, and the value columns
        cols = []
        for c in range(len(header[-1]) if header else 0):
            name = ""
            for cells in reversed(header):
                v, cc = (cells[c] if c < len(cells) else ""), c
                if v == "^":
                    continue
                while v == "<" and cc > 0:
                    cc -= 1
                    v = cells[cc]
                name = v
                break
            cols.append(md_plain(MARK_RE.sub("", name)).strip().lower())
        if "values" in attrs:
            vcols = [c for c, n in enumerate(cols) if c > 0 and n not in NAMED_COLS]
        else:
            vcols = [c for c, n in enumerate(cols) if n in VALUE_COLS]
            if not vcols and "unit" in cols and self.section == "Thermal information":
                vcols = [c for c, n in enumerate(cols) if c > 0 and n not in NAMED_COLS]
        raw = [(ln, cells) for ln, cells in rows[delim + 1:]]
        notes = [n.strip() for n in body[k:] if re.match(r"^\[\d+\]\s+\S", n.strip())]
        self.tables.append({"line": line, "attrs": attrs, "names": names, "rows": data, "spec": spec,
                            "own_notes": own_notes,
                            "cols": cols, "vcols": vcols, "raw": raw, "header": header, "notes": notes,
                            "section": self.section})


# ---------------------------------------------------------------- checks

class Report:
    def __init__(self, path):
        self.path = path
        self.items = []      # (severity, check, line, message)
        self.scanned = False # the converted pages are scans (no text layer)
        self.stats = collections.OrderedDict()

    def add(self, sev, check, line, msg):
        """sev: "error" (the file and the PDF disagree), "manual" (this script
        can't decide it: a person compares), "info"."""
        if sev == "soft":          # an error, unless the page is a scan the script reads by OCR
            sev = "manual" if self.scanned else "error"
            if sev == "manual":
                msg += " [scanned source: check by eye]"
        self.items.append((sev, check, line, msg))

    def count(self, sev):
        return sum(1 for s, *_ in self.items if s == sev)


def check_numbers_terms(doc, src, rep):
    seen_num, seen_term = set(), set()
    n_num = n_num_ok = n_term = n_term_ok = 0
    for line, plain in doc.terms:
        labelled = bool(ISSUE_RE.search(plain))       # the converter's own remark: its numbers count, its words don't
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
                        rep.add("soft", "numbers", line, "%s is not in the source PDF (a value calculated or read from "
                                "a drawing goes under Derived or From images in Conversion notes)" % raw)
                elif re.search(r"\d", t) or re.search(r"[a-z][A-Z]|[A-Z]{2}", raw):
                    if len(t) < 2:
                        continue
                    n_term += 1
                    if src.has(t) or t in doc.known or labelled:
                        n_term_ok += 1
                    elif t not in seen_term:
                        seen_term.add(t)
                        rep.add("soft", "terms", line, '"%s" is not in the source PDF (a name read from a drawing '
                                'goes under From images in Conversion notes)' % raw)
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
                doc.row_pending.append((line, "values %s are not next to this row's text in the source.%s"
                                        % (", ".join(missing), near)))
            elif w_share < ROW_TEXT_MIN and len([x for x in toks if not is_num(x)]) >= 2:
                rep.add("info", "rows", line, "row text not found with its values (%.0f%%).%s"
                        % (100 * w_share, near))
            else:
                ok += 1
    rep.stats["rows matched"] = (ok, total)


def lcs(a, b):
    """Length of the longest common subsequence of two token lists."""
    prev = [0] * (len(b) + 1)
    for x in a:
        cur = [0]
        for j, y in enumerate(b):
            cur.append(prev[j] + 1 if x == y else max(prev[j + 1], cur[j]))
        prev = cur
    return prev[-1]


def same_joins(a, b):
    """Two token lists with "power down" and "powerdown" written one way:
    a word broken at a line end, or hyphenated in one text only."""
    def join(x, other):
        have, out, i = set(other), [], 0
        while i < len(x):
            if i + 1 < len(x) and x[i] not in have and x[i] + x[i + 1] in have:
                out.append(x[i] + x[i + 1])
                i += 2
            elif x[i] not in have and len(x[i]) > 3 and x[i][1:] in have:
                out.append(x[i][1:])          # "tPHL" where the other text sets "t PHL"
                i += 1
            else:
                out.append(x[i])
                i += 1
        return out
    return join(a, b), join(b, a)


def matched_stretch(mine, theirs, slack=None):
    """The stretch of a source sentence (a token list) that a file sentence
    matches: the window, a half longer than the file's sentence, that holds
    the most of its words. The PDF's "sentences" run notes and headings
    together, so a "not" outside that stretch is another sentence's."""
    size = len(mine) + (max(3, len(mine) // 2) if slack is None else slack)
    want = collections.Counter(mine)
    best, top = theirs, -1
    for start in range(len(theirs) - size + 1):
        win = theirs[start:start + size]
        hit = sum((want & collections.Counter(win)).values())
        while win and win[0] not in want:
            win = win[1:]
        while win and win[-1] not in want:
            win = win[:-1]
        if hit > top or (hit == top and len(win) < len(best)):
            best, top = win, hit
    return best


def in_layout(src, toks):
    """Whether a sentence is printed, in order, on a few lines of the page as
    laid out: for a page whose reading-order text is scrambled (justified
    type set one word to a line, columns run together)."""
    split = [t[1:] if re.fullmatch(r"t[a-z]{2,3}", t) else t for t in toks]      # "t PHL" for tPHL
    for words in ([toks] if split == toks else [toks, split]):
        score, lid = src.best_lines(words)
        if lid is None or score < PROSE_MIN:
            continue
        page, seq = src.lines[lid][0], []
        for k in range(lid - 4, lid + 5):
            if 0 <= k < len(src.lines) and src.lines[k][0] == page:
                seq += content(src.lines[k][2])
        if lcs(*same_joins(words, seq)) / len(words) >= PROSE_ORDER:
            return True
    return False


def across_pages(src, toks):
    """Whether a sentence is printed, in order, over a page break: the end of
    one page and the start of the next, with the footer and header between."""
    if not hasattr(src, "breaks"):
        src.breaks = []
        first = {}
        for k, x in enumerate(src.sents):
            first.setdefault(x[0], k)
        for page, k in sorted(first.items()):
            if page - 1 in first:
                seq = [t for j in range(max(first[page - 1], k - 8), min(len(src.sents), k + 10))
                       if src.sents[j][0] in (page - 1, page) for t in src.sents[j][3]]
                full = []
                for i, t in enumerate(seq):
                    full.append(t)
                    if t in src.sections and i and seq[i - 1].startswith("section"):
                        full += src.sections[t]
                src.breaks.append(full)
    want = set(toks)
    for seq in src.breaks:
        if len(want & set(seq)) / len(want) >= 0.9 and lcs(*same_joins(toks, seq)) / len(toks) >= 0.9:
            return True
    return False


def check_prose(doc, src, rep):
    total = ok = 0
    kept = {}            # source sentence -> line of the file sentence that matched it
    before = None        # (line, source sentence, printed once, layout line) of the last sentence matched
    indent = lambda t: len(t) - len(t.lstrip())
    # where the source's "Detailed Description" and "Application and Implementation" chapters start
    last = lambda pat: max((k for k, x in enumerate(src.sents) if re.search(pat, x[1])), default=None)
    detail, app = last(r"\b\d+ Detailed Description\b"), last(r"\b\d+ Application and Implementation\b")
    heads = [(k + 1, m.group(1)) for k, x in enumerate(doc.text.split("\n")) for m in [re.match(r"^## (.+)$", x)] if m]
    section = lambda ln: next((t for k, t in reversed(heads) if k <= ln), "")
    for line, text in doc.units:
        if ISSUE_RE.search(text):
            continue          # the converter's own note on a source problem
        text = text.replace("#none#", " ")          # a broken cross-reference the source prints; not a word
        toks = content(tokens(norm(text)))
        if len(set(toks)) < 3:
            continue
        # "tPHL" where the text layer sets the subscript apart, "t PHL"
        toks = [t[1:] if t not in src.vocab and len(t) > 3 and t[1:] in src.vocab else t for t in toks]
        total += 1
        if all(t in doc.known for t in toks):
            ok += 1
            continue          # quoted under From images: read from a picture, checked by a person
        if (line, text) in doc.row_text:
            # a table cell: the PDF sets it in sub-columns and wrapped lines, so it is matched by place
            if not src.micro:
                toks = [t.replace("μ", "m") for t in toks]      # a text layer without µ prints it as m
            score, lid = src.best_lines(toks)
            if score < PROSE_MIN:
                rep.add("soft", "prose", line, "cell text is not printed in one place of the source (%.0f%% of its words "
                        "are within five lines of the closest place%s). Text: \"%s\""
                        % (100 * score, ", p.%d \"%s\"" % (src.lines[lid][0], clip(src.lines[lid][1], 80)) if lid is not None else "",
                           clip(text, 90)))
            else:
                ok += 1
            continue
        score, sid = src.best_passage(toks)
        near = ""
        if sid is not None:
            near = " Closest: p.%d \"%s\"" % (src.sents[sid][0], clip(src.sents[sid][1], 140))
        if score < PROSE_MIN and (in_layout(src, toks) or across_pages(src, toks)):
            ok += 1
            lid = src.best_lines(toks)[1]
            if lid is not None:
                doc.where[(line, text)] = ("page", src.lines[lid][0])
            continue
        if score < PROSE_MIN:
            what = "not found in" if score < PROSE_LOW else "reworded from"
            rep.add("soft", "prose", line, "%s the source (%.0f%% of its words are in the closest passage): use the "
                    "source's words, or mark the converter's own remark \"Conversion note:\".%s Text: \"%s\""
                    % (what, 100 * score, near, clip(text, 90)))
            continue
        ok += 1
        kept[sid] = line
        if (line, text) not in doc.row_text and (line, text) not in doc.pin_units and len(toks) >= 3:
            # the sentence before it in the file comes before it in the source: both printed once, in one
            # column of one page (two columns run together in the text, so the layout has to agree)
            good = [k for k in src.rivals(toks, sid) if k == sid or lcs(*same_joins(toks, src.run(k))) / len(toks) >= PROSE_ORDER]
            once = len(good) == 1
            lid = src.best_lines(toks)[1]
            if before and once and before[2] and src.sents[sid][0] > 1 and 0 <= line - before[0] <= 3 and src.sents[before[1]][0] == src.sents[sid][0] \
                    and 0 < before[1] - sid <= 12 and section(line) == section(before[0]) \
                    and lid is not None and before[3] is not None and 0 < before[3] - lid <= 10 \
                    and abs(indent(src.lines[lid][1]) - indent(src.lines[before[3]][1])) <= 8:
                rep.add("soft", "prose", line, "in the source this sentence comes before the one above it (p.%d): sentences "
                        "keep the source's order. Text: \"%s\"" % (src.sents[sid][0], clip(text, 80)))
            before = (line, sid, once, lid)
            # a semicolon where the source ends a sentence joins two sentences
            if ";" in text:
                a, b = text.split(";", 1)
                ta, tb = content(tokens(norm(a))), content(tokens(norm(b)))
                run = " ".join(src.sents[k][1] for k in (sid - 1, sid, sid + 1) if 0 <= k < len(src.sents))
                if len(ta) >= 4 and len(tb) >= 4 and ";" not in run and ":" not in a[-3:] and not across_pages(src, toks):
                    rep.add("soft", "prose", line, "two source sentences are joined with a semicolon the source doesn't have: "
                            "keep them apart. Text: \"%s\"" % clip(text, 100))
        window, ordered = set(), []
        for k in (sid - 1, sid, sid + 1):
            if 0 <= k < len(src.sents) and src.sents[k][0] == src.sents[sid][0]:
                window |= src.sents[k][2]
                ordered += src.sents[k][3]
        if (line, text) not in doc.pin_units:
            seq = 0
            for k in src.rivals(toks, sid):
                theirs = src.run(k)
                got = lcs(*same_joins(toks, theirs)) / len(toks)
                if got > seq:
                    seq, best_sid = got, k
                if seq >= PROSE_ORDER:
                    break
            if seq >= PROSE_ORDER and best_sid != sid:
                sid = best_sid
                near = " Closest: p.%d \"%s\"" % (src.sents[sid][0], clip(src.sents[sid][1], 140))
                window = set(src.run(sid))
            if seq < PROSE_ORDER and (in_layout(src, toks) or across_pages(src, toks)):
                doc.where[(line, text)] = ("page", src.sents[sid][0])
                continue
            if seq < PROSE_ORDER:
                rep.add("soft", "prose", line, "reworded: only %.0f%% of its words follow the order of the source passage "
                        "(joined, reordered or added words): write the source's sentences, shortened only by deleting.%s "
                        "Text: \"%s\"" % (100 * seq, near, clip(text, 90)))
                continue
        doc.where[(line, text)] = ("sent", [sid] + [k for k in src.rivals(toks, sid)[1:]
                                                  if lcs(*same_joins(toks, src.run(k))) / len(toks) >= PROSE_ORDER])
        if detail is not None and app is not None and detail < app and (line, text) not in doc.row_text:
            sec = section(line)
            inside = lambda k: detail < k < app
            if sec in ("Application information", "Functional description") and len(toks) >= 6:
                wrong = inside(sid) if sec == "Application information" else sid > app
                if wrong and not any((inside(k) if sec == "Functional description" else k > app)
                                     and lcs(*same_joins(toks, src.run(k))) / len(toks) >= PROSE_ORDER
                                     for k in src.rivals(toks, sid)[1:]):
                    rep.add("soft", "prose", line, "this sentence is in %s, and the source has it in its %s chapter (p.%d): "
                            "the source's Detailed Description (overview, feature description, functional modes) goes in "
                            "Functional description, its Application and Implementation chapter in Application information. "
                            "Text: \"%s\"" % (sec, "Detailed Description" if sec == "Application information"
                                              else "Application and Implementation", src.sents[sid][0], clip(text, 80)))
        lost = [t for t in toks if is_num(t) and t not in window and t not in doc.derived
                and not (t.isdigit() and int(t) < 10)]
        if lost:
            wide = set()
            for k in range(sid - 3, sid + 4):
                if 0 <= k < len(src.sents) and src.sents[k][0] == src.sents[sid][0]:
                    wide |= src.sents[k][2]
            lost = [t for t in lost if t not in wide]
            if lost:
                rep.add("soft", "prose", line, "%s is not in the source passage this sentence matches.%s"
                        % (", ".join(sorted(set(lost))), near))
        mine_list = tokens(norm(text))
        mine = set(mine_list)
        theirs = matched_stretch(mine_list, tokens(norm(src.sents[sid][1])))
        if (line, text) in doc.row_text:          # a table cell: the row's other cells may hold the "no"
            around = set(tokens(norm(doc.row_text[(line, text)])))
            mine_list = mine_list + [t for t in around if t in NEG or t in OBLIGE]
            mine = set(mine_list)
        mc, tc = set(content(mine_list)), set(content(theirs))
        if score < 0.75 or not tc or len(mc & tc) / len(tc) < 0.5:
            continue
        count = lambda words, toks: sum(t in words for t in toks)
        tight = matched_stretch(mine_list, theirs, 2)      # list items printed without full stops run together
        # a sentence the page layout breaks in two: its stretch in the run of three sentences
        run3 = [t for k in (sid - 1, sid, sid + 1) if 0 <= k < len(src.sents) for t in tokens(norm(src.sents[k][1]))]
        broken = sid > 0 and src.sents[sid - 1][0] == src.sents[sid][0] and not re.search(r"[.!?:;)]\s*$", src.sents[sid - 1][1])
        wide = matched_stretch(mine_list, run3, 2) if broken or len(theirs) < 0.8 * len(mine_list) else tight
        if count(NEG, mine_list) != count(NEG, theirs) and count(NEG, mine_list) != count(NEG, tight) \
                and count(NEG, mine_list) != count(NEG, wide):
            rep.add("soft", "prose", line, "negation differs: the source passage has %d of not/no/never, this sentence %d.%s"
                    % (count(NEG, theirs), count(NEG, mine_list), near))
        elif bool(mine & OBLIGE) != bool(set(theirs) & OBLIGE) and bool(mine & OBLIGE) != bool(set(tight) & OBLIGE) \
                and bool(mine & OBLIGE) != bool(set(wide) & OBLIGE):
            which = "the source makes it a requirement" if set(theirs) & OBLIGE else "the source doesn't say must"
            rep.add("soft", "prose", line, "obligation differs: %s.%s" % (which, near))
    rep.stats["prose supported"] = (ok, total)
    standing = "not part of the TI component specification"
    app_line = next((k for k, t in heads if t == "Application information"), None)
    if app_line and any(standing in re.sub(r"\s+", " ", x[1]) for x in src.sents) \
            and standing not in re.sub(r"\s+", " ", doc.text):
        rep.add("error", "prose", app_line, "the source's application chapter opens with the note that it is \"%s\": "
                "Application information opens with that sentence" % standing)
    # a requirement of the source that sits between two sentences the file keeps
    mine = [set(content(tokens(norm(t)))) for _, t in doc.units] + [set(content(tokens(norm(t)))) for _, t in doc.terms]
    req = re.compile(r"\b(do not|must|shall|never|ensure|cannot|should not|required|can cause damage|may cause damage)\b", re.I)
    order = sorted(kept)
    for a, b in zip(order, order[1:]):
        if b - a < 2 or b - a > 8 or src.sents[a][0] != src.sents[b][0] or abs(kept[b] - kept[a]) > 12:
            continue
        for k in range(a + 1, b):
            page, text, toks, seq = src.sents[k]
            if len(toks) < 5 or len(seq) > 60 or not req.search(text) or re.match(r"^\s*\d+(\.\d+)+\s", text):
                continue
            if max((len(toks & u) / len(toks) for u in mine), default=0) >= PROSE_MIN:
                continue
            rep.add("soft", "prose", kept[a], "the source has a requirement between this sentence and the next that the file "
                    "leaves out (p.%d): \"%s\"" % (page, clip(text, 200)))


def source_notes(src, pages):
    """Table notes in the PDF: '[n] text' lines and numbered lines after 'Notes:'."""
    notes, cur, in_notes, gap = [], None, False, 0
    for lid, (page, text, _) in enumerate(src.lines):
        if pages and page not in pages:
            continue
        s = text.strip()
        m = (re.match(r"^\[(\d+)\]\s+(\S.{8,})", s)                                # [1] text
             or re.match(r"^\((\d+)\)\s+([A-Z] = \S.{8,})", s)                          # (1) H = high level, under a centred table
             or (text.startswith(s) or len(text) - len(text.lstrip()) < 12) and (
                 re.match(r"^\((\d+)\)\s+([A-Z].{12,})", s)                               # (1) Text, at the left margin
                 or re.match(r"^()[†‡§¶]\s*([A-Z].{12,})", s))                               # † Text
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
    taken = set()
    raw_lines = doc.text.split("\n")
    for page, text in source_notes(src, doc.converted_pages):
        if re.match(r"^\s*(see|refer to)\b", text, re.I) and len(text) < 120:
            continue          # a cross-reference, not a fact
        if re.match(r"^\s*(MIN|TYP|NOM|MAX)\b", text):
            continue          # a table's header line with the caption's note mark in its first cell
        text = re.split(r"\s+(?:Figure|Table) \d", text)[0]
        text = re.sub(r"(\s\(\d+\)){2,}\s.*$", "", text)       # "(1) (1) tPHL tPLH": labels of the figure below it
        text = re.split(r"\s+(?:Input|Output)\s+\d+\s?%|\s+LOAD CIRCUIT\b|\s+VOLTAGE WAVEFORMS\b", text)[0]
        text = re.sub(r"(\s+t[A-Z]{2,3}\s*\(\d+\))+\s*$", "", text)          # "tPZL (2) tPLZ (1)": arrow labels
        if re.search(r"page numbers for previous revisions|this symbol is in accordance with|"
                     r"for more information about traditional and new thermal metrics|"
                     r"\bI ?= ?Input\b|^Signal Types|package option addendum|www\.ti\.com|"
                     r"for all available packages|component qualification in accordance with", text, re.I):
            continue          # revision-history, logic-symbol and thermal-report boilerplate
        toks = set(content(tokens(norm(text))))
        if len(toks) < 3:
            continue
        total += 1
        weight = {t: idf(t) for t in toks}
        tot = sum(weight.values())
        # each note of the source has a sentence of its own in the file: a note printed under two
        # tables is under both in the file
        scored = sorted(((sum(weight[t] for t in toks & u) / tot, -k) for k, u in enumerate(md_units)), reverse=True)
        best, k = scored[0] if scored else (0, 0)
        free = next(((sc, -n) for sc, n in scored if -n not in taken and sc >= NOTE_FULL), None)
        lettered = re.match(r"^[A-H]\.\s", text)          # the lettered notes of a figure: those of a dropped waveform go
        in_table = lambda n: re.match(r"^\[\d+\]\s", raw_lines[doc.terms[n][0] - 1].strip()) if doc.terms[n][0] else None
        there = lambda n: norm(md_plain(raw_lines[doc.terms[n][0] - 1])).lower().replace(" ", "") if doc.terms[n][0] else ""
        missing = lambda n: sorted((t for t in toks - md_units[n] if t not in there(n) and not is_num(t)
                                    and len(src.sent_post.get(t, ())) > 1),
                                   key=lambda t: -weight[t])
        if best >= NOTE_FULL and free is None and not in_table(-k):
            free = (best, -k)                               # a sentence of the text stands for each printing
        if best < NOTE_MIN:
            rep.add("soft", "notes", 0, "source note on p.%d is not in the file: \"%s\"" % (page, clip(text, 120)))
        elif lettered and best < NOTE_FULL:
            ok += 1
        elif not lettered and missing(-k) and (best < NOTE_FULL or len(toks) <= 30):
            lost = missing(-k)
            rep.add("soft", "notes", doc.terms[-k][0], "the source note on p.%d says more than this (%s): a note is "
                    "kept whole. Source: \"%s\"" % (page, ", ".join(lost[:6]), clip(text, 160)))
        elif best < NOTE_FULL:
            ok += 1
        elif free is None:
            rep.add("soft", "notes", doc.terms[-k][0], "the source prints this note again on p.%d (under another table) "
                    "and the file has it once fewer: \"%s\"" % (page, clip(text, 100)))
        else:
            taken.add(free[1])
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
        rep.add("info", "pages", 0, "few of these pages' numbers are in the file: " + ", ".join(low))


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
                rep.add("error", "waveforms", line, 'node %s on %s (column %d) is not on a transition'
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
            rep.add("soft", "waveforms", line, 'edge label "%s" is not in the source PDF' % label)
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
            rep.add("manual", "visual", line, "%s %s: compare by eye with the source%s"
                    % (kind, attrs.get("id", ""), " (likely p.%d)" % page if page else ""))


# ---------------------------------------------------------------- positions

WORD_RE = re.compile(r'<word xMin="([\d.]+)" yMin="([\d.]+)" xMax="([\d.]+)" yMax="([\d.]+)">(.*?)</word>')
PNUM_RE = re.compile(r"(?<![A-Za-z0-9.])(?<![A-Za-z0-9][-/])([±+-]?)(\d+(?:\.\d+)?|\.\d+)(?!\d)(°[CFK](?:/[A-Za-z]{1,2})?|%|[^\W\d_]{0,5}(?:/[^\W\d_]{1,3})?)?")
HEAD_WORDS = {"min": "min", "typ": "typ", "nom": "nom", "max": "max", "minimum": "min", "typical": "typ",
              "maximum": "max", "nominal": "nom"}
HEAD_RE = re.compile(r"^(MIN|TYP|NOM|MAX|VALUE|Min|Typ|Nom|Max)\.?(?:\(\d+\)|[†‡§¶*#]|\d)?$")
LINE_TOL = 4.6         # words whose tops are this close share a line (subscripts sit 3.4 pt lower)
JOIN_GAP = 0.6         # words this close are one word (a symbol and its subscript)
SIGN_GAP = 7.0         # a lone dash this close before a number is its sign
REF_WORDS = {"note", "notes", "figure", "figures", "fig", "table", "section", "see", "page", "and", "through"}


def load_words(pdf, cache):
    """Per page, the words of the text layer with their boxes: [x0, y0, x1, y1, text]."""
    with open(pdf, "rb") as f:
        digest = hashlib.sha1(f.read()).hexdigest()
    path = os.path.join(cache, "%s-words.json" % digest)
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    out = run(["pdftotext", "-enc", "UTF-8", "-bbox-layout", pdf, "-"])
    pages = [[[float(a), float(b), float(c), float(d), html.unescape(t)] for a, b, c, d, t in WORD_RE.findall(chunk)]
             for chunk in out.split("<page ")[1:]]
    os.makedirs(cache, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(pages, f)
    return pages


def unit_norm(u):
    """A unit as compared: no spaces, lower case, µ as u, Ω as ohm, no degree sign."""
    u = unicodedata.normalize("NFKC", md_plain(u)).replace(" ", "").lower()
    u = u.replace("α", "a")       # a Greek alpha set for the A of µA
    u = re.sub(r"^oc(?=$|/)", "c", u)     # a degree sign set as a raised letter o
    return u.replace("μ", "u").replace("µ", "u").replace("ω", "ohm").replace("Ω", "ohm").replace("°", "")


UNIT_NORMS = {unit_norm(u) for u in UNITS}


class PLine:
    """One line of a PDF page: its words left to right, and the numbers in them."""
    __slots__ = ("page", "y", "words", "nums", "toks", "text", "index")

    def __init__(self, page, y, words):
        self.page, self.y = page, y
        merged = []
        for x0, x1, text in sorted(words):
            if merged and x0 - merged[-1][1] <= JOIN_GAP:
                merged[-1] = [merged[-1][0], max(x1, merged[-1][1]), merged[-1][2] + text]
            else:
                merged.append([x0, x1, text])
        self.words = merged
        self.text = " ".join(w[2] for w in merged)
        self.toks = set()
        self.nums = []          # (x0, x1, canonical number, sign, unit suffix, word index)
        sign, prev_num_x1 = None, None
        for wi, (x0, x1, text) in enumerate(merged):
            t = unicodedata.normalize("NFKC", text).translate(DASHES)
            self.toks.update(tokens(norm(text)))
            if wi and len(merged[wi - 1][2]) <= 2:
                self.toks.update(tokens(norm(merged[wi - 1][2] + text)))     # "R θJA" is RθJA
            if t in ("-", "±", "+"):
                ranged = t == "-" and prev_num_x1 is not None and x0 - prev_num_x1 <= SIGN_GAP
                sign = None if ranged else (t, x1)
                continue
            t = re.sub(r"\(\d{1,2}\)", "", t)
            t = re.sub(r"(\d),(\d{3})(?!\d)", r"\1\2", t)
            t = re.sub(r"^\.+(?=[±+\-]\d)|^\.{2,}(?=\d)", "", t)     # ". . .±20mA": leader dots run into the value
            first = True
            for m in PNUM_RE.finditer(t):
                s = m.group(1)
                if first and not s and m.start() == 0 and sign and x0 - sign[1] <= SIGN_GAP:
                    s = sign[0]
                if s == "-" and m.start() > 0 and t[m.start() - 1] in ")":
                    s = ""
                if not s and m.start() > 0 and t[m.start() - 1] in "\"'`^*":
                    s = None          # a glyph the text layer lost (± printed as a quote mark)
                first = False
                v = m.group(2)
                self.nums.append((x0, x1, canon_num("0" + v if v.startswith(".") else v),
                                  "" if s == "+" else s, m.group(3) or "", wi))
                if m.end() < len(t) and t[m.end():m.end() + 1] in "\"'`^*" and s == "":
                    self.nums[-1] = self.nums[-1][:3] + (None,) + self.nums[-1][4:]
            sign = None
            prev_num_x1 = x1 if re.search(r"\d\W*$", t) or re.search(r"\d[^\W\d_]{0,3}$", t) else None

    def ref_nums(self):
        """Word indexes of numbers that are references (Note 1, Figure 3), not values."""
        out = set()
        for wi, (_, _, text) in enumerate(self.words):
            if wi and re.sub(r"\W", "", self.words[wi - 1][2]).lower() in REF_WORDS:
                out.add(wi)
        return out


def scan_pages(pdf, cache):
    """Pages that are a scanned image (a picture of at least 1.5 megapixels):
    their text layer, if any, is OCR and can't be trusted for tables."""
    with open(pdf, "rb") as f:
        digest = hashlib.sha1(f.read()).hexdigest()
    path = os.path.join(cache, "%s-scans.json" % digest)
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    pages = set()
    if shutil.which("pdfimages"):
        for row in run(["pdfimages", "-list", pdf]).split("\n")[2:]:
            f = row.split()
            if len(f) > 4 and f[0].isdigit() and f[3].isdigit() and f[4].isdigit() and int(f[3]) * int(f[4]) >= 1500000:
                pages.add(int(f[0]))
    os.makedirs(cache, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        json.dump(sorted(pages), f)
    return sorted(pages)


class Positions:
    """The PDF's text layer by position: lines, numbers with their x ranges,
    and the Min/Typ/Max and Unit headers above them."""

    def __init__(self, pages):
        self.npages = len(pages)
        self.micro = any("µ" in w[4] or "μ" in w[4] for words in pages for w in words)
        # pages that print a µ: on the others a symbol-font µ may come out as m
        self.micro_pages = {k + 1 for k, words in enumerate(pages) if any("µ" in w[4] or "μ" in w[4] for w in words)}
        self.lines = []                 # every PLine, in page and y order
        self.page_lines = []            # page -> its PLines
        self.num_post = collections.defaultdict(list)       # number -> line indexes
        for pi, words in enumerate(pages):
            rows = []
            words = self.join_subscripts(words)
            for x0, y0, x1, y1, text in sorted(words, key=lambda w: (w[1], w[0])):
                if not text.strip():
                    continue
                if rows and y0 - rows[-1][0] <= LINE_TOL:
                    rows[-1][1].append((x0, x1, text))
                else:
                    rows.append([y0, [(x0, x1, text)]])
            pl = []
            for y, ws in rows:
                line = PLine(pi + 1, y, ws)
                line.index = len(self.lines)
                self.lines.append(line)
                pl.append(line)
                for n in {n[2] for n in line.nums}:
                    self.num_post[n].append(line.index)
            self.page_lines.append(pl)

    @staticmethod
    def join_subscripts(words):
        """Older PDFs set a subscript as a smaller word a few points lower
        ("I" then "CC"), too low to share its line. Join each to the word it
        hangs on, so I_CC reads "ICC" and doesn't start a line of its own."""
        words = [list(w) for w in words if w[4].strip()]
        if not words:
            return words
        words.sort(key=lambda w: (w[0], w[1]))
        by_end = collections.defaultdict(list)      # rounded right edge -> words ending there
        gone = set()
        for i, w in enumerate(words):
            if True:
                base = None
                for key in (round(w[0]) - 1, round(w[0]), round(w[0]) + 1):
                    for j in by_end.get(key, ()):
                        b = words[j]
                        if (j not in gone and abs(b[2] - w[0]) <= 1.5 and 1.5 <= w[1] - b[1] <= 7
                                and w[3] - w[1] <= 0.95 * (b[3] - b[1])):
                            base = j
                if base is not None:
                    b = words[base]
                    by_end[round(b[2])].remove(base)
                    b[2], b[4] = w[2], b[4] + w[4]
                    by_end[round(b[2])].append(base)
                    gone.add(i)
                    continue
            by_end[round(w[2])].append(i)
        return [w for i, w in enumerate(words) if i not in gone]

    def window(self, li, k):
        page = self.lines[li].page
        return [self.lines[j] for j in range(li - k, li + k + 1)
                if 0 <= j < len(self.lines) and self.lines[j].page == page]

    def candidates(self, vals, pages=None):
        """Lines that hold every one of `vals` (a list of canonical numbers)."""
        want = collections.Counter(vals)
        rare = min(want, key=lambda v: len(self.num_post.get(v, ())))
        out = []
        for li in self.num_post.get(rare, ()):
            line = self.lines[li]
            if pages and line.page not in pages:
                continue
            if line.words and line.words[0][2] in ("•", "–", "-"):
                continue          # a bullet of a list (the revision history quotes table values)
            have = collections.Counter(n[2] for n in line.nums)
            if all(have[v] >= c for v, c in want.items()):
                out.append(li)
        return out

    def heads_of(self, line):
        """The Min/Typ/Max header words on a line: [(label, x0, x1)]."""
        found = []
        for wi, (x0, x1, text) in enumerate(line.words):
            m = HEAD_RE.match(text)
            if text == "MA" and any(HEAD_RE.match(w[2]) for w in line.words):
                m = HEAD_RE.match("MAX")          # "MA" over "X": a header wrapped in a narrow column
            prev = line.words[wi - 1][2] if wi else ""
            nxt = line.words[wi + 1][2] if wi + 1 < len(line.words) else ""
            if m and prev not in ("to",) and not prev.endswith("=") and nxt != "to":
                found.append((m.group(1).lower(), x0, x1))
        if len(line.words) > 16:
            return []
        if any(not w[2].isupper() for w in line.words if HEAD_RE.match(w[2])) and len(found) < 2:
            return []         # "Max tpd of 3.6 ns": a sentence, not a header
        return found

    def in_columns(self, li, vals):
        """Whether every value has an occurrence right of the conditions (under the headers, when there are any)."""
        heads = self.header(li)[0]
        if not heads:
            return True
        gaps = sorted(b[1] - a[1] for a, b in zip(heads, heads[1:]))
        lo = heads[0][1] - (gaps[len(gaps) // 2] / 2 if gaps else 25.0)
        right = collections.Counter(n[2] for n in self.lines[li].nums if (n[0] + n[1]) / 2 >= lo)
        want = collections.Counter(vals)
        return all(right[v] >= c for v, c in want.items())

    def header(self, li):
        """The Min/Typ/Max header words above a line: [(label, x0, x1)], and the Unit header's x range."""
        line = self.lines[li]
        heads, unit = None, None
        for j in range(li - 1, -1, -1):
            up = self.lines[j]
            if up.page != line.page:
                break
            found = self.heads_of(up)
            if heads is None and not found and SECTION_START_RE.match(up.text.strip()):
                break             # another table's headers are above this heading or note
            for x0, x1, text in up.words:
                if (re.sub(r"[^A-Za-z]", "", text).lower() in ("unit", "units") or text == "UNI") and unit is None:
                    unit = (x0, x1, up.y)
            if found and heads is None:
                # "MA" over "X", set half a line above MIN and TYP: a MAX header wrapped in a narrow column
                for k in (j - 1, j - 2):
                    if 0 <= k and self.lines[k].page == line.page and 0 < up.y - self.lines[k].y < 8:
                        more = [("max", x0, x1) for x0, x1, text in self.lines[k].words if text == "MA"]
                        if len(self.lines[k].words) <= 6:       # "TYP‡" set two points above MIN and MAX
                            more += [(HEAD_RE.match(text).group(1).lower(), x0, x1) for x0, x1, text in self.lines[k].words
                                     if HEAD_RE.match(text) and text.isupper() is not False and text != "MA"]
                        found = sorted(found + more, key=lambda h: h[1])
                heads = (found, up.y)
                if unit and unit[2] - up.y > 30:
                    unit = None          # a Unit header of an earlier table
            if heads and (unit or heads[1] - up.y > 30):
                break
            if not heads and unit and unit[2] - up.y > 60:
                break
        return (heads[0] if heads else []), unit, (heads[1] if heads else None)

    @staticmethod
    def under(heads, x0, x1):
        """The header a number sits under, or None when it is under none."""
        if not heads:
            return None
        xc = (x0 + x1) / 2
        best, dist = None, 1e9
        for label, h0, h1 in heads:
            d = min(abs(x1 - h1), abs(xc - (h0 + h1) / 2), abs(x0 - h0))
            if d < dist:
                best, dist = label, d
        gaps = [b[1] - a[1] for a, b in zip(heads, heads[1:])]
        tol = max(10.0, 0.45 * min(gaps)) if gaps else 25.0
        return best if dist <= tol else None


SECTION_START_RE = re.compile(r"(absolute maximum ratings|recommended operating conditions|electrical characteristics|"
                              r"switching characteristics|timing requirements|operating characteristics|thermal information)\b"
                              r"|[†‡§¶]\s*[A-Z].{40,}", re.I)
VALUE_CELL_RE = re.compile(r"^[<>≤≥]?\s?([±+-]?)\s?(\d+(?:\.\d+)?)$")


def cell_numbers(cell, value):
    """(number, sign) for each number in a table cell. The sign is None where it can't be told."""
    if value:
        cell = re.sub(r"(\d)\^[-−–]?\d+\^", r"\1", cell)       # ±10^-5^: compared by its mantissa, as the text layer prints it
    plain = md_plain(MARK_RE.sub("", cell)).translate(DASHES).strip()
    m = VALUE_CELL_RE.match(plain) if value else None
    if m:
        return [(canon_num(m.group(2)), "" if m.group(1) == "+" else m.group(1))]
    plain = re.sub(r"(\d) (\d{3})(?!\d)", r"\1\2", plain)
    return [(canon_num(n), None) for n in re.findall(r"(?<![A-Za-z0-9.])\d+(?:\.\d+)?", plain)]


def check_cells(doc, pos, rep):
    """Compare each spec-table row with the PDF line that holds its values:
    which column each value is under, its sign, the row's unit, symbol and
    conditions, and the PDF rows between two file rows."""
    located = total = 0
    doc.located, doc.loose, doc.columns, doc.strays, doc.row_vals, doc.label_rows = set(), [], {}, [], [], []
    for t in doc.tables:
        if not t["vcols"] or not (t["spec"] or "values" in t["attrs"] or t["section"] == "Thermal information"):
            continue
        cols, vcols = t["cols"], t["vcols"]
        ucol = cols.index("unit") if "unit" in cols else None
        scol = cols.index("symbol") if "symbol" in cols else None
        # numbers the table states once for all rows
        common, heading = set(), set()
        texts = list(t["notes"])
        banners = [t["attrs"].get("caption", ""), t["attrs"].get("conditions", "")] + [c for h in t["header"] for c in h]
        rows = []
        above = {}
        for ln, cells in t["raw"]:
            if len(cells) > 1 and all(c == "<" for c in cells[1:]):
                banners.append(cells[0])
                continue
            full = []
            for n, c in enumerate(cells):
                if c == "^":
                    c = above.get(n, "")
                above[n] = c
                full.append("" if c == "<" else c)
            vals = []
            for c in vcols:
                if c < len(full):
                    vals += [(c, v, s) for v, s in cell_numbers(full[c], True)]
            # "3.8¹": a note mark the text layer glues to its value, 3.81
            glued = []
            for c in vcols:
                m = re.fullmatch(r"\s*([±+-]?)(\d+(?:\.\d+)?)\s*\[(\d)\]\s*", full[c]) if c < len(full) else None
                glued += [(c, canon_num(m.group(2) + m.group(3)), "" if m.group(1) == "+" else m.group(1))] if m else \
                    [(c, v, sg) for cc, v, sg in vals if cc == c]
            if glued != vals and vals and not pos.candidates([v for _, v, _ in vals]) \
                    and pos.candidates([v for _, v, _ in glued]):
                vals = glued
            other = [full[c] for c in range(len(full)) if c not in vcols]
            words = [x for x in tokens(norm(md_plain(" ".join(other)))) if x not in STOP]
            if set(MARK_RE.findall(" ".join(cells))) & t["own_notes"]:
                doc.placed.add(ln)        # a cell corrected or doubted under the converter's note: read by a person
            rows.append({"line": ln, "raw": cells, "full": full, "vals": vals, "words": words,
                         "nums": {v for c in full for v, _ in cell_numbers(c, False)}})
        for x in banners:
            heading.update(v for v, _ in cell_numbers(x, False))
        for x in texts + banners:
            common.update(v for v, _ in cell_numbers(x, False))
        symbols = {}
        if scol is not None:
            for r in rows:
                key = "".join(tokens(norm(md_plain(MARK_RE.sub("", r["full"][scol])))))
                r["symbol"] = key
                if key:
                    symbols[key] = r["full"][scol]
        # candidates, and the best run of rows in page order
        for r in rows:
            r["cand"] = []
            r["allvals"] = [v for _, v, _ in r["vals"]]
            if not any(sign is not None for _, _, sign in r["vals"]):
                r["vals"] = []        # only expressions (VCC - 0.1): nothing to place by
            if not r["vals"]:
                continue
            total += 1
            pure = [v for _, v, sign in r["vals"] if sign is not None]
            cand = [li for li in pos.candidates(pure, doc.converted_pages)
                    if pos.lines[li].page not in doc.scans and pos.in_columns(li, pure)][:150]
            for li in cand:
                near = set().union(*(x.toks for x in pos.window(li, 1)))
                far = set().union(*(x.toks for x in pos.window(li, 4)))
                w = r["words"]
                own = pos.lines[li].toks
                score = (sum(x in own for x in w) + 0.6 * sum(x in near for x in w)
                         + 0.3 * sum(x in far for x in w)) / (1.9 * len(w)) if w else 0.5
                extra = sum(1 for n in pos.lines[li].nums if n[2] not in r["nums"] and n[2] not in common)
                r["cand"].append((li, score - min(0.6, 0.15 * extra)))
        best = {}       # (row index, cand index) -> (total score, back pointer)
        order = [i for i, r in enumerate(rows) if r["cand"]]
        for n, i in enumerate(order):
            for k, (li, score) in enumerate(rows[i]["cand"]):
                top, back = 0.0, None
                here = pos.lines[li]
                for m in range(n):
                    j = order[m]
                    for k2, (li2, _) in enumerate(rows[j]["cand"]):
                        if li2 >= li:
                            continue
                        there = pos.lines[li2]
                        close = j == i - 1 and there.page == here.page and here.y - there.y <= 45
                        if best[(j, k2)][0] + 0.5 * close > top:
                            top, back = best[(j, k2)][0] + 0.5 * close, (j, k2)
                best[(i, k)] = (top + 1 + score, back)
        at = max(best, key=lambda key: best[key][0]) if best else None
        chain = {}
        while at:
            chain[at[0]] = rows[at[0]]["cand"][at[1]][0]
            at = best[at][1]
        gaps = sorted(pos.lines[chain[b]].y - pos.lines[chain[a]].y for a, b in zip(sorted(chain), sorted(chain)[1:])
                      if pos.lines[chain[a]].page == pos.lines[chain[b]].page and chain[a] != chain[b])
        half = max(4.0, min(10.0, gaps[len(gaps) // 2] / 2)) if gaps else 6.0
        groups = collections.defaultdict(list)
        for li in chain.values():
            groups[(pos.lines[li].page, pos.header(li)[2])].append(li)
        labels = {}
        for group, lis in groups.items():
            plain = not pos.header(lis[0])[0]
            found, cols = column_labels(pos, lis, {chain[i]: set(rows[i]["allvals"]) for i in chain} if plain else None)
            labels.update(found)
            if cols:
                old = doc.columns.get(group)
                doc.columns[group] = (cols[0], (old[1] if old else []) + cols[1])
        labels["columns"] = doc.columns
        labels["strays"] = doc.strays
        labels["known"] = doc.known
        left = {}
        for li in chain.values():
            page = pos.lines[li].page
            left[page] = min(left.get(page, 1e9), pos.lines[li].words[0][0])
        doc.located.update(chain.values())
        doc.row_vals += [collections.Counter(r["allvals"]) for r in rows if r["allvals"]]
        doc.loose += [collections.Counter(r["allvals"]) for i, r in enumerate(rows) if i not in chain and r["allvals"]]
        for i, r in enumerate(rows):
            if not r["vals"]:
                continue
            if i not in chain:
                if between(rep, t, rows, i, chain, pos, labels, half):
                    continue
                if r["cand"]:
                    li = max(r["cand"], key=lambda c: c[1])[0]       # out of order: line checks only
                    located += 1
                    check_line(rep, t, r, pos, li, None, None, {"columns": doc.columns, "strays": doc.strays})
                elif len(r["vals"]) >= 2 and t["spec"]:
                    if not report_scatter(rep, t, r, pos, doc) and not rep.scanned:
                        explain_loose(rep, t, r, pos, doc)
                continue
            located += 1
            li = chain[i]
            line = pos.lines[li]
            heads, unit, head_y = pos.header(li)
            lo, hi = line.y - half, line.y + half
            if i - 1 in chain and pos.lines[chain[i - 1]].page == line.page and line.y - pos.lines[chain[i - 1]].y < 90:
                lo = (pos.lines[chain[i - 1]].y + line.y) / 2
            elif i == 0 and head_y is not None and line.y - head_y < 90:
                lo = head_y + 2
            if i + 1 in chain and pos.lines[chain[i + 1]].page == line.page and pos.lines[chain[i + 1]].y - line.y < 90:
                hi = (pos.lines[chain[i + 1]].y + line.y) / 2
            elif i == len(rows) - 1:
                hi = line.y + 5 * half        # the table's last row: look two rows further for dropped ones
                for x in pos.page_lines[line.page - 1]:
                    if line.y < x.y <= hi and (re.match(r"^(\(\d+\)|\[\d+\]|[†‡§*]|notes?\b|\d+(\.\d+)+ [A-Z])", x.text, re.I)
                                               or pos.header(x.index)[2] != head_y):
                        hi = x.y - 1          # a footnote, a heading or the next table
                        break
            band = [x for x in pos.page_lines[line.page - 1]
                    if x is line or (lo < x.y <= hi and not FOOTER_RE.search(x.text))]
            # how many rows below share this row's merged cells (a name centred on a tall group)
            doc.placed.add(r["line"])
            r["span"] = 0
            for later in rows[i + 1:]:
                if "^" not in later["raw"]:
                    break
                r["span"] += 1
            check_line(rep, t, r, pos, li, band, (symbols, (common, heading), ucol, scol,
                                                   rows[i - 1] if i - 1 in chain else None,
                                                   rows[i + 1] if i + 1 in chain else None,
                                                   left[line.page], half), labels)
        check_labels(rep, t, rows, chain, pos, doc, half, left)
        check_header_words(rep, t, chain, pos, doc)
    for ln, x, word in doc.label_rows:
        if x.index not in doc.located:      # not a row another table of the file was placed on
            rep.add("error", "coverage", ln, "the source table has a row starting \"%s\" that the file's table "
                    "does not have: p.%d \"%s\"" % (word, x.page, clip(x.text, 90)))
    seen = set()
    for ln, x, what, own, v in doc.strays:
        # a line another table of the file was placed on is that table's row (the source lists
        # thermal values among the absolute maximum ratings; the file gives them their own table)
        if (x.index, what) in seen or (not own and x.index in doc.located) or v in doc.issue_nums:
            continue
        if not own and any(v in loose for loose in doc.loose):
            continue          # a value of a file row that has no single line (one joined from several)
        seen.add((x.index, what))
        rep.add("error", "cells", ln, "the source has %s %s, and the file's row does not: a dropped %s? p.%d \"%s\""
                % (what, "on this row" if own else "on a line next to this row", "value" if own else "row",
                   x.page, clip(x.text, 90)))
    rep.stats["rows located"] = (located, total)


FOOTER_RE = re.compile(r"Copyright|Submit Document Feedback|POST OFFICE BOX|www\.ti\.com", re.I)
TABLE_END_RE = re.compile(r"^(\(\d+\)|\[\d+\]|[†‡§¶*•]|notes?\b|caution\b|(?:static|dynamic) electrical|typical performance|\d+(\.\d+)+ [A-Z]|absolute maximum|recommended operating|"
                          r"electrical characteristics|switching characteristics|operating characteristics|timing requirements|"
                          r"noise characteristics|thermal information|esd ratings|parameter measurement)|Copyright|Submit Document Feedback|"
                          r"POST OFFICE BOX|www\.ti\.com", re.I)


def check_coverage(doc, pos, rep):
    """Every row of a source table with Min/Typ/Max headers, on a converted
    page, is in the file. A row is a line with numbers standing in the
    table's value columns (the columns the file's rows were found in). A
    table none of whose rows is in the file is reported once."""
    pages = (doc.converted_pages or set(range(1, pos.npages + 1))) - doc.scans
    total = ok = 0
    for p in sorted(pages):
        if p > pos.npages:
            continue
        heads, head_y, last_y, orphan = None, None, None, []

        def flush():
            # a table with no row in the file: its lines must share value columns to be a table
            rows = [(x, vals) for x, vals in orphan
                    if any(abs(n[1] - m[1]) <= 1.5 or abs(n[0] - m[0]) <= 1.5
                           for y, other in orphan if y is not x for n in vals for m in other)]
            # a table the source prints twice with the same values: the file's rows already hold it
            twice = all(any(not collections.Counter(n[2] for n in vals) - used for used in doc.row_vals)
                        for _, vals in rows)
            if len(rows) >= 2 and not twice:
                rep.add("error", "coverage", 0, "a source table is not in the file: p.%d, %d rows from \"%s\" to \"%s\""
                        % (p, len(rows), clip(rows[0][0].text, 60), clip(rows[-1][0].text, 60)))
            orphan.clear()

        for x in pos.page_lines[p - 1]:
            own = pos.heads_of(x)
            if own:
                flush()
                heads, head_y, last_y = own, x.y, x.y
                continue
            if heads is None:
                continue
            if TABLE_END_RE.search(x.text) or x.y - last_y > 42:
                flush()
                heads = None
                continue
            cols = doc.columns.get((p, head_y))
            refs = x.ref_nums()
            if cols:
                vals = [n for n in x.nums if n[5] not in refs and column_of(cols, n)]
            else:
                vals = [n for n in x.nums if n[5] not in refs and pos.under(heads, n[0], n[1])]
            if not vals:
                continue
            last_y = x.y
            prev = pos.lines[x.index - 1] if x.index and pos.lines[x.index - 1].page == p else None
            if prev is not None and x.y - prev.y < 14 and any(re.search(r"\d\.$|[-–]0\.?$", w[2]) for w in prev.words):
                continue          # the rest of a number the line above breaks ("VCC-0." / "1")
            if all(n[2] in doc.issue_nums for n in vals):
                continue          # quoted in a Source error / Unconfirmed remark
            have = collections.Counter(n[2] for n in vals)
            if x.index in doc.located or any(not have - loose for loose in doc.loose):
                total, ok = total + 1, ok + 1
                continue
            if not cols:
                orphan.append((x, vals))
                continue
            total += 1
            rep.add("error", "coverage", 0, "a source table row is not in the file: p.%d \"%s\" (%s)"
                    % (p, clip(x.text, 100), ", ".join("%s%s under %s" % (n[3] or "", n[2], column_of(cols, n).upper())
                                                         for n in vals)))
        flush()
    rep.stats["source rows covered"] = (ok, total)


def check_function_tables(doc, src, rep):
    """A function-table row (H, L, X cells) is printed as such a row in the source."""
    letters = {"h", "l", "x", "z"}
    runs = set()
    for _, text, _ in src.lines:
        words = [w.lower() for w in text.split()]
        run = []
        for w in words + [""]:
            if w in letters:
                run.append(w)
            else:
                for a in range(len(run)):
                    for b in range(a + 2, len(run) + 1):
                        runs.add(tuple(run[a:b]))
                run = []
    # INPUTS / OUTPUT printed over the columns of the source's function table
    groups = set()
    for lid, (_, text, _) in enumerate(src.lines):
        if re.search(r"\bINPUTS?\b", text):
            top = " ".join(x[1] for x in src.lines[lid:lid + 3])
            below = " ".join(x[1] for x in src.lines[lid + 1:lid + 8])
            if re.search(r"\bOUTPUTS?\b", top) and re.search(r"(?<!\w)[HL]\s+[HLX]\s", below):
                groups = {"input", "output"}
    for t in doc.tables:
        if t["section"] != "Function tables":
            continue
        head = {w.rstrip("s") for cells in t["header"] for c in cells for w in re.findall(r"[a-z]+", md_plain(c).lower())}
        if groups and t["rows"] and all(md_plain(c).strip().lower() in letters for c in t["rows"][0][1]) \
                and not groups <= head:
            rep.add("soft", "cells", t["line"] + 1, "the source's function table has the header row INPUTS / OUTPUT over "
                    "its columns: keep it (| Inputs | < | Output |), with the note markers it carries")
        elif not groups and rep.scanned and len(t["header"]) < 2:
            rep.add("manual", "cells", t["line"] + 1, "function table: if the scan prints INPUTS / OUTPUT over the columns, "
                    "the file keeps that header row")
        for ln, cells in t["rows"]:
            row = tuple(md_plain(c).strip().lower() for c in cells)
            if len(row) >= 2 and all(c in letters for c in row) and row not in runs:
                rep.add("soft", "rows", ln, "function-table row %s is not a row of the source" % " ".join(c.upper() for c in row))


def between(rep, t, rows, i, chain, pos, labels, half):
    """A row with no line of its own, between rows that have one: compare it
    with the source rows that sit between its neighbours. True when reported."""
    r = rows[i]
    a = max((j for j in chain if j < i), default=None)
    b = min((j for j in chain if j > i), default=None)
    if a is None and b is None:
        return False
    la = pos.lines[chain[a]] if a is not None else None
    lb = pos.lines[chain[b]] if b is not None else None
    if la and lb and la.page != lb.page:
        return False
    ref = la or lb
    lo = la.y if la else lb.y - 6 * half * (b - i)
    hi = lb.y if lb else la.y + 6 * half * (i - a)
    heads = pos.header(ref.index)[0]
    cols = labels.get("columns", {}).get((ref.page, pos.header(ref.index)[2]))
    under = lambda x, n: labels[(x.index, n[5])] if (x.index, n[5]) in labels else (
        column_of(cols, n) if cols else pos.under(heads, n[0], n[1]))
    lines = []
    for x in pos.page_lines[ref.page - 1]:
        if lo + 1 < x.y < hi - 1:
            vals = [n for n in x.nums if under(x, n)]
            if vals:
                lines.append((x, vals))
    mine = collections.Counter(v for _, v, sign in r["vals"] if sign is not None)
    if not lines:
        if la and lb:
            rep.add("error", "cells", r["line"], "the source has no row with values between the rows before and after this one (p.%d)" % ref.page)
            return True
        return False
    gap = [j for j in range((a if a is not None else -1) + 1, b if b is not None else len(rows)) if rows[j]["vals"]]
    if la and lb and len(gap) == len(lines):
        x, vals = lines[gap.index(i)]            # as many rows as lines: they pair off in order
    else:
        x, vals = max(lines, key=lambda lv: sum((mine & collections.Counter(n[2] for n in lv[1])).values()))
    theirs = collections.Counter(n[2] for n in vals)
    if mine == theirs or not (mine & theirs) and not (la and lb and len(gap) == len(lines)):
        return False
    if not theirs - mine:
        # every source value is in the file's row; the rest may come from the same
        # row of another source table (a row joined from one table per device)
        def elsewhere(v):
            for li in pos.num_post.get(v, ()):
                h = pos.header(li)[0]
                got = collections.Counter(n[2] for n in pos.lines[li].nums if pos.under(h, n[0], n[1]))
                if got and not got - mine:
                    return True
            return False
        if all(elsewhere(v) for v in (mine - theirs)):
            return False
    if set(MARK_RE.findall(" ".join(r["raw"]))) & t["own_notes"]:
        return False          # a cell corrected under a Source error note: the note says what the source prints
    rep.add("error", "cells", r["line"], "this row has %s where the source row has %s. p.%d \"%s\""
            % (", ".join(sorted((mine - theirs).elements())) or "nothing",
               ", ".join("%s%s" % (n[3] or "", n[2]) for n in vals if (theirs - mine)[n[2]]) or "nothing",
               x.page, clip(x.text, 90)))
    return True


def column_labels(pos, lis, values=None):
    """Which Min/Typ/Max header each number of these lines is under:
    {(line index, word index): label}. The numbers are grouped into columns
    by their shared left edge, centre or right edge; when there are as many
    columns as headers they pair off in order (headers are not always
    centred over their values), otherwise each column takes the nearest
    header."""
    heads, unit, _ = pos.header(lis[0])
    if not heads or values:
        # no Min/Typ/Max header (one column per package): the columns are where the file's values stand
        atoms = [(li, n) for li in lis for n in pos.lines[li].nums if n[2] in values.get(li, ())] if values else []
        if not atoms:
            return {}, None
        key = lambda n: n[1]
        xs = sorted(atoms, key=lambda a: key(a[1]))
        groups = [[xs[0]]]
        for a in xs[1:]:
            if key(a[1]) - key(groups[-1][-1][1]) <= 3.0:
                groups[-1].append(a)
            else:
                groups.append([a])
        out = {(li, n[5]): "value" for li, n in atoms}
        return out, (key, [(sum(key(n) for _, n in g) / len(g), "value") for g in groups])
    gaps = sorted(b[1] - a[1] for a, b in zip(heads, heads[1:]))
    lo = heads[0][1] - (gaps[len(gaps) // 2] / 2 if gaps else 25.0)       # left of this are conditions
    hi = unit[0] - 1 if unit and unit[0] > heads[-1][1] else 1e9
    atoms = [(li, n) for li in lis for n in pos.lines[li].nums if lo <= (n[0] + n[1]) / 2 and n[1] <= hi]
    if not atoms:
        return {}, None
    def marked(li, n):
        # "7.9 (1)": a value set left of its footnote mark keeps the column's right edge at the mark
        words = pos.lines[li].words
        nxt = words[n[5] + 1] if n[5] + 1 < len(words) else None
        if nxt and re.fullmatch(r"\(\d+\)", nxt[2]) and nxt[0] - n[1] < 8:
            return n[:1] + (nxt[1],) + n[2:]
        return n
    atoms = [(li, marked(li, n)) for li, n in atoms]
    keys = (lambda n: n[0], lambda n: (n[0] + n[1]) / 2, lambda n: n[1])
    best = None
    for which, key in enumerate(keys):
        xs = sorted(atoms, key=lambda a: key(a[1]))
        clusters = [[xs[0]]]
        for a in xs[1:]:
            if key(a[1]) - key(clusters[-1][-1][1]) <= 3.0:
                clusters[-1].append(a)
            else:
                clusters.append([a])
        if best is None or len(clusters) <= len(best[1]):
            best = (which, clusters)
    which, clusters = best
    out = {}
    hkey = (lambda h: h[1], lambda h: (h[1] + h[2]) / 2, lambda h: h[2])[which]
    means = [sum(keys[which](n) for _, n in cl) / len(cl) for cl in clusters]
    # Columns pair off with headers left to right, no two columns under one
    # header, at the least total distance; a column may stay unpaired.
    SKIP, FAR, INF = 60.0, 45.0, 1e18
    dp = [[INF] * (len(heads) + 1) for _ in range(len(clusters) + 1)]      # dp[k][u]: first k columns, headers below u free
    choice = {}
    dp[0][0] = 0.0
    for k in range(len(clusters)):
        for u in range(len(heads) + 1):
            if dp[k][u] == INF:
                continue
            if dp[k][u] + SKIP < dp[k + 1][u]:
                dp[k + 1][u] = dp[k][u] + SKIP
                choice[(k + 1, u)] = (u, None)
            for h in range(u, len(heads)):
                d = abs(hkey(heads[h]) - means[k])
                if d <= FAR and dp[k][u] + d < dp[k + 1][h + 1]:
                    dp[k + 1][h + 1] = dp[k][u] + d
                    choice[(k + 1, h + 1)] = (u, h)
    u = min(range(len(heads) + 1), key=lambda x: dp[len(clusters)][x])
    columns = []
    for k in range(len(clusters), 0, -1):
        u, h = choice[(k, u)]
        if h is not None:
            columns.append((means[k - 1], heads[h][0]))
        for li, n in clusters[k - 1]:
            # an unpaired column (a value set off from its column by a footnote mark) takes the nearest header
            out[(li, n[5])] = heads[h][0] if h is not None else Positions.under(heads, n[0], n[1])
    return out, (keys[which], columns)


def column_of(cols, n):
    """The header of the value column a number of another line stands in, or None."""
    if not cols:
        return None
    key, columns = cols
    for mean, label in columns:
        if abs(key(n) - mean) <= 3.5:
            return label
    return None


def report_scatter(rep, t, r, pos, doc):
    """A row whose values are on no single PDF line: say which line comes closest."""
    vals = [v for _, v, _ in r["vals"]]
    want = collections.Counter(vals)
    best, top = None, 0
    for v in want:
        for li in pos.num_post.get(v, ()):
            line = pos.lines[li]
            if doc.converted_pages and line.page not in doc.converted_pages:
                continue
            have = collections.Counter(n[2] for n in line.nums)
            hit = sum(min(c, have[x]) for x, c in want.items())
            near = set().union(*(x.toks for x in pos.window(li, 2)))
            score = hit + (sum(x in near for x in r["words"]) / len(r["words"]) if r["words"] else 0)
            if score > top:
                best, top = li, score
    if best is None:
        return False
    line = pos.lines[best]
    have = collections.Counter(n[2] for n in line.nums)
    missing = list((want - have).elements())
    if len(missing) != 1 or len(vals) < 2:
        return False      # a row joined from several source tables, or one that wraps
    near = set().union(*(x.toks for x in pos.window(best, 2)))
    if r["words"] and sum(x in near for x in r["words"]) / len(r["words"]) < 0.6:
        return False
    for li in pos.num_post.get(missing[0], ()):          # the same row in another table: a joined row
        if abs(li - best) > 3 and (not doc.converted_pages or pos.lines[li].page in doc.converted_pages):
            far = set().union(*(x.toks for x in pos.window(li, 2)))
            if not r["words"] or sum(x in far for x in r["words"]) / len(r["words"]) >= 0.6:
                return False
    heads = pos.header(best)[0]
    extra = [n[2] for n in line.nums if n[2] not in want and pos.under(heads, n[0], n[1])]
    if len(extra) != 1 or sum(1 for n in line.nums if pos.under(heads, n[0], n[1])) != len(vals):
        return False      # not the same row with one value different
    rep.add("error", "cells", r["line"], "value %s is not on this row in the source: the row with %s has %s. p.%d \"%s\""
            % (missing[0], ", ".join(v for v in vals if v != missing[0]) or "its text",
               ", ".join(extra) or "no other value", line.page, clip(line.text)))
    return True


def explain_loose(rep, t, r, pos, doc):
    """A row whose values are on no single source line. It may join the same
    row of several source tables (one table per device or temperature
    range): then each piece is a whole source row of a different table.
    Pieces from one table are several source rows made into one (rows stay
    rows); values that are on no row with the rest are wrong."""
    want = collections.Counter(v for _, v, sign in r["vals"] if sign is not None)
    left = collections.Counter(want)
    groups = []
    for _ in range(6):
        best, top = None, 0
        for v in left:
            for li in pos.num_post.get(v, ()):
                line = pos.lines[li]
                if (doc.converted_pages and line.page not in doc.converted_pages) or line.page in doc.scans:
                    continue
                heads, _, head_y = pos.header(li)
                cols = doc.columns.get((line.page, head_y))
                have = collections.Counter(n[2] for n in line.nums
                                           if (column_of(cols, n) if cols else pos.under(heads, n[0], n[1])))
                if not have or have - want:
                    continue          # not a piece of this row: it has values the row lacks
                hit = sum((have & left).values())
                if hit > top:
                    best, top = (li, (line.page, head_y), have), hit
        if best is None:
            break
        groups.append(best)
        left -= best[2]
        if not left:
            break
    if left:
        if any(v not in pos.num_post for v in left):
            return            # a value the text layer doesn't hold (a picture): From images covers it
        rep.add("error", "cells", r["line"], "%s %s on no source row together with this row's other values (%s)"
                % (", ".join(sorted(left.elements())), "is" if sum(left.values()) == 1 else "are",
                   ", ".join(v for _, v, _ in r["vals"])))
    elif len({g for _, g, _ in groups}) < len(groups):
        lines = [pos.lines[li] for li, _, _ in groups]
        rep.add("error", "cells", r["line"], "this row is made of %d rows of one source table (rows stay rows): p.%d %s"
                % (len(groups), lines[0].page, "; ".join('"%s"' % clip(x.text, 50) for x in lines[:3])))


def check_labels(rep, t, rows, chain, pos, doc, half, left):
    """A source row with no values (N/A) can still be dropped: every line of
    the placed table that starts at the table's left edge starts a row, and
    its first word must be in the file's table."""
    if len(chain) < 2:
        return
    mine = set(doc.known)
    for _, cells in t["raw"]:             # every row as written, banner rows (| NOISE | < | < |) too
        mine.update(tokens(norm(md_plain(MARK_RE.sub("", " ".join(cells))))))
    for r in rows:
        mine.update(tokens(norm(md_plain(" ".join(r["full"])))))
    if not pos.micro:
        mine.update(x.replace("μ", "m") for x in list(mine))
    by_page = collections.defaultdict(list)
    for li in chain.values():
        by_page[pos.lines[li].page].append(pos.lines[li].y)
    for page, ys in by_page.items():
        if len(ys) < 2:
            continue
        lo, hi = min(ys) - 1, max(ys) + 5 * half
        for x in pos.page_lines[page - 1]:
            if x.y <= lo:
                continue
            if x.y > hi or (x.y > max(ys) + 1 and (TABLE_END_RE.search(x.text) or pos.heads_of(x))):
                break
            x0, _, word = x.words[0]
            toks = [w for w in tokens(norm(word)) if not is_num(w) and len(w) > 1]
            if x0 > left[page] + 3 or not toks or x.index in doc.located:
                continue
            if any(not any(w in m for m in mine) for w in toks):
                doc.label_rows.append((t["line"], x, word))


def check_header_words(rep, t, chain, pos, doc):
    """The words of a table's headers and of each placed row's cells are printed in the source:
    a header on the pages of its rows, a cell within six lines of its row."""
    pages = {pos.lines[li].page for li in chain.values()}
    if not pages:
        return
    fix = (lambda w: w.replace("μ", "m")) if not pos.micro else (lambda w: w)
    words = lambda text: [fix(w) for w in tokens(norm(md_plain(MARK_RE.sub("", text))))
                          if not is_num(w) and len(w) > 1 and w not in STOP and w not in UNITS]
    on_pages = set()
    for p in pages:
        for x in pos.page_lines[p - 1]:
            on_pages |= x.toks
            row = [w.lower() for _, _, w in x.words]
            on_pages.update(a + b for a, b in zip(row, row[1:]))      # "CAPACITAN CE": a word the page breaks
    # the other way round: a word of the source's header lines is in the file's headers, caption or conditions
    mine = set()
    for text in [t["attrs"].get("caption", ""), t["attrs"].get("conditions", "")] + [c for cells in t["header"] for c in cells]:
        mine.update(tokens(norm(md_plain(MARK_RE.sub("", text)))))
    mine |= {a + b for a in list(mine) for b in list(mine) if len(a) + len(b) <= 14}
    allowed = mine | {"unit", "units", "parameter", "test", "conditions", "condition", "symbol", "metric", "thermal", "description"}
    seen = set()
    for li in sorted(chain.values()):
        hy = pos.header(li)[2]
        line = pos.lines[li]
        if hy is None or (line.page, hy) in seen:
            continue
        seen.add((line.page, hy))
        for x in pos.page_lines[line.page - 1]:
            if not (hy - 14 <= x.y <= hy + 9) or x.y >= line.y or SECTION_START_RE.match(x.text.strip()):
                continue
            for x0, x1, word in x.words:
                bare = re.sub(r"\(\d+\)|[†‡§¶]", "", word).strip("()")
                if not re.fullmatch(r"[A-Z][A-Z\d]{3,}", bare) or HEAD_RE.match(bare):
                    continue          # header words are set in capitals; anything else is a row's text
                w = bare.lower()
                if (w in STOP or w in UNITS or w in mine or w.rstrip("s") in mine or any(k.startswith(w) for k in allowed)
                        or (w in ("input", "output") and {"from", "to"} & mine)):
                    continue
                rep.add("soft", "cells", t["line"] + 1, "the source's header has \"%s\" (p.%d \"%s\"), which no header, "
                        "caption or conditions of this table has: a header row or group the source prints is kept"
                        % (word, x.page, clip(x.text, 80)))
    for cells in t["header"]:
        for c in cells:
            w = words(c)
            if not w or md_plain(MARK_RE.sub("", c)).strip().lower() in NAMED_COLS or ISSUE_RE.search(c):
                continue
            if set(MARK_RE.findall(c)) & t["own_notes"]:
                continue          # the converter's own note says how the header differs from the source's
            if sum(x in on_pages or x in doc.known for x in w) / len(w) < 0.6:
                rep.add("soft", "cells", t["line"] + 1, "header \"%s\" is not printed on the source page of this table (p.%s): "
                        "a heading of the converter's own is not written as the source's"
                        % (md_plain(c), ", ".join(map(str, sorted(pages)))))


def check_line(rep, t, r, pos, li, band, ctx, labels):
    line = pos.lines[li]
    doc_known = labels.get("known", ())
    heads, unit, _ = pos.header(li)
    known = labels.get("columns", {}).get((line.page, pos.header(li)[2]))
    under = lambda x, n: labels[(x.index, n[5])] if (x.index, n[5]) in labels else (
        column_of(known, n) if known else pos.under(heads, n[0], n[1]))
    where = 'p.%d "%s"' % (line.page, clip(line.text, 90))
    cols = t["cols"]
    # columns, order and signs
    last_x = -1.0
    used, chosen = set(), []
    for c, v, sign in r["vals"]:
        occ = [n for n in line.nums if n[2] == v and n[5] not in used]
        if not occ:
            continue
        label = cols[c] if cols[c] in VALUE_COLS else None
        labelled = [(n, under(line, n)) for n in occ]
        if label and sign is not None and heads and any(h for _, h in labelled):
            match = [n for n, h in labelled if h == label]
            if not match:
                rep.add("error", "cells", r["line"], "%s is under %s in the source, not %s. %s"
                        % (v, "/".join(sorted({h.upper() for _, h in labelled if h})), label.capitalize(), where))
                continue
            occ = match
        after = [n for n in occ if n[0] >= last_x - 0.5]
        if not after and sign is None:
            continue
        if not after:
            rep.add("error", "cells", r["line"], "%s is left of the value before it in the source: the columns are in another order. %s"
                    % (v, where))
            continue
        n = after[0]
        used.add(n[5])
        chosen.append(n)
        last_x = n[0]
        if sign is not None and n[3] is not None and sign != n[3] and not any(x[3] == sign for x in after) \
                and not set(MARK_RE.findall(" ".join(r["raw"]))) & t["own_notes"]:
            rep.add("error", "cells", r["line"], "sign differs: the source has %s%s, the file %s%s. %s"
                    % (n[3], v, sign, v, where))
    theirs_na = sum(1 for x0, x1, w in line.words if w.strip("()") in ("N/A", "NA"))
    mine_na = sum(1 for c in r["raw"] if md_plain(MARK_RE.sub("", c)).strip() == "N/A")
    if theirs_na > mine_na:
        rep.add("error", "cells", r["line"], "the source prints N/A %d time(s) on this row and the file has %d: a cell the "
                "source fills with N/A is N/A in the file, not empty. %s" % (theirs_na, mine_na, where))
    if band is None:
        return
    symbols, (common, heading), ucol, scol, prev, nxt, left, reach = ctx
    # the words of each cell the row states itself are printed within six lines of the row
    tall = 2 * r.get("span", 0) + 2
    near = set().union(*(x.toks for x in pos.window(li, max(10, tall))))
    for x in pos.window(li, max(10, tall)):          # "R θJA": a symbol the text layer prints in two pieces
        ws = ["".join(tokens(norm(w[2]))) for w in x.words]
        near.update(a + b for a, b in zip(ws, ws[1:]))
    fix = (lambda w: w.replace("μ", "m")) if not pos.micro else (lambda w: w)
    for c, cell in enumerate(r["raw"]):
        if c in t["vcols"] or c == ucol or cell in ("", "^", "<") or set(MARK_RE.findall(cell)) & t["own_notes"]:
            continue
        w = [fix(x) for x in tokens(norm(md_plain(MARK_RE.sub("", cell))))
             if not is_num(x) and len(x) > 1 and x not in STOP and x not in UNITS]
        close = near
        if cols[c] == "parameter":
            # a name is printed beside its row, or lower when it is centred on the rows it spans
            close = set().union(*(x.toks for x in pos.window(li, max(PARAM_REACH[1], tall)) if x.index >= li - PARAM_REACH[0]))
        if w and sum(x in close or x in doc_known for x in w) / len(w) < 0.6:
            rep.add("error", "cells", r["line"], "cell \"%s\" (%s) is not printed on this row in the source: a name or "
                    "condition the source doesn't give is left out, or labelled. %s"
                    % (md_plain(cell), cols[c] or "column %d" % (c + 1), where))
    # symbol: a line that starts with another row's symbol
    if scol is not None and r.get("symbol"):
        # the line's first word is another row's symbol; not "(VT+ – VT–)", the wrapped end of a
        # parameter name, and not the first word of this row's own symbol ("VCC or GND")
        seen = {k for k in symbols for x in band for w in x.words[:1]
                if w[0] <= left + 45 and not w[2].startswith("(") and not w[2].endswith(")")
                and not (len(x.words) > 1 and x.words[1][2].startswith("="))
                and "".join(tokens(norm(w[2]))) == k}
        if seen and r["symbol"] not in seen and not (" " in md_plain(r["full"][scol]).strip() and any(r["symbol"].startswith(k) for k in seen)) \
                and not set(MARK_RE.findall(" ".join(r["raw"]))) & t["own_notes"]:
            rep.add("error", "cells", r["line"], "the source row with these values is %s, not %s. %s"
                    % (", ".join(md_plain(symbols[k]) for k in sorted(seen)), md_plain(r["full"][scol]), where))
    # unit
    if ucol is not None and r["full"][ucol].strip():
        mine = unit_norm(MARK_RE.sub("", r["full"][ucol]))
        if unit:
            ux0, ux1 = unit[0] - 12, unit[1] + 12
            in_col = lambda x: [w[2] for w in x.words if ux0 <= (w[0] + w[1]) / 2 <= ux1 and not re.search(r"\d", w[2])]
            found = [u for x in band for u in in_col(x)]
            if not found:
                page = pos.page_lines[line.page - 1]
                up = [u for x in page if unit[2] < x.y < line.y for u in in_col(x)][-1:]      # centred on a tall group
                down = [u for x in page if x.y > line.y and x.y - line.y < max(80, 14 * r.get("span", 0)) for u in in_col(x)][:1]
                found = up + down
        else:
            # no Unit column: the unit follows the value ("7 V", "20mA")
            found = []
            for n in chosen:
                after = line.words[n[5] + 1][2] if n[5] + 1 < len(line.words) else ""
                if n[4] or (re.fullmatch(r"[^\W\d_°%/]{1,6}|°C|%", after) and after.lower() not in ("to", "or", "and")):
                    found.append(n[4] or after)
        found = [u for u in found if unit_norm(u) and unit_norm(u) not in ("unit", "units", "load")
                 and re.search(r"[^\W\d_]|[%°]", u)
                 and (unit_norm(u) in UNIT_NORMS or re.fullmatch(r"[a-zA-Zµμ°Ω%/]{1,6}", u) and not u.islower() or len(u) <= 2)]
        theirs = {unit_norm(u) for u in found}
        # a text layer without µ prints it as m; so does one page of it, when the file says the µ was
        # read from the image (From images: µA)
        if not pos.micro or (line.page not in pos.micro_pages and any(k[:1] in "µμ" for k in doc_known)):
            theirs |= {"u" + u[1:] for u in theirs if u.startswith("m") and len(u) > 1}
        if found and mine not in theirs and not any(
                u.startswith(mine) or mine.startswith(u) for u in theirs if len(u) > 1):
            rep.add("error", "cells", r["line"], "unit %s: the source row has %s. %s"
                    % (md_plain(r["full"][ucol]), ", ".join(sorted(set(found))), where))
    # numbers on this row in the source that the file's row doesn't have
    stray, conds = [], []
    mine = collections.Counter(r["allvals"])
    printed = collections.Counter()
    for x in band:
        refs = x.ref_nums()
        row_marks = set(MARK_RE.findall(" ".join(r["raw"])))
        for n in x.nums:
            if n[5] in refs or (unit and n[0] > unit[1] + 12):
                continue
            if x.words[n[5]][2].endswith(")") and "(" not in x.words[n[5]][2]:
                continue
            head = under(x, n)
            if n[5] == 0 and len(x.words) > 1 and re.match(r"^\d+(\.\d+)+$", x.words[0][2]) and x.words[1][2][:1].isupper():
                continue          # a section heading: "5.2 ESD Ratings"
            if not head and heads and (n[0] + n[1]) / 2 < heads[0][1] - 8 and abs(x.y - line.y) <= reach:
                lost = n[5] == 1 and len(n[2]) == 1 and len(x.words[0][2]) <= 2      # "V 1": a subscript I read as 1
                marker = n[2] in row_marks and len(n[2]) == 1       # a bare superscript note mark the row carries as [n]
                if n[2] not in r["nums"] and n[2] not in heading and not lost and not marker:
                    conds.append("%s%s" % (n[3] or "", n[2]))
            # a value under a header: on the row's own line the file must have it, as often as
            # it is printed there; on a line between two rows it belongs to a row the file lacks
            if head and x is line and n[2] in mine and n[2] not in common:
                printed[n[2]] += 1
                if printed[n[2]] > mine[n[2]]:
                    stray.append((x, "%s%s a %s time (under %s)" % (n[3] or "", n[2],
                                  {2: "second", 3: "third"}.get(printed[n[2]], "further"), head.upper()), n[2]))
                continue
            if head and (x is not line or (n[2] not in r["nums"] and n[2] not in common)):
                stray.append((x, "%s%s under %s" % (n[3] or "", n[2], head.upper()), n[2]))
    if conds:
        rep.add("error", "cells", r["line"], "the source row with these values has %s in its conditions, and the file's row does not. %s"
                % (", ".join(conds), where))
    for x, what, v in stray:
        labels["strays"].append((r["line"], x, what, x is line, v))


# ---------------------------------------------------------------- ordering table, front matter, figures

STATUS = ("Active", "Obsolete", "NRND", "Last Time Buy", "Preview")
ADD_COLS = ("part", "status", "material", "package", "qty", "rohs", "finish", "msl", "temp", "marking")
MONTHS = {m: i + 1 for i, m in enumerate("january february march april may june july august september october "
                                         "november december".split())}
PARTS_DIR = os.path.join(REPO, "schematical", "schematical", "Parts")
_CATALOG = None


def catalog():
    """Part name -> (part, device, "kind:id"), from the part files."""
    global _CATALOG
    if _CATALOG is None:
        _CATALOG = {}
        for name in sorted(os.listdir(PARTS_DIR)):
            if not name.endswith(".json") or name in ("datasheets.json", "footprints.json"):
                continue
            with open(os.path.join(PARTS_DIR, name), encoding="utf-8") as f:
                data = json.load(f)
            for dev in data.get("devices", []):
                for p in dev["parts"]:
                    _CATALOG[p["name"]] = (p, dev, "%s:%s" % (data.get("kind"), dev["id"]))
    return _CATALOG


def claimed_parts(fm):
    cat = catalog()
    names = [n for n, (_, _, key) in cat.items() if key in fm.get("devices", [])]
    out = []
    for p in fm.get("parts", []):
        if "*" in p or "?" in p:
            out += sorted(n for n in names if fnmatch.fnmatchcase(n, p))
        elif p in cat:
            out.append(p)
    return out


def addendum(pos):
    """The vendor's ordering table (TI's package option addendum), read by
    position: {orderable: {status, package, pins, qty, carrier, temp, marking, marks, page}}.
    Wrapped cells are joined; a marking's wrapped pieces join without a space
    unless the piece before ends a marking (the table's own rule)."""
    rows = {}
    for lines in pos.page_lines:
        head = next((x for x in lines if {"Orderable", "Status", "RoHS"} <= {w[2] for w in x.words}
                     and any(w[2] == "marking" for w in x.words)), None)
        if head is None:
            continue
        ws = head.words
        starts = [k for k, w in enumerate(ws) if w[2] in ("Orderable", "Status", "Material", "Package", "RoHS", "Lead", "MSL", "Op")]
        starts.append(max(k for k, w in enumerate(ws) if w[2] == "Part"))
        if len(starts) != len(ADD_COLS):
            continue
        centres = []
        for a, b in zip(starts, starts[1:] + [len(ws)]):
            centres.append((ws[a][0] + ws[b - 1][1]) / 2)
        cur, last_y = None, None
        for x in lines:
            if x.y <= head.y:
                continue
            cells = []
            for w in x.words:
                if cells and w[0] - cells[-1][1] <= 6.5:
                    cells[-1] = [cells[-1][0], w[1], cells[-1][2] + " " + w[2]]
                else:
                    cells.append([w[0], w[1], w[2]])
            got = {}
            for x0, x1, text in cells:
                c = min(range(len(centres)), key=lambda k: abs(centres[k] - (x0 + x1) / 2))
                got[ADD_COLS[c]] = (got[ADD_COLS[c]] + " " + text) if ADD_COLS[c] in got else text
            if "part" in got and got.get("status", "").split(" ")[0] in ("Active", "Obsolete", "NRND", "Last", "Preview"):
                cur = {k: [v] for k, v in got.items()}
                cur["page"] = x.page
                rows[got["part"].replace(" ", "")] = cur
                last_y = x.y
            elif cur is not None and "part" not in got and not re.match(r"^\(\d+\)", x.text) and (
                    x.y - last_y <= 16 or (x.y - last_y <= 30 and set(got) == {"marking"})):
                for k, v in got.items():
                    cur.setdefault(k, []).append(v)
                last_y = x.y
            else:
                cur = None
    out = {}
    for part, r in rows.items():
        marking = ""
        for piece in r.get("marking", []):
            marking += piece if not marking or not re.search(r"[,)]$", marking) else " " + piece
        qty, _, carrier = " ".join(r.get("qty", [])).partition("|")
        pkg, _, pins = " ".join(r.get("package", [])).partition("|")
        carrier = carrier.strip()
        out[part] = {
            "status": " ".join(r.get("status", [])),
            "package": pkg.strip(), "pins": int(pins) if pins.strip().isdigit() else None,
            "qty": int(qty) if qty.strip().isdigit() else None,
            "carrier": "" if carrier.lower() in ("", "-", "null") else carrier,
            "carrier_raw": carrier,
            "temp": "" if " ".join(r.get("temp", [])).strip() in ("", "-") else " ".join(r.get("temp", [])).strip(),
            "marking": marking,
            "marks": marks(marking),
            "marks_split": marks(", ".join(r.get("marking", []))),      # each printed line a marking of its own
            "page": r["page"],
        }
    return out


def marks(text):
    """The set of markings in a marking cell: alternatives are separated by
    commas (or ~, or parentheses); spaces inside one are line breaks on the part."""
    # "U08(R, Z)": one marking with a last character that varies, U08R and U08Z
    text = re.sub(r"(\w+)\(([^()]*)\)", lambda m: ", ".join(m.group(1) + x.strip() for x in m.group(2).split(",")), text)
    return {m for m in (re.sub(r"\s+", "", x) for x in re.split(r"[(),~]", text)) if m and m != "-"}


def packing_text(row):
    """The ordering block's packing field for an addendum row."""
    if row["qty"] is None:
        return ""
    c = row["carrier"].upper()
    kind = "Reel" if "T&R" in c or "REEL" in c else "Tube" if "TUBE" in c else "Tray" if "TRAY" in c else None
    return "%s of %d" % (kind, row["qty"]) if kind else str(row["qty"])


def check_ordering(doc, pos, rep):
    add = addendum(pos)
    claimed = claimed_parts(doc.fm)
    lines = []
    for line, kind, attrs, body, section in doc.fences:
        if kind == "ordering":
            for k, b in enumerate(body):
                if b.strip():
                    lines.append((line + 1 + k, [c.strip() for c in b.split("|")]))
    rep.stats["ordering rows checked"] = (0, 0)
    if not add:
        if lines:
            rep.add("manual", "ordering", lines[0][0], "the source has no ordering table this script can read "
                    "(TI package option addendum); the ordering lines must be checked by a person")
        return
    cat = catalog()
    ok = 0
    listed = set()
    for ln, cells in lines:
        if len(cells) not in (4, 5):
            continue
        part, packing, temp, marking = cells[:4]
        status = cells[4] if len(cells) == 5 else ""
        listed.add(part)
        row = add.get(part)
        if row is None:
            rep.add("error", "ordering", ln, "%s has no row in the source's ordering table (p.%s)"
                    % (part, ", ".join(str(p) for p in sorted({r["page"] for r in add.values()}))))
            continue
        bad = False
        want = packing_text(row)
        if packing != want:
            bad = True
            rep.add("error", "ordering", ln, '%s packing "%s": the source row has "%s | %s", written "%s" (p.%d)'
                    % (part, packing, row["qty"] if row["qty"] is not None else "-", row["carrier_raw"] or "-", want, row["page"]))
        if re.sub(r"\s+", " ", temp.translate(DASHES)) != re.sub(r"\s+", " ", row["temp"].translate(DASHES)):
            bad = True
            rep.add("error", "ordering", ln, '%s temperature "%s": the source row has "%s" (p.%d)'
                    % (part, temp, row["temp"] or "-", row["page"]))
        if "…" in marking or "..." in marking or marks(marking) not in (row["marks"], row["marks_split"]):
            bad = True
            rep.add("error", "ordering", ln, '%s marking "%s": the source row has "%s", written "%s" (p.%d)'
                    % (part, marking, row["marking"] or "-", ", ".join(sorted(row["marks"])), row["page"]))
        want = "" if row["status"] == "Active" else row["status"]
        if status != want:
            bad = True
            rep.add("error", "ordering", ln, '%s status "%s": the source row says %s; the fifth field must be "%s" (p.%d)'
                    % (part, status, row["status"], want, row["page"]))
        ok += not bad
    missing = [p for p in claimed if p in add and p not in listed]
    if missing:
        rep.add("error", "ordering", lines[0][0] if lines else 0,
                "the ordering block has no line for %d claimed part(s) the source's ordering table lists: %s"
                % (len(missing), ", ".join("%s | %s | %s | %s%s" % (p, packing_text(add[p]), add[p]["temp"],
                   ", ".join(sorted(add[p]["marks"])), "" if add[p]["status"] == "Active" else " | " + add[p]["status"])
                   for p in missing[:40])))
    rep.stats["ordering rows checked"] = (ok, len(lines))
    # the catalog against the ordering table
    packages = doc.fm.get("packages", {})
    for p in claimed:
        row = add.get(p)
        if row is None:
            continue
        part, dev, _ = cat[p]
        flags = part.get("flags", [])
        if row["status"] == "Obsolete" and "obsolete" not in flags:
            rep.add("error", "catalog", 0, '%s is Obsolete in the source\'s ordering table (p.%d); the catalog part has no "obsolete" flag'
                    % (p, row["page"]))
        pins = [x for x in dev["pinouts"][part["pinout"]] if x.get("role") != "absent"]
        if row["pins"] is not None and row["pins"] != len(pins):
            rep.add("error", "catalog", 0, "%s has %d pins in the source's ordering table (p.%d); catalog pinout %s has %d"
                    % (p, row["pins"], row["page"], part["pinout"], len(pins)))
        m = re.search(r"\(([A-Z0-9]+)\)", row["package"])
        if m and packages:
            named = {x.strip() for x in packages.get(part["pinout"], "").split(",")}
            if m.group(1) not in named:
                rep.add("error", "frontmatter", 2, 'packages: %s is package %s in the source (p.%d); "%s" lists %s'
                        % (p, m.group(1), row["page"], part["pinout"], ", ".join(sorted(named - {""})) or "nothing"))


def check_frontmatter(doc, src, pos, rep):
    fm = doc.fm["source"]
    if fm.get("pages") != len(src.pages):
        rep.add("error", "frontmatter", 2, "source.pages is %s; the PDF has %d pages" % (fm.get("pages"), len(src.pages)))
    docno = fm.get("document", "")
    if not docno:
        return
    flat = lambda t: re.sub(r"[\s\-–—]", "", t).upper()
    hits = [(pi, text) for pi, (_, lay, _) in enumerate(src.pages) for text in lay.split("\n") if flat(docno) in flat(text)]
    if not hits and 1 in src.ocr_pages and re.search(r"(?<![A-Z0-9])%s(?![A-Z0-9])" % re.escape(flat(docno)), flat(pdf_info(doc))):
        return            # a scan: the number is in the PDF's own metadata
    if not hits:
        base = re.sub(r"[A-Z]$", "", flat(docno))
        seen = sorted({m for _, lay, _ in src.pages for m in re.findall(re.escape(base) + r"[A-Z]?\b", flat(lay))})
        rep.add("error", "frontmatter", 2, 'source.document "%s" is not printed in the PDF%s'
                % (docno, " (it prints %s)" % ", ".join(seen[:3]) if seen else ""))
        return
    dates = []
    for _, text in hits:
        for mon, year in re.findall(r"\b([A-Za-z]{3,9})\.? (\d{4})\b", text):
            key = next((k for k in MONTHS if k.startswith(mon.lower()[:3]) and mon.lower() == k[:len(mon)]), None)
            if key:
                dates.append("%s-%02d" % (year, MONTHS[key]))
    if dates and fm.get("date"):
        latest = max(dates)
        if not latest.startswith(fm["date"]) or (len(fm["date"]) == 4 and False):
            rep.add("error", "frontmatter", 2, 'source.date "%s": the PDF\'s document line gives %s as its latest date'
                    % (fm["date"], latest))


def cite_label_on_page(src, page, label):
    """Whether "Table 6-3" / "Figure 8-1" is printed on the page (a dash may be set as any dash)."""
    want = re.escape(norm(label)).replace("\\ ", r"\s+")
    pat = re.compile(r"(?<![\w])" + want + r"(?![\w]|-\d)")
    return any(pat.search(norm(src.pages[page - 1][i])) for i in (0, 1))


def table_numbers(doc, t):
    """The distinct numbers of a table (single digits and derived values left out)."""
    out = set()
    for _, cells in t["rows"]:
        for c in cells:
            out.update(x for x in tokens(norm(md_plain(MARK_RE.sub("", c)))) if is_num(x)
                       and not (x.isdigit() and int(x) < 10) and x not in doc.derived)
    return out


def check_cites(doc, src, rep):
    """Citations [[page::n]], [[page::Table 6-3]]: the page exists and was converted, the paragraph
    number is one the page has, a label is printed on the page, each sentence is in a paragraph its
    own paragraph cites, and a table's values, a pin table's names or an ordering table's parts are on
    the cited pages."""
    n = len(src.pages)
    conv = doc.converted_pages
    owners = collections.defaultdict(list)
    eye = set()
    fence_kind = {("fence", f[0]): f[1] for f in doc.fences}
    for line, page, ref, owner in doc.cites:
        owners[owner].append((line, page, ref))
        if page < 1 or page > n:
            rep.add("error", "cites", line, "[[%d::%s]]: the PDF has %d pages" % (page, ref, n))
            continue
        if conv and page not in conv and fence_kind.get(owner) != "ordering":
            rep.add("error", "cites", line, "[[%d::%s]]: page %d is not among the pages Conversion notes list as converted"
                    % (page, ref, page))
        scan = page in src.scans
        if ref.isdigit():
            if scan:
                if page not in eye:
                    eye.add(page)
                    rep.add("manual", "cites", line, "page %d is a picture: its paragraph numbers are counted by a person" % page)
            elif page in src.ocr_pages:
                pass
            elif int(ref) < 1 or int(ref) > len(src.paras[page - 1]):
                rep.add("error", "cites", line, "[[%d::%s]]: page %d has %d paragraphs (python3 -I cite.py list <pdf> %d)"
                        % (page, ref, page, len(src.paras[page - 1]), page))
        elif re.fullmatch(r"(?:Table|Figure) \S+", ref):
            if not scan and not cite_label_on_page(src, page, ref):
                rep.add("error", "cites", line, "[[%d::%s]]: \"%s\" is not printed on page %d" % (page, ref, ref, page))
        elif re.fullmatch(r"(?:table|figure) #\d+", ref):
            rep.add("info", "cites", line, "[[%d::%s]]: the page's %s is counted by a person" % (page, ref, ref.split()[0]))
    # prose: every sentence is in a paragraph its own paragraph cites, and every cited paragraph is used
    units_of = collections.defaultdict(list)
    for u, para in doc.unit_para.items():
        units_of[para].append(u)
    for para_line, section in doc.paras:
        cites = owners.get(("para", para_line), [])
        if not cites:
            continue
        num = {(page, int(ref)) for _, page, ref in cites if ref.isdigit() and 1 <= page <= n}
        pages = {page for _, page, _ in cites}
        used, bad = set(), []
        for u in sorted(units_of.get(para_line, [])):
            w = doc.where.get(u)
            if not w or ISSUE_RE.search(u[1]):
                continue
            if w[0] == "page":
                if not ({w[1] - 1, w[1], w[1] + 1} & pages):
                    bad.append((u, "page %d" % w[1]))
                used |= {x for x in num if x[0] == w[1]}      # found on the page, not in one paragraph
                continue
            locs = {(src.sents[k][0], src.sent_para[k]) for k in w[1]}
            hit = locs & num
            if hit:
                used |= hit
                continue
            if any(pn is None or pg in src.ocr_pages or pg in src.scans for pg, pn in locs) and \
                    {pg for pg, _ in locs} & pages:
                used |= {x for x in num if x[0] in {pg for pg, _ in locs}}
                continue          # printed outside a numbered paragraph, or on a page read by OCR
            if any(pg - 1 in pages or pg + 1 in pages for pg, pn in locs) and num and \
                    any(abs(pg - q) == 1 for pg, _ in locs for q, _ in num):
                continue          # a sentence over a page break
            if pages & src.scans:
                continue          # it cites a picture page: the text found elsewhere may be a caption or an OCR match
            bad.append((u, ", ".join("p.%d %s" % (pg, "¶%d" % pn if pn else "outside a numbered paragraph")
                                     for pg, pn in sorted(locs, key=lambda x: (x[0], x[1] or 0)))))
        if bad:
            (ln, text), where = bad[0]
            rep.add("error", "cites", cites[-1][0], "this paragraph cites %s, but the source has the sentence \"%s\" (line %d) at %s%s"
                    % (" ".join("[[%d::%s]]" % (p_, r_) for _, p_, r_ in cites), clip(text, 70), ln, where,
                       " (and %d more sentences)" % (len(bad) - 1) if len(bad) > 1 else ""))
        for page, number in sorted(num - used):
            if any(abs(page - q) == 1 for q, _ in used) or page in src.ocr_pages or page in src.scans:
                continue
            if not used and not any(doc.where.get(u) for u in units_of.get(para_line, [])):
                continue
            first = next((t for k, t in src.paras[page - 1] if k == number), "") if page <= len(src.paras) else ""
            rep.add("error", "cites", cites[-1][0], "[[%d::%d]] is cited, and no sentence of this paragraph is in it (it begins \"%s\")"
                    % (page, number, clip(first, 60)))
    # blocks
    tables = {t["line"]: t for t in doc.tables}
    for line, kind, attrs, body, section in doc.fences:
        if section.lower() in SKIP_SECTIONS:
            continue
        cites = owners.get(("fence", line), [])
        pages = sorted({p for _, p, _ in cites if 1 <= p <= n})
        if not cites or not pages or any(p in src.scans for p in pages):
            continue
        if kind == "table" and line in tables:
            nums = table_numbers(doc, tables[line])
            if len(nums) >= 6:
                got = set().union(*(src.page_nums[p - 1] for p in pages))
                share = len(nums & got) / len(nums)
                if share < 0.5:
                    best = max(range(1, n + 1), key=lambda p: len(nums & src.page_nums[p - 1]))
                    rep.add("error", "cites", cites[-1][0], "this table's values are not on the cited page%s (%d%% of them): "
                            "they are on page %d (%d%%)" % ("s" if len(pages) > 1 else "", 100 * share, best,
                                                           100 * len(nums & src.page_nums[best - 1]) / len(nums)))
        elif kind in ("pins", "ordering"):
            names = set()
            for b in body:
                head = b.partition(":")[0] if kind == "pins" else b.split("|")[0]
                names.update(x for x in tokens(norm(md_plain(head))) if not x.isdigit() or len(x) > 2)
            have = set().union(*(src.page_toks[p - 1] for p in pages))
            if len(names) >= 3 and len(names & have) / len(names) < 0.5:
                rep.add("error", "cites", cites[-1][0], "this %s block's names are not on the cited page%s (%d%% of them)"
                        % (kind, "s" if len(pages) > 1 else "", 100 * len(names & have) / len(names)))


def check_note_pages(doc, src, rep):
    """A page a Conversion note gives for a figure, a table or the addendum holds that thing."""
    lines = doc.text.split("\n")
    start = next((k for k, x in enumerate(lines) if x.strip() == "## Conversion notes"), None)
    if start is None:
        return
    text_of = lambda p: " ".join(x[1] for x in src.lines if x[0] == p)
    for k in range(start + 1, len(lines)):
        for m in re.finditer(r"(Figures? (\d+(?:-\d+)?)|Table (\d+(?:-\d+)?)|package option addendum)[^.;()]{0,30}\(?,? ?p\.\s?(\d+)\)?",
                             lines[k], re.I):
            page = int(m.group(4))
            if page > len(src.pages):
                rep.add("error", "notes", k + 1, "Conversion notes: p.%d is past the end of the PDF (%d pages)" % (page, len(src.pages)))
                continue
            there = text_of(page)
            what = m.group(1)
            found = (re.search(r"package option addendum", there, re.I) if what.lower().startswith("package")
                     else re.search(r"(Figure|Table)\s+%s\b" % re.escape(m.group(2) or m.group(3)), there, re.I))
            if not found and page not in src.ocr_pages:
                rep.add("soft", "notes", k + 1, "Conversion notes: \"%s\" is not on p.%d of the PDF (pages are counted "
                        "from the PDF's first page)" % (what, page))


def pdf_info(doc):
    """The PDF's metadata (title, subject, keywords), as pdfinfo prints it."""
    path = os.path.join(DATASHEETS_DIR, doc.fm["source"]["pdf"])
    try:
        return subprocess.run(["pdfinfo", path], capture_output=True, text=True, timeout=30).stdout if path else ""
    except (OSError, subprocess.SubprocessError):
        return ""


def check_page_ranges(doc, src, rep):
    """The Conversion notes' "pages A–B converted" and "(C–D) dropped" against the PDF."""
    n = len(src.pages)
    conv = doc.converted_pages or set()
    if not conv:
        rep.add("error", "pages", 0, 'Conversion notes must say which source pages were converted ("Source pages 1–6 converted; …")')
        return
    if max(conv) > n:
        rep.add("error", "pages", 0, "Conversion notes: page %d converted, but the PDF has %d pages" % (max(conv), n))
    for line, section, text in doc.prose:
        if section != "Conversion notes":
            continue
        for a, b in re.findall(r"\((\d+)\s*[–-]\s*(\d+)\)", text):
            a, b = int(a), int(b)
            if b > n:
                rep.add("error", "pages", line, "Conversion notes: pages (%d–%d), but the PDF has %d pages" % (a, b, n))
            both = sorted(conv & set(range(a, b + 1)))
            if both and re.search(r"dropped", text) and not re.search(r"converted", text.split("(%d" % a)[0][-40:]):
                rep.add("error", "pages", line, "Conversion notes: page %s is listed as converted and inside the dropped range (%d–%d)"
                        % (", ".join(map(str, both)), a, b))


def check_figures(doc, src, rep):
    """A shared figure's parameters (CL: 50 pF): every number is in the source
    (or listed under Derived / From images), and printed there with its unit."""
    pairs = set()
    for _, _, toks in src.lines:
        pairs.update(zip(toks, toks[1:]))
    for line, kind, attrs, body, section in doc.fences:
        if kind != "figure":
            continue
        for k, b in enumerate(body):
            key, sep, value = b.partition(":")
            if not sep or not value.strip():
                continue
            toks = tokens(norm(md_plain(value)))
            for n, v in enumerate(toks):
                if not is_num(v) or v in doc.derived:
                    continue
                unit = toks[n + 1] if n + 1 < len(toks) and toks[n + 1] in UNITS else None
                if re.search(r"(?<![\d.])%s\s?%%" % re.escape(v), md_plain(value)) and not any(
                        re.search(r"(?<![\d.])%s\s?%%" % re.escape(v), raw) for raw, _, _ in src.pages):
                    rep.add("error", "figures", line + 1 + k, "figure parameter \"%s\": the source never prints %s %%"
                            % (b.strip(), v))
                    continue
                alts = {unit, "w" if unit == "ω" else unit, "kw" if unit == "kω" else unit, "mw" if unit == "mω" else unit}
                if not src.has(v):
                    rep.add("error", "figures", line + 1 + k, "figure parameter \"%s\": %s is not in the source PDF "
                            "(a value read from the drawing goes under From images in Conversion notes)" % (b.strip(), v))
                elif unit and not any((v, u) in pairs for u in alts):
                    rep.add("error", "figures", line + 1 + k, "figure parameter \"%s\": the source never prints %s %s"
                            % (b.strip(), v, unit))


def check_variant(doc, src, rep):
    """A -Q1 or -EP datasheet names its own device: text carried over from the
    catalog device's datasheet names the part without the suffix."""
    names = re.findall(r"\b([A-Z]{2,}\d[A-Z0-9]*)-(Q1|EP)\b", doc.fm.get("title", ""))
    for base, suffix in names:
        bare = re.compile(r"\b%s\b(?!-%s|[A-Z0-9])" % (re.escape(base), suffix))
        pages = doc.converted_pages or range(1, len(src.pages) + 1)
        in_pdf = any(bare.search(src.pages[p - 1][0]) for p in pages if p <= len(src.pages))
        if in_pdf:
            continue
        for line, section, text in doc.prose:
            if section != "Conversion notes" and bare.search(text) and not ISSUE_RE.search(text):
                rep.add("error", "terms", line, "names %s; the source's converted pages only say %s-%s" % (base, base, suffix))


_SOURCES = {}


def sources(pdf, ocr, cache):
    """The PDF's text (Source) and word positions (Positions), read once per run."""
    key = (pdf, os.path.getmtime(pdf), ocr)
    if key not in _SOURCES:
        src = Source(load_pdf(pdf, ocr, cache), load_blocks(pdf, cache))
        src.scans = set(scan_pages(pdf, cache)) | set(src.ocr_pages)
        _SOURCES[key] = (src, Positions(load_words(pdf, cache)))
    return _SOURCES[key]


def validate(path, ocr, cache):
    rep = Report(path)
    doc = Doc(path)
    pdf = os.path.join(DATASHEETS_DIR, doc.fm["source"]["pdf"])
    if not os.path.exists(pdf):
        rep.add("error", "source", 0, "source PDF %s not found" % doc.fm["source"]["pdf"])
        return rep
    src, pos = sources(pdf, ocr, cache)
    conv = doc.converted_pages or set(range(1, len(src.pages) + 1))
    scans = sorted(conv & src.scans)
    doc.scans = src.scans
    rep.scanned = 2 * len(scans) >= len(conv)        # a scanned datasheet, not one with a picture page
    if scans:
        rep.add("manual", "source", 0, "pages %s are pictures (a scan, or a figure page): this script can't read "
                "tables and values from them; a person compares those pages%s" % (", ".join(map(str, scans)),
                "" if rep.scanned else ", and values read from them go under From images"))
    cat = catalog()
    for key in doc.fm.get("devices", []):
        for part, dev, k in cat.values():
            if k == key:
                for pins in dev["pinouts"].values():
                    for pin in pins:
                        doc.known.update(tokens(norm(pin["name"])))
                break
    check_numbers_terms(doc, src, rep)
    check_rows(doc, src, rep)
    check_cells(doc, pos, rep)
    for line, msg in doc.row_pending:
        if line not in doc.placed:        # the cells check put the row on the line that holds its values
            rep.add("soft", "rows", line, msg)
    check_coverage(doc, pos, rep)
    check_function_tables(doc, src, rep)
    check_ordering(doc, pos, rep)
    check_frontmatter(doc, src, pos, rep)
    check_page_ranges(doc, src, rep)
    check_note_pages(doc, src, rep)
    check_figures(doc, src, rep)
    check_variant(doc, src, rep)
    check_prose(doc, src, rep)
    check_cites(doc, src, rep)
    check_notes(doc, src, rep)
    check_pages(doc, src, rep)
    check_registers(doc, rep)
    check_blocks(doc, src, rep)
    flags = ISSUE_RE.findall(doc.text)
    rep.stats["source issues flagged"] = (len(flags), len(flags))
    rep.items.sort(key=lambda x: ({"error": 0, "manual": 1, "info": 2}[x[0]], x[2]))
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
        print("%s: %d errors, %d to check by eye; %s" % (path, rep.count("error"), rep.count("manual"), stats))
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
