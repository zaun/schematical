/*
 * cpu_core_test: runs the vendored floooh/chips cores (m6502, w65c02, z80;
 * unmodified, see src/third_party/chips/README.md) against the committed
 * SingleStepTests subsets in tests/cpu (plan-09-A §4.4, §4.5).
 *
 * 6502 / 65C02: every bus cycle (address, data, R/W), then the final
 * registers and RAM. P is loaded with bits 4-5 set and compared without
 * them, because the cores keep B and bit 5 in P.
 * Z80: the final registers (incl. WZ, R, I, IFF, IM), RAM, the T-state count
 * and the ordered bus transactions (type, address, data).
 *
 * The tests named in the expected-failure lists are known deviations of the
 * cores. Each must still fail, so a core update that fixes one is noticed.
 * The exit status is non-zero for any unexpected failure, and for any
 * expected failure that passed.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "read_file.h"

#include "../src/third_party/chips/m6502.h"
#include "../src/third_party/chips/w65c02.h"
#include "../src/third_party/chips/z80.h"

static uint8_t mem[65536];

static int num(const cJSON *o, const char *key)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(o, key);
    return cJSON_IsNumber(item) ? item->valueint : -1;
}

static int at(const cJSON *array, int index)
{
    return cJSON_GetArrayItem(array, index)->valueint;
}

static void load_ram(const cJSON *state)
{
    const cJSON *e;
    memset(mem, 0, sizeof mem);
    cJSON_ArrayForEach(e, cJSON_GetObjectItemCaseSensitive(state, "ram"))
        mem[at(e, 0)] = (uint8_t)at(e, 1);
}

static bool ram_matches(const cJSON *state)
{
    const cJSON *e;
    cJSON_ArrayForEach(e, cJSON_GetObjectItemCaseSensitive(state, "ram"))
        if (mem[at(e, 0)] != at(e, 1))
            return false;
    return true;
}

static bool listed(const char *name, const char *const *list)
{
    for (; *list; list++)
        if (strcmp(name, *list) == 0)
            return true;
    return false;
}

/* ---- 6502 family ------------------------------------------------------ */

/* Both cores share the pin layout (M6502_* == W65C02_*); the rest goes
   through this table. */
typedef struct {
    uint64_t (*init)(void *cpu);
    uint64_t (*tick)(void *cpu, uint64_t pins);
    void (*set)(void *cpu, int a, int x, int y, int s, int p, int pc);
    void (*get)(void *cpu, int *a, int *x, int *y, int *s, int *p);
} Ops65;

static uint64_t m6502_init_(void *c)
{
    return m6502_init(c, &(m6502_desc_t){0});
}
static uint64_t m6502_tick_(void *c, uint64_t pins)
{
    return m6502_tick(c, pins);
}
static void m6502_set_(void *c, int a, int x, int y, int s, int p, int pc)
{
    m6502_set_a(c, (uint8_t)a);
    m6502_set_x(c, (uint8_t)x);
    m6502_set_y(c, (uint8_t)y);
    m6502_set_s(c, (uint8_t)s);
    m6502_set_p(c, (uint8_t)p);
    m6502_set_pc(c, (uint16_t)pc);
}
static void m6502_get_(void *c, int *a, int *x, int *y, int *s, int *p)
{
    *a = m6502_a(c);
    *x = m6502_x(c);
    *y = m6502_y(c);
    *s = m6502_s(c);
    *p = m6502_p(c);
}
static uint64_t w65c02_init_(void *c)
{
    return w65c02_init(c, &(w65c02_desc_t){0});
}
static uint64_t w65c02_tick_(void *c, uint64_t pins)
{
    return w65c02_tick(c, pins);
}
static void w65c02_set_(void *c, int a, int x, int y, int s, int p, int pc)
{
    w65c02_set_a(c, (uint8_t)a);
    w65c02_set_x(c, (uint8_t)x);
    w65c02_set_y(c, (uint8_t)y);
    w65c02_set_s(c, (uint8_t)s);
    w65c02_set_p(c, (uint8_t)p);
    w65c02_set_pc(c, (uint16_t)pc);
}
static void w65c02_get_(void *c, int *a, int *x, int *y, int *s, int *p)
{
    *a = w65c02_a(c);
    *x = w65c02_x(c);
    *y = w65c02_y(c);
    *s = w65c02_s(c);
    *p = w65c02_p(c);
}

