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
//   --skip-generic-check   for fixtures: do not require the generic shapes (see below)
//   --write           plan-51-F: writes schematical/schematical/Parts/socket.json (every `add` row of candidates.tsv
//                     plus 15 generic sockets), replaces every SKT- footprint of footprints.json by the ones those
//                     parts name, and the datasheets.json entries of the maker PDFs; idempotent
//   --check           exits 1 when --write would change one of those files (gen-headers.js has no such mode)
//   --root DIR        the tree --write / --check read and write (default: this repository; the tests give a fixture)
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
const plccIdOf = (row, n) => `SKT-PLCC${n}-SM-${MAKER_CODE[row.maker]}-${idSeries(row.series)}`;

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
    id: plccIdOf(row, n),
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

// ---- The parts (plan-51-F) ----------------------------------------------------------------------------------------

const MAKER_NAME = { 'Mill-Max': 'Mill-Max Manufacturing Corp.', 'Preci-Dip': 'Preci-Dip', 'Assmann': 'Assmann WSW Components' };
const DATASHEET_PREFIX = { 'Mill-Max': 'mill-max', 'Preci-Dip': 'precidip', 'Assmann': 'assmann' };
const SOCKET_FILE_HEAD = ['{', '  "format": "schematical-parts",', '  "version": 1,', '  "kind": "socket",', '  "devices": ['];

// `datasheets/ic-socket/assmann/ASS_4852_CO.pdf` -> `assmann-ass-4852-co` (the local PDF's name in lower case).
function datasheetId(maker, pdf) {
  return `${DATASHEET_PREFIX[maker]}-${path.basename(pdf, '.pdf').toLowerCase().replace(/_/g, '-')}`;
}

const mountingWord = (m) => (m === 'Through Hole' ? 'through-hole' : 'surface-mount');

