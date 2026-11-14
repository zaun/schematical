#!/usr/bin/env node
// The active-low pin review tool (plan-39-D). Run with --help (HELP below).

'use strict';

const fs = require('fs');
const os = require('os');
const path = require('path');
const childProcess = require('child_process');

const IN_SCOPE = ['counter', 'decoder', 'latch', 'register', 'shift', 'arith', 'dff', 'jkff', 'mux',
    'cpu', 'pio', 'rtc', 'buffer', 'oscillator', 'eeprom', 'sram', 'serial', 'converter', 'mcu'];
const SIGNAL = new Set(['input', 'output', 'inout']);
const NOT_KINDS = new Set(['datasheets', 'footprints']);
const LEDGER_HEADER = ['kind', 'device', 'pinout', 'source', 'pages', 'flagged', 'note'];
const CACHE = '/tmp/pin-polarity';
const USER_AGENT = 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 ' +
    '(KHTML, like Gecko) Version/17.0 Safari/605.1.15';

const HELP = `The active-low pin review tool (plan-39-D).

usage: node scripts/pin-polarity.js [--root <dir>] <command> [args]

A pinout of an in-scope kind is reviewed against one datasheet; its pins the
datasheet marks active low get "activeLow" in the part file (plan-39-A), and
the review is recorded as one row of planning/plan-39-data/ledger.tsv.

Commands
  list [--kind K] [--devices A-B] [--todo]
      One line per pinout: kind, device, pinout, signal pins, parts, todo|done.
      Kinds in review order, devices in ASCII id order; --devices takes
      1-based inclusive indices into the kind's sorted devices (needs --kind).
  sources <device> <pinout>
      The pinout's datasheet sources, best first: a part's datasheet id
      (non -q1/-ep first, then local before URL, then id order), the local
      PDF of the "no longer hosts" table, or sibling:<pinout> for a pinout
      with no part. Each line: source, then the local path or URL.
  fetch <id>
      Download a datasheet URL to ${CACHE}/<id>/<id>.pdf (curl -L with a
      browser User-Agent, then bare curl). Prints the path; a cached or a
      local copy is reused.
  pins <device> <pinout>
      The pinout's pins: designator, name, role, flag.
  set <device> <pinout|all> <name|@designator> <true|ALT[,ALT...]|off>
      Set a pin's flag by editing its line in the part file (nothing else in
      the file changes). A list that covers every alternative is written as
      true. Refuses power, ground, nc and absent pins.
  record <device> <pinout> --source S --pages P [--note N]
      Add or replace the pinout's ledger row; "flagged" is read from the part
      file. S is a datasheet id, local:datasheets/<group>/<file>.pdf or
      sibling:<pinout>.
  check [--kind K] [--devices A-B] [--complete]
      The rules R1-R8 (plan-39-D § 3). Without a scope every rule runs; with
      one, R7 (every pinout has a row) runs only with --complete.
  status
      Reviewed and total pinouts per kind.
  seed-hash
      Flag every alternative ending in "#" of every in-scope signal pin.
  golden-check [--since REV]
      The diff of backend/tests/golden/parts.txt against REV (default HEAD,
      the working tree's change) must only add ":low" / ":low=..." to pins,
      and traces.txt and footprints.txt must not change.

Exit status: 0 on success, 1 when a check fails or a command is refused.`;

// ---- JSON with positions ------------------------------------------------------

