/*
 * library_check: load_parts accepts well-formed part files and rejects every
 * malformed case with a path-style message, adding nothing on failure
 * (plan-01-B section 3.7; C, D, E add their kinds' cases).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "library_internal.h"
#include "parts.h"

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

/* Loads text into a fresh library and expects failure with `expect` in the message. */
static void expect_fail(const char *name, const char *text, const char *expect) {
    PartLibrary *library = part_library_create();
    char *source = json(text);
    char *error = NULL;
    int result = load_parts(library, source, "test.json", &error);

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
    int result = load_parts(library, source, "test.json", error);

    free(source);
    return result;
}

#define HEAD(KIND) "{'format':'schematical-parts','version':1,'kind':'" KIND "','devices':["
#define TAIL "]}"

#define VCC_DEVICE \
    "{'id':'VCC','function':'VCC','description':'d','drawer':[]," \
    "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1," \
    "'parts':[{'name':'VCC','package':'','pinout':'pin'}]}"
#define GND_DEVICE \
    "{'id':'GND','function':'GND','description':'d','drawer':[],'note':'n'," \
    "'pinouts':{'pin':[{'number':1,'name':'GND','role':'ground','group':'default'}]},'level':0," \
    "'parts':[{'name':'GND','package':'','pinout':'pin','flags':['obsolete','military']}]}"

