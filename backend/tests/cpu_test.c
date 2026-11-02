/*
 * cpu_test: the cpu kind's bus adapters against the datasheet rules
 * (plan-09-B section 5).
 *
 * The cases run on inline fixture devices that carry exactly the data
 * plan-09-D ships (W65C02S grade 14, MOS 6502 grades 1/A/B/C, Z84C00 grades
 * 6/8/10/20, PDIP-40 pinouts). A bench component clocks the CPU at 1 MHz and serves a 64 KB
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
    ",{'id':'FIXTURE-Z84C00','function':'Z80','description':'8-bit CMOS microprocessor (Zilog Z80)',"
    "'drawer':['Processor'],'note':'cpu_test fixture: the Z84C00 device','core':'z80',"
    "'pinouts':{'p40':["
    ADR(1, "A11") "," ADR(2, "A12") "," ADR(3, "A13") "," ADR(4, "A14") "," ADR(5, "A15") ","
    PIN(6, "CLK", "input") "," DAT(7, "D4") "," DAT(8, "D3") "," DAT(9, "D5") "," DAT(10, "D6") ","
    PIN(11, "+5V", "power") "," DAT(12, "D2") "," DAT(13, "D7") "," DAT(14, "D0") "," DAT(15, "D1") ","
    PIN(16, "INT#", "input") "," PIN(17, "NMI#", "input") "," PIN(18, "HALT#", "output") ","
    PIN(19, "MREQ#", "output") "," PIN(20, "IORQ#", "output") "," PIN(21, "RD#", "output") ","
    PIN(22, "WR#", "output") "," PIN(23, "BUSACK#", "output") "," PIN(24, "WAIT#", "input") ","
    PIN(25, "BUSREQ#", "input") "," PIN(26, "RESET#", "input") "," PIN(27, "M1#", "output") ","
    PIN(28, "RFSH#", "output") "," PIN(29, "GND", "ground") ","
    ADR(30, "A0") "," ADR(31, "A1") "," ADR(32, "A2") "," ADR(33, "A3") "," ADR(34, "A4") ","
    ADR(35, "A5") "," ADR(36, "A6") "," ADR(37, "A7") "," ADR(38, "A8") "," ADR(39, "A9") ","
    ADR(40, "A10") "]},"
    ADDRESS_KEYS
    "'clock':'CLK','m1':'M1#','mreq':'MREQ#','iorq':'IORQ#','rd':'RD#','wr':'WR#','rfsh':'RFSH#',"
    "'halt':'HALT#','busack':'BUSACK#','wait':'WAIT#','int':'INT#','nmi':'NMI#','reset':'RESET#',"
    "'busreq':'BUSREQ#',"
    "'grades':{"
    "'6':{'tcc':162,'twch':65,'twcl':65,'tdcra':90,'tdcfmreqf':70,'tdcrmreqr':70,'tdcfmreqr':70,'tdcfrdf':80,'tdcrrdr':70,'tsdcr':30,'tswaitcf':60,'tdcrm1f':80,'tdcrm1r':80,'tdcrrfshf':110,'tdcrrfshr':100,'tdcfrdr':70,'tdcrrdf':70,'tsdcf':40,'tdcriorqf':65,'tdcfiorqr':70,'tdcfwrf':70,'tdcfwrr':70,'tdcrwrf':60,'tdcfhalt':260,'twnmi':60,'tsbusreq':50,'tdcrbusackf':90,'tdcfbusackr':90,'tdcrdz':80,'tdcrctz':70,'tdcraz':80,'tsreset':60,'tsint':70,'tdcfiorqf':70,'tdcfd':130,'vcc':[4.5,5.5]},"
    "'8':{'tcc':125,'twch':55,'twcl':55,'tdcra':80,'tdcfmreqf':60,'tdcrmreqr':60,'tdcfmreqr':60,'tdcfrdf':70,'tdcrrdr':60,'tsdcr':30,'tswaitcf':50,'tdcrm1f':70,'tdcrm1r':70,'tdcrrfshf':95,'tdcrrfshr':85,'tdcfrdr':60,'tdcrrdf':60,'tsdcf':30,'tdcriorqf':55,'tdcfiorqr':60,'tdcfwrf':60,'tdcfwrr':60,'tdcrwrf':60,'tdcfhalt':225,'twnmi':60,'tsbusreq':40,'tdcrbusackf':80,'tdcfbusackr':80,'tdcrdz':70,'tdcrctz':60,'tdcraz':70,'tsreset':45,'tsint':55,'tdcfiorqf':60,'tdcfd':115,'vcc':[4.5,5.5]},"
    "'10':{'tcc':100,'twch':40,'twcl':40,'tdcra':65,'tdcfmreqf':55,'tdcrmreqr':55,'tdcfmreqr':55,'tdcfrdf':65,'tdcrrdr':55,'tsdcr':25,'tswaitcf':20,'tdcrm1f':65,'tdcrm1r':65,'tdcrrfshf':80,'tdcrrfshr':80,'tdcfrdr':55,'tdcrrdf':55,'tsdcf':25,'tdcriorqf':50,'tdcfiorqr':55,'tdcfwrf':55,'tdcfwrr':55,'tdcrwrf':50,'tdcfhalt':90,'twnmi':60,'tsbusreq':30,'tdcrbusackf':75,'tdcfbusackr':75,'tdcrdz':65,'tdcrctz':65,'tdcraz':75,'tsreset':40,'tsint':50,'tdcfiorqf':55,'tdcfd':110,'vcc':[4.5,5.5]},"
    "'20':{'tcc':50,'twch':20,'twcl':20,'tdcra':57,'tdcfmreqf':40,'tdcrmreqr':40,'tdcfmreqr':40,'tdcfrdf':40,'tdcrrdr':40,'tsdcr':12,'tswaitcf':7.5,'tdcrm1f':45,'tdcrm1r':45,'tdcrrfshf':60,'tdcrrfshr':60,'tdcfrdr':40,'tdcrrdf':40,'tsdcf':12,'tdcriorqf':40,'tdcfiorqr':40,'tdcfwrf':40,'tdcfwrr':40,'tdcrwrf':40,'tdcfhalt':70,'twnmi':60,'tsbusreq':15,'tdcrbusackf':40,'tdcfbusackr':40,'tdcrdz':40,'tdcrctz':40,'tdcraz':40,'tsreset':15,'tsint':15,'tdcfiorqf':45,'tdcfd':75,'vcc':[4.5,5.5]}"
    "},"
    "'parts':[{'name':'FIXTURE-Z84C0010PEG','package':'PDIP','pinout':'p40',"
    "'footprint':'FIXTURE-PDIP-40','grade':'10','flags':['obsolete']}]}"
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

/* ---- The Z80 bench (plan-09-C section 5.1) ---------------------------------- */

