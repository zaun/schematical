// Tests for scripts/pin-polarity.js (plan-39-D). Run: node --test scripts/pin-polarity.test.js

'use strict';

const test = require('node:test');
const assert = require('node:assert');
const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const TOOL = path.join(__dirname, 'pin-polarity.js');

function pin(number, name, role, extra = '') {
    return `          {"number":${number},"name":"${name}","role":"${role}"${extra}}`;
}

function sramFile() {
    return `{
  "format": "schematical-parts",
  "version": 1,
  "kind": "sram",
  "devices": [
    {
      "id": "M2",
      "pinouts": {
        "p6": [
${pin(1, 'A0', 'input')},
${pin(2, 'CE#', 'input')},
${pin(3, 'OE#', 'input')},
${pin(4, 'VCC', 'power')}
        ],
        "bga": [
${pin(1, 'A0', 'input')},
${pin(2, 'CE#', 'input')},
${pin(3, 'OE#', 'input')},
${pin(4, 'VCC', 'power')}
        ]
      },
      "chipEnable": ["CE#"],
      "outputEnable": "OE#",
      "parts": [
        {"name": "OLDPART", "package": "DIP", "pinout": "p6"}
      ]
    },
    {
      "id": "M1",
      "pinouts": {
        "p6": [
${pin(1, 'A0', 'input')},
${pin(2, 'CE#', 'input')},
${pin(3, 'OE#', 'input')},
${pin(4, 'VCC', 'power')}
        ],
        "tsop": [
${pin(1, 'VCC', 'power')},
${pin(2, 'A0', 'input')},
${pin(3, 'CE#', 'input')},
${pin(4, 'OE#', 'input')}
        ]
      },
      "chipEnable": ["CE#"],
      "outputEnable": "OE#",
      "parts": [
        {"name": "M1-Q", "package": "DIP", "pinout": "p6", "datasheet": "m1-q1"},
        {"name": "M1-U", "package": "DIP", "pinout": "p6", "datasheet": "a-url"},
        {"name": "M1-P", "package": "DIP", "pinout": "p6", "datasheet": "m1"},
        {"name": "M1-T", "package": "TSOP", "pinout": "tsop", "datasheet": "m1url"}
      ]
    }
  ]
}
`;
}

function converterFile() {
    return `{
  "format": "schematical-parts",
  "version": 1,
  "kind": "converter",
  "devices": [
    {
      "id": "C1",
      "pinouts": {
        "p5": [
${pin(1, 'AIN0', 'input')},
${pin(2, 'A0/CS', 'input')},
${pin(3, 'AIN7/GPIO3/DRDY/CLK', 'inout')},
${pin(4, 'GND', 'ground')},
${pin(5, 'D0', 'output')}
        ]
      },
      "parts": [
        {"name": "C1-P", "package": "SOIC", "pinout": "p5", "datasheet": "c1url"}
      ]
    }
  ]
}
`;
}

function gateFile(extra = '') {
    return `{
  "format": "schematical-parts",
  "version": 1,
  "kind": "gate",
  "devices": [
    {
      "id": "G1",
      "pinouts": {
        "p2": [
${pin(1, 'A', 'input', extra)},
${pin(2, 'Y', 'output')}
        ]
      },
      "parts": []
    }
  ]
}
`;
}

const DOCS = `# Parts

A part whose maker no longer hosts its datasheet has a \`"manufacturer"\`
but no \`"datasheet"\`; \`listing_check\` prints how many there are. They are:

| Manufacturer | Datasheet (local copy) | Parts |
|---|---|---|
| Old Co | \`old\` | OLDPART, OTHER |

## Footprints
`;

function write(root, rel, text) {
    const f = path.join(root, rel);
    fs.mkdirSync(path.dirname(f), { recursive: true });
    fs.writeFileSync(f, text);
}

