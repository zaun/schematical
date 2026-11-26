// Tests for scripts/source-size.js (plan-68-A). Run: node --test scripts/source-size.test.js
'use strict';
const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');
const size = require('./source-size.js');

const TOOL = path.join(__dirname, 'source-size.js');

test('blank and comment-only lines do not count, in every language', () => {
  assert.strictEqual(size.countCode('let a = 1\n\n   \n// note\nlet b = 2\n', '.swift'), 2);
  assert.strictEqual(size.countCode('int a;\n/* one\n   two */\nint b; // tail\n', '.c'), 2);
  assert.strictEqual(size.countCode('#define X 1\n', '.h'), 1);
  assert.strictEqual(size.countCode('x = 1\n# note\n  # indented\n\ny = 2', '.py'), 2);
  assert.strictEqual(size.countCode('#!/bin/sh\necho hi\n', '.sh'), 1);
  assert.strictEqual(size.countCode('', '.js'), 0);
  assert.strictEqual(size.countCode('a();', '.js'), 1);
});

test('Swift block comments nest', () => {
  assert.strictEqual(size.countCode('/* a /* b */ still comment */\nlet x = 1\n', '.swift'), 1);
  // In C the first */ ends the comment, so the rest of the line is code.
  assert.strictEqual(size.countCode('/* a /* b */ still */\n', '.c'), 1);
});

test('text inside strings is code', () => {
  assert.strictEqual(size.countCode('let s = """\n// not a comment\n\n"""\n', '.swift'), 3);
  assert.strictEqual(size.countCode('let s = #"a"b " // x"#\n// c\n', '.swift'), 1);
  assert.strictEqual(size.countCode('let s = ##"""\n"""#\n/* in */\n"""##\n', '.swift'), 4);
  assert.strictEqual(size.countCode('let s = "a\\"/* b"\nlet t = 1\n', '.swift'), 2);
  assert.strictEqual(size.countCode("char c = '/'; char d = '\\'';\n// x\n", '.c'), 1);
  assert.strictEqual(size.countCode('const t = `\n/* in template */\n`;\n', '.js'), 3);
  assert.strictEqual(size.countCode("const s = '//';\n", '.js'), 1);
  // In Swift a # that does not start a raw string is code.
  assert.strictEqual(size.countCode('#if DEBUG\n#endif\n', '.swift'), 2);
  // An apostrophe is not a quote in Swift.
  assert.strictEqual(size.countCode("let it's = 1 // a\n", '.swift'), 1);
});

test('an unknown extension is an error', () => {
  assert.throws(() => size.countCode('x', '.rb'), /no counter for \.rb/);
});

test('scope: the excluded folders, vendored files and other extensions are out', () => {
  assert.ok(size.inScope('schematical/schematical/Canvas/X.swift'));
  assert.ok(size.inScope('backend/src/models/gate.c'));
  assert.ok(size.inScope('backend/tests/serial_test.c'));
  assert.ok(size.inScope('backend/tools/jsmooch-gen/run.js'));
  assert.ok(size.inScope('scripts/kicad-compare.py'));
  for (const out of ['datasheets/a/x.c', 'planning/plan-1-data/x.js', 'examples/x.swift',
    'routing-example/x.c', 'backend/src/third_party/cjson/cJSON.c', 'backend/tools/avr8js/dist/x.js',
    'schematical/schematical/Datasheet/Renderer/markdown-it.min.js',
    'backend/tools/jsmooch-gen/generation.js', 'backend/tools/jsmooch-gen/wdc65816_opcodes.js',
    'backend/tools/jsmooch-gen/wdc65816_core_generator.js', 'docs/testing.md', 'x.html', 'x.css', 'x.json']) {
    assert.ok(!size.inScope(out), out);
  }
});

test('check: over, grew, stale and pass', () => {
  const files = [{ file: 'a.c', code: 1001 }, { file: 'b.c', code: 1200 }, { file: 'c.c', code: 1100 },
    { file: 'd.c', code: 900 }, { file: 'e.c', code: 1300 }, { file: 'f.c', code: 10 }];
  const ratchet = new Map([['b.c', 1100], ['c.c', 1200], ['d.c', 1050], ['e.c', 1300], ['gone.c', 2000]]);
  assert.deepStrictEqual(size.check(files, ratchet), [
    'over: a.c 1001',
    'grew: b.c 1200 > 1100',
    'stale: c.c 1100 (row 1200)',
    'stale: d.c 900 (row 1050)',
    'stale: gone.c gone (row 2000)',
  ]);
  assert.deepStrictEqual(size.check([{ file: 'x.c', code: 1000 }], new Map()), []);
  // A row at exactly its count but at or under the limit is stale.
  assert.deepStrictEqual(size.check([{ file: 'y.c', code: 1000 }], new Map([['y.c', 1000]])), ['stale: y.c 1000 (row 1000)']);
});

function repo(files) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'source-size-'));
  childProcess.execFileSync('git', ['init', '-q', root]);
  for (const [name, text] of Object.entries(files)) {
    fs.mkdirSync(path.dirname(path.join(root, name)), { recursive: true });
    fs.writeFileSync(path.join(root, name), text);
  }
  childProcess.execFileSync('git', ['-C', root, 'add', '-A']);
  return root;
}

function run(args) {
  return childProcess.spawnSync('node', [TOOL, ...args], { encoding: 'utf8' });
}

const big = 'x();\n'.repeat(1001);

test('the command line: list, --all, --check, unknown argument', () => {
  const root = repo({ 'src/big.c': big, 'src/small.c': 'int a;\n', 'planning/huge.c': big, 'notes.md': big });
  let r = run(['--root', root]);
  assert.strictEqual(r.status, 0);
  assert.strictEqual(r.stdout, '1001\tsrc/big.c\n');
  r = run(['--all', '--root', root]);
  assert.strictEqual(r.stdout, '1001\tsrc/big.c\n1\tsrc/small.c\n');
  r = run(['--check', '--root', root]);
  assert.strictEqual(r.status, 1);
  assert.strictEqual(r.stdout, 'over: src/big.c 1001\n');
  fs.mkdirSync(path.join(root, 'scripts'));
  fs.writeFileSync(path.join(root, 'scripts/source-size-ratchet.tsv'), '1001\tsrc/big.c\n\n');
  r = run(['--check', '--root', root]);
  assert.strictEqual(r.status, 0, r.stdout);
  assert.strictEqual(r.stdout, '');
  r = run(['--bogus']);
  assert.strictEqual(r.status, 2);
  assert.match(r.stderr, /unknown argument --bogus/);
});

test('a tracked file deleted from the working tree is skipped', () => {
  const root = repo({ 'a.c': 'int a;\n', 'b.c': 'int b;\n' });
  fs.unlinkSync(path.join(root, 'b.c'));
  assert.deepStrictEqual(size.census(root), [{ file: 'a.c', code: 1 }]);
});

test('main writes to the streams it is given', () => {
  const root = repo({ 'a.c': 'int a;\n' });
  const out = [];
  const code = size.main(['--all', '--root', root], { write: (s) => out.push(s) }, { write: () => {} });
  assert.strictEqual(code, 0);
  assert.deepStrictEqual(out, ['1\ta.c\n']);
  assert.deepStrictEqual(size.readRatchet(root), new Map());
});
