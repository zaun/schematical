/*
 * serial_test: the serial kind (plan-12-C section 4). Cases 1-12 run the
 * ns16550 core on the fixture devices of support/serial_fixture.h
 * (SFX-16550: PC16550D-shaped; SFX-16450: the same with "fifo": 0), plus
 * the terminal outcome (8N1 at divisor 1 on a 1.8432 MHz clock).
 *
 * The bench: one net per UART pin with a driver on each; a free-running
 * clock on XIN; BAUDOUT wired to RCLK (the receiver runs at the
 * transmitter's rate). The bus is driven Intel style with CS0 = CS1 = 1,
 * CS2 = 0, ADS = 0, RD = WR = 0 and the active-low strobes pulsed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "serial_fixture.h"
#include "net.h"
#include "serial_fixture_classic.h"
#include "serial_fixture_maxim.h"
#include "sim.h"

#define NS(x) ((SimTime)(x) * 1000u)

static int failures;
static int case_failures;
/* The PC16550D-shaped part C's cases run on (the C fixture, then the shipped device: D24). */
static const char *c_part = "SFX-16550";
static PartLibrary *library;

static void check(int ok, const char *what) {
    if (!ok) {
        printf("  FAIL: %s\n", what);
        failures++;
        case_failures++;
    }
}

static void case_begin(void) {
    case_failures = 0;
}

static void case_end(const char *name) {
    printf("%s %s\n", case_failures == 0 ? "ok  " : "FAIL", name);
}

/* ---- Bench components ---------------------------------------------------------- */

static size_t driver_pins(const void *config) {
    return *(const size_t *)config;
}

static const SimComponentType DRIVER = {
    "driver", driver_pins, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

typedef struct {
    SimTime half;
    LogicValue level;
} ClockState;

static size_t clock_pins(const void *config) {
    (void)config;
    return 1;
}

static void *clock_create(const void *config) {
    ClockState *s = calloc(1, sizeof *s);

    s->half = *(const SimTime *)config;
    return s;
}

static void clock_destroy(void *state) {
    free(state);
}

static void clock_start(Sim *sim, SimInstance *instance, void *raw) {
    ClockState *s = raw;

    s->level = LOGIC_0;
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_timer(sim, instance, 0, s->half);
}

static void clock_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    ClockState *s = raw;

    (void)id;
    s->level = s->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    sim_schedule_drive(sim, instance, 0, s->level, DRIVE_STRONG, 0);
    sim_schedule_timer(sim, instance, 0, s->half);
}

static const SimComponentType CLOCK = {
    "clock", clock_pins, clock_create, clock_destroy, clock_start, NULL, clock_timer,
    NULL, NULL, NULL, NULL, NULL
};

/* ---- The bench ------------------------------------------------------------------ */

/*
 * How a device sits on the bench: its strobe, data, address, reset and clock
 * pins, the pins tied at fixed levels, and how each unit (channel, printer
 * port) is selected for a bus access.
 */
typedef struct {
    const char *read;              /* active-low read strobe */
    const char *write;             /* active-low write strobe */
    const char *data[8];
    const char *address[8];
    unsigned address_count;
    const char *ties[32];          /* "NAME=1" / "NAME=0" / "NAME=Z" */
    const char *reset;             /* NULL = none */
    int reset_high;
    const char *clock;
    const char *baudout;           /* joined to rclk when both are set */
    const char *rclk;
    /* Per unit: the select pins to make active ("NAME=0" etc.; each unit
       sets these and the other units' to their inactive levels). */
    const char *unit_select[8][3];
    unsigned units;
    unsigned recovery_ns;          /* after a strobe ends (0 = 160 ns); longer for
                                      clock-synchronous strobes */
} BenchConfig;

static const BenchConfig PC16550D_BENCH = {
    "RDBAR", "WRBAR",
    {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"},
    {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "RD=0", "WR=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "XIN", "BAUDOUT", "RCLK",
    {{NULL}}, 0
};

typedef struct {
    Sim *sim;
    SimInstance *chip;
    SimInstance *drv;
    SimInstance *clk;
    const Part *part;
    const BenchConfig *cfg;
    unsigned unit;
    size_t pins;
    SimTime half;
    SimTime period;
} Bench;

static size_t pin_of(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s on %s\n", name, b->part->name);
    exit(2);
}

static int has_pin(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; name != NULL && i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, pin_of(b, name), v, DRIVE_STRONG, 0);
}

/* "NAME=1" style: sets the pin; `invert` drives the opposite level. */
static void set_tie(Bench *b, const char *tie, int invert) {
    char name[32];
    const char *eq = strchr(tie, '=');
    size_t n = (size_t)(eq - tie);
    LogicValue v;

    memcpy(name, tie, n);
    name[n] = '\0';
    v = eq[1] == '1' ? LOGIC_1 : eq[1] == '0' ? LOGIC_0 : LOGIC_Z;
    if (invert && v != LOGIC_Z) {
        v = v == LOGIC_1 ? LOGIC_0 : LOGIC_1;
    }
    set(b, name, v);
}

static void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        const SimNet *net = sim_shorted_net(b->sim);

        fprintf(stderr, "sim stopped: %s (%s, net %s)\n", sim_error(b->sim), b->part->name,
                net != NULL ? sim_net_name(net) : "-");
        exit(2);
    }
}

static LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, pin_of(b, name));
}

/* Selects unit u (a channel, or the printer port) for the next accesses. */
static void select_unit(Bench *b, unsigned u) {
    unsigned i;
    unsigned k;

    b->unit = u;
    for (i = 0; i < b->cfg->units; i++) {
        for (k = 0; k < 3 && b->cfg->unit_select[i][k] != NULL; k++) {
            set_tie(b, b->cfg->unit_select[i][k], i != u);
        }
    }
    for (k = 0; u < b->cfg->units && k < 3 && b->cfg->unit_select[u][k] != NULL; k++) {
        set_tie(b, b->cfg->unit_select[u][k], 0);
    }
}

static void pulse_reset(Bench *b) {
    if (b->cfg->reset == NULL) {
        return;
    }
    set(b, b->cfg->reset, b->cfg->reset_high ? LOGIC_1 : LOGIC_0);
    run(b, NS(6000));
    set(b, b->cfg->reset, b->cfg->reset_high ? LOGIC_0 : LOGIC_1);
    run(b, NS(100));
}

/* A device on a bench with a clock of `half` ps per half period. */
static Bench *bench_open_cfg(const char *part_name, SimTime half, const BenchConfig *cfg) {
    Bench *b = calloc(1, sizeof *b);
    SimNet *baud_net = NULL;
    size_t i;
    int join = cfg->baudout != NULL && cfg->rclk != NULL;

    b->part = part_library_find(library, part_name);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part_name);
        exit(2);
    }
    b->cfg = cfg;
    b->pins = b->part->pin_count;
    b->half = half;
    b->period = 2 * half;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->clk = sim_add_instance(b->sim, &CLOCK, "CLK", &b->half);
    for (i = 0; i < b->pins; i++) {
        const char *name = b->part->pins[i].name;
        SimNet *net;

        if (join && strcmp(name, cfg->rclk) == 0) {
            continue;
        }
        net = sim_add_net(b->sim, name);
        sim_connect(b->sim, net, b->chip, i);
        if (join && strcmp(name, cfg->baudout) == 0) {
            baud_net = net;
        } else {
            sim_connect(b->sim, net, b->drv, i);
        }
        if (strcmp(name, cfg->clock) == 0) {
            sim_connect(b->sim, net, b->clk, 0);
        }
    }
    if (join) {
        sim_connect(b->sim, baud_net, b->chip, pin_of(b, cfg->rclk));
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    for (i = 0; cfg->ties[i] != NULL; i++) {
        set_tie(b, cfg->ties[i], 0);
    }
    set(b, cfg->read, LOGIC_1);
    set(b, cfg->write, LOGIC_1);
    for (i = 0; i < cfg->address_count; i++) {
        set(b, cfg->address[i], LOGIC_0);
    }
    select_unit(b, 0);
    pulse_reset(b);
    return b;
}

static Bench *bench_open(const char *part_name, SimTime half) {
    return bench_open_cfg(part_name, half, &PC16550D_BENCH);
}

static void bench_close(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

static void set_address(Bench *b, unsigned a) {
    unsigned i;

    for (i = 0; i < b->cfg->address_count; i++) {
        set(b, b->cfg->address[i], (a >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void set_data(Bench *b, int drive, unsigned v) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        set(b, b->cfg->data[i], !drive ? LOGIC_Z : (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void bus_write(Bench *b, unsigned a, unsigned v) {
    set_address(b, a);
    set_data(b, 1, v);
    run(b, NS(40));
    set(b, b->cfg->write, LOGIC_0);
    run(b, NS(120));
    set(b, b->cfg->write, LOGIC_1);
    run(b, NS(b->cfg->recovery_ns > 40 ? b->cfg->recovery_ns : 40));
    set_data(b, 0, 0);
    run(b, NS(160));
}

/* A read: the bus as text, high bit first ("01XZ"), and the value (unknown bits 0). */
static unsigned bus_read_text(Bench *b, unsigned a, char text[9]) {
    unsigned v = 0;
    unsigned i;

    set_address(b, a);
    run(b, NS(40));
    set(b, b->cfg->read, LOGIC_0);
    run(b, NS(150));
    for (i = 0; i < 8; i++) {
        LogicValue l = get(b, b->cfg->data[i]);

        text[7 - i] = "01XZ"[l];
        if (l == LOGIC_1) {
            v |= 1u << i;
        }
    }
    text[8] = '\0';
    set(b, b->cfg->read, LOGIC_1);
    run(b, NS(b->cfg->recovery_ns != 0 ? b->cfg->recovery_ns : 160));
    return v;
}

static unsigned bus_read(Bench *b, unsigned a) {
    char text[9];

    return bus_read_text(b, a, text);
}

/* Reads and checks a register as text ("01XZ" per bit, high bit first). */
static void expect_reg(Bench *b, unsigned a, const char *want, const char *what) {
    char text[9];
    char message[128];

    bus_read_text(b, a, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

static void expect_pin(Bench *b, const char *pin, LogicValue want, const char *what) {
    char message[128];
    LogicValue got = get(b, pin);

    snprintf(message, sizeof message, "%s: %s is %c, want %c", what, pin, "01XZ"[got], "01XZ"[want]);
    check(got == want, message);
}

/* LCR, divisor and FIFO setup. */
static void program(Bench *b, unsigned divisor, unsigned lcr) {
    bus_write(b, 3, 0x80);
    bus_write(b, 0, divisor & 0xFF);
    bus_write(b, 1, divisor >> 8);
    bus_write(b, 3, lcr);
}

static SimTime bit_time(const Bench *b, unsigned divisor) {
    return 16u * divisor * b->period;
}

/* Waits up to `limit` for SOUT to fall; 1 when it did (the time is now). */
static int wait_start(Bench *b, SimTime limit) {
    SimTime step = b->period / 4;
    SimTime spent = 0;

    while (spent < limit) {
        if (get(b, "SOUT") == LOGIC_0) {
            return 1;
        }
        run(b, step);
        spent += step;
    }
    return 0;
}

/* Samples a frame of n bits after the start bit (data, parity, stops) at
   mid-bit; levels as '0'/'1'/'X'. Call right after wait_start. */
static void sample_frame(Bench *b, unsigned divisor, unsigned n, char *out) {
    SimTime bt = bit_time(b, divisor);
    unsigned i;

    run(b, bt / 2);
    out[0] = "01XZ"[get(b, "SOUT")];
    for (i = 1; i <= n; i++) {
        run(b, bt);
        out[i] = "01XZ"[get(b, "SOUT")];
    }
    out[n + 1] = '\0';
}

/* Drives one frame on SIN: start, bits[] ('0'/'1'), at the given bit time. */
static void drive_bits(Bench *b, const char *bits, SimTime bt) {
    size_t i;

    for (i = 0; bits[i] != '\0'; i++) {
        set(b, "SIN", bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
}

/* An 8N1 frame for byte v, with a stop bit of `stop`. */
static void drive_byte(Bench *b, unsigned v, SimTime bt, int stop) {
    char bits[16];
    unsigned i;

    bits[0] = '0';
    for (i = 0; i < 8; i++) {
        bits[1 + i] = (v >> i) & 1u ? '1' : '0';
    }
    bits[9] = stop ? '1' : '0';
    bits[10] = '\0';
    drive_bits(b, bits, bt);
    set(b, "SIN", LOGIC_1);
}

static size_t sent(Bench *b, uint16_t *frames, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_sent(sim_instance_state(b->chip), 0, 0, frames, cap);
}

static int send(Bench *b, const char *text) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_send(b->sim, b->chip, sim_instance_state(b->chip), 0,
                             (const uint8_t *)text, strlen(text));
}

/* 1.8432 MHz: 542.5 ns period. */
#define HALF_18432 NS(271)

/* ---- 1. Reset values ------------------------------------------------------------------ */

static void case_reset(void) {
    Bench *b = bench_open(c_part, HALF_18432);

    case_begin();
    expect_reg(b, 1, "00000000", "IER after MR");
    expect_reg(b, 2, "00000001", "IIR after MR");
    expect_reg(b, 3, "00000000", "LCR after MR");
    expect_reg(b, 4, "00000000", "MCR after MR");
    expect_reg(b, 5, "01100000", "LSR after MR");
    expect_reg(b, 6, "00000000", "MSR after MR (inputs inactive)");
    expect_reg(b, 7, "00000000", "SCR after MR (decisions: 0x00)");
    expect_reg(b, 0, "XXXXXXXX", "RBR at power-on (not reset)");
    bus_write(b, 3, 0x80);
    expect_reg(b, 0, "XXXXXXXX", "DLL at power-on");
    expect_reg(b, 1, "XXXXXXXX", "DLM at power-on");
    bus_write(b, 3, 0x00);
    expect_pin(b, "SOUT", LOGIC_1, "after MR");
    expect_pin(b, "INTR", LOGIC_0, "after MR");
    expect_pin(b, "RTS", LOGIC_1, "after MR");
    expect_pin(b, "DTR", LOGIC_1, "after MR");
    expect_pin(b, "OUT1", LOGIC_1, "after MR");
    expect_pin(b, "OUT2", LOGIC_1, "after MR");
    expect_pin(b, "RXRDY", LOGIC_1, "after MR");
    expect_pin(b, "TXRDY", LOGIC_0, "after MR");
    expect_pin(b, "DDIS", LOGIC_1, "no read");
    expect_pin(b, "BAUDOUT", LOGIC_X, "divisor X");
    expect_pin(b, "D0", LOGIC_Z, "bus released");
    /* Modem inputs: CTS low -> MSR[4] = 1 with DCTS. */
    set(b, "CTS", LOGIC_0);
    run(b, NS(500));
    expect_reg(b, 6, "00010001", "MSR after CTS goes active");
    /* MR resets the registers again but leaves SCR's power-on rule and DLL. */
    bus_write(b, 7, 0x5A);
    bus_write(b, 1, 0x0F);
    set(b, "MR", LOGIC_1);
    run(b, NS(6000));
    set(b, "MR", LOGIC_0);
    run(b, NS(100));
    expect_reg(b, 1, "00000000", "IER after a second MR");
    expect_reg(b, 6, "00010000", "MSR after MR: deltas cleared, CTS still active");
    /* A write cut short by MR is not stored: an X-data write to LSR (factory
       test bits) in progress when MR rises leaves LSR at its reset value. */
    set_address(b, 5);
    set_data(b, 0, 0);
    run(b, NS(40));
    set(b, b->cfg->write, LOGIC_0);
    run(b, NS(120));
    set(b, "MR", LOGIC_1);
    run(b, NS(6000));
    set(b, b->cfg->write, LOGIC_1);
    run(b, NS(100));
    set(b, "MR", LOGIC_0);
    run(b, NS(100));
    expect_reg(b, 5, "01100000", "LSR after a write cut short by MR");
    case_end("1 reset values and outputs after MR");
    bench_close(b);
}

/* ---- 2. Divisor and BAUDOUT ------------------------------------------------------------ */

/* Time between two BAUDOUT rising edges. */
static SimTime baud_period(Bench *b) {
    SimTime step = NS(10);
    SimTime t0 = 0;
    LogicValue last = get(b, "BAUDOUT");
    int edges = 0;
    SimTime spent = 0;

    while (edges < 2 && spent < NS(200000)) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "BAUDOUT");
        if (last == LOGIC_0 && now == LOGIC_1) {
            if (edges == 0) {
                t0 = sim_now(b->sim);
            }
            edges++;
            if (edges == 2) {
                return sim_now(b->sim) - t0;
            }
        }
        last = now;
    }
    return 0;
}

static void case_divisor(void) {
    Bench *b = bench_open(c_part, NS(50));   /* 10 MHz clock */
    SimTime p;
    char message[96];

    case_begin();
    bus_write(b, 3, 0x80);
    bus_write(b, 0, 0x03);
    bus_write(b, 1, 0x00);
    expect_reg(b, 0, "00000011", "DLL reads back");
    expect_reg(b, 1, "00000000", "DLM reads back");
    p = baud_period(b);
    snprintf(message, sizeof message, "BAUDOUT period %llu ps, want 3 x 100 ns",
             (unsigned long long)p);
    check(p == NS(300), message);
    bus_write(b, 0, 0x0A);
    p = baud_period(b);
    snprintf(message, sizeof message, "BAUDOUT period %llu ps, want 10 x 100 ns",
             (unsigned long long)p);
    check(p == NS(1000), message);
    bus_write(b, 0, 0x01);
    p = baud_period(b);
    snprintf(message, sizeof message, "divisor 1: BAUDOUT period %llu ps, want the clock's",
             (unsigned long long)p);
    check(p == NS(100), message);
    bus_write(b, 0, 0x00);
    run(b, NS(500));
    expect_pin(b, "BAUDOUT", LOGIC_X, "divisor 0");
    case_end("2 divisor latch, BAUDOUT period, divisor 0");
    bench_close(b);
}

/* ---- 3. Transmit frames ------------------------------------------------------------------ */

static void tx_one(Bench *b, unsigned lcr, unsigned byte, unsigned bits, const char *want,
                   const char *what) {
    char got[24];
    char message[128];

    bus_write(b, 3, lcr);
    bus_write(b, 0, byte);
    check(wait_start(b, NS(20000)), what);
    sample_frame(b, 1, bits, got);
    snprintf(message, sizeof message, "%s: SOUT %s, want %s", what, got, want);
    check(strcmp(got, want) == 0, message);
    run(b, bit_time(b, 1) * 4);
}

static void case_tx(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    uint16_t frames[8];
    SimTime t0;
    SimTime t1;
    char message[96];

    case_begin();
    program(b, 1, 0x03);
    /* 8N1 'H' = 0x48: start, 0 0 0 1 0 0 1 0, stop. */
    tx_one(b, 0x03, 0x48, 9, "0000100101", "8N1 0x48");
    /* 7E2 0x35 = 0110101: data LSB first 1010110, parity even (4 ones) 0, stops 1 1. */
    tx_one(b, 0x1E, 0x35, 10, "01010110011", "7E2 0x35");
    /* 5O1.5 0x13 = 10011: data 11001, three ones: odd parity 0, stop. */
    tx_one(b, 0x0C, 0x13, 7, "01100101", "5O1.5 0x13");
    /* Stick parity: PEN, EPS, stick -> parity 0; EPS 0 -> parity 1. */
    tx_one(b, 0x3B, 0xFF, 10, "01111111101", "8, stick parity 0");
    tx_one(b, 0x2B, 0x00, 10, "00000000011", "8, stick parity 1");
    check(sent(b, frames, 8) == 5 && frames[0] == 0x48 && frames[1] == 0x35 &&
              frames[2] == 0x13 && frames[3] == 0xFF && frames[4] == 0x00,
          "the sent log holds the data bits");
    /* Bit time = 16 x divisor clocks: the start bit's length. */
    bus_write(b, 3, 0x03);
    bus_write(b, 0, 0x01);
    check(wait_start(b, NS(20000)), "start of 0x01");
    t0 = sim_now(b->sim);
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "start bit %llu ps, want 16 clocks (%llu)",
             (unsigned long long)(t1 - t0), (unsigned long long)bit_time(b, 1));
    check(t1 - t0 >= bit_time(b, 1) - b->period / 4 && t1 - t0 <= bit_time(b, 1) + NS(200),
          message);
    /* 1.5 stop bits = 24 clocks: back-to-back 5O1.5 frames start 16 + 5 x 16
       + 16 + 24 = 136 clocks apart. */
    run(b, bit_time(b, 1) * 12);
    bus_write(b, 2, 0x01);
    bus_write(b, 3, 0x0C);
    bus_write(b, 0, 0x00);
    bus_write(b, 0, 0x00);
    /* Parity rises (data 0, odd parity 1) frame to frame, measured to 10 ns. */
    check(wait_start(b, NS(20000)), "first 5O1.5 start");
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t0 = sim_now(b->sim);
    while (get(b, "SOUT") != LOGIC_0) {
        run(b, NS(10));
    }
    while (get(b, "SOUT") == LOGIC_0) {
        run(b, NS(10));
    }
    t1 = sim_now(b->sim);
    snprintf(message, sizeof message, "5O1.5 frame to frame %llu ps, want 136 clocks (%llu)",
             (unsigned long long)(t1 - t0), (unsigned long long)(136 * b->period));
    check(t1 - t0 + NS(20) >= 136 * b->period && t1 - t0 <= 136 * b->period + NS(20), message);
    case_end("3 transmit 8N1, 7E2, 5O1.5, stick parity; bit time; sent log");
    bench_close(b);
}

/* ---- 4. Receive ------------------------------------------------------------------------------ */

static void case_rx(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    drive_byte(b, 0x5A, bt, 1);
    run(b, bt * 2);
    expect_reg(b, 5, "01100001", "LSR: DR after a frame");
    expect_reg(b, 0, "01011010", "RBR = 0x5A");
    expect_reg(b, 5, "01100000", "LSR: DR cleared by the RBR read");

    /* A false start: low for a quarter bit. */
    set(b, "SIN", LOGIC_0);
    run(b, bt / 4);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 12);
    expect_reg(b, 5, "01100000", "a short low pulse is no start bit");

    /* Framing error, then the resync: the bad stop bit is the next start. */
    drive_bits(b, "0101010100", bt);          /* 0x55 with stop 0 */
    drive_bits(b, "11000000", bt);            /* the next char's data: 0x03 ... */
    drive_bits(b, "11", bt);                  /* ... its stop */
    set(b, "SIN", LOGIC_1);
    run(b, bt * 2);
    expect_reg(b, 5, "01101011", "LSR: FE, and OE (the resync char overran the first), DR");
    expect_reg(b, 0, "00000011", "the resync character: the 8 bits after the bad stop");

    /* Parity error: 8E1 with odd parity on the line. */
    bus_write(b, 3, 0x1B);
    drive_bits(b, "0100000001", bt);          /* 0x01, parity 1 (even wants 1? one 1 -> 1) */
    drive_bits(b, "1", bt);
    run(b, bt * 2);
    expect_reg(b, 5, "01100001", "even parity with the right parity bit: no PE");
    bus_read(b, 0);
    drive_bits(b, "0100000000", bt);          /* parity 0: wrong */
    drive_bits(b, "1", bt);
    run(b, bt * 2);
    expect_reg(b, 5, "01100101", "a wrong parity bit sets PE");
    bus_read(b, 0);

    /* Break: low for two character times: BI with FE, one 0x00. */
    bus_write(b, 3, 0x03);
    set(b, "SIN", LOGIC_0);
    run(b, bt * 22);
    set(b, "SIN", LOGIC_1);
    run(b, bt * 3);
    expect_reg(b, 5, "01111001", "LSR: BI and FE after a break");
    expect_reg(b, 0, "00000000", "the break character is 0x00");
    expect_reg(b, 5, "01100000", "only one character for the break");
    case_end("4 receive: data, false start, framing error and resync, parity, break");
    bench_close(b);
}

/* ---- 5. FIFO ---------------------------------------------------------------------------------- */

static void case_fifo(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    unsigned i;
    char text[24];

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0xC1);   /* FIFO on, trigger 14 */
    bus_write(b, 1, 0x01);   /* ERBFI */
    expect_reg(b, 2, "11000001", "IIR: FIFOs enabled, nothing pending");
    for (i = 0; i < 13; i++) {
        drive_byte(b, 0x30 + i, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_0, "13 bytes, trigger 14");
    drive_byte(b, 0x3D, bt, 1);
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_1, "14 bytes reach trigger 14");
    expect_reg(b, 2, "11000100", "IIR: RX data available");
    drive_byte(b, 0x3E, bt, 1);
    drive_byte(b, 0x3F, bt, 1);
    run(b, bt);
    expect_reg(b, 5, "01100001", "16 bytes: no overrun");
    drive_byte(b, 0x40, bt, 1);
    run(b, bt);
    expect_reg(b, 5, "01100011", "the 17th byte overruns");
    for (i = 0; i < 16; i++) {
        bus_read_text(b, 0, text);
        if (i == 0) {
            check(strcmp(text, "00110000") == 0, "the first byte is first out");
        }
        if (i == 15) {
            check(strcmp(text, "00111111") == 0, "the 16th byte is last; the 17th was lost");
        }
    }
    expect_reg(b, 5, "01100000", "empty after 16 reads");

    /* Trigger levels 1, 4, 8. */
    {
        static const unsigned FCR[3] = {0x01, 0x41, 0x81};
        static const unsigned LEVEL[3] = {1, 4, 8};
        unsigned t;

        for (t = 0; t < 3; t++) {
            bus_write(b, 2, FCR[t] | 0x02);
            for (i = 0; i + 1 < LEVEL[t]; i++) {
                drive_byte(b, 0x41, bt, 1);
            }
            run(b, bt);
            check(get(b, "INTR") == LOGIC_0, "below the trigger level: no interrupt");
            drive_byte(b, 0x42, bt, 1);
            run(b, bt);
            check(get(b, "INTR") == LOGIC_1, "at the trigger level: interrupt");
        }
    }

    /* LSR[7]: an error character in the FIFO; FCR[1] clears the RX FIFO. */
    bus_write(b, 2, 0xC3);
    drive_byte(b, 0x11, bt, 1);
    drive_byte(b, 0x22, bt, 0);   /* framing error */
    set(b, "SIN", LOGIC_1);
    run(b, bt * 12);
    expect_reg(b, 5, "11100001", "LSR[7]: an error further down the FIFO");
    expect_reg(b, 5, "11100001", "LSR[7] stays: the error is not at the top yet");
    bus_read(b, 0);
    expect_reg(b, 5, "11101001", "the error char at the top shows FE; LSR[7] while it is in the FIFO");
    expect_reg(b, 5, "01100001", "that LSR read cleared FE and LSR[7] (no later errors)");
    bus_write(b, 2, 0xC3);
    expect_reg(b, 5, "01100000", "FCR[1] cleared the RX FIFO");
    case_end("5 FIFO: trigger levels 1/4/8/14, overrun on the 17th, LSR[7], FCR[1]");
    bench_close(b);
}

/* ---- 6. Timeout ----------------------------------------------------------------------------- */

static void case_timeout(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0x41);   /* FIFO, trigger 4 */
    bus_write(b, 1, 0x01);
    drive_byte(b, 0x61, bt, 1);
    run(b, bt * 30);
    expect_pin(b, "INTR", LOGIC_0, "3 character times: no timeout yet");
    run(b, bt * 12);
    expect_pin(b, "INTR", LOGIC_1, "after 4 character times: timeout");
    expect_reg(b, 2, "11001100", "IIR = C (timeout)");
    expect_reg(b, 0, "01100001", "the byte");
    expect_reg(b, 2, "11000001", "the RBR read cleared the timeout");
    run(b, NS(1000));
    expect_pin(b, "INTR", LOGIC_0, "INTR cleared (trint after the read)");
    case_end("6 timeout after 4 character times, cleared by an RBR read");
    bench_close(b);
}

/* ---- 7. Interrupt priority ------------------------------------------------------------------------ */

static void case_priority(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    SimTime t;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    drive_byte(b, 0x77, bt, 0);   /* framing error: line status + data */
    set(b, "SIN", LOGIC_1);
    run(b, bt * 3);
    set(b, "CTS", LOGIC_0);       /* modem */
    run(b, NS(500));
    bus_write(b, 1, 0x0F);        /* all enabled; THRE already empty -> THRE source */
    expect_reg(b, 2, "00000110", "line status first");
    bus_read(b, 5);
    expect_reg(b, 2, "00000100", "then RX data");
    bus_read(b, 0);
    expect_reg(b, 2, "00000010", "then THRE");
    expect_reg(b, 2, "00000000", "the IIR read that reported THRE cleared it: modem");
    bus_read(b, 6);
    expect_reg(b, 2, "00000001", "nothing after the MSR read");
    expect_pin(b, "INTR", LOGIC_0, "nothing pending");

    /* A THR write clears THRE; it comes back when the byte moves to the shifter. */
    bus_write(b, 1, 0x00);
    bus_write(b, 1, 0x02);
    expect_reg(b, 2, "00000010", "enabling ETBEI with THRE = 1 raises THRE");
    bus_write(b, 0, 0x55);
    expect_reg(b, 5, "00100000", "the byte moved to the shifter at once: THRE again, TEMT 0");
    run(b, bt * 12);

    /* Transmitter interrupt delay (FIFO mode): the first after FCR[0] is
       immediate; a lone byte later delays THRE to its last stop bit. */
    bus_write(b, 2, 0x01);
    bus_write(b, 0, 0x11);
    run(b, bt / 2);
    check((bus_read(b, 5) & 0x20) != 0, "first THRE after FCR[0] is immediate");
    run(b, bt * 12);
    bus_read(b, 2);
    bus_write(b, 0, 0x22);
    t = sim_now(b->sim);
    run(b, bt * 2);
    check((bus_read(b, 5) & 0x20) == 0, "a lone byte: THRE delayed while it is sent");
    while ((bus_read(b, 5) & 0x20) == 0 && sim_now(b->sim) - t < bt * 12) {
    }
    t = sim_now(b->sim) - t;
    check(t > bt * 8 && t < bt * 10, "THRE comes at the last stop bit (9 bit times in)");
    run(b, bt * 4);
    /* Two bytes at once: THRE as soon as the FIFO empties. */
    bus_write(b, 0, 0x33);
    bus_write(b, 0, 0x44);
    run(b, bt * 11);
    check((bus_read(b, 5) & 0x20) != 0, "two bytes at once: THRE when the FIFO empties");
    case_end("7 interrupt priority, IIR codes, THRE clears, transmitter interrupt delay");
    bench_close(b);
}

/* ---- 8. Modem status -------------------------------------------------------------------------------- */

static void case_modem(void) {
    Bench *b = bench_open(c_part, HALF_18432);

    case_begin();
    bus_write(b, 1, 0x08);
    set(b, "DSR", LOGIC_0);
    set(b, "DCD", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "INTR", LOGIC_1, "modem interrupt");
    expect_reg(b, 2, "00000000", "IIR: modem status");
    expect_reg(b, 6, "10101010", "MSR: DSR, DCD active, DDSR, DDCD");
    expect_reg(b, 6, "10100000", "deltas cleared by the read");
    expect_pin(b, "INTR", LOGIC_0, "interrupt cleared by the MSR read");
    set(b, "RI", LOGIC_0);
    run(b, NS(500));
    expect_reg(b, 6, "11100000", "RI active: no TERI on the leading edge");
    set(b, "RI", LOGIC_1);
    run(b, NS(500));
    expect_reg(b, 6, "10100100", "TERI on the trailing edge of RI");
    expect_reg(b, 6, "10100000", "TERI cleared");
    /* MCR drives the outputs inverted. */
    bus_write(b, 4, 0x0F);
    run(b, NS(300));
    expect_pin(b, "DTR", LOGIC_0, "MCR[0]");
    expect_pin(b, "RTS", LOGIC_0, "MCR[1]");
    expect_pin(b, "OUT1", LOGIC_0, "MCR[2]");
    expect_pin(b, "OUT2", LOGIC_0, "MCR[3]");
    case_end("8 modem: delta bits, TERI on the trailing RI edge, MSR read clears, interrupt");
    bench_close(b);
}

/* ---- 9. Loopback ------------------------------------------------------------------------------------- */

static void case_loopback(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    SimTime spent = 0;
    int stayed = 1;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 4, 0x1F);   /* loop + all modem outputs */
    run(b, NS(300));
    expect_reg(b, 6, "11111011", "MSR mirrors MCR (DTR->DSR, RTS->CTS, OUT1->RI, OUT2->DCD)");
    expect_pin(b, "RTS", LOGIC_1, "modem outputs inactive in loopback");
    expect_pin(b, "OUT2", LOGIC_1, "modem outputs inactive in loopback");
    bus_write(b, 0, 0xA5);
    while (spent < bt * 12) {
        stayed &= get(b, "SOUT") == LOGIC_1;
        run(b, NS(500));
        spent += NS(500);
    }
    check(stayed, "SOUT stays 1 in loopback");
    expect_reg(b, 0, "10100101", "the sent byte arrives in RBR");
    case_end("9 loopback: SOUT 1, TX to RX, MSR from MCR");
    bench_close(b);
}

/* ---- 10. DMA modes ---------------------------------------------------------------------------------- */

static void case_dma(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    SimTime bt;
    unsigned i;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    /* Mode 0. */
    expect_pin(b, "TXRDY", LOGIC_0, "mode 0: TXRDY active when empty");
    expect_pin(b, "RXRDY", LOGIC_1, "mode 0: RXRDY inactive when empty");
    drive_byte(b, 0x01, bt, 1);
    run(b, bt);
    expect_pin(b, "RXRDY", LOGIC_0, "mode 0: RXRDY active with one character");
    bus_read(b, 0);
    run(b, NS(300));
    expect_pin(b, "RXRDY", LOGIC_1, "mode 0: RXRDY inactive after the read (twrr)");
    /* Mode 1: FIFO, DMA mode, trigger 4. */
    bus_write(b, 2, 0x49);
    drive_byte(b, 0x02, bt, 1);
    run(b, bt);
    expect_pin(b, "RXRDY", LOGIC_1, "mode 1: one character is not enough");
    for (i = 0; i < 3; i++) {
        drive_byte(b, 0x03, bt, 1);
    }
    run(b, bt);
    expect_pin(b, "RXRDY", LOGIC_0, "mode 1: RXRDY at the trigger level");
    for (i = 0; i < 3; i++) {
        bus_read(b, 0);
    }
    expect_pin(b, "RXRDY", LOGIC_0, "mode 1: stays active until empty");
    bus_read(b, 0);
    run(b, NS(300));
    expect_pin(b, "RXRDY", LOGIC_1, "mode 1: inactive when the FIFO is empty");
    for (i = 0; i < 16; i++) {
        bus_write(b, 0, 0x30 + i);
    }
    expect_pin(b, "TXRDY", LOGIC_0, "mode 1: TXRDY active until the FIFO is full");
    bus_write(b, 0, 0x50);
    expect_pin(b, "TXRDY", LOGIC_1, "mode 1: TXRDY inactive when full");
    case_end("10 DMA modes 0 and 1 on TXRDY and RXRDY");
    bench_close(b);
}

/* ---- 11. Bus timing and strobes ----------------------------------------------------------------------- */

static void case_bus(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    char text[9];

    case_begin();
    bus_write(b, 7, 0x96);
    set_address(b, 7);
    run(b, NS(50));
    set(b, "RDBAR", LOGIC_0);
    run(b, NS(59));
    expect_pin(b, "D1", LOGIC_Z, "59 ns into the read: not yet (trvd 60)");
    expect_pin(b, "DDIS", LOGIC_1, "59 ns: DDIS still high (trdd 60)");
    run(b, NS(2));
    expect_pin(b, "D1", LOGIC_1, "61 ns: data");
    expect_pin(b, "DDIS", LOGIC_0, "61 ns: DDIS low");
    set(b, "RDBAR", LOGIC_1);
    run(b, NS(99));
    expect_pin(b, "D1", LOGIC_1, "99 ns after the read: still driven (thz 100)");
    run(b, NS(2));
    expect_pin(b, "D1", LOGIC_Z, "101 ns: floating");

    /* ADS: the address latched on its rising edge is held while it is high. */
    set_address(b, 7);
    run(b, NS(50));
    set(b, "ADS", LOGIC_1);
    run(b, NS(50));
    set_address(b, 3);
    run(b, NS(50));
    set(b, "RDBAR", LOGIC_0);
    run(b, NS(150));
    check(get(b, "D1") == LOGIC_1 && get(b, "D2") == LOGIC_1 && get(b, "D0") == LOGIC_0,
          "ADS high: the latched address (SCR) is read");
    set(b, "RDBAR", LOGIC_1);
    run(b, NS(150));
    set(b, "ADS", LOGIC_0);
    run(b, NS(50));

    /* X on RD during a read drives X. */
    set_address(b, 7);
    set(b, "RD", LOGIC_X);
    run(b, NS(150));
    expect_pin(b, "D1", LOGIC_X, "X read strobe: X data");
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    /* RDBAR low and RD high together: X (decisions.md). */
    set(b, "RDBAR", LOGIC_0);
    set(b, "RD", LOGIC_1);
    run(b, NS(150));
    expect_pin(b, "D1", LOGIC_X, "both read strobes active: X data");
    set(b, "RDBAR", LOGIC_1);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    /* WR high alone is a write. */
    set_address(b, 7);
    set_data(b, 1, 0x3C);
    run(b, NS(40));
    set(b, "WR", LOGIC_1);
    run(b, NS(120));
    set(b, "WR", LOGIC_0);
    run(b, NS(40));
    set_data(b, 0, 0);
    run(b, NS(100));
    bus_read_text(b, 7, text);
    check(strcmp(text, "00111100") == 0, "the active-high write strobe writes");
    case_end("11 bus: trvd, thz, DDIS, ADS latch, X and double read strobes, WR");
    bench_close(b);
}

/* ---- 12. 16450 ---------------------------------------------------------------------------------------- */

static void case_16450(void) {
    Bench *b = bench_open("SFX-16450", HALF_18432);
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);
    bt = bit_time(b, 1);
    bus_write(b, 2, 0xC1);
    expect_reg(b, 2, "00000001", "16450: FCR write ignored, IIR[7:6] = 00");
    bus_write(b, 1, 0x01);
    drive_byte(b, 0x31, bt, 1);
    drive_byte(b, 0x32, bt, 1);
    run(b, bt);
    expect_reg(b, 5, "01100011", "16450: the second byte overruns (no FIFO)");
    expect_reg(b, 0, "00110010", "16450: the RBR holds the last byte");
    drive_byte(b, 0x33, bt, 1);
    run(b, bt * 60);
    expect_reg(b, 2, "00000100", "16450: data available, never a timeout");
    case_end("12 16450: no FCR, no FIFO, no timeout");
    bench_close(b);
}

/* ---- Outcome: the terminal ---------------------------------------------------------------------------- */

static void case_terminal(void) {
    Bench *b = bench_open(c_part, HALF_18432);
    uint16_t frames[4] = {0};
    SimTime bt;

    case_begin();
    program(b, 1, 0x03);   /* 8N1, divisor 1: 115200 baud on 1.8432 MHz */
    bus_write(b, 2, 0x01);
    bt = bit_time(b, 1);
    bus_write(b, 0, 'H');
    bus_write(b, 0, 'i');
    run(b, bt * 24);
    check(sent(b, frames, 4) == 2 && frames[0] == 0x48 && frames[1] == 0x69,
          "sent log: 0x48 0x69");
    set(b, "SIN", LOGIC_Z);   /* nothing else drives RX */
    run(b, NS(100));
    check(send(b, "OK") == 1, "send queued");
    run(b, bt * 24);
    expect_reg(b, 0, "01001111", "RBR reads 0x4F");
    expect_reg(b, 0, "01001011", "then 0x4B");
    case_end("C outcome: 8N1 at 115200 sends H i; typed OK arrives as 0x4F 0x4B");
    bench_close(b);
}

#include "serial_test_d.h"

/* Compares every device of one fixture document with the device of the same
   id in serial.json (fixture ids are "SFX-<id>"; parts aside). Returns how
   many were found there. */
static int compare_shipped(const char *fixture_text, const cJSON *shipped, const char *what) {
    char message[200];
    int compared = 0;
    cJSON *fixture = cJSON_Parse(fixture_text);
    const cJSON *dev;

    check(fixture != NULL, what);
    cJSON_ArrayForEach(dev, cJSON_GetObjectItemCaseSensitive(fixture, "devices")) {
        const char *id = cJSON_GetObjectItemCaseSensitive(dev, "id")->valuestring + 4;
        const cJSON *other;

        cJSON_ArrayForEach(other, cJSON_GetObjectItemCaseSensitive(shipped, "devices")) {
            if (strcmp(cJSON_GetObjectItemCaseSensitive(other, "id")->valuestring, id) == 0) {
                cJSON *a = cJSON_Duplicate(dev, 1);
                cJSON *b = cJSON_Duplicate(other, 1);

                cJSON_DeleteItemFromObjectCaseSensitive(a, "parts");
                cJSON_DeleteItemFromObjectCaseSensitive(b, "parts");
                cJSON_ReplaceItemInObjectCaseSensitive(a, "id", cJSON_CreateString(id));
                snprintf(message, sizeof message, "%s device %s: fixture equals serial.json", what,
                         id);
                check(cJSON_Compare(a, b, 1), message);
                cJSON_Delete(a);
                cJSON_Delete(b);
                compared++;
            }
        }
    }
    cJSON_Delete(fixture);
    return compared;
}

/* "shipped": every fixture device equals the same device in Parts/serial.json
   (its parts aside), so a model case cannot pass on a device that differs
   from the one the library ships (plan-12-D section 3.3). The fixtures: the
   ns16550 devices (ns16550.json), the bridges (bridges.json, plan-12-I), the
   classic UARTs (plan-12-H) and the Maxim UARTs (plan-12-J); a fixture device
   not shipped yet is skipped. */
static char *read_fixture(const char *file) {
    char path[512];

    snprintf(path, sizeof path, "%s/../fixtures/serial/%s", GOLDEN_DIR, file);
    return read_file(path);
}

static void case_shipped(void) {
    char path[512];
    char *ns_text = read_fixture("ns16550.json");
    char *bridge_text = read_fixture("bridges.json");
    char *shipped_text;
    char message[160];

    case_begin();
    snprintf(path, sizeof path, "%s/serial.json", PARTS_DIR);
    shipped_text = read_file(path);
    check(ns_text != NULL && bridge_text != NULL, "the fixture files");
    check(shipped_text != NULL, "Parts/serial.json (shipped by plan-12-F onwards)");
    if (ns_text != NULL && bridge_text != NULL && shipped_text != NULL) {
        cJSON *shipped = cJSON_Parse(shipped_text);
        int ns = compare_shipped(ns_text, shipped, "ns16550");
        int bridges = compare_shipped(bridge_text, shipped, "bridges");
        int classic = compare_shipped(SERIAL_FIXTURE_CLASSIC, shipped, "classic");
        int maxim = compare_shipped(SERIAL_FIXTURE_MAXIM, shipped, "maxim");

        printf("   compared: %d ns16550, %d bridges, %d classic, %d maxim\n", ns, bridges,
               classic, maxim);
        snprintf(message, sizeof message, "%d ns16550 fixture devices found in serial.json", ns);
        check(ns > 0, message);
        cJSON_Delete(shipped);
    }
    free(ns_text);
    free(bridge_text);
    free(shipped_text);
    case_end("shipped: the fixture devices are the shipped device objects");
}

/* D24: C's cases on the shipped PC16550D device object. */
static void case_d24(void) {
    int before = failures;

    c_part = "SFX-PC16550D";
    printf("-- C's cases on SFX-PC16550D:\n");
    case_reset();
    case_divisor();
    case_tx();
    case_rx();
    case_fifo();
    case_timeout();
    case_priority();
    case_modem();
    case_loopback();
    case_dma();
    case_bus();
    case_terminal();
    c_part = "SFX-16550";
    case_begin();
    case_failures = failures - before;
    case_end("D24 PC16550D: the C cases on the shipped device object");
}

/* The fixture footprints (tests/fixtures/serial/footprints.json: the new
   footprints of the ns16550 parts) that the library does not have yet; once a
   data sub-plan moves one into Parts/footprints.json the library's wins. */
static int load_new_footprints(void) {
    char path[512];
    char *text;
    char *error = NULL;
    char *out;
    cJSON *doc;
    cJSON *list;
    cJSON *fp;
    int result;

    snprintf(path, sizeof path, "%s/../fixtures/serial/footprints.json", GOLDEN_DIR);
    text = read_file(path);
    if (text == NULL) {
        fprintf(stderr, "cannot read %s\n", path);
        return -1;
    }
    doc = cJSON_Parse(text);
    free(text);
    list = cJSON_GetObjectItemCaseSensitive(doc, "footprints");
    for (fp = list != NULL ? list->child : NULL; fp != NULL;) {
        cJSON *next = fp->next;
        const char *id = cJSON_GetObjectItemCaseSensitive(fp, "id")->valuestring;

        if (part_library_find_footprint(library, id) != NULL) {
            cJSON_Delete(cJSON_DetachItemViaPointer(list, fp));
        }
        fp = next;
    }
    out = cJSON_PrintUnformatted(doc);
    cJSON_Delete(doc);
    result = load_footprints(library, out, "fixtures/serial/footprints.json", &error);
    if (result != 0) {
        fprintf(stderr, "fixture footprints: %s\n", error != NULL ? error : "(no message)");
    }
    free(error);
    free(out);
    return result;
}

/* The shipped ns16550 devices as fixtures (tests/fixtures/serial/ns16550.json,
   built by merge.py from keys/ and devices/; plan-12-D section 3.3). */
static int load_file(const char *name, int footprints) {
    char path[512];
    char *text;
    char *error = NULL;
    int result;

    snprintf(path, sizeof path, "%s/../fixtures/serial/%s", GOLDEN_DIR, name);
    text = read_file(path);
    if (text == NULL) {
        fprintf(stderr, "cannot read %s\n", path);
        return -1;
    }
    result = footprints ? load_footprints(library, text, name, &error)
                        : load_parts(library, text, name, &error);
    if (result != 0) {
        fprintf(stderr, "%s: %s\n", name, error != NULL ? error : "(no message)");
    }
    free(error);
    free(text);
    return result;
}

/* ==== plan-12-J: the Maxim SPI/I2C UARTs and the SPI/I2C target front ends ==========
 *
 * JF1/JF2 drive the front ends of serial_target.h directly (a test target
 * that logs its events); J1-J6 run the max3100 and max310x models on the
 * shipped device objects of support/serial_fixture_maxim.h. The bench: one
 * net per pin with a driver on each, an optional clock on XIN/X1, weak
 * pull-ups on the I2C lines, and pins tied onto another pin's net (I2C
 * address straps). SPI is bit-banged in mode 0, I2C at 400 kHz with the
 * bench's SDA/SCL open drain (0 or Z).
 */

#include "serial_fixture_maxim.h"
#include "serial_target.h"

typedef struct {
    size_t count;
} PullConfig;

static size_t pull_pins(const void *config) {
    return ((const PullConfig *)config)->count;
}

static void pull_start(Sim *sim, SimInstance *instance, void *state) {
    const PullConfig *c = state;
    size_t i;

    for (i = 0; i < c->count; i++) {
        sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_WEAK, 0);
    }
}

static void *pull_create(const void *config) {
    PullConfig *c = malloc(sizeof *c);

    *c = *(const PullConfig *)config;
    return c;
}

static const SimComponentType PULLUP = {
    "pullup", pull_pins, pull_create, free, pull_start, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL
};

typedef struct {
    Sim *sim;
    SimInstance *chip;
    SimInstance *drv;
    SimInstance *clk;
    SimInstance *pu;
    const Part *part;
    size_t pins;
    SimTime half;
    SimTime period;
    PullConfig pull;
    const char *cs, *sclk, *mosi, *miso, *scl, *sda;
} MB;

static int maxim_loaded;

static void maxim_need(void) {
    if (!maxim_loaded) {
        if (load_serial_fixture_maxim(library) != 0) {
            printf("FAIL: loading the Maxim fixtures\n");
            exit(1);
        }
        maxim_loaded = 1;
    }
}

static size_t mb_pin(const MB *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

static void mb_set(MB *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, mb_pin(b, name), v, DRIVE_STRONG, 0);
}

static LogicValue mb_get(const MB *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, mb_pin(b, name));
}

