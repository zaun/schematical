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
 * bus cycle (24-bit address, data and the eight flags VDA VPA VPB RWB E M X
 * MLB), then the final registers and RAM; an ABORT case; and, when CMake
 * found node, a check that backend/tools/jsmooch-gen/run.js reproduces the
 * committed wdc65816_generated_opcodes.c.
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

/* The pins' flags as SingleStepTests writes them: "dpvremxl" (VDA, VPA,
   VPB, r/w, E, M, X, MLB), '-' where off. */
static void flags816(const struct WDC65816_pins *pins, char *out)
{
    out[0] = pins->VDA ? 'd' : '-';
    out[1] = pins->VPA ? 'p' : '-';
    out[2] = pins->VPB ? 'v' : '-';
    out[3] = pins->RW ? 'w' : 'r';
    out[4] = pins->E ? 'e' : '-';
    out[5] = pins->M ? 'm' : '-';
    out[6] = pins->X ? 'x' : '-';
    out[7] = pins->ML ? 'l' : '-';
    out[8] = '\0';
}

/* Compares one bus cycle with a test's [address, value|null, "dpvremxl"]:
   the 24-bit address, the data (where the test gives it) and every flag. */
static bool cycle816_matches(const struct WDC65816_pins *pins, const cJSON *c, bool debug)
{
    uint32_t addr = ((uint32_t)pins->BA << 16) | (pins->Addr & 0xFFFF);
    const cJSON *value = cJSON_GetArrayItem(c, 1);
    const char *f = cJSON_GetArrayItem(c, 2)->valuestring;
    char got[9];
    bool ok = true;

    flags816(pins, got);
    if (cJSON_IsNull(cJSON_GetArrayItem(c, 0))) {
        ok = true;   /* the bus is stopped (STP, WAI): no cycle */
    } else if (addr != (uint32_t)at(c, 0) || strcmp(got, f) != 0) {
        ok = false;
    } else if (cJSON_IsNumber(value) && (pins->D & 0xFF) != (uint32_t)value->valueint) {
        ok = false;
    }
    if (debug)
        printf("  got %06x %02x %s  want %06x %3d %s%s\n", addr, (unsigned)(pins->D & 0xFF), got,
               cJSON_IsNull(cJSON_GetArrayItem(c, 0)) ? 0u : (unsigned)at(c, 0),
               cJSON_IsNumber(value) ? value->valueint : -1, f, ok ? "" : "   <--");
    return ok;
}

static bool run816(const cJSON *t)
{
    static struct WDC65816 cpu;
    static u64 clock;
    const cJSON *in = cJSON_GetObjectItemCaseSensitive(t, "initial");
    const cJSON *out = cJSON_GetObjectItemCaseSensitive(t, "final");
    const cJSON *cycles = cJSON_GetObjectItemCaseSensitive(t, "cycles");
    int n = cJSON_GetArraySize(cycles);
    const char *want_debug = getenv("SST65816");
    bool debug = want_debug != NULL &&
                 strcmp(want_debug, cJSON_GetObjectItemCaseSensitive(t, "name")->valuestring) == 0;
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
    cpu.pins.VDA = cpu.pins.VPA = 1;
    cpu.pins.VPB = 0;
    cpu.pins.ML = 0;
    cpu.pins.E = (u8)cpu.regs.E;
    cpu.pins.M = cpu.regs.P.M;
    cpu.pins.X = cpu.regs.P.X;
    if (n < 1 || !cycle816_matches(&cpu.pins, cJSON_GetArrayItem(cycles, 0), debug))
        ok = false;

    for (int i = 1; i <= n && (ok || debug); i++) {
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
        if (i < n && !cycle816_matches(&cpu.pins, cJSON_GetArrayItem(cycles, i), debug))
            ok = false;
    }
    /* The last call ended the instruction: the pins fetch the next opcode. */
    /* STP and WAI leave PC past the opcode and the bus stopped (WAI until an
       interrupt). */
    bool stopped = cpu.regs.STP || cpu.regs.WAI;
    ok = ok && (stopped || cpu.regs.TCU == 0) &&
         (stopped ? cpu.regs.PC : cpu.pins.Addr & 0xFFFF) == (u32)num(out, "pc") &&
         cpu.regs.PBR == num(out, "pbr") &&
         cpu.regs.C == num(out, "a") && cpu.regs.X == (u32)num(out, "x") &&
         cpu.regs.Y == (u32)num(out, "y") && cpu.regs.S == (u32)num(out, "s") &&
         cpu.regs.D == (u32)num(out, "d") && cpu.regs.DBR == (u32)num(out, "dbr") &&
         cpu.regs.P.v == num(out, "p") && cpu.regs.E == (u32)num(out, "e") &&
         ram816_matches(out);
    if (debug)
        printf("  final pc %04x/%04x pbr %02x/%02x a %04x/%04x x %04x/%04x y %04x/%04x s %04x/%04x "
               "d %04x/%04x dbr %02x/%02x p %02x/%02x e %u/%d ram %d\n",
               (unsigned)(stopped ? cpu.regs.PC : cpu.pins.Addr & 0xFFFF), num(out, "pc"),
               (unsigned)cpu.regs.PBR, num(out, "pbr"), (unsigned)cpu.regs.C, num(out, "a"),
               (unsigned)cpu.regs.X, num(out, "x"), (unsigned)cpu.regs.Y, num(out, "y"),
               (unsigned)cpu.regs.S, num(out, "s"), (unsigned)cpu.regs.D, num(out, "d"),
               (unsigned)cpu.regs.DBR, num(out, "dbr"), (unsigned)cpu.regs.P.v, num(out, "p"),
               (unsigned)cpu.regs.E, num(out, "e"), ram816_matches(out));
    WDC65816_delete(&cpu);
    clear816();
    return ok;
}

