// The avr8js oracle (plan-62): runs the vendored avr8js (../avr8js, MIT) and
// prints what it does, in the text formats the C tests print for the C port
// (backend/src/models/avr_*.c), so the two can be compared line by line.
//
//   node oracle.js step <seed> <states>
//       Every opcode word 0..65535, <states> random states each, one
//       instruction per state. One line per vector.
//   node oracle.js asm <file.asm>
//       Assembles with avr8js's assembler; prints Intel HEX.
//   node oracle.js build <name.asm> <name.meta.json>
//       Prints a scenario file: the meta file's settings plus the program.
//   node oracle.js layout-check <layout.json>
//       Compares a register layout printed by the C side
//       (avr_periph_test --layout atmega328 32768) with avr8js's own
//       ATmega328P configs. Prints "layout <name> equal".
//   node oracle.js run <scenario.json> <layout.json>
//       Runs a scenario's program on avr8js peripherals built from the
//       layout; prints the trace.

'use strict';

const fs = require('fs');
const path = require('path');

const avr8jsDir = path.join(__dirname, '..', 'avr8js', 'dist', 'cjs');
const avr8js = require(path.join(avr8jsDir, 'index.js'));
const { assemble } = require(path.join(avr8jsDir, 'utils', 'assembler.js'));

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

// ---- asm, build -----------------------------------------------------------

function fail(message) {
  process.stderr.write(message + '\n');
  process.exit(1);
}

function assembleFile(file) {
  const result = assemble(fs.readFileSync(file, 'utf8'));
  if (result.errors.length > 0) {
    fail(file + ': ' + result.errors.join('; '));
  }
  return result.bytes;
}

// Intel HEX: 16 data bytes per record, then the end record.
function intelHex(bytes) {
  const lines = [];
  for (let offset = 0; offset < bytes.length; offset += 16) {
    const chunk = bytes.subarray(offset, Math.min(offset + 16, bytes.length));
    let sum = chunk.length + (offset >> 8) + (offset & 0xff);
    let text = ':' + hex(chunk.length, 2) + hex(offset, 4) + '00';
    for (const byte of chunk) {
      text += hex(byte, 2);
      sum += byte;
    }
    lines.push((text + hex(-sum & 0xff, 2)).toUpperCase());
  }
  lines.push(':00000001FF');
  return lines.join('\n') + '\n';
}

function parseIntelHex(text, bytes) {
  for (const line of text.split('\n')) {
    if (line[0] !== ':') {
      continue;
    }
    const count = parseInt(line.substr(1, 2), 16);
    const address = parseInt(line.substr(3, 4), 16);
    const type = parseInt(line.substr(7, 2), 16);
    if (type !== 0) {
      continue;
    }
    for (let i = 0; i < count; i++) {
      bytes[address + i] = parseInt(line.substr(9 + i * 2, 2), 16);
    }
  }
}

function build(asmFile, metaFile) {
  const meta = JSON.parse(fs.readFileSync(metaFile, 'utf8'));
  const scenario = Object.assign({}, meta, { program: intelHex(assembleFile(asmFile)) });
  process.stdout.write(JSON.stringify(scenario, null, 1) + '\n');
}

// ---- layouts ----------------------------------------------------------------

// A layout's port config, as avr8js's AVRPortConfig.
function portConfig(port) {
  const config = { PIN: port.PIN, DDR: port.DDR, PORT: port.PORT, externalInterrupts: port.externalInterrupts };
  if (port.pinChange) {
    config.pinChange = port.pinChange;
  }
  return config;
}

// Both sides to one shape: a dividers object becomes an array.
function normalize(value) {
  if (Array.isArray(value)) {
    return value.map(normalize);
  }
  if (value && typeof value === 'object') {
    const keys = Object.keys(value);
    if (keys.length === 8 && keys.every((key, index) => key === String(index))) {
      return keys.map((key) => normalize(value[key]));
    }
    const out = {};
    for (const key of keys.sort()) {
      out[key] = normalize(value[key]);
    }
    return out;
  }
  return value;
}

