// Tests for scripts/gen-sockets.js (plan-51-D). Run: node --test scripts/gen-sockets.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const TOOL = path.join(__dirname, 'gen-sockets.js');
const HEADER = ['maker', 'series', 'family', 'mounting', 'spacing', 'pins', 'rule', 'drill', 'pad_w', 'pad_h', 'span', 'pitch',
  'sides', 'body_w', 'body_len', 'pin1', 'generic', 'drawing', 'note'];

function row(over) {
  return Object.assign({
    maker: 'Mill-Max', series: '110', family: 'DIP', mounting: 'Through Hole', spacing: '0.3', pins: '14', rule: 'R5',
    drill: '0.80', pad_w: '1.50', pad_h: '1.50', span: '7.62', pitch: '2.54', sides: '-', body_w: '10.16',
    body_len: '14:17.78', pin1: 'rect', generic: '-', drawing: '088-145.pdf p.3', note: '-',
  }, over);
}

function workspace(rows) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'gen-sockets-'));
  const series = path.join(dir, 'series.tsv');
  fs.writeFileSync(series, [HEADER.join('\t'), ...rows.map((r) => HEADER.map((h) => r[h]).join('\t'))].join('\n') + '\n');
  return { dir, series, out: path.join(dir, 'footprints-D.json') };
}

function run(ws, ...extra) {
  return childProcess.spawnSync('node', [TOOL, '--letter', 'D', '--series', ws.series, '--out', ws.out, ...extra], { encoding: 'utf8' });
}

const footprints = (ws) => JSON.parse(fs.readFileSync(ws.out, 'utf8')).footprints;

test('a 14-pin through-hole row gives one dual object with the socket pad rules applied', () => {
  const ws = workspace([row({})]);
  const result = run(ws, '--skip-generic-check');
  assert.strictEqual(result.status, 0, result.stderr);
  const [fp, ...rest] = footprints(ws);
  assert.strictEqual(rest.length, 0);
  assert.strictEqual(fp.id, 'SKT-DIP14-300-TH-MM-110');
  assert.strictEqual(fp.style, 'dual');
  assert.strictEqual(fp.pins, 14);
  assert.strictEqual(fp.span, 7.62);
  assert.strictEqual(fp.pitch, 2.54);
  assert.strictEqual(fp.pin1Shape, 'rect');
  assert.strictEqual(fp.drill, 0.8);
  assert.deepStrictEqual(fp.pad, { width: 1.5, height: 1.5, shape: 'round' });
  assert.deepStrictEqual(fp.body, { width: 10.16, height: 17.78 });
  assert.match(fp.note, /088-145\.pdf p\.3/);
});

test('every pin count of a row is an object; the 0.6 in row has 600 in its id', () => {
  const ws = workspace([row({ spacing: '0.6', pins: '24,28', span: '15.24', body_w: '17.78', body_len: '24:30.48;28:35.56' })]);
  assert.strictEqual(run(ws, '--skip-generic-check').status, 0);
  assert.deepStrictEqual(footprints(ws).map((f) => f.id), ['SKT-DIP24-600-TH-MM-110', 'SKT-DIP28-600-TH-MM-110']);
});

test('two rows with equal geometry give one object, with the first id', () => {
  const ws = workspace([row({}), row({ series: '111', drawing: '088-145.pdf p.4' })]);
  assert.strictEqual(run(ws, '--skip-generic-check').status, 0);
  const list = footprints(ws);
  assert.strictEqual(list.length, 1);
  assert.strictEqual(list[0].id, 'SKT-DIP14-300-TH-MM-110');
});

test('a row whose geometry differs keeps its own object', () => {
  const ws = workspace([row({}), row({ series: '115', drill: '1.05', pad_w: '1.75', pad_h: '1.75' })]);
  assert.strictEqual(run(ws, '--skip-generic-check').status, 0);
  assert.deepStrictEqual(footprints(ws).map((f) => f.id), ['SKT-DIP14-300-TH-MM-110', 'SKT-DIP14-300-TH-MM-115']);
});