static const Ops65 OPS_6502 = {m6502_init_, m6502_tick_, m6502_set_, m6502_get_};
static const Ops65 OPS_65C02 = {w65c02_init_, w65c02_tick_, w65c02_set_, w65c02_get_};

static bool run65(const Ops65 *ops, const cJSON *t)
{
    union {
        m6502_t m6502;
        w65c02_t w65c02;
    } cpu;
    const cJSON *in = cJSON_GetObjectItemCaseSensitive(t, "initial");
    const cJSON *out = cJSON_GetObjectItemCaseSensitive(t, "final");
    const cJSON *cycles = cJSON_GetObjectItemCaseSensitive(t, "cycles");
    bool ok = true;

    load_ram(in);
    ops->init(&cpu);
    int pc = num(in, "pc");
    ops->set(&cpu, num(in, "a"), num(in, "x"), num(in, "y"), num(in, "s"),
             num(in, "p") | 0x30, pc);

    /* The prefetch: SYNC with the opcode on the bus. */
    uint64_t pins = M6502_SYNC | M6502_RW;
    M6502_SET_ADDR(pins, pc);
    M6502_SET_DATA(pins, mem[pc]);

    int n = cJSON_GetArraySize(cycles);
    for (int i = 0; i < n; i++) {
        const cJSON *c = cJSON_GetArrayItem(cycles, i);
        bool read = strcmp(cJSON_GetArrayItem(c, 2)->valuestring, "read") == 0;
        if (M6502_GET_ADDR(pins) != at(c, 0) || M6502_GET_DATA(pins) != at(c, 1) ||
            ((pins & M6502_RW) != 0) != read)
            ok = false;
        pins = ops->tick(&cpu, pins);
        uint16_t addr = M6502_GET_ADDR(pins);
        if (pins & M6502_RW)
            M6502_SET_DATA(pins, mem[addr])
        else
            mem[addr] = M6502_GET_DATA(pins);
    }

    int a, x, y, s, p;
    ops->get(&cpu, &a, &x, &y, &s, &p);
    if (a != num(out, "a") || x != num(out, "x") || y != num(out, "y") ||
        s != num(out, "s") || (p & 0xCF) != (num(out, "p") & 0xCF))
        ok = false;
    return ok && ram_matches(out);
}

/* ---- Z80 -------------------------------------------------------------- */

typedef struct {
    char type; /* R W I O: memory read/write, port in/out */
    int addr, data; /* data -1: not recorded */
} Tx;

enum { MAX_TX = 64 };

/* The expected transactions: the first cycle of each run of cycles carrying
   r or w with m or i, its data the first non-null data in that run. */
static int want_transactions(const cJSON *cycles, Tx *want)
{
    int nw = 0;
    bool in_run = false;
    const cJSON *c;
    cJSON_ArrayForEach(c, cycles) {
        const char *fl = cJSON_GetArrayItem(c, 2)->valuestring;
        const cJSON *dv = cJSON_GetArrayItem(c, 1);
        char type = 0;
        /* Flags "rwmi": read, write, memory, I/O; '-' when inactive. */
        if (strlen(fl) >= 4 && (fl[2] == 'm' || fl[3] == 'i')) {
            if (fl[0] == 'r')
                type = fl[3] == 'i' ? 'I' : 'R';
            else if (fl[1] == 'w')
                type = fl[3] == 'i' ? 'O' : 'W';
        }
        if (!type) {
            in_run = false;
            continue;
        }
        int addr = at(c, 0);
        int data = cJSON_IsNumber(dv) ? dv->valueint : -1;
        if (in_run && want[nw - 1].type == type && want[nw - 1].addr == addr) {
            if (want[nw - 1].data < 0)
                want[nw - 1].data = data;
            continue;
        }
        if (nw == MAX_TX)
            return -1;
        want[nw++] = (Tx){type, addr, data};
        in_run = true;
    }
    return nw;
}