// Parses JSON text; objects become {kind:'object', entries:[{key, keyStart, value,
// valueStart, valueEnd}], start, end}, arrays {kind:'array', items, start, end},
// scalars {kind:'scalar', value, start, end}. Offsets are into `text`.
function parsePositioned(text) {
    let i = 0;
    const ws = () => { while (i < text.length && ' \t\r\n'.includes(text[i])) i++; };
    const fail = (what) => { throw new Error(`JSON: ${what} at offset ${i}`); };
    function str() {
        const start = i;
        i++;
        while (text[i] !== '"') {
            if (i >= text.length) fail('unterminated string');
            if (text[i] === '\\') i++;
            i++;
        }
        i++;
        return JSON.parse(text.slice(start, i));
    }
    function value() {
        ws();
        const start = i;
        const c = text[i];
        if (c === '{') {
            const entries = [];
            i++;
            ws();
            if (text[i] === '}') { i++; return { kind: 'object', entries, start, end: i }; }
            for (;;) {
                ws();
                if (text[i] !== '"') fail('expected a key');
                const keyStart = i;
                const key = str();
                ws();
                if (text[i] !== ':') fail('expected :');
                i++;
                const v = value();
                entries.push({ key, keyStart, value: v, valueStart: v.start, valueEnd: v.end });
                ws();
                if (text[i] === ',') { i++; continue; }
                if (text[i] === '}') { i++; break; }
                fail('expected , or }');
            }
            return { kind: 'object', entries, start, end: i };
        }
        if (c === '[') {
            const items = [];
            i++;
            ws();
            if (text[i] === ']') { i++; return { kind: 'array', items, start, end: i }; }
            for (;;) {
                items.push(value());
                ws();
                if (text[i] === ',') { i++; continue; }
                if (text[i] === ']') { i++; break; }
                fail('expected , or ]');
            }
            return { kind: 'array', items, start, end: i };
        }
        if (c === '"') return { kind: 'scalar', value: str(), start, end: i };
        const m = /^(true|false|null|-?\d+(\.\d+)?([eE][+-]?\d+)?)/.exec(text.slice(i, i + 40));
        if (!m) fail('unexpected character');
        i += m[0].length;
        return { kind: 'scalar', value: JSON.parse(m[0]), start, end: i };
    }
    const v = value();
    ws();
    if (i !== text.length) fail('trailing text');
    return v;
}

const get = (node, key) => {
    const e = node && node.kind === 'object' ? node.entries.find((x) => x.key === key) : undefined;
    return e ? e.value : undefined;
};
const plain = (node) => (node === undefined ? undefined : JSON.parse(JSON.stringify(toPlain(node))));
function toPlain(node) {
    if (node.kind === 'scalar') return node.value;
    if (node.kind === 'array') return node.items.map(toPlain);
    const o = {};
    for (const e of node.entries) o[e.key] = toPlain(e.value);
    return o;
}

// ---- The library --------------------------------------------------------------

const alternatives = (name) => name.split('/');

// The flag value as written: true, an array, or undefined.
function maskOf(name, flag) {
    const alts = alternatives(name);
    if (flag === true) return alts.map(() => true);
    if (Array.isArray(flag)) return alts.map((a) => flag.includes(a));
    return alts.map(() => false);
}

// The canonical flag for a set of flagged alternatives (strings).
function canonical(name, flaggedSet) {
    const alts = alternatives(name);
    const bits = alts.map((a) => flaggedSet.has(a));
    if (!bits.some(Boolean)) return undefined;
    if (bits.every(Boolean)) return true;
    const out = [];
    alts.forEach((a, k) => { if (bits[k] && !out.includes(a)) out.push(a); });
    return out;
}

class Library {
    constructor(root) {
        this.root = root;
        this.partsDir = path.join(root, 'schematical', 'schematical', 'Parts');
        this.ledgerPath = path.join(root, 'planning', 'plan-39-data', 'ledger.tsv');
        this.kinds = {};
        for (const kind of IN_SCOPE) {
            const file = path.join(this.partsDir, `${kind}.json`);
            if (fs.existsSync(file)) this.kinds[kind] = this.loadKind(kind, file);
        }
    }

    loadKind(kind, file) {
        const text = fs.readFileSync(file, 'utf8');
        const tree = parsePositioned(text);
        const devices = [];
        for (const d of get(tree, 'devices').items) {
            const pinouts = [];
            for (const e of get(d, 'pinouts').entries) {
                pinouts.push({
                    name: e.key,
                    pins: e.value.items.map((p) => ({
                        node: p,
                        designator: String(plain(get(p, 'number')) ?? plain(get(p, 'ball'))),
                        name: plain(get(p, 'name')),
                        role: plain(get(p, 'role')),
                        flag: plain(get(p, 'activeLow')),
                        hasFlag: get(p, 'activeLow') !== undefined,
                    })),
                });
            }
            const parts = get(d, 'parts') ? get(d, 'parts').items.map(plain) : [];
            devices.push({ kind, id: plain(get(d, 'id')), pinouts, parts, json: plain(d) });
        }
        devices.sort((a, b) => (a.id < b.id ? -1 : a.id > b.id ? 1 : 0));
        return { kind, file, text, devices };
    }

    allDevices() {
        return IN_SCOPE.filter((k) => this.kinds[k]).flatMap((k) => this.kinds[k].devices);
    }

    device(id) {
        const d = this.allDevices().find((x) => x.id === id);
        if (!d) throw new UserError(`no in-scope device "${id}"`);
        return d;
    }