function firstDifference(ours, theirs, where) {
  if (typeof ours !== typeof theirs || Array.isArray(ours) !== Array.isArray(theirs)) {
    return where + ': ours ' + JSON.stringify(ours) + ', avr8js ' + JSON.stringify(theirs);
  }
  if (ours && typeof ours === 'object') {
    const keys = Array.from(new Set(Object.keys(ours).concat(Object.keys(theirs)))).sort();
    for (const key of keys) {
      const difference = firstDifference(ours[key], theirs[key], where + '.' + key);
      if (difference) {
        return difference;
      }
    }
    return null;
  }
  return ours === theirs ? null : where + ': ours ' + JSON.stringify(ours) + ', avr8js ' + JSON.stringify(theirs);
}

function layoutCheck(layoutFile) {
  const layout = JSON.parse(fs.readFileSync(layoutFile, 'utf8'));
  const pairs = [];
  for (const port of layout.ports) {
    pairs.push(['port' + port.letter, portConfig(port), avr8js['port' + port.letter + 'Config']]);
  }
  pairs.push(['clock', layout.clock, avr8js.clockConfig]);
  layout.timers.forEach((timer, index) => {
    pairs.push(['timer' + index, timer, avr8js['timer' + index + 'Config']]);
  });
  // The first of each serial peripheral is the ATmega328P's only one.
  pairs.push(['usart0', layout.usarts[0], avr8js.usart0Config]);
  pairs.push(['spi', layout.spis[0], avr8js.spiConfig]);
  pairs.push(['twi', layout.twis[0], avr8js.twiConfig]);
  pairs.push(['adc', layout.adc, avr8js.adcConfig]);
  pairs.push(['eeprom', layout.eeprom, avr8js.eepromConfig]);
  pairs.push(['watchdog', layout.watchdog, avr8js.watchdogConfig]);
  for (const [name, ours, theirs] of pairs) {
    if (!theirs) {
      fail('layout ' + layout.name + ': avr8js has no config for ' + name);
    }
    const difference = firstDifference(normalize(ours), normalize(theirs), name);
    if (difference) {
      fail('layout ' + layout.name + ' differs: ' + difference);
    }
  }
  process.stdout.write('layout ' + layout.name + ' equal\n');
}

// ---- run ----------------------------------------------------------------------

const RUN_HZ = 16000000;

