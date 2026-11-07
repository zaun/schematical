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
 *
 * The last cases (plan-09-D section 5) run whole boards of library parts:
 * the CPU parts of cpu.json with an AT28C256 EEPROM, an AS6C62256 SRAM and
 * an SN74HC00N decoder (tests/support/cpu_board.c).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "cpu_board.h"
#include "io_board.h"
#include "cpu.h"
#include "cpu_state.h"
#include "kind.h"
#include "../src/third_party/jsmooch/wdc65816.h"
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

/* 3b. RES released inside tPCS before a fall (an asynchronous reset switch):
   the release takes effect one cycle later; the CPU starts normally. */
static void case_reset_setup(const char *name) {
    Bench *b = bench_open(name);
    const SimTime *t = ((const CpuGrade *)part_internal(b->part)->data)->t;
    size_t i;
    int ok = 1;

    load_byte(b, 0xFFFC, 0x00);
    load_byte(b, 0xFFFD, 0xC0);
    for (i = 0; i < 0x100; i++) {
        load_byte(b, (uint16_t)(0xC000 + i), 0xEA);
    }
    set_pin(b, b->res, LOGIC_0);
    run_cycles(b, 3);
    run_to(b, b->next_fall - t[9] + NS(1));   /* t[9] = tpcs */
    set_pin(b, b->res, LOGIC_1);
    b->release = b->next_fall + NS(1000);   /* one cycle later than in time */
    run_cycles(b, 2);
    b->reset_log = b->log_count;
    run_to_cycle(b, 6);
    for (i = 0; i < 7; i++) {
        ok = ok && cycle(b, i)->rw == '1';
    }
    expect(ok && cycle(b, 4)->addr == 0xFFFC && cycle(b, 6)->addr == 0xC000 &&
               status_is(b, CPU_RUNNING),
           label(name, "3: RES released inside tPCS: reset ends one cycle later, CPU runs"));
    bench_close(b);
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

/* 3b. RESET# released inside tSRESET before a rise: the first fetch starts
   one T-state later; the CPU runs. */
static void zcase_reset_setup(void) {
    ZBench *b = zbench_open();
    size_t i;

    for (i = 0; i < 0x100; i++) {
        b->known[i] = 1;
    }
    zset(b, b->reset, LOGIC_0);
    zrun_rises(b, 4);
    zrun_to(b, b->next_rise - ztiming(b)[ZT_SRESET] + NS(1));
    zset(b, b->reset, LOGIC_1);
    b->first = b->rises + 1;   /* the rise after next is T1 */
    zrun_rises(b, 8);
    expect(b->log_count > 0 && b->log[0].type == 'F' && b->log[0].addr == 0x0000 &&
               b->log[0].t1 == b->first && zstatus_is(b, CPU_RUNNING),
           "Z80 3: RESET# released inside tSRESET: the first fetch one T later, CPU runs");
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

static uint32_t reg_value(const CpuState *state, const char *name) {
    size_t i;

    for (i = 0; i < state->register_count; i++) {
        if (strcmp(state->registers[i].name, name) == 0) {
            return state->registers[i].value;
        }
    }
    return 0xEEEE;
}

/* ---- Whole-system boards (plan-09-D section 5) ------------------------------- */

static int sram_is(const CpuBoard *board, uint32_t address, uint8_t expected) {
    uint8_t value;

    return cpu_board_sram(board, address, &value) && value == expected;
}

/* The 6502 boards: count 1..16 in zero page and at 0200, then STP (W65C02S)
   or JMP $8012 (MPS6502). */
static void board_6502(const char *name, int wdc) {
    static const uint8_t COMMON[] = {0xA2, 0xFF, 0x9A, 0xA9, 0x00, 0x85, 0x10, 0xE6, 0x10, 0xA5,
                                     0x10, 0x8D, 0x00, 0x02, 0xC9, 0x10, 0xD0, 0xF5};
    uint8_t image[sizeof COMMON + 3];
    size_t count = sizeof COMMON;
    CpuBoard *board;
    CpuState state;
    char what[160];
    uint16_t pc;

    memcpy(image, COMMON, sizeof COMMON);
    if (wdc) {
        image[count++] = 0xDB;
    } else {
        image[count++] = 0x4C;
        image[count++] = 0x12;
        image[count++] = 0x80;
    }
    board = cpu_board_create(library, name, image, count, 0x8000);
    if (board == NULL || !cpu_board_run(board, NS(2000000))) {
        expect(0, "board did not run");
        cpu_board_destroy(board);
        return;
    }
    cpu_board_state(board, &state);
    pc = (uint16_t)reg_value(&state, "PC");
    snprintf(what, sizeof what, "%s board: SRAM 0010 = 10 and 0200 = 10", name);
    expect(sram_is(board, 0x0010, 0x10) && sram_is(board, 0x0200, 0x10), what);
    snprintf(what, sizeof what, "%s board: A = 10, X = FF, S = FF", name);
    expect(state.register_count > 0 && reg_value(&state, "A") == 0x10 &&
               reg_value(&state, "X") == 0xFF && reg_value(&state, "S") == 0xFF,
           what);
    /* The core's PC is the next byte to read: 8012-8015 in the JMP loop. */
    snprintf(what, sizeof what, "%s board: %s", name,
             wdc ? "status STOPPED, last instruction STP"
                 : "RUNNING with PC in 8012-8015, last instruction JMP $8012");
    expect(wdc ? state.status == CPU_STOPPED && strcmp(state.instruction_text, "STP") == 0
               : state.status == CPU_RUNNING && pc >= 0x8012 && pc <= 0x8015 &&
                     strcmp(state.instruction_text, "JMP $8012") == 0 &&
                     state.instruction_address == 0x8012,
           what);
    cpu_board_destroy(board);
}

/* ---- plan-14-H: the W65C816S adapter (core "65816") ----------------------- */

/*
 * A 24-bit bench: PHI2 1 MHz (500 ns high, 500 ns low); the bench latches
 * D0-D7 at each PHI2 rise as the bank (Figure 5-1's latch, closed at the
 * rise) and serves bank:address from rise + 100 ns to fall + 20 ns; a write
 * is stored 1 ps before the fall. Each cycle's log entry also holds probes
 * of D0-D7, E and MX at the datasheet's edges.
 */

#define W65C816S "W65C816S6TPG-14"   /* the shipped part (plan-14-I) */

enum { B8_RISE = 100, B8_FALL, B8_SERVE, B8_SAMPLE, B8_RELEASE, B8_LOWEND,
       B8_F4, B8_F5, B8_F9, B8_F10, B8_F32, B8_F33, B8_R4, B8_R5, B8_R9, B8_R10, B8_R29, B8_R30 };

typedef struct {
    int addr;                   /* A0-A15, -1 when not all 0/1 */
    int bank;                   /* latched at the rise, -1 X */
    char rw, vda, vpa, vpb, mlb, e, rdy;
    char mx_high, mx_low;       /* MX 1 ps before the fall / before the rise */
    int data;                   /* D0-D7 1 ps before the fall (-1 X, -2 Z) */
    int f9, f10, f32, f33;      /* D0-D7 at fall + 9, 10, 32, 33 ns */
    int r9, r10, r29, r30;      /* D0-D7 at rise + 9, 10, 29, 30 ns */
    char e4, e5, mxf4, mxf5, mxr4, mxr5;
} Cycle816;

enum { LOG816 = 4096 };

static uint8_t mem816[1 << 24];
static uint8_t known816[1 << 24];

typedef struct {
    Sim *sim;
    SimInstance *cpu, *tb;
    const Part *part;
    size_t a[16], d[8];
    size_t clk, rw, irq, nmi, res, rdy, abortb, vda, vpa, vpb, mlb, e, mx, be;
    SimTime next_fall;
    int bank;
    int fill;                   /* >= 0: served when the address is not known (BE low) */
    int serving;
    Cycle816 cur;
    Cycle816 log[LOG816];
    size_t log_count;
    size_t reset_log;
} Bench816;

static Bench816 *b8_current;

static char b8_char(const Bench816 *b, size_t pin) {
    return "01XZ"[sim_read_pin(b->sim, b->tb, pin)];
}

static int b8_addr(const Bench816 *b) {
    int value = 0;
    int i;

    for (i = 0; i < 16; i++) {
        LogicValue v = sim_read_pin(b->sim, b->tb, b->a[i]);

        if (v != LOGIC_0 && v != LOGIC_1) return -1;
        value |= (v == LOGIC_1) << i;
    }
    return value;
}

static int b8_data(const Bench816 *b) {
    int value = 0;
    int z = 0;
    int bad = 0;
    int i;

    for (i = 0; i < 8; i++) {
        LogicValue v = sim_read_pin(b->sim, b->tb, b->d[i]);

        z += v == LOGIC_Z;
        if (v == LOGIC_1) value |= 1 << i;
        else if (v != LOGIC_0) bad = 1;
    }
    return z == 8 ? -2 : bad ? -1 : value;
}

static void b8_drive(Bench816 *b, size_t pin, LogicValue v) {
    sim_schedule_drive(b->sim, b->tb, pin, v, DRIVE_STRONG, 0);
}

static size_t b8_pins(const void *config) {
    return ((const Bench816 *)config)->part->pin_count;
}

static void *b8_create(const void *config) {
    return (void *)config;
}

static void b8_destroy(void *state) {
    (void)state;
}

static void b8_start(Sim *sim, SimInstance *instance, void *state) {
    Bench816 *b = state;

    (void)instance;
    b8_drive(b, b->clk, LOGIC_0);
    sim_schedule_timer(sim, b->tb, B8_LOWEND, NS(500) - 1);
    sim_schedule_timer(sim, b->tb, B8_RISE, NS(500));
    b->next_fall = NS(1000);
}

static void b8_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    Bench816 *b = state;
    Cycle816 *c = &b->cur;
    int i;

    (void)instance;
    switch (id) {
        case B8_RISE:
            b8_drive(b, b->clk, LOGIC_1);
            /* The bank latch keeps its value when D0-D7 hold no bank (a
               cycle RDY holds: the board gates the latch with RDY). */
            if (b8_data(b) >= 0) b->bank = b8_data(b);
            c->bank = b->bank;
            sim_schedule_timer(sim, b->tb, B8_R4, NS(4));
            sim_schedule_timer(sim, b->tb, B8_R5, NS(5));
            sim_schedule_timer(sim, b->tb, B8_R9, NS(9));
            sim_schedule_timer(sim, b->tb, B8_R10, NS(10));
            sim_schedule_timer(sim, b->tb, B8_R29, NS(29));
            sim_schedule_timer(sim, b->tb, B8_R30, NS(30));
            sim_schedule_timer(sim, b->tb, B8_SERVE, NS(100));
            sim_schedule_timer(sim, b->tb, B8_SAMPLE, NS(500) - 1);
            sim_schedule_timer(sim, b->tb, B8_FALL, NS(500));
            b->next_fall = sim_now(sim) + NS(500);
            break;
        case B8_FALL:
            b8_drive(b, b->clk, LOGIC_0);
            sim_schedule_timer(sim, b->tb, B8_F4, NS(4));
            sim_schedule_timer(sim, b->tb, B8_F5, NS(5));
            sim_schedule_timer(sim, b->tb, B8_F9, NS(9));
            sim_schedule_timer(sim, b->tb, B8_F10, NS(10));
            sim_schedule_timer(sim, b->tb, B8_RELEASE, NS(20));
            sim_schedule_timer(sim, b->tb, B8_F32, NS(32));
            sim_schedule_timer(sim, b->tb, B8_F33, NS(33));
            sim_schedule_timer(sim, b->tb, B8_LOWEND, NS(500) - 1);
            sim_schedule_timer(sim, b->tb, B8_RISE, NS(500));
            b->next_fall = sim_now(sim) + NS(1000);
            break;
        case B8_SERVE:
            if (b8_char(b, b->rw) != '0') {
                int addr = b8_addr(b);
                int known = addr >= 0 && b->bank >= 0 && b8_char(b, b->rw) == '1';
                uint32_t full = known ? ((uint32_t)b->bank << 16) | (uint32_t)addr : 0;
                int value = known && known816[full] ? mem816[full] : addr < 0 ? b->fill : -1;

                for (i = 0; i < 8; i++) {
                    b8_drive(b, b->d[i], value >= 0 ? ((value >> i) & 1 ? LOGIC_1 : LOGIC_0) : LOGIC_X);
                }
                b->serving = 1;
            }
            break;
        case B8_RELEASE:
            if (b->serving) {
                for (i = 0; i < 8; i++) b8_drive(b, b->d[i], LOGIC_Z);
                b->serving = 0;
            }
            break;
        case B8_LOWEND:
            c->mx_low = b8_char(b, b->mx);
            break;
        case B8_F4: c->e4 = b8_char(b, b->e); c->mxf4 = b8_char(b, b->mx); break;
        case B8_F5: c->e5 = b8_char(b, b->e); c->mxf5 = b8_char(b, b->mx); break;
        case B8_F9: c->f9 = b8_data(b); break;
        case B8_F10: c->f10 = b8_data(b); break;
        case B8_F32: c->f32 = b8_data(b); break;
        case B8_F33: c->f33 = b8_data(b); break;
        case B8_R4: c->mxr4 = b8_char(b, b->mx); break;
        case B8_R5: c->mxr5 = b8_char(b, b->mx); break;
        case B8_R9: c->r9 = b8_data(b); break;
        case B8_R10: c->r10 = b8_data(b); break;
        case B8_R29: c->r29 = b8_data(b); break;
        case B8_R30: c->r30 = b8_data(b); break;
        case B8_SAMPLE:
            c->addr = b8_addr(b);
            c->rw = b8_char(b, b->rw);
            c->vda = b8_char(b, b->vda);
            c->vpa = b8_char(b, b->vpa);
            c->vpb = b8_char(b, b->vpb);
            c->mlb = b8_char(b, b->mlb);
            c->e = b8_char(b, b->e);
            c->rdy = b8_char(b, b->rdy);
            c->mx_high = b8_char(b, b->mx);
            c->data = b8_data(b);
            if (c->rw == '0' && c->addr >= 0 && c->bank >= 0) {
                uint32_t full = ((uint32_t)c->bank << 16) | (uint32_t)c->addr;

                mem816[full] = (uint8_t)(c->data >= 0 ? c->data : 0);
                known816[full] = c->data >= 0;
            }
            if (b->log_count < LOG816) b->log[b->log_count++] = *c;
            memset(c, 0, sizeof *c);
            break;
    }
}

static const SimComponentType BENCH816_TYPE = {
    "bench816", b8_pins, b8_create, b8_destroy, b8_start, NULL, b8_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void b8_run_to(Bench816 *b, SimTime t) {
    if (!sim_run_until(b->sim, t)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* n more PHI2 falls, stopping 200 ns after the last. */
static void b8_cycles(Bench816 *b, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) b8_run_to(b, b->next_fall + NS(200));
}

static void b8_load(uint32_t at, const uint8_t *bytes, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        mem816[(at + i) & 0xFFFFFF] = bytes[i];
        known816[(at + i) & 0xFFFFFF] = 1;
    }
}

static void b8_byte(uint32_t at, uint8_t value) {
    b8_load(at, &value, 1);
}

static Bench816 *b8_open(void) {
    Bench816 *b = calloc(1, sizeof(*b));
    const Part *part = part_library_find(library, W65C816S);
    size_t i;
    char pin[8];

    if (b == NULL || part == NULL) {
        fprintf(stderr, "no part %s\n", W65C816S);
        exit(2);
    }
    memset(mem816, 0, sizeof mem816);
    memset(known816, 0, sizeof known816);
    b->part = part;
    for (i = 0; i < 16; i++) {
        snprintf(pin, sizeof pin, "A%zu", i);
        b->a[i] = pin_named(part, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "D%zu", i);
        b->d[i] = pin_named(part, pin);
    }
    b->clk = pin_named(part, "PHI2");
    b->rw = pin_named(part, "RWB");
    b->irq = pin_named(part, "IRQB");
    b->nmi = pin_named(part, "NMIB");
    b->res = pin_named(part, "RESB");
    b->rdy = pin_named(part, "RDY");
    b->abortb = pin_named(part, "ABORTB");
    b->vda = pin_named(part, "VDA");
    b->vpa = pin_named(part, "VPA");
    b->vpb = pin_named(part, "VPB");
    b->mlb = pin_named(part, "MLB");
    b->e = pin_named(part, "E");
    b->mx = pin_named(part, "MX");
    b->be = pin_named(part, "BE");
    b->bank = -1;
    b->fill = -1;
    b->sim = sim_create();
    b->cpu = sim_add_instance(b->sim, kind_model(part), "U1", part);
    b->tb = sim_add_instance(b->sim, &BENCH816_TYPE, "TB", b);
    for (i = 0; i < part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, "n");

        sim_connect(b->sim, net, b->cpu, i);
        sim_connect(b->sim, net, b->tb, i);
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "sim_start: %s\n", sim_error(b->sim));
        exit(2);
    }
    b8_drive(b, b->irq, LOGIC_1);
    b8_drive(b, b->nmi, LOGIC_1);
    b8_drive(b, b->res, LOGIC_1);
    b8_drive(b, b->abortb, LOGIC_1);
    b8_drive(b, b->be, LOGIC_1);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    b8_current = b;
    return b;
}

