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

test('explicit style: two holes per pole, staggered rows, an off-centre first column, numbering from the right', () => {
  const { dir, files } = workspace();
  const t = JSON.parse(fs.readFileSync(files.table, 'utf8'));
  t.variants.push(
    // two holes per pole (rows 5.5 apart), the first column 2 from the left edge, width (n-1)*3.81 + 4
    { stem: 'TEST-TWO-3.81-VT', letter: 'C', positions: [2, 3], style: 'explicit', holes: [0, 5.5], edgeX: 2, describe: 'Test two-hole block',
      note: 'test', pitch: 3.81, drill: 1.1, firstRowY: -2.65, bodyExtra: 0.19, bodyDepth: 7.7 },
    // poles alternate between two rows 14 apart
    { stem: 'TEST-ZIG-7.50-VT', letter: 'C', positions: [3], style: 'explicit', stagger: [0, 14], edgeX: 6.55, describe: 'Test staggered block',
      note: 'test', pitch: 7.5, drill: 2.1, firstRowY: -6.67, bodyExtra: 0, bodyDepth: 18.5 },
    // numbered from the right, rows alternate
    { stem: 'TEST-REV-3.81-AT', letter: 'C', positions: [3], style: 'explicit', stagger: [0, 5], reverse: true, edgeX: 3.25, describe: 'Test reversed block',
      note: 'test', pitch: 3.81, drill: 1, firstRowY: -3.7, bodyExtra: 1.5, bodyDepth: 12 });
  fs.writeFileSync(files.table, JSON.stringify(t));
  const out = path.join(dir, 'out.json');
  const r = run(files, '--letter', 'C', '--out', out);
  assert.strictEqual(r.status, 0, r.stderr);
  const by = Object.fromEntries(JSON.parse(fs.readFileSync(out, 'utf8')).footprints.map((o) => [o.id, o]));

  const two = by['TB-TEST-TWO-3.81-VT-03'];
  assert.strictEqual(two.style, 'explicit');
  assert.strictEqual(two.pins, 3);
  assert.strictEqual(two.pads.length, 6);
  assert.strictEqual(two.body.width, 11.62);                 // 3 * 3.81 + 0.19
  // first column: -width/2 + 2 = -3.81; then 3.81 apart; rows -2.65 and 2.85
  assert.deepStrictEqual(two.pads.filter((p) => p.pin === 1).map((p) => [p.x, p.y, p.shape]), [[-3.81, -2.65, 'rect'], [-3.81, 2.85, 'rect']]);
  assert.deepStrictEqual(two.pads.filter((p) => p.pin === 3).map((p) => [p.x, p.y, p.shape]), [[3.81, -2.65, 'round'], [3.81, 2.85, 'round']]);
  assert.strictEqual(two.pads[0].drill, 1.1);
  assert.strictEqual(two.pads[0].width, 1.8);
  assert.ok(!('pad' in two) && !('drill' in two) && !('pitch' in two) && !('rows' in two), 'explicit has no footprint-level pad keys');

  const zig = by['TB-TEST-ZIG-7.50-VT-03'];
  assert.strictEqual(zig.pads.length, 3);
  // width 3 * 7.5 = 22.5, first column -11.25 + 6.55 = -4.7; poles 1 and 3 on the first row, pole 2 14 below
  assert.deepStrictEqual(zig.pads.map((p) => [p.pin, p.x, p.y]), [[1, -4.7, -6.67], [2, 2.8, 7.33], [3, 10.3, -6.67]]);

  const rev = by['TB-TEST-REV-3.81-AT-03'];
  assert.strictEqual(rev.body.width, 12.93);                 // 3 * 3.81 + 1.5
  // numbered from the right: the leftmost column (-6.465 + 3.25) is pole 3, pole 1 is the rightmost; rows alternate
  assert.deepStrictEqual(rev.pads.map((p) => [p.pin, p.x, p.y, p.shape]),
    [[1, 4.405, -3.7, 'rect'], [2, 0.595, 1.3, 'round'], [3, -3.215, -3.7, 'round']]);
});

