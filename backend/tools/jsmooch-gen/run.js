// Regenerates the vendored 65816 core's opcode table from jsmooch's code
// generator (plan-14-G; README in backend/src/third_party/jsmooch/).
// Usage: node run.js [out.c]   (default: the vendored wdc65816_generated_opcodes.c)
const fs = require('fs'), vm = require('vm'), path = require('path');
const dir = __dirname;
const ctx = { console }; ctx.window = ctx; ctx.document = {};
vm.createContext(ctx);
for (const f of ['generation.js', 'wdc65816_opcodes.js', 'wdc65816_core_generator.js'])
  vm.runInContext(fs.readFileSync(path.join(dir, f), 'utf8'), ctx, { filename: f });
vm.runInContext("set_gentarget('c'); __out = decode_opcodes_c();", ctx);
fs.writeFileSync(process.argv[2] ||
  path.join(dir, '..', '..', 'src', 'third_party', 'jsmooch', 'wdc65816_generated_opcodes.c'), ctx.__out);