/*
 * A second bench for the Z80: it drives CLK (1 MHz, 50 %) and the inputs,
 * and reacts to the strobes: on MREQ#+RD# (not refresh) it drives memory
 * from RD# fall + 100 ns to RD# rise + 20 ns; on MREQ#+WR# it stores at WR#
 * rise; I/O the same on IORQ# (reads return io_in); an int-ack (M1#+IORQ#)
 * gets `vector`. Every machine cycle with a strobe is logged with its T1
 * (the index of its first CLK rise) and the strobe's fall and rise times.
 */

enum { ZT_RISE, ZT_FALL, ZT_SERVE, ZT_RELEASE, ZT_POKE };
enum { ZLOG_MAX = 4096, RISES_MAX = 100000 };

/* type: F fetch, R memory read, W memory write, I I/O read, O I/O write,
   A int-ack. data -1: X. */
typedef struct {
    char type;
    int addr;
    int data;
    size_t t1;
    SimTime start, end;      /* the strobe (RD#, WR#, IORQ#) fall and rise */
    SimTime data_at;         /* writes: the last data-bus change before WR# rose */
    SimTime m1_fall;         /* int-ack: when M1# fell */
} ZCycle;

typedef struct {
    Sim *sim;
    SimInstance *cpu;
    SimInstance *tb;
    const Part *part;
    size_t a[16], d[8];
    size_t clk, m1, mreq, iorq, rd, wr, rfsh, halt, busack, wait, intr, nmi, reset, busreq;
    SimTime half;
    SimTime high_once;       /* nonzero: the next high phase */
    SimTime next_rise;
    size_t rises;
    SimTime *rise_at;
    uint8_t mem[65536];
    uint8_t known[65536];
    uint8_t io_in;
    uint8_t vector;
    int serving;
    int serve_value;
    uint8_t poke_value;
    ZCycle log[ZLOG_MAX];
    size_t log_count;
    long open_read, open_write;
    SimTime data_changed, m1_fell, halt_fell;
    size_t first;            /* the rise index of T1 of the first fetch after reset */
} ZBench;

static size_t zbench_pins(const void *config) {
    return ((const ZBench *)config)->part->pin_count;
}

static LogicValue zpin(const ZBench *b, size_t pin) {
    return sim_read_pin(b->sim, b->tb, pin);
}

static void zdrive(ZBench *b, size_t pin, LogicValue value, SimTime delay) {
    sim_schedule_drive(b->sim, b->tb, pin, value, DRIVE_STRONG, delay);
}

static int zaddress(const ZBench *b) {
    int value = 0;
    int i;

    for (i = 0; i < 16; i++) {
        LogicValue v = zpin(b, b->a[i]);

        if (v != LOGIC_0 && v != LOGIC_1) {
            return -1;
        }
        value |= (v == LOGIC_1) << i;
    }
    return value;
}

/* The data bus: value, -1 X or mixed, -2 all Z. */
static int zdata(const ZBench *b) {
    int value = 0;
    int z = 0;
    int i;

    for (i = 0; i < 8; i++) {
        LogicValue v = zpin(b, b->d[i]);

        z += v == LOGIC_Z;
        if (v == LOGIC_1) {
            value |= 1 << i;
        } else if (v != LOGIC_0) {
            value = -1000;
        }
    }
    return z == 8 ? -2 : value < 0 ? -1 : value;
}

static void zdrive_data(ZBench *b, int value) {
    int i;

    for (i = 0; i < 8; i++) {
        zdrive(b, b->d[i], value < 0 ? LOGIC_X : (value >> i) & 1 ? LOGIC_1 : LOGIC_0, 0);
    }
}

static long zopen(ZBench *b, char type, int addr, size_t back) {
    ZCycle *c;

    if (b->log_count >= ZLOG_MAX) {
        return -1;
    }
    c = &b->log[b->log_count];
    memset(c, 0, sizeof(*c));
    c->type = type;
    c->addr = addr;
    c->data = -1;
    c->t1 = b->rises - 1 - back;
    c->start = sim_now(b->sim);
    c->m1_fall = b->m1_fell;
    return (long)b->log_count++;
}