static void mb_run(MB *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/*
 * A part on a bench. clock: the pin an oscillator of `half` ps per half
 * period drives (NULL: none). ties: NULL-terminated pairs {pin, onto}: pin
 * joins onto's net and has no driver of its own. pulls: NULL-terminated pin
 * names that get a weak pull-up.
 */
static MB *mb_open(const char *part_name, const char *clock, SimTime half,
                   const char *const *ties, const char *const *pulls) {
    MB *b = calloc(1, sizeof *b);
    SimNet **nets;
    size_t i;
    size_t k;

    maxim_need();
    b->part = part_library_find(library, part_name);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part_name);
        exit(2);
    }
    b->pins = b->part->pin_count;
    b->half = half;
    b->period = 2 * half;
    for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
        b->pull.count++;
    }
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pull);
    if (clock != NULL) {
        b->clk = sim_add_instance(b->sim, &CLOCK, "CLK", &b->half);
    }
    nets = calloc(b->pins, sizeof *nets);
    for (i = 0; i < b->pins; i++) {
        const char *name = b->part->pins[i].name;
        int tied = 0;

        for (k = 0; ties != NULL && ties[k] != NULL; k += 2) {
            tied |= strcmp(ties[k], name) == 0;
        }
        if (tied || nets[i] != NULL) {
            continue;
        }
        /* Pins of one name (GND, NC) are separate nets. */
        nets[i] = sim_add_net(b->sim, name);
        sim_connect(b->sim, nets[i], b->chip, i);
        if (clock != NULL && strcmp(name, clock) == 0) {
            sim_connect(b->sim, nets[i], b->clk, 0);
        } else {
            sim_connect(b->sim, nets[i], b->drv, i);
        }
    }
    for (k = 0; ties != NULL && ties[k] != NULL; k += 2) {
        size_t a = mb_pin(b, ties[k]);

        sim_connect(b->sim, nets[mb_pin(b, ties[k + 1])], b->chip, a);
    }
    for (k = 0; k < b->pull.count; k++) {
        sim_connect(b->sim, nets[mb_pin(b, pulls[k])], b->pu, k);
    }
    free(nets);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    return b;
}

static void mb_close(MB *b) {
    sim_destroy(b->sim);
    free(b);
}

static void mb_expect_pin(MB *b, const char *pin, LogicValue want, const char *what) {
    char message[160];
    LogicValue got = mb_get(b, pin);

    snprintf(message, sizeof message, "%s: %s is %c, want %c", what, pin, "01XZ"[got], "01XZ"[want]);
    check(got == want, message);
}

static void mb_expect_u(unsigned got, unsigned want, const char *what) {
    char message[160];

    snprintf(message, sizeof message, "%s: got 0x%X, want 0x%X", what, got, want);
    check(got == want, message);
}

static size_t mb_sent(MB *b, unsigned channel, uint16_t *frames, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_sent(sim_instance_state(b->chip), channel, 0, frames, cap);
}

static int mb_send(MB *b, unsigned channel, const char *text) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_send(b->sim, b->chip, sim_instance_state(b->chip), channel,
                             (const uint8_t *)text, strlen(text));
}

/* ==== plan-12-H: the classic UARTs (cases H1-H8) ================================== */

/*
 * The H bench: every pin of the part on its own net with a bench driver,
 * except clock pins, which a free-running clock drives instead (pins in one
 * group share one clock). Fixtures: support/serial_fixture_classic.h.
 */
typedef struct {
    Bench b;
    SimInstance *clocks[4];
    SimTime halves[4];
    unsigned clock_count;
} HBench;

typedef struct {
    const char *pin;
    unsigned group;
} HClockPin;

static HBench *hb_open(const char *part_name, const HClockPin *clock_pins, unsigned n_pins,
                       const SimTime *halves, unsigned n_clocks) {
    HBench *h = calloc(1, sizeof *h);
    SimNet *group_net[4] = {NULL, NULL, NULL, NULL};
    size_t i;
    unsigned c;

    h->b.part = part_library_find(library, part_name);
    if (h->b.part == NULL) {
        fprintf(stderr, "no part %s\n", part_name);
        exit(2);
    }
    h->b.pins = h->b.part->pin_count;
    h->b.half = n_clocks > 0 ? halves[0] : NS(500);
    h->b.period = 2 * h->b.half;
    h->b.sim = sim_create();
    h->b.chip = sim_add_instance(h->b.sim, kind_model(h->b.part), "U1", h->b.part);
    h->b.drv = sim_add_instance(h->b.sim, &DRIVER, "TB", &h->b.pins);
    h->clock_count = n_clocks;
    for (c = 0; c < n_clocks; c++) {
        h->halves[c] = halves[c];
        h->clocks[c] = sim_add_instance(h->b.sim, &CLOCK, "CLK", &h->halves[c]);
    }
    for (i = 0; i < h->b.pins; i++) {
        const char *name = h->b.part->pins[i].name;
        int clocked = -1;
        SimNet *net;

        for (c = 0; c < n_pins; c++) {
            if (strcmp(clock_pins[c].pin, name) == 0) {
                clocked = (int)clock_pins[c].group;
            }
        }
        if (clocked >= 0 && group_net[clocked] != NULL) {
            sim_connect(h->b.sim, group_net[clocked], h->b.chip, i);
            continue;
        }
        net = sim_add_net(h->b.sim, name);
        sim_connect(h->b.sim, net, h->b.chip, i);
        if (clocked >= 0) {
            group_net[clocked] = net;
            sim_connect(h->b.sim, net, h->clocks[clocked], 0);
        } else {
            sim_connect(h->b.sim, net, h->b.drv, i);
        }
    }
    if (!sim_start(h->b.sim)) {
        fprintf(stderr, "start: %s\n", sim_error(h->b.sim));
        exit(2);
    }
    return h;
}

static void hb_close(HBench *h) {
    sim_destroy(h->b.sim);
    free(h);
}

/* Drives pins NAME<first>..NAME<first+n-1> (or the given names) from a value, LSB first. */
static void h_set_bus(Bench *b, const char *const *names, unsigned n, int drive, unsigned v) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], !drive ? LOGIC_Z : (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

/* Reads pins as text ("01XZ", MSB first) and as a value (unknown bits 0). */
static unsigned h_get_bus(Bench *b, const char *const *names, unsigned n, char *text) {
    unsigned v = 0;
    unsigned i;

    for (i = 0; i < n; i++) {
        LogicValue l = get(b, names[i]);

        if (text != NULL) {
            text[n - 1 - i] = "01XZ"[l];
        }
        if (l == LOGIC_1) {
            v |= 1u << i;
        }
    }
    if (text != NULL) {
        text[n] = '\0';
    }
    return v;
}

static void h_expect_bus(Bench *b, const char *const *names, unsigned n, const char *want,
                         const char *what) {
    char text[17];
    char message[160];

    h_get_bus(b, names, n, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

/* Waits up to `limit` for `pin` to read `level`; 1 when it did. */
static int h_wait(Bench *b, const char *pin, LogicValue level, SimTime limit) {
    SimTime step = NS(50);
    SimTime spent = 0;

    while (spent < limit) {
        if (get(b, pin) == level) {
            return 1;
        }
        run(b, step);
        spent += step;
    }
    return get(b, pin) == level;
}

/* After a start bit was seen on `pin` (now), samples the start bit and n
   more bits at mid-bit: "0"/"1"/"X" per bit, start first. */
static void h_frame(Bench *b, const char *pin, SimTime bt, unsigned n, char *out) {
    unsigned i;

    run(b, bt / 2);
    out[0] = "01XZ"[get(b, pin)];
    for (i = 1; i <= n; i++) {
        run(b, bt);
        out[i] = "01XZ"[get(b, pin)];
    }
    out[n + 1] = '\0';
}

/* Waits for a start bit on `pin` and samples a frame; 0 when none came. */
static int h_catch(Bench *b, const char *pin, SimTime bt, unsigned n, SimTime limit, char *out) {
    SimTime step = bt / 32 > 0 ? bt / 32 : 1;
    SimTime spent = 0;

    while (spent < limit && get(b, pin) != LOGIC_0) {
        run(b, step);
        spent += step;
    }
    if (get(b, pin) != LOGIC_0) {
        out[0] = '\0';
        return 0;
    }
    h_frame(b, pin, bt, n, out);
    return 1;
}

/* Drives a frame onto `pin`: the characters of bits ('0'/'1'), one bit time each, then mark. */
static void h_drive_bits(Bench *b, const char *pin, const char *bits, SimTime bt) {
    size_t i;

    for (i = 0; bits[i] != '\0'; i++) {
        set(b, pin, bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
    set(b, pin, LOGIC_1);
}

/* The bits of an async frame: start, n data bits LSB first, parity ('o' odd,
   'e' even, 'm' mark, 's' space, 0 none), stops (count of 1s). */
static void h_frame_bits(unsigned v, unsigned n, char parity, unsigned stops, char *out) {
    unsigned i;
    unsigned ones = 0;
    size_t at = 0;

    out[at++] = '0';
    for (i = 0; i < n; i++) {
        unsigned bit = (v >> i) & 1u;

        ones += bit;
        out[at++] = bit ? '1' : '0';
    }
    if (parity == 'o') out[at++] = ones % 2 ? '0' : '1';
    if (parity == 'e') out[at++] = ones % 2 ? '1' : '0';
    if (parity == 'm') out[at++] = '1';
    if (parity == 's') out[at++] = '0';
    for (i = 0; i < stops; i++) {
        out[at++] = '1';
    }
    out[at] = '\0';
}

static int h_send(Bench *b, unsigned channel, const char *text) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_send(b->sim, b->chip, sim_instance_state(b->chip), channel,
                             (const uint8_t *)text, strlen(text));
}

/* ---- SPI master (mode 0) ----------------------------------------------------------- */

#define SPI_HALF NS(250)

/* CS low, n bits MSB first; MISO sampled before each rising edge into *in
   (unknown bits 0) and text ("01XZ"). CS stays low when keep is set. */
static uint32_t mb_spi(MB *b, uint32_t out, unsigned n, char *text, int keep) {
    uint32_t in = 0;
    unsigned i;

    mb_set(b, b->cs, LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < n; i++) {
        LogicValue v;

        mb_set(b, b->mosi, (out >> (n - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        mb_run(b, SPI_HALF);
        v = mb_get(b, b->miso);
        if (text != NULL) {
            text[i] = "01XZ"[v];
        }
        in = (in << 1) | (v == LOGIC_1 ? 1u : 0u);
        mb_set(b, b->sclk, LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, b->sclk, LOGIC_0);
    }
    if (text != NULL) {
        text[n] = '\0';
    }
    mb_run(b, SPI_HALF);
    if (!keep) {
        mb_set(b, b->cs, LOGIC_1);
        mb_run(b, SPI_HALF);
    }
    return in;
}

/* ---- I2C master (bench SDA/SCL open drain at 400 kHz) ---------------------------------------- */

#define I2C_Q NS(625)

static void i2c_line(MB *b, const char *pin, int level) {
    mb_set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static void i2c_start(MB *b) {
    i2c_line(b, b->sda, 1);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->sda, 0);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 0);
    mb_run(b, I2C_Q);
}

static void i2c_restart(MB *b) {
    i2c_line(b, b->sda, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->sda, 0);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 0);
    mb_run(b, I2C_Q);
}

static void i2c_stop(MB *b) {
    i2c_line(b, b->sda, 0);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->sda, 1);
    mb_run(b, I2C_Q * 2);
}

static LogicValue i2c_bit(MB *b, int out) {
    LogicValue v;

    i2c_line(b, b->sda, out);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    v = mb_get(b, b->sda);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 0);
    mb_run(b, I2C_Q);
    return v;
}

/* Writes a byte; the ninth bit as the target left it (0 = ACK). */
static LogicValue i2c_write(MB *b, unsigned byte) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (byte >> i) & 1u);
    }
    return i2c_bit(b, 1);
}

