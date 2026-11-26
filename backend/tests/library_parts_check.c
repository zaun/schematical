/*
 * library_parts_check: split from library_check.c by plan-68-H; the shared helpers are
 * support/library_check_bench.c.
 */

#include "library_check_bench.h"

static void check_supply_cases(void) {
    expect_fail("bad JSON", HEAD("supply") VCC_DEVICE ",", "invalid JSON at byte");
    expect_fail("wrong format",
                "{'format':'parts','version':1,'kind':'supply','devices':[]}",
                "\"format\" must be \"schematical-parts\"");
    expect_fail("version 2",
                "{'format':'schematical-parts','version':2,'kind':'supply','devices':[]}",
                "unsupported version 2");
    expect_fail("unknown kind", HEAD("no-such-kind") TAIL, "unknown kind \"no-such-kind\"");
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
                "{'id':'VCC','function':'VCC','description':'d','drawer':['a','b','c','d'],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':5,"
                "'parts':[]}" TAIL,
                "\"drawer\" has at most 3 levels");
    expect_fail("supply volts -1",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':-1,"
                "'parts':[]}" TAIL,
                "devices[0] (VCC): \"volts\" must be from 0 to 500");
    expect_fail("supply volts 501",
                HEAD("supply")
                "{'id':'VCC','function':'VCC','description':'d','drawer':[],"
                "'pinouts':{'pin':[{'number':1,'name':'VCC','role':'power'}]},'volts':501,"
                "'parts':[]}" TAIL,
                "devices[0] (VCC): \"volts\" must be from 0 to 500");
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
    /* plan-20-B: AXC, LXC, TXU, TXV (FCT: plan-19-A above). */
    {
        static const char *const FAMILIES[] = {"AXC", "LXC", "TXU", "TXV"};
        static const char *const FILES[] = {
            HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                     "{'AXC':{'rise':5,'fall':5,'vcc':[0.65,3.6]}}", "A", "AXC") TAIL,
            HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                     "{'LXC':{'rise':5,'fall':5,'vcc':[1.1,5.5]}}", "A", "LXC") TAIL,
            HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                     "{'TXU':{'rise':5,'fall':5,'vcc':[1.1,5.5]}}", "A", "TXU") TAIL,
            HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                     "{'TXV':{'rise':5,'fall':5,'vcc':[0.9,3.6]}}", "A", "TXV") TAIL,
        };
        size_t f;

        for (f = 0; f < 4; f++) {
            library = part_library_create();
            error = NULL;
            CHECK(load(library, FILES[f], &error) == 0);
            part = part_library_find(library, "G1");
            CHECK(part != NULL && part->logic != NULL &&
                  strcmp(part->logic->family, FAMILIES[f]) == 0);
            printf("ok   gate part with family %s%s%s\n", FAMILIES[f], error != NULL ? ": " : "",
                   error != NULL ? error : "");
            free(error);
            part_library_destroy(library);
        }
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
    expect_fail("gate buf with 2 inputs",
                HEAD("gate") GATE_DEVICE("[{'op':'buf','inputs':['A','B'],'output':'Y'}]",
                                         "{'HC':{'rise':9,'fall':9}}", "A", "HC") TAIL,
                "gates[0]: a \"buf\" gate has exactly 1 input");
    expect_fail("gate unknown family",
                HEAD("gate") GATE_DEVICE("[{'op':'nand','inputs':['A','B','C'],'output':'Y'}]",
                                         "{'HX':{'rise':9,'fall':9}}", "A", "HC") TAIL,
                "timing.HX: unknown family \"HX\"");
}


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

/* A USB connector device with KEYS (JSON members, each ending in a comma). */
#define USB_DEVICE(KEYS) \
    HEAD("connector") "{'id':'u','function':'USB-C Port','description':'d','drawer':['Connector','USB']," \
    KEYS "'pinouts':{'p':[{'number':1,'name':'VBUS','role':'inout'},{'number':2,'name':'D-','role':'inout'}," \
    "{'number':3,'name':'D+','role':'inout'},{'number':4,'name':'GND','role':'inout'}," \
    "{'number':5,'name':'SHIELD','role':'inout'}]}," \
    "'parts':[{'name':'USB-T','manufacturer':'M','package':'SMD right angle','pinout':'p','footprint':'T5'}]}" TAIL

