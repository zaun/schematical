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

// ---- --write / --check (plan-51-F) -------------------------------------------------------------------------------

const CAND_HEADER = ['maker', 'mpn', 'series', 'family', 'pins', 'spacing', 'mounting', 'stock', 'status', 'datasheet', 'pdf', 'state',
  'reason', 'stock_checked', 'finish'];

function cand(over) {
  return Object.assign({
    maker: 'Mill-Max', mpn: '110-93-314-41-001000', series: '110', family: 'DIP', pins: '14', spacing: '0.3', mounting: 'Through Hole',
    stock: '100', status: 'Active', datasheet: '-', pdf: 'datasheets/ic-socket/mill-max/088-145.pdf', state: 'add', reason: '',
    stock_checked: '2026-11-24', finish: 'gold',
  }, over);
}

// A tree with two devices: a 14-pin 0.3 in DIP and a 28-pin PLCC, each with a generic row and one maker part.
function tree() {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'gen-sockets-root-'));
  const put = (rel, text) => {
    fs.mkdirSync(path.dirname(path.join(root, rel)), { recursive: true });
    fs.writeFileSync(path.join(root, rel), text);
  };
  const tsv = (header, rows) => [header.join('\t'), ...rows.map((r) => header.map((h) => r[h]).join('\t'))].join('\n') + '\n';
  put('planning/plan-51-data/series.tsv', tsv(HEADER, [
    row({ generic: 'yes' }),
    plccRow({ pins: '28', generic: 'yes', span: '28:15.30/15.25', sides: '28:7,7,7,7', body_w: '28:18.19', body_len: '28:18.19' }),
  ]));
  put('planning/plan-51-data/candidates.tsv', tsv(CAND_HEADER, [
    cand({}),
    cand({ mpn: '110-93-314-41-009999', state: 'out', reason: 'no stock' }),
    cand({ maker: 'Assmann', mpn: 'A-CCS 028-Z-SM', series: 'ASS_0142', family: 'PLCC', pins: '28', spacing: '', mounting: 'Surface Mount',
      pdf: 'datasheets/ic-socket/assmann/ASS_0142_CO.pdf', finish: 'tin' }),
  ]));
  put('planning/plan-51-data/fits.tsv', 'family\tpins\tspacing\tfits\nDIP\t14\t0.3\tPDIP-14,CDIP-14\nPLCC\t28\t\tPLCC-28\n');
  put('datasheets/ic-socket/mill-max/datasheets.tsv', 'file\turl\tbytes\tparts\n088-145.pdf\thttps://www.mill-max.com/catalog/088-145.pdf\t1\t1\n');
  put('datasheets/ic-socket/assmann/datasheets.tsv', 'file\turl\tbytes\tparts\nASS_0142_CO.pdf\thttps://www.assmann-wsw.com/uploads/datasheets/ASS_0142_CO.pdf\t1\t1\n');
  put('schematical/schematical/Parts/footprints.json', `{
  "format": "schematical-footprints",
  "version": 1,
  "footprints": [
    {"id":"PDIP-14","style":"dual","pins":14},
    {"id":"SKT-OLD-STALE","style":"dual","pins":2},
    {"id":"PLCC-28","style":"quad","pins":28}
  ]
}
`);
  put('schematical/schematical/Parts/datasheets.json', `{
  "format": "schematical-datasheets",
  "version": 1,
  "datasheets": [
    {"id": "assmann-ass-0142-co", "url": "https://old.example/stale.pdf"},
    {"id": "sn74hc00", "url": "https://www.ti.com/lit/ds/symlink/sn74hc00.pdf"}
  ]
}
`);
  return root;
}

function gensock(root, ...args) {
  return childProcess.spawnSync('node', [TOOL, ...args, '--root', root, '--skip-generic-check'], { encoding: 'utf8' });
}

const readJson = (root, rel) => JSON.parse(fs.readFileSync(path.join(root, 'schematical', 'schematical', 'Parts', rel), 'utf8'));

test('the datasheet id is the local PDF name in lower case under the maker', () => {
  assert.strictEqual(gen.datasheetId('Assmann', 'datasheets/ic-socket/assmann/ASS_4852_CO.pdf'), 'assmann-ass-4852-co');
  assert.strictEqual(gen.datasheetId('Preci-Dip', 'datasheets/ic-socket/preci-dip/CATALOG-DILSOCKETS.pdf'), 'precidip-catalog-dilsockets');
  assert.strictEqual(gen.datasheetId('Mill-Max', 'datasheets/ic-socket/mill-max/088-145.pdf'), 'mill-max-088-145');
});