function fixture() {
    const root = fs.mkdtempSync(path.join(os.tmpdir(), 'pin-polarity-test-'));
    const parts = 'schematical/schematical/Parts/';
    write(root, parts + 'sram.json', sramFile());
    write(root, parts + 'converter.json', converterFile());
    write(root, parts + 'gate.json', gateFile());
    write(root, parts + 'footprints.json', '{"footprints":[]}\n');
    write(root, parts + 'datasheets.json', JSON.stringify({ format: 'x', version: 1, datasheets: [
        { id: 'm1', url: 'https://example.com/m1.pdf' },
        { id: 'm1-q1', url: 'https://example.com/m1-q1.pdf' },
        { id: 'a-url', url: 'https://example.com/a.pdf' },
        { id: 'm1url', url: 'https://example.com/m1url.pdf' },
        { id: 'c1url', url: 'https://example.com/c1.pdf' },
    ] }) + '\n');
    write(root, 'datasheets/sram/m1.pdf', '%PDF-1.4\n');
    write(root, 'datasheets/sram/M1-Q1.pdf', '%PDF-1.4\n');
    write(root, 'datasheets/sram/old.pdf', '%PDF-1.4\n');
    write(root, 'docs/parts-format.md', DOCS);
    return root;
}

function run(root, ...args) {
    const r = childProcess.spawnSync('node', [TOOL, '--root', root, ...args], { encoding: 'utf8' });
    return { status: r.status, out: r.stdout, err: r.stderr };
}

function ok(root, ...args) {
    const r = run(root, ...args);
    assert.strictEqual(r.status, 0, `${args.join(' ')}: ${r.out}${r.err}`);
    return r.out;
}

const sramPath = (root) => path.join(root, 'schematical/schematical/Parts/sram.json');
const convPath = (root) => path.join(root, 'schematical/schematical/Parts/converter.json');

// Records a row for every pinout of the fixture (siblings last).
function recordAll(root) {
    ok(root, 'record', 'M1', 'p6', '--source', 'm1', '--pages', 'p.1');
    ok(root, 'record', 'M1', 'tsop', '--source', 'm1url', '--pages', 'p.2');
    ok(root, 'record', 'M2', 'p6', '--source', 'local:datasheets/sram/old.pdf', '--pages', 'p.1');
    ok(root, 'record', 'M2', 'bga', '--source', 'sibling:p6', '--pages', 'p.1');
    ok(root, 'record', 'C1', 'p5', '--source', 'c1url', '--pages', 'p.3');
}

const rules = (out) => new Set(out.split('\n').filter((l) => /^R\d /.test(l)).map((l) => l.slice(0, 2)));

test('list orders kinds, devices and pinouts, and takes --devices', () => {
    const root = fixture();
    const lines = ok(root, 'list').trim().split('\n').map((l) => l.split('\t').slice(0, 3).join(' '));
    assert.deepStrictEqual(lines, ['sram M1 p6', 'sram M1 tsop', 'sram M2 p6', 'sram M2 bga', 'converter C1 p5']);
    assert.deepStrictEqual(ok(root, 'list', '--kind', 'sram', '--devices', '2-2').trim().split('\n')
        .map((l) => l.split('\t').slice(1, 3).join(' ')), ['M2 p6', 'M2 bga']);
    const first = ok(root, 'list', '--kind', 'sram').split('\n')[0].split('\t');
    assert.deepStrictEqual(first, ['sram', 'M1', 'p6', '3', '3', 'todo']);
    ok(root, 'record', 'M1', 'p6', '--source', 'm1', '--pages', 'p.1');
    assert.ok(!ok(root, 'list', '--todo').includes('M1\tp6'));
    assert.ok(ok(root, 'list').includes('M1\tp6\t3\t3\tdone'));
    assert.strictEqual(run(root, 'list', '--kind', 'sram', '--devices', '1-3').status, 1);
});

