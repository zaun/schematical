/*
 * rtc_test (plan-24-F §3): the rtc kind on fixture devices shaped like the
 * plan-24 RTCs (planning/plan-24-data/specs): time from simulated time,
 * carries and leap years, the stop bit, UIP, 12-hour mode, periodic and alarm
 * interrupts, the read latch, battery-backed RAM and a timer interrupt.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"

static int failures;
static PartLibrary *library;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static char *json(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    size_t i;

    for (i = 0; text[i] != '\0'; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    copy[i] = '\0';
    return copy;
}

#define DUAL(ID, PINS) \
    "{'id':'" ID "','description':'d','style':'dual','pins':" PINS ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
#define PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define PART(NAME, FP) "'parts':[{'name':'" NAME "','package':'SOIC','pinout':'p','footprint':'" FP "'}]}"
#define REG(NAME, ADDR, FIELDS) "{'name':'" NAME "','address':" #ADDR ",'width':8,'fields':[" FIELDS "]}"
#define F(NAME, HI, LO) "{'name':'" NAME "','bits':[" #HI "," #LO "]}"

/* BQ32000-shape: I2C pointer with auto-increment, BCD time, STOP bit. */
#define I2C_RTC \
    "{'id':'I2CRTC','function':'RTC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "X1", "input") "," PIN(2, "X2", "output") "," PIN(3, "VBACK", "power") "," \
    PIN(4, "GND", "ground") "," PIN(5, "SDA", "inout") "," PIN(6, "SCL", "input") "," \
    PIN(7, "IRQ", "output") "," PIN(8, "VCC", "power") "]},'vcc':[3,5.5],'analog':['X1','X2','VBACK']," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[104]},'registers':[" \
    REG("SECONDS", 0, F("STOP", 7, 7) "," F("S", 6, 0)) "," REG("MINUTES", 1, F("M", 6, 0)) "," \
    REG("HOURS", 2, F("CENT", 6, 6) "," F("H", 5, 0)) "," REG("DAY", 3, F("D", 2, 0)) "," \
    REG("DATE", 4, F("D", 5, 0)) "," REG("MONTH", 5, F("M", 4, 0)) "," REG("YEARS", 6, F("Y", 7, 0)) "]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'P','in':'aaaaaaaa','action':'pointer','address':'a','increment':1}],'read':'pointer'}," \
    "'clock':{'fields':[{'field':'SECONDS.S','unit':'second'},{'field':'MINUTES.M','unit':'minute'}," \
    "{'field':'HOURS.H','unit':'hour'},{'field':'DAY.D','unit':'weekday','first':1}," \
    "{'field':'DATE.D','unit':'date'},{'field':'MONTH.M','unit':'month'},{'field':'YEARS.Y','unit':'year','base':2000}]," \
    "'stop':{'field':'SECONDS.STOP','stopped':1,'powerOn':0},'update':'none'}," PART("I2CRTCX", "T8")

/* The BQ3285 / DP857x / BQ4802 shapes: a parallel bus. */
#define AD8 PIN(1, "AD7", "inout") "," PIN(2, "AD6", "inout") "," PIN(3, "AD5", "inout") "," \
    PIN(4, "AD4", "inout") "," PIN(5, "AD3", "inout") "," PIN(6, "AD2", "inout") "," \
    PIN(7, "AD1", "inout") "," PIN(8, "AD0", "inout")
#define AD8_NAMES "'AD7','AD6','AD5','AD4','AD3','AD2','AD1','AD0'"