test('--write makes one device per fits.tsv row, a generic part and one part per add row', () => {
  const root = tree();
  const result = gensock(root, '--write');
  assert.strictEqual(result.status, 0, result.stderr);
  const doc = readJson(root, 'socket.json');
  assert.strictEqual(doc.kind, 'socket');
  assert.deepStrictEqual(doc.devices.map((d) => d.id), ['dip-14-300', 'plcc-28']);
  const dip = doc.devices[0];
  assert.deepStrictEqual(dip.drawer, ['Connector', 'IC Socket', 'DIP']);
  assert.strictEqual(dip.family, 'DIP');
  assert.strictEqual(dip.rowSpacing, 7.62);
  assert.strictEqual(dip.function, '14-pin, 0.3 in');
  assert.strictEqual(dip.pinouts.p14.length, 14);
  assert.deepStrictEqual(dip.pinouts.p14[13], { number: 14, name: '14', role: 'inout' });
  assert.deepStrictEqual(dip.parts.map((p) => p.name), ['GENERIC-SOCKET-DIP-14-300', '110-93-314-41-001000']);
  const [generic, maker] = dip.parts;
  assert.deepStrictEqual(generic, {
    name: 'GENERIC-SOCKET-DIP-14-300', manufacturer: 'Generic', package: 'DIP socket', pinout: 'p14', footprint: 'SKT-DIP14-300-TH-MM-110',
    mounting: 'through-hole', fits: ['PDIP-14', 'CDIP-14'],
  });
  assert.deepStrictEqual(maker, {
    name: '110-93-314-41-001000', manufacturer: 'Mill-Max Manufacturing Corp.', datasheet: 'mill-max-088-145', package: 'DIP socket',
    pinout: 'p14', footprint: 'SKT-DIP14-300-TH-MM-110', mounting: 'through-hole', finish: 'gold', fits: ['PDIP-14', 'CDIP-14'],
  });
  const plcc = doc.devices[1];
  assert.strictEqual('rowSpacing' in plcc, false);
  assert.deepStrictEqual(plcc.parts.map((p) => p.name), ['GENERIC-SOCKET-PLCC-28', 'A-CCS 028-Z-SM']);
  assert.strictEqual(plcc.parts[0].mounting, 'surface-mount');
  assert.strictEqual(plcc.parts[1].finish, 'tin');
  assert.strictEqual(plcc.parts[1].footprint, 'SKT-PLCC28-SM-AS-ASS-0142');
});

test('--write replaces the SKT- footprints and the maker datasheet entries and keeps every other line', () => {
  const root = tree();
  assert.strictEqual(gensock(root, '--write').status, 0);
  const ids = readJson(root, 'footprints.json').footprints.map((f) => f.id);
  assert.deepStrictEqual(ids, ['PDIP-14', 'PLCC-28', 'SKT-DIP14-300-TH-MM-110', 'SKT-PLCC28-SM-AS-ASS-0142']);
  const sheets = readJson(root, 'datasheets.json').datasheets;
  assert.deepStrictEqual(sheets.map((s) => s.id), ['sn74hc00', 'mill-max-088-145', 'assmann-ass-0142-co']);
  assert.strictEqual(sheets.find((s) => s.id === 'assmann-ass-0142-co').url, 'https://www.assmann-wsw.com/uploads/datasheets/ASS_0142_CO.pdf');
});

test('--write twice changes nothing, and --check agrees; a hand edit makes --check fail', () => {
  const root = tree();
  assert.strictEqual(gensock(root, '--write').status, 0);
  const files = ['socket.json', 'footprints.json', 'datasheets.json'].map((f) => path.join(root, 'schematical', 'schematical', 'Parts', f));
  const before = files.map((f) => fs.readFileSync(f, 'utf8'));
  const again = gensock(root, '--write');
  assert.match(again.stdout, /\(0 files changed\)/);
  assert.deepStrictEqual(files.map((f) => fs.readFileSync(f, 'utf8')), before);
  assert.strictEqual(gensock(root, '--check').status, 0);
  fs.writeFileSync(files[0], before[0].replace('"finish":"gold"', '"finish":"tin"'));
  const stale = gensock(root, '--check');
  assert.strictEqual(stale.status, 1);
  assert.match(stale.stderr, /socket\.json/);
});

test('--write fails, writing nothing, when an add row has no series.tsv row', () => {
  const root = tree();
  const file = path.join(root, 'planning', 'plan-51-data', 'candidates.tsv');
  fs.writeFileSync(file, fs.readFileSync(file, 'utf8').replace('\t110\tDIP', '\t999\tDIP'));
  const result = gensock(root, '--write');
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /no series\.tsv row for 110-93-314-41-001000/);
  assert.strictEqual(fs.existsSync(path.join(root, 'schematical', 'schematical', 'Parts', 'socket.json')), false);
});

test('--write without the generic coverage fails', () => {
  const root = tree();
  const result = childProcess.spawnSync('node', [TOOL, '--write', '--root', root], { encoding: 'utf8' });
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /generic sockets are not covered/);
});

test('a generic row must be through hole', () => {
  const ws = workspace([row({ mounting: 'Surface Mount', generic: 'yes', drill: '0', pin1: 'none' })]);
  const result = run(ws);
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /generic DIP row must be through hole/);
});
