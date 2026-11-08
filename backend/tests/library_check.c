/*
 * library_check: load_parts accepts well-formed part files and rejects every
 * malformed case with a path-style message, adding nothing on failure
 * (plan-01-B section 3.7; C, D, E add their kinds' cases).
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "library_internal.h"
#include "model_pins.h"
#include "parts.h"
#include "serial_fixture.h"

static int failures;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            printf("%s:%d: check failed: %s\n", __FILE__, __LINE__, #cond); \
            failures++;                                                   \
        }                                                                 \
    } while (0)

/* Single quotes stand for double quotes, to keep the cases readable. */
static char *json(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    size_t i;

    for (i = 0; text[i] != '\0'; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    copy[i] = '\0';
    return copy;
}

/*
 * Footprints the fixture parts name ("footprint" is required on every part
 * but a supply): T<n> is an n-pin dual, BGA2 holds balls A1 B2, BGA4 the
 * 2 x 2 grid.
 */
#define FIXTURE_DUAL(ID, PINS, EXTRA) \
    "{'id':'" ID "','description':'d','style':'dual','pins':" PINS EXTRA ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
#define FIXTURE_FOOTPRINTS \
    "{'format':'schematical-footprints','version':1,'footprints':[" \
    FIXTURE_DUAL("T1", "1", ",'perSide':1") "," FIXTURE_DUAL("T2", "2", "") "," \
    FIXTURE_DUAL("T4", "4", "") "," FIXTURE_DUAL("T5", "5", ",'perSide':3") "," \
    FIXTURE_DUAL("T6", "6", "") "," FIXTURE_DUAL("T26", "26", "") "," \
    FIXTURE_DUAL("T40", "40", "") "," \
    "{'id':'BGA2','description':'d','style':'grid','pins':2,'rows':2,'columns':2,'pitch':0.5," \
    "'balls':['A1','B2'],'body':{'width':1,'height':1},'pad':{'width':0.2,'height':0.2,'shape':'round'}}," \
    "{'id':'BGA4','description':'d','style':'grid','pins':4,'rows':2,'columns':2,'pitch':0.5," \
    "'balls':['A1','A2','B1','B2'],'body':{'width':1,'height':1},'pad':{'width':0.2,'height':0.2,'shape':'round'}}]}"

/* Gives a library with no footprints the fixture set. */
static void load_fixture_footprints(PartLibrary *library) {
    char *source;
    char *error = NULL;

    if (part_library_footprint_count(library) > 0) {
        return;
    }
    source = json(FIXTURE_FOOTPRINTS);
    if (load_footprints(library, source, "fixtures.json", &error) != 0) {
        printf("FAIL fixture footprints: %s\n", error != NULL ? error : "(none)");
        failures++;
    }
    free(error);
    free(source);
}

/* Loads text into a fresh library and expects failure with `expect` in the message. */
static void expect_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *source = json(text);
    char *error = NULL;
    int result;

    load_fixture_footprints(library);
    result = load_parts(library, source, "test.json", &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    free(source);
    part_library_destroy(library);
}

static int load(PartLibrary *library, const char *text, char **error) {
    char *source = json(text);
    int result;

    load_fixture_footprints(library);
    result = load_parts(library, source, "test.json", error);

    free(source);
    return result;
}

#define HEAD(KIND) "{'format':'schematical-parts','version':1,'kind':'" KIND "','devices':["
#define TAIL "]}"

#define VCC_DEVICE \
    "{'id':'5V','function':'5V','description':'d','drawer':[]," \
    "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5," \
    "'parts':[{'name':'5V','package':'','pinout':'pin'}]}"
#define GND_DEVICE \
    "{'id':'GND','function':'GND','description':'d','drawer':[],'note':'n'," \
    "'pinouts':{'pin':[{'number':1,'name':'GND','role':'ground','group':'default'}]},'volts':0," \
    "'parts':[{'name':'GND','package':'','pinout':'pin','flags':['obsolete','military']}]}"

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
               "'parts':[{'name':'PULLDOWN','package':'','pinout':'pin'}]}" TAIL,
               &error) == 0);
    CHECK(error == NULL);
    pd = part_library_find(library, "PULLDOWN");
    vcc = part_library_find(library, "5V");
    CHECK(pd != NULL && vcc != NULL);
    if (pd != NULL && vcc != NULL) {
        CHECK(pd->kind == PART_SUPPLY && pd->supply->pull == 1 && pd->supply->level == 0 &&
              pd->supply->mv == 0 && pd->pins[0].role == PIN_OUTPUT);
        CHECK(vcc->supply->pull == 0);
    }
    free(error);
    part_library_destroy(library);
    printf("ok   valid pull part\n");
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
              strcmp(part->serial->grade, "X") == 0 && part->vcc != NULL &&
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

#define LED_DEVICE(ANODE2) \
    "{'id':'led-red','function':'LED','description':'d','drawer':['Indicators','LED']," \
    "'pinouts':{'two-pin':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}]," \
    "'dip4':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'" ANODE2 "','role':'input'}," \
    "{'number':3,'name':'NC','role':'nc'},{'number':4,'name':'NC','role':'nc'}]}," \
    "'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-RED-0603','package':'0603','pinout':'two-pin','footprint':'T2'}," \
    "{'name':'LED-RED-DIP','package':'DIP','pinout':'dip4','footprint':'T4'}]}"

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

/* ---- plan-18-A: LED "colors" and the "footprintChoices" part key -------- */

#define CHOICE_DEVICE(COLORS, PART_EXTRA) \
    "{'id':'led-c','function':'LED','description':'d','drawer':['Indicators','LED']," \
    "'pinouts':{'two-pin':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}]}," \
    COLORS ",'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-C','pinout':'two-pin'" PART_EXTRA "}]}"
#define TWO_CHOICES "'footprintChoices':[{'package':'0603','footprint':'T2'},{'package':'DIP','footprint':'T4'}]"

static void check_footprint_choices(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, HEAD("led") CHOICE_DEVICE("'colors':['Red','Blue']",
               ",'package':'0603','footprint':'T2'," TWO_CHOICES) TAIL, &error) == 0);
    part = part_library_find(library, "LED-C");
    CHECK(part != NULL && part->led->color_count == 2 && strcmp(part->led->colors[0], "Red") == 0 &&
          strcmp(part->led->colors[1], "Blue") == 0);
    CHECK(part != NULL && part->footprint_choice_count == 2 &&
          strcmp(part->footprint_choices[0].package, "0603") == 0 &&
          strcmp(part->footprint_choices[1].package, "DIP") == 0 &&
          strcmp(part->footprint_choices[1].footprint->id, "T4") == 0 &&
          part->footprint_choices[1].footprint->pad_count == 4 && part->pin_count == 2 &&
          part->footprint == part->footprint_choices[0].footprint);
    if (error != NULL) {
        printf("FAIL valid footprintChoices: %s\n", error);
        failures++;
    }
    free(error);
    part_library_destroy(library);
    printf("ok   valid LED with 2 colors and 2 footprintChoices\n");

    expect_fail("LED empty colors",
                HEAD("led") CHOICE_DEVICE("'colors':[]", ",'package':'0603','footprint':'T2'") TAIL,
                "\"colors\" must not be empty");
    expect_fail("LED device with the old color key",
                HEAD("led") CHOICE_DEVICE("'color':'Red'", ",'package':'0603','footprint':'T2'") TAIL,
                "unknown key \"color\"");
    expect_fail("footprintChoices empty",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'0603','footprint':'T2','footprintChoices':[]") TAIL,
                "\"footprintChoices\" must be a non-empty array of {\"package\", \"footprint\"}");
    expect_fail("footprintChoices extra key",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'0603','footprint':'T2','footprintChoices':[{'package':'0603','footprint':'T2','x':1}]") TAIL,
                "\"footprintChoices\" must be a non-empty array of {\"package\", \"footprint\"}");
    expect_fail("footprintChoices unknown footprint",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'0603','footprint':'T2','footprintChoices':[{'package':'0603','footprint':'T2'},{'package':'X','footprint':'NOPE'}]") TAIL,
                "unknown footprintChoices footprint \"NOPE\"");
    expect_fail("footprintChoices pin without a pad",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'0603','footprint':'T2','footprintChoices':[{'package':'0603','footprint':'T2'},{'package':'X','footprint':'T1'}]") TAIL,
                "pin \"2\" has no pad in footprintChoices footprint \"T1\"");
    expect_fail("footprintChoices listed twice",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'0603','footprint':'T2','footprintChoices':[{'package':'0603','footprint':'T2'},{'package':'Y','footprint':'T2'}]") TAIL,
                "footprintChoices footprint \"T2\" listed twice");
    expect_fail("footprintChoices without the default",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'0603','footprint':'T2','footprintChoices':[{'package':'DIP','footprint':'T4'}]") TAIL,
                "\"footprint\" \"T2\" is not in footprintChoices");
    expect_fail("footprintChoices package mismatch",
                HEAD("led") CHOICE_DEVICE("'colors':['Red']",
                ",'package':'X','footprint':'T2'," TWO_CHOICES) TAIL,
                "\"package\" \"X\" does not match footprintChoices \"T2\" (\"0603\")");
    expect_fail("supply part with footprintChoices",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','footprintChoices':[{'package':'X','footprint':'T1'}]}]}" TAIL,
                "parts[0] (VCC): a supply has no footprint");
}

static void check_duplicate_across_calls(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;

    CHECK(load(library, HEAD("supply") VCC_DEVICE TAIL, &error) == 0);
    CHECK(part_library_count(library) == 1);
    /* The second file repeats 5V (another device id), after a good GND. */
    CHECK(load(library,
               HEAD("supply") GND_DEVICE ","
               "{'id':'5V-2','function':'5V','description':'d','drawer':[],"
               "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
               "'parts':[{'name':'5V','package':'','pinout':'pin'}]}" TAIL,
               &error) == -1);
    CHECK(error != NULL && strstr(error, "duplicate part name \"5V\"") != NULL);
    printf("ok   duplicate across calls: %s\n", error != NULL ? error : "(none)");
    CHECK(part_library_count(library) == 1);
    CHECK(part_library_find(library, "5V") != NULL);
    CHECK(part_library_find(library, "GND") == NULL);
    free(error);
    error = NULL;
    /* The library still takes a good file afterwards. */
    CHECK(load(library, HEAD("supply") GND_DEVICE TAIL, &error) == 0);
    CHECK(part_library_count(library) == 2);
    free(error);
    part_library_destroy(library);
}

/* plan-19-B: a decoder device on a 26-pin fixture pinout; KEYS are the model's keys. */
#define DEC_PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define DEC_DEVICE(KEYS) \
    "{'id':'d','function':'Decoder','description':'d','drawer':['Logic'],'pinouts':{'p26':[" \
    DEC_PIN(1, "A", "input") "," DEC_PIN(2, "B", "input") "," DEC_PIN(3, "C", "input") "," \
    DEC_PIN(4, "D", "input") "," DEC_PIN(5, "E", "input") "," DEC_PIN(6, "G", "input") "," \
    DEC_PIN(7, "LE", "input") "," DEC_PIN(8, "CLK", "input") "," DEC_PIN(9, "Y0", "output") "," \
    DEC_PIN(10, "Y1", "output") "," DEC_PIN(11, "Y2", "output") "," DEC_PIN(12, "Y3", "output") "," \
    DEC_PIN(13, "Y4", "output") "," DEC_PIN(14, "Y5", "output") "," DEC_PIN(15, "Y6", "output") "," \
    DEC_PIN(16, "Y7", "output") "," DEC_PIN(17, "Y8", "output") "," DEC_PIN(18, "Y9", "output") "," \
    DEC_PIN(19, "Y10", "output") "," DEC_PIN(20, "Y11", "output") "," DEC_PIN(21, "Y12", "output") "," \
    DEC_PIN(22, "Y13", "output") "," DEC_PIN(23, "RBI", "input") "," DEC_PIN(24, "RBO", "inout") "," \
    DEC_PIN(25, "GND", "ground") "," DEC_PIN(26, "VCC", "power") "]}," KEYS "," \
    "'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}}," \
    "'parts':[{'name':'D1','package':'DIP','pinout':'p26','footprint':'T26','family':'HC'}]}"
#define DEC_BIN(UNIT, REST) "'model':'binary','units':[" UNIT "],'outputsActive':'low'" REST
#define DEC_U2 "{'address':['A','B'],'outputs':['Y0','Y1','Y2','Y3']"
#define DEC_PRI(REST) \
    "'model':'priority','inputs':['A','B','C','D'],'inputsActive':'low','code':['Y0','Y1']," \
    "'codeActive':'low'" REST
#define DEC_GLYPHS(G0) \
    "'glyphs':['" G0 "','0110000','1101101','1111001','0110011','1011011','0011111','1110000'," \
    "'1111111','1110011','0000000','0000000','0000000','0000000','0000000','0000000']"
#define DEC_SEG(REST, G0) \
    "'model':'seg7','inputs':['A','B','C','D'],'segments':['Y0','Y1','Y2','Y3','Y4','Y5','Y6']," \
    "'segmentsActive':'low'," DEC_GLYPHS(G0) REST