/* BQ3285-shape: multiplexed AD bus latched by AS, UIP/SET, 12-hour HF, alarms, periodic. */
#define MUX_RTC \
    "{'id':'MUXRTC','function':'RTC','description':'d','drawer':['Interface'],'pinouts':{'p':[" AD8 "," \
    PIN(9, "AS", "input") "," PIN(10, "DS", "input") "," PIN(11, "R/W", "input") "," \
    PIN(12, "CS", "input") "," PIN(13, "INT", "output") "," PIN(14, "VCC", "power") "," \
    PIN(15, "GND", "ground") "," PIN(16, "X1", "input") "]},'vcc':[4.5,5.5],'analog':['X1']," \
    "'bus':{'style':'parallel','data':[" AD8_NAMES "],'cs':{'pin':'CS','active':0},'rd':{'pin':'DS','active':0}," \
    "'wr':{'pin':'R/W','active':0},'address':[],'ale':{'pin':'AS','edge':'fall','bits':8}},'registers':[" \
    REG("SECONDS", 0, "") "," REG("ASEC", 1, "") "," REG("MINUTES", 2, "") "," REG("AMIN", 3, "") "," \
    REG("HOURS", 4, F("PM", 7, 7) "," F("H", 6, 0)) "," REG("AHOUR", 5, "") "," REG("DAY", 6, "") "," \
    REG("DATE", 7, "") "," REG("MONTH", 8, "") "," REG("YEAR", 9, "") "," \
    "{'name':'REGA','address':10,'width':8,'readOnly':128,'reset':0,'fields':[" F("UIP", 7, 7) "," F("RS", 3, 0) "]}," \
    "{'name':'REGB','address':11,'width':8,'reset':2,'fields':[" F("SET", 7, 7) "," F("PIE", 6, 6) "," \
    F("AIE", 5, 5) "," F("DF", 2, 2) "," F("HF", 1, 1) "]}," \
    "{'name':'REGC','address':12,'width':8,'readOnly':true,'clearOnRead':240,'fields':[" F("PF", 6, 6) "," F("AF", 5, 5) "]}," \
    "{'name':'REGD','address':13,'width':8,'readOnly':true,'reset':128}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'aaaaaaaa dddddddd','action':'write','address':'a','data':'d'}]," \
    "'read':{'0':'[SECONDS]','1':'[ASEC]','2':'[MINUTES]','3':'[AMIN]','4':'[HOURS]','5':'[AHOUR]','6':'[DAY]'," \
    "'7':'[DATE]','8':'[MONTH]','9':'[YEAR]','10':'[REGA]','11':'[REGB]','12':'[REGC]','13':'[REGD]'}}," \
    "'clock':{'fields':[{'field':'SECONDS','unit':'second','format':{'field':'REGB.DF','binary':1}}," \
    "{'field':'MINUTES','unit':'minute','format':{'field':'REGB.DF','binary':1}}," \
    "{'field':'HOURS.H','unit':'hour','format':{'field':'REGB.DF','binary':1},'h12':'REGB.HF','h12Level':0,'pm':'HOURS.PM'}," \
    "{'field':'DAY','unit':'weekday','first':1},{'field':'DATE','unit':'date'},{'field':'MONTH','unit':'month'}," \
    "{'field':'YEAR','unit':'year','base':2000}],'update':{'uip':'REGA.UIP','inhibit':'REGB.SET'}," \
    "'alarms':[{'match':[{'field':'ASEC','unit':'second','dontCare':'11xxxxxx'},{'field':'AMIN','unit':'minute','dontCare':'11xxxxxx'}," \
    "{'field':'AHOUR','unit':'hour','dontCare':'11xxxxxx'}],'flag':'REGC.AF','enable':'REGB.AIE'," \
    "'pin':{'pin':'INT','active':0,'openDrain':true}}]," \
    "'periodic':{'field':'REGA.RS','periods':{'3':122070.3125,'15':500000000},'flag':'REGC.PF','enable':'REGB.PIE','pin':'INT'}," \
    "'ram':{'space':'ram','bytes':114,'address':14}}," PART("MUXRTCX", "T16")

