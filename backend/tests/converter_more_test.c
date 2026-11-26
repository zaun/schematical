/*
 * converter_more_test (plan-68-M): converter-kind paths the fixtures of
 * converter_test do not reach, on fixture devices driven pin by pin
 * (planning/plan-24-data/schema.md; docs/parts-format.md § converter):
 * stream resynchronisation, setbits / clearbits / stop / updateall / clear,
 * wide words, a two-wire SPI ADC, an I2C/SPI dual interface, I2C straps and
 * per-part addresses, and parallel / clocked bus corners. More in
 * converter_more2_test.
 */

#include "converter_bench.h"

#define STRM \
    HEADX("STRM", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DIN", "input") "," \
    PIN(4, "DOUT", "output") "," PIN(5, "CS", "input") "," PIN(6, "BUSY", "output") "," \
    PIN(7, "GND", "ground") "," PIN(8, "AIN", "input") "]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6],'analog':['AIN']," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','miso':'DOUT','mode':0}," \
    "'registers':[{'name':'R0','address':0,'width':8},{'name':'R1','address':1,'width':8,'reset':5}," \
    "{'name':'R2','address':2,'width':16}]," \
    "'protocol':{'frame':'stream','commands':[" \
    "{'in':'0101aaaa 0000nnnn','action':'wreg','address':'a','count':'n','countOffset':1}," \
    "{'in':'0001aaaa 0000nnnn','action':'rreg','address':'a','count':'n','countOffset':1}," \
    "{'in':'11111110','action':'reset'}," \
    "{'in':'11001100','action':'reset'}," \
    "{'in':'0010 dddd','action':'setbits','register':'R1','data':'d'}," \
    "{'in':'0011 dddd','action':'clearbits','register':'R1','data':'d'}," \
    "{'in':'0100 dddd','action':'setbits','data':'d'}," \
    "{'in':'10101010 11001100 1','action':'nop'}," \
    "{'in':'01100000','action':'wreg','register':'R2','wordBits':24}," \
    "{'in':'01100001','action':'stop'}," \
    "{'in':'01100010','action':'start'}," \
    "{'in':'01110000','action':'setbits','register':'R0','data':1,'when':'R=0'}," \
    "{'in':'01110001','action':'setbits','register':'R0','data':2,'when':'R=1'}," \
    "{'in':'01110010','action':'setbits','register':'R0','data':4,'when':'CH=0'}]}," \
    "'conversion':{'time':1000,'start':['command']}," \
    "'ready':[{'pin':'BUSY','active':1,'asserts':'busy'}]," PART("STRMX", "T8")

/* The same commands in fixed frames: one command per CS frame. */
#define FIXW \
    HEADX("FIXW", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DIN", "input") "," \
    PIN(4, "DOUT", "output") "," PIN(5, "CS", "input") "," PIN(6, "GND", "ground") "]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':{'pin':'DIN','values':{'1':'offset'}},'vcc':[2.7,3.6]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','miso':'DOUT','mode':0}," \
    "'registers':[{'name':'R0','address':0,'width':8},{'name':'R1','address':1,'width':8}]," \
    "'protocol':{'frame':'fixed','commands':[" \
    "{'in':'0101aaaa','action':'wreg','address':'a'}," \
    "{'in':'0001aaaa','action':'rreg','address':'a'}," \
    "{'in':'1111 0000','action':'nop','reply':'R{12}'}]}," \
    "'conversion':{'time':0,'start':['cs-fall']}," PART("FIXWX", "T6")

