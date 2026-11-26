/* cpu_test_bench: shared by the programs split from cpu_test.c (plan-68-H). */
#include "cpu_test_bench.h"


int failures = 0;
PartLibrary *library;

void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

char *unquote(const char *text, size_t size) {
    char *copy = malloc(size);
    size_t i;

    for (i = 0; i < size; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

int load_fixture(void) {
    char *footprint = unquote(FIXTURE_FOOTPRINT, sizeof FIXTURE_FOOTPRINT);
    char *parts = unquote(FIXTURE, sizeof FIXTURE);
    char *error = NULL;
    int result = load_footprints(library, footprint, "cpu_fixture_footprints.json", &error);

    if (result == 0) {
        result = load_parts(library, parts, "cpu_fixture.json", &error);
    }
    if (result != 0) {
        fprintf(stderr, "fixture: %s\n", error != NULL ? error : "(no message)");
    }
    free(error);
    free(footprint);
    free(parts);
    return result;
}

size_t bench_pins(const void *config) {
    return ((const Bench *)config)->part->pin_count;
}

void *bench_create(const void *config) {
    return (void *)config;
}

void bench_destroy(void *state) {
    (void)state;
}

LogicValue pin_value(const Bench *b, size_t pin) {
    return sim_read_pin(b->sim, b->tb, pin);
}

char pin_char(const Bench *b, size_t pin) {
    return "01XZ"[pin_value(b, pin)];
}

/* The address bus: its value, or -1 when a bit is not 0/1. */
int bus_address(const Bench *b) {
    int value = 0;
    int i;

    for (i = 0; i < 16; i++) {
        LogicValue v = pin_value(b, b->a[i]);

        if (v != LOGIC_0 && v != LOGIC_1) {
            return -1;
        }
        value |= (v == LOGIC_1) << i;
    }
    return value;
}

/* The data bus: its value, -1 when a bit is X or mixed, -2 when all Z. */
int bus_data(const Bench *b) {
    int value = 0;
    int z = 0;
    int i;

    for (i = 0; i < 8; i++) {
        LogicValue v = pin_value(b, b->d[i]);

        z += v == LOGIC_Z;
        if (v == LOGIC_1) {
            value |= 1 << i;
        } else if (v != LOGIC_0) {
            value = -1000;
        }
    }
    return z == 8 ? -2 : value < 0 ? -1 : value;
}

void drive(Bench *b, size_t pin, LogicValue value, SimTime delay) {
    sim_schedule_drive(b->sim, b->tb, pin, value, DRIVE_STRONG, delay);
}

void drive_data(Bench *b, int value) {
    int i;

    for (i = 0; i < 8; i++) {
        drive(b, b->d[i], value < 0 ? LOGIC_X : (value >> i) & 1 ? LOGIC_1 : LOGIC_0, 0);
    }
}

SimTime min_time(SimTime x, SimTime y) {
    return x < y ? x : y;
}

void bench_start(Sim *sim, SimInstance *instance, void *state) {
    Bench *b = state;

    (void)instance;
    drive(b, b->clk, LOGIC_0, 0);
    sim_schedule_timer(sim, b->tb, T_RISE, b->low);
    b->next_fall = b->low + b->high;
}

void bench_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    Bench *b = state;
    int addr;
    int rw;

    (void)instance;
    switch (id) {
        case T_RISE:
            drive(b, b->clk, LOGIC_1, 0);
            sim_schedule_timer(sim, b->tb, T_SERVE, min_time(NS(100), b->high / 2));
            sim_schedule_timer(sim, b->tb, T_SAMPLE, b->high - 1);
            sim_schedule_timer(sim, b->tb, T_FALL, b->high);
            b->next_fall = sim_now(sim) + b->high;
            break;
        case T_FALL:
            drive(b, b->clk, LOGIC_0, 0);
            b->falls++;
            sim_schedule_timer(sim, b->tb, T_RELEASE, min_time(NS(20), b->low / 2));
            sim_schedule_timer(sim, b->tb, T_RISE, b->low);
            b->next_fall = sim_now(sim) + b->low + b->high;
            break;
        case T_SERVE:
            rw = pin_value(b, b->rw);
            if (rw == LOGIC_1 || rw == LOGIC_X) {
                addr = bus_address(b);
                drive_data(b, rw == LOGIC_1 && addr >= 0 && b->known[addr] ? b->mem[addr] : -1);
                b->serving = 1;
            }
            break;
        case T_SAMPLE:
            if (b->log_count < LOG_MAX) {
                Cycle *c = &b->log[b->log_count++];

                c->addr = bus_address(b);
                c->rw = pin_char(b, b->rw);
                c->sync = pin_char(b, b->sync);
                c->vpb = b->wdc ? pin_char(b, b->vpb) : '1';
                c->mlb = b->wdc ? pin_char(b, b->mlb) : '1';
                c->data = bus_data(b);
                c->at = sim_now(sim);
                if (c->rw == '0' && c->addr >= 0) {
                    b->mem[c->addr] = (uint8_t)(c->data >= 0 ? c->data : 0);
                    b->known[c->addr] = c->data >= 0;
                    b->stores++;
                }
            }
            break;
        case T_RELEASE:
            if (b->serving) {
                int i;

                for (i = 0; i < 8; i++) {
                    drive(b, b->d[i], LOGIC_Z, 0);
                }
                b->serving = 0;
            }
            break;
        case T_POKE:
            if (b->serving) {
                drive_data(b, b->poke_value);
            }
            break;
    }
}

const SimComponentType BENCH_TYPE = {
    "bench", bench_pins, bench_create, bench_destroy, bench_start, NULL, bench_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

size_t pin_named(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s on %s\n", name, part->name);
    exit(2);
}

void run_to(Bench *b, SimTime time) {
    if (!sim_run_until(b->sim, time)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* Runs n more PHI2 falls, stopping 200 ns after the last. */
void run_cycles(Bench *b, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        run_to(b, b->next_fall + NS(200));
    }
}

void set_pin(Bench *b, size_t pin, LogicValue value) {
    drive(b, pin, value, 0);
}

/* A bench at power-up: 1 MHz clock (500 ns high), every input inactive
   (RDY a weak pull-up, so WAI can pull it low), memory all X. */
Bench *bench_open(const char *name) {
    Bench *b = calloc(1, sizeof(*b));
    const Part *part = part_library_find(library, name);
    size_t i;
    char pin[8];

    if (b == NULL || part == NULL || part->kind != PART_CPU) {
        fprintf(stderr, "no cpu part %s\n", name);
        exit(2);
    }
    b->part = part;
    b->wdc = strcmp(part->cpu->core, "65c02") == 0;
    for (i = 0; i < 16; i++) {
        snprintf(pin, sizeof pin, "A%zu", i);
        b->a[i] = pin_named(part, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "D%zu", i);
        b->d[i] = pin_named(part, pin);
    }
    b->sync = pin_named(part, "SYNC");
    b->rdy = pin_named(part, "RDY");
    if (b->wdc) {
        b->clk = pin_named(part, "PHI2");
        b->rw = pin_named(part, "RWB");
        b->irq = pin_named(part, "IRQB");
        b->nmi = pin_named(part, "NMIB");
        b->res = pin_named(part, "RESB");
        b->so = pin_named(part, "SOB");
        b->phi1 = pin_named(part, "PHI1O");
        b->phi2 = pin_named(part, "PHI2O");
        b->vpb = pin_named(part, "VPB");
        b->mlb = pin_named(part, "MLB");
        b->be = pin_named(part, "BE");
    } else {
        b->clk = pin_named(part, "PHI0IN");
        b->rw = pin_named(part, "R/W");
        b->irq = pin_named(part, "IRQ#");
        b->nmi = pin_named(part, "NMI#");
        b->res = pin_named(part, "RES#");
        b->so = pin_named(part, "SO#");
        b->phi1 = pin_named(part, "PHI1OUT");
        b->phi2 = pin_named(part, "PHI2OUT");
    }
    b->high = NS(500);
    b->low = NS(500);
    b->sim = sim_create();
    b->cpu = sim_add_instance(b->sim, kind_model(part), "U1", part);
    b->tb = sim_add_instance(b->sim, &BENCH_TYPE, "TB", b);
    for (i = 0; i < part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->cpu, i);
        sim_connect(b->sim, net, b->tb, i);
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "sim_start: %s\n", sim_error(b->sim));
        exit(2);
    }
    set_pin(b, b->irq, LOGIC_1);
    set_pin(b, b->nmi, LOGIC_1);
    set_pin(b, b->res, LOGIC_1);
    set_pin(b, b->so, LOGIC_1);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    if (b->wdc) {
        set_pin(b, b->be, LOGIC_1);
    }
    return b;
}

void bench_close(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

int cpu_state_of(const Bench *b, CpuState *out) {
    const SimComponentType *type = sim_instance_type(b->cpu);

    return type->cpu_state != NULL && type->cpu_state(sim_instance_state(b->cpu), out);
}

/* Bytes the bench serves from address. */
void load(Bench *b, uint16_t address, const uint8_t *bytes, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        b->mem[(uint16_t)(address + i)] = bytes[i];
        b->known[(uint16_t)(address + i)] = 1;
    }
}

void load_byte(Bench *b, uint16_t address, uint8_t value) {
    load(b, address, &value, 1);
}

/* RES low for 3 cycles, then high; returns 200 ns after the PHI2 fall that
   releases it. b->release is that fall; b->reset_log the log index of the
   first cycle after it. */
void do_reset(Bench *b) {
    set_pin(b, b->res, LOGIC_0);
    run_cycles(b, 3);
    set_pin(b, b->res, LOGIC_1);
    b->release = b->next_fall;
    run_cycles(b, 1);
    b->reset_log = b->log_count;
}

/* A bench with the vectors (reset C000, IRQ D000, NMI E000), RTI at D000
   and E000, C000-C0FF NOPs, the program at C000, and a reset done. */
Bench *boot(const char *name, const uint8_t *program, size_t count) {
    static const uint8_t VECTORS[] = {0x00, 0xE0, 0x00, 0xC0, 0x00, 0xD0};
    Bench *b = bench_open(name);
    size_t i;

    load(b, 0xFFFA, VECTORS, sizeof VECTORS);
    load_byte(b, 0xD000, 0x40);
    load_byte(b, 0xE000, 0x40);
    for (i = 0; i < 0x100; i++) {
        load_byte(b, (uint16_t)(0xC000 + i), 0xEA);
    }
    load(b, 0xC000, program, count);
    do_reset(b);
    return b;
}

/* The k-th PHI2 fall after the release (standard 1 MHz clock). */
SimTime fall_at(const Bench *b, unsigned k) {
    return b->release + (SimTime)k * NS(1000);
}

/* Cycle i after reset (logged at fall i + 1). */
const Cycle *cycle(const Bench *b, size_t i) {
    static const Cycle none = {-1, '?', '?', '?', '?', -1, 0};

    return b->reset_log + i < b->log_count ? &b->log[b->reset_log + i] : &none;
}

/* Runs until cycle i after reset has been logged. */
void run_to_cycle(Bench *b, size_t i) {
    while (b->log_count <= b->reset_log + i) {
        run_cycles(b, 1);
    }
}

int status_is(const Bench *b, CpuStatus status) {
    CpuState state;

    return cpu_state_of(b, &state) && state.status == status;
}

const char *label(const char *name, const char *text) {
    static char buffer[160];

    snprintf(buffer, sizeof buffer, "%s %s", name, text);
    return buffer;
}

uint8_t reg(const CpuState *state, const char *name) {
    size_t i;

    for (i = 0; i < state->register_count; i++) {
        if (strcmp(state->registers[i].name, name) == 0) {
            return (uint8_t)state->registers[i].value;
        }
    }
    return 0xEE;
}

int address_is(const Bench *b, int value) {
    return bus_address(b) == value;
}

/* ---- Whole-system boards (plan-09-D section 5) ------------------------------- */

int sram_is(const CpuBoard *board, uint32_t address, uint8_t expected) {
    uint8_t value;

    return cpu_board_sram(board, address, &value) && value == expected;
}

uint32_t reg_value(const CpuState *state, const char *name) {
    size_t i;

    for (i = 0; i < state->register_count; i++) {
        if (strcmp(state->registers[i].name, name) == 0) {
            return state->registers[i].value;
        }
    }
    return 0xEEEE;
}