/* BQ4802-shape: UTI freezes the read registers while counting continues. */
#define LATCH_RTC \
    "{'id':'LATRTC','function':'RTC','description':'d','drawer':['Interface'],'pinouts':{'p':[" AD8 "," \
    PIN(9, "A3", "input") "," PIN(10, "A2", "input") "," PIN(11, "A1", "input") "," PIN(12, "A0", "input") "," \
    PIN(13, "CS", "input") "," PIN(14, "OE", "input") "," PIN(15, "WE", "input") "," PIN(16, "VCC", "power") "," \
    PIN(17, "GND", "ground") "," PIN(18, "X1", "input") "," PIN(19, "X2", "output") "," PIN(20, "WDI", "input") "," \
    PIN(21, "WDO", "output") "," PIN(22, "NC", "nc") "," PIN(23, "NC", "nc") "," PIN(24, "NC", "nc") "]}," \
    "'vcc':[4.5,5.5],'analog':['X1','X2']," \
    "'bus':{'style':'parallel','data':[" AD8_NAMES "],'address':['A3','A2','A1','A0'],'cs':{'pin':'CS','active':0}," \
    "'rd':{'pin':'OE','active':0},'wr':{'pin':'WE','active':0}},'registers':[" \
    REG("SECONDS", 0, "") "," REG("MINUTES", 2, "") "," REG("HOURS", 4, "") "," \
    REG("RATES", 11, F("WD", 6, 5)) "," REG("CONTROL", 14, F("UTI", 3, 3) "," F("H24", 1, 1)) "]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'aaaa dddddddd','action':'write','address':'a','data':'d'}]," \
    "'read':{'0':'[SECONDS]','2':'[MINUTES]','4':'[HOURS]','11':'[RATES]','14':'[CONTROL]'}}," \
    "'clock':{'fields':[{'field':'SECONDS','unit':'second'},{'field':'MINUTES','unit':'minute'}," \
    "{'field':'HOURS','unit':'hour'}],'update':{'latch':'CONTROL.UTI','write':'CONTROL.UTI','writeHalts':false}," \
    "'watchdog':{'field':'RATES.WD','periods':{'1':100000,'2':200000},'kick':{'pin':'WDI','edge':'both'}," \
    "'pin':{'pin':'WDO','active':0}}}," PART("LATRTCX", "T24")

/* DP8570-shape: RAM bytes and a 1 ms timer interrupt (periodic array form). */
#define TIMER_RTC \
    "{'id':'TMRTC','function':'RTC','description':'d','drawer':['Interface'],'pinouts':{'p':[" AD8 "," \
    PIN(9, "A3", "input") "," PIN(10, "A2", "input") "," PIN(11, "A1", "input") "," PIN(12, "A0", "input") "," \
    PIN(13, "CS", "input") "," PIN(14, "RD", "input") "," PIN(15, "WR", "input") "," PIN(16, "INTR", "output") "," \
    PIN(17, "VCC", "power") "," PIN(18, "GND", "ground") "," PIN(19, "A4", "input") "," PIN(20, "X1", "input") "]}," \
    "'vcc':[4.5,5.5],'analog':['X1']," \
    "'bus':{'style':'parallel','data':[" AD8_NAMES "],'address':['A4','A3','A2','A1','A0'],'cs':{'pin':'CS','active':0}," \
    "'rd':{'pin':'RD','active':0},'wr':{'pin':'WR','active':0}},'registers':[" \
    REG("HUNDREDTHS", 1, "") "," REG("SECONDS", 2, "") "," REG("MINUTES", 3, "") "," REG("HOURS", 4, "") "," \
    "{'name':'PFR','address':5,'width':8,'readOnly':true,'clearOnRead':255,'fields':[" F("MS1", 0, 0) "]}," \
    REG("ICR0", 6, F("MS1", 0, 0)) "]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'aaaaa dddddddd','action':'write','address':'a','data':'d'}]," \
    "'read':{'1':'[HUNDREDTHS]','2':'[SECONDS]','3':'[MINUTES]','4':'[HOURS]','5':'[PFR]','6':'[ICR0]'}}," \
    "'clock':{'fields':[{'field':'HUNDREDTHS','unit':'hundredth'},{'field':'SECONDS','unit':'second'}," \
    "{'field':'MINUTES','unit':'minute'},{'field':'HOURS','unit':'hour'}],'update':'none'," \
    "'periodic':[{'periodNs':1000000,'flag':'PFR.MS1','enable':'ICR0.MS1','pin':{'pin':'INTR','active':0,'openDrain':true}}]," \
    "'ram':{'bytes':4,'address':20}}," PART("TMRTCX", "T20")