static void b8_close(Bench816 *b) {
    sim_destroy(b->sim);
    free(b);
}

static int b8_state(const Bench816 *b, CpuState *out) {
    const SimComponentType *type = sim_instance_type(b->cpu);

    return type->cpu_state != NULL && type->cpu_state(sim_instance_state(b->cpu), out);
}

static uint32_t b8_reg(const CpuState *s, const char *name) {
    size_t i;

    for (i = 0; i < s->register_count; i++) {
        if (strcmp(s->registers[i].name, name) == 0) return s->registers[i].value;
    }
    return 0xEEEEEEEE;
}

/* RESB low for 3 cycles, then high; the log index of the first cycle after it. */
static void b8_reset(Bench816 *b) {
    b8_drive(b, b->res, LOGIC_0);
    b8_cycles(b, 3);
    b8_drive(b, b->res, LOGIC_1);
    b8_cycles(b, 1);
    b->reset_log = b->log_count;
}

/* Vectors: reset 8000; native ABORT A000, IRQ B000, NMI C000; emulation
   IRQ/BRK D000, NMI C000. Then the program at 00:8000 and a reset. */
static Bench816 *b8_boot(const uint8_t *program, size_t n) {
    Bench816 *b = b8_open();

    b8_byte(0xFFFC, 0x00); b8_byte(0xFFFD, 0x80);
    b8_byte(0xFFE8, 0x00); b8_byte(0xFFE9, 0xA0);
    b8_byte(0xFFEE, 0x00); b8_byte(0xFFEF, 0xB0);
    b8_byte(0xFFEA, 0x00); b8_byte(0xFFEB, 0xC0);
    b8_byte(0xFFFE, 0x00); b8_byte(0xFFFF, 0xD0);
    b8_byte(0xFFFA, 0x00); b8_byte(0xFFFB, 0xC0);
    b8_byte(0xA000, 0xDB);   /* ABORT handler: STP */
    b8_byte(0xB000, 0x40);   /* RTI */
    b8_byte(0xC000, 0x40);
    b8_byte(0xD000, 0x40);
    b8_load(0x8000, program, n);
    b8_reset(b);
    return b;
}