/* ABORTB (plan-14-G): set during LDA $1234 at 00:8000 (native, 8-bit). The
   LDA runs to its end; the core then pushes PBR, the LDA's address and P,
   pulls the vector at 00FFE8/9 with VPB, sets I, clears D and fetches the
   handler at 9000. */
static int abort816(void)
{
    static struct WDC65816 cpu;
    static u64 clock;
    static const uint8_t program[] = {0xAD, 0x34, 0x12, 0xEA};
    uint32_t vector_reads = 0;
    bool fetched_handler = false;
    int errors = 0;

    for (unsigned i = 0; i < sizeof program; i++) {
        mem816[0x8000 + i] = program[i];
        touch(0x8000 + i);
    }
    mem816[0x1234] = 0x5A;
    mem816[0xFFE8] = 0x00;
    mem816[0xFFE9] = 0x90;
    touch(0x1234);
    touch(0xFFE8);
    touch(0xFFE9);
    WDC65816_init(&cpu, &clock);
    cpu.regs.E = 0;
    cpu.regs.P.v = 0x38;          /* M, X 8-bit; D set (cleared by the abort) */
    cpu.regs.S = 0x01FF;
    cpu.regs.TCU = 0;
    cpu.pins.Addr = 0x8000;
    cpu.pins.BA = 0;
    cpu.pins.RW = 0;
    cpu.regs.PC = 0x8001;
    cpu.pins.D = mem816[0x8000];
    for (int i = 0; i < 40 && !fetched_handler; i++) {
        WDC65816_cycle(&cpu);
        if (i == 1)
            cpu.regs.ABORT_pending = 1;   /* during the LDA's operand fetch */
        uint32_t addr = ((uint32_t)cpu.pins.BA << 16) | (cpu.pins.Addr & 0xFFFF);
        if (cpu.pins.RW) {
            mem816[addr] = (uint8_t)cpu.pins.D;
            touch(addr);
        } else {
            cpu.pins.D = mem816[addr];
        }
        if (cpu.pins.VPB && (addr == 0xFFE8 || addr == 0xFFE9))
            vector_reads++;
        if (cpu.regs.TCU == 0 && addr == 0x9000 && cpu.pins.VPA)
            fetched_handler = true;
    }
    if (!fetched_handler || vector_reads != 2) {
        printf("65816: abort: handler fetched %d, vector reads with VPB %u\n", fetched_handler,
               vector_reads);
        errors++;
    }
    /* The LDA completed (A = 5A); the pushes: PBR, PCH, PCL of the LDA, P. */
    if ((cpu.regs.C & 0xFF) != 0x5A || mem816[0x01FF] != 0x00 || mem816[0x01FE] != 0x80 ||
        mem816[0x01FD] != 0x00 || mem816[0x01FC] != 0x38 || cpu.regs.S != 0x01FB ||
        !cpu.regs.P.I || cpu.regs.P.D || cpu.regs.PBR != 0) {
        printf("65816: abort: A %02x stack %02x %02x %02x %02x S %04x I %u D %u PBR %02x\n",
               (unsigned)(cpu.regs.C & 0xFF), mem816[0x01FF], mem816[0x01FE], mem816[0x01FD],
               mem816[0x01FC], (unsigned)cpu.regs.S, cpu.regs.P.I, cpu.regs.P.D,
               (unsigned)cpu.regs.PBR);
        errors++;
    }
    WDC65816_delete(&cpu);
    clear816();
    printf("65816: abort %s\n", errors ? "FAIL" : "ok");
    return errors;
}

/* The vendored generator reproduces the committed opcode table (when CMake
   found node). */
static int generator816(void)
{
#ifdef JSMOOCH_NODE
    char cmd[2048];
    char out[1024];
    snprintf(out, sizeof out, "%s/jsmooch-gen-check.c", JSMOOCH_TMP);
    snprintf(cmd, sizeof cmd, "\"%s\" \"%s\" \"%s\"", JSMOOCH_NODE, JSMOOCH_GEN, out);
    if (system(cmd) != 0) {
        printf("65816: generator: run.js failed\n");
        return 1;
    }
    char *made = read_file(out);
    char *committed = read_file(JSMOOCH_C);
    bool same = made && committed && strcmp(made, committed) == 0;
    free(made);
    free(committed);
    remove(out);
    printf("65816: generator %s\n", same ? "reproduces wdc65816_generated_opcodes.c"
                                         : "FAIL: output differs from wdc65816_generated_opcodes.c");
    return same ? 0 : 1;
#else
    printf("65816: generator check skipped (no node)\n");
    return 0;
#endif
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
    errors += abort816();
    errors += generator816();
    return errors ? 1 : 0;
}
