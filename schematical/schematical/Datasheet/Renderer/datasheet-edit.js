// datasheet-edit.js: the inline editor of the Verify Datasheet window.
//
// It edits a datasheet's Markdown source (planning/spike-datasheet-format/format.md) one block at a
// time, inside the format: a click on a block opens an editor for that block in its place, and Apply
// puts the block's lines back into the file's text and renders the page again. Nothing is written to
// disk; the app keeps the edited text (message "edit", type "state") so a later step can send it on.
//
// Two halves:
//   the model   plain functions over lines of text (parse a block, serialize it, check it). Run by
//               scripts/datasheet-edit.test.js in node. A parser returns null for a block it would not
//               write back exactly as it read it; the editor then falls back to raw text.
//   the editor  the DOM: block editors, citation fields, preview, undo. Needs datasheet.js's window.dsRender.
//
// App → page:  dsEdit.setEditing(bool), dsEdit.undo(), dsEdit.revertAll(), dsEdit.setPdfPage(n)
// Page → app:  "edit" {type: "state", markdown, modified, edited, undo} and {type: "pdfPage"};
//              "cite" {page, ref, text} (the existing handler) to show a citation in the PDF.

"use strict";

(function (root) {
  // =====================================================================================
  // The model
  // =====================================================================================

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

  if (typeof window === "undefined") {
    root.exports = model;
    return;
  }

  // =====================================================================================
  // The editor
  // =====================================================================================

  const R = () => window.dsRender;
  const state = {
    payload: null,
    original: "",
    markdown: "",
    fileLines: [],
    headCount: 0,
    blocks: [],
    undo: [],
    editing: false,
    open: null,
    originalCounts: null,
    pendingPage: null,
  };

  function h(tag, props, ...kids) {
    const el = document.createElement(tag);
    for (const [k, v] of Object.entries(props || {})) {
      if (k === "class") el.className = v;
      else if (k === "text") el.textContent = v;
      else if (k === "value") el.value = v;
      else if (k.startsWith("on")) el.addEventListener(k.slice(2), v);
      else if (v === true) el.setAttribute(k, "");
      else if (v !== false && v != null) el.setAttribute(k, v);
    }
    for (const kid of kids.flat()) if (kid != null) el.append(kid);
    return el;
  }

  function post(message) {
    const handler = window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.edit;
    if (handler) handler.postMessage(message);
  }

  function bodyLines() {
    return state.fileLines.slice(state.headCount);
  }

  function sourcePages() {
    const front = R().ctx().front;
    return (front.source && front.source.pages) || 0;
  }

  // ---- Small widgets ----------------------------------------------------------------

  // The text field the markup buttons act on.
  let lastField = null;

  function track(field) {
    field.addEventListener("focus", () => { lastField = field; });
    return field;
  }

  function insertAtSelection(field, before, after, placeholder) {
    if (!field || !field.isConnected) return;
    const start = field.selectionStart;
    const end = field.selectionEnd;
    const selected = field.value.slice(start, end) || placeholder || "";
    field.setRangeText(before + selected + after, start, end, "select");
    field.setSelectionRange(start + before.length, start + before.length + selected.length);
    field.dispatchEvent(new Event("input", { bubbles: true }));
    field.focus();
  }

  function markupBar() {
    const button = (label, title, run) => h("button", {
      type: "button", class: "ds-ed-tool", title,
      onmousedown: (e) => e.preventDefault(),
      onclick: () => run(lastField),
    }, label);
    const chars = ["µ", "Ω", "°", "±", "×", "≤", "≥", "θ", "Δ", "–"].map((c) =>
      button(c, "Insert " + c, (f) => insertAtSelection(f, c, "", "")));
    return h("div", { class: "ds-ed-tools" },
      button("x₂", "Subscript: ~text~", (f) => insertAtSelection(f, "~", "~", "x")),
      button("x²", "Superscript: ^text^", (f) => insertAtSelection(f, "^", "^", "x")),
      button("x̄", "Overline: !{text}", (f) => insertAtSelection(f, "!{", "}", "x")),
      h("span", { class: "ds-ed-sep" }), chars);
  }

  function autosize(area) {
    const fit = () => { area.style.height = "auto"; area.style.height = area.scrollHeight + 2 + "px"; };
    area.addEventListener("input", fit);
    setTimeout(fit, 0);
    return area;
  }

  /** The citations field: a row per citation with page, reference, and buttons to take the PDF's page,
   *  show the spot in the PDF, and remove it. `getText` gives the text the app looks for on the page. */
  function citesField(cites, onChange, getText) {
    const root = h("div", { class: "ds-cites" });
    function draw() {
      root.textContent = "";
      root.append(h("div", { class: "ds-cites-title" }, "Source citations"));
      cites.forEach((c, i) => {
        const page = track(h("input", {
          class: "ds-cite-page", type: "text", inputmode: "numeric", value: c.page, placeholder: "page", size: 3,
          title: "Page of the PDF, counted from the first page",
          oninput: (e) => { c.page = e.target.value.trim(); onChange(); },
        }));
        const ref = track(h("input", {
          class: "ds-cite-ref", type: "text", value: c.ref, placeholder: "¶ number, Table 6-3, table #2",
          title: "A paragraph number on that page, a printed label (Table 6-3), or table #2 / figure #1",
          oninput: (e) => { c.ref = e.target.value.trim(); onChange(); },
        }));
        root.append(h("div", { class: "ds-cite-row" },
          h("span", { class: "ds-cite-label" }, "page"), page,
          h("button", {
            type: "button", class: "ds-ed-small", title: "Use the page the PDF is showing now",
            onclick: () => {
              state.pendingPage = (n) => { c.page = String(n); draw(); onChange(); };
              post({ type: "pdfPage" });
            },
          }, "Use PDF page"),
          h("span", { class: "ds-cite-label" }, "ref"), ref,
          h("button", {
            type: "button", class: "ds-ed-small", title: "Scroll the PDF to this spot",
            onclick: () => {
              const handler = window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.cite;
              if (handler && /^[1-9]\d*$/.test(c.page) && c.ref) {
                handler.postMessage({ page: Number(c.page), ref: c.ref, text: getText ? getText() : "" });
              }
            },
          }, "Show in PDF"),
          h("button", {
            type: "button", class: "ds-ed-small danger", title: "Remove this citation",
            onclick: () => { cites.splice(i, 1); draw(); onChange(); },
          }, "×")));
      });
      root.append(h("button", {
        type: "button", class: "ds-ed-small",
        onclick: () => { cites.push({ page: "", ref: "" }); draw(); onChange(); root.querySelector(".ds-cite-row:last-of-type input").focus(); },
      }, "Add citation"));
    }
    draw();
    return root;
  }

  // ---- Block editors ----------------------------------------------------------------
  // Each returns {node, read}: read() gives {lines, errors, warnings, preview} for the unit, where
  // `lines` replace the unit's lines in the body. `onChange` asks the frame to read again.

  function proseEditor(unit, onChange) {
    const cites = unit.cites.map((c) => Object.assign({}, c));
    const area = autosize(track(h("textarea", {
      class: "ds-ed-text", rows: Math.max(2, unit.text.length), value: unit.text.join("\n"), spellcheck: "true",
      oninput: onChange,
    })));
    const node = h("div", null,
      unit.raw ? h("div", { class: "ds-ed-note" }, "This block is edited as raw Markdown.") : null,
      markupBar(), area, citesField(cites, onChange, () => area.value));
    return {
      node, focus: () => area.focus(),
      read() {
        const text = area.value.replace(/\s+$/, "");
        const errors = [];
        const warnings = [];
        if (!text.trim()) errors.push("The text is empty.");
        if (!unit.raw && /\n[ \t]*\n/.test(text)) warnings.push("A blank line starts a new paragraph, which gets no citation.");
        const found = model.checkCites(cites, sourcePages());
        const lines = text.split("\n");
        if (cites.length) lines[lines.length - 1] += " " + model.citeText(cites);
        return { lines, errors: errors.concat(found.errors), warnings: warnings.concat(found.warnings),
          preview: text.trim() ? R().md.render(text, { noBlocks: true }) : "" };
      },
    };
  }

  function listEditor(unit, onChange) {
    const list = { ordered: unit.list.ordered, start: unit.list.start, delim: unit.list.delim, items: unit.list.items.map((i) => ({
      marker: i.marker, gap: i.gap, indents: i.indents.slice(), lines: i.lines.slice(), cites: i.cites.map((c) => Object.assign({}, c)),
    })) };
    let renumber = false;
    const itemsNode = h("div", { class: "ds-ed-items" });
    const fields = [];
    function draw() {
      itemsNode.textContent = "";
      fields.length = 0;
      list.items.forEach((item, i) => {
        const area = autosize(track(h("textarea", {
          class: "ds-ed-text", rows: Math.max(1, item.lines.length), value: item.lines.join("\n"), spellcheck: "true",
          oninput: (e) => { item.lines = e.target.value.replace(/\s+$/, "").split("\n"); onChange(); },
        })));
        fields.push(area);
        itemsNode.append(h("div", { class: "ds-ed-item" },
          h("div", { class: "ds-ed-item-head" },
            h("span", { class: "ds-ed-item-n" }, list.ordered ? (i + 1) + "." : "•"),
            h("span", { class: "ds-ed-spacer" }),
            h("button", {
              type: "button", class: "ds-ed-small", title: "Add an item after this one",
              onclick: () => {
                const gap = item.gap;
                list.items.splice(i + 1, 0, { marker: item.marker, gap, indents: item.indents.slice(0, 1), lines: [""], cites: [] });
                renumber = true; draw(); onChange(); fields[i + 1].focus();
              },
            }, "Add item below"),
            h("button", {
              type: "button", class: "ds-ed-small danger", title: "Remove this item",
              onclick: () => { if (list.items.length > 1) { list.items.splice(i, 1); renumber = true; draw(); onChange(); } },
            }, "Remove")),
          area, citesField(item.cites, onChange, () => area.value)));
      });
    }
    draw();
    return {
      node: h("div", null, markupBar(), itemsNode), focus: () => fields[0] && fields[0].focus(),
      read() {
        const errors = [];
        let warnings = [];
        list.items.forEach((item, i) => {
          if (!item.lines.join("").trim()) errors.push("Item " + (i + 1) + " is empty.");
          const found = model.checkCites(item.cites, sourcePages());
          errors.push(...found.errors.map((e) => "Item " + (i + 1) + ": " + e));
          warnings = warnings.concat(found.warnings.map((e) => "Item " + (i + 1) + ": " + e));
        });
        const lines = model.serializeList(list, renumber);
        return { lines, errors, warnings, preview: R().md.render(lines.join("\n"), { noBlocks: true }) };
      },
    };
  }

  function codeEditor(unit, onChange, attrsOnly) {
    const kind = (/^(\S+)/.exec(unit.info) || [, ""])[1];
    const rest = unit.info.slice(kind.length).trim();
    const attrs = track(h("input", { class: "ds-ed-attrs", type: "text", value: rest, oninput: onChange,
      title: "The block's attributes, as written after its type" }));
    const area = autosize(track(h("textarea", {
      class: "ds-ed-text code", rows: Math.max(3, unit.body.length), value: unit.body.join("\n"), spellcheck: "false",
      oninput: onChange,
    })));
    const cites = unit.cites.map((c) => Object.assign({}, c));
    const node = h("div", null,
      h("label", { class: "ds-ed-field" }, h("span", null, kind + " attributes"), attrs),
      attrsOnly ? null : h("div", { class: "ds-ed-note" }, "The block's lines, as written in the file."),
      markupBar(), area, citesField(cites, onChange, () => ""));
    return {
      node, focus: () => area.focus(),
      read() {
        const errors = [];
        const text = area.value.replace(/\s+$/, "");
        const info = (kind + " " + attrs.value.trim()).trim();
        if (!text.trim() && kind !== "pinout" && kind !== "package") errors.push("The block is empty.");
        if (kind === "wavedrom" || kind === "curve") {
          try { JSON.parse(text); } catch (e) { errors.push("Not valid JSON: " + e.message); }
        }
        const found = model.checkCites(cites, sourcePages());
        let preview = "";
        if (!errors.length) {
          const parsed = model.parseInfo(info);
          const attrMap = {};
          for (const a of (parsed ? parsed.attrs : [])) attrMap[a.key] = a.value;
          preview = R().renderBlock(kind, text, attrMap);
        }
        return { info, body: text.split("\n"), cites, errors: errors.concat(found.errors), warnings: found.warnings, preview };
      },
    };
  }

  /** A grid of text inputs over rows of cells: the `pins` and `ordering` blocks. */
  function gridEditor(unit, kind, rows, onChange) {
    const spec = GRIDS[kind];
    const cites = unit.cites.map((c) => Object.assign({}, c));
    const table = h("table", { class: "ds-ed-grid" });
    const inputs = [];
    const ctx = R().ctx();
    const known = new Set();
    for (const dev of ctx.devices) {
      for (const pins of Object.values(dev.pinouts || {})) for (const pin of pins) known.add(pin.name);
    }
    const claimed = new Set(ctx.parts.map((p) => p.name));
    function width() { return Math.max(spec.columns.length, ...rows.map((r) => r.cells.length)); }
    function draw() {
      table.textContent = "";
      inputs.length = 0;
      const w = kind === "pins" ? 2 : width();
      const head = h("tr", null, h("th", null));
      for (let c = 0; c < w; c++) head.append(h("th", null, spec.columns[c] || "Field " + (c + 1)));
      head.append(h("th", null));
      table.append(head);
      rows.forEach((row, r) => {
        while (row.cells.length < w) row.cells.push("");
        const tr = h("tr", null, h("td", { class: "ds-ed-rowtools" },
          h("button", { type: "button", class: "ds-ed-tiny", title: "Move up", onclick: () => move(r, -1) }, "↑"),
          h("button", { type: "button", class: "ds-ed-tiny", title: "Move down", onclick: () => move(r, 1) }, "↓")));
        inputs[r] = [];
        for (let c = 0; c < w; c++) {
          const input = track(h("input", {
            type: "text", class: "ds-ed-cell" + (kind === "pins" && c === 1 ? " wide" : ""), value: row.cells[c],
            oninput: (e) => { row.cells[c] = e.target.value.trim(); onChange(); },
            onkeydown: (e) => navigate(e, r, c),
          }));
          inputs[r][c] = input;
          tr.append(h("td", null, input));
        }
        tr.append(h("td", { class: "ds-ed-rowtools" },
          h("button", { type: "button", class: "ds-ed-tiny", title: "Add a row below", onclick: () => addAfter(r) }, "+"),
          h("button", { type: "button", class: "ds-ed-tiny danger", title: "Delete this row", onclick: () => remove(r) }, "×")));
        table.append(tr);
      });
    }
    function move(r, d) {
      const to = r + d;
      if (to < 0 || to >= rows.length) return;
      rows.splice(to, 0, rows.splice(r, 1)[0]);
      draw(); onChange();
    }
    function addAfter(r) {
      rows.splice(r + 1, 0, { cells: new Array(kind === "pins" ? 2 : width()).fill(""), origCells: null, orig: undefined });
      draw(); onChange(); inputs[r + 1][0].focus();
    }
    function remove(r) {
      if (rows.length > 1) { rows.splice(r, 1); draw(); onChange(); }
    }
    function navigate(e, r, c) {
      if (e.metaKey || e.ctrlKey || e.altKey) return;
      const to = e.key === "ArrowDown" || (e.key === "Enter" && !e.shiftKey) ? r + 1
        : e.key === "ArrowUp" || (e.key === "Enter" && e.shiftKey) ? r - 1 : null;
      if (to === null) return;
      e.preventDefault();
      if (inputs[to] && inputs[to][c]) inputs[to][c].focus();
    }
    draw();
    const node = h("div", null, h("div", { class: "ds-ed-grid-wrap" }, table),
      h("button", { type: "button", class: "ds-ed-small", onclick: () => addAfter(rows.length - 1) }, "Add row"),
      citesField(cites, onChange, () => ""));
    return {
      node, focus: () => inputs[0] && inputs[0][0].focus(),
      read() {
        const errors = [];
        const warnings = [];
        rows.forEach((row, r) => {
          if (!row.cells[0]) errors.push("Row " + (r + 1) + " has no " + (kind === "pins" ? "pin name" : "part") + ".");
          if (kind === "ordering" && row.cells.some((c) => c.includes("|"))) errors.push("Row " + (r + 1) + ": a cell cannot contain |.");
          if (kind === "pins") {
            if (!row.cells[1]) warnings.push("Row " + (r + 1) + " has no description.");
            if (known.size) {
              for (const name of row.cells[0].split(",").map((s) => s.trim()).filter(Boolean)) {
                if (!known.has(name)) warnings.push('"' + name + '" is not a pin name of the catalog parts.');
              }
            }
          } else if (claimed.size && row.cells[0] && !claimed.has(row.cells[0])) {
            warnings.push('"' + row.cells[0] + '" is not one of the parts this datasheet claims.');
          }
        });
        const found = model.checkCites(cites, sourcePages());
        const body = model.serializeGrid(kind, rows);
        return { info: unit.info, body, cites, errors: errors.concat(found.errors), warnings: warnings.concat(found.warnings),
          preview: R().renderBlock(kind, body.join("\n"), {}) };
      },
    };
  }

  function tableEditor(unit, t, info, section, onChange) {
    const cites = unit.cites.map((c) => Object.assign({}, c));
    const attrs = info.attrs.map((a) => Object.assign({}, a));
    const attr = (key) => (attrs.find((a) => a.key === key) || { value: "" }).value;
    function setAttr(key, value) {
      const found = attrs.find((a) => a.key === key);
      if (found) {
        if (value === "") attrs.splice(attrs.indexOf(found), 1);
        else found.value = value;
      } else if (value !== "") {
        attrs.push({ key, value, quoted: false });
      }
    }
    const field = (key, label, placeholder) => h("label", { class: "ds-ed-field" }, h("span", null, label),
      track(h("input", { type: "text", value: attr(key), placeholder, oninput: (e) => { setAttr(key, e.target.value.trim()); onChange(); } })));
    const others = attrs.filter((a) => !["id", "caption", "conditions", "parts"].includes(a.key));
    const head = h("div", { class: "ds-ed-attrs-box" },
      h("div", { class: "ds-ed-id" }, attr("id") ? "id: " + attr("id") : "no id"),
      field("caption", "Caption", ""), field("conditions", "Conditions", "Over operating free-air temperature range…"),
      field("parts", "Applies to parts", "SN74*, SN54*"),
      others.map((a) => field(a.key, a.key, "")));

    const grid = h("table", { class: "ds-ed-grid" });
    const cellInputs = [];
    function sizeOf(v) { return Math.min(40, Math.max(3, v.length + 1)); }
    function draw() {
      grid.textContent = "";
      cellInputs.length = 0;
      const w = model.tableWidth(t);
      const tools = h("tr", { class: "ds-ed-coltools" }, h("td", null));
      for (let c = 0; c < w; c++) {
        const aligns = ["left", "right", "center"];
        const letter = { left: "L", right: "R", center: "C" };
        const current = () => model.alignOf(t.delim.cells[c] || "---");
        const align = h("button", {
          type: "button", class: "ds-ed-tiny", title: "Column alignment: " + current() + " (click to change)",
          onclick: (e) => {
            const next = aligns[(aligns.indexOf(current()) + 1) % 3];
            while (t.delim.cells.length <= c) t.delim.cells.push("---");
            t.delim.cells[c] = model.delimFor(next);
            e.target.textContent = letter[next];
            e.target.title = "Column alignment: " + next + " (click to change)";
            onChange();
          },
        }, letter[current()]);
        tools.append(h("td", null,
          h("button", { type: "button", class: "ds-ed-tiny", title: "Add a column before this one", onclick: () => { model.insertColumn(t, c); draw(); onChange(); } }, "+←"),
          h("button", { type: "button", class: "ds-ed-tiny", title: "Add a column after this one", onclick: () => { model.insertColumn(t, c + 1); draw(); onChange(); } }, "→+"),
          h("button", { type: "button", class: "ds-ed-tiny danger", title: "Delete this column", onclick: () => { if (model.deleteColumn(t, c)) { draw(); onChange(); } } }, "×"),
          align));
      }
      tools.append(h("td", null));
      grid.append(tools);
      t.rows.forEach((row, r) => {
        const header = r < t.headerCount;
        const tr = h("tr", { class: header ? "header" : "" });
        tr.append(h("td", { class: "ds-ed-rowtools" },
          h("button", { type: "button", class: "ds-ed-tiny", title: "Add a row above", onclick: () => { model.insertRow(t, r, header); draw(); onChange(); cellInputs[r][0].focus(); } }, "+↑"),
          h("button", { type: "button", class: "ds-ed-tiny", title: "Add a row below", onclick: () => { model.insertRow(t, r + 1, header); draw(); onChange(); cellInputs[r + 1][0].focus(); } }, "+↓")));
        cellInputs[r] = [];
        for (let c = 0; c < w; c++) {
          const value = row.cells[c] === undefined ? "" : row.cells[c];
          const input = track(h("input", {
            type: "text", class: "ds-ed-cell" + (value === "^" || value === "<" ? " merge" : ""),
            value, size: sizeOf(value), spellcheck: "false",
            oninput: (e) => {
              while (row.cells.length <= c) row.cells.push("");
              row.cells[c] = e.target.value.trim();
              e.target.size = sizeOf(e.target.value);
              e.target.classList.toggle("merge", row.cells[c] === "^" || row.cells[c] === "<");
              onChange();
            },
            onkeydown: (e) => navigate(e, r, c),
          }));
          cellInputs[r][c] = input;
          tr.append(h("td", null, input));
        }
        tr.append(h("td", { class: "ds-ed-rowtools" },
          h("button", { type: "button", class: "ds-ed-tiny danger", title: header && t.headerCount <= 1 ? "A table needs a header row" : "Delete this row",
            onclick: () => { if (model.deleteRow(t, r)) { draw(); onChange(); } } }, "×")));
        grid.append(tr);
        if (r === t.headerCount - 1) {
          grid.append(h("tr", { class: "ds-ed-delim" }, h("td", { colspan: w + 2 }, "header above, body below")));
        }
      });
    }
    function navigate(e, r, c) {
      if (e.metaKey || e.ctrlKey || e.altKey) return;
      const to = e.key === "ArrowDown" || (e.key === "Enter" && !e.shiftKey) ? r + 1
        : e.key === "ArrowUp" || (e.key === "Enter" && e.shiftKey) ? r - 1 : null;
      if (to === null) return;
      e.preventDefault();
      if (cellInputs[to] && cellInputs[to][c]) cellInputs[to][c].focus();
    }
    draw();

    const notesNode = h("div", { class: "ds-ed-notes" });
    function drawNotes() {
      notesNode.textContent = "";
      notesNode.append(h("div", { class: "ds-cites-title" }, "Notes under the table"));
      t.notes.forEach((n, i) => {
        notesNode.append(h("div", { class: "ds-ed-noterow" },
          h("span", null, "["),
          track(h("input", { class: "ds-ed-noten", type: "text", value: n.n, size: 2, oninput: (e) => { n.n = e.target.value.trim(); onChange(); } })),
          h("span", null, "]"),
          track(h("input", { class: "ds-ed-notetext", type: "text", value: n.text, oninput: (e) => { n.text = e.target.value.trim(); onChange(); } })),
          h("button", { type: "button", class: "ds-ed-tiny danger", title: "Delete this note",
            onclick: () => { t.notes.splice(i, 1); if (!t.notes.length) t.gap = 0; drawNotes(); onChange(); } }, "×")));
      });
      notesNode.append(h("button", {
        type: "button", class: "ds-ed-small",
        onclick: () => {
          if (!t.notes.length) t.gap = Math.max(t.gap, 1);
          const next = Math.max(0, ...t.notes.map((n) => parseInt(n.n, 10) || 0)) + 1;
          t.notes.push({ n: String(next), text: "", origN: undefined, origText: undefined, orig: undefined });
          drawNotes(); onChange();
          notesNode.querySelector(".ds-ed-noterow:last-of-type .ds-ed-notetext").focus();
        },
      }, "Add note"));
    }
    drawNotes();

    const node = h("div", null, head,
      h("div", { class: "ds-ed-note" }, "^ merges a cell with the one above, < with the one to its left. Write a note marker as [1]."),
      markupBar(), h("div", { class: "ds-ed-grid-wrap" }, grid), notesNode, citesField(cites, onChange, () => ""));
    return {
      node, focus: () => cellInputs[0] && cellInputs[0][0].focus(),
      read() {
        const attrMap = {};
        for (const a of attrs) attrMap[a.key] = a.value;
        const found = model.checkTable(t, attrMap, section);
        const cite = model.checkCites(cites, sourcePages());
        const errors = found.errors.concat(cite.errors);
        if (attrs.some((a) => a.value.includes('"'))) errors.push('An attribute cannot contain ".');
        const previewAttrs = Object.assign({}, attrMap);
        delete previewAttrs.parts;
        const body = model.serializeTable(t);
        return {
          info: model.formatInfo({ kind: "table", attrs }), body, cites, errors, warnings: found.warnings.concat(cite.warnings),
          preview: errors.length ? "" : R().renderBlock("table", body.join("\n"), previewAttrs).replace(/\sid="[^"]*"/g, ""),
        };
      },
    };
  }

  /** Prose, lists and every fence but the two generated from the catalog can be edited. */
  function isEditable(unit) {
    if (unit.kind === "prose" || unit.kind === "list") return true;
    if (unit.kind !== "fence") return false;
    const kind = (/^(\S+)/.exec(unit.info) || [, ""])[1];
    return kind !== "pinout" && kind !== "package";
  }

  /** The editor for a unit (null when it is read-only). */
  function makeEditor(unit, onChange) {
    if (!isEditable(unit)) return null;
    if (unit.kind === "prose") return { title: unit.raw ? "Edit block (raw Markdown)" : "Edit paragraph", ed: proseEditor(unit, onChange) };
    if (unit.kind === "list") return { title: "Edit list", ed: listEditor(unit, onChange) };
    if (unit.kind !== "fence") return null;
    const kind = (/^(\S+)/.exec(unit.info) || [, ""])[1];
    if (kind === "table") {
      const t = model.parseTable(unit.body);
      const info = model.parseInfo(unit.info);
      if (t && info) {
        const section = model.sectionAt(bodyLines(), unit.l0);
        return { title: "Edit table", ed: tableEditor(unit, t, info, section, onChange) };
      }
    }
    if (kind === "pins" || kind === "ordering") {
      const rows = model.parseGrid(kind, unit.body);
      if (rows) return { title: kind === "pins" ? "Edit pin descriptions" : "Edit ordering information", ed: gridEditor(unit, kind, rows, onChange) };
    }
    return { title: "Edit " + kind + " block", ed: codeEditor(unit, onChange) };
  }

  // ---- The editor frame -------------------------------------------------------------

  function unitLines(unit) {
    const l = bodyLines();
    return l.slice(unit.l0, unit.l1);
  }

  function elementsOf(unit) {
    return [...document.querySelectorAll("#page [data-l0]")].filter((el) => {
      const l0 = Number(el.dataset.l0);
      return l0 >= unit.l0 && l0 < unit.l1;
    });
  }

  function closeEditor() {
    const open = state.open;
    if (!open) return;
    open.frame.remove();
    for (const el of open.hidden) el.style.display = "";
    state.open = null;
  }

  function openEditor(index) {
    const lines = bodyLines();
    const owner = model.ownerIndex(lines, state.blocks, index);
    const unit = model.describeUnit(lines, state.blocks, owner);
    if (unit.kind === "readonly") return;
    if (state.open) {
      if (state.open.dirty()) {
        state.open.frame.classList.remove("flash");
        void state.open.frame.offsetWidth;
        state.open.frame.classList.add("flash");
        return;
      }
      closeEditor();
    }
    const original = unitLines(unit);
    let result = null;
    const messages = h("div", { class: "ds-ed-msgs" });
    const preview = h("div", { class: "ds-ed-preview" });
    const apply = h("button", { type: "button", class: "ds-ed-apply", onclick: () => commit() }, "Apply");
    let timer = null;

    function newLines(r) {
      if (unit.kind === "fence") {
        const out = [unit.fence + r.info, ...r.body, unit.closer];
        if (r.cites.length) out.push(model.citeText(r.cites));
        return out;
      }
      return r.lines;
    }

    function refresh() {
      result = made.ed.read();
      messages.textContent = "";
      for (const e of result.errors) messages.append(h("div", { class: "ds-ed-error" }, e));
      for (const w of result.warnings) messages.append(h("div", { class: "ds-ed-warning" }, w));
      const lines = newLines(result);
      const dirty = !same(lines, original);
      apply.disabled = result.errors.length > 0 || !dirty;
      apply.title = result.errors.length ? "Fix the errors first" : dirty ? "Put the change in the datasheet (Cmd-Return)" : "Nothing changed";
      preview.textContent = "";
      if (result.preview) {
        preview.append(h("div", { class: "ds-cites-title" }, "Preview"));
        const box = h("div", { class: "ds-ed-preview-box" });
        box.innerHTML = result.preview;
        preview.append(box);
      }
    }
    function onChange() {
      clearTimeout(timer);
      timer = setTimeout(refresh, 90);
    }
    function commit() {
      refresh();
      if (result.errors.length) return;
      const lines2 = newLines(result);
      if (same(lines2, original)) { cancel(); return; }
      closeEditor();
      state.undo.push(state.markdown);
      state.fileLines.splice(state.headCount + unit.l0, unit.l1 - unit.l0, ...lines2);
      rerender(state.fileLines.join("\n"));
    }
    function cancel() { closeEditor(); }

    const made = makeEditor(unit, onChange);
    if (!made) return;
    const frame = h("div", { class: "ds-editor" },
      h("div", { class: "ds-ed-bar" },
        h("span", { class: "ds-ed-title" }, made.title),
        h("span", { class: "ds-ed-spacer" }),
        h("button", { type: "button", class: "ds-ed-cancel", title: "Discard this edit (Esc)", onclick: cancel }, "Cancel"),
        apply),
      made.ed.node, messages, preview);
    frame.addEventListener("keydown", (e) => {
      if (e.key === "Escape") { e.preventDefault(); cancel(); }
      else if (e.key === "Enter" && (e.metaKey || e.ctrlKey)) { e.preventDefault(); commit(); }
    });
    const hidden = elementsOf(unit);
    hidden[0].before(frame);
    for (const el of hidden) el.style.display = "none";
    state.open = { frame, hidden, dirty: () => !!result && !same(newLines(result), original) };
    refresh();
    frame.scrollIntoView({ block: "nearest" });
    if (made.ed.focus) made.ed.focus();
  }

  // ---- Page, undo, state ------------------------------------------------------------

  function rerender(markdown) {
    const y = window.scrollY;
    state.open = null;
    window.renderDatasheet(Object.assign({}, state.payload, { markdown, editing: state.editing, rerender: true }));
    window.scrollTo(0, y);
  }

  function originalCounts() {
    if (!state.originalCounts) {
      const lines = state.original.split("\n");
      const head = lines[0] === "---" && lines.indexOf("---", 1) > 0 ? lines.indexOf("---", 1) + 1 : 0;
      const body = lines.slice(head);
      const counts = new Map();
      for (const b of R().parseBlocks(body.join("\n"))) {
        const text = body.slice(b.l0, b.l1).join("\n");
        counts.set(text, (counts.get(text) || 0) + 1);
      }
      state.originalCounts = counts;
    }
    return new Map(state.originalCounts);
  }

  /** Marks the blocks that differ from the file as it was loaded, and which blocks can be edited. */
  function decorate() {
    const lines = bodyLines();
    const counts = originalCounts();
    const edited = new Set();
    state.blocks.forEach((b, index) => {
      const text = lines.slice(b.l0, b.l1).join("\n");
      const owner = model.ownerIndex(lines, state.blocks, index);
      const el = document.querySelector('#page [data-l0="' + b.l0 + '"]');
      if (!el) return;
      const left = counts.get(text) || 0;
      if (left > 0) counts.set(text, left - 1);
      else { el.classList.add("ds-edited"); edited.add(owner); }
      if (isEditable(model.describeUnit(lines, state.blocks, owner))) {
        el.classList.add("ds-editable");
        if (!el.title) el.title = "Click to edit";
      }
    });
    return edited.size;
  }

  function postState(edited) {
    post({
      type: "state", path: state.payload ? state.payload.path : "", markdown: state.markdown, modified: state.markdown !== state.original,
      edited, undo: state.undo.length, editing: state.editing,
    });
  }

  function onClick(event) {
    if (!state.editing) return;
    const target = event.target;
    if (target.closest("a, button, input, textarea, select, .ds-editor")) return;
    const el = target.closest("[data-l0]");
    if (!el) return;
    const index = state.blocks.findIndex((b) => b.l0 === Number(el.dataset.l0));
    if (index >= 0) openEditor(index);
  }

  const api = {
    /** Called by datasheet.js after it renders: payload, {front, blocks}. */
    rendered(payload, info) {
      if (!payload.rerender) {
        state.payload = payload;
        state.original = payload.original || payload.markdown;
        state.originalCounts = null;
        state.undo = [];
        state.editing = !!payload.editing;
      }
      state.open = null;
      state.markdown = payload.markdown;
      state.fileLines = payload.markdown.split("\n");
      state.headCount = state.fileLines[0] === "---" && state.fileLines.indexOf("---", 1) > 0
        ? state.fileLines.indexOf("---", 1) + 1 : 0;
      state.blocks = info.blocks;
      const page = document.getElementById("page");
      if (!page.dataset.editHooked) {
        page.addEventListener("click", onClick);
        page.dataset.editHooked = "1";
      }
      const edited = state.editing ? decorate() : 0;
      postState(edited);
    },
    setEditing(on) {
      closeEditor();
      state.editing = !!on;
      rerender(state.markdown);
    },
    undo() {
      closeEditor();
      if (!state.undo.length) return;
      const previous = state.undo.pop();
      rerender(previous);
    },
    revertAll() {
      closeEditor();
      if (state.markdown === state.original) return;
      state.undo.push(state.markdown);
      rerender(state.original);
    },
    setPdfPage(n) {
      const done = state.pendingPage;
      state.pendingPage = null;
      if (done) done(n);
    },
  };
  window.dsEdit = api;
})(typeof module !== "undefined" ? module : this);
