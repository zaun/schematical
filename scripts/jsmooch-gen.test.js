// Tests for backend/tools/jsmooch-gen/run.js (plan-14-G; plan-68-T). Run: node --test scripts/jsmooch-gen.test.js
//
// The check cpu_core_test makes: the generator, run into a temporary file,
// reproduces the vendored wdc65816_generated_opcodes.c byte for byte.

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const ROOT = path.join(__dirname, '..');
const RUN = path.join(ROOT, 'backend/tools/jsmooch-gen/run.js');
const VENDORED = path.join(ROOT, 'backend/src/third_party/jsmooch/wdc65816_generated_opcodes.c');

test('run.js <out.c> writes the vendored 65816 opcode table', () => {
  const out = path.join(fs.mkdtempSync(path.join(os.tmpdir(), 'jsmooch-gen-')), 'opcodes.c');
  const result = childProcess.spawnSync('node', [RUN, out], { encoding: 'utf8' });
  assert.strictEqual(result.status, 0, result.stderr);
  const written = fs.readFileSync(out, 'utf8');
  assert.ok(written.length > 100000, `only ${written.length} bytes`);
  assert.strictEqual(written, fs.readFileSync(VENDORED, 'utf8'));
});