    pinout(device, name) {
        const p = device.pinouts.find((x) => x.name === name);
        if (!p) throw new UserError(`device ${device.id} has no pinout "${name}" ` +
            `(${device.pinouts.map((x) => x.name).join(', ')})`);
        return p;
    }

    // Devices in scope: [{kind, device}] in list order.
    scope(kind, range) {
        if (range && !kind) throw new UserError('--devices needs --kind');
        if (kind && !IN_SCOPE.includes(kind)) throw new UserError(`"${kind}" is not an in-scope kind`);
        const kinds = kind ? [kind] : IN_SCOPE;
        const out = [];
        for (const k of kinds) {
            if (!this.kinds[k]) continue;
            let devices = this.kinds[k].devices;
            if (range) {
                const [a, b] = range;
                if (a < 1 || b > devices.length || a > b) {
                    throw new UserError(`--devices ${a}-${b} is outside 1-${devices.length} for ${k}`);
                }
                devices = devices.slice(a - 1, b);
            }
            out.push(...devices);
        }
        return out;
    }

    // ---- datasheets
    datasheetUrls() {
        if (!this._urls) {
            const file = path.join(this.partsDir, 'datasheets.json');
            this._urls = new Map();
            if (fs.existsSync(file)) {
                for (const d of JSON.parse(fs.readFileSync(file, 'utf8')).datasheets) this._urls.set(d.id, d.url);
            }
        }
        return this._urls;
    }

    localPdfs() {
        if (!this._local) {
            this._local = new Map();
            const dir = path.join(this.root, 'datasheets');
            if (fs.existsSync(dir)) {
                for (const group of fs.readdirSync(dir).sort()) {
                    const g = path.join(dir, group);
                    if (!fs.statSync(g).isDirectory()) continue;
                    for (const f of fs.readdirSync(g).sort()) {
                        if (!f.toLowerCase().endsWith('.pdf')) continue;
                        const key = f.slice(0, -4).toLowerCase();
                        if (!this._local.has(key)) this._local.set(key, `datasheets/${group}/${f}`);
                    }
                }
            }
        }
        return this._local;
    }

