#!/usr/bin/env python3
"""Check plan-24 spec files (planning/plan-24-data/specs/*.json) against schema.md.

usage: validate.py [spec.json ...]   (no arguments: every spec file)
Prints one line per problem; exit status 1 when any.
"""
import json, os, re, sys, glob

HERE = os.path.dirname(os.path.abspath(__file__))
DATA = os.path.dirname(HERE)
ROOT = os.path.dirname(os.path.dirname(DATA))
FOOTPRINTS = os.path.join(ROOT, 'schematical/schematical/Parts/footprints.json')
FP_IDS_TSV = os.path.join(DATA, 'footprint-ids.tsv')

ROLES = {'input', 'output', 'inout', 'power', 'ground', 'nc', 'absent'}
ACTIONS = {'nop', 'wreg', 'rreg', 'write', 'read', 'pointer', 'start', 'stop', 'reset',
           'powerdown', 'wakeup', 'update', 'clear', 'setbits', 'clearbits'}
STARTS = {'cs-fall', 'cs-rise', 'command', 'pin', 'read', 'power-on'}
UNITS = {'second', 'minute', 'hour', 'weekday', 'date', 'month', 'year', 'century', 'hundredth'}
NAME_RE = re.compile(r'^[A-Z][A-Z0-9_]*$')


def footprint_ids():
    ids = set()
    try:
        ids |= {f['id'] for f in json.load(open(FOOTPRINTS))['footprints']}
    except Exception:
        pass
    if os.path.exists(FP_IDS_TSV):
        for l in open(FP_IDS_TSV):
            p = l.rstrip('\n').split('\t')
            if len(p) > 1 and not l.startswith('#'):
                ids.add(p[1])
    return ids


class Ctx:
    def __init__(self, path):
        self.path = path
        self.errors = []

    def err(self, where, msg):
        self.errors.append(f'{os.path.basename(self.path)}: {where}: {msg}')


def tokenize(pat, ctx, where):
    """-> list of tokens: ('bit', '0'|'1'|'x'|'z'), ('cap', letter), ('R',),
    ('ref', name, hi, lo), ('@',)."""
    out = []
    s, depth = '', 0
    for ch in pat:
        if ch == '[':
            depth += 1
        elif ch == ']':
            depth -= 1
        if ch == ' ' or (ch == '_' and depth == 0):
            continue
        s += ch
    i = 0
    while i < len(s):
        c = s[i]
        if c == '[':
            j = s.find(']', i)
            if j < 0:
                ctx.err(where, f'unclosed [ in {pat!r}')
                return out
            body = s[i + 1:j].split(':')
            if len(body) == 1:
                tok = ('ref', body[0], None, None)
            elif len(body) == 3 and body[1].isdigit() and body[2].isdigit():
                tok = ('ref', body[0], int(body[1]), int(body[2]))
            else:
                ctx.err(where, f'bad [{s[i+1:j]}] in {pat!r}')
                return out
            i = j + 1
        elif c in '01xz':
            tok = ('bit', c); i += 1
        elif c == '@':
            tok = ('@',); i += 1
        elif c == 'R':
            tok = ('R',); i += 1
        elif c.islower():
            tok = ('cap', c); i += 1
        else:
            ctx.err(where, f'bad character {c!r} in {pat!r}')
            return out
        n = 1
        if i < len(s) and s[i] == '{':
            j = s.find('}', i)
            if j < 0 or not s[i + 1:j].isdigit():
                ctx.err(where, f'bad repeat in {pat!r}')
                return out
            n = int(s[i + 1:j]); i = j + 1
        out.extend([tok] * n)
    return out


class Dev:
    def __init__(self, d):
        self.d = d
        self.regs = {}
        self.fields = {}