static void check_decoder_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library,
               HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 ",'enables':[{'pin':'G','active':'low'}]},"
                                          DEC_U2 ",'latch':'LE'}", ",'latchActive':'high'")) TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_DECODER && part->logic != NULL &&
          strcmp(part->logic->family, "HC") == 0 && part->logic->units == 2);
    printf("ok   valid decoder (binary, 2 units)%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("decoder unknown model",
                HEAD("decoder") DEC_DEVICE("'model':'ternary'") TAIL,
                "devices[0] (d): unknown model \"ternary\"");
    expect_fail("decoder key of another model",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 "}", ",'segments':['Y0']")) TAIL,
                "\"segments\" is not a key of model \"binary\"");
    expect_fail("decoder no units",
                HEAD("decoder") DEC_DEVICE(DEC_BIN("", "")) TAIL,
                "\"units\" must be an array of 1 to 4 units");
    expect_fail("decoder 5 address pins",
                HEAD("decoder") DEC_DEVICE(DEC_BIN("{'address':['A','B','C','D','E'],'outputs':['Y0']}", "")) TAIL,
                "units[0]: \"address\" must have 1 to 4 names");
    expect_fail("decoder no outputs",
                HEAD("decoder") DEC_DEVICE(DEC_BIN("{'address':['A'],'outputs':[]}", "")) TAIL,
                "units[0]: \"outputs\" must have 1 to 16 names");
    expect_fail("decoder more outputs than codes",
                HEAD("decoder") DEC_DEVICE(DEC_BIN("{'address':['A'],'outputs':['Y0','Y1','Y2']}", "")) TAIL,
                "units[0]: \"outputs\" must have at most 2 names (2 to the number of address pins)");
    expect_fail("decoder latch and clock",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 ",'latch':'LE','clock':'CLK'}", ",'latchActive':'high'")) TAIL,
                "units[0]: a unit cannot have both \"latch\" and \"clock\"");
    expect_fail("decoder 4 enables",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 ",'enables':[{'pin':'C','active':'low'},"
                    "{'pin':'D','active':'low'},{'pin':'E','active':'low'},{'pin':'G','active':'low'}]}", "")) TAIL,
                "units[0]: \"enables\" must be an array of 0 to 3 controls");
    expect_fail("decoder enable without active",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 ",'enables':[{'pin':'G'}]}", "")) TAIL,
                "units[0]: enables[0]: missing \"active\"");
    expect_fail("decoder outputsActive",
                HEAD("decoder") DEC_DEVICE("'model':'binary','units':[" DEC_U2 "}],'outputsActive':'up'") TAIL,
                "\"outputsActive\" must be \"high\" or \"low\", not \"up\"");
    expect_fail("decoder unselected",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 "}", ",'unselected':'off'")) TAIL,
                "\"unselected\" must be \"inactive\" or \"z\", not \"off\"");
    expect_fail("decoder disabled",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 "}", ",'disabled':'float'")) TAIL,
                "\"disabled\" must be \"inactive\" or \"z\", not \"float\"");
    expect_fail("decoder latch without latchActive",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 ",'latch':'LE'}", "")) TAIL,
                "missing \"latchActive\"");
    expect_fail("decoder latchActive without latch",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 "}", ",'latchActive':'high'")) TAIL,
                "\"latchActive\" needs a unit with \"latch\"");
    expect_fail("decoder output style",
                HEAD("decoder") DEC_DEVICE(DEC_BIN(DEC_U2 "}", ",'output':'open-collector'")) TAIL,
                "\"output\" must be \"push-pull\" or \"open-drain\"");
    expect_fail("decoder unknown address pin",
                HEAD("decoder") DEC_DEVICE(DEC_BIN("{'address':['A','Q'],'outputs':['Y0']}", "")) TAIL,
                "pinouts.p26: units[0]: no pin named \"Q\"");
    expect_fail("decoder priority 3 inputs",
                HEAD("decoder") DEC_DEVICE("'model':'priority','inputs':['A','B','C'],'inputsActive':'low',"
                    "'code':['Y0','Y1'],'codeActive':'low'") TAIL,
                "\"inputs\" must have 4 to 10 names");
    expect_fail("decoder priority 1 code pin",
                HEAD("decoder") DEC_DEVICE("'model':'priority','inputs':['A','B','C','D'],'inputsActive':'low',"
                    "'code':['Y0'],'codeActive':'low'") TAIL,
                "\"code\" must have 2 to 4 names");
    expect_fail("decoder priority offset",
                HEAD("decoder") DEC_DEVICE(DEC_PRI(",'offset':1")) TAIL,
                "\"offset\" plus the number of inputs must be at most 4 (2 to the number of code pins)");
    expect_fail("decoder priority idle",
                HEAD("decoder") DEC_DEVICE(DEC_PRI(",'idle':4")) TAIL,
                "\"idle\" must be below 4 (2 to the number of code pins)");
    expect_fail("decoder seg7 glyph length",
                HEAD("decoder") DEC_DEVICE(DEC_SEG("", "111111")) TAIL,
                "glyphs[0]: must be 7 characters 0 or 1 (segments a to g)");
    expect_fail("decoder seg7 glyph character",
                HEAD("decoder") DEC_DEVICE(DEC_SEG("", "11111x1")) TAIL,
                "glyphs[0]: must be 7 characters 0 or 1 (segments a to g)");
    expect_fail("decoder seg7 6 segments",
                HEAD("decoder") DEC_DEVICE("'model':'seg7','inputs':['A','B','C','D'],"
                    "'segments':['Y0','Y1','Y2','Y3','Y4','Y5'],'segmentsActive':'low'," DEC_GLYPHS("1111110")) TAIL,
                "\"segments\" must have 7 names");
    expect_fail("decoder seg7 rbi without rbo",
                HEAD("decoder") DEC_DEVICE(DEC_SEG(",'rbi':'RBI'", "1111110")) TAIL,
                "\"rbi\" and \"rbo\" go together");
    expect_fail("decoder seg7 rbo not inout",
                HEAD("decoder") DEC_DEVICE(DEC_SEG(",'rbi':'RBI','rbo':'Y13'", "1111110")) TAIL,
                "\"rbo\" pin \"Y13\" must have role inout");
}

/* plan-21-B: a mux device on the decoder's 26-pin fixture pinout. */
#define MUX_SEL(UNIT, REST) "'model':'select','units':[" UNIT "]" REST
#define MUX_U1 "{'data':['A','B'],'select':['G'],'y':'Y0'"
#define MUX_STO(REST) "'model':'stored','units':[" MUX_U1 ",'w':'Y1'}]" REST
#define MUX_RF_FILE(D, Q) \
    "{'d':[" D "],'wa':['A','B'],'we':{'pin':'CLK','active':'low'},'ra':['E','G']," \
    "'re':{'pin':'LE','active':'low'},'q':[" Q "]}"
#define MUX_RF(WORDS, BITS, FILES) \
    "'model':'regfile','words':" WORDS ",'bits':" BITS ",'files':[" FILES "]"

static void check_mux_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library,
               HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 ",'w':'Y1','strobe':{'pin':'LE','active':'low'}},"
                                      "{'data':['A','B','C','D'],'select':['E','G'],'w':'Y2',"
                                      "'oe':{'pin':'CLK','active':'low'}}",
                                      ",'disabled':'z','invert':true")) TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_MUX && part->logic != NULL &&
          strcmp(part->logic->family, "HC") == 0 && part->logic->units == 2);
    printf("ok   valid mux (select, 2 units)%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("mux unknown model",
                HEAD("mux") DEC_DEVICE("'model':'crossbar'") TAIL,
                "devices[0] (d): unknown model \"crossbar\"");
    expect_fail("mux key of another kind",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}", ",'outputsActive':'low'")) TAIL,
                "devices[0]: unknown key \"outputsActive\"");
    expect_fail("mux no units",
                HEAD("mux") DEC_DEVICE(MUX_SEL("", "")) TAIL,
                "\"units\" must be an array of 1 to 6 units");
    expect_fail("mux 7 units",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}," MUX_U1 "}," MUX_U1 "}," MUX_U1 "},"
                                               MUX_U1 "}," MUX_U1 "}," MUX_U1 "}", "")) TAIL,
                "\"units\" must be an array of 1 to 6 units");
    expect_fail("mux unknown unit key",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 ",'q':'Y1'}", "")) TAIL,
                "units[0]: unknown key \"q\"");
    expect_fail("mux 5 select pins",
                HEAD("mux") DEC_DEVICE(MUX_SEL("{'data':['A','B'],'select':['A','B','C','D','E'],'y':'Y0'}", "")) TAIL,
                "units[0]: \"select\" must have 1 to 4 names");
    expect_fail("mux 1 data pin",
                HEAD("mux") DEC_DEVICE(MUX_SEL("{'data':['A'],'select':['G'],'y':'Y0'}", "")) TAIL,
                "units[0]: \"data\" must have 2 to 16 names");
    expect_fail("mux data not 2^select",
                HEAD("mux") DEC_DEVICE(MUX_SEL("{'data':['A','B','C'],'select':['E','G'],'y':'Y0'}", "")) TAIL,
                "units[0]: \"data\" must have 4 names (2 to the number of select pins)");
    expect_fail("mux no y or w",
                HEAD("mux") DEC_DEVICE(MUX_SEL("{'data':['A','B'],'select':['G']}", "")) TAIL,
                "units[0]: a unit needs \"y\" or \"w\"");
    expect_fail("mux strobe without active",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 ",'strobe':{'pin':'LE'}}", "")) TAIL,
                "units[0]: strobe: missing \"active\"");
    expect_fail("mux disabled",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}", ",'disabled':'high'")) TAIL,
                "\"disabled\" must be \"low\" or \"z\", not \"high\"");
    expect_fail("mux invert not bool",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}", ",'invert':'yes'")) TAIL,
                "\"invert\"");
    expect_fail("mux output style",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}", ",'output':'open-collector'")) TAIL,
                "\"output\" must be \"push-pull\" or \"open-drain\"");
    expect_fail("mux unknown data pin",
                HEAD("mux") DEC_DEVICE(MUX_SEL("{'data':['A','Q'],'select':['G'],'y':'Y0'}", "")) TAIL,
                "pinouts.p26: units[0]: no pin named \"Q\"");
    expect_fail("mux selectLatch without active",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}", ",'selectLatch':{'pin':'LE'}")) TAIL,
                "selectLatch: missing \"active\"");
    expect_fail("mux key of another model",
                HEAD("mux") DEC_DEVICE(MUX_SEL(MUX_U1 "}", ",'words':4")) TAIL,
                "\"words\" is not a key of model \"select\"");

    /* plan-21-C: stored and regfile. */
    CHECK(load(library = part_library_create(),
               HEAD("mux") DEC_DEVICE(MUX_STO(",'dataClock':{'pin':'CLK','edge':'rising'},"
                                      "'selectLatch':{'pin':'LE','active':'high'},"
                                      "'oes':[{'pin':'D','active':'low'},{'pin':'E','active':'high'}]")) TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_MUX && part->logic->units == 1);
    printf("ok   valid mux (stored)%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    error = NULL;
    part_library_destroy(library);
    CHECK(load(library = part_library_create(),
               HEAD("mux") DEC_DEVICE(MUX_RF("4", "2", MUX_RF_FILE("'C','D'", "'Y0','Y1'") ","
                                             MUX_RF_FILE("'C','D'", "'Y2','Y3'"))) TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_MUX && part->logic->units == 2);
    printf("ok   valid mux (regfile, 2 files)%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("mux stored without storage",
                HEAD("mux") DEC_DEVICE(MUX_STO("")) TAIL,
                "model \"stored\" needs \"selectLatch\", \"dataLatch\", \"dataClock\" or \"outputClock\"");
    expect_fail("mux stored dataLatch and dataClock",
                HEAD("mux") DEC_DEVICE(MUX_STO(",'dataLatch':{'pin':'LE','active':'high'},"
                                               "'dataClock':{'pin':'CLK','edge':'rising'}")) TAIL,
                "a device cannot have both \"dataLatch\" and \"dataClock\"");
    expect_fail("mux stored bad edge",
                HEAD("mux") DEC_DEVICE(MUX_STO(",'outputClock':{'pin':'CLK','edge':'up'}")) TAIL,
                "outputClock");
    expect_fail("mux stored 4 oes",
                HEAD("mux") DEC_DEVICE(MUX_STO(",'outputClock':{'pin':'CLK','edge':'rising'},"
                    "'oes':[{'pin':'C','active':'low'},{'pin':'D','active':'low'},"
                    "{'pin':'E','active':'low'},{'pin':'LE','active':'low'}]")) TAIL,
                "\"oes\" must be an array of 1 to 3 controls");
    expect_fail("mux stored oe without active",
                HEAD("mux") DEC_DEVICE(MUX_STO(",'outputClock':{'pin':'CLK','edge':'rising'},"
                    "'oes':[{'pin':'C'}]")) TAIL,
                "oes[0]: missing \"active\"");
    expect_fail("mux stored unknown clock pin",
                HEAD("mux") DEC_DEVICE(MUX_STO(",'dataClock':{'pin':'Q','edge':'rising'}")) TAIL,
                "no pin named \"Q\"");
    expect_fail("mux regfile 3 words",
                HEAD("mux") DEC_DEVICE(MUX_RF("3", "2", MUX_RF_FILE("'C','D'", "'Y0','Y1'"))) TAIL,
                "\"words\" must be 2, 4, 8 or 16 and \"bits\" 1 to 8");
    expect_fail("mux regfile 9 bits",
                HEAD("mux") DEC_DEVICE(MUX_RF("4", "9", MUX_RF_FILE("'C','D'", "'Y0','Y1'"))) TAIL,
                "\"bits\"");
    expect_fail("mux regfile no files",
                HEAD("mux") DEC_DEVICE(MUX_RF("4", "2", "")) TAIL,
                "\"files\" must be an array of 1 to 2 files");
    expect_fail("mux regfile 3 files",
                HEAD("mux") DEC_DEVICE(MUX_RF("4", "2", MUX_RF_FILE("'C','D'", "'Y0','Y1'") ","
                    MUX_RF_FILE("'C','D'", "'Y2','Y3'") "," MUX_RF_FILE("'C','D'", "'Y4','Y5'"))) TAIL,
                "\"files\" must be an array of 1 to 2 files");
    expect_fail("mux regfile q count",
                HEAD("mux") DEC_DEVICE(MUX_RF("4", "2", MUX_RF_FILE("'C','D'", "'Y0'"))) TAIL,
                "files[0]: \"q\" must have 2 names");
    expect_fail("mux regfile address count",
                HEAD("mux") DEC_DEVICE(MUX_RF("8", "2", MUX_RF_FILE("'C','D'", "'Y0','Y1'"))) TAIL,
                "files[0]: \"wa\" must have 3 names");
    expect_fail("mux regfile missing we",
                HEAD("mux") DEC_DEVICE(MUX_RF("4", "2", "{'d':['C','D'],'wa':['A','B'],"
                    "'ra':['E','G'],'re':{'pin':'LE','active':'low'},'q':['Y0','Y1']}")) TAIL,
                "files[0]: missing \"we\"");
    expect_fail("mux regfile unknown file key",
                HEAD("mux") DEC_DEVICE(MUX_RF("4", "2", "{'d':['C','D'],'wa':['A','B'],"
                    "'we':{'pin':'CLK','active':'low'},'ra':['E','G'],'re':{'pin':'LE','active':'low'},"
                    "'q':['Y0','Y1'],'oe':'Y2'}")) TAIL,
                "files[0]: unknown key \"oe\"");
}

static void check_logic_family(void) {
    KindContext ctx;

    memset(&ctx, 0, sizeof ctx);
    ctx.source = "test.json";
    CHECK(kind_logic_family(&ctx, "HC") != NULL && ctx.error == NULL);
    CHECK(kind_logic_family(&ctx, "HX") == NULL && ctx.error != NULL &&
          strstr(ctx.error, "unknown family \"HX\"") != NULL);
    printf("ok   kind_logic_family: %s\n", ctx.error != NULL ? ctx.error : "(none)");
    free(ctx.error);
}

static void check_supply_cases(void) {
    expect_fail("bad JSON", HEAD("supply") VCC_DEVICE ",", "invalid JSON at byte");
    expect_fail("wrong format",
                "{'format':'parts','version':1,'kind':'supply','devices':[]}",
                "\"format\" must be \"schematical-parts\"");
    expect_fail("version 2",
                "{'format':'schematical-parts','version':2,'kind':'supply','devices':[]}",
                "unsupported version 2");
    expect_fail("unknown kind", HEAD("resistor") TAIL, "unknown kind \"resistor\"");
    expect_fail("unknown file key",
                "{'format':'schematical-parts','version':1,'kind':'supply','devices':[],'x':1}",
                "unknown key \"x\"");
    expect_fail("unknown device key",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],'colour':'x',"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin'}]}" TAIL,
                "devices[0]: unknown key \"colour\"");
    expect_fail("unknown part key",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','family':'HC'}]}" TAIL,
                "devices[0] (VCC): parts[0] (VCC): unknown key \"family\"");
    expect_fail("pin numbers 1,3",
                HEAD("led")
                "{'id':'led-red','function':'LED','description':'d','drawer':[],"
                "'pinouts':{'two-pin':[{'number':1,'name':'K','role':'input'},{'number':3,'name':'A','role':'input'}]},"
                "'colors':['Red'],'anode':'A','cathode':'K','parts':[]}" TAIL,
                "pinouts.two-pin: pin 2: pin numbers must be 1..N in order (found 3)");
    expect_fail("unknown role",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'hidden'}]},'volts':5,"
                "'parts':[]}" TAIL,
                "unknown role \"hidden\"");
    expect_fail("part names a missing pinout",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'dip8'}]}" TAIL,
                "parts[0] (VCC): no pinout \"dip8\"");
    expect_fail("LED anode missing from the second pinout", HEAD("led") LED_DEVICE("AA") TAIL,
                "devices[0] (led-red): pinouts.dip4: no pin named \"A\"");
    expect_fail("duplicate part in one file",
                HEAD("supply") VCC_DEVICE ","
                "{'id':'5V-2','function':'5V','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'5V','package':'','pinout':'pin'}]}" TAIL,
                "duplicate part name \"5V\"");
    expect_fail("duplicate device id", HEAD("supply") VCC_DEVICE "," VCC_DEVICE TAIL,
                "duplicate device id \"5V\"");
    expect_fail("bad flag",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','flags':['vintage']}]}" TAIL,
                "unknown flag \"vintage\"");
    expect_fail("drawer too deep",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':['a','b','c'],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[]}" TAIL,
                "\"drawer\" has at most 2 levels");
    expect_fail("supply volts -1",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':-1,"
                "'parts':[]}" TAIL,
                "devices[0] (VCC): \"volts\" must be from 0 to 30");
    expect_fail("supply volts 31",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':31,"
                "'parts':[]}" TAIL,
                "devices[0] (VCC): \"volts\" must be from 0 to 30");
    expect_fail("supply pull not a boolean",
                HEAD("supply")
                "{'id':'PU','function':'Pull-up','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'PU','role':'output'}]},'volts':5,'pull':'yes',"
                "'parts':[]}" TAIL,
                "devices[0] (PU): \"pull\" must be true or false");
    expect_fail("oscillator defaultHz out of range",
                HEAD("oscillator")
                "{'id':'o','function':'f','description':'d','drawer':[],"
                "'pinouts':{'p':[{'number':1,'name':'OUT','role':'output'}]},"
                "'family':'X','minHz':10,'maxHz':20,'outputs':['OUT'],"
                "'parts':[{'name':'O1','package':'','pinout':'p','footprint':'T1','defaultHz':30}]}" TAIL,
                "defaultHz 30 is outside minHz..maxHz");
    expect_fail("oscillator output naming a shared pin",
                HEAD("oscillator")
                "{'id':'o','function':'f','description':'d','drawer':[],"
                "'pinouts':{'p':[{'number':1,'name':'OUT','role':'output'},{'number':2,'name':'OUT','role':'output'}]},"
                "'family':'X','minHz':10,'maxHz':20,'outputs':['OUT'],'parts':[]}" TAIL,
                "pin name \"OUT\" is used by 2 pins; one is needed");
}