static int load_fixtures(void) {
    char *fp = json("{'format':'schematical-footprints','version':1,'footprints':["
                    DUAL("T8", "8") "," DUAL("T16", "16") "," DUAL("T20", "20") "," DUAL("T24", "24") "]}");
    char *parts = json("{'format':'schematical-parts','version':1,'kind':'rtc','devices':["
                       I2C_RTC "," MUX_RTC "," LATCH_RTC "," TIMER_RTC "]}");
    char *error = NULL;
    int ok;

    library = part_library_create();
    ok = load_footprints(library, fp, "fixtures.json", &error) == 0 &&
         load_parts(library, parts, "rtc-fixtures.json", &error) == 0;
    if (!ok) {
        printf("FAIL loading the fixtures: %s\n", error ? error : "?");
        failures++;
    }
    free(error);
    free(fp);
    free(parts);
    return ok;
}

/* ---- The bench (as converter_test) ---------------------------------------------------------- */

typedef struct {
    size_t count;
} PinCount;

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
        sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_WEAK, 0);
    }
}

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};
static const SimComponentType PULLUP = {"pu", drv_pins, drv_create, free, pull_start, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};

typedef struct {
    Sim *sim;
    SimInstance *chip, *drv, *pu;
    const Part *part;
    PinCount pins, pulls;
} Bench;

enum { HALF = 100000 };

static size_t bpin(const Bench *b, const char *name) {
    size_t i;

    for (i = 0; i < b->part->pin_count; i++) {
        if (strcmp(b->part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "no pin %s\n", name);
    exit(2);
}

static void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, bpin(b, name), v, DRIVE_STRONG, 0);
}

static LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, bpin(b, name));
}

static void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

static void run_to(Bench *b, SimTime t) {
    if (t > sim_now(b->sim)) {
        run(b, t - sim_now(b->sim));
    }
}

static Bench *bench(const char *part, const char *const *pulls) {
    Bench *b = calloc(1, sizeof *b);
    size_t i, k;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
        b->pulls.count++;
    }
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pulls);
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
            if (strcmp(pulls[k], b->part->pins[i].name) == 0) {
                sim_connect(b->sim, net, b->pu, k);
            }
        }
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* ---- I2C -------------------------------------------------------------------------------------- */

static void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static LogicValue i2c_bit(Bench *b, int out) {
    LogicValue v;

    line(b, "SDA", out);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    v = get(b, "SDA");
    line(b, "SCL", 0);
    run(b, HALF);
    return v;
}

static void i2c_start(Bench *b) {
    line(b, "SDA", 1);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 0);
    run(b, HALF);
}

static void i2c_stop(Bench *b) {
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 1);
    run(b, HALF);
}

static int i2c_byte(Bench *b, unsigned v) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (v >> i) & 1u);
    }
    return i2c_bit(b, 1) == LOGIC_0;
}

static unsigned i2c_in(Bench *b, int ack) {
    unsigned v = 0;
    int i;

    for (i = 0; i < 8; i++) {
        v = (v << 1) | (i2c_bit(b, 1) == LOGIC_1 ? 1u : 0u);
    }
    i2c_bit(b, !ack);
    return v;
}

static void rtc_write(Bench *b, unsigned reg, const unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, 0x68 << 1);
    i2c_byte(b, reg);
    for (i = 0; i < n; i++) {
        i2c_byte(b, v[i]);
    }
    i2c_stop(b);
}

static void rtc_read(Bench *b, unsigned reg, unsigned *v, size_t n) {
    size_t i;

    i2c_start(b);
    i2c_byte(b, 0x68 << 1);
    i2c_byte(b, reg);
    i2c_stop(b);
    i2c_start(b);
    i2c_byte(b, (0x68 << 1) | 1u);
    for (i = 0; i < n; i++) {
        v[i] = i2c_in(b, i + 1 < n);
    }
    i2c_stop(b);
}