static const Cycle816 *c8(const Bench816 *b, size_t i) {
    static const Cycle816 none = {-1, -1, '?', '?', '?', '?', '?', '?', '?', '?', '?', -1};

    return b->reset_log + i < b->log_count ? &b->log[b->reset_log + i] : &none;
}

/* The first cycle at or after `from` (index after reset) with this 24-bit
   address, or SIZE_MAX. */
static size_t c8_find(const Bench816 *b, size_t from, uint32_t full) {
    size_t i;

    for (i = from; b->reset_log + i < b->log_count; i++) {
        const Cycle816 *c = c8(b, i);

        if (c->addr >= 0 && c->bank >= 0 &&
            (((uint32_t)c->bank << 16) | (uint32_t)c->addr) == full) {
            return i;
        }
    }
    return (size_t)-1;
}

/* The first opcode fetch (VDA = VPA = 1) at or after `from` at this address. */
static size_t c8_fetch(const Bench816 *b, size_t from, uint32_t full) {
    size_t i = from;

    while ((i = c8_find(b, i, full)) != (size_t)-1) {
        if (c8(b, i)->vda == '1' && c8(b, i)->vpa == '1') return i;
        i++;
    }
    return (size_t)-1;
}

/* The first write cycle at or after `from` at this address. */
static size_t c8_write(const Bench816 *b, size_t from, uint32_t full) {
    size_t i = from;

    while ((i = c8_find(b, i, full)) != (size_t)-1) {
        if (c8(b, i)->rw == '0') return i;
        i++;
    }
    return (size_t)-1;
}