static void zserve(ZBench *b, int value) {
    b->serve_value = value;
    sim_schedule_timer(b->sim, b->tb, ZT_SERVE, NS(100));
}

static void zbench_start(Sim *sim, SimInstance *instance, void *state) {
    ZBench *b = state;

    (void)instance;
    zdrive(b, b->clk, LOGIC_0, 0);
    sim_schedule_timer(sim, b->tb, ZT_RISE, b->half);
    b->next_rise = b->half;
}

static void zbench_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    ZBench *b = state;
    SimTime high;
    int i;

    (void)instance;
    switch (id) {
        case ZT_RISE:
            zdrive(b, b->clk, LOGIC_1, 0);
            if (b->rises < RISES_MAX) {
                b->rise_at[b->rises] = sim_now(sim);
            }
            b->rises++;
            high = b->high_once != 0 ? b->high_once : b->half;
            b->high_once = 0;
            sim_schedule_timer(sim, b->tb, ZT_FALL, high);
            b->next_rise = sim_now(sim) + high + b->half;
            break;
        case ZT_FALL:
            zdrive(b, b->clk, LOGIC_0, 0);
            sim_schedule_timer(sim, b->tb, ZT_RISE, b->half);
            break;
        case ZT_SERVE:
            zdrive_data(b, b->serve_value);
            b->serving = 1;
            break;
        case ZT_RELEASE:
            for (i = 0; i < 8; i++) {
                zdrive(b, b->d[i], LOGIC_Z, 0);
            }
            b->serving = 0;
            break;
        case ZT_POKE:
            if (b->serving) {
                zdrive_data(b, b->poke_value);
            }
            break;
    }
}

static void zbench_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    ZBench *b = state;
    SimTime now = sim_now(sim);
    int i;
    long e;

    (void)instance;
    for (i = 0; i < 8; i++) {
        if (pin == b->d[i]) {
            b->data_changed = now;
        }
    }
    if (pin == b->m1 && new_value == LOGIC_0) {
        b->m1_fell = now;
    }
    if (pin == b->halt && new_value == LOGIC_0) {
        b->halt_fell = now;
    }
    if (pin == b->rd && new_value == LOGIC_0 && old_value == LOGIC_1) {
        int addr = zaddress(b);

        if (zpin(b, b->mreq) == LOGIC_0 && zpin(b, b->rfsh) == LOGIC_1) {
            e = zopen(b, zpin(b, b->m1) == LOGIC_0 ? 'F' : 'R', addr, 0);
            if (e >= 0) {
                b->log[e].data = addr >= 0 && b->known[addr] ? b->mem[addr] : -1;
                zserve(b, b->log[e].data);
                b->open_read = e;
            }
        } else if (zpin(b, b->iorq) == LOGIC_0) {
            e = zopen(b, 'I', addr, 1);
            if (e >= 0) {
                b->log[e].data = b->io_in;
                zserve(b, b->io_in);
                b->open_read = e;
            }
        }
    } else if (pin == b->rd && new_value == LOGIC_1 && b->open_read >= 0) {
        b->log[b->open_read].end = now;
        b->open_read = -1;
        sim_schedule_timer(sim, b->tb, ZT_RELEASE, NS(20));
    } else if (pin == b->wr && new_value == LOGIC_0 && old_value == LOGIC_1) {
        int addr = zaddress(b);

        if (zpin(b, b->mreq) == LOGIC_0) {
            b->open_write = zopen(b, 'W', addr, 1);   /* WR# falls in T2 */
        } else if (zpin(b, b->iorq) == LOGIC_0) {
            b->open_write = zopen(b, 'O', addr, 1);
        }
    } else if (pin == b->wr && new_value == LOGIC_1 && b->open_write >= 0) {
        ZCycle *c = &b->log[b->open_write];

        c->end = now;
        c->data = zdata(b);
        c->data_at = b->data_changed;
        if (c->type == 'W' && c->addr >= 0) {
            b->mem[c->addr] = (uint8_t)(c->data >= 0 ? c->data : 0);
            b->known[c->addr] = c->data >= 0;
        }
        b->open_write = -1;
    } else if (pin == b->iorq && new_value == LOGIC_0 && zpin(b, b->m1) == LOGIC_0) {
        e = zopen(b, 'A', zaddress(b), 2);
        if (e >= 0) {
            b->log[e].data = b->vector;
            zserve(b, b->vector);
            b->open_read = e;
        }
    } else if (pin == b->iorq && new_value == LOGIC_1 && b->open_read >= 0 &&
               b->log[b->open_read].type == 'A') {
        b->log[b->open_read].end = now;
        b->open_read = -1;
        sim_schedule_timer(sim, b->tb, ZT_RELEASE, NS(20));
    }
}

