/*
 * rtc_more_test (plan-68-M): rtc-kind paths rtc_test's fixtures do not reach
 * (models/rtc.c; planning/plan-24-data/schema.md § 10): the year rollover,
 * a written date of 0, the inhibit bit and a latch write that halts
 * counting, an alarm matching a unit the clock does not keep, a periodic
 * interrupt with a summary flag on an open-drain active-high pin, the
 * watchdog's flag and reset pulse, and circuit_rtc_time on a part that is
 * no RTC.
 */

#include "circuit.h"
#include "converter_bench.h"
#include "model_pins.h"

#define REG(NAME, ADDR, FIELDS) "{'name':'" NAME "','address':" #ADDR ",'width':8,'fields':[" FIELDS "]}"
#define F(NAME, HI, LO) "{'name':'" NAME "','bits':[" #HI "," #LO "]}"
#define POINTER "'protocol':{'frame':'fixed','commands':[{'in':'aaaaaaaa','action':'pointer','address':'a','increment':1}]," \
    "'read':'pointer'},"
#define TIME_FIELDS \
    "{'field':'SEC.S','unit':'second'},{'field':'MIN.M','unit':'minute'},{'field':'HR.H','unit':'hour'}," \
    "{'field':'DATE.D','unit':'date'},{'field':'MON.M','unit':'month'},{'field':'YEAR.Y','unit':'year','base':2000}"
#define TIME_REGS \
    REG("SEC", 0, F("S", 6, 0)) "," REG("MIN", 1, F("M", 6, 0)) "," REG("HR", 2, F("H", 5, 0)) "," \
    REG("DATE", 3, F("D", 5, 0)) "," REG("MON", 4, F("M", 4, 0)) "," REG("YEAR", 5, F("Y", 7, 0))

/* UIP with an inhibit bit, an alarm, a periodic interrupt and a watchdog. */
#define RTCA \
    "{'id':'RTCA','function':'RTC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "SCL", "input") "," PIN(2, "SDA", "inout") "," PIN(3, "INT", "output") "," PIN(4, "SQW", "output") "," \
    PIN(5, "RST", "output") "," PIN(6, "VCC", "power") "," PIN(7, "GND", "ground") "," PIN(8, "NC", "nc") "]}," \
    "'vcc':[2,5.5],'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[104]},'registers':[" \
    TIME_REGS "," REG("CTL", 6, F("UIP", 7, 7) "," F("INH", 6, 6)) "," \
    REG("FLG", 7, F("AF", 0, 0) "," F("PF", 1, 1) "," F("IRQF", 2, 2) "," F("WF", 3, 3)) "," \
    REG("ASEC", 8, F("S", 6, 0)) "," REG("AWD", 9, F("D", 2, 0)) "," REG("WD", 10, F("T", 1, 0)) "]," POINTER \
    "'clock':{'fields':[" TIME_FIELDS "],'update':{'uip':'CTL.UIP','inhibit':'CTL.INH'}," \
    "'alarms':[{'match':[{'field':'ASEC.S','unit':'second'},{'field':'AWD.D','unit':'weekday'}]," \
    "'flag':'FLG.AF','pin':'INT'}]," \
    "'periodic':{'periodNs':250000,'flag':'FLG.PF','summary':'FLG.IRQF','pin':{'pin':'SQW','active':1,'openDrain':true}}," \
    "'watchdog':{'field':'WD.T','periods':{'1':1000000},'flag':'FLG.WF','resetPin':{'pin':'RST','pulse':{'1':100000}}}}," \
    PART("RTCAX", "T8")

/* A read latch with a write bit that halts counting; the minute register is 32 bits wide and its
   format (BCD or binary) is a control bit. */