static int h_failures;

/* On a failure: the log from cycle `from` (after reset), n cycles. */
static void b8_dump(const Bench816 *b, size_t from, size_t n) {
    size_t i;

    for (i = from; i < from + n && b->reset_log + i < b->log_count; i++) {
        const Cycle816 *c = c8(b, i);

        printf("  %3zu %02x:%04x rw%c vda%c vpa%c vpb%c mlb%c e%c(%c%c) mx%c/%c d%4d f33 %4d r30 %4d\n",
               i, (unsigned)(c->bank & 0xFF), (unsigned)(c->addr & 0xFFFF), c->rw, c->vda, c->vpa,
               c->vpb, c->mlb, c->e, c->e4, c->e5, c->mx_low, c->mx_high, c->data, c->f33, c->r30);
    }
}

static void h_check(int ok, const char *what) {
    if (!ok) {
        printf("FAIL: %s\n", what);
        failures++;
        h_failures++;
    }
}

static void h_begin(void) {
    h_failures = 0;
}

static void h_end(const char *name) {
    printf("%s w65c816s_%s\n", h_failures == 0 ? "ok" : "FAIL", name);
}

/* H1: LOST at power-up; reset; the vector read with VPB low; E = 1, MX high. */
static void h1_reset(void) {
    static const uint8_t PROGRAM[] = {0xEA, 0xEA, 0x80, 0xFC};
    Bench816 *b = b8_open();
    CpuState s;
    size_t i;
    size_t v;

    h_begin();
    b8_cycles(b, 2);
    h_check(b8_state(b, &s) && s.status == CPU_LOST, "H1 power-up: LOST");
    b8_byte(0xFFFC, 0x00);
    b8_byte(0xFFFD, 0x80);
    b8_load(0x8000, PROGRAM, sizeof PROGRAM);
    b8_reset(b);
    b8_cycles(b, 12);
    v = c8_find(b, 0, 0x00FFFC);
    h_check(v != (size_t)-1 && c8(b, v)->vpb == '0' && c8(b, v)->vda == '1' &&
                c8(b, v + 1)->addr == 0xFFFD && c8(b, v + 1)->vpb == '0' && c8(b, v + 1)->bank == 0,
            "H1 vector read at 00FFFC/D with VPB low");
    h_check(v != (size_t)-1 && c8(b, v + 2)->addr == 0x8000 && c8(b, v + 2)->vpa == '1' &&
                c8(b, v + 2)->vda == '1' && c8(b, v + 2)->vpb == '1',
            "H1 then the opcode fetch at 008000");
    for (i = 0; i < 6; i++) {
        h_check(c8(b, i)->e == '1' && c8(b, i)->mx_high == '1' && c8(b, i)->mx_low == '1',
                "H1 E = 1, MX high in both phases");
    }
    h_check(b8_state(b, &s) && s.status == CPU_RUNNING, "H1 RUNNING");
    b8_close(b);
    h_end("h1_reset");
}

/* H2: native LDA $123456 (16-bit A): the bank on D0-D7 in PHI2 low. */
static void h2_bank(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xC2, 0x20, 0xAF, 0x56, 0x34, 0x12, 0xDB};
    Bench816 *b;
    CpuState s;
    size_t i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x123456, 0x56);
    b8_byte(0x123457, 0x34);
    b8_cycles(b, 20);
    i = c8_find(b, 0, 0x123456);
    h_check(i != (size_t)-1, "H2 the data read at 123456");
    if (i != (size_t)-1) {
        const Cycle816 *c = c8(b, i);

        h_check(c->f32 == -1 && c->f33 == 0x12, "H2 bank 12 valid at fall + 33 ns (X at 32)");
        h_check(c->r9 == 0x12 && c->r10 == -2, "H2 bank held to rise + 10 ns, then Z");
        h_check(c->vda == '1' && c->vpa == '0' && c->rw == '1', "H2 VDA 1, VPA 0, read");
        h_check(c->data == 0x56 && c8(b, i + 1)->addr == 0x3457 && c8(b, i + 1)->bank == 0x12,
                "H2 then 123457");
    }
    h_check(b8_state(b, &s) && b8_reg(&s, "C") == 0x3456, "H2 C = 3456");
    b8_close(b);
    h_end("h2_bank");
}

/* H3: STA $7F0010: the bank in PHI2 low, then the data from rise + 30 ns,
   held to the next fall + 10 ns. */
static void h3_write(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xA9, 0xA5, 0x8F, 0x10, 0x00, 0x7F, 0xDB};
    Bench816 *b;
    size_t i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 20);
    i = c8_find(b, 0, 0x7F0010);
    h_check(i != (size_t)-1, "H3 the write at 7F0010");
    if (i != (size_t)-1) {
        const Cycle816 *c = c8(b, i);
        const Cycle816 *next = c8(b, i + 1);

        h_check(c->rw == '0' && c->vda == '1' && c->f33 == 0x7F && c->r9 == 0x7F,
                "H3 bank 7F in PHI2 low");
        h_check(c->r10 == -1 && c->r29 == -1 && c->r30 == 0xA5 && c->data == 0xA5,
                "H3 X at rise + 10 ns, data A5 at rise + 30 ns");
        h_check(next->f9 == 0xA5 && next->f10 == -1, "H3 held to the next fall + 10 ns");
        h_check(known816[0x7F0010] && mem816[0x7F0010] == 0xA5, "H3 stored");
    }
    b8_close(b);
    h_end("h3_write");
}

/* H4: per cycle, the bus equals the core's own pins (a bare core on the
   same memory). */