test('sources: non -q1 first, local before URL, then id order; table and sibling sources', () => {
    const root = fixture();
    const ids = ok(root, 'sources', 'M1', 'p6').trim().split('\n').map((l) => l.split('\t')[0]);
    assert.deepStrictEqual(ids, ['m1', 'a-url', 'm1-q1']);
    assert.strictEqual(ok(root, 'sources', 'M1', 'p6').split('\n')[0], 'm1\tdatasheets/sram/m1.pdf');
    assert.strictEqual(ok(root, 'sources', 'M2', 'p6').trim(), 'local:datasheets/sram/old.pdf\tdatasheets/sram/old.pdf');
    assert.strictEqual(ok(root, 'sources', 'M2', 'bga').trim(), 'sibling:p6\tno ledger row yet');
    ok(root, 'record', 'M2', 'p6', '--source', 'local:datasheets/sram/old.pdf', '--pages', 'p.1');
    assert.strictEqual(ok(root, 'sources', 'M2', 'bga').trim(), 'sibling:p6\thas a ledger row');
});

test('set changes exactly one line, writes true for a full list, refuses a power pin', () => {
    const root = fixture();
    const before = fs.readFileSync(convPath(root), 'utf8').split('\n');
    ok(root, 'set', 'C1', 'p5', 'A0/CS', 'CS');
    const after = fs.readFileSync(convPath(root), 'utf8').split('\n');
    assert.strictEqual(after.length, before.length);
    const changed = before.map((l, k) => (l === after[k] ? -1 : k)).filter((k) => k >= 0);
    assert.strictEqual(changed.length, 1);
    assert.strictEqual(after[changed[0]], '          {"number":2,"name":"A0/CS","role":"input","activeLow":["CS"]},');
    ok(root, 'set', 'C1', 'p5', 'A0/CS', 'CS,A0');
    assert.ok(fs.readFileSync(convPath(root), 'utf8').includes('"name":"A0/CS","role":"input","activeLow":true}'));
    ok(root, 'set', 'C1', 'p5', '@3', 'DRDY,AIN7');
    assert.ok(fs.readFileSync(convPath(root), 'utf8').includes('"activeLow":["AIN7","DRDY"]}'));
    ok(root, 'set', 'C1', 'p5', 'A0/CS', 'off');
    const off = fs.readFileSync(convPath(root), 'utf8').split('\n');
    assert.strictEqual(off[changed[0]], before[changed[0]]);
    const r = run(root, 'set', 'C1', 'p5', 'GND', 'true');
    assert.strictEqual(r.status, 1);
    assert.match(r.err, /ground/);
    assert.strictEqual(run(root, 'set', 'C1', 'p5', 'A0/CS', 'CE').status, 1);
    ok(root, 'set', 'M1', 'all', 'A0', 'true');
    assert.strictEqual((fs.readFileSync(sramPath(root), 'utf8').match(/"name":"A0","role":"input","activeLow":true/g) || []).length, 2);
});

test('record writes the flagged JSON and keeps the ledger sorted', () => {
    const root = fixture();
    ok(root, 'set', 'C1', 'p5', 'A0/CS', 'CS');
    ok(root, 'set', 'C1', 'p5', 'D0', 'true');
    ok(root, 'record', 'C1', 'p5', '--source', 'c1url', '--pages', 'pp.3-4', '--note', 'n');
    ok(root, 'record', 'M1', 'tsop', '--source', 'm1url', '--pages', 'p.2');
    ok(root, 'record', 'M1', 'p6', '--source', 'm1', '--pages', 'p.1');
    const ledger = fs.readFileSync(path.join(root, 'planning/plan-39-data/ledger.tsv'), 'utf8').replace(/\n$/, '').split('\n');
    assert.deepStrictEqual(ledger, [
        'kind\tdevice\tpinout\tsource\tpages\tflagged\tnote',
        'converter\tC1\tp5\tc1url\tpp.3-4\t{"A0/CS":["CS"],"D0":true}\tn',
        'sram\tM1\tp6\tm1\tp.1\t{}\t',
        'sram\tM1\ttsop\tm1url\tp.2\t{}\t',
    ]);
    assert.strictEqual(run(root, 'record', 'M1', 'p6', '--source', 'nope', '--pages', 'p.1').status, 1);
    assert.strictEqual(run(root, 'record', 'M2', 'bga', '--source', 'sibling:tsop', '--pages', 'p.1').status, 1);
});