static int port_value(const cJSON *ports, int addr)
{
    const cJSON *e;
    int v = 0xFF;
    cJSON_ArrayForEach(e, ports)
        if (at(e, 0) == addr && cJSON_GetArrayItem(e, 2)->valuestring[0] == 'r')
            v = at(e, 1);
    return v;
}

static bool runz80(const cJSON *t)
{
    const cJSON *in = cJSON_GetObjectItemCaseSensitive(t, "initial");
    const cJSON *out = cJSON_GetObjectItemCaseSensitive(t, "final");
    const cJSON *cycles = cJSON_GetObjectItemCaseSensitive(t, "cycles");
    const cJSON *ports = cJSON_GetObjectItemCaseSensitive(t, "ports");
    Tx want[MAX_TX], got[MAX_TX + 1];
    int ng = 0;

    load_ram(in);
    z80_t c;
    z80_init(&c);
    c.a = (uint8_t)num(in, "a");
    c.f = (uint8_t)num(in, "f");
    c.b = (uint8_t)num(in, "b");
    c.c = (uint8_t)num(in, "c");
    c.d = (uint8_t)num(in, "d");
    c.e = (uint8_t)num(in, "e");
    c.h = (uint8_t)num(in, "h");
    c.l = (uint8_t)num(in, "l");
    c.i = (uint8_t)num(in, "i");
    c.r = (uint8_t)num(in, "r");
    c.wz = (uint16_t)num(in, "wz");
    c.ix = (uint16_t)num(in, "ix");
    c.iy = (uint16_t)num(in, "iy");
    c.sp = (uint16_t)num(in, "sp");
    c.af2 = (uint16_t)num(in, "af_");
    c.bc2 = (uint16_t)num(in, "bc_");
    c.de2 = (uint16_t)num(in, "de_");
    c.hl2 = (uint16_t)num(in, "hl_");
    c.im = (uint8_t)num(in, "im");
    c.iff1 = num(in, "iff1") != 0;
    c.iff2 = num(in, "iff2") != 0;

    int nw = want_transactions(cycles, want);
    if (nw < 0)
        return false;

    uint64_t pins = z80_prefetch(&c, (uint16_t)num(in, "pc"));
    int n = cJSON_GetArraySize(cycles);
    for (int i = 0; i <= n; i++) {
        pins = z80_tick(&c, pins);
        uint16_t addr = Z80_GET_ADDR(pins);
        Tx tx = {0, addr, 0};
        if (pins & Z80_MREQ) {
            if (pins & Z80_RD) {
                Z80_SET_DATA(pins, mem[addr]);
                tx = (Tx){'R', addr, mem[addr]};
            } else if (pins & Z80_WR) {
                mem[addr] = Z80_GET_DATA(pins);
                tx = (Tx){'W', addr, mem[addr]};
            }
        } else if (pins & Z80_IORQ) {
            if (pins & Z80_RD) {
                int v = port_value(ports, addr);
                Z80_SET_DATA(pins, v);
                tx = (Tx){'I', addr, v};
            } else if (pins & Z80_WR) {
                tx = (Tx){'O', addr, Z80_GET_DATA(pins)};
            }
        }
        if (tx.type) {
            if (ng > MAX_TX)
                return false;
            got[ng++] = tx;
        }
    }
    /* The final M1 fetch (the next opcode) is not in the test's list. */
    if (ng && got[ng - 1].type == 'R' && (pins & Z80_M1))
        ng--;

    bool ok = z80_opdone(&c);
#define REG(value, key) ok = ok && (int)(value) == num(out, key)
    REG(c.a, "a");
    REG(c.f, "f");
    REG(c.b, "b");
    REG(c.c, "c");
    REG(c.d, "d");
    REG(c.e, "e");
    REG(c.h, "h");
    REG(c.l, "l");
    REG(c.i, "i");
    REG(c.r, "r");
    REG(c.wz, "wz");
    REG(c.ix, "ix");
    REG(c.iy, "iy");
    REG(c.sp, "sp");
    REG((uint16_t)(c.pc - 1), "pc");
    REG(c.af2, "af_");
    REG(c.bc2, "bc_");
    REG(c.de2, "de_");
    REG(c.hl2, "hl_");
    REG(c.im, "im");
    REG(c.iff1, "iff1");
    REG(c.iff2, "iff2");
#undef REG
    ok = ok && ram_matches(out) && ng == nw;
    for (int i = 0; ok && i < nw; i++)
        if (got[i].type != want[i].type || got[i].addr != want[i].addr ||
            (want[i].data >= 0 && got[i].data != want[i].data))
            ok = false;
    return ok;
}