static void check_valid_supply(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *vcc;
    const Part *gnd;

    CHECK(load(library, HEAD("supply") VCC_DEVICE "," GND_DEVICE TAIL, &error) == 0);
    CHECK(error == NULL);
    CHECK(part_library_count(library) == 2);
    vcc = part_library_find(library, "VCC");
    gnd = part_library_find(library, "GND");
    CHECK(vcc != NULL && gnd != NULL);
    CHECK(part_library_find(library, "NOPE") == NULL);
    if (vcc != NULL && gnd != NULL) {
        CHECK(vcc->kind == PART_SUPPLY && vcc->supply != NULL && vcc->supply->level == 1);
        CHECK(gnd->supply->level == 0);
        CHECK(strcmp(vcc->function, "VCC") == 0 && vcc->drawer_count == 0);
        CHECK(vcc->pin_count == 1 && vcc->pins[0].number == 1 && vcc->pins[0].role == PIN_POWER);
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
    "'color':'Red','anode':'A','cathode':'K'," \
    "'parts':[{'name':'LED-RED-0603','package':'0603','pinout':'two-pin'}," \
    "{'name':'LED-RED-DIP','package':'DIP','pinout':'dip4'}]}"

static void check_valid_led(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *dip;

    CHECK(load(library, HEAD("led") LED_DEVICE("A") TAIL, &error) == 0);
    dip = part_library_find(library, "LED-RED-DIP");
    CHECK(dip != NULL && dip->pin_count == 4 && strcmp(dip->led->color, "Red") == 0);
    CHECK(dip != NULL && dip->drawer_count == 2 && strcmp(dip->drawer[1], "LED") == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   valid LED file with two pinouts\n");
}

static void check_duplicate_across_calls(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;

    CHECK(load(library, HEAD("supply") VCC_DEVICE TAIL, &error) == 0);
    CHECK(part_library_count(library) == 1);
    /* The second file repeats VCC (another device id), after a good GND. */
    CHECK(load(library,
               HEAD("supply") GND_DEVICE ","
               "{'id':'VCC2','function':'VCC','description':'d','drawer':[],"
               "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
               "'parts':[{'name':'VCC','package':'','pinout':'pin'}]}" TAIL,
               &error) == -1);
    CHECK(error != NULL && strstr(error, "duplicate part name \"VCC\"") != NULL);
    printf("ok   duplicate across calls: %s\n", error != NULL ? error : "(none)");
    CHECK(part_library_count(library) == 1);
    CHECK(part_library_find(library, "VCC") != NULL);
    CHECK(part_library_find(library, "GND") == NULL);
    free(error);
    error = NULL;
    /* The library still takes a good file afterwards. */
    CHECK(load(library, HEAD("supply") GND_DEVICE TAIL, &error) == 0);
    CHECK(part_library_count(library) == 2);
    free(error);
    part_library_destroy(library);
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
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin'}]}" TAIL,
                "devices[0]: unknown key \"colour\"");
    expect_fail("unknown part key",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','family':'HC'}]}" TAIL,
                "devices[0] (VCC): parts[0] (VCC): unknown key \"family\"");
    expect_fail("pin numbers 1,3",
                HEAD("led")
                "{'id':'led-red','function':'LED','description':'d','drawer':[],"
                "'pinouts':{'two-pin':[{'number':1,'name':'K','role':'input'},{'number':3,'name':'A','role':'input'}]},"
                "'color':'Red','anode':'A','cathode':'K','parts':[]}" TAIL,
                "pinouts.two-pin: pin 2: pin numbers must be 1..N in order (found 3)");
    expect_fail("unknown role",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'hidden'}]},'level':1,"
                "'parts':[]}" TAIL,
                "unknown role \"hidden\"");
    expect_fail("part names a missing pinout",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
                "'parts':[{'name':'VCC','package':'','pinout':'dip8'}]}" TAIL,
                "parts[0] (VCC): no pinout \"dip8\"");
    expect_fail("LED anode missing from the second pinout", HEAD("led") LED_DEVICE("AA") TAIL,
                "devices[0] (led-red): pinouts.dip4: no pin named \"A\"");
    expect_fail("duplicate part in one file",
                HEAD("supply") VCC_DEVICE ","
                "{'id':'VCC2','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin'}]}" TAIL,
                "duplicate part name \"VCC\"");
    expect_fail("duplicate device id", HEAD("supply") VCC_DEVICE "," VCC_DEVICE TAIL,
                "duplicate device id \"VCC\"");
    expect_fail("bad flag",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
                "'parts':[{'name':'VCC','package':'','pinout':'pin','flags':['vintage']}]}" TAIL,
                "unknown flag \"vintage\"");
    expect_fail("drawer too deep",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':['a','b','c'],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':1,"
                "'parts':[]}" TAIL,
                "\"drawer\" has at most 2 levels");
    expect_fail("supply level 2",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'level':2,"
                "'parts':[]}" TAIL,
                "\"level\" must be a whole number from 0 to 1");
    expect_fail("oscillator defaultHz out of range",
                HEAD("oscillator")
                "{'id':'o','function':'f','description':'d','drawer':[],"
                "'pinouts':{'p':[{'number':1,'name':'OUT','role':'output'}]},"
                "'family':'X','minHz':10,'maxHz':20,'outputs':['OUT'],"
                "'parts':[{'name':'O1','package':'','pinout':'p','defaultHz':30}]}" TAIL,
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
    "'parts':[{'name':'G1','package':'DIP','pinout':'p4','family':'" FAMILY "'}," \
    "{'name':'G2','package':'LCCC','pinout':'p5','family':'" FAMILY "'}]}"

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
               "'timing':{'HC':{'rise':20,'fall':18,'clearWins':false}},"
               "'parts':[{'name':'D1','package':'DIP','pinout':'p6','family':'HC'}]}" TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_DFF && part->logic->units == 1);
    printf("ok   valid dff file%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

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
    "'grades':{'55':{'trc':55,'taa':55,'twpOeLow':45,'note':'n'}}," \
    "'parts':[{'name':'M1','package':'DIP','pinout':'p6','grade':'" GRADE "'}]}" TAIL

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

int main(void) {
    check_sram_cases();
    check_switch_cases();
    check_flipflop_cases();
    check_gate_cases();
    check_valid_supply();
    check_valid_led();
    check_duplicate_across_calls();
    check_logic_family();
    check_supply_cases();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
