/*
 * library_pins_check: split from library_check.c by plan-68-H; the shared helpers are
 * support/library_check_bench.c.
 */

#include "library_check_bench.h"

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

/* A pin's optional "side": in or out, on input, output and inout pins only. */
static void check_pin_side_cases(void) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    CHECK(load(library, SRAM_PINS_DEVICE("55", ",'side':'in'", ""), &error) == 0);
    part = part_library_find(library, "M1");
    CHECK(part != NULL && part->pins[1].side == PIN_SIDE_IN && part->pins[0].side == PIN_SIDE_AUTO);
    printf("ok   pin side%s%s\n", error != NULL ? ": " : "", error != NULL ? error : "");
    free(error);
    part_library_destroy(library);

    expect_fail("unknown pin side", SRAM_PINS_DEVICE("55", ",'side':'left'", ""),
                "pin 2: unknown side \"left\" (in or out)");
    expect_fail("side on a power pin", SRAM_PINS_DEVICE("55", "", ",'side':'out'"),
                "pin 6: \"side\" is for input, output and inout pins");
}

/* Pin 1 named NAME (the address pin) with more keys A_EXTRA; CE# (pin 3)
   with C_EXTRA; VCC (pin 6) with V_EXTRA. */
#define SRAM_LOW_DEVICE(NAME, A_EXTRA, C_EXTRA, V_EXTRA) \
    HEAD("sram") "{'id':'m','function':'1 KB','description':'d','drawer':['Memory','SRAM']," \
    "'pinouts':{'p6':[{'number':1,'name':'" NAME "','role':'input','group':'address'" A_EXTRA "}," \
    "{'number':2,'name':'DQ0','role':'inout'},{'number':3,'name':'CE#','role':'input'" C_EXTRA "}," \
    "{'number':4,'name':'OE#','role':'input'},{'number':5,'name':'WE#','role':'input'}," \
    "{'number':6,'name':'VCC','role':'power'" V_EXTRA "}]}," \
    "'address':['" NAME "'],'data':['DQ0'],'chipEnable':['CE#'],'chipEnableHigh':[]," \
    "'outputEnable':'OE#','writeEnable':'WE#','byteEnable':[]," \
    "'grades':{'55':{'trc':55,'taa':55,'twpOeLow':45,'note':'n','vcc':[2.7,5.5]}}," \
    "'parts':[{'name':'M1','package':'DIP','pinout':'p6','footprint':'T6','grade':'55'}]}" TAIL

/* Loads `text`, expects success and pins[0].active_low == mask0, pins[2] == mask2. */
static void expect_active_low(const char *name, const char *text, uint32_t mask0, uint32_t mask2) {
    PartLibrary *library = part_library_create();
    char *error = NULL;
    const Part *part;

    if (load(library, text, &error) != 0) {
        printf("FAIL %s: %s\n", name, error != NULL ? error : "(none)");
        failures++;
    } else {
        part = part_library_find(library, "M1");
        if (part == NULL || part->pins[0].active_low != mask0 || part->pins[2].active_low != mask2) {
            printf("FAIL %s: masks %u, %u (wanted %u, %u)\n", name,
                   part != NULL ? (unsigned)part->pins[0].active_low : 0u,
                   part != NULL ? (unsigned)part->pins[2].active_low : 0u,
                   (unsigned)mask0, (unsigned)mask2);
            failures++;
        } else {
            printf("ok   %s\n", name);
        }
    }
    free(error);
    part_library_destroy(library);
}