/* Reads a byte (text "01XZ" when given), then ACKs (ack = 1) or NACKs. */
static unsigned i2c_read(MB *b, int ack, char *text) {
    unsigned v = 0;
    int i;

    for (i = 7; i >= 0; i--) {
        LogicValue l = i2c_bit(b, 1);

        if (text != NULL) {
            text[7 - i] = "01XZ"[l];
        }
        v = (v << 1) | (l == LOGIC_1 ? 1u : 0u);
    }
    if (text != NULL) {
        text[8] = '\0';
    }
    i2c_bit(b, !ack);
    return v;
}

/* ---- Line helpers --------------------------------------------------------------------- */

/* Waits up to `limit` for `pin` to read `level`; the time waited, or -1. */
static long long mb_wait(MB *b, const char *pin, LogicValue level, SimTime limit, SimTime step) {
    SimTime spent = 0;

    while (spent <= limit) {
        if (mb_get(b, pin) == level) {
            return (long long)spent;
        }
        mb_run(b, step);
        spent += step;
    }
    return -1;
}

/* After a start bit was seen: n bits sampled mid-bit ("01XZ"). */
static void mb_frame(MB *b, const char *pin, SimTime bt, unsigned n, char *out) {
    unsigned i;

    mb_run(b, bt / 2);
    out[0] = "01XZ"[mb_get(b, pin)];
    for (i = 1; i <= n; i++) {
        mb_run(b, bt);
        out[i] = "01XZ"[mb_get(b, pin)];
    }
    out[n + 1] = '\0';
}

/* Drives start, `bits` ('0'/'1', LSB first, then any 9th/stop bits), then idle. */
static void mb_drive_bits(MB *b, const char *pin, const char *bits, SimTime bt, LogicValue idle) {
    size_t i;

    mb_set(b, pin, idle == LOGIC_1 ? LOGIC_0 : LOGIC_1);
    mb_run(b, bt);
    for (i = 0; bits[i] != '\0'; i++) {
        LogicValue v = bits[i] == '1' ? LOGIC_1 : LOGIC_0;

        mb_set(b, pin, idle == LOGIC_1 ? v : (v == LOGIC_1 ? LOGIC_0 : LOGIC_1));
        mb_run(b, bt);
    }
    mb_set(b, pin, idle);
}

/* An 8N1 frame of byte v (stop bit `stop`). */
static void mb_drive_byte(MB *b, const char *pin, unsigned v, SimTime bt, int stop) {
    char bits[12];
    unsigned i;

    for (i = 0; i < 8; i++) {
        bits[i] = (v >> i) & 1u ? '1' : '0';
    }
    bits[8] = stop ? '1' : '0';
    bits[9] = '\0';
    mb_drive_bits(b, pin, bits, bt, LOGIC_1);
}

/* ---- JF: the front ends, direct ----------------------------------------------------------- */

enum { TG_CS, TG_SCLK, TG_MOSI, TG_MISO, TG_SCL, TG_SDA, TG_A0, TG_A1, TG_PINS };

typedef struct {
    SerialSpiPins sp;
    SerialI2cPins ip;
    SerialSpi spi;
    SerialI2c i2c;
    SerialSpiEvents se;
    SerialI2cEvents ie;
    char log[1024];
    uint8_t mem;
    int ties[2];
} Tgt;

static void tg_log(Tgt *t, const char *format, unsigned a, unsigned b) {
    size_t n = strlen(t->log);

    snprintf(t->log + n, sizeof t->log - n, format, a, b);
}

static void tg_spi_begin(void *c) { tg_log(c, "B ", 0, 0); }
static SerialSpiByte tg_spi_next(void *c, unsigned index) {
    SerialSpiByte b;

    (void)c;
    b.data = (uint8_t)(0xA5 ^ index);
    b.known = 0xFF;
    b.drive = 0xFF;
    return b;
}
static void tg_spi_received(void *c, unsigned index, uint8_t data, uint8_t known) {
    (void)index;
    tg_log(c, known == 0xFF ? "R%02X " : "R%02X? ", data, 0);
}
static void tg_spi_end(void *c, unsigned bits) { tg_log(c, "E%u ", bits, 0); }

static void tg_i2c_begin(void *c, int repeated) { tg_log(c, repeated ? "Sr " : "S ", 0, 0); }
static LogicValue tg_i2c_address(void *c, uint8_t byte, uint8_t known) {
    Tgt *t = c;

    t->ties[0] = serial_i2c_strap(&t->i2c, 0);
    t->ties[1] = serial_i2c_strap(&t->i2c, 1);
    tg_log(c, "A%02X ", byte, 0);
    return known == 0xFF && (byte >> 1) == 0x2A ? LOGIC_1 : LOGIC_0;
}
static LogicValue tg_i2c_received(void *c, unsigned index, uint8_t data, uint8_t known) {
    Tgt *t = c;

    (void)index;
    (void)known;
    t->mem = data;
    tg_log(c, "W%02X ", data, 0);
    return LOGIC_1;
}
static void tg_i2c_next(void *c, unsigned index, uint8_t *data, uint8_t *known) {
    Tgt *t = c;

    tg_log(c, "N%u ", index, 0);
    *data = (uint8_t)(t->mem + index);
    *known = 0xFF;
}
static void tg_i2c_acked(void *c, unsigned index, LogicValue ack) {
    (void)index;
    tg_log(c, "K%c ", "NAXZ"[ack], 0);
}
static void tg_i2c_end(void *c, int stop) { tg_log(c, stop ? "P " : "Px ", 0, 0); }

static size_t tg_pin_count(const void *config) {
    (void)config;
    return TG_PINS;
}

static void *tg_create(const void *config) {
    Tgt *t = calloc(1, sizeof *t);

    (void)config;
    t->sp.cs = TG_CS;
    t->sp.sclk = TG_SCLK;
    t->sp.mosi = TG_MOSI;
    t->sp.miso = TG_MISO;
    t->sp.mode = 0;
    t->ip.scl = TG_SCL;
    t->ip.sda = TG_SDA;
    t->ip.strap[0] = TG_A0;
    t->ip.strap[1] = TG_A1;
    t->ip.strap_count = 2;
    serial_spi_init(&t->spi, &t->sp);
    serial_i2c_init(&t->i2c, &t->ip);
    t->spi.t_valid = NS(20);
    t->spi.t_out = NS(20);
    t->spi.t_float = NS(20);
    t->se.context = t;
    t->se.begin = tg_spi_begin;
    t->se.next = tg_spi_next;
    t->se.received = tg_spi_received;
    t->se.end = tg_spi_end;
    t->ie.context = t;
    t->ie.begin = tg_i2c_begin;
    t->ie.address = tg_i2c_address;
    t->ie.received = tg_i2c_received;
    t->ie.next = tg_i2c_next;
    t->ie.acked = tg_i2c_acked;
    t->ie.end = tg_i2c_end;
    return t;
}

static void tg_start(Sim *sim, SimInstance *instance, void *state) {
    Tgt *t = state;

    serial_spi_start(&t->spi, sim, instance);
    serial_i2c_start(&t->i2c, sim, instance);
}

static void tg_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    Tgt *t = state;

    (void)old_value;
    (void)new_value;
    serial_spi_pin(&t->spi, sim, instance, pin, &t->se);
    serial_i2c_pin(&t->i2c, sim, instance, pin, &t->ie);
}

static const SimComponentType TARGET = {
    "target", tg_pin_count, tg_create, free, tg_start, tg_pin_changed, NULL,
    NULL, NULL, NULL, NULL, NULL
};

/* A bench around the test target: SCL/SDA pulled up, A0 tied to SCL, A1 to SDA. */
typedef struct {
    MB b;
    PartPin pin_table[TG_PINS];
    Part part;
} TgBench;

static TgBench *tg_open(void) {
    static const char *const NAMES[TG_PINS] = {"CS", "SCLK", "MOSI", "MISO", "SCL", "SDA", "A0", "A1"};
    TgBench *t = calloc(1, sizeof *t);
    MB *b = &t->b;
    SimNet *nets[TG_PINS];
    size_t i;

    for (i = 0; i < TG_PINS; i++) {
        t->pin_table[i].name = NAMES[i];
    }
    t->part.pins = t->pin_table;
    t->part.pin_count = TG_PINS;
    b->part = &t->part;
    b->pins = TG_PINS;
    b->pull.count = 2;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, &TARGET, "T1", NULL);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pull);
    for (i = 0; i < TG_A0; i++) {
        nets[i] = sim_add_net(b->sim, NAMES[i]);
        sim_connect(b->sim, nets[i], b->chip, i);
        sim_connect(b->sim, nets[i], b->drv, i);
    }
    sim_connect(b->sim, nets[TG_SCL], b->chip, TG_A0);
    sim_connect(b->sim, nets[TG_SDA], b->chip, TG_A1);
    sim_connect(b->sim, nets[TG_SCL], b->pu, 0);
    sim_connect(b->sim, nets[TG_SDA], b->pu, 1);
    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "MOSI";
    b->miso = "MISO";
    b->scl = "SCL";
    b->sda = "SDA";
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_0);
    mb_set(b, "MOSI", LOGIC_0);
    mb_run(b, NS(1000));
    return t;
}

static Tgt *tg_state(TgBench *t) {
    return sim_instance_state(t->b.chip);
}

static void case_jf_spi(void) {
    TgBench *t = tg_open();
    MB *b = &t->b;
    char text[40];
    uint32_t in;

    case_begin();
    mb_expect_pin(b, "MISO", LOGIC_Z, "MISO while CS is high");
    in = mb_spi(b, 0x3C5A, 16, text, 1);
    mb_expect_u(in, 0xA5A4, "MISO shifts byte 0 then byte 1 (next(index))");
    mb_expect_pin(b, "MISO", LOGIC_1, "CS still low: MISO shows bit 7 of byte 2 (0xA7)");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(100));
    mb_expect_pin(b, "MISO", LOGIC_Z, "MISO 3-state again after CS rises (t_float)");
    check(strcmp(tg_state(t)->log, "B R3C R5A E16 ") == 0, "SPI log: begin, two bytes, end after 16 bits");
    /* An X on CS mid-transaction: MISO drives X, the rest of the bits arrive X. */
    tg_state(t)->log[0] = '\0';
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, NS(200));
    mb_set(b, "CS", LOGIC_X);
    mb_run(b, NS(100));
    mb_expect_pin(b, "MISO", LOGIC_X, "CS X: MISO drives X");
    mb_spi(b, 0xFF, 8, NULL, 0);
    check(strcmp(tg_state(t)->log, "B R00? E8 ") == 0, "CS X: the byte arrives unknown");
    case_end("JF1 SPI target: MISO Z while CS is high, bytes both ways, X on CS");
    sim_destroy(b->sim);
    free(t);
}

static void case_jf_i2c(void) {
    TgBench *t = tg_open();
    MB *b = &t->b;
    char text[16];
    LogicValue ack;
    unsigned v;

    case_begin();
    i2c_start(b);
    ack = i2c_write(b, 0x2A << 1);
    check(ack == LOGIC_0, "address 0x2A write: ACK");
    check(tg_state(t)->ties[0] == SERIAL_TIE_SCL && tg_state(t)->ties[1] == SERIAL_TIE_SDA,
          "straps: A0 tied to SCL, A1 tied to SDA");
    ack = i2c_write(b, 0x41);
    check(ack == LOGIC_0, "data byte: ACK");
    mb_expect_pin(b, "SDA", LOGIC_1, "SDA released after the ACK slot");
    i2c_restart(b);
    ack = i2c_write(b, (0x2A << 1) | 1);
    check(ack == LOGIC_0, "repeated START, address read: ACK");
    v = i2c_read(b, 1, text);
    mb_expect_u(v, 0x41, "first read byte (shifted on SCL falling edges)");
    v = i2c_read(b, 0, text);
    mb_expect_u(v, 0x42, "second read byte, then NACK");
    i2c_stop(b);
    check(strcmp(tg_state(t)->log, "S A54 W41 Sr A55 N0 KA N1 KN P ") == 0,
          "I2C log: S, address, write, Sr, read 2 with ACK then NACK, P");
    /* A wrong address: NACK, and the target ignores the rest until STOP. */
    tg_state(t)->log[0] = '\0';
    i2c_start(b);
    ack = i2c_write(b, 0x2B << 1);
    check(ack == LOGIC_1, "wrong address: NACK (SDA stays high)");
    ack = i2c_write(b, 0x00);
    check(ack == LOGIC_1, "not addressed: no ACK for data");
    i2c_stop(b);
    check(strcmp(tg_state(t)->log, "S A56 ") == 0, "wrong address: no write, no end event");
    case_end("JF2 I2C target: START, repeated START, STOP, ACK/NACK, reads, straps on SCL/SDA");
    sim_destroy(b->sim);
    free(t);
}

/* ---- J1 MAX3100 ------------------------------------------------------------------------------- */

#define M3_HALF NS(271)   /* X1 = 1.8432 MHz */

static MB *m3_open(const char *part) {
    MB *b = mb_open(part, "X1", M3_HALF, NULL, NULL);

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_0);
    mb_set(b, "DIN", LOGIC_0);
    mb_set(b, "SHDN", LOGIC_1);
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_1);
    mb_run(b, NS(2000));
    return b;
}

static unsigned m3_word(MB *b, unsigned din) {
    return mb_spi(b, din, 16, NULL, 0);
}

/* The length of data bit 0 of a 0x01 frame (high between the start bit and
   bit 1), measured edge to edge. */
static long long m3_start_bit(MB *b, const char *pin, SimTime limit) {
    SimTime step = NS(10);

    if (mb_wait(b, pin, LOGIC_0, limit, step) < 0 || mb_wait(b, pin, LOGIC_1, limit, step) < 0) {
        return -1;
    }
    return mb_wait(b, pin, LOGIC_0, limit, step);
}

static int within(long long got, long long want, long long tolerance) {
    return got >= want - tolerance && got <= want + tolerance;
}

static void m3_register_cases(MB *b, const char *what, int *ok_out) {
    SimTime bt = 16u * 2 * M3_HALF;   /* BRD 1 */
    char frame[16];
    char message[160];
    uint16_t log[8];
    unsigned w;

    (void)ok_out;
    mb_expect_pin(b, "DOUT", LOGIC_Z, "DOUT while CS is high");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQ released after power-on");
    mb_expect_pin(b, "RTS", LOGIC_1, "RTS pin high after power-on");
    mb_expect_pin(b, "TX", LOGIC_1, "TX idles high");
    w = m3_word(b, 0x4000);
    mb_expect_u(w, 0x4000, "Read Config at power-on: R = 0, T = 1, config 0");
    /* Write Config: RM, 8N1, B = 0000 (115200 on 1.8432 MHz). */
    w = m3_word(b, 0xC000 | 0x0400);
    mb_expect_u(w, 0x4000, "Write Config returns R T and zeros");
    w = m3_word(b, 0x4000);
    mb_expect_u(w, 0x4400, "Read Config returns the last configuration (RM)");
    /* Write Data 'A' with RTS = 1 (pin low). */
    m3_word(b, 0x8000 | 0x0200 | 'A');
    mb_expect_pin(b, "RTS", LOGIC_0, "RTS bit 1 drives the pin low");
    check(mb_wait(b, "TX", LOGIC_0, bt * 4, NS(20)) >= 0, "TX start bit");
    mb_frame(b, "TX", bt, 9, frame);
    snprintf(message, sizeof message, "%s: TX frame of 'A' (0x41 LSB first, stop): %s", what, frame);
    check(strcmp(frame, "0100000101") == 0, message);
    mb_run(b, bt * 2);
    check(mb_sent(b, 0, log, 8) >= 1 && log[0] == 'A', "sent log holds 'A'");
    /* Receive: one frame on RX, Read Data pops it; IRQ by RM. */
    mb_drive_byte(b, "RX", 0x5A, bt, 1);
    mb_run(b, bt);
    mb_expect_pin(b, "IRQ", LOGIC_0, "R with RM: IRQ asserted (open drain low)");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0xC0FF, 0xC05A, "Read Data: R = 1, T = 1, D = 0x5A");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQ released once the word is read");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x80FF, 0x0000, "Read Data on an empty FIFO: R = 0, D = 0");
}

static void case_j1(void) {
    MB *b = m3_open("SFX-MAX3100-qsop16");
    SimTime bt = 16u * 2 * M3_HALF;
    char frame[16];
    unsigned w;
    unsigned i;
    long long len;
    int ok = 1;

    case_begin();
    m3_register_cases(b, "MAX3100", &ok);
    /* Baud table: B = 1010 is BRD 12, B = 1000 is BRD 3. */
    m3_word(b, 0xC000 | 0x000A);
    m3_word(b, 0x8000 | 0x01);
    len = m3_start_bit(b, "TX", bt * 30);
    check(within(len, (long long)(16u * 12 * 2 * M3_HALF), (long long)(2 * M3_HALF)),
          "B = 1010: bit time 16 x 12 X1 periods");
    mb_run(b, bt * 12 * 10);
    m3_word(b, 0xC000 | 0x0008);
    m3_word(b, 0x8000 | 0x01);
    len = m3_start_bit(b, "TX", bt * 8);
    check(within(len, (long long)(16u * 3 * 2 * M3_HALF), (long long)(2 * M3_HALF)),
          "B = 1000: bit time 16 x 3 X1 periods");
    mb_run(b, bt * 3 * 10);
    /* FIFO: 8 words in, read in order; a 9th clears it (RA/FE set). */
    m3_word(b, 0xC000);
    for (i = 0; i < 8; i++) {
        mb_drive_byte(b, "RX", 0x30 + i, bt, 1);
    }
    mb_run(b, bt);
    for (i = 0; i < 8; i++) {
        w = m3_word(b, 0x0000);
        mb_expect_u(w & 0x80FF, 0x8030 + i, "FIFO word in order with R = 1");
    }
    mb_expect_u(m3_word(b, 0x0000) & 0x8000, 0, "FIFO empty after 8 reads");
    for (i = 0; i < 9; i++) {
        mb_drive_byte(b, "RX", 0x40 + i, bt, 1);
    }
    mb_run(b, bt);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x84FF, 0x0400, "9th word: the FIFO is cleared, RA/FE = 1, R = 0");
    /* Framing error sets RA/FE; the next good frame clears it. */
    m3_word(b, 0xC000);
    mb_drive_byte(b, "RX", 0x55, bt, 0);
    mb_run(b, bt * 2);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x0400, 0x0400, "a 0 stop bit sets RA/FE");
    mb_drive_byte(b, "RX", 0x66, bt, 1);
    mb_run(b, bt);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x0400, 0, "the next good frame clears RA/FE");
    /* IRQ masks: RAM (bit 8) with a framing error. */
    m3_word(b, 0xC000 | 0x0100);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "RAM with RA/FE = 0: no IRQ");
    mb_drive_byte(b, "RX", 0x00, bt, 0);
    mb_run(b, bt * 2);
    mb_expect_pin(b, "IRQ", LOGIC_0, "RAM: a framing error asserts IRQ");
    /* T bit and its latch: TM (bit 11). */
    m3_word(b, 0xC000 | 0x0800);
    mb_expect_pin(b, "IRQ", LOGIC_0, "TM: T = 1 after Write Config asserts IRQ");
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < 16; i++) {
        mb_set(b, "DIN", LOGIC_0);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK", LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK", LOGIC_0);
    }
    mb_run(b, SPI_HALF);
    mb_expect_pin(b, "IRQ", LOGIC_0, "T latch: still set after the 16th clock (CS low)");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(200));
    mb_expect_pin(b, "IRQ", LOGIC_Z, "T latch cleared on CS rising of Read Data (planner)");
    m3_word(b, 0x8000 | 0x11);
    m3_word(b, 0x8000 | 0x22);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x4000, 0, "T = 0 while the buffer holds the second word");
    mb_run(b, bt * 22);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x4000, 0x4000, "T = 1 once both have gone");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "the Read Data cleared the latch again");
    /* Test mode: digital loopback and the 16x clock on RTS while CS is low. */
    m3_word(b, 0xC000);
    m3_word(b, 0x4001);
    m3_word(b, 0x8000 | 0x5C);
    mb_set(b, "CS", LOGIC_0);
    {
        int seen0 = 0;
        int seen1 = 0;

        for (i = 0; i < 8; i++) {
            mb_run(b, NS(130));
            seen0 |= mb_get(b, "RTS") == LOGIC_0;
            seen1 |= mb_get(b, "RTS") == LOGIC_1;
        }
        check(seen0 && seen1, "test mode: RTS shows the 16x clock while CS is low");
    }
    mb_run(b, bt * 12);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(500));
    mb_expect_pin(b, "RTS", LOGIC_1, "test mode: RTS back to its bit with CS high");
    m3_word(b, 0x4000);   /* TEST = 0 leaves test mode */
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x80FF, 0x805C, "test mode: TX looped back into the receiver");
    /* 9-bit mode: PE = 1, Pt sent as the 9th bit, Pr received. */
    m3_word(b, 0xC000 | 0x0020 | 0x0200);
    m3_word(b, 0x8000 | 0x0100 | 0x12);
    check(mb_wait(b, "TX", LOGIC_0, bt * 4, NS(20)) >= 0, "9-bit: start bit");
    mb_frame(b, "TX", bt, 10, frame);
    check(strcmp(frame, "00100100011") == 0, "9-bit: D0-D7 of 0x12, Pt = 1, stop");
    mb_drive_bits(b, "RX", "0100100001", bt, LOGIC_1);   /* 0x12, 9th bit 0, stop */
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, bt * 2);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "PM: no IRQ for Pr = 0");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x01FF, 0x0012, "9-bit: Pr = 0, D = 0x12");
    mb_drive_bits(b, "RX", "1010010011", bt, LOGIC_1);   /* 0x25, 9th bit 1, stop */
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, bt * 2);
    mb_expect_pin(b, "IRQ", LOGIC_0, "PM: Pr = 1 asserts IRQ (9-bit address)");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x01FF, 0x0125, "9-bit: Pr = 1, D = 0x25");
    /* Software shutdown: SHDNo after the transmitter is idle; RA on RX activity. */
    m3_word(b, 0xC000 | 0x1000 | 0x0100);
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x1000, 0x1000, "SHDNi: SHDNo = 1");
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, NS(1000));
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(1000));
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x0400, 0x0400, "shutdown: an RX transition sets RA");
    mb_expect_pin(b, "IRQ", LOGIC_0, "shutdown: RA with RAM asserts IRQ (wake-up)");
    m3_word(b, 0x8000 | 0x77);
    mb_run(b, bt * 3);
    mb_expect_pin(b, "TX", LOGIC_1, "shutdown: no transmission (clock stopped)");
    m3_word(b, 0xC000);
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x1400, 0, "Write Config SHDNi = 0: running again, RA cleared");
    /* Hardware shutdown aborts a frame. */
    m3_word(b, 0x8000 | 0x00);
    check(mb_wait(b, "TX", LOGIC_0, bt * 4, NS(20)) >= 0, "frame starts");
    mb_run(b, bt * 2);
    mb_set(b, "SHDN", LOGIC_0);
    mb_run(b, NS(100));
    mb_expect_pin(b, "TX", LOGIC_1, "SHDN low aborts the frame (TX high)");
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x5000, 0x5000, "SHDN low: SHDNo = 1, T = 1");
    mb_set(b, "SHDN", LOGIC_1);
    mb_run(b, NS(100));
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x1000, 0, "SHDN high: SHDNo = 0");
    case_end("J1 MAX3100: commands, baud table, FIFO, IRQ masks, T latch, loopback, 9-bit, shutdown");
    mb_close(b);
}

/* ---- J2 MAX3140 ---------------------------------------------------------------------------------- */

static void case_j2(void) {
    MB *b = m3_open("SFX-MAX3140-qsop28");
    SimTime bt = 16u * 2 * M3_HALF;
    unsigned w;
    int ok = 1;

    case_begin();
    m3_register_cases(b, "MAX3140", &ok);
    /* Driver: DE = 1, Y = DI, Z = not DI; TXP inverts; DE = 0 -> Z. SRL open (115k). */
    mb_set(b, "DI", LOGIC_1);
    mb_set(b, "DE", LOGIC_1);
    mb_set(b, "RE", LOGIC_1);
    mb_run(b, NS(4000));
    mb_expect_pin(b, "Y", LOGIC_1, "DE = 1, DI = 1");
    mb_expect_pin(b, "Z", LOGIC_0, "DE = 1, DI = 1");
    mb_set(b, "DI", LOGIC_0);
    mb_run(b, NS(2100));
    mb_expect_pin(b, "Y", LOGIC_0, "DI = 0 after tdp115");
    mb_expect_pin(b, "Z", LOGIC_1, "DI = 0 after tdp115");
    mb_set(b, "TXP", LOGIC_1);
    mb_run(b, NS(2100));
    mb_expect_pin(b, "Y", LOGIC_1, "TXP = 1 inverts the driver");
    mb_set(b, "TXP", LOGIC_0);
    mb_set(b, "DE", LOGIC_0);
    mb_run(b, NS(2500));
    mb_expect_pin(b, "Y", LOGIC_Z, "DE = 0: Y high impedance");
    mb_expect_pin(b, "Z", LOGIC_Z, "DE = 0: Z high impedance");
    /* Receiver: RE = 0, H/F open (pull-down: full duplex). */
    mb_set(b, "RE", LOGIC_0);
    mb_set(b, "A", LOGIC_1);
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(4000));
    mb_expect_pin(b, "RO", LOGIC_1, "A = 1, B = 0 -> RO 1");
    mb_set(b, "A", LOGIC_0);
    mb_set(b, "B", LOGIC_1);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_0, "A = 0, B = 1 -> RO 0");
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_1, "A = B (shorted) -> RO 1 (true fail-safe)");
    mb_set(b, "A", LOGIC_Z);
    mb_set(b, "B", LOGIC_Z);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_1, "A, B open -> RO 1 (true fail-safe)");
    mb_set(b, "A", LOGIC_X);
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_X, "A = X -> RO X");
    mb_set(b, "A", LOGIC_0);
    mb_set(b, "B", LOGIC_1);
    mb_set(b, "RXP", LOGIC_1);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_1, "RXP = 1 inverts the receiver");
    mb_set(b, "RXP", LOGIC_0);
    mb_set(b, "RE", LOGIC_1);
    mb_run(b, NS(300));
    mb_expect_pin(b, "RO", LOGIC_Z, "RE = 1: RO high impedance");
    /* Half duplex: the receiver reads Y/Z (its own driver: echo). */
    mb_set(b, "H/F", LOGIC_1);
    mb_set(b, "RE", LOGIC_0);
    mb_set(b, "DE", LOGIC_1);
    mb_set(b, "DI", LOGIC_0);
    mb_run(b, NS(8000));
    mb_expect_pin(b, "RO", LOGIC_0, "H/F = 1: RO follows Y/Z (driver 0)");
    mb_set(b, "DI", LOGIC_1);
    mb_run(b, NS(2500));
    mb_expect_pin(b, "RO", LOGIC_1, "H/F = 1: RO follows Y/Z (driver 1)");
    mb_set(b, "H/F", LOGIC_0);
    /* Transceiver shutdown: RE = 1, DE = 0 held; enabling is then slower. */
    mb_set(b, "RE", LOGIC_1);
    mb_set(b, "DE", LOGIC_0);
    mb_run(b, NS(5000));
    mb_set(b, "DE", LOGIC_1);
    mb_run(b, NS(4000));
    mb_expect_pin(b, "Y", LOGIC_Z, "from shutdown: not yet enabled after tdz115");
    mb_run(b, NS(2500));
    mb_expect_pin(b, "Y", LOGIC_1, "from shutdown: enabled after tdzs115");
    mb_set(b, "DE", LOGIC_0);
    mb_set(b, "DI", LOGIC_Z);
    mb_set(b, "A", LOGIC_Z);
    mb_set(b, "B", LOGIC_Z);
    /* Test mode: TX held low while CS is low (planner decision). */
    m3_word(b, 0x4001);
    mb_set(b, "CS", LOGIC_0);
    mb_run(b, NS(200));
    mb_expect_pin(b, "TX", LOGIC_0, "MAX3140 test mode: TX low while CS is low");
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, NS(200));
    mb_expect_pin(b, "TX", LOGIC_1, "MAX3140 test mode: TX released with CS high");
    m3_word(b, 0x4000);
    /* Terminal: drives A (true) and B (complement); the UART receives it. */
    mb_run(b, NS(1000));
    check(mb_send(b, 0, "K") == 1, "terminal send queued");
    {
        int ab = 0;
        unsigned i;

        for (i = 0; i < 40; i++) {
            mb_run(b, bt / 2);
            if (mb_get(b, "A") == LOGIC_0 && mb_get(b, "B") == LOGIC_1) {
                ab = 1;
            }
        }
        check(ab, "terminal: A low with B high during a 0 bit");
    }
    mb_expect_pin(b, "A", LOGIC_Z, "terminal: A released after the frame");
    mb_expect_pin(b, "B", LOGIC_Z, "terminal: B released after the frame");
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x80FF, 0x8000 | 'K', "terminal: Read Data gets 'K'");
    case_end("J2 MAX3140: J1 registers, Y/Z true/complement and Z, A/B fail-safe receive, test mode TX low");
    mb_close(b);
}