/* A gate device; GATES and TIMING and the second pinout's pin 2 name vary. */
#define GATE_DEVICE(GATES, TIMING, PIN2, FAMILY) \
    "{'id':'g','function':'NAND','description':'d','drawer':['Logic']," \
    "'pinouts':{'p4':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'}," \
    "{'number':3,'name':'C','role':'input'},{'number':4,'name':'Y','role':'output'}]," \
    "'p5':[{'number':1,'name':'NC','role':'nc'},{'number':2,'name':'" PIN2 "','role':'input'}," \
    "{'number':3,'name':'B','role':'input'},{'number':4,'name':'C','role':'input'},{'number':5,'name':'Y','role':'output'}]}," \
    "'gates':" GATES ",'timing':" TIMING "," \
    "'parts':[{'name':'G1','package':'DIP','pinout':'p4','footprint':'T4','family':'" FAMILY "','vcc':[2,6]}," \
    "{'name':'G2','package':'LCCC','pinout':'p5','footprint':'T5','family':'" FAMILY "','vcc':[2,6]}]}"

static void check_gate_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library,
               HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                        "{'HC':{'rise':9,'fall':4.5,'note':'n'}}", "A", "HC") TAIL,
               &error) == 0);
    part = part_library_find(library, "G2");
    CHECK(part != NULL && part->kind == PART_GATE && part->logic != NULL &&
          strcmp(part->logic->family, "HC") == 0 && part->logic->units == 1);
    printf("ok   valid gate file with two pinouts%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    library = part_library_create();
    error = NULL;
    CHECK(load(library,
               HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                        "{'TPIC':{'rise':20,'fall':20}}", "A", "TPIC") TAIL,
               &error) == 0);
    part = part_library_find(library, "G1");
    CHECK(part != NULL && part->logic != NULL && strcmp(part->logic->family, "TPIC") == 0);
    printf("ok   gate part with family TPIC%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    /* plan-19-A: FCT, L and ALV are logic families. */
    library = part_library_create();
    error = NULL;
    CHECK(load(library,
               HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                        "{'FCT':{'rise':5,'fall':5}}", "A", "FCT") TAIL,
               &error) == 0);
    part = part_library_find(library, "G1");
    CHECK(part != NULL && part->logic != NULL && strcmp(part->logic->family, "FCT") == 0);
    printf("ok   gate part with family FCT%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);
    {
        KindContext ctx;

        memset(&ctx, 0, sizeof(ctx));
        ctx.source = "test.json";
        CHECK(kind_logic_family(&ctx, "L") != NULL && kind_logic_family(&ctx, "ALV") != NULL &&
              ctx.error == NULL);
        printf("ok   families L and ALV\n");
    }

    expect_fail("gate otherLow on a 3-input device",
                HEAD("gate") GATE_DEVICE("[{'op':'xor','inputs':['A','B','C'],'output':'Y'}]",
                                         "{'HC':{'otherLow':{'rise':1,'fall':1},'otherHigh':{'rise':2,'fall':2}}}",
                                         "A", "HC") TAIL,
                "otherLow/otherHigh timing needs every gate to have exactly 2 inputs");
    expect_fail("gate family missing from timing",
                HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                         "{'HC':{'rise':9,'fall':9}}", "A", "LS") TAIL,
                "parts[0] (G1): family \"LS\" has no timing entry");
    expect_fail("gate input absent from the second pinout",
                HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                         "{'HC':{'rise':9,'fall':9}}", "AA", "HC") TAIL,
                "pinouts.p5: gates[0]: no pin named \"A\"");
    expect_fail("gate op nandd",
                HEAD("gate") GATE_DEVICE("[{'op':'nandd','inputs':['A','B','C'],'output':'Y'}]",
                                         "{'HC':{'rise':9,'fall':9}}", "A", "HC") TAIL,
                "gates[0]: unknown op \"nandd\"");
    expect_fail("gate not with 2 inputs",
                HEAD("gate") GATE_DEVICE("[{'op':'not','inputs':['A','B'],'output':'Y'}]",
                                         "{'HC':{'rise':9,'fall':9}}", "A", "HC") TAIL,
                "gates[0]: a \"not\" gate has exactly 1 input");
    expect_fail("gate unknown family",
                HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                         "{'HX':{'rise':9,'fall':9}}", "A", "HC") TAIL,
                "timing.HX: unknown family \"HX\"");
}

#define FF_PINS \
    "'pinouts':{'p6':[{'number':1,'name':'D','role':'input'},{'number':2,'name':'CLK','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'QN','role':'output'}," \
    "{'number':5,'name':'GND','role':'ground'},{'number':6,'name':'VCC','role':'power'}]},"

static void check_flipflop_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library,
               HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':['Logic']," FF_PINS
               "'units':[{'d':'D','clk':'CLK','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
               "'clearActiveHigh':false,'clockEdge':'rising',"
               "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false,'vcc':[2,6]}},"
               "'parts':[{'name':'D1','package':'DIP','pinout':'p6','footprint':'T6','family':'HC'}]}" TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_DFF && part->logic->units == 1);
    printf("ok   valid dff file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    library = part_library_create();
    error = NULL;
    CHECK(load(library,
               HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':['Logic']," FF_PINS
               "'units':[{'d':'D','clk':'CLK','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
               "'clearActiveHigh':false,'clockEdge':'rising','output':'open-drain',"
               "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false,'vcc':[2,6]}},"
               "'parts':[{'name':'D1','package':'DIP','pinout':'p6','footprint':'T6','family':'HC'}]}" TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part_pin_open_drain(part, 2) && part_pin_open_drain(part, 3) &&
          !part_pin_open_drain(part, 0));
    printf("ok   dff with \"output\": \"open-drain\"%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("dff output x",
                HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':[]," FF_PINS
                "'units':[{'d':'D','clk':'CLK','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
                "'clearActiveHigh':false,'clockEdge':'rising','output':'x',"
                "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false}},'parts':[]}" TAIL,
                "\"output\" must be \"push-pull\" or \"open-drain\"");
    expect_fail("dff unit naming a missing pin",
                HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':[]," FF_PINS
                "'units':[{'d':'D','clk':'CLK','pre':'PRE','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
                "'clearActiveHigh':false,'clockEdge':'rising',"
                "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false}},'parts':[]}" TAIL,
                "pinouts.p6: units[0]: no pin named \"PRE\"");
    expect_fail("dff clockEdge up",
                HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':[]," FF_PINS
                "'units':[{'d':'D','clk':'CLK','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
                "'clearActiveHigh':false,'clockEdge':'up',"
                "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false}},'parts':[]}" TAIL,
                "\"clockEdge\" must be \"rising\" or \"falling\", not \"up\"");
    expect_fail("jkff with 3 units",
                HEAD("jkff") "{'id':'j','function':'JK flip-flop','description':'d','drawer':[]," FF_PINS
                "'units':[{'j':'D','k':'D','clk':'CLK'},{'j':'D','k':'D','clk':'CLK'},{'j':'D','k':'D','clk':'CLK'}],"
                "'kInverted':false,'presetActiveHigh':false,'clearActiveHigh':false,'clockEdge':'falling',"
                "'timing':{'HC':{'rise':20,'fall':18}},'parts':[]}" TAIL,
                "\"units\" must be an array of 1 to 2 units");
    expect_fail("jkff timing with clearWins",
                HEAD("jkff") "{'id':'j','function':'JK flip-flop','description':'d','drawer':[]," FF_PINS
                "'units':[{'j':'D','k':'D','clk':'CLK'}],"
                "'kInverted':false,'presetActiveHigh':false,'clearActiveHigh':false,'clockEdge':'falling',"
                "'timing':{'HC':{'rise':20,'fall':18,'clearWins':true}},'parts':[]}" TAIL,
                "timing.HC: unknown key \"clearWins\"");
}

static void check_switch_cases(void) {
    expect_fail("switch contact naming one pin twice",
                HEAD("switch")
                "{'id':'s','function':'GDS','description':'d','drawer':['Switch','DIP'],"
                "'pinouts':{'p4':[{'number':1,'name':'1A','role':'inout'},{'number':2,'name':'2A','role':'inout'},"
                "{'number':3,'name':'2B','role':'inout'},{'number':4,'name':'1B','role':'inout'}]},"
                "'actuator':'Low profile','contacts':[['1A','1B'],['2A','2A']],'parts':[]}" TAIL,
                "contacts[1]: a contact joins two different pins, not \"2A\" twice");
}

#define SRAM_DEVICE(GRADE) \
    HEAD("sram") "{'id':'m','function':'1 KB','description':'d','drawer':['Memory','SRAM']," \
    "'pinouts':{'p6':[{'number':1,'name':'A0','role':'input','group':'address'}," \
    "{'number':2,'name':'DQ0','role':'inout'},{'number':3,'name':'CE#','role':'input'}," \
    "{'number':4,'name':'OE#','role':'input'},{'number':5,'name':'WE#','role':'input'}," \
    "{'number':6,'name':'VCC','role':'power'}]}," \
    "'address':['A0'],'data':['DQ0'],'chipEnable':['CE#'],'chipEnableHigh':[]," \
    "'outputEnable':'OE#','writeEnable':'WE#','byteEnable':[]," \
    "'grades':{'55':{'trc':55,'taa':55,'twpOeLow':45,'note':'n','vcc':[2.7,5.5]}}," \
    "'parts':[{'name':'M1','package':'DIP','pinout':'p6','footprint':'T6','grade':'" GRADE "'}]}" TAIL

static void check_sram_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, SRAM_DEVICE("55"), &error) == 0);
    part = part_library_find(library, "M1");
    CHECK(part != NULL && part->sram != NULL && part->sram->words == 2 && part->sram->bits == 1 &&
          part->sram->access_ps == 55000 && strcmp(part->sram->grade, "55") == 0);
    printf("ok   valid sram file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("sram grade missing from grades", SRAM_DEVICE("70"),
                "parts[0] (M1): grade \"70\" is not in \"grades\"");
}

/* A 32K x 8 EEPROM (15 address bits, 26 pins) with the given keys. */
#define EE_ADDR(N, P) "{'number':" #P ",'name':'A" #N "','role':'input','group':'address'},"
#define EE_DATA(N, P) "{'number':" #P ",'name':'D" #N "','role':'inout'},"
#define EEPROM_DEVICE(DATA, PAGE, COMMANDS, PROTECTION, GRADE, PART_GRADE) \
    HEAD("eeprom") "{'id':'E','function':'32 KB','description':'d','drawer':['Memory','EEPROM']," \
    "'pinouts':{'p26':[" \
    EE_ADDR(0, 1) EE_ADDR(1, 2) EE_ADDR(2, 3) EE_ADDR(3, 4) EE_ADDR(4, 5) EE_ADDR(5, 6) \
    EE_ADDR(6, 7) EE_ADDR(7, 8) EE_ADDR(8, 9) EE_ADDR(9, 10) EE_ADDR(10, 11) EE_ADDR(11, 12) \
    EE_ADDR(12, 13) EE_ADDR(13, 14) EE_ADDR(14, 15) \
    EE_DATA(0, 16) EE_DATA(1, 17) EE_DATA(2, 18) EE_DATA(3, 19) EE_DATA(4, 20) EE_DATA(5, 21) \
    EE_DATA(6, 22) EE_DATA(7, 23) \
    "{'number':24,'name':'CE#','role':'input'},{'number':25,'name':'OE#','role':'input'}," \
    "{'number':26,'name':'WE#','role':'input'}]}," \
    "'address':['A0','A1','A2','A3','A4','A5','A6','A7','A8','A9','A10','A11','A12','A13','A14']," \
    "'data':[" DATA "],'chipEnable':'CE#','outputEnable':'OE#','writeEnable':'WE#'," \
    "'pageSize':" PAGE ",'commandAddresses':" COMMANDS ",'protection':'" PROTECTION "'," \
    "'grades':{'15':" GRADE "}," \
    "'parts':[{'name':'E1','package':'DIP','pinout':'p26','footprint':'T26','grade':'" PART_GRADE "'}]}" TAIL
