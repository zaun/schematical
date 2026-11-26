#ifndef CPU_TEST_BENCH_H
#define CPU_TEST_BENCH_H

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

#define W65C02S "FIXTURE-W65C02S6TPG-14"
#define MPS6502 "FIXTURE-MPS6502"

/* ---- The bench ------------------------------------------------------------ */

enum { T_RISE, T_FALL, T_SERVE, T_SAMPLE, T_RELEASE, T_POKE };

enum { LOG_MAX = 8192 };

#define W65C816S "W65C816S6TPG-14"   /* the shipped part (plan-14-I) */

#define Z84C00 "FIXTURE-Z84C0010PEG"
enum { ZLOG_MAX = 4096, RISES_MAX = 100000 };

enum { LOG816 = 4096 };

extern int failures;
extern PartLibrary *library;
void expect(int ok, const char *what);

/* ---- Fixtures (plan-09-D section 4.2) ------------------------------------ */

static const char FIXTURE_FOOTPRINT[] =
    "{'format':'schematical-footprints','version':1,'footprints':["
    "{'id':'FIXTURE-PDIP-40','description':'40-pin 600 mil PDIP','style':'dual','pins':40,"
    "'body':{'width':13.91,'height':52.33},'pitch':2.54,'span':15.24,"
    "'pad':{'width':1.5,'height':1.5,'shape':'round'},'drill':0.8,'pin1Shape':'rect'}]}";

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
char *unquote(const char *text, size_t size);
int load_fixture(void);

/* One bus cycle, sampled 1 ps before PHI2 falls. addr/data -1: not all
   bits 0 or 1 (data -2: Z). rw, sync, vpb, mlb: '0', '1', 'X' or 'Z'. */
typedef struct {
    int addr;
    char rw, sync, vpb, mlb;
    int data;
    SimTime at;
} Cycle;

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
size_t bench_pins(const void *config);
void *bench_create(const void *config);
void bench_destroy(void *state);
LogicValue pin_value(const Bench *b, size_t pin);
char pin_char(const Bench *b, size_t pin);
int bus_address(const Bench *b);
int bus_data(const Bench *b);
void drive(Bench *b, size_t pin, LogicValue value, SimTime delay);
void drive_data(Bench *b, int value);
SimTime min_time(SimTime x, SimTime y);
void bench_start(Sim *sim, SimInstance *instance, void *state);
void bench_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id);
extern const SimComponentType BENCH_TYPE;
size_t pin_named(const Part *part, const char *name);
void run_to(Bench *b, SimTime time);
void run_cycles(Bench *b, unsigned n);
void set_pin(Bench *b, size_t pin, LogicValue value);
Bench *bench_open(const char *name);
void bench_close(Bench *b);
int cpu_state_of(const Bench *b, CpuState *out);

static const char *const CORE_NAMES[] = {W65C02S, MPS6502};
void load(Bench *b, uint16_t address, const uint8_t *bytes, size_t count);
void load_byte(Bench *b, uint16_t address, uint8_t value);
void do_reset(Bench *b);
Bench *boot(const char *name, const uint8_t *program, size_t count);
SimTime fall_at(const Bench *b, unsigned k);
const Cycle *cycle(const Bench *b, size_t i);
void run_to_cycle(Bench *b, size_t i);
int status_is(const Bench *b, CpuStatus status);
const char *label(const char *name, const char *text);
uint8_t reg(const CpuState *state, const char *name);
int address_is(const Bench *b, int value);
int sram_is(const CpuBoard *board, uint32_t address, uint8_t expected);

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
uint32_t reg_value(const CpuState *state, const char *name);

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

#endif