    // part name -> datasheet id of the "no longer hosts" table.
    noLongerHosted() {
        if (!this._unhosted) {
            this._unhosted = new Map();
            const file = path.join(this.root, 'docs', 'parts-format.md');
            const text = fs.existsSync(file) ? fs.readFileSync(file, 'utf8') : '';
            const at = text.indexOf('no longer hosts its datasheet');
            if (at >= 0) {
                const lines = text.slice(at).split('\n');
                let inTable = false;
                for (const line of lines) {
                    if (line.startsWith('|')) {
                        inTable = true;
                        const cells = line.split('|').slice(1, -1).map((c) => c.trim());
                        const m = /^`([^`]+)`$/.exec(cells[1] || '');
                        if (!m) continue;
                        for (const part of cells[2].split(',').map((x) => x.trim()).filter(Boolean)) {
                            this._unhosted.set(part, m[1]);
                        }
                    } else if (inTable) {
                        break;
                    }
                }
            }
        }
        return this._unhosted;
    }

    // [{source, where}] best first.
    sources(device, pinoutName) {
        const parts = device.parts.filter((p) => p.pinout === pinoutName);
        const local = this.localPdfs();
        const urls = this.datasheetUrls();
        const out = [];
        if (parts.length > 0) {
            const ids = [...new Set(parts.map((p) => p.datasheet).filter(Boolean))];
            const rank = (id) => [/-(q1|ep)$/i.test(id) ? 1 : 0, local.has(id.toLowerCase()) ? 0 : 1];
            ids.sort((a, b) => {
                const ra = rank(a), rb = rank(b);
                return ra[0] - rb[0] || ra[1] - rb[1] || (a < b ? -1 : a > b ? 1 : 0);
            });
            for (const id of ids) {
                const where = local.has(id.toLowerCase()) ? local.get(id.toLowerCase()) : urls.get(id) || '(not in datasheets.json)';
                out.push({ source: id, where });
            }
            if (ids.length === 0) {
                const unhosted = this.noLongerHosted();
                const seen = new Set();
                for (const p of parts) {
                    const id = unhosted.get(p.name);
                    if (!id || seen.has(id)) continue;
                    seen.add(id);
                    const file = local.get(id.toLowerCase());
                    if (file) out.push({ source: `local:${file}`, where: file });
                }
            }
        } else {
            const rows = this.ledger();
            for (const p of device.pinouts) {
                if (p.name === pinoutName) continue;
                const has = rows.some((r) => r.device === device.id && r.pinout === p.name);
                out.push({ source: `sibling:${p.name}`, where: has ? 'has a ledger row' : 'no ledger row yet' });
            }
        }
        return out;
    }

    resolves(device, source) {
        if (source.startsWith('local:')) return fs.existsSync(path.join(this.root, source.slice(6)));
        if (source.startsWith('sibling:')) {
            const name = source.slice(8);
            return device.pinouts.some((p) => p.name === name) &&
                this.ledger().some((r) => r.device === device.id && r.pinout === name);
        }
        return this.datasheetUrls().has(source);
    }

    // ---- ledger
    ledger() {
        if (!this._ledger) {
            this._ledger = [];
            if (fs.existsSync(this.ledgerPath)) {
                const lines = fs.readFileSync(this.ledgerPath, 'utf8').split('\n').filter((l) => l !== '');
                for (const line of lines.slice(1)) {
                    const c = line.split('\t');
                    const row = {};
                    LEDGER_HEADER.forEach((h, k) => { row[h] = c[k] === undefined ? '' : c[k]; });
                    this._ledger.push(row);
                }
            }
        }
        return this._ledger;
    }

    writeLedger(rows) {
        const kinds = IN_SCOPE.slice().sort();
        const order = new Map();
        for (const d of this.allDevices()) d.pinouts.forEach((p, k) => order.set(`${d.id}\t${p.name}`, k));
        const sorted = rows.slice().sort((a, b) => {
            const ka = kinds.indexOf(a.kind), kb = kinds.indexOf(b.kind);
            if (ka !== kb) return ka - kb;
            if (a.device !== b.device) return a.device < b.device ? -1 : 1;
            return (order.get(`${a.device}\t${a.pinout}`) ?? 0) - (order.get(`${b.device}\t${b.pinout}`) ?? 0);
        });
        const text = [LEDGER_HEADER.join('\t'), ...sorted.map((r) => LEDGER_HEADER.map((h) => r[h] || '').join('\t'))]
            .join('\n') + '\n';
        fs.mkdirSync(path.dirname(this.ledgerPath), { recursive: true });
        fs.writeFileSync(this.ledgerPath, text);
        this._ledger = sorted;
    }
}

// A pinout's flags as the ledger writes them: {"CE#":true,"A0/CS":["CS"]}.
function flaggedOf(pinout) {
    const o = {};
    for (const p of pinout.pins) if (p.hasFlag) o[p.name] = p.flag;
    return JSON.stringify(o);
}

class UserError extends Error {}

// ---- Locking: reviews may run in parallel against one tree ---------------------

function withLock(root, fn) {
    const lock = path.join(os.tmpdir(), `pin-polarity-${Buffer.from(path.resolve(root)).toString('hex').slice(-40)}.lock`);
    const deadline = Date.now() + 120000;
    for (;;) {
        try {
            fs.mkdirSync(lock);
            break;
        } catch (e) {
            if (e.code !== 'EEXIST') throw e;
            try {
                if (Date.now() - fs.statSync(lock).mtimeMs > 60000) { fs.rmdirSync(lock); continue; }
            } catch (_) { /* gone */ }
            if (Date.now() > deadline) throw new UserError(`could not take the lock ${lock}`);
            childProcess.spawnSync('sleep', ['0.05']);
        }
    }
    try {
        return fn();
    } finally {
        fs.rmdirSync(lock);
    }
}

// ---- Editing a pin's line ---------------------------------------------------------

function formatFlag(flag, spaced) {
    if (flag === true) return 'true';
    return '[' + flag.map((s) => JSON.stringify(s)).join(spaced ? ', ' : ',') + ']';
}

// Returns text with the pin object's "activeLow" set to `flag` (undefined removes it).
function editPin(text, node, flag) {
    const entries = node.entries;
    const existing = entries.findIndex((e) => e.key === 'activeLow');
    // The separator style of this object: what lies between the first two members.
    let sep = ',';
    let colon = ':';
    if (entries.length >= 2) sep = text.slice(entries[0].valueEnd, entries[1].keyStart);
    const c0 = text.slice(entries[0].keyStart, entries[0].valueStart);
    colon = c0.slice(c0.indexOf('":') + 1);
    const spaced = / $/.test(colon) || / $/.test(sep);
    if (existing >= 0) {
        const e = entries[existing];
        if (flag === undefined) {
            const from = existing > 0 ? entries[existing - 1].valueEnd : e.keyStart;
            const to = existing > 0 ? e.valueEnd : (entries[1] ? entries[1].keyStart : e.valueEnd);
            return text.slice(0, from) + text.slice(to);
        }
        return text.slice(0, e.valueStart) + formatFlag(flag, spaced) + text.slice(e.valueEnd);
    }
    if (flag === undefined) return text;
    const last = entries[entries.length - 1];
    return text.slice(0, last.valueEnd) + `${sep}"activeLow"${colon}${formatFlag(flag, spaced)}` +
        text.slice(last.valueEnd);
}

// Applies [{node, flag}] edits to one kind file (offsets from the parsed text).
function applyEdits(kindData, edits) {
    let text = kindData.text;
    const sorted = edits.slice().sort((a, b) => b.node.start - a.node.start);
    for (const { node, flag } of sorted) {
        const before = text.slice(0, node.start);
        const objText = text.slice(node.start, node.end);
        const local = parsePositioned(objText);
        const edited = editPin(objText, local, flag);
        text = before + edited + text.slice(node.end);
    }
    fs.writeFileSync(kindData.file, text);
}

// ---- Commands -------------------------------------------------------------------

function parseRange(s) {
    const m = /^(\d+)-(\d+)$/.exec(s || '');
    if (!m) throw new UserError(`--devices wants A-B, got "${s}"`);
    return [Number(m[1]), Number(m[2])];
}

function options(args, names, flags = []) {
    const o = { _: [] };
    for (let k = 0; k < args.length; k++) {
        const a = args[k];
        if (names.includes(a)) {
            if (k + 1 >= args.length) throw new UserError(`${a} needs a value`);
            o[a.slice(2)] = args[++k];
        } else if (flags.includes(a)) {
            o[a.slice(2)] = true;
        } else if (a.startsWith('--')) {
            throw new UserError(`unknown option ${a}`);
        } else {
            o._.push(a);
        }
    }
    return o;
}

function cmdList(lib, args, out) {
    const o = options(args, ['--kind', '--devices'], ['--todo']);
    const rows = lib.ledger();
    const done = new Set(rows.map((r) => `${r.device}\t${r.pinout}`));
    for (const d of lib.scope(o.kind, o.devices ? parseRange(o.devices) : null)) {
        for (const p of d.pinouts) {
            const isDone = done.has(`${d.id}\t${p.name}`);
            if (o.todo && isDone) continue;
            const signal = p.pins.filter((x) => SIGNAL.has(x.role)).length;
            const parts = d.parts.filter((x) => x.pinout === p.name).length;
            out.push([d.kind, d.id, p.name, signal, parts, isDone ? 'done' : 'todo'].join('\t'));
        }
    }
    return 0;
}

function cmdSources(lib, args, out) {
    const [id, pinoutName] = args;
    if (!pinoutName) throw new UserError('usage: sources <device> <pinout>');
    const d = lib.device(id);
    lib.pinout(d, pinoutName);
    const s = lib.sources(d, pinoutName);
    if (s.length === 0) throw new UserError(`no source for ${id} ${pinoutName}`);
    for (const x of s) out.push(`${x.source}\t${x.where}`);
    return 0;
}

function cmdFetch(lib, args, out) {
    const [id] = args;
    if (!id) throw new UserError('usage: fetch <id>');
    const local = lib.localPdfs().get(id.toLowerCase());
    if (local) { out.push(path.join(lib.root, local)); return 0; }
    const url = lib.datasheetUrls().get(id);
    if (!url) throw new UserError(`"${id}" is not in datasheets.json`);
    const dir = path.join(CACHE, id);
    const file = path.join(dir, `${id}.pdf`);
    const isPdf = (f) => {
        try {
            const fd = fs.openSync(f, 'r');
            const b = Buffer.alloc(4);
            fs.readSync(fd, b, 0, 4, 0);
            fs.closeSync(fd);
            return b.toString('latin1') === '%PDF';
        } catch (_) { return false; }
    };
    if (isPdf(file)) { out.push(file); return 0; }
    fs.mkdirSync(dir, { recursive: true });
    for (const extra of [['-A', USER_AGENT], []]) {
        childProcess.spawnSync('curl', ['-sSL', '--max-time', '120', ...extra, '-o', file, url], { stdio: 'ignore' });
        if (isPdf(file)) { out.push(file); return 0; }
    }
    try { fs.unlinkSync(file); } catch (_) { /* none */ }
    throw new UserError(`${url} did not give a PDF`);
}

function cmdPins(lib, args, out) {
    const [id, pinoutName] = args;
    if (!pinoutName) throw new UserError('usage: pins <device> <pinout>');
    const p = lib.pinout(lib.device(id), pinoutName);
    for (const pin of p.pins) {
        const flag = pin.hasFlag ? (pin.flag === true ? 'low' : `low=${[].concat(pin.flag).join(';')}`) : '';
        out.push([pin.designator, pin.name, pin.role, flag].join('\t').replace(/\t$/, ''));
    }
    return 0;
}

function cmdSet(lib, args, out) {
    const [id, which, name, value] = args;
    if (value === undefined) throw new UserError('usage: set <device> <pinout|all> <name|@designator> <true|ALT[,ALT...]|off>');
    const d = lib.device(id);
    const pinouts = which === 'all' ? d.pinouts : [lib.pinout(d, which)];
    const edits = [];
    for (const p of pinouts) {
        for (const pin of p.pins) {
            if (name.startsWith('@') ? pin.designator !== name.slice(1) : pin.name !== name) continue;
            if (!SIGNAL.has(pin.role)) {
                throw new UserError(`${d.id} ${p.name} pin ${pin.designator} (${pin.name}) is ${pin.role}; ` +
                    'only input, output and inout pins take "activeLow"');
            }
            let flag;
            if (value === 'off') flag = undefined;
            else if (value === 'true') flag = true;
            else {
                const listed = value.split(',');
                const alts = alternatives(pin.name);
                for (const a of listed) {
                    if (!alts.includes(a)) throw new UserError(`"${pin.name}" has no alternative "${a}"`);
                }
                flag = canonical(pin.name, new Set(listed));
            }
            edits.push({ node: pin.node, flag, label: `${p.name} ${pin.designator} ${pin.name}` });
        }
    }
    if (edits.length === 0) throw new UserError(`no pin ${name} in ${d.id} ${which}`);
    applyEdits(lib.kinds[d.kind], edits);
    for (const e of edits) out.push(`${d.id} ${e.label} -> ${e.flag === undefined ? 'off' : JSON.stringify(e.flag)}`);
    return 0;
}

function cmdRecord(lib, args, out) {
    const o = options(args, ['--source', '--pages', '--note']);
    const [id, pinoutName] = o._;
    if (!pinoutName || !o.source || !o.pages) {
        throw new UserError('usage: record <device> <pinout> --source S --pages P [--note N]');
    }
    const d = lib.device(id);
    const p = lib.pinout(d, pinoutName);
    if (!lib.resolves(d, o.source)) throw new UserError(`source "${o.source}" does not resolve (R1)`);
    for (const v of [o.source, o.pages, o.note || '']) {
        if (/[\t\n]/.test(v)) throw new UserError('a ledger field may not hold a tab or newline');
    }
    if (o.pages.trim() === '') throw new UserError('--pages is empty');
    const rows = lib.ledger().filter((r) => !(r.device === d.id && r.pinout === p.name));
    const row = { kind: d.kind, device: d.id, pinout: p.name, source: o.source, pages: o.pages,
        flagged: flaggedOf(p), note: o.note || '' };
    rows.push(row);
    lib.writeLedger(rows);
    out.push(LEDGER_HEADER.map((h) => row[h]).join('\t'));
    return 0;
}

// Problems with one pin's flag as written (R3), or [].
function flagProblems(pin) {
    if (!pin.hasFlag) return [];
    if (!SIGNAL.has(pin.role)) return [`a flag on a ${pin.role} pin`];
    const alts = alternatives(pin.name);
    if (pin.flag === true) return [];
    if (!Array.isArray(pin.flag)) return [`"activeLow" is ${JSON.stringify(pin.flag)}`];
    if (pin.flag.length === 0) return ['"activeLow" is []'];
    const bad = [];
    for (const s of pin.flag) {
        if (typeof s !== 'string' || !alts.includes(s)) bad.push(`lists ${JSON.stringify(s)}, not an alternative`);
    }
    if (new Set(pin.flag).size !== pin.flag.length) bad.push('repeats a string');
    if (bad.length === 0 && alts.every((a) => pin.flag.includes(a))) bad.push('lists every alternative (write true)');
    return bad;
}

function cmdCheck(lib, args, out) {
    const o = options(args, ['--kind', '--devices'], ['--complete']);
    const scoped = Boolean(o.kind || o.devices);
    const devices = lib.scope(o.kind, o.devices ? parseRange(o.devices) : null);
    const inScope = new Set(devices.map((d) => d.id));
    const problems = [];
    const say = (rule, text) => problems.push(`${rule} ${text}`);
    const rows = lib.ledger();
    const byId = new Map(lib.allDevices().map((d) => [d.id, d]));
    const seen = new Set();

    // R1, R2
    for (const r of rows) {
        const d = byId.get(r.device);
        if (scoped && !inScope.has(r.device)) continue;
        const key = `${r.device}\t${r.pinout}`;
        const p = d && d.kind === r.kind ? d.pinouts.find((x) => x.name === r.pinout) : undefined;
        if (!p) { say('R1', `ledger row ${r.kind} ${r.device} ${r.pinout} names no in-scope pinout`); continue; }
        if (seen.has(key)) say('R1', `ledger row ${r.device} ${r.pinout} is repeated`);
        seen.add(key);
        if (r.pages.trim() === '') say('R1', `ledger row ${r.device} ${r.pinout} has empty pages`);
        if (!lib.resolves(d, r.source)) say('R1', `ledger row ${r.device} ${r.pinout}: source "${r.source}" does not resolve`);
        let rowFlags;
        try { rowFlags = JSON.stringify(JSON.parse(r.flagged)); } catch (_) { rowFlags = r.flagged; }
        if (rowFlags !== flaggedOf(p)) {
            say('R2', `${r.device} ${r.pinout}: ledger says ${r.flagged}, the part file has ${flaggedOf(p)}`);
        }
    }

    for (const d of devices) {
        const reviewed = new Set(rows.filter((r) => r.device === d.id).map((r) => r.pinout));
        const seenFlags = new Map();
        for (const p of d.pinouts) {
            for (const pin of p.pins) {
                const where = `${d.id} ${p.name} pin ${pin.designator} (${pin.name})`;
                for (const why of flagProblems(pin)) say('R3', `${where}: ${why}`);
                // R4
                const bits = maskOf(pin.name, pin.flag);
                alternatives(pin.name).forEach((a, k) => {
                    if (a.endsWith('#') && !bits[k]) say('R4', `${where}: "${a}" ends in # and is not flagged`);
                });
                // R5
                if (reviewed.has(p.name)) {
                    const key = `${pin.name}\t${pin.role}`;
                    const val = JSON.stringify(canonical(pin.name, new Set(alternatives(pin.name).filter((_, k) => bits[k]))) ?? null);
                    if (seenFlags.has(key) && seenFlags.get(key).val !== val) {
                        say('R5', `${where} is flagged ${val}, but ${seenFlags.get(key).where} is ${seenFlags.get(key).val}`);
                    } else if (!seenFlags.has(key)) {
                        seenFlags.set(key, { val, where });
                    }
                }
            }
            // R7
            if ((!scoped || o.complete) && !reviewed.has(p.name)) say('R7', `${d.kind} ${d.id} ${p.name} has no ledger row`);
        }
        // R6
        if (d.kind === 'sram' || d.kind === 'eeprom') {
            const names = new Set();
            for (const key of ['chipEnable', 'outputEnable', 'writeEnable', 'byteEnable']) {
                const v = d.json[key];
                for (const n of v === undefined ? [] : [].concat(v)) names.add(n);
            }
            for (const p of d.pinouts) {
                for (const pin of p.pins) {
                    if (names.has(pin.name) && !pin.hasFlag) {
                        say('R6', `${d.id} ${p.name} pin ${pin.designator} (${pin.name}) is a control pin and not flagged`);
                    }
                }
            }
        }
    }

    // R8
    if (!scoped) {
        for (const f of fs.readdirSync(lib.partsDir).sort()) {
            const kind = f.replace(/\.json$/, '');
            if (!f.endsWith('.json') || NOT_KINDS.has(kind) || IN_SCOPE.includes(kind)) continue;
            const n = (fs.readFileSync(path.join(lib.partsDir, f), 'utf8').match(/"activeLow"/g) || []).length;
            if (n > 0) say('R8', `${f} (out of scope) has ${n} "activeLow" key(s)`);
        }
    }

    for (const p of problems) out.push(p);
    const checked = devices.reduce((n, d) => n + d.pinouts.length, 0);
    out.push(problems.length === 0 ? `ok: ${checked} pinouts, ${seen.size} ledger rows checked`
        : `${problems.length} problem(s)`);
    return problems.length === 0 ? 0 : 1;
}

