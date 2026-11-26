/*
 * cpu_65816_test: split from cpu_test.c by plan-68-H; the shared helpers are
 * support/cpu_test_bench.c.
 */

#include "cpu_test_bench.h"

/* ---- plan-14-H: the W65C816S adapter (core "65816") ----------------------- */

/*
 * A 24-bit bench: PHI2 1 MHz (500 ns high, 500 ns low); the bench latches
 * D0-D7 at each PHI2 rise as the bank (Figure 5-1's latch, closed at the
 * rise) and serves bank:address from rise + 100 ns to fall + 20 ns; a write
 * is stored 1 ps before the fall. Each cycle's log entry also holds probes
 * of D0-D7, E and MX at the datasheet's edges.
 */


enum { B8_RISE = 100, B8_FALL, B8_SERVE, B8_SAMPLE, B8_RELEASE, B8_LOWEND,
       B8_F4, B8_F5, B8_F9, B8_F10, B8_F32, B8_F33, B8_R4, B8_R5, B8_R9, B8_R10, B8_R29, B8_R30 };

static uint8_t mem816[1 << 24];
static uint8_t known816[1 << 24];

static Bench816 *b8_current;

static char b8_char(const Bench816 *b, size_t pin) {
    return "01XZ"[sim_read_pin(b->sim, b->tb, pin)];
}

static int b8_addr(const Bench816 *b) {
    int value = 0;
    int i;

    for (i = 0; i < 16; i++) {
        LogicValue v = sim_read_pin(b->sim, b->tb, b->a[i]);

        if (v != LOGIC_0 && v != LOGIC_1) return -1;
        value |= (v == LOGIC_1) << i;
    }
    return value;
}

static int b8_data(const Bench816 *b) {
    int value = 0;
    int z = 0;
    int bad = 0;
    int i;

    for (i = 0; i < 8; i++) {
        LogicValue v = sim_read_pin(b->sim, b->tb, b->d[i]);

        z += v == LOGIC_Z;
        if (v == LOGIC_1) value |= 1 << i;
        else if (v != LOGIC_0) bad = 1;
    }
    return z == 8 ? -2 : bad ? -1 : value;
}

static void b8_drive(Bench816 *b, size_t pin, LogicValue v) {
    sim_schedule_drive(b->sim, b->tb, pin, v, DRIVE_STRONG, 0);
}

static size_t b8_pins(const void *config) {
    return ((const Bench816 *)config)->part->pin_count;
}

static void *b8_create(const void *config) {
    return (void *)config;
}

static void b8_destroy(void *state) {
    (void)state;
}

static void b8_start(Sim *sim, SimInstance *instance, void *state) {
    Bench816 *b = state;

    (void)instance;
    b8_drive(b, b->clk, LOGIC_0);
    sim_schedule_timer(sim, b->tb, B8_LOWEND, NS(500) - 1);
    sim_schedule_timer(sim, b->tb, B8_RISE, NS(500));
    b->next_fall = NS(1000);
}

