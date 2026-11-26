// Loaded with `node --require` before scripts/gen-headers.js
// (gen-headers.test.js): the script's header.json and footprints.json, which
// it finds at schematical/schematical/Parts/ beside scripts/, are read and
// written in the folder GEN_HEADERS_PARTS instead, so a test never touches
// the checked-in files.

'use strict';

const fs = require('fs');
const path = require('path');

const REAL = path.join(__dirname, '..', '..', '..', 'schematical', 'schematical', 'Parts');
const INSTEAD = process.env.GEN_HEADERS_PARTS;
if (!INSTEAD) throw new Error('redirect-parts.js: GEN_HEADERS_PARTS is not set');

const moved = (file) => (typeof file === 'string' && path.dirname(file) === REAL ? path.join(INSTEAD, path.basename(file)) : file);

for (const name of ['readFileSync', 'writeFileSync', 'existsSync']) {
  const original = fs[name];
  fs[name] = (file, ...rest) => original.call(fs, moved(file), ...rest);
}
