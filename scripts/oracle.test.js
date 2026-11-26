// Tests for backend/tools/avr8js-oracle/oracle.js (plan-62; plan-68-T). Run: node --test scripts/oracle.test.js
//
// Each command is spawned the way avr_core_test / avr_periph_test run it, and
// its output is checked against what those tests compare it with: the hashes
// in backend/tests/avr/golden.txt, and the committed .hex / scenario .json.
// The layout files in scripts/fixtures/avr8js-oracle/ are the C side's
// `avr_periph_test --layout <name> <flash>` output; dasm.txt is a few lines of
// `avr_periph_test --dasm`.

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const ROOT = path.join(__dirname, '..');
const ORACLE = path.join(ROOT, 'backend/tools/avr8js-oracle/oracle.js');
const AVR = path.join(ROOT, 'backend/tests/avr');
const FIXTURES = path.join(__dirname, 'fixtures/avr8js-oracle');

function oracle(...args) {
  return childProcess.spawnSync('node', [ORACLE, ...args], { encoding: 'utf8', maxBuffer: 1 << 28 });
}

// FNV-1a 64 over the text's bytes, as avr_core_test's fnv64_add: two 32-bit halves.
function fnv64(text) {
  const bytes = Buffer.from(text, 'utf8');
  let hi = 0xcbf29ce4;
  let lo = 0x84222325;
  for (let i = 0; i < bytes.length; i++) {
    lo = (lo ^ bytes[i]) >>> 0;
    const low = lo * 0x1b3;
    const carry = Math.floor(low / 0x100000000);
    hi = (Math.imul(hi, 0x1b3) + (lo << 8) + carry) >>> 0;
    lo = low >>> 0;
  }
  return hi.toString(16).padStart(8, '0') + lo.toString(16).padStart(8, '0');
}

const lineCount = (text) => text.split('\n').length - 1;

function golden() {
  const out = { run: new Map() };
  for (const line of fs.readFileSync(path.join(AVR, 'golden.txt'), 'utf8').split('\n')) {
    const f = line.split(' ');
    if (f[0] === 'step') out.step = { seed: f[1], states: f[2], lines: Number(f[3]), hash: f[4] };
    if (f[0] === 'run') out.run.set(f[1], { lines: Number(f[2]), hash: f[3] });
  }
  return out;
}

function tmp() {
  return fs.mkdtempSync(path.join(os.tmpdir(), 'oracle-test-'));
}

const layoutFile = (name, flash) => path.join(FIXTURES, `${name}-${flash}.json`);
const readJson = (file) => JSON.parse(fs.readFileSync(file, 'utf8'));

test('fnv64 is FNV-1a 64 (the empty text and "a")', () => {
  assert.strictEqual(fnv64(''), 'cbf29ce484222325');
  assert.strictEqual(fnv64('a'), 'af63dc4c8601ec8c');
});

test('step <seed> <states> prints the golden step lines', () => {
  const g = golden().step;
  const result = oracle('step', g.seed, g.states);
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(lineCount(result.stdout), g.lines);
  assert.strictEqual(fnv64(result.stdout), g.hash);
  assert.match(result.stdout.slice(0, 40), /^0000 [0-9a-f]{4} 1 [0-9a-f]{2} [0-9a-f]{4} [0-9a-f]{8}\n/);
});

test('every scenario run equals its golden trace', () => {
  const g = golden().run;
  const scenarios = fs.readdirSync(AVR).filter((f) => f.endsWith('.json')).sort();
  assert.strictEqual(scenarios.length, g.size);
  for (const file of scenarios) {
    const name = file.replace(/\.json$/, '');
    const scenario = readJson(path.join(AVR, file));
    const result = oracle('run', path.join(AVR, file), layoutFile(scenario.layout, scenario.flash));
    assert.strictEqual(result.status, 0, `${name}: ${result.stderr}`);
    assert.deepStrictEqual({ lines: lineCount(result.stdout), hash: fnv64(result.stdout) }, g.get(name), name);
  }
});

test('run fails on a stimulus it does not know', () => {
  const dir = tmp();
  const scenario = Object.assign(readJson(path.join(AVR, 'gpio_out.json')), { stimuli: [[0, 'bogus', 1]] });
  fs.writeFileSync(path.join(dir, 's.json'), JSON.stringify(scenario));
  const result = oracle('run', path.join(dir, 's.json'), layoutFile('atmega328', 32768));
  assert.strictEqual(result.status, 1);
  assert.strictEqual(result.stderr, 'unknown stimulus [0,"bogus",1]\n');
});

test('run with no stimuli list runs its steps', () => {
  const dir = tmp();
  const scenario = readJson(path.join(AVR, 'gpio_out.json'));
  delete scenario.stimuli;
  scenario.steps = 3;
  fs.writeFileSync(path.join(dir, 's.json'), JSON.stringify(scenario));
  const result = oracle('run', path.join(dir, 's.json'), layoutFile('atmega328', 32768));
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(result.stdout.split('\n').filter((l) => /^i [0-9a-f]+ [0-9a-f]+ [0-9a-f]{2} [0-9a-f]{4} [0-9a-f]{8}$/.test(l)).length, 3);
});

