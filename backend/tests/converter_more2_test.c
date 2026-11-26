/*
 * converter_more2_test (plan-68-M): more converter-kind paths on fixture
 * devices (planning/plan-24-data/schema.md § conversion, § protocol): start
 * triggers (a write to a register, a mode field entering continuous, START
 * by edge and by level, CONVST without an edge), WREG with a zero count, a
 * pointer command's "then", read words wider than the register, a frame
 * longer than the bit buffer aligned to its end, and I2C reads with a
 * repeated start, an undefined address bit and no read pattern.
 */

#include "converter_bench.h"

/* Fixed SPI frames (mode 0); BUSY high while converting (1 us). */
#define TRIG \
    HEADX("TRIG", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DIN", "input") "," \
    PIN(4, "DOUT", "output") "," PIN(5, "CS", "input") "," PIN(6, "BUSY", "output") "," \
    PIN(7, "START", "input") "," PIN(8, "CONV", "input") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','miso':'DOUT','mode':0}," \
    "'registers':[{'name':'R0','address':0,'width':8,'fields':[{'name':'C','bits':[0,0]}]}," \
    "{'name':'R1','address':1,'width':8},{'name':'R2','address':2,'width':8}," \
    "{'name':'R3','address':3,'width':8}]," \
    "'protocol':{'frame':'fixed','commands':[" \
    "{'in':'0000aaaa d{8}','action':'write','address':'a','data':'d'}," \
    "{'in':'0001aaaa','action':'rreg','address':'a','wordBits':16}," \
    "{'in':'0010 nnnn','action':'wreg','register':'R2','count':'n'}," \
    "{'in':'0011 0000','action':'pointer','address':3,'then':['start']}]}," \
    "'conversion':{'time':1000,'start':['write:R1','pin'],'mode':{'field':'R0.C','continuous':1}}," \
    "'pins':{'start':{'pin':'START','edge':'rise'},'convst':{'pin':'CONV','active':1}}," \
    "'ready':[{'pin':'BUSY','active':1,'asserts':'busy'}]," PART("TRIGX", "T8")

/* START by level: conversions follow one another while it is high. */
#define LVL \
    HEADX("LVL", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DOUT", "output") "," \
    PIN(4, "CS", "input") "," PIN(5, "BUSY", "output") "," PIN(6, "START", "input") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','miso':'DOUT','mode':0}," \
    "'protocol':{'frame':'fixed','out':'R{8}','commands':[]}," \
    "'conversion':{'time':1000,'start':['pin']},'pins':{'start':{'pin':'START','edge':'level'}}," \
    "'ready':[{'pin':'BUSY','active':1,'asserts':'busy'}]," PART("LVLX", "T6")

/* align "end": a frame matches on its last bits, however long it is. */
#define ALIGN \
    HEADX("ALIGN", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DIN", "input") "," \
    PIN(4, "CS", "input") "," PIN(5, "BUSY", "output") "," PIN(6, "GND", "ground") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','mode':0}," \
    "'registers':[{'name':'R0','address':0,'width':8}]," \
    "'protocol':{'frame':'fixed','align':'end','commands':[{'in':'1010 d{8}','action':'write','register':'R0','data':'d'}]}," \
    "'conversion':{'time':1000,'start':['write:R0=90']}," \
    "'ready':[{'pin':'BUSY','active':1,'asserts':'busy'}]," PART("ALIGNX", "T6")

/* I2C with a pointer and no read pattern at all; a conversion has no time (0). */
#define NORD \
    HEADX("NORD", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCL", "input") "," PIN(3, "SDA", "inout") "," \
    PIN(4, "GND", "ground") "," PIN(5, "AIN", "input") "," PIN(6, "NC", "nc") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[72]}," \
    "'registers':[{'name':'R0','address':0,'width':8,'reset':66},{'name':'R1','address':1,'width':8,'reset':99}]," \
    "'protocol':{'frame':'fixed','commands':[{'in':'aaaaaaaa','action':'pointer','address':'a'}]}," \
    "'conversion':{'start':['cs-fall']}," PART("NORDX", "T6")

/* The same with reads at the pointer. */
#define RPT \
    HEADX("RPT", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCL", "input") "," PIN(3, "SDA", "inout") "," \
    PIN(4, "GND", "ground") "," PIN(5, "AIN", "input") "," PIN(6, "NC", "nc") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[73]}," \
    "'registers':[{'name':'R0','address':0,'width':8,'reset':66},{'name':'R1','address':1,'width':8,'reset':99}]," \
    "'protocol':{'frame':'fixed','commands':[{'in':'aaaaaaaa','action':'pointer','address':'a'}],'read':'pointer'}," \
    "'conversion':{'time':0,'start':['read']}," PART("RPTX", "T6")

/* A one-digit shared-pin LED driver (MAX6950-shaped, plan-56-D) whose blink half period is a
   plain number of microseconds: plane P0 in the first half of each period, P1 in the second. */
#define BLINK \
    "{'format':'schematical-parts','version':1,'kind':'leddriver','devices':[" \
    "{'id':'BLINK','function':'driver','description':'d','drawer':['Displays & Indicators'],'pinouts':{'p':[" \
    PIN(1, "CS", "input") "," PIN(2, "SCLK", "input") "," PIN(3, "DIN", "input") "," PIN(4, "S0", "output") "," \
    PIN(5, "S1", "output") "," PIN(6, "S2", "output") "," PIN(7, "S3", "output") "," PIN(8, "S4", "output") "," \
    PIN(9, "S5", "output") "," PIN(10, "S6", "output") "," PIN(11, "S7", "output") "," PIN(12, "S8", "output") "," \
    PIN(13, "VCC", "power") "," PIN(14, "GND", "ground") "," PIN(15, "NC", "nc") "," PIN(16, "NC", "nc") "]}," \
    "'vcc':[4.5,5.5],'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','mode':0}," \
    "'registers':[{'name':'P0','address':1,'width':8},{'name':'P1','address':2,'width':8}," \
    "{'name':'CONFIG','address':3,'width':8,'fields':[{'name':'E','bits':[0,0]}]}]," \
    "'protocol':{'frame':'fixed','commands':[{'in':'aaaaaaaa dddddddd','action':'write','address':'a','data':'d'}]}," \
    "'display':{'driver':'digits','digits':1,'sharedPins':['S0','S1','S2','S3','S4','S5','S6','S7','S8']," \
    "'digitActive':'low','segmentActive':'high','segmentLines':['a','b','c','d','e','f','g','dp']," \
    "'ram':['P0'],'scan':{'periodUs':100},'blink':{'enable':'CONFIG.E=1','ram':['P1'],'halfPeriodUs':1000}}," \
    "'parts':[{'name':'BLINKX','package':'SOIC','pinout':'p','footprint':'T16'}]}]}"

/* The eight segment lines S1..S8 now, one bit each (1 = high). */
static unsigned segs(Bench *b) {
    static const char *const S[] = {"S1", "S2", "S3", "S4", "S5", "S6", "S7", "S8"};
    unsigned v = 0, i;

    for (i = 0; i < 8; i++) {
        v = (v << 1) | (get(b, S[i]) == LOGIC_1);
    }
    return v;
}

/* The segment lines at simulated time t (ps from 0). */
static unsigned segs_at(Bench *b, SimTime t) {
    run(b, t - sim_now(b->sim));
    return segs(b);
}

static void blink(void) {
    char *text = json(BLINK);
    char *error = NULL;
    Bench *b;
    unsigned p0, p1;

    if (load_parts(library, text, "blink.json", &error) != 0) {
        check(0, error != NULL ? error : "loading the blink fixture");
    }
    free(error);
    free(text);
    b = bench("BLINKX", NULL, NULL);
    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    spi(b, 0, 0x0181, 16, 1);
    spi(b, 0, 0x0242, 16, 1);
    spi(b, 0, 0x0301, 16, 1);
    p0 = segs_at(b, 500000000ull);
    p1 = segs_at(b, 1500000000ull);
    check(get(b, "S0") == LOGIC_0 && p0 != p1 && p0 != 0 && p1 != 0,
          "blink 'halfPeriodUs': 1000 (a number): plane P0 in 0-1 ms, P1 in 1-2 ms, digit 0 selected");
    check(segs_at(b, 2500000000ull) == p0 && segs_at(b, 3500000000ull) == p1, "and again in 2-3 ms, 3-4 ms");
    spi(b, 0, 0x0300, 16, 1);
    check(segs_at(b, 5500000000ull) == p0, "blink off (E = 0): plane P0 in the second half too");
    close_bench(b);
}

static Bench *trig_bench(void) {
    Bench *b = bench("TRIGX", NULL, NULL);

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    set(b, "CS", LOGIC_1);
    set(b, "START", LOGIC_0);
    set(b, "CONV", LOGIC_0);
    run(b, HALF);
    return b;
}

/* RREG: a 16-bit word, the register in its top 8 bits (wordBits, docs/parts-format.md). */
static unsigned trig_word(Bench *b, unsigned a) {
    spi(b, 0, 0x10u | a, 8, 0);
    return spi(b, 0, 0, 16, 1);
}

static unsigned trig_read(Bench *b, unsigned a) {
    return trig_word(b, a) >> 8;
}

/* BUSY now, and 2 us later. */
static int busy_then_done(Bench *b) {
    int now = get(b, "BUSY") == LOGIC_1;

    run(b, 2000000);
    return now && get(b, "BUSY") == LOGIC_0;
}

static void triggers(void) {
    Bench *b = trig_bench();

    check(get(b, "BUSY") == LOGIC_0, "TRIG: idle, BUSY low");
    spi(b, 0, 0x013C, 16, 1);
    check(busy_then_done(b), "a write to R1 ('write:R1', any value) starts a 1 us conversion");
    check(trig_word(b, 1) == 0x3C00, "a 16-bit read word of an 8-bit register: R1 in the top bits, zeros below");
    spi(b, 0, 0x0201, 16, 1);
    check(busy_then_done(b) == 0 && trig_read(b, 2) == 0x01, "a write to R2 starts nothing");
    spi(b, 0, 0x20FF, 16, 1);
    check(trig_read(b, 2) == 0x01, "WREG with a count of 0 writes nothing");
    spi(b, 0, 0x21A5, 16, 1);
    check(trig_read(b, 2) == 0xA5, "WREG with a count of 1 writes R2");
    spi(b, 0, 0x30A5, 16, 1);
    check(busy_then_done(b) && trig_read(b, 3) == 0xA5,
          "a pointer command's 'then' (start) runs after its data byte is written (R3 = 0xA5)");
    spi(b, 0, 0x0001, 16, 1);
    run(b, 1500000);
    check(get(b, "BUSY") == LOGIC_1, "R0.C = 1 enters continuous mode: conversions follow one another");
    spi(b, 0, 0x0000, 16, 1);
    run(b, 2000000);
    check(get(b, "BUSY") == LOGIC_0, "R0.C = 0 leaves it: the conversion in progress ends, no new one");
    set(b, "START", LOGIC_1);
    run(b, HALF);
    check(busy_then_done(b), "START rising (edge rise) starts a conversion");
    set(b, "START", LOGIC_0);
    run(b, HALF);
    check(get(b, "BUSY") == LOGIC_0, "START falling starts none");
    set(b, "CONV", LOGIC_1);
    run(b, HALF);
    check(busy_then_done(b), "CONVST active high with no edge given starts on its rising edge");
    set(b, "CONV", LOGIC_0);
    run(b, HALF);
    check(get(b, "BUSY") == LOGIC_0, "CONVST falling starts none");
    close_bench(b);

    b = bench("LVLX", NULL, NULL);
    set(b, "CS", LOGIC_1);
    set(b, "START", LOGIC_0);
    run(b, HALF);
    set(b, "START", LOGIC_1);
    run(b, 1500000);
    check(get(b, "BUSY") == LOGIC_1, "START by level: while it is high a conversion follows the last one");
    set(b, "START", LOGIC_0);
    run(b, 1500000);
    check(get(b, "BUSY") == LOGIC_0, "START low: the conversion in progress completes, no new one");
    close_bench(b);
}

static void align_end(void) {
    Bench *b = bench("ALIGNX", NULL, NULL);
    unsigned i;
    const unsigned tail = 0xA5A;    /* 1010, then 0x5A = 90 */

    set(b, "CS", LOGIC_1);
    set(b, "SCLK", LOGIC_0);
    run(b, HALF);
    set(b, "CS", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < 4100 + 12; i++) {
        set(b, "DIN", i >= 4100 && ((tail >> (4111 - i)) & 1u) ? LOGIC_1 : LOGIC_0);
        run(b, HALF / 2);
        set(b, "SCLK", LOGIC_1);
        run(b, HALF / 2);
        set(b, "SCLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    check(get(b, "BUSY") == LOGIC_1,
          "align end: a 4112-bit frame (past the 4096-bit buffer) matches on its last 12 bits (R0 = 90 starts)");
    close_bench(b);
}

static void i2c_reads(void) {
    static const char *const ups[] = {"SDA", "SCL", NULL};
    Bench *b = bench("NORDX", ups, NULL);
    unsigned rx[1] = {0xFF};
    LogicValue ack;
    int i;

    b->scl = "SCL";
    b->sda = "SDA";
    check(i2c_rx(b, 72, rx, 1) && rx[0] == 0x00, "I2C with no read pattern: a read sends idle 0s");
    i2c_start(b);
    for (i = 7; i >= 0; i--) {
        if (i == 3) {
            set(b, "SDA", LOGIC_X);
            run(b, HALF);
            line(b, "SCL", 1);
            run(b, HALF);
            line(b, "SCL", 0);
            run(b, HALF);
        } else {
            i2c_bit(b, ((72u << 1) >> i) & 1u);
        }
    }
    ack = i2c_bit(b, 1);
    i2c_stop(b);
    check(ack == LOGIC_X, "an address byte with an undefined bit gets an undefined acknowledge");
    close_bench(b);

    b = bench("RPTX", ups, NULL);
    b->scl = "SCL";
    b->sda = "SDA";
    i2c_start(b);
    i2c_write(b, 73u << 1);
    i2c_write(b, 1);
    i2c_start(b);
    i2c_write(b, (73u << 1) | 1u);
    rx[0] = i2c_read(b, 0);
    i2c_stop(b);
    check(rx[0] == 99, "a repeated start ends the write (pointer = 1) and the read is at the pointer (R1 = 99)");
    close_bench(b);
}

int main(void) {
    if (!load_converters(FILE_OF(TRIG "," LVL "," ALIGN "," NORD "," RPT))) {
        part_library_destroy(library);
        return 1;
    }
    triggers();
    align_end();
    i2c_reads();
    blink();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("converter_more2_test: all passed\n");
    return 0;
}