function cmdStatus(lib, args, out) {
    const rows = lib.ledger();
    let done = 0, total = 0;
    for (const k of IN_SCOPE) {
        if (!lib.kinds[k]) continue;
        const t = lib.kinds[k].devices.reduce((n, d) => n + d.pinouts.length, 0);
        const r = rows.filter((x) => x.kind === k).length;
        done += r; total += t;
        out.push(`${k}\t${r}/${t}`);
    }
    out.push(`total\t${done}/${total}`);
    return 0;
}

function cmdSeedHash(lib, args, out) {
    let n = 0;
    for (const k of IN_SCOPE) {
        if (!lib.kinds[k]) continue;
        const edits = [];
        for (const d of lib.kinds[k].devices) {
            for (const p of d.pinouts) {
                for (const pin of p.pins) {
                    if (!SIGNAL.has(pin.role)) continue;
                    const alts = alternatives(pin.name);
                    if (!alts.some((a) => a.endsWith('#'))) continue;
                    const bits = maskOf(pin.name, pin.flag);
                    const set = new Set(alts.filter((a, i) => bits[i] || a.endsWith('#')));
                    const flag = canonical(pin.name, set);
                    if (JSON.stringify(flag) === JSON.stringify(pin.hasFlag ? pin.flag : undefined)) continue;
                    edits.push({ node: pin.node, flag });
                }
            }
        }
        if (edits.length > 0) {
            applyEdits(lib.kinds[k], edits);
            out.push(`${k}: ${edits.length} pin(s)`);
            n += edits.length;
        }
    }
    out.push(`seeded ${n} pin(s)`);
    return 0;
}