def ref_width(dev, name, ctx, where):
    if name in ('R',):
        return dev.d.get('bits', 0)
    if name == 'CH':
        return 8
    if '.' in name:
        r, f = name.split('.', 1)
        if r not in dev.regs:
            ctx.err(where, f'unknown register {r!r}'); return 0
        fl = {x['name']: x for x in dev.regs[r].get('fields', [])}
        if f not in fl:
            ctx.err(where, f'unknown field {name!r}'); return 0
        hi, lo = fl[f]['bits']
        return hi - lo + 1
    if name not in dev.regs:
        ctx.err(where, f'unknown register {name!r}'); return 0
    return dev.regs[name].get('width', 0)


def check_pattern(dev, pat, kind, ctx, where):
    """kind 'in' or 'out'. Returns (length, captures set)."""
    if not isinstance(pat, str):
        ctx.err(where, 'pattern must be a string'); return 0, set()
    toks = tokenize(pat, ctx, where)
    caps = set(); n = 0
    for t in toks:
        if t[0] == 'bit':
            if kind == 'in' and t[1] == 'z':
                ctx.err(where, f'"z" in an input pattern {pat!r}')
            if kind == 'out' and t[1] == 'x':
                ctx.err(where, f'"x" in an output pattern {pat!r}')
            n += 1
        elif t[0] == 'cap':
            caps.add(t[1]); n += 1
        elif t[0] == 'R':
            if kind == 'in':
                ctx.err(where, f'"R" in an input pattern {pat!r}')
            n += 1
        elif t[0] == '@':
            if kind == 'in':
                ctx.err(where, f'"@" in an input pattern')
        elif t[0] == 'ref':
            w = ref_width(dev, t[1], ctx, where)
            if t[2] is not None:
                if not (w > t[2] >= t[3] >= 0):
                    ctx.err(where, f'[{t[1]}:{t[2]}:{t[3]}] outside width {w}')
                w = t[2] - t[3] + 1
            n += w
    run = best = 0
    for t in toks:
        run = run + 1 if t[0] == 'R' else 0
        best = max(best, run)
    if best > dev.d.get('bits', 0) and dev.d.get('type') == 'adc':
        ctx.err(where, f'a run of {best} R bits > bits {dev.d.get("bits")} (use [R][R] for two results)')
    return n, caps


def check_field_value(dev, spec, ctx, where):
    m = re.match(r'^([A-Z0-9_.]+)(!?=)(\d+)$', spec)
    if not m:
        ctx.err(where, f'bad condition {spec!r}'); return
    ref_width(dev, m.group(1), ctx, where)