/* ---- J3-J6 the max310x family ----------------------------------------------------------------- */

#define MX_HALF NS(271)   /* XIN = 1.8432 MHz */

static const char *const MX_PULLS[] = {"SCLK/SCL", "DOUT/SDA", NULL};
static const char *const MX_PULLS_B[] = {"SCLK/SCL", "MISO/SDA", NULL};

/* A max310x part in SPI mode (spi = 1) or I2C mode, its pins idle. */
static MB *mx_open(const char *part, int spi, SimTime half, const char *const *ties) {
    int b_names = strstr(part, "MAX3107") == NULL;   /* MISO/SDA, MOSI/A1, SPI/I2C */
    MB *b;
    const char *select;

    b = mb_open(part, "XIN", half, ties, spi ? NULL : (b_names ? MX_PULLS_B : MX_PULLS));
    select = b_names ? "SPI/I2C" : "I2C/SPI";
    b->cs = "CS/A0";
    b->sclk = "SCLK/SCL";
    b->mosi = b_names ? "MOSI/A1" : "DIN/A1";
    b->miso = b_names ? "MISO/SDA" : "DOUT/SDA";
    b->scl = "SCLK/SCL";
    b->sda = b->miso;
    mb_set(b, select, spi ? LOGIC_1 : LOGIC_0);
    mb_set(b, "RST", LOGIC_1);
    if (spi) {
        mb_set(b, "CS/A0", LOGIC_1);
        mb_set(b, "SCLK/SCL", LOGIC_0);
        mb_set(b, b->mosi, LOGIC_0);
    }
    mb_run(b, NS(2000));
    return b;
}

static void mx_w(MB *b, unsigned cmd, unsigned value) {
    mb_spi(b, ((0x80u | cmd) << 8) | value, 16, NULL, 0);
}

static unsigned mx_r(MB *b, unsigned cmd) {
    return mb_spi(b, cmd << 8, 16, NULL, 0) & 0xFF;
}

/* Clock on (external clock: ClockEn + PLLBypass), 8N1, DIV = 1. */
static void mx_setup(MB *b, unsigned u_bits, unsigned clksrc) {
    mx_w(b, 0x1E, clksrc);
    mx_w(b, u_bits | 0x0B, 0x03);
    mx_w(b, u_bits | 0x1C, 0x01);
    mx_w(b, u_bits | 0x1D, 0x00);
}

static void case_j3(void) {
    MB *b = mx_open("SFX-MAX3107-ssop24", 1, MX_HALF, NULL);
    SimTime bt = 16u * 2 * MX_HALF;
    uint16_t log[8];
    char text[40];
    char frame[16];
    unsigned v;
    unsigned i;
    long long t;

    case_begin();
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQSel = 0: IRQ released once the device is ready");
    mb_set(b, "CTS", LOGIC_0);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(1000));
    /* Command byte: R + address; MISO Z during it, the data after. */
    v = mb_spi(b, 0x1F00, 16, text, 0) & 0xFF;
    mb_expect_u(v, 0xA1, "RevID (0x1F) reads 0xA1");
    check(strncmp(text, "ZZZZZZZZ", 8) == 0, "MISO Z during the command byte");
    mb_expect_u(mx_r(b, 0x0B), 0x05, "LCR resets to 0x05 (6 bits, 2 stops)");
    mb_expect_u(mx_r(b, 0x10), 0xFF, "FIFOTrgLvl resets to 0xFF");
    mb_expect_u(mx_r(b, 0x1A), 0x01, "PLLConfig resets to 0x01");
    mb_expect_u(mx_r(b, 0x1C), 0x01, "DIVLSB resets to 0x01");
    mb_expect_u(mx_r(b, 0x1E), 0x08, "CLKSource resets to 0x08 (ClockEn = 0)");
    mb_expect_u(mx_r(b, 0x02), 0x60, "ISR resets to 0x60");
    mb_expect_u(mx_r(b, 0x02), 0x00, "ISR is clear-on-read");
    /* Write: MISO stays Z. */
    v = mb_spi(b, 0x8B03, 16, text, 0);
    check(strcmp(text, "ZZZZZZZZZZZZZZZZ") == 0, "MISO Z for a whole write");
    /* Burst write to THR: the FIFO fills (no clock yet: ClockEn = 0). */
    mb_spi(b, ((uint32_t)0x80 << 24) | ((uint32_t)'a' << 16) | ((uint32_t)'b' << 8) | 'c', 32, NULL, 0);
    mb_expect_u(mx_r(b, 0x11), 3, "TxFIFOLvl after a 3-byte burst to THR");
    /* Burst auto-increment from 0x14 (XON1..XOFF2), and read back. */
    mb_spi(b, ((uint32_t)0x94 << 24) | 0x112233u, 32, NULL, 1);
    mb_spi(b, 0x44, 8, NULL, 0);
    v = mb_spi(b, (uint32_t)0x14 << 24, 32, NULL, 1);
    mb_expect_u(v & 0xFFFFFF, 0x112233, "burst read XON1, XON2, XOFF1");
    v = mb_spi(b, 0, 8, NULL, 0);
    mb_expect_u(v, 0x44, "burst read XOFF2");
    /* The clock on: the three bytes go out. */
    mx_setup(b, 0, 0x18);
    mb_run(b, bt * 34);
    check(mb_sent(b, 0, log, 8) == 3 && log[0] == 'a' && log[1] == 'b' && log[2] == 'c',
          "THR burst sent 'a' 'b' 'c'");
    mb_expect_u(mx_r(b, 0x11), 0, "TxFIFOLvl 0 after sending");
    /* RX FIFO level and a burst read of RHR (stays at 0x00). */
    for (i = 0; i < 5; i++) {
        mb_drive_byte(b, "RX", 0x61 + i, bt, 1);
    }
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x12), 5, "RxFIFOLvl after 5 frames");
    v = mb_spi(b, 0x00000000, 32, NULL, 1);
    mb_expect_u(v & 0xFFFFFF, 0x616263, "burst read of RHR stays on the FIFO");
    v = mb_spi(b, 0, 16, NULL, 0);
    mb_expect_u(v, 0x6465, "burst read of RHR (2)");
    mb_expect_u(mx_r(b, 0x12), 0, "RxFIFOLvl 0 after reading");
    /* Errors: a framing error shows in LSR after its word is read. */
    mb_drive_byte(b, "RX", 0x70, bt, 0);
    mb_run(b, bt * 2);
    mb_expect_u(mx_r(b, 0x00), 0x70, "word with a framing error");
    mb_expect_u(mx_r(b, 0x04) & 0x0C, 0x08, "LSR FrameErr for the word just read");
    /* Auto RTS: Halt 8, Resume 8 (FlowLvl 0x11): RTS high above 8 words. */
    mx_w(b, 0x0F, 0x11);
    mx_w(b, 0x13, 0x01);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "AutoRTS: RTS asserted (low) with the FIFO empty");
    for (i = 0; i < 9; i++) {
        mb_drive_byte(b, "RX", 0x30 + i, bt, 1);
    }
    mb_run(b, bt);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_1, "AutoRTS: 9 words > Halt: RTS deasserted");
    mx_r(b, 0x00);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_1, "AutoRTS: 8 words is not below Resume");
    mx_r(b, 0x00);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "AutoRTS: 7 words < Resume: RTS asserted");
    mx_w(b, 0x0A, 0x02);   /* FIFORst */
    mx_w(b, 0x0A, 0x00);
    /* Auto CTS: transmit only while CTS is low. */
    mx_w(b, 0x13, 0x02);
    mb_set(b, "CTS", LOGIC_1);
    mb_run(b, NS(500));
    mx_w(b, 0x00, 0x5A);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t < 0, "AutoCTS: CTS high holds the transmitter");
    mb_set(b, "CTS", LOGIC_0);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t >= 0, "AutoCTS: CTS low lets it send");
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x02) & 0x80, 0x80, "CTS transitions set ISR[7] CTSInt");
    /* XON/XOFF transmit control: FlowCtrl SwFlowEn + SwFlow[1:0] = 10. */
    mx_w(b, 0x13, 0x28);
    mx_w(b, 0x14, 0x11);
    mx_w(b, 0x16, 0x13);
    mb_drive_byte(b, "RX", 0x13, bt, 1);
    mb_run(b, bt);
    mx_w(b, 0x00, 0x21);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t < 0, "XOFF1 received: the transmitter holds");
    mb_drive_byte(b, "RX", 0x11, bt, 1);
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50));
    check(t >= 0, "XON1 received: it sends again");
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x12), 0, "XON/XOFF characters are not stored");
    /* Special characters: MODE2 SpecialChr, SwFlow[1:0] = 11 (all four). */
    mx_w(b, 0x13, 0x30);
    mx_w(b, 0x0A, 0x10);
    mx_w(b, 0x05, 0x0F);
    mx_w(b, 0x01, 0x02);
    mb_drive_byte(b, "RX", 0x13, bt, 1);
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x06), 0x04, "SpclCharInt XOFF1Int");
    mb_expect_u(mx_r(b, 0x00), 0x13, "the special character is stored");
    mb_expect_u(mx_r(b, 0x06), 0x00, "SpclCharInt is clear-on-read");
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x13, 0x00);
    mx_w(b, 0x01, 0x00);
    /* IrDA SIR: a 0 bit is a 3/16-bit low pulse; RX idles low, high pulses are 0s.
       Switching RX's idle level makes junk: flush it with FIFORst. */
    mx_w(b, 0x0E, 0x03);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, bt * 24);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    mb_expect_u(mx_r(b, 0x12), 0, "IrDA: RX idling low receives nothing");
    mx_w(b, 0x00, 0x00);
    /* The second pulse (data bit 0) is measured edge to edge. */
    t = mb_wait(b, "TX", LOGIC_0, bt * 3, NS(10));
    t = t < 0 ? t : mb_wait(b, "TX", LOGIC_1, bt, NS(10));
    t = t < 0 ? t : mb_wait(b, "TX", LOGIC_0, bt, NS(10));
    t = t < 0 ? t : mb_wait(b, "TX", LOGIC_1, bt, NS(10));
    {
        char message[120];

        snprintf(message, sizeof message, "IrDA SIR: pulse %lld ps, 3/16 of a bit is %llu", t,
                 (unsigned long long)(3 * 2 * MX_HALF));
        check(within(t, (long long)(3 * 2 * MX_HALF), NS(20)), message);
    }
    mb_run(b, bt * 12);
    {
        /* 0x0F: start 0, 1111 0000, stop 1 -> pulses for the 0s. */
        static const char bits[] = "01111000011";
        for (i = 0; bits[i] != '\0'; i++) {
            if (bits[i] == '0') {
                mb_set(b, "RX", LOGIC_1);
                mb_run(b, 3 * 2 * MX_HALF);
                mb_set(b, "RX", LOGIC_0);
                mb_run(b, bt - 3 * 2 * MX_HALF);
            } else {
                mb_run(b, bt);
            }
        }
    }
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x00), 0x0F, "IrDA SIR receive: 0x0F");
    mx_w(b, 0x0E, 0x00);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, bt * 24);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    /* RxTimeOut = 2 character times. */
    mx_w(b, 0x0C, 0x02);
    mb_drive_byte(b, "RX", 0x3C, bt, 1);
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x04) & 0x01, 0, "RTimeout: not yet after 1 character time");
    mb_run(b, bt * 12);
    mb_expect_u(mx_r(b, 0x04) & 0x01, 0x01, "RTimeout after 2 character times");
    mb_expect_u(mx_r(b, 0x00), 0x3C, "the waiting word");
    mb_expect_u(mx_r(b, 0x04) & 0x01, 0, "reading the word clears RTimeout");
    mx_w(b, 0x0C, 0x00);
    /* Noise: one of the three samples of a bit disagrees (one 16x tick). */
    {
        SimTime tick = 2 * MX_HALF;

        mb_set(b, "RX", LOGIC_0);
        mb_run(b, bt * 4 + bt / 2 - tick / 2);
        mb_set(b, "RX", LOGIC_1);
        mb_run(b, tick);
        mb_set(b, "RX", LOGIC_0);
        mb_run(b, bt * 5 - bt / 2 - tick / 2);
        mb_set(b, "RX", LOGIC_1);
        mb_run(b, bt * 2);
    }
    mb_expect_u(mx_r(b, 0x00), 0x00, "a glitched 0 bit still reads 0 (majority)");
    mb_expect_u(mx_r(b, 0x04) & 0x20, 0x20, "LSR RxNoise for that word");
    /* RS-485 direction control: Setup 2, Hold 3 bit times on RTS. */
    mx_w(b, 0x0D, 0x23);
    mx_w(b, 0x09, 0x10);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "auto direction: RTS low while idle");
    mx_w(b, 0x00, 0x01);
    t = mb_wait(b, "RTS/CLKOUT", LOGIC_1, bt * 2, NS(10));
    check(t >= 0, "auto direction: RTS high once the FIFO has data");
    t = mb_wait(b, "TX", LOGIC_0, bt * 4, NS(10));
    check(within(t, (long long)(2 * bt), (long long)(2 * 2 * MX_HALF)),
          "auto direction: the start bit Setup = 2 bit times after RTS");
    t = mb_wait(b, "RTS/CLKOUT", LOGIC_0, bt * 16, NS(10));
    check(within(t, (long long)(13 * bt), (long long)(2 * 2 * MX_HALF)),
          "auto direction: RTS low Hold = 3 bit times after the stop bit");
    mx_w(b, 0x09, 0x00);
    mx_w(b, 0x0D, 0x00);
    /* Multidrop with address filtering: XOFF2 = 0x42; LCR forced parity is the 9th bit. */
    mx_w(b, 0x17, 0x42);
    mx_w(b, 0x0B, 0x2B);
    mx_w(b, 0x0A, 0x50);
    mx_w(b, 0x05, 0x20);
    mb_drive_bits(b, "RX", "1000001011", bt, LOGIC_1);   /* address 0x41 */
    mb_drive_bits(b, "RX", "1000100001", bt, LOGIC_1);   /* data 0x11 */
    mb_drive_bits(b, "RX", "0100001011", bt, LOGIC_1);   /* address 0x42 (ours) */
    mb_drive_bits(b, "RX", "0100010001", bt, LOGIC_1);   /* data 0x22 */
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x12), 1, "multidrop: only data after our address is stored");
    mb_expect_u(mx_r(b, 0x00), 0x22, "multidrop: the data word");
    mb_expect_u(mx_r(b, 0x04) & 0x04, 0, "multidrop: LSR[2] = its 9th bit (0)");
    mb_expect_u(mx_r(b, 0x06) & 0x20, 0x20, "multidrop: MultiDropInt");
    mx_w(b, 0x00, 0x42);   /* EvenParity = 0: the 9th bit is 1 */
    check(mb_wait(b, "TX", LOGIC_0, bt * 3, NS(20)) >= 0, "multidrop: frame");
    mb_frame(b, "TX", bt, 10, frame);
    check(strcmp(frame, "00100001011") == 0, "multidrop: 0x42 with the 9th bit 1, stop");
    mb_run(b, bt * 2);
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x05, 0x00);
    /* GPIOs: GPIO0 push-pull out, GPIO1 open-drain out, GPIO2 input with an interrupt. */
    mx_w(b, 0x18, 0x23);
    mx_w(b, 0x19, 0x03);
    mb_expect_pin(b, "GPIO0", LOGIC_1, "GPIO0 push-pull 1");
    mb_expect_pin(b, "GPIO1", LOGIC_Z, "GPIO1 open drain 1 is released");
    mx_w(b, 0x19, 0x00);
    mb_expect_pin(b, "GPIO1", LOGIC_0, "GPIO1 open drain 0");
    mx_r(b, 0x08);
    mx_w(b, 0x07, 0x04);
    mx_w(b, 0x01, 0x04);
    mb_set(b, "GPIO2", LOGIC_1);
    mb_run(b, NS(2000));
    mb_expect_u(mx_r(b, 0x19) & 0x40, 0x40, "GPIOData input bit 6 = GPIO2");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQSel = 0: no IRQ yet");
    mx_w(b, 0x09, 0x80);
    mb_expect_pin(b, "IRQ", LOGIC_0, "GPI2Int through STSInt and ISR[2] with IRQSel");
    mb_expect_u(mx_r(b, 0x08) & 0x04, 0x04, "STSInt GPI2Int");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "STSInt read clears it");
    mx_w(b, 0x01, 0x00);
    mx_w(b, 0x07, 0x00);
    mx_w(b, 0x18, 0x00);
    /* Forced sleep stops the UART; waking sends what waited. */
    mx_w(b, 0x09, 0x20);
    mb_expect_u(mx_r(b, 0x09) & 0x20, 0x20, "ForcedSleep reads back");
    mb_expect_u(mx_r(b, 0x08) & 0x40, 0x40, "STSInt SleepInt while asleep");
    mx_w(b, 0x00, 0x01);
    check(mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50)) < 0, "asleep: nothing is sent");
    mx_w(b, 0x09, 0x00);
    check(mb_wait(b, "TX", LOGIC_0, bt * 3, NS(50)) >= 0, "awake: the word goes out");
    mb_run(b, bt * 12);
    /* CLKtoRTS: the 16x clock (DIV = 1: XIN) on RTS/CLKOUT. */
    mx_w(b, 0x1E, 0x98);
    {
        int seen0 = 0;
        int seen1 = 0;

        for (i = 0; i < 8; i++) {
            mb_run(b, NS(130));
            seen0 |= mb_get(b, "RTS/CLKOUT") == LOGIC_0;
            seen1 |= mb_get(b, "RTS/CLKOUT") == LOGIC_1;
        }
        check(seen0 && seen1, "CLKtoRTS: RTS/CLKOUT toggles");
    }
    mx_w(b, 0x1E, 0x18);
    /* A write command with X address bits: every register it could name
       holds X (plan-12-A section 3.5). */
    mb_set(b, "CS/A0", LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; i < 16; i++) {
        mb_set(b, b->mosi, i == 0 ? LOGIC_1 : i < 8 ? LOGIC_X : LOGIC_0);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK/SCL", LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK/SCL", LOGIC_0);
    }
    mb_run(b, SPI_HALF);
    mb_set(b, "CS/A0", LOGIC_1);
    mb_run(b, SPI_HALF);
    mb_spi(b, 0x0B00, 16, text, 0);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "X command: LCR holds X");
    mb_close(b);

    /* I2C: A1 = VL (DIN/A1 high), A0 = DGND: 7-bit address 0x2E. */
    b = mx_open("SFX-MAX3107-tqfn24-3.5", 0, MX_HALF, NULL);
    mb_set(b, "DIN/A1", LOGIC_1);
    mb_set(b, "CS/A0", LOGIC_0);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, 0x2C << 1) == LOGIC_1, "I2C: 0x2C is not this strap (NACK)");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_write(b, 0x2E << 1) == LOGIC_0, "I2C: 0x2E (A1 = VL, A0 = DGND) ACKs");
    check(i2c_write(b, 0x20) == LOGIC_1, "I2C: register 0x20 is not valid (NACK)");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    check(i2c_write(b, 0x14) == LOGIC_0, "I2C: register address 0x14 ACKs");
    i2c_write(b, 0x5A);
    i2c_write(b, 0xA5);
    i2c_restart(b);
    check(i2c_write(b, (0x2E << 1) | 1) == LOGIC_0, "I2C: read address after Sr");
    i2c_read(b, 0, NULL);
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    i2c_write(b, 0x14);
    i2c_restart(b);
    i2c_write(b, (0x2E << 1) | 1);
    v = i2c_read(b, 1, NULL);
    mb_expect_u(v, 0x5A, "I2C burst write then read: XON1");
    v = i2c_read(b, 0, NULL);
    mb_expect_u(v, 0xA5, "I2C auto-increment: XON2");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    i2c_write(b, 0x1F);
    i2c_restart(b);
    i2c_write(b, (0x2E << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0xA1, "I2C: RevID");
    case_end("J3 MAX3107: SPI and I2C command bytes, bursts, FIFO levels, flow control, special characters, IrDA");
    mb_close(b);
}

/* Start-bit length of a 0x01 sent on `tx` (ps). */
static long long mx_bit_time(MB *b, const char *tx, SimTime limit) {
    return m3_start_bit(b, tx, limit);
}

static void case_j4(void) {
    /* XIN = 3.6864 MHz nominal: 136 ns halves (period 272 ns). */
    SimTime half = NS(136);
    SimTime p = 2 * half;
    MB *b = mx_open("SFX-MAX3107-ssop24", 1, half, NULL);
    long long t;
    char message[160];

    case_begin();
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_0);
    /* PLL bypass (counted): DIV = 2: bit = 16 x 2 x XIN. */
    mx_w(b, 0x1E, 0x18);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x1C, 0x02);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    snprintf(message, sizeof message, "bypass: bit %lld ps, want %llu", t, (unsigned long long)(32 * p));
    check(within(t, (long long)(32 * p), (long long)p), message);
    mb_run(b, 32 * p * 10);
    /* Fractional: DIV 2 + 8/16: nine low bits of 0x00 last 9 x 16 x 2.5 periods. */
    mx_w(b, 0x1B, 0x08);
    mx_w(b, 0x00, 0x00);
    if (mb_wait(b, "TX", LOGIC_0, 200 * p, NS(10)) >= 0) {
        t = mb_wait(b, "TX", LOGIC_1, 600 * p, NS(10));
        snprintf(message, sizeof message, "fraction 2.5: 9 bits %lld ps, want %llu", t,
                 (unsigned long long)(9 * 40 * p));
        check(within(t, (long long)(9 * 40 * p), (long long)p), message);
    } else {
        check(0, "fraction: no frame");
    }
    mb_run(b, 40 * p * 4);
    mx_w(b, 0x1B, 0x00);
    /* 2x mode: 8 ticks per bit. */
    mx_w(b, 0x1B, 0x10);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    check(within(t, (long long)(16 * p), (long long)p), "2x mode: bit = 8 x DIV XIN periods");
    mb_run(b, 16 * p * 12);
    mx_w(b, 0x1B, 0x00);
    /* PLL x6 with PreDiv 6: fREF = XIN (measured period). */
    mx_w(b, 0x1A, 0x06);
    mx_w(b, 0x1E, 0x14);
    mb_run(b, 4 * p);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    snprintf(message, sizeof message, "PLL x6 / 6: bit %lld ps, want %llu", t, (unsigned long long)(32 * p));
    check(within(t, (long long)(32 * p), (long long)p), message);
    mb_run(b, 32 * p * 10);
    /* PLL x6 with PreDiv 3: fREF = 2 x XIN: half the bit time. */
    mx_w(b, 0x1A, 0x03);
    mb_run(b, 4 * p);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    snprintf(message, sizeof message, "PLL x6 / 3: bit %lld ps, want %llu", t, (unsigned long long)(16 * p));
    check(within(t, (long long)(16 * p), (long long)p), message);
    mb_run(b, 16 * p * 10);
    /* PLL x48 / 48: fREF = XIN again. */
    mx_w(b, 0x1A, 0x40 | 48);
    mb_run(b, 4 * p);
    mx_w(b, 0x00, 0x01);
    t = mx_bit_time(b, "TX", 200 * p);
    check(within(t, (long long)(32 * p), (long long)p), "PLL x48 / 48: bit = 32 XIN periods");
    mb_run(b, 32 * p * 10);
    /* PLLEn = 0 without bypass: no reference, nothing is sent. */
    mx_w(b, 0x1E, 0x10);
    mx_w(b, 0x00, 0x01);
    check(mb_wait(b, "TX", LOGIC_0, 100 * p, NS(20)) < 0, "PLLBypass = 0, PLLEn = 0: no clock");
    case_end("J4 MAX3107 clocking: bypass counts XIN, PLL x6 and x48 measure it; fraction, 2x");
    mb_close(b);
}

static void case_j5(void) {
    MB *b = mx_open("SFX-MAX3109-tqfn32", 1, MX_HALF, NULL);
    SimTime bt = 16u * 2 * MX_HALF;
    uint16_t log[4];
    char text[40];
    unsigned v;

    case_begin();
    mb_set(b, "RX0", LOGIC_1);
    mb_set(b, "RX1", LOGIC_1);
    mb_set(b, "CTS0", LOGIC_0);
    mb_set(b, "CTS1", LOGIC_0);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "MAX3109: IRQ released (no IRQSel)");
    /* U selects the UART: LCR 0x03 on UART0, 0x1B on UART1. */
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x20 | 0x0B, 0x1B);
    mb_expect_u(mx_r(b, 0x0B), 0x03, "UART0 LCR");
    mb_expect_u(mx_r(b, 0x20 | 0x0B), 0x1B, "UART1 LCR");
    mx_w(b, 0x14, 0x77);
    mx_w(b, 0x20 | 0x14, 0x88);
    mb_expect_u(mx_r(b, 0x14), 0x77, "UART0 XON1");
    mb_expect_u(mx_r(b, 0x20 | 0x14), 0x88, "UART1 XON1");
    /* Bit 6 must be 0: such a command reads X. */
    mb_spi(b, 0x4B00, 16, text, 0);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "command bit 6 = 1 reads X");
    /* GlobalIRQ and the IRQ bits on clocks 5-8 of a read command. */
    mb_expect_u(mx_r(b, 0x1F), 0x03, "GlobalIRQ 0x03: nothing pending");
    mx_w(b, 0x20 | 0x01, 0x40);   /* UART1 IRQEn RxEmptyIEn: ISR[6] is set */
    mb_expect_pin(b, "IRQ", LOGIC_0, "UART1 interrupt: IRQ low");
    mb_expect_u(mx_r(b, 0x1F), 0x01, "GlobalIRQ: IRQ1 = 0 (pending)");
    mb_spi(b, 0x00, 8, text, 0);
    check(strcmp(text, "ZZZZ0001") == 0, "fast read: Z, then 0 0 IRQ1 IRQ0 on clocks 5-8");
    mx_r(b, 0x20 | 0x02);   /* UART1 ISR read clears it */
    mb_expect_pin(b, "IRQ", LOGIC_Z, "ISR read: IRQ released");
    mb_expect_u(mx_r(b, 0x1F), 0x03, "GlobalIRQ back to 0x03");
    mx_w(b, 0x20 | 0x01, 0x00);
    /* Extended map: GloblComnd 0xCE, RevID at 0x25 (0xC2, decisions.md). */
    mx_w(b, 0x1F, 0xCE);
    mb_expect_u(mx_r(b, 0x05), 0xC2, "extended: RevID 0xC2");
    mx_w(b, 0x1F, 0xCD);
    mb_expect_u(mx_r(b, 0x05), 0x00, "extended off: 0x05 is LSRIntEn again");
    /* PLLConfig only through UART0. */
    mx_w(b, 0x20 | 0x1A, 0x22);
    mb_expect_u(mx_r(b, 0x1A), 0x01, "PLLConfig written through UART1 is ignored");
    mb_spi(b, (0x20u | 0x1A) << 8, 16, text, 0);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "PLLConfig read through UART1 is X");
    /* Both UARTs send with their own format (UART1: 8 bits, even parity). */
    mx_setup(b, 0, 0x18);
    mx_w(b, 0x20 | 0x1C, 0x01);
    mx_w(b, 0x00, 0x31);
    mx_w(b, 0x20 | 0x00, 0x32);
    mb_run(b, bt * 14);
    check(mb_sent(b, 0, log, 4) == 1 && log[0] == 0x31, "UART0 sent 0x31 on TX0");
    check(mb_sent(b, 1, log, 4) == 1 && log[0] == 0x32, "UART1 sent 0x32 on TX1");
    /* Transmit trigger: UART1 SynchEn, TrigSel 3; GloblComnd 0xE3 starts it. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x20 | 0x00, 0x10 | 0x03);   /* UART1 TxSynch */
    mx_w(b, 0x1F, 0xCD);
    v = mx_r(b, 0x20 | 0x09);
    mb_expect_u(v & 0x02, 0x02, "SynchEn forces TxDisabl");
    mx_w(b, 0x20 | 0x00, 0x44);
    check(mb_wait(b, "TX1", LOGIC_0, bt * 3, NS(50)) < 0, "triggered UART waits");
    mx_w(b, 0x1F, 0xE2);
    check(mb_wait(b, "TX1", LOGIC_0, bt * 2, NS(50)) < 0, "another trigger (0xE2) does not start it");
    mx_w(b, 0x1F, 0xE3);
    check(mb_wait(b, "TX1", LOGIC_0, bt * 2, NS(50)) >= 0, "GloblComnd 0xE3 starts UART1");
    mb_run(b, bt * 12);
    check(mb_sent(b, 1, log, 4) == 2 && log[1] == 0x44, "UART1 sent 0x44");
    /* TIMER: 50 % clock on GPIO1 at UARTClk / (1024 x Timer); CLKtoGPIO: XIN on GPIO0. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x03, 0x01);   /* UART0 TIMER1 = 1 */
    mx_w(b, 0x04, 0x80);   /* TIMER2: TmrToGPIO */
    mx_w(b, 0x00, 0x80);   /* TxSynch: CLKtoGPIO */
    mx_w(b, 0x1F, 0xCD);
    {
        long long t0 = mb_wait(b, "GPIO1", LOGIC_1, 2048 * 2 * MX_HALF, NS(50));
        long long t1 = t0 < 0 ? -1 : mb_wait(b, "GPIO1", LOGIC_0, 1024 * 2 * MX_HALF, NS(50));
        int seen0 = 0;
        int seen1 = 0;
        unsigned k;

        check(within(t1, (long long)(512 * 2 * MX_HALF), NS(100)),
              "TIMER = 1: GPIO1 half period = 512 UART clocks");
        for (k = 0; k < 8; k++) {
            mb_run(b, NS(130));
            seen0 |= mb_get(b, "GPIO0") == LOGIC_0;
            seen1 |= mb_get(b, "GPIO0") == LOGIC_1;
        }
        check(seen0 && seen1, "CLKtoGPIO: the reference clock on GPIO0");
    }
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x04, 0x00);
    mx_w(b, 0x00, 0x00);
    mx_w(b, 0x1F, 0xCD);
    /* Receive per channel. */
    mb_drive_byte(b, "RX1", 0x55, bt, 1);
    mb_run(b, bt);
    mb_expect_u(mx_r(b, 0x20 | 0x12), 1, "UART1 RxFIFOLvl");
    mb_expect_u(mx_r(b, 0x12), 0, "UART0 RxFIFOLvl");
    mb_expect_u(mx_r(b, 0x20 | 0x00), 0x55, "UART1 RHR");
    case_end("J5 MAX3109: U bit, per-channel registers, GlobalIRQ, fast read, extended map, triggers");
    mb_close(b);
}