test('a surface-mount row has no drill, no pin1Shape and rect pads', () => {
  const ws = workspace([row({ mounting: 'Surface Mount', rule: 'R1', drill: '0', pad_w: '3.51', pad_h: '1.55', span: '10.21', pin1: 'none' })]);
  assert.strictEqual(run(ws, '--skip-generic-check').status, 0);
  const [fp] = footprints(ws);
  assert.strictEqual(fp.id, 'SKT-DIP14-300-SM-MM-110');
  assert.strictEqual('drill' in fp, false);
  assert.strictEqual('pin1Shape' in fp, false);
  assert.deepStrictEqual(fp.pad, { width: 3.51, height: 1.55, shape: 'rect' });
  assert.strictEqual(fp.span, 10.21);
});

test('a surface-mount row with equal pad sides has round pads', () => {
  const ws = workspace([row({ mounting: 'Surface Mount', series: '114', rule: 'R1', drill: '0', pad_w: '1.50', pad_h: '1.50', pin1: 'none' })]);
  assert.strictEqual(run(ws, '--skip-generic-check').status, 0);
  assert.deepStrictEqual(footprints(ws)[0].pad, { width: 1.5, height: 1.5, shape: 'round' });
});

test('a series with an underscore gets a dash in the id (ids allow A-Z 0-9 . -)', () => {
  const ws = workspace([row({ maker: 'Assmann', series: 'ASS_0810', drawing: 'ASS_0810_CO.pdf p.1' })]);
  assert.strictEqual(run(ws, '--skip-generic-check').status, 0);
  assert.strictEqual(footprints(ws)[0].id, 'SKT-DIP14-300-TH-AS-ASS-0810');
  assert.match(footprints(ws)[0].id, /^[A-Z0-9.-]+$/);
});

test('a row with no body_len for one of its pin counts fails and writes nothing', () => {
  const ws = workspace([row({ pins: '14,16' })]);
  const result = run(ws, '--skip-generic-check');
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /no body_len for 16 pins/);
  assert.strictEqual(fs.existsSync(ws.out), false);
});

test('without the generic through-hole rows for the 11 DIP shapes the tool exits 1 and writes nothing', () => {
  const ws = workspace([row({})]);
  const result = run(ws);
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /generic DIP sockets are not covered/);
  assert.match(result.stderr, /8-pin 0\.3/);
  assert.strictEqual(fs.existsSync(ws.out), false);
});

test('the 11 shapes supplied once each by generic rows pass; one supplied twice fails', () => {
  const g3 = row({ generic: 'yes', pins: '8,14,16,20,24,28', body_len: '8:10.16;14:17.78;16:20.32;20:25.4;24:30.48;28:35.56' });
  const g6 = row({ generic: 'yes', spacing: '0.6', pins: '24,28,32,40,48', span: '15.24', body_w: '17.78',
    body_len: '24:30.48;28:35.56;32:40.64;40:50.8;48:60.96' });
  const ok = workspace([g3, g6]);
  const result = run(ok);
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(footprints(ok).length, 11);
  const twice = workspace([g3, g6, row({ series: '111', generic: '14' })]);
  const failed = run(twice);
  assert.strictEqual(failed.status, 1);
  assert.match(failed.stderr, /14-pin 0\.3.*generic from .* and /);
});

// ---- PLCC (plan-51-E) ---------------------------------------------------------------------------------------------

const gen = require('./gen-sockets.js');

function plccRow(over) {
  return row(Object.assign({
    maker: 'Assmann', series: 'ASS_0142', family: 'PLCC', mounting: 'Surface Mount', spacing: '-', pins: '28,32', rule: 'R1',
    drill: '0', pad_w: '1.70', pad_h: '0.76', span: '28:15.30/15.25;32:15.30/17.80', pitch: '1.27',
    sides: '28:7,7,7,7;32:9,7,9,7', body_w: '28:18.19;32:18.19', body_len: '28:18.19;32:20.80', pin1: 'none', generic: '-',
    drawing: 'ASS_0142_CO.pdf p.1',
  }, over));
}

function runE(ws, ...extra) {
  return childProcess.spawnSync('node', [TOOL, '--letter', 'E', '--series', ws.series, '--out', ws.out, ...extra], { encoding: 'utf8' });
}

test('the slot formula reproduces the slots of every PLCC chip footprint in footprints.json', () => {
  const file = path.join(__dirname, '..', 'schematical', 'schematical', 'Parts', 'footprints.json');
  const chips = JSON.parse(fs.readFileSync(file, 'utf8')).footprints.filter((f) => /^PLCC-/.test(f.id) && f.style === 'quad');
  assert.ok(chips.length >= 5, `found ${chips.length} PLCC footprints`);
  for (const chip of chips) {
    assert.deepStrictEqual(gen.plccSlots(chip.sides), chip.slots, chip.id);
  }
});