test('flange holes: non-plated pegs or plated unnumbered pads beyond each end of the row; a gap leaves an empty slot', () => {
  const { dir, files } = workspace();
  const t = JSON.parse(fs.readFileSync(files.table, 'utf8'));
  t.variants.push(
    // two non-plated 2.3 pegs, 5.08 beyond the end poles, on the pole row
    { stem: 'TEST-FLG-5.08-VT', letter: 'C', positions: [2, 5], style: 'explicit', flange: { dx: 5.08, dy: 0, drill: 2.3 }, describe: 'Test flange block',
      note: 'test', pitch: 5.08, drill: 1.45, firstRowY: 0.4, bodyExtra: 10, bodyDepth: 8.4 },
    // plated solder flange pins 3.8 beyond the ends and 2.3 below the row
    { stem: 'TEST-FLP-3.81-HT', letter: 'C', positions: [3], style: 'explicit', flange: { dx: 3.8, dy: 2.3, drill: 1.55, plated: true }, describe: 'Test solder flange',
      note: 'test', pitch: 3.81, drill: 1.35, firstRowY: -3.3, bodyExtra: 10.6, bodyDepth: 9.2 },
    // 4 poles on a 5 slot housing: no pin in slot 4
    { stem: 'TEST-GAP-7.62-HT', letter: 'C', positions: [4], style: 'explicit', gaps: [3], edgeX: 3.5, describe: 'Test gap block',
      note: 'test', pitch: 7.62, drill: 1.45, firstRowY: -1.45, bodyExtra: 7, bodyDepth: 12.7 });
  fs.writeFileSync(files.table, JSON.stringify(t));
  const out = path.join(dir, 'out.json');
  const r = run(files, '--letter', 'C', '--out', out);
  assert.strictEqual(r.status, 0, r.stderr);
  const by = Object.fromEntries(JSON.parse(fs.readFileSync(out, 'utf8')).footprints.map((o) => [o.id, o]));

  const flg = by['TB-TEST-FLG-5.08-VT-02'];
  assert.strictEqual(flg.pads.length, 4);                   // 2 poles + 2 pegs
  assert.deepStrictEqual(flg.pads.filter((p) => p.pin !== 0).map((p) => [p.pin, p.x]), [[1, -2.54], [2, 2.54]]);
  assert.deepStrictEqual(flg.pads.filter((p) => p.pin === 0),
    [{ pin: 0, x: -7.62, y: 0.4, width: 2.3, height: 2.3, shape: 'round', drill: 2.3, plated: false },
     { pin: 0, x: 7.62, y: 0.4, width: 2.3, height: 2.3, shape: 'round', drill: 2.3, plated: false }]);
  assert.strictEqual(flg.pins, 2);                          // the pegs are not pins

  const flp = by['TB-TEST-FLP-3.81-HT-03'];
  const solder = flp.pads.filter((p) => p.pin === 0);
  assert.deepStrictEqual(solder.map((p) => [p.x, p.y, p.width, p.drill, p.plated]), [[-7.61, -1, 2.25, 1.55, undefined], [7.61, -1, 2.25, 1.55, undefined]]);   // the end poles are at +-3.81, the pins 3.8 beyond

  const gap = by['TB-TEST-GAP-7.62-HT-04'];
  // width 4 * 7.62 + 7 = 37.48, first column 3.5 from the left end; poles 1, 2, 3 then slot 4 empty, pole 4 in slot 5
  assert.strictEqual(gap.body.width, 37.48);
  assert.deepStrictEqual(gap.pads.map((p) => [p.pin, p.x]), [[1, -15.24], [2, -7.62], [3, 0], [4, 15.24]]);
});

