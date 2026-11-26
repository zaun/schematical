/*
 * library_logic_check: split from library_check.c by plan-68-H; the shared helpers are
 * support/library_check_bench.c.
 */

#include "library_check_bench.h"

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

/* plan-22-B: arith devices on the decoder's 26-pin fixture pinout. */
#define ARI_CMP(REST) \
    "'model':'compare','a':['A','B'],'b':['C','D'],'outputs':{'gt':{'pin':'Y0','active':'high'}," \
    "'eq':{'pin':'Y1','active':'low'}}" REST
#define ARI_CAS ",'cascade':{'gt':'E','eq':'G'}"
#define ARI_PAR(REST) "'model':'parity','inputs':['A','B','C']" REST
#define ARI_BUS ",'bus':{'pin':'RBO','xmit':{'pin':'LE','active':'low'},'drives':'even'}"

static void check_arith_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library,
               HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'in':'HL','out':'HL'},"
                                       "{'in':'XH','out':'LL'},{'in':'LL','out':'HH'},"
                                       "{'when':'gt','in':'XX','out':'HH'}],"
                                       "'enable':{'pin':'LE','active':'low'},"
                                       "'latch':{'pin':'CLK','active':'low'},"
                                       "'signed':{'pin':'RBI','active':'high'},'openDrain':['Y1']")) TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_ARITH && part->logic != NULL &&
          strcmp(part->logic->family, "HC") == 0 && part->logic->units == 1 &&
          part_pin_open_drain(part, 9) == 1 && part_pin_open_drain(part, 8) == 0);
    printf("ok   valid arith (compare)%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    error = NULL;
    part_library_destroy(library);
    library = part_library_create();
    CHECK(load(library,
               HEAD("arith") DEC_DEVICE(ARI_PAR(",'even':'Y0','odd':'Y1'" ARI_BUS
                                               ",'error':{'pin':'Y2','active':'low'},'errorOn':'even'")) TAIL,
               &error) == 0);
    part = part_library_find(library, "D1");
    CHECK(part != NULL && part->kind == PART_ARITH && part->logic->units == 1);
    printf("ok   valid arith (parity)%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("arith unknown model",
                HEAD("arith") DEC_DEVICE("'model':'divider'") TAIL,
                "devices[0] (d): unknown model \"divider\"");
    expect_fail("arith key of another kind",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'units':[]")) TAIL,
                "devices[0]: unknown key \"units\"");
    expect_fail("arith key of another model",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'errorOn':'even'")) TAIL,
                "\"errorOn\" is not a key of model \"compare\"");
    expect_fail("arith compare a and b lengths",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A','B'],'b':['C'],"
                                         "'outputs':{'eq':{'pin':'Y0','active':'high'}}") TAIL,
                "\"a\" and \"b\" must have the same number of names");
    expect_fail("arith compare 13 bits",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A','A','A','A','A','A','A','A','A','A','A','A','A'],"
                                         "'b':['C'],'outputs':{'eq':{'pin':'Y0','active':'high'}}") TAIL,
                "\"a\" must have 1 to 12 names");
    expect_fail("arith compare no outputs",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A'],'b':['C']") TAIL,
                "missing \"outputs\"");
    expect_fail("arith compare outputs not object",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A'],'b':['C'],'outputs':['Y0']") TAIL,
                "outputs: must be an object of \"gt\", \"eq\", \"lt\"");
    expect_fail("arith compare unknown output",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A'],'b':['C'],"
                                         "'outputs':{'ne':{'pin':'Y0','active':'high'}}") TAIL,
                "outputs: unknown key \"ne\"");
    expect_fail("arith compare empty outputs",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A'],'b':['C'],'outputs':{}") TAIL,
                "\"outputs\" needs at least one of \"gt\", \"eq\", \"lt\"");
    expect_fail("arith compare output without active",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['A'],'b':['C'],"
                                         "'outputs':{'eq':{'pin':'Y0'}}") TAIL,
                "missing \"active\"");
    expect_fail("arith compare cascade not object",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'cascade':['E']")) TAIL,
                "cascade: must be an object of \"gt\", \"eq\", \"lt\"");
    expect_fail("arith compare cascade unknown key",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'cascade':{'ge':'E'}")) TAIL,
                "cascade: unknown key \"ge\"");
    expect_fail("arith compare empty cascade",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'cascade':{}")) TAIL,
                "\"cascade\" needs at least one of \"gt\", \"eq\", \"lt\"");
    expect_fail("arith compare cascade output without input",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'cascade':{'gt':'E'}")) TAIL,
                "output \"eq\" has no cascade input of its name: give \"cascadeTable\"");
    expect_fail("arith compare table without cascade",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'cascadeTable':[{'in':'','out':'HL'}]")) TAIL,
                "\"cascadeTable\" needs \"cascade\"");
    expect_fail("arith compare table not array",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':{}")) TAIL,
                "\"cascadeTable\" must be an array of 1 to 24 rows");
    expect_fail("arith compare table row key",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'in':'XX','out':'HL','if':1}]")) TAIL,
                "cascadeTable[0]: unknown key \"if\"");
    expect_fail("arith compare table in length",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'in':'XXX','out':'HL'}]")) TAIL,
                "cascadeTable[0]: \"in\" must have 2 levels");
    expect_fail("arith compare table out level",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'in':'XX','out':'HX'}]")) TAIL,
                "cascadeTable[0]: \"out\" levels must be H, L");
    expect_fail("arith compare table in level",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'in':'X1','out':'HL'}]")) TAIL,
                "cascadeTable[0]: \"in\" levels must be H, L or X");
    expect_fail("arith compare table when",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'when':'ne','in':'XX','out':'HL'}]")) TAIL,
                "cascadeTable[0]: \"when\" must be \"gt\", \"eq\" or \"lt\", not \"ne\"");
    expect_fail("arith compare table gap",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':[{'in':'HX','out':'HL'}]")) TAIL,
                "\"cascadeTable\" rows for \"eq\" miss a cascade level combination");
    expect_fail("arith compare table 25 rows",
                HEAD("arith") DEC_DEVICE(ARI_CMP(ARI_CAS ",'cascadeTable':["
                    "{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},"
                    "{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},"
                    "{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},"
                    "{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},"
                    "{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},"
                    "{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},{'in':'XX','out':'HL'},"
                    "{'in':'XX','out':'HL'}]")) TAIL,
                "\"cascadeTable\" must be an array of 1 to 24 rows");
    expect_fail("arith compare latch without active",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'latch':{'pin':'CLK'}")) TAIL,
                "latch: missing \"active\"");
    expect_fail("arith compare unknown pin",
                HEAD("arith") DEC_DEVICE("'model':'compare','a':['Q'],'b':['C'],"
                                         "'outputs':{'eq':{'pin':'Y0','active':'high'}}") TAIL,
                "pinouts.p26: no pin named \"Q\"");
    expect_fail("arith output style",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'output':'open-collector'")) TAIL,
                "\"output\" must be \"push-pull\" or \"open-drain\"");
    expect_fail("arith openDrain not an output",
                HEAD("arith") DEC_DEVICE(ARI_CMP(",'openDrain':['Y5']")) TAIL,
                "\"openDrain\": \"Y5\" is not an output of the model");
    expect_fail("arith parity no inputs",
                HEAD("arith") DEC_DEVICE("'model':'parity','even':'Y0'") TAIL,
                "missing \"inputs\"");
    expect_fail("arith parity 10 inputs",
                HEAD("arith") DEC_DEVICE("'model':'parity','inputs':['A','B','C','D','E','G','LE','CLK','RBI','A'],"
                                         "'even':'Y0'") TAIL,
                "\"inputs\" must have 1 to 9 names");
    expect_fail("arith parity no outputs",
                HEAD("arith") DEC_DEVICE(ARI_PAR("")) TAIL,
                "a parity device needs \"even\", \"odd\" or \"bus\"");
    expect_fail("arith parity bus not object",
                HEAD("arith") DEC_DEVICE(ARI_PAR(",'bus':'RBO'")) TAIL,
                "bus: must be an object {\"pin\", \"xmit\", \"drives\"}");
    expect_fail("arith parity bus without xmit",
                HEAD("arith") DEC_DEVICE(ARI_PAR(",'bus':{'pin':'RBO','drives':'even'}")) TAIL,
                "bus: missing \"xmit\"");
    expect_fail("arith parity bus drives",
                HEAD("arith") DEC_DEVICE(ARI_PAR(",'bus':{'pin':'RBO','xmit':{'pin':'LE','active':'low'},"
                                                 "'drives':'none'}")) TAIL,
                "bus: \"drives\" must be \"even\" or \"odd\", not \"none\"");
    expect_fail("arith parity bus not inout",
                HEAD("arith") DEC_DEVICE(ARI_PAR(",'bus':{'pin':'Y3','xmit':{'pin':'LE','active':'low'},"
                                                 "'drives':'even'}")) TAIL,
                "\"bus\": pin \"Y3\" must have role inout");
    expect_fail("arith parity error without bus",
                HEAD("arith") DEC_DEVICE(ARI_PAR(",'even':'Y0','error':{'pin':'Y2','active':'low'},"
                                                 "'errorOn':'even'")) TAIL,
                "\"error\" needs \"bus\"");
    expect_fail("arith parity errorOn",
                HEAD("arith") DEC_DEVICE(ARI_PAR(ARI_BUS ",'error':{'pin':'Y2','active':'low'},"
                                                 "'errorOn':'both'")) TAIL,
                "\"errorOn\" must be \"even\" or \"odd\", not \"both\"");
    expect_fail("arith parity errorOn without error",
                HEAD("arith") DEC_DEVICE(ARI_PAR(ARI_BUS ",'errorOn':'even'")) TAIL,
                "\"errorOn\" needs \"error\"");
}

