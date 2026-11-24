// Tests for scripts/gen-terminals.js (plan-45-C). Run: node --test scripts/gen-terminals.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const TOOL = path.join(__dirname, 'gen-terminals.js');

// A 5.00 mm vertical block (pad from the default rule R5-T) and a 3.50 mm horizontal one.
const TABLE = {
  variants: [
    { stem: 'TEST-PT-5.00-VT', letter: 'C', positions: [2, 3, 12], style: 'header', describe: 'Test PT 5.00 mm PCB terminal block, vertical',
      note: 'test: hole 1.3, pitch 5.00, body 9.0 deep', pitch: 5, drill: 1.3, firstRowY: 0, bodyExtra: 3, bodyDepth: 9 },
    { stem: 'TEST-LSF-3.50-HT', letter: 'C', positions: [2, 4], style: 'header', describe: 'Test LSF 3.50 mm PCB terminal block, horizontal',
      note: 'test: hole 1.1, pitch 3.50, body 8.0 deep, 2.5 from the rear edge', pitch: 3.5, drill: 1.1, firstRowY: -1.5, bodyExtra: 2, bodyDepth: 8 },
    { stem: 'JST-TEST-2.50-VT', letter: 'E', positions: [3], style: 'header', describe: 'JST test header 2.50 mm, vertical',
      note: 'test', pitch: 2.5, drill: 1, firstRowY: 0, bodyExtra: 2.5, bodyDepth: 5.8, pad: { width: 1.7, height: 1.7, shape: 'round' } },
  ],
};

function workspace() {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'gen-terminals-'));
  const files = { table: path.join(dir, 'table.json'), fp: path.join(dir, 'footprints.json'), parts: path.join(dir, 'connector.json') };
  fs.writeFileSync(files.table, JSON.stringify(TABLE));
  fs.writeFileSync(files.fp, `{
  "format": "schematical-footprints",
  "version": 1,
  "footprints": [
    {"id":"LED-0402","style":"dual","pins":2},
    {"id":"TB-OLD-STALE-02","style":"header","pins":2},
    {"id":"SOIC-8","style":"dual","pins":8}
  ]
}
`);
  fs.writeFileSync(files.parts, '{"devices":[{"parts":[{"name":"A","footprint":"TB-TEST-PT-5.00-VT-03"},{"name":"B","footprint":"TB-TEST-LSF-3.50-HT-04","footprintChoices":[{"footprint":"TB-TEST-LSF-3.50-HT-02"}]},{"name":"C","footprint":"JST-TEST-2.50-VT-03"},{"name":"D","footprint":"SOIC-8"}]}]}');
  return { dir, files };
}

function run(files, ...args) {
  return childProcess.spawnSync('node', [TOOL, '--table', files.table, '--footprints', files.fp, '--parts', files.parts, ...args], { encoding: 'utf8' });
}

test('--letter C writes the objects of the letter with the expected numbers', () => {
  const { dir, files } = workspace();
  const out = path.join(dir, 'out.json');
  const r = run(files, '--letter', 'C', '--out', out);
  assert.strictEqual(r.status, 0, r.stderr);
  const objects = JSON.parse(fs.readFileSync(out, 'utf8')).footprints;
  assert.deepStrictEqual(objects.map((o) => o.id), ['TB-TEST-PT-5.00-VT-02', 'TB-TEST-PT-5.00-VT-03', 'TB-TEST-PT-5.00-VT-12',
    'TB-TEST-LSF-3.50-HT-02', 'TB-TEST-LSF-3.50-HT-04']);
  const v12 = objects.find((o) => o.id === 'TB-TEST-PT-5.00-VT-12');
  assert.strictEqual(v12.pins, 12);
  assert.strictEqual(v12.rows, 1);
  assert.strictEqual(v12.style, 'header');
  assert.strictEqual(v12.body.width, 63);          // 12 * 5 + 3
  assert.strictEqual(v12.body.height, 9);
  assert.strictEqual(v12.firstRowY, 0);
  assert.strictEqual(v12.pin1Shape, 'rect');
  assert.strictEqual(v12.drill, 1.3);
  const h4 = objects.find((o) => o.id === 'TB-TEST-LSF-3.50-HT-04');
  assert.strictEqual(h4.body.width, 16);           // 4 * 3.5 + 2
  assert.strictEqual(h4.firstRowY, -1.5);
  // R5-T: drill + 0.7, at most pitch - 0.8
  assert.strictEqual(v12.pad.width, 2);
  assert.strictEqual(h4.pad.width, 1.8);
  for (const o of objects) {
    assert.ok(o.pad.width <= o.pitch - 0.8 + 1e-9, `${o.id}: pad ${o.pad.width} leaves a gap under 0.8 mm`);
    assert.ok(o.pad.width > o.drill);
    assert.match(o.id, /^[A-Z0-9.-]+$/);
    assert.ok(o.note.length > 0 && o.description.length > 0);
  }
  assert.strictEqual(new Set(objects.map((o) => o.id)).size, objects.length);
});

