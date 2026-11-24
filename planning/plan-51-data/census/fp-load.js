#!/usr/bin/env node
// plan-51-D: loads a side file of footprints through the real loader.
//   node fp-load.js <side.json>     (or fp-load.sh)
//
// Copies backend/ and schematical/schematical/Parts/ to /tmp/p51-fp/ (keeping their relative layout, which is how
// PARTS_DIR finds the parts; the cmake build there is kept between runs), appends the side file's footprints to the
// copy's footprints.json, builds `footprint_check` there and runs it. footprint_check compares every footprint with
// tests/golden/footprints.txt and fails on a footprint no part uses, so for the n new footprints it must print
// exactly n `extra footprint` lines and n `unused footprint` lines, for the same ids, and nothing else but its
// summary: no loader error (overlapping pads, a drill larger than its pad, a bad key), no `differs`, no `missing`.
// Prints `<n> new footprints: load ok` and exits 0, or says what is wrong and exits 1.

'use strict';

const fs = require('fs');
const path = require('path');
const childProcess = require('child_process');

const ROOT = path.resolve(__dirname, '..', '..', '..');
const COPY = '/tmp/p51-fp';

function sh(command, args, options) {
  return childProcess.spawnSync(command, args, { encoding: 'utf8', maxBuffer: 256 * 1024 * 1024, ...options });
}

function main() {
  const side = process.argv[2];
  if (!side) {
    console.error('usage: fp-load.js <side.json>');
    return 2;
  }
  const added = JSON.parse(fs.readFileSync(side, 'utf8')).footprints;
  if (!Array.isArray(added) || added.length === 0) {
    console.error(`${side}: no "footprints" array, or it is empty`);
    return 1;
  }
  const newIds = new Set(added.map((fp) => fp.id));
  if (newIds.size !== added.length) {
    console.error(`${side}: a footprint id is listed twice`);
    return 1;
  }

  fs.mkdirSync(path.join(COPY, 'schematical', 'schematical'), { recursive: true });
  for (const [from, to] of [[path.join(ROOT, 'backend'), path.join(COPY, 'backend')],
    [path.join(ROOT, 'schematical', 'schematical', 'Parts'), path.join(COPY, 'schematical', 'schematical', 'Parts')]]) {
    const r = sh('rsync', ['-a', '--delete', '--exclude', 'build', from + '/', to + '/']);
    if (r.status !== 0) throw new Error(`rsync ${from}: ${r.stderr}`);
  }
  const fpFile = path.join(COPY, 'schematical', 'schematical', 'Parts', 'footprints.json');
  const doc = JSON.parse(fs.readFileSync(fpFile, 'utf8'));
  const old = new Set(doc.footprints.map((fp) => fp.id));
  const clash = [...newIds].filter((id) => old.has(id));
  if (clash.length > 0) {
    console.error(`ids already in footprints.json: ${clash.slice(0, 5).join(', ')}${clash.length > 5 ? ', …' : ''}`);
    return 1;
  }
  doc.footprints.push(...added);
  fs.writeFileSync(fpFile, JSON.stringify(doc));

  const build = path.join(COPY, 'build');
  let r = sh('cmake', ['-S', path.join(COPY, 'backend'), '-B', build, '-DCMAKE_BUILD_TYPE=Release']);
  if (r.status === 0) r = sh('cmake', ['--build', build, '-j8', '--target', 'footprint_check']);
  if (r.status !== 0) {
    console.error(`build failed:\n${(r.stdout + r.stderr).split('\n').slice(-20).join('\n')}`);
    return 1;
  }
  r = sh(path.join(build, 'footprint_check'), []);
  const lines = (r.stdout + r.stderr).split('\n').filter((l) => l.trim() !== '');
  const extra = new Set();
  const unused = new Set();
  const other = [];
  let mismatches = 0;
  for (const line of lines) {
    let m;
    if ((m = line.match(/^extra footprint \(not in the golden\): ([^|]+)\|/))) extra.add(m[1]);
    else if ((m = line.match(/^unused footprint: (.+)$/))) unused.add(m[1]);
    else if ((m = line.match(/^FAIL: (\d+) mismatches$/))) mismatches = Number(m[1]);
    else if (/^\d+ footprints compared, \d+ parts loaded$/.test(line)) continue;
    else other.push(line);
  }
  // footprint_check prints at most 10 `extra footprint` lines (the golden comparison) but counts them all, and
  // prints every `unused footprint`: n extra + n unused = 2n mismatches, so anything else counted would show here.
  const n = newIds.size;
  const problems = [];
  if (other.length > 0) problems.push(`footprint_check printed:\n  ${other.slice(0, 12).join('\n  ')}`);
  if (![...extra].every((id) => newIds.has(id)) || extra.size !== Math.min(10, n)) {
    problems.push(`"extra footprint" lines name ${extra.size} ids (${[...extra].filter((id) => !newIds.has(id)).length} not new), expected ${Math.min(10, n)} new ones`);
  }
  if (unused.size !== n || ![...unused].every((id) => newIds.has(id))) {
    problems.push(`"unused footprint" lines name ${unused.size} ids, the side file has ${n}`);
  }
  if (mismatches !== 2 * n) problems.push(`footprint_check counted ${mismatches} mismatches, expected ${2 * n} (${n} extra + ${n} unused)`);
  if (problems.length === 0) problems.push(...overlaps(path.join(build, 'footprint_check'), newIds));
  if (problems.length > 0) {
    console.error(problems.join('\n'));
    return 1;
  }
  console.log(`${newIds.size} new footprints: load ok`);
  return 0;
}

// The loader checks a drill against its pad but NOT pad overlap (docs/parts-format.md § `explicit` only advises
// it), so this does: every pad of each new footprint, as `footprint_check --print` expanded it
// (`<designator>:<x>,<y>,<w>x<h>,<round|rect>[,d<drill>…]`), against every other pad of the same footprint.
// Round pads are circles of the shorter side when w = h, else boxes; pads that only touch do not overlap.
function overlaps(checker, newIds) {
  const r = sh(checker, ['--print']);
  const problems = [];
  for (const line of r.stdout.split('\n')) {
    const id = line.split('|')[0];
    if (!newIds.has(id)) continue;
    const pads = line.split('|pads=')[1].split(';').map((p) => {
      const m = p.match(/^([^:]*):(-?[\d.]+),(-?[\d.]+),([\d.]+)x([\d.]+),(round|rect)/);
      if (!m) throw new Error(`cannot read pad "${p}" of ${id}`);
      return { name: m[1], x: Number(m[2]), y: Number(m[3]), w: Number(m[4]), h: Number(m[5]), round: m[6] === 'round' && m[4] === m[5] };
    });
    const bad = [];
    for (let i = 0; i < pads.length; i++) {
      for (let j = i + 1; j < pads.length; j++) {
        const a = pads[i];
        const b = pads[j];
        const eps = 0.0005;
        const hit = a.round && b.round
          ? Math.hypot(a.x - b.x, a.y - b.y) < (a.w + b.w) / 2 - eps
          : Math.abs(a.x - b.x) < (a.w + b.w) / 2 - eps && Math.abs(a.y - b.y) < (a.h + b.h) / 2 - eps;
        if (hit) bad.push(`${a.name}/${b.name}`);
      }
    }
    if (bad.length > 0) problems.push(`${id}: overlapping pads ${bad.slice(0, 4).join(', ')}${bad.length > 4 ? ', …' : ''}`);
  }
  return problems;
}

process.exit(main());
