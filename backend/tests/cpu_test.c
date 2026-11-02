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

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
        case_power_up(CORE_NAMES[i]);
    }
    case_circuit_api();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