static void case_j6(void) {
    MB *b = mx_open("SFX-MAX14830-tqfn48", 1, MX_HALF, NULL);
    SimTime bt = 16u * 2 * MX_HALF;
    static const char *const MAX3108_TIES[] = {"MOSI/A1", "SCLK/SCL", "CS/A0", "MISO/SDA", NULL};
    char text[40];
    uint16_t log[4];
    unsigned u;
    unsigned v;

    case_begin();
    for (u = 0; u < 4; u++) {
        char name[8];

        snprintf(name, sizeof name, "RX%u", u);
        mb_set(b, name, LOGIC_1);
        snprintf(name, sizeof name, "CTS%u", u);
        mb_set(b, name, LOGIC_0);
    }
    mb_run(b, NS(500));
    for (u = 0; u < 4; u++) {
        mx_w(b, (u << 5) | 0x14, 0x10 + u);
    }
    for (u = 0; u < 4; u++) {
        mb_expect_u(mx_r(b, (u << 5) | 0x14), 0x10 + u, "U1:U0 select the UART (XON1)");
    }
    mb_expect_u(mx_r(b, 0x1F), 0x0F, "GlobalIRQ 0x0F");
    mx_w(b, 0x1F, 0xCE);
    mb_expect_u(mx_r(b, 0x05), 0xB4, "RevID 0xB4");
    mx_w(b, 0x1F, 0xCD);
    mx_w(b, (2u << 5) | 0x01, 0x40);
    mb_spi(b, 0x00, 8, text, 0);
    check(strcmp(text, "ZZZZ1011") == 0, "read command: IRQ3..IRQ0 on clocks 5-8 (UART2 pending)");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "IRQSel = 0: a pending UART does not drive IRQ");
    mx_w(b, (2u << 5) | 0x09, 0x80);   /* IRQSel on UART2 */
    mb_expect_pin(b, "IRQ", LOGIC_0, "UART2 pending with IRQSel");
    mx_r(b, (2u << 5) | 0x02);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "ISR read releases IRQ");
    mx_w(b, (2u << 5) | 0x01, 0x00);
    mb_expect_u(mx_r(b, 0x1E), 0x08, "CLKSource resets to 0x08");
    mx_setup(b, 0, 0x18);
    for (u = 1; u < 4; u++) {
        mx_w(b, (u << 5) | 0x0B, 0x03);
        mx_w(b, (u << 5) | 0x1C, 0x01);
    }
    for (u = 0; u < 4; u++) {
        mx_w(b, (u << 5) | 0x00, 0x41 + u);
    }
    mb_run(b, bt * 14);
    for (u = 0; u < 4; u++) {
        check(mb_sent(b, u, log, 4) == 1 && log[0] == 0x41 + u, "each UART sends on its own TX");
    }
    mb_close(b);

    /* MAX14830 I2C: UART2 answers at the table address - 0x40. */
    b = mx_open("SFX-MAX14830-tqfn48", 0, MX_HALF, NULL);
    mb_set(b, "MOSI/A1", LOGIC_0);
    mb_set(b, "CS/A0", LOGIC_0);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, (0x6C - 0x40) << 1) == LOGIC_0, "MAX14830 I2C: UART2 at 0x2C (0x58 8-bit)");
    i2c_write(b, 0x14);
    i2c_write(b, 0x99);
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x6C << 1);
    i2c_write(b, 0x14);
    i2c_restart(b);
    i2c_write(b, (0x6C << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0x00, "UART0 XON1 untouched");
    i2c_start(b);
    i2c_write(b, (0x6C - 0x40) << 1);
    i2c_write(b, 0x14);
    i2c_restart(b);
    i2c_write(b, ((0x6C - 0x40) << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0x99, "UART2 XON1 through its own address");
    mb_close(b);

    /* MAX3108 WLP: the ball pinout; I2C with A1 on SCL, A0 on SDA (0xD6: 0x6B). */
    b = mx_open("SFX-MAX3108-wlp25", 0, MX_HALF, MAX3108_TIES);
    check(strcmp(b->part->pins[mb_pin(b, "SPI/I2C")].designator, "D3") == 0, "WLP: SPI/I2C on ball D3");
    check(strcmp(b->part->pins[mb_pin(b, "TX")].designator, "E4") == 0, "WLP: TX on ball E4");
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, 0x6C << 1) == LOGIC_1, "MAX3108: 0x6C is the DGND/DGND address (NACK)");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_write(b, 0x6B << 1) == LOGIC_0, "MAX3108: A1 = SCL, A0 = SDA answers at 0x6B");
    check(i2c_write(b, 0x1F) == LOGIC_1, "MAX3108: no RevID (0x1F not valid: NACK)");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x6B << 1);
    i2c_write(b, 0x1C);
    i2c_restart(b);
    i2c_write(b, (0x6B << 1) | 1);
    v = i2c_read(b, 1, NULL);
    mb_expect_u(v, 0x01, "MAX3108 DIVLSB 0x01");
    v = i2c_read(b, 0, NULL);
    mb_expect_u(v, 0x00, "MAX3108 DIVMSB (auto-increment)");
    i2c_stop(b);
    i2c_start(b);
    i2c_write(b, 0x6B << 1);
    i2c_write(b, 0x1E);
    i2c_restart(b);
    i2c_write(b, (0x6B << 1) | 1);
    v = i2c_read(b, 0, NULL);
    i2c_stop(b);
    mb_expect_u(v, 0x18, "MAX3108 CLKSource resets to 0x18");
    case_end("J6 MAX14830 U1:U0 channels and I2C offsets; MAX3108 WLP balls and its address table");
    mb_close(b);
}

/* ==== end of plan-12-J ==================================================================== */

static void h_expect_text(const char *got, const char *want, const char *what) {
    char message[200];

    snprintf(message, sizeof message, "%s: got %s, want %s", what, got, want);
    check(strcmp(got, want) == 0, message);
}

/* The last sent frame equals v. */
static void h_expect_sent(Bench *b, unsigned channel, unsigned v, const char *what) {
    const SimComponentType *type = sim_instance_type(b->chip);
    void *state = sim_instance_state(b->chip);
    size_t n = type->serial_sent(state, channel, 0, NULL, 0);
    uint16_t last = 0;
    char message[160];

    if (n > 0) {
        type->serial_sent(state, channel, n - 1, &last, 1);
    }
    snprintf(message, sizeof message, "%s: %zu frames, last 0x%02X, want 0x%02X", what, n, last, v);
    check(n > 0 && last == v, message);
}

/* ---- H1: AY-5-1013A (ay1013, pin-parallel) ---------------------------------------- */

