#!/usr/bin/env node
// Line coverage of the app, the backend and the scripts (AGENTS.md § Source
// size and coverage; docs/testing.md § Coverage).
//
//   node scripts/coverage.js [--backend] [--app] [--scripts]   none = all three
//     --only <prefix>      print, check and write only files under the prefix
//     --tests <A,B,…>      app tier: run only these test classes
//     --check              fail when a file has more uncovered lines than its ratchet row
//     --write-ratchet      rewrite the ratchet rows of what was measured
'use strict';
const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');
const sizeRule = require('./source-size.js');

const ROOT = path.resolve(__dirname, '..');
const WORK = '/tmp/logic-cov';
const RATCHET = 'scripts/coverage-ratchet.tsv';
const TIERS = ['backend', 'app', 'scripts'];
const APP_TIMEOUT_S = 3600;
const SCRIPT_JS = ['schematical/schematical/Datasheet/Renderer/datasheet.js',
  'schematical/schematical/Datasheet/Renderer/datasheet-edit.js',
  'schematical/schematical/Datasheet/Renderer/datasheet-edit-model.js',
  'backend/tools/avr8js-oracle/oracle.js', 'backend/tools/jsmooch-gen/run.js'];

// Which tier measures a repo-relative path, or null when none does.
function tierOf(file) {
  if (/^backend\/src\/.*\.[ch]$/.test(file) && !file.startsWith('backend/src/third_party/')) return 'backend';
  if (file === 'basicsim/main.c') return 'backend';
  if (/^schematical\/schematical\/.*\.swift$/.test(file)) return 'app';
  if (/^scripts\/[^/]+\.(js|py)$/.test(file) && !file.endsWith('.test.js')) return 'scripts';
  if (SCRIPT_JS.includes(file)) return 'scripts';
  return null;
}

function relative(root, file) {
  const abs = path.resolve(root, file);
  return abs.startsWith(root + path.sep) ? abs.slice(root.length + 1) : null;
}

// lcov text → Map(file → Map(line → count)), keeping files `keep` accepts.
function parseLcov(text, root, keep) {
  const out = new Map();
  let lines = null;
  for (const raw of text.split('\n')) {
    const line = raw.trim();
    if (line.startsWith('SF:')) {
      const file = relative(root, line.slice(3));
      lines = file !== null && keep(file) ? new Map() : null;
      if (lines) out.set(file, lines);
    } else if (line.startsWith('DA:') && lines) {
      const [number, count] = line.slice(3).split(',');
      const n = Number(number);
      lines.set(n, Math.max(lines.get(n) || 0, Number(count)));
    } else if (line === 'end_of_record') {
      lines = null;
    }
  }
  return out;
}

// `xccov view --archive --json` → Map(file → Map(line → count)).
function parseXccovArchive(json, root, keep) {
  const out = new Map();
  for (const [abs, entries] of Object.entries(JSON.parse(json))) {
    const file = relative(root, abs);
    if (file === null || !keep(file)) continue;
    const lines = new Map();
    for (const entry of entries) {
      if (entry.isExecutable) lines.set(entry.line, entry.executionCount || 0);
    }
    out.set(file, lines);
  }
  return out;
}

// A `python3 -m trace --missing` .cover file → Map(line → count): a counted
// line starts "<n>:", a missed executable line starts ">>>>>>".
function parseTraceCover(text) {
  const lines = new Map();
  text.split('\n').forEach((line, index) => {
    const counted = /^\s*(\d+):/.exec(line);
    if (counted) lines.set(index + 1, Number(counted[1]));
    else if (line.startsWith('>>>>>>')) lines.set(index + 1, 0);
  });
  return lines;
}

function uncovered(lines) {
  return [...lines].filter(([, count]) => count === 0).map(([n]) => n).sort((a, b) => a - b);
}

function defaultRun(cmd, args, options = {}) {
  const r = childProcess.spawnSync(cmd, args, { encoding: 'utf8', maxBuffer: 1 << 30, ...options });
  return { status: r.status === null ? -1 : r.status, stdout: r.stdout || '', stderr: r.stderr || '' };
}

class TierError extends Error {}

function must(result, what) {
  if (result.status !== 0) throw new TierError(`${what} failed (exit ${result.status}): ${result.stderr.trim().split('\n').slice(-3).join(' | ')}`);
  return result;
}

