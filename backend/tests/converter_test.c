/*
 * converter_test (plan-24-D §4, plan-24-E §3): the converter kind's engine on
 * fixture devices shaped like real ones (planning/plan-24-data/specs), driven
 * pin by pin from a test bench: SPI in modes 0-3, I2C with pull-ups, parallel
 * and clocked buses.
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

static const char *const FOOTPRINTS =
    "{'format':'schematical-footprints','version':1,'footprints':["
    DUAL("T6", "6") "," DUAL("T8", "8") "," DUAL("T10", "10") "," DUAL("T12", "12") ","
    DUAL("T16", "16") "," DUAL("T20", "20") "]}";

#define PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define PART(NAME, PINOUT, FP) "'parts':[{'name':'" NAME "','package':'SOIC','pinout':'" PINOUT \
    "','footprint':'" FP "'}]}"

/* ADS7042-shape: CS fall starts, 2 zeros then 12 result bits; no MOSI. */
#define FRAME_ADC(ID, BITS, FORMAT, OUT) \
    "{'id':'" ID "','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "DVDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "SDO", "output") "," \
    PIN(4, "CS", "input") "," PIN(5, "AIN", "input") "," PIN(6, "GND", "ground") "]}," \
    "'type':'adc','bits':" BITS ",'channels':1,'codeFormat':'" FORMAT "','vcc':[1.65,3.6]," \
    "'analog':['AIN'],'bus':{'style':'spi','cs':'CS','sclk':'SCLK','miso':'SDO','mode':0}," \
    "'timing':{'tValid':10,'tOut':30,'tFloat':5},'registers':[]," \
    "'protocol':{'frame':'fixed','out':'" OUT "','idle':'0','commands':[]}," \
    "'conversion':{'time':0,'start':['cs-fall']}," PART(ID "X", "p", "T6")

/* ADS1115-shape: I2C pointer registers, OS start, data-rate timing, ALERT/RDY. */
#define I2C_ADC \
    "{'id':'I2CADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "ADDR", "input") "," PIN(2, "ALERT/RDY", "output") "," PIN(3, "GND", "ground") "," \
    PIN(4, "AIN0", "input") "," PIN(5, "AIN1", "input") "," PIN(6, "VDD", "power") "," \
    PIN(7, "SDA", "inout") "," PIN(8, "SCL", "input") "]}," \
    "'type':'adc','bits':16,'channels':4,'codeFormat':'twos','vcc':[2,5.5],'analog':['AIN0','AIN1']," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':['ADDR'],'table':[72,73,75,74]}," \
    "'registers':[{'name':'CONVERSION','address':0,'width':16,'readOnly':true,'result':true}," \
    "{'name':'CONFIG','address':1,'width':16,'reset':34179,'readOnly':32768,'fields':[" \
    "{'name':'OS','bits':[15,15]},{'name':'MODE','bits':[8,8]},{'name':'DR','bits':[7,5]}," \
    "{'name':'COMP_QUE','bits':[1,0]}]}," \
    "{'name':'LO','address':2,'width':16,'reset':32768,'fields':[{'name':'MSB','bits':[15,15]}]}," \
    "{'name':'HI','address':3,'width':16,'reset':32767,'fields':[{'name':'MSB','bits':[15,15]}]}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'P','in':'xxxxxxaa','action':'pointer','address':'a'}]," \
    "'read':'pointer','generalCall':'reset'}," \
    "'conversion':{'time':{'field':'CONFIG.DR','values':{'4':7812500,'7':1162791}}," \
    "'start':['write:CONFIG.OS=1'],'mode':{'field':'CONFIG.MODE','continuous':0}," \
    "'busy':{'CONFIG.OS':0},'done':{'CONFIG.OS':1}}," \
    "'ready':[{'pin':'ALERT/RDY','active':0,'asserts':'done','clears':'start','openDrain':true," \
    "'enable':['CONFIG.MODE=1','HI.MSB=1','LO.MSB=0','CONFIG.COMP_QUE!=3']}]," PART("I2CADCX", "p", "T8")

/* ADS1256-shape: SPI mode 1 byte commands in a stream, DRDY, RDATA. */
#define CMD_ADC \
    "{'id':'CMDADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "DVDD", "power") "," PIN(2, "SCLK", "input") "," PIN(3, "DIN", "input") "," \
    PIN(4, "DOUT", "output") "," PIN(5, "CS", "input") "," PIN(6, "DRDY", "output") "," \
    PIN(7, "DGND", "ground") "," PIN(8, "AIN", "input") "]}," \
    "'type':'adc','bits':24,'channels':1,'codeFormat':'twos','vcc':[1.8,3.6],'analog':['AIN']," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','miso':'DOUT','mode':1}," \
    "'registers':[{'name':'STATUS','address':0,'width':8,'reset':48}," \
    "{'name':'MUX','address':1,'width':8,'reset':1},{'name':'ADCON','address':2,'width':8,'reset':32}," \
    "{'name':'DRATE','address':3,'width':8,'reset':240}]," \
    "'protocol':{'frame':'stream','commands':[" \
    "{'name':'WREG','in':'0101aaaa 0000nnnn','action':'wreg','address':'a','count':'n','countOffset':1}," \
    "{'name':'RREG','in':'0001aaaa 0000nnnn','action':'rreg','address':'a','count':'n','countOffset':1}," \
    "{'name':'RDATA','in':'00000001','reply':'R{24}'}," \
    "{'name':'WAKEUP','in':'00000000','action':'start'}," \
    "{'name':'RESET','in':'11111110','action':'reset'}]}," \
    "'conversion':{'time':50000,'start':['command']}," \
    "'ready':[{'pin':'DRDY','active':0,'asserts':'done','clears':'read'}]," PART("CMDADCX", "p", "T8")