static void h4_flags(void) {
    static const uint8_t PROGRAM[] = {
        0x18, 0xFB,             /* CLC XCE */
        0xA9, 0x12,             /* LDA #$12 */
        0x8D, 0x00, 0x02,       /* STA $0200 */
        0xEE, 0x00, 0x02,       /* INC $0200 */
        0x6C, 0x10, 0x00,       /* JMP ($0010) -> 8020 */
    };
    static struct WDC65816 core;
    static u64 clock;
    Bench816 *b;
    size_t n = 40;
    size_t i;
    int same = 1;
    char message[160];

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x0010, 0x20);
    b8_byte(0x0011, 0x80);
    b8_byte(0x8020, 0xEA);
    b8_byte(0x8021, 0x80);
    b8_byte(0x8022, 0xFD);   /* BRA 8020 */
    b8_byte(0x0200, 0x00);
    b8_cycles(b, (unsigned)n + 2);
    /* The reference: the bare core from reset on the bench's memory image. */
    {
        static uint8_t ram[1 << 16];

        memcpy(ram, mem816, sizeof ram);
        ram[0x0200] = 0x00;
        WDC65816_init(&core, &clock);
        core.regs.E = 1;
        core.regs.P.M = core.regs.P.X = 1;
        WDC65816_reset(&core);
        WDC65816_cycle(&core);
        for (i = 0; i < n && same; i++) {
            const Cycle816 *c = c8(b, i);
            uint32_t addr = core.pins.Addr & 0xFFFF;
            char rw = core.pins.RW ? '0' : '1';

            if (c->addr != (int)addr || c->bank != core.pins.BA || c->rw != rw ||
                c->vda != (core.pins.VDA ? '1' : '0') || c->vpa != (core.pins.VPA ? '1' : '0') ||
                c->vpb != (core.pins.VPB ? '0' : '1')) {
                snprintf(message, sizeof message,
                         "H4 cycle %zu: bus %02x:%04x %c vda %c vpa %c vpb %c, core %02x:%04x %c %u %u %u",
                         i, (unsigned)c->bank, (unsigned)c->addr, c->rw, c->vda, c->vpa, c->vpb,
                         (unsigned)core.pins.BA, (unsigned)addr, rw, core.pins.VDA, core.pins.VPA,
                         core.pins.VPB);
                h_check(0, message);
                same = 0;
            }
            if (core.pins.RW) ram[addr] = (uint8_t)core.pins.D;
            else core.pins.D = ram[addr];
            WDC65816_cycle(&core);
        }
        WDC65816_delete(&core);
    }
    h_check(same, "H4 every cycle's VDA, VPA, VPB, RWB and address equal the core's");
    b8_close(b);
    h_end("h4_flags");
}

/* H5: CLC XCE REP #$10: native, M = 1, X = 0: MX high in PHI2 high, low in
   PHI2 low, changing 5 ns after each edge. */
static void h5_mx(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xC2, 0x10, 0xEA, 0xEA, 0xEA, 0xDB};
    Bench816 *b;
    size_t i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 16);
    i = c8_fetch(b, 0, 0x008005);   /* the second NOP's fetch: after REP */
    h_check(i != (size_t)-1, "H5 the NOP after REP");
    if (i != (size_t)-1) {
        const Cycle816 *c = c8(b, i);

        h_check(c->mx_high == '1' && c->mx_low == '0', "H5 MX: M = 1 in PHI2 high, X = 0 in PHI2 low");
        h_check(c->mxr4 == '0' && c->mxr5 == '1', "H5 MX turns to M at rise + 5 ns");
        h_check(c->mxf4 == '1' && c->mxf5 == '0', "H5 MX turns to X at fall + 5 ns");
    }
    b8_close(b);
    h_end("h5_mx");
}

/* H6: E falls 5 ns after the fall that ends XCE; SEC XCE raises it. */
static void h6_e(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xEA, 0x38, 0xFB, 0xEA, 0xDB};
    Bench816 *b;
    size_t i;
    size_t j;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 20);
    i = c8_fetch(b, 0, 0x008002);   /* the NOP after CLC XCE */
    j = c8_fetch(b, 0, 0x008005);   /* the NOP after SEC XCE */
    h_check(i != (size_t)-1 && j != (size_t)-1, "H6 the NOPs");
    if (i != (size_t)-1 && j != (size_t)-1) {
        h_check(c8(b, i - 1)->e == '1' && c8(b, i)->e4 == '1' && c8(b, i)->e5 == '0' &&
                    c8(b, i)->e == '0',
                "H6 E falls at the fall after XCE's last cycle + 5 ns");
        h_check(c8(b, j - 1)->e == '0' && c8(b, j)->e4 == '0' && c8(b, j)->e5 == '1',
                "H6 SEC XCE: E rises the same way");
    }
    if (h_failures) b8_dump(b, 0, 20);
    b8_close(b);
    h_end("h6_e");
}

/* H7: INC $10: MLB low on the read, modify and write cycles only. */
static void h7_mlb(void) {
    static const uint8_t PROGRAM[] = {0xE6, 0x10, 0xEA, 0xDB};
    Bench816 *b;
    size_t i;
    size_t k;
    int lows = 0;
    int only = 1;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x0010, 0x41);
    b8_cycles(b, 16);
    i = c8_find(b, 0, 0x000010);
    h_check(i != (size_t)-1, "H7 the read of 0010");
    for (k = 0; k < 14; k++) {
        if (c8(b, k)->mlb == '0') {
            lows++;
            only &= c8(b, k)->addr == 0x0010;
        }
    }
    h_check(i != (size_t)-1 && c8(b, i)->mlb == '0' && c8(b, i + 1)->mlb == '0' &&
                c8(b, i + 2)->mlb == '0' && c8(b, i + 2)->rw == '0' && c8(b, i + 3)->mlb == '1',
            "H7 MLB low on read, modify, write");
    h_check(lows == 3 && only, "H7 MLB low on those three cycles only");
    h_check(mem816[0x0010] == 0x42, "H7 the INC stored 42");
    b8_close(b);
    h_end("h7_mlb");
}

/* H8: ABORTB low for one cycle during INX: X unchanged, the INX's address
   pushed, the vector 00FFE8/9 with VPB low; a write made by an aborted STA
   stays. */
