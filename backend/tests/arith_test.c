/*
 * arith_test: the arith kind's models against the function tables of the
 * plan-22 devices (plan-22-B section 4, plan-22-C section 2). Fixture devices
 * copy the real devices' pinouts and model keys from
 * planning/plan-22-data/specs.md, with 10 ns delays; levels are checked
 * 50 ns after each step. Every expected value comes from a reference written
 * from the datasheet's table or equations, not from the model.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"
#include "model_pins.h"
#include "parts.h"
#include "sim.h"

#define NS(value) ((SimTime)((value) * 1000ull))

static int failures = 0;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* FIXTURE BEGIN */
/* Generated from planning/plan-22-data/specs.md (the spec JSON of each device). */
static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'arith','devices':[{'id':'fx-74_85','func"
    "tion':'Comparator','description':'4-bit magnitude comparators','drawer':[],'pinouts':{'p16"
    "':[{'number':1,'name':'B3','role':'input'},{'number':2,'name':'A<BIN','role':'input'},{'nu"
    "mber':3,'name':'A=BIN','role':'input'},{'number':4,'name':'A>BIN','role':'input'},{'number"
    "':5,'name':'A>BOUT','role':'output'},{'number':6,'name':'A=BOUT','role':'output'},{'number"
    "':7,'name':'A<BOUT','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9"
    ",'name':'B0','role':'input'},{'number':10,'name':'A0','role':'input'},{'number':11,'name':"
    "'B1','role':'input'},{'number':12,'name':'A1','role':'input'},{'number':13,'name':'A2','ro"
    "le':'input'},{'number':14,'name':'B2','role':'input'},{'number':15,'name':'A3','role':'inp"
    "ut'},{'number':16,'name':'VCC','role':'power'}]},'model':'compare','a':['A0','A1','A2','A3"
    "'],'b':['B0','B1','B2','B3'],'outputs':{'gt':{'pin':'A>BOUT','active':'high'},'eq':{'pin':"
    "'A=BOUT','active':'high'},'lt':{'pin':'A<BOUT','active':'high'}},'cascade':{'gt':'A>BIN','"
    "eq':'A=BIN','lt':'A<BIN'},'cascadeTable':[{'in':'HLL','out':'HLL'},{'in':'LLH','out':'LLH'"
    "},{'in':'XHX','out':'LHL'},{'in':'HLH','out':'LLL'},{'in':'LLL','out':'HLH'}],'timing':{'H"
    "C':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_85','package':'DIP','pinout'"
    ":'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4063B','function':'Comparator','descr"
    "iption':'CMOS 4-bit magnitude comparator','drawer':[],'pinouts':{'p16':[{'number':1,'name'"
    ":'B3','role':'input'},{'number':2,'name':'A<BIN','role':'input'},{'number':3,'name':'A=BIN"
    "','role':'input'},{'number':4,'name':'A>BIN','role':'input'},{'number':5,'name':'A>BOUT','"
    "role':'output'},{'number':6,'name':'A=BOUT','role':'output'},{'number':7,'name':'A<BOUT','"
    "role':'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'B0','role':'"
    "input'},{'number':10,'name':'A0','role':'input'},{'number':11,'name':'B1','role':'input'},"
    "{'number':12,'name':'A1','role':'input'},{'number':13,'name':'A2','role':'input'},{'number"
    "':14,'name':'B2','role':'input'},{'number':15,'name':'A3','role':'input'},{'number':16,'na"
    "me':'VDD','role':'power'}]},'model':'compare','a':['A0','A1','A2','A3'],'b':['B0','B1','B2"
    "','B3'],'outputs':{'gt':{'pin':'A>BOUT','active':'high'},'eq':{'pin':'A=BOUT','active':'hi"
    "gh'},'lt':{'pin':'A<BOUT','active':'high'}},'cascade':{'gt':'A>BIN','eq':'A=BIN','lt':'A<B"
    "IN'},'cascadeTable':[{'in':'HLL','out':'HLL'},{'in':'LHL','out':'LHL'},{'in':'LLH','out':'"
    "LLH'},{'in':'LLL','out':'LLL'},{'in':'LHH','out':'LLH'},{'in':'HLH','out':'HLH'},{'in':'HH"
    "L','out':'HLL'},{'in':'HHH','out':'HLH'}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}"
    "},'parts':[{'name':'FX-4063B','package':'DIP','pinout':'p16','footprint':'FX16','family':'"
    "HC'}]},{'id':'fx-4585B','function':'Comparator','description':'CMOS 4-bit magnitude compar"
    "ator','drawer':[],'pinouts':{'p16':[{'number':1,'name':'B2','role':'input'},{'number':2,'n"
    "ame':'A2','role':'input'},{'number':3,'name':'A=BOUT','role':'output'},{'number':4,'name':"
    "'A>BIN','role':'input'},{'number':5,'name':'A<BIN','role':'input'},{'number':6,'name':'A=B"
    "IN','role':'input'},{'number':7,'name':'A1','role':'input'},{'number':8,'name':'VSS','role"
    "':'ground'},{'number':9,'name':'B1','role':'input'},{'number':10,'name':'A0','role':'input"
    "'},{'number':11,'name':'B0','role':'input'},{'number':12,'name':'A<BOUT','role':'output'},"
    "{'number':13,'name':'A>BOUT','role':'output'},{'number':14,'name':'B3','role':'input'},{'n"
    "umber':15,'name':'A3','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':"
    "'compare','a':['A0','A1','A2','A3'],'b':['B0','B1','B2','B3'],'outputs':{'gt':{'pin':'A>BO"
    "UT','active':'high'},'eq':{'pin':'A=BOUT','active':'high'},'lt':{'pin':'A<BOUT','active':'"
    "high'}},'cascade':{'gt':'A>BIN','eq':'A=BIN','lt':'A<BIN'},'cascadeTable':[{'in':'HLL','ou"
    "t':'HLL'},{'in':'XHL','out':'LHL'},{'in':'XLH','out':'LLH'},{'in':'LLL','out':'LLL'},{'in'"
    ":'XHH','out':'LHH'},{'when':'gt','in':'HXX','out':'HLL'},{'when':'gt','in':'LXX','out':'LL"
    "L'}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4585B','packag"
    "e':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_682','function':'"
    "Comparator','description':'8-bit magnitude comparators','drawer':[],'pinouts':{'p20':[{'nu"
    "mber':1,'name':'P>Q','role':'output'},{'number':2,'name':'P0','role':'input'},{'number':3,"
    "'name':'Q0','role':'input'},{'number':4,'name':'P1','role':'input'},{'number':5,'name':'Q1"
    "','role':'input'},{'number':6,'name':'P2','role':'input'},{'number':7,'name':'Q2','role':'"
    "input'},{'number':8,'name':'P3','role':'input'},{'number':9,'name':'Q3','role':'input'},{'"
    "number':10,'name':'GND','role':'ground'},{'number':11,'name':'P4','role':'input'},{'number"
    "':12,'name':'Q4','role':'input'},{'number':13,'name':'P5','role':'input'},{'number':14,'na"
    "me':'Q5','role':'input'},{'number':15,'name':'P6','role':'input'},{'number':16,'name':'Q6'"
    ",'role':'input'},{'number':17,'name':'P7','role':'input'},{'number':18,'name':'Q7','role':"
    "'input'},{'number':19,'name':'P=Q','role':'output'},{'number':20,'name':'VCC','role':'powe"
    "r'}]},'model':'compare','a':['P0','P1','P2','P3','P4','P5','P6','P7'],'b':['Q0','Q1','Q2',"
    "'Q3','Q4','Q5','Q6','Q7'],'outputs':{'gt':{'pin':'P>Q','active':'low'},'eq':{'pin':'P=Q','"
    "active':'low'}},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_"
    "682','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_684',"
    "'function':'Comparator','description':'8-bit magnitude comparators','drawer':[],'pinouts':"
    "{'p20':[{'number':1,'name':'P>Q','role':'output'},{'number':2,'name':'P0','role':'input'},"
    "{'number':3,'name':'Q0','role':'input'},{'number':4,'name':'P1','role':'input'},{'number':"
    "5,'name':'Q1','role':'input'},{'number':6,'name':'P2','role':'input'},{'number':7,'name':'"
    "Q2','role':'input'},{'number':8,'name':'P3','role':'input'},{'number':9,'name':'Q3','role'"
    ":'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'P4','role':'inpu"
    "t'},{'number':12,'name':'Q4','role':'input'},{'number':13,'name':'P5','role':'input'},{'nu"
    "mber':14,'name':'Q5','role':'input'},{'number':15,'name':'P6','role':'input'},{'number':16"
    ",'name':'Q6','role':'input'},{'number':17,'name':'P7','role':'input'},{'number':18,'name':"
    "'Q7','role':'input'},{'number':19,'name':'P=Q','role':'output'},{'number':20,'name':'VCC',"
    "'role':'power'}]},'model':'compare','a':['P0','P1','P2','P3','P4','P5','P6','P7'],'b':['Q0"
    "','Q1','Q2','Q3','Q4','Q5','Q6','Q7'],'outputs':{'gt':{'pin':'P>Q','active':'low'},'eq':{'"
    "pin':'P=Q','active':'low'}},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'n"
    "ame':'FX-74_684','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':"
    "'fx-74_688','function':'Comparator','description':'8-bit identity comparators','drawer':[]"
    ",'pinouts':{'p20':[{'number':1,'name':'G','role':'input'},{'number':2,'name':'P0','role':'"
    "input'},{'number':3,'name':'Q0','role':'input'},{'number':4,'name':'P1','role':'input'},{'"
    "number':5,'name':'Q1','role':'input'},{'number':6,'name':'P2','role':'input'},{'number':7,"
    "'name':'Q2','role':'input'},{'number':8,'name':'P3','role':'input'},{'number':9,'name':'Q3"
    "','role':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'P4','rol"
    "e':'input'},{'number':12,'name':'Q4','role':'input'},{'number':13,'name':'P5','role':'inpu"
    "t'},{'number':14,'name':'Q5','role':'input'},{'number':15,'name':'P6','role':'input'},{'nu"
    "mber':16,'name':'Q6','role':'input'},{'number':17,'name':'P7','role':'input'},{'number':18"
    ",'name':'Q7','role':'input'},{'number':19,'name':'P=Q','role':'output'},{'number':20,'name"
    "':'VCC','role':'power'}]},'model':'compare','a':['P0','P1','P2','P3','P4','P5','P6','P7'],"
    "'b':['Q0','Q1','Q2','Q3','Q4','Q5','Q6','Q7'],'outputs':{'eq':{'pin':'P=Q','active':'low'}"
    "},'enable':{'pin':'G','active':'low'},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'p"
    "arts':[{'name':'FX-74_688','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'"
    "}]},{'id':'fx-74_518','function':'Comparator','description':'8-bit identity comparators','"
    "drawer':[],'pinouts':{'p20':[{'number':1,'name':'G','role':'input'},{'number':2,'name':'P0"
    "','role':'input'},{'number':3,'name':'Q0','role':'input'},{'number':4,'name':'P1','role':'"
    "input'},{'number':5,'name':'Q1','role':'input'},{'number':6,'name':'P2','role':'input'},{'"
    "number':7,'name':'Q2','role':'input'},{'number':8,'name':'P3','role':'input'},{'number':9,"
    "'name':'Q3','role':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name'"
    ":'P4','role':'input'},{'number':12,'name':'Q4','role':'input'},{'number':13,'name':'P5','r"
    "ole':'input'},{'number':14,'name':'Q5','role':'input'},{'number':15,'name':'P6','role':'in"
    "put'},{'number':16,'name':'Q6','role':'input'},{'number':17,'name':'P7','role':'input'},{'"
    "number':18,'name':'Q7','role':'input'},{'number':19,'name':'P=Q','role':'output'},{'number"
    "':20,'name':'VCC','role':'power'}]},'model':'compare','a':['P0','P1','P2','P3','P4','P5','"
    "P6','P7'],'b':['Q0','Q1','Q2','Q3','Q4','Q5','Q6','Q7'],'outputs':{'eq':{'pin':'P=Q','acti"
    "ve':'high'}},'enable':{'pin':'G','active':'low'},'output':'open-drain','timing':{'HC':{'ri"
    "se':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_518','package':'DIP','pinout':'p20'"
    ",'footprint':'FX20','family':'HC'}]},{'id':'fx-74_521','function':'Comparator','descriptio"
    "n':'8-bit identity comparators','drawer':[],'pinouts':{'p20':[{'number':1,'name':'G','role"
    "':'input'},{'number':2,'name':'P0','role':'input'},{'number':3,'name':'Q0','role':'input'}"
    ",{'number':4,'name':'P1','role':'input'},{'number':5,'name':'Q1','role':'input'},{'number'"
    ":6,'name':'P2','role':'input'},{'number':7,'name':'Q2','role':'input'},{'number':8,'name':"
    "'P3','role':'input'},{'number':9,'name':'Q3','role':'input'},{'number':10,'name':'GND','ro"
    "le':'ground'},{'number':11,'name':'P4','role':'input'},{'number':12,'name':'Q4','role':'in"
    "put'},{'number':13,'name':'P5','role':'input'},{'number':14,'name':'Q5','role':'input'},{'"
    "number':15,'name':'P6','role':'input'},{'number':16,'name':'Q6','role':'input'},{'number':"
    "17,'name':'P7','role':'input'},{'number':18,'name':'Q7','role':'input'},{'number':19,'name"
    "':'P=Q','role':'output'},{'number':20,'name':'VCC','role':'power'}]},'model':'compare','a'"
    ":['P0','P1','P2','P3','P4','P5','P6','P7'],'b':['Q0','Q1','Q2','Q3','Q4','Q5','Q6','Q7'],'"
    "outputs':{'eq':{'pin':'P=Q','active':'low'}},'enable':{'pin':'G','active':'low'},'timing':"
    "{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_521','package':'DIP','pin"
    "out':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_885','function':'Comparator','"
    "description':'8-bit magnitude comparators','drawer':[],'pinouts':{'p24':[{'number':1,'name"
    "':'L/A','role':'input'},{'number':2,'name':'P<QIN','role':'input'},{'number':3,'name':'P>Q"
    "IN','role':'input'},{'number':4,'name':'Q7','role':'input'},{'number':5,'name':'Q6','role'"
    ":'input'},{'number':6,'name':'Q5','role':'input'},{'number':7,'name':'Q4','role':'input'},"
    "{'number':8,'name':'Q3','role':'input'},{'number':9,'name':'Q2','role':'input'},{'number':"
    "10,'name':'Q1','role':'input'},{'number':11,'name':'Q0','role':'input'},{'number':12,'name"
    "':'GND','role':'ground'},{'number':13,'name':'P>QOUT','role':'output'},{'number':14,'name'"
    ":'P<QOUT','role':'output'},{'number':15,'name':'P0','role':'input'},{'number':16,'name':'P"
    "1','role':'input'},{'number':17,'name':'P2','role':'input'},{'number':18,'name':'P3','role"
    "':'input'},{'number':19,'name':'P4','role':'input'},{'number':20,'name':'P5','role':'input"
    "'},{'number':21,'name':'P6','role':'input'},{'number':22,'name':'P7','role':'input'},{'num"
    "ber':23,'name':'PLE','role':'input'},{'number':24,'name':'VCC','role':'power'}]},'model':'"
    "compare','a':['P0','P1','P2','P3','P4','P5','P6','P7'],'b':['Q0','Q1','Q2','Q3','Q4','Q5',"
    "'Q6','Q7'],'outputs':{'gt':{'pin':'P>QOUT','active':'high'},'lt':{'pin':'P<QOUT','active':"
    "'high'}},'cascade':{'gt':'P>QIN','lt':'P<QIN'},'latch':{'pin':'PLE','active':'low'},'signe"
    "d':{'pin':'L/A','active':'low'},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':"
    "[{'name':'FX-74_885','package':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]},{'"
    "id':'fx-74_280','function':'Parity','description':'9-bit odd/even parity generators/checke"
    "rs','drawer':[],'pinouts':{'p14':[{'number':1,'name':'G','role':'input'},{'number':2,'name"
    "':'H','role':'input'},{'number':3,'name':'NC','role':'nc'},{'number':4,'name':'I','role':'"
    "input'},{'number':5,'name':'EVEN','role':'output'},{'number':6,'name':'ODD','role':'output"
    "'},{'number':7,'name':'GND','role':'ground'},{'number':8,'name':'A','role':'input'},{'numb"
    "er':9,'name':'B','role':'input'},{'number':10,'name':'C','role':'input'},{'number':11,'nam"
    "e':'D','role':'input'},{'number':12,'name':'E','role':'input'},{'number':13,'name':'F','ro"
    "le':'input'},{'number':14,'name':'VCC','role':'power'}]},'model':'parity','inputs':['A','B"
    "','C','D','E','F','G','H','I'],'even':'EVEN','odd':'ODD','timing':{'HC':{'rise':10,'fall':"
    "10,'vcc':[2,6]}},'parts':[{'name':'FX-74_280','package':'DIP','pinout':'p14','footprint':'"
    "FX14','family':'HC'}]},{'id':'fx-74_286','function':'Parity','description':'9-bit parity g"
    "enerators/checkers with bus-driver parity I/O port','drawer':[],'pinouts':{'p14':[{'number"
    "':1,'name':'G','role':'input'},{'number':2,'name':'H','role':'input'},{'number':3,'name':'"
    "XMIT','role':'input'},{'number':4,'name':'I','role':'input'},{'number':5,'name':'PARITY ER"
    "ROR','role':'output'},{'number':6,'name':'PARITY I/O','role':'inout'},{'number':7,'name':'"
    "GND','role':'ground'},{'number':8,'name':'A','role':'input'},{'number':9,'name':'B','role'"
    ":'input'},{'number':10,'name':'C','role':'input'},{'number':11,'name':'D','role':'input'},"
    "{'number':12,'name':'E','role':'input'},{'number':13,'name':'F','role':'input'},{'number':"
    "14,'name':'VCC','role':'power'}]},'model':'parity','inputs':['A','B','C','D','E','F','G','"
    "H','I'],'bus':{'pin':'PARITY I/O','xmit':{'pin':'XMIT','active':'low'},'drives':'even'},'e"
    "rror':{'pin':'PARITY ERROR','active':'low'},'errorOn':'even','timing':{'HC':{'rise':10,'fa"
    "ll':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_286','package':'DIP','pinout':'p14','footprin"
    "t':'FX14','family':'HC'}]},{'id':'fx-74_11286','function':'Parity','description':'9-bit pa"
    "rity generator/checker with bus driver parity I/O ports','drawer':[],'pinouts':{'p14':[{'n"
    "umber':1,'name':'B','role':'input'},{'number':2,'name':'A','role':'input'},{'number':3,'na"
    "me':'PARITY I/O','role':'inout'},{'number':4,'name':'GND','role':'ground'},{'number':5,'na"
    "me':'PARITY ERROR','role':'output'},{'number':6,'name':'XMIT','role':'input'},{'number':7,"
    "'name':'I','role':'input'},{'number':8,'name':'H','role':'input'},{'number':9,'name':'G','"
    "role':'input'},{'number':10,'name':'F','role':'input'},{'number':11,'name':'VCC','role':'p"
    "ower'},{'number':12,'name':'E','role':'input'},{'number':13,'name':'D','role':'input'},{'n"
    "umber':14,'name':'C','role':'input'}]},'model':'parity','inputs':['A','B','C','D','E','F',"
    "'G','H','I'],'bus':{'pin':'PARITY I/O','xmit':{'pin':'XMIT','active':'low'},'drives':'even"
    "'},'error':{'pin':'PARITY ERROR','active':'low'},'errorOn':'even','timing':{'HC':{'rise':1"
    "0,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_11286','package':'DIP','pinout':'p14','f"
    "ootprint':'FX14','family':'HC'}]}]}";

/* Pin counts of the fixture devices: [14, 16, 20, 24] */
/* FIXTURE END */

/* Dual-row fixture footprints FX<n>, one per pin count above. */
#define FX_FOOTPRINT(N) \
    "{'id':'FX" #N "','description':'d','style':'dual','pins':" #N ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[" FX_FOOTPRINT(14) ","
    FX_FOOTPRINT(16) "," FX_FOOTPRINT(20) "," FX_FOOTPRINT(24) "]}";

static char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

/* Loads text (single-quote style); returns the error (caller frees) or NULL. */
static char *load(PartLibrary *lib, const char *text, const char *source) {
    char *json = dequote(text);
    char *error = NULL;

    if (load_parts(lib, json, source, &error) == 0) {
        free(error);
        error = NULL;
    } else if (error == NULL) {
        error = strdup("(no message)");
    }
    free(json);
    return error;
}

static PartLibrary *fixture_library(void) {
    PartLibrary *lib = part_library_create();
    char *json = dequote(FIXTURE_FOOTPRINTS);
    char *error = NULL;

    if (lib == NULL || load_footprints(lib, json, "fx-footprints.json", &error) != 0) {
        fprintf(stderr, "fixture footprints: %s\n", error != NULL ? error : "(no message)");
        exit(1);
    }
    free(json);
    free(error);
    return lib;
}

/* ---- Bench helpers -------------------------------------------------------- */

static const LogicValue L0 = LOGIC_0, L1 = LOGIC_1, LX = LOGIC_X, LZ = LOGIC_Z;

static const Part *fx_part(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = part_library_find(library, name);
    if (part == NULL || part->kind != PART_ARITH) {
        fprintf(stderr, "no arith part %s\n", name);
        exit(1);
    }
    return part;
}

static size_t pin_index(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    fprintf(stderr, "%s has no pin %s\n", part->name, name);
    exit(1);
}

static Bench *open_fx(const char *device) {
    return bench_create(fx_part(device));
}

static void step(Bench *b) {
    bench_run(b, NS(50));
}

static void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

static LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

static LogicValue level(int high) {
    return high ? LOGIC_1 : LOGIC_0;
}

/* Sets the pins fmt % i (i < n) from value's bits. */
static void set_bus(Bench *b, const char *fmt, unsigned first, unsigned n, unsigned value) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, first + i);
        set(b, name, bit(value, i));
    }
}

