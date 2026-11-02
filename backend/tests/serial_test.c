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

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_serial_fixture(library) != 0 ||
        load_file("footprints_test.json", 1) != 0 || load_file("ns16550.json", 0) != 0) {
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
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