#define EE_DATA8 "'D0','D1','D2','D3','D4','D5','D6','D7'"
#define EE_CMDS "['5555','2AAA']"
#define EE_GRADE "{'tacc':150,'tce':150,'toe':70,'twp':100,'tblc':150000,'twc':10000000,'note':'n','vcc':[4.5,5.5]}"

static void check_eeprom_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, EEPROM_DEVICE(EE_DATA8, "64", EE_CMDS, "off", EE_GRADE, "15"), &error) == 0);
    part = part_library_find(library, "E1");
    CHECK(part != NULL && part->kind == PART_EEPROM && part->eeprom != NULL &&
          part->eeprom->words == 32768 && part->eeprom->bits == 8 &&
          part->eeprom->access_ps == 150000 && strcmp(part->eeprom->grade, "15") == 0 &&
          part->eeprom->page_bytes == 64 && part->eeprom->write_cycle_ps == 10000000000ull);
    CHECK(part != NULL && part->sram == NULL);
    printf("ok   valid eeprom file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("eeprom data count 7",
                EEPROM_DEVICE("'D0','D1','D2','D3','D4','D5','D6'", "64", EE_CMDS, "off", EE_GRADE, "15"),
                "devices[0] (E): \"data\" needs exactly 8 pin names");
    expect_fail("eeprom pageSize 48", EEPROM_DEVICE(EE_DATA8, "48", EE_CMDS, "off", EE_GRADE, "15"),
                "devices[0] (E): \"pageSize\" must be a power of two");
    expect_fail("eeprom pageSize twice the words",
                EEPROM_DEVICE(EE_DATA8, "65536", EE_CMDS, "off", EE_GRADE, "15"),
                "\"pageSize\" 65536 is more than the 32768 words");
    expect_fail("eeprom command address past the end",
                EEPROM_DEVICE(EE_DATA8, "64", "['8000','2AAA']", "off", EE_GRADE, "15"),
                "commandAddresses[0] \"8000\" is past the last address (7FFF)");
    expect_fail("eeprom command address not hex",
                EEPROM_DEVICE(EE_DATA8, "64", "['55G5','2AAA']", "off", EE_GRADE, "15"),
                "commandAddresses[0] \"55G5\" is not a hex address");
    expect_fail("eeprom protection sometimes",
                EEPROM_DEVICE(EE_DATA8, "64", EE_CMDS, "sometimes", EE_GRADE, "15"),
                "\"protection\" must be \"off\" or \"always\"");
    expect_fail("eeprom twc 0",
                EEPROM_DEVICE(EE_DATA8, "64", EE_CMDS, "off", "{'tacc':150,'tblc':150000,'twc':0}", "15"),
                "grades.15: \"twc\" must be more than 0");
    expect_fail("eeprom unknown timing name",
                EEPROM_DEVICE(EE_DATA8, "64", EE_CMDS, "off", "{'tacc':150,'trc':150,'tblc':1,'twc':1}", "15"),
                "grades.15: unknown key \"trc\"");
    expect_fail("eeprom grade missing from grades",
                EEPROM_DEVICE(EE_DATA8, "64", EE_CMDS, "off", EE_GRADE, "70"),
                "parts[0] (E1): grade \"70\" is not in \"grades\"");
}

/* A 40-pin processor (plan-09-B section 5.4): A0-A15 on 1-16, D0-D7 on 17-24, the
   W65C02S controls on 25-37. PINOUT2 is a second pinout (may be empty). */
#define CP(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define CPU_P40(NAME, PIN31) \
    "'" NAME "':[" \
    CP(1, "A0", "output") "," CP(2, "A1", "output") "," CP(3, "A2", "output") "," \
    CP(4, "A3", "output") "," CP(5, "A4", "output") "," CP(6, "A5", "output") "," \
    CP(7, "A6", "output") "," CP(8, "A7", "output") "," CP(9, "A8", "output") "," \
    CP(10, "A9", "output") "," CP(11, "A10", "output") "," CP(12, "A11", "output") "," \
    CP(13, "A12", "output") "," CP(14, "A13", "output") "," CP(15, "A14", "output") "," \
    CP(16, "A15", "output") "," CP(17, "D0", "inout") "," CP(18, "D1", "inout") "," \
    CP(19, "D2", "inout") "," CP(20, "D3", "inout") "," CP(21, "D4", "inout") "," \
    CP(22, "D5", "inout") "," CP(23, "D6", "inout") "," CP(24, "D7", "inout") "," \
    CP(25, "PHI2", "input") "," CP(26, "RWB", "output") "," CP(27, "SYNC", "output") "," \
    CP(28, "IRQB", "input") "," CP(29, "NMIB", "input") "," CP(30, "RESB", "input") "," \
    CP(31, PIN31, "input") "," CP(32, "RDY", "inout") "," CP(33, "PHI1O", "output") "," \
    CP(34, "PHI2O", "output") "," CP(35, "VPB", "output") "," CP(36, "MLB", "output") "," \
    CP(37, "BE", "input") "," CP(38, "VDD", "power") "," CP(39, "VSS", "ground") "," \
    CP(40, "NC", "nc") "]"
#define CPU_ADDR16 "'A0','A1','A2','A3','A4','A5','A6','A7','A8','A9','A10','A11','A12','A13','A14','A15'"
#define CPU_DATA8 "'D0','D1','D2','D3','D4','D5','D6','D7'"
#define CPU_KEYS(SYNC) \
    "'clock':'PHI2','rw':'RWB'," SYNC "'irq':'IRQB','nmi':'NMIB','reset':'RESB','so':'SOB'," \
    "'rdy':'RDY','phi1Out':'PHI1O','phi2Out':'PHI2O'"
#define CPU_SYNC "'sync':'SYNC',"
#define CPU_WDC ",'vectorPull':'VPB','memoryLock':'MLB','busEnable':'BE'"
#define CPU_GRADE(TCYC) \
    "{'tcyc':" TCYC ",'tpwh':35,'tpwl':35,'tads':30,'tah':10,'tsync':30,'tmds':25,'tdhw':10," \
    "'tdsr':10,'tpcs':10,'tbvd':25,'note':'n','vcc':[1.71,5.25]}"
#define CPU_PART(NAME, PINOUT, EXTRA) \
    "{'name':'" NAME "','package':'DIP','pinout':'" PINOUT "','footprint':'T40','grade':'14'" EXTRA "}"
#define CPU_DEVICE(CORE, ADDR, DATA, KEYS, GRADE, PINOUTS, PARTS) \
    HEAD("cpu") "{'id':'C','function':'65C02','description':'d','drawer':['Processor']," \
    "'pinouts':{" PINOUTS "},'core':'" CORE "','address':[" ADDR "],'data':[" DATA "]," KEYS "," \
    "'grades':{'14':" GRADE "},'parts':[" PARTS "]}" TAIL
#define CPU_OK(CORE, KEYS) \
    CPU_DEVICE(CORE, CPU_ADDR16, CPU_DATA8, KEYS, CPU_GRADE("70"), CPU_P40("p40", "SOB"), \
               CPU_PART("C1", "p40", ""))

/* A 40-pin Z80: A0-A15 on 1-16, D0-D7 on 17-24, the controls on 25-38.
   PIN40 names pin 40 (GND, or a second M1#). */
#define Z80_P40(PIN40) \
    "'p40':[" \
    CP(1, "A0", "output") "," CP(2, "A1", "output") "," CP(3, "A2", "output") "," \
    CP(4, "A3", "output") "," CP(5, "A4", "output") "," CP(6, "A5", "output") "," \
    CP(7, "A6", "output") "," CP(8, "A7", "output") "," CP(9, "A8", "output") "," \
    CP(10, "A9", "output") "," CP(11, "A10", "output") "," CP(12, "A11", "output") "," \
    CP(13, "A12", "output") "," CP(14, "A13", "output") "," CP(15, "A14", "output") "," \
    CP(16, "A15", "output") "," CP(17, "D0", "inout") "," CP(18, "D1", "inout") "," \
    CP(19, "D2", "inout") "," CP(20, "D3", "inout") "," CP(21, "D4", "inout") "," \
    CP(22, "D5", "inout") "," CP(23, "D6", "inout") "," CP(24, "D7", "inout") "," \
    CP(25, "CLK", "input") "," CP(26, "M1#", "output") "," CP(27, "MREQ#", "output") "," \
    CP(28, "IORQ#", "output") "," CP(29, "RD#", "output") "," CP(30, "WR#", "output") "," \
    CP(31, "RFSH#", "output") "," CP(32, "HALT#", "output") "," CP(33, "BUSACK#", "output") "," \
    CP(34, "WAIT#", "input") "," CP(35, "INT#", "input") "," CP(36, "NMI#", "input") "," \
    CP(37, "RESET#", "input") "," CP(38, "BUSREQ#", "input") "," CP(39, "+5V", "power") "," \
    CP(40, PIN40, "ground") "]"
#define Z80_KEYS(BUSACK) \
    "'clock':'CLK','m1':'M1#','mreq':'MREQ#','iorq':'IORQ#','rd':'RD#','wr':'WR#','rfsh':'RFSH#'," \
    "'halt':'HALT#'," BUSACK "'wait':'WAIT#','int':'INT#','nmi':'NMI#','reset':'RESET#','busreq':'BUSREQ#'"
#define Z80_BUSACK "'busack':'BUSACK#',"
#define Z80_GRADE(TCC) "{'tcc':" TCC ",'twch':40,'twcl':40,'tdcra':65,'note':'n','vcc':[4.5,5.5]}"
#define Z80_DEVICE(KEYS, GRADE, PIN40) \
    CPU_DEVICE("z80", CPU_ADDR16, CPU_DATA8, KEYS, GRADE, Z80_P40(PIN40), CPU_PART("Z1", "p40", ""))

static void check_z80_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, Z80_DEVICE(Z80_KEYS(Z80_BUSACK), Z80_GRADE("100"), "GND"), &error) == 0);
    part = part_library_find(library, "Z1");
    CHECK(part != NULL && part->cpu != NULL && strcmp(part->cpu->core, "z80") == 0 &&
          part->cpu->max_hz == 10000000);
    printf("ok   valid z80 cpu file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("z80 with a sync key",
                Z80_DEVICE(Z80_KEYS(Z80_BUSACK) ",'sync':'M1#'", Z80_GRADE("100"), "GND"),
                "devices[0] (C): \"sync\" is not a key of core \"z80\"");
    expect_fail("z80 missing busack", Z80_DEVICE(Z80_KEYS(""), Z80_GRADE("100"), "GND"),
                "devices[0] (C): missing \"busack\"");
    expect_fail("z80 tcc 0", Z80_DEVICE(Z80_KEYS(Z80_BUSACK), Z80_GRADE("0"), "GND"),
                "grades.14: \"tcc\" must be more than 0");
    expect_fail("z80 m1 naming two pins", Z80_DEVICE(Z80_KEYS(Z80_BUSACK), Z80_GRADE("100"), "M1#"),
                "m1: pin name \"M1#\" is used by 2 pins; one is needed");
}

static void check_cpu_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, CPU_OK("65c02", CPU_KEYS(CPU_SYNC) CPU_WDC), &error) == 0);
    part = part_library_find(library, "C1");
    CHECK(part != NULL && part->kind == PART_CPU && part->cpu != NULL &&
          strcmp(part->cpu->core, "65c02") == 0 && strcmp(part->cpu->grade, "14") == 0 &&
          part->cpu->max_hz == 14285714 && part->eeprom == NULL && part->vcc != NULL &&
          part->vcc->min_mv == 1710 && part->vcc->max_mv == 5250);
    printf("ok   valid cpu file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    library = part_library_create();
    error = NULL;
    CHECK(load(library, CPU_OK("6502", CPU_KEYS(CPU_SYNC)), &error) == 0);
    part = part_library_find(library, "C1");
    CHECK(part != NULL && part->cpu != NULL && strcmp(part->cpu->core, "6502") == 0);
    printf("ok   valid 6502 cpu file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("cpu unknown core", CPU_OK("6809", CPU_KEYS(CPU_SYNC)),
                "devices[0] (C): unknown core \"6809\"");
    expect_fail("cpu vectorPull on a 6502", CPU_OK("6502", CPU_KEYS(CPU_SYNC) CPU_WDC),
                "devices[0] (C): \"vectorPull\" is not a key of core \"6502\"");
    /* plan-14-H: the 65816's keys belong to it alone, and its key set has no sync. */
    expect_fail("cpu abort on a 65c02", CPU_OK("65c02", CPU_KEYS(CPU_SYNC) CPU_WDC ",'abort':'NC'"),
                "devices[0] (C): \"abort\" is not a key of core \"65c02\"");
    expect_fail("cpu sync on a 65816", CPU_OK("65816", CPU_KEYS(CPU_SYNC) CPU_WDC),
                "devices[0] (C): \"sync\" is not a key of core \"65816\"");
    expect_fail("cpu missing sync", CPU_OK("65c02", CPU_KEYS("") CPU_WDC),
                "devices[0] (C): missing \"sync\"");
    expect_fail("cpu 15 address names",
                CPU_DEVICE("65c02", "'A0','A1','A2','A3','A4','A5','A6','A7','A8','A9','A10','A11','A12','A13','A14'",
                           CPU_DATA8, CPU_KEYS(CPU_SYNC) CPU_WDC, CPU_GRADE("70"),
                           CPU_P40("p40", "SOB"), CPU_PART("C1", "p40", "")),
                "devices[0] (C): \"address\" needs exactly 16 pin names");
    expect_fail("cpu 7 data names",
                CPU_DEVICE("65c02", CPU_ADDR16, "'D0','D1','D2','D3','D4','D5','D6'",
                           CPU_KEYS(CPU_SYNC) CPU_WDC, CPU_GRADE("70"), CPU_P40("p40", "SOB"),
                           CPU_PART("C1", "p40", "")),
                "devices[0] (C): \"data\" needs exactly 8 pin names");
    expect_fail("cpu control pin absent from one pinout",
                CPU_DEVICE("65c02", CPU_ADDR16, CPU_DATA8, CPU_KEYS(CPU_SYNC) CPU_WDC,
                           CPU_GRADE("70"), CPU_P40("p40", "SOB") "," CPU_P40("p40b", "SO"),
                           CPU_PART("C1", "p40", "") "," CPU_PART("C2", "p40b", "")),
                "so: no pin named \"SOB\"");
    expect_fail("cpu tcyc 0",
                CPU_DEVICE("65c02", CPU_ADDR16, CPU_DATA8, CPU_KEYS(CPU_SYNC) CPU_WDC,
                           CPU_GRADE("0"), CPU_P40("p40", "SOB"), CPU_PART("C1", "p40", "")),
                "grades.14: \"tcyc\" must be more than 0");
    expect_fail("cpu grade missing from grades",
                CPU_DEVICE("65c02", CPU_ADDR16, CPU_DATA8, CPU_KEYS(CPU_SYNC) CPU_WDC,
                           CPU_GRADE("70"), CPU_P40("p40", "SOB"),
                           "{'name':'C1','package':'DIP','pinout':'p40','footprint':'T40','grade':'8'}"),
                "parts[0] (C1): grade \"8\" is not in \"grades\"");
    expect_fail("cpu inputMax below the grade's vcc max",
                CPU_DEVICE("65c02", CPU_ADDR16, CPU_DATA8, CPU_KEYS(CPU_SYNC) CPU_WDC,
                           CPU_GRADE("70"), CPU_P40("p40", "SOB"),
                           CPU_PART("C1", "p40", ",'inputMax':5")),
                "parts[0] (C1): \"inputMax\" 5 is below the \"vcc\" max 5.25");
}

