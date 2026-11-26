/*
 * library_check: split from library_check.c by plan-68-H; the shared helpers are
 * support/library_check_bench.c.
 */

#include "library_check_bench.h"

/* plan-12-A: "pull": true makes a pull part; 0 V is a pull-down. */
static void check_valid_pull(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *pd;
    const Part *vcc;

    CHECK(load(library,
               HEAD("supply") VCC_DEVICE ","
               "{'id':'PULLDOWN','function':'Pull-down','description':'d','drawer':[],"
               "'pinouts':{'pin':[{'number':1,'name':'PD','role':'output'}]},'volts':0,'pull':true,"
               "'parts':[{'name':'PULLDOWN','package':'0603','pinout':'pin','footprint':'T2',"
               "'footprintChoices':[{'package':'0603','footprint':'T2'}]}]}" TAIL,
               &error) == 0);
    CHECK(error == NULL);
    pd = part_library_find(library, "PULLDOWN");
    vcc = part_library_find(library, "5V");
    CHECK(pd != NULL && vcc != NULL);
    if (pd != NULL && vcc != NULL) {
        CHECK(pd->kind == PART_SUPPLY && pd->supply->pull == 1 && pd->supply->level == 0 &&
              pd->supply->mv == 0 && pd->pins[0].role == PIN_OUTPUT);
        CHECK(vcc->supply->pull == 0);
        /* plan-26-A: a pull is a resistor; its 2-pad footprint's pad 2 is the rail end. */
        CHECK(pd->footprint != NULL && strcmp(pd->footprint->id, "T2") == 0);
        CHECK(pd->footprint_choice_count == 1);
        CHECK(vcc->footprint == NULL);
    }
    free(error);
    part_library_destroy(library);
    printf("ok   valid pull part\n");

#define PULL_DEVICE(PART_KEYS) \
    HEAD("supply") \
    "{'id':'PULLDOWN','function':'Pull-down','description':'d','drawer':[]," \
    "'pinouts':{'pin':[{'number':1,'name':'PD','role':'output'}]},'volts':0,'pull':true," \
    "'parts':[{'name':'PULLDOWN','package':'0603','pinout':'pin'" PART_KEYS "}]}" TAIL
    expect_fail("pull part without footprint", PULL_DEVICE(""),
                "parts[0] (PULLDOWN): missing \"footprint\"");
    expect_fail("pull part with connectorFootprint",
                PULL_DEVICE(",'footprint':'T2','connectorFootprint':'T2'"),
                "parts[0] (PULLDOWN): a pull part has no connectorFootprint");
    expect_fail("pull part footprint with fewer pads than pins",
                PULL_DEVICE(",'footprint':'BGA2'"),
                "pin \"1\" has no pad in footprint \"BGA2\"");
#undef PULL_DEVICE
}

/* plan-12-C: the serial kind, on the PC16550D fixture keys (serial_fixture.h). */
#define SERIAL_HEAD(EXTRA) \
    HEAD("serial") "{'id':'U','function':'UART','description':'d','drawer':['Interface']," \
    "'pinouts':{" SFX_DIP40 "}," EXTRA
#define SERIAL_PART(GRADE) \
    "'parts':[{'name':'U-DIP','package':'DIP','pinout':'dip40','footprint':'T40','grade':'" GRADE "'}]}" TAIL

static void check_serial(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library,
               SERIAL_HEAD("'model':'ns16550'," SFX_MODEL_KEYS("16", "") "," SFX_GRADES ",")
               SERIAL_PART("X"), &error) == 0);
    CHECK(error == NULL);
    part = part_library_find(library, "U-DIP");
    CHECK(part != NULL);
    if (part != NULL) {
        CHECK(part->kind == PART_SERIAL && part->serial != NULL &&
              strcmp(part->serial->model, "ns16550") == 0 && part->serial->channels == 1 &&
              strcmp(part->serial->grade, "X") == 0 &&
              strcmp(part->serial->host, "parallel") == 0 && part->vcc != NULL &&
              part->vcc->min_mv == 4500 && part->logic == NULL);
    }
    free(error);
    part_library_destroy(library);
    printf("ok   valid serial part\n");

    expect_fail("serial unknown model",
                SERIAL_HEAD("'model':'x','grades':{'X':{}},") SERIAL_PART("X"),
                "devices[0] (U): unknown model \"x\"");
    expect_fail("serial missing grades",
                SERIAL_HEAD("'model':'ns16550'," SFX_MODEL_KEYS("16", "") ",") SERIAL_PART("X"),
                "devices[0] (U): missing \"grades\"");
    expect_fail("serial grade not in grades",
                SERIAL_HEAD("'model':'ns16550'," SFX_MODEL_KEYS("16", "") "," SFX_GRADES ",")
                SERIAL_PART("Z"),
                "parts[0] (U-DIP): grade \"Z\" is not in \"grades\"");
    expect_fail("serial bad fifo",
                SERIAL_HEAD("'model':'ns16550'," SFX_MODEL_KEYS("12", "") "," SFX_GRADES ",")
                SERIAL_PART("X"),
                "devices[0] (U): \"fifo\" must be 0, 16, 32, 64, 128 or \"750\"");
}

/* plan-24-D: the converter kind's schema errors. */
#define CONV_DEVICE(REGS, COMMANDS) \
    HEAD("converter") "{'id':'C','function':'ADC','description':'d','drawer':['Interface']," \
    "'pinouts':{'p':[{'number':1,'name':'VDD','role':'power'},{'number':2,'name':'SCLK','role':'input'}," \
    "{'number':3,'name':'SDO','role':'output'},{'number':4,'name':'CS','role':'input'}," \
    "{'number':5,'name':'DIN','role':'input'},{'number':6,'name':'GND','role':'ground'}]}," \
    "'type':'adc','bits':12,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6],'analog':[]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','miso':'SDO','mode':0}," \
    "'registers':[" REGS "],'protocol':{'frame':'stream','commands':[" COMMANDS "]}," \
    "'conversion':{'time':0,'start':['cs-fall']}," \
    "'parts':[{'name':'C-1','package':'SOT-23','pinout':'p','footprint':'T6'}]}" TAIL