static void check_usb_connector(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, USB_DEVICE("'usb':'C','ratedVolts':20,'ratedAmps':5,"), &error) == 0);
    part = part_library_find(library, "USB-T");
    CHECK(part != NULL && part->kind == PART_CONNECTOR && part->connector->usb == 'C' &&
          part->connector->rated_mv == 20000 && part->connector->rated_ma == 5000 &&
          part->connector->contacts == 0 && part->connector->center_diameter_um == 0);
    free(error);
    error = NULL;
    part_library_destroy(library);
    library = part_library_create();
    CHECK(load(library, USB_DEVICE("'usb':'A',"), &error) == 0);
    part = part_library_find(library, "USB-T");
    CHECK(part != NULL && part->connector->usb == 'A' && part->connector->rated_mv == 0 &&
          part->connector->rated_ma == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   USB connector loads; ratings optional\n");

    expect_fail("usb: bad type", USB_DEVICE("'usb':'D',"), "\"usb\" must be \"A\", \"B\" or \"C\"");
    expect_fail("usb: centre pin", USB_DEVICE("'usb':'A','centerPin':2,"), "a USB connector has no \"centerPin\"");
    expect_fail("usb: contacts", USB_DEVICE("'usb':'A','contacts':[['VBUS','GND']],"),
                "a USB connector has no \"contacts\"");
    expect_fail("usb: zero amps", USB_DEVICE("'usb':'B','ratedAmps':0,"),
                "\"ratedAmps\" must be above 0 and at most 100");
    expect_fail("barrel: still needs its centre pin", USB_DEVICE("'ratedVolts':20,'ratedAmps':5,"),
                "\"centerPin\"");
}

/* A card slot device with KEYS (JSON members, each ending in a comma). */
#define CARD_DEVICE(KEYS) \
    HEAD("connector") "{'id':'c','function':'microSD','description':'d','drawer':['Connector','Card slot','microSD']," \
    KEYS "'pinouts':{'p':[{'number':1,'name':'CLK','role':'inout'},{'number':2,'name':'CMD','role':'inout'}," \
    "{'number':3,'name':'DAT0','role':'inout'},{'number':4,'name':'VSS','role':'inout'}," \
    "{'number':5,'name':'SHIELD','role':'inout'}]}," \
    "'parts':[{'name':'CARD-T','manufacturer':'M','package':'SMD right angle','pinout':'p','footprint':'T5'}]}" TAIL

static void check_card_slot_connector(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, CARD_DEVICE("'card':'microSD',"), &error) == 0);
    part = part_library_find(library, "CARD-T");
    CHECK(part != NULL && part->kind == PART_CONNECTOR && part->connector->card != NULL &&
          strcmp(part->connector->card, "microSD") == 0 && part->connector->card_pitch_um == 0 &&
          part->connector->usb == 0 && part->connector->contacts == 0 && part->connector->rated_ma == 0);
    free(error);
    error = NULL;
    part_library_destroy(library);
    library = part_library_create();
    CHECK(load(library, CARD_DEVICE("'card':'PCI Express','cardPitch':1.0,"), &error) == 0);
    part = part_library_find(library, "CARD-T");
    CHECK(part != NULL && strcmp(part->connector->card, "PCI Express") == 0 && part->connector->card_pitch_um == 1000);
    free(error);
    part_library_destroy(library);
    printf("ok   card slot connectors load; malformed ones fail\n");

    expect_fail("card: a number", CARD_DEVICE("'card':3,"), "\"card\" must be a string of 1 to 40 characters");
    expect_fail("card: empty", CARD_DEVICE("'card':'',"), "\"card\" must be a string of 1 to 40 characters");
    expect_fail("card: 41 bytes", CARD_DEVICE("'card':'12345678901234567890123456789012345678901',"),
                "\"card\" must be a string of 1 to 40 characters");
    expect_fail("card: pitch 0", CARD_DEVICE("'card':'M.2','cardPitch':0,"),
                "\"cardPitch\" must be above 0 and at most 20 mm");
    expect_fail("card: pitch 21", CARD_DEVICE("'card':'M.2','cardPitch':21,"),
                "\"cardPitch\" must be above 0 and at most 20 mm");
    expect_fail("card: pitch without card", CARD_DEVICE("'cardPitch':1.0,"), "\"cardPitch\" is only for card slots");
    expect_fail("card: usb", CARD_DEVICE("'card':'SD','usb':'A',"), "a card slot has no \"usb\"");
    expect_fail("card: centre pin", CARD_DEVICE("'card':'SD','centerPin':2,"), "a card slot has no \"centerPin\"");
    expect_fail("card: contacts", CARD_DEVICE("'card':'SD','contacts':[['CLK','CMD']],"),
                "a card slot has no \"contacts\"");
    expect_fail("card: rated volts", CARD_DEVICE("'card':'SD','ratedVolts':5,"), "a card slot has no \"ratedVolts\"");
    expect_fail("card: rated amps", CARD_DEVICE("'card':'SD','ratedAmps':1,"), "a card slot has no \"ratedAmps\"");
}