#define DISPLAY_DEVICE(COMMON2, SEGMENT_SIDE) \
    HEAD("7seg") "{'id':'LTS-X','function':'7-Segment','description':'d','drawer':['Indicators','LED 7-Seg']," \
    "'pinouts':{'p4':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'CC','role':'input'}," \
    "{'number':3,'name':'L','role':'input'},{'number':4,'name':'NP','role':'absent'}]}," \
    "'digits':1,'commonAnode':false,'colonAfter':0,'dotAfter':0,'color':'Red','size':'0.3 in'," \
    "'chip':'c','vf':[2,2.6],'leds':[{'digit':1,'segment':'A'},{'digit':0,'segment':'L1'}]," \
    "'segmentPins':{'A':{'digit':0,'segments':['A','L1']}}," \
    "'commonPins':{'CC':1,'L':" COMMON2 "},'segmentSide':" SEGMENT_SIDE ",'commonSide':['CC','L']," \
    "'parts':[{'name':'LTS-X','package':'DIP','pinout':'p4','footprint':'T4'}]}" TAIL

static void check_display_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, DISPLAY_DEVICE("'extras'", "['A']"), &error) == 0);
    part = part_library_find(library, "LTS-X");
    CHECK(part != NULL && part->display != NULL && part->display->led_count == 2 &&
          part->display->leds[1].digit == 0 && strcmp(part->display->leds[1].segment, "L1") == 0 &&
          part->display->segment_pin_count == 1 && part->display->segment_pins[0] == 0 &&
          part->display->common_pin_count == 2 && part->display->common_pins[1] == 2 &&
          part->pins[3].role == PIN_ABSENT);
    printf("ok   valid 7seg file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("7seg common extras misspelled", DISPLAY_DEVICE("'extra'", "['A']"),
                "commonPins.L: a common pin is a digit from 1 to 1 or \"extras\"");
    expect_fail("7seg segmentSide naming a non-segment pin", DISPLAY_DEVICE("'extras'", "['A','CC']"),
                "segmentSide[1]: \"CC\" is not a segment pin");
}

/* An LED with a numbered and a ball pinout; BGA_PINS is the ball pinout's pins. */
#define BALL_LED(BGA_PINS) \
    HEAD("led") "{'id':'led-ball','function':'LED','description':'d','drawer':[]," \
    "'pinouts':{'dip2':[{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}]," \
    "'bga':[" BGA_PINS "]},'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-BALL-DIP','package':'DIP','pinout':'dip2','footprint':'T2'}," \
    "{'name':'LED-BALL-BGA','package':'BGA','pinout':'bga','footprint':'BGA2'}]}" TAIL
#define BALL_LED_PINS "{'ball':'A1','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"

/* An oscillator with a ball pinout; ENABLE is the part's enablePin value. */
#define BALL_OSC(ENABLE) \
    HEAD("oscillator") "{'id':'o','function':'f','description':'d','drawer':[]," \
    "'pinouts':{'bga':[{'ball':'A1','name':'OUT','role':'output'},{'ball':'A2','name':'OE','role':'input'}," \
    "{'ball':'B1','name':'GND','role':'ground'},{'ball':'B2','name':'VDD','role':'power'}]}," \
    "'family':'X','minHz':10,'maxHz':20,'outputs':['OUT'],'vcc':[1.6,5.5]," \
    "'parts':[{'name':'O1','package':'BGA','pinout':'bga','footprint':'BGA4','defaultHz':15,'enablePin':" ENABLE "}]}" TAIL

static void check_ball_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;
    size_t i;

    CHECK(load(library, BALL_LED(BALL_LED_PINS), &error) == 0);
    part = part_library_find(library, "LED-BALL-BGA");
    CHECK(part != NULL && part->pin_count == 2 && strcmp(part->pins[0].designator, "A1") == 0 &&
          strcmp(part->pins[1].designator, "B2") == 0 && strcmp(part->pins[1].name, "A") == 0 &&
          part->led != NULL && part_internal(part)->bound != NULL);
    part = part_library_find(library, "LED-BALL-DIP");
    CHECK(part != NULL && strcmp(part->pins[1].designator, "2") == 0);
    printf("ok   ball pinout beside a numbered one%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    error = NULL;
    part_library_destroy(library);

    library = part_library_create();
    CHECK(load(library, BALL_OSC("'A2'"), &error) == 0);
    part = part_library_find(library, "O1");
    CHECK(part != NULL && part->oscillator != NULL && part->oscillator->enable_pin != NULL);
    for (i = 0; part != NULL && i < part->pin_count; i++) {
        if (strcmp(part->pins[i].designator, "A2") == 0) {
            CHECK(part->oscillator->enable_pin == part->pins[i].designator);
        }
    }
    printf("ok   ball oscillator with enablePin \"A2\"%s%s\n", error != NULL ? ": " : "",
           error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("ball pin with both keys",
                BALL_LED("{'ball':'A1','number':1,'name':'K','role':'input'},"
                         "{'ball':'B2','name':'A','role':'input'}"),
                "pinouts.bga: pin 1: a pin has \"number\" or \"ball\", not both");
    expect_fail("ball pinout pin with neither key",
                BALL_LED("{'ball':'A1','name':'K','role':'input'},{'name':'A','role':'input'}"),
                "pinouts.bga: pin 2: a pin needs \"number\" or \"ball\"");
    expect_fail("ball and number mixed in one pinout",
                BALL_LED("{'ball':'A1','name':'K','role':'input'},{'number':2,'name':'A','role':'input'}"),
                "pin 2: every pin of a pinout uses \"number\", or every pin uses \"ball\"");
    expect_fail("ball row I",
                BALL_LED("{'ball':'I1','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"),
                "\"I1\" is not a ball name");
    expect_fail("ball column 0",
                BALL_LED("{'ball':'A0','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"),
                "\"A0\" is not a ball name");
    expect_fail("ball column with a leading zero",
                BALL_LED("{'ball':'A01','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"),
                "\"A01\" is not a ball name");
    expect_fail("ball row of three letters",
                BALL_LED("{'ball':'AAA1','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"),
                "\"AAA1\" is not a ball name");
    expect_fail("ball used twice",
                BALL_LED("{'ball':'A1','name':'K','role':'input'},{'ball':'A1','name':'A','role':'input'}"),
                "pin 2: ball \"A1\" is used twice");
    expect_fail("absent ball in a ball pinout",
                BALL_LED(BALL_LED_PINS ",{'ball':'C3','name':'NP','role':'absent'}"),
                "pin 3: a ball pinout lists only real balls; leave \"absent\" positions out");
    expect_fail("ball oscillator enablePin Z9", BALL_OSC("'Z9'"),
                "\"enablePin\" Z9 is not a pin of pinout \"bga\"");
    expect_fail("ball oscillator enablePin true", BALL_OSC("true"),
                "\"enablePin\" must be a pin number or a ball name");
}

/* ---- Footprints (plan-03-A) ------------------------------------------------ */

#define FP_FILE(FPS) "{'format':'schematical-footprints','version':1,'footprints':[" FPS "]}"

static int load_fp(PartLibrary *library, const char *text, char **error) {
    char *source = json(text);
    int result = load_footprints(library, source, "fp.json", error);

    free(source);
    return result;
}

/* Loads footprints into a fresh library; expects failure with `expect`, nothing added. */
static void expect_fp_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    int result = load_fp(library, text, &error);

    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_footprint_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_footprint_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    part_library_destroy(library);
}

/* Loads good footprints, then a part file expected to fail with `expect`. */
static void expect_part_fail(const char *name, const char *footprints, const char *parts,
                             const char *expect) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    int result;

    CHECK(load_fp(library, footprints, &error) == 0);
    free(error);
    error = NULL;
    result = load(library, parts, &error);
    if (result != -1 || error == NULL || strstr(error, expect) == NULL ||
        part_library_count(library) != 0) {
        printf("FAIL %s: result %d, count %zu, error \"%s\" (wanted \"%s\")\n", name, result,
               part_library_count(library), error != NULL ? error : "(none)", expect);
        failures++;
    } else {
        printf("ok   %s: %s\n", name, error);
    }
    free(error);
    part_library_destroy(library);
}

static int near(double a, double b) {
    return fabs(a - b) < 1e-9;
}

/* Checks pad `index` of a footprint; designator NULL = unnumbered. */
static void check_pad(const PartFootprint *fp, size_t index, const char *designator, double x,
                      double y, double width, double height, double drill, PadShape shape) {
    const FootprintPad *pad;
    int same_name;

    if (fp == NULL || index >= fp->pad_count) {
        printf("FAIL pad %zu of %s: missing\n", index, fp != NULL ? fp->id : "(null)");
        failures++;
        return;
    }
    pad = &fp->pads[index];
    same_name = designator == NULL ? pad->designator == NULL
                                   : pad->designator != NULL && strcmp(pad->designator, designator) == 0;
    if (!same_name || !near(pad->x, x) || !near(pad->y, y) || !near(pad->width, width) ||
        !near(pad->height, height) || !near(pad->drill, drill) || pad->shape != shape) {
        printf("FAIL pad %zu of %s: %s %g,%g %gx%g d%g %d (wanted %s %g,%g %gx%g d%g %d)\n", index,
               fp->id, pad->designator != NULL ? pad->designator : "-", pad->x, pad->y, pad->width,
               pad->height, pad->drill, (int)pad->shape, designator != NULL ? designator : "-", x, y,
               width, height, drill, (int)shape);
        failures++;
    }
}

#define FP_DIP14 \
    "{'id':'DIP-14','description':'d','note':'n','style':'dual','pins':14," \
    "'body':{'width':6.35,'height':19.3},'pitch':2.54,'span':7.62," \
    "'pad':{'width':1.6,'height':1.6,'shape':'round'},'drill':0.8,'pin1Shape':'rect'}"
#define FP_SOT5 \
    "{'id':'SOT-5','description':'d','style':'dual','pins':5,'perSide':3,'slots':[1,2,3,4,6]," \
    "'body':{'width':1.6,'height':2.9},'pitch':0.95,'span':2.6," \
    "'pad':{'width':1.1,'height':0.6,'shape':'rect'}}"
#define FP_LCCC20 \
    "{'id':'LCCC-20','description':'d','style':'quad','pins':20,'sides':[5,5,5,5]," \
    "'slots':[18,19,20,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]," \
    "'body':{'width':8.9,'height':8.9},'pitch':1.27,'spanX':8,'spanY':8," \
    "'pad':{'width':2,'height':0.65,'shape':'rect'},'centre':{'width':3,'height':3,'shape':'rect'}}"
#define FP_X2SON5 \
    "{'id':'X2SON-5','description':'d','style':'dual','pins':5,'perSide':2,'slots':[1,2,0,3,4]," \
    "'body':{'width':0.8,'height':0.8},'pitch':1,'span':2," \
    "'pad':{'width':0.5,'height':0.3,'shape':'rect'},'centre':{'width':0.4,'height':0.4,'shape':'round'}}"
#define FP_EXPLICIT4 \
    "{'id':'OSC-4','description':'d','style':'explicit','pins':4,'body':{'width':2,'height':1.2}," \
    "'pads':[{'pin':2,'x':-0.3,'y':0.4,'width':0.3,'height':0.3,'shape':'rect'}," \
    "{'pin':1,'x':-0.9,'y':0,'width':0.6,'height':1.2,'shape':'rect','drill':0.2}," \
    "{'pin':0,'x':0,'y':0,'width':0.2,'height':0.2,'shape':'round'}," \
    "{'pin':4,'x':0.9,'y':0,'width':0.6,'height':1.2,'shape':'rect'}," \
    "{'pin':3,'x':0.3,'y':0.4,'width':0.3,'height':0.3,'shape':'round'}]}"
#define FP_GRID \
    "{'id':'BGA-4','description':'d','style':'grid','pins':4,'rows':2,'columns':3,'pitch':0.5," \
    "'balls':['B3','A1','A2','B1'],'body':{'width':1.5,'height':1,'round':true}," \
    "'pad':{'width':0.25,'height':0.25,'shape':'round'}}"