test('pin 1 is on the middle slot of the top side', () => {
  for (const sides of [[7, 7, 7, 7], [9, 7, 9, 7], [11, 11, 11, 11], [17, 17, 17, 17]]) {
    const [l, b, r, t] = sides;
    const slots = gen.plccSlots(sides);
    assert.strictEqual(slots[0], l + b + r + (t + 1) / 2, sides.join(','));
    assert.strictEqual(slots.length, l + b + r + t);
    assert.strictEqual(new Set(slots).size, slots.length);
  }
  assert.throws(() => gen.plccSlots([8, 8, 8, 8]), /odd number/);
});

test('a PLCC row gives one quad object per pin count, with sides, spans, slots and rect pads', () => {
  const ws = workspace([plccRow({})]);
  assert.strictEqual(runE(ws, '--skip-generic-check').status, 0);
  const [a, b] = footprints(ws);
  assert.strictEqual(a.id, 'SKT-PLCC28-SM-AS-ASS-0142');
  assert.strictEqual(b.id, 'SKT-PLCC32-SM-AS-ASS-0142');
  assert.strictEqual(a.style, 'quad');
  assert.deepStrictEqual(b.sides, [9, 7, 9, 7]);
  assert.strictEqual(b.spanX, 15.3);
  assert.strictEqual(b.spanY, 17.8);
  assert.strictEqual(b.pitch, 1.27);
  assert.deepStrictEqual(b.pad, { width: 1.7, height: 0.76, shape: 'rect' });
  assert.strictEqual('drill' in b, false);
  assert.strictEqual(b.slots[0], 29);
  assert.deepStrictEqual(a.body, { width: 18.19, height: 18.19 });
});

test('a PLCC row whose sides do not add to the pin count fails with a message and writes nothing', () => {
  const ws = workspace([plccRow({ sides: '28:7,7,7,6;32:9,7,9,7' })]);
  const result = runE(ws, '--skip-generic-check');
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /sides 7,7,7,6 add to 27, not 28/);
  assert.strictEqual(fs.existsSync(ws.out), false);
});

test('a through-hole PLCC row is refused (the through-hole PLCC groups are out)', () => {
  const ws = workspace([plccRow({ mounting: 'Through Hole' })]);
  const result = runE(ws, '--skip-generic-check');
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /must be surface mount/);
});

test('the four generic PLCC shapes are required, each supplied once', () => {
  const none = workspace([plccRow({})]);
  const failed = runE(none);
  assert.strictEqual(failed.status, 1);
  assert.match(failed.stderr, /generic PLCC sockets are not covered/);
  assert.match(failed.stderr, /44-pin PLCC socket: no generic row/);
  assert.strictEqual(fs.existsSync(none.out), false);

  const a = plccRow({ pins: '28,32,44', generic: '28,32,44', span: '28:15.30/15.25;32:15.30/17.80;44:20.30/20.30',
    sides: '28:7,7,7,7;32:9,7,9,7;44:11,11,11,11', body_w: '28:18.19;32:18.19;44:23.25', body_len: '28:18.19;32:20.80;44:23.25' });
  const b = plccRow({ maker: 'Mill-Max', series: '940', pins: '32,68', generic: '68', span: '32:11.02/13.56;68:23.72/23.72',
    sides: '32:9,7,9,7;68:17,17,17,17', body_w: '32:18.11;68:30.81', body_len: '32:20.65;68:30.81', drawing: '088-145.pdf p.56' });
  const ok = workspace([a, b]);
  const result = runE(ok);
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(footprints(ok).length, 5);
  const twice = workspace([a, plccRow({ maker: 'Mill-Max', series: '940', pins: '32,68', generic: '32,68', span: b.span,
    sides: b.sides, body_w: b.body_w, body_len: b.body_len, drawing: b.drawing })]);
  assert.strictEqual(runE(twice).status, 1);
});

test('a generic row must be through hole', () => {
  const ws = workspace([row({ mounting: 'Surface Mount', generic: 'yes', drill: '0', pin1: 'none' })]);
  const result = run(ws);
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /generic DIP row must be through hole/);
});