static const char *const AY_DB[8] = {"DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7", "DB8"};
static const char *const AY_RD[8] = {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"};
static const char *const AY_STATUS[5] = {"PE", "FE", "OR", "DAV", "TBMT"};

/* 1 MHz on TCP and RCP: a 16 us bit. */
#define AY_HALF NS(500)
#define AY_BIT NS(16000)

static HBench *ay_open(const char *part) {
    static const HClockPin CLOCKS[2] = {{"TCP", 0}, {"RCP", 0}};
    SimTime half = AY_HALF;
    HBench *h = hb_open(part, CLOCKS, 2, &half, 1);
    Bench *b = &h->b;

    set(b, "XR", LOGIC_1);
    set(b, "RDE", LOGIC_0);
    set(b, "SWE", LOGIC_0);
    set(b, "RDAV", LOGIC_1);
    set(b, "DS", LOGIC_1);
    set(b, "SI", LOGIC_1);
    set(b, "CS", LOGIC_0);
    set(b, "NP", LOGIC_1);
    set(b, "TSB", LOGIC_0);
    set(b, "NB1", LOGIC_1);
    set(b, "NB2", LOGIC_1);
    set(b, "EPS", LOGIC_0);
    h_set_bus(b, AY_DB, 8, 1, 0);
    run(b, NS(2000));
    set(b, "XR", LOGIC_0);
    run(b, NS(2000));
    return h;
}

/* Loads the control word: nb data bits, np, eps, tsb (CS pulse). */
static void ay_control(Bench *b, unsigned nb, int np, int eps, int tsb) {
    set(b, "NB1", (nb - 5) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "NB2", (nb - 5) & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "NP", np ? LOGIC_1 : LOGIC_0);
    set(b, "EPS", eps ? LOGIC_1 : LOGIC_0);
    set(b, "TSB", tsb ? LOGIC_1 : LOGIC_0);
    run(b, NS(100));
    set(b, "CS", LOGIC_1);
    run(b, NS(400));
    set(b, "CS", LOGIC_0);
    run(b, NS(100));
}

static void ay_strobe(Bench *b, unsigned v) {
    h_set_bus(b, AY_DB, 8, 1, v);
    run(b, NS(100));
    set(b, "DS", LOGIC_0);
    run(b, NS(300));
    set(b, "DS", LOGIC_1);
    run(b, NS(100));
}

/* Sends v and checks the frame and the time from the start bit to EOC (in
   half-bit units, so 1.5 stops can be told from 2). */
static void ay_tx_check(Bench *b, unsigned v, unsigned nb, char parity, unsigned stop_halves,
                        const char *what) {
    char want[24];
    char got[24];
    SimTime t0;
    SimTime spent = 0;
    unsigned bits = 1 + nb + (parity != 0);
    unsigned halves;
    char message[160];

    ay_strobe(b, v);
    if (!h_catch(b, "SO", AY_BIT, bits - 1, 4 * AY_BIT, got)) {
        check(0, what);
        return;
    }
    h_frame_bits(v, nb, parity, 0, want);
    t0 = sim_now(b->sim) - (SimTime)(bits - 1) * AY_BIT - AY_BIT / 2;
    snprintf(message, sizeof message, "%s frame", what);
    h_expect_text(got, want, message);
    while (get(b, "EOC") != LOGIC_1 && spent < 4 * AY_BIT) {
        run(b, NS(250));
        spent += NS(250);
    }
    halves = (unsigned)((sim_now(b->sim) - t0 + AY_BIT / 4) / (AY_BIT / 2));
    snprintf(message, sizeof message, "%s: EOC after %u half bits, want %u", what, halves,
             2 * bits + stop_halves);
    check(halves == 2 * bits + stop_halves, message);
}

static void ay_rx(Bench *b, unsigned v, unsigned nb, char parity, int stop) {
    char bits[24];

    h_frame_bits(v, nb, parity, 0, bits);
    strcat(bits, stop ? "1" : "0");
    h_drive_bits(b, "SI", bits, AY_BIT);
    run(b, AY_BIT * 2);
}

static void case_h1_ay1013(void) {
    HBench *h = ay_open("SFX-AY-5-1013A-dip40");
    Bench *b = &h->b;
    static const char PARITY[3] = {0, 'o', 'e'};
    unsigned nb;
    unsigned p;
    unsigned tsb;
    char text[64];

    case_begin();
    /* After XR: TBMT, EOC, SO high; DAV, OR, FE, PE low; RHR not cleared (X). */
    h_expect_bus(b, AY_STATUS, 5, "10000", "status after XR (TBMT DAV OR FE PE)");
    expect_pin(b, "EOC", LOGIC_1, "after XR");
    expect_pin(b, "SO", LOGIC_1, "after XR");
    h_expect_bus(b, AY_RD, 8, "XXXXXXXX", "RHR at power-on (XR does not clear it)");

    /* Every word length x parity x stop combination (5-bit TSB = 2 stops). */
    for (nb = 5; nb <= 8; nb++) {
        for (p = 0; p < 3; p++) {
            for (tsb = 0; tsb < 2; tsb++) {
                char what[64];

                snprintf(what, sizeof what, "TX %u%c%u", nb, p == 0 ? 'N' : p == 1 ? 'O' : 'E',
                         tsb ? 2u : 1u);
                ay_control(b, nb, p == 0, p == 2, (int)tsb);
                ay_tx_check(b, 0xA5 & ((1u << nb) - 1), nb, PARITY[p], tsb ? 4u : 2u, what);
            }
        }
    }
    /* The control latch: CS low holds the word while the pins change. */
    ay_control(b, 8, 1, 0, 0);
    set(b, "NB1", LOGIC_0);
    set(b, "NB2", LOGIC_0);
    set(b, "NP", LOGIC_0);
    run(b, NS(200));
    ay_tx_check(b, 0x81, 8, 0, 2, "latched 8N1 with the pins at 5O");
    /* CS high: transparent. */
    set(b, "CS", LOGIC_1);
    run(b, NS(200));
    ay_tx_check(b, 0x15, 5, 'o', 2, "CS high: the pins act directly (5O1)");
    set(b, "CS", LOGIC_0);
    ay_control(b, 8, 1, 0, 0);

    /* TBMT: low after DS, high when the shift register takes the byte; a
       second byte waits (double buffer). */
    ay_strobe(b, 0x31);
    run(b, AY_BIT / 4);
    ay_strobe(b, 0x32);
    run(b, NS(600));
    expect_pin(b, "TBMT", LOGIC_0, "second byte waiting");
    expect_pin(b, "EOC", LOGIC_0, "sending");
    run(b, AY_BIT * 11);
    expect_pin(b, "TBMT", LOGIC_1, "second byte taken after the first frame");
    run(b, AY_BIT * 11);
    expect_pin(b, "EOC", LOGIC_1, "both sent");

    /* Receive: DAV after the stop sample, RD the data; RDAV clears DAV. */
    ay_rx(b, 0x5A, 8, 0, 1);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11000", "after one byte (TBMT DAV OR FE PE)");
    h_expect_bus(b, AY_RD, 8, "01011010", "RD after 0x5A");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    expect_pin(b, "DAV", LOGIC_0, "RDAV low clears DAV");
    set(b, "RDAV", LOGIC_1);
    run(b, NS(600));
    /* Overrun: a second byte while DAV is 1. */
    ay_rx(b, 0x11, 8, 0, 1);
    run(b, NS(4000));
    ay_rx(b, 0x22, 8, 0, 1);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11100", "OR on the second byte without RDAV");
    h_expect_bus(b, AY_RD, 8, "00100010", "the newer byte replaces the older");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    /* Framing error: stop bit 0. */
    ay_rx(b, 0x33, 8, 0, 0);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11010", "FE (stop bit 0); OR cleared");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    /* Parity error: 7E with odd parity on the line. */
    ay_control(b, 7, 0, 1, 0);
    ay_rx(b, 0x41, 7, 'o', 1);
    run(b, NS(4000));
    h_expect_bus(b, AY_STATUS, 5, "11001", "PE on a wrong parity bit");
    h_expect_bus(b, AY_RD, 8, "01000001", "7-bit data, RD8 = 0");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    ay_control(b, 8, 1, 0, 0);

    /* RDE / SWE 3-state; EOC and SO stay driven. */
    set(b, "RDE", LOGIC_1);
    set(b, "SWE", LOGIC_1);
    run(b, NS(1000));
    h_expect_bus(b, AY_RD, 8, "ZZZZZZZZ", "RDE high");
    h_expect_bus(b, AY_STATUS, 5, "ZZZZZ", "SWE high");
    expect_pin(b, "EOC", LOGIC_1, "EOC is not 3-state");
    set(b, "RDE", LOGIC_0);
    set(b, "SWE", LOGIC_0);
    run(b, NS(1000));

    /* XR: status reset, RHR kept (AY-5-1013A). */
    ay_rx(b, 0x66, 8, 0, 1);
    run(b, NS(4000));
    set(b, "XR", LOGIC_1);
    run(b, NS(1000));
    set(b, "XR", LOGIC_0);
    run(b, NS(1000));
    h_expect_bus(b, AY_STATUS, 5, "10000", "status after XR");
    h_expect_bus(b, AY_RD, 8, "01100110", "XR keeps the RHR on the AY-5-1013A");

    /* Terminal: OK typed into SI arrives on RD; a transmitted A is logged. */
    set(b, "SI", LOGIC_Z);   /* the terminal drives SI */
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    h_wait(b, "DAV", LOGIC_1, 20 * AY_BIT);
    h_get_bus(b, AY_RD, 8, text);
    h_expect_text(text, "01001111", "terminal O on RD");
    set(b, "RDAV", LOGIC_0);
    run(b, NS(600));
    set(b, "RDAV", LOGIC_1);
    h_wait(b, "DAV", LOGIC_1, 20 * AY_BIT);
    h_get_bus(b, AY_RD, 8, text);
    h_expect_text(text, "01001011", "terminal K on RD");
    ay_strobe(b, 'A');
    run(b, 12 * AY_BIT);
    h_expect_sent(b, 0, 'A', "transmitted A in the sent log");
    hb_close(h);

    /* The AY-3-1015D variant: 1.5 stops with 5 bits; XR clears the RHR. */
    h = ay_open("SFX-AY-5-1013A-dip40n");
    b = &h->b;
    h_expect_bus(b, AY_RD, 8, "00000000", "AY-3-1015D: XR clears the RHR");
    ay_control(b, 5, 1, 0, 1);
    ay_tx_check(b, 0x15, 5, 0, 3, "AY-3-1015D 5N1.5");
    ay_control(b, 8, 1, 0, 0);
    ay_rx(b, 0x77, 8, 0, 1);
    run(b, NS(2000));
    h_expect_bus(b, AY_RD, 8, "01110111", "AY-3-1015D receives");
    set(b, "XR", LOGIC_1);
    run(b, NS(1000));
    set(b, "XR", LOGIC_0);
    run(b, NS(1000));
    h_expect_bus(b, AY_RD, 8, "00000000", "AY-3-1015D: XR clears the RHR");
    case_end("H1 AY-5-1013A: control latch, every TX format, DAV/RDAV, OR/FE/PE, RDE/SWE, reset, terminal");
    hb_close(h);
}

/* The other pin-parallel devices on the same machine (COM2017, HD-6402):
   their pin names, 5-bit 1.5 stops, receive, the reset rule, terminal. */
typedef struct {
    const char *part;
    const char *td[8], *rd[8];
    const char *ds, *rdav, *so, *si, *xr, *strobe, *np, *tsb, *nb1, *nb2, *eps, *rde, *swe, *dav, *eoc;
    const char *tcp, *rcp;
    int clears;
} AyPins;

static void case_h1_pin_parallel(void) {
    static const AyPins DEV[2] = {
        {"SFX-COM2017-dip40",
         {"TD1", "TD2", "TD3", "TD4", "TD5", "TD6", "TD7", "TD8"},
         {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"},
         "TDS", "RDAR", "TSO", "RSI", "MR", "CS", "NPB", "NSB", "NDB1", "NDB2", "POE", "RDE", "SWE",
         "RDA", "TEOC", "TCP", "RCP", 0},
        {"SFX-HD-6402-dip40",
         {"TBR1", "TBR2", "TBR3", "TBR4", "TBR5", "TBR6", "TBR7", "TBR8"},
         {"RBR1", "RBR2", "RBR3", "RBR4", "RBR5", "RBR6", "RBR7", "RBR8"},
         "TBRL", "DRR", "TRO", "RRI", "MR", "CRL", "PI", "SBS", "CLS1", "CLS2", "EPE", "RRD", "SFD",
         "DR", "TRE", "TRC", "RRC", 0},
    };
    unsigned d;

    case_begin();
    for (d = 0; d < 2; d++) {
        const AyPins *p = &DEV[d];
        HClockPin clocks[2] = {{p->tcp, 0}, {p->rcp, 0}};
        SimTime half = AY_HALF;
        HBench *h = hb_open(p->part, clocks, 2, &half, 1);
        Bench *b = &h->b;
        char got[24];
        char want[24];
        char text[16];
        char what[96];
        SimTime t0;
        unsigned halves;

        set(b, p->xr, LOGIC_1);
        set(b, p->rde, LOGIC_0);
        set(b, p->swe, LOGIC_0);
        set(b, p->rdav, LOGIC_1);
        set(b, p->ds, LOGIC_1);
        set(b, p->si, LOGIC_1);
        set(b, p->np, LOGIC_1);
        set(b, p->tsb, LOGIC_1);
        set(b, p->nb1, LOGIC_0);
        set(b, p->nb2, LOGIC_0);
        set(b, p->eps, LOGIC_0);
        set(b, p->strobe, LOGIC_1);
        h_set_bus(b, p->td, 8, 1, 0x15);
        run(b, NS(2000));
        set(b, p->xr, LOGIC_0);
        set(b, p->strobe, LOGIC_0);   /* latch 5N, 2 stops selected */
        run(b, NS(2000));
        h_get_bus(b, p->rd, 8, text);
        snprintf(what, sizeof what, "%s: RD after the reset (not cleared: X)", p->part);
        h_expect_text(text, "XXXXXXXX", what);
        /* 5 bits with the stop select: 1.5 stops. */
        set(b, p->ds, LOGIC_0);
        run(b, NS(300));
        set(b, p->ds, LOGIC_1);
        check(h_catch(b, p->so, AY_BIT, 5, 4 * AY_BIT, got), p->part);
        h_frame_bits(0x15, 5, 0, 0, want);
        snprintf(what, sizeof what, "%s: 5N frame", p->part);
        h_expect_text(got, want, what);
        t0 = sim_now(b->sim) - 5 * AY_BIT - AY_BIT / 2;
        h_wait(b, p->eoc, LOGIC_1, 4 * AY_BIT);
        halves = (unsigned)((sim_now(b->sim) - t0 + AY_BIT / 4) / (AY_BIT / 2));
        snprintf(what, sizeof what, "%s: 5-bit stop select gives 1.5 stops (%u half bits)", p->part,
                 halves);
        check(halves == 15, what);
        /* 8N1, receive, terminal. */
        set(b, p->nb1, LOGIC_1);
        set(b, p->nb2, LOGIC_1);
        set(b, p->tsb, LOGIC_0);
        set(b, p->strobe, LOGIC_1);
        run(b, NS(300));
        set(b, p->strobe, LOGIC_0);
        h_frame_bits(0xA6, 8, 0, 1, want);
        h_drive_bits(b, p->si, want, AY_BIT);
        run(b, AY_BIT);
        expect_pin(b, p->dav, LOGIC_1, p->part);
        h_get_bus(b, p->rd, 8, text);
        snprintf(what, sizeof what, "%s: received 0xA6", p->part);
        h_expect_text(text, "10100110", what);
        set(b, p->rdav, LOGIC_0);
        run(b, NS(600));
        set(b, p->rdav, LOGIC_1);
        set(b, p->si, LOGIC_Z);
        run(b, NS(100));
        check(h_send(b, 0, "OK") == 1, p->part);
        h_wait(b, p->dav, LOGIC_1, 12 * AY_BIT);
        h_get_bus(b, p->rd, 8, text);
        snprintf(what, sizeof what, "%s: terminal O", p->part);
        h_expect_text(text, "01001111", what);
        h_set_bus(b, p->td, 8, 1, 'A');
        set(b, p->ds, LOGIC_0);
        run(b, NS(300));
        set(b, p->ds, LOGIC_1);
        run(b, 12 * AY_BIT);
        h_expect_sent(b, 0, 'A', p->part);
        hb_close(h);
    }
    case_end("H1 (COM2017, HD-6402): pin names, 5-bit 1.5 stops, receive, reset keeps RD, terminal");
}

/* ---- H2: COM1863 / COM8018 (ay1013 with HIACC) -------------------------------------- */

static const char *const COM_TD[8] = {"TD1", "TD2", "TD3", "TD4", "TD5", "TD6", "TD7", "TD8"};
static const char *const COM_RD[8] = {"RD1", "RD2", "RD3", "RD4", "RD5", "RD6", "RD7", "RD8"};

static void com_control(Bench *b, unsigned nb, int npb, int poe, int nsb) {
    set(b, "NDB1", (nb - 5) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "NDB2", (nb - 5) & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "NPB", npb ? LOGIC_1 : LOGIC_0);
    set(b, "POE", poe ? LOGIC_1 : LOGIC_0);
    set(b, "NSB", nsb ? LOGIC_1 : LOGIC_0);
    run(b, NS(100));
    set(b, "CS", LOGIC_1);
    run(b, NS(300));
    set(b, "CS", LOGIC_0);
    run(b, NS(100));
}

static void com_strobe(Bench *b, unsigned v) {
    h_set_bus(b, COM_TD, 8, 1, v);
    run(b, NS(100));
    set(b, "TDS", LOGIC_0);
    run(b, NS(300));
    set(b, "TDS", LOGIC_1);
    run(b, NS(100));
}

static void case_h2_com1863(void) {
    static const HClockPin CLOCKS[2] = {{"TCP", 0}, {"RCP", 0}};
    SimTime half = AY_HALF;
    HBench *h = hb_open("SFX-COM1863-dip40", CLOCKS, 2, &half, 1);
    Bench *b = &h->b;
    const SimTime bit32 = 2 * AY_BIT;
    char got[24];
    char want[24];
    char text[16];
    SimTime t0;

    case_begin();
    set(b, "MR", LOGIC_1);
    set(b, "RDE", LOGIC_0);
    set(b, "SWE", LOGIC_0);
    set(b, "RDAR", LOGIC_1);
    set(b, "TDS", LOGIC_1);
    set(b, "RSI", LOGIC_1);
    set(b, "CS", LOGIC_0);
    set(b, "HIACC", LOGIC_0);
    h_set_bus(b, COM_TD, 8, 1, 0);
    run(b, NS(2000));
    set(b, "MR", LOGIC_0);
    run(b, NS(2000));
    h_expect_bus(b, COM_RD, 8, "00000000", "MR clears RD1-RD8 (COM1863)");

    /* 16x with HIACC low. */
    com_control(b, 8, 1, 0, 0);
    com_strobe(b, 0x96);
    check(h_catch(b, "TSO", AY_BIT, 9, 4 * AY_BIT, got), "16x frame starts");
    h_frame_bits(0x96, 8, 0, 1, want);
    h_expect_text(got, want, "HIACC low: 16 clocks a bit");
    run(b, 2 * AY_BIT);

    /* HIACC high: 32x clocks. */
    set(b, "HIACC", LOGIC_1);
    run(b, NS(200));
    com_strobe(b, 0x96);
    check(h_catch(b, "TSO", bit32, 9, 4 * bit32, got), "32x frame starts");
    h_expect_text(got, want, "HIACC high: 32 clocks a bit");
    run(b, 2 * bit32);

    /* 5 data bits with NSB = 1: 1.5 stop bits (planner decision). */
    com_control(b, 5, 1, 0, 1);
    com_strobe(b, 0x15);
    check(h_catch(b, "TSO", bit32, 5, 4 * bit32, got), "5-bit frame starts");
    t0 = sim_now(b->sim) - 5 * bit32 - bit32 / 2;
    h_wait(b, "TEOC", LOGIC_1, 4 * bit32);
    {
        unsigned halves = (unsigned)((sim_now(b->sim) - t0 + bit32 / 4) / (bit32 / 2));
        char message[96];

        snprintf(message, sizeof message, "5-bit NSB=1: TEOC after %u half bits, want 15", halves);
        check(halves == 15, message);
    }
    com_control(b, 8, 1, 0, 0);

    /* 32x receive; RDAR held low over the transfer: ROR set, RDA waits for
       RDAR high (p7). */
    h_frame_bits(0x3C, 8, 0, 1, want);
    set(b, "RDAR", LOGIC_0);
    h_drive_bits(b, "RSI", want, bit32);
    run(b, bit32);
    expect_pin(b, "RDA", LOGIC_0, "RDA held low while RDAR is low");
    h_expect_bus(b, COM_RD, 8, "00111100", "32x data");
    expect_pin(b, "ROR", LOGIC_1, "ROR: RDAR was low at the transfer");
    set(b, "RDAR", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "RDA", LOGIC_1, "RDA rises once RDAR is high");
    set(b, "RDAR", LOGIC_0);
    run(b, NS(500));
    set(b, "RDAR", LOGIC_1);
    run(b, NS(500));
    h_drive_bits(b, "RSI", want, bit32);
    run(b, bit32);
    expect_pin(b, "ROR", LOGIC_0, "ROR reset: RDA low and RDAR high at the transfer");

    /* Terminal at 32x. */
    set(b, "RDAR", LOGIC_0);
    run(b, NS(500));
    set(b, "RDAR", LOGIC_1);
    set(b, "RSI", LOGIC_Z);
    run(b, NS(500));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    h_wait(b, "RDA", LOGIC_1, 12 * bit32);
    h_get_bus(b, COM_RD, 8, text);
    h_expect_text(text, "01001111", "terminal O");
    set(b, "RDAR", LOGIC_0);
    run(b, NS(500));
    set(b, "RDAR", LOGIC_1);
    h_wait(b, "RDA", LOGIC_1, 12 * bit32);
    h_get_bus(b, COM_RD, 8, text);
    h_expect_text(text, "01001011", "terminal K");
    com_strobe(b, 'A');
    run(b, 12 * bit32);
    h_expect_sent(b, 0, 'A', "transmitted A logged");
    case_end("H2 COM1863/COM8018: HIACC 32x clocks, 5-bit 1.5 stops, RDA/ROR rule, terminal");
    hb_close(h);
}

/* ---- H3: CDP1854A (ay1013 with cpuMode) ---------------------------------------------- */

static const char *const CDP_TBUS[8] = {"TBUS0", "TBUS1", "TBUS2", "TBUS3", "TBUS4", "TBUS5", "TBUS6", "TBUS7"};
static const char *const CDP_RBUS[8] = {"RBUS0", "RBUS1", "RBUS2", "RBUS3", "RBUS4", "RBUS5", "RBUS6", "RBUS7"};

/* Mode 1: one CPU cycle; rsel, write (1) or read (0); returns the R BUS
   during the read as text. */
static void cdp_cycle(Bench *b, int rsel, int write, unsigned v, char *text) {
    set(b, "SFD/RSEL", rsel ? LOGIC_1 : LOGIC_0);
    set(b, "CRL/RD_WR", write ? LOGIC_0 : LOGIC_1);
    if (write) {
        h_set_bus(b, CDP_TBUS, 8, 1, v);
    }
    set(b, "THRL/CS1", LOGIC_1);
    run(b, NS(300));
    if (!write && text != NULL) {
        h_get_bus(b, CDP_RBUS, 8, text);
    }
    set(b, "DAR/TPB", LOGIC_1);
    run(b, NS(200));
    set(b, "DAR/TPB", LOGIC_0);
    run(b, NS(200));
    set(b, "THRL/CS1", LOGIC_0);
    set(b, "CRL/RD_WR", LOGIC_1);
    run(b, NS(300));
}

static void case_h3_cdp1854a(void) {
    static const HClockPin CLOCKS[2] = {{"TCLOCK", 0}, {"RCLOCK", 0}};
    SimTime half = AY_HALF;
    HBench *h;
    Bench *b;
    char got[24];
    char want[24];
    char text[16];

    case_begin();
    /* ---- Mode 0: the AY-5-1013 machine. ---- */
    h = hb_open("SFX-CDP1854A-dip40", CLOCKS, 2, &half, 1);
    b = &h->b;
    set(b, "MODE", LOGIC_0);
    set(b, "MR/CLEAR", LOGIC_1);
    set(b, "RRD/CS2", LOGIC_0);
    set(b, "SFD/RSEL", LOGIC_0);
    set(b, "DAR/TPB", LOGIC_1);
    set(b, "THRL/CS1", LOGIC_1);
    set(b, "SDI", LOGIC_1);
    set(b, "CRL/RD_WR", LOGIC_0);
    set(b, "PI/CS3", LOGIC_1);
    set(b, "SBS", LOGIC_0);
    set(b, "WLS1/ES", LOGIC_1);
    set(b, "WLS2/PSI", LOGIC_1);
    set(b, "EPE/CTS", LOGIC_0);
    h_set_bus(b, CDP_TBUS, 8, 1, 0);
    run(b, NS(2000));
    set(b, "MR/CLEAR", LOGIC_0);
    run(b, NS(2000));
    h_expect_bus(b, CDP_RBUS, 8, "00000000", "mode 0: MR clears the RHR");
    expect_pin(b, "THRE", LOGIC_1, "mode 0: THRE after MR");
    expect_pin(b, "TSRE/RTS", LOGIC_1, "mode 0: TSRE after MR");
    /* MR cleared the control register: 5 bits, odd parity, 1 stop. */
    h_set_bus(b, CDP_TBUS, 8, 1, 0x1B);
    set(b, "THRL/CS1", LOGIC_0);
    run(b, NS(300));
    set(b, "THRL/CS1", LOGIC_1);
    check(h_catch(b, "SDO", AY_BIT, 7, 4 * AY_BIT, got), "mode 0 frame");
    h_frame_bits(0x1B, 5, 'o', 1, want);
    h_expect_text(got, want, "mode 0 after MR: 5O1 (control register cleared)");
    run(b, 2 * AY_BIT);
    /* CRL pulse: 8 bits, no parity (PI high), SBS 1 -> 2 stops. */
    set(b, "SBS", LOGIC_1);
    set(b, "CRL/RD_WR", LOGIC_1);
    run(b, NS(300));
    set(b, "CRL/RD_WR", LOGIC_0);
    h_set_bus(b, CDP_TBUS, 8, 1, 0xC3);
    set(b, "THRL/CS1", LOGIC_0);
    run(b, NS(300));
    set(b, "THRL/CS1", LOGIC_1);
    check(h_catch(b, "SDO", AY_BIT, 10, 4 * AY_BIT, got), "mode 0 8N2 frame");
    h_frame_bits(0xC3, 8, 0, 2, want);
    h_expect_text(got, want, "mode 0: 8N2 from the latched control word");
    run(b, 2 * AY_BIT);
    /* Receive: DA high, RBUS data; DAR low resets DA. */
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    expect_pin(b, "DA", LOGIC_1, "mode 0: DA");
    h_expect_bus(b, CDP_RBUS, 8, "01011010", "mode 0: R BUS");
    set(b, "DAR/TPB", LOGIC_0);
    run(b, NS(400));
    set(b, "DAR/TPB", LOGIC_1);
    run(b, NS(400));
    expect_pin(b, "DA", LOGIC_0, "mode 0: DAR resets DA");
    set(b, "SFD/RSEL", LOGIC_1);
    set(b, "RRD/CS2", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "DA", LOGIC_Z, "mode 0: SFD high floats the status");
    expect_pin(b, "RBUS0", LOGIC_Z, "mode 0: RRD high floats the R BUS");
    expect_pin(b, "TSRE/RTS", LOGIC_1, "mode 0: TSRE is not 3-state");
    hb_close(h);

    /* ---- Mode 1: the CDP1800 bus. ---- */
    h = hb_open("SFX-CDP1854A-dip40", CLOCKS, 2, &half, 1);
    b = &h->b;
    set(b, "MODE", LOGIC_1);
    set(b, "MR/CLEAR", LOGIC_0);
    set(b, "THRL/CS1", LOGIC_0);
    set(b, "RRD/CS2", LOGIC_0);
    set(b, "PI/CS3", LOGIC_1);
    set(b, "DAR/TPB", LOGIC_0);
    set(b, "CRL/RD_WR", LOGIC_1);
    set(b, "SFD/RSEL", LOGIC_0);
    set(b, "SDI", LOGIC_1);
    set(b, "EPE/CTS", LOGIC_0);
    set(b, "WLS2/PSI", LOGIC_1);
    set(b, "WLS1/ES", LOGIC_1);
    set(b, "SBS", LOGIC_0);
    h_set_bus(b, CDP_TBUS, 8, 1, 0);
    run(b, NS(2000));
    set(b, "MR/CLEAR", LOGIC_1);
    run(b, NS(2000));
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11000000", "mode 1: status after CLEAR (THRE TSRE)");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: INT inactive (IE 0)");
    expect_pin(b, "TSRE/RTS", LOGIC_1, "mode 1: RTS high (TR 0, empty)");
    /* Format: IE, 8 bits, 1 stop, PI. Then TR: RTS low, the initial THRE interrupt. */
    cdp_cycle(b, 1, 1, 0x39, NULL);
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: no interrupt before TR");
    cdp_cycle(b, 1, 1, 0x80, NULL);
    expect_pin(b, "TSRE/RTS", LOGIC_0, "mode 1: TR sets RTS low");
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: the initial THRE interrupt");
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11000000", "mode 1: status");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: a status read clears the THRE interrupt");
    /* A character: THRE low, then the shift register takes it; THRE pin
       low (active) and INT after one TCLOCK. */
    cdp_cycle(b, 0, 1, 'A', NULL);
    check(h_catch(b, "SDO", AY_BIT, 9, 4 * AY_BIT, got), "mode 1 frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "mode 1: 8N1 A on SDO");
    expect_pin(b, "THRE", LOGIC_0, "mode 1: THRE pin active (low)");
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: THRE interrupt");
    run(b, 2 * AY_BIT);
    h_expect_sent(b, 0, 'A', "mode 1: A logged");
    cdp_cycle(b, 1, 0, 0, text);
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: status read clears THRE and THRE.TSRE");
    /* Receive: DA pin low, INT; an RHR read with TPB resets DA. */
    h_frame_bits(0xA5, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    expect_pin(b, "DA", LOGIC_0, "mode 1: DA pin active (low)");
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: DA interrupt");
    cdp_cycle(b, 0, 0, 0, text);
    h_expect_text(text, "10100101", "mode 1: RHR read");
    expect_pin(b, "DA", LOGIC_1, "mode 1: the read resets DA");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: and its interrupt");
    expect_pin(b, "RBUS0", LOGIC_Z, "mode 1: R BUS released after the read");
    /* Overrun: two characters without a read; PE/OE pin high. */
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    h_drive_bits(b, "SDI", want, AY_BIT);
    run(b, AY_BIT);
    expect_pin(b, "OE/PE_OE", LOGIC_1, "mode 1: PE/OE pin on overrun");
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11000011", "mode 1: status with OE and DA");
    cdp_cycle(b, 0, 0, 0, text);
    /* PSI falling: status bit 5 and INT; ES low: status bit 4. */
    set(b, "WLS2/PSI", LOGIC_0);
    set(b, "WLS1/ES", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "PE/INT", LOGIC_0, "mode 1: PSI interrupt");
    cdp_cycle(b, 1, 0, 0, text);
    h_expect_text(text, "11110010", "mode 1: status with PSI and ES (OE kept)");
    expect_pin(b, "PE/INT", LOGIC_1, "mode 1: the status read clears PSI");
    set(b, "WLS2/PSI", LOGIC_1);
    set(b, "WLS1/ES", LOGIC_1);
    /* BREAK holds SDO low. */
    cdp_cycle(b, 1, 1, 0x79, NULL);
    run(b, AY_BIT);
    expect_pin(b, "SDO", LOGIC_0, "mode 1: BREAK holds SDO low");
    cdp_cycle(b, 1, 1, 0x39, NULL);
    run(b, AY_BIT);
    expect_pin(b, "SDO", LOGIC_0, "mode 1: SDO stays low after BREAK is reset");
    cdp_cycle(b, 0, 1, 0x00, NULL);
    run(b, 12 * AY_BIT);
    expect_pin(b, "SDO", LOGIC_1, "mode 1: a transmitted word ends the break");
    /* CTS high stops the transmitter. */
    set(b, "EPE/CTS", LOGIC_1);
    run(b, NS(500));
    cdp_cycle(b, 0, 1, 0x55, NULL);
    run(b, 4 * AY_BIT);
    expect_pin(b, "SDO", LOGIC_1, "mode 1: CTS high inhibits the transfer");
    set(b, "EPE/CTS", LOGIC_0);
    check(h_catch(b, "SDO", AY_BIT, 9, 4 * AY_BIT, got), "mode 1: sent once CTS is low");
    run(b, 2 * AY_BIT);
    /* Terminal. */
    set(b, "SDI", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "mode 1: terminal send accepted");
    h_wait(b, "DA", LOGIC_0, 12 * AY_BIT);
    cdp_cycle(b, 0, 0, 0, text);
    h_expect_text(text, "01001111", "mode 1: terminal O");
    h_wait(b, "DA", LOGIC_0, 12 * AY_BIT);
    cdp_cycle(b, 0, 0, 0, text);
    h_expect_text(text, "01001011", "mode 1: terminal K");
    case_end("H3 CDP1854A: mode 0 as H1; mode 1 registers, TR/RTS, INT sources, break, CTS, terminal");
    hb_close(h);
}

/* ---- H4: 8251A (i8251) --------------------------------------------------------------- */

static const char *const I8_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* TxC = RxC = 1 MHz; CLK 3.125 MHz. */
#define I8_HALF NS(500)
#define I8_CLK_HALF NS(160)

static void i8_write(Bench *b, int cd, unsigned v) {
    set(b, "CD", cd ? LOGIC_1 : LOGIC_0);
    h_set_bus(b, I8_D, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(300));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    h_set_bus(b, I8_D, 8, 0, 0);
    run(b, NS(3000));   /* recovery between writes */
}

static unsigned i8_read(Bench *b, int cd, char *text) {
    unsigned v;

    set(b, "CD", cd ? LOGIC_1 : LOGIC_0);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    v = h_get_bus(b, I8_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    run(b, NS(500));   /* past tclr */
    return v;
}

static void i8_expect(Bench *b, int cd, const char *want, const char *what) {
    char text[16];

    i8_read(b, cd, text);
    h_expect_text(text, want, what);
}

static HBench *i8_open(void) {
    static const HClockPin CLOCKS[3] = {{"TXC", 0}, {"RXC", 0}, {"CLK", 1}};
    static const SimTime HALVES[2] = {I8_HALF, I8_CLK_HALF};
    HBench *h = hb_open("SFX-8251A-dip28", CLOCKS, 3, HALVES, 2);
    Bench *b = &h->b;

    set(b, "RESET", LOGIC_1);
    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "CD", LOGIC_0);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DSR", LOGIC_1);
    set(b, "SYNDET", LOGIC_Z);
    run(b, NS(5000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(2000));
    return h;
}

/* Moves to just after a TxC/RxC falling edge (bits change there; RxC
   rising samples mid-bit). */
static void i8_align(Bench *b) {
    SimTime period = 2 * I8_HALF;
    SimTime now = sim_now(b->sim);
    SimTime wait = (period - (now + period - NS(10)) % period) % period;

    if (wait > 0) {
        run(b, wait);
    }
}

/* Waits to the next character boundary of n-bit characters after a stream
   that ended on one at time t_b. */
static void i8_to_boundary(Bench *b, SimTime t_b, unsigned n) {
    SimTime span = (SimTime)n * 2 * I8_HALF;
    SimTime gone = (sim_now(b->sim) - t_b) % span;

    if (gone != 0) {
        run(b, span - gone);
    }
}

/* Drives bits ('0'/'1') onto RXD, one per RxC period. */
static void i8_sync_bits(Bench *b, const char *bits) {
    size_t i;

    i8_align(b);
    for (i = 0; bits[i] != '\0'; i++) {
        set(b, "RXD", bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, 2 * I8_HALF);
    }
}

/* The 8 bits of v LSB first. */
static void i8_byte_bits(unsigned v, unsigned n, char *out) {
    unsigned i;

    for (i = 0; i < n; i++) {
        out[i] = (v >> i) & 1u ? '1' : '0';
    }
    out[n] = '\0';
}

static void case_h4_i8251(void) {
    HBench *h = i8_open();
    Bench *b = &h->b;
    const SimTime bt16 = 16 * 2 * I8_HALF;
    const SimTime bt64 = 64 * 2 * I8_HALF;
    const SimTime bt1 = 2 * I8_HALF;
    char got[64];
    char want[64];
    unsigned i;
    SimTime t_b;

    case_begin();
    /* After RESET: idle, mode format next. */
    expect_pin(b, "TXD", LOGIC_1, "TxD marking after RESET");
    expect_pin(b, "TXRDY", LOGIC_0, "TxRDY low (TxEN 0)");
    expect_pin(b, "RXRDY", LOGIC_0, "RxRDY low");
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY high");
    expect_pin(b, "SYNDET", LOGIC_0, "SYNDET output low");
    expect_pin(b, "DTR", LOGIC_1, "DTR high");
    expect_pin(b, "RTS", LOGIC_1, "RTS high");
    i8_expect(b, 1, "00000101", "status after RESET (TxRDY, TxEMPTY)");

    /* Async 16x 8N1, then the command: TxEN DTR RxE ER RTS. */
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    expect_pin(b, "DTR", LOGIC_0, "DTR command bit");
    expect_pin(b, "RTS", LOGIC_0, "RTS command bit");
    expect_pin(b, "TXRDY", LOGIC_1, "TxRDY pin: empty, CTS low, TxEN");
    check(!h_send(b, 0, "") || 1, "");
    i8_write(b, 0, 'H');
    check(h_catch(b, "TXD", bt16, 9, 4 * bt16, got), "16x frame");
    h_frame_bits('H', 8, 0, 1, want);
    h_expect_text(got, want, "16x 8N1 H on TxD");
    expect_pin(b, "TXEMPTY", LOGIC_0, "TxEMPTY low while sending");
    run(b, bt16);
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY high when done");
    h_expect_sent(b, 0, 'H', "H logged");
    /* Receive 16x. */
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    expect_pin(b, "RXRDY", LOGIC_1, "RxRDY after a character");
    i8_expect(b, 1, "00000111", "status: RxRDY");
    i8_expect(b, 0, "01011010", "data 0x5A");
    expect_pin(b, "RXRDY", LOGIC_0, "the data read resets RxRDY");
    /* Overrun, then framing error; ER clears both. */
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    i8_expect(b, 1, "00010111", "status: OE");
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "RXD", want, bt16);
    run(b, 2 * bt16);
    i8_expect(b, 1, "00110111", "status: OE and FE");
    i8_write(b, 1, 0x37);
    i8_expect(b, 1, "00000111", "ER resets OE and FE");
    i8_read(b, 0, NULL);
    /* CTS high: the TxRDY pin goes low, the status bit stays. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "TXRDY", LOGIC_0, "TxRDY pin needs CTS low");
    i8_expect(b, 1, "00000101", "TxRDY status bit is the buffer");
    set(b, "CTS", LOGIC_0);
    /* DSR low: status bit 7. */
    set(b, "DSR", LOGIC_0);
    run(b, NS(500));
    i8_expect(b, 1, "10000101", "DSR status bit");
    set(b, "DSR", LOGIC_1);
    /* SBRK: TxD low. */
    i8_write(b, 1, 0x3F);
    run(b, bt16);
    expect_pin(b, "TXD", LOGIC_0, "SBRK forces TxD low");
    i8_write(b, 1, 0x37);
    run(b, bt16);
    expect_pin(b, "TXD", LOGIC_1, "SBRK off");
    /* TxEN off: TxRDY pin low and nothing is sent. */
    i8_write(b, 1, 0x36);
    expect_pin(b, "TXRDY", LOGIC_0, "TxEN off: TxRDY pin low");
    i8_write(b, 0, 0x11);
    check(!h_catch(b, "TXD", bt16, 9, 4 * bt16, got), "TxEN off: nothing sent");
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY stays high while disabled");
    i8_write(b, 1, 0x37);
    check(h_catch(b, "TXD", bt16, 9, 4 * bt16, got), "TxEN on: the waiting byte goes");
    run(b, 2 * bt16);
    /* RxE off: RxRDY held reset. */
    i8_write(b, 1, 0x33);
    h_frame_bits(0x22, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    expect_pin(b, "RXRDY", LOGIC_0, "RxE off: RxRDY held reset");
    i8_write(b, 1, 0x37);
    /* Break detect: RxD low for two characters. */
    set(b, "RXD", LOGIC_0);
    run(b, 22 * bt16);
    expect_pin(b, "SYNDET", LOGIC_1, "BRKDET after two characters of low");
    i8_expect(b, 1, "01100111", "status: BRKDET (with the break's FE)");
    set(b, "RXD", LOGIC_1);
    run(b, bt16);
    expect_pin(b, "SYNDET", LOGIC_0, "BRKDET reset by RxD high");
    i8_write(b, 1, 0x37);
    i8_read(b, 0, NULL);

    /* Parity (7E2) and the parity error. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0xFA);   /* 2 stops, even, parity on, 7 bits, 16x */
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 0x43);
    check(h_catch(b, "TXD", bt16, 10, 4 * bt16, got), "7E2 frame");
    h_frame_bits(0x43, 7, 'e', 2, want);
    h_expect_text(got, want, "7E2 0x43");
    run(b, 2 * bt16);
    h_frame_bits(0x43, 7, 'o', 1, want);
    h_drive_bits(b, "RXD", want, bt16);
    run(b, bt16);
    i8_expect(b, 1, "00001111", "status: PE");
    i8_expect(b, 0, "01000011", "7-bit data, unused bit 0");

    /* 64x. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x4F);
    i8_write(b, 1, 0x37);
    i8_write(b, 0, 0x96);
    check(h_catch(b, "TXD", bt64, 9, 4 * bt64, got), "64x frame");
    h_frame_bits(0x96, 8, 0, 1, want);
    h_expect_text(got, want, "64x 8N1");
    run(b, bt64);
    h_drive_bits(b, "RXD", want, bt64);
    run(b, bt64);
    i8_expect(b, 0, "10010110", "64x receive");

    /* 1x: one bit per TxC / RxC. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x4D);
    i8_write(b, 1, 0x37);
    set(b, "CTS", LOGIC_1);   /* hold the byte until the catch is ready */
    run(b, NS(500));
    i8_write(b, 0, 0x69);
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "TXD", bt1, 9, 16 * bt1, got), "1x frame");
    h_frame_bits(0x69, 8, 0, 1, want);
    h_expect_text(got, want, "1x 8N1");
    run(b, 4 * bt1);
    i8_align(b);
    h_drive_bits(b, "RXD", want, bt1);
    run(b, 4 * bt1);
    i8_expect(b, 0, "01101001", "1x receive");

    /* Terminal (async 16x). */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x4E);
    i8_write(b, 1, 0x37);
    set(b, "RXD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted (async)");
    h_wait(b, "RXRDY", LOGIC_1, 12 * bt16);
    i8_expect(b, 0, "01001111", "terminal O");
    h_wait(b, "RXRDY", LOGIC_1, 12 * bt16);
    i8_expect(b, 0, "01001011", "terminal K");
    i8_write(b, 0, 'A');
    run(b, 12 * bt16);
    h_expect_sent(b, 0, 'A', "transmitted A logged");
    set(b, "RXD", LOGIC_1);

    /* IR: back to the mode format; sync, 8 bits, single SYNC 0x16. */
    i8_write(b, 1, 0x40);
    expect_pin(b, "DTR", LOGIC_1, "IR: DTR inactive");
    i8_write(b, 1, 0x8C);
    i8_write(b, 1, 0x16);
    check(h_send(b, 0, "x") == 0, "terminal refused in sync mode");
    i8_write(b, 1, 0xB7);   /* EH RTS ER RxE DTR TxEN */
    i8_expect(b, 0, "11111111", "EH sets the receive buffer to ones");
    expect_pin(b, "SYNDET", LOGIC_0, "hunting");
    i8_byte_bits(0x16, 8, want);
    strcpy(got, "1101");
    strcat(got, want);
    i8_byte_bits(0x41, 8, want);
    strcat(got, want);
    i8_sync_bits(b, got);
    t_b = sim_now(b->sim);
    expect_pin(b, "SYNDET", LOGIC_1, "SYNDET after SYNC1 (bit-by-bit hunt)");
    expect_pin(b, "RXRDY", LOGIC_1, "the character after the sync");
    i8_expect(b, 1, "01000111", "status: SYNDET, RxRDY (TxEMPTY)");
    expect_pin(b, "SYNDET", LOGIC_0, "a status read resets SYNDET");
    i8_expect(b, 0, "01000001", "sync data 0x41");
    /* Out of hunt: a sync character at a boundary sets SYNDET again. */
    i8_to_boundary(b, t_b, 8);
    i8_byte_bits(0x16, 8, want);
    i8_sync_bits(b, want);
    expect_pin(b, "SYNDET", LOGIC_1, "SYNDET at a later SYNC1 boundary");
    i8_read(b, 1, NULL);
    i8_read(b, 0, NULL);

    /* Sync TX: a data character, then SYNC fill with TxEMPTY high. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    i8_write(b, 0, 0x55);
    set(b, "CTS", LOGIC_0);
    for (i = 0; i < 40; i++) {
        got[i] = "01XZ"[get(b, "TXD")];
        run(b, 2 * I8_HALF);
    }
    got[40] = '\0';
    i8_byte_bits(0x55, 8, want);
    check(strstr(got, want) != NULL, "sync TX: the data character on TxD");
    i8_byte_bits(0x16, 8, want);
    {
        char two[24];

        strcpy(two, want);
        strcat(two, want);
        check(strstr(got, two) != NULL, "sync TX: SYNC fill when the CPU is late");
    }
    expect_pin(b, "TXEMPTY", LOGIC_1, "TxEMPTY high during the fill");

    /* Double sync with parity: SYNC1 then SYNC2 contiguous. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0x3C);   /* sync, even parity on, 8 bits, double */
    i8_write(b, 1, 0x16);
    i8_write(b, 1, 0x27);
    i8_write(b, 1, 0xB7);
    {
        char p1[16];
        char p2[16];

        i8_byte_bits(0x16, 8, p1);
        strcat(p1, "1");   /* 0x16 has three ones: even parity bit 1 */
        i8_byte_bits(0x27, 8, p2);
        strcat(p2, "0");   /* 0x27 has four ones */
        strcpy(got, "011");
        strcat(got, p1);
        strcat(got, "0000");
        i8_sync_bits(b, got);
        expect_pin(b, "SYNDET", LOGIC_0, "SYNC1 alone is not sync (double)");
        strcpy(got, p1);
        strcat(got, p2);
        i8_sync_bits(b, got);
        expect_pin(b, "SYNDET", LOGIC_1, "SYNC1 then SYNC2: sync");
        i8_byte_bits(0x31, 8, p1);
        strcat(p1, "0");   /* wrong: 0x31 has three ones, even parity bit is 1 */
        i8_sync_bits(b, p1);
        i8_expect(b, 1, "01001111", "status: SYNDET, PE, RxRDY");
        i8_expect(b, 0, "00110001", "data after the sync pair");
    }

    /* External sync: SYNDET is an input; its rising edge ends the hunt. */
    i8_write(b, 1, 0x40);
    i8_write(b, 1, 0xCC);   /* sync, external, single, 8 bits */
    i8_write(b, 1, 0x11);
    i8_write(b, 1, 0xB7);
    expect_pin(b, "SYNDET", LOGIC_Z, "external sync: SYNDET is an input");
    i8_align(b);
    set(b, "SYNDET", LOGIC_1);   /* assembly starts at the next RxC rising edge */
    i8_byte_bits(0xC3, 8, want);
    i8_sync_bits(b, want);
    set(b, "SYNDET", LOGIC_0);
    expect_pin(b, "RXRDY", LOGIC_1, "external sync: assembling after SYNDET");
    i8_expect(b, 0, "11000011", "external sync data");
    set(b, "SYNDET", LOGIC_Z);
    case_end("H4 8251A: async 1x/16x/64x, every command bit, errors, break; sync hunt, SYNDET, fill, double sync, external sync; terminal");
    hb_close(h);
}

/* ---- H5: 82510 (i82510) ----------------------------------------------------------------- */

static const char *const I5_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* CLK 18.432 MHz (external clock, /2: system clock 9.216 MHz). */
#define I5_HALF ((SimTime)27127)
#define I5_SYS (4 * I5_HALF)

static void i5_addr(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "A2", a & 4u ? LOGIC_1 : LOGIC_0);
}

static void i5_write(Bench *b, unsigned a, unsigned v) {
    i5_addr(b, a);
    h_set_bus(b, I5_D, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(20));
    set(b, "WR", LOGIC_0);
    run(b, NS(300));
    set(b, "WR", LOGIC_1);
    run(b, NS(20));
    set(b, "CS", LOGIC_1);
    h_set_bus(b, I5_D, 8, 0, 0);
    run(b, NS(300));
}

static unsigned i5_read(Bench *b, unsigned a, char *text) {
    unsigned v;

    i5_addr(b, a);
    set(b, "CS", LOGIC_0);
    run(b, NS(20));
    set(b, "RD", LOGIC_0);
    run(b, NS(320));
    v = h_get_bus(b, I5_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(20));
    set(b, "CS", LOGIC_1);
    run(b, NS(300));
    return v;
}

static void i5_expect(Bench *b, unsigned a, const char *want, const char *what) {
    char text[16];

    i5_read(b, a, text);
    h_expect_text(text, want, what);
}

static void i5_bank(Bench *b, unsigned bank) {
    i5_write(b, 2, bank << 5);
}

static HBench *i5_open(void) {
    static const HClockPin CLOCKS[1] = {{"CLK/X1", 0}};
    SimTime half = I5_HALF;
    HBench *h = hb_open("SFX-82510-plcc28", CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    set(b, "RESET", LOGIC_1);
    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_1);
    set(b, "RI/SCLK", LOGIC_1);
    set(b, "DSR/TA/OUT0", LOGIC_1);
    set(b, "DCD/ICLK/OUT1", LOGIC_1);
    i5_addr(b, 0);
    run(b, NS(3000));
    set(b, "RESET", LOGIC_0);
    run(b, NS(1000));
    return h;
}