/* DAC8562-shape: 24-bit frames latched at SYNC rise, LDAC and CLR pins; a read-back
   command (a fixture addition) shows a DAC register in the next frame. */
#define SPI_DAC \
    "{'id':'SPIDAC','function':'DAC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "VOUTA", "output") "," PIN(2, "VOUTB", "output") "," PIN(3, "GND", "ground") "," \
    PIN(4, "VREF", "power") "," PIN(5, "SYNC", "input") "," PIN(6, "SCLK", "input") "," \
    PIN(7, "DIN", "input") "," PIN(8, "VDD", "power") "," PIN(9, "LDAC", "input") "," \
    PIN(10, "CLR", "input") "," PIN(11, "SDO", "output") "," PIN(12, "NC", "nc") "]}," \
    "'type':'dac','bits':16,'channels':2,'vcc':[2.7,5.5],'analog':['VOUTA','VOUTB','VREF']," \
    "'bus':{'style':'spi','cs':'SYNC','sclk':'SCLK','mosi':'DIN','miso':'SDO','mode':1}," \
    "'registers':[{'name':'INA','space':'input','address':0,'width':16}," \
    "{'name':'INB','space':'input','address':1,'width':16}," \
    "{'name':'DACA','space':'dac','address':0,'width':16},{'name':'DACB','space':'dac','address':1,'width':16}]," \
    "'protocol':{'frame':'fixed','out':'z{24}','idle':'z','commands':[" \
    "{'name':'WRITE','in':'xx000aaa d{16}','action':'write','channel':'a','channelMap':{'7':[0,1]},'data':'d','commit':'end'}," \
    "{'name':'UPDATE','in':'xx001aaa x{16}','action':'update','channel':'a','channelMap':{'7':[0,1]},'commit':'end'}," \
    "{'name':'WRITEUPDATEALL','in':'xx010aaa d{16}','action':'write','channel':'a','data':'d','then':['updateall'],'commit':'end'}," \
    "{'name':'WRITEUPDATE','in':'xx011aaa d{16}','action':'write','channel':'a','data':'d','then':['update'],'commit':'end'}," \
    "{'name':'READBACK','in':'xx111aaa x{16}','action':'rreg','space':'dac','address':'a','next':'@ 0{8}','reply':'z{16}'}]}," \
    "'pins':{'ldac':{'pin':'LDAC','active':0,'edge':'fall'},'clear':{'pin':'CLR','active':0}}," \
    "'dac':{'input':['INA','INB'],'output':['DACA','DACB'],'clearCode':32768,'ldac':'pin'}," \
    PART("SPIDACX", "p", "T12")

/* DAC5571-shape: I2C, two bytes 00pp dddd dddd xxxx, read back. */
#define I2C_DAC \
    "{'id':'I2CDAC','function':'DAC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "VOUT", "output") "," PIN(2, "GND", "ground") "," PIN(3, "VDD", "power") "," \
    PIN(4, "SDA", "inout") "," PIN(5, "SCL", "input") "," PIN(6, "A0", "input") "]}," \
    "'type':'dac','bits':8,'channels':1,'vcc':[2.7,5.5],'analog':['VOUT']," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':['A0'],'table':[76,77,null,null]}," \
    "'registers':[{'name':'DAC','width':8},{'name':'CTRL','width':2,'fields':[{'name':'PD','bits':[1,0]}]}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'00ppdddd ddddxxxx','action':'write'," \
    "'channel':0,'data':'d','set':{'CTRL.PD':'p'}}],'read':'00[CTRL.PD][DAC]0000'}," \
    "'dac':{'input':['DAC'],'output':['DAC'],'clearCode':0,'ldac':'sync'}," PART("I2CDACX", "p", "T6")


/* plan-24-E: an ADS7816-style 12-bit parallel ADC: CONVST, BUSY, CS/RD reads. */
#define D12 PIN(1, "DB11", "output") "," PIN(2, "DB10", "output") "," PIN(3, "DB9", "output") "," \
    PIN(4, "DB8", "output") "," PIN(5, "DB7", "output") "," PIN(6, "DB6", "output") "," \
    PIN(7, "DB5", "output") "," PIN(8, "DB4", "output") "," PIN(9, "DB3", "output") "," \
    PIN(10, "DB2", "output") "," PIN(11, "DB1", "output") "," PIN(12, "DB0", "output")