test('smd: rect pads without a drill and two unnumbered tabs; pegs: single holes at one end', () => {
  const { dir, files } = workspace();
  const t = JSON.parse(fs.readFileSync(files.table, 'utf8'));
  t.variants.push(
    // 1.25 mm pitch, 0.6 x 1.7 pads on the row 2.3 above the body centre, 1.0 x 2.8 tabs 1.85 beyond the end pads, 1.03 below
    { stem: 'JST-TEST-GH-1.25-VS', letter: 'E', positions: [3, 12], style: 'explicit', describe: 'JST test GH', note: 'test', pitch: 1.25,
      firstRowY: -2.3, bodyExtra: 3.25, bodyDepth: 4.3, smd: { pad: { width: 0.6, height: 1.7 }, tab: { dx: 1.85, dy: 3.33, width: 1, height: 2.8 } } },
    // through hole with a boss peg 1.6 beyond pole 1 and 2 above the row; plated is false
    { stem: 'JST-TEST-XH-2.50-VT', letter: 'E', positions: [3], style: 'explicit', describe: 'JST test XH', note: 'test', pitch: 2.5, drill: 1,
      firstRowY: -0.5, bodyExtra: 2.4, bodyDepth: 5.75, pegs: [{ end: 'first', dx: 1.6, dy: -2, drill: 1.2 }] });
  fs.writeFileSync(files.table, JSON.stringify(t));
  const out = path.join(dir, 'out.json');
  const r = run(files, '--letter', 'E', '--out', out);
  assert.strictEqual(r.status, 0, r.stderr);
  const by = Object.fromEntries(JSON.parse(fs.readFileSync(out, 'utf8')).footprints.map((o) => [o.id, o]));

  const gh3 = by['JST-TEST-GH-1.25-VS-03'];
  assert.strictEqual(gh3.pins, 3);
  assert.strictEqual(gh3.pads.length, 5);                    // 3 pads + 2 tabs
  assert.ok(gh3.pads.every((p) => p.shape === 'rect' && p.drill === undefined));
  assert.deepStrictEqual(gh3.pads.filter((p) => p.pin !== 0).map((p) => [p.pin, p.x, p.y, p.width, p.height]),
    [[1, -1.25, -2.3, 0.6, 1.7], [2, 0, -2.3, 0.6, 1.7], [3, 1.25, -2.3, 0.6, 1.7]]);
  assert.deepStrictEqual(gh3.pads.filter((p) => p.pin === 0).map((p) => [p.x, p.y, p.width, p.height]), [[-3.1, 1.03, 1, 2.8], [3.1, 1.03, 1, 2.8]]);
  assert.strictEqual(gh3.body.width, 7);                      // 3 * 1.25 + 3.25
  const gh12 = by['JST-TEST-GH-1.25-VS-12'];
  assert.strictEqual(gh12.pads.length, 14);
  assert.deepStrictEqual(gh12.pads.filter((p) => p.pin === 0).map((p) => p.x), [-6.875 - 1.85, 6.875 + 1.85]);   // the end pads of 12 are at +-11/2 x 1.25

  const xh = by['JST-TEST-XH-2.50-VT-03'];
  assert.deepStrictEqual(xh.pads.filter((p) => p.pin === 0),
    [{ pin: 0, x: -2.5 - 1.6, y: -2.5, width: 1.2, height: 1.2, shape: 'round', drill: 1.2, plated: false }]);   // pole 1 is at x = -2.5
});

test('edgeX, holes, stagger and reverse need the explicit style', () => {
  const { dir, files } = workspace();
  const t = JSON.parse(fs.readFileSync(files.table, 'utf8'));
  t.variants[0].edgeX = 2;
  fs.writeFileSync(files.table, JSON.stringify(t));
  const r = run(files, '--letter', 'C', '--out', path.join(dir, 'o.json'));
  assert.notStrictEqual(r.status, 0);
  assert.match(r.stderr, /need style "explicit"/);
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