static void h8_abort(void) {
    static const uint8_t PROGRAM[] = {
        0x18, 0xFB,             /* 8000 CLC XCE */
        0xC2, 0x30,             /* 8002 REP #$30 */
        0xA9, 0x34, 0x12,       /* 8004 LDA #$1234 */
        0xA2, 0x00, 0x10,       /* 8007 LDX #$1000 */
        0xE8,                   /* 800A INX */
        0xEA, 0xDB,             /* 800B NOP STP */
    };
    static const uint8_t STA_PROGRAM[] = {   /* emulation mode */
        0xA9, 0x77,                   /* 8000 LDA #$77 */
        0x8D, 0x00, 0x03,             /* 8002 STA $0300 */
        0xEA, 0xDB,
    };
    Bench816 *b;
    CpuState s;
    size_t i;
    size_t v;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    while (c8_fetch(b, 0, 0x00800A) == (size_t)-1) b8_cycles(b, 1);
    /* INX's fetch has just ended: ABORTB low for its idle cycle's fall. */
    b8_drive(b, b->abortb, LOGIC_0);
    b8_cycles(b, 1);
    b8_drive(b, b->abortb, LOGIC_1);
    b8_cycles(b, 14);
    h_check(b8_state(b, &s) && b8_reg(&s, "X") == 0x1000, "H8 X unchanged (1000)");
    v = c8_find(b, 0, 0x00FFE8);
    h_check(v != (size_t)-1 && c8(b, v)->vpb == '0' && c8(b, v + 1)->addr == 0xFFE9 &&
                c8(b, v + 1)->vpb == '0',
            "H8 vector read at 00FFE8/9 with VPB low");
    /* Native: PBR at S, then PCH, PCL of the INX, then P. */
    i = c8_write(b, 0, 0x0001FF);
    h_check(i != (size_t)-1 && c8(b, i)->rw == '0' && c8(b, i)->data == 0x00 &&
                c8(b, i + 1)->data == 0x80 && c8(b, i + 2)->data == 0x0A,
            "H8 the stack holds 00 80 0A (the INX's address)");
    if (h_failures) b8_dump(b, 0, 40);
    h_check(b8_state(b, &s) && s.status == CPU_STOPPED && c8_find(b, 0, 0x00A000) != (size_t)-1,
            "H8 the handler ran (STP at A000)");
    b8_close(b);

    b = b8_boot(STA_PROGRAM, sizeof STA_PROGRAM);
    b8_byte(0xFFF8, 0x00);
    b8_byte(0xFFF9, 0xA0);
    while (c8_find(b, 0, 0x008004) == (size_t)-1) b8_cycles(b, 1);   /* STA's last operand */
    b8_drive(b, b->abortb, LOGIC_0);
    b8_cycles(b, 1);
    b8_drive(b, b->abortb, LOGIC_1);
    b8_cycles(b, 14);
    h_check(known816[0x0300] && mem816[0x0300] == 0x77, "H8 the aborted STA's write stays");
    h_check(c8_find(b, 0, 0x00FFF8) != (size_t)-1, "H8 emulation abort vector 00FFF8");
    b8_close(b);
    h_end("h8_abort");
}

/* H9: RDY low holds a read and a write cycle; the data pins stay in their
   transfer state through PHI2 low; RDY X -> LOST. */
static void h9_rdy(void) {
    static const uint8_t PROGRAM[] = {0x18, 0xFB, 0xA9, 0x5A, 0x8D, 0x00, 0x02, 0xAD, 0x00, 0x02, 0xDB};
    Bench816 *b;
    CpuState s;
    size_t i;
    size_t k;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    /* The write: hold it 2 cycles. */
    while (c8_find(b, 0, 0x008006) == (size_t)-1) b8_cycles(b, 1);   /* STA's last operand */
    /* Now in the write cycle's PHI2 low. */
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_0, DRIVE_STRONG, 0);
    b8_cycles(b, 2);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    b8_cycles(b, 6);
    i = c8_find(b, 0, 0x000200);
    h_check(i != (size_t)-1 && c8(b, i)->rw == '0' && c8(b, i + 1)->addr == 0x0200 &&
                c8(b, i + 1)->rw == '0' && c8(b, i + 2)->addr == 0x0200,
            "H9 the write cycle repeats while RDY is low");
    h_check(i != (size_t)-1 && c8(b, i + 1)->f33 == 0x5A && c8(b, i + 2)->f33 == 0x5A,
            "H9 a held write keeps its data through PHI2 low (no bank)");
    /* The read 0200: hold it. */
    k = c8_find(b, i + 3, 0x000200);
    h_check(k != (size_t)-1 && c8(b, k)->rw == '1', "H9 the read of 0200");
    b8_close(b);

    b = b8_boot(PROGRAM, sizeof PROGRAM);
    while (c8_find(b, 0, 0x008009) == (size_t)-1) b8_cycles(b, 1);   /* LDA's last operand */
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_0, DRIVE_STRONG, 0);
    b8_cycles(b, 2);
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_1, DRIVE_WEAK, 0);
    b8_cycles(b, 4);
    k = c8_find(b, 0, 0x000200);
    k = c8_find(b, k + 1, 0x000200);   /* the read (after the write) */
    h_check(k != (size_t)-1 && c8(b, k)->rw == '1' && c8(b, k + 1)->addr == 0x0200 &&
                c8(b, k + 1)->rw == '1' && c8(b, k + 1)->f33 == -2,
            "H9 a held read repeats; D0-D7 stay Z through PHI2 low");
    h_check(b8_state(b, &s) && b8_reg(&s, "C") == 0x5A, "H9 the read completes after (A = 5A)");
    if (h_failures) {
        printf("  C %x status %d\n", b8_reg(&s, "C"), s.status);
        b8_dump(b, 0, 30);
    }
    /* RDY X: LOST. */
    sim_schedule_drive(b->sim, b->tb, b->rdy, LOGIC_X, DRIVE_STRONG, 0);
    b8_cycles(b, 2);
    h_check(b8_state(b, &s) && s.status == CPU_LOST, "H9 RDY X: LOST");
    b8_close(b);
    h_end("h9_rdy");
}

/* H10: WAI pulls RDY low until an IRQ (I set) ends it; STP; reset restarts. */
static void h10_wai_stp(void) {
    static const uint8_t PROGRAM[] = {0x78, 0xCB, 0xE8, 0xDB};   /* SEI WAI INX STP */
    Bench816 *b;
    CpuState s;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 16);
    h_check(b8_state(b, &s) && s.status == CPU_WAITING, "H10 WAI: WAITING");
    h_check(b8_char(b, b->rdy) == '0', "H10 WAI pulls RDY low");
    b8_drive(b, b->irq, LOGIC_0);
    b8_cycles(b, 6);
    b8_drive(b, b->irq, LOGIC_1);
    b8_cycles(b, 4);
    h_check(b8_char(b, b->rdy) == '1', "H10 the IRQ releases RDY");
    h_check(b8_state(b, &s) && s.status == CPU_STOPPED && b8_reg(&s, "X") == 1,
            "H10 I set: the next instruction (INX) runs, then STP: STOPPED");
    b8_reset(b);
    b8_cycles(b, 10);
    h_check(b8_state(b, &s) && (s.status == CPU_WAITING || s.status == CPU_RUNNING),
            "H10 reset restarts it");
    b8_close(b);
    h_end("h10_wai_stp");
}

/* H11: BE low floats A, D and RWB 25 ns later; VDA/VPA keep driving; the
   CPU keeps counting cycles. */