/* A test point device (plan-57) with KEYS (JSON members, each ending in a comma) and PINS (JSON pin objects). */
#define TEST_POINT_DEVICE(KEYS, PINS) \
    HEAD("connector") "{'id':'tp','function':'Test Point','description':'d','drawer':['Connector']," \
    KEYS "'pinouts':{'p':[" PINS "]}," \
    "'parts':[{'name':'TP-T','manufacturer':'M','package':'Pad','pinout':'p','footprint':'T1'}]}" TAIL
#define TP_PIN "{'number':1,'name':'TP','role':'inout'}"

static void check_test_point_connector(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, TEST_POINT_DEVICE("'testPoint':true,", TP_PIN), &error) == 0);
    part = part_library_find(library, "TP-T");
    CHECK(part != NULL && part->kind == PART_CONNECTOR && part->pin_count == 1 && part->connector->test_point == 1 &&
          part->connector->card == NULL && part->connector->usb == 0 && part->connector->contacts == 0 &&
          part->connector->rated_ma == 0 && part->connector->center_diameter_um == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   test point connectors load; malformed ones fail\n");

    expect_fail("test point: false", TEST_POINT_DEVICE("'testPoint':false,", TP_PIN), "\"testPoint\" must be true");
    expect_fail("test point: a string", TEST_POINT_DEVICE("'testPoint':'yes',", TP_PIN), "\"testPoint\" must be true");
    expect_fail("test point: usb", TEST_POINT_DEVICE("'testPoint':true,'usb':'A',", TP_PIN),
                "a test point has no \"usb\"");
    expect_fail("test point: centre pin", TEST_POINT_DEVICE("'testPoint':true,'centerPin':2,", TP_PIN),
                "a test point has no \"centerPin\"");
    expect_fail("test point: contacts", TEST_POINT_DEVICE("'testPoint':true,'contacts':[['A','B']],", TP_PIN),
                "a test point has no \"contacts\"");
    expect_fail("test point: rated volts", TEST_POINT_DEVICE("'testPoint':true,'ratedVolts':5,", TP_PIN),
                "a test point has no \"ratedVolts\"");
    expect_fail("test point: rated amps", TEST_POINT_DEVICE("'testPoint':true,'ratedAmps':1,", TP_PIN),
                "a test point has no \"ratedAmps\"");
    expect_fail("test point: card", TEST_POINT_DEVICE("'testPoint':true,'card':'SD',", TP_PIN),
                "a test point has no \"card\"");
    expect_fail("test point: card pitch", TEST_POINT_DEVICE("'testPoint':true,'cardPitch':1.0,", TP_PIN),
                "a test point has no \"cardPitch\"");
    expect_fail("test point: two pins",
                TEST_POINT_DEVICE("'testPoint':true,", TP_PIN ",{'number':2,'name':'TP2','role':'inout'}"),
                "a test point has exactly one pin");
    expect_fail("connector: one pin without testPoint",
                TEST_POINT_DEVICE("'usb':'A',", TP_PIN), "a connector has at least two pins");
}