function backendTier(ctx) {
  const { run, root, log, work } = ctx;
  const dir = path.join(work, 'backend');
  run('rm', ['-rf', dir]);
  must(run('mkdir', ['-p', path.join(dir, 'prof')]), 'mkdir');
  must(run('cmake', ['-S', path.join(root, 'basicsim'), '-B', dir, '-DCMAKE_BUILD_TYPE=Debug',
    '-DCMAKE_C_FLAGS=-fprofile-instr-generate -fcoverage-mapping',
    '-DCMAKE_EXE_LINKER_FLAGS=-fprofile-instr-generate']), 'cmake configure');
  must(run('cmake', ['--build', dir, '-j8']), 'cmake build');
  const tests = run('ctest', ['--test-dir', dir, '-j8'],
    { env: { ...process.env, LLVM_PROFILE_FILE: path.join(dir, 'prof', '%p.profraw') } });
  if (tests.status !== 0) log(`backend: ctest reported failures:\n${tests.stdout.split('\n').filter((l) => /\*\*\*Failed|Not Run/.test(l)).join('\n')}`);
  const listed = JSON.parse(must(run('ctest', ['--test-dir', dir, '--show-only=json-v1']), 'ctest --show-only').stdout);
  const objects = [...new Set(listed.tests.map((t) => t.command[0]).filter((c) => c.startsWith(dir)))].sort();
  const raws = must(run('find', [path.join(dir, 'prof'), '-name', '*.profraw']), 'find').stdout.split('\n').filter(Boolean);
  if (raws.length === 0) throw new TierError('backend: no profile was written');
  const merged = path.join(dir, 'all.profdata');
  must(run('xcrun', ['llvm-profdata', 'merge', '-sparse', ...raws, '-o', merged]), 'llvm-profdata merge');
  const lcov = must(run('xcrun', ['llvm-cov', 'export', '-format=lcov', '-instr-profile', merged,
    ...objects.flatMap((o) => ['-object', o])]), 'llvm-cov export').stdout;
  return parseLcov(lcov, root, (f) => tierOf(f) === 'backend');
}

function appTier(ctx) {
  const { run, root, tests, log, work } = ctx;
  const dir = path.join(work, 'app');
  const bundle = path.join(dir, 'result.xcresult');
  run('rm', ['-rf', bundle]);
  must(run('mkdir', ['-p', dir]), 'mkdir');
  const only = tests.map((t) => `-only-testing:schematicalTests/${t}`);
  const result = run('perl', ['-e', `alarm ${APP_TIMEOUT_S}; exec @ARGV`, 'xcodebuild', 'test',
    '-project', path.join(root, 'schematical/schematical.xcodeproj'), '-scheme', 'schematical',
    '-destination', 'platform=macOS', '-derivedDataPath', path.join(dir, 'dd'),
    '-enableCodeCoverage', 'YES', 'CODE_SIGNING_ALLOWED=NO', '-resultBundlePath', bundle, ...only]);
  run('pkill', ['-f', path.join(dir, 'dd/Build/Products/Debug/schematical.app')]);
  if (result.status !== 0) {
    const failed = result.stdout.split('\n').filter((l) => /Test Case .* failed/.test(l));
    log(`app: xcodebuild exited ${result.status}; ${failed.length} failed test cases:\n${failed.join('\n')}`);
  }
  const archive = must(run('xcrun', ['xccov', 'view', '--archive', '--json', bundle]), 'xccov view --archive').stdout;
  const files = parseXccovArchive(archive, root, (f) => tierOf(f) === 'app');
  if (files.size === 0) throw new TierError('app: the result bundle has no Swift coverage (a signed, sandboxed test host cannot write it)');
  return files;
}

// Executable lines of Python files no test ran, by the same rule `trace` uses.
const PY_EXECUTABLE = [
  'import json, sys',
  'def lines(code, out):',
  '    for _, _, n in code.co_lines():',
  '        if n: out.add(n)',
  '    for c in code.co_consts:',
  '        if hasattr(c, "co_lines"): lines(c, out)',
  'res = {}',
  'for f in sys.argv[1:]:',
  '    s = set(); lines(compile(open(f).read(), f, "exec"), s); res[f] = sorted(s)',
  'print(json.dumps(res))',
].join('\n');