/* A pin's optional "activeLow" (plan-39-A § 3). */
static void check_pin_active_low_cases(void) {
    /* 33 alternatives: one more than the mask holds. */
#define ALT33 "B0/B1/B2/B3/B4/B5/B6/B7/B8/B9/B10/B11/B12/B13/B14/B15/B16/B17/B18/B19/" \
              "B20/B21/B22/B23/B24/B25/B26/B27/B28/B29/B30/B31/B32"

    expect_active_low("activeLow absent", SRAM_LOW_DEVICE("A0", "", "", ""), 0, 0);
    expect_active_low("activeLow true, one alternative", SRAM_LOW_DEVICE("A0", "", ",'activeLow':true", ""),
                      0, 1);
    expect_active_low("activeLow true, two alternatives",
                      SRAM_LOW_DEVICE("A0/CS", ",'activeLow':true", "", ""), 3, 0);
    expect_active_low("activeLow list", SRAM_LOW_DEVICE("A0/CS", ",'activeLow':['CS']", "", ""), 2, 0);
    expect_active_low("activeLow list, four alternatives",
                      SRAM_LOW_DEVICE("AIN7/GPIO3/DRDY/CLK", ",'activeLow':['DRDY','AIN7']", "", ""), 5, 0);
    expect_active_low("activeLow list matches every equal alternative",
                      SRAM_LOW_DEVICE("X/A/X", ",'activeLow':['X']", "", ""), 5, 0);
    expect_active_low("activeLow true, 33 alternatives",
                      SRAM_LOW_DEVICE(ALT33, ",'activeLow':true", "", ""), 0xFFFFFFFFu, 0);

    expect_fail("activeLow on a power pin", SRAM_LOW_DEVICE("A0", "", "", ",'activeLow':true"),
                "pin 6: \"activeLow\" is for input, output and inout pins");
    expect_fail("activeLow false", SRAM_LOW_DEVICE("A0", "", ",'activeLow':false", ""),
                "pin 3: \"activeLow\" must be true or a list of the name's alternatives");
    expect_fail("activeLow string", SRAM_LOW_DEVICE("A0", "", ",'activeLow':'CE#'", ""),
                "pin 3: \"activeLow\" must be true or a list of the name's alternatives");
    expect_fail("activeLow number", SRAM_LOW_DEVICE("A0", "", ",'activeLow':1", ""),
                "pin 3: \"activeLow\" must be true or a list of the name's alternatives");
    expect_fail("activeLow object", SRAM_LOW_DEVICE("A0", "", ",'activeLow':{}", ""),
                "pin 3: \"activeLow\" must be true or a list of the name's alternatives");
    expect_fail("activeLow list holding a number", SRAM_LOW_DEVICE("A0/CS", ",'activeLow':[1]", "", ""),
                "pin 1: \"activeLow\" must be true or a list of the name's alternatives");
    expect_fail("activeLow empty", SRAM_LOW_DEVICE("A0/CS", ",'activeLow':[]", "", ""),
                "pin 1: \"activeLow\" is empty");
    expect_fail("activeLow unknown alternative", SRAM_LOW_DEVICE("A0/CS", ",'activeLow':['CE']", "", ""),
                "pin 1: \"activeLow\" has no alternative \"CE\" in \"A0/CS\"");
    expect_fail("activeLow repeat", SRAM_LOW_DEVICE("A0/CS", ",'activeLow':['CS','CS']", "", ""),
                "pin 1: \"activeLow\" lists \"CS\" twice");
    expect_fail("activeLow every alternative", SRAM_LOW_DEVICE("A0/CS", ",'activeLow':['CS','A0']", "", ""),
                "pin 1: \"activeLow\" lists every alternative of \"A0/CS\"; write true");
    expect_fail("activeLow list on a one-alternative name", SRAM_LOW_DEVICE("A0", "", ",'activeLow':['CE#']", ""),
                "pin 3: \"activeLow\" lists every alternative of \"CE#\"; write true");
    expect_fail("activeLow list on 33 alternatives", SRAM_LOW_DEVICE(ALT33, ",'activeLow':['B3']", "", ""),
                "pin 1: \"activeLow\" is for names of at most 32 alternatives");
#undef ALT33
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

int main(void) {
    check_ball_cases();
    check_display_cases();
    check_sram_cases();
    check_pin_side_cases();
    check_pin_active_low_cases();
    check_eeprom_cases();
    check_cpu_cases();
    check_z80_cases();
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("library_check: all passed\n");
    return 0;
}