function run(scenarioFile, layoutFile) {
  const scenario = JSON.parse(fs.readFileSync(scenarioFile, 'utf8'));
  const layout = JSON.parse(fs.readFileSync(layoutFile, 'utf8'));
  const flash = new Uint16Array(scenario.flash / 2).fill(0xffff); // erased
  parseIntelHex(scenario.program, new Uint8Array(flash.buffer));
  const cpu = new avr8js.CPU(flash, scenario.sram);
  const out = [];

  // The order the C chip builds its peripherals in (avr_chip_create).
  const ports = {};
  for (const port of layout.ports) {
    const gpio = new avr8js.AVRIOPort(cpu, portConfig(port));
    ports[port.letter] = gpio;
    gpio.addListener((value) => {
      out.push('p ' + cpu.cycles.toString(16) + ' ' + port.letter + ' ' + hex(value, 2) + ' ' + hex(cpu.data[port.DDR], 2));
    });
  }
  const clock = new avr8js.AVRClock(cpu, RUN_HZ, layout.clock);
  for (const timer of layout.timers) {
    new avr8js.AVRTimer(cpu, timer);
  }
  // The scripted outside of the serial peripherals: the C test's is the same.
  const usarts = layout.usarts.map((config, index) => {
    const usart = new avr8js.AVRUSART(cpu, config, RUN_HZ);
    usart.onByteTransmit = (value) => {
      out.push('u ' + cpu.cycles.toString(16) + ' ' + index + ' ' + hex(value, 2));
    };
    return usart;
  });
  layout.spis.forEach((config, index) => {
    const spi = new avr8js.AVRSPI(cpu, config, RUN_HZ);
    spi.onByte = (value) => {
      out.push('s ' + cpu.cycles.toString(16) + ' ' + index + ' ' + hex(value, 2));
      cpu.addClockEvent(() => spi.completeTransfer(value ^ 0xa5), spi.transferCycles);
    };
  });
  layout.twis.forEach((config, index) => {
    const twi = new avr8js.AVRTWI(cpu, config, RUN_HZ);
    let nextRead = 0x10;
    const request = (name, argument, answer) => {
      out.push('t ' + cpu.cycles.toString(16) + ' ' + index + ' ' + name + ' ' + hex(argument, 2));
      cpu.addClockEvent(answer, 40);
    };
    twi.eventHandler = {
      start: (repeated) => request('start', repeated ? 1 : 0, () => twi.completeStart()),
      stop: () => request('stop', 0, () => twi.completeStop()),
      connectToSlave: (address, write) =>
        request('connect', (address << 1) | (write ? 0 : 1), () => twi.completeConnect(address === 0x50)),
      writeByte: (value) => request('write', value, () => twi.completeWrite(true)),
      readByte: (ack) => {
        const value = nextRead;
        nextRead = (nextRead + 1) & 0xff;
        request('read', ack ? 1 : 0, () => twi.completeRead(value));
      },
    };
  });
  const adc = new avr8js.AVRADC(cpu, layout.adc);
  new avr8js.AVREEPROM(cpu, new avr8js.EEPROMMemoryBackend(scenario.eeprom), layout.eeprom);
  new avr8js.AVRWatchdog(cpu, layout.watchdog, clock);

  const stimuli = scenario.stimuli || [];
  let next = 0;
  for (let i = 0; i < scenario.steps; i++) {
    while (next < stimuli.length && stimuli[next][0] <= cpu.cycles) {
      const stimulus = stimuli[next++];
      if (stimulus[1] === 'pin') {
        ports[stimulus[2]].setPin(stimulus[3], !!stimulus[4]);
      } else if (stimulus[1] === 'rx') {
        usarts[stimulus[2]].writeByte(stimulus[3]);
      } else if (stimulus[1] === 'adc') {
        adc.channelValues[stimulus[2]] = stimulus[3] / 1000;
      } else {
        fail('unknown stimulus ' + JSON.stringify(stimulus));
      }
    }
    avr8js.avrInstruction(cpu);
    cpu.tick();
    out.push('i ' + cpu.pc.toString(16) + ' ' + cpu.cycles.toString(16) + ' ' + hex(cpu.data[95], 2) + ' ' +
      hex(cpu.SP, 4) + ' ' + hex(fnv32(cpu.data), 8));
    if (out.length >= 4096) {
      process.stdout.write(out.join('\n') + '\n');
      out.length = 0;
    }
  }
  if (out.length > 0) {
    process.stdout.write(out.join('\n') + '\n');
  }
}

// ---- main ---------------------------------------------------------------

function usage() {
  process.stderr.write(
    'usage: node oracle.js step <seed> <states>\n' +
      '       node oracle.js asm <file.asm>\n' +
      '       node oracle.js build <name.asm> <name.meta.json>\n' +
      '       node oracle.js layout-check <layout.json>\n' +
      '       node oracle.js run <scenario.json> <layout.json>\n',
  );
  process.exit(2);
}

function main(argv) {
  const command = argv[0];
  if (command === 'step' && argv.length === 3) {
    step(parseInt(argv[1], 10), parseInt(argv[2], 10));
  } else if (command === 'asm' && argv.length === 2) {
    process.stdout.write(intelHex(assembleFile(argv[1])));
  } else if (command === 'build' && argv.length === 3) {
    build(argv[1], argv[2]);
  } else if (command === 'layout-check' && argv.length === 2) {
    layoutCheck(argv[1]);
  } else if (command === 'run' && argv.length === 3) {
    run(argv[1], argv[2]);
  } else {
    usage();
  }
}

main(process.argv.slice(2));