static void b8_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    Bench816 *b = state;
    Cycle816 *c = &b->cur;
    int i;

    (void)instance;
    switch (id) {
        case B8_RISE:
            b8_drive(b, b->clk, LOGIC_1);
            /* The bank latch keeps its value when D0-D7 hold no bank (a
               cycle RDY holds: the board gates the latch with RDY). */
            if (b8_data(b) >= 0) b->bank = b8_data(b);
            c->bank = b->bank;
            sim_schedule_timer(sim, b->tb, B8_R4, NS(4));
            sim_schedule_timer(sim, b->tb, B8_R5, NS(5));
            sim_schedule_timer(sim, b->tb, B8_R9, NS(9));
            sim_schedule_timer(sim, b->tb, B8_R10, NS(10));
            sim_schedule_timer(sim, b->tb, B8_R29, NS(29));
            sim_schedule_timer(sim, b->tb, B8_R30, NS(30));
            sim_schedule_timer(sim, b->tb, B8_SERVE, NS(100));
            sim_schedule_timer(sim, b->tb, B8_SAMPLE, NS(500) - 1);
            sim_schedule_timer(sim, b->tb, B8_FALL, NS(500));
            b->next_fall = sim_now(sim) + NS(500);
            break;
        case B8_FALL:
            b8_drive(b, b->clk, LOGIC_0);
            sim_schedule_timer(sim, b->tb, B8_F4, NS(4));
            sim_schedule_timer(sim, b->tb, B8_F5, NS(5));
            sim_schedule_timer(sim, b->tb, B8_F9, NS(9));
            sim_schedule_timer(sim, b->tb, B8_F10, NS(10));
            sim_schedule_timer(sim, b->tb, B8_RELEASE, NS(20));
            sim_schedule_timer(sim, b->tb, B8_F32, NS(32));
            sim_schedule_timer(sim, b->tb, B8_F33, NS(33));
            sim_schedule_timer(sim, b->tb, B8_LOWEND, NS(500) - 1);
            sim_schedule_timer(sim, b->tb, B8_RISE, NS(500));
            b->next_fall = sim_now(sim) + NS(1000);
            break;
        case B8_SERVE:
            if (b8_char(b, b->rw) != '0') {
                int addr = b8_addr(b);
                int known = addr >= 0 && b->bank >= 0 && b8_char(b, b->rw) == '1';
                uint32_t full = known ? ((uint32_t)b->bank << 16) | (uint32_t)addr : 0;
                int value = known && known816[full] ? mem816[full] : addr < 0 ? b->fill : -1;

                for (i = 0; i < 8; i++) {
                    b8_drive(b, b->d[i], value >= 0 ? ((value >> i) & 1 ? LOGIC_1 : LOGIC_0) : LOGIC_X);
                }
                b->serving = 1;
            }
            break;
        case B8_RELEASE:
            if (b->serving) {
                for (i = 0; i < 8; i++) b8_drive(b, b->d[i], LOGIC_Z);
                b->serving = 0;
            }
            break;
        case B8_LOWEND:
            c->mx_low = b8_char(b, b->mx);
            break;
        case B8_F4: c->e4 = b8_char(b, b->e); c->mxf4 = b8_char(b, b->mx); break;
        case B8_F5: c->e5 = b8_char(b, b->e); c->mxf5 = b8_char(b, b->mx); break;
        case B8_F9: c->f9 = b8_data(b); break;
        case B8_F10: c->f10 = b8_data(b); break;
        case B8_F32: c->f32 = b8_data(b); break;
        case B8_F33: c->f33 = b8_data(b); break;
        case B8_R4: c->mxr4 = b8_char(b, b->mx); break;
        case B8_R5: c->mxr5 = b8_char(b, b->mx); break;
        case B8_R9: c->r9 = b8_data(b); break;
        case B8_R10: c->r10 = b8_data(b); break;
        case B8_R29: c->r29 = b8_data(b); break;
        case B8_R30: c->r30 = b8_data(b); break;
        case B8_SAMPLE:
            c->addr = b8_addr(b);
            c->rw = b8_char(b, b->rw);
            c->vda = b8_char(b, b->vda);
            c->vpa = b8_char(b, b->vpa);
            c->vpb = b8_char(b, b->vpb);
            c->mlb = b8_char(b, b->mlb);
            c->e = b8_char(b, b->e);
            c->rdy = b8_char(b, b->rdy);
            c->mx_high = b8_char(b, b->mx);
            c->data = b8_data(b);
            if (c->rw == '0' && c->addr >= 0 && c->bank >= 0) {
                uint32_t full = ((uint32_t)c->bank << 16) | (uint32_t)c->addr;

                mem816[full] = (uint8_t)(c->data >= 0 ? c->data : 0);
                known816[full] = c->data >= 0;
            }
            if (b->log_count < LOG816) b->log[b->log_count++] = *c;
            memset(c, 0, sizeof *c);
            break;
    }
}

static const SimComponentType BENCH816_TYPE = {
    "bench816", b8_pins, b8_create, b8_destroy, b8_start, NULL, b8_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void b8_run_to(Bench816 *b, SimTime t) {
    if (!sim_run_until(b->sim, t)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* n more PHI2 falls, stopping 200 ns after the last. */
static void b8_cycles(Bench816 *b, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) b8_run_to(b, b->next_fall + NS(200));
}

static void b8_load(uint32_t at, const uint8_t *bytes, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        mem816[(at + i) & 0xFFFFFF] = bytes[i];
        known816[(at + i) & 0xFFFFFF] = 1;
    }
}

static void b8_byte(uint32_t at, uint8_t value) {
    b8_load(at, &value, 1);
}

static Bench816 *b8_open(void) {
    Bench816 *b = calloc(1, sizeof(*b));
    const Part *part = part_library_find(library, W65C816S);
    size_t i;
    char pin[8];

    if (b == NULL || part == NULL) {
        fprintf(stderr, "no part %s\n", W65C816S);
        exit(2);
    }
    memset(mem816, 0, sizeof mem816);
    memset(known816, 0, sizeof known816);
    b->part = part;
    for (i = 0; i < 16; i++) {
        snprintf(pin, sizeof pin, "A%zu", i);
        b->a[i] = pin_named(part, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "D%zu", i);
        b->d[i] = pin_named(part, pin);
    }
    b->clk = pin_named(part, "PHI2");
    b->rw = pin_named(part, "RWB");
    b->irq = pin_named(part, "IRQB");
    b->nmi = pin_named(part, "NMIB");
    b->res = pin_named(part, "RESB");
    b->rdy = pin_named(part, "RDY");
    b->abortb = pin_named(part, "ABORTB");
    b->vda = pin_named(part, "VDA");
    b->vpa = pin_named(part, "VPA");
    b->vpb = pin_named(part, "VPB");
    b->mlb = pin_named(part, "MLB");
    b->e = pin_named(part, "E");
    b->mx = pin_named(part, "MX");
    b->be = pin_named(part, "BE");
    b->bank = -1;
    b->fill = -1;
    b->sim = sim_create();
    b->cpu = sim_add_instance(b->sim, kind_model(part), "U1", part);
    b->tb = sim_add_instance(b->sim, &BENCH816_TYPE, "TB", b);
    for (i = 0; i < part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->cpu, i);
        sim_connect(b->sim, net, b->tb, i);
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "sim_start: %s\n", sim_error(b->sim));
        exit(2);
    }
    b8_drive(b, b->irq, LOGIC_1);
    b8_drive(b, b->nmi, LOGIC_1);
    b8_drive(b, b->res, LOGIC_1);
    b8_drive(b, b->abortb, LOGIC_1);
    b8_drive(b, b->be, LOGIC_1);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    b8_current = b;
    return b;
}

