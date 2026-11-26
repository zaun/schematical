/* converter_bench (plan-68-M): the pin-by-pin bench of converter_more_test and
   converter_more2_test (the same helpers as converter_test's, which keeps its own). */
#include "converter_bench.h"

int failures;
PartLibrary *library;

void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

char *json(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    size_t i;

    for (i = 0; text[i] != '\0'; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    copy[i] = '\0';
    return copy;
}

/* Loads the footprints T6, T8 and T16 and one converter part file of `devices`. */
int load_converters(const char *devices) {
    char *fp = json("{'format':'schematical-footprints','version':1,'footprints':[" CB_DUAL("T6", "6") ","
                    CB_DUAL("T8", "8") "," CB_DUAL("T16", "16") "]}");
    char *parts = json(devices);
    char *error = NULL;
    int ok;

    library = part_library_create();
    ok = load_footprints(library, fp, "fixtures.json", &error) == 0 &&
         load_parts(library, parts, "converter-more.json", &error) == 0;
    if (!ok) {
        printf("FAIL loading the fixtures: %s\n", error ? error : "?");
        failures++;
    }
    free(error);
    free(fp);
    free(parts);
    return ok;
}

static size_t drv_pins(const void *config) {
    return ((const PinCount *)config)->count;
}

static void *drv_create(const void *config) {
    PinCount *c = malloc(sizeof *c);

    *c = *(const PinCount *)config;
    return c;
}

static void pull_start(Sim *sim, SimInstance *instance, void *state) {
    size_t i;

    for (i = 0; i < ((PinCount *)state)->count; i++) {
        sim_schedule_drive(sim, instance, i, ((PinCount *)state)->level, DRIVE_WEAK, 0);
    }
}

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};
static const SimComponentType PULL = {"pu", drv_pins, drv_create, free, pull_start, NULL, NULL,
                                      NULL, NULL, NULL, NULL, NULL};

size_t bpin(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, bpin(b, name), v, DRIVE_STRONG, 0);
}

LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, bpin(b, name));
}

void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

static int listed(const char *const *names, const char *name, size_t *k) {
    for (*k = 0; names != NULL && names[*k] != NULL; (*k)++) {
        if (strcmp(names[*k], name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* ups / downs: pins given a weak pull-up / pull-down (from time 0). */
Bench *bench(const char *part, const char *const *ups, const char *const *downs) {
    Bench *b = calloc(1, sizeof *b);
    size_t i, k;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    for (k = 0; ups != NULL && ups[k] != NULL; k++) {
    }
    b->ups.count = k;
    b->ups.level = LOGIC_1;
    for (k = 0; downs != NULL && downs[k] != NULL; k++) {
    }
    b->downs.count = k;
    b->downs.level = LOGIC_0;
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULL, "PU", &b->ups);
    b->pd = sim_add_instance(b->sim, &PULL, "PD", &b->downs);
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        if (listed(ups, b->part->pins[i].name, &k)) {
            sim_connect(b->sim, net, b->pu, k);
        }
        if (listed(downs, b->part->pins[i].name, &k)) {
            sim_connect(b->sim, net, b->pd, k);
        }
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    return b;
}

void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* One SPI frame of n bits in mode 0 or 1 (MSB first); the MISO bits come back. */
uint32_t spi(Bench *b, unsigned mode, uint32_t out, unsigned n, int deselect) {
    uint32_t in = 0;
    unsigned i;

    set(b, b->sclk, LOGIC_0);
    run(b, HALF);
    set(b, b->cs, LOGIC_0);
    run(b, HALF);
    for (i = 0; i < n; i++) {
        if (mode & 1u) {
            set(b, b->sclk, LOGIC_1);
        }
        if (b->mosi != NULL) {
            set(b, b->mosi, (out >> (n - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        }
        run(b, HALF);
        in = (in << 1) | (b->miso != NULL && get(b, b->miso) == LOGIC_1 ? 1u : 0u);
        set(b, b->sclk, (mode & 1u) ? LOGIC_0 : LOGIC_1);
        run(b, HALF);
        if (!(mode & 1u)) {
            set(b, b->sclk, LOGIC_0);
        }
    }
    run(b, HALF);
    if (deselect) {
        set(b, b->cs, LOGIC_1);
        run(b, HALF);
    }
    return in;
}

void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

void i2c_start(Bench *b) {
    line(b, b->sda, 1);
    line(b, b->scl, 1);
    run(b, HALF);
    line(b, b->sda, 0);
    run(b, HALF);
    line(b, b->scl, 0);
    run(b, HALF);
}

void i2c_stop(Bench *b) {
    line(b, b->sda, 0);
    run(b, HALF);
    line(b, b->scl, 1);
    run(b, HALF);
    line(b, b->sda, 1);
    run(b, HALF);
}

LogicValue i2c_bit(Bench *b, int out) {
    LogicValue v;

    line(b, b->sda, out);
    run(b, HALF);
    line(b, b->scl, 1);
    run(b, HALF);
    v = get(b, b->sda);
    line(b, b->scl, 0);
    run(b, HALF);
    return v;
}

int i2c_write(Bench *b, unsigned byte) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (byte >> i) & 1u);
    }
    return i2c_bit(b, 1) == LOGIC_0;
}

unsigned i2c_read(Bench *b, int ack) {
    unsigned v = 0;
    int i;

    for (i = 0; i < 8; i++) {
        v = (v << 1) | (i2c_bit(b, 1) == LOGIC_1 ? 1u : 0u);
    }
    i2c_bit(b, !ack);
    return v;
}

int i2c_tx(Bench *b, unsigned address, const unsigned *bytes, size_t count) {
    int ok;
    size_t i;

    i2c_start(b);
    ok = i2c_write(b, address << 1);
    for (i = 0; ok && i < count; i++) {
        ok = i2c_write(b, bytes[i]);
    }
    i2c_stop(b);
    return ok;
}

/* A read of `count` bytes; 0 when the address is NACKed. */
int i2c_rx(Bench *b, unsigned address, unsigned *bytes, size_t count) {
    int ok;
    size_t i;

    i2c_start(b);
    ok = i2c_write(b, (address << 1) | 1u);
    for (i = 0; ok && i < count; i++) {
        bytes[i] = i2c_read(b, i + 1 < count);
    }
    i2c_stop(b);
    return ok;
}

uint32_t bus4(Bench *b, const char *const *pins, int *z) {
    uint32_t v = 0;
    size_t i;

    *z = 1;
    for (i = 0; i < 4; i++) {
        LogicValue l = get(b, pins[i]);

        *z &= l == LOGIC_Z;
        v = (v << 1) | (l == LOGIC_1 ? 1u : 0u);
    }
    return v;
}