#define D12_NAMES "'DB11','DB10','DB9','DB8','DB7','DB6','DB5','DB4','DB3','DB2','DB1','DB0'"
#define PAR_ADC \
    "{'id':'PARADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" D12 "," \
    PIN(13, "CS", "input") "," PIN(14, "RD", "input") "," PIN(15, "CONVST", "input") "," \
    PIN(16, "BUSY", "output") "," PIN(17, "VDD", "power") "," PIN(18, "GND", "ground") "," \
    PIN(19, "AIN", "input") "," PIN(20, "NC", "nc") "]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':'offset','vcc':[4.75,5.25],'analog':['AIN']," \
    "'bus':{'style':'parallel','data':[" D12_NAMES "],'cs':{'pin':'CS','active':0},'rd':{'pin':'RD','active':0}}," \
    "'timing':{'tAcc':30,'tDf':20},'protocol':{'frame':'fixed','commands':[],'read':'R{12}'}," \
    "'conversion':{'time':1000,'start':['pin']},'pins':{'convst':{'pin':'CONVST','edge':'fall'}}," \
    "'ready':[{'pin':'BUSY','active':1,'asserts':'busy'}]," PART("PARADCX", "p", "T20")

/* An 8-bit-bus 12-bit ADC with HBEN: high byte 0000 + bits 11-8, low byte bits 7-0. */
#define BYTE_ADC \
    "{'id':'BYTEADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "D7", "output") "," PIN(2, "D6", "output") "," PIN(3, "D5", "output") "," PIN(4, "D4", "output") "," \
    PIN(5, "D3", "output") "," PIN(6, "D2", "output") "," PIN(7, "D1", "output") "," PIN(8, "D0", "output") "," \
    PIN(9, "CS", "input") "," PIN(10, "RD", "input") "," PIN(11, "HBEN", "input") "," \
    PIN(12, "CONVST", "input") "," PIN(13, "VDD", "power") "," PIN(14, "GND", "ground") "," \
    PIN(15, "AIN", "input") "," PIN(16, "NC", "nc") "]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':'offset','vcc':[4.75,5.25],'analog':['AIN']," \
    "'bus':{'style':'parallel','data':['D7','D6','D5','D4','D3','D2','D1','D0'],'cs':{'pin':'CS','active':0}," \
    "'rd':{'pin':'RD','active':0},'byteSelect':{'pin':'HBEN','high':1}}," \
    "'protocol':{'frame':'fixed','commands':[],'read':{'high':'0000[R:11:8]','low':'[R:7:0]'}}," \
    "'conversion':{'time':0,'start':['pin']},'pins':{'convst':{'pin':'CONVST','edge':'fall'}}," \
    PART("BYTEADCX", "p", "T16")

/* A DAC7625-style double-buffered parallel DAC: R/W, A1 A0, LDAC copies all; reads
   at A1 = 1 show the DAC registers (a fixture addition, to observe them). */
#define D12D PIN(1, "D11", "inout") "," PIN(2, "D10", "inout") "," PIN(3, "D9", "inout") "," \
    PIN(4, "D8", "inout") "," PIN(5, "D7", "inout") "," PIN(6, "D6", "inout") "," \
    PIN(7, "D5", "inout") "," PIN(8, "D4", "inout") "," PIN(9, "D3", "inout") "," \
    PIN(10, "D2", "inout") "," PIN(11, "D1", "inout") "," PIN(12, "D0", "inout")
#define PAR_DAC \
    "{'id':'PARDAC','function':'DAC','description':'d','drawer':['Interface'],'pinouts':{'p':[" D12D "," \
    PIN(13, "A1", "input") "," PIN(14, "A0", "input") "," PIN(15, "CS", "input") "," \
    PIN(16, "R/W", "input") "," PIN(17, "LDAC", "input") "," PIN(18, "VDD", "power") "," \
    PIN(19, "GND", "ground") "," PIN(20, "VOUTA", "output") "]}," \
    "'type':'dac','bits':12,'channels':2,'vcc':[4.75,5.25],'analog':['VOUTA']," \
    "'bus':{'style':'parallel','data':['D11','D10','D9','D8','D7','D6','D5','D4','D3','D2','D1','D0']," \
    "'address':['A1','A0'],'cs':{'pin':'CS','active':0},'rw':{'pin':'R/W','read':1}}," \
    "'registers':[{'name':'IN0','space':'input','address':0,'width':12}," \
    "{'name':'IN1','space':'input','address':1,'width':12}," \
    "{'name':'DAC0','space':'dac','address':0,'width':12},{'name':'DAC1','space':'dac','address':1,'width':12}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'0a d{12}','action':'write','channel':'a','data':'d'}]," \
    "'read':{'0':'[IN0]','1':'[IN1]','2':'[DAC0]','3':'[DAC1]'}}," \
    "'pins':{'ldac':{'pin':'LDAC','active':0,'edge':'fall'}}," \
    "'dac':{'input':['IN0','IN1'],'output':['DAC0','DAC1'],'clearCode':0,'ldac':'pin'}," \
    PART("PARDACX", "p", "T20")

/* A clocked 12-bit pipelined ADC: CLK rising, OE, DRY following the clock. */
#define CLK_ADC \
    "{'id':'CLKADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" D12 "," \
    PIN(13, "CLK", "input") "," PIN(14, "OE", "input") "," PIN(15, "DRY", "output") "," \
    PIN(16, "VDD", "power") "," PIN(17, "GND", "ground") "," PIN(18, "AIN", "input") "," \
    PIN(19, "NC", "nc") "," PIN(20, "NC", "nc") "]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':'offset','vcc':[3,3.6],'analog':['AIN']," \
    "'bus':{'style':'clocked','clock':{'pin':'CLK','edge':'rise'},'data':[" D12_NAMES "]," \
    "'oe':{'pin':'OE','active':0},'clockOut':{'pin':'DRY','invert':false}},'timing':{'tOut':5}," \
    "'protocol':{'frame':'fixed','commands':[],'read':'R{12}'}," PART("CLKADCX", "p", "T20")

