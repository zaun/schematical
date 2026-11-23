#!/usr/bin/env python3
"""Citations of a converted datasheet: list the PDF's numbered paragraphs, and
fill in the citations of a file.

A citation points from the file back to its source, in PDF page numbers
(counted from the first page, as Conversion notes count them):

    [[5::2]]            paragraph 2 of page 5
    [[5::Table 6-3]]    the table the PDF labels "Table 6-3", on page 5
    [[7::Figure 8-1]]   the figure the PDF labels "Figure 8-1", on page 7
    [[4::table #2]]     the 2nd table of page 4, which the PDF doesn't label
    [[4::figure #1]]    likewise for a figure

format.md § Citations says where they go. The renderer does not show them.

Paragraph numbers are the PDF's text blocks as validate.py reads them (a
block of reading-order text that is prose once the running header and
footer are out), so a number can be looked up:

    python3 -I cite.py list <id or pdf> [page ...]      # the numbered paragraphs
    python3 -I cite.py fill [--dry] <file.md> ...       # add or redo the citations
    python3 -I cite.py scans <file.md> ...              # the citations on picture pages (counted by a person)

`fill` places each citation from where validate.py finds the file's words in
the PDF; run check.py and validate.py afterwards. It rewrites only the
citations. Everything it cannot place it leaves, and the checkers say so.
"""

import importlib.util
import os
import re
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
DATASHEETS = os.path.join(REPO, "datasheets")


def load(name):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HERE, name + ".py"))
    mod = importlib.util.module_from_spec(spec)
    sys.modules[name] = mod
    spec.loader.exec_module(mod)
    return mod


def find_pdf(arg):
    if os.path.exists(arg) and arg.endswith(".pdf"):
        return arg
    for root, _, files in os.walk(DATASHEETS):
        if arg + ".pdf" in files:
            return os.path.join(root, arg + ".pdf")
    sys.exit("no PDF %s" % arg)


def cmd_list(argv):
    v = load("validate")
    pdf = find_pdf(argv[0])
    cache = os.path.join(tempfile.gettempdir(), "dsvalidate")
    pages = v.load_pdf(pdf, "auto", cache)
    paras = v.numbered_blocks(pages, v.load_blocks(pdf, cache))
    want = {int(a) for a in argv[1:]} or set(range(1, len(pages) + 1))
    for pi, found in enumerate(paras):
        if pi + 1 not in want:
            continue
        print("page %d%s" % (pi + 1, " (OCR: the numbers depend on tesseract)" if pages[pi][2] else ""))
        for n, text in found:
            print("  [[%d::%d]] %s%s" % (pi + 1, n, text[:100], " ..." if len(text) > 100 else ""))
    return 0


CITE_RE = re.compile(r"[ \t]*\[\[(\d+)::([^\[\]]+?)\]\]")
LABEL_RE = re.compile(r"^\s*(Table|Figure)\s+([A-Z]?\d+(?:[-.–—−‑]\d+)*[A-Za-z]?)\b[.:]?\s*(.*)$")
CITED = {"table", "pins", "ordering", "wavedrom", "curve", "figure"}      # blocks that carry a citation


def strip(text):
    """The file's text without citations (a line with nothing else is dropped)."""
    out, fence = [], False
    for line in text.split("\n"):
        if fence:
            fence = line.strip() != "```"
            out.append(line)
            continue
        if line.startswith("```") and len(line.strip()) > 3:
            fence = True
            out.append(line)
            continue
        new = CITE_RE.sub("", line).rstrip()
        if new or not line.strip() or not CITE_RE.search(line):
            out.append(new if CITE_RE.search(line) else line)
    return "\n".join(out)


def stem(words):
    return {w[:-1] if len(w) > 3 and w.endswith("s") else w for w in words}


def words_of(v, text):
    return stem(v.content(v.tokens(v.norm(text))))


SEGMENT_RE = re.compile(r"\S+(?:\s{1,2}\S+)*")