/* A DAC with update-all and clear commands, and a read-back. */
#define CDAC \
    HEADX("CDAC", "DAC") PIN(1, "VOUT", "output") "," PIN(2, "SYNC", "input") "," PIN(3, "SCLK", "input") "," \
    PIN(4, "DIN", "input") "," PIN(5, "SDO", "output") "," PIN(6, "VDD", "power") "]}," \
    "'type':'dac','bits':8,'channels':1,'vcc':[2.7,5.5],'analog':['VOUT']," \
    "'bus':{'style':'spi','cs':'SYNC','sclk':'SCLK','mosi':'DIN','miso':'SDO','mode':1}," \
    "'registers':[{'name':'IN','space':'input','address':0,'width':8},{'name':'OUT','space':'dac','address':0,'width':8}]," \
    "'protocol':{'frame':'fixed','out':'z{16}','idle':'z','commands':[" \
    "{'in':'0000 0000 d{8}','action':'write','channel':0,'data':'d'}," \
    "{'in':'0001 0000 x{8}','action':'updateall'}," \
    "{'in':'0010 0000 x{8}','action':'clear'}," \
    "{'in':'0011 0000','action':'rreg','space':'dac','address':0,'reply':'[OUT] R{8}'}]}," \
    "'dac':{'input':['IN'],'output':['OUT'],'clearCode':{'pin':'VOUT','values':{'1':255}},'ldac':'command'}," \
    PART("CDACX", "T6")

/* A two-wire SPI ADC (no CS): each finished conversion starts a frame (mode 0). */
#define TWO \
    HEADX("TWO", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DOUT", "output") "," \
    PIN(4, "GND", "ground") "," PIN(5, "AIN", "input") "," PIN(6, "NC", "nc") "]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':'offset','vcc':[2.7,3.6],'analog':['AIN']," \
    "'bus':{'style':'spi','sclk':'SCLK','miso':'DOUT','mode':0}," \
    "'protocol':{'frame':'fixed','out':'R{12}','idle':'0','commands':[]}," \
    "'conversion':{'time':10000,'start':['power-on'],'mode':'continuous'}," PART("TWOX", "T6")