#define CONV_REG "{'name':'CFG','address':1,'width':8,'fields':[{'name':'A','bits':[3,0]}]}"

static void check_converter_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, CONV_DEVICE(CONV_REG,
                                    "{'name':'W','in':'0100aaaa','action':'wreg','address':'a'}"),
               &error) == 0);
    part = part_library_find(library, "C-1");
    CHECK(part != NULL && part->kind == PART_CONVERTER && part->converter != NULL &&
          strcmp(part->converter->type, "adc") == 0 && part->converter->bits == 12 &&
          part->vcc != NULL && part->vcc->max_mv == 3600);
    free(error);
    part_library_destroy(library);
    printf("ok   valid converter part\n");

    expect_fail("converter unknown action",
                CONV_DEVICE(CONV_REG, "{'name':'W','in':'0100aaaa','action':'fly','address':'a'}"),
                "commands[0]: unknown action \"fly\"");
    expect_fail("converter field outside its register",
                CONV_DEVICE("{'name':'CFG','address':1,'width':8,'fields':[{'name':'A','bits':[9,4]}]}",
                            ""),
                "field \"A\" is outside the register's 8 bits");
    expect_fail("converter overlapping fields",
                CONV_DEVICE("{'name':'CFG','address':1,'width':8,'fields':[{'name':'A','bits':[3,0]},"
                            "{'name':'B','bits':[4,2]}]}", ""),
                "field \"B\" overlaps another field");
    expect_fail("converter registers sharing an address",
                CONV_DEVICE(CONV_REG ",{'name':'CFG2','address':1,'width':8}", ""),
                "registers \"CFG\" and \"CFG2\" share address 1");
    expect_fail("converter unknown field in a pattern",
                CONV_DEVICE(CONV_REG, "{'name':'W','in':'0100 [CFG.Q]','action':'nop'}"),
                "register \"CFG\" has no field \"Q\"");
    expect_fail("converter capture not in the pattern",
                CONV_DEVICE(CONV_REG, "{'name':'W','in':'0100aaaa','action':'wreg','address':'b'}"),
                "\"address\" 'b' is not a capture of \"in\"");
    expect_fail("converter parallel bus with too few data pins",
                HEAD("converter") "{'id':'P','function':'ADC','description':'d','drawer':['Interface'],"
                "'pinouts':{'p':[{'number':1,'name':'VDD','role':'power'},{'number':2,'name':'D0','role':'output'}]},"
                "'type':'adc','bits':8,'channels':1,'codeFormat':'straight','vcc':[4.5,5.5],"
                "'bus':{'style':'parallel','data':['D0']},'protocol':{'frame':'fixed','read':'R{8}'},"
                "'conversion':{'time':0,'start':['read']},"
                "'parts':[{'name':'P-1','package':'DIP','pinout':'p','footprint':'T2'}]}" TAIL,
                "\"data\" needs 4-24 pins");
    expect_fail("converter clocked bus without a clock",
                HEAD("converter") "{'id':'K','function':'ADC','description':'d','drawer':['Interface'],"
                "'pinouts':{'p':[{'number':1,'name':'VDD','role':'power'},{'number':2,'name':'D0','role':'output'}]},"
                "'type':'adc','bits':1,'channels':1,'codeFormat':'straight','vcc':[4.5,5.5],"
                "'bus':{'style':'clocked','data':['D0']},'protocol':{'frame':'fixed','read':'10'},"
                "'parts':[{'name':'K-1','package':'DIP','pinout':'p','footprint':'T2'}]}" TAIL,
                "\"clock\" needs a pin");
}

/* plan-24-F: the rtc kind. */
#define RTC_DEVICE(CLOCK) \
    HEAD("rtc") "{'id':'R','function':'RTC','description':'d','drawer':['Interface']," \
    "'pinouts':{'p':[{'number':1,'name':'VCC','role':'power'},{'number':2,'name':'SCL','role':'input'}," \
    "{'number':3,'name':'SDA','role':'inout'},{'number':4,'name':'GND','role':'ground'}]},'vcc':[2,5.5]," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[104]}," \
    "'registers':[{'name':'SEC','address':0,'width':8},{'name':'MIN','address':1,'width':8}," \
    "{'name':'HR','address':2,'width':8}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'P','in':'aaaaaaaa','action':'pointer','address':'a'}],'read':'pointer'}," \
    CLOCK "'parts':[{'name':'R-1','package':'SOIC','pinout':'p','footprint':'T4'}]}" TAIL
#define RTC_FIELDS "'fields':[{'field':'SEC','unit':'second'},{'field':'MIN','unit':'minute'},{'field':'HR','unit':'hour'}]"

static void check_rtc_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, RTC_DEVICE("'clock':{" RTC_FIELDS "},"), &error) == 0);
    part = part_library_find(library, "R-1");
    CHECK(part != NULL && part->kind == PART_RTC && part->vcc != NULL && part->converter == NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   valid rtc part\n");
    expect_fail("rtc without a clock", RTC_DEVICE(""), "devices[0] (R): an RTC needs \"clock\"");
    expect_fail("rtc unknown unit",
                RTC_DEVICE("'clock':{'fields':[{'field':'SEC','unit':'fortnight'}]},"),
                "unknown unit \"fortnight\"");
    expect_fail("rtc without an hour field",
                RTC_DEVICE("'clock':{'fields':[{'field':'SEC','unit':'second'},{'field':'MIN','unit':'minute'}]},"),
                "\"fields\" need second, minute and hour");
    expect_fail("rtc alarm on an unknown register",
                RTC_DEVICE("'clock':{" RTC_FIELDS ",'alarms':[{'match':[{'field':'ASEC','unit':'second'}]}]},"),
                "unknown register \"ASEC\"");
}
#undef RTC_DEVICE
#undef RTC_FIELDS
#undef CONV_DEVICE
#undef CONV_REG