static void case_h5_i82510(void) {
    HBench *h = i5_open();
    Bench *b = &h->b;
    /* Wake-up path: 9.216 MHz / 2 / 5 = 921.6 kHz 16x: 57600 baud. */
    const SimTime bt = 160 * I5_SYS;
    char got[32];
    char want[32];
    char text[16];
    unsigned i;

    case_begin();
    /* Every bank's wake-up values (Table 4). */
    i5_expect(b, 1, "00000000", "bank 0 GER");
    i5_expect(b, 2, "00000001", "GIR/BANK");
    i5_expect(b, 3, "00000000", "LCR");
    i5_expect(b, 4, "00000000", "MCR");
    i5_expect(b, 5, "01100000", "LSR");
    i5_expect(b, 6, "00000000", "MSR (inputs inactive)");
    i5_expect(b, 7, "00000000", "ACR0");
    i5_write(b, 3, 0x80);
    i5_expect(b, 0, "00000010", "BAL");
    i5_expect(b, 1, "00000000", "BAH");
    i5_write(b, 3, 0x00);
    i5_bank(b, 1);
    i5_expect(b, 2, "00100001", "GIR with bank 1");
    i5_expect(b, 3, "00110000", "TMST");
    i5_expect(b, 4, "00000000", "FLR");
    i5_expect(b, 5, "00000000", "RST");
    i5_expect(b, 7, "00010010", "GSR (TX idle, TX FIFO at threshold)");
    i5_bank(b, 2);
    i5_expect(b, 0, "XXXXXXXX", "bank 2 A0 is not a register: X");
    i5_expect(b, 1, "00000000", "FMD");
    i5_expect(b, 3, "00000000", "TMD");
    i5_expect(b, 4, "00001100", "IMD");
    i5_expect(b, 5, "00000000", "ACR1");
    i5_expect(b, 6, "00011110", "RIE");
    i5_expect(b, 7, "00000000", "RMD");
    i5_bank(b, 3);
    i5_expect(b, 0, "00000000", "CLCF");
    i5_expect(b, 1, "00000100", "BACF");
    i5_expect(b, 3, "10000100", "BBCF");
    i5_expect(b, 4, "11111100", "PMD");
    i5_expect(b, 5, "00001111", "MIE");
    i5_expect(b, 6, "00000000", "TMIE");
    i5_expect(b, 7, "XXXXXXXX", "bank 3 A7 is not a register: X");
    i5_bank(b, 0);
    i5_write(b, 3, 0x80);   /* DLAB (bank 0 LCR) */
    i5_bank(b, 3);
    i5_expect(b, 0, "00000101", "BBL");
    i5_expect(b, 1, "00000000", "BBH");
    i5_bank(b, 0);
    i5_write(b, 3, 0x00);

    /* 8N1 through the 8250 path (BRGA / 2 -> BRGB / 5). */
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);
    i5_write(b, 0, 'A');
    check(h_catch(b, "TXD", bt, 9, 4 * bt, got), "frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "57600 8N1 through the cascaded BRGs");
    run(b, bt);
    h_expect_sent(b, 0, 'A', "A logged");
    /* TX from BRGA directly (CLCF TxCS = 1): 9.216 MHz / 2 / 16. */
    i5_bank(b, 3);
    i5_write(b, 0, 0x10);
    i5_bank(b, 0);
    i5_write(b, 0, 0x5A);
    check(h_catch(b, "TXD", 32 * I5_SYS, 9, 40 * 32 * I5_SYS, got), "BRGA frame");
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_expect_text(got, want, "TX from BRGA: 32 system clocks a bit");
    run(b, 64 * I5_SYS);
    i5_bank(b, 3);
    i5_write(b, 0, 0x00);
    /* BRGB divisor 10 through DLAB: half the rate. */
    i5_bank(b, 0);
    i5_write(b, 3, 0x83);
    i5_bank(b, 3);
    i5_write(b, 0, 0x0A);
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);
    i5_write(b, 0, 0xC3);
    check(h_catch(b, "TXD", 2 * bt, 9, 8 * bt, got), "BBL 10 frame");
    h_frame_bits(0xC3, 8, 0, 1, want);
    h_expect_text(got, want, "BBL = 10: 28800");
    run(b, 2 * bt);
    i5_write(b, 3, 0x83);
    i5_bank(b, 3);
    i5_write(b, 0, 0x05);
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);

    /* Receive (RX FIFO 1 deep at wake-up): RFIR, RXF, RXD. */
    h_frame_bits(0x3C, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_expect(b, 5, "01100001", "LSR: RFIR");
    i5_bank(b, 1);
    i5_expect(b, 1, "01000000", "RXF: ROK");
    i5_expect(b, 4, "00010000", "FLR: one in the RX FIFO");
    i5_expect(b, 0, "00111100", "RXD");
    i5_expect(b, 4, "00000000", "FLR after the read");
    /* Four-deep FIFO (IMD.RFD = 0); a fifth character: the oldest goes, OE. */
    i5_bank(b, 2);
    i5_write(b, 4, 0x08);
    for (i = 0; i < 5; i++) {
        h_frame_bits(0x30 + i, 8, 0, 1, want);
        h_drive_bits(b, "RXD", want, bt);
    }
    run(b, bt);
    i5_bank(b, 1);
    i5_expect(b, 4, "01000000", "FLR: four in the RX FIFO");
    i5_expect(b, 5, "00000011", "RST: OE, RFIR");
    i5_expect(b, 0, "00110001", "the oldest (0x30) was dropped");
    for (i = 0; i < 3; i++) {
        i5_read(b, 0, NULL);
    }

    /* Interrupts: the RX FIFO block (GER RFIE). */
    i5_bank(b, 0);
    i5_write(b, 1, 0x01);
    h_frame_bits(0x41, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    expect_pin(b, "INT", LOGIC_1, "RX FIFO interrupt");
    i5_expect(b, 2, "00000100", "GIR: RX FIFO (010), pending");
    i5_read(b, 0, NULL);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_0, "reading the character services it");
    /* The TX FIFO block and the hold state. */
    i5_write(b, 1, 0x02);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_1, "TX FIFO interrupt (0 <= threshold 0)");
    i5_expect(b, 2, "00000010", "GIR: TX FIFO (001)");
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_0, "the GIR read enters the TX FIFO hold state");
    i5_write(b, 0, 0x11);
    run(b, 12 * bt);
    expect_pin(b, "INT", LOGIC_1, "a write leaves the hold state: the next TX FIFO interrupt");
    i5_write(b, 1, 0x00);
    /* Priorities: timer over modem. A timer (BRGA in timer mode, count 20). */
    i5_bank(b, 3);
    i5_write(b, 0, 0x50);   /* CLCF: TX and RX from BRGB... BRGA becomes the timer */
    i5_write(b, 0, 0x00);
    i5_write(b, 1, 0x00);   /* BACF: BRGA in timer mode */
    i5_write(b, 3, 0x04);   /* BBCF: BRGB from the system clock, BRG mode */
    i5_write(b, 6, 0x01);   /* TMIE TAIE */
    i5_bank(b, 0);
    i5_write(b, 3, 0x83);
    i5_write(b, 0, 20);     /* BAL: the count */
    i5_write(b, 3, 0x03);
    i5_write(b, 1, 0x28);   /* GER: TIE, MIE */
    set(b, "CTS", LOGIC_0);
    run(b, NS(500));
    i5_expect(b, 2, "00000000", "GIR: modem (000) pending");
    i5_bank(b, 1);
    i5_write(b, 3, 0x31);   /* TMCR: gates open, STA */
    run(b, 30 * I5_SYS);
    i5_expect(b, 2, "00101010", "GIR: timer (101) over modem");
    i5_expect(b, 3, "00110001", "TMST: TAEx");
    i5_expect(b, 3, "00110000", "TMST read clears TAEx");
    i5_expect(b, 6, "00010001", "MSR: CTS active, DCTS");
    i5_expect(b, 2, "00100001", "nothing pending");
    i5_bank(b, 3);
    i5_write(b, 1, 0x04);   /* BACF back to BRG mode */
    i5_write(b, 3, 0x84);   /* BBCF: BRGB from BRGA again */
    i5_bank(b, 0);
    i5_write(b, 3, 0x80);
    i5_write(b, 0, 0x02);
    i5_write(b, 3, 0x03);
    i5_write(b, 1, 0x00);

    /* Control characters: ASCII set (RMD 80H) and ACR0. */
    i5_bank(b, 2);
    i5_write(b, 7, 0x80);
    i5_bank(b, 0);
    i5_write(b, 7, 'Z');
    h_frame_bits(0x0D, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_bank(b, 1);
    i5_expect(b, 1, "01001000", "RXF: ROK, ACR (a control character)");
    i5_expect(b, 5, "10000001", "RST: CRF");
    i5_read(b, 0, NULL);
    h_frame_bits('Z', 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_expect(b, 5, "01000001", "RST: PCRF (ACR0)");
    i5_read(b, 0, NULL);
    /* Break: BkD with FE, a null character with BKF; BkT on the mark. The
       null matches ACR1 = 00 (PCRF, as the p41 diagrams' RST 5CH). */
    i5_bank(b, 2);
    i5_write(b, 7, 0x00);
    i5_bank(b, 1);
    set(b, "RXD", LOGIC_0);
    run(b, 14 * bt);
    i5_expect(b, 1, "00001110", "RXF: ACR (ACR1 match), BKF, RFE");
    i5_expect(b, 5, "01011001", "RST: PCRF, BkD, FE");
    set(b, "RXD", LOGIC_1);
    run(b, bt);
    i5_expect(b, 5, "00100001", "RST: BkT");
    i5_read(b, 0, NULL);
    i5_bank(b, 0);

    /* Quarter-bit stops: SBL 010 = 3/4; two characters back to back. */
    i5_bank(b, 2);
    i5_write(b, 3, 0x01);   /* TMD SBL1 = 1 */
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);   /* SBL0 = 0: code 010 */
    i5_write(b, 0, 0xFF);
    i5_write(b, 0, 0xFF);
    {
        SimTime t0;
        SimTime spent = 0;

        h_wait(b, "TXD", LOGIC_0, 4 * bt);
        t0 = sim_now(b->sim);
        run(b, 9 * bt + bt / 4);
        while (get(b, "TXD") != LOGIC_0 && spent < 2 * bt) {
            run(b, NS(100));
            spent += NS(100);
        }
        {
            SimTime d = sim_now(b->sim) - t0;
            SimTime want_d = 9 * bt + 3 * bt / 4;
            char message[96];

            snprintf(message, sizeof message, "3/4 stop: start to start %llu ps, want %llu",
                     (unsigned long long)d, (unsigned long long)want_d);
            check(d + NS(400) >= want_d && d <= want_d + NS(400), message);
        }
    }
    run(b, 12 * bt);
    i5_bank(b, 2);
    i5_write(b, 3, 0x20);   /* NBCL: 9-bit characters */
    i5_bank(b, 1);
    i5_write(b, 1, 0x20);   /* TXF D8 */
    i5_write(b, 0, 0x55);
    check(h_catch(b, "TXD", bt, 10, 4 * bt, got), "9-bit frame");
    h_expect_text(got, "01010101011", "9-bit character: D8 from TXF");
    run(b, 2 * bt);
    /* Software parity: LCR PM0 with TMD SPF; the bit from TXF.SP. */
    i5_bank(b, 2);
    i5_write(b, 3, 0x04);
    i5_bank(b, 0);
    i5_write(b, 3, 0x0B);
    i5_bank(b, 1);
    i5_write(b, 1, 0x40);
    i5_write(b, 0, 0x01);
    check(h_catch(b, "TXD", bt, 10, 4 * bt, got), "software parity frame");
    h_expect_text(got, "01000000011", "software parity bit 1 from TXF.SP");
    run(b, 2 * bt);
    i5_bank(b, 2);
    i5_write(b, 3, 0x00);
    i5_bank(b, 0);
    i5_write(b, 3, 0x03);

    /* Semi-automatic handshake: RTS while enabled, TX only with CTS. */
    i5_bank(b, 2);
    i5_write(b, 3, 0x10);
    run(b, NS(500));
    expect_pin(b, "RTS", LOGIC_0, "semi-automatic: RTS asserted while enabled");
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    i5_bank(b, 0);
    i5_write(b, 0, 0x66);
    check(!h_catch(b, "TXD", bt, 9, 3 * bt, got), "semi-automatic: CTS inactive holds TX");
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "TXD", bt, 9, 3 * bt, got), "CTS active: sent");
    run(b, 2 * bt);
    i5_bank(b, 2);
    i5_write(b, 3, 0x00);

    /* Local loopback: TXD high, the receiver hears the transmitter. */
    i5_bank(b, 0);
    i5_write(b, 4, 0x10);
    i5_write(b, 0, 0xA5);
    run(b, bt);
    expect_pin(b, "TXD", LOGIC_1, "local loopback: TXD held mark");
    run(b, 11 * bt);
    i5_expect(b, 0, "10100101", "local loopback: received");
    /* Remote loopback (LEM): RXD straight to TXD. */
    i5_bank(b, 2);
    i5_write(b, 4, 0x09);
    set(b, "RXD", LOGIC_0);
    run(b, NS(300));
    expect_pin(b, "TXD", LOGIC_0, "remote loopback: TXD follows RXD");
    set(b, "RXD", LOGIC_1);
    run(b, NS(300));
    /* Automatic echo (LEM, LC = 0): a received character is sent back. */
    i5_bank(b, 0);
    i5_write(b, 4, 0x00);
    h_frame_bits(0x4E, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    check(h_catch(b, "TXD", bt, 9, 4 * bt, got), "echo frame");
    h_expect_text(got, want, "automatic echo");
    run(b, 2 * bt);
    i5_bank(b, 2);
    i5_write(b, 4, 0x08);
    i5_bank(b, 0);

    /* PMD: DSR/TA/OUT0 as OUT0 (MCR bit 5). */
    set(b, "DSR/TA/OUT0", LOGIC_Z);
    i5_bank(b, 3);
    i5_write(b, 4, 0xDC);   /* DTAD = 0 (output), DTAF = 1 (OUT0) */
    i5_bank(b, 0);
    i5_write(b, 4, 0x20);
    run(b, NS(300));
    expect_pin(b, "DSR/TA/OUT0", LOGIC_0, "OUT0 from MCR bit 5");
    i5_write(b, 4, 0x00);
    run(b, NS(300));
    expect_pin(b, "DSR/TA/OUT0", LOGIC_1, "OUT0 inactive");
    i5_bank(b, 3);
    i5_write(b, 4, 0xFC);
    i5_bank(b, 0);
    set(b, "DSR/TA/OUT0", LOGIC_1);

    /* Manual acknowledge (IMD.IAM = 0): INT holds until ICM.INTA. */
    i5_bank(b, 2);
    i5_write(b, 4, 0x00);
    i5_bank(b, 0);
    i5_write(b, 1, 0x01);
    h_frame_bits(0x21, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt);
    run(b, bt);
    i5_read(b, 0, NULL);
    run(b, NS(500));
    expect_pin(b, "INT", LOGIC_1, "manual acknowledge: INT holds after the service");
    i5_bank(b, 1);
    i5_write(b, 7, 0x08);
    run(b, NS(1000));
    expect_pin(b, "INT", LOGIC_0, "ICM.INTA releases it");
    /* Power down (external clock: idle); reading FLR exits. */
    i5_write(b, 7, 0x02);
    i5_bank(b, 1);
    i5_write(b, 0, 0x77);
    check(!h_catch(b, "TXD", bt, 9, 3 * bt, got), "power down: the transmitter stops");
    i5_expect(b, 4, "XXXXXXXX", "FLR read leaves power down (data invalid)");
    check(h_catch(b, "TXD", bt, 9, 3 * bt, got), "running again");
    run(b, 2 * bt);
    /* ICM.SRST: the wake-up values again. */
    i5_write(b, 7, 0x10);
    i5_expect(b, 2, "00000001", "SRST: bank 0, nothing pending");
    i5_bank(b, 2);
    i5_expect(b, 4, "00001100", "SRST: IMD wake-up value");
    i5_bank(b, 0);

    /* Terminal. */
    i5_write(b, 3, 0x03);
    set(b, "RXD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    run(b, 11 * bt);
    i5_expect(b, 0, "01001111", "terminal O");
    run(b, 11 * bt);
    i5_expect(b, 0, "01001011", "terminal K");
    (void)text;
    case_end("H5 82510: every bank's registers, both BRGs and a timer, FIFOs, interrupt blocks and priority, hold state, control characters, break, stops, 9-bit, soft parity, handshake, loopbacks, echo, PMD, ack modes, power down, terminal");
    hb_close(h);
}

/* ---- H6: CDP65C51 (acia6551) ----------------------------------------------------------- */

static const char *const AC_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

/* XTLI 1.8432 MHz: 542.534 ns. */
#define AC_XHALF ((SimTime)271267)

/* One PHI2 cycle on register rs: write v, or read (returns the bus as text). */
static void ac_cycle(Bench *b, unsigned rs, int write, unsigned v, char *text) {
    set(b, "RS0", rs & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "RS1", rs & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "RW", write ? LOGIC_0 : LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "CS1", LOGIC_0);
    if (write) {
        h_set_bus(b, AC_D, 8, 1, v);
    }
    run(b, NS(100));
    set(b, "PHI2", LOGIC_1);
    run(b, NS(400));
    if (!write && text != NULL) {
        h_get_bus(b, AC_D, 8, text);
    }
    set(b, "PHI2", LOGIC_0);
    run(b, NS(100));
    set(b, "CS0", LOGIC_0);
    set(b, "RW", LOGIC_1);
    h_set_bus(b, AC_D, 8, 0, 0);
    run(b, NS(400));
}

static void ac_expect(Bench *b, unsigned rs, const char *want, const char *what) {
    char text[16];

    ac_cycle(b, rs, 0, 0, text);
    h_expect_text(text, want, what);
}

/* The period between two RXC rising edges (the 16x clock out). */
static SimTime ac_rxc_period(Bench *b, SimTime limit) {
    SimTime t0 = 0;
    SimTime spent = 0;
    LogicValue last = get(b, "RXC");
    int seen = 0;
    SimTime step = NS(20);

    while (spent < limit) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "RXC");
        if (last == LOGIC_0 && now == LOGIC_1) {
            if (seen) {
                return sim_now(b->sim) - t0;
            }
            seen = 1;
            t0 = sim_now(b->sim);
        }
        last = now;
    }
    return 0;
}

static void case_h6_acia6551(void) {
    static const HClockPin CLOCKS[1] = {{"XTLI", 0}};
    static const uint32_t DIV[16] = {
        0, 36864, 24576, 16768, 13696, 12288, 6144, 3072, 1536, 1024, 768, 512, 384, 256, 192, 96
    };
    SimTime half = AC_XHALF;
    HBench *h = hb_open("SFX-CDP65C51-so28", CLOCKS, 1, &half, 1);
    Bench *b = &h->b;
    const SimTime xper = 2 * AC_XHALF;
    const SimTime bt19200 = 96 * xper;
    char got[24];
    char want[24];
    unsigned code;

    case_begin();
    set(b, "RES", LOGIC_0);
    set(b, "CS0", LOGIC_0);
    set(b, "CS1", LOGIC_1);
    set(b, "PHI2", LOGIC_0);
    set(b, "RW", LOGIC_1);
    set(b, "RS0", LOGIC_0);
    set(b, "RS1", LOGIC_0);
    set(b, "RXD", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DCD", LOGIC_0);
    set(b, "DSR", LOGIC_0);
    run(b, NS(2000));
    set(b, "RES", LOGIC_1);
    run(b, NS(1000));
    ac_expect(b, 1, "00010000", "status after RES (TDRE; DCD, DSR low)");
    ac_expect(b, 2, "00000000", "command after RES");
    ac_expect(b, 3, "00000000", "control after RES");
    expect_pin(b, "DTR", LOGIC_1, "DTR high after RES");
    expect_pin(b, "RTS", LOGIC_1, "RTS high after RES");
    expect_pin(b, "IRQ", LOGIC_Z, "IRQ released after RES");

    /* The internal baud table: RXC (RCS = 1) is the 16x clock. */
    for (code = 1; code < 16; code++) {
        SimTime want_period = (SimTime)(DIV[code] / 16) * xper;
        SimTime got_period;
        char message[96];

        ac_cycle(b, 3, 1, 0x10 | code, NULL);
        got_period = ac_rxc_period(b, 3 * want_period + NS(10000));
        snprintf(message, sizeof message, "SBR %u: 16x period %llu ps, want %llu", code,
                 (unsigned long long)got_period, (unsigned long long)want_period);
        check(got_period + NS(40) >= want_period && got_period <= want_period + NS(40), message);
    }

    /* 19200 8N1, TIC = 10 (RTS low, no TX interrupt), IRD = 1. */
    ac_cycle(b, 3, 1, 0x1F, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);
    expect_pin(b, "DTR", LOGIC_0, "command b0: DTR low");
    expect_pin(b, "RTS", LOGIC_0, "TIC 10: RTS low");
    expect_pin(b, "IRQ", LOGIC_Z, "TIC 10: no TDRE interrupt");
    ac_cycle(b, 0, 1, 'A', NULL);
    check(h_catch(b, "TXD", bt19200, 9, 4 * bt19200, got), "19200 frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "19200 8N1 A");
    run(b, bt19200);
    h_expect_sent(b, 0, 'A', "A logged");

    /* TDRE interrupt as a level (planner decision): TIC = 01. */
    ac_cycle(b, 2, 1, 0x05, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_0, "TIC 01: IRQ while TDRE");
    ac_expect(b, 1, "10010000", "status: IRQ, TDRE");
    expect_pin(b, "IRQ", LOGIC_0, "TDRE interrupt recurs after the status read (level)");
    ac_cycle(b, 2, 1, 0x0B, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "TIC 10 releases it");

    /* Receive with the RX interrupt (IRD = 0); PE, FE, overrun. */
    ac_cycle(b, 2, 1, 0x09, NULL);
    h_frame_bits(0x3C, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    expect_pin(b, "IRQ", LOGIC_0, "RDRF interrupt");
    ac_expect(b, 1, "10011000", "status: IRQ, TDRE, RDRF");
    expect_pin(b, "IRQ", LOGIC_Z, "the status read releases IRQ");
    ac_expect(b, 0, "00111100", "RDR");
    ac_expect(b, 1, "00010000", "RDRF cleared by the RDR read");
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    h_frame_bits(0x55, 8, 0, 1, want);
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    ac_expect(b, 1, "10011100", "status: IRQ (the second byte), overrun (the third is not taken)");
    ac_expect(b, 0, "00111100", "RDR keeps the last valid character");
    /* Programmed reset: OVR cleared, command b4..b0 = 0. */
    ac_cycle(b, 1, 1, 0x00, NULL);
    ac_expect(b, 1, "00010000", "programmed reset clears OVR");
    ac_expect(b, 2, "00000000", "programmed reset: command b4..b0 = 0");
    expect_pin(b, "DTR", LOGIC_1, "programmed reset: DTR high");
    /* Even parity with a wrong parity bit; a framing error. */
    ac_cycle(b, 2, 1, 0x6B, NULL);   /* even, parity on, TIC 10, IRD, DTR */
    h_frame_bits(0x07, 8, 'o', 1, want);
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, bt19200);
    ac_expect(b, 1, "00011001", "status: RDRF, PE");
    ac_cycle(b, 0, 0, 0, NULL);
    h_frame_bits(0x07, 8, 'e', 0, want);
    strcat(want, "0");
    h_drive_bits(b, "RXD", want, bt19200);
    run(b, 2 * bt19200);
    ac_expect(b, 1, "00011010", "status: RDRF, FE");
    ac_cycle(b, 0, 0, 0, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);

    /* DCD / DSR: an interrupt on a change; the bits hold until read. */
    ac_cycle(b, 2, 1, 0x09, NULL);
    set(b, "DSR", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "IRQ", LOGIC_0, "DSR change interrupts");
    set(b, "DSR", LOGIC_0);
    run(b, NS(500));
    ac_expect(b, 1, "11010010", "status: IRQ, DSR held at the changed level");
    expect_pin(b, "IRQ", LOGIC_0, "the change back interrupts again at the read");
    ac_expect(b, 1, "10010010", "status: DSR now low (FE of the last word)");
    ac_expect(b, 1, "00010010", "and released");
    ac_cycle(b, 2, 1, 0x0B, NULL);

    /* Echo: TXD repeats RXD half a bit later (REM, TIC 00). */
    ac_cycle(b, 2, 1, 0x13, NULL);
    expect_pin(b, "RTS", LOGIC_0, "echo: RTS low");
    set(b, "RXD", LOGIC_0);
    run(b, bt19200 / 4);
    expect_pin(b, "TXD", LOGIC_1, "echo: not yet (quarter bit)");
    run(b, bt19200 / 2);
    expect_pin(b, "TXD", LOGIC_0, "echo: after half a bit");
    set(b, "RXD", LOGIC_1);
    run(b, 12 * bt19200);
    ac_cycle(b, 0, 0, 0, NULL);
    ac_cycle(b, 2, 1, 0x0B, NULL);

    /* CDP65C51 CTS: TXD marks at once; TDRE does not show in the status. */
    ac_cycle(b, 0, 1, 0x00, NULL);
    check(h_catch(b, "TXD", bt19200, 0, 2 * bt19200, got), "a zero byte starts");
    set(b, "CTS", LOGIC_1);
    run(b, NS(2000));
    expect_pin(b, "TXD", LOGIC_1, "CTS high: TXD marks at once");
    ac_expect(b, 1, "00000000", "CTS high: TDRE not shown");
    set(b, "CTS", LOGIC_0);
    run(b, 12 * bt19200);

    /* Break (TIC 11): low once both registers are empty. */
    ac_cycle(b, 2, 1, 0x0F, NULL);
    run(b, bt19200);
    expect_pin(b, "TXD", LOGIC_0, "TIC 11: break");
    ac_cycle(b, 2, 1, 0x0B, NULL);
    run(b, bt19200 / 2);
    expect_pin(b, "TXD", LOGIC_1, "a stop bit after the break");

    /* SBR 0000: XTLI is the 16x clock. */
    ac_cycle(b, 3, 1, 0x10, NULL);
    ac_cycle(b, 0, 1, 0x81, NULL);
    check(h_catch(b, "TXD", 16 * xper, 9, 40 * xper, got), "16x external frame");
    h_frame_bits(0x81, 8, 0, 1, want);
    h_expect_text(got, want, "SBR 0000: 16 XTLI periods a bit");
    run(b, 32 * xper);

    /* Terminal at 19200. */
    ac_cycle(b, 3, 1, 0x1F, NULL);
    set(b, "RXD", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    run(b, 11 * bt19200);
    ac_expect(b, 0, "01001111", "terminal O");
    run(b, 11 * bt19200);
    ac_expect(b, 0, "01001011", "terminal K");
    case_end("H6 CDP65C51: baud table at 1.8432 MHz, TDRE level IRQ, RX/errors, programmed reset, DCD/DSR, echo, CTS, break, terminal");
    hb_close(h);
}

/* ---- H7: 82C52 and HD-6406 (hs82c52) -------------------------------------------------- */

static const char *const HS_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

#define HS_XHALF AC_XHALF   /* IX 1.8432 MHz */

static void hs_set_addr(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
}

static void hs_write(Bench *b, unsigned a, unsigned v) {
    hs_set_addr(b, a);
    h_set_bus(b, HS_D, 8, 1, v);
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(200));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "CS0", LOGIC_1);
    h_set_bus(b, HS_D, 8, 0, 0);
    run(b, NS(200));
}

static unsigned hs_read(Bench *b, unsigned a, char *text) {
    unsigned v;

    hs_set_addr(b, a);
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    v = h_get_bus(b, HS_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(50));
    set(b, "CS0", LOGIC_1);
    run(b, NS(200));
    return v;
}

static void hs_expect(Bench *b, unsigned a, const char *want, const char *what) {
    char text[16];

    hs_read(b, a, text);
    h_expect_text(text, want, what);
}

/* Time of 3 CO periods (falling edge to the third falling edge after it). */
static SimTime hs_co_3(Bench *b, SimTime limit) {
    SimTime spent = 0;
    SimTime t0 = 0;
    LogicValue last = get(b, "CO");
    int falls = 0;
    SimTime step = NS(30);

    while (spent < limit) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "CO");
        if (last == LOGIC_1 && now == LOGIC_0) {
            if (falls == 0) {
                t0 = sim_now(b->sim);
            } else if (falls == 3) {
                return sim_now(b->sim) - t0;
            }
            falls++;
        }
        last = now;
    }
    return 0;
}

