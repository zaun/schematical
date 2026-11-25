// The avr8js oracle (plan-62): runs the vendored avr8js (../avr8js, MIT) and
// prints what it does, in the text formats the C tests print for the C port
// (backend/src/models/avr_*.c), so the two can be compared line by line.
//
//   node oracle.js step <seed> <states>
//       Every opcode word 0..65535, <states> random states each, one
//       instruction per state. One line per vector.

'use strict';

const path = require('path');

const avr8js = require(path.join(__dirname, '..', 'avr8js', 'dist', 'cjs', 'index.js'));

function hex(value, digits) {
  return (value >>> 0).toString(16).padStart(digits, '0');
}

// FNV-1a, 32 bits, over bytes.
function fnv32(bytes) {
  let h = 0x811c9dc5;
  for (let i = 0; i < bytes.length; i++) {
    h ^= bytes[i];
    h = Math.imul(h, 0x01000193) >>> 0;
  }
  return h;
}

// xorshift32: the C side draws the same numbers from the same seed.
function makeRandom(seed) {
  let s = seed >>> 0;
  return function draw() {
    s ^= s << 13;
    s >>>= 0;
    s ^= s >>> 17;
    s ^= s << 5;
    s >>>= 0;
    return s;
  };
}

// ---- step ---------------------------------------------------------------

const STEP_FLASH_WORDS = 0x4000; // ATmega328P: 32 KB
const STEP_SRAM_BYTES = 2048;

function step(seed, states) {
  const draw = makeRandom(seed);
  const flash = new Uint16Array(STEP_FLASH_WORDS);
  const cpu = new avr8js.CPU(flash, STEP_SRAM_BYTES);
  const data = cpu.data;
  for (let opcode = 0; opcode < 0x10000; opcode++) {
    const lines = [];
    for (let k = 0; k < states; k++) {
      for (let i = 0; i < data.length; i += 4) {
        const r = draw();
        data[i] = r;
        data[i + 1] = r >>> 8;
        data[i + 2] = r >>> 16;
        data[i + 3] = r >>> 24;
      }
      const pc = 0x1000 + (draw() % 0x2000);
      cpu.pc = pc;
      flash[pc] = opcode;
      flash[pc + 1] = draw() & 0xffff;
      flash[pc + 2] = draw() & 0xffff;
      cpu.SP = 0x108 + (draw() % 0x7f0);
      cpu.cycles = 0;
      avr8js.avrInstruction(cpu);
      lines.push(
        hex(opcode, 4) + ' ' + hex(cpu.pc, 4) + ' ' + cpu.cycles.toString(16) + ' ' +
          hex(data[95], 2) + ' ' + hex(cpu.SP, 4) + ' ' + hex(fnv32(data), 8),
      );
    }
    process.stdout.write(lines.join('\n') + '\n');
  }
}

// ---- main ---------------------------------------------------------------

function usage() {
  process.stderr.write('usage: node oracle.js step <seed> <states>\n');
  process.exit(2);
}

function main(argv) {
  const command = argv[0];
  if (command === 'step' && argv.length === 3) {
    step(parseInt(argv[1], 10), parseInt(argv[2], 10));
  } else {
    usage();
  }
}

main(process.argv.slice(2));