static void b8_close(Bench816 *b) {
    sim_destroy(b->sim);
    free(b);
}

static int b8_state(const Bench816 *b, CpuState *out) {
    const SimComponentType *type = sim_instance_type(b->cpu);

    return type->cpu_state != NULL && type->cpu_state(sim_instance_state(b->cpu), out);
}

static uint32_t b8_reg(const CpuState *s, const char *name) {
    size_t i;

    for (i = 0; i < s->register_count; i++) {
        if (strcmp(s->registers[i].name, name) == 0) return s->registers[i].value;
    }
    return 0xEEEEEEEE;
}

/* RESB low for 3 cycles, then high; the log index of the first cycle after it. */
static void b8_reset(Bench816 *b) {
    b8_drive(b, b->res, LOGIC_0);
    b8_cycles(b, 3);
    b8_drive(b, b->res, LOGIC_1);
    b8_cycles(b, 1);
    b->reset_log = b->log_count;
}

/* Vectors: reset 8000; native ABORT A000, IRQ B000, NMI C000; emulation
   IRQ/BRK D000, NMI C000. Then the program at 00:8000 and a reset. */
static Bench816 *b8_boot(const uint8_t *program, size_t n) {
    Bench816 *b = b8_open();

    b8_byte(0xFFFC, 0x00); b8_byte(0xFFFD, 0x80);
    b8_byte(0xFFE8, 0x00); b8_byte(0xFFE9, 0xA0);
    b8_byte(0xFFEE, 0x00); b8_byte(0xFFEF, 0xB0);
    b8_byte(0xFFEA, 0x00); b8_byte(0xFFEB, 0xC0);
    b8_byte(0xFFFE, 0x00); b8_byte(0xFFFF, 0xD0);
    b8_byte(0xFFFA, 0x00); b8_byte(0xFFFB, 0xC0);
    b8_byte(0xA000, 0xDB);   /* ABORT handler: STP */
    b8_byte(0xB000, 0x40);   /* RTI */
    b8_byte(0xC000, 0x40);
    b8_byte(0xD000, 0x40);
    b8_load(0x8000, program, n);
    b8_reset(b);
    return b;
}

static const Cycle816 *c8(const Bench816 *b, size_t i) {
    static const Cycle816 none = {-1, -1, '?', '?', '?', '?', '?', '?', '?', '?', '?', -1};

    return b->reset_log + i < b->log_count ? &b->log[b->reset_log + i] : &none;
}

/* The first cycle at or after `from` (index after reset) with this 24-bit
   address, or SIZE_MAX. */
static size_t c8_find(const Bench816 *b, size_t from, uint32_t full) {
    size_t i;

    for (i = from; b->reset_log + i < b->log_count; i++) {
        const Cycle816 *c = c8(b, i);

        if (c->addr >= 0 && c->bank >= 0 &&
            (((uint32_t)c->bank << 16) | (uint32_t)c->addr) == full) {
            return i;
        }
    }
    return (size_t)-1;
}

/* The first opcode fetch (VDA = VPA = 1) at or after `from` at this address. */
static size_t c8_fetch(const Bench816 *b, size_t from, uint32_t full) {
    size_t i = from;

    while ((i = c8_find(b, i, full)) != (size_t)-1) {
        if (c8(b, i)->vda == '1' && c8(b, i)->vpa == '1') return i;
        i++;
    }
    return (size_t)-1;
}

/* The first write cycle at or after `from` at this address. */
static size_t c8_write(const Bench816 *b, size_t from, uint32_t full) {
    size_t i = from;

    while ((i = c8_find(b, i, full)) != (size_t)-1) {
        if (c8(b, i)->rw == '0') return i;
        i++;
    }
    return (size_t)-1;
}

static int h_failures;