static int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

/* Pins fmt % i (i < n) equal value's bits (LSB first). */
static int bus_is(Bench *b, const char *fmt, unsigned first, unsigned n, unsigned value) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, first + i);
        ok &= is(b, name, bit(value, i));
    }
    return ok;
}

static unsigned ones(unsigned v) {
    unsigned n = 0;

    for (; v != 0; v >>= 1) {
        n += v & 1u;
    }
    return n;
}

/* ---- compare: 4-bit magnitude comparators with cascade ---------------------- */

/*
 * The reference outputs (gt, eq, lt as bits 2, 1, 0) for equal words and
 * cascade inputs g, e, l, as each datasheet gives them.
 */
typedef unsigned (*CascadeRef)(unsigned g, unsigned e, unsigned l);

/* sn74ls85 p.2 / cd74hc85 p.10: HLL→HLL, LLH→LLH, XHX→LHL, HLH→LLL, LLL→HLH. */
static unsigned ref_85(unsigned g, unsigned e, unsigned l) {
    if (e) {
        return 2;
    }
    if (g && !l) {
        return 4;
    }
    if (!g && l) {
        return 1;
    }
    return g ? 0 : 5;
}

/* cd4063b p.2 table + p.3 logic diagram: gt = g, lt = l, eq = e AND NOT g AND NOT l. */
static unsigned ref_4063(unsigned g, unsigned e, unsigned l) {
    return (g << 2) | ((e && !g && !l) << 1) | l;
}

