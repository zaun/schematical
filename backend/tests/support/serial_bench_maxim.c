/* serial_bench_maxim (split from the serial tests, plan-68-G). */
#include "serial_bench.h"
#include "serial_bench_maxim.h"


size_t pull_pins(const void *config) {
    return ((const PullConfig *)config)->count;
}

void pull_start(Sim *sim, SimInstance *instance, void *state) {
    const PullConfig *c = state;
    size_t i;

    for (i = 0; i < c->count; i++) {
        sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_WEAK, 0);
    }
}

void *pull_create(const void *config) {
    PullConfig *c = malloc(sizeof *c);

    *c = *(const PullConfig *)config;
    return c;
}

const SimComponentType PULLUP = {
    "pullup", pull_pins, pull_create, free, pull_start, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL
};

int maxim_loaded;

void maxim_need(void) {
    if (!maxim_loaded) {
        if (load_serial_fixture_maxim(library) != 0) {
            printf("FAIL: loading the Maxim fixtures\n");
            exit(1);
        }
        maxim_loaded = 1;
    }
}

size_t mb_pin(const MB *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

void mb_set(MB *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, mb_pin(b, name), v, DRIVE_STRONG, 0);
}

LogicValue mb_get(const MB *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, mb_pin(b, name));
}

void mb_run(MB *b, SimTime ps) {
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
MB *mb_open(const char *part_name, const char *clock, SimTime half,
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

void mb_close(MB *b) {
    sim_destroy(b->sim);
    free(b);
}

void mb_expect_pin(MB *b, const char *pin, LogicValue want, const char *what) {
    char message[160];
    LogicValue got = mb_get(b, pin);

    snprintf(message, sizeof message, "%s: %s is %c, want %c", what, pin, "01XZ"[got], "01XZ"[want]);
    check(got == want, message);
}

void mb_expect_u(unsigned got, unsigned want, const char *what) {
    char message[160];

    snprintf(message, sizeof message, "%s: got 0x%X, want 0x%X", what, got, want);
    check(got == want, message);
}

size_t mb_sent(MB *b, unsigned channel, uint16_t *frames, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_sent(sim_instance_state(b->chip), channel, 0, frames, cap);
}

int mb_send(MB *b, unsigned channel, const char *text) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_send(b->sim, b->chip, sim_instance_state(b->chip), channel,
                             (const uint8_t *)text, strlen(text));
}

/* CS low, n bits MSB first; MISO sampled before each rising edge into *in
   (unknown bits 0) and text ("01XZ"). CS stays low when keep is set. */
uint32_t mb_spi(MB *b, uint32_t out, unsigned n, char *text, int keep) {
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

void i2c_line(MB *b, const char *pin, int level) {
    mb_set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

void i2c_start(MB *b) {
    i2c_line(b, b->sda, 1);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->sda, 0);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 0);
    mb_run(b, I2C_Q);
}

void i2c_restart(MB *b) {
    i2c_line(b, b->sda, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->sda, 0);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 0);
    mb_run(b, I2C_Q);
}

void i2c_stop(MB *b) {
    i2c_line(b, b->sda, 0);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    i2c_line(b, b->sda, 1);
    mb_run(b, I2C_Q * 2);
}

LogicValue i2c_bit(MB *b, int out) {
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
LogicValue i2c_write(MB *b, unsigned byte) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (byte >> i) & 1u);
    }
    return i2c_bit(b, 1);
}

