// Tests for scripts/coverage.js (plan-68-A). The tiers' tools are replaced by a
// fake runner that answers from fixtures, so nothing is built here.
// Run: node --test scripts/coverage.test.js
'use strict';
const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');
const cov = require('./coverage.js');

const TOOL = path.join(__dirname, 'coverage.js');
const FIXTURES = path.join(__dirname, 'fixtures', 'coverage');

function fixture(name, root) {
  return fs.readFileSync(path.join(FIXTURES, name), 'utf8').split('ROOT').join(root);
}

function tmp(prefix) {
  return fs.realpathSync(fs.mkdtempSync(path.join(os.tmpdir(), prefix)));
}

// A runner that does rm/mkdir/find for real and asks `answer` about the rest.
function fakeRun(answer, calls = []) {
  return (cmd, args, options = {}) => {
    calls.push([cmd, ...args]);
    if (cmd === 'rm' || cmd === 'mkdir' || cmd === 'find') return cov.defaultRun(cmd, args, options);
    return { status: 0, stdout: '', stderr: '', ...(answer(cmd, args, options) || {}) };
  };
}

function sink() {
  const parts = [];
  return { write: (s) => parts.push(s), text: () => parts.join('') };
}

test('tierOf names the tier that measures each path', () => {
  assert.strictEqual(cov.tierOf('backend/src/models/gate.c'), 'backend');
  assert.strictEqual(cov.tierOf('backend/src/models/gate.h'), 'backend');
  assert.strictEqual(cov.tierOf('basicsim/main.c'), 'backend');
  assert.strictEqual(cov.tierOf('backend/src/third_party/cjson/cJSON.c'), null);
  assert.strictEqual(cov.tierOf('backend/tests/gate_test.c'), null);
  assert.strictEqual(cov.tierOf('schematical/schematical/Canvas/X.swift'), 'app');
  assert.strictEqual(cov.tierOf('schematical/schematicalTests/XTests.swift'), null);
  assert.strictEqual(cov.tierOf('scripts/digikey.js'), 'scripts');
  assert.strictEqual(cov.tierOf('scripts/kicad-compare.py'), 'scripts');
  assert.strictEqual(cov.tierOf('scripts/digikey.test.js'), null);
  assert.strictEqual(cov.tierOf('scripts/harness/x.swift'), null);
  assert.strictEqual(cov.tierOf('schematical/schematical/Datasheet/Renderer/datasheet.js'), 'scripts');
  assert.strictEqual(cov.tierOf('backend/tools/avr8js-oracle/oracle.js'), 'scripts');
  assert.strictEqual(cov.tierOf('backend/tools/jsmooch-gen/run.js'), 'scripts');
  assert.strictEqual(cov.tierOf('scripts/safari-digikey.sh'), null);
});

test('parseLcov keeps the files asked for and the highest count of a line', () => {
  const root = '/repo';
  const files = cov.parseLcov(fixture('sample.lcov', root), root, (f) => cov.tierOf(f) === 'backend');
  assert.deepStrictEqual([...files.keys()], ['backend/src/a.c', 'basicsim/main.c']);
  assert.deepStrictEqual([...files.get('backend/src/a.c')], [[1, 5], [2, 0], [3, 2], [4, 0]]);
  assert.deepStrictEqual(cov.uncovered(files.get('backend/src/a.c')), [2, 4]);
});

test('parseXccovArchive keeps executable lines; a missing count is 0', () => {
  const root = '/repo';
  const files = cov.parseXccovArchive(fixture('archive.json', root), root, (f) => cov.tierOf(f) === 'app');
  assert.deepStrictEqual([...files.keys()], ['schematical/schematical/A.swift']);
  assert.deepStrictEqual([...files.get('schematical/schematical/A.swift')], [[2, 3], [3, 0], [4, 1], [5, 0]]);
});

