/* The serial programs' shared bench (split from serial_test.c, plan-68-G). */
#include "serial_bench.h"


int failures;
int case_failures;
/* The PC16550D-shaped part C's cases run on (the C fixture, then the shipped device: D24). */
const char *c_part = "SFX-16550";
PartLibrary *library;

void check(int ok, const char *what) {
    if (!ok) {
        printf("  FAIL: %s\n", what);
        failures++;
        case_failures++;
    }
}

void case_begin(void) {
    case_failures = 0;
}

void case_end(const char *name) {
    printf("%s %s\n", case_failures == 0 ? "ok  " : "FAIL", name);
}

/* ---- Bench components ---------------------------------------------------------- */

size_t driver_pins(const void *config) {
    return *(const size_t *)config;
}

const SimComponentType DRIVER = {
    "driver", driver_pins, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

size_t clock_pins(const void *config) {
    (void)config;
    return 1;
}

void *clock_create(const void *config) {
    ClockState *s = calloc(1, sizeof *s);

    s->half = *(const SimTime *)config;
    return s;
}

void clock_destroy(void *state) {
    free(state);
}

void clock_start(Sim *sim, SimInstance *instance, void *raw) {
    ClockState *s = raw;

    s->level = LOGIC_0;
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_timer(sim, instance, 0, s->half);
}

void sb_clock_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    ClockState *s = raw;

    (void)id;
    s->level = s->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    sim_schedule_drive(sim, instance, 0, s->level, DRIVE_STRONG, 0);
    sim_schedule_timer(sim, instance, 0, s->half);
}

const SimComponentType CLOCK = {
    "clock", clock_pins, clock_create, clock_destroy, clock_start, NULL, sb_clock_timer,
    NULL, NULL, NULL, NULL, NULL
};

const BenchConfig PC16550D_BENCH = {
    "RDBAR", "WRBAR",
    {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"},
    {"A0", "A1", "A2"}, 3,
    {"CS0=1", "CS1=1", "CS2=0", "ADS=0", "RD=0", "WR=0", "SIN=1", "CTS=1", "DSR=1", "DCD=1",
     "RI=1", NULL},
    "MR", 1, "XIN", "BAUDOUT", "RCLK",
    {{NULL}}, 0
};

size_t pin_of(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s on %s\n", name, b->part->name);
    exit(2);
}

int has_pin(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; name != NULL && i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, pin_of(b, name), v, DRIVE_STRONG, 0);
}

/* "NAME=1" style: sets the pin; `invert` drives the opposite level. */
void set_tie(Bench *b, const char *tie, int invert) {
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

void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        const SimNet *net = sim_shorted_net(b->sim);

        fprintf(stderr, "sim stopped: %s (%s, net %s)\n", sim_error(b->sim), b->part->name,
                net != NULL ? sim_net_name(net) : "-");
        exit(2);
    }
}

LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, pin_of(b, name));
}