/* A delta-sigma modulator: one data pin, the 0 V stream 1010..., CLKOUT = CLK / 2. */
#define MOD_ADC \
    "{'id':'MODADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "VDD", "power") "," PIN(2, "CLK", "input") "," PIN(3, "DOUT", "output") "," \
    PIN(4, "CLKOUT", "output") "," PIN(5, "AINP", "input") "," PIN(6, "GND", "ground") "]}," \
    "'type':'adc','bits':1,'channels':1,'codeFormat':'straight','vcc':[4.5,5.5],'analog':['AINP']," \
    "'bus':{'style':'clocked','clock':{'pin':'CLK','edge':'rise'},'data':['DOUT']," \
    "'clockOut':[{'pin':'CLKOUT','divide':2}]},'protocol':{'frame':'fixed','commands':[],'read':'10'}," \
    PART("MODADCX", "p", "T6")

/* A clocked 8-bit ADC with a 3-wire SPI configuration port (SDATA shared). */
#define CFG_ADC \
    "{'id':'CFGADC','function':'ADC','description':'d','drawer':['Interface'],'pinouts':{'p':[" \
    PIN(1, "D7", "output") "," PIN(2, "D6", "output") "," PIN(3, "D5", "output") "," PIN(4, "D4", "output") "," \
    PIN(5, "D3", "output") "," PIN(6, "D2", "output") "," PIN(7, "D1", "output") "," PIN(8, "D0", "output") "," \
    PIN(9, "CLK", "input") "," PIN(10, "SEN", "input") "," PIN(11, "SCLK", "input") "," \
    PIN(12, "SDATA", "inout") "," PIN(13, "VDD", "power") "," PIN(14, "GND", "ground") "," \
    PIN(15, "AIN", "input") "," PIN(16, "NC", "nc") "]}," \
    "'type':'adc','bits':8,'channels':1,'codeFormat':'twos','vcc':[1.7,1.9],'analog':['AIN']," \
    "'bus':{'style':'clocked','clock':{'pin':'CLK','edge':'rise'},'data':['D7','D6','D5','D4','D3','D2','D1','D0']}," \
    "'configBus':{'style':'spi','cs':'SEN','sclk':'SCLK','mosi':'SDATA','miso':'SDATA','mode':0}," \
    "'registers':[{'name':'R0','address':0,'width':8},{'name':'R1','address':1,'width':8,'reset':3}]," \
    "'protocol':{'frame':'fixed','out':'z{16}','idle':'z','commands':[" \
    "{'name':'WRITE','in':'0aaaaaaa d{8}','action':'write','address':'a','data':'d'}," \
    "{'name':'READ','in':'1aaaaaaa','action':'rreg','address':'a'}],'read':'R{8}'}," \
    PART("CFGADCX", "p", "T16")

#define FILE_OF(DEVICES) "{'format':'schematical-parts','version':1,'kind':'converter','devices':[" DEVICES "]}"

static int load_fixtures(void) {
    char *fp = json(FOOTPRINTS);
    char *parts = json(FILE_OF(
        FRAME_ADC("FRAMEADC", "12", "straight", "00 R{12}") ","
        FRAME_ADC("OFFADC", "16", "offset", "R{16}") ","
        I2C_ADC "," CMD_ADC "," SPI_DAC "," I2C_DAC "," PAR_ADC "," BYTE_ADC "," PAR_DAC ","
        CLK_ADC "," MOD_ADC "," CFG_ADC));
    char *error = NULL;
    int ok;

    library = part_library_create();
    ok = load_footprints(library, fp, "fixtures.json", &error) == 0 &&
         load_parts(library, parts, "converter-fixtures.json", &error) == 0;
    if (!ok) {
        printf("FAIL loading the fixtures: %s\n", error ? error : "?");
        failures++;
    }
    free(error);
    free(fp);
    free(parts);
    return ok;
}

/* ---- The bench ------------------------------------------------------------------------------ */

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
    PinCount pins;
    PinCount pulls;
    unsigned mode;
    const char *cs, *sclk, *mosi, *miso, *scl, *sda;
} Bench;

enum { HALF = 100000 };     /* 100 ns: a 5 MHz bus */

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

/* pulls: pin names given a weak pull-up instead of a driver. */
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
        int pulled = 0;

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
            if (strcmp(pulls[k], b->part->pins[i].name) == 0) {
                sim_connect(b->sim, net, b->pu, k);
                pulled = 1;
            }
        }
        (void)pulled;
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