static const SimComponentType ZBENCH_TYPE = {
    "zbench", zbench_pins, bench_create, bench_destroy, zbench_start, zbench_pin_changed,
    zbench_timer, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

#define Z84C00 "FIXTURE-Z84C0010PEG"

static void zset(ZBench *b, size_t pin, LogicValue value) {
    zdrive(b, pin, value, 0);
}

static void zrun_to(ZBench *b, SimTime time) {
    if (!sim_run_until(b->sim, time)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* Runs n more CLK rises, stopping 200 ns after the last. */
static void zrun_rises(ZBench *b, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        zrun_to(b, b->next_rise + NS(200));
    }
}

static ZBench *zbench_open(void) {
    ZBench *b = calloc(1, sizeof(*b));
    const Part *part = part_library_find(library, Z84C00);
    size_t i;
    char pin[8];

    if (b == NULL || part == NULL) {
        fprintf(stderr, "no part %s\n", Z84C00);
        exit(2);
    }
    b->rise_at = calloc(RISES_MAX, sizeof(*b->rise_at));
    b->part = part;
    for (i = 0; i < 16; i++) {
        snprintf(pin, sizeof pin, "A%zu", i);
        b->a[i] = pin_named(part, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "D%zu", i);
        b->d[i] = pin_named(part, pin);
    }
    b->clk = pin_named(part, "CLK");
    b->m1 = pin_named(part, "M1#");
    b->mreq = pin_named(part, "MREQ#");
    b->iorq = pin_named(part, "IORQ#");
    b->rd = pin_named(part, "RD#");
    b->wr = pin_named(part, "WR#");
    b->rfsh = pin_named(part, "RFSH#");
    b->halt = pin_named(part, "HALT#");
    b->busack = pin_named(part, "BUSACK#");
    b->wait = pin_named(part, "WAIT#");
    b->intr = pin_named(part, "INT#");
    b->nmi = pin_named(part, "NMI#");
    b->reset = pin_named(part, "RESET#");
    b->busreq = pin_named(part, "BUSREQ#");
    b->half = NS(500);
    b->open_read = -1;
    b->open_write = -1;
    b->sim = sim_create();
    b->cpu = sim_add_instance(b->sim, kind_model(part), "U1", part);
    b->tb = sim_add_instance(b->sim, &ZBENCH_TYPE, "TB", b);
    for (i = 0; i < part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->cpu, i);
        sim_connect(b->sim, net, b->tb, i);
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "sim_start: %s\n", sim_error(b->sim));
        exit(2);
    }
    zset(b, b->intr, LOGIC_1);
    zset(b, b->nmi, LOGIC_1);
    zset(b, b->wait, LOGIC_1);
    zset(b, b->busreq, LOGIC_1);
    zset(b, b->reset, LOGIC_1);
    return b;
}

static void zbench_close(ZBench *b) {
    sim_destroy(b->sim);
    free(b->rise_at);
    free(b);
}

static void zload(ZBench *b, uint16_t address, const uint8_t *bytes, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        b->mem[(uint16_t)(address + i)] = bytes[i];
        b->known[(uint16_t)(address + i)] = 1;
    }
}

/* RESET# low for 4 clocks; returns 200 ns after T1 rise of the first fetch. */
static void zreset(ZBench *b) {
    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 4);
    zset(b, b->reset, LOGIC_1);
    b->first = b->rises;
    zrun_rises(b, 1);
}

/* A bench with NOPs at 0000-00FF, 0038, 0066 and 0500, the program at
   0000, and a reset done. */
static ZBench *zboot(const uint8_t *program, size_t count) {
    ZBench *b = zbench_open();
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->mem[i] = 0x00;
        b->known[i] = 1;
    }
    zload(b, 0x0000, program, count);
    zreset(b);
    return b;
}

static int zstatus_is(const ZBench *b, CpuStatus status) {
    CpuState state;
    const SimComponentType *type = sim_instance_type(b->cpu);

    return type->cpu_state(sim_instance_state(b->cpu), &state) && state.status == status;
}

static void zstate(const ZBench *b, CpuState *state) {
    const SimComponentType *type = sim_instance_type(b->cpu);

    type->cpu_state(sim_instance_state(b->cpu), state);
}

static uint32_t zreg(const CpuState *state, const char *name) {
    size_t i;

    for (i = 0; i < state->register_count; i++) {
        if (strcmp(state->registers[i].name, name) == 0) {
            return state->registers[i].value;
        }
    }
    return 0xEEEE;
}

static long zfind(const ZBench *b, char type, int addr, size_t from) {
    size_t i;

    for (i = from; i < b->log_count; i++) {
        if (b->log[i].type == type && (addr < 0 || b->log[i].addr == addr)) {
            return (long)i;
        }
    }
    return -1;
}

/* T-states of log entry i: up to the next entry's T1. */
static size_t zlength(const ZBench *b, long i) {
    return i >= 0 && (size_t)i + 1 < b->log_count ? b->log[i + 1].t1 - b->log[i].t1 : 0;
}

static const SimTime *ztiming(const ZBench *b) {
    return ((const CpuGrade *)part_internal(b->part)->data)->t;
}

/* Grade-10 times by name (cpuz80.c order). */
enum { ZT_CC, ZT_WCH, ZT_WCL, ZT_DCRA, ZT_DCFMREQF, ZT_DCRMREQR, ZT_DCFMREQR, ZT_DCFRDF,
       ZT_DCRRDR, ZT_SDCR, ZT_SWAITCF, ZT_DCRM1F, ZT_DCRM1R, ZT_DCRRFSHF, ZT_DCRRFSHR, ZT_DCFRDR,
       ZT_DCRRDF, ZT_SDCF, ZT_DCRIORQF, ZT_DCFIORQR, ZT_DCFWRF, ZT_DCFWRR, ZT_DCRWRF,
       ZT_DCFHALT, ZT_WNMI, ZT_SBUSREQ, ZT_DCRBUSACKF, ZT_DCFBUSACKR, ZT_DCRDZ, ZT_DCRCTZ,
       ZT_DCRAZ, ZT_SRESET, ZT_SINT, ZT_DCFIORQF, ZT_DCFD };