static void i2c_rtc(void) {
    static const char *const pulls[] = {"SDA", "SCL", NULL};
    Bench *b = bench("I2CRTCX", pulls);
    unsigned t[7];

    run(b, HALF);
    rtc_read(b, 0, t, 7);
    check(t[0] == 0 && t[1] == 0 && t[2] == 0 && t[3] == 7 && t[4] == 1 && t[5] == 1 && t[6] == 0,
          "I2C RTC: power-on time 2000-01-01 00:00:00, a Saturday (day 7)");
    rtc_write(b, 0, (const unsigned[]){0x10}, 1);
    run(b, 2000000000000ull + 1000);
    rtc_read(b, 0, t, 1);
    check(t[0] == 0x12, "I2C RTC: 2 simulated seconds later the seconds read +2");
    rtc_write(b, 0, (const unsigned[]){0x58, 0x59, 0x23, 0x04, 0x28, 0x02, 0x24}, 7);
    run(b, 2000000000000ull + 1000);
    rtc_read(b, 0, t, 7);
    check(t[0] == 0x00 && t[1] == 0x00 && t[2] == 0x00 && t[3] == 0x05 && t[4] == 0x29 && t[5] == 0x02,
          "I2C RTC: 59 -> 00 carries into minutes, hours, the day; 2024-02-28 -> 02-29 (leap year)");
    rtc_write(b, 0, (const unsigned[]){0x58, 0x59, 0x23, 0x04, 0x28, 0x02, 0x23}, 7);
    run(b, 2000000000000ull + 1000);
    rtc_read(b, 0, t, 7);
    check(t[4] == 0x01 && t[5] == 0x03 && t[6] == 0x23, "I2C RTC: 2023-02-28 -> 03-01");
    rtc_write(b, 0, (const unsigned[]){0x80 | 0x30}, 1);
    run(b, 3000000000000ull);
    rtc_read(b, 0, t, 1);
    check(t[0] == 0xB0, "I2C RTC: the oscillator stop bit halts counting");
    rtc_write(b, 0, (const unsigned[]){0x30}, 1);
    run(b, 1000000000000ull + 1000);
    rtc_read(b, 0, t, 1);
    check(t[0] == 0x31, "I2C RTC: clearing the stop bit restarts counting");
    close_bench(b);
}

/* ---- parallel buses --------------------------------------------------------------------------- */

static const char *const AD[] = {"AD7", "AD6", "AD5", "AD4", "AD3", "AD2", "AD1", "AD0"};