/* One SPI frame of n bits: out MSB first on MOSI; MISO bits into text ('0','1','X','Z'). */
static uint32_t spi(Bench *b, uint32_t out, unsigned n, char *text, int deselect) {
    LogicValue idle = (b->mode & 2u) ? LOGIC_1 : LOGIC_0;
    LogicValue active = idle == LOGIC_1 ? LOGIC_0 : LOGIC_1;
    uint32_t in = 0;
    unsigned i;

    set(b, b->sclk, idle);
    run(b, HALF);
    if (b->cs != NULL) {
        set(b, b->cs, LOGIC_0);
    }
    run(b, HALF);
    for (i = 0; i < n; i++) {
        LogicValue v;

        if (b->mode & 1u) {
            set(b, b->sclk, active);    /* the device shifts */
        }
        if (b->mosi != NULL) {
            set(b, b->mosi, (out >> (n - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        }
        run(b, HALF);
        v = b->miso != NULL ? get(b, b->miso) : LOGIC_Z;
        if (text != NULL) {
            text[i] = "01XZ"[v];
        }
        in = (in << 1) | (v == LOGIC_1 ? 1u : 0u);
        set(b, b->sclk, (b->mode & 1u) ? idle : active);   /* the device samples */
        run(b, HALF);
        if (!(b->mode & 1u)) {
            set(b, b->sclk, idle);
        }
    }
    if (text != NULL) {
        text[n] = '\0';
    }
    run(b, HALF);
    if (deselect && b->cs != NULL) {
        set(b, b->cs, LOGIC_1);
        run(b, HALF);
    }
    return in;
}

/* I2C: 1 releases the line (pull-up), 0 drives low. */
static void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static void i2c_start(Bench *b) {
    line(b, b->sda, 1);
    line(b, b->scl, 1);
    run(b, HALF);
    line(b, b->sda, 0);
    run(b, HALF);
    line(b, b->scl, 0);
    run(b, HALF);
}

static void i2c_stop(Bench *b) {
    line(b, b->sda, 0);
    run(b, HALF);
    line(b, b->scl, 1);
    run(b, HALF);
    line(b, b->sda, 1);
    run(b, HALF);
}

static LogicValue i2c_bit(Bench *b, int out) {
    LogicValue v;

    line(b, b->sda, out);
    run(b, HALF);
    line(b, b->scl, 1);
    run(b, HALF);
    v = get(b, b->sda);
    line(b, b->scl, 0);
    run(b, HALF);
    return v;
}

/* A byte out; 1 when the device ACKed. */
static int i2c_write(Bench *b, unsigned byte) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (byte >> i) & 1u);
    }
    return i2c_bit(b, 1) == LOGIC_0;
}

static unsigned i2c_read(Bench *b, int ack) {
    unsigned v = 0;
    int i;

    for (i = 0; i < 8; i++) {
        v = (v << 1) | (i2c_bit(b, 1) == LOGIC_1 ? 1u : 0u);
    }
    i2c_bit(b, !ack);
    return v;
}

/* Write `count` bytes to the 7-bit address; 1 when every byte was ACKed. */
static int i2c_tx(Bench *b, unsigned address, const unsigned *bytes, size_t count) {
    int ok;
    size_t i;

    i2c_start(b);
    ok = i2c_write(b, address << 1);
    for (i = 0; ok && i < count; i++) {
        ok = i2c_write(b, bytes[i]);
    }
    i2c_stop(b);
    return ok;
}

static int i2c_rx(Bench *b, unsigned address, unsigned *bytes, size_t count) {
    int ok;
    size_t i;

    i2c_start(b);
    ok = i2c_write(b, (address << 1) | 1u);
    for (i = 0; ok && i < count; i++) {
        bytes[i] = i2c_read(b, i + 1 < count);
    }
    i2c_stop(b);
    return ok;
}

/* ---- Cases ------------------------------------------------------------------------------------- */

static void frame_adc(void) {
    Bench *b = bench("FRAMEADCX", NULL);
    char text[40];

    b->cs = "CS";
    b->sclk = "SCLK";
    b->miso = "SDO";
    b->mode = 0;
    set(b, "CS", LOGIC_1);
    set(b, "SCLK", LOGIC_0);
    run(b, HALF);
    check(get(b, "SDO") == LOGIC_Z, "frame ADC: SDO is Z while CS is high");
    spi(b, 0, 16, text, 0);
    check(strcmp(text, "0000000000000000") == 0, "frame ADC: 2 zeros, 12 result bits (0 V = 0), then 0");
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    check(get(b, "SDO") == LOGIC_Z, "frame ADC: SDO floats after CS rises");
    close_bench(b);

    b = bench("OFFADCX", NULL);
    b->cs = "CS";
    b->sclk = "SCLK";
    b->miso = "SDO";
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    check(spi(b, 0, 16, NULL, 1) == 0x8000, "offset-binary ADC: 0 V reads 0x8000 (16 bits)");
    close_bench(b);
}

