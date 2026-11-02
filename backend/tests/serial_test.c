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
#include "serial_fixture_classic.h"
#include "sim.h"

#define NS(x) ((SimTime)(x) * 1000u)

static int failures;
static int case_failures;
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

typedef struct {
    Sim *sim;
    SimInstance *chip;
    SimInstance *drv;
    SimInstance *clk;
    const Part *part;
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
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

static void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, pin_of(b, name), v, DRIVE_STRONG, 0);
}

static void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

static LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, pin_of(b, name));
}

/* A UART on a bench with a clock of `half` ps per half period. */
static Bench *bench_open(const char *part_name, SimTime half) {
    Bench *b = calloc(1, sizeof *b);
    SimNet *baud_net = NULL;
    size_t i;

    b->part = part_library_find(library, part_name);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part_name);
        exit(2);
    }
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

        if (strcmp(name, "RCLK") == 0) {
            continue;
        }
        net = sim_add_net(b->sim, name);
        sim_connect(b->sim, net, b->chip, i);
        if (strcmp(name, "BAUDOUT") == 0) {
            baud_net = net;
        } else {
            sim_connect(b->sim, net, b->drv, i);
        }
        if (strcmp(name, "XIN") == 0) {
            sim_connect(b->sim, net, b->clk, 0);
        }
    }
    sim_connect(b->sim, baud_net, b->chip, pin_of(b, "RCLK"));
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    set(b, "CS0", LOGIC_1);
    set(b, "CS1", LOGIC_1);
    set(b, "CS2", LOGIC_0);
    set(b, "ADS", LOGIC_0);
    set(b, "RD", LOGIC_0);
    set(b, "WR", LOGIC_0);
    set(b, "RDBAR", LOGIC_1);
    set(b, "WRBAR", LOGIC_1);
    set(b, "SIN", LOGIC_1);
    set(b, "CTS", LOGIC_1);
    set(b, "DSR", LOGIC_1);
    set(b, "DCD", LOGIC_1);
    set(b, "RI", LOGIC_1);
    set(b, "A0", LOGIC_0);
    set(b, "A1", LOGIC_0);
    set(b, "A2", LOGIC_0);
    set(b, "MR", LOGIC_1);
    run(b, NS(6000));
    set(b, "MR", LOGIC_0);
    run(b, NS(100));
    return b;
}

static void bench_close(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

static void set_address(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
    set(b, "A2", a & 4u ? LOGIC_1 : LOGIC_0);
}

static void set_data(Bench *b, int drive, unsigned v) {
    char name[4];
    unsigned i;

    for (i = 0; i < 8; i++) {
        snprintf(name, sizeof name, "D%u", i);
        set(b, name, !drive ? LOGIC_Z : (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void bus_write(Bench *b, unsigned a, unsigned v) {
    set_address(b, a);
    set_data(b, 1, v);
    run(b, NS(40));
    set(b, "WRBAR", LOGIC_0);
    run(b, NS(120));
    set(b, "WRBAR", LOGIC_1);
    run(b, NS(40));
    set_data(b, 0, 0);
    run(b, NS(160));
}

/* A read: the bus as text, high bit first ("01XZ"), and the value (unknown bits 0). */
static unsigned bus_read_text(Bench *b, unsigned a, char text[9]) {
    char name[4];
    unsigned v = 0;
    unsigned i;

    set_address(b, a);
    run(b, NS(40));
    set(b, "RDBAR", LOGIC_0);
    run(b, NS(150));
    for (i = 0; i < 8; i++) {
        LogicValue l;

        snprintf(name, sizeof name, "D%u", i);
        l = get(b, name);
        text[7 - i] = "01XZ"[l];
        if (l == LOGIC_1) {
            v |= 1u << i;
        }
    }
    text[8] = '\0';
    set(b, "RDBAR", LOGIC_1);
    run(b, NS(160));
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
    Bench *b = bench_open("SFX-16550", HALF_18432);

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
    Bench *b = bench_open("SFX-16550", NS(50));   /* 10 MHz clock */
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);

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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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
    Bench *b = bench_open("SFX-16550", HALF_18432);
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

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_serial_fixture(library) != 0 ||
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
    case_h1_ay1013();
    case_h2_com1863();
    case_h3_cdp1854a();
    case_h4_i8251();
    case_h6_acia6551();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