/* plan-56-B: the leddriver and lcddriver kinds (the output drivers run in display_out_test). */
#define DISPLAY_DEVICE(KIND, DISPLAY) \
    HEAD(KIND) "{'id':'L','function':'driver','description':'d','drawer':['Displays & Indicators']," \
    "'pinouts':{'p':[{'number':1,'name':'VCC','role':'power'},{'number':2,'name':'CS','role':'input'}," \
    "{'number':3,'name':'SCLK','role':'input'},{'number':4,'name':'DIN','role':'input'}," \
    "{'number':5,'name':'O1','role':'output'},{'number':6,'name':'O2','role':'output'}]},'vcc':[4.5,5.5]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','mode':0}," \
    "'registers':[{'name':'V','address':1,'width':8,'fields':[{'name':'F','bits':[3,0]}]}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'aaaaaaaa dddddddd','action':'write'," \
    "'address':'a','data':'d'}]}" DISPLAY \
    ",'parts':[{'name':'L-1','package':'SOIC','pinout':'p','footprint':'T6'}]}" TAIL
#define DISPLAY_STATIC(GROUPS) ",'display':{'driver':'static','segmentActive':'high','groups':[" GROUPS "]}"
#define DISPLAY_GROUP "{'pins':['O1','O2'],'ram':'V'}"

static void check_driver_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, DISPLAY_DEVICE("leddriver", DISPLAY_STATIC(DISPLAY_GROUP)), &error) == 0);
    part = part_library_find(library, "L-1");
    CHECK(part != NULL && part->kind == PART_LEDDRIVER && part->vcc != NULL && part->converter == NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   valid leddriver part\n");
    library = part_library_create();
    CHECK(load(library, DISPLAY_DEVICE("lcddriver", DISPLAY_STATIC(DISPLAY_GROUP)), &error) == 0);
    part = part_library_find(library, "L-1");
    CHECK(part != NULL && part->kind == PART_LCDDRIVER && part->vcc != NULL && part->converter == NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   valid lcddriver part\n");

    expect_fail("leddriver without a display", DISPLAY_DEVICE("leddriver", ""),
                "devices[0] (L): a display driver needs \"display\"");
    expect_fail("leddriver unknown driver",
                DISPLAY_DEVICE("leddriver", ",'display':{'driver':'blink'}"),
                "devices[0] (L): display: unknown driver \"blink\"");
    expect_fail("leddriver unknown register in a group",
                DISPLAY_DEVICE("leddriver", DISPLAY_STATIC("{'pins':['O1','O2'],'ram':'NOPE'}")),
                "unknown register \"NOPE\"");
    expect_fail("leddriver unknown field in a decodeField",
                DISPLAY_DEVICE("leddriver", ",'display':{'driver':'static','segmentActive':'high','decode':'hex',"
                               "'groups':[{'pins':['O1','O2'],'ram':'V','decodeField':'V.Q'}]}"),
                "register \"V\" has no field \"Q\"");
    expect_fail("leddriver pin missing from the pinout",
                DISPLAY_DEVICE("leddriver", DISPLAY_STATIC("{'pins':['O1','O9'],'ram':'V'}")),
                "no pin \"O9\" in this pinout");
    expect_fail("lcddriver unknown key in display",
                DISPLAY_DEVICE("lcddriver", ",'display':{'driver':'ports','ports':[],'blink':1}"),
                "unknown key \"blink\"");
    expect_fail("converter device with a display",
                DISPLAY_DEVICE("converter", DISPLAY_STATIC(DISPLAY_GROUP)), "unknown key \"display\"");
    expect_fail("rtc device with a display",
                DISPLAY_DEVICE("rtc", DISPLAY_STATIC(DISPLAY_GROUP)), "unknown key \"display\"");
    expect_fail("leddriver device with a conversion",
                DISPLAY_DEVICE("leddriver", DISPLAY_STATIC(DISPLAY_GROUP) ",'conversion':{}"),
                "unknown key \"conversion\"");
}
#undef DISPLAY_DEVICE
#undef DISPLAY_STATIC
#undef DISPLAY_GROUP

/* plan-29-H: the diode kind. */
#define DIODE_PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define DIODE_DEVICE(TYPE, PINS, DIODES, PART_KEYS) \
    HEAD("diode") "{'id':'D','function':'Diode','description':'d','drawer':['Discrete','Diodes']," \
    "'pinouts':{'p':[" PINS "]},'type':'" TYPE "','diodes':[" DIODES "]," \
    "'parts':[{'name':'D-1','package':'SMA','pinout':'p','footprint':'T2'" PART_KEYS "}]}" TAIL
#define DIODE_KA DIODE_PIN(1, "K", "inout") "," DIODE_PIN(2, "A", "inout")
#define DIODE_PAIR "{'anode':'A','cathode':'K'}"

static void check_diode_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, DIODE_DEVICE("schottky", DIODE_KA, DIODE_PAIR, ",'vr':40,'io':1"), &error) == 0);
    part = part_library_find(library, "D-1");
    CHECK(part != NULL && part->kind == PART_DIODE && part->vcc == NULL && part->diode != NULL &&
          strcmp(part->diode->type, "schottky") == 0 && part->diode->diode_count == 1 &&
          part->diode->vr_mv == 40000 && part->diode->io_ma == 1000);
    free(error);
    part_library_destroy(library);
    library = part_library_create();
    error = NULL;
    CHECK(load(library, DIODE_DEVICE("zener", DIODE_KA, DIODE_PAIR, ",'vz':5.1,'pd':0.5"), &error) == 0);
    part = part_library_find(library, "D-1");
    CHECK(part != NULL && part->diode->vz_mv == 5100 && part->diode->pd_mw == 500 &&
          part->diode->vr_mv == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   valid diode parts\n");
    expect_fail("diode unknown type", DIODE_DEVICE("tunnel", DIODE_KA, DIODE_PAIR, ",'vr':40,'io':1"),
                "unknown diode type \"tunnel\"");
    expect_fail("diode empty diodes", DIODE_DEVICE("rectifier", DIODE_KA, "", ",'vr':40,'io':1"),
                "\"diodes\" must be a non-empty array");
    expect_fail("diode unknown pin name",
                DIODE_DEVICE("rectifier", DIODE_KA, "{'anode':'A','cathode':'C'}", ",'vr':40,'io':1"),
                "no pin named \"C\"");
    expect_fail("diode input pin",
                DIODE_DEVICE("rectifier", DIODE_PIN(1, "K", "inout") "," DIODE_PIN(2, "A", "input"),
                             DIODE_PAIR, ",'vr':40,'io':1"),
                "diode pin \"A\" must be inout");
    expect_fail("diode inout pin in no diode",
                DIODE_DEVICE("rectifier", DIODE_KA "," DIODE_PIN(3, "X", "inout"), DIODE_PAIR,
                             ",'vr':40,'io':1"),
                "pin \"X\" is in no diode");
    expect_fail("diode vz on a rectifier",
                DIODE_DEVICE("rectifier", DIODE_KA, DIODE_PAIR, ",'vr':40,'io':1,'vz':5"),
                "\"vz\" is only for a zener");
    expect_fail("diode missing vr", DIODE_DEVICE("rectifier", DIODE_KA, DIODE_PAIR, ",'io':1"),
                "missing \"vr\"");
    expect_fail("diode vr out of range", DIODE_DEVICE("rectifier", DIODE_KA, DIODE_PAIR, ",'vr':0,'io':1"),
                "\"vr\" must be above 0 and at most 10000");
    expect_fail("zener with vr", DIODE_DEVICE("zener", DIODE_KA, DIODE_PAIR, ",'vr':5,'vz':5.1,'pd':0.5"),
                "\"vr\" is not a zener key");
}
#undef DIODE_PIN
#undef DIODE_DEVICE