static void i2c_adc(void) {
    static const char *const pulls[] = {"SDA", "SCL", "ALERT/RDY", NULL};
    Bench *b = bench("I2CADCX", pulls);
    unsigned rx[2] = {0, 0};

    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "ADDR", LOGIC_0);
    run(b, HALF);
    check(i2c_tx(b, 0x48, (const unsigned[]){1}, 1) && i2c_rx(b, 0x48, rx, 2) &&
              rx[0] == 0x85 && rx[1] == 0x83,
          "I2C ADC: ADDR low = 0x48; CONFIG resets to 0x8583");
    check(!i2c_tx(b, 0x49, (const unsigned[]){1}, 1), "I2C ADC: a wrong address is NACKed");
    i2c_tx(b, 0x48, (const unsigned[]){1, 0x03, 0x83}, 3);
    i2c_rx(b, 0x48, rx, 2);
    check(rx[0] == 0x83 && rx[1] == 0x83, "I2C ADC: CONFIG write 0x0383 reads back 0x8383 (OS read-only)");
    i2c_tx(b, 0x48, (const unsigned[]){3, 0x80, 0x00}, 3);
    i2c_tx(b, 0x48, (const unsigned[]){2, 0x00, 0x00}, 3);
    check(get(b, "ALERT/RDY") == LOGIC_1, "I2C ADC: ALERT/RDY released before a conversion");
    i2c_tx(b, 0x48, (const unsigned[]){1, 0x83, 0x80}, 3);
    i2c_rx(b, 0x48, rx, 2);
    check(rx[0] == 0x03 && rx[1] == 0x80, "I2C ADC: OS=1 starts a single-shot conversion (OS reads 0)");
    run(b, 7812500000ull);
    i2c_rx(b, 0x48, rx, 2);
    check(rx[0] == 0x83 && rx[1] == 0x80, "I2C ADC: OS reads 1 after the 128 SPS data-rate time");
    check(get(b, "ALERT/RDY") == LOGIC_0, "I2C ADC: ALERT/RDY asserts (low) at the end of the conversion");
    i2c_tx(b, 0x48, (const unsigned[]){0}, 1);
    rx[0] = rx[1] = 0xFF;
    i2c_rx(b, 0x48, rx, 2);
    check(rx[0] == 0 && rx[1] == 0, "I2C ADC: the conversion register reads 0x0000 (0 V)");
    i2c_tx(b, 0x00, (const unsigned[]){0x06}, 1);
    i2c_tx(b, 0x48, (const unsigned[]){1}, 1);
    i2c_rx(b, 0x48, rx, 2);
    check(rx[0] == 0x85 && rx[1] == 0x83, "I2C ADC: the general-call reset restores CONFIG");
    close_bench(b);
}

static void cmd_adc(void) {
    Bench *b = bench("CMDADCX", NULL);
    char text[40];
    uint32_t v;

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    b->mode = 1;
    set(b, "CS", LOGIC_1);
    set(b, "SCLK", LOGIC_0);
    set(b, "DIN", LOGIC_0);
    run(b, HALF);
    /* WREG 3 registers from 1, then RREG them back */
    spi(b, 0x5102, 16, NULL, 0);
    spi(b, 0x232425, 24, NULL, 1);
    spi(b, 0x1102, 16, NULL, 0);
    v = spi(b, 0, 24, NULL, 1);
    check(v == 0x232425, "command ADC: WREG then RREG of 3 registers");
    check(get(b, "DRDY") == LOGIC_1, "command ADC: DRDY high before a conversion");
    spi(b, 0x00, 8, NULL, 1);
    run(b, 40000000ull);
    check(get(b, "DRDY") == LOGIC_1, "command ADC: DRDY still high during the conversion");
    run(b, 20000000ull);
    check(get(b, "DRDY") == LOGIC_0, "command ADC: DRDY low after the conversion time");
    spi(b, 0x01, 8, NULL, 0);
    spi(b, 0, 24, text, 1);
    check(strcmp(text, "000000000000000000000000") == 0, "command ADC: RDATA returns 24 zero bits");
    check(get(b, "DRDY") == LOGIC_1, "command ADC: reading the data releases DRDY");
    spi(b, 0xFE, 8, NULL, 1);
    spi(b, 0x1100, 16, NULL, 0);
    check(spi(b, 0, 8, NULL, 1) == 0x01, "command ADC: RESET restores MUX");
    /* a frame cut short: CS rises during WREG's data */
    spi(b, 0x5100, 16, NULL, 0);
    spi(b, 0x7, 4, NULL, 1);
    spi(b, 0x1100, 16, NULL, 0);
    check(spi(b, 0, 8, NULL, 1) == 0x01, "abort: CS rising mid-frame discards the write");
    close_bench(b);
}

static unsigned dac_read(Bench *b, unsigned ch) {
    spi(b, (0x38u | ch) << 16, 24, NULL, 1);
    /* the read-out frame carries command 100 (no command), not a write */
    return spi(b, 0x200000, 24, NULL, 1) >> 8;
}

static void spi_dac(void) {
    Bench *b = bench("SPIDACX", NULL);

    b->cs = "SYNC";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "SDO";
    b->mode = 1;
    set(b, "SYNC", LOGIC_1);
    set(b, "LDAC", LOGIC_1);
    set(b, "CLR", LOGIC_1);
    run(b, HALF);
    spi(b, 0x001234, 24, NULL, 1);
    check(dac_read(b, 0) == 0, "SPI DAC: a write goes to the input register only");
    set(b, "LDAC", LOGIC_0);
    run(b, HALF);
    set(b, "LDAC", LOGIC_1);
    run(b, HALF);
    check(dac_read(b, 0) == 0x1234, "SPI DAC: LDAC falling copies input to DAC");
    spi(b, 0x195678, 24, NULL, 1);
    check(dac_read(b, 1) == 0x5678, "SPI DAC: write-and-update loads DAC B at once");
    spi(b, 0x00AAAA, 24, NULL, 1);
    spi(b, 0x0F0000, 24, NULL, 1);
    check(dac_read(b, 0) == 0xAAAA, "SPI DAC: the software update command (all channels)");
    spi(b, 0x00, 20, NULL, 1);
    check(dac_read(b, 0) == 0xAAAA, "abort: a 20-bit frame (commit at SYNC rise) changes nothing");
    set(b, "CLR", LOGIC_0);
    run(b, HALF);
    set(b, "CLR", LOGIC_1);
    run(b, HALF);
    check(dac_read(b, 0) == 0x8000 && dac_read(b, 1) == 0x8000, "SPI DAC: CLR loads the clear code");
    close_bench(b);
}