static void h11_be(void) {
    static const uint8_t PROGRAM[] = {0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0x80, 0xF8};
    Bench816 *b;
    CpuState s;
    uint64_t before;
    SimTime t;
    int floats = 1;
    int i;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_cycles(b, 4);
    b8_state(b, &s);
    before = s.cycles;
    b->fill = 0xEA;   /* another bus master's memory: NOPs */
    b8_drive(b, b->be, LOGIC_0);
    t = sim_now(b->sim);
    b8_run_to(b, t + NS(24));
    h_check(b8_char(b, b->a[0]) != 'Z', "H11 driven before tBVD");
    b8_run_to(b, t + NS(25));
    for (i = 0; i < 16; i++) floats &= b8_char(b, b->a[i]) == 'Z';
    for (i = 0; i < 8; i++) floats &= b8_char(b, b->d[i]) == 'Z';
    floats &= b8_char(b, b->rw) == 'Z';
    h_check(floats, "H11 A, D, RWB Z at + 25 ns");
    h_check(b8_char(b, b->vda) != 'Z' && b8_char(b, b->vpa) != 'Z', "H11 VDA, VPA still driven");
    b8_cycles(b, 3);
    b8_state(b, &s);
    h_check(s.status == CPU_RUNNING && s.cycles >= before + 3, "H11 the CPU keeps counting cycles");
    if (h_failures) printf("  cycles %llu -> %llu status %d\n", (unsigned long long)before,
                           (unsigned long long)s.cycles, s.status);
    b8_drive(b, b->be, LOGIC_1);
    b8_cycles(b, 1);
    h_check(b8_char(b, b->a[0]) != 'Z' && b8_char(b, b->rw) != 'Z', "H11 BE high drives again");
    b8_close(b);
    h_end("h11_be");
}

/* H12: read data that changed 5 ns before the fall reads X and is used ->
   LOST; IRQB X while I = 1 is harmless. */
static void h12_setup(void) {
    static const uint8_t PROGRAM[] = {0x78, 0xEA, 0xEA, 0xEA, 0xAD, 0x00, 0x02, 0xDB};
    Bench816 *b;
    CpuState s;

    h_begin();
    b = b8_boot(PROGRAM, sizeof PROGRAM);
    b8_byte(0x0200, 0x11);
    b8_cycles(b, 4);
    b8_drive(b, b->irq, LOGIC_X);
    b8_cycles(b, 3);
    h_check(b8_state(b, &s) && s.status == CPU_RUNNING, "H12 IRQB X with I = 1: still RUNNING");
    b8_drive(b, b->irq, LOGIC_1);
    /* LDA $0200: change the data 5 ns before the fall of its data read. */
    while (c8_find(b, 0, 0x008006) == (size_t)-1) b8_cycles(b, 1);
    b8_run_to(b, b->next_fall - NS(5));
    b8_drive(b, b->d[0], LOGIC_0);
    b8_cycles(b, 4);
    h_check(b8_state(b, &s) && s.status == CPU_LOST, "H12 data X at the fall, used: LOST");
    b8_close(b);
    h_end("h12_setup");
}

/* H13: registers after a native program in bank 1; the instruction text and
   24-bit address. */
static void h13_state(void) {
    static const uint8_t BANK0[] = {0x18, 0xFB, 0x5C, 0x00, 0x80, 0x01};   /* CLC XCE JML $018000 */
    static const uint8_t BANK1[] = {
        0xC2, 0x20, 0xEA,             /* 018000 REP #$20; NOP */
        0xAF, 0x56, 0x34, 0x12,       /* 018003 LDA $123456 */
        0xDB,                         /* 018007 STP */
    };
    Bench816 *b;
    CpuState s;

    h_begin();
    b = b8_boot(BANK0, sizeof BANK0);
    b8_load(0x018000, BANK1, sizeof BANK1);
    b8_byte(0x123456, 0x44);
    b8_byte(0x123457, 0x33);
    b8_cycles(b, 30);
    h_check(b8_state(b, &s) && s.status == CPU_STOPPED, "H13 STOPPED at the STP");
    h_check(s.register_count == 10 && b8_reg(&s, "C") == 0x3344 && b8_reg(&s, "PBR") == 0x01 &&
                b8_reg(&s, "E") == 0 && b8_reg(&s, "DBR") == 0 && b8_reg(&s, "D") == 0 &&
                b8_reg(&s, "PC") == 0x8008,
            "H13 C 3344, PBR 01, E 0, DBR 00, D 0000, PC 8008");
    h_check(strcmp(s.registers[0].name, "C") == 0 && strcmp(s.registers[9].name, "E") == 0 &&
                s.registers[0].bits == 16 && s.registers[5].bits == 8 && s.registers[9].bits == 1,
            "H13 register order C X Y S D DBR PBR PC P E and widths");
    h_check(s.has_instruction && s.instruction_address == 0x018003 &&
                strcmp(s.instruction_text, "LDA $123456") == 0 && s.instruction_length == 4,
            "H13 last instruction 018003 LDA $123456");
    if (!(s.has_instruction && strcmp(s.instruction_text, "LDA $123456") == 0)) {
        printf("  (instruction %06x '%s' length %zu)\n", (unsigned)s.instruction_address,
               s.instruction_text, s.instruction_length);
    }
    b8_close(b);
    h_end("h13_state");
}

static void run_w65c816s_cases(void) {
    h1_reset();
    h2_bank();
    h3_write();
    h4_flags();
    h5_mx();
    h6_e();
    h7_mlb();
    h8_abort();
    h9_rdy();
    h10_wai_stp();
    h11_be();
    h12_setup();
    h13_state();
}

/* ---- The W65C02S I/O board (plan-14-E section 3.4) --------------------------- */

/*
 * The program, at 8000 (EEPROM offset 0):
 *
 *   8000 A2 FF     RESET: LDX #$FF
 *   8002 9A               TXS
 *   8003 9C 00 02         STZ $0200
 *   8006 A9 FF            LDA #$FF
 *   8008 8D 02 60         STA $6002      ; VIA DDRB
 *   800B A9 55            LDA #$55
 *   800D 8D 00 60         STA $6000      ; VIA ORB
 *   8010 A9 FF            LDA #$FF
 *   8012 8D 00 50         STA $5000      ; PIA DDRA (CRA bit 2 = 0 after reset)
 *   8015 A9 04            LDA #$04
 *   8017 8D 01 50         STA $5001      ; PIA CRA: select ORA
 *   801A A9 AA            LDA #$AA
 *   801C 8D 00 50         STA $5000      ; PIA ORA
 *   801F 8D 01 40         STA $4001      ; ACIA programmed reset
 *   8022 A9 1F            LDA #$1F
 *   8024 8D 03 40         STA $4003      ; 1 stop, 8 bits, baud generator, 19200
 *   8027 A9 0B            LDA #$0B
 *   8029 8D 02 40         STA $4002      ; no parity, RTS low, no TX/RX int, DTR
 *   802C A9 48            LDA #'H'
 *   802E 8D 00 40         STA $4000
 *   8031 20 6C 80         JSR DELAY
 *   8034 A9 49            LDA #'I'
 *   8036 8D 00 40         STA $4000
 *   8039 20 6C 80         JSR DELAY
 *   803C A9 00            LDA #$00
 *   803E 8D 0B 60         STA $600B      ; ACR: T1 one-shot
 *   8041 A9 C0            LDA #$C0
 *   8043 8D 0E 60         STA $600E      ; IER: enable T1
 *   8046 A9 E8            LDA #$E8
 *   8048 8D 04 60         STA $6004
 *   804B A9 03            LDA #$03
 *   804D 8D 05 60         STA $6005      ; T1 = 1000
 *   8050 58               CLI
 *   8051 AD 00 02  WAIT:  LDA $0200
 *   8054 F0 FB            BEQ WAIT
 *   8056 8D 01 02         STA $0201
 *   8059 AD 00 40         LDA $4000      ; the ACIA's RDR: the looped-back byte
 *   805C 8D 02 02         STA $0202
 *   805F 80 FE     DONE:  BRA DONE
 *   8061 48        IRQ:   PHA
 *   8062 AD 04 60         LDA $6004      ; reading T1C-L clears IFR6
 *   8065 A9 01            LDA #$01
 *   8067 8D 00 02         STA $0200
 *   806A 68               PLA
 *   806B 40               RTI
 *   806C A2 80     DELAY: LDX #$80
 *   806E CA        D1:    DEX
 *   806F D0 FD            BNE D1         ; 640 cycles > one 19200-baud character (521 us)
 *   8071 60               RTS
 *
 *   FFFC 00 80 (RESET), FFFE 61 80 (IRQ)
 */