static void check_footprint_styles(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library, FP_FILE(FP_DIP14 "," FP_SOT5 "," FP_LCCC20), &error) == 0);
    CHECK(error == NULL);
    free(error);
    error = NULL;
    /* A second call adds to the first. */
    CHECK(load_fp(library, FP_FILE(FP_X2SON5 "," FP_EXPLICIT4 "," FP_GRID), &error) == 0);
    CHECK(error == NULL);
    CHECK(part_library_footprint_count(library) == 6);
    CHECK(part_library_footprint_at(library, 0) == part_library_find_footprint(library, "DIP-14"));
    CHECK(part_library_footprint_at(library, 5) == part_library_find_footprint(library, "BGA-4"));
    CHECK(part_library_find_footprint(library, "NOPE") == NULL);

    fp = part_library_find_footprint(library, "DIP-14");
    CHECK(fp != NULL && fp->pins == 14 && fp->pad_count == 14 && near(fp->body_height, 19.3) &&
          !fp->body_round && strcmp(fp->description, "d") == 0);
    check_pad(fp, 0, "1", -3.81, -7.62, 1.6, 1.6, 0.8, PAD_RECT);
    check_pad(fp, 1, "2", -3.81, -5.08, 1.6, 1.6, 0.8, PAD_ROUND);
    check_pad(fp, 6, "7", -3.81, 7.62, 1.6, 1.6, 0.8, PAD_ROUND);
    check_pad(fp, 7, "8", 3.81, 7.62, 1.6, 1.6, 0.8, PAD_ROUND);
    check_pad(fp, 13, "14", 3.81, -7.62, 1.6, 1.6, 0.8, PAD_ROUND);

    fp = part_library_find_footprint(library, "SOT-5");
    CHECK(fp != NULL && fp->pad_count == 5);
    check_pad(fp, 0, "1", -1.3, -0.95, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 1, "2", -1.3, 0, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 2, "3", -1.3, 0.95, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 3, "4", 1.3, 0.95, 1.1, 0.6, 0, PAD_RECT);
    check_pad(fp, 4, "5", 1.3, -0.95, 1.1, 0.6, 0, PAD_RECT);   /* slot 6; slot 5 is empty */

    fp = part_library_find_footprint(library, "LCCC-20");
    CHECK(fp != NULL && fp->pad_count == 21);
    check_pad(fp, 0, "1", 0, -4, 0.65, 2, 0, PAD_RECT);         /* top centre, turned */
    check_pad(fp, 2, "3", -2.54, -4, 0.65, 2, 0, PAD_RECT);     /* top, leftmost */
    check_pad(fp, 3, "4", -4, -2.54, 2, 0.65, 0, PAD_RECT);     /* left, top */
    check_pad(fp, 12, "13", 2.54, 4, 0.65, 2, 0, PAD_RECT);     /* bottom, rightmost */
    check_pad(fp, 13, "14", 4, 2.54, 2, 0.65, 0, PAD_RECT);     /* right, bottom */
    check_pad(fp, 20, NULL, 0, 0, 3, 3, 0, PAD_RECT);           /* unnumbered centre, last */

    fp = part_library_find_footprint(library, "X2SON-5");
    CHECK(fp != NULL && fp->pad_count == 5);
    check_pad(fp, 0, "1", -1, -0.5, 0.5, 0.3, 0, PAD_RECT);
    check_pad(fp, 1, "2", -1, 0.5, 0.5, 0.3, 0, PAD_RECT);
    check_pad(fp, 2, "3", 0, 0, 0.4, 0.4, 0, PAD_ROUND);        /* pin 3 is the centre */
    check_pad(fp, 3, "4", 1, 0.5, 0.5, 0.3, 0, PAD_RECT);
    check_pad(fp, 4, "5", 1, -0.5, 0.5, 0.3, 0, PAD_RECT);

    fp = part_library_find_footprint(library, "OSC-4");
    CHECK(fp != NULL && fp->pad_count == 5);
    check_pad(fp, 0, "1", -0.9, 0, 0.6, 1.2, 0.2, PAD_RECT);
    check_pad(fp, 1, "2", -0.3, 0.4, 0.3, 0.3, 0, PAD_RECT);
    check_pad(fp, 2, "3", 0.3, 0.4, 0.3, 0.3, 0, PAD_ROUND);
    check_pad(fp, 3, "4", 0.9, 0, 0.6, 1.2, 0, PAD_RECT);
    check_pad(fp, 4, NULL, 0, 0, 0.2, 0.2, 0, PAD_ROUND);

    fp = part_library_find_footprint(library, "BGA-4");
    CHECK(fp != NULL && fp->pad_count == 4 && fp->body_round);
    check_pad(fp, 0, "A1", -0.5, -0.25, 0.25, 0.25, 0, PAD_ROUND);
    check_pad(fp, 1, "A2", 0, -0.25, 0.25, 0.25, 0, PAD_ROUND);
    check_pad(fp, 2, "B1", -0.5, 0.25, 0.25, 0.25, 0, PAD_ROUND);
    check_pad(fp, 3, "B3", 0.5, 0.25, 0.25, 0.25, 0, PAD_ROUND);

    /* Ids are unique across calls; a failed file adds nothing. */
    free(error);
    error = NULL;
    CHECK(load_fp(library, FP_FILE(FP_GRID), &error) == -1);
    CHECK(error != NULL && strstr(error, "footprints[0] (BGA-4): duplicate footprint id \"BGA-4\"") != NULL);
    printf("ok   footprint id repeated across calls: %s\n", error != NULL ? error : "(none)");
    CHECK(part_library_footprint_count(library) == 6);
    free(error);
    part_library_destroy(library);
    printf("ok   footprints of every style expand to the expected pads\n");
}

/* One footprint F of a style, KEYS supplying the rest. */
#define FP_DUAL(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'dual','body':{'width':1,'height':1}," \
            "'pad':{'width':0.5,'height':0.5,'shape':'rect'}," KEYS "}")
#define FP_QUAD(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'quad','body':{'width':1,'height':1}," \
            "'pad':{'width':0.5,'height':0.5,'shape':'rect'},'pitch':0.5,'spanX':3,'spanY':3," KEYS "}")
#define FP_GRID_WITH(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'grid','body':{'width':1,'height':1}," \
            "'pad':{'width':0.2,'height':0.2,'shape':'round'},'rows':2,'columns':2,'pitch':0.5," KEYS "}")
#define FP_EXPLICIT_WITH(KEYS) \
    FP_FILE("{'id':'F','description':'d','style':'explicit','pins':2,'body':{'width':1,'height':1}," KEYS "}")
#define EPAD(PIN, X) "{'pin':" PIN ",'x':" X ",'y':0,'width':0.4,'height':0.4,'shape':'rect'}"

/* Checks a pad's corner keys: radius, chamfer and the PAD_CORNER_* bits. */
static void check_corners(const PartFootprint *fp, size_t index, double radius, double chamfer,
                          unsigned corners) {
    const FootprintPad *pad = fp != NULL && index < fp->pad_count ? &fp->pads[index] : NULL;

    if (pad == NULL || !near(pad->radius, radius) || !near(pad->chamfer, chamfer) ||
        pad->corners != corners) {
        printf("FAIL corners of pad %zu of %s: r%g c%g %u (wanted r%g c%g %u)\n", index,
               fp != NULL ? fp->id : "(null)", pad != NULL ? pad->radius : -1,
               pad != NULL ? pad->chamfer : -1, pad != NULL ? pad->corners : 0u, radius, chamfer,
               corners);
        failures++;
    }
}

enum { TL = PAD_CORNER_TL, TR = PAD_CORNER_TR, BR = PAD_CORNER_BR, BL = PAD_CORNER_BL };

static void check_footprint_corners(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library, FP_FILE(
        /* Inner corners cut, named for the top-left pad; centre a diamond. */
        "{'id':'CUT-5','description':'d','style':'dual','pins':5,'perSide':3,'slots':[1,2,3,0,4]," \
        "'body':{'width':1,'height':1},'pitch':0.5,'span':1," \
        "'pad':{'width':0.4,'height':0.2,'shape':'rect','chamfer':0.05,'corners':['br']}," \
        "'centre':{'width':0.3,'height':0.3,'shape':'rect','chamfer':0.15,'corners':['tl','tr','br','bl']}}," \
        /* Rounded quad pads; pin 1 keeps its plain pin1Shape. */
        "{'id':'ROUND-Q','description':'d','style':'quad','pins':4,'sides':[1,1,1,1]," \
        "'body':{'width':2,'height':2},'pitch':0.5,'spanX':2,'spanY':2," \
        "'pad':{'width':0.6,'height':0.3,'shape':'rect','radius':0.15},'pin1Shape':'rect'}," \
        "{'id':'CUT-Q','description':'d','style':'quad','pins':8,'sides':[2,2,2,2]," \
        "'body':{'width':2,'height':2},'pitch':0.5,'spanX':2,'spanY':2," \
        "'pad':{'width':0.6,'height':0.3,'shape':'rect','chamfer':0.1,'corners':['tl']}}," \
        /* Explicit pads take corners as named. */
        "{'id':'CUT-E','description':'d','style':'explicit','pins':2,'body':{'width':1,'height':1}," \
        "'pads':[{'pin':1,'x':-0.5,'y':0,'width':0.4,'height':0.4,'shape':'rect','chamfer':0.1,'corners':['tl','bl']}," \
        "{'pin':2,'x':0.5,'y':0,'width':0.4,'height':0.2,'shape':'rect','radius':0.1}]}"), &error) == 0);
    CHECK(error == NULL);

    fp = part_library_find_footprint(library, "CUT-5");
    check_pad(fp, 0, "1", -0.5, -0.5, 0.4, 0.2, 0, PAD_RECT);
    check_corners(fp, 0, 0, 0.05, BR);           /* up-left: as named */
    check_corners(fp, 1, 0, 0.05, BR | TR);      /* on the centre line: both inner corners */
    check_corners(fp, 2, 0, 0.05, TR);           /* down-left: top/bottom mirrored */
    check_corners(fp, 3, 0, 0.15, TL | TR | BR | BL);   /* pin 4 = the centre, as named */
    check_corners(fp, 4, 0, 0.05, TL);           /* pin 5 on slot 4, down-right: both mirrored */

    fp = part_library_find_footprint(library, "ROUND-Q");
    check_pad(fp, 0, "1", -1, 0, 0.6, 0.3, 0, PAD_RECT);
    check_corners(fp, 0, 0, 0, 0);               /* pin1Shape: plain */
    check_pad(fp, 1, "2", 0, 1, 0.3, 0.6, 0, PAD_RECT);
    check_corners(fp, 1, 0.15, 0, 0);            /* turned, still rounded */

    fp = part_library_find_footprint(library, "CUT-Q");
    check_corners(fp, 0, 0, 0.1, TL);            /* left side, upper */
    check_corners(fp, 1, 0, 0.1, BL);            /* left side, lower */
    check_corners(fp, 2, 0, 0.1, BL);            /* bottom row, left (turned) */
    check_corners(fp, 3, 0, 0.1, BR);            /* bottom row, right */
    check_corners(fp, 6, 0, 0.1, TR);            /* top row, right */
    check_corners(fp, 7, 0, 0.1, TL);            /* top row, left */

    fp = part_library_find_footprint(library, "CUT-E");
    check_corners(fp, 0, 0, 0.1, TL | BL);
    check_corners(fp, 1, 0.1, 0, 0);
    free(error);
    part_library_destroy(library);
    printf("ok   rounded and cut pad corners expand as named and mirrored\n");

#define CPAD(KEYS) FP_DUAL("'pins':2,'pitch':1,'span':1,'centre':{'width':0.4,'height':0.2," KEYS "}")
    expect_fp_fail("pad corners: radius on a round pad", CPAD("'shape':'round','radius':0.05"),
                   "centre: \"radius\" needs a \"rect\" pad");
    expect_fp_fail("pad corners: chamfer on a round pad",
                   CPAD("'shape':'round','chamfer':0.05,'corners':['tl']"),
                   "centre: \"chamfer\" needs a \"rect\" pad");
    expect_fp_fail("pad corners: both", CPAD("'shape':'rect','radius':0.05,'chamfer':0.05,'corners':['tl']"),
                   "a pad takes \"radius\" or \"chamfer\", not both");
    expect_fp_fail("pad corners: radius too big", CPAD("'shape':'rect','radius':0.11"),
                   "\"radius\" 0.11 is more than half the pad's shorter side (0.1)");
    expect_fp_fail("pad corners: radius 0", CPAD("'shape':'rect','radius':0"),
                   "\"radius\" must be greater than 0");
    expect_fp_fail("pad corners: chamfer past an edge", CPAD("'shape':'rect','chamfer':0.25,'corners':['tl']"),
                   "\"chamfer\" 0.25 cuts past the left edge (0.2 long)");
    expect_fp_fail("pad corners: cuts meet on an edge", CPAD("'shape':'rect','chamfer':0.15,'corners':['tl','bl']"),
                   "\"chamfer\" 0.15 cuts both ends of the left edge (0.2 long)");
    expect_fp_fail("pad corners: chamfer without corners", CPAD("'shape':'rect','chamfer':0.05"),
                   "\"chamfer\" needs \"corners\"");
    expect_fp_fail("pad corners: corners without chamfer", CPAD("'shape':'rect','corners':['tl']"),
                   "\"corners\" needs \"chamfer\"");
    expect_fp_fail("pad corners: corners with radius", CPAD("'shape':'rect','radius':0.05,'corners':['tl']"),
                   "\"corners\" needs \"chamfer\"; \"radius\" rounds every corner");
    expect_fp_fail("pad corners: empty corners", CPAD("'shape':'rect','chamfer':0.05,'corners':[]"),
                   "\"corners\" must be a non-empty array of \"tl\", \"tr\", \"br\", \"bl\"");
    expect_fp_fail("pad corners: unknown corner", CPAD("'shape':'rect','chamfer':0.05,'corners':['top']"),
                   "\"corners\" must be a non-empty array of \"tl\", \"tr\", \"br\", \"bl\"");
    expect_fp_fail("pad corners: corner twice", CPAD("'shape':'rect','chamfer':0.05,'corners':['tl','tl']"),
                   "corner \"tl\" is named twice");
#undef CPAD
    expect_fp_fail("pad corners: mirrored onto the centre line",
                   FP_FILE("{'id':'F','description':'d','style':'dual','body':{'width':1,'height':1},"
                           "'pins':6,'pitch':1,'span':2,'pad':{'width':0.5,'height':0.2,'shape':'rect',"
                           "'chamfer':0.15,'corners':['br']}}"),
                   "pad 2 (mirrored corners): \"chamfer\" 0.15 cuts both ends of the right edge (0.2 long)");
    expect_fp_fail("pad corners: explicit pad",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':1,'y':0,'width':0.4,"
                                    "'height':0.4,'shape':'round','radius':0.1}]"),
                   "pads[1]: \"radius\" needs a \"rect\" pad");
}