/* cd4585b p.2 table + Fig. 4: lt = l, eq = e, gt = g AND NOT e AND NOT l. */
static unsigned ref_4585(unsigned g, unsigned e, unsigned l) {
    return ((g && !e && !l) << 2) | (e << 1) | l;
}

static int outputs_are(Bench *b, unsigned want) {
    return is(b, "A>BOUT", bit(want, 2)) & is(b, "A=BOUT", bit(want, 1)) &
           is(b, "A<BOUT", bit(want, 0));
}

/*
 * Every A, B pair with every cascade combination: equal words follow ref;
 * A > B gives gt only (4585B: gt only while A>B in is high, else nothing),
 * A < B lt only.
 */
static void check_cascade4(const char *device, CascadeRef ref, int gt_gated) {
    Bench *b = open_fx(device);
    unsigned a, v, c;
    int ok = 1;

    for (c = 0; c < 8 && ok; c++) {
        set(b, "A>BIN", bit(c, 2));
        set(b, "A=BIN", bit(c, 1));
        set(b, "A<BIN", bit(c, 0));
        for (a = 0; a < 16 && ok; a++) {
            for (v = 0; v < 16 && ok; v++) {
                unsigned want = a == v ? ref((c >> 2) & 1u, (c >> 1) & 1u, c & 1u)
                                : a > v ? (gt_gated && !((c >> 2) & 1u) ? 0u : 4u)
                                        : 1u;

                set_bus(b, "A%u", 0, 4, a);
                set_bus(b, "B%u", 0, 4, v);
                step(b);
                if (!outputs_are(b, want)) {
                    printf("  %s A=%u B=%u cascade %u\n", device, a, v, c);
                    ok = 0;
                }
            }
        }
    }
    printf("%s: %s 256 word pairs x 8 cascade levels (datasheet table)\n", ok ? "PASS" : "FAIL",
           device);
    failures += !ok;
    bench_destroy(b);
}