// Everything --write writes, as text, from the data files under `root`.
function buildAll(root, skipGeneric) {
  const data = path.join(root, 'planning', 'plan-51-data');
  const parts = path.join(root, 'schematical', 'schematical', 'Parts');
  const rows = readTsv(path.join(data, 'series.tsv'));
  const candidates = readTsv(path.join(data, 'candidates.tsv')).filter((c) => c.state === 'add');
  const fits = readTsv(path.join(data, 'fits.tsv'));
  if (!skipGeneric) {
    const problems = [...genericProblems(rows), ...genericPlccProblems(rows)];
    if (problems.length > 0) throw new Error('the generic sockets are not covered:\n  ' + problems.join('\n  '));
  }
  const d = dipFootprints(rows);
  const e = plccFootprints(rows);
  const idOf = new Map([...d.idOf, ...e.idOf]);
  const byId = new Map([...d.unique, ...e.unique].map((fp) => [fp.id, fp]));
  const footprintOf = (row, n) => idOf.get(row.family === 'DIP' ? dipId(row, n) : plccIdOf(row, n));

  const rowOf = new Map(rows.map((r) => [[r.maker, r.series, r.family, r.mounting, r.spacing].join('|'), r]));
  const generic = new Map();   // `<family>:<spacing>:<pins>` -> footprint id
  for (const row of rows) {
    if (row.generic === '-') continue;
    for (const n of (row.generic === 'yes' ? row.pins : row.generic).split(',').map(Number)) {
      generic.set(`${row.family}:${row.family === 'DIP' ? row.spacing : ''}:${n}`, footprintOf(row, n));
    }
  }

  const used = new Set();
  const sheets = new Map();    // datasheet id -> url
  const names = new Set();
  const devices = [];
  for (const f of fits) {
    const n = Number(f.pins);
    const dip = f.family === 'DIP';
    const tag = dip ? `${n}-${f.spacing === '0.3' ? '300' : '600'}` : `${n}`;
    const list = [];
    const genericFootprint = generic.get(`${f.family}:${dip ? f.spacing : ''}:${n}`);
    const fitList = f.fits.split(',');
    const base = { package: dip ? 'DIP socket' : 'PLCC socket', pinout: `p${n}` };
    if (genericFootprint) {
      list.push({ name: `GENERIC-SOCKET-${dip ? 'DIP' : 'PLCC'}-${tag}`, manufacturer: 'Generic', ...base, footprint: genericFootprint,
        mounting: dip ? 'through-hole' : 'surface-mount', fits: fitList });
      used.add(genericFootprint);
    } else if (!skipGeneric) {
      throw new Error(`no generic footprint for ${f.family} ${n} ${f.spacing}`);
    }
    for (const c of candidates) {
      if (c.family !== f.family || c.pins !== f.pins || (dip && c.spacing !== f.spacing)) continue;
      const row = rowOf.get([c.maker, c.series, c.family, c.mounting, c.spacing || '-'].join('|'));
      if (!row) throw new Error(`no series.tsv row for ${c.mpn} (${c.maker} ${c.series} ${c.family} ${c.mounting} ${c.spacing})`);
      const footprint = footprintOf(row, n);
      if (!footprint) throw new Error(`no footprint for ${c.mpn}`);
      const sheet = datasheetId(c.maker, c.pdf);
      if (!sheets.has(sheet)) {
        const table = readTsv(path.join(root, path.dirname(c.pdf), 'datasheets.tsv'));
        const entry = table.find((t) => t.file === path.basename(c.pdf));
        if (!entry) throw new Error(`${c.pdf} is not in its datasheets.tsv`);
        sheets.set(sheet, entry.url);
      }
      if (names.has(c.mpn)) throw new Error(`part name ${c.mpn} twice`);
      names.add(c.mpn);
      const part = { name: c.mpn, manufacturer: MAKER_NAME[c.maker], datasheet: sheet, ...base, footprint, mounting: mountingWord(c.mounting) };
      if (c.finish && c.finish !== '-') part.finish = c.finish;
      part.fits = fitList;
      list.push(part);
      used.add(footprint);
    }
    devices.push({ f, n, dip, tag, list });
  }
  const unusedUnique = [...byId.keys()].filter((id) => !used.has(id));
  if (unusedUnique.length > 0) throw new Error(`footprints no part names: ${unusedUnique.slice(0, 5).join(', ')} (${unusedUnique.length})`);

  const socketText = [
    ...SOCKET_FILE_HEAD,
    devices.map(({ f, n, dip, tag, list }) => [
      '    {',
      `      "id": "${dip ? 'dip' : 'plcc'}-${tag}",`,
      `      "function": ${JSON.stringify(dip ? `${n}-pin, ${f.spacing} in` : `${n}-pin`)},`,
      `      "description": ${JSON.stringify(dip ? `${n}-pin ${f.spacing} in DIP socket` : `${n}-pin PLCC socket`)},`,
      `      "drawer": ["Connector","IC Socket","${dip ? 'DIP' : 'PLCC'}"],`,
      `      "family": "${f.family}",`,
      ...(dip ? [`      "rowSpacing": ${ROW_SPACING_MM[f.spacing]},`] : []),
      '      "pinouts": {',
      `        "p${n}": [${Array.from({ length: n }, (_, i) => JSON.stringify({ number: i + 1, name: String(i + 1), role: 'inout' })).join(',')}]`,
      '      },',
      '      "parts": [',
      list.map((p) => '        ' + JSON.stringify(p)).join(',\n'),
      '      ]',
      '    }',
    ].join('\n')).join(',\n'),
    '  ]', '}', '',
  ].join('\n');

  return {
    socketText, socketFile: path.join(parts, 'socket.json'),
    footprints: [...byId.values()], footprintFile: path.join(parts, 'footprints.json'),
    sheets, datasheetFile: path.join(parts, 'datasheets.json'),
    partCount: devices.reduce((s, x) => s + x.list.length, 0), deviceCount: devices.length,
  };
}