/* On a failure: the log from cycle `from` (after reset), n cycles. */
static void b8_dump(const Bench816 *b, size_t from, size_t n) {
    size_t i;

    for (i = from; i < from + n && b->reset_log + i < b->log_count; i++) {
        const Cycle816 *c = c8(b, i);

        printf("  %3zu %02x:%04x rw%c vda%c vpa%c vpb%c mlb%c e%c(%c%c) mx%c/%c d%4d f33 %4d r30 %4d\n",
               i, (unsigned)(c->bank & 0xFF), (unsigned)(c->addr & 0xFFFF), c->rw, c->vda, c->vpa,
               c->vpb, c->mlb, c->e, c->e4, c->e5, c->mx_low, c->mx_high, c->data, c->f33, c->r30);
    }
}

static void h_check(int ok, const char *what) {
    if (!ok) {
        printf("FAIL: %s\n", what);
        failures++;
        h_failures++;
    }
}

static void h_begin(void) {
    h_failures = 0;
}

static void h_end(const char *name) {
    printf("%s w65c816s_%s\n", h_failures == 0 ? "ok" : "FAIL", name);
}

/* H1: LOST at power-up; reset; the vector read with VPB low; E = 1, MX high. */
static void h1_reset(void) {
    static const uint8_t PROGRAM[] = {0xEA, 0xEA, 0x80, 0xFC};
    Bench816 *b = b8_open();
    CpuState s;
    size_t i;
    size_t v;

    h_begin();
    b8_cycles(b, 2);
    h_check(b8_state(b, &s) && s.status == CPU_LOST, "H1 power-up: LOST");
    b8_byte(0xFFFC, 0x00);
    b8_byte(0xFFFD, 0x80);
    b8_load(0x8000, PROGRAM, sizeof PROGRAM);
    b8_reset(b);
    b8_cycles(b, 12);
    v = c8_find(b, 0, 0x00FFFC);
    h_check(v != (size_t)-1 && c8(b, v)->vpb == '0' && c8(b, v)->vda == '1' &&
                c8(b, v + 1)->addr == 0xFFFD && c8(b, v + 1)->vpb == '0' && c8(b, v + 1)->bank == 0,
            "H1 vector read at 00FFFC/D with VPB low");
    h_check(v != (size_t)-1 && c8(b, v + 2)->addr == 0x8000 && c8(b, v + 2)->vpa == '1' &&
                c8(b, v + 2)->vda == '1' && c8(b, v + 2)->vpb == '1',
            "H1 then the opcode fetch at 008000");
    for (i = 0; i < 6; i++) {
        h_check(c8(b, i)->e == '1' && c8(b, i)->mx_high == '1' && c8(b, i)->mx_low == '1',
                "H1 E = 1, MX high in both phases");
    }
    h_check(b8_state(b, &s) && s.status == CPU_RUNNING, "H1 RUNNING");
    b8_close(b);
    h_end("h1_reset");
}

/* H2: native LDA $123456 (16-bit A): the bank on D0-D7 in PHI2 low. */
static void h2_bank(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xC2, 0x20, 0xAF, 0x56, 0x34, 0x12, 0xDB};
    Bench816 *b;
    CpuState s;
    size_t i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x123456, 0x56);
    b8_byte(0x123457, 0x34);
    b8_cycles(b, 20);
    i = c8_find(b, 0, 0x123456);
    h_check(i != (size_t)-1, "H2 the data read at 123456");
    if (i != (size_t)-1) {
        const Cycle816 *c = c8(b, i);

        h_check(c->f32 == -1 && c->f33 == 0x12, "H2 bank 12 valid at fall + 33 ns (X at 32)");
        h_check(c->r9 == 0x12 && c->r10 == -2, "H2 bank held to rise + 10 ns, then Z");
        h_check(c->vda == '1' && c->vpa == '0' && c->rw == '1', "H2 VDA 1, VPA 0, read");
        h_check(c->data == 0x56 && c8(b, i + 1)->addr == 0x3457 && c8(b, i + 1)->bank == 0x12,
                "H2 then 123457");
    }
    h_check(b8_state(b, &s) && b8_reg(&s, "C") == 0x3456, "H2 C = 3456");
    b8_close(b);
    h_end("h2_bank");
}

/* H3: STA $7F0010: the bank in PHI2 low, then the data from rise + 30 ns,
   held to the next fall + 10 ns. */
static void h3_write(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xA9, 0xA5, 0x8F, 0x10, 0x00, 0x7F, 0xDB};
    Bench816 *b;
    size_t i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 20);
    i = c8_find(b, 0, 0x7F0010);
    h_check(i != (size_t)-1, "H3 the write at 7F0010");
    if (i != (size_t)-1) {
        const Cycle816 *c = c8(b, i);
        const Cycle816 *next = c8(b, i + 1);

        h_check(c->rw == '0' && c->vda == '1' && c->f33 == 0x7F && c->r9 == 0x7F,
                "H3 bank 7F in PHI2 low");
        h_check(c->r10 == -1 && c->r29 == -1 && c->r30 == 0xA5 && c->data == 0xA5,
                "H3 X at rise + 10 ns, data A5 at rise + 30 ns");
        h_check(next->f9 == 0xA5 && next->f10 == -1, "H3 held to the next fall + 10 ns");
        h_check(known816[0x7F0010] && mem816[0x7F0010] == 0xA5, "H3 stored");
    }
    b8_close(b);
    h_end("h3_write");
}