def labels(v, src, page):
    """The labelled tables and figures a page prints: (label, caption word set). Two figures side by
    side print their labels on one line; a caption may go on in the same column of the next line."""
    out = []
    rows = [[(m.start(), m.group(0)) for m in SEGMENT_RE.finditer(line)] for line in src.pages[page - 1][1].split("\n")]
    for k, segs in enumerate(rows):
        for at, seg in segs:
            m = LABEL_RE.match(seg)
            if m:
                label = "%s %s" % (m.group(1), re.sub("[–—−‑]", "-", m.group(2)))
                text = m.group(3)
                if k + 1 < len(rows):
                    text += " " + " ".join(t for a2, t in rows[k + 1] if abs(a2 - at) <= 14 and not LABEL_RE.match(t))
                out.append((label, words_of(v, text)))
    return out


def find_label(v, src, prefix, want, pages):
    """The (page, label) of the table or figure whose printed caption the words `want` say."""
    best, top = None, (0.0, 0)
    for page in pages:
        for label, cap in labels(v, src, page):
            if not label.startswith(prefix) or not cap:
                continue
            hit = len(cap & want)
            score = (hit / len(cap), hit)
            if hit and score > top:
                best, top = (page, label), score
    return best if best and top[0] >= 0.5 else None


def find_heading(v, src, want, pages):
    """The page that prints a heading of these words (an unlabelled table or figure): a short line that
    says nearly only them, or says them with "table" or "figure"."""
    best, top = None, (0.0, 0)
    for page in pages:
        for text in src.pages[page - 1][1].split("\n"):
            low = v.norm(text)
            have = words_of(v, text) - {"table", "figure"}
            hit = len(have & want)
            if not have or len(have) > len(want) + 2 or hit < 1:
                continue
            tagged = bool(re.search(r"\b(table|figure)\b", low))
            if hit < min(2, len(want)) and not tagged:
                continue
            score = (hit / len(have), hit)
            if score[0] >= 0.6 and score > top:
                best, top = page, score
    return best


def cover(v, src, want, pool, pages):
    """The pages (of `pages`) that between them print most of the `want` words."""
    chosen, got = [], set()
    while want - got and len(chosen) < 4:
        best, top = None, 0
        for p in pages:
            if p in chosen:
                continue
            gain = len((want & pool[p - 1]) - got)
            if gain > top:
                best, top = p, gain
        if best is None or top < max(min(2, len(want)), 0.1 * len(want)):
            break
        chosen.append(best)
        got |= want & pool[best - 1]
    return sorted(chosen), (len(got) / len(want) if want else 0)