/* A terminal block or JST header device (plan-45) with KEYS (JSON members, each ending in a comma). */
#define PITCH_DEVICE(KEYS) \
    HEAD("connector") "{'id':'t','function':'Screw, vertical','description':'d'," \
    "'drawer':['Connector','Terminal block','5.08 mm']," \
    KEYS "'pinouts':{'p':[{'number':1,'name':'1','role':'inout'},{'number':2,'name':'2','role':'inout'}]}," \
    "'parts':[{'name':'TB-T','manufacturer':'M','package':'Through hole vertical','pinout':'p','footprint':'T2'}]}" TAIL

/* One clamp / series: loads with the pitch, the code and the ratings of KEYS (and 0 without them). */
#define TERMINAL_CASE(KEY, VALUE, FIELD, CODE, PITCH_KEY, PITCH_UM) do { \
        PartLibrary *lib_ = part_library_create(); char *err_ = NULL; const Part *p_; \
        CHECK(load(lib_, PITCH_DEVICE("'" KEY "':'" VALUE "','pitch':" PITCH_KEY ",'ratedVolts':250,'ratedAmps':16,"), &err_) == 0); \
        p_ = part_library_find(lib_, "TB-T"); \
        CHECK(p_ != NULL && p_->kind == PART_CONNECTOR && p_->connector->FIELD == CODE && \
              p_->connector->pitch_um == PITCH_UM && p_->connector->rated_mv == 250000 && p_->connector->rated_ma == 16000 && \
              p_->connector->usb == 0 && p_->connector->card == NULL && p_->connector->contacts == 0 && \
              p_->connector->center_diameter_um == 0 && p_->connector->test_point == 0 && \
              (CODE == p_->connector->terminal || p_->connector->terminal == 0) && \
              (CODE == p_->connector->jst || p_->connector->jst == 0)); \
        free(err_); part_library_destroy(lib_); \
    } while (0)