/* H4: per cycle, the bus equals the core's own pins (a bare core on the
   same memory). */
static void h4_flags(void) {
    static const uint8_t PROGRAM[] = {
        0x18, 0xFB,             /* CLC XCE */
        0xA9, 0x12,             /* LDA #$12 */
        0x8D, 0x00, 0x02,       /* STA $0200 */
        0xEE, 0x00, 0x02,       /* INC $0200 */
        0x6C, 0x10, 0x00,       /* JMP ($0010) -> 8020 */
    };
    static struct WDC65816 core;
    static u64 clock;
    Bench816 *b;
    size_t n = 40;
    size_t i;
    int same = 1;
    char message[160];

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x0010, 0x20);
    b8_byte(0x0011, 0x80);
    b8_byte(0x8020, 0xEA);
    b8_byte(0x8021, 0x80);
    b8_byte(0x8022, 0xFD);   /* BRA 8020 */
    b8_byte(0x0200, 0x00);
    b8_cycles(b, (unsigned)n + 2);
    /* The reference: the bare core from reset on the bench's memory image. */
    {
        static uint8_t ram[1 << 16];

        memcpy(ram, mem816, sizeof ram);
        ram[0x0200] = 0x00;
        WDC65816_init(&core, &clock);
        core.regs.E = 1;
        core.regs.P.M = core.regs.P.X = 1;
        WDC65816_reset(&core);
        WDC65816_cycle(&core);
        for (i = 0; i < n && same; i++) {
            const Cycle816 *c = c8(b, i);
            uint32_t addr = core.pins.Addr & 0xFFFF;
            char rw = core.pins.RW ? '0' : '1';

            if (c->addr != (int)addr || c->bank != core.pins.BA || c->rw != rw ||
                c->vda != (core.pins.VDA ? '1' : '0') || c->vpa != (core.pins.VPA ? '1' : '0') ||
                c->vpb != (core.pins.VPB ? '0' : '1')) {
                snprintf(message, sizeof message,
                         "H4 cycle %zu: bus %02x:%04x %c vda %c vpa %c vpb %c, core %02x:%04x %c %u %u %u",
                         i, (unsigned)c->bank, (unsigned)c->addr, c->rw, c->vda, c->vpa, c->vpb,
                         (unsigned)core.pins.BA, (unsigned)addr, rw, core.pins.VDA, core.pins.VPA,
                         core.pins.VPB);
                h_check(0, message);
                same = 0;
            }
            if (core.pins.RW) ram[addr] = (uint8_t)core.pins.D;
            else core.pins.D = ram[addr];
            WDC65816_cycle(&core);
        }
        WDC65816_delete(&core);
    }
    h_check(same, "H4 every cycle's VDA, VPA, VPB, RWB and address equal the core's");
    b8_close(b);
    h_end("h4_flags");
}

/* H5: CLC XCE REP #$10: native, M = 1, X = 0: MX high in PHI2 high, low in
   PHI2 low, changing 5 ns after each edge. */
static void h5_mx(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xC2, 0x10, 0xEA, 0xEA, 0xEA, 0xDB};
    Bench816 *b;
    size_t i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 16);
    i = c8_fetch(b, 0, 0x008005);   /* the second NOP's fetch: after REP */
    h_check(i != (size_t)-1, "H5 the NOP after REP");
    if (i != (size_t)-1) {
        const Cycle816 *c = c8(b, i);

        h_check(c->mx_high == '1' && c->mx_low == '0', "H5 MX: M = 1 in PHI2 high, X = 0 in PHI2 low");
        h_check(c->mxr4 == '0' && c->mxr5 == '1', "H5 MX turns to M at rise + 5 ns");
        h_check(c->mxf4 == '1' && c->mxf5 == '0', "H5 MX turns to X at fall + 5 ns");
    }
    b8_close(b);
    h_end("h5_mx");
}

/* H6: E falls 5 ns after the fall that ends XCE; SEC XCE raises it. */
static void h6_e(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xEA, 0x38, 0xFB, 0xEA, 0xDB};
    Bench816 *b;
    size_t i;
    size_t j;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 20);
    i = c8_fetch(b, 0, 0x008002);   /* the NOP after CLC XCE */
    j = c8_fetch(b, 0, 0x008005);   /* the NOP after SEC XCE */
    h_check(i != (size_t)-1 && j != (size_t)-1, "H6 the NOPs");
    if (i != (size_t)-1 && j != (size_t)-1) {
        h_check(c8(b, i - 1)->e == '1' && c8(b, i)->e4 == '1' && c8(b, i)->e5 == '0' &&
                    c8(b, i)->e == '0',
                "H6 E falls at the fall after XCE's last cycle + 5 ns");
        h_check(c8(b, j - 1)->e == '0' && c8(b, j)->e4 == '0' && c8(b, j)->e5 == '1',
                "H6 SEC XCE: E rises the same way");
    }
    if (h_failures) b8_dump(b, 0, 20);
    b8_close(b);
    h_end("h6_e");
}