static void i2c_dac(void) {
    static const char *const pulls[] = {"SDA", "SCL", NULL};
    Bench *b = bench("I2CDACX", pulls);
    unsigned rx[2] = {0, 0};

    b->scl = "SCL";
    b->sda = "SDA";
    set(b, "A0", LOGIC_1);
    run(b, HALF);
    check(i2c_tx(b, 0x4D, (const unsigned[]){0x1A, 0xB0}, 2), "I2C DAC: A0 high = 0x4D, write ACKed");
    check(i2c_rx(b, 0x4D, rx, 2) && rx[0] == 0x1A && rx[1] == 0xB0,
          "I2C DAC: read back 00 PD DAC 0000 (PD=01, DAC=0xAB)");
    close_bench(b);
}


/* ---- plan-24-E: parallel and clocked buses -------------------------------------------- */

static uint32_t bus_value(Bench *b, const char *const *pins, size_t n, int *z) {
    uint32_t v = 0;
    size_t i;

    *z = 1;
    for (i = 0; i < n; i++) {
        LogicValue l = get(b, pins[i]);

        if (l != LOGIC_Z) {
            *z = 0;
        }
        v = (v << 1) | (l == LOGIC_1 ? 1u : 0u);
    }
    return v;
}

static const char *const DB12[] = {"DB11", "DB10", "DB9", "DB8", "DB7", "DB6", "DB5", "DB4",
                                   "DB3", "DB2", "DB1", "DB0"};
static const char *const D8[] = {"D7", "D6", "D5", "D4", "D3", "D2", "D1", "D0"};
static const char *const D12P[] = {"D11", "D10", "D9", "D8", "D7", "D6", "D5", "D4", "D3",
                                   "D2", "D1", "D0"};

static void par_adc(void) {
    Bench *b = bench("PARADCX", NULL);
    int z;
    uint32_t v;

    set(b, "CS", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "CONVST", LOGIC_1);
    run(b, HALF);
    bus_value(b, DB12, 12, &z);
    check(z, "parallel ADC: data Z while not selected");
    check(get(b, "BUSY") == LOGIC_0, "parallel ADC: BUSY low before a conversion");
    set(b, "CONVST", LOGIC_0);
    run(b, 500000);
    check(get(b, "BUSY") == LOGIC_1, "parallel ADC: CONVST falling starts a conversion (BUSY high)");
    run(b, 600000);
    check(get(b, "BUSY") == LOGIC_0, "parallel ADC: BUSY low after the 1 us conversion");
    set(b, "RD", LOGIC_0);
    run(b, HALF);
    bus_value(b, DB12, 12, &z);
    check(z, "bus contention: RD without CS leaves the data Z");
    set(b, "CS", LOGIC_0);
    run(b, HALF);
    v = bus_value(b, DB12, 12, &z);
    check(!z && v == 0x800, "parallel ADC: CS and RD read the 0 V code (offset binary 0x800)");
    set(b, "RD", LOGIC_1);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    bus_value(b, DB12, 12, &z);
    check(z, "parallel ADC: data float after the read");
    close_bench(b);

    b = bench("BYTEADCX", NULL);
    set(b, "CS", LOGIC_0);
    set(b, "HBEN", LOGIC_1);
    set(b, "RD", LOGIC_0);
    run(b, HALF);
    v = bus_value(b, D8, 8, &z);
    check(v == 0x08, "byte ADC: HBEN high reads the high byte (0000 + bits 11-8)");
    set(b, "RD", LOGIC_1);
    run(b, HALF);
    set(b, "HBEN", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, HALF);
    check(bus_value(b, D8, 8, &z) == 0x00, "byte ADC: HBEN low reads the low byte");
    close_bench(b);
}