static void board_io(void) {
    static const uint8_t PROGRAM[] = {
        0xA2, 0xFF, 0x9A, 0x9C, 0x00, 0x02, 0xA9, 0xFF, 0x8D, 0x02, 0x60, 0xA9, 0x55, 0x8D,
        0x00, 0x60, 0xA9, 0xFF, 0x8D, 0x00, 0x50, 0xA9, 0x04, 0x8D, 0x01, 0x50, 0xA9, 0xAA,
        0x8D, 0x00, 0x50, 0x8D, 0x01, 0x40, 0xA9, 0x1F, 0x8D, 0x03, 0x40, 0xA9, 0x0B, 0x8D,
        0x02, 0x40, 0xA9, 0x48, 0x8D, 0x00, 0x40, 0x20, 0x6C, 0x80, 0xA9, 0x49, 0x8D, 0x00,
        0x40, 0x20, 0x6C, 0x80, 0xA9, 0x00, 0x8D, 0x0B, 0x60, 0xA9, 0xC0, 0x8D, 0x0E, 0x60,
        0xA9, 0xE8, 0x8D, 0x04, 0x60, 0xA9, 0x03, 0x8D, 0x05, 0x60, 0x58, 0xAD, 0x00, 0x02,
        0xF0, 0xFB, 0x8D, 0x01, 0x02, 0xAD, 0x00, 0x40, 0x8D, 0x02, 0x02, 0x80, 0xFE, 0x48,
        0xAD, 0x04, 0x60, 0xA9, 0x01, 0x8D, 0x00, 0x02, 0x68, 0x40, 0xA2, 0x80, 0xCA, 0xD0,
        0xFD, 0x60
    };
    static uint8_t image[32768];
    int before = failures;
    IoBoard *board;
    CpuState state;
    uint16_t sent[8];
    size_t n;
    uint8_t v;

    memset(image, 0xFF, sizeof image);
    memcpy(image, PROGRAM, sizeof PROGRAM);
    image[0x7FFC] = 0x00;
    image[0x7FFD] = 0x80;
    image[0x7FFE] = 0x61;
    image[0x7FFF] = 0x80;
    expect(PROGRAM[0x61] == 0x48 && PROGRAM[0x6C] == 0xA2, "io_board: IRQ at 8061, DELAY at 806C");
    board = io_board_create(library, image, sizeof image);
    if (board == NULL || !io_board_run(board, NS(4000000))) {
        expect(0, "io_board: the board did not run");
        io_board_destroy(board);
        printf("io_board FAIL\n");
        return;
    }
    io_board_state(board, &state);
    expect(state.status == CPU_RUNNING, "io_board: CPU RUNNING");
    expect(io_board_sram(board, 0x0200, &v) && v == 0x01 && io_board_sram(board, 0x0201, &v) &&
               v == 0x01,
           "io_board: SRAM 0200 = 01, 0201 = 01 (the T1 interrupt ran)");
    expect(io_board_port(board, "VIA", 'B', &v) && v == 0x55, "io_board: VIA PB = 55");
    expect(io_board_port(board, "PIA", 'A', &v) && v == 0xAA, "io_board: PIA PA = AA");
    n = io_board_sent(board, sent, 8);
    expect(n == 2 && sent[0] == 'H' && sent[1] == 'I', "io_board: the ACIA sent HI");
    expect(io_board_sram(board, 0x0202, &v) && v == 'I',
           "io_board: SRAM 0202 = 'I' (the ACIA's RDR, looped back)");
    io_board_destroy(board);
    printf("io_board %s\n", failures == before ? "ok" : "FAIL");
}

/* The Z80 board: count to 10 at 8200, CALL a store at 8201, HALT. */
static void board_z80(void) {
    static const uint8_t IMAGE[] = {
        0x31, 0x00, 0x00, 0x3E, 0x00, 0x3C, 0x32, 0x00, 0x82, 0xFE, 0x10, 0x20, 0xF8, 0xCD,
        0x20, 0x00, 0x76, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0x32, 0x01, 0x82, 0xC9
    };
    CpuBoard *board = cpu_board_create(library, "Z84C0010PEG", IMAGE, sizeof IMAGE, -1);
    CpuState state;

    if (board == NULL || !cpu_board_run(board, NS(2000000))) {
        expect(0, "Z84C0010PEG board did not run");
        cpu_board_destroy(board);
        return;
    }
    cpu_board_state(board, &state);
    expect(sram_is(board, 0x0200, 0x10) && sram_is(board, 0x0201, 0x10),
           "Z84C0010PEG board: SRAM 0200 = 10 and 0201 = 10 (the CALLed store)");
    expect(sram_is(board, 0x7FFE, 0x10) && sram_is(board, 0x7FFF, 0x00),
           "Z84C0010PEG board: SRAM 7FFE-7FFF = 10 00 (the return address)");
    expect(state.status == CPU_HALTED && (reg_value(&state, "AF") >> 8) == 0x10 &&
               strcmp(state.instruction_text, "HALT") == 0,
           "Z84C0010PEG board: HALTED, A = 10, last instruction HALT");
    cpu_board_destroy(board);
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
        case_reset_setup(CORE_NAMES[i]);
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
    zcase_reset_setup();
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
    board_6502("W65C02S6TPG-14", 1);
    board_6502("MPS6502", 0);
    board_z80();
    board_io();
    run_w65c816s_cases();
    case_circuit_api();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
