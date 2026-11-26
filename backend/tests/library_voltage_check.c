/*
 * library_voltage_check: split from library_check.c by plan-68-H; the shared helpers are
 * support/library_check_bench.c.
 */

#include "library_check_bench.h"

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

/* A four-terminal tactile switch (plan-27-D) with the given extra device keys. */
#define VTACT(KEYS) \
    HEAD("switch") "{'id':'t','function':'Normal Close','description':'d','drawer':['Switch','Tactile']," \
    "'pinouts':{'p4':[{'number':1,'name':'A1','role':'inout'},{'number':2,'name':'A2','role':'inout'}," \
    "{'number':3,'name':'B1','role':'inout'},{'number':4,'name':'B2','role':'inout'}]}," \
    "'actuator':'Standard','contacts':[['A1','B1']],'ratedVolts':12" KEYS "," \
    "'parts':[{'name':'T1','package':'THT','pinout':'p4','footprint':'T4'}]}" TAIL
/* The same with a pin name used twice. */
#define VTACT_DUP(KEYS) \
    HEAD("switch") "{'id':'t','function':'Normal Open','description':'d','drawer':['Switch','Tactile']," \
    "'pinouts':{'p4':[{'number':1,'name':'A1','role':'inout'},{'number':2,'name':'A','role':'inout'}," \
    "{'number':3,'name':'B1','role':'inout'},{'number':4,'name':'A','role':'inout'}]}," \
    "'actuator':'Standard','contacts':[['A1','B1']],'ratedVolts':12" KEYS "," \
    "'parts':[{'name':'T1','package':'THT','pinout':'p4','footprint':'T4'}]}" TAIL

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

/* A drawer path takes up to three levels (Discrete → Resistors → Chip). */
static void check_three_level_drawer(void) {
    PartLibrary *library = expect_load(
        "three-level drawer",
        HEAD("supply")
        "{'id':'VCC','function':'VCC','description':'d','drawer':['Discrete','Resistors','Chip'],"
        "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
        "'parts':[{'name':'VCC','package':'','pinout':'pin'}]}" TAIL);
    const Part *vcc = part_library_find(library, "VCC");

    CHECK(vcc != NULL && vcc->drawer_count == 3 && strcmp(vcc->drawer[0], "Discrete") == 0 &&
          strcmp(vcc->drawer[2], "Chip") == 0);
    part_library_destroy(library);
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
    expect_fail("vcc max > 100", VGATE("{'HC':{'rise':9,'fall':9,'vcc':[2,101]}}", "", ""),
                "\"vcc\": each value must be from 0 to 100");
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

/* Tactile switch keys on the switch kind (plan-27-D section 4.1). */
static void check_tactile_cases(void) {
    PartLibrary *library;
    const Part *part;

    expect_fail("momentary not a bool", VTACT(",'momentary':1"), "\"momentary\" must be true or false");
    expect_fail("normallyClosed not a bool", VTACT(",'momentary':true,'normallyClosed':'yes'"),
                "\"normallyClosed\" must be true or false");
    expect_fail("normallyClosed without momentary", VTACT(",'normallyClosed':true"),
                "\"normallyClosed\" needs \"momentary\"");
    expect_fail("joined empty", VTACT(",'momentary':true,'joined':[]"),
                "\"joined\" must be a non-empty array of [pin, pin]");
    expect_fail("joined not an array", VTACT(",'momentary':true,'joined':'A1'"),
                "\"joined\" must be a non-empty array of [pin, pin]");
    expect_fail("joined pair names one pin twice", VTACT(",'momentary':true,'joined':[['A1','A1']]"),
                "joined[0]: a joined pair joins two different pins, not \"A1\" twice");
    expect_fail("joined names a missing pin", VTACT(",'momentary':true,'joined':[['A1','A9']]"),
                "joined[0]: no pin named \"A9\"");
    expect_fail("joined names a pin used by two pins",
                VTACT_DUP(",'momentary':true,'joined':[['A1','A']]"),
                "joined[0]: pin name \"A\" is used by 2 pins; one is needed");
    expect_fail("bodySize not a pair", VTACT(",'momentary':true,'bodySize':[6]"),
                "\"bodySize\" must be [length, width] in mm, each above 0 and at most 100");
    expect_fail("bodySize zero", VTACT(",'momentary':true,'bodySize':[6,0]"),
                "\"bodySize\" must be [length, width] in mm, each above 0 and at most 100");
    expect_fail("bodySize width first", VTACT(",'momentary':true,'bodySize':[3,6]"),
                "\"bodySize\" gives the length (the larger side) first");

    library = expect_load("tactile switch keys",
                          VTACT(",'momentary':true,'normallyClosed':true,'joined':[['A1','A2'],['B1','B2']],"
                                "'bodySize':[6.2,3.5]"));
    part = part_library_find(library, "T1");
    CHECK(part != NULL && part->dip_switch->positions == 1 && part->dip_switch->momentary == 1 &&
          part->dip_switch->normally_closed == 1 && part->dip_switch->joined == 2 &&
          part->dip_switch->body_um[0] == 6200 && part->dip_switch->body_um[1] == 3500);
    printf("ok   tactile switch momentary, normallyClosed, joined, bodySize\n");
    part_library_destroy(library);

    library = expect_load("dip switch without the keys", VSWITCH(",'ratedVolts':24"));
    part = part_library_find(library, "S1");
    CHECK(part != NULL && part->dip_switch->momentary == 0 && part->dip_switch->normally_closed == 0 &&
          part->dip_switch->joined == 0 && part->dip_switch->body_um[0] == 0);
    printf("ok   dip switch has no tactile keys\n");
    part_library_destroy(library);
}

int main(void) {
    check_tactile_cases();
    check_three_level_drawer();
    check_voltage_cases();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