/* An I2C/SPI dual interface: SEL high = SPI, low = I2C (pointer + data, reads at the pointer). */
#define DUALB \
    HEADX("DUALB", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCL", "input") "," PIN(3, "SDA", "inout") "," \
    PIN(4, "CS", "input") "," PIN(5, "SEL", "input") "," PIN(6, "GND", "ground") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'i2c-spi','selectPin':'SEL','spiLevel':1,'spi':{'cs':'CS','sclk':'SCL','mosi':'SDA','mode':0}," \
    "'i2c':{'scl':'SCL','sda':'SDA','address':[],'table':[40]}}," \
    "'registers':[{'name':'R0','address':0,'width':8},{'name':'R1','address':1,'width':8}]," \
    "'protocol':{'frame':'fixed','commands':[{'in':'aaaaaaaa','action':'pointer','address':'a','increment':1}]," \
    "'read':'pointer'},'conversion':{'time':0,'start':['read']}," PART("DUALBX", "T6")

/* An I2C ADC whose address straps are A0 (on the pinout) and A9 (tied inside: no pin), with a
   per-part address and per-part straps on two more parts; reads by an address map. */
#define STRAP \
    HEADX("STRAP", "ADC") PIN(1, "VDD", "power") "," PIN(2, "SCL", "input") "," PIN(3, "SDA", "inout") "," \
    PIN(4, "A0", "input") "," PIN(5, "RDY", "output") "," PIN(6, "GND", "ground") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':['A0','A9']," \
    "'table':[32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47]}," \
    "'registers':[{'name':'R0','address':0,'width':8,'fields':[{'name':'L','bits':[0,0]}]}]," \
    "'protocol':{'frame':'fixed','commands':[],'read':{'0':'10100101'}}," \
    "'conversion':{'time':0,'start':['read']},'pins':{'levels':[{'pin':'A9','field':'R0.L'}]}," \
    "'ready':[{'pin':'A9','active':0,'asserts':'busy'}]," \
    "'parts':[{'name':'STRAPX','package':'SOIC','pinout':'p','footprint':'T6'}," \
    "{'name':'STRAPF','package':'SOIC','pinout':'p','footprint':'T6','i2cAddress':80}," \
    "{'name':'STRAPP','package':'SOIC','pinout':'p','footprint':'T6','i2c':{'address':['RDY']," \
    "'table':[96,97,98,99]}}]}"

/* A clocked 8-bit ADC with an I2C configuration port; its SDA is open drain. */
#define CFGI \
    HEADX("CFGI", "ADC") PIN(1, "D0", "output") "," PIN(2, "D1", "output") "," PIN(3, "D2", "output") "," \
    PIN(4, "D3", "output") "," PIN(5, "CLK", "input") "," PIN(6, "SCL", "input") "," PIN(7, "SDA", "inout") "," \
    PIN(8, "GND", "ground") "]}," \
    "'type':'adc','bits':4,'channels':1,'codeFormat':'offset','vcc':[1.7,1.9]," \
    "'bus':{'style':'clocked','clock':{'pin':'CLK','edge':'rise'},'data':['D3','D2','D1','D0']}," \
    "'configBus':{'style':'i2c','scl':'SCL','sda':'SDA','address':['A9'],'table':[16,17,18,19]}," \
    "'registers':[{'name':'R0','address':0,'width':8,'reset':170}]," \
    "'protocol':{'frame':'fixed','commands':[{'in':'aaaaaaaa','action':'pointer','address':'a'}]," \
    "'read':{'0':'R{4}'},'reads':[{'when':'R0=170','read':'pointer'},{'when':'R0=1','read':{'9':'1111'}}]}," \
    PART("CFGIX", "T8")

/* A parallel ADC whose read is by pointer (set by a write), with an address pin. */
#define PPTR \
    HEADX("PPTR", "ADC") PIN(1, "D3", "inout") "," PIN(2, "D2", "inout") "," PIN(3, "D1", "inout") "," \
    PIN(4, "D0", "inout") "," PIN(5, "CS", "input") "," PIN(6, "RD", "input") "," PIN(7, "WR", "input") "," \
    PIN(8, "A0", "input") "]}," \
    "'type':'adc','bits':4,'channels':1,'codeFormat':'straight','vcc':[4.5,5.5]," \
    "'bus':{'style':'parallel','data':['D3','D2','D1','D0'],'address':['A0'],'cs':{'pin':'CS','active':0}," \
    "'rd':{'pin':'RD','active':0},'wr':{'pin':'WR','active':0}}," \
    "'registers':[{'name':'R0','address':0,'width':4,'reset':9},{'name':'R1','address':1,'width':4,'reset':6}]," \
    "'protocol':{'frame':'fixed','commands':[{'in':'a d{4}','action':'write','address':'a','data':'d'}]," \
    "'read':'pointer'},'conversion':{'time':0,'start':['read']}," PART("PPTRX", "T8")

static int load_fixtures(void) {
    return load_converters(FILE_OF(STRM "," FIXW "," CDAC "," TWO "," DUALB "," STRAP "," CFGI "," PPTR));
}

/* ---- Cases ------------------------------------------------------------------------------------- */

static Bench *strm_bench(void) {
    Bench *b = bench("STRMX", NULL, NULL);

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    set(b, "CS", LOGIC_1);
    set(b, "DIN", LOGIC_0);
    run(b, HALF);
    return b;
}

/* RREG of one register at address a: the next 8 or 16 bits. */
static uint32_t strm_read(Bench *b, unsigned a, unsigned bits) {
    spi(b, 0, 0x1000u | (a << 8), 16, 0);
    return spi(b, 0, 0, bits, 1);
}

static void stream_actions(void) {
    Bench *b = strm_bench();

    spi(b, 0, 0x25, 8, 1);
    check(strm_read(b, 1, 8) == 0x05, "stream: R1 resets to 5; setbits 5 on it keeps 5");
    spi(b, 0, 0x2A, 8, 1);
    check(strm_read(b, 1, 8) == 0x0F, "setbits ORs the data into the register (5 | 0xA = 0xF)");
    spi(b, 0, 0x33, 8, 1);
    check(strm_read(b, 1, 8) == 0x0C, "clearbits clears the data's bits (0xF & ~3 = 0xC)");
    spi(b, 0, 0x4F, 8, 1);
    check(strm_read(b, 0, 8) == 0 && strm_read(b, 1, 8) == 0x0C,
          "setbits naming no register changes nothing");
    spi(b, 0, 0x60, 8, 0);
    spi(b, 0, 0xABCD12, 24, 1);
    check(strm_read(b, 2, 16) == 0xABCD, "a 24-bit word into a 16-bit register keeps its top 16 bits");
    spi(b, 0, 0x70, 8, 1);
    spi(b, 0, 0x71, 8, 1);
    spi(b, 0, 0x72, 8, 1);
    check(strm_read(b, 0, 8) == 5, "when R=0 (the 0 V code) and CH=0 hold, R=1 does not (1 | 4)");
    spi(b, 0, 0x62, 8, 1);
    check(get(b, "BUSY") == LOGIC_1, "the start command starts a conversion (BUSY high)");
    spi(b, 0, 0x61, 8, 1);
    check(get(b, "BUSY") == LOGIC_0, "the stop command ends it before its 1 us (BUSY low)");
    run(b, 2000000);
    check(get(b, "BUSY") == LOGIC_0, "a stopped conversion does not finish later");
    /* resynchronising: no command matches 10101010 11001100 0, so the stream drops a byte and the
       rest, 11001100 0, matches the second reset at once */
    spi(b, 0, 0x5101, 16, 0);
    spi(b, 0, 0x77, 8, 1);
    check(strm_read(b, 1, 8) == 0x77, "WREG writes R1 = 0x77");
    spi(b, 0, 0x15598, 17, 1);
    check(strm_read(b, 1, 8) == 0x05, "a byte no command matches is dropped; the next bits reset the device");
    spi(b, 0, 0x5101, 16, 0);
    spi(b, 0, 0x33, 8, 1);
    spi(b, 0, 0xA9, 8, 0);
    spi(b, 0, 0xFE, 8, 1);
    check(strm_read(b, 1, 8) == 0x05, "after a dropped byte the stream matches a reset that follows");
    /* an undefined CS or SCLK spoils the frame */
    spi(b, 0, 0x5101, 16, 0);
    set(b, "CS", LOGIC_X);
    run(b, HALF);
    spi(b, 0, 0x99, 8, 1);
    check(strm_read(b, 1, 8) == 0x05, "CS going X mid-frame discards the write");
    spi(b, 0, 0x5101, 16, 0);
    set(b, "SCLK", LOGIC_X);
    run(b, HALF);
    spi(b, 0, 0x99, 8, 1);
    check(strm_read(b, 1, 8) == 0x05, "SCLK going X mid-frame discards the write");
    close_bench(b);
}

static void fixed_frames(void) {
    Bench *b = bench("FIXWX", NULL, NULL);

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    spi(b, 0, 0x5000u << 8 | 0x3C << 8 | 0xFF, 24, 1);
    spi(b, 0, 0x10, 8, 0);
    check(spi(b, 0, 0, 8, 1) == 0x3C, "fixed frame: WREG writes one register; the frame ends there");
    spi(b, 0, 0x11, 8, 0);
    check(spi(b, 0, 0, 8, 1) == 0, "the bits after the payload were ignored (R1 still 0)");
    set(b, "DIN", LOGIC_0);
    check(spi(b, 0, 0xF0, 8, 0) == 0 && spi(b, 0, 0, 12, 1) == 0x800,
          "codeFormat by a pin with no listed value (DIN low) uses the first: offset, 0x800");
    close_bench(b);
}

static void dac_commands(void) {
    Bench *b = bench("CDACX", NULL, NULL);

    b->cs = "SYNC";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "SDO";
    set(b, "SYNC", LOGIC_1);
    run(b, HALF);
    spi(b, 1, 0x005A, 16, 1);
    spi(b, 1, 0x30, 8, 0);
    check((spi(b, 1, 0, 16, 1) >> 8) == 0, "DAC: a write waits for an update (ldac by command)");
    spi(b, 1, 0x1000, 16, 1);
    spi(b, 1, 0x30, 8, 0);
    check(spi(b, 1, 0, 16, 1) == 0x5A00, "the update-all command loads the DAC register; R reads 0 on a DAC");
    spi(b, 1, 0x2000, 16, 1);
    spi(b, 1, 0x30, 8, 0);
    check((spi(b, 1, 0, 16, 1) >> 8) == 255, "the clear command loads the clear code (first value: 255)");
    close_bench(b);
}

/* Two-wire SPI: SCLK held high when a conversion ends, then clocked from its falling edge. */
static unsigned two_wire_frame(Bench *b, SimTime until, LogicValue *msb) {
    unsigned v = 0, i;

    set(b, "SCLK", LOGIC_1);
    run(b, until - sim_now(b->sim));
    set(b, "SCLK", LOGIC_0);
    run(b, HALF);
    *msb = get(b, "DOUT");
    for (i = 0; i < 11; i++) {
        set(b, "SCLK", LOGIC_1);
        run(b, HALF);
        set(b, "SCLK", LOGIC_0);
        run(b, HALF);
        v = (v << 1) | (get(b, "DOUT") == LOGIC_1);
    }
    return v;
}

static void two_wire(void) {
    Bench *b = bench("TWOX", NULL, NULL);
    LogicValue msb;

    check(two_wire_frame(b, 10100000, &msb) == 0 && msb == LOGIC_1,
          "two-wire SPI: the first falling edge shows the queued MSB, then 11 zeros (0x800)");
    check(two_wire_frame(b, 20100000, &msb) == 0 && msb == LOGIC_1,
          "the next conversion (continuous, 10 us) opens a new frame over the old one: 0x800 again");
    close_bench(b);
}

static void dual_interface(void) {
    static const char *const ups[] = {"SDA", "SCL", NULL};
    static const char *const sel_hi[] = {"SEL", NULL};
    Bench *b = bench("DUALBX", ups, sel_hi);
    unsigned rx[2] = {0, 0};

    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    check(i2c_tx(b, 40, (const unsigned[]){0, 0x12, 0x34}, 3) && i2c_tx(b, 40, (const unsigned[]){0}, 1) &&
              i2c_rx(b, 40, rx, 2) && rx[0] == 0x12 && rx[1] == 0x34,
          "I2C/SPI: SEL low (at start) is I2C: writes and reads at an auto-incrementing pointer");
    set(b, "SEL", LOGIC_1);
    run(b, HALF);
    check(!i2c_tx(b, 40, (const unsigned[]){0}, 1), "SEL high turns the I2C side off (no ACK)");
    b->cs = "CS";
    b->sclk = "SCL";
    b->mosi = "SDA";
    spi(b, 0, 0x0156, 16, 1);
    set(b, "SEL", LOGIC_0);
    set(b, "SDA", LOGIC_Z);
    set(b, "SCL", LOGIC_Z);
    run(b, HALF);
    check(i2c_tx(b, 40, (const unsigned[]){1}, 1) && i2c_rx(b, 40, rx, 1) && rx[0] == 0x56,
          "SEL high is SPI: an SPI pointer frame wrote R1 = 0x56");
    set(b, "SEL", LOGIC_1);
    run(b, HALF);
    spi(b, 0, 0x01, 8, 0);
    set(b, "SEL", LOGIC_0);
    set(b, "SDA", LOGIC_Z);
    set(b, "SCL", LOGIC_Z);
    run(b, HALF);
    check(i2c_tx(b, 40, (const unsigned[]){1, 0x77}, 2) && i2c_tx(b, 40, (const unsigned[]){1}, 1) &&
              i2c_rx(b, 40, rx, 1) && rx[0] == 0x77,
          "an SPI frame left open (CS low) when SEL turns to I2C ends at the I2C address: R1 = 0x77");
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    close_bench(b);
}

static void straps(void) {
    static const char *const ups[] = {"SDA", "SCL", NULL};
    Bench *b = bench("STRAPX", ups, NULL);
    unsigned rx[1] = {0};

    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "A0", LOGIC_1);
    run(b, HALF);
    check(i2c_rx(b, 33, rx, 1) && rx[0] == 0xA5,
          "a strap with no pin (A9) counts as tied low: A0 high = table[1] = 33; the read map's 0 entry");
    check(get(b, "RDY") == LOGIC_Z && get(b, "A0") == LOGIC_1, "a ready output on a missing pin drives nothing");
    set(b, "A0", LOGIC_Z);
    run(b, HALF);
    check(!i2c_rx(b, 33, rx, 1) && !i2c_rx(b, 32, rx, 1), "a floating strap matches no address (NACK)");
    close_bench(b);

    b = bench("STRAPF", ups, NULL);
    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "A0", LOGIC_1);
    run(b, HALF);
    check(i2c_rx(b, 80, rx, 1) && !i2c_rx(b, 33, rx, 1), "a part's i2cAddress replaces the strap table");
    close_bench(b);

    b = bench("STRAPP", ups, NULL);
    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "RDY", LOGIC_1);
    set(b, "A0", LOGIC_0);
    run(b, HALF);
    check(i2c_rx(b, 97, rx, 1) && !i2c_rx(b, 32, rx, 1), "a part's own straps and table (RDY high: tie 1 = 97)");
    close_bench(b);
}

