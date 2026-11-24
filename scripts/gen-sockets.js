#!/usr/bin/env node
// Generates the IC socket footprints (plan-51-D: DIP; plan-51-E: PLCC) from
// planning/plan-51-data/series.tsv, and (plan-51-F) the socket parts.
//
//   node scripts/gen-sockets.js --letter D --out planning/plan-51-data/footprints-D.json
//
// One `dual` object per (row, pin count) of a DIP row of series.tsv (the land-pattern facts
// plan-51-C read from the makers' drawings). Rows whose objects are equal in every key but
// id, description and note share the first one's id: no two footprints have the same pads.
//
// Options:
//   --letter D|E      D: the DIP rows (`dual`); E: the PLCC rows (`quad` with `slots`, pin 1 top centre like the
//                     chip footprints PLCC-28, -32, -44, -68)
//   --out FILE        the side file written ({"format","version","footprints"}); plan-51-F merges it
//   --series FILE     series.tsv (default planning/plan-51-data/series.tsv; the tests give a fixture)
//   --skip-generic-check   for fixtures: do not require the generic DIP shapes (see below)
//
// Exit 1, writing nothing, when a DIP shape of fits.tsv (8, 14, 16, 20, 24, 28 pins at 0.3 in;
// 24, 28, 32, 40, 48 at 0.6 in) is not supplied exactly once by a through-hole row whose
// `generic` cell says `yes` (or lists the pin count), or (letter E) a PLCC shape (28, 32, 44, 68)
// is not supplied exactly once by a PLCC row's `generic` cell: the generic sockets (plan-51-F) need them.

'use strict';

const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const DEFAULT_SERIES = path.join(ROOT, 'planning', 'plan-51-data', 'series.tsv');

const MAKER_CODE = { 'Mill-Max': 'MM', 'Preci-Dip': 'PD', 'Assmann': 'AS' };
const DIP_SHAPES = { '0.3': [8, 14, 16, 20, 24, 28], '0.6': [24, 28, 32, 40, 48] };
const ROW_SPACING_MM = { '0.3': 7.62, '0.6': 15.24 };
const ROW_SPACING_ID = { '0.3': '300', '0.6': '600' };

function readTsv(file) {
  const lines = fs.readFileSync(file, 'utf8').split('\n').filter((l) => l.trim() !== '');
  const header = lines[0].split('\t');
  return lines.slice(1).map((line, i) => {
    const cells = line.split('\t');
    if (cells.length !== header.length) throw new Error(`${file}: line ${i + 2} has ${cells.length} cells, expected ${header.length}`);
    return Object.fromEntries(header.map((h, c) => [h, cells[c]]));
  });
}

// `8:10.16;14:17.78` -> { 8: 10.16, 14: 17.78 }
function perPin(text, what) {
  const out = {};
  for (const part of text.split(';')) {
    const [n, v] = part.split(':');
    if (!/^\d+$/.test(n) || !Number.isFinite(Number(v))) throw new Error(`${what}: "${part}" is not N:number`);
    out[Number(n)] = Number(v);
  }
  return out;
}

const idSeries = (series) => series.replace(/_/g, '-');

function dipId(row, n) {
  const mounting = row.mounting === 'Through Hole' ? 'TH' : 'SM';
  return `SKT-DIP${n}-${ROW_SPACING_ID[row.spacing]}-${mounting}-${MAKER_CODE[row.maker]}-${idSeries(row.series)}`;
}

// The `dual` object of one pin count of a DIP row.
function dipFootprint(row, n) {
  if (!MAKER_CODE[row.maker]) throw new Error(`no maker code for ${row.maker}`);
  if (!ROW_SPACING_ID[row.spacing]) throw new Error(`${row.maker} ${row.series}: spacing ${row.spacing} is not 0.3 or 0.6`);
  const lengths = perPin(row.body_len, `${row.maker} ${row.series} body_len`);
  if (!(n in lengths)) throw new Error(`${row.maker} ${row.series}: no body_len for ${n} pins`);
  const through = row.mounting === 'Through Hole';
  const padW = Number(row.pad_w);
  const padH = Number(row.pad_h);
  const [file, page] = row.drawing.split(' p.');
  const fp = {
    id: dipId(row, n),
    description: `${n}-pin DIP socket, ${row.maker} ${row.series}, ${through ? 'through hole' : 'surface mount'}, ${ROW_SPACING_MM[row.spacing]} mm row spacing`,
    note: `${row.maker} ${file} p.${page}: ${through ? `hole Ø${Number(row.drill)}, ` : ''}pad ${padW === padH ? `Ø${padW}` : `${padW} x ${padH}`}, `
      + `${through ? 'row spacing' : 'pad-centre span'} ${Number(row.span)}, body ${Number(row.body_w)} x ${lengths[n]}; ${row.rule}`,
    style: 'dual',
    pins: n,
    body: { width: Number(row.body_w), height: lengths[n] },
    pitch: Number(row.pitch),
    span: Number(row.span),
    pad: { width: padW, height: padH, shape: through || padW === padH ? 'round' : 'rect' },
  };
  if (through) {
    fp.drill = Number(row.drill);
    fp.pin1Shape = 'rect';
  }
  return fp;
}