/* H7: INC $10: MLB low on the read, modify and write cycles only. */
static void h7_mlb(void) {
    static const uint8_t PROGRAM[] = {0xE6, 0x10, 0xEA, 0xDB};
    Bench816 *b;
    size_t i;
    size_t k;
    int lows = 0;
    int only = 1;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x0010, 0x41);
    b8_cycles(b, 16);
    i = c8_find(b, 0, 0x000010);
    h_check(i != (size_t)-1, "H7 the read of 0010");
    for (k = 0; k < 14; k++) {
        if (c8(b, k)->mlb == '0') {
            lows++;
            only &= c8(b, k)->addr == 0x0010;
        }
    }
    h_check(i != (size_t)-1 && c8(b, i)->mlb == '0' && c8(b, i + 1)->mlb == '0' &&
                c8(b, i + 2)->mlb == '0' && c8(b, i + 2)->rw == '0' && c8(b, i + 3)->mlb == '1',
            "H7 MLB low on read, modify, write");
    h_check(lows == 3 && only, "H7 MLB low on those three cycles only");
    h_check(mem816[0x0010] == 0x42, "H7 the INC stored 42");
    b8_close(b);
    h_end("h7_mlb");
}

/* H8: ABORTB low for one cycle during INX: X unchanged, the INX's address
   pushed, the vector 00FFE8/9 with VPB low; a write made by an aborted STA
   stays. */
static void h8_abort(void) {
    static const uint8_t PROGRAM[] = {
        0x18, 0xFB,             /* 8000 CLC XCE */
        0xC2, 0x30,             /* 8002 REP #$30 */
        0xA9, 0x34, 0x12,       /* 8004 LDA #$1234 */
        0xA2, 0x00, 0x10,       /* 8007 LDX #$1000 */
        0xE8,                   /* 800A INX */
        0xEA, 0xDB,             /* 800B NOP STP */
    };
    static const uint8_t STA_PROGRAM[] = {   /* emulation mode */
        0xA9, 0x77,                   /* 8000 LDA #$77 */
        0x8D, 0x00, 0x03,             /* 8002 STA $0300 */
        0xEA, 0xDB,
    };
    Bench816 *b;
    CpuState s;
    size_t i;
    size_t v;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    while (c8_fetch(b, 0, 0x00800A) == (size_t)-1) b8_cycles(b, 1);
    /* INX's fetch has just ended: ABORTB low for its idle cycle's fall. */
    b8_drive(b, b->abortb, LOGIC_0);
    b8_cycles(b, 1);
    b8_drive(b, b->abortb, LOGIC_1);
    b8_cycles(b, 14);
    h_check(b8_state(b, &s) && b8_reg(&s, "X") == 0x1000, "H8 X unchanged (1000)");
    v = c8_find(b, 0, 0x00FFE8);
    h_check(v != (size_t)-1 && c8(b, v)->vpb == '0' && c8(b, v + 1)->addr == 0xFFE9 &&
                c8(b, v + 1)->vpb == '0',
            "H8 vector read at 00FFE8/9 with VPB low");
    /* Native: PBR at S, then PCH, PCL of the INX, then P. */
    i = c8_write(b, 0, 0x0001FF);
    h_check(i != (size_t)-1 && c8(b, i)->rw == '0' && c8(b, i)->data == 0x00 &&
                c8(b, i + 1)->data == 0x80 && c8(b, i + 2)->data == 0x0A,
            "H8 the stack holds 00 80 0A (the INX's address)");
    if (h_failures) b8_dump(b, 0, 40);
    h_check(b8_state(b, &s) && s.status == CPU_STOPPED && c8_find(b, 0, 0x00A000) != (size_t)-1,
            "H8 the handler ran (STP at A000)");
    b8_close(b);

    b = b8_boot(STA_PROGRAM, sizeof STA_PROGRAM);
    b8_byte(0xFFF8, 0x00);
    b8_byte(0xFFF9, 0xA0);
    while (c8_find(b, 0, 0x008004) == (size_t)-1) b8_cycles(b, 1);   /* STA's last operand */
    b8_drive(b, b->abortb, LOGIC_0);
    b8_cycles(b, 1);
    b8_drive(b, b->abortb, LOGIC_1);
    b8_cycles(b, 14);
    h_check(known816[0x0300] && mem816[0x0300] == 0x77, "H8 the aborted STA's write stays");
    h_check(c8_find(b, 0, 0x00FFF8) != (size_t)-1, "H8 emulation abort vector 00FFF8");
    b8_close(b);
    h_end("h8_abort");
}