test('parseTraceCover reads counted and missed lines', () => {
  assert.deepStrictEqual([...cov.parseTraceCover(fixture('sample.cover', '/r'))], [[3, 1], [5, 1], [6, 2], [8, 0], [9, 0]]);
});

test('check and rewriteRatchet judge only the measured tiers under the prefix', () => {
  const files = new Map([
    ['backend/src/a.c', new Map([[1, 0], [2, 0]])],
    ['backend/src/b.c', new Map([[1, 0]])],
    ['backend/src/c.c', new Map([[1, 1]])],
    ['backend/src/d.c', new Map([[1, 0]])],
  ]);
  const ratchet = new Map([['backend/src/a.c', 1], ['backend/src/b.c', 3], ['backend/src/d.c', 1],
    ['backend/src/gone.c', 4], ['schematical/schematical/X.swift', 9]]);
  assert.deepStrictEqual(cov.check(files, ratchet, ['backend'], null), [
    'worse: backend/src/a.c 2 > 1',
    'stale: backend/src/b.c 1 (row 3)',
    'stale: backend/src/gone.c gone (row 4)',
  ]);
  assert.deepStrictEqual(cov.check(files, ratchet, ['backend'], 'backend/src/d'), []);
  assert.deepStrictEqual(cov.check(files, ratchet, ['app'], null), ['stale: schematical/schematical/X.swift gone (row 9)']);
  assert.strictEqual(cov.rewriteRatchet(files, ratchet, ['backend'], null),
    '2\tbackend/src/a.c\n1\tbackend/src/b.c\n1\tbackend/src/d.c\n9\tschematical/schematical/X.swift\n');
  assert.strictEqual(cov.rewriteRatchet(files, ratchet, ['backend'], 'backend/src/a'),
    '2\tbackend/src/a.c\n3\tbackend/src/b.c\n1\tbackend/src/d.c\n4\tbackend/src/gone.c\n9\tschematical/schematical/X.swift\n');
});

test('parseArgs reads every option and defaults to all tiers', () => {
  assert.deepStrictEqual(cov.parseArgs([]), { tiers: ['backend', 'app', 'scripts'], only: null, tests: [], check: false, write: false });
  assert.deepStrictEqual(cov.parseArgs(['--app', '--backend', '--scripts', '--only', 'x/', '--tests', 'A,,B', '--check', '--write-ratchet']),
    { tiers: ['app', 'backend', 'scripts'], only: 'x/', tests: ['A', 'B'], check: true, write: true });
  assert.throws(() => cov.parseArgs(['--only']), cov.TierError);
  assert.throws(() => cov.parseArgs(['--nope']), /unknown argument --nope/);
});

test('defaultRun returns the exit status, and -1 for a killed process', () => {
  assert.strictEqual(cov.defaultRun('node', ['-e', 'process.stdout.write("hi"); process.exit(3)']).status, 3);
  assert.strictEqual(cov.defaultRun('node', ['-e', 'process.stdout.write("hi")']).stdout, 'hi');
  assert.strictEqual(cov.defaultRun('node', ['-e', 'process.kill(process.pid, "SIGKILL")']).status, -1);
});

function backendAnswer(root, opts = {}) {
  return (cmd, args, options) => {
    if (cmd === 'ctest' && args.includes('--show-only=json-v1')) {
      const dir = args[args.indexOf('--test-dir') + 1];
      return { stdout: JSON.stringify({ tests: [{ command: [`${dir}/b_test`] }, { command: ['/usr/bin/node', 'x.js'] }, { command: [`${dir}/a_test`] }, { command: [`${dir}/a_test`] }] }) };
    }
    if (cmd === 'ctest') {
      if (opts.profile !== false) fs.writeFileSync(options.env.LLVM_PROFILE_FILE.replace('%p', '1'), 'raw');
      return opts.fail ? { status: 8, stdout: '1/2 Test #1: a_test ...***Failed\n2/2 Test #2: b_test ... Passed\n' } : {};
    }
    if (cmd === 'xcrun' && args[0] === 'llvm-cov') return { stdout: fixture('sample.lcov', root) };
    if (cmd === 'cmake' && opts.cmake) return { status: 1, stderr: 'line1\nCMake Error: boom\n' };
    return null;
  };
}