def fill_file(v, path, dry):
    with open(path, encoding="utf-8") as f:
        original = f.read()
    clean = strip(original)
    tmp = os.path.join(tempfile.mkdtemp(), os.path.basename(path))
    with open(tmp, "w", encoding="utf-8") as f:
        f.write(clean)
    doc = v.Doc(tmp)
    pdf = os.path.join(v.DATASHEETS_DIR, doc.fm["source"]["pdf"])
    src, _ = v.sources(pdf, "auto", os.path.join(tempfile.gettempdir(), "dsvalidate"))
    rep = v.Report(path)
    cat = v.catalog()
    for key in doc.fm.get("devices", []):
        for part, dev, k in cat.values():
            if k == key:
                for pins in dev["pinouts"].values():
                    for pin in pins:
                        doc.known.update(v.tokens(v.norm(pin["name"])))
                break
    v.check_prose(doc, src, rep)
    lines = clean.split("\n")
    n = len(src.pages)
    conv = sorted(doc.converted_pages or range(1, n + 1))
    conv = [p for p in conv if 1 <= p <= n]
    unplaced = []
    append, after = {}, {}          # line index -> citation text appended; line index -> line inserted after
    join_up = set()                 # lines that join the line before

    everything = v.numbered_blocks(src.pages, v.load_blocks(pdf, os.path.join(tempfile.gettempdir(), "dsvalidate")), True)

    def para_of(page, toks, floor=0.3):
        """The number of the paragraph of the page that holds most of the words; a block that is not a
        paragraph (a short list item) gives the paragraph before it, or the one after it."""
        blocks = everything[page - 1]
        at, top = None, floor - 1e-9
        for i, (k, text) in enumerate(blocks):
            share = len(toks & v.Source.block_tokens(text)) / max(1, len(toks))
            if share > top:
                at, top = i, share
        if at is None:
            return None
        order = list(range(at, -1, -1)) + list(range(at + 1, len(blocks)))
        return next((blocks[j][0] for j in order if blocks[j][0] is not None), None)

    # prose
    units_of = {}
    for u, para in doc.unit_para.items():
        units_of.setdefault(para, []).append(u)
    for start, section in doc.paras:
        if re.match(r"(Conversion note|Source error|Source conflict|Unconfirmed):", lines[start - 1].lstrip("-* ")):
            continue
        options = []            # per sentence: the places it may be (a sentence the source prints twice has two)
        for u in units_of.get(start, []):
            w = doc.where.get(u)
            if not w:
                continue
            toks = set(v.content(v.tokens(v.norm(u[1]))))
            places = []
            if w[0] == "sent":
                for sid in w[1]:
                    page, k = src.sents[sid][0], src.sent_para[sid]
                    if k is None:
                        k = para_of(page, toks)
                    if k is not None and (page, k) not in places:
                        places.append((page, k))
            else:
                k = para_of(w[1], toks)
                if k is not None:
                    places.append((w[1], k))
            if places:
                options.append(places)
        # a sentence printed twice is where the others are: the converted pages, and the pages the paragraph already cites
        got = set()
        for places in sorted(options, key=len):
            inside = [x for x in places if x[0] in conv] or places
            seen = {pg for pg, _ in got}
            got.add(next((x for x in inside if x[0] in seen), inside[0]))
        if not got:
            # a caption, or a short item the script does not match: the page that prints its words
            for u in units_of.get(start, []):
                if v.ISSUE_RE.search(u[1]):
                    continue
                toks = v.content(v.tokens(v.norm(u[1])))
                score, lid = src.best_lines(toks) if toks else (0, None)
                if lid is not None and score >= 0.8:
                    page = src.lines[lid][0]
                    k = para_of(page, set(toks), 0.5)
                    if k is not None:
                        got.add((page, k))
                        continue
                    best = find_label(v, src, "", words_of(v, u[1]), [page])
                    if best:
                        got.add(best)
        if not got:
            if any(not v.ISSUE_RE.search(u[1]) for u in units_of.get(start, [])):
                unplaced.append((start, "paragraph \"%s\"" % lines[start - 1][:50]))
            continue
        end = start - 1
        while end + 1 < len(lines) and lines[end + 1].strip() and not lines[end + 1].startswith(("#", "```")) \
                and not re.match(r"^\s*([-*]|\d+\.)\s+", lines[end + 1]):
            end += 1
        if end > start - 1 and re.match(r"^\s*\d+\.(\s|$)", lines[end]):
            join_up.add(end)        # a last line "17." would read as a list item once a citation follows it
        append[end] = "".join(" [[%d::%s]]" % x for x in sorted(got, key=lambda x: (x[0], str(x[1]).zfill(4))))
    # blocks
    used_ordinal = {}
    titles = {}
    for line, kind, attrs, body, section in doc.fences:
        if kind not in CITED or section.lower() in v.SKIP_SECTIONS:
            continue
        close = line
        while close < len(lines) and lines[close].strip() != "```":
            close += 1
        head = ""
        for back in range(line - 2, max(-1, line - 5), -1):
            if lines[back].startswith("#"):
                head = lines[back].lstrip("# ")
                break
        want_words = stem(set(v.content(v.tokens(v.norm(" ".join([attrs.get("caption", ""), head, section])))))) - {"table", "figure"}
        pages = []
        if kind == "table":
            t = next((t for t in doc.tables if t["line"] == line), None)
            nums = v.table_numbers(doc, t) if t else set()
            if len(nums) >= 6:
                pages, share = cover(v, src, nums, src.page_nums, conv)
            elif t:
                words = {x for _, cells in t["rows"] for c in cells for x in v.tokens(v.norm(v.md_plain(c))) if len(x) > 2}
                pages, share = cover(v, src, words, src.page_toks, conv)
        elif kind in ("pins", "ordering"):
            names = set()
            for b in body:
                headcell = b.partition(":")[0] if kind == "pins" else b.split("|")[0]
                names.update(x for x in v.tokens(v.norm(v.md_plain(headcell))) if not x.isdigit() or len(x) > 2)
            pages, share = cover(v, src, names, src.page_toks, conv if kind == "pins" else range(1, n + 1))
        refs = []
        if pages:
            for page in pages:
                cands = [c for c in labels(v, src, page) if c[0].startswith("Table")]
                best = max(cands, key=lambda c: len(c[1] & want_words) / max(1, len(c[1])), default=None)
                if best and best[1] & want_words and len(best[1] & want_words) / max(1, len(best[1])) >= 0.5:
                    refs.append((page, best[0]))
                else:
                    used_ordinal[("table", page)] = used_ordinal.get(("table", page), 0) + 1
                    refs.append((page, "table #%d" % used_ordinal[("table", page)]))
        else:
            caption = set(v.content(v.tokens(v.norm(" ".join([attrs.get("caption", ""), head]))))) - {"figure", "table"}
            caption = stem(caption)
            best = find_label(v, src, "Table" if kind == "table" else "Figure", caption, conv)
            if best:
                refs.append(best)
            else:
                page = find_heading(v, src, caption, conv)
                if page:
                    used_ordinal[(kind, page)] = used_ordinal.get((kind, page), 0) + 1
                    refs.append((page, "%s #%d" % ("table" if kind == "table" else "figure", used_ordinal[(kind, page)])))
        if not refs:
            unplaced.append((line, "%s block \"%s\"" % (kind, attrs.get("caption", head)[:40])))
            continue
        after[close] = " ".join("[[%d::%s]]" % r for r in sorted(refs, key=lambda r: r[0]))
    out = []
    for i, text in enumerate(lines):
        if i in join_up:
            out[-1] += " " + text.strip() + append.get(i, "")
        else:
            out.append(text + append.get(i, ""))
        if i in after:
            out.append(after[i])
    new = "\n".join(out)
    if new != original and not dry:
        with open(path, "w", encoding="utf-8") as f:
            f.write(new)
    return unplaced, new != original