/* The time of CLK rise index i (past or future, 1 MHz). */
static SimTime zrise(const ZBench *b, size_t i) {
    return b->rise_at[b->first] + (SimTime)(i - b->first) * NS(1000);
}

/* ---- 5.1 Z80 cases ------------------------------------------------------- */

/* 1. Power-up lost. */
static void zcase_power_up(void) {
    ZBench *b = zbench_open();

    zrun_rises(b, 5);
    expect(zaddress(b) == -1 && zpin(b, b->m1) == LOGIC_X && zpin(b, b->mreq) == LOGIC_X &&
               zpin(b, b->rd) == LOGIC_X && zstatus_is(b, CPU_LOST),
           "Z80 1: power-up outputs X, status LOST");
    zbench_close(b);
}

/* 2. Reset: the first MC is a fetch at 0000. */
static void zcase_reset(void) {
    ZBench *b = zboot(NULL, 0);

    zrun_rises(b, 4);
    expect(b->log_count > 0 && b->log[0].type == 'F' && b->log[0].addr == 0x0000 &&
               b->log[0].t1 == b->first && zstatus_is(b, CPU_RUNNING),
           "Z80 2: after reset the first MC is a fetch at 0000; RUNNING");
    zbench_close(b);
}

/* 3. A 2-clock reset is too short. */
static void zcase_short_reset(void) {
    ZBench *b = zbench_open();

    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 2);
    zset(b, b->reset, LOGIC_1);
    zrun_rises(b, 3);
    expect(zstatus_is(b, CPU_LOST), "Z80 3: RESET# low for 2 clocks: LOST");
    zbench_close(b);
}

/* 4. The first fetch's waveform. */
static void zcase_fetch(void) {
    ZBench *b = zbench_open();
    const SimTime *t = ztiming(b);
    SimTime r, f;
    int a_before, a_after;
    LogicValue m1_before, m1_after, mreq_t1, rd_t1, rfsh_t3, rd_t3, mreq_t3, m1_t3;
    LogicValue mreq_rf, mreq_t4;
    int a_t3;
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->known[i] = 1;
    }
    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 4);
    zset(b, b->reset, LOGIC_1);
    b->first = b->rises;
    r = b->next_rise;          /* T1 rise */
    f = r + NS(500);           /* T1 fall */
    zrun_to(b, r + t[ZT_DCRA] - NS(1));
    a_before = zaddress(b);
    m1_before = zpin(b, b->m1);
    zrun_to(b, r + t[ZT_DCRA] + NS(1));
    a_after = zaddress(b);
    m1_after = zpin(b, b->m1);
    zrun_to(b, f + t[ZT_DCFRDF] + NS(1));
    mreq_t1 = zpin(b, b->mreq);
    rd_t1 = zpin(b, b->rd);
    zrun_to(b, r + NS(2000) + t[ZT_DCRRFSHF] + NS(1));   /* T3 rise + the longest delay */
    rfsh_t3 = zpin(b, b->rfsh);
    rd_t3 = zpin(b, b->rd);
    mreq_t3 = zpin(b, b->mreq);
    m1_t3 = zpin(b, b->m1);
    a_t3 = zaddress(b);
    zrun_to(b, r + NS(2500) + t[ZT_DCFMREQF] + NS(1));   /* T3 fall */
    mreq_rf = zpin(b, b->mreq);
    zrun_to(b, r + NS(3500) + t[ZT_DCFMREQR] + NS(1));   /* T4 fall */
    mreq_t4 = zpin(b, b->mreq);
    expect(a_before == -1 && a_after == 0x0000, "Z80 4: A valid at T1 rise + tDCRA");
    expect(m1_after == LOGIC_0 && (m1_before == LOGIC_1 || r + t[ZT_DCRM1F] < r + t[ZT_DCRA] - NS(1)),
           "Z80 4: M1# low at T1 rise + tDCRM1F");
    expect(mreq_t1 == LOGIC_0 && rd_t1 == LOGIC_0 && b->log_count > 0 &&
               b->log[0].start == f + t[ZT_DCFRDF],
           "Z80 4: MREQ# and RD# low after T1 fall (RD# at + tDCFRDF)");
    expect(rd_t3 == LOGIC_1 && mreq_t3 == LOGIC_1 && m1_t3 == LOGIC_1 && rfsh_t3 == LOGIC_0 &&
               a_t3 == 0x0000 && b->log[0].end == r + NS(2000) + t[ZT_DCRRDR],
           "Z80 4: T3 rise: RD#, MREQ#, M1# high, A = I:R = 0000, RFSH# low");
    expect(mreq_rf == LOGIC_0 && mreq_t4 == LOGIC_1, "Z80 4: refresh MREQ# low at T3 fall, high at T4 fall");
    zbench_close(b);
}