def check_converter(dev, ctx):
    d = dev.d
    t = d.get('type')
    if t not in ('adc', 'dac'):
        ctx.err('type', f'must be "adc" or "dac", not {t!r}')
    if not isinstance(d.get('bits'), int) or not 1 <= d['bits'] <= 32:
        ctx.err('bits', 'must be 1-32')
    if not isinstance(d.get('channels'), int) or d['channels'] < 1:
        ctx.err('channels', 'must be >= 1')
    cf = d.get('codeFormat')
    if isinstance(cf, dict):
        if not cf.get('values') or any(v not in ('straight', 'twos', 'offset') for v in cf['values'].values()):
            ctx.err('codeFormat', 'values must be straight/twos/offset')
        if 'field' in cf:
            ref_width(dev, cf['field'], ctx, 'codeFormat')
        elif 'pin' not in cf:
            ctx.err('codeFormat', 'needs pin or field')
    elif t == 'adc' and cf not in ('straight', 'twos', 'offset'):
        ctx.err('codeFormat', 'ADC needs straight/twos/offset')
    if t == 'adc' and 'conversion' not in d and d.get('bus', {}).get('style') != 'clocked':
        ctx.err('conversion', 'ADC needs "conversion"')
    if t == 'dac' and 'dac' not in d:
        ctx.err('dac', 'DAC needs "dac"')
    if t == 'dac':
        dc = d.get('dac', {})
        ins, outs = dc.get('input', []), dc.get('output', [])
        if len(ins) != len(outs) or len(ins) != d.get('channels'):
            ctx.err('dac', f'input/output need one register per channel ({d.get("channels")})')
        for r in ins + outs:
            if r not in dev.regs:
                ctx.err('dac', f'unknown register {r!r}')
        if dc.get('ldac') not in ('pin', 'command', 'sync'):
            ctx.err('dac.ldac', 'pin/command/sync')
        if dc.get('ldac') == 'pin' and 'ldac' not in d.get('pins', {}):
            ctx.err('dac.ldac', '"pin" needs pins.ldac')
    conv = d.get('conversion')
    if conv is not None:
        tm = conv.get('time')
        if isinstance(tm, dict):
            for f in (tm['fields'] if 'fields' in tm else [tm.get('field', '')]):
                ref_width(dev, f, ctx, 'conversion.time')
            if not tm.get('values'):
                ctx.err('conversion.time', 'needs values')
        elif not isinstance(tm, (int, float)) or tm < 0:
            ctx.err('conversion.time', 'ns or {field, values}')
        st = conv.get('start')
        if not isinstance(st, list) or not st:
            ctx.err('conversion.start', 'non-empty array')
        else:
            for s in st:
                if s.startswith('write:') and '=' not in s:
                    ref_width(dev, s[6:], ctx, 'conversion.start')
                elif s.startswith('write:'):
                    check_field_value(dev, s[6:], ctx, 'conversion.start')
                elif s not in STARTS:
                    ctx.err('conversion.start', f'unknown trigger {s!r}')
                if s == 'pin' and not ({'convst', 'start'} & set(d.get('pins', {}))):
                    ctx.err('conversion.start', '"pin" needs pins.convst or pins.start')
        mode = conv.get('mode', 'single')
        if isinstance(mode, dict):
            ref_width(dev, mode.get('field', ''), ctx, 'conversion.mode')
            if ('continuous' in mode) == ('single' in mode):
                ctx.err('conversion.mode', 'needs "continuous" or "single"')
            if set(mode) - {'field', 'continuous', 'single', 'startOnEnter'}:
                ctx.err('conversion.mode', 'unknown keys')
        elif mode not in ('single', 'continuous'):
            ctx.err('conversion.mode', 'single/continuous/{field}')
        for k in ('busy', 'done', 'onRead'):
            for f in conv.get(k, {}):
                ref_width(dev, f, ctx, f'conversion.{k}')
        if 'channel' in conv:
            ref_width(dev, conv['channel'], ctx, 'conversion.channel')
        extra = set(conv) - {'time', 'start', 'mode', 'busy', 'done', 'onRead', 'channel', 'note'}
        if extra:
            ctx.err('conversion', f'unknown keys {sorted(extra)}')


def check_rtc(dev, ctx):
    c = dev.d.get('clock')
    if not isinstance(c, dict):
        ctx.err('clock', 'rtc needs "clock"'); return
    units = set()
    for i, f in enumerate(c.get('fields', [])):
        ref_width(dev, f.get('field', ''), ctx, f'clock.fields[{i}]')
        if f.get('unit') not in UNITS:
            ctx.err(f'clock.fields[{i}]', f'unit {f.get("unit")!r}')
        units.add(f.get('unit'))
        for k in ('h12', 'pm'):
            if k in f:
                ref_width(dev, f[k], ctx, f'clock.fields[{i}].{k}')
    for u in ('second', 'minute', 'hour'):
        if u not in units:
            ctx.err('clock.fields', f'no {u} field')
    if 'stop' in c:
        ref_width(dev, c['stop'].get('field', ''), ctx, 'clock.stop')
    for i, a in enumerate(c.get('alarms', [])):
        for m in a.get('match', []):
            ref_width(dev, m.get('field', ''), ctx, f'clock.alarms[{i}]')
            if 'mask' in m:
                ref_width(dev, m['mask'], ctx, f'clock.alarms[{i}].mask')
        for k in ('flag', 'enable'):
            if k in a:
                ref_width(dev, a[k], ctx, f'clock.alarms[{i}].{k}')