test('asm of each firmware source equals the committed .hex', () => {
  const names = fs.readdirSync(AVR).filter((f) => f.endsWith('.hex')).map((f) => f.replace(/\.hex$/, ''));
  assert.ok(names.length >= 20);
  for (const name of names) {
    const result = oracle('asm', path.join(AVR, 'src', `${name}.asm`));
    assert.strictEqual(result.status, 0, `${name}: ${result.stderr}`);
    assert.strictEqual(result.stdout, fs.readFileSync(path.join(AVR, `${name}.hex`), 'utf8'), name);
  }
});

test('asm of a source with an error fails with the assembler\'s message', () => {
  const file = path.join(tmp(), 'bad.asm');
  fs.writeFileSync(file, 'BOGUS r1\n');
  const result = oracle('asm', file);
  assert.strictEqual(result.status, 1);
  assert.ok(result.stderr.startsWith(file + ': '), result.stderr);
  assert.strictEqual(result.stdout, '');
});

test('build of each scenario source equals the committed scenario', () => {
  const metas = fs.readdirSync(path.join(AVR, 'src')).filter((f) => f.endsWith('.meta.json'));
  assert.ok(metas.length >= 20);
  for (const meta of metas) {
    const name = meta.replace(/\.meta\.json$/, '');
    const result = oracle('build', path.join(AVR, 'src', `${name}.asm`), path.join(AVR, 'src', meta));
    assert.strictEqual(result.status, 0, `${name}: ${result.stderr}`);
    assert.strictEqual(result.stdout, fs.readFileSync(path.join(AVR, `${name}.json`), 'utf8'), name);
  }
});

test('layout-check: the C side\'s ATmega328 and ATtiny85 layouts equal avr8js\'s', () => {
  for (const [name, flash] of [['atmega328', 32768], ['attiny85', 8192]]) {
    const result = oracle('layout-check', layoutFile(name, flash));
    assert.strictEqual(result.status, 0, result.stderr);
    assert.strictEqual(result.stdout, `layout ${name} equal\n`);
  }
});

function changedLayout(name, flash, change) {
  const layout = readJson(layoutFile(name, flash));
  change(layout);
  const file = path.join(tmp(), 'layout.json');
  fs.writeFileSync(file, JSON.stringify(layout));
  return oracle('layout-check', file);
}

test('layout-check names the first value that differs', () => {
  const result = changedLayout('atmega328', 32768, (l) => { l.clock.CLKPR += 1; });
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /^layout atmega328 differs: clock\.CLKPR: ours \d+, avr8js \d+\n$/);
});

test('layout-check names a key one side lacks', () => {
  const result = changedLayout('atmega328', 32768, (l) => { delete l.adc.ADMUX; });
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /^layout atmega328 differs: adc\.ADMUX: ours undefined, avr8js \d+\n$/);
});

test('layout-check fails on a peripheral avr8js has no config for', () => {
  const result = changedLayout('atmega328', 32768, (l) => { l.timers.push(l.timers[0]); });
  assert.strictEqual(result.status, 1);
  assert.strictEqual(result.stderr, 'layout atmega328: avr8js has no config for timer3\n');
});

test('layout-check of an ATtiny85 compares its Timer1 alone', () => {
  const result = changedLayout('attiny85', 8192, (l) => { l.tinyTimer1.TCCR1 += 1; });
  assert.strictEqual(result.status, 1);
  assert.match(result.stderr, /^layout attiny85 differs: timer1\.TCCR1: ours \d+, avr8js \d+\n$/);
});

test('dasm-check counts round-trips, .DW words and far jumps', () => {
  const result = oracle('dasm-check', path.join(FIXTURES, 'dasm.txt'));
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(result.stdout,
    'dasm: 7 words round-trip (1 are .DW, 1 are a JMP or CALL above the assembler\'s 0x400000)\n');
});

test('dasm-check fails on a text that does not assemble back to its word', () => {
  const file = path.join(tmp(), 'dasm.txt');
  fs.writeFileSync(file, 'e000 1234 LDI R16, 0x01\n');
  const wrong = oracle('dasm-check', file);
  assert.strictEqual(wrong.status, 1);
  assert.strictEqual(wrong.stderr, 'dasm: e000 "LDI R16, 0x01" assembles to 01e0\n');
  fs.writeFileSync(file, 'e000 1234 BOGUS\n');
  const bad = oracle('dasm-check', file);
  assert.strictEqual(bad.status, 1);
  assert.match(bad.stderr, /^dasm: e000 "BOGUS" assembles to .+\n$/);
  assert.doesNotMatch(bad.stderr, /assembles to \n/);
});

test('a wrong command line prints the usage and exits 2', () => {
  for (const args of [[], ['step', '1'], ['nope']]) {
    const result = oracle(...args);
    assert.strictEqual(result.status, 2);
    assert.match(result.stderr, /^usage: node oracle\.js step <seed> <states>\n/);
    assert.match(result.stderr, /node oracle\.js run <scenario\.json> <layout\.json>\n$/);
  }
});
