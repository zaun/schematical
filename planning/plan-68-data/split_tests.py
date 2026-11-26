"""plan-68-H: split over-limit C test files into test programs on a shared bench.

For each file a list of cut points: (marker, owner). The text from a marker to
the next one belongs to `owner` (a program name, or 'bench'); text before the
first marker is the bench's; `main` is handled apart. Each program gets the
old main with the calls to functions it does not define (and the bench does
not) removed, so setup and teardown are unchanged.
"""
import json, re, sys
sys.path.insert(0, '/private/tmp/claude-501/p68')
from csplit import bench, items

T = 'backend/tests/'
SPECS = {
    'library_check': [
        (('fn', 'check_valid_pull'), 'library_check'),
        (('sec', 'plan-18-A: LED "colors"'), 'library_logic_check'),
        (('fn', 'check_supply_cases'), 'library_parts_check'),
        (('fn', 'check_sram_cases'), 'library_pins_check'),
        (('sec', 'Footprints (plan-03-A)'), 'library_footprint_check'),
        (('sec', 'Voltage keys (plan-07-A)'), 'library_voltage_check'),
    ],
    'cpu_test': [
        (('sec', '5.1 Both cores'), 'cpu_test'),
        (('sec', 'The Z80 bench'), 'cpu_z80_test'),
        (('sec', 'Whole-system boards'), 'cpu_test'),
        (('sec', 'plan-14-H: the W65C816S'), 'cpu_65816_test'),
        (('sec', 'The W65C02S I/O board'), 'cpu_io_board_test'),
    ],
    'counter_test': [
        (('sec', '74_161 / 163 / 169'), 'counter_test'),
        (('sec', '4522B'), 'counter_div_test'),
        (('sec', '---- Loader ----'), 'counter_test'),
        (('sec', 'plan-10-E: johnson'), 'counter_div_test'),
    ],
    'charlcd_test': [
        (('sec', 'Case 20: loader'), 'charlcd_test'),
        (('sec', 'Character ROM'), 'bench'),
        (('sec', 'Cases 1-13'), 'charlcd_test'),
        (('sec', 'plan-35-D cases 2-9'), 'charlcd_4row_test'),
        (('sec', 'Cases 17-18'), 'charlcd_4row_test'),
        (('sec', 'plan-35-E section 4.1'), 'charlcd_4row_test'),
        (('sec', 'plan-11-C section 4.6: the CPU board'), 'charlcd_board_test'),
    ],
    'shift_test': [
        (('sec', '164 / 264 / 165 / 166'), 'shift_test'),
        (('sec', 'plan-10-H: ls673 / ls674'), 'shift_special_test'),
        (('sec', 'Loader and voltage'), 'shift_test'),
        (('sec', 'plan-10-I: real parts'), 'shift_special_test'),
    ],
    'pio_test': [
        (('sec', 'A: bus65'), 'pio_test'),
        (('sec', 'C: via6522'), 'pio_via_test'),
    ],
    'circuit_check': [
        (('sec', 'plan-12-C: the serial terminal API'), 'circuit_check'),
        (('sec', 'plan-24-G: an ADC read by a CPU'), 'circuit_adc_check'),
    ],
    'serialmem_test': [
        (('sec', '4-Kbit EEPROM (E40)'), 'serialmem_test'),
        (('sec', '---- Flash ----'), 'serialmem_flash_test'),
        (('sec', 'Facts, listing, circuit'), 'serialmem_test'),
    ],
    'latch_test': [
        (('sec', '---- transparent ----'), 'latch_test'),
    ],
}


def doc_start(lines, k):
    """0-based index of the doc comment directly above line k (0-based)."""
    j = k
    while j > 0 and (lines[j - 1].startswith((' *', '/*', '//')) or lines[j - 1].strip() == '*/'):
        j -= 1
    return j


def marker_line(lines, m):
    kind, key = m
    for k, l in enumerate(lines):
        if kind == 'sec' and l.startswith('/* ') and key in l:
            return k
        if kind == 'fn' and re.match(r'^(static )?[^(]*\b' + re.escape(key) + r'\(', l) and not l.rstrip().endswith(';'):
            return doc_start(lines, k)
    raise SystemExit(f'marker {m} not found')


def defined(text):
    return set(re.findall(r'^(?:static )?[A-Za-z_][\w \*]*?\b(\w+)\([^;]*$', text, re.M))