const PLCC_SHAPES = [28, 32, 44, 68];

// `32:9,7,9,7` -> { 32: [9, 7, 9, 7] }: the slots per side, left, bottom, right, top.
function sidesPerPin(text, what) {
  const out = {};
  for (const part of text.split(';')) {
    const [n, v] = part.split(':');
    const sides = (v || '').split(',').map(Number);
    if (!/^\d+$/.test(n) || sides.length !== 4 || sides.some((s) => !Number.isInteger(s) || s < 0)) {
      throw new Error(`${what}: "${part}" is not N:left,bottom,right,top`);
    }
    out[Number(n)] = sides;
  }
  return out;
}

// `32:11.02/13.56` -> { 32: [11.02, 13.56] }: spanX / spanY per pin count.
function spansPerPin(text, what) {
  const out = {};
  for (const part of text.split(';')) {
    const [n, v] = part.split(':');
    const xy = (v || '').split('/').map(Number);
    if (!/^\d+$/.test(n) || xy.length !== 2 || xy.some((s) => !(s > 0))) throw new Error(`${what}: "${part}" is not N:x/y`);
    out[Number(n)] = xy;
  }
  return out;
}

// The slot of each pin of a PLCC: pin 1 on the middle of the top side (the footprints.json PLCC chips'
// convention: pin 1 top centre, numbers counter-clockwise: left, bottom, right, top), so pin p sits on slot
// ((first - 1 + p - 1) mod total) + 1 with first = L + B + R + (T + 1) / 2. T must be odd.
function plccSlots(sides) {
  const [l, b, r, t] = sides;
  const total = l + b + r + t;
  if (t % 2 === 0) throw new Error(`sides ${sides.join(',')}: pin 1 is the middle of the top side, which must have an odd number of slots`);
  const first = l + b + r + (t + 1) / 2;
  return Array.from({ length: total }, (_, i) => ((first - 1 + i) % total) + 1);
}

// The `quad` object of one pin count of a PLCC row (surface mount: the through-hole PLCC groups are out, plan-51-C).
function plccFootprint(row, n) {
  const what = `${row.maker} ${row.series}`;
  if (!MAKER_CODE[row.maker]) throw new Error(`no maker code for ${row.maker}`);
  if (row.mounting !== 'Surface Mount') throw new Error(`${what}: a PLCC row must be surface mount (through-hole PLCC groups are out)`);
  const sides = sidesPerPin(row.sides, `${what} sides`)[n];
  const span = spansPerPin(row.span, `${what} span`)[n];
  const bodyW = perPin(row.body_w, `${what} body_w`)[n];
  const bodyH = perPin(row.body_len, `${what} body_len`)[n];
  if (!sides || !span || !bodyW || !bodyH) throw new Error(`${what}: no sides, span or body for ${n} pins`);
  const total = sides.reduce((a, b) => a + b, 0);
  if (total !== n) throw new Error(`${what}: sides ${sides.join(',')} add to ${total}, not ${n}`);
  const [file, page] = row.drawing.split(' p.');
  const padW = Number(row.pad_w);
  const padH = Number(row.pad_h);
  return {
    id: `SKT-PLCC${n}-SM-${MAKER_CODE[row.maker]}-${idSeries(row.series)}`,
    description: `${n}-pin PLCC socket, ${row.maker} ${row.series}, surface mount`,
    note: `${row.maker} ${file} p.${page}: pad ${padW} x ${padH}, pad-centre span ${span[0]} x ${span[1]}, body ${bodyW} x ${bodyH}; ${row.rule}`,
    style: 'quad',
    pins: n,
    body: { width: bodyW, height: bodyH },
    sides,
    pitch: Number(row.pitch),
    spanX: span[0],
    spanY: span[1],
    pad: { width: padW, height: padH, shape: 'rect' },
    slots: plccSlots(sides),
  };
}

// Objects equal in every key but id, description and note are one footprint.
function dedupe(footprints) {
  const byGeometry = new Map();
  const unique = [];
  const idOf = new Map();
  for (const fp of footprints) {
    const { id, description, note, ...geometry } = fp;
    const key = JSON.stringify(geometry);
    if (!byGeometry.has(key)) {
      byGeometry.set(key, id);
      unique.push(fp);
    }
    idOf.set(id, byGeometry.get(key));
  }
  return { unique, idOf };
}