/* Reads a byte (text "01XZ" when given), then ACKs (ack = 1) or NACKs. */
unsigned i2c_read(MB *b, int ack, char *text) {
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
long long mb_wait(MB *b, const char *pin, LogicValue level, SimTime limit, SimTime step) {
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
void mb_frame(MB *b, const char *pin, SimTime bt, unsigned n, char *out) {
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
void mb_drive_bits(MB *b, const char *pin, const char *bits, SimTime bt, LogicValue idle) {
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
void mb_drive_byte(MB *b, const char *pin, unsigned v, SimTime bt, int stop) {
    char bits[12];
    unsigned i;

    for (i = 0; i < 8; i++) {
        bits[i] = (v >> i) & 1u ? '1' : '0';
    }
    bits[8] = stop ? '1' : '0';
    bits[9] = '\0';
    mb_drive_bits(b, pin, bits, bt, LOGIC_1);
}

void tg_log(Tgt *t, const char *format, unsigned a, unsigned b) {
    size_t n = strlen(t->log);

    snprintf(t->log + n, sizeof t->log - n, format, a, b);
}

void tg_spi_begin(void *c) { tg_log(c, "B ", 0, 0); }
SerialSpiByte tg_spi_next(void *c, unsigned index) {
    SerialSpiByte b;

    (void)c;
    b.data = (uint8_t)(0xA5 ^ index);
    b.known = 0xFF;
    b.drive = 0xFF;
    return b;
}
void tg_spi_received(void *c, unsigned index, uint8_t data, uint8_t known) {
    (void)index;
    tg_log(c, known == 0xFF ? "R%02X " : "R%02X? ", data, 0);
}
void tg_spi_end(void *c, unsigned bits) { tg_log(c, "E%u ", bits, 0); }

void tg_i2c_begin(void *c, int repeated) { tg_log(c, repeated ? "Sr " : "S ", 0, 0); }
LogicValue tg_i2c_address(void *c, uint8_t byte, uint8_t known) {
    Tgt *t = c;

    t->ties[0] = serial_i2c_strap(&t->i2c, 0);
    t->ties[1] = serial_i2c_strap(&t->i2c, 1);
    tg_log(c, "A%02X ", byte, 0);
    return known == 0xFF && (byte >> 1) == 0x2A ? LOGIC_1 : LOGIC_0;
}
LogicValue tg_i2c_received(void *c, unsigned index, uint8_t data, uint8_t known) {
    Tgt *t = c;

    (void)index;
    (void)known;
    t->mem = data;
    tg_log(c, "W%02X ", data, 0);
    return LOGIC_1;
}
void tg_i2c_next(void *c, unsigned index, uint8_t *data, uint8_t *known) {
    Tgt *t = c;

    tg_log(c, "N%u ", index, 0);
    *data = (uint8_t)(t->mem + index);
    *known = 0xFF;
}
void tg_i2c_acked(void *c, unsigned index, LogicValue ack) {
    (void)index;
    tg_log(c, "K%c ", "NAXZ"[ack], 0);
}
void tg_i2c_end(void *c, int stop) { tg_log(c, stop ? "P " : "Px ", 0, 0); }

size_t tg_pin_count(const void *config) {
    (void)config;
    return TG_PINS;
}

void *tg_create(const void *config) {
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

void tg_start(Sim *sim, SimInstance *instance, void *state) {
    Tgt *t = state;

    serial_spi_start(&t->spi, sim, instance);
    serial_i2c_start(&t->i2c, sim, instance);
}

void tg_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    Tgt *t = state;

    (void)old_value;
    (void)new_value;
    serial_spi_pin(&t->spi, sim, instance, pin, &t->se);
    serial_i2c_pin(&t->i2c, sim, instance, pin, &t->ie);
}

const SimComponentType TARGET = {
    "target", tg_pin_count, tg_create, free, tg_start, tg_pin_changed, NULL,
    NULL, NULL, NULL, NULL, NULL
};

TgBench *tg_open(void) {
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

Tgt *tg_state(TgBench *t) {
    return sim_instance_state(t->b.chip);
}

long long m3_start_bit(MB *b, const char *pin, SimTime limit) {
    SimTime step = NS(10);

    if (mb_wait(b, pin, LOGIC_0, limit, step) < 0 || mb_wait(b, pin, LOGIC_1, limit, step) < 0) {
        return -1;
    }
    return mb_wait(b, pin, LOGIC_0, limit, step);
}
int within(long long got, long long want, long long tolerance) {
    return got >= want - tolerance && got <= want + tolerance;
}