static void ad_drive(Bench *b, unsigned v) {
    size_t i;

    for (i = 0; i < 8; i++) {
        set(b, AD[i], (v >> (7 - i)) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void ad_release(Bench *b) {
    size_t i;

    for (i = 0; i < 8; i++) {
        set(b, AD[i], LOGIC_Z);
    }
}

static unsigned ad_value(Bench *b) {
    unsigned v = 0;
    size_t i;

    for (i = 0; i < 8; i++) {
        v = (v << 1) | (get(b, AD[i]) == LOGIC_1 ? 1u : 0u);
    }
    return v;
}

/* BQ3285-shape: AS latches the address from AD, then CS with DS (read) or R/W (write). */
static void mux_latch(Bench *b, unsigned addr) {
    ad_drive(b, addr);
    set(b, "AS", LOGIC_1);
    run(b, HALF);
    set(b, "AS", LOGIC_0);
    run(b, HALF);
    ad_release(b);
}

static void mux_write(Bench *b, unsigned addr, unsigned v) {
    mux_latch(b, addr);
    ad_drive(b, v);
    set(b, "CS", LOGIC_0);
    set(b, "R/W", LOGIC_0);
    run(b, HALF);
    set(b, "R/W", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    ad_release(b);
    run(b, HALF);
}

static unsigned mux_read(Bench *b, unsigned addr) {
    unsigned v;

    mux_latch(b, addr);
    set(b, "CS", LOGIC_0);
    set(b, "DS", LOGIC_0);
    run(b, HALF);
    v = ad_value(b);
    set(b, "DS", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    return v;
}

static void mux_rtc(void) {
    static const char *const pulls[] = {"INT", NULL};
    Bench *b = bench("MUXRTCX", pulls);

    set(b, "AS", LOGIC_0);
    set(b, "DS", LOGIC_1);
    set(b, "R/W", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run_to(b, 1000000000000ull - 100000000ull);
    check((mux_read(b, 10) & 0x80) != 0, "mux RTC: UIP is set in the 244 us before an update");
    run_to(b, 1000000000000ull + 1000000ull);
    check((mux_read(b, 10) & 0x80) == 0 && mux_read(b, 0) == 0x01,
          "mux RTC: UIP clears after the update (seconds 01); AS latches the address");
    /* 12-hour mode (HF = 0): 11:59:59 PM -> 12:00:00 AM, 11:59:59 AM -> 12:00:00 PM */
    mux_write(b, 11, 0x80);
    mux_write(b, 4, 0x80 | 0x11);
    mux_write(b, 2, 0x59);
    mux_write(b, 0, 0x59);
    mux_write(b, 11, 0x00);
    run(b, 1000000000000ull + 1000);
    check(mux_read(b, 4) == 0x12, "mux RTC: 12-hour mode, 11:59:59 PM + 1 s = 12 AM (PM bit 0)");
    mux_write(b, 4, 0x11);
    mux_write(b, 2, 0x59);
    mux_write(b, 0, 0x59);
    run(b, 1000000000000ull + 1000);
    check(mux_read(b, 4) == 0x92, "mux RTC: 11:59:59 AM + 1 s = 12 PM (PM bit set)");
    /* periodic interrupt at RS = 15 (500 ms) */
    mux_write(b, 10, 0x0F);
    mux_write(b, 11, 0x40);
    check(get(b, "INT") == LOGIC_1, "mux RTC: INT released before the periodic interval");
    run(b, 500000000000ull + 1000);
    check(get(b, "INT") == LOGIC_0, "mux RTC: INT asserts after the 500 ms periodic interval");
    check((mux_read(b, 12) & 0x40) != 0, "mux RTC: REGC shows PF");
    check(get(b, "INT") == LOGIC_1, "mux RTC: reading REGC clears PF and releases INT");
    /* alarm: hours/minutes don't care (11xxxxxx), seconds match */
    mux_write(b, 10, 0x00);
    mux_write(b, 0, 0x20);
    mux_write(b, 1, 0x22);
    mux_write(b, 3, 0xC0);
    mux_write(b, 5, 0xC0);
    mux_write(b, 11, 0x20);
    run(b, 1000000000000ull + 1000);
    check(get(b, "INT") == LOGIC_1, "mux RTC: no alarm at :21");
    run(b, 1000000000000ull);
    check(get(b, "INT") == LOGIC_0, "mux RTC: the alarm at :22 asserts INT");
    check((mux_read(b, 12) & 0x20) != 0, "mux RTC: REGC shows AF");
    /* battery-backed RAM in the same address map */
    mux_write(b, 14, 0xA5);
    mux_write(b, 127, 0x3C);
    check(mux_read(b, 14) == 0xA5 && mux_read(b, 127) == 0x3C, "mux RTC: RAM bytes 14 and 127 read back");
    close_bench(b);
}

static void addr_set(Bench *b, const char *const *pins, size_t n, unsigned addr) {
    size_t i;

    for (i = 0; i < n; i++) {
        set(b, pins[i], (addr >> (n - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void par_write(Bench *b, const char *const *apins, size_t an, const char *wr, unsigned addr,
                      unsigned v) {
    addr_set(b, apins, an, addr);
    ad_drive(b, v);
    set(b, "CS", LOGIC_0);
    set(b, wr, LOGIC_0);
    run(b, HALF);
    set(b, wr, LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    ad_release(b);
    run(b, HALF);
}

static unsigned par_read(Bench *b, const char *const *apins, size_t an, const char *rd, unsigned addr) {
    unsigned v;

    addr_set(b, apins, an, addr);
    set(b, "CS", LOGIC_0);
    set(b, rd, LOGIC_0);
    run(b, HALF);
    v = ad_value(b);
    set(b, rd, LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    return v;
}

static void latch_rtc(void) {
    static const char *const A4[] = {"A3", "A2", "A1", "A0"};
    Bench *b = bench("LATRTCX", NULL);

    set(b, "CS", LOGIC_1);
    set(b, "OE", LOGIC_1);
    set(b, "WE", LOGIC_1);
    run(b, HALF);
    par_write(b, A4, 4, "WE", 0, 0x00);
    par_write(b, A4, 4, "WE", 14, 0x08);
    run(b, 3000000000000ull + 1000);
    check(par_read(b, A4, 4, "OE", 0) == 0x00, "latch RTC: UTI freezes the read registers");
    par_write(b, A4, 4, "WE", 14, 0x00);
    check(par_read(b, A4, 4, "OE", 0) == 0x03, "latch RTC: counting continued (3 s) behind the latch");
    par_write(b, A4, 4, "WE", 14, 0x08);
    par_write(b, A4, 4, "WE", 0, 0x30);
    par_write(b, A4, 4, "WE", 14, 0x00);
    run(b, 1000);
    check(par_read(b, A4, 4, "OE", 0) == 0x30, "latch RTC: a write under UTI loads the clock when it clears");
    /* the watchdog: WD = 1 (100 us); WDI edges restart it */
    set(b, "WDI", LOGIC_0);
    run(b, HALF);
    check(get(b, "WDO") == LOGIC_1, "watchdog: WDO inactive while the watchdog is off (WD = 0)");
    par_write(b, A4, 4, "WE", 11, 0x20);
    run(b, 60000000ull);
    set(b, "WDI", LOGIC_1);
    run(b, 60000000ull);
    check(get(b, "WDO") == LOGIC_1, "watchdog: a WDI edge within 100 us keeps WDO inactive");
    run(b, 60000000ull);
    check(get(b, "WDO") == LOGIC_0, "watchdog: no edge for 100 us asserts WDO");
    set(b, "WDI", LOGIC_0);
    run(b, HALF);
    check(get(b, "WDO") == LOGIC_1, "watchdog: the next WDI edge releases WDO");
    close_bench(b);
}

static void timer_rtc(void) {
    static const char *const A5[] = {"A4", "A3", "A2", "A1", "A0"};
    static const char *const pulls[] = {"INTR", NULL};
    Bench *b = bench("TMRTCX", pulls);

    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    run(b, HALF);
    par_write(b, A5, 5, "WR", 20, 0x5A);
    par_write(b, A5, 5, "WR", 23, 0xC3);
    check(par_read(b, A5, 5, "RD", 20) == 0x5A && par_read(b, A5, 5, "RD", 23) == 0xC3,
          "timer RTC: RAM write and read back");
    par_write(b, A5, 5, "WR", 6, 0x01);
    run(b, 1000000000ull + 1000);
    check(get(b, "INTR") == LOGIC_0, "timer RTC: the 1 ms timer interrupt asserts INTR");
    check((par_read(b, A5, 5, "RD", 5) & 1u) != 0 && get(b, "INTR") == LOGIC_1,
          "timer RTC: reading the flag register clears it and releases INTR");
    run(b, 50000000000ull);
    check(par_read(b, A5, 5, "RD", 1) == 0x05, "timer RTC: hundredths count (50 ms -> 05)");
    close_bench(b);
}

int main(void) {
    if (!load_fixtures()) {
        return 1;
    }
    check(part_library_find(library, "I2CRTCX")->kind == PART_RTC, "the fixtures load as rtc parts");
    i2c_rtc();
    mux_rtc();
    latch_rtc();
    timer_rtc();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("rtc_test: all passed\n");
    return 0;
}
