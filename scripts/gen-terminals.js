#!/usr/bin/env node
// Generates the terminal-block and JST header footprints (plan-45-C, D, E) from scripts/terminal-footprints.json:
//
//   node scripts/gen-terminals.js --letter C --out planning/plan-45-data/footprints-C.json
//       the ready objects of the table's entries of one letter (C fixed terminal blocks, D board-side headers of
//       pluggable blocks, E JST headers), one JSON object per line: `{"footprints": [ ... ]}`.
//   node scripts/gen-terminals.js --write
//       puts into schematical/schematical/Parts/footprints.json the footprints the `connector` parts name
//       (`"footprint"` and `"footprintChoices"` in Parts/connector.json): every line whose id starts with TB- or
//       JST- is replaced by the generated lines (at the end of the array), every other line is kept. Running it
//       twice changes nothing; `footprint_check` never sees an unused TB- / JST- footprint.
//   node scripts/gen-terminals.js --check
//       exits 1 when footprints.json differs from what --write would write.
//
// Test and tooling overrides: --table <file>, --footprints <file>, --parts <connector.json>.
//
// The table: one entry per variant (a footprint family) of planning/plan-45-data/footprints-needed.txt:
//   stem        the line's label (`PHO-COMBICON-PT-5.00-HT`); the id is TB-<stem>-<NN> for letters C and D and
//               <stem>-<NN> for E (its stems start with JST-), NN the position count in two digits
//   letter      "C", "D" or "E"
//   positions   the position counts to write
//   style       "header" (one row of through-hole pads at the pitch, centred on the body) or "explicit" (any of
//               the options below; the footprint is then a list of pads)
//   describe    the description's start; ", <n> positions" is appended
//   note        the provenance: maker, item, file, page, the numbers read
//   pitch, drill, firstRowY, bodyExtra, bodyDepth
//   pad         optional {width, height, shape}; default R5-T: a round pad of diameter drill + 0.7, at most
//               pitch - 0.8 (a copper gap of at least 0.8 mm), rounded to 0.01
//   explicit only:
//   edgeX       distance from the body's left edge to the leftmost column of holes (default: the row centred)
//   holes       [dy, ...] one pad per entry for every pole, dy from the pole's row (several pins per pole)
//   stagger     [dy, ...] pole p's row is firstRowY + stagger[(p - 1) % length] (poles alternate between rows)
//   reverse     true: the poles are numbered from the right
// Body width = n * pitch + bodyExtra, height = bodyDepth. Pin 1's pads are square (rect).

'use strict';

const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const PARTS = path.join(ROOT, 'schematical', 'schematical', 'Parts');

function arg(name, fallback) {
  const i = process.argv.indexOf(name);
  return i >= 0 ? process.argv[i + 1] : fallback;
}
const has = (name) => process.argv.includes(name);

const TABLE_FILE = arg('--table', path.join(__dirname, 'terminal-footprints.json'));
const FOOTPRINT_FILE = arg('--footprints', path.join(PARTS, 'footprints.json'));
const CONNECTOR_FILE = arg('--parts', path.join(PARTS, 'connector.json'));
const PREFIXES = ['TB-', 'JST-'];

const two = (n) => String(n).padStart(2, '0');
const mm = (n) => Number(Number(n).toFixed(4));
const idOf = (v, n) => `${v.letter === 'E' ? '' : 'TB-'}${v.stem}-${two(n)}`;

function padOf(v) {
  if (v.pad) return v.pad;
  const d = Math.round(Math.min(v.drill + 0.7, v.pitch - 0.8) * 100) / 100;
  return { width: d, height: d, shape: 'round' };
}

function buildExplicit(v, n) {
  const pad = padOf(v);
  const width = mm(n * v.pitch + v.bodyExtra);
  const edgeX = v.edgeX !== undefined ? v.edgeX : (width - (n - 1) * v.pitch) / 2;
  const holes = v.holes || [0];
  const stagger = v.stagger || [0];
  const pads = [];
  for (let p = 1; p <= n; p++) {
    const c = v.reverse ? n - p : p - 1;                       // the column from the left
    const x = mm(-width / 2 + edgeX + c * v.pitch);
    const rowY = v.firstRowY + stagger[(p - 1) % stagger.length];
    for (const dy of holes) {
      pads.push({ pin: p, x, y: mm(rowY + dy), width: pad.width, height: pad.height, shape: p === 1 ? 'rect' : pad.shape, drill: v.drill });
    }
  }
  pads.sort((a, b) => a.pin - b.pin || a.y - b.y);
  return {
    id: idOf(v, n),
    description: `${v.describe}, ${n} positions`,
    note: v.note,
    style: 'explicit',
    pins: n,
    body: { width, height: v.bodyDepth },
    pads,
  };
}

function build(v, n) {
  if (v.style === 'explicit') return buildExplicit(v, n);
  const pad = padOf(v);
  return {
    id: idOf(v, n),
    description: `${v.describe}, ${n} positions`,
    note: v.note,
    style: v.style,
    pins: n,
    rows: 1,
    body: { width: mm(n * v.pitch + v.bodyExtra), height: v.bodyDepth },
    pitch: v.pitch,
    firstRowY: v.firstRowY,
    pad,
    drill: v.drill,
    pin1Shape: 'rect',
  };
}

