"""plan-68-G: serial_test.c, serial_test_d.h, serial_test_i.h → benches + 13 programs."""
import re, sys
sys.path.insert(0, '/private/tmp/claude-501/p68')
from csplit import bench

T = 'backend/tests/'
# Names that, made global, would clash with the backend (display_clock.c's
# clock_timer) or the C library (send): renamed sb_* here, in every file.
_text = open(T + 'serial_test.c').read()
_text = re.sub(r'\bclock_timer\b', 'sb_clock_timer', _text)
_text = re.sub(r'\bsend\(', 'sb_send(', _text)
st = _text.split('\n')
dh = open(T + 'serial_test_d.h').read().replace('send(', 'sb_send(').replace('_sb_send(', '_send(').split('\n')
ih = open(T + 'serial_test_i.h').read().replace('send(', 'sb_send(').replace('_sb_send(', '_send(').split('\n')


def find(lines, pred, start=0):
    for k in range(start, len(lines)):
        if pred(lines[k]):
            return k + 1  # 1-based
    raise SystemExit('not found')


def rng(lines, a, b):
    """Lines a..b, 1-based inclusive."""
    return '\n'.join(lines[a - 1:b]) + '\n'


def func(lines, name):
    """(start, end) 1-based of a top-level `static ... name(` function."""
    s = find(lines, lambda l: re.match(r'^static [^(]*\b' + re.escape(name) + r'\(', l))
    e = find(lines, lambda l: l == '}', s - 1)
    return s, e


def hdr(name):
    return find(st, lambda l: l.startswith('/* ') and name in l)


# ---- section boundaries in serial_test.c (1-based) ----
b_comp = hdr('---- Bench components')
c_1 = hdr('---- 1. Reset values')
out_term = hdr('---- Outcome: the terminal')
j_head = hdr('==== plan-12-J:')
h_head = hdr('==== plan-12-H:')
spi = hdr('---- SPI master (mode 0)')
jf = hdr('---- JF: the front ends')
j3 = hdr('---- J3-J6 the max310x family')
h1 = hdr('---- H1: AY-5-1013A')
h4 = hdr('---- H4: 8251A')
h6 = hdr('---- H6: CDP65C51')
h7 = hdr('---- H7: 82C52')
i_head = hdr('==== plan-12-I:')
e_head = hdr('==== plan-12-E:')
e1 = hdr('---- E1 ST16C550')
e16 = hdr('---- E16-E21')
main_at = find(st, lambda l: l == 'int main(void) {')
include_d = find(st, lambda l: l == '#include "serial_test_d.h"')
include_i = find(st, lambda l: l == '#include "serial_test_i.h"')
first_include = find(st, lambda l: l.startswith('#include'))

term_s, term_e = func(st, 'case_terminal')
strip_s, _ = func(st, 'strip_active_low')
_, readfix_e = func(st, 'read_fixture')
shipped_s, shipped_e = func(st, 'case_shipped')
d24_s, d24_e = func(st, 'case_d24')
lnf_s, _ = func(st, 'load_new_footprints')
_, lf_e = func(st, 'load_file')
host_s, host_e = func(st, 'case_host')
rune_s, rune_e = func(st, 'run_e_cases')
runp_s, runp_e = func(st, 'run_p14_e_cases')

# comment blocks directly above a function belong to it
def with_doc(lines, s):
    k = s - 1
    while k >= 1 and (lines[k - 1].startswith((' *', '/*')) or lines[k - 1].startswith('//')):
        k -= 1
    return k + 1

# ---- the common bench ----
common = (rng(st, first_include, c_1 - 1)
          + rng(st, with_doc(st, strip_s), readfix_e)
          + rng(st, with_doc(st, lnf_s), lf_e))
# main's loading block, as a function
main = st[main_at - 1:]
load_from = next(k for k, l in enumerate(main) if 'library = part_library_create();' in l)
load_to = next(k for k, l in enumerate(main) if l.strip() == 'return 1;' and k > load_from) + 1  # the closing brace of the if
load_body = '\n'.join(main[load_from:load_to + 1])
common += '''
/* Loads every part file and fixture the serial programs use (the old
   serial_test main's first step). */
int serial_bench_load(void) {
''' + load_body + '''
    return 0;
}

/* The ending of every serial program: frees the library and reports. */
int serial_bench_finish(void) {
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\\n", failures);
        return 1;
    }
    printf("PASS\\n");
    return 0;
}
'''
c, h = bench(common, 'SERIAL_BENCH_H', ['serial_bench.h'])
h = h.replace('#endif\n', 'int serial_bench_load(void);\nint serial_bench_finish(void);\n\n#endif\n') if 'serial_bench_load' not in h else h
open(T + 'support/serial_bench.c', 'w').write('/* The serial programs\' shared bench (split from serial_test.c, plan-68-G). */\n' + c)
open(T + 'support/serial_bench.h', 'w').write(h)