/* plan-33-E: the capacitor kind. */
#define CAP_PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define CAP_DEVICE(TYPE, DEVICE_KEYS, PINS, PART_KEYS) \
    HEAD("capacitor") "{'id':'C','function':'100 nF','description':'c','drawer':['Capacitors']," \
    "'pinouts':{'p':[" PINS "]},'type':'" TYPE "'" DEVICE_KEYS "," \
    "'parts':[{'name':'C-1','package':'0402','pinout':'p','footprint':'T2'" PART_KEYS "}]}" TAIL
#define CAP_12 CAP_PIN(1, "1", "input") "," CAP_PIN(2, "2", "input")
#define CAP_PM CAP_PIN(1, "+", "input") "," CAP_PIN(2, "-", "input")
#define CAP_POL ",'farads':1e-5,'positive':'+','negative':'-'"

static void check_capacitor_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, CAP_DEVICE("ceramic", ",'farads':1e-7", CAP_12,
                                   ",'volts':50,'dielectric':'X7R','tolerance':'10%'"), &error) == 0);
    part = part_library_find(library, "C-1");
    CHECK(part != NULL && part->kind == PART_CAPACITOR && part->vcc == NULL &&
          part->capacitor != NULL && strcmp(part->capacitor->type, "ceramic") == 0 &&
          part->capacitor->polarized == 0 && part->capacitor->capacitance_ff == 100000000ULL &&
          part->capacitor->rated_mv == 50000 && strcmp(part->capacitor->dielectric, "X7R") == 0);
    free(error);
    part_library_destroy(library);
    library = part_library_create();
    error = NULL;
    CHECK(load(library, CAP_DEVICE("tantalum", CAP_POL, CAP_PM, ",'volts':6.3"), &error) == 0);
    part = part_library_find(library, "C-1");
    CHECK(part != NULL && part->capacitor->polarized == 1 && part->capacitor->rated_mv == 6300);
    free(error);
    part_library_destroy(library);
    printf("ok   valid capacitor parts\n");
    expect_fail("capacitor unknown type", CAP_DEVICE("film", ",'farads':1e-7", CAP_12, ""),
                "unknown capacitor type \"film\"");
    expect_fail("capacitor farads 0", CAP_DEVICE("ceramic", ",'farads':0", CAP_12, ""),
                "\"farads\" must be above 0 and at most 10");
    expect_fail("capacitor farads 11", CAP_DEVICE("ceramic", ",'farads':11", CAP_12, ""),
                "\"farads\" must be above 0 and at most 10");
    expect_fail("ceramic capacitor with positive",
                CAP_DEVICE("ceramic", ",'farads':1e-7,'positive':'1','negative':'2'", CAP_12, ""),
                "a ceramic capacitor has no polarity");
    expect_fail("tantalum capacitor without positive",
                CAP_DEVICE("tantalum", ",'farads':1e-5", CAP_PM, ""), "missing \"positive\"");
    expect_fail("capacitor with 3 pins",
                CAP_DEVICE("ceramic", ",'farads':1e-7", CAP_12 "," CAP_PIN(3, "3", "input"), ""),
                "a capacitor has 2 pins");
    expect_fail("capacitor inout pin",
                CAP_DEVICE("ceramic", ",'farads':1e-7",
                           CAP_PIN(1, "1", "input") "," CAP_PIN(2, "2", "inout"), ""),
                "capacitor pin \"2\" must be input");
    expect_fail("dielectric on a tantalum part",
                CAP_DEVICE("tantalum", CAP_POL, CAP_PM, ",'dielectric':'X7R'"),
                "\"dielectric\" is only for a ceramic capacitor");
    expect_fail("capacitor volts 0", CAP_DEVICE("ceramic", ",'farads':1e-7", CAP_12, ",'volts':0"),
                "\"volts\" must be above 0 and at most 10000");
}
/* plan-36-F: the mcu kind. A device must say "simulated": false; core,
   clock, flash and ram are required; a pinout needs power and ground; a
   part must resolve a vcc. */