function cmdGoldenCheck(lib, args, out) {
    const o = options(args, ['--since']);
    const rev = o.since ? [o.since] : [];
    const git = (a) => childProcess.spawnSync('git', ['-C', lib.root, ...a], { encoding: 'utf8', maxBuffer: 1 << 30 });
    const diff = git(['diff', '-U0', ...rev, '--', 'backend/tests/golden/parts.txt']);
    if (diff.status !== 0) throw new UserError(`git diff failed: ${diff.stderr.trim()}`);
    const removed = [], added = [];
    for (const line of diff.stdout.split('\n')) {
        if (line.startsWith('---') || line.startsWith('+++')) continue;
        if (line.startsWith('-')) removed.push(line.slice(1));
        else if (line.startsWith('+')) added.push(line.slice(1));
    }
    const strip = (l) => l.replace(/:low(=[^,|]*)?(?=[,|]|$)/g, '');
    let bad = 0;
    if (removed.length !== added.length) {
        out.push(`parts.txt: ${removed.length} line(s) removed, ${added.length} added`);
        bad++;
    } else {
        for (let k = 0; k < removed.length; k++) {
            if (strip(added[k]) !== strip(removed[k])) {
                out.push(`parts.txt: a change other than :low:\n- ${removed[k].slice(0, 200)}\n+ ${added[k].slice(0, 200)}`);
                bad++;
                if (bad > 10) break;
            }
        }
    }
    for (const f of ['traces.txt', 'footprints.txt']) {
        const r = git(['diff', '--quiet', ...rev, '--', `backend/tests/golden/${f}`]);
        if (r.status !== 0) { out.push(`${f} changed`); bad++; }
    }
    out.push(bad === 0 ? `ok: ${added.length} parts.txt line(s) changed, only by :low` : 'golden-check failed');
    return bad === 0 ? 0 : 1;
}