# ---- helpers a case section defines and other programs call ----
end_j = hdr('==== end of plan-12-J')


def take(lines, name):
    """Cut a function (with its doc comment) out of `lines`; return its text."""
    s0, e0 = func(lines, name)
    s0 = with_doc(lines, s0)
    text = rng(lines, s0, e0)
    for k in range(s0 - 1, e0):
        lines[k] = ''
    return text


moved = {
    'serial_bench_maxim': ''.join(take(st, n) for n in ['m3_start_bit', 'within']),
    'serial_bench_exar': ''.join(take(st, n) for n in ['e_motorola_dual', 'e_powersave']),
    'serial_bench_classic': rng(st, end_j, h1 - 1),
}
# The CDP65C51 crystal half period: H6 defines it, H7 (HS_XHALF) uses it too.
_ac = find(st, lambda l: l.startswith('#define AC_XHALF '))
moved['serial_bench_classic'] += st[_ac - 1] + '\n'
st[_ac - 1] = ''
for k in range(end_j - 1, h1 - 1):
    st[k] = ''

# ---- family benches ----
# The JF section's target component (Tgt, TARGET) serves the SC18IS604 cases too.
jf_cases = with_doc(st, func(st, 'case_jf_spi')[0])
d_cfg_s = find(dh, lambda l: l.startswith('/* ') and 'Bench configurations' in l)
d1 = find(dh, lambda l: l.startswith('/* ') and 'D1 TL16C550C' in l)
d10 = find(dh, lambda l: l.startswith('/* ') and 'D10/D11' in l)
d_run = func(dh, 'run_d_cases')
fam = {
    'serial_bench_d': rng(dh, d_cfg_s, d1 - 1),
    'serial_bench_maxim': rng(st, j_head, h_head - 1) + rng(st, spi, jf - 1) + rng(st, jf, jf_cases - 1),
    'serial_bench_classic': rng(st, h_head, spi - 1),
    'serial_bench_exar': rng(st, e_head, e1 - 1),
    'serial_bench_bridge': rng(ih, 1, find(ih, lambda l: l.startswith('/* ') and 'I1 SC16IS750 over I2C' in l) - 1),
}
for name, text in fam.items():
    text = text + moved.get(name, '')
    c, h = bench(text, name.upper() + '_H', ['serial_bench.h', name + '.h'])
    base = '#include "serial_bench.h"\n' + {'serial_bench_bridge': '#include "serial_bench_maxim.h"\n',
                                             'serial_bench_exar': '#include "serial_bench_d.h"\n'}.get(name, '')
    h = h.replace('#define ' + name.upper() + '_H\n', '#define ' + name.upper() + '_H\n\n' + base, 1)
    open(T + 'support/' + name + '.c', 'w').write(f'/* {name} (split from the serial tests, plan-68-G). */\n' + c)
    open(T + 'support/' + name + '.h', 'w').write(h)

# ---- programs ----
def program(name, desc, benches, body, calls):
    inc = ''.join(f'#include "{b}.h"\n' for b in ['serial_bench'] + benches)
    text = (f'/*\n * {name}: {desc}\n * (split from serial_test.c by plan-68-G; the bench is support/serial_bench.c).\n */\n\n'
            + inc + '\n' + body + '\nint main(void) {\n    if (serial_bench_load() != 0) {\n        return 1;\n    }\n'
            + ''.join(f'    {c};\n' for c in calls) + '    return serial_bench_finish();\n}\n')
    open(T + name + '.c', 'w').write(text)


def calls_of(text):
    return re.findall(r'^static void (case_[a-z0-9_]+)\(void\)', text, re.M)


def run_calls(lines, s, e):
    body = '\n'.join(lines[s:e - 1])
    return [x.strip().rstrip(';') for x in re.findall(r'\n?    (case_[^;]+;)', '\n' + body, re.S)]