#define MCU_PINS(GND_ROLE) \
    "{'number':1,'name':'VCC','role':'power'},{'number':2,'name':'GND','role':'" GND_ROLE "'}," \
    "{'number':3,'name':'PB0','role':'inout'},{'number':4,'name':'PB1','role':'inout'}"
#define MCU_DEVICE(KEYS, GND_ROLE, PART_KEYS) \
    HEAD("mcu") "{'id':'M','function':'MCU','description':'d','drawer':['Microcontroller']," \
    "'pinouts':{'p':[" MCU_PINS(GND_ROLE) "]}" KEYS "," \
    "'parts':[{'name':'M-1','package':'DIP','pinout':'p','footprint':'T4'" PART_KEYS "}]}" TAIL
#define MCU_OK ",'simulated':false,'core':'avr','clock':20000000,'flash':8192,'ram':512"
#define MCU_MODEL ",'model':'atmega328','core':'avr','clock':20000000,'flash':8192,'ram':512"

static void check_mcu_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, MCU_DEVICE(MCU_OK ",'eeprom':256,'vcc':[1.8,5.5]", "ground",
                                   ",'clock':10000000,'psram':2097152"), &error) == 0);
    part = part_library_find(library, "M-1");
    CHECK(part != NULL && part->kind == PART_MCU && part->simulated == 0 && part->mcu != NULL &&
          strcmp(part->mcu->core, "avr") == 0 && part->mcu->max_hz == 10000000ULL &&
          part->mcu->flash == 8192 && part->mcu->ram == 512 && part->mcu->eeprom == 256 &&
          part->mcu->psram == 2097152 && part->vcc != NULL && part->vcc->min_mv == 1800 &&
          part->vcc->max_mv == 5500);
    free(error);
    part_library_destroy(library);
    printf("ok   valid mcu part (part clock and psram override the device)\n");
    expect_fail("mcu unknown core",
                MCU_DEVICE(",'simulated':false,'core':'z80','clock':1,'flash':0,'ram':0,'vcc':[3,3.6]",
                           "ground", ""),
                "unknown mcu core \"z80\"");
    expect_fail("mcu clock 0",
                MCU_DEVICE(",'simulated':false,'core':'avr','clock':0,'flash':0,'ram':0,'vcc':[3,3.6]",
                           "ground", ""),
                "\"clock\" must be above 0");
    expect_fail("mcu without simulated",
                MCU_DEVICE(",'core':'avr','clock':1,'flash':0,'ram':0,'vcc':[3,3.6]", "ground", ""),
                "an mcu has no simulation model: \"simulated\" must be false");
    expect_fail("mcu simulated true",
                MCU_DEVICE(",'simulated':true,'core':'avr','clock':1,'flash':0,'ram':0,'vcc':[3,3.6]",
                           "ground", ""),
                "an mcu has no simulation model: \"simulated\" must be false");
    expect_fail("mcu pinout with no ground", MCU_DEVICE(MCU_OK ",'vcc':[3,3.6]", "inout", ""),
                "an mcu pinout needs a ground pin");
    expect_fail("mcu without vcc", MCU_DEVICE(MCU_OK, "ground", ""), "missing \"vcc\"");

    /* plan-62-E: a device with a "model" is simulated. */
    library = part_library_create();
    error = NULL;
    CHECK(load(library, MCU_DEVICE(MCU_MODEL ",'vcc':[1.8,5.5]", "ground", ""), &error) == 0);
    part = part_library_find(library, "M-1");
    CHECK(part != NULL && part->simulated == 1 && part->mcu != NULL && part->mcu->model != NULL &&
          strcmp(part->mcu->model, "atmega328") == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   valid mcu part with a model (simulated)\n");
    expect_fail("mcu model with another core",
                MCU_DEVICE(",'model':'atmega328','core':'avrxt','clock':1,'flash':8192,'ram':512,"
                           "'vcc':[3,3.6]", "ground", ""),
                "\"model\" needs core \"avr\"");
    expect_fail("mcu unknown model",
                MCU_DEVICE(",'model':'atmega4809','core':'avr','clock':1,'flash':8192,'ram':512,"
                           "'vcc':[3,3.6]", "ground", ""),
                "unknown mcu model \"atmega4809\"");
    expect_fail("mcu model with simulated false",
                MCU_DEVICE(MCU_MODEL ",'simulated':false,'vcc':[3,3.6]", "ground", ""),
                "an mcu with a \"model\" is simulated: remove \"simulated\": false");
    expect_fail("mcu model with a pin of a port it lacks",
                HEAD("mcu") "{'id':'M','function':'MCU','description':'d','drawer':['Microcontroller'],"
                "'pinouts':{'p':[" MCU_PINS("ground") ",{'number':5,'name':'PE0','role':'inout'}]}"
                MCU_MODEL ",'vcc':[3,3.6],"
                "'parts':[{'name':'M-1','package':'DIP','pinout':'p','footprint':'T4'}]}" TAIL,
                "mcu model \"atmega328\" has no pin \"PE0\"");
    expect_fail("mcu model with an odd flash size",
                MCU_DEVICE(",'model':'atmega328','core':'avr','clock':1,'flash':8191,'ram':512,"
                           "'vcc':[3,3.6]", "ground", ""),
                "an mcu with a \"model\" needs \"flash\" of 512 bytes or more, an even number");
    expect_fail("mcu model part overriding flash",
                MCU_DEVICE(MCU_MODEL ",'vcc':[3,3.6]", "ground", ",'flash':4096"),
                "a part of an mcu with a \"model\" cannot override \"flash\"");
}
#undef MCU_MODEL
#undef MCU_OK
#undef MCU_DEVICE
#undef MCU_PINS

