// Count code lines (non-blank, not comment-only) per file.
// usage: node loc.js file...   -> prints "<code>\t<path>"
const fs = require('fs');
const path = require('path');

function countCStyle(src, { nested = false, swift = false, js = false }) {
  let code = 0, i = 0, depth = 0, lineHasCode = false;
  const n = src.length;
  let inStr = null; // '"', "'", '`', '"""'
  let rawHashes = 0;
  while (i < n) {
    const c = src[i];
    if (c === '\n') { if (lineHasCode) code++; lineHasCode = false; i++; continue; }
    if (depth > 0) {
      if (nested && c === '/' && src[i + 1] === '*') { depth++; i += 2; continue; }
      if (c === '*' && src[i + 1] === '/') { depth--; i += 2; continue; }
      i++; continue;
    }
    if (inStr) {
      lineHasCode = lineHasCode || !/\s/.test(c);
      if (inStr === '"""') {
        if (src.startsWith('"""' + '#'.repeat(rawHashes), i)) { i += 3 + rawHashes; inStr = null; continue; }
        if (c === '\\' && rawHashes === 0) { i += 2; continue; }
        i++; continue;
      }
      if (c === '\\' && rawHashes === 0) { i += 2; continue; }
      if (c === inStr && src.startsWith('#'.repeat(rawHashes), i + 1)) { i += 1 + rawHashes; inStr = null; continue; }
      i++; continue;
    }
    if (c === '/' && src[i + 1] === '/') { while (i < n && src[i] !== '\n') i++; continue; }
    if (c === '/' && src[i + 1] === '*') { depth = 1; i += 2; continue; }
    if (swift && c === '#') {
      let j = i; while (src[j] === '#') j++;
      if (src[j] === '"') {
        rawHashes = j - i; lineHasCode = true;
        if (src.startsWith('"""', j)) { inStr = '"""'; i = j + 3; } else { inStr = '"'; i = j + 1; }
        continue;
      }
    }
    if (c === '"') {
      lineHasCode = true; rawHashes = 0;
      if (swift && src.startsWith('"""', i)) { inStr = '"""'; i += 3; } else { inStr = '"'; i++; }
      continue;
    }
    if (!swift && c === "'") { lineHasCode = true; rawHashes = 0; inStr = "'"; i++; continue; }
    if (js && c === '`') { lineHasCode = true; rawHashes = 0; inStr = '`'; i++; continue; }
    if (!/\s/.test(c)) lineHasCode = true;
    i++;
  }
  if (lineHasCode) code++;
  return code;
}

function countLineComment(src, marker) {
  return src.split('\n').filter(l => { const t = l.trim(); return t && !t.startsWith(marker); }).length;
}

function count(file) {
  const src = fs.readFileSync(file, 'utf8');
  const ext = path.extname(file).toLowerCase();
  switch (ext) {
    case '.swift': return countCStyle(src, { nested: true, swift: true });
    case '.c': case '.h': case '.css': return countCStyle(src, {});
    case '.js': case '.mjs': case '.cjs': return countCStyle(src, { js: true });
    case '.py': case '.sh': return countLineComment(src, '#');
    case '.asm': case '.s': return countLineComment(src, ';');
    default: return null;
  }
}

module.exports = { countCStyle, count };
if (require.main === module) for (const f of process.argv.slice(2)) {
  const c = count(f);
  if (c !== null) console.log(`${c}\t${f}`);
}
