/*
 * cpu_test: the cpu kind's bus adapters against the datasheet rules
 * (plan-09-B section 5).
 *
 * The cases run on inline fixture devices that carry exactly the data
 * plan-09-D ships (W65C02S grade 14, MOS 6502 grades 1/A/B/C, PDIP-40
 * pinouts). A bench component clocks the CPU at 1 MHz and serves a 64 KB
 * memory: on a read it drives the byte from PHI2 rise + 100 ns until PHI2
 * fall + 20 ns; it samples (and logs) every cycle 1 ps before PHI2 falls,
 * and stores a write's data then. Every byte the bench was not given reads
 * X, like a powered-up SRAM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "cpu.h"
#include "cpu_state.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

#define NS(value) ((SimTime)((value) * 1000ull))

static int failures = 0;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- Fixtures (plan-09-D section 4.2) ------------------------------------ */

static const char FIXTURE_FOOTPRINT[] =
    "{'format':'schematical-footprints','version':1,'footprints':["
    "{'id':'FIXTURE-PDIP-40','description':'40-pin 600 mil PDIP','style':'dual','pins':40,"
    "'body':{'width':13.91,'height':52.33},'pitch':2.54,'span':15.24,"
    "'pad':{'width':1.5,'height':1.5,'shape':'round'},'drill':0.8,'pin1Shape':'rect'}]}";

#define PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define ADR(N, NAME) "{'number':" #N ",'name':'" NAME "','role':'output','group':'address'}"
#define DAT(N, NAME) "{'number':" #N ",'name':'" NAME "','role':'inout','group':'data'}"
#define A0_A11 \
    ADR(9, "A0") "," ADR(10, "A1") "," ADR(11, "A2") "," ADR(12, "A3") "," ADR(13, "A4") "," \
    ADR(14, "A5") "," ADR(15, "A6") "," ADR(16, "A7") "," ADR(17, "A8") "," ADR(18, "A9") "," \
    ADR(19, "A10") "," ADR(20, "A11")
#define A12_D0 \
    ADR(22, "A12") "," ADR(23, "A13") "," ADR(24, "A14") "," ADR(25, "A15") "," \
    DAT(26, "D7") "," DAT(27, "D6") "," DAT(28, "D5") "," DAT(29, "D4") "," DAT(30, "D3") "," \
    DAT(31, "D2") "," DAT(32, "D1") "," DAT(33, "D0")
#define ADDRESS_KEYS \
    "'address':['A0','A1','A2','A3','A4','A5','A6','A7','A8','A9','A10','A11','A12','A13','A14','A15']," \
    "'data':['D0','D1','D2','D3','D4','D5','D6','D7'],"

