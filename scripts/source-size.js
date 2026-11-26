#!/usr/bin/env node
// The source size rule (AGENTS.md § Source size and coverage; docs/testing.md):
// no in-scope file over LIMIT lines of code. A line of code is a line with
// something other than whitespace and comments on it.
//
//   node scripts/source-size.js            files over the limit, most first
//   node scripts/source-size.js --all      every in-scope file
//   node scripts/source-size.js --check    fail on a violation (ctest: source_size_check)
//   --root <dir>                           the checkout to look at (default: this one)
'use strict';
const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

const LIMIT = 1000;
const RATCHET = 'scripts/source-size-ratchet.tsv';
const EXTENSIONS = new Set(['.swift', '.c', '.h', '.js', '.py', '.sh']);
const EXCLUDED_DIRS = ['datasheets/', 'planning/', 'examples/', 'routing-example/',
  'backend/src/third_party/', 'backend/tools/avr8js/'];
const VENDORED_FILES = new Set([
  'backend/tools/jsmooch-gen/generation.js',
  'backend/tools/jsmooch-gen/wdc65816_opcodes.js',
  'backend/tools/jsmooch-gen/wdc65816_core_generator.js',
]);

function isSpace(c) {
  return c === ' ' || c === '\t' || c === '\r' || c === '\f' || c === '\v';
}

// C-family text: Swift (nested block comments, """ and #"…"# strings), C and
// headers, JavaScript (template literals).
function countCStyle(src, lang) {
  const swift = lang === 'swift', js = lang === 'js';
  let code = 0, i = 0, depth = 0, hasCode = false, quote = null, hashes = 0;
  const n = src.length;
  while (i < n) {
    const c = src[i];
    if (c === '\n') {
      if (hasCode) code++;
      hasCode = false;
      i++;
      continue;
    }
    if (depth > 0) {
      if (swift && c === '/' && src[i + 1] === '*') { depth++; i += 2; continue; }
      if (c === '*' && src[i + 1] === '/') { depth--; i += 2; continue; }
      i++;
      continue;
    }
    if (quote !== null) {
      if (!isSpace(c)) hasCode = true;
      const close = quote === '"""' ? '"""' : quote;
      if (c === '\\' && hashes === 0) { i += 2; continue; }
      if (src.startsWith(close + '#'.repeat(hashes), i)) { i += close.length + hashes; quote = null; continue; }
      i++;
      continue;
    }
    if (c === '/' && src[i + 1] === '/') {
      while (i < n && src[i] !== '\n') i++;
      continue;
    }
    if (c === '/' && src[i + 1] === '*') { depth = 1; i += 2; continue; }
    if (swift && c === '#') {
      let j = i;
      while (src[j] === '#') j++;
      if (src[j] === '"') {
        hashes = j - i;
        hasCode = true;
        if (src.startsWith('"""', j)) { quote = '"""'; i = j + 3; } else { quote = '"'; i = j + 1; }
        continue;
      }
    }
    if (c === '"') {
      hasCode = true;
      hashes = 0;
      if (swift && src.startsWith('"""', i)) { quote = '"""'; i += 3; } else { quote = '"'; i++; }
      continue;
    }
    if (!swift && c === "'") { hasCode = true; hashes = 0; quote = "'"; i++; continue; }
    if (js && c === '`') { hasCode = true; hashes = 0; quote = '`'; i++; continue; }
    if (!isSpace(c)) hasCode = true;
    i++;
  }
  if (hasCode) code++;
  return code;
}

function countHashComments(src) {
  return src.split('\n').filter((line) => {
    const t = line.trim();
    return t !== '' && !t.startsWith('#');
  }).length;
}

// Code lines of `text`, a file with extension `ext` (".swift" …).
function countCode(text, ext) {
  switch (ext) {
    case '.swift': return countCStyle(text, 'swift');
    case '.c': case '.h': return countCStyle(text, 'c');
    case '.js': return countCStyle(text, 'js');
    case '.py': case '.sh': return countHashComments(text);
    default: throw new Error(`no counter for ${ext}`);
  }
}

// Whether a repo-relative path is under the size rule.
function inScope(file) {
  if (!EXTENSIONS.has(path.extname(file))) return false;
  if (EXCLUDED_DIRS.some((dir) => file.startsWith(dir))) return false;
  if (file.endsWith('.min.js') || VENDORED_FILES.has(file)) return false;
  return true;
}

function trackedFiles(root) {
  const out = childProcess.execFileSync('git', ['-C', root, 'ls-files', '-z'], { encoding: 'utf8' });
  return out.split('\0').filter((f) => f !== '' && inScope(f));
}

// [{file, code}] for every in-scope file, most code first.
function census(root) {
  return trackedFiles(root)
    .filter((file) => fs.existsSync(path.join(root, file)))
    .map((file) => ({ file, code: countCode(fs.readFileSync(path.join(root, file), 'utf8'), path.extname(file)) }))
    .sort((a, b) => b.code - a.code || a.file.localeCompare(b.file));
}

function readRatchet(root) {
  const file = path.join(root, RATCHET);
  const rows = new Map();
  if (!fs.existsSync(file)) return rows;
  for (const line of fs.readFileSync(file, 'utf8').split('\n')) {
    if (line.trim() === '') continue;
    const [count, name] = line.split('\t');
    rows.set(name, Number(count));
  }
  return rows;
}

// The failures of the rule against the ratchet, as printable lines.
function check(files, ratchet) {
  const failures = [];
  const seen = new Set();
  for (const { file, code } of files) {
    const row = ratchet.get(file);
    if (row === undefined) {
      if (code > LIMIT) failures.push(`over: ${file} ${code}`);
      continue;
    }
    seen.add(file);
    if (code > row) failures.push(`grew: ${file} ${code} > ${row}`);
    else if (code < row || code <= LIMIT) failures.push(`stale: ${file} ${code} (row ${row})`);
  }
  for (const [file, row] of ratchet) {
    if (!seen.has(file)) failures.push(`stale: ${file} gone (row ${row})`);
  }
  return failures;
}

function main(argv, out = process.stdout, err = process.stderr) {
  let root = path.resolve(__dirname, '..');
  let mode = 'over';
  for (let i = 0; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === '--all') mode = 'all';
    else if (arg === '--check') mode = 'check';
    else if (arg === '--root' && i + 1 < argv.length) root = path.resolve(argv[++i]);
    else {
      err.write(`source-size.js: unknown argument ${arg}\nusage: source-size.js [--all|--check] [--root <dir>]\n`);
      return 2;
    }
  }
  const files = census(root);
  if (mode === 'check') {
    const failures = check(files, readRatchet(root));
    for (const line of failures) out.write(line + '\n');
    return failures.length === 0 ? 0 : 1;
  }
  for (const { file, code } of files) {
    if (mode === 'all' || code > LIMIT) out.write(`${code}\t${file}\n`);
  }
  return 0;
}

module.exports = { countCode, inScope, census, check, readRatchet, main, LIMIT };

if (require.main === module) process.exitCode = main(process.argv.slice(2));