static void check_terminal_connector(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    TERMINAL_CASE("terminal", "screw", terminal, 's', "5.08", 5080);
    TERMINAL_CASE("terminal", "spring", terminal, 'c', "5.08", 5080);
    TERMINAL_CASE("terminal", "push-in", terminal, 'p', "5.08", 5080);
    TERMINAL_CASE("terminal", "pluggable", terminal, 'h', "5.08", 5080);
    TERMINAL_CASE("jst", "XH", jst, 'X', "2.5", 2500);
    TERMINAL_CASE("jst", "PH", jst, 'P', "2.5", 2500);
    TERMINAL_CASE("jst", "EH", jst, 'E', "2.5", 2500);
    TERMINAL_CASE("jst", "ZH", jst, 'Z', "2.5", 2500);
    TERMINAL_CASE("jst", "SH", jst, 'S', "2.5", 2500);
    TERMINAL_CASE("jst", "GH", jst, 'G', "2.5", 2500);
    TERMINAL_CASE("jst", "VH", jst, 'V', "2.5", 2500);
    CHECK(load(library, PITCH_DEVICE("'terminal':'screw','pitch':3.5,"), &error) == 0);   /* the ratings are optional */
    part = part_library_find(library, "TB-T");
    CHECK(part != NULL && part->connector->terminal == 's' && part->connector->pitch_um == 3500 &&
          part->connector->rated_mv == 0 && part->connector->rated_ma == 0 && part->connector->jst == 0);
    free(error);
    part_library_destroy(library);
    printf("ok   terminal block and JST header connectors load; malformed ones fail\n");

    expect_fail("terminal and jst", PITCH_DEVICE("'terminal':'screw','jst':'XH','pitch':5,"),
                "a connector is one of \"usb\", \"terminal\" and \"jst\"");
    expect_fail("terminal and usb", PITCH_DEVICE("'terminal':'screw','usb':'A','pitch':5,"),
                "a connector is one of \"usb\", \"terminal\" and \"jst\"");
    expect_fail("jst and usb", PITCH_DEVICE("'jst':'XH','usb':'A','pitch':5,"),
                "a connector is one of \"usb\", \"terminal\" and \"jst\"");
    expect_fail("terminal: unknown clamp", PITCH_DEVICE("'terminal':'clip','pitch':5,"),
                "\"terminal\" must be \"screw\", \"spring\", \"push-in\" or \"pluggable\"");
    expect_fail("jst: unknown series", PITCH_DEVICE("'jst':'JH','pitch':2.5,"),
                "\"jst\" must be \"XH\", \"PH\", \"EH\", \"ZH\", \"SH\", \"GH\" or \"VH\"");
    expect_fail("terminal: no pitch", PITCH_DEVICE("'terminal':'screw',"), "a terminal block needs \"pitch\"");
    expect_fail("jst: no pitch", PITCH_DEVICE("'jst':'XH',"), "a JST connector needs \"pitch\"");
    expect_fail("terminal: pitch 0", PITCH_DEVICE("'terminal':'screw','pitch':0,"),
                "\"pitch\" must be above 0 and at most 20 mm");
    expect_fail("terminal: pitch 21", PITCH_DEVICE("'terminal':'screw','pitch':21,"),
                "\"pitch\" must be above 0 and at most 20 mm");
    expect_fail("jst: pitch 21", PITCH_DEVICE("'jst':'PH','pitch':21,"), "\"pitch\" must be above 0 and at most 20 mm");
    expect_fail("usb: pitch", PITCH_DEVICE("'usb':'A','pitch':5,"), "\"pitch\" is only for terminal blocks and JST connectors");
    expect_fail("barrel: pitch", PITCH_DEVICE("'centerPin':2,'ratedVolts':20,'ratedAmps':5,'pitch':5,"),
                "\"pitch\" is only for terminal blocks and JST connectors");
    expect_fail("terminal: centre pin", PITCH_DEVICE("'terminal':'screw','pitch':5,'centerPin':2,"),
                "a terminal block has no \"centerPin\"");
    expect_fail("terminal: contacts", PITCH_DEVICE("'terminal':'screw','pitch':5,'contacts':[['1','2']],"),
                "a terminal block has no \"contacts\"");
    expect_fail("jst: centre pin", PITCH_DEVICE("'jst':'XH','pitch':2.5,'centerPin':2,"),
                "a JST connector has no \"centerPin\"");
    expect_fail("jst: contacts", PITCH_DEVICE("'jst':'XH','pitch':2.5,'contacts':[['1','2']],"),
                "a JST connector has no \"contacts\"");
    expect_fail("terminal: zero amps", PITCH_DEVICE("'terminal':'screw','pitch':5,'ratedAmps':0,"),
                "\"ratedAmps\" must be above 0 and at most 100");
    expect_fail("jst: zero volts", PITCH_DEVICE("'jst':'XH','pitch':2.5,'ratedVolts':0,"), "ratings must be above 0");
    expect_fail("card: terminal", CARD_DEVICE("'card':'SD','terminal':'screw',"), "a card slot has no \"terminal\"");
    expect_fail("card: pitch", CARD_DEVICE("'card':'SD','pitch':5,"), "a card slot has no \"pitch\"");
    expect_fail("test point: jst", TEST_POINT_DEVICE("'testPoint':true,'jst':'XH',", TP_PIN),
                "a test point has no \"jst\"");
    expect_fail("barrel: still needs its centre pin", PITCH_DEVICE("'ratedVolts':20,'ratedAmps':5,"), "\"centerPin\"");
}

/* DQ_EXTRA, VCC_EXTRA: more keys on DQ0 (pin 2) and VCC (pin 6), such as "side". */

int main(void) {
    check_usb_connector();
    check_card_slot_connector();
    check_test_point_connector();
    check_terminal_connector();
    check_switch_cases();
    check_flipflop_cases();
    check_gate_cases();
    check_supply_cases();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