/* H9: RDY low holds a read and a write cycle; the data pins stay in their
   transfer state through PHI2 low; RDY X -> LOST. */
static void h9_rdy(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xA9, 0x5A, 0x8D, 0x00, 0x02, 0xAD, 0x00, 0x02, 0xDB};
    Bench816 *b;
    CpuState s;
    size_t i;
    size_t k;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    /* The write: hold it 2 cycles. */
    while (c8_find(b, 0, 0x008006) == (size_t)-1) b8_cycles(b, 1);   /* STA's last operand */
    /* Now in the write cycle's PHI2 low. */
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_0, DRIVE_STRONG, 0);
    b8_cycles(b, 2);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    b8_cycles(b, 6);
    i = c8_find(b, 0, 0x000200);
    h_check(i != (size_t)-1 && c8(b, i)->rw == '0' && c8(b, i + 1)->addr == 0x0200 &&
                c8(b, i + 1)->rw == '0' && c8(b, i + 2)->addr == 0x0200,
            "H9 the write cycle repeats while RDY is low");
    h_check(i != (size_t)-1 && c8(b, i + 1)->f33 == 0x5A && c8(b, i + 2)->f33 == 0x5A,
            "H9 a held write keeps its data through PHI2 low (no bank)");
    /* The read 0200: hold it. */
    k = c8_find(b, i + 3, 0x000200);
    h_check(k != (size_t)-1 && c8(b, k)->rw == '1', "H9 the read of 0200");
    b8_close(b);

    b = b8_boot(PROGRAM, sizeof PROGRAM);
    while (c8_find(b, 0, 0x008009) == (size_t)-1) b8_cycles(b, 1);   /* LDA's last operand */
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_0, DRIVE_STRONG, 0);
    b8_cycles(b, 2);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    b8_cycles(b, 4);
    k = c8_find(b, 0, 0x000200);
    k = c8_find(b, k + 1, 0x000200);   /* the read (after the write) */
    h_check(k != (size_t)-1 && c8(b, k)->rw == '1' && c8(b, k + 1)->addr == 0x0200 &&
                c8(b, k + 1)->rw == '1' && c8(b, k + 1)->f33 == -2,
            "H9 a held read repeats; D0-D7 stay Z through PHI2 low");
    h_check(b8_state(b, &s) && b8_reg(&s, "C") == 0x5A, "H9 the read completes after (A = 5A)");
    if (h_failures) {
        printf("  C %x status %d\n", b8_reg(&s, "C"), s.status);
        b8_dump(b, 0, 30);
    }
    /* RDY X: LOST. */
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_X, DRIVE_STRONG, 0);
    b8_cycles(b, 2);
    h_check(b8_state(b, &s) && s.status == CPU_LOST, "H9 RDY X: LOST");
    b8_close(b);
    h_end("h9_rdy");
}

/* H10: WAI pulls RDY low until an IRQ (I set) ends it; STP; reset restarts. */
static void h10_wai_stp(void) {
    static const uint8_t PROGRAM[] = {0x78, 0xCB, 0xE8, 0xDB};   /* SEI WAI INX STP */
    Bench816 *b;
    CpuState s;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 16);
    h_check(b8_state(b, &s) && s.status == CPU_WAITING, "H10 WAI: WAITING");
    h_check(b8_char(b, b->rdy) == '0', "H10 WAI pulls RDY low");
    b8_drive(b, b->irq, LOGIC_0);
    b8_cycles(b, 6);
    b8_drive(b, b->irq, LOGIC_1);
    b8_cycles(b, 4);
    h_check(b8_char(b, b->rdy) == '1', "H10 the IRQ releases RDY");
    h_check(b8_state(b, &s) && s.status == CPU_STOPPED && b8_reg(&s, "X") == 1,
            "H10 I set: the next instruction (INX) runs, then STP: STOPPED");
    b8_reset(b);
    b8_cycles(b, 10);
    h_check(b8_state(b, &s) && (s.status == CPU_WAITING || s.status == CPU_RUNNING),
            "H10 reset restarts it");
    b8_close(b);
    h_end("h10_wai_stp");
}

/* H11: BE low floats A, D and RWB 25 ns later; VDA/VPA keep driving; the
   CPU keeps counting cycles. */