def check_protocol(dev, ctx):
    p = dev.d.get('protocol')
    if not isinstance(p, dict):
        ctx.err('protocol', 'missing'); return
    extra = set(p) - {'frame', 'out', 'idle', 'commands', 'read', 'generalCall', 'align', 'outs', 'reads', 'note'}
    for k in ('outs', 'reads'):
        for j, o in enumerate(p.get(k, [])):
            w = o.get('when', [])
            for e in ([w] if isinstance(w, str) else w):
                check_field_value(dev, e, ctx, f'protocol.{k}[{j}].when')
            check_pattern(dev, o.get('out', o.get('read', '')), 'out', ctx, f'protocol.{k}[{j}]')
    if p.get('align', 'start') not in ('start', 'end'):
        ctx.err('protocol.align', 'start/end')
    if extra:
        ctx.err('protocol', f'unknown keys {sorted(extra)}')
    if p.get('frame') not in ('fixed', 'stream'):
        ctx.err('protocol.frame', 'fixed or stream')
    if 'out' in p:
        check_pattern(dev, p['out'], 'out', ctx, 'protocol.out')
    if p.get('idle', '0') not in ('0', '1', 'z', 'repeat'):
        ctx.err('protocol.idle', '0/1/z/repeat')
    rd = p.get('read')
    if isinstance(rd, str) and rd != 'pointer':
        check_pattern(dev, rd, 'out', ctx, 'protocol.read')
    elif isinstance(rd, dict):
        for k, v in rd.items():
            check_pattern(dev, v, 'out', ctx, f'protocol.read[{k}]')
    style = dev.d.get('bus', {}).get('style')
    if style in ('i2c', 'parallel', 'clocked') and rd is None and dev.d.get('type') != 'dac':
        ctx.err('protocol.read', f'{style} needs "read"')
    for i, c in enumerate(p.get('commands', [])):
        w = f'commands[{i}] ({c.get("name")})'
        extra = set(c) - {'name', 'in', 'action', 'address', 'space', 'register', 'count',
                          'countOffset', 'data', 'channel', 'channelMap', 'set', 'reply',
                          'next', 'commit', 'then', 'wordBits', 'when', 'increment', 'note'}
        wh = c.get('when', [])
        for e in ([wh] if isinstance(wh, str) else wh):
            check_field_value(dev, e, ctx, w + '.when')
        if extra:
            ctx.err(w, f'unknown keys {sorted(extra)}')
        n, caps = check_pattern(dev, c.get('in', ''), 'in', ctx, w + '.in')
        if isinstance(c.get('increment'), str) and c['increment'] not in caps:
            ctx.err(w, f'increment {c["increment"]!r} is not a capture of "in"')
        if n == 0:
            ctx.err(w, 'empty "in"')
        a = c.get('action', 'nop')
        if a not in ACTIONS:
            ctx.err(w, f'unknown action {a!r}')
        for x in c.get('then', []):
            if x not in ACTIONS:
                ctx.err(w, f'unknown then action {x!r}')
        for k in ('address', 'count', 'data', 'channel'):
            v = c.get(k)
            if isinstance(v, str) and v not in caps:
                ctx.err(w, f'{k} {v!r} is not a capture of "in"')
        for f, v in c.get('set', {}).items():
            ref_width(dev, f, ctx, w + '.set')
            if isinstance(v, str) and v not in caps:
                ctx.err(w, f'set value {v!r} is not a capture')
        if a in ('wreg', 'rreg', 'read', 'pointer') and 'address' not in c and 'register' not in c:
            ctx.err(w, f'{a} needs address or register')
        if a == 'write' and 'data' not in c and dev.d.get('type') != 'dac':
            ctx.err(w, 'write needs data')
        if 'register' in c and c['register'] not in dev.regs:
            ctx.err(w, f'unknown register {c["register"]!r}')
        space = c.get('space', 'reg')
        if isinstance(c.get('address'), int):
            if not any(r.get('space', 'reg') == space and r.get('address') == c['address'] for r in dev.regs.values()):
                ctx.err(w, f'no register at address {c["address"]} in space {space}')
        for k in ('reply', 'next'):
            if k in c:
                check_pattern(dev, c[k], 'out', ctx, w + '.' + k)
        if c.get('commit', 'now') not in ('now', 'end'):
            ctx.err(w, 'commit now/end')


