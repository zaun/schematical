#!/usr/bin/env node
// Generates the generic pin headers (plan-23-A):
//
//   schematical/schematical/Parts/header.json   80 parts, GENERIC-HEADER-1X01 … -2X40
//   schematical/schematical/Parts/footprints.json  the 160 PINHDR-<r>X<nn>-V / -RA lines
//
// Run from anywhere: `node scripts/gen-headers.js`. In footprints.json every
// line whose id starts with PINHDR- is replaced by the generated lines (put at
// the end of the array), and every other line is kept. Running it twice
// changes nothing.
//
// Numbers are from the Würth WR-PHD drawings in datasheets/connector-header/
// (page 1 of each). No drawing gives a pad, so pads follow plan-03-B R5:
// round, drill + 0.7 (at least 1.5). Pin 1's pad is square.

'use strict';

const fs = require('fs');
const path = require('path');

const ROOT = path.resolve(__dirname, '..');
const PARTS = path.join(ROOT, 'schematical', 'schematical', 'Parts');
const HEADER_FILE = path.join(PARTS, 'header.json');
const FOOTPRINT_FILE = path.join(PARTS, 'footprints.json');

const PITCH = 2.54;
const MAX_PER_ROW = 40;

// One entry per footprint type: rows, vertical or right angle.
const TYPES = [
  {
    rows: 1, suffix: 'V', kind: 'vertical', depth: 2.54, firstRowY: 0, drill: 1.1, pad: 1.8,
    note: 'Würth WR-PHD 61300111121 p.1: hole Ø1.10, pitch 2.54, body 2.54 deep; pad R5 (drill + 0.7)',
  },
  {
    rows: 2, suffix: 'V', kind: 'vertical', depth: 5.08, firstRowY: -1.27, rowPitch: 2.54, drill: 1.1, pad: 1.8,
    note: 'Würth WR-PHD 61300421121 p.1: hole Ø1.10, pitch and row pitch 2.54, body 5.08 deep; pad R5 (drill + 0.7)',
  },
  {
    rows: 1, suffix: 'RA', kind: 'right angle', depth: 2.54, firstRowY: -2.77, drill: 1.1, pad: 1.8,
    note: 'Würth WR-PHD 61300211021 p.1: hole Ø1.1, pitch 2.54, housing 2.54 deep, 1.5 from the hole row; pad R5 (drill + 0.7)',
  },
  {
    rows: 2, suffix: 'RA', kind: 'right angle', depth: 2.54, firstRowY: -2.77, rowPitch: -2.54, drill: 1.02, pad: 1.72,
    note: 'Würth WR-PHD 61300421021 p.1: hole Ø1.02, row pitch 2.54, housing 1.5 from row 1; pad R5 (drill + 0.7)',
  },
];

const PACKAGES = { V: 'Straight', RA: 'Right angle' };

const two = (n) => String(n).padStart(2, '0');
const mm = (n) => Number(n.toFixed(4));
const size = (rows, perRow) => `${rows}X${two(perRow)}`;
const footprintId = (rows, perRow, suffix) => `PINHDR-${size(rows, perRow)}-${suffix}`;

function footprintLine(type, perRow) {
  const fp = {
    id: footprintId(type.rows, perRow, type.suffix),
    description: `${type.rows}x${perRow} pin header, 2.54 mm pitch, ${type.kind}`,
    note: type.note,
    style: 'header',
    pins: type.rows * perRow,
    rows: type.rows,
    body: { width: mm(perRow * PITCH), height: type.depth },
    pitch: PITCH,
  };
  if (type.rowPitch !== undefined) fp.rowPitch = type.rowPitch;
  fp.firstRowY = type.firstRowY;
  fp.pad = { width: type.pad, height: type.pad, shape: 'round' };
  fp.drill = type.drill;
  fp.pin1Shape = 'rect';
  return '    ' + JSON.stringify(fp);
}

function writeFootprints() {
  const text = fs.readFileSync(FOOTPRINT_FILE, 'utf8');
  const lines = text.split('\n');
  const open = lines.findIndex((l) => l.trim() === '"footprints": [');
  const close = lines.lastIndexOf('  ]');
  if (open < 0 || close < open) throw new Error('footprints.json: no "footprints": [ … ] block');

  const kept = lines.slice(open + 1, close)
    .map((l) => l.replace(/,$/, ''))
    .filter((l) => !l.startsWith('    {"id":"PINHDR-'));
  const generated = [];
  for (const suffix of ['V', 'RA']) {
    for (const rows of [1, 2]) {
      const type = TYPES.find((t) => t.rows === rows && t.suffix === suffix);
      for (let perRow = 1; perRow <= MAX_PER_ROW; perRow++) generated.push(footprintLine(type, perRow));
    }
  }
  const entries = kept.concat(generated);
  const out = [
    ...lines.slice(0, open + 1),
    ...entries.map((l, i) => (i < entries.length - 1 ? l + ',' : l)),
    ...lines.slice(close),
  ].join('\n');
  if (out !== text) fs.writeFileSync(FOOTPRINT_FILE, out);
  return generated.length;
}

function device(rows) {
  const pinouts = [];
  const parts = [];
  for (let perRow = 1; perRow <= MAX_PER_ROW; perRow++) {
    const name = `${rows}x${two(perRow)}`;
    const pins = [];
    for (let n = 1; n <= rows * perRow; n++) pins.push(JSON.stringify({ number: n, name: String(n), role: 'inout' }));
    pinouts.push(`        ${JSON.stringify(name)}: [${pins.join(',')}]`);
    const choices = ['V', 'RA'].map((s) => JSON.stringify({ package: PACKAGES[s], footprint: footprintId(rows, perRow, s) }));
    parts.push('        ' + JSON.stringify({
      name: `GENERIC-HEADER-${size(rows, perRow)}`,
      package: PACKAGES.V,
      pinout: name,
      footprint: footprintId(rows, perRow, 'V'),
    }).replace(/}$/, `,"footprintChoices":[${choices.join(',')}]}`));
  }
  return [
    '    {',
    `      "id": "header-${rows}row",`,
    '      "function": "Header",',
    `      "description": "Generic pin header, ${rows} ${rows === 1 ? 'row' : 'rows'}",`,
    '      "drawer": ["Connector","Header"],',
    `      "rows": ${rows},`,
    '      "genders": ["Male","Female"],',
    '      "pinouts": {',
    pinouts.join(',\n'),
    '      },',
    '      "parts": [',
    parts.join(',\n'),
    '      ]',
    '    }',
  ].join('\n');
}

function writeHeaders() {
  const out = [
    '{',
    '  "format": "schematical-parts",',
    '  "version": 1,',
    '  "kind": "header",',
    '  "devices": [',
    device(1) + ',',
    device(2),
    '  ]',
    '}',
    '',
  ].join('\n');
  const old = fs.existsSync(HEADER_FILE) ? fs.readFileSync(HEADER_FILE, 'utf8') : null;
  if (out !== old) fs.writeFileSync(HEADER_FILE, out);
  return 2 * MAX_PER_ROW;
}

const partCount = writeHeaders();
const footprintCount = writeFootprints();
console.log(`header.json: ${partCount} parts; footprints.json: ${footprintCount} PINHDR- footprints`);