test('seed-hash flags every # pin', () => {
    const root = fixture();
    assert.match(ok(root, 'seed-hash'), /seeded 8 pin/);
    assert.strictEqual((fs.readFileSync(sramPath(root), 'utf8').match(/#","role":"input","activeLow":true/g) || []).length, 8);
    assert.match(ok(root, 'seed-hash'), /seeded 0 pin/);
});

test('R1: a row naming no pinout, a repeat, empty pages, a source that does not resolve', () => {
    const root = fixture();
    ok(root, 'seed-hash');
    recordAll(root);
    assert.ok(!rules(ok(root, 'check')).has('R1'));
    const ledger = path.join(root, 'planning/plan-39-data/ledger.tsv');
    const good = fs.readFileSync(ledger, 'utf8');
    for (const bad of ['sram\tM9\tp6\tm1\tp.1\t{}\t', 'sram\tM1\tp6\tm1\tp.1\t{"CE#":true,"OE#":true}\t',
        'sram\tM1\tdip\tm1\tp.1\t{}\t']) {
        fs.writeFileSync(ledger, good + bad + '\n');
        assert.ok(rules(run(root, 'check').out).has('R1'), bad);
    }
    fs.writeFileSync(ledger, good.replace('\tc1url\tp.3\t', '\tc1url\t \t'));
    assert.ok(rules(run(root, 'check').out).has('R1'));
    fs.writeFileSync(ledger, good.replace('\tc1url\tp.3\t', '\tgone\tp.3\t'));
    assert.ok(rules(run(root, 'check').out).has('R1'));
    fs.writeFileSync(ledger, good.replace('\tsibling:p6\t', '\tlocal:datasheets/none.pdf\t'));
    assert.ok(rules(run(root, 'check').out).has('R1'));
    fs.writeFileSync(ledger, good);
    assert.strictEqual(run(root, 'check').status, 0);
});

test('R2: the ledger and the part file disagree', () => {
    const root = fixture();
    ok(root, 'seed-hash');
    recordAll(root);
    ok(root, 'set', 'C1', 'p5', 'D0', 'true');
    assert.ok(rules(run(root, 'check').out).has('R2'));
    ok(root, 'record', 'C1', 'p5', '--source', 'c1url', '--pages', 'p.3');
    assert.strictEqual(run(root, 'check').status, 0);
});

test('R3: a flag the loader would reject', () => {
    const root = fixture();
    ok(root, 'seed-hash');
    recordAll(root);
    const good = fs.readFileSync(convPath(root), 'utf8');
    for (const value of ['false', '[]', '["A0","CS"]', '["CE"]', '"CS"']) {
        fs.writeFileSync(convPath(root), good.replace('"name":"A0/CS","role":"input"}', `"name":"A0/CS","role":"input","activeLow":${value}}`));
        assert.ok(rules(run(root, 'check', '--kind', 'converter').out).has('R3'), value);
    }
    fs.writeFileSync(convPath(root), good.replace('"name":"GND","role":"ground"}', '"name":"GND","role":"ground","activeLow":true}'));
    assert.ok(rules(run(root, 'check', '--kind', 'converter').out).has('R3'));
    fs.writeFileSync(convPath(root), good);
    assert.strictEqual(run(root, 'check', '--kind', 'converter').status, 0);
});

test('R4 and R6: an unflagged # pin and an unflagged memory control pin', () => {
    const root = fixture();
    const out = run(root, 'check', '--kind', 'sram').out;
    assert.ok(rules(out).has('R4'));
    assert.ok(rules(out).has('R6'));
    ok(root, 'seed-hash');
    assert.strictEqual(run(root, 'check', '--kind', 'sram').status, 0);
    // R6 alone: a control pin whose name has no #.
    fs.writeFileSync(sramPath(root), fs.readFileSync(sramPath(root), 'utf8').replace('"outputEnable": "OE#"', '"outputEnable": "A0"'));
    const r6 = rules(run(root, 'check', '--kind', 'sram').out);
    assert.ok(r6.has('R6') && !r6.has('R4'));
});

test('R5: one device, the same pin flagged differently in two reviewed pinouts', () => {
    const root = fixture();
    ok(root, 'seed-hash');
    ok(root, 'set', 'M1', 'p6', 'A0', 'true');
    recordAll(root);
    assert.ok(rules(run(root, 'check').out).has('R5'));
    ok(root, 'set', 'M1', 'tsop', 'A0', 'true');
    ok(root, 'record', 'M1', 'tsop', '--source', 'm1url', '--pages', 'p.2');
    assert.strictEqual(run(root, 'check').status, 0);
});

test('R7: a pinout without a row, scoped only with --complete', () => {
    const root = fixture();
    ok(root, 'seed-hash');
    ok(root, 'record', 'C1', 'p5', '--source', 'c1url', '--pages', 'p.3');
    assert.ok(rules(run(root, 'check').out).has('R7'));
    assert.strictEqual(run(root, 'check', '--kind', 'sram').status, 0);
    assert.ok(rules(run(root, 'check', '--kind', 'sram', '--complete').out).has('R7'));
    assert.strictEqual(run(root, 'check', '--kind', 'converter', '--devices', '1-1', '--complete').status, 0);
    recordAll(root);
    assert.strictEqual(run(root, 'check').status, 0);
});

test('R8: a flag in an out-of-scope kind', () => {
    const root = fixture();
    ok(root, 'seed-hash');
    recordAll(root);
    const gate = path.join(root, 'schematical/schematical/Parts/gate.json');
    fs.writeFileSync(gate, gateFile(',"activeLow":true'));
    assert.ok(rules(run(root, 'check').out).has('R8'));
    fs.writeFileSync(gate, gateFile());
    assert.strictEqual(run(root, 'check').status, 0);
});

test('golden-check: only :low added passes; anything else fails; with and without --since', () => {
    const root = fixture();
    const git = (...a) => {
        const r = childProcess.spawnSync('git', ['-C', root, ...a], { encoding: 'utf8' });
        assert.strictEqual(r.status, 0, r.stderr);
    };
    const golden = (f) => path.join(root, 'backend/tests/golden', f);
    write(root, 'backend/tests/golden/parts.txt',
        'M1-P|sram|DIP||1 KB|||1:A0:input:default,2:CE#:input:default|mfr=x\n' +
        'C1-P|converter|SOIC||c|||1:AIN0:input:default,2:A0/CS:input:default:in|mfr=y\n' +
        'Z|gate|DIP||z|||1:A:input:default|mfr=z\n');
    write(root, 'backend/tests/golden/traces.txt', 't\n');
    write(root, 'backend/tests/golden/footprints.txt', 'f\n');
    git('init', '-q');
    git('-c', 'user.email=t@t', '-c', 'user.name=t', 'add', '-A');
    git('-c', 'user.email=t@t', '-c', 'user.name=t', 'commit', '-qm', 'base');
    const base = fs.readFileSync(golden('parts.txt'), 'utf8');
    const flagged = base.replace('2:CE#:input:default|', '2:CE#:input:default:low|')
        .replace('2:A0/CS:input:default:in|', '2:A0/CS:input:default:in:low=CS|');
    fs.writeFileSync(golden('parts.txt'), flagged);
    assert.strictEqual(run(root, 'golden-check').status, 0);
    fs.writeFileSync(golden('parts.txt'), flagged.replace('mfr=z', 'mfr=w'));
    assert.strictEqual(run(root, 'golden-check').status, 1);
    fs.writeFileSync(golden('parts.txt'), flagged.replace('1:AIN0:input:default,', '1:AIN0:output:default,'));
    assert.strictEqual(run(root, 'golden-check').status, 1);
    fs.writeFileSync(golden('parts.txt'), flagged);
    fs.writeFileSync(golden('traces.txt'), 'u\n');
    assert.strictEqual(run(root, 'golden-check').status, 1);
    fs.writeFileSync(golden('traces.txt'), 't\n');
    git('-c', 'user.email=t@t', '-c', 'user.name=t', 'commit', '-qam', 'flags');
    assert.strictEqual(run(root, 'golden-check').status, 0);
    assert.strictEqual(run(root, 'golden-check', '--since', 'HEAD~1').status, 0);
    fs.writeFileSync(golden('parts.txt'), flagged + 'NEW|gate|DIP||n|||1:A:input:default|mfr=n\n');
    git('-c', 'user.email=t@t', '-c', 'user.name=t', 'commit', '-qam', 'new part');
    assert.strictEqual(run(root, 'golden-check', '--since', 'HEAD~2').status, 1);
    assert.strictEqual(run(root, 'golden-check', '--since', 'HEAD~1').status, 1);
});

// ---- the remaining paths (plan-68-T) -------------------------------------------------------------------------------

const polarity = require('./pin-polarity.js');

test('the positioned parser names a missing comma in an object and in an array', () => {
    assert.throws(() => polarity.parsePositioned('{"a": 1 "b": 2}'), { message: 'JSON: expected , or } at offset 8' });
    assert.throws(() => polarity.parsePositioned('[1 2]'), { message: 'JSON: expected , or ] at offset 3' });
});

test('an unknown device or pinout is refused with what there is', () => {
    const root = fixture();
    const device = run(root, 'pins', 'NOPE', 'p6');
    assert.strictEqual(device.status, 1);
    assert.strictEqual(device.err, 'no in-scope device "NOPE"\n');
    const pinout = run(root, 'pins', 'M1', 'qfn');
    assert.strictEqual(pinout.status, 1);
    assert.strictEqual(pinout.err, 'device M1 has no pinout "qfn" (p6, tsop)\n');
});

test('record without --source or --pages prints its usage', () => {
    const root = fixture();
    const r = run(root, 'record', 'M1', 'p6', '--pages', 'p.1');
    assert.strictEqual(r.status, 1);
    assert.strictEqual(r.err, 'usage: record <device> <pinout> --source S --pages P [--note N]\n');
});

test('status counts the reviewed and total pinouts per kind', () => {
    const root = fixture();
    assert.strictEqual(ok(root, 'status'), 'sram\t0/4\nconverter\t0/1\ntotal\t0/5\n');
    recordAll(root);
    assert.strictEqual(ok(root, 'status'), 'sram\t4/4\nconverter\t1/1\ntotal\t5/5\n');
});

test('no command, --help, -h and help print the help', () => {
    const root = fixture();
    for (const args of [[], ['--help'], ['-h'], ['help']]) {
        const out = ok(root, ...args);
        assert.ok(out.startsWith('The active-low pin review tool (plan-39-D).\n\nusage: node scripts/pin-polarity.js'), args.join(' '));
        assert.ok(out.endsWith('Exit status: 0 on success, 1 when a check fails or a command is refused.\n'));
    }
});

// The lock a writing command takes for a root (withLock).
const lockOf = (root) => path.join(os.tmpdir(), `pin-polarity-${Buffer.from(path.resolve(root)).toString('hex').slice(-40)}.lock`);

test('a writer takes over a stale lock, and waits for a live one', async () => {
    const root = fixture();
    const lock = lockOf(root);
    fs.mkdirSync(lock);
    const old = new Date(Date.now() - 5 * 60 * 1000);
    fs.utimesSync(lock, old, old);
    assert.strictEqual(ok(root, 'set', 'M1', 'p6', 'A0', 'true'), 'M1 p6 1 A0 -> true\n');
    assert.strictEqual(fs.existsSync(lock), false);

    fs.mkdirSync(lock);
    const started = Date.now();
    const child = childProcess.spawn('node', [TOOL, '--root', root, 'set', 'M1', 'p6', 'A0', 'off'], { stdio: ['ignore', 'pipe', 'pipe'] });
    let stdout = '';
    child.stdout.on('data', (b) => { stdout += b; });
    const done = new Promise((resolve) => child.on('exit', resolve));
    await new Promise((resolve) => setTimeout(resolve, 600));
    assert.strictEqual(fs.readFileSync(sramPath(root), 'utf8').includes('"name":"A0","role":"input","activeLow":true'), true,
        'the waiting writer changed the file while the lock was held');
    fs.rmdirSync(lock);
    assert.strictEqual(await done, 0);
    assert.ok(Date.now() - started >= 600);
    assert.strictEqual(stdout, 'M1 p6 1 A0 -> off\n');
    assert.strictEqual(fs.readFileSync(sramPath(root), 'utf8'), sramFile());
});

test('a lock folder that cannot be made is an error, not a wait', () => {
    const root = fixture();
    const r = childProcess.spawnSync('node', [TOOL, '--root', root, 'set', 'M1', 'p6', 'A0', 'true'],
        { encoding: 'utf8', env: Object.assign({}, process.env, { TMPDIR: path.join(root, 'no-such-dir') }) });
    assert.notStrictEqual(r.status, 0);
    assert.match(r.stderr, /ENOENT: no such file or directory, mkdir '.*no-such-dir\/pin-polarity-[0-9a-f]+\.lock'/);
    assert.strictEqual(fs.readFileSync(sramPath(root), 'utf8'), sramFile());
});

test('fetch: usage, a local copy, an unknown id; a URL is downloaded once, a bad one refused', () => {
    const root = fixture();
    const usage = run(root, 'fetch');
    assert.strictEqual(usage.status, 1);
    assert.strictEqual(usage.err, 'usage: fetch <id>\n');
    assert.strictEqual(ok(root, 'fetch', 'M1'), path.join(root, 'datasheets/sram/m1.pdf') + '\n');
    const unknown = run(root, 'fetch', 'zz');
    assert.strictEqual(unknown.status, 1);
    assert.strictEqual(unknown.err, '"zz" is not in datasheets.json\n');

    // file:// URLs: curl copies them without the network.
    const served = path.join(root, 'served');
    write(root, 'served/good.pdf', '%PDF-1.7 the sheet\n');
    write(root, 'served/page.html', '<html>moved</html>\n');
    const tag = `pp-test-${process.pid}-${Date.now()}`;
    const good = `${tag}-good`, bad = `${tag}-bad`;
    write(root, 'schematical/schematical/Parts/datasheets.json', JSON.stringify({ datasheets: [
        { id: good, url: `file://${served}/good.pdf` }, { id: bad, url: `file://${served}/page.html` }] }));
    const cache = (id) => path.join('/tmp/pin-polarity', id);
    try {
        const file = path.join(cache(good), `${good}.pdf`);
        assert.strictEqual(ok(root, 'fetch', good), file + '\n');
        assert.strictEqual(fs.readFileSync(file, 'utf8'), '%PDF-1.7 the sheet\n');
        fs.unlinkSync(path.join(served, 'good.pdf'));
        assert.strictEqual(ok(root, 'fetch', good), file + '\n', 'the cached copy is reused');
        const refused = run(root, 'fetch', bad);
        assert.strictEqual(refused.status, 1);
        assert.strictEqual(refused.err, `file://${served}/page.html did not give a PDF\n`);
        assert.strictEqual(fs.existsSync(path.join(cache(bad), `${bad}.pdf`)), false);
    } finally {
        fs.rmSync(cache(good), { recursive: true, force: true });
        fs.rmSync(cache(bad), { recursive: true, force: true });
    }
});

test('pins lists designator, name, role and the flag as low or low=ALT', () => {
    const root = fixture();
    const usage = run(root, 'pins', 'C1');
    assert.strictEqual(usage.status, 1);
    assert.strictEqual(usage.err, 'usage: pins <device> <pinout>\n');
    ok(root, 'set', 'C1', 'p5', 'A0/CS', 'CS');
    ok(root, 'set', 'C1', 'p5', 'D0', 'true');
    assert.strictEqual(ok(root, 'pins', 'C1', 'p5'),
        '1\tAIN0\tinput\n2\tA0/CS\tinput\tlow=CS\n3\tAIN7/GPIO3/DRDY/CLK\tinout\n4\tGND\tground\n5\tD0\toutput\tlow\n');
});
