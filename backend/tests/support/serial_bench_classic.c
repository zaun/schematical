/* serial_bench_classic (split from the serial tests, plan-68-G). */
#include "serial_bench.h"
#include "serial_bench_classic.h"


HBench *hb_open(const char *part_name, const HClockPin *clock_pins, unsigned n_pins,
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

void hb_close(HBench *h) {
    sim_destroy(h->b.sim);
    free(h);
}

/* Drives pins NAME<first>..NAME<first+n-1> (or the given names) from a value, LSB first. */
void h_set_bus(Bench *b, const char *const *names, unsigned n, int drive, unsigned v) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], !drive ? LOGIC_Z : (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

/* Reads pins as text ("01XZ", MSB first) and as a value (unknown bits 0). */
unsigned h_get_bus(Bench *b, const char *const *names, unsigned n, char *text) {
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

void h_expect_bus(Bench *b, const char *const *names, unsigned n, const char *want,
                         const char *what) {
    char text[17];
    char message[160];

    h_get_bus(b, names, n, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

/* Waits up to `limit` for `pin` to read `level`; 1 when it did. */
int h_wait(Bench *b, const char *pin, LogicValue level, SimTime limit) {
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
void h_frame(Bench *b, const char *pin, SimTime bt, unsigned n, char *out) {
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
int h_catch(Bench *b, const char *pin, SimTime bt, unsigned n, SimTime limit, char *out) {
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
void h_drive_bits(Bench *b, const char *pin, const char *bits, SimTime bt) {
    size_t i;

    for (i = 0; bits[i] != '\0'; i++) {
        set(b, pin, bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
    set(b, pin, LOGIC_1);
}

/* The bits of an async frame: start, n data bits LSB first, parity ('o' odd,
   'e' even, 'm' mark, 's' space, 0 none), stops (count of 1s). */
void h_frame_bits(unsigned v, unsigned n, char parity, unsigned stops, char *out) {
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

int h_send(Bench *b, unsigned channel, const char *text) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_send(b->sim, b->chip, sim_instance_state(b->chip), channel,
                             (const uint8_t *)text, strlen(text));
}

/* ==== end of plan-12-J ==================================================================== */

void h_expect_text(const char *got, const char *want, const char *what) {
    char message[200];

    snprintf(message, sizeof message, "%s: got %s, want %s", what, got, want);
    check(strcmp(got, want) == 0, message);
}

/* The last sent frame equals v. */
void h_expect_sent(Bench *b, unsigned channel, unsigned v, const char *what) {
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