static void check_footprint_cases(void) {
    expect_fp_fail("footprints: bad JSON",
                   "{'format':'schematical-footprints','version':1,'footprints':[" FP_SOT5 ",",
                   "invalid JSON at byte");
    expect_fp_fail("footprints: wrong format",
                   "{'format':'schematical-parts','version':1,'footprints':[]}",
                   "fp.json: \"format\" must be \"schematical-footprints\"");
    expect_fp_fail("footprints: version 2",
                   "{'format':'schematical-footprints','version':2,'footprints':[]}",
                   "unsupported version 2");
    expect_fp_fail("footprints: unknown file key",
                   "{'format':'schematical-footprints','version':1,'footprints':[],'kind':'led'}",
                   "unknown key \"kind\"");
    expect_fp_fail("footprint: unknown key", FP_DUAL("'pins':2,'pitch':1,'span':1,'colour':'x'"),
                   "footprints[0] (F): unknown key \"colour\"");
    expect_fp_fail("footprint: another style's key", FP_DUAL("'pins':2,'pitch':1,'span':1,'rows':2"),
                   "footprints[0] (F): unknown key \"rows\"");
    expect_fp_fail("footprint: duplicate key", FP_DUAL("'pins':2,'pitch':1,'span':1,'span':2"),
                   "duplicate key \"span\"");
    expect_fp_fail("footprint: missing span", FP_DUAL("'pins':2,'pitch':1"),
                   "footprints[0] (F): missing \"span\"");
    expect_fp_fail("footprint: missing body",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"),
                   "missing \"body\"");
    expect_fp_fail("footprint: unknown style",
                   FP_FILE("{'id':'F','description':'d','style':'radial','pins':2,'body':{'width':1,'height':1}}"),
                   "unknown style \"radial\"");
    expect_fp_fail("footprint: bad id",
                   FP_FILE("{'id':'soic 14','description':'d','style':'dual','pins':2}"),
                   "\"id\" must use only A-Z, 0-9, '.' and '-'");
    expect_fp_fail("footprint: duplicate id in one file", FP_FILE(FP_SOT5 "," FP_SOT5),
                   "footprints[1] (SOT-5): duplicate footprint id \"SOT-5\"");
    expect_fp_fail("footprint: pins 0", FP_DUAL("'pins':0,'pitch':1,'span':1"),
                   "\"pins\" must be at least 1");
    expect_fp_fail("footprint: body width 0",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'body':{'width':0,'height':1},'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"),
                   "footprints[0] (F): body: \"width\" must be greater than 0");
    expect_fp_fail("footprint: pad height -1",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'body':{'width':1,'height':1},'pad':{'width':0.5,'height':-1,'shape':'rect'}}"),
                   "pad: \"height\" must be greater than 0");
    expect_fp_fail("footprint: pad shape oval",
                   FP_FILE("{'id':'F','description':'d','style':'dual','pins':2,'pitch':1,'span':1,"
                           "'body':{'width':1,'height':1},'pad':{'width':0.5,'height':0.5,'shape':'oval'}}"),
                   "\"shape\" must be \"rect\" or \"round\", not \"oval\"");
    expect_fp_fail("footprint: pitch 0", FP_DUAL("'pins':2,'pitch':0,'span':1"),
                   "\"pitch\" must be greater than 0");
    expect_fp_fail("footprint: span -1", FP_DUAL("'pins':2,'pitch':1,'span':-1"),
                   "\"span\" must be greater than 0");
    expect_fp_fail("footprint: drill as wide as the pad", FP_DUAL("'pins':2,'pitch':1,'span':1,'drill':0.5"),
                   "\"drill\" 0.5 must be smaller than the pad (0.5 x 0.5)");
    expect_fp_fail("footprint: pin1Shape square", FP_DUAL("'pins':2,'pitch':1,'span':1,'pin1Shape':'square'"),
                   "\"pin1Shape\" must be \"rect\" or \"round\"");
    expect_fp_fail("dual: odd pins without perSide", FP_DUAL("'pins':5,'pitch':1,'span':1"),
                   "an odd \"pins\" (5) needs \"perSide\"");
    expect_fp_fail("dual: perSide 0", FP_DUAL("'pins':2,'perSide':0,'pitch':1,'span':1"),
                   "\"perSide\" must be at least 1");
    expect_fp_fail("dual: perSide x 2 < pins without slots", FP_DUAL("'pins':5,'perSide':2,'pitch':1,'span':1"),
                   "5 pins do not fit on 4 slots");
    expect_fp_fail("quad: slot total < pins without slots", FP_QUAD("'pins':5,'sides':[1,1,1,1]"),
                   "5 pins do not fit on 4 slots");
    expect_fp_fail("quad: three sides", FP_QUAD("'pins':3,'sides':[1,1,1]"),
                   "\"sides\" must be 4 slot counts [left, bottom, right, top]");
    expect_fp_fail("slots: wrong length", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,2]"),
                   "slots: \"slots\" must be an array of 3 slots, one per pin");
    expect_fp_fail("slots: duplicate", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,2,2]"),
                   "slots: slot 2 is used twice");
    expect_fp_fail("slots: beyond the slot count",
                   FP_DUAL("'pins':5,'perSide':3,'pitch':1,'span':1,'slots':[1,2,3,4,7]"),
                   "footprints[0] (F): slots: slot 7 is beyond 6 slots");
    expect_fp_fail("slots: not a whole number", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,2,2.5]"),
                   "slots[2] must be a whole number");
    expect_fp_fail("slots: 0 without centre", FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,0,2]"),
                   "slot 0 (the centre pad) needs \"centre\"");
    expect_fp_fail("slots: two 0s",
                   FP_DUAL("'pins':3,'perSide':2,'pitch':1,'span':1,'slots':[1,0,0],"
                           "'centre':{'width':0.3,'height':0.3,'shape':'rect'}"),
                   "two pins are on slot 0 (the centre pad)");
    expect_fp_fail("grid: centre", FP_GRID_WITH("'pins':1,'balls':['A1'],'centre':{'width':0.3,'height':0.3,'shape':'rect'}"),
                   "\"centre\" is not allowed on a grid footprint");
    expect_fp_fail("grid: slots", FP_GRID_WITH("'pins':1,'balls':['A1'],'slots':[1]"),
                   "\"slots\" is not allowed on a grid footprint");
    expect_fp_fail("grid: pin1Shape", FP_GRID_WITH("'pins':1,'balls':['A1'],'pin1Shape':'rect'"),
                   "\"pin1Shape\" is not allowed on a grid footprint");
    expect_fp_fail("grid: rows 21",
                   FP_FILE("{'id':'F','description':'d','style':'grid','body':{'width':1,'height':1},"
                           "'pad':{'width':0.2,'height':0.2,'shape':'round'},'rows':21,'columns':2,"
                           "'pitch':0.5,'pins':1,'balls':['A1']}"),
                   "\"rows\" must be a whole number from 0 to 20");
    expect_fp_fail("grid: ball outside the grid (row)", FP_GRID_WITH("'pins':2,'balls':['A1','C1']"),
                   "balls: ball \"C1\" is outside the 2-row, 2-column grid");
    expect_fp_fail("grid: ball outside the grid (column)", FP_GRID_WITH("'pins':2,'balls':['A1','B3']"),
                   "ball \"B3\" is outside the 2-row, 2-column grid");
    expect_fp_fail("grid: two-letter row", FP_GRID_WITH("'pins':2,'balls':['A1','AA1']"),
                   "ball \"AA1\" is outside the 2-row, 2-column grid");
    expect_fp_fail("grid: duplicate ball", FP_GRID_WITH("'pins':2,'balls':['A1','A1']"),
                   "ball \"A1\" is used twice");
    expect_fp_fail("grid: wrong balls length", FP_GRID_WITH("'pins':3,'balls':['A1','A2']"),
                   "\"balls\" must be an array of 3 ball names, one per pin");
    expect_fp_fail("grid: not a ball name", FP_GRID_WITH("'pins':2,'balls':['A1','I1']"),
                   "\"I1\" is not a ball name");
    expect_fp_fail("explicit: pin missing", FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("0", "1") "]"),
                   "pin 2 has no pad");
    expect_fp_fail("explicit: pin beyond pins",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("3", "1") "]"),
                   "pads[1]: \"pin\" must be a whole number from 0 to 2");
    expect_fp_fail("explicit: parametric pad",
                   FP_EXPLICIT_WITH("'pad':{'width':0.5,'height':0.5,'shape':'rect'},"
                                    "'pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"),
                   "\"pad\" is not allowed on an explicit footprint");
    expect_fp_fail("explicit: parametric pitch",
                   FP_EXPLICIT_WITH("'pitch':1,'pads':[" EPAD("1", "0") "," EPAD("2", "1") "]"),
                   "unknown key \"pitch\"");
    expect_fp_fail("explicit: pad drill too big",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") ",{'pin':2,'x':1,'y':0,'width':0.4,"
                                    "'height':0.4,'shape':'rect','drill':0.4}]"),
                   "pads[1]: \"drill\" 0.4 must be smaller than the pad (0.4 x 0.4)");
}

/* plan-11-A: an explicit pin may have several pads; they come out grouped by
   pin, file order within a pin, unnumbered last. */
static void check_explicit_repeated_pins(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const PartFootprint *fp;

    CHECK(load_fp(library,
                  FP_EXPLICIT_WITH("'pads':[" EPAD("1", "0") "," EPAD("2", "1") "," EPAD("1", "2") "]"),
                  &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->pins == 2 && fp->pad_count == 3);
    check_pad(fp, 0, "1", 0, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 1, "1", 2, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 2, "2", 1, 0, 0.4, 0.4, 0, PAD_RECT);
    part_library_destroy(library);
    printf("ok   explicit: pin repeated loads as 1, 1, 2\n");

    library = part_library_create();
    CHECK(load_fp(library,
                  FP_EXPLICIT_WITH("'pads':[" EPAD("2", "0") "," EPAD("1", "-1") "," EPAD("0", "3") ","
                                   EPAD("1", "1") "]"),
                  &error) == 0);
    fp = part_library_find_footprint(library, "F");
    CHECK(fp != NULL && fp->pins == 2 && fp->pad_count == fp->pins + 2);
    check_pad(fp, 0, "1", -1, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 1, "1", 1, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 2, "2", 0, 0, 0.4, 0.4, 0, PAD_RECT);
    check_pad(fp, 3, NULL, 3, 0, 0.4, 0.4, 0, PAD_RECT);
    free(error);
    part_library_destroy(library);
    printf("ok   explicit: pin 1 at x -1 and 1 comes out pads[0], pads[1], file order\n");

    expect_fp_fail("explicit: pin 1 twice, pin 2 missing",
                   FP_EXPLICIT_WITH("'pads':[" EPAD("1", "-1") "," EPAD("1", "1") "]"), "pin 2 has no pad");
}

/* An LED with one pinout of PINS whose part names FOOTPRINT (a JSON value). */
#define FP_LED(PINS, FOOTPRINT) \
    HEAD("led") "{'id':'led-fp','function':'LED','description':'d','drawer':[]," \
    "'pinouts':{'p':[" PINS "]},'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-FP','package':'X','pinout':'p','footprint':" FOOTPRINT "}]}" TAIL
#define FP_LED_NUMBERED "{'number':1,'name':'K','role':'input'},{'number':2,'name':'A','role':'input'}"
#define FP_LED_BALLS "{'ball':'A1','name':'K','role':'input'},{'ball':'B2','name':'A','role':'input'}"
#define FP_CHIP2 \
    "{'id':'CHIP-2','description':'d','style':'dual','pins':2,'perSide':1,'body':{'width':1,'height':0.5}," \
    "'pitch':1,'span':0.9,'pad':{'width':0.5,'height':0.7,'shape':'rect'}}"
#define FP_BGA2 \
    "{'id':'BGA-2','description':'d','style':'grid','pins':2,'rows':2,'columns':2,'pitch':0.5," \
    "'balls':['A1','A2'],'body':{'width':1,'height':1},'pad':{'width':0.2,'height':0.2,'shape':'round'}}"

#define FP_CHIP2B \
    "{'id':'CHIP-2B','description':'d','style':'dual','pins':2,'perSide':1,'body':{'width':1,'height':0.5}," \
    "'pitch':2,'span':0.9,'pad':{'width':0.5,'height':0.7,'shape':'rect'}}"
/* The FP_LED part on CHIP-2 with CONN (a JSON value) as its connectorFootprint. */
#define FP_LED_CONN(CONN) \
    HEAD("led") "{'id':'led-fp','function':'LED','description':'d','drawer':[]," \
    "'pinouts':{'p':[" FP_LED_NUMBERED "]},'colors':['Red'],'anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-FP','package':'X','pinout':'p','footprint':'CHIP-2'," \
    "'connectorFootprint':" CONN "}]}" TAIL

static void check_part_footprints(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load_fp(library, FP_FILE(FP_CHIP2), &error) == 0);
    CHECK(load(library, FP_LED(FP_LED_NUMBERED, "'CHIP-2'"), &error) == 0);
    CHECK(load(library, HEAD("supply") VCC_DEVICE TAIL, &error) == 0);
    part = part_library_find(library, "LED-FP");
    CHECK(part != NULL && part->footprint == part_library_find_footprint(library, "CHIP-2"));
    part = part_library_find(library, "5V");
    CHECK(part != NULL && part->footprint == NULL);
    free(error);
    part_library_destroy(library);
    printf("ok   a part's \"footprint\" resolves; a part without one has none\n");

    expect_part_fail("part: unknown footprint", FP_FILE(FP_CHIP2), FP_LED(FP_LED_NUMBERED, "'CHIP-3'"),
                     "parts[0] (LED-FP): unknown footprint \"CHIP-3\"");
    expect_part_fail("part: pad count mismatch", FP_FILE(FP_SOT5), FP_LED(FP_LED_NUMBERED, "'SOT-5'"),
                     "footprint \"SOT-5\" has 5 pads, pinout \"p\" has 2 pins");
    expect_part_fail("part: pin without a pad", FP_FILE(FP_BGA2), FP_LED(FP_LED_BALLS, "'BGA-2'"),
                     "pin \"B2\" has no pad in footprint \"BGA-2\"");
    expect_part_fail("part: numbered pins on a grid footprint", FP_FILE(FP_BGA2),
                     FP_LED(FP_LED_NUMBERED, "'BGA-2'"), "pin \"1\" has no pad in footprint \"BGA-2\"");
    expect_part_fail("part: footprint not a string", FP_FILE(FP_CHIP2), FP_LED(FP_LED_NUMBERED, "2"),
                     "\"footprint\" must be a string");

    /* plan-11-A: "connectorFootprint", optional, checked like "footprint". */
    {
        PartLibrary *lib = part_library_create();
        char *err = NULL;

        CHECK(load_fp(lib, FP_FILE(FP_CHIP2 "," FP_CHIP2B), &err) == 0);
        CHECK(load(lib, FP_LED_CONN("'CHIP-2B'"), &err) == 0);
        part = part_library_find(lib, "LED-FP");
        CHECK(part != NULL && part->footprint == part_library_find_footprint(lib, "CHIP-2") &&
              part->connector_footprint != NULL &&
              strcmp(part->connector_footprint->id, "CHIP-2B") == 0);
        free(err);
        part_library_destroy(lib);
        printf("ok   a part's \"connectorFootprint\" resolves\n");
    }
    expect_part_fail("part: unknown connectorFootprint", FP_FILE(FP_CHIP2), FP_LED_CONN("'HDR-1X17'"),
                     "parts[0] (LED-FP): unknown connectorFootprint \"HDR-1X17\"");
    expect_part_fail("part: connectorFootprint pad count mismatch", FP_FILE(FP_CHIP2 "," FP_SOT5),
                     FP_LED_CONN("'SOT-5'"),
                     "connectorFootprint \"SOT-5\" has 5 pads, pinout \"p\" has 2 pins");
    expect_part_fail("part: pin without a pad in connectorFootprint", FP_FILE(FP_CHIP2 "," FP_BGA2),
                     FP_LED_CONN("'BGA-2'"), "pin \"1\" has no pad in connectorFootprint \"BGA-2\"");
    expect_fail("supply part with connectorFootprint",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','connectorFootprint':'T1'}]}" TAIL,
                "parts[0] (VCC): a supply has no footprint");

    /* plan-03-C: the key is required on every part but a supply, which rejects it. */
    expect_fail("non-supply part without footprint",
                HEAD("led") "{'id':'led-nf','function':'LED','description':'d','drawer':[],"
                "'pinouts':{'p':[" FP_LED_NUMBERED "]},'colors':['Red'],'anode':'A','cathode':'K',"
                "'parts':[{'name':'LED-NF','package':'X','pinout':'p'}]}" TAIL,
                "parts[0] (LED-NF): missing \"footprint\"");
    expect_fail("supply part with footprint",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','footprint':'T1'}]}" TAIL,
                "parts[0] (VCC): a supply has no footprint");
}

/* ---- Voltage keys (plan-07-A) -------------------------------------------- */

