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
 *
 * 65816 (plan-14-G): the vendored jsmooch core (src/third_party/jsmooch,
 * README.md) on the first 10 tests of each SingleStepTests/65816 file: every
 * bus cycle (24-bit address, data, RWB), then the final registers and RAM.
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
#include "../src/third_party/jsmooch/wdc65816.h"

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

/* ---- 65816 (plan-14-G: third_party/jsmooch) ----------------------------- */

/* 24-bit memory; the addresses a test touches are cleared after it. */
static uint8_t mem816[1 << 24];
static uint32_t touched[4096];
static int ntouched;

static void touch(uint32_t addr)
{
    if (ntouched < (int)(sizeof touched / sizeof touched[0]))
        touched[ntouched++] = addr;
}

static void clear816(void)
{
    for (int i = 0; i < ntouched; i++)
        mem816[touched[i]] = 0;
    ntouched = 0;
}

static void load_ram816(const cJSON *state)
{
    const cJSON *e;
    cJSON_ArrayForEach(e, cJSON_GetObjectItemCaseSensitive(state, "ram")) {
        uint32_t addr = (uint32_t)at(e, 0) & 0xFFFFFF;
        mem816[addr] = (uint8_t)at(e, 1);
        touch(addr);
    }
}

static bool ram816_matches(const cJSON *state)
{
    const cJSON *e;
    cJSON_ArrayForEach(e, cJSON_GetObjectItemCaseSensitive(state, "ram"))
        if (mem816[(uint32_t)at(e, 0) & 0xFFFFFF] != at(e, 1))
            return false;
    return true;
}

/* Compares one bus cycle with a test's [address, value|null, "dpvremxl"]. */
static bool cycle816_matches(const struct WDC65816_pins *pins, const cJSON *c, bool flags)
{
    uint32_t addr = ((uint32_t)pins->BA << 16) | (pins->Addr & 0xFFFF);
    const cJSON *value = cJSON_GetArrayItem(c, 1);
    const char *f = cJSON_GetArrayItem(c, 2)->valuestring;
    bool write = f[3] == 'w';

    if (cJSON_IsNull(cJSON_GetArrayItem(c, 0)))
        return true;   /* the bus is stopped (STP): no address */
    if (addr != (uint32_t)at(c, 0) || (pins->RW != 0) != write)
        return false;
    if (cJSON_IsNumber(value) && (pins->D & 0xFF) != (uint32_t)value->valueint)
        return false;
    (void)flags;
    return true;
}