def split(base):
    path = T + base + '.c'
    lines = open(path).read().split('\n')
    whole = '\n'.join(lines)
    main_k = next(k for k, l in enumerate(lines) if re.match(r'^int main\(', l))
    main_k = doc_start(lines, main_k)
    # a "---- main ----" section header belongs to main too
    if main_k > 0 and lines[main_k - 1].startswith('/* ---- main'):
        main_k -= 1
    cuts = sorted((marker_line(lines, m), owner) for m, owner in SPECS[base])
    owners = {}
    regions = [(0, 'bench')] + cuts + [(main_k, 'main')]
    for (a, owner), (b, _) in zip(regions, regions[1:]):
        owners.setdefault(owner, []).append('\n'.join(lines[a:b]) + '\n')
    main = '\n'.join(lines[main_k:])
    bench_text = ''.join(owners.pop('bench'))
    # Cases written in the bench's own region (a file's top section) run in
    # the first program of the split only.
    first_prog = SPECS[base][0][1]
    bench_cases = defined(bench_text)
    owners.pop('main', None)
    # A macro one program defines and another uses goes to the bench (a macro
    # defined and #undef'd around its own functions stays where it is).
    progs = {k: ''.join(v) for k, v in owners.items()}
    for prog in list(progs):
        for m in list(re.finditer(r'^#define (\w+)\b(?:[^\n]*\\\n)*[^\n]*\n', progs[prog], re.M)):
            name = m.group(1)
            if re.search(r'^#undef ' + name + r'\b', progs[prog], re.M):
                continue
            used_elsewhere = any(re.search(r'\b' + name + r'\b', t) and not re.search(r'^#define ' + name + r'\b', t, re.M)
                                 for other, t in progs.items() if other != prog) or re.search(r'\b' + name + r'\b', main)
            if used_elsewhere:
                bench_text += m.group(0)
                progs[prog] = progs[prog].replace(m.group(0), '', 1)
    # Functions, data and enums one program defines and another (or main, or
    # the bench) uses go to the bench, until nothing more needs to move.
    def names_of(code):
        head = code.split('{', 1)[0]
        if code.lstrip().startswith(('enum', 'typedef enum')):
            body = code[code.find('{') + 1:code.rfind('}')]
            return set(re.findall(r'\b([A-Z_][A-Z0-9_]*)\b\s*(?:=|,|$)', body, re.M))
        m = re.search(r'\b(\w+)\s*\(', head) if '(' in head and '=' not in head else re.search(r'\b(\w+)\s*(?:\[[^]]*\])*\s*(?:=|;)', code)
        return {m.group(1)} if m else set()

    def chunks(text):
        out, pending = [], ''
        for kind, t in items(text):
            if kind in ('blank', 'comment'):
                pending += t
                continue
            if kind == 'pp':
                m = re.match(r'\s*#\s*define\s+(\w+)', t)
                names = ({m.group(1)} if m and not re.search(r'^#undef ' + m.group(1) + r'\b', text, re.M)
                         and len(re.findall(r'^#\s*define\s+' + m.group(1) + r'\b', whole, re.M)) == 1 else set())
            else:
                names = names_of(t) if kind == 'code' else set()
            out.append((pending + t, names))
            pending = ''
        if pending:
            out.append((pending, set()))
        return out

    # main's lines other than plain calls: its setup and teardown, which every
    # program keeps.
    main_setup = '\n'.join(l for l in main.split('\n') if not re.match(r'^\s+\w+\(.*\);\s*$', l))
    origin = {}
    changed = True
    while changed:
        changed = False
        for prog in list(progs):
            for text, names in chunks(progs[prog]):
                if not names:
                    continue
                others = [t for o, t in progs.items() if o != prog] + [main_setup, bench_text]
                if any(re.search(r'\b' + re.escape(n) + r'\b', t) for n in names for t in others):
                    for n in names:
                        origin.setdefault(n, prog)
                    bench_text += text
                    progs[prog] = progs[prog].replace(text, '', 1)
                    changed = True
    owners = {k: [v] for k, v in progs.items()}
    bench_name = base.replace('_test', '').replace('_check', '') + '_bench'
    bench_name = {'library_check': 'library_check_bench', 'cpu_test': 'cpu_test_bench'}.get(base, bench_name)
    guard = bench_name.upper() + '_H'
    sized = set(re.findall(r'sizeof\s*\(?\s*(\w+)\s*\)?', ''.join(''.join(v) for v in owners.values()) + main + bench_text))
    c, h = bench(bench_text, guard, [bench_name + '.h'], sized)
    open(T + 'support/' + bench_name + '.c', 'w').write(f'/* {bench_name}: shared by the programs split from {base}.c (plan-68-H). */\n' + c)
    open(T + 'support/' + bench_name + '.h', 'w').write(h)
    bench_defs = defined(bench_text)
    all_defs = defined('\n'.join(lines[:main_k]))
    made = {}
    for prog, parts in owners.items():
        body = ''.join(parts)
        mine = defined(body)
        keep = []
        # whole statements: a call may run over several lines
        stmts, cur, depth = [], [], 0
        for l in main.split('\n'):
            if not cur and not re.match(r'^\s+\w+\(', l):
                stmts.append(l)
                continue
            cur.append(l)
            depth += l.count('(') - l.count(')')
            if depth <= 0 and l.rstrip().endswith(';'):
                stmts.append('\n'.join(cur))
                cur, depth = [], 0
        stmts += cur
        for l in stmts:
            m = re.match(r'^\s+(\w+)\((?:.|\n)*\);\s*$', l)
            if m and m.group(1) in all_defs and m.group(1) not in mine and m.group(1) not in bench_defs:
                continue
            if m and m.group(1) in origin and origin[m.group(1)] != prog:
                continue  # a case moved to the bench still runs only in its own program
            if m and m.group(1) in bench_cases and m.group(1) not in origin and prog != first_prog:
                continue
            keep.append(l)
        text = (f'/*\n * {prog}: split from {base}.c by plan-68-H; the shared helpers are\n'
                f' * support/{bench_name}.c.\n */\n\n#include "{bench_name}.h"\n\n' + body + '\n'.join(keep))
        open(T + prog + '.c', 'w').write(text)
        made[prog] = bench_name
    return made


if __name__ == '__main__':
    result = {}
    for base in (sys.argv[1:] or SPECS):
        result[base] = split(base)
    print(json.dumps(result))