/* X rule: an unknown bit that cannot change the result leaves the outputs known. */
static void check_85_unknown(void) {
    Bench *b = open_fx("74_85");
    int ok = 1;

    set(b, "A>BIN", L0);
    set(b, "A=BIN", L1);
    set(b, "A<BIN", L0);
    set_bus(b, "A%u", 0, 4, 8);
    set_bus(b, "B%u", 0, 4, 3);
    set(b, "A0", LZ);   /* Z reads X */
    step(b);
    ok &= outputs_are(b, 4);
    set_bus(b, "A%u", 0, 4, 3);
    set(b, "A0", LX);
    step(b);
    /* A is 2 or 3 against B = 3 with A=B in high: A>B out is low either way */
    ok &= is(b, "A>BOUT", L0) & is(b, "A=BOUT", LX) & is(b, "A<BOUT", LX);
    set(b, "A0", L1);
    set(b, "A=BIN", LX);
    step(b);
    ok &= is(b, "A>BOUT", LX) & is(b, "A=BOUT", LX) & is(b, "A<BOUT", LX);
    expect(ok, "74_85 X rule: a deciding known bit wins over an unknown one; X cascade at A=B gives X");
    bench_destroy(b);
}

/* ---- compare: 8-bit magnitude / identity comparators ------------------------- */