function scriptsTier(ctx) {
  const { run, root, work } = ctx;
  const dir = path.join(work, 'scripts');
  run('rm', ['-rf', dir]);
  must(run('mkdir', ['-p', dir]), 'mkdir');
  const lcovFile = path.join(dir, 'node.lcov');
  const testFiles = fs.readdirSync(path.join(root, 'scripts')).filter((f) => f.endsWith('.test.js')).sort()
    .map((f) => path.join(root, 'scripts', f));
  must(run('node', ['--test', '--experimental-test-coverage', '--test-reporter=lcov',
    `--test-reporter-destination=${lcovFile}`, ...testFiles], { cwd: root }), 'node --test');
  const files = parseLcov(fs.readFileSync(lcovFile, 'utf8'), root, (f) => tierOf(f) === 'scripts');
  const inTier = sizeRule.census(root).map((e) => e.file).filter((f) => tierOf(f) === 'scripts');
  for (const file of inTier.filter((f) => f.endsWith('.js') && !files.has(f))) {
    // Never loaded by a test: every code line is uncovered.
    const text = fs.readFileSync(path.join(root, file), 'utf8').split('\n');
    const lines = new Map();
    text.forEach((line, i) => { if (sizeRule.countCode(line, '.js') > 0) lines.set(i + 1, 0); });
    files.set(file, lines);
  }
  const python = inTier.filter((f) => f.endsWith('.py'));
  if (python.length > 0) {
    const counts = path.join(dir, 'python.counts');
    const coverDir = path.join(dir, 'cover');
    const testsDir = path.join(root, 'scripts/tests');
    // unittest exits non-zero when it finds no test, so run it only when one exists.
    if (fs.existsSync(testsDir) && fs.readdirSync(testsDir).some((f) => /^test.*\.py$/.test(f))) {
      must(run('python3', ['-m', 'trace', '--count', `--file=${counts}`, '--no-report', '--module', 'unittest',
        'discover', '-s', path.join(root, 'scripts/tests')],
      { cwd: root, env: { ...process.env, SCRIPTS_COVERAGE_FILE: counts } }), 'python unittest under trace');
      if (fs.existsSync(counts)) {
        must(run('python3', ['-m', 'trace', '--report', `--file=${counts}`, '--missing', `--coverdir=${coverDir}`], { cwd: root }), 'trace --report');
      }
    }
    const exec = JSON.parse(must(run('python3', ['-c', PY_EXECUTABLE, ...python.map((f) => path.join(root, f))]), 'python executable lines').stdout);
    // trace names a .cover file by the module's dotted name; match its last part.
    const covers = fs.existsSync(coverDir) ? fs.readdirSync(coverDir) : [];
    for (const file of python) {
      const name = `${path.basename(file, '.py')}.cover`;
      const cover = covers.find((c) => c === name || c.endsWith(`.${name}`));
      if (cover !== undefined) {
        files.set(file, parseTraceCover(fs.readFileSync(path.join(coverDir, cover), 'utf8')));
      } else {
        files.set(file, new Map(exec[path.join(root, file)].map((n) => [n, 0])));
      }
    }
  }
  return files;
}

function readRatchet(root) {
  const rows = new Map();
  const file = path.join(root, RATCHET);
  if (!fs.existsSync(file)) return rows;
  for (const line of fs.readFileSync(file, 'utf8').split('\n')) {
    if (line.trim() === '') continue;
    const [count, name] = line.split('\t');
    rows.set(name, Number(count));
  }
  return rows;
}

const under = (prefix) => (file) => prefix === null || file.startsWith(prefix);

// Failures of the measured files against the ratchet: `tiers` were measured,
// `prefix` limits what is judged.
function check(files, ratchet, tiers, prefix) {
  const judged = (f) => tiers.includes(tierOf(f)) && under(prefix)(f);
  const failures = [];
  for (const [file, lines] of [...files].sort(([a], [b]) => a.localeCompare(b))) {
    if (!judged(file)) continue;
    const n = uncovered(lines).length;
    const row = ratchet.has(file) ? ratchet.get(file) : 0;
    if (n > row) failures.push(`worse: ${file} ${n} > ${row}`);
    else if (n < row) failures.push(`stale: ${file} ${n} (row ${row})`);
  }
  for (const [file, row] of ratchet) {
    if (judged(file) && !files.has(file)) failures.push(`stale: ${file} gone (row ${row})`);
  }
  return failures;
}