/* 5. Memory read and write. */
static void zcase_memory(void) {
    static const uint8_t PROGRAM[] = {0x3A, 0x00, 0x80, 0x32, 0x01, 0x80};
    ZBench *b = zbench_open();
    const SimTime *t = ztiming(b);
    CpuState state;
    long r, w;
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->known[i] = 1;
    }
    zload(b, 0x0000, PROGRAM, sizeof PROGRAM);
    zload(b, 0x8000, (const uint8_t[]){0xAB}, 1);
    zreset(b);
    zrun_rises(b, 30);
    zstate(b, &state);
    r = zfind(b, 'R', 0x8000, 0);
    w = zfind(b, 'W', 0x8001, 0);
    expect(r >= 0 && zlength(b, r) == 3 && b->log[r].start == zrise(b, b->log[r].t1) + NS(500) + t[ZT_DCFRDF] &&
               b->log[r].end == zrise(b, b->log[r].t1 + 2) + NS(500) + t[ZT_DCFRDR] &&
               (zreg(&state, "AF") >> 8) == 0xAB,
           "Z80 5: LD A,(8000h): a 3-T read, RD# from T1 fall to T3 fall, A = AB");
    expect(w >= 0 && zlength(b, w) == 3 &&
               b->log[w].data_at == zrise(b, b->log[w].t1) + NS(500) + t[ZT_DCFD] &&
               b->log[w].start == zrise(b, b->log[w].t1 + 1) + NS(500) + t[ZT_DCFWRF] &&
               b->log[w].end == zrise(b, b->log[w].t1 + 2) + NS(500) + t[ZT_DCFWRR] &&
               b->known[0x8001] && b->mem[0x8001] == 0xAB,
           "Z80 5: LD (8001h),A: D valid at T1 fall + tDCFD, WR# T2 fall to T3 fall, stored AB");
    zbench_close(b);
}

/* 6. I/O write and read. */
static void zcase_io(void) {
    static const uint8_t PROGRAM[] = {0x3E, 0x12, 0xD3, 0x10, 0xDB, 0x20};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    const SimTime *t = ztiming(b);
    CpuState state;
    long o, in;

    b->io_in = 0x5A;
    zrun_rises(b, 30);
    zstate(b, &state);
    o = zfind(b, 'O', -1, 0);
    in = zfind(b, 'I', -1, 0);
    expect(o >= 0 && b->log[o].addr == 0x1210 && b->log[o].data == 0x12 && zlength(b, o) == 4 &&
               b->log[o].start == zrise(b, b->log[o].t1 + 1) + t[ZT_DCRWRF],
           "Z80 6: OUT (10h),A: a 4-T write of 12 to port 1210, WR# low from T2 rise");
    expect(in >= 0 && b->log[in].addr == 0x1220 && zlength(b, in) == 4 &&
               b->log[in].start == zrise(b, b->log[in].t1 + 1) + t[ZT_DCRRDF] &&
               (zreg(&state, "AF") >> 8) == 0x5A,
           "Z80 6: IN A,(20h): a 4-T read of port 1220, A = 5A");
    zbench_close(b);
}

/* 7. WAIT on a memory read; WAIT X at T2 fall. */
static void zcase_wait(void) {
    static const uint8_t PROGRAM[] = {0x3A, 0x00, 0x80};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    long r;

    zload(b, 0x8000, (const uint8_t[]){0xAB}, 1);
    /* MCs: fetch T 0-3, read 0001 T 4-6, read 0002 T 7-9, read 8000 from T 10. */
    zrun_to(b, zrise(b, b->first + 11) + NS(200));
    zset(b, b->wait, LOGIC_0);
    zrun_to(b, zrise(b, b->first + 13) + NS(200));
    zset(b, b->wait, LOGIC_1);
    zrun_rises(b, 8);
    r = zfind(b, 'R', 0x8000, 0);
    expect(r >= 0 && b->log[r].t1 == b->first + 10 && zlength(b, r) == 5 &&
               b->log[r].end == zrise(b, b->log[r].t1 + 4) + NS(500) + ztiming(b)[ZT_DCFRDR],
           "Z80 7: WAIT# low across T2 fall for 2 clocks: a 5-T read, RD# low through Tw");
    zbench_close(b);

    b = zboot(PROGRAM, sizeof PROGRAM);
    zrun_to(b, zrise(b, b->first + 11) + NS(200));
    zset(b, b->wait, LOGIC_X);
    zrun_to(b, zrise(b, b->first + 12) + NS(200));
    expect(zstatus_is(b, CPU_LOST), "Z80 7: WAIT# X at the T2 fall that checks it: LOST");
    zbench_close(b);
}

/* 8. The opcode byte changes just inside / outside tSDCR before T3 rise. */
static void zcase_setup(void) {
    int pass;

    for (pass = 0; pass < 2; pass++) {
        ZBench *b = zboot(NULL, 0);
        SimTime t3 = zrise(b, b->first + 4 + 2);   /* the second fetch (0001): T3 rise */
        SimTime tsdcr = ztiming(b)[ZT_SDCR];
        SimTime poke = pass == 0 ? t3 - tsdcr + NS(1) : t3 - tsdcr - NS(1);

        zrun_to(b, t3 - NS(300));
        b->poke_value = 0x76;
        sim_schedule_timer(b->sim, b->tb, ZT_POKE, poke - sim_now(b->sim));
        zrun_rises(b, 8);
        if (pass == 0) {
            expect(zstatus_is(b, CPU_LOST), "Z80 8: opcode change tSDCR - 1 ns before T3 rise: LOST");
        } else {
            expect(zstatus_is(b, CPU_HALTED) && zpin(b, b->halt) == LOGIC_0,
                   "Z80 8: opcode change tSDCR + 1 ns before T3 rise: HALT executes");
        }
        zbench_close(b);
    }
}