/* A 2-input gate device; TIMING is its timing object, P1/P2 its parts' extra keys. */
#define VGATE(TIMING, P1, P2) \
    HEAD("gate") "{'id':'g','function':'NAND','description':'d','drawer':['Logic']," \
    "'pinouts':{'p4':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'}," \
    "{'number':3,'name':'Y','role':'output'},{'number':4,'name':'VCC','role':'power'}]}," \
    "'gates':[{'op':'nand','inputs':['A','B'],'output':'Y'}],'timing':" TIMING "," \
    "'parts':[{'name':'G1','package':'DIP','pinout':'p4','footprint':'T4','family':'HC'" P1 "}," \
    "{'name':'G2','package':'DIP','pinout':'p4','footprint':'T4','family':'HC'" P2 "}]}" TAIL

/* SRAM_DEVICE with the given grade object and part extra keys. */
#define VSRAM(GRADE, P1) \
    HEAD("sram") "{'id':'m','function':'1 KB','description':'d','drawer':['Memory','SRAM']," \
    "'pinouts':{'p6':[{'number':1,'name':'A0','role':'input','group':'address'}," \
    "{'number':2,'name':'DQ0','role':'inout'},{'number':3,'name':'CE#','role':'input'}," \
    "{'number':4,'name':'OE#','role':'input'},{'number':5,'name':'WE#','role':'input'}," \
    "{'number':6,'name':'VCC','role':'power'}]}," \
    "'address':['A0'],'data':['DQ0'],'chipEnable':['CE#'],'chipEnableHigh':[]," \
    "'outputEnable':'OE#','writeEnable':'WE#','byteEnable':[]," \
    "'grades':{'55':" GRADE "}," \
    "'parts':[{'name':'M1','package':'DIP','pinout':'p6','footprint':'T6','grade':'55'" P1 "}]}" TAIL

#define VSWITCH(RATED) \
    HEAD("switch") "{'id':'s','function':'GDS','description':'d','drawer':['Switch','DIP']," \
    "'pinouts':{'p2':[{'number':1,'name':'1A','role':'inout'},{'number':2,'name':'1B','role':'inout'}]}," \
    "'actuator':'Low profile','contacts':[['1A','1B']]" RATED "," \
    "'parts':[{'name':'S1','package':'DIP','pinout':'p2','footprint':'T2'}]}" TAIL

#define VDISPLAY(VF) \
    HEAD("7seg") "{'id':'LTS-X','function':'7-Segment','description':'d','drawer':['Indicators','LED 7-Seg']," \
    "'pinouts':{'p4':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'CC','role':'input'}," \
    "{'number':3,'name':'L','role':'input'},{'number':4,'name':'NP','role':'absent'}]}," \
    "'digits':1,'commonAnode':false,'colonAfter':0,'dotAfter':0,'color':'Red','size':'0.3 in'," \
    "'chip':'c','leds':[{'digit':1,'segment':'A'},{'digit':0,'segment':'L1'}]," \
    "'segmentPins':{'A':{'digit':0,'segments':['A','L1']}}," \
    "'commonPins':{'CC':1,'L':'extras'},'segmentSide':['A'],'commonSide':['CC','L']" VF "," \
    "'parts':[{'name':'LTS-X','package':'DIP','pinout':'p4','footprint':'T4'}]}" TAIL

static int voltage_is(const PartVoltage *v, uint32_t min, uint32_t max, uint32_t input) {
    return v != NULL && v->min_mv == min && v->max_mv == max && v->input_max_mv == input;
}

/* Loads text into a fresh library and expects success; returns the library. */
static PartLibrary *expect_load(const char *name, const char *text) {
    PartLibrary *library = part_library_create();
    char *error = NULL;

    if (load(library, text, &error) != 0) {
        printf("FAIL %s: %s\n", name, error != NULL ? error : "(none)");
        failures++;
    } else {
        printf("ok   %s\n", name);
    }
    free(error);
    return library;
}

static void check_voltage_cases(void) {
    PartLibrary *library;
    char *error = NULL;
    const Part *g1;
    const Part *g2;
    const Part *part;

    library = expect_load("entry vcc inherited by two parts",
                          VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,6]}}", "", ""));
    g1 = part_library_find(library, "G1");
    g2 = part_library_find(library, "G2");
    CHECK(g1 != NULL && g2 != NULL && voltage_is(g1->vcc, 2000, 6000, 6000) && g1->vcc == g2->vcc);
    part_library_destroy(library);

    library = expect_load(
        "part vcc override keeps the entry inputMax",
        VGATE("{'HC':{'rise':9,'fall':9,'vcc':[1.65,3.6],'inputMax':5.5}}", ",'vcc':[2,3.6]", ""));
    g1 = part_library_find(library, "G1");
    g2 = part_library_find(library, "G2");
    CHECK(g1 != NULL && g2 != NULL && voltage_is(g1->vcc, 2000, 3600, 5500) &&
          voltage_is(g2->vcc, 1650, 3600, 5500) && g1->vcc != g2->vcc);
    part_library_destroy(library);

    library = expect_load("part vcc override with no inputMax anywhere",
                          VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,6]}}", ",'vcc':[4.5,5.5]", ""));
    g1 = part_library_find(library, "G1");
    CHECK(g1 != NULL && voltage_is(g1->vcc, 4500, 5500, 5500));
    part_library_destroy(library);

    library = expect_load(
        "split entry vcc next to a plain entry's",
        HEAD("gate") "{'id':'x','function':'XOR','description':'d','drawer':['Logic'],"
        "'pinouts':{'p3':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'},"
        "{'number':3,'name':'Y','role':'output'},{'number':4,'name':'VCC','role':'power'}]},"
        "'gates':[{'op':'xor','inputs':['A','B'],'output':'Y'}],"
        "'timing':{'LS':{'otherLow':{'rise':1,'fall':1},'otherHigh':{'rise':2,'fall':2},"
        "'vcc':[4.75,5.25]},'HC':{'rise':9,'fall':9,'vcc':[2,6]}},"
        "'parts':[{'name':'X1','package':'DIP','pinout':'p3','footprint':'T4','family':'LS'},"
        "{'name':'X2','package':'DIP','pinout':'p3','footprint':'T4','family':'HC'}]}" TAIL);
    part = part_library_find(library, "X1");
    CHECK(part != NULL && voltage_is(part->vcc, 4750, 5250, 5250));
    part = part_library_find(library, "X2");
    CHECK(part != NULL && voltage_is(part->vcc, 2000, 6000, 6000));
    part_library_destroy(library);

    library = expect_load(
        "sram grade with vcc, inputMax and timings",
        VSRAM("{'trc':55,'taa':55,'twpOeLow':45,'note':'n','vcc':[2.7,5.5],'inputMax':6}", ""));
    part = part_library_find(library, "M1");
    CHECK(part != NULL && part->sram != NULL && part->sram->access_ps == 55000 &&
          voltage_is(part->vcc, 2700, 5500, 6000));
    part_library_destroy(library);

    library = expect_load(
        "dff entry vcc and a part inputMax",
        HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':['Logic']," FF_PINS
        "'units':[{'d':'D','clk':'CLK','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
        "'clearActiveHigh':false,'clockEdge':'rising',"
        "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false,'vcc':[2,6]}},"
        "'parts':[{'name':'D1','package':'DIP','pinout':'p6','footprint':'T6','family':'HC'},"
        "{'name':'D2','package':'DIP','pinout':'p6','footprint':'T6','family':'HC','inputMax':7}]}" TAIL);
    CHECK(voltage_is(part_library_find(library, "D1")->vcc, 2000, 6000, 6000));
    CHECK(voltage_is(part_library_find(library, "D2")->vcc, 2000, 6000, 7000));
    CHECK(load(library,
               HEAD("jkff") "{'id':'j','function':'JK flip-flop','description':'d','drawer':[]," FF_PINS
               "'units':[{'j':'D','k':'D','clk':'CLK'}],"
               "'kInverted':false,'presetActiveHigh':false,'clearActiveHigh':false,'clockEdge':'falling',"
               "'timing':{'LS':{'rise':20,'fall':18,'vcc':[4.75,5.25]}},"
               "'parts':[{'name':'J1','package':'DIP','pinout':'p6','footprint':'T6','family':'LS'}]}" TAIL,
               &error) == 0);
    CHECK(load(library,
               HEAD("oscillator") "{'id':'o','function':'f','description':'d','drawer':[],"
               "'pinouts':{'p':[{'number':1,'name':'OUT','role':'output'}]},"
               "'family':'X','minHz':10,'maxHz':20,'outputs':['OUT'],'vcc':[2.37,3.5],"
               "'parts':[{'name':'O1','package':'','pinout':'p','footprint':'T1','defaultHz':15},"
               "{'name':'O2','package':'','pinout':'p','footprint':'T1','defaultHz':15,'vcc':[1.7,1.9]}]}" TAIL,
               &error) == 0);
    CHECK(error == NULL);
    CHECK(voltage_is(part_library_find(library, "J1")->vcc, 4750, 5250, 5250));
    CHECK(voltage_is(part_library_find(library, "O1")->vcc, 2370, 3500, 3500));
    CHECK(voltage_is(part_library_find(library, "O2")->vcc, 1700, 1900, 1900));
    printf("ok   jkff entry vcc; oscillator device vcc and part override\n");
    free(error);
    error = NULL;
    part_library_destroy(library);

    library = expect_load("7seg vf", VDISPLAY(",'vf':[2,2.6]"));
    CHECK(load(library, VSWITCH(",'ratedVolts':24"), &error) == 0 && error == NULL);
    part = part_library_find(library, "LTS-X");
    CHECK(part != NULL && part->display->vf_min_mv == 2000 && part->display->vf_max_mv == 2600 &&
          part->vcc == NULL);
    part = part_library_find(library, "S1");
    CHECK(part != NULL && part->dip_switch->rated_mv == 24000 && part->vcc == NULL);
    printf("ok   switch ratedVolts\n");
    free(error);
    error = NULL;
    part_library_destroy(library);

    expect_fail("vcc not an array of 2", VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,4,6]}}", "", ""),
                "devices[0] (g): timing.HC: \"vcc\" must be [min, max] in volts");
    expect_fail("vcc a number", VGATE("{'HC':{'rise':9,'fall':9,'vcc':5}}", "", ""),
                "\"vcc\" must be [min, max] in volts");
    expect_fail("vcc min >= max", VGATE("{'HC':{'rise':9,'fall':9,'vcc':[6,6]}}", "", ""),
                "timing.HC: \"vcc\": min must be below max");
    expect_fail("vcc max > 30", VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,31]}}", "", ""),
                "\"vcc\": each value must be from 0 to 30");
    expect_fail("vcc min 0", VGATE("{'HC':{'rise':9,'fall':9,'vcc':[0,5]}}", "", ""),
                "\"vcc\": min must be above 0");
    expect_fail("inputMax below the entry max",
                VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,3.6],'inputMax':3.3}}", "", ""),
                "timing.HC: \"inputMax\" 3.3 is below the \"vcc\" max 3.6");
    expect_fail("inputMax below the part max",
                VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,3.6],'inputMax':5}}", "", ",'vcc':[2,5.5]"),
                "parts[1] (G2): \"inputMax\" 5 is below the \"vcc\" max 5.5");
    expect_fail("part inputMax without vcc", VGATE("{'HC':{'rise':9,'fall':9}}", ",'inputMax':5.5", ""),
                "parts[0] (G1): \"inputMax\" without \"vcc\"");
    expect_fail("entry inputMax without vcc",
                VGATE("{'HC':{'rise':9,'fall':9,'inputMax':5.5}}", "", ""),
                "timing.HC: \"inputMax\" without \"vcc\"");
    expect_fail("vcc inside otherLow",
                HEAD("gate") "{'id':'x','function':'XOR','description':'d','drawer':['Logic'],"
                "'pinouts':{'p3':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'},"
                "{'number':3,'name':'Y','role':'output'},{'number':4,'name':'VCC','role':'power'}]},"
                "'gates':[{'op':'xor','inputs':['A','B'],'output':'Y'}],"
                "'timing':{'LS':{'otherLow':{'rise':1,'fall':1,'vcc':[2,6]},'otherHigh':{'rise':2,'fall':2}}},"
                "'parts':[]}" TAIL,
                "otherLow: unknown key \"vcc\"");
    expect_fail("unknown key next to vcc in a grade",
                VSRAM("{'trc':55,'taa':55,'vcc':[2.7,5.5],'vccMax':6}", ""),
                "unknown key \"vccMax\"");
    expect_fail("vf min > max", VDISPLAY(",'vf':[2.6,2]"), "\"vf\": min must be at most max");
    expect_fail("ratedVolts 0", VSWITCH(",'ratedVolts':0"), "\"ratedVolts\" must be above 0");

    /* The keys are required (plan-07-F). */
    expect_fail("gate part with no vcc", VGATE("{'HC':{'rise':9,'fall':9}}", "", ""),
                "devices[0] (g): parts[0] (G1): missing \"vcc\"");
    expect_fail("dff part with no vcc",
                HEAD("dff") "{'id':'d','function':'D flip-flop','description':'d','drawer':[]," FF_PINS
                "'units':[{'d':'D','clk':'CLK','q':'Q','qn':'QN'}],'presetActiveHigh':false,"
                "'clearActiveHigh':false,'clockEdge':'rising',"
                "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false}},"
                "'parts':[{'name':'D1','package':'DIP','pinout':'p6','footprint':'T6','family':'HC'}]}" TAIL,
                "parts[0] (D1): missing \"vcc\"");
    expect_fail("sram part with no vcc", VSRAM("{'trc':55,'taa':55}", ""),
                "parts[0] (M1): missing \"vcc\"");
    expect_fail("oscillator part with no vcc",
                HEAD("oscillator") "{'id':'o','function':'f','description':'d','drawer':[],"
                "'pinouts':{'p':[{'number':1,'name':'OUT','role':'output'}]},"
                "'family':'X','minHz':10,'maxHz':20,'outputs':['OUT'],"
                "'parts':[{'name':'O1','package':'','pinout':'p','footprint':'T1','defaultHz':15}]}" TAIL,
                "parts[0] (O1): missing \"vcc\"");
    expect_fail("7seg with no vf", VDISPLAY(""), "devices[0] (LTS-X): missing \"vf\"");
    expect_fail("switch with no ratedVolts", VSWITCH(""), "devices[0] (s): missing \"ratedVolts\"");
}

int main(void) {
    check_footprint_styles();
    check_explicit_repeated_pins();
    check_footprint_corners();
    check_footprint_cases();
    check_part_footprints();
    check_ball_cases();
    check_display_cases();
    check_sram_cases();
    check_eeprom_cases();
    check_cpu_cases();
    check_z80_cases();
    check_switch_cases();
    check_flipflop_cases();
    check_gate_cases();
    check_valid_supply();
    check_valid_pull();
    check_serial();
    check_pio();
    check_valid_led();
    check_footprint_choices();
    check_duplicate_across_calls();
    check_logic_family();
    check_decoder_cases();
    check_mux_cases();
    check_supply_cases();
    check_voltage_cases();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