static void h11_be(void) {
    static const uint8_t PROGRAM[] = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x80, 0xF8};
    Bench816 *b;
    CpuState s;
    uint64_t before;
    SimTime t;
    int floats = 1;
    int i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 4);
    b8_state(b, &s);
    before = s.cycles;
    b->fill = 0xEA;   /* another bus master's memory: NOPs */
    b8_drive(b, b->be, LOGIC_0);
    t = sim_now(b->sim);
    b8_run_to(b, t + NS(24));
    h_check(b8_char(b, b->a[0]) != 'Z', "H11 driven before tBVD");
    b8_run_to(b, t + NS(25));
    for (i = 0; i < 16; i++) floats &= b8_char(b, b->a[i]) == 'Z';
    for (i = 0; i < 8; i++) floats &= b8_char(b, b->d[i]) == 'Z';
    floats &= b8_char(b, b->rw) == 'Z';
    h_check(floats, "H11 A, D, RWB Z at + 25 ns");
    h_check(b8_char(b, b->vda) != 'Z' && b8_char(b, b->vpa) != 'Z', "H11 VDA, VPA still driven");
    b8_cycles(b, 3);
    b8_state(b, &s);
    h_check(s.status == CPU_RUNNING && s.cycles >= before + 3, "H11 the CPU keeps counting cycles");
    if (h_failures) printf("  cycles %llu -> %llu status %d\n", (unsigned long long)before,
                           (unsigned long long)s.cycles, s.status);
    b8_drive(b, b->be, LOGIC_1);
    b8_cycles(b, 1);
    h_check(b8_char(b, b->a[0]) != 'Z' && b8_char(b, b->rw) != 'Z', "H11 BE high drives again");
    b8_close(b);
    h_end("h11_be");
}

/* H12: read data that changed 5 ns before the fall reads X and is used ->
   LOST; IRQB X while I = 1 is harmless. */
static void h12_setup(void) {
    static const uint8_t PROGRAM[] = {0x78, 0xEA, 0xEA, 0xEA, 0xAD, 0x00, 0x02, 0xDB};
    Bench816 *b;
    CpuState s;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x0200, 0x11);
    b8_cycles(b, 4);
    b8_drive(b, b->irq, LOGIC_X);
    b8_cycles(b, 3);
    h_check(b8_state(b, &s) && s.status == CPU_RUNNING, "H12 IRQB X with I = 1: still RUNNING");
    b8_drive(b, b->irq, LOGIC_1);
    /* LDA $0200: change the data 5 ns before the fall of its data read. */
    while (c8_find(b, 0, 0x008006) == (size_t)-1) b8_cycles(b, 1);
    b8_run_to(b, b->next_fall - NS(5));
    b8_drive(b, b->d[0], LOGIC_0);
    b8_cycles(b, 4);
    h_check(b8_state(b, &s) && s.status == CPU_LOST, "H12 data X at the fall, used: LOST");
    b8_close(b);
    h_end("h12_setup");
}

/* H13: registers after a native program in bank 1; the instruction text and
   24-bit address. */
static void h13_state(void) {
    static const uint8_t BANK0[] = {0x18, 0xFB, 0x5C, 0x00, 0x80, 0x01};   /* CLC XCE JML $018000 */
    static const uint8_t BANK1[] = {
        0xC2, 0x20, 0xEA,             /* 018000 REP #$20; NOP */
        0xAF, 0x56, 0x34, 0x12,       /* 018003 LDA $123456 */
        0xDB,                         /* 018007 STP */
    };
    Bench816 *b;
    CpuState s;

    h_begin();
    b = b8_boot(BANK0, sizeof BANK0);
    b8_load(0x018000, BANK1, sizeof BANK1);
    b8_byte(0x123456, 0x44);
    b8_byte(0x123457, 0x33);
    b8_cycles(b, 30);
    h_check(b8_state(b, &s) && s.status == CPU_STOPPED, "H13 STOPPED at the STP");
    h_check(s.register_count == 10 && b8_reg(&s, "C") == 0x3344 && b8_reg(&s, "PBR") == 0x01 &&
                b8_reg(&s, "E") == 0 && b8_reg(&s, "DBR") == 0 && b8_reg(&s, "D") == 0 &&
                b8_reg(&s, "PC") == 0x8008,
            "H13 C 3344, PBR 01, E 0, DBR 00, D 0000, PC 8008");
    h_check(strcmp(s.registers[0].name, "C") == 0 && strcmp(s.registers[9].name, "E") == 0 &&
                s.registers[0].bits == 16 && s.registers[5].bits == 8 && s.registers[9].bits == 1,
            "H13 register order C X Y S D DBR PBR PC P E and widths");
    h_check(s.has_instruction && s.instruction_address == 0x018003 &&
                strcmp(s.instruction_text, "LDA $123456") == 0 && s.instruction_length == 4,
            "H13 last instruction 018003 LDA $123456");
    if (!(s.has_instruction && strcmp(s.instruction_text, "LDA $123456") == 0)) {
        printf("  (instruction %06x '%s' length %zu)\n", (unsigned)s.instruction_address,
               s.instruction_text, s.instruction_length);
    }
    b8_close(b);
    h_end("h13_state");
}

static void run_w65c816s_cases(void) {
    h1_reset();
    h2_bank();
    h3_write();
    h4_flags();
    h5_mx();
    h6_e();
    h7_mlb();
    h8_abort();
    h9_rdy();
    h10_wai_stp();
    h11_be();
    h12_setup();
    h13_state();
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
    }
    run_w65c816s_cases();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