/* 9. IM 1: an int-ack of 6 T, two pushes, the fetch at 0038. */
static void zcase_im1(void) {
    static const uint8_t PROGRAM[] = {0xED, 0x56, 0xFB};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    const SimTime *t = ztiming(b);
    long a;

    zrun_rises(b, 12);
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 30);
    a = zfind(b, 'A', -1, 0);
    expect(a >= 0 && b->log[a].m1_fall == zrise(b, b->log[a].t1) + t[ZT_DCRM1F] &&
               b->log[a].start == zrise(b, b->log[a].t1 + 2) + NS(500) + t[ZT_DCFIORQF] &&
               b->log[a].end == zrise(b, b->log[a].t1 + 4) + t[ZT_DCRM1R],
           "Z80 9: int-ack: M1# low from T1 rise, IORQ# from TW*1 fall, both high at T3 rise (6 T)");
    expect(a >= 0 && b->log[a].addr >= 0x0003 && b->log[a].addr <= 0x0010,
           "Z80 9: the int-ack puts PC on the address bus");
    expect(a >= 0 && (size_t)a + 3 < b->log_count && b->log[a + 1].type == 'W' &&
               b->log[a + 1].t1 == b->log[a].t1 + 7 && b->log[a + 2].type == 'W' &&
               b->log[a + 3].type == 'F' && b->log[a + 3].addr == 0x0038 &&
               b->log[a + 1].data == (b->log[a].addr >> 8) && b->log[a + 2].data == (b->log[a].addr & 0xFF),
           "Z80 9: then PC pushed (two writes) and a fetch at 0038");
    zbench_close(b);
}

/* 10. IM 2 (I = 12, vector 34) and IM 0 (FF on the bus: RST 38h). */
static void zcase_im2_im0(void) {
    static const uint8_t IM2[] = {0x3E, 0x12, 0xED, 0x47, 0xED, 0x5E, 0xFB};
    static const uint8_t IM0[] = {0xED, 0x46, 0xFB};
    ZBench *b = zboot(IM2, sizeof IM2);
    long a;

    zload(b, 0x1234, (const uint8_t[]){0x00, 0x05}, 2);
    zload(b, 0x0500, (const uint8_t[]){0x00, 0x00, 0x00}, 3);
    b->vector = 0x34;
    zrun_rises(b, 30);
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 30);
    a = zfind(b, 'A', -1, 0);
    expect(a >= 0 && (size_t)a + 5 < b->log_count && b->log[a + 3].type == 'R' &&
               b->log[a + 3].addr == 0x1234 && b->log[a + 4].addr == 0x1235 &&
               b->log[a + 5].type == 'F' && b->log[a + 5].addr == 0x0500,
           "Z80 10: IM 2: reads at 1234/1235, then a fetch at 0500");
    zbench_close(b);

    b = zboot(IM0, sizeof IM0);
    b->vector = 0xFF;
    zrun_rises(b, 20);
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 30);
    a = zfind(b, 'A', -1, 0);
    expect(a >= 0 && (size_t)a + 3 < b->log_count && b->log[a + 1].type == 'W' &&
               b->log[a + 2].type == 'W' && b->log[a + 3].type == 'F' && b->log[a + 3].addr == 0x0038,
           "Z80 10: IM 0 with FF on the bus: RST 38h (two pushes, a fetch at 0038)");
    zbench_close(b);
}

/* 11. NMI; a pulse shorter than tWNMI. */
static void zcase_nmi(void) {
    ZBench *b = zboot(NULL, 0);
    long f;

    zrun_rises(b, 10);
    zset(b, b->nmi, LOGIC_0);
    zrun_rises(b, 30);
    f = zfind(b, 'F', 0x0066, 0);
    expect(f >= 3 && b->log[f - 1].type == 'W' && b->log[f - 2].type == 'W' &&
               b->log[f - 3].type == 'F' && zlength(b, f - 3) == 5,
           "Z80 11: NMI# 1->0: a 5-T fetch, two pushes, a fetch at 0066");
    expect(zfind(b, 'F', 0x0066, (size_t)f + 1) == -1, "Z80 11: NMI# held low does not re-trigger");
    zbench_close(b);

    b = zboot(NULL, 0);
    zrun_to(b, zrise(b, b->first + 10) + NS(300));
    zset(b, b->nmi, LOGIC_0);
    zrun_to(b, sim_now(b->sim) + NS(30));
    zset(b, b->nmi, LOGIC_1);
    zrun_rises(b, 3);
    expect(zstatus_is(b, CPU_LOST), "Z80 11: an NMI# pulse shorter than tWNMI: LOST");
    zbench_close(b);
}

