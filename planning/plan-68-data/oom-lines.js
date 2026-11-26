// plan-68-K: the uncovered lines that are out-of-memory paths. Reads the
// `path:line` list `node scripts/coverage.js --backend` prints, and reports a
// line that says "out of memory", or that is the body of a NULL check on a
// variable assigned from logic_malloc/calloc/realloc/strdup just before it.
//   node scripts/coverage.js --backend | node planning/plan-68-data/oom-lines.js
'use strict';
const fs = require('fs');
const path = require('path');
const root = path.resolve(__dirname, '..', '..');
const input = fs.readFileSync(0, 'utf8').split('\n').filter((l) => /^backend\/src\/[^:]+:\d+$/.test(l));
const cache = new Map();
const lines = (file) => {
  if (!cache.has(file)) cache.set(file, fs.readFileSync(path.join(root, file), 'utf8').split('\n'));
  return cache.get(file);
};
let found = 0;
for (const entry of input) {
  const [file, n] = entry.split(':');
  const src = lines(file);
  const k = Number(n) - 1;
  const text = src[k];
  let oom = /out of memory/.test(text);
  if (!oom) {
    // the nearest enclosing "if (... == NULL ...)" above, within the same block
    for (let j = k - 1, depth = 0; j >= 0 && j >= k - 12; j--) {
      const t = src[j];
      if (/^[A-Za-z].*\(.*\)? *\{\s*$/.test(t) || /^}/.test(t)) break; // left the function
      depth += (t.match(/}/g) || []).length - (t.match(/{/g) || []).length;
      const m = /if \((.*)\) \{\s*$/.exec(t);
      if (m && depth < 0) {
        const vars = [...m[1].matchAll(/([A-Za-z_][\w.>-]*)\s*==\s*NULL/g)].map((x) => x[1]);
        let start = j;
        while (start > 0 && !/^[A-Za-z]/.test(src[start - 1]) && j - start < 40) start--;
        const before = src.slice(start, j).join('\n');
        oom = vars.some((v) => new RegExp(v.replace(/[.>-]/g, '\\$&') + '\\s*=\\s*logic_(malloc|calloc|realloc|strdup)\\(').test(before));
        break;
      }
    }
  }
  if (oom) {
    found++;
    console.log(`${entry}\t${text.trim()}`);
  }
}
console.error(`${found} out-of-memory lines uncovered (of ${input.length} uncovered)`);
process.exitCode = found === 0 ? 0 : 1;