const COMMANDS = {
    list: cmdList, sources: cmdSources, fetch: cmdFetch, pins: cmdPins, set: cmdSet, record: cmdRecord,
    check: cmdCheck, status: cmdStatus, 'seed-hash': cmdSeedHash, 'golden-check': cmdGoldenCheck,
};
const WRITES = new Set(['set', 'record', 'seed-hash']);

function main(argv) {
    let root = path.join(__dirname, '..');
    const args = argv.slice();
    const r = args.indexOf('--root');
    if (r >= 0) { root = args[r + 1]; args.splice(r, 2); }
    if (args.length === 0 || args[0] === '--help' || args[0] === '-h' || args[0] === 'help') {
        console.log(HELP);
        return 0;
    }
    const fn = COMMANDS[args[0]];
    if (!fn) { console.error(`unknown command "${args[0]}" (--help)`); return 1; }
    const out = [];
    try {
        const run = () => fn(new Library(root), args.slice(1), out);
        const status = WRITES.has(args[0]) ? withLock(root, run) : run();
        if (out.length) console.log(out.join('\n'));
        return status;
    } catch (e) {
        if (out.length) console.log(out.join('\n'));
        if (e instanceof UserError) { console.error(e.message); return 1; }
        throw e;
    }
}

module.exports = { parsePositioned, editPin, canonical };

if (require.main === module) process.exitCode = main(process.argv.slice(2));
