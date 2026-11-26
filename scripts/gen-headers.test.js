// Tests for scripts/gen-headers.js (plan-23-A; plan-68-T). Run: node --test scripts/gen-headers.test.js
//
// The script writes fixed paths under schematical/schematical/Parts/; here
// fixtures/gen-headers/redirect-parts.js (loaded with `node --require`) points
// those paths at a temporary folder.

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const TOOL = path.join(__dirname, 'gen-headers.js');
const REDIRECT = path.join(__dirname, 'fixtures/gen-headers/redirect-parts.js');
const PARTS = path.join(__dirname, '..', 'schematical/schematical/Parts');
const SUMMARY = 'header.json: 80 parts; footprints.json: 160 PINHDR- footprints\n';

const OTHER_A = '    {"id":"LED-0402","style":"dual","pins":2}';
const OTHER_B = '    {"id":"X1SON-9","style":"quad","pins":9}';
const STALE = '    {"id":"PINHDR-1X01-V","style":"header","pins":99}';

function footprintsText(lines) {
  return ['{', '  "format": "schematical-footprints",', '  "version": 1,', '  "footprints": [',
    ...lines.map((l, i) => (i < lines.length - 1 ? l + ',' : l)), '  ]', '}', ''].join('\n');
}

function workspace(footprints) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'gen-headers-'));
  fs.writeFileSync(path.join(dir, 'footprints.json'), footprints);
  return dir;
}

function run(dir) {
  return childProcess.spawnSync('node', ['--require', REDIRECT, TOOL], {
    encoding: 'utf8',
    env: Object.assign({}, process.env, { GEN_HEADERS_PARTS: dir }),
  });
}

const read = (dir, name) => fs.readFileSync(path.join(dir, name), 'utf8');
const pinhdrLines = (text) => text.split('\n').filter((l) => l.startsWith('    {"id":"PINHDR-')).map((l) => l.replace(/,$/, ''));

test('the generated header.json and PINHDR- lines are the checked-in ones', () => {
  const dir = workspace(footprintsText([OTHER_A, STALE, OTHER_B]));
  const result = run(dir);
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(result.stdout, SUMMARY);
  assert.strictEqual(read(dir, 'header.json'), fs.readFileSync(path.join(PARTS, 'header.json'), 'utf8'));
  assert.deepStrictEqual(pinhdrLines(read(dir, 'footprints.json')),
    pinhdrLines(fs.readFileSync(path.join(PARTS, 'footprints.json'), 'utf8')));
});

test('other footprints are kept in order, the PINHDR- lines replaced and put at the end', () => {
  const dir = workspace(footprintsText([OTHER_A, STALE, OTHER_B]));
  assert.strictEqual(run(dir).status, 0);
  const text = read(dir, 'footprints.json');
  const json = JSON.parse(text);
  assert.strictEqual(json.format, 'schematical-footprints');
  assert.deepStrictEqual(json.footprints.slice(0, 2).map((f) => f.id), ['LED-0402', 'X1SON-9']);
  assert.strictEqual(json.footprints.length, 162);
  const first = json.footprints[2];
  assert.deepStrictEqual(first, {
    id: 'PINHDR-1X01-V', description: '1x1 pin header, 2.54 mm pitch, vertical',
    note: 'Würth WR-PHD 61300111121 p.1: hole Ø1.10, pitch 2.54, body 2.54 deep; pad R5 (drill + 0.7)',
    style: 'header', pins: 1, rows: 1, body: { width: 2.54, height: 2.54 }, pitch: 2.54, firstRowY: 0,
    pad: { width: 1.8, height: 1.8, shape: 'round' }, drill: 1.1, pin1Shape: 'rect',
  });
  const last = json.footprints[161];
  assert.strictEqual(last.id, 'PINHDR-2X40-RA');
  assert.strictEqual(last.rowPitch, -2.54);
  assert.strictEqual(last.pins, 80);
  assert.strictEqual(last.body.width, 101.6);
  assert.ok(text.endsWith('"pin1Shape":"rect"}\n  ]\n}\n'));
});

test('header.json has both devices with 40 sizes, each with straight and right-angle choices', () => {
  const dir = workspace(footprintsText([OTHER_A]));
  assert.strictEqual(run(dir).status, 0);
  const json = JSON.parse(read(dir, 'header.json'));
  assert.deepStrictEqual(json.devices.map((d) => [d.id, d.description, d.parts.length]),
    [['header-1row', 'Generic pin header, 1 row', 40], ['header-2row', 'Generic pin header, 2 rows', 40]]);
  const part = json.devices[1].parts[2];
  assert.deepStrictEqual(part, {
    name: 'GENERIC-HEADER-2X03', manufacturer: 'Generic', package: 'Straight', pinout: '2x03', footprint: 'PINHDR-2X03-V',
    footprintChoices: [{ package: 'Straight', footprint: 'PINHDR-2X03-V' }, { package: 'Right angle', footprint: 'PINHDR-2X03-RA' }],
  });
  assert.strictEqual(json.devices[1].pinouts['2x03'].length, 6);
});

test('a second run writes nothing', () => {
  const dir = workspace(footprintsText([OTHER_A]));
  assert.strictEqual(run(dir).status, 0);
  const old = new Date('2001-01-01T00:00:00Z');
  for (const name of ['header.json', 'footprints.json']) fs.utimesSync(path.join(dir, name), old, old);
  const again = run(dir);
  assert.strictEqual(again.status, 0, again.stderr);
  assert.strictEqual(again.stdout, SUMMARY);
  for (const name of ['header.json', 'footprints.json']) {
    assert.strictEqual(fs.statSync(path.join(dir, name)).mtime.getTime(), old.getTime(), `${name} was rewritten`);
  }
});

test('a footprints.json without the footprints block fails and leaves it alone', () => {
  const broken = '{\n  "format": "schematical-footprints",\n  "items": [\n  ]\n}\n';
  const dir = workspace(broken);
  const result = run(dir);
  assert.notStrictEqual(result.status, 0);
  assert.match(result.stderr, /footprints\.json: no "footprints": \[ … \] block/);
  assert.strictEqual(read(dir, 'footprints.json'), broken);
});
