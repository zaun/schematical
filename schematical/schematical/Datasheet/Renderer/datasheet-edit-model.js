// datasheet-edit-model.js: the model half of the Verify Datasheet window's inline editor
// (datasheet-edit.js is the editor half; plan-68-T split them).
//
// Plain functions over lines of text (parse a block, serialize it, check it). Run by
// scripts/datasheet-edit.test.js in node. A parser returns null for a block it would not write
// back exactly as it read it; the editor then falls back to raw text. The page loads this file
// before datasheet-edit.js, which reads it as window.dsEditModel; node reads module.exports.

"use strict";

(function (root) {
  const model = {};

  const CITE_SOURCE = "\\[\\[(\\d+)::([^\\[\\]]+)\\]\\]";
  const TRAILING_CITES = new RegExp("((?:[ \\t]*" + CITE_SOURCE + ")+)[ \\t]*$");
  const NOTE = /^\[(\d+)\]\s+(.*)$/;
  const MARKER = /(?<![\w\\])\[(\d+)\]/g;
  const VALUE_COLS = new Set(["min", "typ", "nom", "max", "value"]);
  const SPEC_SECTIONS = new Set(["Absolute maximum ratings", "Recommended operating conditions",
    "Thermal information", "Electrical characteristics", "Switching characteristics"]);
  const NUM_RE = /^[<>≤≥]?\s?[±+-]?\d+(\.\d+)?$/;
  const RATIO_RE = /^\d+(\.\d+)?[/:]\d+(\.\d+)?$/;
  const EXPR_RE = /^[A-Za-z0-9~ .+\-×()/]+$/;
  const RAILS = new Set(["GND", "VCC", "VDD", "VSS", "AVCC", "AREF"]);

  function same(a, b) {
    return a.length === b.length && a.every((v, i) => v === b[i]);
  }

  // ---- Citations --------------------------------------------------------------------

  /** The citations a line ends with, and the line without them. */
  model.splitTrailingCites = function (line) {
    const m = TRAILING_CITES.exec(line);
    if (!m) return { text: line, cites: [] };
    const cites = [];
    const re = new RegExp(CITE_SOURCE, "g");
    let c;
    while ((c = re.exec(m[1])) !== null) cites.push({ page: c[1], ref: c[2] });
    return { text: line.slice(0, m.index).replace(/[ \t]+$/, ""), cites };
  };

  model.citeText = function (cites) {
    return cites.map((c) => "[[" + c.page + "::" + c.ref + "]]").join(" ");
  };

  /** A line that is only citations (the line after a block's closing fence). */
  model.isCiteLine = function (line) {
    const split = model.splitTrailingCites(line);
    return split.cites.length > 0 && split.text === "";
  };

  /** Problems with a list of citations: {errors, warnings}. `pages` is the PDF's page count (or 0). */
  model.checkCites = function (cites, pages) {
    const errors = [];
    const warnings = [];
    for (const c of cites) {
      const where = "Citation " + (c.page || "?") + "::" + (c.ref || "?");
      if (!/^[1-9]\d*$/.test(c.page)) errors.push(where + ": the page must be a number from 1.");
      else if (pages && Number(c.page) > pages) warnings.push(where + ": the PDF has only " + pages + " pages.");
      if (!c.ref) errors.push(where + ": say which paragraph, table or figure.");
      else if (/[\[\]]/.test(c.ref)) errors.push(where + ": no brackets in a reference.");
      else if (!/^(\d+|(Table|Figure) [A-Za-z0-9.\-]+|(table|figure) #\d+)$/.test(c.ref)) {
        warnings.push(where + ": a reference is a paragraph number, a printed label (Table 6-3) or table #2.");
      }
    }
    return { errors, warnings };
  };

  // ---- Fence info strings -----------------------------------------------------------

  function formatAttr(a) {
    const quote = a.quoted || a.value === "" || /\s/.test(a.value);
    return a.key + "=" + (quote ? '"' + a.value + '"' : a.value);
  }

  /** "table id=roc caption="A b"" → {kind, attrs: [{key, value, quoted}]}, or null when writing it back
   *  would not give the same text. */
  model.parseInfo = function (info) {
    const text = info.trim();
    const m = /^(\S+)\s*(.*)$/.exec(text);
    if (!m) return null;
    const attrs = [];
    const re = /([\w-]+)=("([^"]*)"|\S+)/g;
    let a;
    while ((a = re.exec(m[2])) !== null) {
      const quoted = a[3] !== undefined;
      attrs.push({ key: a[1], value: quoted ? a[3] : a[2], quoted });
    }
    const parsed = { kind: m[1], attrs };
    return model.formatInfo(parsed) === text ? parsed : null;
  };

  model.formatInfo = function (p) {
    return [p.kind].concat(p.attrs.map(formatAttr)).join(" ");
  };

  // ---- Lists ------------------------------------------------------------------------

  /** A list whose items are plain paragraphs: {ordered, items: [{marker, gap, indent, lines, cites}]}.
   *  Nested lists, loose lists and odd indents give null. */
  model.parseList = function (lines) {
    const items = [];
    let cur = null;
    for (const line of lines) {
      const m = /^([-*+]|\d+[.)])([ \t]+)(\S.*)$/.exec(line);
      if (m) {
        cur = { marker: m[1], gap: m[2], indents: [], lines: [m[3]], cites: [] };
        items.push(cur);
      } else if (cur && /^[ \t]+\S/.test(line)) {
        const indent = /^[ \t]*/.exec(line)[0];
        if (/^([-*+]|\d+[.)])[ \t]/.test(line.slice(indent.length))) return null;
        cur.indents.push(indent);
        cur.lines.push(line.slice(indent.length));
      } else {
        return null;
      }
    }
    if (!items.length) return null;
    for (const item of items) {
      const last = model.splitTrailingCites(item.lines[item.lines.length - 1]);
      item.cites = last.cites;
      item.lines[item.lines.length - 1] = last.text;
    }
    const list = { ordered: /\d/.test(items[0].marker), start: parseInt(items[0].marker, 10) || 1,
      delim: items[0].marker.slice(-1), items };
    const back = model.serializeList(list);
    return same(back, lines) ? list : null;
  };

  model.serializeList = function (list, renumber) {
    const out = [];
    list.items.forEach((item, index) => {
      const marker = list.ordered && renumber ? (list.start + index) + list.delim : item.marker;
      const fallback = item.indents.length ? item.indents[item.indents.length - 1]
        : " ".repeat(marker.length + item.gap.length);
      const lines = item.lines.slice();
      if (item.cites.length) lines[lines.length - 1] += " " + model.citeText(item.cites);
      lines.forEach((text, k) => out.push(k === 0 ? marker + item.gap + text
        : (item.indents[k - 1] !== undefined ? item.indents[k - 1] : fallback) + text));
    });
    return out;
  };

  // ---- Tables -----------------------------------------------------------------------

  model.splitRow = function (line) {
    const s = line.trim();
    if (!s.startsWith("|") || !s.endsWith("|") || s.length < 2) return null;
    return s.slice(1, -1).split("|").map((c) => c.trim());
  };

  model.formatRow = function (cells) {
    return "|" + cells.map((c) => (c === "" ? " |" : " " + c + " |")).join("");
  };

  /** The lines inside a `table` fence → {headerCount, rows, delim, notes, gap}; null if not exactly
   *  reproducible. A row keeps its original line until its cells change. */
  model.parseTable = function (lines) {
    const rows = [];
    let i = 0;
    while (i < lines.length && lines[i].trim()) {
      const cells = model.splitRow(lines[i]);
      if (!cells) {
        if (NOTE.test(lines[i].trim())) break;
        return null;
      }
      rows.push({ cells, origCells: cells.slice(), orig: lines[i] });
      i++;
    }
    let gap = 0;
    while (i < lines.length && !lines[i].trim()) { gap++; i++; }
    const notes = [];
    for (; i < lines.length; i++) {
      const m = NOTE.exec(lines[i]);
      if (!m) return null;
      notes.push({ n: m[1], text: m[2], origN: m[1], origText: m[2], orig: lines[i] });
    }
    const delimIndex = rows.findIndex((r) => r.cells.every((c) => /^:?-+:?$/.test(c)));
    if (delimIndex < 0) return null;
    const delim = rows.splice(delimIndex, 1)[0];
    const table = { headerCount: delimIndex, rows, delim, notes, gap };
    return same(model.serializeTable(table), lines) ? table : null;
  };

  model.serializeTable = function (t) {
    const rowLine = (r) => (r.orig !== undefined && same(r.cells, r.origCells) ? r.orig : model.formatRow(r.cells));
    const out = [];
    for (let k = 0; k < t.headerCount; k++) out.push(rowLine(t.rows[k]));
    out.push(t.delim.orig !== undefined && same(t.delim.cells, t.delim.origCells)
      ? t.delim.orig : "|" + t.delim.cells.join("|") + "|");
    for (let k = t.headerCount; k < t.rows.length; k++) out.push(rowLine(t.rows[k]));
    if (t.notes.length || t.gap) {
      for (let g = 0; g < t.gap; g++) out.push("");
      for (const n of t.notes) {
        out.push(n.orig !== undefined && n.n === n.origN && n.text === n.origText ? n.orig : "[" + n.n + "] " + n.text);
      }
    }
    return out;
  };

  model.tableWidth = function (t) {
    return Math.max(t.delim.cells.length, ...t.rows.map((r) => r.cells.length));
  };

  function pad(t) {
    const width = model.tableWidth(t);
    for (const r of t.rows) while (r.cells.length < width) r.cells.push("");
    while (t.delim.cells.length < width) t.delim.cells.push("---");
  }

  /** A new empty row at `index` of `rows` (in the header when `header`). */
  model.insertRow = function (t, index, header) {
    pad(t);
    t.rows.splice(index, 0, { cells: new Array(model.tableWidth(t)).fill(""), origCells: null, orig: undefined });
    if (header) t.headerCount++;
  };

  model.deleteRow = function (t, index) {
    if (index < t.headerCount) {
      if (t.headerCount <= 1) return false;
      t.headerCount--;
    }
    t.rows.splice(index, 1);
    return true;
  };

  model.insertColumn = function (t, at) {
    pad(t);
    for (const r of t.rows) r.cells.splice(at, 0, "");
    t.delim.cells.splice(at, 0, "---");
  };

  model.deleteColumn = function (t, at) {
    pad(t);
    if (model.tableWidth(t) <= 1) return false;
    for (const r of t.rows) r.cells.splice(at, 1);
    t.delim.cells.splice(at, 1);
    return true;
  };

  model.alignOf = function (delimCell) {
    if (delimCell.startsWith(":") && delimCell.endsWith(":")) return "center";
    if (delimCell.endsWith(":")) return "right";
    return "left";
  };

  model.delimFor = function (align) {
    return align === "center" ? ":-:" : align === "right" ? "--:" : "---";
  };

  /** Problems with a table, from check.py's rules: {errors, warnings}. `attrs` are the fence's
   *  attributes by key; `section` is the `##` title the table is under. Errors stop Apply. */
  model.checkTable = function (t, attrs, section) {
    const errors = [];
    const warnings = [];
    const width = model.tableWidth(t);
    if (t.headerCount < 1) errors.push("A table needs a header row.");
    t.rows.forEach((r, k) => {
      if (r.cells.some((c) => c.includes("|"))) errors.push("Row " + (k + 1) + ": a cell cannot contain |.");
      if (r.cells.length !== width) warnings.push("Row " + (k + 1) + " has " + r.cells.length + " cells, not " + width + ".");
    });
    // note markers and notes
    const used = new Set();
    const text = [attrs.caption || "", attrs.conditions || ""];
    for (const r of t.rows) text.push(...r.cells);
    for (const s of text) for (const m of s.matchAll(MARKER)) used.add(m[1]);
    const defined = new Set();
    for (const n of t.notes) {
      if (!/^\d+$/.test(n.n)) errors.push("A note number must be a number.");
      else if (defined.has(n.n)) errors.push("Note [" + n.n + "] is there twice.");
      defined.add(n.n);
      if (!n.text.trim()) errors.push("Note [" + n.n + "] is empty.");
    }
    for (const n of [...used].sort()) if (!defined.has(n)) warnings.push("Marker [" + n + "] has no note.");
    for (const n of [...defined].sort()) if (!used.has(n)) warnings.push("Note [" + n + "] has no marker.");
    // merges
    t.rows.forEach((r, k) => {
      if ((k === 0 || k === t.headerCount) && r.cells.includes("^")) {
        warnings.push("Row " + (k + 1) + ": ^ in the first row of the header or body has nothing above it.");
      }
      if (r.cells[0] === "<") warnings.push("Row " + (k + 1) + ": < in the first column has nothing to its left.");
    });
    // spec tables: value cells
    if (SPEC_SECTIONS.has(section) && t.headerCount > 0) {
      const names = (t.rows[t.headerCount - 1].cells || []).map((c) => c.replace(/\s*\[\d+\]/g, "").trim().toLowerCase());
      const valueCols = [];
      names.forEach((n, c) => { if (VALUE_COLS.has(n)) valueCols.push(c); });
      for (let k = t.headerCount; k < t.rows.length; k++) {
        const cells = t.rows[k].cells;
        if (cells.length > 1 && cells.slice(1).every((c) => c === "<")) continue;
        for (const c of valueCols) {
          const v = (cells[c] || "").replace(/\s*\[\d+\]/g, "").trim();
          if (v === "" || v === "^" || v === "<" || NUM_RE.test(v) || RATIO_RE.test(v) || RAILS.has(v) ||
              v === "N/A" || v === "Unlimited" || (EXPR_RE.test(v) && (v.includes("~") || v.includes("×"))) ||
              /^[±+-]?\d+(\.\d+)?\^[-−–]?\d+\^$/.test(v)) continue;
          warnings.push('Row ' + (k + 1) + ': "' + cells[c] + '" in ' + (names[c] || "column " + (c + 1)) +
            " is not a number, ratio or symbol expression.");
        }
      }
    }
    return { errors, warnings };
  };

  // ---- Line grids: `pins` and `ordering` blocks -------------------------------------

  const GRIDS = {
    pins: {
      columns: ["Pin name(s)", "Description"],
      parse(line) {
        const k = line.indexOf(":");
        return k < 0 ? null : [line.slice(0, k).trim(), line.slice(k + 1).trim()];
      },
      format: (cells) => cells[0] + ": " + (cells[1] || ""),
    },
    ordering: {
      columns: ["Part", "Packing", "Temperature, °C", "Marking", "Status"],
      parse: (line) => line.split("|").map((c) => c.trim()),
      format: (cells) => cells.reduce((line, c, k) => (k === 0 ? c : line + (c === "" ? " |" : " | " + c)), ""),
    },
  };
  model.GRIDS = GRIDS;

  /** The lines of a `pins` or `ordering` fence → [{cells, origCells, orig}]; null if a line would not
   *  be written back the same. */
  model.parseGrid = function (kind, lines) {
    const grid = GRIDS[kind];
    const rows = [];
    for (const line of lines) {
      if (!line.trim()) return null;
      const cells = grid.parse(line);
      if (!cells || grid.format(cells) !== line) return null;
      rows.push({ cells, origCells: cells.slice(), orig: line });
    }
    return rows;
  };

  model.serializeGrid = function (kind, rows) {
    const grid = GRIDS[kind];
    return rows.map((r) => {
      if (r.orig !== undefined && same(r.cells, r.origCells)) return r.orig;
      const cells = r.cells.slice();
      // a row typed in the editor does not carry empty cells at its end
      while (kind === "ordering" && cells.length > 1 && cells[cells.length - 1] === "") cells.pop();
      return grid.format(cells);
    });
  };

  // ---- Units ------------------------------------------------------------------------

  /** Where a block's editable unit is in the body (`blocks` from datasheet.js, `lines` the body's
   *  lines): the block itself, and for a fence the line of citations right after it. */
  model.describeUnit = function (lines, blocks, index) {
    const b = blocks[index];
    const unit = { index, l0: b.l0, l1: b.l1, kind: "readonly" };
    if (b.type === "fence") {
      const open = /^(`{3,}|~{3,})(.*)$/.exec(lines[b.l0] || "");
      const close = lines[b.l1 - 1] || "";
      if (!open || b.l1 - b.l0 < 2 || !new RegExp("^" + open[1][0] + "{" + open[1].length + ",}\\s*$").test(close)) return unit;
      unit.kind = "fence";
      unit.fence = open[1];
      unit.info = open[2].trim();
      unit.body = lines.slice(b.l0 + 1, b.l1 - 1);
      unit.closer = close;
      unit.cites = [];
      const next = blocks[index + 1];
      if (next && next.type === "paragraph_open" && next.l0 === b.l1 && next.l1 === next.l0 + 1 &&
          model.isCiteLine(lines[next.l0])) {
        unit.cites = model.splitTrailingCites(lines[next.l0]).cites;
        unit.l1 = next.l1;
      }
      unit.hadCiteLine = unit.l1 !== b.l1;
      return unit;
    }
    // markdown-it's range for a list or quote can run over the blank line after it
    while (unit.l1 > unit.l0 + 1 && !(lines[unit.l1 - 1] || "").trim()) unit.l1--;
    if (b.type === "paragraph_open") {
      const text = lines.slice(unit.l0, unit.l1);
      const last = model.splitTrailingCites(text[text.length - 1]);
      text[text.length - 1] = last.text;
      unit.kind = "prose";
      unit.text = text;
      unit.cites = last.cites;
      return unit;
    }
    if (b.type === "bullet_list_open" || b.type === "ordered_list_open") {
      const list = model.parseList(lines.slice(unit.l0, unit.l1));
      if (list) {
        unit.kind = "list";
        unit.list = list;
        return unit;
      }
    }
    if (b.type === "bullet_list_open" || b.type === "ordered_list_open" || b.type === "blockquote_open" ||
        b.type === "code_block") {
      const text = lines.slice(unit.l0, unit.l1);
      const last = model.splitTrailingCites(text[text.length - 1]);
      text[text.length - 1] = last.text;
      unit.kind = "prose";
      unit.raw = true;
      unit.text = text;
      unit.cites = last.cites;
    }
    return unit;
  };

  /** The editable unit a block belongs to: a line of citations after a fence belongs to the fence. */
  model.ownerIndex = function (lines, blocks, index) {
    const b = blocks[index];
    const prev = blocks[index - 1];
    if (prev && prev.type === "fence" && b.type === "paragraph_open" && b.l0 === prev.l1 && b.l1 === b.l0 + 1 &&
        model.isCiteLine(lines[b.l0])) return index - 1;
    return index;
  };

  /** The `##` title a body line is under. */
  model.sectionAt = function (lines, line) {
    for (let k = Math.min(line, lines.length - 1); k >= 0; k--) {
      const m = /^## (.+)$/.exec(lines[k]);
      if (m) return m[1].trim();
    }
    return "";
  };

  model.same = same;

  if (typeof window === "undefined") {
    root.exports = model;
  } else {
    window.dsEditModel = model;
  }
})(typeof module !== "undefined" ? module : this);