/* Selects unit u (a channel, or the printer port) for the next accesses. */
void select_unit(Bench *b, unsigned u) {
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

void pulse_reset(Bench *b) {
    if (b->cfg->reset == NULL) {
        return;
    }
    set(b, b->cfg->reset, b->cfg->reset_high ? LOGIC_1 : LOGIC_0);
    run(b, NS(6000));
    set(b, b->cfg->reset, b->cfg->reset_high ? LOGIC_0 : LOGIC_1);
    run(b, NS(100));
}

/* A device on a bench with a clock of `half` ps per half period. */
Bench *bench_open_cfg(const char *part_name, SimTime half, const BenchConfig *cfg) {
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

Bench *bench_open(const char *part_name, SimTime half) {
    return bench_open_cfg(part_name, half, &PC16550D_BENCH);
}

void bench_close(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

void set_address(Bench *b, unsigned a) {
    unsigned i;

    for (i = 0; i < b->cfg->address_count; i++) {
        set(b, b->cfg->address[i], (a >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

void set_data(Bench *b, int drive, unsigned v) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        set(b, b->cfg->data[i], !drive ? LOGIC_Z : (v >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

void bus_write(Bench *b, unsigned a, unsigned v) {
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
unsigned bus_read_text(Bench *b, unsigned a, char text[9]) {
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

unsigned bus_read(Bench *b, unsigned a) {
    char text[9];

    return bus_read_text(b, a, text);
}

/* Reads and checks a register as text ("01XZ" per bit, high bit first). */
void expect_reg(Bench *b, unsigned a, const char *want, const char *what) {
    char text[9];
    char message[128];

    bus_read_text(b, a, text);
    snprintf(message, sizeof message, "%s: read %s, want %s", what, text, want);
    check(strcmp(text, want) == 0, message);
}

void expect_pin(Bench *b, const char *pin, LogicValue want, const char *what) {
    char message[128];
    LogicValue got = get(b, pin);

    snprintf(message, sizeof message, "%s: %s is %c, want %c", what, pin, "01XZ"[got], "01XZ"[want]);
    check(got == want, message);
}

/* LCR, divisor and FIFO setup. */
void program(Bench *b, unsigned divisor, unsigned lcr) {
    bus_write(b, 3, 0x80);
    bus_write(b, 0, divisor & 0xFF);
    bus_write(b, 1, divisor >> 8);
    bus_write(b, 3, lcr);
}

SimTime bit_time(const Bench *b, unsigned divisor) {
    return 16u * divisor * b->period;
}

/* Waits up to `limit` for SOUT to fall; 1 when it did (the time is now). */
int wait_start(Bench *b, SimTime limit) {
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
void sample_frame(Bench *b, unsigned divisor, unsigned n, char *out) {
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
void drive_bits(Bench *b, const char *bits, SimTime bt) {
    size_t i;

    for (i = 0; bits[i] != '\0'; i++) {
        set(b, "SIN", bits[i] == '1' ? LOGIC_1 : LOGIC_0);
        run(b, bt);
    }
}

/* An 8N1 frame for byte v, with a stop bit of `stop`. */
void drive_byte(Bench *b, unsigned v, SimTime bt, int stop) {
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

size_t sent(Bench *b, uint16_t *frames, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_sent(sim_instance_state(b->chip), 0, 0, frames, cap);
}

int sb_send(Bench *b, const char *text) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_send(b->sim, b->chip, sim_instance_state(b->chip), 0,
                             (const uint8_t *)text, strlen(text));
}

void strip_active_low(cJSON *device) {
    const cJSON *pinout;

    cJSON_ArrayForEach(pinout, cJSON_GetObjectItemCaseSensitive(device, "pinouts")) {
        cJSON *pin;

        cJSON_ArrayForEach(pin, pinout) {
            cJSON_DeleteItemFromObjectCaseSensitive(pin, "activeLow");
        }
    }
}

/* Compares every device of one fixture document with the device of the same
   id in serial.json (fixture ids are "SFX-<id>"; parts and pins' "activeLow"
   aside). Returns how many were found there. */
int compare_shipped(const char *fixture_text, const cJSON *shipped, const char *what) {
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
                strip_active_low(a);
                strip_active_low(b);
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
char *read_fixture(const char *file) {
    char path[512];

    snprintf(path, sizeof path, "%s/../fixtures/serial/%s", GOLDEN_DIR, file);
    return read_file(path);
}
int load_new_footprints(void) {
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
int load_file(const char *name, int footprints) {
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

/* Loads every part file and fixture the serial programs use (the old
   serial_test main's first step). */
int serial_bench_load(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_serial_fixture(library) != 0 ||
        load_new_footprints() != 0 || load_file("footprints_test.json", 1) != 0 ||
        load_file("ns16550.json", 0) != 0 || load_file("bridges.json", 0) != 0 ||
        load_serial_fixture_classic(library) != 0) {
        printf("FAIL: loading parts\n");
        return 1;
    }
    return 0;
}

/* The ending of every serial program: frees the library and reports. */
int serial_bench_finish(void) {
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