#define RTCB \
    "{'id':'RTCB','function':'RTC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "SCL", "input") "," PIN(2, "SDA", "inout") "," PIN(3, "VCC", "power") "," PIN(4, "GND", "ground") "," \
    PIN(5, "NC", "nc") "," PIN(6, "NC", "nc") "]}," \
    "'vcc':[2,5.5],'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[105]},'registers':[" \
    REG("SEC", 0, F("S", 6, 0)) ",{'name':'MIN','address':1,'width':32,'fields':[{'name':'M','bits':[31,0]}]}," \
    REG("HR", 2, F("H", 5, 0)) "," REG("CTL", 3, F("R", 0, 0) "," F("W", 1, 1) "," F("B", 2, 2)) "]," POINTER \
    "'clock':{'fields':[{'field':'SEC.S','unit':'second'},{'field':'MIN.M','unit':'minute'," \
    "'format':{'field':'CTL.B','binary':1}},{'field':'HR.H','unit':'hour'}]," \
    "'update':{'latch':'CTL.R','write':'CTL.W'}}," PART("RTCBX", "T6")

/* Not an RTC: an ADC, for circuit_rtc_time. */
#define ADC \
    "{'id':'ADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DOUT", "output") "," PIN(4, "CS", "input") "," \
    PIN(5, "AIN", "input") "," PIN(6, "GND", "ground") "]},'type':'adc','bits':8,'channels':1," \
    "'codeFormat':'straight','vcc':[2.7,3.6],'bus':{'style':'spi','cs':'CS','sclk':'SCLK','miso':'DOUT','mode':0}," \
    "'protocol':{'frame':'fixed','out':'R{8}','commands':[]},'conversion':{'time':0,'start':['cs-fall']}," \
    PART("ADCX", "T6")

static Bench *rtc_bench(const char *part) {
    static const char *const ups[] = {"SDA", "SCL", NULL};
    Bench *b = bench(part, ups, NULL);

    b->scl = "SCL";
    b->sda = "SDA";
    run(b, HALF);
    return b;
}

static void rtc_write(Bench *b, unsigned address, unsigned reg, const unsigned *v, size_t n) {
    unsigned bytes[8];
    size_t i;

    bytes[0] = reg;
    for (i = 0; i < n; i++) {
        bytes[i + 1] = v[i];
    }
    i2c_tx(b, address, bytes, n + 1);
}

static unsigned rtc_read1(Bench *b, unsigned address, unsigned reg) {
    unsigned v = 0xFFFF;

    i2c_tx(b, address, &reg, 1);
    i2c_rx(b, address, &v, 1);
    return v;
}

/* Runs to simulated time t (ps). */
static void run_to(Bench *b, SimTime t) {
    run(b, t - sim_now(b->sim));
}

static void calendar(void) {
    Bench *b = rtc_bench("RTCAX");
    unsigned t[6] = {0};
    size_t i;

    rtc_write(b, 104, 0, (const unsigned[]){0x59, 0x59, 0x23, 0x31, 0x12, 0x99}, 6);
    run_to(b, 1500000000000ull);
    i2c_tx(b, 104, (const unsigned[]){0}, 1);
    i2c_rx(b, 104, t, 6);
    check(t[0] == 0 && t[1] == 0 && t[2] == 0 && t[3] == 1 && t[4] == 1 && t[5] == 0,
          "RTC: 2099-12-31 23:59:59 rolls over to (20)00-01-01 00:00:00");
    rtc_write(b, 104, 3, (const unsigned[]){0x00}, 1);
    run_to(b, 3200000000000ull);
    check(rtc_read1(b, 104, 3) == 0x01, "a written date of 0 counts as the 1st (shown at the next second)");
    rtc_write(b, 104, 6, (const unsigned[]){0x40}, 1);
    t[0] = rtc_read1(b, 104, 0);
    run_to(b, 5500000000000ull);
    check(rtc_read1(b, 104, 0) == t[0], "the inhibit bit (CTL.INH) halts counting");
    rtc_write(b, 104, 6, (const unsigned[]){0x00}, 1);
    /* the alarm matches a weekday the clock does not keep: it never fires */
    rtc_write(b, 104, 8, (const unsigned[]){0x00, 0x00}, 2);
    for (i = 0; i < 3; i++) {
        run_to(b, sim_now(b->sim) + 1000000000000ull);
    }
    check((rtc_read1(b, 104, 7) & 1u) == 0 && get(b, "INT") != LOGIC_0,
          "an alarm on a unit the clock does not keep (weekday) never matches");
    close_bench(b);
}