function loadTable() {
  const table = JSON.parse(fs.readFileSync(TABLE_FILE, 'utf8'));
  const variants = table.variants || [];
  const seen = new Set();
  for (const v of variants) {
    for (const k of ['stem', 'letter', 'positions', 'style', 'describe', 'note', 'pitch', 'drill', 'firstRowY', 'bodyExtra', 'bodyDepth']) {
      if (v[k] === undefined) throw new Error(`${v.stem || '?'}: missing "${k}"`);
    }
    if (!['header', 'explicit'].includes(v.style)) throw new Error(`${v.stem}: style ${v.style} is not supported`);
    if (v.style === 'header' && ['edgeX', 'holes', 'stagger', 'reverse'].some((k) => v[k] !== undefined)) {
      throw new Error(`${v.stem}: edgeX, holes, stagger and reverse need style "explicit"`);
    }
    if (!['C', 'D', 'E'].includes(v.letter)) throw new Error(`${v.stem}: letter ${v.letter}`);
    if (!(v.drill > 0 && v.pitch > 0)) throw new Error(`${v.stem}: drill and pitch must be > 0`);
    for (const n of v.positions) {
      const id = idOf(v, n);
      if (!/^[A-Z0-9.-]+$/.test(id)) throw new Error(`id ${id} has characters outside A-Z 0-9 . -`);
      if (seen.has(id)) throw new Error(`duplicate id ${id}`);
      seen.add(id);
      if (!Number.isInteger(n) || n < 1) throw new Error(`${id}: position count ${n}`);
    }
    const pad = padOf(v);
    if (pad.width >= v.pitch || pad.height >= v.pitch) throw new Error(`${v.stem}: pad ${pad.width}x${pad.height} does not fit the pitch ${v.pitch}`);
    if (pad.width <= v.drill || pad.height <= v.drill) throw new Error(`${v.stem}: pad smaller than the drill`);
  }
  return variants;
}

function line(o) {
  return '    ' + JSON.stringify(o);
}

function objectsFor(variants, wanted) {
  const out = [];
  for (const v of variants) for (const n of v.positions) if (!wanted || wanted.has(idOf(v, n))) out.push(build(v, n));
  return out;
}

// The ids the connector parts name as their footprint.
function namedIds() {
  if (!fs.existsSync(CONNECTOR_FILE)) return new Set();
  const text = fs.readFileSync(CONNECTOR_FILE, 'utf8');
  const ids = new Set();
  for (const m of text.matchAll(/"footprint"\s*:\s*"([^"]+)"/g)) if (PREFIXES.some((p) => m[1].startsWith(p))) ids.add(m[1]);
  return ids;
}

function expectedText() {
  const text = fs.readFileSync(FOOTPRINT_FILE, 'utf8');
  const lines = text.split('\n');
  const open = lines.findIndex((l) => l.trim() === '"footprints": [');
  const close = lines.lastIndexOf('  ]');
  if (open < 0 || close < open) throw new Error('footprints.json: no "footprints": [ ... ] block');
  const kept = lines.slice(open + 1, close)
    .map((l) => l.replace(/,$/, ''))
    .filter((l) => !PREFIXES.some((p) => l.startsWith(`    {"id":"${p}`)));
  const variants = loadTable();
  const wanted = namedIds();
  const generated = objectsFor(variants, wanted);
  const have = new Set(generated.map((o) => o.id));
  const unknown = [...wanted].filter((i) => !have.has(i));
  if (unknown.length) throw new Error(`parts name footprints the table does not generate: ${unknown.slice(0, 5).join(', ')}`);
  const entries = kept.concat(generated.map(line));
  const out = [
    ...lines.slice(0, open + 1),
    ...entries.map((l, i) => (i < entries.length - 1 ? l + ',' : l)),
    ...lines.slice(close),
  ].join('\n');
  return { text, out, count: generated.length };
}

function main() {
  if (has('--write') || has('--check')) {
    const { text, out, count } = expectedText();
    if (has('--check')) {
      if (out !== text) { console.error(`gen-terminals: ${path.basename(FOOTPRINT_FILE)} differs from the generated one (run --write)`); process.exit(1); }
      console.log(`ok: ${count} TB-/JST- footprints`);
      return;
    }
    if (out !== text) fs.writeFileSync(FOOTPRINT_FILE, out);
    console.log(`${count} TB-/JST- footprints written`);
    return;
  }
  const letter = arg('--letter');
  const outFile = arg('--out');
  if (!letter || !outFile) { console.error('usage: gen-terminals.js --letter C|D|E --out <file> | --write | --check'); process.exit(2); }
  const variants = loadTable().filter((v) => v.letter === letter);
  const objects = objectsFor(variants);
  const body = objects.map(line).join(',\n');
  fs.writeFileSync(outFile, `{\n  "format": "schematical-footprints",\n  "version": 1,\n  "footprints": [\n${body}\n  ]\n}\n`);
  console.log(`${objects.length} footprints of ${variants.length} variants (letter ${letter}) written`);
}

main();