/* ---- Driver ----------------------------------------------------------- */

static const char *const EXPECTED_6502[] = {
    /* JAM: the core re-reads FFFF; the vectors record other bytes. */
    "02 1", "12 1", "22 1", "32 1", "42 1", "52 1",
    "62 1", "72 1", "92 1", "b2 1", "d2 1", "f2 1",
    /* SHA/SHY/SHX: the high address byte on a page cross. */
    "93 1", "9c 1", "9e 1",
    NULL,
};
static const char *const EXPECTED_65C02[] = {NULL};
static const char *const EXPECTED_Z80[] = {
    /* SCF/CCF: the Q register (undocumented XF/YF) is not modelled. */
    "37 0000", "FD 37 0000", "FD 3F 0000",
    /* Repeating block instructions: the interrupted-repeat flags. */
    "ED B1 0000", "ED B2 0000", "ED B8 0000",
    "ED B9 0000", "ED BA 0000", "ED BB 0000",
    /* HALT: the core keeps PC on the opcode and re-fetches it. */
    "76 0000", "DD 76 0000", "FD 76 0000",
    NULL,
};

typedef struct {
    const char *label, *file;
    const Ops65 *ops; /* NULL: Z80 */
    const char *const *expected;
} Suite;

static int run_suite(const Suite *suite)
{
    char path[1024];
    snprintf(path, sizeof path, "%s/%s", CPU_VECTORS_DIR, suite->file);
    char *text = read_file(path);
    if (!text) {
        printf("%s: cannot read %s\n", suite->label, path);
        return 1;
    }
    int pass = 0, xfail = 0, errors = 0, lines = 0, listed_seen = 0;
    for (char *line = text, *next; line && *line; line = next) {
        next = strchr(line, '\n');
        if (next)
            *next++ = '\0';
        if (!*line)
            continue;
        lines++;
        cJSON *t = cJSON_Parse(line);
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(t, "name");
        if (!t || !cJSON_IsString(name)) {
            printf("%s: line %d is not a test\n", suite->label, lines);
            errors++;
            cJSON_Delete(t);
            continue;
        }
        bool ok = suite->ops ? run65(suite->ops, t) : runz80(t);
        bool expected = listed(name->valuestring, suite->expected);
        listed_seen += expected;
        if (ok && !expected) {
            pass++;
        } else if (!ok && expected) {
            xfail++;
        } else if (ok) {
            printf("%s: expected failure passed: %s\n", suite->label, name->valuestring);
            errors++;
        } else {
            printf("%s: FAIL %s\n", suite->label, name->valuestring);
            errors++;
        }
        cJSON_Delete(t);
    }
    free(text);

    int nexpected = 0;
    while (suite->expected[nexpected])
        nexpected++;
    if (listed_seen != nexpected) {
        printf("%s: %d of %d expected failures are not in the vectors\n",
               suite->label, nexpected - listed_seen, nexpected);
        errors++;
    }
    printf("%s: %d pass, %d expected failures\n", suite->label, pass, xfail);
    return errors;
}

int main(void)
{
    static const Suite suites[] = {
        {"6502", "sst-6502.jsonl", &OPS_6502, EXPECTED_6502},
        {"65C02", "sst-65c02.jsonl", &OPS_65C02, EXPECTED_65C02},
        {"Z80", "sst-z80.jsonl", NULL, EXPECTED_Z80},
    };
    int errors = 0;
    for (size_t i = 0; i < sizeof suites / sizeof suites[0]; i++)
        errors += run_suite(&suites[i]);
    return errors ? 1 : 0;
}