#undef CAP_PIN
#undef CAP_DEVICE
#undef CAP_12
#undef CAP_PM
#undef CAP_POL
#undef DIODE_KA
#undef DIODE_PAIR

/* plan-14-A: the pio kind checks "model" against its model table. */
#define PIO_DEVICE(KEYS) \
    HEAD("pio") "{'id':'U','function':'PIA','description':'d','drawer':['Interface']," \
    "'pinouts':{'p':[{'number':1,'name':'VSS','role':'ground'}]}," KEYS \
    "'parts':[{'name':'U-P','package':'DIP','pinout':'p','footprint':'T40','grade':'14'}]}" TAIL

static void check_pio(void) {
    expect_fail("pio unknown model", PIO_DEVICE("'model':'nope','grades':{'14':{}},"),
                "devices[0] (U): unknown pio model \"nope\"");
    expect_fail("pio missing model", PIO_DEVICE("'grades':{'14':{}},"),
                "devices[0] (U): \"model\" is required");
    /* plan-14-B: pia6521 keys. */
#define PIA_KEYS(PA, CA2) \
    "'model':'pia6521','phi2':'PHI2','rw':'RWB','reset':'RESB'," \
    "'data':['D0','D1','D2','D3','D4','D5','D6','D7'],'csHigh':['CS0','CS1'],'csLow':['CS2B']," \
    "'rs':['RS0','RS1'],'pa':[" PA "],'pb':['PB0','PB1','PB2','PB3','PB4','PB5','PB6','PB7']," \
    "'ca1':'CA1'," CA2 "'cb1':'CB1','cb2':'CB2','irqa':'IRQAB','irqb':'IRQBB'," \
    "'grades':{'14':{'tcyc':70,'tc':35}},"
#define PA8 "'PA0','PA1','PA2','PA3','PA4','PA5','PA6','PA7'"
    expect_fail("pia6521 missing ca2", PIO_DEVICE(PIA_KEYS(PA8, "")),
                "devices[0] (U): missing \"ca2\"");
    expect_fail("pia6521 pa of 7", PIO_DEVICE(PIA_KEYS("'PA0','PA1','PA2','PA3','PA4','PA5','PA6'",
                                                       "'ca2':'CA2',")),
                "devices[0] (U): \"pa\" needs exactly 8 pin names");
#undef PIA_KEYS
    /* plan-14-C: via6522 keys. */
#define VIA_KEYS(RS, OPEN_DRAIN) \
    "'model':'via6522','phi2':'PHI2','rw':'RWB','reset':'RESB'," \
    "'data':['D0','D1','D2','D3','D4','D5','D6','D7'],'csHigh':['CS1'],'csLow':['CS2B']," \
    "'rs':[" RS "],'pa':[" PA8 "],'pb':['PB0','PB1','PB2','PB3','PB4','PB5','PB6','PB7']," \
    "'ca1':'CA1','ca2':'CA2','cb1':'CB1','cb2':'CB2','irq':'IRQB'," OPEN_DRAIN "'busHold':false," \
    "'grades':{'14':{'tcyc':70,'tpwh':35,'tpwl':35}},"
    expect_fail("via6522 missing irqOpenDrain",
                PIO_DEVICE(VIA_KEYS("'RS0','RS1','RS2','RS3'", "")),
                "devices[0] (U): missing \"irqOpenDrain\"");
    expect_fail("via6522 rs of 3", PIO_DEVICE(VIA_KEYS("'RS0','RS1','RS2'", "'irqOpenDrain':true,")),
                "devices[0] (U): \"rs\" needs exactly 4 pin names");
#undef VIA_KEYS
#undef PA8
}

static void check_valid_supply(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *vcc;
    const Part *gnd;

    CHECK(load(library, HEAD("supply") VCC_DEVICE "," GND_DEVICE TAIL, &error) == 0);
    CHECK(error == NULL);
    CHECK(part_library_count(library) == 2);
    vcc = part_library_find(library, "5V");
    gnd = part_library_find(library, "GND");
    CHECK(vcc != NULL && gnd != NULL);
    CHECK(part_library_find(library, "NOPE") == NULL);
    if (vcc != NULL && gnd != NULL) {
        CHECK(vcc->kind == PART_SUPPLY && vcc->supply != NULL && vcc->supply->level == 1 &&
              vcc->supply->mv == 5000 && vcc->vcc == NULL);
        CHECK(gnd->supply->level == 0 && gnd->supply->mv == 0);
        CHECK(strcmp(vcc->function, "5V") == 0 && vcc->drawer_count == 0);
        CHECK(vcc->pin_count == 1 && strcmp(vcc->pins[0].designator, "1") == 0 && vcc->pins[0].role == PIN_POWER);
        CHECK(strcmp(vcc->pins[0].group, "default") == 0);
        CHECK(gnd->flags == (PART_OBSOLETE | PART_MILITARY) && vcc->flags == 0);
        CHECK(vcc->logic == NULL && vcc->led == NULL && vcc->oscillator == NULL);
        CHECK(kind_model(vcc) != NULL);
    }
    free(error);
    part_library_destroy(library);
    printf("ok   valid supply file\n");
}