test('backend tier: builds instrumented, merges, exports, reports', () => {
  const root = tmp('cov-root-');
  const work = tmp('cov-work-');
  const calls = [];
  const out = sink(), err = sink();
  const code = cov.main(['--backend'], { root, work, out, err, run: fakeRun(backendAnswer(root, { fail: true }), calls) });
  assert.strictEqual(code, 0);
  assert.strictEqual(out.text(), 'backend/src/a.c:2\nbackend/src/a.c:4\n2\t4\tbackend/src/a.c\nbackend: 5 lines, 2 uncovered, 60.00%\n');
  assert.match(err.text(), /backend: ctest reported failures:\n1\/2 Test #1: a_test \.\.\.\*\*\*Failed\n/);
  const exp = calls.find((c) => c[0] === 'xcrun' && c[1] === 'llvm-cov');
  assert.deepStrictEqual(exp.slice(-4), ['-object', `${work}/backend/a_test`, '-object', `${work}/backend/b_test`]);
  assert.ok(calls.some((c) => c[0] === 'cmake' && c.includes(path.join(root, 'basicsim'))));
});

test('backend tier: no profile written, and a failing build, are errors', () => {
  const root = tmp('cov-root-');
  const err = sink();
  assert.strictEqual(cov.main(['--backend'], { root, work: tmp('cov-work-'), out: sink(), err, run: fakeRun(backendAnswer(root, { profile: false })) }), 3);
  assert.match(err.text(), /backend: no profile was written/);
  const err2 = sink();
  assert.strictEqual(cov.main(['--backend'], { root, work: tmp('cov-work-'), out: sink(), err: err2, run: fakeRun(backendAnswer(root, { cmake: true })) }), 3);
  assert.match(err2.text(), /cmake configure failed \(exit 1\): line1 \| CMake Error: boom/);
});

function appAnswer(root, opts = {}) {
  return (cmd, args) => {
    if (cmd === 'perl') return { status: 65, stdout: "Test Case '-[T a]' passed\nTest Case '-[T b]' failed (1 s)\n" };
    if (cmd === 'xcrun' && args[0] === 'xccov') return { stdout: opts.empty ? '{}' : fixture('archive.json', root) };
    return null;
  };
}

test('app tier: runs the named classes unsigned with coverage, kills the host, reads the archive', () => {
  const root = tmp('cov-root-');
  const work = tmp('cov-work-');
  const calls = [];
  const out = sink(), err = sink();
  assert.strictEqual(cov.main(['--app', '--tests', 'NetLabelTests,BusTests'], { root, work, out, err, run: fakeRun(appAnswer(root), calls) }), 0);
  assert.strictEqual(out.text(), 'schematical/schematical/A.swift:3\nschematical/schematical/A.swift:5\n2\t4\tschematical/schematical/A.swift\napp: 4 lines, 2 uncovered, 50.00%\n');
  assert.match(err.text(), /xcodebuild exited 65; 1 failed test cases:\nTest Case '-\[T b\]' failed/);
  const perl = calls.find((c) => c[0] === 'perl');
  assert.ok(perl.includes('CODE_SIGNING_ALLOWED=NO'));
  assert.ok(perl.includes('-only-testing:schematicalTests/NetLabelTests'));
  assert.ok(perl.includes('-only-testing:schematicalTests/BusTests'));
  assert.ok(calls.some((c) => c[0] === 'pkill'));
});

test('app tier: a bundle with no Swift coverage is an error', () => {
  const root = tmp('cov-root-');
  const err = sink();
  assert.strictEqual(cov.main(['--app'], { root, work: tmp('cov-work-'), out: sink(), err, run: fakeRun(appAnswer(root, { empty: true })) }), 3);
  assert.match(err.text(), /no Swift coverage/);
});

function scriptsRepo(files) {
  const root = tmp('cov-scripts-');
  childProcess.execFileSync('git', ['init', '-q', root]);
  for (const [name, text] of Object.entries(files)) {
    fs.mkdirSync(path.dirname(path.join(root, name)), { recursive: true });
    fs.writeFileSync(path.join(root, name), text);
  }
  childProcess.execFileSync('git', ['-C', root, 'add', '-A']);
  return root;
}

function scriptsAnswer(root, opts = {}) {
  return (cmd, args, options) => {
    if (cmd === 'node') {
      const dest = args.find((a) => a.startsWith('--test-reporter-destination=')).split('=')[1];
      fs.writeFileSync(dest, `SF:${root}/scripts/x.js\nDA:1,1\nDA:2,0\nend_of_record\n`);
      return null;
    }
    if (cmd === 'python3' && args[0] === '-m' && args.includes('--count')) {
      if (opts.counts !== false) fs.writeFileSync(options.env.SCRIPTS_COVERAGE_FILE, 'counts');
      return null;
    }
    if (cmd === 'python3' && args.includes('--report')) {
      const dir = args.find((a) => a.startsWith('--coverdir=')).split('=')[1];
      fs.mkdirSync(dir, { recursive: true });
      fs.writeFileSync(path.join(dir, 'p.cover'), fixture('sample.cover', root));
      fs.writeFileSync(path.join(dir, 'unittest.case.cover'), '');
      return null;
    }
    if (cmd === 'python3' && args[0] === '-c') return cov.defaultRun(cmd, args, options);
    return null;
  };
}

const SCRIPT_FILES = {
  'scripts/x.js': 'a();\nb();\n',
  'scripts/y.js': '// never loaded\nc();\n\nd();\n',
  'scripts/x.test.js': 'require("./x.js");\n',
  'scripts/p.py': 'import sys\n',
  'scripts/q.py': '"""Doc."""\nimport os\n\ndef f():\n    return 1\n',
};

test('scripts tier: node coverage, never-loaded JavaScript, Python under trace or never run', () => {
  const root = scriptsRepo({ ...SCRIPT_FILES, 'scripts/tests/__init__.py': '', 'scripts/tests/test_p.py': '' });
  const calls = [];
  const out = sink();
  assert.strictEqual(cov.main(['--scripts', '--only', 'scripts/'], { root, work: tmp('cov-work-'), out, err: sink(), run: fakeRun(scriptsAnswer(root), calls) }), 0);
  const text = out.text();
  assert.match(text, /^scripts\/p\.py:8\nscripts\/p\.py:9\n/);
  assert.match(text, /scripts\/q\.py:1\nscripts\/q\.py:2\nscripts\/q\.py:4\nscripts\/q\.py:5\n/);
  assert.match(text, /scripts\/x\.js:2\n/);
  assert.match(text, /scripts\/y\.js:2\nscripts\/y\.js:4\n/);
  assert.match(text, /scripts: 13 lines, 9 uncovered, 30\.77%\n$/);
  const node = calls.find((c) => c[0] === 'node');
  assert.ok(node.includes(path.join(root, 'scripts', 'x.test.js')));
  assert.ok(calls.some((c) => c[0] === 'python3' && c.includes('--report')));
});

test('scripts tier: no Python tests folder, a run that wrote no counts, and no Python at all', () => {
  const emptyTests = scriptsRepo({ ...SCRIPT_FILES, 'scripts/tests/__init__.py': '' });
  const calls0 = [];
  assert.strictEqual(cov.main(['--scripts'], { root: emptyTests, work: tmp('cov-work-'), out: sink(), err: sink(), run: fakeRun(scriptsAnswer(emptyTests), calls0) }), 0);
  assert.ok(!calls0.some((c) => c[0] === 'python3' && c.includes('--count')));
  const noTests = scriptsRepo(SCRIPT_FILES);
  const calls = [];
  assert.strictEqual(cov.main(['--scripts'], { root: noTests, work: tmp('cov-work-'), out: sink(), err: sink(), run: fakeRun(scriptsAnswer(noTests), calls) }), 0);
  assert.ok(!calls.some((c) => c[0] === 'python3' && c.includes('--count')));
  const noCounts = scriptsRepo({ ...SCRIPT_FILES, 'scripts/tests/__init__.py': '', 'scripts/tests/test_p.py': '' });
  const calls2 = [];
  const out = sink();
  assert.strictEqual(cov.main(['--scripts'], { root: noCounts, work: tmp('cov-work-'), out, err: sink(), run: fakeRun(scriptsAnswer(noCounts, { counts: false }), calls2) }), 0);
  assert.ok(!calls2.some((c) => c.includes('--report')));
  assert.match(out.text(), /scripts\/p\.py:1\n/);
  const jsOnly = scriptsRepo({ 'scripts/x.js': 'a();\n', 'scripts/x.test.js': '' });
  const calls3 = [];
  assert.strictEqual(cov.main(['--scripts'], { root: jsOnly, work: tmp('cov-work-'), out: sink(), err: sink(), run: fakeRun(scriptsAnswer(jsOnly), calls3) }), 0);
  assert.ok(!calls3.some((c) => c[0] === 'python3'));
});

test('--write-ratchet and --check: write then pass, worse fails, the empty tier is 100%', () => {
  const root = scriptsRepo({ 'scripts/x.js': 'a();\n', 'scripts/x.test.js': '' });
  fs.mkdirSync(path.join(root, 'scripts'), { recursive: true });
  const run = fakeRun(scriptsAnswer(root));
  const deps = () => ({ root, work: tmp('cov-work-'), out: sink(), err: sink(), run });
  assert.strictEqual(cov.main(['--scripts', '--check'], deps()), 1);
  assert.strictEqual(cov.main(['--scripts', '--write-ratchet', '--check'], deps()), 0);
  assert.strictEqual(fs.readFileSync(path.join(root, 'scripts/coverage-ratchet.tsv'), 'utf8'), '1\tscripts/x.js\n');
  fs.writeFileSync(path.join(root, 'scripts/coverage-ratchet.tsv'), '\n1\tscripts/x.js\n');
  assert.deepStrictEqual(cov.readRatchet(root), new Map([['scripts/x.js', 1]]));
  assert.strictEqual(cov.main(['--scripts', '--check'], deps()), 0);
  const out = sink();
  assert.strictEqual(cov.main(['--scripts', '--only', 'nothing/'], { ...deps(), out }), 0);
  assert.strictEqual(out.text(), 'scripts: 0 lines, 0 uncovered, 100.00%\n');
});

test('argument errors exit 2; an unexpected failure is not swallowed; the command line runs', () => {
  const err = sink();
  assert.strictEqual(cov.main(['--bogus'], { err }), 2);
  assert.match(err.text(), /unknown argument --bogus\nusage:/);
  assert.throws(() => cov.main(['--backend'], { root: tmp('cov-root-'), work: tmp('cov-work-'), out: sink(), err: sink(), run: () => { throw new Error('boom'); } }), /boom/);
  const r = childProcess.spawnSync('node', [TOOL, '--bogus'], { encoding: 'utf8' });
  assert.strictEqual(r.status, 2);
});

test('the Python executable-line helper sees nested functions', () => {
  const dir = tmp('cov-py-');
  const file = path.join(dir, 'm.py');
  fs.writeFileSync(file, 'x = 1\n\ndef f():\n    def g():\n        return 2\n    return g\n');
  const r = cov.defaultRun('python3', ['-c', cov.PY_EXECUTABLE, file]);
  assert.deepStrictEqual(JSON.parse(r.stdout)[file].filter((n) => n > 0), [1, 3, 4, 5, 6]);
});