d_calls = run_calls(dh, *d_run)
i_calls = run_calls(ih, *func(ih, 'run_i_cases'))
e_calls = run_calls(st, rune_s, rune_e)
p_calls = run_calls(st, runp_s, runp_e)

main_calls = [x.strip().rstrip(';') for x in main if re.match(r'^    (case_|run_)', x)]
order = []
for c in main_calls:
    expand = {'run_d_cases()': d_calls, 'run_i_cases()': i_calls, 'run_e_cases()': e_calls,
              'run_p14_e_cases()': p_calls}
    order += expand.get(c, [c])


def defined(text):
    return set(re.findall(r'^static void (case_[a-z0-9_]+)\(', text, re.M))


def calls_for(text):
    names = defined(text)
    return [c for c in order if c.split('(')[0] in names]


ns_body = rng(st, c_1, term_e) + rng(st, with_doc(st, d24_s), d24_e) + rng(st, with_doc(st, host_s), host_e)
program('serial_ns16550_test', 'the ns16550 core on the fixture devices (cases 1-12), the terminal outcome, the host port',
        [], ns_body, calls_for(ns_body))
d_first = rng(dh, d1, d10 - 1)
first_names = set(re.findall(r'^static void (case_[a-z0-9_]+)\(', d_first, re.M))
program('serial_d_test', 'the TI parts, D1-D9', ['serial_bench_d'], d_first,
        calls_for(d_first))
d_more = rng(dh, d10, d_run[0] - 1) + rng(st, with_doc(st, shipped_s), shipped_e)
program('serial_d_more_test', 'the TI and other 16550 parts D10-D24, and the shipped-device comparison', ['serial_bench_d'],
        d_more, calls_for(d_more))
maxim = rng(st, jf_cases, j3 - 1)
program('serial_maxim_test', 'the SPI/I2C target front ends (JF) and the MAX3100 / MAX3140 (J1, J2)', ['serial_bench_maxim'],
        maxim, calls_for(maxim))
max310x = rng(st, j3, h_head - 1) if False else rng(st, j3, h1 - 1)
program('serial_max310x_test', 'the MAX310x family (J3-J6)', ['serial_bench_maxim'], max310x, calls_for(max310x))
for name, desc, a, b in [('serial_ay1013_test', 'the AY-5-1013A family (H1-H3)', h1, h4),
                         ('serial_intel_test', 'the 8251A and 82510 (H4, H5)', h4, h6),
                         ('serial_acia_test', 'the CDP65C51 and W65C51N (H6, plan-14-E)', h6, h7),
                         ('serial_hs_com_test', 'the 82C52 / HD-6406 and COM78C80x (H7, H8)', h7, i_head)]:
    text = rng(st, a, b - 1)
    if name == 'serial_acia_test':
        text = text.replace(rng(st, runp_s, runp_e), '')
        calls = calls_for(text)
    else:
        calls = calls_for(text)
    program(name, desc, ['serial_bench_classic'], text, calls)
i1 = find(ih, lambda l: l.startswith('/* ') and 'I1 SC16IS750 over I2C' in l)
i8 = find(ih, lambda l: l.startswith('/* ') and 'I8 / I9' in l)
ir = func(ih, 'run_i_cases')
program('serial_bridge_test', 'the SC16IS / PI7C9X / XR20M bridges (I1-I7)', ['serial_bench_maxim', 'serial_bench_bridge'],
        rng(ih, i1, i8 - 1), calls_for(rng(ih, i1, i8 - 1)))
program('serial_sc18is604_test', 'the SC18IS604 (I8, I9)', ['serial_bench_maxim', 'serial_bench_bridge'],
        rng(ih, i8, ir[0] - 1), calls_for(rng(ih, i8, ir[0] - 1)))
exar1 = rng(st, e1, e16 - 1)
names1 = set(re.findall(r'^static void (case_[a-z0-9_]+)\(', exar1, re.M))
program('serial_exar_test', 'the Exar / MaxLinear options E1-E15', ['serial_bench_d', 'serial_bench_exar'], exar1,
        calls_for(exar1))
exar2 = rng(st, e16, rune_s - 1)
program('serial_exar_v_test', 'the Exar / MaxLinear options E16-E28', ['serial_bench_d', 'serial_bench_exar'], exar2,
        calls_for(exar2))
print('ok', len(order), len(set(order)))