function dipRows(rows) {
  return rows.filter((r) => r.family === 'DIP');
}

function plccRows(rows) {
  return rows.filter((r) => r.family === 'PLCC');
}

// Every DIP shape of fits.tsv must be supplied exactly once by a generic through-hole row.
function genericProblems(rows) {
  const supplied = {};
  for (const row of dipRows(rows)) {
    if (row.generic === '-') continue;
    const pins = row.generic === 'yes' ? row.pins.split(',') : row.generic.split(',');
    for (const p of pins) {
      const key = `${row.spacing}:${p}`;
      if (row.mounting !== 'Through Hole') throw new Error(`${row.maker} ${row.series}: a generic DIP row must be through hole`);
      (supplied[key] = supplied[key] || []).push(`${row.maker} ${row.series}`);
    }
  }
  const problems = [];
  for (const [spacing, shapes] of Object.entries(DIP_SHAPES)) {
    for (const n of shapes) {
      const who = supplied[`${spacing}:${n}`] || [];
      if (who.length !== 1) problems.push(`${n}-pin ${spacing} in DIP socket: ${who.length === 0 ? 'no generic through-hole row' : `generic from ${who.join(' and ')}`}`);
    }
  }
  return problems;
}

// Every PLCC shape (28, 32, 44, 68) must be supplied exactly once by a generic row (surface mount: plan-51-C).
function genericPlccProblems(rows) {
  const supplied = {};
  for (const row of plccRows(rows)) {
    if (row.generic === '-') continue;
    const pins = row.generic === 'yes' ? row.pins.split(',') : row.generic.split(',');
    for (const p of pins) (supplied[p] = supplied[p] || []).push(`${row.maker} ${row.series}`);
  }
  const problems = [];
  for (const n of PLCC_SHAPES) {
    const who = supplied[n] || [];
    if (who.length !== 1) problems.push(`${n}-pin PLCC socket: ${who.length === 0 ? 'no generic row' : `generic from ${who.join(' and ')}`}`);
  }
  return problems;
}

function dipFootprints(rows) {
  const all = [];
  for (const row of dipRows(rows)) {
    for (const n of row.pins.split(',').map(Number)) all.push(dipFootprint(row, n));
  }
  return dedupe(all);
}

function plccFootprints(rows) {
  const all = [];
  for (const row of plccRows(rows)) {
    for (const n of row.pins.split(',').map(Number)) all.push(plccFootprint(row, n));
  }
  return dedupe(all);
}

function sideFile(footprints) {
  return ['{', '  "format": "schematical-footprints",', '  "version": 1,', '  "footprints": [',
    footprints.map((fp, i) => '    ' + JSON.stringify(fp) + (i < footprints.length - 1 ? ',' : '')).join('\n'),
    '  ]', '}', ''].join('\n');
}

function main(argv) {
  const args = { series: DEFAULT_SERIES, skipGeneric: false };
  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === '--letter') args.letter = argv[++i];
    else if (argv[i] === '--out') args.out = argv[++i];
    else if (argv[i] === '--series') args.series = argv[++i];
    else if (argv[i] === '--skip-generic-check') args.skipGeneric = true;
    else throw new Error(`unknown option ${argv[i]}`);
  }
  if (!['D', 'E'].includes(args.letter) || !args.out) {
    console.error('usage: node scripts/gen-sockets.js --letter D|E --out FILE [--series FILE] [--skip-generic-check]');
    process.exit(2);
  }
  const rows = readTsv(args.series);
  const dip = args.letter === 'D';
  if (!args.skipGeneric) {
    const problems = dip ? genericProblems(rows) : genericPlccProblems(rows);
    if (problems.length > 0) {
      console.error(`gen-sockets: the generic ${dip ? 'DIP' : 'PLCC'} sockets are not covered:\n  ` + problems.join('\n  '));
      process.exit(1);
    }
  }
  const { unique } = dip ? dipFootprints(rows) : plccFootprints(rows);
  fs.writeFileSync(args.out, sideFile(unique));
  console.log(`${args.out}: ${unique.length} ${dip ? 'DIP' : 'PLCC'} socket footprints from ${(dip ? dipRows(rows) : plccRows(rows)).length} rows`);
}

if (require.main === module) {
  try {
    main(process.argv.slice(2));
  } catch (error) {
    console.error(`gen-sockets: ${error.message}`);
    process.exit(1);
  }
} else {
  module.exports = {
    readTsv, perPin, dipFootprint, dedupe, dipFootprints, genericProblems, sideFile,
    plccSlots, plccFootprint, plccFootprints, genericPlccProblems,
  };
}