static void check_valid_led(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *dip;

    CHECK(load(library, HEAD("led") LED_DEVICE("A") TAIL, &error) == 0);
    dip = part_library_find(library, "LED-RED-DIP");
    CHECK(dip != NULL && dip->pin_count == 4 && strcmp(dip->led->colors[0], "Red") == 0);
    CHECK(dip != NULL && dip->drawer_count == 2 && strcmp(dip->drawer[1], "LED") == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   valid LED file with two pinouts\n");
}

/* ---- Manufacturer and datasheet part keys ---------------------------------- */

#define DS_FILE(ENTRIES) "{'format':'schematical-datasheets','version':1,'datasheets':[" ENTRIES "]}"
#define DS_SUPPLY(KEYS) \
    HEAD("supply") "{'id':'5V','function':'5V','description':'d','drawer':[]," \
    "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5," \
    "'parts':[{'name':'5V'" KEYS ",'package':'','pinout':'pin'}]}" TAIL

static int load_ds(PartLibrary *library, const char *text, char **error) {
    char *source = json(text);
    int result = load_datasheets(library, source, "datasheets.json", error);

    free(source);
    return result;
}

static void expect_ds_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    int result = load_ds(library, text, &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_datasheet_count(library) != 0) {
        printf("FAIL %s: result %d, error \"%s\" (wanted \"%s\")\n", name, result,
               error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    part_library_destroy(library);
}

/* A part file that should fail against a library holding datasheet "a". */
static void expect_source_fail(const char *name, const char *parts, const char *expect) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    int result;

    CHECK(load_ds(library, DS_FILE("{'id':'a','url':'https://example.com/a.pdf'}"), &error) == 0);
    free(error);
    error = NULL;
    result = load(library, parts, &error);
    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_count(library) != 0) {
        printf("FAIL %s: result %d, error \"%s\" (wanted \"%s\")\n", name, result,
               error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    part_library_destroy(library);
}

static void check_datasheets(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load_ds(library, DS_FILE("{'id':'a','url':'https://example.com/a.pdf'},"
                                   "{'id':'b','url':'https://example.com/b.pdf'}"), &error) == 0);
    CHECK(part_library_datasheet_count(library) == 2);
    CHECK(part_library_find_datasheet(library, "b") != NULL &&
          strcmp(part_library_find_datasheet(library, "b")->url, "https://example.com/b.pdf") == 0);
    CHECK(part_library_find_datasheet(library, "b")->md == NULL);
    CHECK(load(library, DS_SUPPLY(",'manufacturer':'Acme','datasheet':'b'"), &error) == 0);
    part = part_library_find(library, "5V");
    CHECK(part != NULL && strcmp(part->manufacturer, "Acme") == 0 && part->datasheet != NULL &&
          strcmp(part->datasheet->id, "b") == 0);
    /* Ids stay unique across calls. */
    free(error);
    error = NULL;
    CHECK(load_ds(library, DS_FILE("{'id':'a','url':'https://example.com/c.pdf'}"), &error) == -1 &&
          error != NULL && strstr(error, "duplicate datasheet id \"a\"") != NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   datasheets file and the manufacturer/datasheet part keys\n");

    library = part_library_create();
    error = NULL;
    CHECK(load_ds(library, DS_FILE("{'id':'m','url':'https://example.com/m.pdf','md':'gate-and/m.md'}"),
                  &error) == 0);
    CHECK(part_library_find_datasheet(library, "m") != NULL &&
          strcmp(part_library_find_datasheet(library, "m")->md, "gate-and/m.md") == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   datasheet md path\n");

    library = part_library_create();
    error = NULL;
    CHECK(load(library, DS_SUPPLY(",'manufacturer':'Generic'"), &error) == 0);
    part = part_library_find(library, "5V");
    CHECK(part != NULL && strcmp(part->manufacturer, "Generic") == 0 && part->datasheet == NULL);
    free(error);
    part_library_destroy(library);
    library = part_library_create();
    error = NULL;
    CHECK(load(library, DS_SUPPLY(""), &error) == 0);
    part = part_library_find(library, "5V");
    CHECK(part != NULL && part->manufacturer == NULL && part->datasheet == NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   Generic and keyless parts\n");

    expect_ds_fail("datasheets: wrong format", "{'format':'x','version':1,'datasheets':[]}",
                   "\"format\" must be \"schematical-datasheets\"");
    expect_ds_fail("datasheets: duplicate id",
                   DS_FILE("{'id':'a','url':'https://e.com/a'},{'id':'a','url':'https://e.com/b'}"),
                   "duplicate datasheet id \"a\"");
    expect_ds_fail("datasheets: http url", DS_FILE("{'id':'a','url':'http://e.com/a.pdf'}"),
                   "\"url\" must start with https://");
    expect_ds_fail("datasheets: empty id", DS_FILE("{'id':'','url':'https://e.com/a.pdf'}"),
                   "empty datasheet id");
    expect_ds_fail("datasheets: md not .md", DS_FILE("{'id':'a','url':'https://e.com/a.pdf','md':'a.txt'}"),
                   "\"md\" must be a relative path to a .md file");
    expect_ds_fail("datasheets: md absolute", DS_FILE("{'id':'a','url':'https://e.com/a.pdf','md':'/a.md'}"),
                   "\"md\" must be a relative path to a .md file");
    expect_ds_fail("datasheets: md parent", DS_FILE("{'id':'a','url':'https://e.com/a.pdf','md':'../a.md'}"),
                   "\"md\" must be a relative path to a .md file");
    expect_ds_fail("datasheets: md empty", DS_FILE("{'id':'a','url':'https://e.com/a.pdf','md':''}"),
                   "\"md\" must be a relative path to a .md file");
    expect_ds_fail("datasheets: unknown key",
                   DS_FILE("{'id':'a','url':'https://e.com/a.pdf','title':'t'}"), "title");
    expect_source_fail("part: unknown datasheet", DS_SUPPLY(",'manufacturer':'Acme','datasheet':'zz'"),
                       "unknown datasheet \"zz\"");
    expect_source_fail("part: datasheet without manufacturer", DS_SUPPLY(",'datasheet':'a'"),
                       "\"datasheet\" needs a \"manufacturer\"");
    expect_source_fail("part: Generic with datasheet",
                       DS_SUPPLY(",'manufacturer':'Generic','datasheet':'a'"),
                       "a Generic part has no datasheet");
    expect_source_fail("part: empty manufacturer", DS_SUPPLY(",'manufacturer':''"),
                       "empty \"manufacturer\"");
}

/* ---- plan-23-A: the header kind ------------------------------------------ */

#define HDR_PINS2 "[{'number':1,'name':'1','role':'inout'},{'number':2,'name':'2','role':'inout'}]"
#define HDR_PINS3 "[{'number':1,'name':'1','role':'inout'},{'number':2,'name':'2','role':'inout'}," \
    "{'number':3,'name':'3','role':'inout'}]"
#define HDR_PINS4 "[{'number':1,'name':'1','role':'inout'},{'number':2,'name':'2','role':'inout'}," \
    "{'number':3,'name':'3','role':'inout'},{'number':4,'name':'4','role':'inout'}]"
#define HDR_DEVICE(KEYS, PINOUTS, PARTS) \
    HEAD("header") "{'id':'header-2row','function':'Header','description':'d','drawer':['Connector','Header']," \
    KEYS ",'pinouts':{" PINOUTS "},'parts':[" PARTS "]}" TAIL

static void check_header_kind(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *two, *four;

    CHECK(load(library,
               HDR_DEVICE("'rows':2,'genders':['Male','Female']",
                          "'2x01':" HDR_PINS2 ",'2x02':" HDR_PINS4,
                          "{'name':'H-2X01','package':'Straight','pinout':'2x01','footprint':'T2'},"
                          "{'name':'H-2X02','package':'Straight','pinout':'2x02','footprint':'T4'}"),
               &error) == 0);
    if (error != NULL) {
        printf("FAIL valid header: %s\n", error);
        failures++;
    }
    two = part_library_find(library, "H-2X01");
    four = part_library_find(library, "H-2X02");
    CHECK(two != NULL && four != NULL);
    if (two != NULL && four != NULL) {
        CHECK(two->kind == PART_HEADER && two->pin_count == 2 && four->pin_count == 4);
        CHECK(two->header != NULL && two->header->rows == 2 && two->header->gender_count == 2 &&
              strcmp(two->header->genders[0], "Male") == 0 &&
              strcmp(two->header->genders[1], "Female") == 0);
        CHECK(four->header == two->header);
    }
    free(error);
    part_library_destroy(library);
    printf("ok   valid 2-row header with parts of 2 and 4 pins\n");

    expect_fail("header rows 3",
                HDR_DEVICE("'rows':3,'genders':['Male']", "'p':" HDR_PINS3,
                           "{'name':'H','package':'Straight','pinout':'p','footprint':'T4'}"),
                "\"rows\" must be 1 or 2");
    expect_fail("header empty genders",
                HDR_DEVICE("'rows':1,'genders':[]", "'p':" HDR_PINS2,
                           "{'name':'H','package':'Straight','pinout':'p','footprint':'T2'}"),
                "\"genders\" must not be empty");
    expect_fail("header 3 pins on 2 rows",
                HDR_DEVICE("'rows':2,'genders':['Male']", "'p':" HDR_PINS3,
                           "{'name':'H','package':'Straight','pinout':'p','footprint':'T4'}"),
                "a header's pin count must be a multiple of its rows");

    /* plan-36-F: "simulated", a common device key any kind may carry. */
    library = part_library_create();
    error = NULL;
    CHECK(load(library,
               HDR_DEVICE("'rows':1,'genders':['Male'],'simulated':false", "'p':" HDR_PINS2,
                          "{'name':'H-UNSIM','package':'Straight','pinout':'p','footprint':'T2'}"),
               &error) == 0);
    if (error != NULL) {
        printf("FAIL header with simulated false: %s\n", error);
        failures++;
    }
    two = part_library_find(library, "H-UNSIM");
    CHECK(two != NULL && two->simulated == 0);
    free(error);
    part_library_destroy(library);
    library = part_library_create();
    error = NULL;
    CHECK(load(library,
               HDR_DEVICE("'rows':1,'genders':['Male'],'simulated':true", "'p':" HDR_PINS2,
                          "{'name':'H-SIM','package':'Straight','pinout':'p','footprint':'T2'}"),
               &error) == 0);
    two = part_library_find(library, "H-SIM");
    CHECK(two != NULL && two->simulated == 1);
    free(error);
    part_library_destroy(library);
    printf("ok   header with \"simulated\": false loads unsimulated parts; true and absent simulate\n");
    expect_fail("header simulated 1",
                HDR_DEVICE("'rows':1,'genders':['Male'],'simulated':1", "'p':" HDR_PINS2,
                           "{'name':'H','package':'Straight','pinout':'p','footprint':'T2'}"),
                "\"simulated\" must be true or false");
}

int main(void) {
    check_datasheets();
    check_header_kind();
    check_valid_supply();
    check_valid_pull();
    check_serial();
    check_pio();
    check_converter_cases();
    check_rtc_cases();
    check_driver_cases();
    check_diode_cases();
    check_capacitor_cases();
    check_mcu_cases();
    check_valid_led();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