def cmd_fill(argv):
    dry = "--dry" in argv
    v = load("validate")
    status = 0
    for path in [a for a in argv if a != "--dry"]:
        unplaced, changed = fill_file(v, path, dry)
        print("%s: %s%s" % (path, "citations written" if changed and not dry else "citations ok" if not changed else "would change",
                            "; %d not placed: %s" % (len(unplaced), ", ".join("line %d %s" % x for x in unplaced[:8])) if unplaced else ""))
        status |= bool(unplaced)
    return status


def cmd_scans(argv):
    """The citations that sit on a page that is a picture: their numbers are counted by a person."""
    v = load("validate")
    total = 0
    for path in argv:
        doc = v.Doc(path)
        pdf = os.path.join(v.DATASHEETS_DIR, doc.fm["source"]["pdf"])
        src, _ = v.sources(pdf, "auto", os.path.join(tempfile.gettempdir(), "dsvalidate"))
        mine = [(ln, p, r) for ln, p, r, _ in doc.cites if p in src.scans]
        total += len(mine)
        if mine:
            print("%s: %d on picture pages %s" % (path, len(mine), sorted({p for _, p, _ in mine})))
    print("%d citations on picture pages" % total)
    return 0


def main(argv):
    if not argv or argv[0] not in ("list", "fill", "scans"):
        sys.exit(__doc__)
    if argv[0] == "list":
        return cmd_list(argv[1:])
    if argv[0] == "scans":
        return cmd_scans(argv[1:])
    return cmd_fill(argv[1:])


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