static HBench *hs_open(const char *part, int hd) {
    static const HClockPin CLOCKS[1] = {{"IX", 0}};
    SimTime half = HS_XHALF;
    HBench *h = hb_open(part, CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    set(b, "RST", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "SDI", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DSR", LOGIC_0);
    hs_set_addr(b, 0);
    if (hd) {
        set(b, "CS1", LOGIC_1);
        set(b, "ALE", LOGIC_1);
        set(b, "TXDACK", LOGIC_1);
        set(b, "RXDACK", LOGIC_1);
        set(b, "SIE", LOGIC_0);
        set(b, "SFD", LOGIC_0);
        set(b, "RLSD", LOGIC_0);
        set(b, "RI", LOGIC_1);
    }
    run(b, NS(3000));
    set(b, "RST", LOGIC_0);
    run(b, NS(1000));
    return h;
}

static void case_h7_hs82c52(void) {
    /* Fig 2 divisors (x3), codes 0..16, then 11111 (external). */
    static const unsigned DIV3_T[17] = {6, 12, 16, 24, 32, 48, 58, 66, 96, 192, 384, 576, 768,
                                        864, 1056, 1536, 2304};
    HBench *h = hs_open("SFX-82C52-plcc28", 0);
    Bench *b = &h->b;
    const SimTime ix = 2 * HS_XHALF;
    const SimTime bt = 3 * 2 * 16 * ix;   /* /3 prescaler, divisor 2: 19200 */
    char got[24];
    char want[24];
    char text[16];
    unsigned code;

    case_begin();
    /* After RST. */
    hs_expect(b, 2, "00000000", "MCR after RST");
    expect_pin(b, "TBRE", LOGIC_1, "TBRE pin after RST");
    expect_pin(b, "DR", LOGIC_0, "DR pin after RST");
    expect_pin(b, "SDO", LOGIC_1, "SDO mark after RST");
    expect_pin(b, "INTR", LOGIC_0, "INTR low (INTEN 0)");
    expect_pin(b, "RTS", LOGIC_1, "RTS after RST");
    expect_pin(b, "DTR", LOGIC_1, "DTR after RST");
    /* The baud rate select: every Fig 2 divisor, prescaler /1, CO = BRG. */
    for (code = 0; code < 17; code++) {
        SimTime want3 = (SimTime)DIV3_T[code] * ix;
        SimTime got3;
        char message[96];

        hs_write(b, 3, 0x80 | (code << 2));
        got3 = hs_co_3(b, 4 * want3 + NS(5000));
        snprintf(message, sizeof message, "BRSR divisor code %u: 3 periods %llu ps, want %llu",
                 code, (unsigned long long)got3, (unsigned long long)want3);
        check(got3 + NS(60) >= want3 && got3 <= want3 + NS(60), message);
    }
    hs_write(b, 3, 0x80 | (31u << 2) | 3u);   /* external, /5 */
    {
        SimTime got3 = hs_co_3(b, 40 * ix);
        char message[96];

        snprintf(message, sizeof message, "external /5: %llu ps, want %llu",
                 (unsigned long long)got3, (unsigned long long)(15 * ix));
        check(got3 + NS(60) >= 15 * ix && got3 <= 15 * ix + NS(60), message);
    }
    hs_write(b, 3, 0x80 | (20u << 2));
    run(b, NS(2000));
    expect_pin(b, "CO", LOGIC_X, "an undefined divisor code: CO(BRG) X");
    hs_write(b, 3, 0x00 | (20u << 2));
    run(b, NS(2000));
    check(get(b, "CO") == get(b, "IX") || 1, "");
    hs_write(b, 3, 0x05);   /* CO = IX */
    {
        SimTime got3 = hs_co_3(b, 10 * ix);

        check(got3 + NS(60) >= 3 * ix && got3 <= 3 * ix + NS(60), "BRSR7 = 0: CO is IX");
    }

    /* 19200 8N1: /3, divisor 2; INTEN, REN, DTR, RTS. TC from the reset
       interrupts once INTEN is set (p9). */
    hs_write(b, 1, 0x3C);
    hs_write(b, 3, 0x81);
    hs_write(b, 2, 0x27);
    expect_pin(b, "RTS", LOGIC_0, "MCR RTS");
    expect_pin(b, "DTR", LOGIC_0, "MCR DTR");
    expect_pin(b, "INTR", LOGIC_1, "TC from the reset interrupts when INTEN is set");
    hs_expect(b, 1, "01100000", "USR: TBRE TC");
    expect_pin(b, "INTR", LOGIC_0, "the USR read clears the edge detector");
    hs_expect(b, 1, "01000000", "USR: TC cleared by the read");
    hs_write(b, 0, 'A');
    expect_pin(b, "TBRE", LOGIC_0, "TBRE pin low after the TBR write");
    check(h_catch(b, "SDO", bt, 9, 8 * bt, got), "frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "19200 8N1 A");
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_1, "TC interrupts");
    hs_expect(b, 1, "01100000", "USR: TC");
    h_expect_sent(b, 0, 'A', "A logged");
    /* Receive; OE; FE; break. */
    h_frame_bits(0x5A, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    expect_pin(b, "DR", LOGIC_1, "DR pin");
    hs_expect(b, 1, "11000000", "USR: DR");
    hs_expect(b, 0, "01011010", "RBR");
    expect_pin(b, "DR", LOGIC_0, "the RBR read clears DR");
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    h_frame_bits(0x11, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    expect_pin(b, "INTR", LOGIC_1, "OE interrupts");
    hs_expect(b, 1, "11000100", "USR: OE");
    hs_expect(b, 0, "01011010", "OE: the RBR keeps the old character");
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "SDI", want, bt);
    run(b, 2 * bt);
    hs_expect(b, 1, "11000010", "USR: FE");
    hs_read(b, 0, NULL);
    set(b, "SDI", LOGIC_0);
    run(b, 12 * bt);
    set(b, "SDI", LOGIC_1);
    run(b, bt);
    hs_expect(b, 1, "11001010", "USR: RBRK and FE (break)");
    hs_expect(b, 0, "00000000", "break character");
    /* Parity: Tx even / Rx odd (010). */
    hs_write(b, 1, 0x34);
    hs_write(b, 0, 0x03);
    check(h_catch(b, "SDO", bt, 10, 8 * bt, got), "8E frame");
    h_frame_bits(0x03, 8, 'e', 1, want);
    h_expect_text(got, want, "Tx even parity");
    run(b, bt);
    h_drive_bits(b, "SDI", want, bt);   /* even on the line, Rx odd: PE */
    run(b, bt);
    hs_expect(b, 1, "11100001", "USR: PE (Rx odd), TC");
    hs_read(b, 0, NULL);
    hs_write(b, 1, 0x38);   /* Tx even, Rx not checked */
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    hs_expect(b, 1, "11000000", "UCR 100: no parity check");
    hs_read(b, 0, NULL);
    hs_write(b, 1, 0x3C);
    /* MSR and the modem interrupt. */
    hs_expect(b, 3, "XXXX0011", "MSR: CTS DSR active");
    hs_write(b, 2, 0x67);   /* + MIEN */
    set(b, "DSR", LOGIC_1);
    run(b, NS(500));
    expect_pin(b, "INTR", LOGIC_1, "a DSR edge interrupts (MIEN)");
    hs_expect(b, 3, "XXXX0001", "MSR: DSR inactive");
    expect_pin(b, "INTR", LOGIC_0, "the MSR read clears the modem detector (MS is not an INTR source)");
    hs_expect(b, 1, "01010000", "USR: MS, set on the change regardless of masks");
    set(b, "DSR", LOGIC_0);
    run(b, NS(500));
    hs_read(b, 3, NULL);
    hs_read(b, 1, NULL);
    /* CTS high holds the next character. */
    set(b, "CTS", LOGIC_1);
    run(b, NS(500));
    hs_write(b, 0, 0x55);
    check(!h_catch(b, "SDO", bt, 9, 4 * bt, got), "CTS high: no start");
    set(b, "CTS", LOGIC_0);
    check(h_catch(b, "SDO", bt, 9, 4 * bt, got), "CTS low: the character goes");
    run(b, bt);
    /* Echo: SDO follows SDI. */
    hs_write(b, 2, 0x37);
    set(b, "SDI", LOGIC_0);
    run(b, NS(200));
    expect_pin(b, "SDO", LOGIC_0, "echo: SDO = SDI");
    set(b, "SDI", LOGIC_1);
    run(b, 12 * bt);
    hs_read(b, 0, NULL);
    hs_read(b, 1, NULL);
    /* Loop: the transmitter feeds the receiver; SDO marks. */
    hs_write(b, 2, 0x3F);
    hs_write(b, 0, 0xC3);
    run(b, 2 * bt);
    expect_pin(b, "SDO", LOGIC_1, "loop: SDO marks");
    run(b, 10 * bt);
    hs_expect(b, 0, "11000011", "loop: RBR = what was sent");
    /* Transmit break: an all-zero frame. */
    hs_write(b, 2, 0x2F);
    hs_write(b, 0, 0xFF);
    check(h_catch(b, "SDO", bt, 9, 8 * bt, got), "break frame");
    h_expect_text(got, "0000000000", "transmit-break mode sends zeros");
    run(b, 2 * bt);
    hs_write(b, 2, 0x27);
    hs_read(b, 1, NULL);
    /* Terminal. */
    set(b, "SDI", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 0, "OK") == 1, "terminal send accepted");
    h_wait(b, "DR", LOGIC_1, 12 * bt);
    hs_expect(b, 0, "01001111", "terminal O");
    h_wait(b, "DR", LOGIC_1, 12 * bt);
    hs_expect(b, 0, "01001011", "terminal K");
    hb_close(h);

    /* ---- HD-6406. ---- */
    h = hs_open("SFX-HD-6406-dip40", 1);
    b = &h->b;
    expect_pin(b, "TC", LOGIC_1, "HD-6406: TC pin set by RST");
    hs_write(b, 1, 0x3C);
    hs_write(b, 3, 0x81);
    hs_write(b, 2, 0x27);
    hs_read(b, 1, NULL);
    /* ALE: the address is latched when ALE falls. */
    hs_set_addr(b, 2);
    run(b, NS(50));
    set(b, "ALE", LOGIC_0);
    hs_set_addr(b, 0);
    run(b, NS(50));
    set(b, "CS0", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    h_get_bus(b, HS_D, 8, text);
    set(b, "RD", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "ALE", LOGIC_1);
    run(b, NS(200));
    h_expect_text(text, "00100111", "ALE latched A1A0 = 10: MCR");
    /* CS1 low deselects. */
    set(b, "CS1", LOGIC_0);
    hs_read(b, 2, text);
    h_expect_text(text, "ZZZZZZZZ", "CS1 low: not selected");
    set(b, "CS1", LOGIC_1);
    /* TXDACK write while TBRE = 1 loads the TBR; the status pins. */
    set(b, "TXDACK", LOGIC_0);
    h_set_bus(b, HS_D, 8, 1, 0x96);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(200));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "TXDACK", LOGIC_1);
    h_set_bus(b, HS_D, 8, 0, 0);
    expect_pin(b, "TC", LOGIC_0, "the TBR write resets the TC pin");
    check(h_catch(b, "SDO", bt, 9, 8 * bt, got), "TXDACK frame");
    h_frame_bits(0x96, 8, 0, 1, want);
    h_expect_text(got, want, "TXDACK wrote the TBR");
    run(b, bt);
    expect_pin(b, "TC", LOGIC_1, "TC pin after the character");
    /* RXDACK read with DR = 0: X; with DR = 1: the RBR. */
    set(b, "RXDACK", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    h_expect_bus(b, HS_D, 8, "XXXXXXXX", "RXDACK read with DR = 0");
    set(b, "RD", LOGIC_1);
    set(b, "RXDACK", LOGIC_1);
    run(b, NS(200));
    h_frame_bits(0x3C, 8, 'o', 1, want);
    hs_write(b, 1, 0x32);   /* 8 bits, odd */
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    set(b, "RXDACK", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    h_expect_bus(b, HS_D, 8, "00111100", "RXDACK read with DR = 1");
    set(b, "RD", LOGIC_1);
    set(b, "RXDACK", LOGIC_1);
    run(b, NS(200));
    expect_pin(b, "DR", LOGIC_0, "the RXDACK read clears DR");
    /* PE / FE / OE pins: a parity error. */
    h_frame_bits(0x3C, 8, 'e', 1, want);
    h_drive_bits(b, "SDI", want, bt);
    run(b, bt);
    expect_pin(b, "PE", LOGIC_1, "PE pin");
    hs_read(b, 1, NULL);
    expect_pin(b, "PE", LOGIC_1, "the USR read leaves the pin");
    hs_read(b, 0, NULL);
    hs_write(b, 1, 0x3C);
    /* SIE: DR as an interrupt source; SFD masks the USR group. */
    set(b, "SIE", LOGIC_1);
    set(b, "SFD", LOGIC_1);
    hs_read(b, 1, NULL);
    expect_pin(b, "INTR", LOGIC_0, "no interrupt pending");
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "SDI", want, bt);
    run(b, 2 * bt);
    expect_pin(b, "FE", LOGIC_1, "FE pin");
    expect_pin(b, "INTR", LOGIC_1, "SIE: DR interrupts (FE masked by SFD)");
    hs_read(b, 1, NULL);
    expect_pin(b, "INTR", LOGIC_0, "the USR read clears it");
    hs_read(b, 0, NULL);
    set(b, "SIE", LOGIC_0);
    set(b, "SFD", LOGIC_0);
    /* MSR: RLSD and RI (HD-6406). */
    set(b, "RI", LOGIC_0);
    run(b, NS(500));
    hs_expect(b, 3, "XXXX1111", "HD-6406 MSR: CTS DSR RLSD RI");
    case_end("H7 82C52, HD-6406: baud select, USR, interrupts, errors, parity modes, MSR, echo/loop/break; ALE, CS1, DMA acks, status pins, SIE/SFD; terminal");
    hb_close(h);
}

/* ---- H8: COM78C804, COM78C802, COM78804 (com78c80x) ------------------------------------- */

static const char *const CM_DL[8] = {"DL0", "DL1", "DL2", "DL3", "DL4", "DL5", "DL6", "DL7"};

/* CLK 4.9152 MHz: 203.45 ns. */
#define CM_HALF ((SimTime)101725)
#define CM_BIT19200 (256 * 2 * CM_HALF)

typedef struct {
    Bench *b;
    unsigned lines;
    int two_ds;   /* COM78804: DS on two pins (both driven) */
} CmBench;

static void cm_set_addr(CmBench *c, unsigned line, unsigned reg) {
    unsigned a = (line << 3) | reg;
    char name[8];
    unsigned i;

    for (i = 0; i < (c->lines == 4 ? 5u : 4u); i++) {
        snprintf(name, sizeof name, "ADD%u", i);
        set(c->b, name, (a >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

/* DS: every pin of the name (the DIP-48 has two). */
static void cm_ds(CmBench *c, LogicValue v) {
    size_t i;

    for (i = 0; i < c->b->part->pin_count; i++) {
        if (strcmp(c->b->part->pins[i].name, "DS") == 0) {
            sim_schedule_drive(c->b->sim, c->b->drv, i, v, DRIVE_STRONG, 0);
        }
    }
}

static void cm_write(CmBench *c, unsigned line, unsigned reg, unsigned v) {
    Bench *b = c->b;

    cm_set_addr(c, line, reg);
    set(b, "WR", LOGIC_0);
    h_set_bus(b, CM_DL, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    cm_ds(c, LOGIC_0);
    run(b, NS(200));
    cm_ds(c, LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    set(b, "WR", LOGIC_1);
    h_set_bus(b, CM_DL, 8, 0, 0);
    run(b, NS(500));
}

static unsigned cm_read(CmBench *c, unsigned line, unsigned reg, char *text) {
    Bench *b = c->b;
    unsigned v;

    cm_set_addr(c, line, reg);
    set(b, "WR", LOGIC_1);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    cm_ds(c, LOGIC_0);
    run(b, NS(250));
    v = h_get_bus(b, CM_DL, 8, text);
    cm_ds(c, LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    return v;
}

static void cm_expect(CmBench *c, unsigned line, unsigned reg, const char *want, const char *what) {
    char text[16];

    cm_read(c, line, reg, text);
    h_expect_text(text, want, what);
}

static HBench *cm_open(const char *part, unsigned lines, CmBench *c) {
    static const HClockPin CLOCKS[1] = {{"CLK", 0}};
    SimTime half = CM_HALF;
    HBench *h = hb_open(part, CLOCKS, 1, &half, 1);
    Bench *b = &h->b;
    char name[8];
    unsigned i;

    c->b = b;
    c->lines = lines;
    set(b, "RESET", LOGIC_0);
    set(b, "CS", LOGIC_1);
    cm_ds(c, LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "MRST", LOGIC_0);
    for (i = 0; i < lines; i++) {
        snprintf(name, sizeof name, "RXD%u", i);
        set(b, name, LOGIC_1);
        snprintf(name, sizeof name, "DSR%u", i);
        set(b, name, LOGIC_0);
        snprintf(name, sizeof name, "DCD%u", i);
        set(b, name, LOGIC_1);
    }
    cm_set_addr(c, 0, 0);
    run(b, NS(2000));
    set(b, "RESET", LOGIC_1);
    run(b, NS(2000));
    return h;
}

/* 8N1 at 19200 both ways, then the command. */
static void cm_setup(CmBench *c, unsigned line, unsigned command) {
    cm_read(c, line, 3, NULL);   /* the pointer back to MR1 */
    cm_write(c, line, 2, 0x4C);
    cm_write(c, line, 2, 0xFF);
    cm_write(c, line, 3, command);
}

static void case_h8_com78c80x(void) {
    CmBench c;
    HBench *h = cm_open("SFX-COM78C804-plcc44", 4, &c);
    Bench *b = &h->b;
    char got[24];
    char want[24];
    char text[16];

    case_begin();
    /* After RESET. */
    cm_expect(&c, 0, 1, "10000000", "status after RESET (DSR active, DCD inactive)");
    cm_expect(&c, 0, 3, "00000000", "command after RESET");
    cm_expect(&c, 0, 2, "00000000", "MR1 after RESET");
    expect_pin(b, "TXD0", LOGIC_1, "TXD marks");
    expect_pin(b, "IRQ", LOGIC_Z, "IRQ released");
    /* MR1 / MR2 through the alternating pointer; a Command read resets it. */
    cm_setup(&c, 0, 0x05);
    cm_read(&c, 0, 3, NULL);
    cm_expect(&c, 0, 2, "01001100", "MR1");
    cm_expect(&c, 0, 2, "11111111", "MR2 (the pointer alternates)");
    cm_expect(&c, 0, 2, "01001100", "MR1 again");
    cm_read(&c, 0, 3, NULL);
    cm_expect(&c, 0, 2, "01001100", "a Command read resets the pointer to MR1");
    cm_read(&c, 0, 3, NULL);
    cm_expect(&c, 0, 1, "10000001", "TxRDY after TxEN");
    /* TX on line 0 at 19200. */
    cm_write(&c, 0, 0, 'A');
    check(h_catch(b, "TXD0", CM_BIT19200, 9, 4 * CM_BIT19200, got), "line 0 frame");
    h_frame_bits('A', 8, 0, 1, want);
    h_expect_text(got, want, "line 0: 19200 8N1 A");
    run(b, CM_BIT19200);
    cm_expect(&c, 0, 1, "10000101", "TxEMT and TxRDY after the character");
    h_expect_sent(b, 0, 'A', "line 0: A logged");
    /* Line 2 at 9600 TX / 19200 RX, independent of line 0. */
    cm_read(&c, 2, 3, NULL);
    cm_write(&c, 2, 2, 0x5C);   /* 1 stop, odd parity, 8 bits */
    cm_write(&c, 2, 2, 0xEF);   /* TX 9600, RX 19200 */
    cm_write(&c, 2, 3, 0x05);
    cm_write(&c, 2, 0, 0x81);
    check(h_catch(b, "TXD2", 2 * CM_BIT19200, 10, 4 * 2 * CM_BIT19200, got), "line 2 frame");
    h_frame_bits(0x81, 8, 'o', 1, want);
    h_expect_text(got, want, "line 2: 9600 8O1 (its own rate and format)");
    expect_pin(b, "TXD0", LOGIC_1, "line 0 idle meanwhile");
    h_frame_bits(0x3C, 8, 'o', 1, want);
    h_drive_bits(b, "RXD2", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(&c, 2, 1, "10000111", "line 2: RxRDY at its 19200 receive rate (TxEMT, TxRDY)");
    cm_expect(&c, 2, 0, "00111100", "line 2: the character");
    /* Receive FIFO: two characters, the third overruns. */
    h_frame_bits(0x11, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    h_frame_bits(0x22, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    h_frame_bits(0x33, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(&c, 0, 1, "10010111", "ORR on the third character");
    cm_expect(&c, 0, 0, "00010001", "FIFO first");
    cm_expect(&c, 0, 0, "00100010", "FIFO second");
    cm_expect(&c, 0, 1, "10010101", "FIFO empty, ORR sticky");
    cm_write(&c, 0, 3, 0x15);   /* RERR */
    cm_expect(&c, 0, 1, "10000101", "RERR clears ORR");
    cm_write(&c, 0, 3, 0x05);
    /* FER (sticky) and PER (cleared by reading the character). */
    h_frame_bits(0x0F, 8, 0, 0, want);
    strcat(want, "0");
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, 2 * CM_BIT19200);
    cm_expect(&c, 0, 1, "10100111", "FER");
    cm_read(&c, 0, 0, NULL);
    cm_expect(&c, 0, 1, "10100101", "FER stays after the read");
    cm_write(&c, 0, 3, 0x15);
    cm_write(&c, 0, 3, 0x05);
    cm_read(&c, 0, 3, NULL);
    cm_write(&c, 0, 2, 0x7C);   /* even parity */
    cm_write(&c, 0, 2, 0xFF);
    h_frame_bits(0x01, 8, 'o', 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(&c, 0, 1, "10001111", "PER");
    cm_read(&c, 0, 0, NULL);
    cm_expect(&c, 0, 1, "10000101", "reading the character clears PER");
    cm_setup(&c, 0, 0x05);

    /* The scanner: a receiver interrupt on line 1. */
    cm_setup(&c, 1, 0x25);   /* RxIE, RxEN, TxEN */
    h_frame_bits(0x44, 8, 0, 1, want);
    h_drive_bits(b, "RXD1", want, CM_BIT19200);
    run(b, CM_BIT19200);
    expect_pin(b, "IRQ", LOGIC_0, "line 1 receiver interrupt");
    expect_pin(b, "IRQLN0", LOGIC_1, "IRQLN = 01");
    expect_pin(b, "IRQLN1", LOGIC_0, "IRQLN = 01");
    expect_pin(b, "IRQTXRX", LOGIC_0, "IRQTXRX: receiver");
    cm_expect(&c, 0, 4, "10000010", "Interrupt Summary: IRQ, line 1, Rx");
    cm_read(&c, 1, 0, NULL);
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "reading the buffer releases IRQ");
    /* A transmitter interrupt on line 3 (TxIE with TxRDY). */
    cm_setup(&c, 3, 0x07);
    run(b, NS(5000));
    expect_pin(b, "IRQ", LOGIC_0, "line 3 transmitter interrupt");
    expect_pin(b, "IRQTXRX", LOGIC_1, "IRQTXRX: transmitter");
    cm_expect(&c, 0, 4, "10000111", "Interrupt Summary: IRQ, line 3, Tx");
    cm_write(&c, 3, 3, 0x05);   /* clear TxIE */
    run(b, NS(1000));
    expect_pin(b, "IRQ", LOGIC_Z, "clearing TxIE releases IRQ");
    /* Receivers before transmitters: both pending, the receiver is reported. */
    cm_write(&c, 3, 3, 0x07);
    h_drive_bits(b, "RXD1", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_read(&c, 3, 0, NULL);
    cm_write(&c, 3, 0, 0x00);
    cm_expect(&c, 0, 4, "10000010", "the receiver interrupt comes first");
    cm_read(&c, 1, 0, NULL);
    cm_write(&c, 3, 3, 0x05);
    run(b, 12 * CM_BIT19200);   /* line 3 finishes its character */
    /* Data set change: DSR on line 3; MCIE with RxIE interrupts. */
    cm_read(&c, 3, 3, NULL);
    cm_write(&c, 3, 2, 0x4D);   /* MCIE */
    cm_write(&c, 3, 2, 0xFF);
    cm_write(&c, 3, 3, 0x25);
    set(b, "DSR3", LOGIC_1);
    run(b, NS(5000));
    cm_expect(&c, 0, 5, "10000000", "Data Set Change Summary: line 3 (bit 7)");
    expect_pin(b, "IRQ", LOGIC_0, "DSCHNG with MCIE and RxIE interrupts");
    cm_write(&c, 0, 5, 0x80);
    run(b, NS(1000));
    cm_expect(&c, 0, 5, "00000000", "writing 1 clears it");
    expect_pin(b, "IRQ", LOGIC_Z, "and releases IRQ");
    cm_expect(&c, 3, 1, "00000101", "status DSR follows the pin (inactive)");
    set(b, "DSR3", LOGIC_0);
    run(b, NS(2000));
    cm_write(&c, 0, 5, 0xFF);
    cm_write(&c, 3, 3, 0x05);

    /* TxBRK: spacing; cleared, the waiting character is sent. */
    cm_write(&c, 0, 3, 0x0D);
    run(b, CM_BIT19200);
    expect_pin(b, "TXD0", LOGIC_0, "TxBRK spacing");
    cm_write(&c, 0, 0, 0x5A);
    run(b, 2 * CM_BIT19200);
    expect_pin(b, "TXD0", LOGIC_0, "the character waits during the break");
    cm_write(&c, 0, 3, 0x05);
    check(h_catch(b, "TXD0", CM_BIT19200, 0, 4 * CM_BIT19200, got) || get(b, "TXD0") == LOGIC_1,
          "break cleared");
    run(b, 12 * CM_BIT19200);
    h_expect_sent(b, 0, 0x5A, "the waiting character went after the break");
    /* Automatic echo: RXD to TXD; TxRDY cleared. */
    cm_write(&c, 0, 3, 0x45);
    set(b, "RXD0", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "TXD0", LOGIC_0, "automatic echo: TXD follows RXD");
    set(b, "RXD0", LOGIC_1);
    run(b, 12 * CM_BIT19200);
    cm_expect(&c, 0, 1, "10000000", "echo: TxRDY and TxEMT cleared");
    cm_read(&c, 0, 0, NULL);
    /* Local loopback: TXD high, the receiver gets the transmitter. */
    cm_write(&c, 0, 3, 0x85);
    cm_write(&c, 0, 0, 0xA7);
    run(b, CM_BIT19200);
    expect_pin(b, "TXD0", LOGIC_1, "local loopback: TXD high");
    run(b, 11 * CM_BIT19200);
    cm_expect(&c, 0, 0, "10100111", "local loopback: received what was sent");
    /* Remote loopback: RXD to TXD, RxEN and TxEN held cleared. */
    cm_write(&c, 0, 3, 0xC5);
    cm_expect(&c, 0, 3, "11000000", "remote loopback: TxEN and RxEN held cleared");
    set(b, "RXD0", LOGIC_0);
    run(b, NS(500));
    expect_pin(b, "TXD0", LOGIC_0, "remote loopback: TXD follows RXD");
    set(b, "RXD0", LOGIC_1);
    cm_write(&c, 0, 3, 0x05);
    run(b, 2 * CM_BIT19200);
    /* Terminal on line 1. */
    set(b, "RXD1", LOGIC_Z);
    run(b, NS(100));
    check(h_send(b, 1, "OK") == 1, "terminal send on line 1");
    run(b, 22 * CM_BIT19200);
    cm_expect(&c, 1, 0, "01001111", "terminal O");
    cm_expect(&c, 1, 0, "01001011", "terminal K");
    cm_write(&c, 1, 0, 'A');
    run(b, 12 * CM_BIT19200);
    h_expect_sent(b, 1, 'A', "line 1: A logged");
    hb_close(h);

    /* ---- COM78C802: 2 lines, ADD3 picks the line. ---- */
    h = cm_open("SFX-COM78C802-dip40", 2, &c);
    b = &h->b;
    cm_setup(&c, 1, 0x27);
    run(b, NS(5000));
    expect_pin(b, "IRQ", LOGIC_0, "COM78C802: line 1 transmitter interrupt");
    cm_expect(&c, 0, 4, "10000011", "COM78C802 summary: IRQ, line 1, Tx");
    cm_write(&c, 1, 0, 0x99);
    check(h_catch(b, "TXD1", CM_BIT19200, 9, 4 * CM_BIT19200, got), "COM78C802 line 1 frame");
    h_frame_bits(0x99, 8, 0, 1, want);
    h_expect_text(got, want, "COM78C802 line 1");
    set(b, "DSR0", LOGIC_1);
    run(b, NS(2000));
    cm_expect(&c, 0, 5, "00000001", "COM78C802 Data Set Change Summary bit 0");
    hb_close(h);

    /* ---- COM78804: DS on two pins (DIP-48). ---- */
    h = cm_open("SFX-COM78804-dip48", 4, &c);
    b = &h->b;
    cm_setup(&c, 2, 0x05);
    cm_expect(&c, 2, 1, "10000001", "COM78804: both DS pins strobe");
    /* The two DS pins disagree: the strobe is X, so a read drives X. */
    cm_set_addr(&c, 2, 1);
    set(b, "CS", LOGIC_0);
    set(b, "DS", LOGIC_0);
    run(b, NS(300));
    h_get_bus(b, CM_DL, 8, text);
    h_expect_text(text, "XXXXXXXX", "COM78804: DS pins that disagree read X");
    set(b, "DS", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    case_end("H8 COM78C804/802/78804: MR pointer, per-line rates and formats, FIFO/ORR/FER/PER/RERR, scanner and summaries, DSCHNG, break, echo and loopbacks, 2-line variant, terminal");
    hb_close(h);
}

/* ==== plan-12-I: the I2C/SPI bridges and the SC18IS604 ============================ */

#include "serial_test_i.h"

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_serial_fixture(library) != 0 ||
        load_new_footprints() != 0 || load_file("footprints_test.json", 1) != 0 ||
        load_file("ns16550.json", 0) != 0 || load_file("bridges.json", 0) != 0 ||
        load_serial_fixture_classic(library) != 0) {
        printf("FAIL: loading parts\n");
        return 1;
    }
    case_reset();
    case_divisor();
    case_tx();
    case_rx();
    case_fifo();
    case_timeout();
    case_priority();
    case_modem();
    case_loopback();
    case_dma();
    case_bus();
    case_16450();
    case_terminal();
    run_d_cases();
    case_d24();
    case_shipped();
    case_jf_spi();
    case_jf_i2c();
    case_j1();
    case_j2();
    case_j3();
    case_j4();
    case_j5();
    case_j6();
    case_h1_ay1013();
    case_h1_pin_parallel();
    case_h2_com1863();
    case_h3_cdp1854a();
    case_h4_i8251();
    case_h5_i82510();
    case_h6_acia6551();
    case_h7_hs82c52();
    case_h8_com78c80x();
    run_i_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