static bool run816(const cJSON *t)
{
    static struct WDC65816 cpu;
    static u64 clock;
    const cJSON *in = cJSON_GetObjectItemCaseSensitive(t, "initial");
    const cJSON *out = cJSON_GetObjectItemCaseSensitive(t, "final");
    const cJSON *cycles = cJSON_GetObjectItemCaseSensitive(t, "cycles");
    int n = cJSON_GetArraySize(cycles);
    bool ok = true;

    load_ram816(in);
    WDC65816_init(&cpu, &clock);
    cpu.regs.C = (u16)num(in, "a");
    cpu.regs.X = (u32)num(in, "x");
    cpu.regs.Y = (u32)num(in, "y");
    cpu.regs.S = (u32)num(in, "s");
    cpu.regs.D = (u32)num(in, "d");
    cpu.regs.DBR = (u32)num(in, "dbr");
    cpu.regs.PBR = (u16)num(in, "pbr");
    cpu.regs.P.v = (u8)num(in, "p");
    cpu.regs.E = (u32)num(in, "e");
    /* SingleStepTests quirks: an emulation-mode S may have a high byte other
       than 01; X set may come with XH/YH bits set. */
    if (cpu.regs.E)
        cpu.regs.S = 0x100 | (cpu.regs.S & 0xFF);
    if (cpu.regs.P.X) {
        cpu.regs.X &= 0xFF;
        cpu.regs.Y &= 0xFF;
    }
    /* The opcode fetch, as the previous instruction's last cycle leaves it. */
    cpu.regs.TCU = 0;
    cpu.pins.Addr = (u32)num(in, "pc");
    cpu.pins.BA = (u8)num(in, "pbr");
    cpu.pins.RW = 0;
    cpu.pins.PDV = 1;
    cpu.regs.PC = ((u32)num(in, "pc") + 1) & 0xFFFF;
    cpu.pins.D = mem816[((uint32_t)cpu.pins.BA << 16) | cpu.pins.Addr];
    if (n < 1 || !cycle816_matches(&cpu.pins, cJSON_GetArrayItem(cycles, 0), false))
        ok = false;

    for (int i = 1; i <= n && ok; i++) {
        WDC65816_cycle(&cpu);
        uint32_t addr = ((uint32_t)cpu.pins.BA << 16) | (cpu.pins.Addr & 0xFFFF);
        if (cpu.pins.RW) {
            if (i < n) {
                mem816[addr] = (uint8_t)cpu.pins.D;
                touch(addr);
            }
        } else {
            cpu.pins.D = mem816[addr];
        }
        if (i < n && !cycle816_matches(&cpu.pins, cJSON_GetArrayItem(cycles, i), false))
            ok = false;
    }
    /* The last call ended the instruction: the pins fetch the next opcode. */
    /* STP leaves PC past the opcode and the bus stopped. */
    ok = ok && cpu.regs.TCU == 0 &&
         (cpu.regs.STP ? cpu.regs.PC : cpu.pins.Addr & 0xFFFF) == (u32)num(out, "pc") &&
         cpu.regs.PBR == num(out, "pbr") &&
         cpu.regs.C == num(out, "a") && cpu.regs.X == (u32)num(out, "x") &&
         cpu.regs.Y == (u32)num(out, "y") && cpu.regs.S == (u32)num(out, "s") &&
         cpu.regs.D == (u32)num(out, "d") && cpu.regs.DBR == (u32)num(out, "dbr") &&
         cpu.regs.P.v == num(out, "p") && cpu.regs.E == (u32)num(out, "e") &&
         ram816_matches(out);
    WDC65816_delete(&cpu);
    clear816();
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

static const char *const EXPECTED_65816[] = {
    /* The unmodified core (plan-14-G Phase 1); each fix removes its tests. */
    "00 e 4", "00 e 9", "02 e 5", "02 e 7", "02 e 9", "02 e 10", "10 e 1", "10 e 2",
    "10 e 3", "10 e 4", "10 e 5", "10 e 6", "10 e 8", "10 e 9", "10 e 10", "15 e 1",
    "15 e 2", "15 e 3", "15 e 4", "15 e 5", "15 e 6", "15 e 7", "15 e 8", "15 e 9",
    "15 e 10", "16 e 1", "16 e 2", "16 e 3", "16 e 4", "16 e 5", "16 e 6", "16 e 7",
    "16 e 8", "16 e 9", "16 e 10", "30 e 2", "30 e 3", "30 e 4", "30 e 5", "30 e 6",
    "30 e 7", "30 e 8", "30 e 9", "34 e 1", "34 e 2", "34 e 3", "34 e 4", "34 e 5",
    "34 e 6", "34 e 7", "34 e 8", "34 e 9", "34 e 10", "35 e 1", "35 e 2", "35 e 3",
    "35 e 4", "35 e 5", "35 e 6", "35 e 7", "35 e 8", "35 e 9", "35 e 10", "36 e 1",
    "36 e 2", "36 e 3", "36 e 4", "36 e 5", "36 e 6", "36 e 7", "36 e 8", "36 e 9",
    "36 e 10", "44 e 1", "44 e 2", "44 e 3", "44 e 4", "44 e 5", "44 e 7", "44 e 8",
    "44 e 9", "44 e 10", "48 e 1", "48 e 2", "48 e 3", "48 e 4", "48 e 5", "48 e 6",
    "48 e 7", "48 e 8", "48 e 9", "48 e 10", "50 e 1", "50 e 2", "50 e 3", "50 e 4",
    "50 e 5", "50 e 6", "50 e 7", "50 e 8", "50 e 9", "50 e 10", "54 e 1", "54 e 3",
    "54 e 4", "54 e 5", "54 e 6", "54 e 7", "54 e 8", "54 e 9", "54 e 10", "55 e 1",
    "55 e 2", "55 e 3", "55 e 4", "55 e 5", "55 e 6", "55 e 7", "55 e 8", "55 e 9",
    "55 e 10", "56 e 1", "56 e 2", "56 e 3", "56 e 4", "56 e 5", "56 e 6", "56 e 7",
    "56 e 8", "56 e 9", "56 e 10", "5a e 1", "5a e 2", "5a e 3", "5a e 4", "5a e 5",
    "5a e 6", "5a e 7", "5a e 8", "5a e 9", "5a e 10", "68 e 1", "68 e 2", "68 e 3",
    "68 e 4", "68 e 5", "68 e 6", "68 e 7", "68 e 8", "68 e 9", "68 e 10", "70 e 2",
    "70 e 3", "70 e 4", "70 e 5", "70 e 7", "70 e 8", "70 e 9", "70 e 10", "74 e 1",
    "74 e 2", "74 e 3", "74 e 4", "74 e 5", "74 e 6", "74 e 7", "74 e 8", "74 e 9",
    "74 e 10", "75 e 1", "75 e 2", "75 e 3", "75 e 4", "75 e 5", "75 e 6", "75 e 7",
    "75 e 8", "75 e 9", "75 e 10", "76 e 1", "76 e 2", "76 e 3", "76 e 4", "76 e 5",
    "76 e 6", "76 e 7", "76 e 8", "76 e 9", "76 e 10", "7a e 1", "7a e 2", "7a e 3",
    "7a e 4", "7a e 5", "7a e 6", "7a e 7", "7a e 8", "7a e 9", "7a e 10", "80 e 1",
    "80 e 3", "80 e 5", "80 e 6", "80 e 7", "80 e 9", "80 e 10", "90 e 3", "90 e 5",
    "90 e 6", "90 e 7", "90 e 8", "90 e 9", "90 e 10", "94 e 1", "94 e 2", "94 e 3",
    "94 e 4", "94 e 5", "94 e 6", "94 e 7", "94 e 8", "94 e 9", "94 e 10", "95 e 1",
    "95 e 2", "95 e 3", "95 e 4", "95 e 5", "95 e 6", "95 e 7", "95 e 8", "95 e 9",
    "95 e 10", "96 e 1", "96 e 2", "96 e 3", "96 e 4", "96 e 5", "96 e 6", "96 e 7",
    "96 e 8", "96 e 9", "96 e 10", "9a e 1", "9a e 2", "9a e 3", "9a e 4", "9a e 5",
    "9a e 6", "9a e 7", "9a e 8", "9a e 9", "9a e 10", "b0 e 1", "b0 e 2", "b0 e 4",
    "b0 e 5", "b0 e 6", "b0 e 7", "b0 e 8", "b0 e 9", "b0 e 10", "b4 e 1", "b4 e 2",
    "b4 e 3", "b4 e 4", "b4 e 5", "b4 e 6", "b4 e 7", "b4 e 8", "b4 e 9", "b4 e 10",
    "b5 e 1", "b5 e 2", "b5 e 3", "b5 e 4", "b5 e 5", "b5 e 6", "b5 e 7", "b5 e 8",
    "b5 e 9", "b5 e 10", "b6 e 1", "b6 e 2", "b6 e 3", "b6 e 4", "b6 e 5", "b6 e 6",
    "b6 e 7", "b6 e 8", "b6 e 9", "b6 e 10", "c2 e 1", "c2 e 2", "c2 e 3", "c2 e 4",
    "c2 e 5", "c2 e 6", "c2 e 7", "c2 e 8", "c2 e 9", "c2 e 10", "cb e 1", "cb e 2",
    "cb e 3", "cb e 4", "cb e 5", "cb e 6", "cb e 7", "cb e 8", "cb e 9", "cb e 10",
    "cb n 1", "cb n 2", "cb n 3", "cb n 4", "cb n 5", "cb n 6", "cb n 7", "cb n 8",
    "cb n 9", "cb n 10", "d0 e 1", "d0 e 2", "d0 e 3", "d0 e 4", "d0 e 5", "d0 e 6",
    "d0 e 7", "d0 e 9", "d0 e 10", "d5 e 1", "d5 e 2", "d5 e 3", "d5 e 4", "d5 e 5",
    "d5 e 6", "d5 e 7", "d5 e 8", "d5 e 9", "d5 e 10", "d6 e 1", "d6 e 2", "d6 e 3",
    "d6 e 4", "d6 e 5", "d6 e 6", "d6 e 7", "d6 e 8", "d6 e 9", "d6 e 10", "da e 1",
    "da e 2", "da e 3", "da e 4", "da e 5", "da e 6", "da e 7", "da e 8", "da e 9",
    "da e 10", "f0 e 1", "f0 e 2", "f0 e 3", "f0 e 4", "f0 e 5", "f0 e 6", "f0 e 7",
    "f0 e 8", "f5 e 1", "f5 e 2", "f5 e 3", "f5 e 4", "f5 e 5", "f5 e 6", "f5 e 7",
    "f5 e 8", "f5 e 9", "f5 e 10", "f6 e 1", "f6 e 2", "f6 e 3", "f6 e 4", "f6 e 5",
    "f6 e 6", "f6 e 7", "f6 e 8", "f6 e 9", "f6 e 10", "fa e 1", "fa e 2", "fa e 3",
    "fa e 4", "fa e 5", "fa e 6", "fa e 7", "fa e 8", "fa e 9", "fa e 10",
    NULL,
};

typedef struct {
    const char *label, *file;
    const Ops65 *ops; /* NULL: Z80, or `run` */
    const char *const *expected;
    bool (*run)(const cJSON *t);
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
        bool ok = suite->run ? suite->run(t) : suite->ops ? run65(suite->ops, t) : runz80(t);
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
        {"65816", "sst-65816.jsonl", NULL, EXPECTED_65816, run816},
    };
    int errors = 0;
    for (size_t i = 0; i < sizeof suites / sizeof suites[0]; i++)
        errors += run_suite(&suites[i]);
    return errors ? 1 : 0;
}
