// Tests for scripts/check-datasheets.js (plan-68-T). Run: node --test scripts/check-datasheets.test.js
//
// The script reads the real Parts/datasheets.json (read only) and fetches each
// URL. Here fetch is fixtures/check-datasheets/fetch-stub.js, loaded with
// `node --require`, so nothing reaches the network.

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const path = require('path');
const childProcess = require('child_process');

const TOOL = path.join(__dirname, 'check-datasheets.js');
const STUB = path.join(__dirname, 'fixtures/check-datasheets/fetch-stub.js');
const ENTRIES = require('../schematical/schematical/Parts/datasheets.json').datasheets;

function check(args, kinds = {}) {
  return childProcess.spawnSync('node', ['--require', STUB, TOOL, ...args], {
    encoding: 'utf8',
    env: Object.assign({}, process.env, { CHECK_STUB: JSON.stringify(kinds) }),
  });
}

// The first ten entries, each given one stub kind.
const KINDS = ['pdf', 'moved', 'short', 'offset', 'http404', 'html', 'bare', 'cancel', 'error', 'hang'];
const PICKED = ENTRIES.slice(0, KINDS.length).map((entry, i) => Object.assign({ kind: KINDS[i] }, entry));

test('each kind of reply is reported as ok or with why it failed', () => {
  const kinds = Object.fromEntries(PICKED.map((e) => [e.url, e.kind]));
  const result = check(['--verbose', '--timeout', '1', '--jobs', '3', ...PICKED.map((e) => e.id)], kinds);
  assert.strictEqual(result.status, 1, result.stderr);
  const lines = result.stdout.split('\n').filter((l) => l !== '');
  const why = (e) => {
    const hit = lines.filter((l) => l.includes(` ${e.id}  ${e.url}  `));
    assert.strictEqual(hit.length, 1, `${e.kind}: ${result.stdout}`);
    return hit[0].slice(hit[0].lastIndexOf('  ') + 2);
  };
  const byKind = Object.fromEntries(PICKED.map((e) => [e.kind, e]));
  assert.strictEqual(why(byKind.pdf), 'PDF');
  assert.strictEqual(why(byKind.moved), `PDF -> ${byKind.moved.url}?moved`);
  assert.strictEqual(why(byKind.short), 'PDF');
  assert.strictEqual(why(byKind.offset), 'PDF');
  assert.strictEqual(why(byKind.cancel), 'PDF');
  assert.strictEqual(why(byKind.http404), 'HTTP 404');
  assert.strictEqual(why(byKind.html), 'not a PDF (text/html)');
  assert.strictEqual(why(byKind.bare), 'not a PDF (no content-type)');
  assert.strictEqual(why(byKind.error), 'connect ECONNREFUSED');
  assert.strictEqual(why(byKind.hang), 'timed out');
  for (const e of PICKED) {
    const ok = ['pdf', 'moved', 'short', 'offset', 'cancel'].includes(e.kind);
    assert.ok(lines.some((l) => l.startsWith(ok ? `ok   ${e.id} ` : `FAIL ${e.id} `)), e.kind);
  }
  assert.strictEqual(lines[lines.length - 1], '5 of 10 URLs serve a PDF; 5 failed.');
});

test('without --verbose only failures print, with progress every 100 on stderr', () => {
  const host = 'www.ti.com';
  const count = ENTRIES.filter((e) => new URL(e.url).host === host).length;
  assert.ok(count >= 100, `${count} entries on ${host}`);
  const result = check(['--host', host, '--jobs', '8']);
  assert.strictEqual(result.status, 0, result.stderr);
  assert.strictEqual(result.stdout, `\n${count} of ${count} URLs serve a PDF; 0 failed.\n`);
  assert.ok(result.stderr.startsWith(`  100/${count}\n`), result.stderr);
  assert.strictEqual(result.stderr.split('\n').filter((l) => l !== '').length, Math.floor(count / 100));
});

test('ids pick entries; unknown ids pick none; odd numbers fall back', () => {
  const one = check(['--jobs', 'many', '--timeout', 'soon', PICKED[0].id], { [PICKED[1].url]: 'error' });
  assert.strictEqual(one.status, 0, one.stderr);
  assert.strictEqual(one.stdout, '\n1 of 1 URLs serve a PDF; 0 failed.\n');
  const failed = check([PICKED[1].id], { [PICKED[1].url]: 'http404' });
  assert.strictEqual(failed.status, 1);
  assert.strictEqual(failed.stdout, `FAIL ${PICKED[1].id}  ${PICKED[1].url}  HTTP 404\n\n0 of 1 URLs serve a PDF; 1 failed.\n`);
  const none = check(['no-such-datasheet-id']);
  assert.strictEqual(none.status, 0);
  assert.strictEqual(none.stdout, '\n0 of 0 URLs serve a PDF; 0 failed.\n');
});

test('--help and -h print the help; a bad option prints it on stderr and exits 2', () => {
  for (const flag of ['--help', '-h']) {
    const result = check([flag]);
    assert.strictEqual(result.status, 0);
    assert.match(result.stdout, /^Check the datasheet URLs in schematical\/schematical\/Parts\/datasheets\.json\.\n/);
    assert.match(result.stdout, /--help {11}this text\n$/);
  }
  const unknown = check(['--nope']);
  assert.strictEqual(unknown.status, 2);
  assert.match(unknown.stderr, /^unknown option --nope\n\nCheck the datasheet URLs/);
  const missing = check(['--host']);
  assert.strictEqual(missing.status, 2);
  assert.match(missing.stderr, /^--host needs a value\n\n/);
  assert.strictEqual(missing.stdout, '');
});