static void config_i2c(void) {
    static const char *const ups[] = {"SDA", "SCL", NULL};
    static const char *const D[] = {"D3", "D2", "D1", "D0"};
    Bench *b = bench("CFGIX", ups, NULL);
    unsigned rx[1] = {0};
    LogicValue held;
    int z;

    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    check(i2c_tx(b, 16, (const unsigned[]){0}, 1) && i2c_rx(b, 16, rx, 1) && rx[0] == 0xAA,
          "I2C config port: the strap with no pin is low (address 16); R0 = 0xAA read by pointer");
    i2c_start(b);
    i2c_write(b, (16u << 1) | 1u);
    held = i2c_bit(b, 0);
    i2c_bit(b, 1);
    i2c_stop(b);
    check(held == LOGIC_0, "SDA is open drain: held low under the device's first 1 bit, no contention");
    set(b, "CLK", LOGIC_1);
    run(b, HALF);
    bus4(b, D, &z);
    check(z, "clocked bus: while the pointer-read alternative holds there is no word (data Z)");
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    i2c_tx(b, 16, (const unsigned[]){0, 1}, 2);
    i2c_tx(b, 16, (const unsigned[]){0}, 1);
    check(i2c_rx(b, 16, rx, 1) && rx[0] == 0, "R0 = 1: an address map with no entry for the read sends idle 0s");
    i2c_tx(b, 16, (const unsigned[]){0, 0x55}, 2);
    i2c_tx(b, 16, (const unsigned[]){0}, 1);
    check(i2c_rx(b, 16, rx, 1) && rx[0] == 0x88, "R0 = 0x55: the map's 0 entry R{4} (0x8, repeated) is read");
    set(b, "CLK", LOGIC_1);
    run(b, HALF);
    check(bus4(b, D, &z) == 0x8 && !z, "the clocked bus now drives R{4} = 0x8 (offset binary 0 V)");
    close_bench(b);
}