static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'cpu','devices':["
    "{'id':'FIXTURE-W65C02S','function':'65C02','description':'8-bit CMOS microprocessor (WDC W65C02S)',"
    "'drawer':['Processor'],'note':'cpu_test fixture: the W65C02S device','core':'65c02',"
    "'pinouts':{'p40':["
    PIN(1, "VPB", "output") "," PIN(2, "RDY", "inout") "," PIN(3, "PHI1O", "output") ","
    PIN(4, "IRQB", "input") "," PIN(5, "MLB", "output") "," PIN(6, "NMIB", "input") ","
    PIN(7, "SYNC", "output") "," PIN(8, "VDD", "power") "," A0_A11 "," PIN(21, "VSS", "ground") ","
    A12_D0 "," PIN(34, "RWB", "output") "," PIN(35, "NC", "nc") "," PIN(36, "BE", "input") ","
    PIN(37, "PHI2", "input") "," PIN(38, "SOB", "input") "," PIN(39, "PHI2O", "output") ","
    PIN(40, "RESB", "input") "]},"
    ADDRESS_KEYS
    "'clock':'PHI2','rw':'RWB','sync':'SYNC','irq':'IRQB','nmi':'NMIB','reset':'RESB','so':'SOB',"
    "'rdy':'RDY','phi1Out':'PHI1O','phi2Out':'PHI2O','vectorPull':'VPB','memoryLock':'MLB',"
    "'busEnable':'BE',"
    "'grades':{'14':{'tcyc':70,'tpwh':35,'tpwl':35,'tads':30,'tah':10,'tsync':30,'tmds':25,"
    "'tdhw':10,'tdsr':10,'tpcs':10,'tbvd':25,'vcc':[1.71,5.25]}},"
    "'parts':[{'name':'FIXTURE-W65C02S6TPG-14','package':'PDIP','pinout':'p40',"
    "'footprint':'FIXTURE-PDIP-40','grade':'14'}]},"
    "{'id':'FIXTURE-MCS6502','function':'6502','description':'8-bit NMOS microprocessor (MOS 6502)',"
    "'drawer':['Processor'],'note':'cpu_test fixture: the MCS6502 device','core':'6502',"
    "'pinouts':{'p40':["
    PIN(1, "VSS", "ground") "," PIN(2, "RDY", "input") "," PIN(3, "PHI1OUT", "output") ","
    PIN(4, "IRQ#", "input") "," PIN(5, "NC", "nc") "," PIN(6, "NMI#", "input") ","
    PIN(7, "SYNC", "output") "," PIN(8, "VCC", "power") "," A0_A11 "," PIN(21, "VSS", "ground") ","
    A12_D0 "," PIN(34, "R/W", "output") "," PIN(35, "NC", "nc") "," PIN(36, "NC", "nc") ","
    PIN(37, "PHI0IN", "input") "," PIN(38, "SO#", "input") "," PIN(39, "PHI2OUT", "output") ","
    PIN(40, "RES#", "input") "]},"
    ADDRESS_KEYS
    "'clock':'PHI0IN','rw':'R/W','sync':'SYNC','irq':'IRQ#','nmi':'NMI#','reset':'RES#','so':'SO#',"
    "'rdy':'RDY','phi1Out':'PHI1OUT','phi2Out':'PHI2OUT',"
    "'grades':{"
    "'1':{'tcyc':1000,'tpwh':460,'tpwl':480,'tads':300,'tah':30,'tsync':360,'tmds':200,'tdhw':30,"
    "'tdsr':100,'tpcs':100,'tbvd':0,'vcc':[4.75,5.25]},"
    "'A':{'tcyc':500,'tpwh':240,'tpwl':240,'tads':150,'tah':30,'tsync':175,'tmds':100,'tdhw':30,"
    "'tdsr':50,'tpcs':50,'tbvd':0,'vcc':[4.75,5.25]},"
    "'B':{'tcyc':333,'tpwh':160,'tpwl':163,'tads':125,'tah':10,'tsync':120,'tmds':100,'tdhw':10,"
    "'tdsr':50,'tpcs':50,'tbvd':0,'vcc':[4.75,5.25]},"
    "'C':{'tcyc':250,'tpwh':123,'tpwl':123,'tads':85,'tah':10,'tsync':100,'tmds':90,'tdhw':10,"
    "'tdsr':40,'tpcs':40,'tbvd':0,'vcc':[4.75,5.25]}},"
    "'parts':[{'name':'FIXTURE-MPS6502','package':'PDIP','pinout':'p40',"
    "'footprint':'FIXTURE-PDIP-40','grade':'1','flags':['obsolete']}]}"
    "]}";

#define W65C02S "FIXTURE-W65C02S6TPG-14"
#define MPS6502 "FIXTURE-MPS6502"

static char *unquote(const char *text, size_t size) {
    char *copy = malloc(size);
    size_t i;

    for (i = 0; i < size; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

static int load_fixture(void) {
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

/* ---- The bench ------------------------------------------------------------ */

enum { T_RISE, T_FALL, T_SERVE, T_SAMPLE, T_RELEASE, T_POKE };

/* One bus cycle, sampled 1 ps before PHI2 falls. addr/data -1: not all
   bits 0 or 1 (data -2: Z). rw, sync, vpb, mlb: '0', '1', 'X' or 'Z'. */
typedef struct {
    int addr;
    char rw, sync, vpb, mlb;
    int data;
    SimTime at;
} Cycle;

enum { LOG_MAX = 8192 };

typedef struct {
    Sim *sim;
    SimInstance *cpu;
    SimInstance *tb;
    const Part *part;
    int wdc;
    size_t a[16], d[8];
    size_t clk, rw, sync, irq, nmi, res, so, rdy, phi1, phi2, vpb, mlb, be;
    SimTime high, low;      /* the next clock phases */
    SimTime next_fall;
    uint64_t falls;
    uint8_t mem[65536];
    uint8_t known[65536];   /* 1: mem holds the byte; 0: it reads X */
    int serving;
    uint8_t poke_value;
    unsigned stores;
    Cycle log[LOG_MAX];
    size_t log_count;
    size_t reset_log;       /* log index of the first cycle after reset */
    SimTime release;        /* the PHI2 fall that ended the reset */
} Bench;

static size_t bench_pins(const void *config) {
    return ((const Bench *)config)->part->pin_count;
}

static void *bench_create(const void *config) {
    return (void *)config;
}

static void bench_destroy(void *state) {
    (void)state;
}

static LogicValue pin_value(const Bench *b, size_t pin) {
    return sim_read_pin(b->sim, b->tb, pin);
}

static char pin_char(const Bench *b, size_t pin) {
    return "01XZ"[pin_value(b, pin)];
}

/* The address bus: its value, or -1 when a bit is not 0/1. */
static int bus_address(const Bench *b) {
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
static int bus_data(const Bench *b) {
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

static void drive(Bench *b, size_t pin, LogicValue value, SimTime delay) {
    sim_schedule_drive(b->sim, b->tb, pin, value, DRIVE_STRONG, delay);
}

static void drive_data(Bench *b, int value) {
    int i;

    for (i = 0; i < 8; i++) {
        drive(b, b->d[i], value < 0 ? LOGIC_X : (value >> i) & 1 ? LOGIC_1 : LOGIC_0, 0);
    }
}

static SimTime min_time(SimTime x, SimTime y) {
    return x < y ? x : y;
}

static void bench_start(Sim *sim, SimInstance *instance, void *state) {
    Bench *b = state;

    (void)instance;
    drive(b, b->clk, LOGIC_0, 0);
    sim_schedule_timer(sim, b->tb, T_RISE, b->low);
    b->next_fall = b->low + b->high;
}

static void bench_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
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

static const SimComponentType BENCH_TYPE = {
    "bench", bench_pins, bench_create, bench_destroy, bench_start, NULL, bench_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static size_t pin_named(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s on %s\n", name, part->name);
    exit(2);
}

static void run_to(Bench *b, SimTime time) {
    if (!sim_run_until(b->sim, time)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* Runs n more PHI2 falls, stopping 200 ns after the last. */
static void run_cycles(Bench *b, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        run_to(b, b->next_fall + NS(200));
    }
}

static void set_pin(Bench *b, size_t pin, LogicValue value) {
    drive(b, pin, value, 0);
}

/* A bench at power-up: 1 MHz clock (500 ns high), every input inactive
   (RDY a weak pull-up, so WAI can pull it low), memory all X. */
static Bench *bench_open(const char *name) {
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

static void bench_close(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

static int cpu_state_of(const Bench *b, CpuState *out) {
    const SimComponentType *type = sim_instance_type(b->cpu);

    return type->cpu_state != NULL && type->cpu_state(sim_instance_state(b->cpu), out);
}

static const char *const CORE_NAMES[] = {W65C02S, MPS6502};

/* Bytes the bench serves from address. */
static void load(Bench *b, uint16_t address, const uint8_t *bytes, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        b->mem[(uint16_t)(address + i)] = bytes[i];
        b->known[(uint16_t)(address + i)] = 1;
    }
}

static void load_byte(Bench *b, uint16_t address, uint8_t value) {
    load(b, address, &value, 1);
}

/* RES low for 3 cycles, then high; returns 200 ns after the PHI2 fall that
   releases it. b->release is that fall; b->reset_log the log index of the
   first cycle after it. */
static void do_reset(Bench *b) {
    set_pin(b, b->res, LOGIC_0);
    run_cycles(b, 3);
    set_pin(b, b->res, LOGIC_1);
    b->release = b->next_fall;
    run_cycles(b, 1);
    b->reset_log = b->log_count;
}

/* A bench with the vectors (reset C000, IRQ D000, NMI E000), RTI at D000
   and E000, C000-C0FF NOPs, the program at C000, and a reset done. */
static Bench *boot(const char *name, const uint8_t *program, size_t count) {
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
static SimTime fall_at(const Bench *b, unsigned k) {
    return b->release + (SimTime)k * NS(1000);
}

/* Cycle i after reset (logged at fall i + 1). */
static const Cycle *cycle(const Bench *b, size_t i) {
    static const Cycle none = {-1, '?', '?', '?', '?', -1, 0};

    return b->reset_log + i < b->log_count ? &b->log[b->reset_log + i] : &none;
}

/* Runs until cycle i after reset has been logged. */
static void run_to_cycle(Bench *b, size_t i) {
    while (b->log_count <= b->reset_log + i) {
        run_cycles(b, 1);
    }
}

static int status_is(const Bench *b, CpuStatus status) {
    CpuState state;

    return cpu_state_of(b, &state) && state.status == status;
}

static const char *label(const char *name, const char *text) {
    static char buffer[160];

    snprintf(buffer, sizeof buffer, "%s %s", name, text);
    return buffer;
}

static uint8_t reg(const CpuState *state, const char *name) {
    size_t i;

    for (i = 0; i < state->register_count; i++) {
        if (strcmp(state->registers[i].name, name) == 0) {
            return (uint8_t)state->registers[i].value;
        }
    }
    return 0xEE;
}

static int address_is(const Bench *b, int value) {
    return bus_address(b) == value;
}

/* ---- 5.1 Both cores ------------------------------------------------------- */

/* 1. Power-up lost. */
static void case_power_up(const char *name) {
    Bench *b = bench_open(name);
    CpuState state;
    char what[128];
    int x = 1;
    size_t i;

    run_cycles(b, 5);
    for (i = 0; i < 16; i++) {
        x = x && pin_value(b, b->a[i]) == LOGIC_X;
    }
    snprintf(what, sizeof what, "%s 1: power-up address, R/W, SYNC read X", name);
    expect(x && pin_value(b, b->rw) == LOGIC_X && pin_value(b, b->sync) == LOGIC_X, what);
    snprintf(what, sizeof what, "%s 1: power-up status LOST, no registers", name);
    expect(cpu_state_of(b, &state) && state.status == CPU_LOST && state.register_count == 0, what);
    bench_close(b);
}

/* 1. The same through the circuit API, and its refusal for other kinds. */
static void case_circuit_api(void) {
    Circuit *circuit = circuit_create(library);
    CpuState state;
    size_t cpu, gate;
    const char *error;

    expect(circuit_add_part(circuit, W65C02S, &cpu) == 0 &&
           circuit_add_part(circuit, "SN74HC00N", &gate) == 0 && circuit_start(circuit) == 0 &&
           circuit_advance(circuit, NS(1000)),
           "1: circuit with a W65C02S starts");
    expect(circuit_cpu_state(circuit, cpu, &state) == 0 && state.status == CPU_LOST &&
           state.register_count == 0,
           "1: circuit_cpu_state reports LOST");
    expect(circuit_cpu_state(circuit, gate, &state) == -1 &&
           (error = circuit_error(circuit)) != NULL && strcmp(error, "part 1 is not a processor") == 0,
           "1: circuit_cpu_state on a gate: \"part 1 is not a processor\"");
    circuit_destroy(circuit);
}

/* 2. Reset sequence: dummy reads at 0000 and the stack (the bench serves X
   there), the vector, then the first fetch; the CPU stays running. */
static void case_reset_sequence(const char *name) {
    static const int WANT[] = {0x0000, 0x0100, 0x01FF, 0x01FE, 0xFFFC, 0xFFFD, 0xC000};
    Bench *b = boot(name, NULL, 0);
    int ok = 1;
    size_t i;

    run_to_cycle(b, 6);
    for (i = 0; i < 7; i++) {
        const Cycle *c = cycle(b, i);

        ok = ok && c->addr == WANT[i] && c->rw == '1' && c->sync == (i == 6 ? '1' : '0');
    }
    expect(ok, label(name, "2: reset reads 0000 0100 01FF 01FE FFFC FFFD, then SYNC C000"));
    expect(status_is(b, CPU_RUNNING), label(name, "2: running after X at 0000 and 01xx"));
    bench_close(b);
}

/* 3. A 1-cycle reset is too short. */
static void case_short_reset(const char *name) {
    Bench *b = bench_open(name);

    set_pin(b, b->res, LOGIC_0);
    run_cycles(b, 1);
    set_pin(b, b->res, LOGIC_1);
    run_cycles(b, 3);
    expect(status_is(b, CPU_LOST), label(name, "3: RES low for 1 cycle stays LOST"));
    bench_close(b);
}

static const SimTime *timing(const Bench *b) {
    return ((const CpuGrade *)part_internal(b->part)->data)->t;
}

/* 4. The address window after the PHI2 fall that ends the SYNC cycle of
   A9 41 (C000 -> C001: A0 0 -> 1). */
static void case_address_window(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    const SimTime *t = timing(b);
    SimTime f = fall_at(b, 7);   /* ends cycle 6, the fetch at C000 */
    LogicValue before, during, after;

    run_to(b, f + t[4] - NS(1));
    before = pin_value(b, b->a[0]);
    run_to(b, f + t[4] + NS(1));
    during = pin_value(b, b->a[0]);
    run_to(b, f + t[3] + NS(1));
    after = pin_value(b, b->a[0]);
    expect(cycle(b, 6)->addr == 0xC000 && cycle(b, 6)->sync == '1',
           label(name, "4: cycle 6 is the SYNC fetch at C000"));
    expect(before == LOGIC_0, label(name, "4: A0 still 0 just before tAH"));
    expect(during == LOGIC_X, label(name, "4: A0 X just after tAH"));
    expect(after == LOGIC_1 && address_is(b, 0xC001), label(name, "4: address C001 at tADS"));
    bench_close(b);
}

/* 5. STA $0200 with A = 41: the write cycle's R/W and data windows. */
static void case_write_timing(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    const SimTime *t = timing(b);
    SimTime f = fall_at(b, 11);  /* starts cycle 11, the write */
    SimTime rise = f + NS(500);
    SimTime end = f + NS(1000);
    int rw_before, rw_after, d_x, d_early, d_valid, d_hold, d_z;

    run_to(b, f + t[3] - NS(1));
    rw_before = pin_value(b, b->rw);
    run_to(b, f + t[3] + NS(1));
    rw_after = pin_value(b, b->rw);
    run_to(b, rise + NS(1));
    d_x = bus_data(b);
    run_to(b, rise + t[6] - NS(1));
    d_early = bus_data(b);
    run_to(b, rise + t[6] + NS(1));
    d_valid = bus_data(b);
    run_to(b, end + t[7] - NS(1));
    d_hold = bus_data(b);
    run_to(b, end + t[7] + NS(1));
    d_z = bus_data(b);
    expect(rw_before != LOGIC_0 && rw_after == LOGIC_0, label(name, "5: R/W 0 at tADS"));
    expect(d_x == -1 && d_early == -1, label(name, "5: data X from PHI2 rise"));
    expect(d_valid == 0x41, label(name, "5: data 41 at rise + tMDS"));
    expect(d_hold == 0x41 && d_z == -2, label(name, "5: data held to fall + tDHW, then Z"));
    expect(cycle(b, 11)->addr == 0x0200 && cycle(b, 11)->rw == '0' && b->known[0x200] &&
               b->mem[0x200] == 0x41,
           label(name, "5: the bench stores 41 at 0200"));
    bench_close(b);
}

/* 6. LDA $0300 reading X: lost. */
static void case_used_x(const char *name) {
    static const uint8_t PROGRAM[] = {0xAD, 0x00, 0x03};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);

    run_to_cycle(b, 10);
    expect(cycle(b, 9)->addr == 0x0300 && status_is(b, CPU_LOST),
           label(name, "6: LDA $0300 with X at 0300 goes LOST"));
    bench_close(b);
}

/* 7. The opcode byte changes just inside / just outside tDSR. */
static void case_setup(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0xEA, 0xEA, 0xEA};
    int pass;

    for (pass = 0; pass < 2; pass++) {
        Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
        const SimTime *t = timing(b);
        SimTime f = fall_at(b, 7);   /* ends the fetch at C000 */
        SimTime poke = pass == 0 ? f - t[8] + NS(1) : f - t[8] - NS(1);

        run_to(b, f - NS(300));
        b->poke_value = 0xEA;
        sim_schedule_timer(b->sim, b->tb, T_POKE, poke - sim_now(b->sim));
        run_to_cycle(b, 9);
        if (pass == 0) {
            expect(status_is(b, CPU_LOST),
                   label(name, "7: opcode change tDSR - 1 ns before the fall: LOST"));
        } else {
            expect(status_is(b, CPU_RUNNING) && cycle(b, 8)->addr == 0xC001 &&
                       cycle(b, 8)->sync == '1',
                   label(name, "7: opcode change tDSR + 1 ns before the fall: runs EA (NOP)"));
        }
        bench_close(b);
    }
}

static size_t find_cycle(const Bench *b, size_t from, int addr, char rw) {
    size_t i;

    for (i = from; b->reset_log + i < b->log_count; i++) {
        if (cycle(b, i)->addr == addr && cycle(b, i)->rw == rw) {
            return i;
        }
    }
    return (size_t)-1;
}

static size_t count_cycles(const Bench *b, size_t from, int addr) {
    size_t i;
    size_t n = 0;

    for (i = from; b->reset_log + i < b->log_count; i++) {
        n += cycle(b, i)->addr == addr;
    }
    return n;
}

/* 8. IRQ (after CLI) and NMI. */
static void case_interrupts(const char *name) {
    static const uint8_t CLI[] = {0x58};
    Bench *b = boot(name, CLI, sizeof CLI);
    size_t v;

    run_to_cycle(b, 10);
    set_pin(b, b->irq, LOGIC_0);
    run_to_cycle(b, 30);
    v = find_cycle(b, 10, 0xFFFE, '1');
    expect(v != (size_t)-1 && cycle(b, v + 1)->addr == 0xFFFF,
           label(name, "8: IRQ reads the vector at FFFE/FFFF"));
    expect(v != (size_t)-1 && v >= 3 && cycle(b, v - 1)->rw == '0' &&
               (cycle(b, v - 1)->addr & 0xFF00) == 0x0100 && cycle(b, v - 1)->data >= 0 &&
               (cycle(b, v - 1)->data & 0x10) == 0,
           label(name, "8: the P pushed by IRQ has B = 0"));
    bench_close(b);

    b = boot(name, NULL, 0);
    run_to_cycle(b, 10);
    set_pin(b, b->nmi, LOGIC_0);
    run_to_cycle(b, 60);
    v = find_cycle(b, 10, 0xFFFA, '1');
    expect(v != (size_t)-1 && cycle(b, v + 1)->addr == 0xFFFB,
           label(name, "8: NMI 1->0 reads the vector at FFFA/FFFB"));
    expect(count_cycles(b, 10, 0xFFFA) == 1, label(name, "8: NMI held at 0 does not re-trigger"));
    bench_close(b);
}

static void set_rdy(Bench *b, int level) {
    if (level) {
        sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    } else {
        set_pin(b, b->rdy, LOGIC_0);
    }
}

/* 9. RDY low on the read of 0300 holds it. */
static void case_rdy_read(const char *name) {
    static const uint8_t PROGRAM[] = {0xAD, 0x00, 0x03};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    CpuState before, after;
    int ok = 1;
    size_t i;

    load_byte(b, 0x0300, 0x55);
    run_to(b, fall_at(b, 9) + NS(200));   /* cycle 9 reads 0300 */
    set_rdy(b, 0);
    cpu_state_of(b, &before);
    run_cycles(b, 3);
    cpu_state_of(b, &after);
    set_rdy(b, 1);
    run_to_cycle(b, 13);
    for (i = 9; i <= 12; i++) {
        ok = ok && cycle(b, i)->addr == 0x0300 && cycle(b, i)->rw == '1';
    }
    expect(ok, label(name, "9: the read of 0300 repeats while RDY is low"));
    expect(after.cycles == before.cycles + 3, label(name, "9: the cycle count grows while held"));
    expect(cycle(b, 13)->addr == 0xC003 && cycle(b, 13)->sync == '1',
           label(name, "9: released, the next fetch is C003"));
    bench_close(b);
}

/* 10. SO falling sets V. */
static void case_so(const char *name) {
    Bench *b = boot(name, NULL, 0);
    CpuState before, after;

    run_to_cycle(b, 10);
    cpu_state_of(b, &before);
    set_pin(b, b->so, LOGIC_0);
    run_cycles(b, 2);
    cpu_state_of(b, &after);
    expect((reg(&before, "P") & 0x40) == 0 && (reg(&after, "P") & 0x40) != 0,
           label(name, "10: SO 1->0 sets V"));
    bench_close(b);
}

/* 11. Registers, the last instruction and the cycle count. */
static void case_read_back(const char *name) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0xA2, 0x05, 0xA0, 0x07};
    Bench *b = boot(name, PROGRAM, sizeof PROGRAM);
    CpuState state;

    run_cycles(b, 12);   /* to the fetch of C006 */
    cpu_state_of(b, &state);
    expect(state.register_count == 6 && reg(&state, "A") == 0x41 && reg(&state, "X") == 0x05 &&
               reg(&state, "Y") == 0x07,
           label(name, "11: registers A 41, X 05, Y 07"));
    expect(state.has_instruction && strcmp(state.instruction_text, "LDY #$07") == 0 &&
               state.instruction_address == 0xC004 && state.instruction_length == 2 &&
               state.instruction_bytes[0] == 0xA0 && state.instruction_bytes[1] == 0x07,
           label(name, "11: last instruction LDY #$07 at C004, bytes A0 07"));
    expect(state.cycles == b->log_count - b->reset_log,
           label(name, "11: the cycle count equals the logged cycles since reset"));
    bench_close(b);

    {
        /* JSR reads its high byte after two stack writes; it still counts. */
        static const uint8_t JSR[] = {0x20, 0x00, 0xC1};

        b = boot(name, JSR, sizeof JSR);
        load_byte(b, 0xC100, 0xEA);
        run_to_cycle(b, 12);   /* JSR is cycles 6-11; the fetch at C100 is 12 */
        cpu_state_of(b, &state);
        expect(cycle(b, 12)->addr == 0xC100 && state.has_instruction &&
                   strcmp(state.instruction_text, "JSR $C100") == 0 &&
                   state.instruction_length == 3 && state.instruction_bytes[2] == 0xC1,
               label(name, "11: last instruction JSR $C100, bytes 20 00 C1"));
        bench_close(b);
    }
}

/* 12. A high phase shorter than tPWH; a period shorter than tCYC. */
static void case_clock(const char *name) {
    Bench *b = boot(name, NULL, 0);
    const SimTime *t = timing(b);

    run_cycles(b, 3);
    b->low = NS(600);               /* the next low phase: keeps the period above tCYC */
    run_cycles(b, 1);
    b->high = t[1] - NS(5);         /* the next high phase */
    b->low = NS(500);
    run_to(b, sim_now(b->sim) + NS(500));
    b->high = NS(500);
    run_cycles(b, 2);
    expect(status_is(b, CPU_LOST), label(name, "12: a high phase below tPWH: LOST"));
    bench_close(b);

    b = boot(name, NULL, 0);
    run_cycles(b, 3);
    /* A period below tCYC with each phase at its minimum width. The
       W65C02S's tCYC is tPWH + tPWL, so there the phases are short too. */
    if (t[0] >= t[1] + t[2] + NS(20)) {
        b->low = t[2];
        run_cycles(b, 1);
        b->high = t[1];
    } else {
        b->low = t[0] / 2 - NS(5);
        run_cycles(b, 1);
        b->high = t[0] / 2 - NS(5);
    }
    run_to(b, sim_now(b->sim) + b->low);
    b->high = NS(500);
    b->low = NS(500);
    run_cycles(b, 2);
    expect(status_is(b, CPU_LOST), label(name, "12: a period below tCYC: LOST"));
    bench_close(b);
}

/* ---- 5.2 W65C02S only ------------------------------------------------------- */

/* 13. RDY low during STA's write holds the write. */
static void case_rdy_write_wdc(void) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02};
    Bench *b = boot(W65C02S, PROGRAM, sizeof PROGRAM);
    int ok = 1;
    size_t i;

    run_to(b, fall_at(b, 11) + NS(200));   /* cycle 11 writes 0200 */
    set_rdy(b, 0);
    run_cycles(b, 2);
    set_rdy(b, 1);
    run_to_cycle(b, 14);
    for (i = 11; i <= 13; i++) {
        ok = ok && cycle(b, i)->addr == 0x0200 && cycle(b, i)->rw == '0' &&
             cycle(b, i)->data == 0x41;
    }
    expect(ok, "W65C02S 13: RDY low holds the write: 0200, R/W 0, data 41 on every held cycle");
    expect(cycle(b, 14)->addr == 0xC005 && cycle(b, 14)->sync == '1' && b->mem[0x200] == 0x41,
           "W65C02S 13: released, the write completes and the next fetch is C005");
    bench_close(b);
}

/* 14. VPB on the vector reads of reset and IRQ only. */
static void case_vpb(void) {
    static const uint8_t CLI[] = {0x58};
    Bench *b = boot(W65C02S, CLI, sizeof CLI);
    int ok = 1;
    size_t i;

    run_to_cycle(b, 10);
    set_pin(b, b->irq, LOGIC_0);
    run_to_cycle(b, 24);
    for (i = 0; i <= 24; i++) {
        const Cycle *c = cycle(b, i);
        int vector = c->addr >= 0xFFFC;

        ok = ok && (c->vpb == '0') == vector;
    }
    expect(ok && cycle(b, 4)->vpb == '0' && cycle(b, 5)->vpb == '0' &&
               find_cycle(b, 10, 0xFFFE, '1') != (size_t)-1,
           "W65C02S 14: VPB low exactly on the FFFC/FFFD and FFFE/FFFF reads");
    bench_close(b);
}

/* 15. MLB on the three RMW cycles of INC $0300. */
static void case_mlb(void) {
    static const uint8_t PROGRAM[] = {0xEE, 0x00, 0x03};
    Bench *b = boot(W65C02S, PROGRAM, sizeof PROGRAM);
    int ok = 1;
    size_t i;

    load_byte(b, 0x0300, 0x10);
    run_to_cycle(b, 14);
    for (i = 0; i <= 14; i++) {
        ok = ok && (cycle(b, i)->mlb == '0') == (i >= 9 && i <= 11);
    }
    expect(ok && cycle(b, 9)->addr == 0x0300 && cycle(b, 10)->addr == 0x0300 &&
               cycle(b, 11)->addr == 0x0300 && cycle(b, 11)->rw == '0' && b->mem[0x300] == 0x11,
           "W65C02S 15: MLB low on exactly the read, repeat read and write of 0300");
    bench_close(b);
}

static int bus_floats(const Bench *b) {
    int ok = pin_value(b, b->rw) == LOGIC_Z;
    size_t i;

    for (i = 0; i < 16; i++) {
        ok = ok && pin_value(b, b->a[i]) == LOGIC_Z;
    }
    for (i = 0; i < 8; i++) {
        ok = ok && pin_value(b, b->d[i]) == LOGIC_Z;
    }
    return ok;
}

/* 16. BE low floats the bus; the CPU keeps running. While BE is low the
   bench takes the bus (address C000, R/W 1), so the CPU reads NOPs. */
static void case_be(void) {
    Bench *b = boot(W65C02S, NULL, 0);
    CpuState before, after;
    SimTime t;
    size_t i;
    int restored = 1;

    run_cycles(b, 10);
    cpu_state_of(b, &before);
    t = sim_now(b->sim);
    set_pin(b, b->be, LOGIC_0);
    run_to(b, t + NS(25) + NS(1));
    expect(bus_floats(b), "W65C02S 16: BE 0: address, data and RWB Z after tBVD");
    for (i = 0; i < 16; i++) {
        set_pin(b, b->a[i], (0xC000 >> i) & 1 ? LOGIC_1 : LOGIC_0);
    }
    set_pin(b, b->rw, LOGIC_1);
    run_cycles(b, 3);
    cpu_state_of(b, &after);
    expect(after.status == CPU_RUNNING && after.cycles == before.cycles + 3,
           "W65C02S 16: the CPU keeps running with BE low (cycles +3)");
    for (i = 0; i < 16; i++) {
        set_pin(b, b->a[i], LOGIC_Z);
    }
    set_pin(b, b->rw, LOGIC_Z);
    t = sim_now(b->sim);
    set_pin(b, b->be, LOGIC_1);
    run_to(b, t + NS(25) + NS(1));
    for (i = 0; i < 16; i++) {
        LogicValue v = pin_value(b, b->a[i]);

        restored = restored && (v == LOGIC_0 || v == LOGIC_1);
    }
    expect(restored && pin_value(b, b->rw) == LOGIC_1, "W65C02S 16: BE 1 restores address and RWB");
    bench_close(b);
}

/* 17. WAI: waits with RDY low; IRQ with I set resumes at the next
   instruction. STP: only a reset leaves it. */
static void case_wai_stp(void) {
    static const uint8_t WAI[] = {0xCB};
    static const uint8_t STP[] = {0xDB};
    Bench *b = boot(W65C02S, WAI, sizeof WAI);
    size_t i;
    int no_vector = 1;

    run_to_cycle(b, 12);
    expect(status_is(b, CPU_WAITING) && pin_value(b, b->rdy) == LOGIC_0,
           "W65C02S 17: WAI: status WAITING, RDY driven 0");
    set_pin(b, b->irq, LOGIC_0);
    run_cycles(b, 4);
    for (i = 0; b->reset_log + i < b->log_count; i++) {
        no_vector = no_vector && cycle(b, i)->addr != 0xFFFE;
    }
    expect(status_is(b, CPU_RUNNING) && no_vector && pin_value(b, b->rdy) == LOGIC_1 &&
               find_cycle(b, 13, 0xC002, '1') != (size_t)-1,
           "W65C02S 17: IRQ with I set resumes at the next instruction, no vector, RDY released");
    bench_close(b);

    b = boot(W65C02S, STP, sizeof STP);
    run_to_cycle(b, 12);
    set_pin(b, b->nmi, LOGIC_0);
    set_pin(b, b->irq, LOGIC_0);
    run_cycles(b, 5);
    expect(status_is(b, CPU_STOPPED), "W65C02S 17: STP: STOPPED, and IRQ/NMI do not leave it");
    set_pin(b, b->nmi, LOGIC_1);
    set_pin(b, b->irq, LOGIC_1);
    do_reset(b);
    run_to_cycle(b, 5);   /* the reset sequence's vector read; STP at C000 comes next */
    expect(status_is(b, CPU_RUNNING) && cycle(b, 4)->addr == 0xFFFC,
           "W65C02S 17: a reset leaves STOPPED");
    bench_close(b);
}

/* 18. The program of the sub-plan's outcome. */
static void case_program(void) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02, 0xDB};
    Bench *b = boot(W65C02S, PROGRAM, sizeof PROGRAM);
    CpuState state;
    size_t w;

    run_to_cycle(b, 20);
    w = find_cycle(b, 0, 0x0200, '0');
    cpu_state_of(b, &state);
    expect(w != (size_t)-1 && cycle(b, w)->data == 0x41 && b->mem[0x200] == 0x41,
           "W65C02S 18: the bench sees the write of 41 to 0200");
    expect(state.status == CPU_STOPPED && reg(&state, "A") == 0x41,
           "W65C02S 18: A = 41, status Stopped");
    expect(state.has_instruction && strcmp(state.instruction_text, "STP") == 0 &&
               state.instruction_address == 0xC005,
           "W65C02S 18: last instruction STP");
    bench_close(b);
}

/* ---- 5.3 NMOS only ------------------------------------------------------------ */

/* 19. The NMOS chip ignores RDY on a write. */
static void case_rdy_write_nmos(void) {
    static const uint8_t PROGRAM[] = {0xA9, 0x41, 0x8D, 0x00, 0x02};
    Bench *b = boot(MPS6502, PROGRAM, sizeof PROGRAM);

    run_to(b, fall_at(b, 11) + NS(200));   /* cycle 11 writes 0200 */
    set_rdy(b, 0);
    run_to_cycle(b, 13);
    expect(cycle(b, 11)->addr == 0x0200 && cycle(b, 11)->rw == '0' && cycle(b, 12)->rw == '1' &&
               cycle(b, 12)->addr == 0xC005 && b->mem[0x200] == 0x41,
           "MPS6502 19: RDY low during the write: it completes in one cycle");
    bench_close(b);
}

/* 20. A JAM opcode. */
static void case_jam(void) {
    static const uint8_t PROGRAM[] = {0x02};
    Bench *b = boot(MPS6502, PROGRAM, sizeof PROGRAM);

    run_to_cycle(b, 12);
    expect(status_is(b, CPU_JAMMED), "MPS6502 20: opcode 02: JAMMED");
    bench_close(b);
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
        case_power_up(CORE_NAMES[i]);
        case_reset_sequence(CORE_NAMES[i]);
        case_short_reset(CORE_NAMES[i]);
        case_address_window(CORE_NAMES[i]);
        case_write_timing(CORE_NAMES[i]);
        case_used_x(CORE_NAMES[i]);
        case_setup(CORE_NAMES[i]);
        case_interrupts(CORE_NAMES[i]);
        case_rdy_read(CORE_NAMES[i]);
        case_so(CORE_NAMES[i]);
        case_read_back(CORE_NAMES[i]);
        case_clock(CORE_NAMES[i]);
    }
    case_rdy_write_wdc();
    case_vpb();
    case_mlb();
    case_be();
    case_wai_stp();
    case_program();
    case_rdy_write_nmos();
    case_jam();
    case_circuit_api();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