// Drops the lines of a file's array block for which `drop(line)` is true and puts `fresh` at the end (gen-headers.js'
// way: every other line is kept, in the file's own layout).
function replaceLines(text, drop, fresh) {
  const lines = text.split('\n');
  const open = lines.findIndex((l) => /^  "(footprints|datasheets)": \[$/.test(l));
  const close = lines.lastIndexOf('  ]');
  if (open < 0 || close < open) throw new Error('no array block');
  const kept = lines.slice(open + 1, close).map((l) => l.replace(/,$/, '')).filter((l) => !drop(l));
  const entries = kept.concat(fresh);
  return [...lines.slice(0, open + 1), ...entries.map((l, i) => (i < entries.length - 1 ? l + ',' : l)), ...lines.slice(close)].join('\n');
}

// The three files as --write would leave them: [[path, text], …].
function outputs(root, skipGeneric) {
  const all = buildAll(root, skipGeneric);
  const footprintText = replaceLines(fs.readFileSync(all.footprintFile, 'utf8'), (l) => l.startsWith('    {"id":"SKT-'),
    all.footprints.map((fp) => '    ' + JSON.stringify(fp)));
  const ours = [...all.sheets.keys()];
  const datasheetText = replaceLines(fs.readFileSync(all.datasheetFile, 'utf8'),
    (l) => ours.some((id) => l.startsWith(`    {"id": ${JSON.stringify(id)},`)),
    ours.map((id) => `    {"id": ${JSON.stringify(id)}, "url": ${JSON.stringify(all.sheets.get(id))}}`));
  return { all, files: [[all.socketFile, all.socketText], [all.footprintFile, footprintText], [all.datasheetFile, datasheetText]] };
}

function sideFile(footprints) {
  return ['{', '  "format": "schematical-footprints",', '  "version": 1,', '  "footprints": [',
    footprints.map((fp, i) => '    ' + JSON.stringify(fp) + (i < footprints.length - 1 ? ',' : '')).join('\n'),
    '  ]', '}', ''].join('\n');
}

function main(argv) {
  const args = { series: DEFAULT_SERIES, skipGeneric: false, root: ROOT };
  for (let i = 0; i < argv.length; i++) {
    if (argv[i] === '--letter') args.letter = argv[++i];
    else if (argv[i] === '--out') args.out = argv[++i];
    else if (argv[i] === '--series') args.series = argv[++i];
    else if (argv[i] === '--root') args.root = argv[++i];
    else if (argv[i] === '--write') args.write = true;
    else if (argv[i] === '--check') args.check = true;
    else if (argv[i] === '--skip-generic-check') args.skipGeneric = true;
    else throw new Error(`unknown option ${argv[i]}`);
  }
  if (args.write || args.check) {
    const { all, files } = outputs(args.root, args.skipGeneric);
    const stale = files.filter(([file, text]) => !fs.existsSync(file) || fs.readFileSync(file, 'utf8') !== text);
    if (args.check) {
      if (stale.length > 0) {
        console.error(`gen-sockets --check: ${stale.map(([f]) => path.relative(args.root, f)).join(', ')} differ from what --write writes`);
        process.exit(1);
      }
      console.log(`gen-sockets --check: socket.json (${all.partCount} parts), footprints.json and datasheets.json are up to date`);
      return;
    }
    for (const [file, text] of stale) fs.writeFileSync(file, text);
    console.log(`socket.json: ${all.partCount} parts in ${all.deviceCount} devices; footprints.json: ${all.footprints.length} SKT- footprints; `
      + `datasheets.json: ${all.sheets.size} entries (${stale.length} files changed)`);
    return;
  }
  if (!['D', 'E'].includes(args.letter) || !args.out) {
    console.error('usage: node scripts/gen-sockets.js --letter D|E --out FILE [--series FILE] [--skip-generic-check]\n'
      + '       node scripts/gen-sockets.js --write | --check [--root DIR] [--skip-generic-check]');
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
    plccSlots, plccFootprint, plccFootprints, genericPlccProblems, buildAll, outputs, datasheetId,
  };
}