def check_bus(b, bk, need_pin, ctx):
    st = b.get('style')
    if st == 'spi':
        if 'cs' in b:
            need_pin(b['cs'], bk + '.cs')
        if 'miso' in b:
            need_pin(b['miso'], bk + '.miso')
        for k in ('sclk',):
            if k not in b:
                ctx.err(bk + '', f'spi needs {k!r}')
            else:
                need_pin(b[k], bk + '.' + k)
        if 'mosi' in b:
            need_pin(b['mosi'], bk + '.mosi')
        if b.get('mode', 0) not in (0, 1, 2, 3):
            ctx.err(bk + '.mode', '0-3')
    elif st == 'i2c':
        need_pin(b.get('scl', ''), bk + '.scl'); need_pin(b.get('sda', ''), bk + '.sda')
        for a in b.get('address', []):
            need_pin(a, bk + '.address', strap=True)
        if len(b.get('table', [])) != 4 ** len(b.get('address', [])):
            ctx.err(bk + '.table', f'needs {4 ** len(b.get("address", []))} entries')
    elif st == 'i2c-spi':
        for k in ('spi', 'i2c', 'selectPin'):
            if k not in b:
                ctx.err(bk + '', f'i2c-spi needs {k!r}')
    elif st == 'parallel':
        data = b.get('data', [])
        if not 4 <= len(data) <= 24:
            ctx.err(bk + '.data', '4-24 pins')
        for p_ in data + b.get('address', []):
            need_pin(p_, bk + '.data/address')
        for k in ('cs', 'rd', 'wr', 'byteSelect'):
            if k in b:
                need_pin(b[k].get('pin', ''), bk + '.' + k)
        if 'rw' in b:
            need_pin(b['rw'].get('pin', ''), bk + '.rw')
    elif st == 'clocked':
        need_pin(b.get('clock', {}).get('pin', ''), bk + '.clock')
        data = b.get('data', [])
        if not 1 <= len(data) <= 24:
            ctx.err(bk + '.data', '1-24 pins')
        for p_ in data:
            need_pin(p_, bk + '.data')
        if 'oe' in b:
            need_pin(b['oe'].get('pin', ''), bk + '.oe')
        co = b.get('clockOut', [])
        for c in (co if isinstance(co, list) else [co]):
            need_pin(c.get('pin', ''), bk + '.clockOut')
        if b.get('clock', {}).get('edge', 'rise') not in ('rise', 'fall', 'both'):
            ctx.err(bk + '.clock', 'edge rise/fall/both')
    else:
        ctx.err(bk + '.style', f'{st!r}')