/* plan-22-C: models adder, alu181, wallace. */
#define ARI_ADD(REST) "'model':'adder','a':['A','B'],'b':['C','D'],'sum':['Y0','Y1'],'cin':'E','cout':'Y2'" REST
#define ARI_ALU(REST) \
    "'model':'alu181','a':['A','B','C','D'],'b':['E','G','LE','CLK'],'s':['A','B','C','D'],'m':'RBI'," \
    "'cn':'E','f':['Y0','Y1','Y2','Y3'],'cn4':'Y4','p':'Y5','g':'Y6'" REST
#define ARI_FA1 "{'a':'A','b':'B','c':'C','sum':'s1','carry':'c1'}"
#define ARI_WAL(ADDERS, REST) \
    "'model':'wallace','adders':[" ADDERS "],'outputs':[{'pin':'Y0','node':'s1'},{'pin':'Y1','node':'c1'}]" REST

static void check_arith_alu_cases(void) {
    static const char *const VALID[][2] = {
        {"adder", HEAD("arith") DEC_DEVICE(ARI_ADD("")) TAIL},
        {"alu181", HEAD("arith") DEC_DEVICE(ARI_ALU(",'aeqb':'Y7','openDrain':['Y7']")) TAIL},
        {"wallace", HEAD("arith") DEC_DEVICE(ARI_WAL(ARI_FA1 ",{'a':'s1','b':'D','c':'E','sum':'s2','carry':'c2'}",
                                                   ",'enable':{'pin':'G','active':'low'}")) TAIL},
    };
    size_t i;

    for (i = 0; i < sizeof VALID / sizeof VALID[0]; i++) {
        PartLibrary *library = part_library_create();
        char *error = NULL;
        const Part *part;

        CHECK(load(library, VALID[i][1], &error) == 0);
        part = part_library_find(library, "D1");
        CHECK(part != NULL && part->kind == PART_ARITH && part->logic->units == 1);
        printf("ok   valid arith (%s)%s%s\n", VALID[i][0], error != NULL ? ": " : "",
               error != NULL ? error : "");
        free(error);
        part_library_destroy(library);
    }
    expect_fail("arith adder key of another model",
                HEAD("arith") DEC_DEVICE(ARI_ADD(",'cascade':{'gt':'E'}")) TAIL,
                "\"cascade\" is not a key of model \"adder\"");
    expect_fail("arith adder lengths",
                HEAD("arith") DEC_DEVICE("'model':'adder','a':['A','B'],'b':['C','D'],'sum':['Y0']") TAIL,
                "\"a\", \"b\" and \"sum\" must have the same number of names");
    expect_fail("arith adder 9 bits",
                HEAD("arith") DEC_DEVICE("'model':'adder','a':['A','A','A','A','A','A','A','A','A'],"
                                         "'b':['C'],'sum':['Y0']") TAIL,
                "\"a\" must have 1 to 8 names");
    expect_fail("arith adder unknown pin",
                HEAD("arith") DEC_DEVICE("'model':'adder','a':['A'],'b':['C'],'sum':['Y0'],'cout':'Q'") TAIL,
                "no pin named \"Q\"");
    expect_fail("arith alu181 s count",
                HEAD("arith") DEC_DEVICE("'model':'alu181','a':['A','B','C','D'],'b':['E','G','LE','CLK'],"
                                         "'s':['A','B','C'],'m':'RBI','cn':'E','f':['Y0','Y1','Y2','Y3'],"
                                         "'cn4':'Y4','p':'Y5','g':'Y6','aeqb':'Y7'") TAIL,
                "\"s\" must have 4 names");
    expect_fail("arith alu181 missing aeqb",
                HEAD("arith") DEC_DEVICE(ARI_ALU("")) TAIL,
                "missing \"aeqb\"");
    expect_fail("arith wallace no adders",
                HEAD("arith") DEC_DEVICE("'model':'wallace','outputs':[{'pin':'Y0','node':'s1'}]") TAIL,
                "missing \"adders\"");
    expect_fail("arith wallace 9 adders",
                HEAD("arith") DEC_DEVICE(ARI_WAL(ARI_FA1 "," ARI_FA1 "," ARI_FA1 "," ARI_FA1 "," ARI_FA1 ","
                                                 ARI_FA1 "," ARI_FA1 "," ARI_FA1 "," ARI_FA1, "")) TAIL,
                "\"adders\" must be an array of 1 to 8 full adders");
    expect_fail("arith wallace adder key",
                HEAD("arith") DEC_DEVICE(ARI_WAL("{'a':'A','b':'B','c':'C','d':'D','sum':'s1','carry':'c1'}", "")) TAIL,
                "adders[0]: unknown key \"d\"");
    expect_fail("arith wallace node reused",
                HEAD("arith") DEC_DEVICE(ARI_WAL(ARI_FA1 "," ARI_FA1, "")) TAIL,
                "adders[1]: \"sum\" and \"carry\" must be new node names");
    expect_fail("arith wallace outputs not array",
                HEAD("arith") DEC_DEVICE("'model':'wallace','adders':[" ARI_FA1 "],'outputs':{}") TAIL,
                "\"outputs\" must be an array of 1 to 8 {\"pin\", \"node\"}");
    expect_fail("arith wallace output node",
                HEAD("arith") DEC_DEVICE("'model':'wallace','adders':[" ARI_FA1 "],"
                                         "'outputs':[{'pin':'Y0','node':'q'}]") TAIL,
                "outputs[0]: \"node\" \"q\" is not an adder's sum or carry");
    expect_fail("arith wallace input pin",
                HEAD("arith") DEC_DEVICE(ARI_WAL("{'a':'Q','b':'B','c':'C','sum':'s1','carry':'c1'}", "")) TAIL,
                "no pin named \"Q\"");
    expect_fail("arith wallace enable without active",
                HEAD("arith") DEC_DEVICE(ARI_WAL(ARI_FA1, ",'enable':{'pin':'G'}")) TAIL,
                "enable: missing \"active\"");
}

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

int main(void) {
    check_footprint_choices();
    check_duplicate_across_calls();
    check_logic_family();
    check_decoder_cases();
    check_mux_cases();
    check_arith_cases();
    check_arith_alu_cases();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
