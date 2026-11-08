// datasheet.js: renders a Schematical Markdown datasheet
// (planning/spike-datasheet-format/format.md) into the page.
//
// The app calls renderDatasheet(payload) once the page has loaded:
//   markdown   the .md file's text, front matter included
//   part       the part the window was opened for, or null
//   devices    the catalog devices the front matter names:
//              {kind, id, description, pinouts: {name: [pin]}, parts: [part]}
//   footprints catalog footprints by id, pads expanded:
//              {description, body: {w, h}, round, pads: [{d, x, y, w, h, drill, shape}]}
//   figures    shared figure and sidecar SVG text by ref / src
//
// markdown-it (markdown-it.min.js) renders the prose; WaveDrom
// (wavedrom.min.js) draws the timing diagrams. Everything else is drawn here.

"use strict";

(function () {
  // ---- Sections with fixed anchors (check.py SECTIONS) -------------------

  const SECTION_ANCHORS = {
    "Description": "description",
    "Pinout": "pinout",
    "Pin functions": "pin-functions",
    "Functional description": "functional-description",
    "Programming model": "programming-model",
    "Instruction set": "instruction-set",
    "Memory map": "memory-map",
    "Peripherals": "peripherals",
    "Function tables": "function-tables",
    "Registers and commands": "registers",
    "Absolute maximum ratings": "absolute-maximum-ratings",
    "Recommended operating conditions": "recommended-operating-conditions",
    "Thermal information": "thermal-information",
    "Electrical characteristics": "electrical-characteristics",
    "Switching characteristics": "switching-characteristics",
    "Timing diagrams": "timing-diagrams",
    "Typical characteristics": "typical-characteristics",
    "Parameter measurement information": "parameter-measurement",
    "Application information": "application-information",
    "Package": "package",
    "Ordering information": "ordering",
    "Conversion notes": "conversion-notes",
  };

  const ROLE_NAMES = {
    input: "I", output: "O", bidir: "I/O", power: "Power", ground: "Ground",
    nc: "NC", absent: "", clock: "I", oc: "O (open drain)", tristate: "O (3-state)",
  };

  // ---- Small helpers -----------------------------------------------------

  function esc(s) {
    return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;");
  }

  function slug(title) {
    return title.toLowerCase().replace(/[^\p{L}\p{N}_\- ]/gu, "").replace(/ /g, "-");
  }

  function globMatch(pattern, name) {
    const re = new RegExp("^" + pattern.replace(/[.+^${}()|[\]\\]/g, "\\$&")
      .replace(/\*/g, ".*").replace(/\?/g, ".") + "$");
    return re.test(name);
  }

  function parseAttrs(info) {
    const m = info.trim().match(/^(\S+)\s*(.*)$/);
    const attrs = {};
    if (!m) return { kind: "", attrs };
    const re = /([\w-]+)=("([^"]*)"|\S+)/g;
    let a;
    while ((a = re.exec(m[2])) !== null) attrs[a[1]] = a[3] !== undefined ? a[3] : a[2];
    return { kind: m[1], attrs };
  }

  // A catalog pin name: a trailing "#" is drawn as an overline.
  function pinName(name) {
    return name.endsWith("#") ? '<span class="ov">' + esc(name.slice(0, -1)) + "</span>" : esc(name);
  }

  // ---- Markdown ----------------------------------------------------------

  const md = window.markdownit({ html: false, linkify: false, typographer: false })
    .disable("strikethrough");

  // ~sub~, ^sup^ and !{overline}.
  md.inline.ruler.after("text", "ds_markup", function (state, silent) {
    const src = state.src;
    const pos = state.pos;
    const ch = src[pos];
    let end;
    let type;
    let inner;
    if (ch === "~" || ch === "^") {
      end = src.indexOf(ch, pos + 1);
      if (end < 0 || end === pos + 1) return false;
      inner = src.slice(pos + 1, end);
      if (/\s/.test(inner)) return false;
      type = ch === "~" ? "sub" : "sup";
    } else if (ch === "!" && src[pos + 1] === "{") {
      // Braces nest: !{!{A} + !{B}}.
      let depth = 0;
      end = -1;
      for (let k = pos + 1; k < src.length; k++) {
        if (src[k] === "{") depth++;
        else if (src[k] === "}" && --depth === 0) { end = k; break; }
      }
      if (end < 0) return false;
      inner = src.slice(pos + 2, end);
      type = "ov";
    } else {
      return false;
    }
    if (!silent) {
      const token = state.push("ds_" + type, "", 0);
      token.content = inner;
    }
    state.pos = end + 1;
    return true;
  });
  md.renderer.rules.ds_sub = (tokens, i) => "<sub>" + esc(tokens[i].content) + "</sub>";
  md.renderer.rules.ds_sup = (tokens, i) => "<sup>" + esc(tokens[i].content) + "</sup>";
  md.renderer.rules.ds_ov = (tokens, i) => '<span class="ov">' + md.renderInline(tokens[i].content) + "</span>";

  // Headings get the format's anchors.
  md.core.ruler.push("ds_anchors", function (state) {
    const tokens = state.tokens;
    for (let i = 0; i < tokens.length; i++) {
      if (tokens[i].type !== "heading_open") continue;
      const title = tokens[i + 1].content.trim();
      const id = tokens[i].tag === "h2" && SECTION_ANCHORS[title] ? SECTION_ANCHORS[title] : slug(title);
      tokens[i].attrSet("id", id);
    }
  });

  // Inline text with table-note markers: " [2]" becomes a note reference.
  function inline(text, noteScope) {
    const marks = [];
    const prepared = text.replace(/(^|\s)\[(\d+)\]/g, function (_, before, n) {
      marks.push(n);
      return before + "\u0001" + (marks.length - 1) + "\u0002";
    });
    let html = md.renderInline(prepared);
    html = html.replace(/\u0001(\d+)\u0002/g, function (_, k) {
      const n = marks[Number(k)];
      return noteScope
        ? '<sup class="note-ref"><a href="#' + noteScope + "-note-" + n + '">[' + n + "]</a></sup>"
        : '<sup class="note-ref">[' + n + "]</sup>";
    });
    return html;
  }

  // ---- Context -----------------------------------------------------------

  let ctx = null;
  let waveIndex = 0;
  let tableIndex = 0;

  // The claimed parts, in front-matter order: exact names and globs over the
  // listed devices' parts.
  function claimedParts(front, devices) {
    const all = [];
    for (const dev of devices) {
      for (const p of dev.parts || []) all.push(Object.assign({ device: dev }, p));
    }
    const out = [];
    const seen = new Set();
    for (const pattern of front.parts || []) {
      for (const p of all) {
        if (!seen.has(p.name) && globMatch(pattern, p.name)) {
          seen.add(p.name);
          out.push(p);
        }
      }
    }
    return out;
  }

  // Every pinout of the listed devices: {key, name, pins, devices}. A pinout
  // name can repeat across devices with a different pin order (1G08 and
  // 1G08_DPW both have "p5"), so entries are per device; identical pin lists
  // under one name are one entry.
  function pinouts() {
    const out = [];
    for (const dev of ctx.devices) {
      for (const name of Object.keys(dev.pinouts || {})) {
        const pins = dev.pinouts[name];
        const same = out.find((e) => e.name === name && JSON.stringify(e.pins) === JSON.stringify(pins));
        if (same) same.devices.push(dev);
        else out.push({ key: dev.id + ":" + name, name, pins, devices: [dev] });
      }
    }
    return out;
  }

  function entryFor(device, name) {
    return pinouts().find((e) => e.name === name && e.devices.includes(device)) || null;
  }

  // The pinouts the claimed parts use, in first-use order.
  function usedPinouts() {
    const out = [];
    for (const p of ctx.parts) {
      const e = p.pinout ? entryFor(p.device, p.pinout) : null;
      if (e && !out.some((x) => x.key === e.key)) out.push(e);
    }
    return out;
  }

  // The parts that use a pinout entry: claimed ones first, then any part of
  // its devices.
  function partsFor(entry) {
    const claimed = ctx.parts.filter((p) => p.pinout === entry.name && entry.devices.includes(p.device));
    const others = entry.devices.flatMap((d) => (d.parts || []).filter((p) => p.pinout === entry.name));
    return claimed.concat(others);
  }

  function footprintFor(entry) {
    const p = partsFor(entry).find((x) => x.footprint && ctx.footprints[x.footprint]);
    return p ? ctx.footprints[p.footprint] : null;
  }

  // The package letters from the front matter; when the name is shared by
  // different pin orders, the footprints tell them apart.
  function pinoutLabel(entry) {
    const packages = (ctx.front.packages || {})[entry.name] || entry.name;
    const shared = pinouts().filter((e) => e.name === entry.name).length > 1;
    if (!shared) return packages;
    const fps = [...new Set(partsFor(entry).map((p) => p.footprint).filter(Boolean))];
    return packages + " — " + (fps.length ? fps.join(", ") : entry.devices.map((d) => d.description).join(", "));
  }

  // ---- Fenced blocks -----------------------------------------------------

  function splitRow(line) {
    const s = line.trim();
    if (!s.startsWith("|") || !s.endsWith("|") || s.length < 2) return null;
    return s.slice(1, -1).split("|").map((c) => c.trim());
  }

  function caption(attrs, scope) {
    let html = "";
    if (attrs.caption) html += '<div class="caption">' + inline(attrs.caption, scope) + "</div>";
    if (attrs.conditions) html += '<div class="conditions">' + inline(attrs.conditions, scope) + "</div>";
    return html;
  }

  function renderTable(body, attrs) {
    const scope = attrs.id || "table-" + ++tableIndex;
    const lines = body.split("\n");
    const rows = [];
    let i = 0;
    while (i < lines.length && lines[i].trim()) {
      const cells = splitRow(lines[i]);
      if (cells) rows.push(cells);
      i++;
    }
    const notes = lines.slice(i).map((l) => l.trim()).filter((l) => /^\[\d+\]\s/.test(l));
    const delim = rows.findIndex((r) => r.every((c) => /^:?-+:?$/.test(c)));
    if (delim < 0) return '<pre class="error">Table without a delimiter row</pre>';
    const align = rows[delim].map((c) => (c.startsWith(":") && c.endsWith(":") ? "center"
      : c.endsWith(":") ? "right" : ""));
    const grid = rows.slice(0, delim).concat(rows.slice(delim + 1));
    const headerCount = delim;
    const width = Math.max(...grid.map((r) => r.length));
    // owner[r][c] = the cell [r0, c0] this position merges into.
    const owner = grid.map(() => []);
    for (let r = 0; r < grid.length; r++) {
      for (let c = 0; c < width; c++) {
        const v = grid[r][c] === undefined ? "" : grid[r][c];
        const firstOfPart = r === 0 || r === headerCount;
        if (v === "<" && c > 0) owner[r][c] = owner[r][c - 1];
        else if (v === "^" && !firstOfPart) owner[r][c] = owner[r - 1][c];
        else owner[r][c] = [r, c];
      }
    }
    const span = {};
    for (let r = 0; r < grid.length; r++) {
      for (let c = 0; c < width; c++) {
        const [r0, c0] = owner[r][c];
        const key = r0 + "," + c0;
        const s = span[key] || (span[key] = { rows: 1, cols: 1 });
        s.rows = Math.max(s.rows, r - r0 + 1);
        s.cols = Math.max(s.cols, c - c0 + 1);
      }
    }
    function rowHtml(r, tag) {
      let html = "<tr";
      const cells = grid[r];
      const banner = tag === "td" && cells.length > 1 && cells.slice(1).every((c) => c === "<");
      if (banner) html += ' class="banner"';
      html += ">";
      for (let c = 0; c < width; c++) {
        const [r0, c0] = owner[r][c];
        if (r0 !== r || c0 !== c) continue;
        const s = span[r + "," + c];
        let attrsHtml = "";
        if (s.cols > 1) attrsHtml += ' colspan="' + s.cols + '"';
        if (s.rows > 1) attrsHtml += ' rowspan="' + s.rows + '"';
        if (align[c] && !banner) attrsHtml += ' style="text-align:' + align[c] + '"';
        html += "<" + tag + attrsHtml + ">" + inline(cells[c] || "", scope) + "</" + tag + ">";
      }
      return html + "</tr>";
    }
    let html = "";
    const applies = !attrs.parts || !ctx.part ||
      attrs.parts.split(",").some((g) => globMatch(g.trim(), ctx.part));
    html += '<figure class="ds-table' + (applies ? "" : " not-applicable") + '" id="' + esc(scope) + '">';
    html += caption(attrs, scope);
    if (!applies) {
      html += '<div class="applies">Doesn\'t apply to ' + esc(ctx.part) + " (applies to " +
        esc(attrs.parts) + ")</div>";
    }
    html += '<div class="table-wrap"><table>';
    if (headerCount > 0) {
      html += "<thead>";
      for (let r = 0; r < headerCount; r++) html += rowHtml(r, "th");
      html += "</thead>";
    }
    html += "<tbody>";
    for (let r = headerCount; r < grid.length; r++) html += rowHtml(r, "td");
    html += "</tbody></table></div>";
    if (notes.length) {
      html += '<div class="notes">';
      for (const note of notes) {
        const m = note.match(/^\[(\d+)\]\s+(.*)$/);
        html += '<div class="note" id="' + esc(scope) + "-note-" + m[1] + '"><span class="note-n">[' +
          m[1] + "]</span> " + inline(m[2], null) + "</div>";
      }
      html += "</div>";
    }
    return html + "</figure>";
  }

  function renderPins(body) {
    const used = usedPinouts();
    let html = '<div class="table-wrap"><table class="pins"><thead><tr><th>Pin</th>';
    for (const p of used) html += "<th>" + esc(pinoutLabel(p)) + "</th>";
    html += "<th>Type</th><th>Description</th></tr></thead><tbody>";
    for (const line of body.split("\n")) {
      if (!line.trim()) continue;
      const k = line.indexOf(":");
      const names = line.slice(0, k).split(",").map((s) => s.trim()).filter(Boolean);
      const desc = line.slice(k + 1).trim();
      names.forEach(function (name, index) {
        html += "<tr><td>" + pinName(name) + "</td>";
        let role = "";
        for (const p of used) {
          const pins = p.pins.filter((pin) => pin.name === name);
          if (pins.length && !role) role = pins[0].role;
          html += "<td>" + esc(pins.map((pin) => pin.number !== undefined ? pin.number : pin.ball)
            .join(", ") || "—") + "</td>";
        }
        if (!role) {
          for (const p of pinouts()) {
            const pin = p.pins.find((x) => x.name === name);
            if (pin) { role = pin.role; break; }
          }
        }
        html += "<td>" + esc(ROLE_NAMES[role] !== undefined ? ROLE_NAMES[role] : role) + "</td>";
        if (index === 0) {
          html += '<td rowspan="' + names.length + '">' + inline(desc, null) + "</td>";
        }
        html += "</tr>";
      });
    }
    return html + "</tbody></table></div>";
  }

  // Footprint geometry in mm → an SVG group; returns {svg, box}.
  function footprintSvg(fp, labels, scale) {
    const pads = fp.pads;
    let minX = -fp.body.w / 2;
    let maxX = fp.body.w / 2;
    let minY = -fp.body.h / 2;
    let maxY = fp.body.h / 2;
    for (const p of pads) {
      minX = Math.min(minX, p.x - p.w / 2);
      maxX = Math.max(maxX, p.x + p.w / 2);
      minY = Math.min(minY, p.y - p.h / 2);
      maxY = Math.max(maxY, p.y + p.h / 2);
    }
    // Room for the names: beside pads on the left and right sides, above
    // and below pads on the top and bottom, under pads inside the body.
    // Ball pads ("A1") are labelled in place; other pads by the side of the
    // pad pattern they are on.
    const padX = Math.max(1e-6, ...pads.map((p) => Math.abs(p.x)));
    const padY = Math.max(1e-6, ...pads.map((p) => Math.abs(p.y)));
    function place(p) {
      if (p.d !== null && /^[A-Z]{1,2}\d+$/.test(p.d)) return "inside";
      return Math.abs(p.x) / padX >= Math.abs(p.y) / padY - 1e-6 ? "side" : "topBottom";
    }
    let side = false;
    let topBottom = false;
    let inner = false;
    if (labels) {
      for (const p of pads) {
        const where = place(p);
        if (where === "inside") inner = true;
        else if (where === "side") side = true;
        else topBottom = true;
      }
    }
    const mx = side ? 90 : inner ? 40 : 16;
    const my = topBottom ? 70 : inner ? 28 : 16;
    const W = (maxX - minX) * scale + 2 * mx;
    const H = (maxY - minY) * scale + 2 * my;
    const X = (x) => (x - minX) * scale + mx;
    const Y = (y) => (y - minY) * scale + my;
    let svg = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ' + W.toFixed(1) + " " + H.toFixed(1) +
      '" width="' + W.toFixed(0) + '" height="' + H.toFixed(0) + '" class="drawing">';
    const bw = fp.body.w * scale;
    const bh = fp.body.h * scale;
    if (fp.round) {
      svg += '<ellipse class="body" cx="' + X(0) + '" cy="' + Y(0) + '" rx="' + bw / 2 + '" ry="' + bh / 2 + '"/>';
    } else {
      svg += '<rect class="body" x="' + X(-fp.body.w / 2) + '" y="' + Y(-fp.body.h / 2) + '" width="' + bw +
        '" height="' + bh + '" rx="2"/>';
    }
    for (const p of pads) {
      const cls = p.d === "1" ? "pad pin1" : "pad";
      if (p.shape === "round") {
        svg += '<ellipse class="' + cls + '" cx="' + X(p.x) + '" cy="' + Y(p.y) + '" rx="' + (p.w * scale) / 2 +
          '" ry="' + (p.h * scale) / 2 + '"/>';
      } else {
        svg += '<rect class="' + cls + '" x="' + X(p.x - p.w / 2) + '" y="' + Y(p.y - p.h / 2) + '" width="' +
          p.w * scale + '" height="' + p.h * scale + '"/>';
      }
      if (p.drill > 0) {
        svg += '<circle class="drill" cx="' + X(p.x) + '" cy="' + Y(p.y) + '" r="' + (p.drill * scale) / 2 + '"/>';
      }
      if (labels && p.d !== null && labels[p.d] !== undefined) {
        // Which side of the body the pad is on picks where its name goes.
        const where = place(p);
        const name = labels[p.d];
        if (where === "inside") {
          svg += '<text class="num" x="' + X(p.x) + '" y="' + (Y(p.y) - (p.h * scale) / 2 - 3) +
            '" text-anchor="middle">' + esc(p.d) + "</text>";
          svg += '<text class="name" x="' + X(p.x) + '" y="' + (Y(p.y) + (p.h * scale) / 2 + 11) +
            '" text-anchor="middle">' + svgName(name) + "</text>";
        } else if (where === "side") {
          const left = p.x < 0;
          const ex = left ? X(p.x - p.w / 2) - 4 : X(p.x + p.w / 2) + 4;
          svg += '<text class="name" x="' + ex + '" y="' + (Y(p.y) + 4) + '" text-anchor="' +
            (left ? "end" : "start") + '">' + svgName(name) + "</text>";
          const nx = left ? X(p.x + p.w / 2) + 3 : X(p.x - p.w / 2) - 3;
          svg += '<text class="num" x="' + nx + '" y="' + (Y(p.y) + 4) + '" text-anchor="' +
            (left ? "start" : "end") + '">' + esc(p.d) + "</text>";
        } else {
          const top = p.y < 0;
          const ey = top ? Y(p.y - p.h / 2) - 4 : Y(p.y + p.h / 2) + 4;
          svg += '<text class="name" transform="translate(' + X(p.x) + " " + ey + ') rotate(-90)" x="0" y="4" ' +
            'text-anchor="' + (top ? "start" : "end") + '">' + svgName(name) + "</text>";
          const ny = top ? Y(p.y + p.h / 2) + 10 : Y(p.y - p.h / 2) - 3;
          svg += '<text class="num" x="' + X(p.x) + '" y="' + ny + '" text-anchor="middle">' + esc(p.d) + "</text>";
        }
      }
    }
    return svg + "</svg>";
  }

  function svgName(name) {
    if (name.endsWith("#")) return '<tspan text-decoration="overline">' + esc(name.slice(0, -1)) + "</tspan>";
    return esc(name);
  }

  // Pixels per mm: the drawing about `target` px across, but never so small
  // that neighbouring pads are closer than 22 px (room for a label).
  function scaleFor(fp, target) {
    const size = Math.max(fp.body.w, fp.body.h,
      ...fp.pads.map((p) => Math.max(Math.abs(p.x) * 2, Math.abs(p.y) * 2)));
    let pitch = Infinity;
    const pads = fp.pads;
    for (let i = 0; i < pads.length; i++) {
      for (let j = i + 1; j < pads.length; j++) {
        const d = Math.hypot(pads[i].x - pads[j].x, pads[i].y - pads[j].y);
        if (d > 1e-6) pitch = Math.min(pitch, d);
      }
    }
    const fit = target / size;
    return Math.min(400, Math.max(fit, pitch < Infinity ? 22 / pitch : fit));
  }

  function renderPinout(attrs) {
    const list = attrs.pinout ? pinouts().filter((e) => e.name === attrs.pinout) : usedPinouts();
    if (!list.length) return '<div class="missing">Pinout "' + esc(attrs.pinout || "") + '" not in the catalog</div>';
    let html = "";
    for (const pinout of list) {
      const labels = {};
      for (const pin of pinout.pins) {
        if (pin.role === "absent") continue;
        labels[String(pin.number !== undefined ? pin.number : pin.ball)] = pin.name;
      }
      const fp = footprintFor(pinout);
      html += '<figure class="ds-pinout">';
      if (fp) {
        html += footprintSvg(fp, labels, scaleFor(fp, 220));
      } else {
        html += '<table class="pins"><thead><tr><th>Pin</th><th>Name</th></tr></thead><tbody>';
        for (const [num, name] of Object.entries(labels)) {
          html += "<tr><td>" + esc(num) + "</td><td>" + pinName(name) + "</td></tr>";
        }
        html += "</tbody></table>";
      }
      const label = pinoutLabel(pinout);
      html += '<div class="caption">' + esc(label) + " (top view)</div></figure>";
    }
    return '<div class="pinouts">' + html + "</div>";
  }

  function fmt(mm) {
    return String(Math.round(mm * 1000) / 1000);
  }

  function renderPackage(attrs) {
    const fp = ctx.footprints[attrs.footprint];
    const users = ctx.parts.filter((p) => p.footprint === attrs.footprint).map((p) => p.name);
    if (!fp) return '<div class="missing">Footprint "' + esc(attrs.footprint) + '" not in the catalog</div>';
    let html = '<figure class="ds-package" id="package-' + esc(attrs.footprint.toLowerCase()) + '">';
    html += '<div class="caption">' + esc(attrs.footprint) + ": " + esc(fp.description) + "</div>";
    html += footprintSvg(fp, null, scaleFor(fp, 200));
    const facts = [];
    const pads = fp.pads.filter((p) => p.d !== null);
    if (pads.length) {
      const p1 = pads[0];
      facts.push("Pads: " + pads.length + ", " + fmt(p1.w) + " × " + fmt(p1.h) + " mm" +
        (p1.drill > 0 ? ", drill " + fmt(p1.drill) + " mm" : ""));
      const next = pads.find((p) => p !== p1 && Math.abs(p.x - p1.x) < 1e-6);
      const nextRow = pads.find((p) => p !== p1 && Math.abs(p.y - p1.y) < 1e-6);
      if (next) facts.push("Pitch: " + fmt(Math.abs(next.y - p1.y)) + " mm");
      else if (nextRow) facts.push("Pitch: " + fmt(Math.abs(nextRow.x - p1.x)) + " mm");
      const xs = pads.map((p) => p.x);
      const span = Math.max(...xs) - Math.min(...xs);
      if (span > 0) facts.push("Pad centres, side to side: " + fmt(span) + " mm");
    }
    facts.push("Body: " + fmt(fp.body.w) + " × " + fmt(fp.body.h) + " mm");
    html += '<div class="facts">' + facts.map(esc).join("<br>") + "</div>";
    if (users.length) html += '<div class="used-by">Used by ' + users.map(esc).join(", ") + "</div>";
    return html + "</figure>";
  }

  function renderOrdering(body) {
    const extra = {};
    let vendor = false;
    for (const line of body.split("\n")) {
      if (!line.trim()) continue;
      const cells = line.split("|").map((c) => c.trim());
      extra[cells[0]] = cells.slice(1);
      vendor = true;
    }
    let html = '<div class="table-wrap"><table class="ordering"><thead><tr><th>Part</th><th>Package</th>' +
      "<th>Pinout</th><th>Footprint</th>";
    if (vendor) html += "<th>Packing</th><th>Temperature, °C</th><th>Marking</th>";
    html += "<th>Notes</th></tr></thead><tbody>";
    for (const p of ctx.parts) {
      const flags = (p.flags || []).join(", ");
      const cls = p.name === ctx.part ? ' class="current"' : "";
      html += "<tr" + cls + "><td>" + esc(p.name) + "</td><td>" + esc(p.package || "") + "</td><td>" +
        esc(p.pinout ? (ctx.front.packages || {})[p.pinout] || p.pinout : "") + "</td><td>" + esc(p.footprint || "") + "</td>";
      if (vendor) {
        const e = extra[p.name] || [];
        html += "<td>" + esc(e[0] || "") + "</td><td>" + esc(e[1] || "") + "</td><td>" + esc(e[2] || "") + "</td>";
      }
      html += "<td>" + esc(flags) + "</td></tr>";
    }
    return html + "</tbody></table></div>";
  }

  function renderWavedrom(body, attrs) {
    let html = '<figure class="ds-wave"' + (attrs.id ? ' id="' + esc(attrs.id) + '"' : "") + ">";
    try {
      const source = JSON.parse(body);
      const tree = window.wavedrom.renderAny(waveIndex++, source, window.wavedrom.waveSkin);
      html += '<div class="wave">' + window.wavedrom.onml.stringify(tree) + "</div>";
    } catch (e) {
      html += '<pre class="error">' + esc(String(e)) + "</pre>";
    }
    return html + caption(attrs, attrs.id) + "</figure>";
  }

  function niceTicks(lo, hi, count) {
    const span = hi - lo;
    const raw = span / count;
    const mag = Math.pow(10, Math.floor(Math.log10(raw)));
    const step = [1, 2, 2.5, 5, 10].map((m) => m * mag).find((s) => span / s <= count) || 10 * mag;
    const ticks = [];
    const decimals = Math.max(0, -Math.floor(Math.log10(step)) + 1);
    for (let k = Math.ceil(lo / step - 1e-9); k * step <= hi + step * 1e-6; k++) {
      ticks.push(Number((k * step).toFixed(decimals)));
    }
    return ticks;
  }

  function axisLabel(axis) {
    const label = String(axis.label || "").replace(/_([^\s]+)/g, "~$1~");
    return inlineSvg(label + (axis.unit ? " (" + axis.unit + ")" : ""));
  }

  // ~sub~ in SVG text.
  function inlineSvg(text) {
    return esc(text).replace(/~([^~\s]+)~/g, '<tspan baseline-shift="sub" font-size="75%">$1</tspan>')
      .replace(/!\{([^}]*)\}/g, '<tspan text-decoration="overline">$1</tspan>');
  }

  function renderCurve(body, attrs) {
    let data;
    try {
      data = JSON.parse(body);
    } catch (e) {
      return '<pre class="error">' + esc(String(e)) + "</pre>";
    }
    const W = 520;
    const H = 320;
    const L = 64;
    const R = 16;
    const T = 16;
    const B = 48;
    const xa = data.x;
    const ya = data.y;
    function map(axis, v, a, b) {
      const [lo, hi] = axis.range;
      if (axis.log) return a + ((Math.log10(v) - Math.log10(lo)) / (Math.log10(hi) - Math.log10(lo))) * (b - a);
      return a + ((v - lo) / (hi - lo)) * (b - a);
    }
    function ticks(axis) {
      if (!axis.log) return niceTicks(axis.range[0], axis.range[1], 6);
      const out = [];
      for (let e = Math.floor(Math.log10(axis.range[0])); e <= Math.ceil(Math.log10(axis.range[1])); e++) {
        const v = Math.pow(10, e);
        if (v >= axis.range[0] && v <= axis.range[1]) out.push(v);
      }
      return out;
    }
    const px = (v) => map(xa, v, L, W - R);
    const py = (v) => map(ya, v, H - B, T);
    let svg = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ' + W + " " + H + '" width="' + W +
      '" height="' + H + '" class="chart">';
    for (const t of ticks(xa)) {
      svg += '<line class="grid" x1="' + px(t) + '" y1="' + T + '" x2="' + px(t) + '" y2="' + (H - B) + '"/>';
      svg += '<text x="' + px(t) + '" y="' + (H - B + 16) + '" text-anchor="middle">' + t + "</text>";
    }
    for (const t of ticks(ya)) {
      svg += '<line class="grid" x1="' + L + '" y1="' + py(t) + '" x2="' + (W - R) + '" y2="' + py(t) + '"/>';
      svg += '<text x="' + (L - 6) + '" y="' + (py(t) + 4) + '" text-anchor="end">' + t + "</text>";
    }
    svg += '<rect class="frame" x="' + L + '" y="' + T + '" width="' + (W - L - R) + '" height="' + (H - T - B) + '"/>';
    svg += '<text x="' + (L + W - R) / 2 + '" y="' + (H - 10) + '" text-anchor="middle">' + axisLabel(xa) + "</text>";
    svg += '<text transform="translate(16 ' + (T + H - B) / 2 + ') rotate(-90)" text-anchor="middle">' +
      axisLabel(ya) + "</text>";
    const dashes = ["", "6 3", "2 3", "8 3 2 3", "1 4"];
    (data.series || []).forEach(function (s, k) {
      const pts = s.points.map((p) => px(p[0]).toFixed(1) + "," + py(p[1]).toFixed(1)).join(" ");
      svg += '<polyline class="series s' + (k % 5) + '" points="' + pts + '" stroke-dasharray="' + dashes[k % 5] + '"/>';
      const ly = T + 14 + k * 16;
      svg += '<line class="series s' + (k % 5) + '" x1="' + (W - R - 150) + '" y1="' + (ly - 4) + '" x2="' +
        (W - R - 124) + '" y2="' + (ly - 4) + '" stroke-dasharray="' + dashes[k % 5] + '"/>';
      svg += '<text x="' + (W - R - 118) + '" y="' + ly + '">' + inlineSvg(s.label || "") + "</text>";
    });
    svg += "</svg>";
    return '<figure class="ds-curve"' + (attrs.id ? ' id="' + esc(attrs.id) + '"' : "") + ">" + svg +
      caption(attrs, attrs.id) + "</figure>";
  }

  function renderFigure(body, attrs) {
    const key = attrs.ref || attrs.src;
    const params = {};
    for (const line of body.split("\n")) {
      const k = line.indexOf(":");
      if (k > 0) params[line.slice(0, k).trim()] = line.slice(k + 1).trim();
    }
    let html = '<figure class="ds-figure"' + (attrs.id ? ' id="' + esc(attrs.id) + '"' : "") + ">";
    const svg = ctx.figures[key];
    if (svg) {
      // Fill {KEY} placeholders, then draw ~sub~ in the figure's text.
      let filled = svg.replace(/\{(\w+)\}/g, (_, name) => (params[name] !== undefined ? esc(params[name]) : "—"));
      filled = filled.replace(/(<text[^>]*>)([^<]*)(<\/text>)/g,
        (_, open, text, close) => open + text.replace(/~([^~\s]+)~/g,
          '<tspan baseline-shift="sub" font-size="75%">$1</tspan>') + close);
      html += '<div class="svg">' + filled + "</div>";
    } else {
      html += '<div class="missing">Figure "' + esc(key) + '" is not in the app</div>';
    }
    const listed = Object.entries(params);
    if (listed.length && !svg) {
      html += '<div class="facts">' + listed.map(([k, v]) => esc(k) + " = " + esc(v)).join(", ") + "</div>";
    }
    return html + caption(attrs, attrs.id) + "</figure>";
  }

  function renderEquation(body) {
    return '<div class="equations">' + body.split("\n").filter((l) => l.trim())
      .map((l) => '<div class="equation">' + inline(l, null) + "</div>").join("") + "</div>";
  }

  md.renderer.rules.fence = function (tokens, i) {
    const token = tokens[i];
    const { kind, attrs } = parseAttrs(token.info);
    const body = token.content.replace(/\n$/, "");
    switch (kind) {
      case "table": return renderTable(body, attrs);
      case "pins": return renderPins(body);
      case "pinout": return renderPinout(attrs);
      case "package": return renderPackage(attrs);
      case "ordering": return renderOrdering(body);
      case "wavedrom": return renderWavedrom(body, attrs);
      case "curve": return renderCurve(body, attrs);
      case "figure": return renderFigure(body, attrs);
      case "equation": return renderEquation(body);
      default: return "<pre><code>" + esc(body) + "</code></pre>";
    }
  };

  // ---- Page --------------------------------------------------------------

  function splitFrontMatter(text) {
    const lines = text.split("\n");
    if (lines[0] !== "---") return { front: {}, body: text };
    const end = lines.indexOf("---", 1);
    return {
      front: JSON.parse(lines.slice(1, end).join("\n")),
      body: lines.slice(end + 1).join("\n"),
    };
  }

  function sourceLine(front) {
    const s = front.source || {};
    const bits = [s.vendor, s.document, s.date].filter(Boolean).join(", ");
    return "Converted from " + esc(bits) + (s.pages ? " (" + s.pages + " pages)" : "") +
      (front.converted ? " on " + esc(front.converted) : "") +
      ". Check critical values against the vendor's PDF.";
  }

  function outline(html) {
    const sections = [];
    const re = /<h2 id="([^"]+)">([\s\S]*?)<\/h2>/g;
    let m;
    while ((m = re.exec(html)) !== null) sections.push([m[1], m[2]]);
    return '<nav class="outline">' + sections.map(([id, title]) =>
      '<a href="#' + id + '">' + title + "</a>").join("") + "</nav>";
  }

  window.renderDatasheet = function (payload) {
    const page = document.getElementById("page");
    try {
      const { front, body } = splitFrontMatter(payload.markdown);
      ctx = {
        front,
        part: payload.part || null,
        devices: payload.devices || [],
        footprints: payload.footprints || {},
        figures: payload.figures || {},
      };
      ctx.parts = claimedParts(front, ctx.devices);
      waveIndex = 0;
      tableIndex = 0;
      const html = md.render(body);
      let head = '<div class="source">' + sourceLine(front) + "</div>";
      if (ctx.part) head += '<div class="opened">Opened for ' + esc(ctx.part) + "</div>";
      page.innerHTML = outline(html) + '<article>' + head + html + "</article>";
      document.title = front.title || "";
    } catch (e) {
      page.innerHTML = '<pre class="error">This datasheet could not be shown: ' + esc(String(e)) + "</pre>";
    }
  };
})();