static void par_dac_write(Bench *b, unsigned addr, unsigned value) {
    size_t i;

    set(b, "A1", (addr >> 1) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A0", addr & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "R/W", LOGIC_0);
    for (i = 0; i < 12; i++) {
        set(b, D12P[i], (value >> (11 - i)) & 1u ? LOGIC_1 : LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_0);
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    for (i = 0; i < 12; i++) {
        set(b, D12P[i], LOGIC_Z);
    }
    set(b, "R/W", LOGIC_1);
    run(b, HALF);
}

static unsigned par_dac_read(Bench *b, unsigned addr) {
    int z;
    unsigned v;

    set(b, "A1", (addr >> 1) & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A0", addr & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "R/W", LOGIC_1);
    set(b, "CS", LOGIC_0);
    run(b, HALF);
    v = bus_value(b, D12P, 12, &z);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
    return z ? 0xFFFFFFFFu : v;
}

static void par_dac(void) {
    Bench *b = bench("PARDACX", NULL);

    set(b, "CS", LOGIC_1);
    set(b, "R/W", LOGIC_1);
    set(b, "LDAC", LOGIC_1);
    run(b, HALF);
    par_dac_write(b, 0, 0x123);
    par_dac_write(b, 1, 0x456);
    check(par_dac_read(b, 0) == 0x123 && par_dac_read(b, 1) == 0x456,
          "parallel DAC: CS/WR writes the input registers by address; R/W reads them back");
    check(par_dac_read(b, 2) == 0, "parallel DAC: the DAC registers wait for LDAC");
    set(b, "LDAC", LOGIC_0);
    run(b, HALF);
    set(b, "LDAC", LOGIC_1);
    run(b, HALF);
    check(par_dac_read(b, 2) == 0x123 && par_dac_read(b, 3) == 0x456,
          "parallel DAC: LDAC copies both input registers");
    close_bench(b);
}

static void clocked(void) {
    Bench *b = bench("CLKADCX", NULL);
    int z;
    uint32_t v;
    unsigned ones = 0, i;

    set(b, "OE", LOGIC_0);
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    bus_value(b, DB12, 12, &z);
    check(z, "clocked ADC: data Z before the first clock");
    set(b, "CLK", LOGIC_1);
    run(b, HALF);
    v = bus_value(b, DB12, 12, &z);
    check(!z && v == 0x800, "clocked ADC: a CLK rising edge drives the 0 V code (0x800)");
    check(get(b, "DRY") == LOGIC_1, "clocked ADC: DRY follows CLK");
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    check(get(b, "DRY") == LOGIC_0, "clocked ADC: DRY falls with CLK");
    set(b, "OE", LOGIC_1);
    run(b, HALF);
    bus_value(b, DB12, 12, &z);
    check(z, "clocked ADC: OE inactive floats the data");
    close_bench(b);

    b = bench("MODADCX", NULL);
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < 8; i++) {
        set(b, "CLK", LOGIC_1);
        run(b, HALF);
        ones += get(b, "DOUT") == LOGIC_1 ? (i % 2 == 0 ? 1u : 100u) : 0u;
        set(b, "CLK", LOGIC_0);
        run(b, HALF);
    }
    check(ones == 4, "modulator: the 0 V bit stream alternates 1, 0 (50 % ones)");
    {
        LogicValue a, c;

        set(b, "CLK", LOGIC_1);
        run(b, HALF);
        a = get(b, "CLKOUT");
        set(b, "CLK", LOGIC_0);
        run(b, HALF);
        set(b, "CLK", LOGIC_1);
        run(b, HALF);
        c = get(b, "CLKOUT");
        check(a != c && a != LOGIC_X && c != LOGIC_X, "modulator: CLKOUT = CLK / 2 toggles each rising edge");
    }
    close_bench(b);
}

/* 3-wire SPI: 8 bits out on SDATA, then (read) the pin released and 8 bits in. */
static unsigned sdio_frame(Bench *b, unsigned first, unsigned second, int read) {
    unsigned v = 0;
    int i;

    set(b, "SCLK", LOGIC_0);
    set(b, "SEN", LOGIC_0);
    run(b, HALF);
    for (i = 15; i >= 0; i--) {
        unsigned word = (first << 8) | second;

        if (read && i < 8) {
            set(b, "SDATA", LOGIC_Z);
        } else {
            set(b, "SDATA", (word >> i) & 1u ? LOGIC_1 : LOGIC_0);
        }
        run(b, HALF);
        v = (v << 1) | (get(b, "SDATA") == LOGIC_1 ? 1u : 0u);
        set(b, "SCLK", LOGIC_1);
        run(b, HALF);
        set(b, "SCLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "SEN", LOGIC_1);
    set(b, "SDATA", LOGIC_Z);
    run(b, HALF);
    return v & 0xFFu;
}

static void config_bus(void) {
    static const char *const pulls[] = {"SDATA", NULL};
    Bench *b = bench("CFGADCX", pulls);
    int z;

    set(b, "SEN", LOGIC_1);
    set(b, "CLK", LOGIC_0);
    run(b, HALF);
    check(sdio_frame(b, 0x81, 0, 1) == 0x03, "config bus: 3-wire SPI reads a register's reset value");
    sdio_frame(b, 0x01, 0x5A, 0);
    check(sdio_frame(b, 0x81, 0, 1) == 0x5A, "config bus: a register written over SPI reads back");
    set(b, "CLK", LOGIC_1);
    run(b, HALF);
    check(bus_value(b, D8, 8, &z) == 0 && !z, "config bus: the clocked data bus drives the code");
    close_bench(b);
}

int main(void) {
    if (!load_fixtures()) {
        return 1;
    }
    check(part_library_find(library, "FRAMEADCX")->kind == PART_CONVERTER &&
              strcmp(part_library_find(library, "SPIDACX")->converter->type, "dac") == 0 &&
              part_library_find(library, "I2CADCX")->converter->bits == 16,
          "the fixtures load as converter parts (type, bits)");
    frame_adc();
    i2c_adc();
    cmd_adc();
    spi_dac();
    i2c_dac();
    par_adc();
    par_dac();
    clocked();
    config_bus();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("converter_test: all passed\n");
    return 0;
}