static void parallel_pointer(void) {
    static const char *const D[] = {"D3", "D2", "D1", "D0"};
    static const char *const low[] = {"CS", "RD", NULL};
    Bench *b = bench("PPTRX", NULL, low);
    int z;
    uint32_t v;

    set(b, "WR", LOGIC_1);
    set(b, "A0", LOGIC_0);
    run(b, HALF);
    v = bus4(b, D, &z);
    check(!z && v == 9, "parallel: CS and RD pulled low from time 0 read at the pointer (0: R0 = 9)");
    set(b, "RD", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    set(b, "A0", LOGIC_Z);
    set(b, "D3", LOGIC_0);
    set(b, "D2", LOGIC_0);
    set(b, "D1", LOGIC_1);
    set(b, "D0", LOGIC_1);
    set(b, "CS", LOGIC_0);
    set(b, "WR", LOGIC_0);
    run(b, HALF);
    set(b, "WR", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    set(b, "D3", LOGIC_Z);
    set(b, "D2", LOGIC_Z);
    set(b, "D1", LOGIC_Z);
    set(b, "D0", LOGIC_Z);
    set(b, "A0", LOGIC_0);
    set(b, "CS", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, HALF);
    check(bus4(b, D, &z) == 9, "a write with the address pin floating is ignored (R0 still 9)");
    close_bench(b);
}

int main(void) {
    if (!load_fixtures()) {
        part_library_destroy(library);
        return 1;
    }
    stream_actions();
    fixed_frames();
    dac_commands();
    two_wire();
    dual_interface();
    straps();
    config_i2c();
    parallel_pointer();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("converter_more_test: all passed\n");
    return 0;
}