/* 12. HALT: HALT# low at T4 fall + tDCFHALT; refresh goes on; INT exits. */
static void zcase_halt(void) {
    static const uint8_t PROGRAM[] = {0xED, 0x56, 0xFB, 0x76};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    CpuState before, after;
    long h, a;

    zrun_rises(b, 20);
    h = zfind(b, 'F', 0x0003, 0);
    zstate(b, &before);
    zrun_rises(b, 20);
    zstate(b, &after);
    expect(h >= 0 && b->halt_fell == zrise(b, b->log[h].t1 + 3) + NS(500) + ztiming(b)[ZT_DCFHALT] &&
               after.status == CPU_HALTED,
           "Z80 12: HALT# low at the HALT fetch's T4 fall + tDCFHALT");
    expect((zreg(&after, "R") & 0x7F) != (zreg(&before, "R") & 0x7F) &&
               zfind(b, 'F', 0x0003, (size_t)h + 1) >= 0,
           "Z80 12: the halted M1 cycles keep refreshing (R advances)");
    zset(b, b->intr, LOGIC_0);
    zrun_rises(b, 12);
    a = zfind(b, 'A', -1, 0);
    expect(zpin(b, b->halt) == LOGIC_1 && a >= 0, "Z80 12: INT# with EI exits: HALT# high, an int-ack");
    zbench_close(b);
}

static int zfloats(const ZBench *b) {
    int ok = zpin(b, b->mreq) == LOGIC_Z && zpin(b, b->iorq) == LOGIC_Z &&
             zpin(b, b->rd) == LOGIC_Z && zpin(b, b->wr) == LOGIC_Z;
    size_t i;

    for (i = 0; i < 16; i++) {
        ok = ok && zpin(b, b->a[i]) == LOGIC_Z;
    }
    for (i = 0; i < 8; i++) {
        ok = ok && zpin(b, b->d[i]) == LOGIC_Z;
    }
    return ok;
}

/* 13. BUSREQ. */
static void zcase_busreq(void) {
    ZBench *b = zboot(NULL, 0);
    size_t count;
    int floated, released, idle;

    zrun_rises(b, 9);
    zset(b, b->busreq, LOGIC_0);
    zrun_rises(b, 6);
    floated = zfloats(b);
    released = zpin(b, b->busack) == LOGIC_0 && zstatus_is(b, CPU_BUS_RELEASED);
    count = b->log_count;
    zrun_rises(b, 5);
    idle = b->log_count == count;
    expect(floated && released, "Z80 13: BUSREQ# low: BUSACK# low, bus Z, status BUS_RELEASED");
    expect(idle, "Z80 13: no MC while the bus is released");
    zset(b, b->busreq, LOGIC_1);
    zrun_rises(b, 1);
    zrun_to(b, sim_now(b->sim) + NS(400));   /* past the next fall */
    expect(zpin(b, b->busack) == LOGIC_1, "Z80 13: released: BUSACK# high at the next fall");
    zrun_rises(b, 5);
    expect(b->log_count > count && zstatus_is(b, CPU_RUNNING), "Z80 13: then the pending MC runs");
    zbench_close(b);
}

/* 14. A high phase shorter than tWCH. */
static void zcase_clock(void) {
    ZBench *b = zboot(NULL, 0);

    zrun_rises(b, 4);
    b->high_once = ztiming(b)[ZT_WCH] - NS(5);
    zrun_rises(b, 3);
    expect(zstatus_is(b, CPU_LOST), "Z80 14: a high phase below tWCH: LOST");
    zbench_close(b);
}

/* 15. The program of the sub-plan's outcome. */
static void zcase_program(void) {
    static const uint8_t PROGRAM[] = {0x3E, 0x12, 0x01, 0x56, 0x34, 0x32, 0x00, 0x80,
                                      0xD3, 0x10, 0x76};
    static const size_t LENGTHS[] = {4, 3, 4, 3, 3, 4, 3, 3, 3, 4, 3, 4, 4};
    ZBench *b = zboot(PROGRAM, sizeof PROGRAM);
    CpuState before, state;
    long w, o;
    size_t i;
    int ok = 1;

    while (zfind(b, 'F', 0x000A, 0) < 0) {
        zrun_rises(b, 1);
    }
    zstate(b, &before);
    zrun_rises(b, 12);
    zstate(b, &state);
    for (i = 0; i < sizeof LENGTHS / sizeof LENGTHS[0]; i++) {
        ok = ok && zlength(b, (long)i) == LENGTHS[i];
    }
    w = zfind(b, 'W', 0x8000, 0);
    o = zfind(b, 'O', 0x1210, 0);
    expect(ok, "Z80 15: MCs of 4,3 / 4,3,3 / 4,3,3,3 / 4,3,4 / 4 T-states");
    expect(w >= 0 && b->log[w].data == 0x12 && o >= 0 && b->log[o].data == 0x12,
           "Z80 15: the write of 12 to 8000 and the I/O write of 12 to port 1210");
    expect(zreg(&state, "BC") == 0x3456 && state.status == CPU_HALTED && zpin(b, b->halt) == LOGIC_0 &&
               strcmp(state.instruction_text, "HALT") == 0,
           "Z80 15: BC = 3456, status HALTED, last instruction HALT");
    expect(before.has_instruction && strcmp(before.instruction_text, "OUT (10h),A") == 0 &&
               before.instruction_address == 0x0008,
           "Z80 15: the instruction before it reads back as OUT (10h),A");
    zbench_close(b);
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
    zcase_power_up();
    zcase_reset();
    zcase_short_reset();
    zcase_fetch();
    zcase_memory();
    zcase_io();
    zcase_wait();
    zcase_setup();
    zcase_im1();
    zcase_im2_im0();
    zcase_nmi();
    zcase_halt();
    zcase_busreq();
    zcase_clock();
    zcase_program();
    case_circuit_api();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