static void interrupts(void) {
    Bench *b = rtc_bench("RTCAX");
    const Part *part = part_library_find(library, "RTCAX");

    check(part_pin_open_drain(part, bpin(b, "SQW")) && part_pin_open_drain(part, bpin(b, "INT")) &&
              !part_pin_open_drain(part, bpin(b, "RST")),
          "open drain: the periodic pin (openDrain), the alarm pin named bare; not the reset pin");
    run_to(b, 300000000ull);
    check((rtc_read1(b, 104, 7) & 0x06) == 0x06, "the periodic interrupt sets its flag and the summary flag");
    check(get(b, "SQW") == LOGIC_Z, "an active-high open-drain pin asserted releases the line (Z)");
    check(get(b, "RST") == LOGIC_1, "the watchdog's reset pin is high (inactive) before it runs");
    rtc_write(b, 104, 10, (const unsigned[]){0x01}, 1);
    run_to(b, sim_now(b->sim) + 1050000000ull);
    check(get(b, "RST") == LOGIC_0 && (rtc_read1(b, 104, 7) & 0x08) != 0,
          "the watchdog expires after its 1 ms period: flag WF and the reset pin low");
    run_to(b, sim_now(b->sim) + 200000000ull);
    check(get(b, "RST") == LOGIC_1, "the reset pulse ends after 100 us");
    run_to(b, sim_now(b->sim) + 850000000ull);
    check(get(b, "RST") == LOGIC_0, "and the watchdog runs again from the pulse's end (1 ms later)");
    close_bench(b);
}

static void latch(void) {
    Bench *b = rtc_bench("RTCBX");
    unsigned m[4] = {1, 1, 1, 1};

    rtc_write(b, 105, 3, (const unsigned[]){0x02}, 1);
    run_to(b, 3500000000000ull);
    check(rtc_read1(b, 105, 0) == 0, "a set write bit (CTL.W, writeHalts) halts counting");
    rtc_write(b, 105, 3, (const unsigned[]){0x04}, 1);
    rtc_write(b, 105, 1, (const unsigned[]){0xFF, 0xFF, 0xFF, 0xFF}, 4);
    rtc_write(b, 105, 3, (const unsigned[]){0x00}, 1);
    run_to(b, 5200000000000ull);
    i2c_tx(b, 105, (const unsigned[]){1}, 1);
    i2c_rx(b, 105, m, 4);
    check(rtc_read1(b, 105, 0) == 1 && (m[0] | m[1] | m[2] | m[3]) == 0,
          "a binary minute of 0xFFFFFFFF (negative as a count) shows as BCD 0 once the format is BCD");
    close_bench(b);
}

static void not_an_rtc(void) {
    Circuit *c = circuit_create(library);
    size_t adc = 0, rtc = 0;
    RtcTime t;

    check(c != NULL && circuit_add_part(c, "ADCX", &adc) == 0 && circuit_add_part(c, "RTCBX", &rtc) == 0 &&
              circuit_start(c) == 0,
          "a circuit of an ADC and an RTC starts");
    check(circuit_rtc_time(c, adc, &t) == -1, "circuit_rtc_time: an ADC is not an RTC (-1)");
    check(circuit_rtc_time(c, rtc, &t) == 0 && t.year == 2000, "circuit_rtc_time: the RTC's time (2000)");
    circuit_destroy(c);
}

int main(void) {
    if (!load_converters("{'format':'schematical-parts','version':1,'kind':'rtc','devices':[" RTCA "," RTCB "]}")) {
        part_library_destroy(library);
        return 1;
    }
    {
        char *text = json(FILE_OF(ADC));
        char *error = NULL;

        check(load_parts(library, text, "adc.json", &error) == 0, "the ADC fixture loads");
        free(error);
        free(text);
    }
    calendar();
    interrupts();
    latch();
    not_an_rtc();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("rtc_more_test: all passed\n");
    return 0;
}