/* '682 / '684: P>Q and P=Q, active low. */
static void check_682(const char *device) {
    Bench *b = open_fx(device);
    unsigned p, k;
    int ok = 1;
    char what[100];

    for (p = 0; p < 256 && ok; p++) {
        for (k = 0; k < 18 && ok; k++) {
            unsigned q = k == 16 ? p : k == 17 ? (p + 1) & 255u : (p * 7 + k * 29) & 255u;

            set_bus(b, "P%u", 0, 8, p);
            set_bus(b, "Q%u", 0, 8, q);
            step(b);
            ok &= is(b, "P>Q", level(!(p > q))) & is(b, "P=Q", level(p != q));
            if (!ok) {
                printf("  %s P=%u Q=%u\n", device, p, q);
            }
        }
    }
    snprintf(what, sizeof what, "%s P>Q and P=Q active low over 4608 word pairs", device);
    expect(ok, what);
    bench_destroy(b);
}

/* Identity comparators with an enable G (active low): '688, '521, '518 (open drain). */
static void check_identity(const char *device, int eq_active_high, int open_drain) {
    Bench *b = open_fx(device);
    const Part *part = fx_part(device);
    LogicValue asserted = eq_active_high ? (open_drain ? LZ : L1) : L0;
    LogicValue inactive = eq_active_high ? L0 : L1;
    unsigned p;
    int ok = 1;
    char what[120];

    set(b, "G", L0);
    for (p = 0; p < 256 && ok; p++) {
        set_bus(b, "P%u", 0, 8, p);
        set_bus(b, "Q%u", 0, 8, p);
        step(b);
        ok &= is(b, "P=Q", asserted);
        set_bus(b, "Q%u", 0, 8, p ^ (1u << (p % 8)));
        step(b);
        ok &= is(b, "P=Q", inactive);
    }
    set_bus(b, "Q%u", 0, 8, 255);
    set(b, "G", L1);
    step(b);
    ok &= is(b, "P=Q", inactive);
    set(b, "G", LZ);
    step(b);
    ok &= is(b, "P=Q", LX);
    ok &= part_pin_open_drain(part, pin_index(part, "P=Q")) == open_drain;
    snprintf(what, sizeof what, "%s equal/unequal for 256 words; G high -> P=Q inactive; open drain %d",
             device, open_drain);
    expect(ok, what);
    bench_destroy(b);
}