// The ratchet with the judged rows replaced by this run's numbers.
function rewriteRatchet(files, ratchet, tiers, prefix) {
  const judged = (f) => tiers.includes(tierOf(f)) && under(prefix)(f);
  const rows = new Map([...ratchet].filter(([f]) => !judged(f)));
  for (const [file, lines] of files) {
    const n = uncovered(lines).length;
    if (judged(file) && n > 0) rows.set(file, n);
  }
  return [...rows].sort(([a], [b]) => a.localeCompare(b)).map(([f, n]) => `${n}\t${f}\n`).join('');
}

function parseArgs(argv) {
  const opts = { tiers: [], only: null, tests: [], check: false, write: false };
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === '--backend' || arg === '--app' || arg === '--scripts') opts.tiers.push(arg.slice(2));
    else if (arg === '--check') opts.check = true;
    else if (arg === '--write-ratchet') opts.write = true;
    else if (arg === '--only' && i + 1 < argv.length) opts.only = argv[++i];
    else if (arg === '--tests' && i + 1 < argv.length) opts.tests = argv[++i].split(',').filter(Boolean);
    else throw new TierError(`unknown argument ${arg}`);
  }
  if (opts.tiers.length === 0) opts.tiers = [...TIERS];
  return opts;
}

const RUNNERS = { backend: backendTier, app: appTier, scripts: scriptsTier };

function main(argv, deps = {}) {
  const out = deps.out || process.stdout;
  const err = deps.err || process.stderr;
  const root = deps.root || ROOT;
  const run = deps.run || defaultRun;
  const work = deps.work || WORK;
  const log = (s) => err.write(s + '\n');
  let opts;
  try {
    opts = parseArgs(argv);
  } catch (e) {
    err.write(`coverage.js: ${e.message}\nusage: coverage.js [--backend] [--app] [--scripts] [--only <prefix>] [--tests <A,B>] [--check] [--write-ratchet]\n`);
    return 2;
  }
  const files = new Map();
  for (const tier of opts.tiers) {
    try {
      for (const [file, lines] of RUNNERS[tier]({ run, root, tests: opts.tests, log, work })) files.set(file, lines);
    } catch (e) {
      if (!(e instanceof TierError)) throw e;
      log(`coverage.js: ${tier}: ${e.message}`);
      return 3;
    }
  }
  const shown = [...files].filter(([f]) => under(opts.only)(f)).sort(([a], [b]) => a.localeCompare(b));
  const totals = new Map(TIERS.map((t) => [t, { lines: 0, missed: 0 }]));
  for (const [file, lines] of shown) {
    for (const n of uncovered(lines)) out.write(`${file}:${n}\n`);
  }
  for (const [file, lines] of shown) {
    const missed = uncovered(lines).length;
    const total = totals.get(tierOf(file));
    total.lines += lines.size;
    total.missed += missed;
    if (missed > 0) out.write(`${missed}\t${lines.size}\t${file}\n`);
  }
  for (const tier of opts.tiers) {
    const { lines, missed } = totals.get(tier);
    const pct = lines === 0 ? 100 : (100 * (lines - missed)) / lines;
    out.write(`${tier}: ${lines} lines, ${missed} uncovered, ${pct.toFixed(2)}%\n`);
  }
  const ratchet = readRatchet(root);
  if (opts.write) fs.writeFileSync(path.join(root, RATCHET), rewriteRatchet(files, ratchet, opts.tiers, opts.only));
  if (opts.check) {
    const failures = check(files, opts.write ? readRatchet(root) : ratchet, opts.tiers, opts.only);
    for (const line of failures) out.write(line + '\n');
    return failures.length === 0 ? 0 : 1;
  }
  return 0;
}

module.exports = { tierOf, parseLcov, parseXccovArchive, parseTraceCover, uncovered, check,
  rewriteRatchet, readRatchet, parseArgs, main, TierError, defaultRun, PY_EXECUTABLE };

if (require.main === module) process.exitCode = main(process.argv.slice(2));