def check_device(spec, ctx, fps):
    d = spec.get('device')
    if not isinstance(d, dict):
        ctx.err('device', 'missing'); return
    dev = Dev(d)
    kind = 'rtc' if d.get('function') == 'RTC' else 'converter'
    for k in ('id', 'function', 'description', 'drawer', 'pinouts', 'bus', 'protocol', 'vcc'):
        if k not in d:
            ctx.err('device', f'missing {k!r}')
    if 'parts' in d:
        ctx.err('device', '"parts" belongs to the generator (parts.tsv), not the spec')
    allowed = {'id', 'function', 'description', 'drawer', 'note', 'pinouts', 'type', 'bits',
               'channels', 'codeFormat', 'vcc', 'inputMax', 'analog', 'bus', 'configBus', 'timing',
               'registers', 'protocol', 'conversion', 'ready', 'pins', 'dac', 'clock'}
    extra = set(d) - allowed
    if extra:
        ctx.err('device', f'unknown keys {sorted(extra)}')
    if d.get('function') not in ('ADC', 'DAC', 'RTC'):
        ctx.err('function', 'ADC/DAC/RTC')
    if d.get('drawer') != ['Interface']:
        ctx.err('drawer', 'must be ["Interface"]')
    v = d.get('vcc')
    if not (isinstance(v, list) and len(v) == 2 and 0 < v[0] < v[1] <= 30):
        ctx.err('vcc', '[min, max] volts')
    if 'inputMax' in d and isinstance(v, list) and len(v) == 2 and d['inputMax'] < v[1]:
        ctx.err('inputMax', 'below vcc max')
    # pinouts
    names_by_pinout = {}
    for pn, pins in d.get('pinouts', {}).items():
        if not isinstance(pins, list) or not pins:
            ctx.err(f'pinouts.{pn}', 'empty'); continue
        numbered = 'number' in pins[0]
        names = []
        for i, pin in enumerate(pins):
            w = f'pinouts.{pn}[{i}]'
            if numbered:
                if pin.get('number') != i + 1:
                    ctx.err(w, f'number must be {i+1}')
            elif 'ball' not in pin:
                ctx.err(w, 'needs "ball"')
            if pin.get('role') not in ROLES:
                ctx.err(w, f'role {pin.get("role")!r}')
            if not pin.get('name'):
                ctx.err(w, 'no name')
            names.append(pin.get('name'))
        names_by_pinout[pn] = names
    analog = set(d.get('analog', []))
    all_names = None
    for pn, names in names_by_pinout.items():
        s = set(names)
        for a in analog:
            if a not in s:
                ctx.err(f'pinouts.{pn}', f'analog pin {a!r} missing')
        digital_power = [p for p in d['pinouts'][pn] if p.get('role') == 'power' and p.get('name') not in analog]
        if not digital_power:
            ctx.err(f'pinouts.{pn}', 'no digital power pin (every power pin is in "analog")')
        all_names = s if all_names is None else all_names & s
    all_names = all_names or set()

    fixed = {v.get('pinout') for v in spec.get('packages', {}).values() if 'i2cAddress' in v}

    def need_pin(name, where, strap=False):
        for pn, names in names_by_pinout.items():
            if name not in names and not (strap and pn in fixed):
                ctx.err(where, f'pin {name!r} is not in pinout {pn!r}')
                return

    # registers
    for i, r in enumerate(d.get('registers', [])):
        w = f'registers[{i}] ({r.get("name")})'
        if not NAME_RE.match(str(r.get('name', ''))):
            ctx.err(w, 'bad name')
        if r.get('name') in dev.regs:
            ctx.err(w, 'duplicate name')
        dev.regs[r.get('name')] = r
        wd = r.get('width')
        if not isinstance(wd, int) or not 1 <= wd <= 32:
            ctx.err(w, 'width 1-32'); continue
        if not isinstance(r.get('reset', 0), int) or not 0 <= r.get('reset', 0) < (1 << wd):
            ctx.err(w, 'reset outside width')
        ro = r.get('readOnly', 0)
        if not (ro is True or (isinstance(ro, int) and 0 <= ro < (1 << wd))):
            ctx.err(w, 'readOnly mask outside width')
        used = 0
        seen = set()
        for f in r.get('fields', []):
            hi, lo = f.get('bits', [None, None])
            if not (isinstance(hi, int) and isinstance(lo, int) and wd > hi >= lo >= 0):
                ctx.err(w, f'field {f.get("name")} bits {f.get("bits")} outside width {wd}'); continue
            m = ((1 << (hi + 1)) - 1) ^ ((1 << lo) - 1)
            if used & m:
                ctx.err(w, f'field {f.get("name")} overlaps')
            used |= m
            if f.get('name') in seen or not NAME_RE.match(str(f.get('name', ''))):
                ctx.err(w, f'bad or duplicate field name {f.get("name")!r}')
            seen.add(f.get('name'))
        extra = set(r) - {'name', 'address', 'space', 'width', 'reset', 'readOnly', 'fields',
                          'result', 'resultBits', 'channel', 'clearOnRead', 'writeOneClears', 'note'}
        if isinstance(r.get('result'), str) and r['result'] not in seen:
            ctx.err(w, f'result field {r["result"]!r} is not a field')
        if extra:
            ctx.err(w, f'unknown keys {sorted(extra)}')
    addrs = {}
    for r in d.get('registers', []):
        if 'address' in r:
            k = (r.get('space', 'reg'), r['address'])
            if k in addrs:
                ctx.err('registers', f'{r["name"]} and {addrs[k]} share address {k}')
            addrs[k] = r['name']
    # bus
    for bk in ('bus', 'configBus'):
        if bk in d:
            check_bus(d[bk], bk, need_pin, ctx)
    if d.get('configBus', {}).get('style') not in (None, 'spi', 'i2c'):
        ctx.err('configBus', 'spi or i2c')
    for i, r in enumerate(d.get('ready', [])):
        need_pin(r.get('pin', ''), f'ready[{i}]')
        if r.get('asserts') not in ('busy', 'done'):
            ctx.err(f'ready[{i}]', 'asserts busy/done')
        cl = r.get('clears', 'start')
        for c in (cl if isinstance(cl, list) else [cl]):
            if not (isinstance(c, (int, float)) or c in ('read', 'start', 'cs-fall')):
                ctx.err(f'ready[{i}]', f'clears {c!r}')
        en = r.get('enable', [])
        for e in ([en] if isinstance(en, str) else en):
            check_field_value(dev, e, ctx, f'ready[{i}].enable')
    for k, v in d.get('pins', {}).items():
        if k == 'levels':
            for j, lv in enumerate(v):
                need_pin(lv.get('pin', ''), f'pins.levels[{j}]')
                ref_width(dev, lv.get('field', ''), ctx, f'pins.levels[{j}]')
            continue
        if k not in ('reset', 'convst', 'start', 'powerdown', 'ldac', 'clear'):
            ctx.err('pins', f'unknown control {k!r}')
        need_pin(v.get('pin', ''), f'pins.{k}')
    check_protocol(dev, ctx)
    if kind == 'rtc':
        check_rtc(dev, ctx)
    else:
        check_converter(dev, ctx)
    # packages
    pk = spec.get('packages')
    if not isinstance(pk, dict) or not pk:
        ctx.err('packages', 'missing')
    else:
        for code, v in pk.items():
            if v.get('pinout') not in d.get('pinouts', {}):
                ctx.err(f'packages.{code}', f'unknown pinout {v.get("pinout")!r}')
            if fps and v.get('footprint') not in fps:
                ctx.err(f'packages.{code}', f'footprint {v.get("footprint")!r} not in footprints.json or footprint-ids.tsv')
            if not v.get('package') or (re.search(r'^\d|-\d+$', v.get('package', '')) and v['package'] not in ('SOT-23', 'SC-70')):
                ctx.err(f'packages.{code}', f'package family {v.get("package")!r} (no pin counts)')
            pins = d.get('pinouts', {}).get(v.get('pinout'), [])
            if 'pins' in v and pins and len(pins) != v['pins']:
                ctx.err(f'packages.{code}', f'pinout {v.get("pinout")!r} has {len(pins)} pins, the package {v["pins"]}')
    if not spec.get('pdf'):
        ctx.err('pdf', 'missing')
    if not isinstance(spec.get('notModeled', []), list):
        ctx.err('notModeled', 'array')


def main(argv):
    files = argv[1:] or sorted(glob.glob(os.path.join(DATA, 'specs', '*.json')))
    fps = footprint_ids()
    bad = 0
    for f in files:
        ctx = Ctx(f)
        try:
            spec = json.load(open(f))
        except Exception as e:
            print(f'{os.path.basename(f)}: JSON: {e}'); bad += 1; continue
        check_device(spec, ctx, fps)
        for e in ctx.errors:
            print(e)
        bad += bool(ctx.errors)
    print(f'{len(files)} spec files, {bad} with problems', file=sys.stderr)
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