/* '885: latched P, logical / arithmetic comparison, cascade on equal words. */
static void check_885(void) {
    Bench *b = open_fx("74_885");
    int ok = 1;

    /* power-up clear: the latch holds 0 until PLE goes high */
    set(b, "PLE", L0);
    set(b, "L/A", L1);
    set(b, "P>QIN", L1);
    set(b, "P<QIN", L0);
    set_bus(b, "P%u", 0, 8, 0x55);
    set_bus(b, "Q%u", 0, 8, 0);
    step(b);
    ok &= is(b, "P>QOUT", L1) & is(b, "P<QOUT", L0);   /* held 0 = Q: cascade passes */
    set(b, "P>QIN", L0);
    set(b, "P<QIN", L1);
    step(b);
    ok &= is(b, "P>QOUT", L0) & is(b, "P<QOUT", L1);
    /* transparent while PLE is high */
    set(b, "PLE", L1);
    set_bus(b, "P%u", 0, 8, 5);
    set_bus(b, "Q%u", 0, 8, 5);
    set(b, "P<QIN", L0);
    step(b);
    ok &= is(b, "P>QOUT", L0) & is(b, "P<QOUT", L0);
    /* held while low */
    set(b, "PLE", L0);
    step(b);
    set_bus(b, "P%u", 0, 8, 9);
    step(b);
    ok &= is(b, "P>QOUT", L0) & is(b, "P<QOUT", L0);
    set_bus(b, "Q%u", 0, 8, 3);
    step(b);
    ok &= is(b, "P>QOUT", L1) & is(b, "P<QOUT", L0);
    expect(ok, "74_885 power-up clear, PLE high transparent / low holds P, cascade on equal words");

    ok = 1;
    set(b, "PLE", L1);
    set_bus(b, "P%u", 0, 8, 0x80);
    set_bus(b, "Q%u", 0, 8, 0x01);
    set(b, "L/A", L1);   /* logical: 128 > 1 */
    step(b);
    ok &= is(b, "P>QOUT", L1) & is(b, "P<QOUT", L0);
    set(b, "L/A", L0);   /* arithmetic: -128 < 1 */
    step(b);
    ok &= is(b, "P>QOUT", L0) & is(b, "P<QOUT", L1);
    set(b, "L/A", LZ);
    step(b);
    ok &= is(b, "P>QOUT", LX) & is(b, "P<QOUT", LX);
    set_bus(b, "Q%u", 0, 8, 0x90);   /* both negative, both above 127: same order either way */
    step(b);
    ok &= is(b, "P>QOUT", L0) & is(b, "P<QOUT", L1);
    expect(ok, "74_885 L/A: 0x80 vs 0x01 logical greater, arithmetic less; X L/A only where it matters");
    bench_destroy(b);
}