test('the pad is capped at pitch - 0.8 when drill + 0.7 would be wider', () => {
  const { dir, files } = workspace();
  const t = JSON.parse(fs.readFileSync(files.table, 'utf8'));
  t.variants[1].drill = 2.2;                      // 2.2 + 0.7 = 2.9 > 3.5 - 0.8 = 2.7: capped at 2.7
  fs.writeFileSync(files.table, JSON.stringify(t));
  const out = path.join(dir, 'out.json');
  assert.strictEqual(run(files, '--letter', 'C', '--out', out).status, 0);
  const o = JSON.parse(fs.readFileSync(out, 'utf8')).footprints.find((x) => x.id === 'TB-TEST-LSF-3.50-HT-02');
  assert.strictEqual(o.pad.width, 2.7);
});

test('--write puts the named footprints into footprints.json, twice the same, keeping the other lines', () => {
  const { files } = workspace();
  let r = run(files, '--write');
  assert.strictEqual(r.status, 0, r.stderr);
  const first = fs.readFileSync(files.fp, 'utf8');
  const lib = JSON.parse(first).footprints;
  assert.deepStrictEqual(lib.map((o) => o.id), ['LED-0402', 'SOIC-8', 'TB-TEST-PT-5.00-VT-03', 'TB-TEST-LSF-3.50-HT-02',
    'TB-TEST-LSF-3.50-HT-04', 'JST-TEST-2.50-VT-03']);   // the stale TB- line is gone, the unnamed ones are not written
  r = run(files, '--write');
  assert.strictEqual(r.status, 0, r.stderr);
  assert.strictEqual(fs.readFileSync(files.fp, 'utf8'), first);
  assert.strictEqual(run(files, '--check').status, 0);
});

test('--check fails when footprints.json differs', () => {
  const { files } = workspace();
  const r = run(files, '--check');
  assert.strictEqual(r.status, 1);
  assert.match(r.stderr, /differs/);
});

test('a part naming a footprint the table does not generate is an error', () => {
  const { files } = workspace();
  fs.writeFileSync(files.parts, '{"footprint":"TB-NO-SUCH-VT-03"}');
  const r = run(files, '--write');
  assert.notStrictEqual(r.status, 0);
  assert.match(r.stderr, /does not generate/);
});

test('duplicate ids and a pad that does not fit are rejected', () => {
  const { dir, files } = workspace();
  const t = JSON.parse(fs.readFileSync(files.table, 'utf8'));
  t.variants.push({ ...t.variants[0] });
  fs.writeFileSync(files.table, JSON.stringify(t));
  let r = run(files, '--letter', 'C', '--out', path.join(dir, 'o.json'));
  assert.notStrictEqual(r.status, 0);
  assert.match(r.stderr, /duplicate id/);
  const t2 = JSON.parse(JSON.stringify(TABLE));
  t2.variants[0].pad = { width: 5, height: 5, shape: 'round' };
  fs.writeFileSync(files.table, JSON.stringify(t2));
  r = run(files, '--letter', 'C', '--out', path.join(dir, 'o.json'));
  assert.notStrictEqual(r.status, 0);
  assert.match(r.stderr, /does not fit the pitch/);
});