/* ---- parity ------------------------------------------------------------------ */

static const char *const NINE[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I"};

static void set_nine(Bench *b, unsigned v) {
    unsigned i;

    for (i = 0; i < 9; i++) {
        set(b, NINE[i], bit(v, i));
    }
}

static void check_280(void) {
    Bench *b = open_fx("74_280");
    unsigned v;
    int ok = 1;

    for (v = 0; v < 512; v++) {
        set_nine(b, v);
        step(b);
        ok &= is(b, "EVEN", level(ones(v) % 2 == 0)) & is(b, "ODD", level(ones(v) % 2 == 1));
    }
    expect(ok, "74_280 all 512 input patterns: EVEN high for an even count, ODD the complement");
    ok = 1;
    set(b, "E", LX);
    step(b);
    ok &= is(b, "EVEN", LX) & is(b, "ODD", LX);
    set(b, "E", LZ);
    step(b);
    ok &= is(b, "EVEN", LX) & is(b, "ODD", LX);
    expect(ok, "74_280 an X or Z input makes both outputs X");
    bench_destroy(b);
}

/* '286 / 74ACT11286: XMIT low drives PARITY I/O; XMIT high checks it. */
static void check_286(const char *device) {
    Bench *b = open_fx(device);
    unsigned v, io;
    int ok = 1;
    char what[120];

    set(b, "PARITY I/O", LZ);
    set(b, "XMIT", L0);
    for (v = 0; v < 512; v++) {
        set_nine(b, v);
        step(b);
        ok &= is(b, "PARITY I/O", level(ones(v) % 2 == 0)) & is(b, "PARITY ERROR", L1);
    }
    snprintf(what, sizeof what, "%s XMIT low: PARITY I/O high for an even count of A-I, ERROR high",
             device);
    expect(ok, what);
    ok = 1;
    set(b, "XMIT", L1);
    for (v = 0; v < 512; v++) {
        for (io = 0; io < 2; io++) {
            set_nine(b, v);
            set(b, "PARITY I/O", bit(io, 0));
            step(b);
            /* sn74as286 p.1: even count + I/O high, or odd + low -> H; else L */
            ok &= is(b, "PARITY ERROR", level((ones(v) + io) % 2 == 1));
        }
    }
    set(b, "PARITY I/O", LZ);
    step(b);
    ok &= is(b, "PARITY ERROR", LX) & is(b, "PARITY I/O", LZ);
    set(b, "XMIT", LZ);
    step(b);
    ok &= is(b, "PARITY ERROR", LX) & is(b, "PARITY I/O", LX);
    snprintf(what, sizeof what,
             "%s XMIT high: ERROR low on an even total of A-I and PARITY I/O (1024 cases); X XMIT -> X",
             device);
    expect(ok, what);
    bench_destroy(b);
}

int main(void) {
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "arith_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_cascade4("74_85", ref_85, 0);
    check_cascade4("4063B", ref_4063, 0);
    check_cascade4("4585B", ref_4585, 1);
    check_85_unknown();
    check_682("74_682");
    check_682("74_684");
    check_identity("74_688", 0, 0);
    check_identity("74_521", 0, 0);
    check_identity("74_518", 1, 1);
    check_885();
    check_280();
    check_286("74_286");
    check_286("74_11286");
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("arith_test: all passed\n");
    return 0;
}
