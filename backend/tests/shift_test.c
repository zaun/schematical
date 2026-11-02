/*
 * shift_test: the shift kind's shift model against the function tables of
 * the devices that use it (plan-10-G section 5). Fixture devices copy the
 * real devices' pinouts and keys from the spec sections (no shift part file
 * exists before plan-10-I), with 10 ns delays; levels are checked 50 ns
 * after each step. Cases 24-43 (plan-10-H) cover the ls673, tap64,
 * bus-register, fifo and serial-in models.
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

/* Generated from the spec sections (planning/plan-10-data). */
static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'shift','devices':[{'id':'fx-74_164','f"
    "unction':'Shift register','description':'8-bit parallel-out serial shift register','draw"
    "er':[],'pinouts':{'p14':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','"
    "role':'input'},{'number':3,'name':'QA','role':'output'},{'number':4,'name':'QB','role':'"
    "output'},{'number':5,'name':'QC','role':'output'},{'number':6,'name':'QD','role':'output"
    "'},{'number':7,'name':'GND','role':'ground'},{'number':8,'name':'CLK','role':'input'},{'"
    "number':9,'name':'CLR','role':'input'},{'number':10,'name':'QE','role':'output'},{'numbe"
    "r':11,'name':'QF','role':'output'},{'number':12,'name':'QG','role':'output'},{'number':1"
    "3,'name':'QH','role':'output'},{'number':14,'name':'VCC','role':'power'}]},'model':'shif"
    "t','units':[{'stages':8,'clock':{'pin':'CLK','edge':'rising'},'serial':['A','B'],'clear'"
    ":{'pin':'CLR','active':'low','when':'async'},'outputs':[{'stage':1,'pin':'QA'},{'stage':"
    "2,'pin':'QB'},{'stage':3,'pin':'QC'},{'stage':4,'pin':'QD'},{'stage':5,'pin':'QE'},{'sta"
    "ge':6,'pin':'QF'},{'stage':7,'pin':'QG'},{'stage':8,'pin':'QH'}]}],'timing':{'HC':{'rise"
    "':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_164','package':'DIP','pinout':'p14'"
    ",'footprint':'FX14','family':'HC'}]},{'id':'fx-74_264','function':'Shift register','desc"
    "ription':'8-bit parallel-out serial shift register with inverted outputs','drawer':[],'p"
    "inouts':{'p14':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'in"
    "put'},{'number':3,'name':'QA','role':'output'},{'number':4,'name':'QB','role':'output'},"
    "{'number':5,'name':'QC','role':'output'},{'number':6,'name':'QD','role':'output'},{'numb"
    "er':7,'name':'GND','role':'ground'},{'number':8,'name':'CLK','role':'input'},{'number':9"
    ",'name':'CLR','role':'input'},{'number':10,'name':'QE','role':'output'},{'number':11,'na"
    "me':'QF','role':'output'},{'number':12,'name':'QG','role':'output'},{'number':13,'name':"
    "'QH','role':'output'},{'number':14,'name':'VCC','role':'power'}]},'model':'shift','units"
    "':[{'stages':8,'clock':{'pin':'CLK','edge':'rising'},'serial':['A','B'],'clear':{'pin':'"
    "CLR','active':'low','when':'async'},'outputs':[{'stage':1,'pin':'QA','invert':true},{'st"
    "age':2,'pin':'QB','invert':true},{'stage':3,'pin':'QC','invert':true},{'stage':4,'pin':'"
    "QD','invert':true},{'stage':5,'pin':'QE','invert':true},{'stage':6,'pin':'QF','invert':t"
    "rue},{'stage':7,'pin':'QG','invert':true},{'stage':8,'pin':'QH','invert':true}]}],'timin"
    "g':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_264','package':'DIP'"
    ",'pinout':'p14','footprint':'FX14','family':'HC'}]},{'id':'fx-74_165','function':'Shift "
    "register','description':'Parallel-load 8-bit shift register','drawer':[],'pinouts':{'p16"
    "':[{'number':1,'name':'SH/LD','role':'input'},{'number':2,'name':'CLK','role':'input'},{"
    "'number':3,'name':'E','role':'input'},{'number':4,'name':'F','role':'input'},{'number':5"
    ",'name':'G','role':'input'},{'number':6,'name':'H','role':'input'},{'number':7,'name':'Q"
    "HBAR','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'QH'"
    ",'role':'output'},{'number':10,'name':'SER','role':'input'},{'number':11,'name':'A','rol"
    "e':'input'},{'number':12,'name':'B','role':'input'},{'number':13,'name':'C','role':'inpu"
    "t'},{'number':14,'name':'D','role':'input'},{'number':15,'name':'CLKINH','role':'input'}"
    ",{'number':16,'name':'VCC','role':'power'}]},'model':'shift','units':[{'stages':8,'clock"
    "':{'pin':'CLK','edge':'rising'},'clockGate':{'pin':'CLKINH','op':'or'},'serial':['SER'],"
    "'load':{'pin':'SH/LD','active':'low','when':'async','from':'data'},'data':['A','B','C','"
    "D','E','F','G','H'],'outputs':[{'stage':8,'pin':'QH'},{'stage':8,'pin':'QHBAR','invert':"
    "true}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_165',"
    "'package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_166','fu"
    "nction':'Shift register','description':'Parallel-load 8-bit shift register','drawer':[],"
    "'pinouts':{'p16':[{'number':1,'name':'SER','role':'input'},{'number':2,'name':'A','role'"
    ":'input'},{'number':3,'name':'B','role':'input'},{'number':4,'name':'C','role':'input'},"
    "{'number':5,'name':'D','role':'input'},{'number':6,'name':'CLKINH','role':'input'},{'num"
    "ber':7,'name':'CLK','role':'input'},{'number':8,'name':'GND','role':'ground'},{'number':"
    "9,'name':'CLR','role':'input'},{'number':10,'name':'E','role':'input'},{'number':11,'nam"
    "e':'F','role':'input'},{'number':12,'name':'G','role':'input'},{'number':13,'name':'QH',"
    "'role':'output'},{'number':14,'name':'H','role':'input'},{'number':15,'name':'SH/LD','ro"
    "le':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'shift','units':[{'stag"
    "es':8,'clock':{'pin':'CLK','edge':'rising'},'clockGate':{'pin':'CLKINH','op':'or'},'seri"
    "al':['SER'],'load':{'pin':'SH/LD','active':'low','when':'clock','from':'data'},'data':['"
    "A','B','C','D','E','F','G','H'],'clear':{'pin':'CLR','active':'low','when':'async'},'out"
    "puts':[{'stage':8,'pin':'QH'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'part"
    "s':[{'name':'FX-74_166','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}"
    "]},{'id':'fx-74_194','function':'Shift register','description':'4-bit bidirectional univ"
    "ersal shift register','drawer':[],'pinouts':{'p16':[{'number':1,'name':'CLR','role':'inp"
    "ut'},{'number':2,'name':'SRSER','role':'input'},{'number':3,'name':'A','role':'input'},{"
    "'number':4,'name':'B','role':'input'},{'number':5,'name':'C','role':'input'},{'number':6"
    ",'name':'D','role':'input'},{'number':7,'name':'SLSER','role':'input'},{'number':8,'name"
    "':'GND','role':'ground'},{'number':9,'name':'S0','role':'input'},{'number':10,'name':'S1"
    "','role':'input'},{'number':11,'name':'CLK','role':'input'},{'number':12,'name':'QD','ro"
    "le':'output'},{'number':13,'name':'QC','role':'output'},{'number':14,'name':'QB','role':"
    "'output'},{'number':15,'name':'QA','role':'output'},{'number':16,'name':'VCC','role':'po"
    "wer'}]},'model':'shift','units':[{'stages':4,'clock':{'pin':'CLK','edge':'rising'},'mode"
    "s':{'pins':['S0','S1'],'table':['hold','right','left','load']},'serial':['SRSER'],'leftS"
    "erial':'SLSER','data':['A','B','C','D'],'clear':{'pin':'CLR','active':'low','when':'asyn"
    "c'},'outputs':[{'stage':1,'pin':'QA'},{'stage':2,'pin':'QB'},{'stage':3,'pin':'QC'},{'st"
    "age':4,'pin':'QD'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name'"
    ":'FX-74_194','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'f"
    "x-40194B','function':'Shift register','description':'4-bit bidirectional universal shift"
    " register','drawer':[],'pinouts':{'p16':[{'number':1,'name':'RESET','role':'input'},{'nu"
    "mber':2,'name':'SHIFTRIGHT','role':'input'},{'number':3,'name':'D0','role':'input'},{'nu"
    "mber':4,'name':'D1','role':'input'},{'number':5,'name':'D2','role':'input'},{'number':6,"
    "'name':'D3','role':'input'},{'number':7,'name':'SHIFTLEFT','role':'input'},{'number':8,'"
    "name':'VSS','role':'ground'},{'number':9,'name':'S0','role':'input'},{'number':10,'name'"
    ":'S1','role':'input'},{'number':11,'name':'CLOCK','role':'input'},{'number':12,'name':'Q"
    "3','role':'output'},{'number':13,'name':'Q2','role':'output'},{'number':14,'name':'Q1','"
    "role':'output'},{'number':15,'name':'Q0','role':'output'},{'number':16,'name':'VDD','rol"
    "e':'power'}]},'model':'shift','units':[{'stages':4,'clock':{'pin':'CLOCK','edge':'rising"
    "'},'modes':{'pins':['S0','S1'],'table':['hold','right','left','load']},'serial':['SHIFTR"
    "IGHT'],'leftSerial':'SHIFTLEFT','data':['D0','D1','D2','D3'],'clear':{'pin':'RESET','act"
    "ive':'low','when':'async'},'outputs':[{'stage':1,'pin':'Q0'},{'stage':2,'pin':'Q1'},{'st"
    "age':3,'pin':'Q2'},{'stage':4,'pin':'Q3'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':["
    "2,6]}},'parts':[{'name':'FX-40194B','package':'DIP','pinout':'p16','footprint':'FX16','f"
    "amily':'HC'}]},{'id':'fx-74_195','function':'Shift register','description':'4-bit parall"
    "el-access shift register','drawer':[],'pinouts':{'p16':[{'number':1,'name':'MR','role':'"
    "input'},{'number':2,'name':'J','role':'input'},{'number':3,'name':'K','role':'input'},{'"
    "number':4,'name':'D0','role':'input'},{'number':5,'name':'D1','role':'input'},{'number':"
    "6,'name':'D2','role':'input'},{'number':7,'name':'D3','role':'input'},{'number':8,'name'"
    ":'GND','role':'ground'},{'number':9,'name':'PE','role':'input'},{'number':10,'name':'CP'"
    ",'role':'input'},{'number':11,'name':'Q3BAR','role':'output'},{'number':12,'name':'Q3','"
    "role':'output'},{'number':13,'name':'Q2','role':'output'},{'number':14,'name':'Q1','role"
    "':'output'},{'number':15,'name':'Q0','role':'output'},{'number':16,'name':'VCC','role':'"
    "power'}]},'model':'shift','units':[{'stages':4,'clock':{'pin':'CP','edge':'rising'},'ser"
    "ialJK':{'j':'J','kbar':'K'},'load':{'pin':'PE','active':'low','when':'clock','from':'dat"
    "a'},'data':['D0','D1','D2','D3'],'clear':{'pin':'MR','active':'low','when':'async'},'out"
    "puts':[{'stage':1,'pin':'Q0'},{'stage':2,'pin':'Q1'},{'stage':3,'pin':'Q2'},{'stage':4,'"
    "pin':'Q3'},{'stage':4,'pin':'Q3BAR','invert':true}]}],'timing':{'HC':{'rise':10,'fall':1"
    "0,'vcc':[2,6]}},'parts':[{'name':'FX-74_195','package':'DIP','pinout':'p16','footprint':"
    "'FX16','family':'HC'}]},{'id':'fx-74_299','function':'Shift register','description':'8-b"
    "it universal shift/storage register with 3-state outputs','drawer':[],'pinouts':{'p20':["
    "{'number':1,'name':'S0','role':'input'},{'number':2,'name':'OE1','role':'input'},{'numbe"
    "r':3,'name':'OE2','role':'input'},{'number':4,'name':'G/QG','role':'inout'},{'number':5,"
    "'name':'E/QE','role':'inout'},{'number':6,'name':'C/QC','role':'inout'},{'number':7,'nam"
    "e':'A/QA','role':'inout'},{'number':8,'name':'QA\\u0027','role':'output'},{'number':9,'n"
    "ame':'CLR','role':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name"
    "':'SR','role':'input'},{'number':12,'name':'CLK','role':'input'},{'number':13,'name':'B/"
    "QB','role':'inout'},{'number':14,'name':'D/QD','role':'inout'},{'number':15,'name':'F/QF"
    "','role':'inout'},{'number':16,'name':'H/QH','role':'inout'},{'number':17,'name':'QH\\u0"
    "027','role':'output'},{'number':18,'name':'SL','role':'input'},{'number':19,'name':'S1',"
    "'role':'input'},{'number':20,'name':'VCC','role':'power'}]},'model':'shift','units':[{'s"
    "tages':8,'clock':{'pin':'CLK','edge':'rising'},'modes':{'pins':['S0','S1'],'table':['hol"
    "d','right','left','load']},'serial':['SR'],'leftSerial':'SL','clear':{'pin':'CLR','activ"
    "e':'low','when':'async'},'io':{'pins':['A/QA','B/QB','C/QC','D/QD','E/QE','F/QF','G/QG',"
    "'H/QH'],'outputEnables':[{'pin':'OE1','active':'low'},{'pin':'OE2','active':'low'}],'not"
    "InModes':['load']},'outputs':[{'stage':1,'pin':'QA\\u0027'},{'stage':8,'pin':'QH\\u0027'"
    "}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_299','pac"
    "kage':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_323','functi"
    "on':'Shift register','description':'8-bit universal shift/storage register with synchron"
    "ous reset and 3-state outputs','drawer':[],'pinouts':{'p20':[{'number':1,'name':'S0','ro"
    "le':'input'},{'number':2,'name':'OE1','role':'input'},{'number':3,'name':'OE2','role':'i"
    "nput'},{'number':4,'name':'I/O6','role':'inout'},{'number':5,'name':'I/O4','role':'inout"
    "'},{'number':6,'name':'I/O2','role':'inout'},{'number':7,'name':'I/O0','role':'inout'},{"
    "'number':8,'name':'Q0','role':'output'},{'number':9,'name':'MR','role':'input'},{'number"
    "':10,'name':'GND','role':'ground'},{'number':11,'name':'DS0','role':'input'},{'number':1"
    "2,'name':'CP','role':'input'},{'number':13,'name':'I/O1','role':'inout'},{'number':14,'n"
    "ame':'I/O3','role':'inout'},{'number':15,'name':'I/O5','role':'inout'},{'number':16,'nam"
    "e':'I/O7','role':'inout'},{'number':17,'name':'Q7','role':'output'},{'number':18,'name':"
    "'DS7','role':'input'},{'number':19,'name':'S1','role':'input'},{'number':20,'name':'VCC'"
    ",'role':'power'}]},'model':'shift','units':[{'stages':8,'clock':{'pin':'CP','edge':'risi"
    "ng'},'modes':{'pins':['S0','S1'],'table':['hold','right','left','load']},'serial':['DS0'"
    "],'leftSerial':'DS7','clear':{'pin':'MR','active':'low','when':'clock'},'io':{'pins':['I"
    "/O0','I/O1','I/O2','I/O3','I/O4','I/O5','I/O6','I/O7'],'outputEnables':[{'pin':'OE1','ac"
    "tive':'low'},{'pin':'OE2','active':'low'}],'notInModes':['load']},'outputs':[{'stage':1,"
    "'pin':'Q0'},{'stage':8,'pin':'Q7'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},"
    "'parts':[{'name':'FX-74_323','package':'DIP','pinout':'p20','footprint':'FX20','family':"
    "'HC'}]},{'id':'fx-4014B','function':'Shift register','description':'8-stage static shift"
    " register, synchronous parallel input','drawer':[],'pinouts':{'p16':[{'number':1,'name':"
    "'PI-8','role':'input'},{'number':2,'name':'Q6','role':'output'},{'number':3,'name':'Q8',"
    "'role':'output'},{'number':4,'name':'PI-4','role':'input'},{'number':5,'name':'PI-3','ro"
    "le':'input'},{'number':6,'name':'PI-2','role':'input'},{'number':7,'name':'PI-1','role':"
    "'input'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'P/S','role':'inpu"
    "t'},{'number':10,'name':'CLOCK','role':'input'},{'number':11,'name':'SERIALIN','role':'i"
    "nput'},{'number':12,'name':'Q7','role':'output'},{'number':13,'name':'PI-5','role':'inpu"
    "t'},{'number':14,'name':'PI-6','role':'input'},{'number':15,'name':'PI-7','role':'input'"
    "},{'number':16,'name':'VDD','role':'power'}]},'model':'shift','units':[{'stages':8,'cloc"
    "k':{'pin':'CLOCK','edge':'rising'},'serial':['SERIALIN'],'load':{'pin':'P/S','active':'h"
    "igh','when':'clock','from':'data'},'data':['PI-1','PI-2','PI-3','PI-4','PI-5','PI-6','PI"
    "-7','PI-8'],'outputs':[{'stage':6,'pin':'Q6'},{'stage':7,'pin':'Q7'},{'stage':8,'pin':'Q"
    "8'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4014B','pa"
    "ckage':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4021B','functi"
    "on':'Shift register','description':'8-stage static shift register, asynchronous parallel"
    " input','drawer':[],'pinouts':{'p16':[{'number':1,'name':'PI-8','role':'input'},{'number"
    "':2,'name':'Q6','role':'output'},{'number':3,'name':'Q8','role':'output'},{'number':4,'n"
    "ame':'PI-4','role':'input'},{'number':5,'name':'PI-3','role':'input'},{'number':6,'name'"
    ":'PI-2','role':'input'},{'number':7,'name':'PI-1','role':'input'},{'number':8,'name':'VS"
    "S','role':'ground'},{'number':9,'name':'P/S','role':'input'},{'number':10,'name':'CLOCK'"
    ",'role':'input'},{'number':11,'name':'SERIALIN','role':'input'},{'number':12,'name':'Q7'"
    ",'role':'output'},{'number':13,'name':'PI-5','role':'input'},{'number':14,'name':'PI-6',"
    "'role':'input'},{'number':15,'name':'PI-7','role':'input'},{'number':16,'name':'VDD','ro"
    "le':'power'}]},'model':'shift','units':[{'stages':8,'clock':{'pin':'CLOCK','edge':'risin"
    "g'},'serial':['SERIALIN'],'load':{'pin':'P/S','active':'high','when':'async','from':'dat"
    "a'},'data':['PI-1','PI-2','PI-3','PI-4','PI-5','PI-6','PI-7','PI-8'],'outputs':[{'stage'"
    ":6,'pin':'Q6'},{'stage':7,'pin':'Q7'},{'stage':8,'pin':'Q8'}]}],'timing':{'HC':{'rise':1"
    "0,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4021B','package':'DIP','pinout':'p16','fo"
    "otprint':'FX16','family':'HC'}]},{'id':'fx-4015B','function':'Shift register','descripti"
    "on':'Dual 4-stage static shift register','drawer':[],'pinouts':{'p16':[{'number':1,'name"
    "':'CLOCKB','role':'input'},{'number':2,'name':'Q4B','role':'output'},{'number':3,'name':"
    "'Q3A','role':'output'},{'number':4,'name':'Q2A','role':'output'},{'number':5,'name':'Q1A"
    "','role':'output'},{'number':6,'name':'RESETA','role':'input'},{'number':7,'name':'DATAA"
    "','role':'input'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'CLOCKA',"
    "'role':'input'},{'number':10,'name':'Q4A','role':'output'},{'number':11,'name':'Q3B','ro"
    "le':'output'},{'number':12,'name':'Q2B','role':'output'},{'number':13,'name':'Q1B','role"
    "':'output'},{'number':14,'name':'RESETB','role':'input'},{'number':15,'name':'DATAB','ro"
    "le':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'shift','units':[{'stag"
    "es':4,'clock':{'pin':'CLOCKA','edge':'rising'},'serial':['DATAA'],'clear':{'pin':'RESETA"
    "','active':'high','when':'async'},'outputs':[{'stage':1,'pin':'Q1A'},{'stage':2,'pin':'Q"
    "2A'},{'stage':3,'pin':'Q3A'},{'stage':4,'pin':'Q4A'}]},{'stages':4,'clock':{'pin':'CLOCK"
    "B','edge':'rising'},'serial':['DATAB'],'clear':{'pin':'RESETB','active':'high','when':'a"
    "sync'},'outputs':[{'stage':1,'pin':'Q1B'},{'stage':2,'pin':'Q2B'},{'stage':3,'pin':'Q3B'"
    "},{'stage':4,'pin':'Q4B'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':["
    "{'name':'FX-4015B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'"
    "id':'fx-74_4015','function':'Shift register','description':'Dual 4-stage static shift re"
    "gister','drawer':[],'pinouts':{'p16':[{'number':1,'name':'2CP','role':'input'},{'number'"
    ":2,'name':'2Q3','role':'output'},{'number':3,'name':'1Q2','role':'output'},{'number':4,'"
    "name':'1Q1','role':'output'},{'number':5,'name':'1Q0','role':'output'},{'number':6,'name"
    "':'1MR','role':'input'},{'number':7,'name':'1D','role':'input'},{'number':8,'name':'GND'"
    ",'role':'ground'},{'number':9,'name':'1CP','role':'input'},{'number':10,'name':'1Q3','ro"
    "le':'output'},{'number':11,'name':'2Q2','role':'output'},{'number':12,'name':'2Q1','role"
    "':'output'},{'number':13,'name':'2Q0','role':'output'},{'number':14,'name':'2MR','role':"
    "'input'},{'number':15,'name':'2D','role':'input'},{'number':16,'name':'VCC','role':'powe"
    "r'}]},'model':'shift','units':[{'stages':4,'clock':{'pin':'1CP','edge':'rising'},'serial"
    "':['1D'],'clear':{'pin':'1MR','active':'high','when':'async'},'outputs':[{'stage':1,'pin"
    "':'1Q0'},{'stage':2,'pin':'1Q1'},{'stage':3,'pin':'1Q2'},{'stage':4,'pin':'1Q3'}]},{'sta"
    "ges':4,'clock':{'pin':'2CP','edge':'rising'},'serial':['2D'],'clear':{'pin':'2MR','activ"
    "e':'high','when':'async'},'outputs':[{'stage':1,'pin':'2Q0'},{'stage':2,'pin':'2Q1'},{'s"
    "tage':3,'pin':'2Q2'},{'stage':4,'pin':'2Q3'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc"
    "':[2,6]}},'parts':[{'name':'FX-74_4015','package':'DIP','pinout':'p16','footprint':'FX16"
    "','family':'HC'}]},{'id':'fx-4035B','function':'Shift register','description':'4-stage p"
    "arallel-in/parallel-out shift register with J-K serial inputs and true/complement output"
    "s','drawer':[],'pinouts':{'p16':[{'number':1,'name':'Q1','role':'output'},{'number':2,'n"
    "ame':'T/C','role':'input'},{'number':3,'name':'K','role':'input'},{'number':4,'name':'J'"
    ",'role':'input'},{'number':5,'name':'RESET','role':'input'},{'number':6,'name':'CLOCK','"
    "role':'input'},{'number':7,'name':'P/S','role':'input'},{'number':8,'name':'VSS','role':"
    "'ground'},{'number':9,'name':'PI-1','role':'input'},{'number':10,'name':'PI-2','role':'i"
    "nput'},{'number':11,'name':'PI-3','role':'input'},{'number':12,'name':'PI-4','role':'inp"
    "ut'},{'number':13,'name':'Q4','role':'output'},{'number':14,'name':'Q3','role':'output'}"
    ",{'number':15,'name':'Q2','role':'output'},{'number':16,'name':'VDD','role':'power'}]},'"
    "model':'shift','units':[{'stages':4,'clock':{'pin':'CLOCK','edge':'rising'},'serialJK':{"
    "'j':'J','kbar':'K'},'load':{'pin':'P/S','active':'high','when':'clock','from':'data'},'d"
    "ata':['PI-1','PI-2','PI-3','PI-4'],'clear':{'pin':'RESET','active':'high','when':'async'"
    "},'outputs':[{'stage':1,'pin':'Q1'},{'stage':2,'pin':'Q2'},{'stage':3,'pin':'Q3'},{'stag"
    "e':4,'pin':'Q4'}],'trueComplement':{'pin':'T/C','active':'high'}}],'timing':{'HC':{'rise"
    "':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4035B','package':'DIP','pinout':'p16',"
    "'footprint':'FX16','family':'HC'}]},{'id':'fx-4031B','function':'Shift register','descri"
    "ption':'64-stage static shift register','drawer':[],'pinouts':{'p16':[{'number':1,'name'"
    ":'RECIRC','role':'input'},{'number':2,'name':'CL','role':'input'},{'number':3,'name':'NC"
    "','role':'nc'},{'number':4,'name':'NC','role':'nc'},{'number':5,'name':'Q\\u0027','role'"
    ":'output'},{'number':6,'name':'Q','role':'output'},{'number':7,'name':'QBAR','role':'out"
    "put'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'CLD','role':'output'"
    "},{'number':10,'name':'MODE','role':'input'},{'number':11,'name':'NC','role':'nc'},{'num"
    "ber':12,'name':'NC','role':'nc'},{'number':13,'name':'NC','role':'nc'},{'number':14,'nam"
    "e':'NC','role':'nc'},{'number':15,'name':'DATA','role':'input'},{'number':16,'name':'VDD"
    "','role':'power'}]},'model':'shift','units':[{'stages':64,'clock':{'pin':'CL','edge':'ri"
    "sing'},'serialSelect':{'select':'MODE','low':'DATA','high':'RECIRC'},'outputs':[{'stage'"
    ":64,'pin':'Q'},{'stage':64,'pin':'QBAR','invert':true}],'serialOutputs':[{'pin':'Q\\u002"
    "7','stage':64}],'clockOut':'CLD'}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'pa"
    "rts':[{'name':'FX-4031B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'"
    "}]},{'id':'fx-74_595','function':'Shift register','description':'8-bit shift register wi"
    "th 3-state output registers','drawer':[],'pinouts':{'p16':[{'number':1,'name':'QB','role"
    "':'output'},{'number':2,'name':'QC','role':'output'},{'number':3,'name':'QD','role':'out"
    "put'},{'number':4,'name':'QE','role':'output'},{'number':5,'name':'QF','role':'output'},"
    "{'number':6,'name':'QG','role':'output'},{'number':7,'name':'QH','role':'output'},{'numb"
    "er':8,'name':'GND','role':'ground'},{'number':9,'name':'QH\\u0027','role':'output'},{'nu"
    "mber':10,'name':'SRCLR','role':'input'},{'number':11,'name':'SRCLK','role':'input'},{'nu"
    "mber':12,'name':'RCLK','role':'input'},{'number':13,'name':'OE','role':'input'},{'number"
    "':14,'name':'SER','role':'input'},{'number':15,'name':'QA','role':'output'},{'number':16"
    ",'name':'VCC','role':'power'}]},'model':'shift','units':[{'stages':8,'clock':{'pin':'SRC"
    "LK','edge':'rising'},'serial':['SER'],'clear':{'pin':'SRCLR','active':'low','when':'asyn"
    "c'},'storage':{'clock':{'pin':'RCLK','edge':'rising'},'outputs':['QA','QB','QC','QD','QE"
    "','QF','QG','QH'],'outputEnable':{'pin':'OE','active':'low'}},'outputs':[{'stage':8,'pin"
    "':'QH\\u0027'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX"
    "-74_595','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74"
    "_594','function':'Shift register','description':'8-bit shift register with output regist"
    "er','drawer':[],'pinouts':{'p16':[{'number':1,'name':'QB','role':'output'},{'number':2,'"
    "name':'QC','role':'output'},{'number':3,'name':'QD','role':'output'},{'number':4,'name':"
    "'QE','role':'output'},{'number':5,'name':'QF','role':'output'},{'number':6,'name':'QG','"
    "role':'output'},{'number':7,'name':'QH','role':'output'},{'number':8,'name':'GND','role'"
    ":'ground'},{'number':9,'name':'QH\\u0027','role':'output'},{'number':10,'name':'SRCLR','"
    "role':'input'},{'number':11,'name':'SRCLK','role':'input'},{'number':12,'name':'RCLK','r"
    "ole':'input'},{'number':13,'name':'RCLR','role':'input'},{'number':14,'name':'SER','role"
    "':'input'},{'number':15,'name':'QA','role':'output'},{'number':16,'name':'VCC','role':'p"
    "ower'}]},'model':'shift','units':[{'stages':8,'clock':{'pin':'SRCLK','edge':'rising'},'s"
    "erial':['SER'],'clear':{'pin':'SRCLR','active':'low','when':'async'},'storage':{'clock':"
    "{'pin':'RCLK','edge':'rising'},'clear':{'pin':'RCLR','active':'low'},'outputs':['QA','QB"
    "','QC','QD','QE','QF','QG','QH']},'outputs':[{'stage':8,'pin':'QH\\u0027'}]}],'timing':{"
    "'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_594','package':'DIP','pi"
    "nout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_596','function':'Shift regi"
    "ster','description':'8-bit shift register with open-drain output registers','drawer':[],"
    "'pinouts':{'p16':[{'number':1,'name':'QB','role':'output'},{'number':2,'name':'QC','role"
    "':'output'},{'number':3,'name':'QD','role':'output'},{'number':4,'name':'QE','role':'out"
    "put'},{'number':5,'name':'QF','role':'output'},{'number':6,'name':'QG','role':'output'},"
    "{'number':7,'name':'QH','role':'output'},{'number':8,'name':'GND','role':'ground'},{'num"
    "ber':9,'name':'QH\\u0027','role':'output'},{'number':10,'name':'SRCLR','role':'input'},{"
    "'number':11,'name':'SRCLK','role':'input'},{'number':12,'name':'RCLK','role':'input'},{'"
    "number':13,'name':'OE','role':'input'},{'number':14,'name':'SER','role':'input'},{'numbe"
    "r':15,'name':'QA','role':'output'},{'number':16,'name':'VCC','role':'power'}]},'model':'"
    "shift','units':[{'stages':8,'clock':{'pin':'SRCLK','edge':'rising'},'serial':['SER'],'cl"
    "ear':{'pin':'SRCLR','active':'low','when':'async'},'storage':{'clock':{'pin':'RCLK','edg"
    "e':'rising'},'outputs':['QA','QB','QC','QD','QE','QF','QG','QH'],'outputEnable':{'pin':'"
    "OE','active':'low'},'output':'open-drain'},'outputs':[{'stage':8,'pin':'QH\\u0027'}]}],'"
    "timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_596','package':"
    "'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_597','function':'S"
    "hift register','description':'8-bit shift register with input storage','drawer':[],'pino"
    "uts':{'p16':[{'number':1,'name':'B','role':'input'},{'number':2,'name':'C','role':'input"
    "'},{'number':3,'name':'D','role':'input'},{'number':4,'name':'E','role':'input'},{'numbe"
    "r':5,'name':'F','role':'input'},{'number':6,'name':'G','role':'input'},{'number':7,'name"
    "':'H','role':'input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'QH\\u"
    "0027','role':'output'},{'number':10,'name':'SRCLR','role':'input'},{'number':11,'name':'"
    "SRCK','role':'input'},{'number':12,'name':'RCK','role':'input'},{'number':13,'name':'SRL"
    "OAD','role':'input'},{'number':14,'name':'SER','role':'input'},{'number':15,'name':'A','"
    "role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'shift','units':[{'st"
    "ages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['SER'],'inputRegister':{'clock'"
    ":{'pin':'RCK','edge':'rising'},'data':['A','B','C','D','E','F','G','H']},'load':{'pin':'"
    "SRLOAD','active':'low','when':'async','from':'inputRegister'},'clear':{'pin':'SRCLR','ac"
    "tive':'low','when':'async'},'outputs':[{'stage':8,'pin':'QH\\u0027'}]}],'timing':{'HC':{"
    "'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_597','package':'DIP','pinout':"
    "'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_598','function':'Shift register',"
    "'description':'8-bit shift register with input latches and 3-state I/O','drawer':[],'pin"
    "outs':{'p20':[{'number':1,'name':'A/QA','role':'inout'},{'number':2,'name':'B/QB','role'"
    ":'inout'},{'number':3,'name':'C/QC','role':'inout'},{'number':4,'name':'D/QD','role':'in"
    "out'},{'number':5,'name':'E/QE','role':'inout'},{'number':6,'name':'F/QF','role':'inout'"
    "},{'number':7,'name':'G/QG','role':'inout'},{'number':8,'name':'H/QH','role':'inout'},{'"
    "number':9,'name':'SRLOAD','role':'input'},{'number':10,'name':'GND','role':'ground'},{'n"
    "umber':11,'name':'QH\\u0027','role':'output'},{'number':12,'name':'SRCLR','role':'input'"
    "},{'number':13,'name':'SRCK','role':'input'},{'number':14,'name':'SRCKEN','role':'input'"
    "},{'number':15,'name':'RCK','role':'input'},{'number':16,'name':'G','role':'input'},{'nu"
    "mber':17,'name':'SER1','role':'input'},{'number':18,'name':'SER0','role':'input'},{'numb"
    "er':19,'name':'DS','role':'input'},{'number':20,'name':'VCC','role':'power'}]},'model':'"
    "shift','units':[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serialSelect':{'sele"
    "ct':'DS','low':'SER0','high':'SER1'},'enables':[{'pin':'SRCKEN','active':'low'}],'io':{'"
    "pins':['A/QA','B/QB','C/QC','D/QD','E/QE','F/QF','G/QG','H/QH'],'outputEnables':[{'pin':"
    "'G','active':'low'}]},'inputRegister':{'clock':{'pin':'RCK','edge':'rising'}},'load':{'p"
    "in':'SRLOAD','active':'low','when':'async','from':'inputRegister'},'clear':{'pin':'SRCLR"
    "','active':'low','when':'async'},'outputs':[{'stage':8,'pin':'QH\\u0027'}]}],'timing':{'"
    "HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_598','package':'DIP','pin"
    "out':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-TPIC6595','function':'Shift reg"
    "ister','description':'Power logic 8-bit shift register','drawer':[],'pinouts':{'p20':[{'"
    "number':1,'name':'PGND','role':'ground'},{'number':2,'name':'VCC','role':'power'},{'numb"
    "er':3,'name':'SERIN','role':'input'},{'number':4,'name':'DRAIN0','role':'output'},{'numb"
    "er':5,'name':'DRAIN1','role':'output'},{'number':6,'name':'DRAIN2','role':'output'},{'nu"
    "mber':7,'name':'DRAIN3','role':'output'},{'number':8,'name':'SRCLR','role':'input'},{'nu"
    "mber':9,'name':'G','role':'input'},{'number':10,'name':'PGND','role':'ground'},{'number'"
    ":11,'name':'PGND','role':'ground'},{'number':12,'name':'RCK','role':'input'},{'number':1"
    "3,'name':'SRCK','role':'input'},{'number':14,'name':'DRAIN4','role':'output'},{'number':"
    "15,'name':'DRAIN5','role':'output'},{'number':16,'name':'DRAIN6','role':'output'},{'numb"
    "er':17,'name':'DRAIN7','role':'output'},{'number':18,'name':'SEROUT','role':'output'},{'"
    "number':19,'name':'LGND','role':'ground'},{'number':20,'name':'PGND','role':'ground'}]},"
    "'model':'shift','units':[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['S"
    "ERIN'],'clear':{'pin':'SRCLR','active':'low','when':'async'},'storage':{'clock':{'pin':'"
    "RCK','edge':'rising'},'outputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','"
    "DRAIN6','DRAIN7'],'outputEnable':{'pin':'G','active':'low'},'output':'open-drain','inver"
    "t':true},'outputs':[{'stage':8,'pin':'SEROUT'}]}],'timing':{'HC':{'rise':10,'fall':10,'v"
    "cc':[2,6]}},'parts':[{'name':'FX-TPIC6595','package':'DIP','pinout':'p20','footprint':'F"
    "X20','family':'HC'}]},{'id':'fx-TPIC6A595','function':'Shift register','description':'Po"
    "wer logic 8-bit shift register','drawer':[],'pinouts':{'p20':[{'number':1,'name':'DRAIN2"
    "','role':'output'},{'number':2,'name':'DRAIN3','role':'output'},{'number':3,'name':'SRCL"
    "R','role':'input'},{'number':4,'name':'G','role':'input'},{'number':5,'name':'PGND','rol"
    "e':'ground'},{'number':6,'name':'PGND','role':'ground'},{'number':7,'name':'RCK','role':"
    "'input'},{'number':8,'name':'SRCK','role':'input'},{'number':9,'name':'DRAIN4','role':'o"
    "utput'},{'number':10,'name':'DRAIN5','role':'output'},{'number':11,'name':'DRAIN6','role"
    "':'output'},{'number':12,'name':'DRAIN7','role':'output'},{'number':13,'name':'SEROUT','"
    "role':'output'},{'number':14,'name':'LGND','role':'ground'},{'number':15,'name':'PGND','"
    "role':'ground'},{'number':16,'name':'PGND','role':'ground'},{'number':17,'name':'VCC','r"
    "ole':'power'},{'number':18,'name':'SERIN','role':'input'},{'number':19,'name':'DRAIN0','"
    "role':'output'},{'number':20,'name':'DRAIN1','role':'output'}]},'model':'shift','units':"
    "[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['SERIN'],'clear':{'pin':'S"
    "RCLR','active':'low','when':'async'},'storage':{'clock':{'pin':'RCK','edge':'rising'},'o"
    "utputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','DRAIN6','DRAIN7'],'outpu"
    "tEnable':{'pin':'G','active':'low'},'output':'open-drain','invert':true},'outputs':[{'st"
    "age':8,'pin':'SEROUT'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'n"
    "ame':'FX-TPIC6A595','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{"
    "'id':'fx-TPIC6B595','function':'Shift register','description':'Power logic 8-bit shift r"
    "egister','drawer':[],'pinouts':{'p20':[{'number':1,'name':'NC','role':'nc'},{'number':2,"
    "'name':'VCC','role':'power'},{'number':3,'name':'SERIN','role':'input'},{'number':4,'nam"
    "e':'DRAIN0','role':'output'},{'number':5,'name':'DRAIN1','role':'output'},{'number':6,'n"
    "ame':'DRAIN2','role':'output'},{'number':7,'name':'DRAIN3','role':'output'},{'number':8,"
    "'name':'SRCLR','role':'input'},{'number':9,'name':'G','role':'input'},{'number':10,'name"
    "':'GND','role':'ground'},{'number':11,'name':'GND','role':'ground'},{'number':12,'name':"
    "'RCK','role':'input'},{'number':13,'name':'SRCK','role':'input'},{'number':14,'name':'DR"
    "AIN4','role':'output'},{'number':15,'name':'DRAIN5','role':'output'},{'number':16,'name'"
    ":'DRAIN6','role':'output'},{'number':17,'name':'DRAIN7','role':'output'},{'number':18,'n"
    "ame':'SEROUT','role':'output'},{'number':19,'name':'GND','role':'ground'},{'number':20,'"
    "name':'NC','role':'nc'}]},'model':'shift','units':[{'stages':8,'clock':{'pin':'SRCK','ed"
    "ge':'rising'},'serial':['SERIN'],'clear':{'pin':'SRCLR','active':'low','when':'async'},'"
    "storage':{'clock':{'pin':'RCK','edge':'rising'},'outputs':['DRAIN0','DRAIN1','DRAIN2','D"
    "RAIN3','DRAIN4','DRAIN5','DRAIN6','DRAIN7'],'outputEnable':{'pin':'G','active':'low'},'o"
    "utput':'open-drain','invert':true},'outputs':[{'stage':8,'pin':'SEROUT'}]}],'timing':{'H"
    "C':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-TPIC6B595','package':'DIP','p"
    "inout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-TPIC6C595','function':'Shift "
    "register','description':'Power logic 8-bit shift register','drawer':[],'pinouts':{'p16':"
    "[{'number':1,'name':'VCC','role':'power'},{'number':2,'name':'SERIN','role':'input'},{'n"
    "umber':3,'name':'DRAIN0','role':'output'},{'number':4,'name':'DRAIN1','role':'output'},{"
    "'number':5,'name':'DRAIN2','role':'output'},{'number':6,'name':'DRAIN3','role':'output'}"
    ",{'number':7,'name':'CLR','role':'input'},{'number':8,'name':'G','role':'input'},{'numbe"
    "r':9,'name':'SEROUT','role':'output'},{'number':10,'name':'RCK','role':'input'},{'number"
    "':11,'name':'DRAIN4','role':'output'},{'number':12,'name':'DRAIN5','role':'output'},{'nu"
    "mber':13,'name':'DRAIN6','role':'output'},{'number':14,'name':'DRAIN7','role':'output'},"
    "{'number':15,'name':'SRCK','role':'input'},{'number':16,'name':'GND','role':'ground'}]},"
    "'model':'shift','units':[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['S"
    "ERIN'],'clear':{'pin':'CLR','active':'low','when':'async'},'storage':{'clock':{'pin':'RC"
    "K','edge':'rising'},'outputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','DR"
    "AIN6','DRAIN7'],'outputEnable':{'pin':'G','active':'low'},'output':'open-drain','invert'"
    ":true},'clearScope':'all','outputs':[{'stage':8,'pin':'SEROUT'}]}],'timing':{'HC':{'rise"
    "':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-TPIC6C595','package':'DIP','pinout':'p"
    "16','footprint':'FX16','family':'HC'}]},{'id':'fx-TPIC6A596','function':'Shift register'"
    ",'description':'Power logic 8-bit shift register','drawer':[],'pinouts':{'p20':[{'number"
    "':1,'name':'DRAIN2','role':'output'},{'number':2,'name':'DRAIN3','role':'output'},{'numb"
    "er':3,'name':'SRCLR','role':'input'},{'number':4,'name':'G','role':'input'},{'number':5,"
    "'name':'PGND','role':'ground'},{'number':6,'name':'PGND','role':'ground'},{'number':7,'n"
    "ame':'RCK','role':'input'},{'number':8,'name':'SRCK','role':'input'},{'number':9,'name':"
    "'DRAIN4','role':'output'},{'number':10,'name':'DRAIN5','role':'output'},{'number':11,'na"
    "me':'DRAIN6','role':'output'},{'number':12,'name':'DRAIN7','role':'output'},{'number':13"
    ",'name':'SEROUT','role':'output'},{'number':14,'name':'LGND','role':'ground'},{'number':"
    "15,'name':'PGND','role':'ground'},{'number':16,'name':'PGND','role':'ground'},{'number':"
    "17,'name':'VCC','role':'power'},{'number':18,'name':'SERIN','role':'input'},{'number':19"
    ",'name':'DRAIN0','role':'output'},{'number':20,'name':'DRAIN1','role':'output'}]},'model"
    "':'shift','units':[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['SERIN']"
    ",'clear':{'pin':'SRCLR','active':'low','when':'async'},'storage':{'clock':{'pin':'RCK','"
    "edge':'rising'},'outputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','DRAIN6"
    "','DRAIN7'],'outputEnable':{'pin':'G','active':'low'},'output':'open-drain','invert':tru"
    "e},'clearScope':'all','serialOutputs':[{'pin':'SEROUT','stage':8}]}],'timing':{'HC':{'ri"
    "se':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-TPIC6A596','package':'DIP','pinout':"
    "'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-TPIC6B596','function':'Shift registe"
    "r','description':'Power logic 8-bit shift register','drawer':[],'pinouts':{'p20':[{'numb"
    "er':1,'name':'NC','role':'nc'},{'number':2,'name':'VCC','role':'power'},{'number':3,'nam"
    "e':'SERIN','role':'input'},{'number':4,'name':'DRAIN0','role':'output'},{'number':5,'nam"
    "e':'DRAIN1','role':'output'},{'number':6,'name':'DRAIN2','role':'output'},{'number':7,'n"
    "ame':'DRAIN3','role':'output'},{'number':8,'name':'SRCLR','role':'input'},{'number':9,'n"
    "ame':'G','role':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':"
    "'GND','role':'ground'},{'number':12,'name':'RCK','role':'input'},{'number':13,'name':'SR"
    "CK','role':'input'},{'number':14,'name':'DRAIN4','role':'output'},{'number':15,'name':'D"
    "RAIN5','role':'output'},{'number':16,'name':'DRAIN6','role':'output'},{'number':17,'name"
    "':'DRAIN7','role':'output'},{'number':18,'name':'SEROUT','role':'output'},{'number':19,'"
    "name':'GND','role':'ground'},{'number':20,'name':'NC','role':'nc'}]},'model':'shift','un"
    "its':[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['SERIN'],'clear':{'pi"
    "n':'SRCLR','active':'low','when':'async'},'storage':{'clock':{'pin':'RCK','edge':'rising"
    "'},'outputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','DRAIN6','DRAIN7'],'"
    "outputEnable':{'pin':'G','active':'low'},'output':'open-drain','invert':true},'clearScop"
    "e':'all','serialOutputs':[{'pin':'SEROUT','stage':8}]}],'timing':{'HC':{'rise':10,'fall'"
    ":10,'vcc':[2,6]}},'parts':[{'name':'FX-TPIC6B596','package':'DIP','pinout':'p20','footpr"
    "int':'FX20','family':'HC'}]},{'id':'fx-TPIC6C596','function':'Shift register','descripti"
    "on':'Power logic 8-bit shift register','drawer':[],'pinouts':{'p16':[{'number':1,'name':"
    "'VCC','role':'power'},{'number':2,'name':'SERIN','role':'input'},{'number':3,'name':'DRA"
    "IN0','role':'output'},{'number':4,'name':'DRAIN1','role':'output'},{'number':5,'name':'D"
    "RAIN2','role':'output'},{'number':6,'name':'DRAIN3','role':'output'},{'number':7,'name':"
    "'CLR','role':'input'},{'number':8,'name':'G','role':'input'},{'number':9,'name':'SEROUT'"
    ",'role':'output'},{'number':10,'name':'RCK','role':'input'},{'number':11,'name':'DRAIN4'"
    ",'role':'output'},{'number':12,'name':'DRAIN5','role':'output'},{'number':13,'name':'DRA"
    "IN6','role':'output'},{'number':14,'name':'DRAIN7','role':'output'},{'number':15,'name':"
    "'SRCK','role':'input'},{'number':16,'name':'GND','role':'ground'}]},'model':'shift','uni"
    "ts':[{'stages':8,'clock':{'pin':'SRCK','edge':'rising'},'serial':['SERIN'],'clear':{'pin"
    "':'CLR','active':'low','when':'async'},'storage':{'clock':{'pin':'RCK','edge':'rising'},"
    "'outputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','DRAIN6','DRAIN7'],'out"
    "putEnable':{'pin':'G','active':'low'},'output':'open-drain','invert':true},'clearScope':"
    "'all','serialOutputs':[{'pin':'SEROUT','stage':8}]}],'timing':{'HC':{'rise':10,'fall':10"
    ",'vcc':[2,6]}},'parts':[{'name':'FX-TPIC6C596','package':'DIP','pinout':'p16','footprint"
    "':'FX16','family':'HC'}]},{'id':'fx-4094B','function':'Shift register','description':'8-"
    "stage shift-and-store bus register with 3-state outputs','drawer':[],'pinouts':{'p16':[{"
    "'number':1,'name':'STROBE','role':'input'},{'number':2,'name':'DATA','role':'input'},{'n"
    "umber':3,'name':'CLOCK','role':'input'},{'number':4,'name':'Q1','role':'output'},{'numbe"
    "r':5,'name':'Q2','role':'output'},{'number':6,'name':'Q3','role':'output'},{'number':7,'"
    "name':'Q4','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name'"
    ":'QS','role':'output'},{'number':10,'name':'Q\\u0027S','role':'output'},{'number':11,'na"
    "me':'Q8','role':'output'},{'number':12,'name':'Q7','role':'output'},{'number':13,'name':"
    "'Q6','role':'output'},{'number':14,'name':'Q5','role':'output'},{'number':15,'name':'OE'"
    ",'role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'shift','units':[{'"
    "stages':8,'clock':{'pin':'CLOCK','edge':'rising'},'serial':['DATA'],'storage':{'latch':{"
    "'pin':'STROBE','active':'high'},'outputs':['Q1','Q2','Q3','Q4','Q5','Q6','Q7','Q8'],'out"
    "putEnable':{'pin':'OE','active':'high'}},'outputs':[{'stage':8,'pin':'QS'}],'serialOutpu"
    "ts':[{'pin':'Q\\u0027S','stage':8}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},"
    "'parts':[{'name':'FX-4094B','package':'DIP','pinout':'p16','footprint':'FX16','family':'"
    "HC'}]},{'id':'fx-74_4094','function':'Shift register','description':'8-stage shift-and-s"
    "tore bus register with 3-state outputs','drawer':[],'pinouts':{'p16':[{'number':1,'name'"
    ":'STROBE','role':'input'},{'number':2,'name':'DATA','role':'input'},{'number':3,'name':'"
    "CP','role':'input'},{'number':4,'name':'Q0','role':'output'},{'number':5,'name':'Q1','ro"
    "le':'output'},{'number':6,'name':'Q2','role':'output'},{'number':7,'name':'Q3','role':'o"
    "utput'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'QS1','role':'outpu"
    "t'},{'number':10,'name':'QS2','role':'output'},{'number':11,'name':'Q7','role':'output'}"
    ",{'number':12,'name':'Q6','role':'output'},{'number':13,'name':'Q5','role':'output'},{'n"
    "umber':14,'name':'Q4','role':'output'},{'number':15,'name':'OE','role':'input'},{'number"
    "':16,'name':'VCC','role':'power'}]},'model':'shift','units':[{'stages':8,'clock':{'pin':"
    "'CP','edge':'rising'},'serial':['DATA'],'storage':{'latch':{'pin':'STROBE','active':'hig"
    "h'},'outputs':['Q0','Q1','Q2','Q3','Q4','Q5','Q6','Q7'],'outputEnable':{'pin':'OE','acti"
    "ve':'high'}},'outputs':[{'stage':8,'pin':'QS1'}],'serialOutputs':[{'pin':'QS2','stage':8"
    "}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_4094','pa"
    "ckage':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_673','funct"
    "ion':'Shift register','description':'16-bit serial-in, serial-out shift register with 16"
    "-bit parallel-out storage register','drawer':[],'pinouts':{'p24':[{'number':1,'name':'CS"
    "','role':'input'},{'number':2,'name':'SHCLK','role':'input'},{'number':3,'name':'R/W','r"
    "ole':'input'},{'number':4,'name':'STRCLR','role':'input'},{'number':5,'name':'MODE/STRCL"
    "K','role':'input'},{'number':6,'name':'SER/Q15','role':'inout'},{'number':7,'name':'Y0',"
    "'role':'output'},{'number':8,'name':'Y1','role':'output'},{'number':9,'name':'Y2','role'"
    ":'output'},{'number':10,'name':'Y3','role':'output'},{'number':11,'name':'Y4','role':'ou"
    "tput'},{'number':12,'name':'GND','role':'ground'},{'number':13,'name':'Y5','role':'outpu"
    "t'},{'number':14,'name':'Y6','role':'output'},{'number':15,'name':'Y7','role':'output'},"
    "{'number':16,'name':'Y8','role':'output'},{'number':17,'name':'Y9','role':'output'},{'nu"
    "mber':18,'name':'Y10','role':'output'},{'number':19,'name':'Y11','role':'output'},{'numb"
    "er':20,'name':'Y12','role':'output'},{'number':21,'name':'Y13','role':'output'},{'number"
    "':22,'name':'Y14','role':'output'},{'number':23,'name':'Y15','role':'output'},{'number':"
    "24,'name':'VCC','role':'power'}]},'model':'ls673','clock':'SHCLK','chipSelect':'CS','rea"
    "dWrite':'R/W','mode':'MODE/STRCLK','serialIo':'SER/Q15','storageOutputs':['Y0','Y1','Y2'"
    ",'Y3','Y4','Y5','Y6','Y7','Y8','Y9','Y10','Y11','Y12','Y13','Y14','Y15'],'storageClear':"
    "{'pin':'STRCLR','active':'low'},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts"
    "':[{'name':'FX-74_673','package':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]"
    "},{'id':'fx-74_674','function':'Shift register','description':'16-bit parallel-in, seria"
    "l-out shift register','drawer':[],'pinouts':{'p24':[{'number':1,'name':'CS','role':'inpu"
    "t'},{'number':2,'name':'CLK','role':'input'},{'number':3,'name':'R/W','role':'input'},{'"
    "number':4,'name':'NC','role':'nc'},{'number':5,'name':'MODE','role':'input'},{'number':6"
    ",'name':'SER/Q15','role':'inout'},{'number':7,'name':'P0','role':'input'},{'number':8,'n"
    "ame':'P1','role':'input'},{'number':9,'name':'P2','role':'input'},{'number':10,'name':'P"
    "3','role':'input'},{'number':11,'name':'P4','role':'input'},{'number':12,'name':'GND','r"
    "ole':'ground'},{'number':13,'name':'P5','role':'input'},{'number':14,'name':'P6','role':"
    "'input'},{'number':15,'name':'P7','role':'input'},{'number':16,'name':'P8','role':'input"
    "'},{'number':17,'name':'P9','role':'input'},{'number':18,'name':'P10','role':'input'},{'"
    "number':19,'name':'P11','role':'input'},{'number':20,'name':'P12','role':'input'},{'numb"
    "er':21,'name':'P13','role':'input'},{'number':22,'name':'P14','role':'input'},{'number':"
    "23,'name':'P15','role':'input'},{'number':24,'name':'VCC','role':'power'}]},'model':'ls6"
    "73','clock':'CLK','chipSelect':'CS','readWrite':'R/W','mode':'MODE','serialIo':'SER/Q15'"
    ",'parallel':['P0','P1','P2','P3','P4','P5','P6','P7','P8','P9','P10','P11','P12','P13','"
    "P14','P15'],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_67"
    "4','package':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]},{'id':'fx-4517B','"
    "function':'Shift register','description':'Dual 64-stage static shift register','drawer':"
    "[],'pinouts':{'p16':[{'number':1,'name':'Q16A','role':'inout'},{'number':2,'name':'Q48A'"
    ",'role':'inout'},{'number':3,'name':'WEA','role':'input'},{'number':4,'name':'CLA','role"
    "':'input'},{'number':5,'name':'Q64A','role':'output'},{'number':6,'name':'Q32A','role':'"
    "inout'},{'number':7,'name':'DA','role':'input'},{'number':8,'name':'VSS','role':'ground'"
    "},{'number':9,'name':'DB','role':'input'},{'number':10,'name':'Q32B','role':'inout'},{'n"
    "umber':11,'name':'Q64B','role':'output'},{'number':12,'name':'CLB','role':'input'},{'num"
    "ber':13,'name':'WEB','role':'input'},{'number':14,'name':'Q48B','role':'inout'},{'number"
    "':15,'name':'Q16B','role':'inout'},{'number':16,'name':'VDD','role':'power'}]},'model':'"
    "tap64','units':[{'clock':'CLA','data':'DA','writeEnable':'WEA','taps':['Q16A','Q32A','Q4"
    "8A','Q64A']},{'clock':'CLB','data':'DB','writeEnable':'WEB','taps':['Q16B','Q32B','Q48B'"
    ",'Q64B']}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4517B'"
    ",'package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4034B','fu"
    "nction':'Shift register','description':'8-stage static bidirectional parallel/serial inp"
    "ut/output bus register','drawer':[],'pinouts':{'p24':[{'number':1,'name':'B8','role':'in"
    "out'},{'number':2,'name':'B7','role':'inout'},{'number':3,'name':'B6','role':'inout'},{'"
    "number':4,'name':'B5','role':'inout'},{'number':5,'name':'B4','role':'inout'},{'number':"
    "6,'name':'B3','role':'inout'},{'number':7,'name':'B2','role':'inout'},{'number':8,'name'"
    ":'B1','role':'inout'},{'number':9,'name':'AE','role':'input'},{'number':10,'name':'SI','"
    "role':'input'},{'number':11,'name':'A/B','role':'input'},{'number':12,'name':'VSS','role"
    "':'ground'},{'number':13,'name':'P/S','role':'input'},{'number':14,'name':'A/S','role':'"
    "input'},{'number':15,'name':'CLOCK','role':'input'},{'number':16,'name':'A1','role':'ino"
    "ut'},{'number':17,'name':'A2','role':'inout'},{'number':18,'name':'A3','role':'inout'},{"
    "'number':19,'name':'A4','role':'inout'},{'number':20,'name':'A5','role':'inout'},{'numbe"
    "r':21,'name':'A6','role':'inout'},{'number':22,'name':'A7','role':'inout'},{'number':23,"
    "'name':'A8','role':'inout'},{'number':24,'name':'VDD','role':'power'}]},'model':'bus-reg"
    "ister','a':['A1','A2','A3','A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B5','B6',"
    "'B7','B8'],'clock':'CLOCK','aEnable':'AE','direction':'A/B','parallelSerial':'P/S','asyn"
    "cSync':'A/S','serialIn':'SI','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':["
    "{'name':'FX-4034B','package':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]},{'"
    "id':'fx-40105B','function':'Shift register','description':'FIFO register, 4 bits x 16 wo"
    "rds','drawer':[],'pinouts':{'p16':[{'number':1,'name':'OE','role':'input'},{'number':2,'"
    "name':'DIR','role':'output'},{'number':3,'name':'SI','role':'input'},{'number':4,'name':"
    "'D0','role':'input'},{'number':5,'name':'D1','role':'input'},{'number':6,'name':'D2','ro"
    "le':'input'},{'number':7,'name':'D3','role':'input'},{'number':8,'name':'VSS','role':'gr"
    "ound'},{'number':9,'name':'MR','role':'input'},{'number':10,'name':'Q3','role':'output'}"
    ",{'number':11,'name':'Q2','role':'output'},{'number':12,'name':'Q1','role':'output'},{'n"
    "umber':13,'name':'Q0','role':'output'},{'number':14,'name':'DOR','role':'output'},{'numb"
    "er':15,'name':'SO','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'"
    "fifo','data':['D0','D1','D2','D3'],'outputs':['Q0','Q1','Q2','Q3'],'shiftIn':'SI','shift"
    "Out':'SO','inputReady':'DIR','outputReady':'DOR','reset':{'pin':'MR','active':'high'},'o"
    "utputEnable':{'pin':'OE','active':'low'},'words':16,'unloadNeedsEnable':true,'loadWhileH"
    "igh':false,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'fallThrough':1500}},'parts':"
    "[{'name':'FX-40105B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},"
    "{'id':'fx-74_40105','function':'Shift register','description':'4-bit x 16-word FIFO regi"
    "ster','drawer':[],'pinouts':{'p16':[{'number':1,'name':'OE','role':'input'},{'number':2,"
    "'name':'DIR','role':'output'},{'number':3,'name':'SI','role':'input'},{'number':4,'name'"
    ":'D0','role':'input'},{'number':5,'name':'D1','role':'input'},{'number':6,'name':'D2','r"
    "ole':'input'},{'number':7,'name':'D3','role':'input'},{'number':8,'name':'GND','role':'g"
    "round'},{'number':9,'name':'MR','role':'input'},{'number':10,'name':'Q3','role':'output'"
    "},{'number':11,'name':'Q2','role':'output'},{'number':12,'name':'Q1','role':'output'},{'"
    "number':13,'name':'Q0','role':'output'},{'number':14,'name':'DOR','role':'output'},{'num"
    "ber':15,'name':'SO','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':"
    "'fifo','data':['D0','D1','D2','D3'],'outputs':['Q0','Q1','Q2','Q3'],'shiftIn':'SI','shif"
    "tOut':'SO','inputReady':'DIR','outputReady':'DOR','reset':{'pin':'MR','active':'high'},'"
    "outputEnable':{'pin':'OE','active':'low'},'words':16,'unloadNeedsEnable':false,'loadWhil"
    "eHigh':true,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'fallThrough':1500}},'parts'"
    ":[{'name':'FX-74_40105','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}"
    "]},{'id':'fx-74_8153','function':'Shift register','description':'Serial-to-parallel inte"
    "rface','drawer':[],'pinouts':{'p20':[{'number':1,'name':'VCC1','role':'power'},{'number'"
    ":2,'name':'A0','role':'input'},{'number':3,'name':'A1','role':'input'},{'number':4,'name"
    "':'A2','role':'input'},{'number':5,'name':'D','role':'input'},{'number':6,'name':'OUTSEL"
    "','role':'input'},{'number':7,'name':'RESET','role':'input'},{'number':8,'name':'OE','ro"
    "le':'input'},{'number':9,'name':'SOUT','role':'output'},{'number':10,'name':'GND','role'"
    ":'ground'},{'number':11,'name':'GND','role':'ground'},{'number':12,'name':'Y7','role':'o"
    "utput'},{'number':13,'name':'Y6','role':'output'},{'number':14,'name':'Y5','role':'outpu"
    "t'},{'number':15,'name':'Y4','role':'output'},{'number':16,'name':'Y3','role':'output'},"
    "{'number':17,'name':'Y2','role':'output'},{'number':18,'name':'Y1','role':'output'},{'nu"
    "mber':19,'name':'Y0','role':'output'},{'number':20,'name':'VCC2','role':'power'}]},'mode"
    "l':'serial-in','data':'D','address':['A0','A1','A2'],'outputs':['Y0','Y1','Y2','Y3','Y4'"
    ",'Y5','Y6','Y7'],'outputSelect':'OUTSEL','reset':{'pin':'RESET','active':'low'},'outputE"
    "nable':{'pin':'OE','active':'low'},'serialOut':'SOUT','timing':{'HC':{'rise':10,'fall':1"
    "0,'vcc':[2,6]}},'parts':[{'name':'FX-74_8153','package':'DIP','pinout':'p20','footprint'"
    ":'FX20','family':'HC'}]}]}";

/* Dual-row fixture footprints FX<n>, one per pin count above. */
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[{'id':'FX14','description':"
    "'d','style':'dual','pins':14,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'wi"
    "dth':0.5,'height':0.5,'shape':'rect'}},{'id':'FX16','description':'d','style':'dual','pi"
    "ns':16,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,"
    "'shape':'rect'}},{'id':'FX20','description':'d','style':'dual','pins':20,'body':{'width'"
    ":1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id'"
    ":'FX24','description':'d','style':'dual','pins':24,'body':{'width':1,'height':1},'pitch'"
    ":1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}]}";

static char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

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

static const LogicValue L0 = LOGIC_0, L1 = LOGIC_1, LZ = LOGIC_Z;

static const Part *fx_part(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = part_library_find(library, name);
    if (part == NULL || part->kind != PART_SHIFT) {
        fprintf(stderr, "no shift part %s\n", name);
        exit(1);
    }
    return part;
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

static void pulse(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
    set(b, pin, L0);
    step(b);
}

static LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

static void set_pins(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], bit(value, i));
    }
}

static void release_pins(Bench *b, const char *const *names, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, names[i], LZ);
    }
}

static int pins_value(Bench *b, const char *const *names, unsigned n, unsigned value) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got = bench_get(b, names[i]);

        if (got != bit(value, i)) {
            printf("  %s = %c, want %c (value 0x%X)\n", names[i], "01XZ"[got],
                   "01XZ"[bit(value, i)], value);
            ok = 0;
        }
    }
    return ok;
}

static int pins_all(Bench *b, const char *const *names, unsigned n, LogicValue v) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got = bench_get(b, names[i]);

        if (got != v) {
            printf("  %s = %c, want %c\n", names[i], "01XZ"[got], "01XZ"[v]);
            ok = 0;
        }
    }
    return ok;
}

static int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

/* Shifts the bits of value in, LSB first, through pin on clock pulses. */
static void shift_in(Bench *b, const char *serial, const char *clock, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, serial, bit(value, i));
        step(b);
        pulse(b, clock);
    }
}

static const char *const Q8[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
static const char *const D8[] = {"A", "B", "C", "D", "E", "F", "G", "H"};

/* After shifting bits b0..b(n-1) in (b0 first), stage k holds b(n-1-k). */
static unsigned reversed(unsigned value, unsigned n) {
    unsigned r = 0, i;

    for (i = 0; i < n; i++) {
        r |= ((value >> i) & 1u) << (n - 1 - i);
    }
    return r;
}

/* ---- 164 / 264 / 165 / 166 ------------------------------------------------- */

static void check_164(void) {
    Bench *b = open_fx("74_164");
    int ok;

    set(b, "CLR", L1);
    set(b, "B", L1);
    set(b, "CLK", L0);
    step(b);
    shift_in(b, "A", "CLK", 8, 0xB5);
    ok = pins_value(b, Q8, 8, reversed(0xB5, 8));
    set(b, "A", L1);
    set(b, "B", L0);
    step(b);
    pulse(b, "CLK");   /* A AND B = 0 */
    ok &= pins_value(b, Q8, 8, (reversed(0xB5, 8) << 1) & 0xFF);
    set(b, "CLR", L0);
    step(b);
    expect(ok && pins_value(b, Q8, 8, 0), "1 74_164 shifts A AND B in on CLK rising; CLR low async clears");
    bench_destroy(b);
}

static void check_264(void) {
    Bench *b = open_fx("74_264");
    int ok;

    set(b, "CLR", L1);
    set(b, "B", L1);
    set(b, "CLK", L0);
    step(b);
    shift_in(b, "A", "CLK", 8, 0x0F);
    ok = pins_value(b, Q8, 8, ~reversed(0x0F, 8) & 0xFF);
    set(b, "CLR", L0);
    step(b);
    expect(ok && pins_all(b, Q8, 8, L1), "2 74_264 outputs inverted; after clear all 1");
    bench_destroy(b);
}

static void check_165(void) {
    Bench *b = open_fx("74_165");
    int ok;

    set(b, "CLKINH", L0);
    set(b, "CLK", L0);
    set(b, "SER", L0);
    set_pins(b, D8, 8, 0x80);   /* H = 1 */
    set(b, "SH/LD", L0);
    step(b);
    ok = is(b, "QH", L1) && is(b, "QHBAR", L0);
    set(b, "H", L0);   /* transparent while SH/LD is low */
    step(b);
    ok &= is(b, "QH", L0) && is(b, "QHBAR", L1);
    set(b, "G", L1);   /* data 0x40: G = 1, H = 0 */
    step(b);
    set(b, "SH/LD", L1);
    step(b);
    ok &= is(b, "QH", L0);
    set(b, "CLKINH", L1);   /* CLK INH rising with CLK low: one shift, G -> H */
    step(b);
    ok &= is(b, "QH", L1);
    pulse(b, "CLK");   /* CLK INH high blocks (F = 0 would move in) */
    expect(ok && is(b, "QH", L1),
           "3 74_165 SH/LD low loads and is transparent; CLK INH rising with CLK low shifts; CLK INH "
           "high blocks");
    bench_destroy(b);
}

static void check_166(void) {
    Bench *b = open_fx("74_166");
    int ok;

    set(b, "CLR", L1);
    set(b, "CLKINH", L0);
    set(b, "CLK", L0);
    set(b, "SER", L0);
    set_pins(b, D8, 8, 0x80);
    set(b, "SH/LD", L0);
    step(b);
    ok = is(b, "QH", L0);   /* synchronous: nothing yet */
    pulse(b, "CLK");
    ok &= is(b, "QH", L1);
    set(b, "CLR", L0);
    step(b);
    expect(ok && is(b, "QH", L0), "4 74_166 SH/LD low loads at the edge; CLR async");
    bench_destroy(b);
}

/* ---- 194 / 195 --------------------------------------------------------------- */

static const char *const QAD[] = {"QA", "QB", "QC", "QD"};

static void s194(Bench *b, int s0, int s1) {
    set(b, "S0", s0 ? L1 : L0);
    set(b, "S1", s1 ? L1 : L0);
    step(b);
}

static void check_194(void) {
    Bench *b = open_fx("74_194");
    int ok;

    set(b, "CLR", L1);
    set(b, "CLK", L0);
    set(b, "SRSER", L1);
    set(b, "SLSER", L0);
    set_pins(b, D8, 4, 0x6);
    s194(b, 1, 1);
    pulse(b, "CLK");
    ok = pins_value(b, QAD, 4, 0x6);   /* load */
    s194(b, 0, 0);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0x6);   /* hold */
    s194(b, 1, 0);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0xD);   /* right: QA <- SR (1), QB <- QA ... */
    s194(b, 0, 1);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0x6), "5 74_194 each S1 S0 row: hold, right, left, load");
    bench_destroy(b);
}

static void check_195(void) {
    static const char *const Q[] = {"Q0", "Q1", "Q2", "Q3"};
    static const char *const D[] = {"D0", "D1", "D2", "D3"};
    Bench *b = open_fx("74_195");
    int ok;

    set(b, "MR", L1);
    set(b, "PE", L1);
    set(b, "CP", L0);
    set(b, "J", L1);
    set(b, "K", L1);
    step(b);
    pulse(b, "CP");   /* J = 1, K' = 1: set */
    ok = is(b, "Q0", L1);
    set(b, "J", L0);
    set(b, "K", L0);
    step(b);
    pulse(b, "CP");   /* reset */
    ok &= is(b, "Q0", L0) && is(b, "Q1", L1);
    set(b, "J", L1);
    step(b);
    pulse(b, "CP");   /* J = 1, K' = 0: toggle (0 -> 1) */
    ok &= is(b, "Q0", L1);
    pulse(b, "CP");   /* toggle (1 -> 0) */
    ok &= is(b, "Q0", L0);
    set(b, "J", L0);
    set(b, "K", L1);
    step(b);
    pulse(b, "CP");   /* keep */
    ok &= is(b, "Q0", L0);
    set_pins(b, D, 4, 0x9);
    set(b, "PE", L0);
    step(b);
    pulse(b, "CP");
    expect(ok && pins_value(b, Q, 4, 0x9) && is(b, "Q3BAR", L0),
           "6 74_195 J/K' rows: set, reset, toggle, keep; PE low loads");
    bench_destroy(b);
}

/* ---- 299 / 323 ------------------------------------------------------------------ */

static const char *const IO299[] = {"A/QA", "B/QB", "C/QC", "D/QD", "E/QE", "F/QF", "G/QG",
                                    "H/QH"};

static void check_299(void) {
    Bench *b = open_fx("74_299");
    int ok;

    set(b, "CLR", L1);
    set(b, "CLK", L0);
    set(b, "SR", L1);
    set(b, "SL", L0);
    set(b, "OE1", L1);
    set(b, "OE2", L0);
    set(b, "S0", L1);
    set(b, "S1", L1);   /* load from the pins */
    set_pins(b, IO299, 8, 0x3C);
    step(b);
    pulse(b, "CLK");
    ok = is(b, "QA'", L0) && is(b, "QH'", L0);
    release_pins(b, IO299, 8);
    set(b, "S1", L0);   /* right shift; outputs still off (OE1 high) */
    step(b);
    ok &= pins_all(b, IO299, 8, LZ);
    set(b, "OE1", L0);
    step(b);
    ok &= pins_value(b, IO299, 8, 0x3C);
    pulse(b, "CLK");
    ok &= pins_value(b, IO299, 8, 0x79) && is(b, "QA'", L1);
    set(b, "S1", L1);   /* both S high: load mode, the pins are inputs */
    step(b);
    ok &= pins_all(b, IO299, 8, LZ) && is(b, "QH'", L0);
    expect(ok && bench_error(b) == NULL,
           "7 74_299 I/O pins driven only with OE1 = OE2 = 0 and not both S high; load reads the "
           "pins; QA'/QH' always driven");
    bench_destroy(b);
}

static void check_323(void) {
    static const char *const IO[] = {"I/O0", "I/O1", "I/O2", "I/O3", "I/O4", "I/O5", "I/O6",
                                     "I/O7"};
    Bench *b = open_fx("74_323");
    int ok;

    set(b, "MR", L1);
    set(b, "CP", L0);
    set(b, "OE1", L1);
    set(b, "OE2", L1);
    set(b, "S0", L1);
    set(b, "S1", L1);
    set_pins(b, IO, 8, 0x81);
    step(b);
    pulse(b, "CP");
    release_pins(b, IO, 8);
    set(b, "S1", L0);
    set(b, "S0", L0);   /* hold */
    step(b);
    set(b, "MR", L0);
    step(b);
    ok = is(b, "Q0", L1) && is(b, "Q7", L1);   /* MR low but no edge yet */
    pulse(b, "CP");
    expect(ok && is(b, "Q0", L0) && is(b, "Q7", L0), "8 74_323 MR clears only at the edge");
    bench_destroy(b);
}

/* ---- CD4000 ------------------------------------------------------------------------ */

static const char *const PI8[] = {"PI-1", "PI-2", "PI-3", "PI-4", "PI-5", "PI-6", "PI-7", "PI-8"};
static const char *const Q678[] = {"Q6", "Q7", "Q8"};

static void check_4014(void) {
    Bench *b = open_fx("4014B");
    Bench *c = open_fx("4021B");
    int ok;

    set(b, "CLOCK", L0);
    set(b, "SERIALIN", L0);
    set_pins(b, PI8, 8, 0xE0);
    set(b, "P/S", L1);
    step(b);
    ok = pins_value(b, Q678, 3, 0);   /* 4014B: synchronous */
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q678, 3, 7);
    set(c, "CLOCK", L0);
    set(c, "SERIALIN", L0);
    set_pins(c, PI8, 8, 0xE0);
    set(c, "P/S", L1);
    step(c);
    ok &= pins_value(c, Q678, 3, 7);   /* 4021B: asynchronous */
    set(c, "P/S", L0);
    step(c);
    pulse(c, "CLOCK");
    expect(ok && pins_value(c, Q678, 3, 6), "9 4014B / 4021B synchronous vs asynchronous parallel load");
    bench_destroy(b);
    bench_destroy(c);
}

static void check_4015(void) {
    static const char *const QA4[] = {"Q1A", "Q2A", "Q3A", "Q4A"};
    static const char *const QB4[] = {"Q1B", "Q2B", "Q3B", "Q4B"};
    Bench *b = open_fx("4015B");
    int ok;

    set(b, "RESETA", L0);
    set(b, "RESETB", L0);
    set(b, "CLOCKA", L0);
    set(b, "CLOCKB", L0);
    set(b, "DATAB", L0);
    step(b);
    shift_in(b, "DATAA", "CLOCKA", 4, 0xA);
    ok = pins_value(b, QA4, 4, reversed(0xA, 4)) && pins_value(b, QB4, 4, 0);
    set(b, "RESETA", L1);
    step(b);
    expect(ok && pins_value(b, QA4, 4, 0), "10 4015B two independent units; RESET high clears");
    bench_destroy(b);
}

static void check_4035(void) {
    static const char *const Q[] = {"Q1", "Q2", "Q3", "Q4"};
    static const char *const PI[] = {"PI-1", "PI-2", "PI-3", "PI-4"};
    Bench *b = open_fx("4035B");
    int ok;

    set(b, "RESET", L0);
    set(b, "CLOCK", L0);
    set(b, "J", L0);
    set(b, "K", L0);
    set(b, "T/C", L1);
    set_pins(b, PI, 4, 0x5);
    set(b, "P/S", L1);
    step(b);
    pulse(b, "CLOCK");
    ok = pins_value(b, Q, 4, 0x5);
    set(b, "T/C", L0);
    step(b);
    expect(ok && pins_value(b, Q, 4, 0xA), "11 4035B T/C low inverts the outputs at once");
    bench_destroy(b);
}

static void check_4031(void) {
    Bench *b = open_fx("4031B");
    int ok = 1;
    unsigned k;

    set(b, "MODE", L0);
    set(b, "CL", L0);
    set(b, "RECIRC", L0);
    set(b, "DATA", L1);
    step(b);
    pulse(b, "CL");   /* a single 1 enters stage 1 */
    set(b, "DATA", L0);
    step(b);
    for (k = 2; k <= 64; k++) {
        set(b, "CL", L1);
        step(b);
        ok &= is(b, "CLD", L1) && is(b, "Q", k == 64 ? L1 : L0) && is(b, "Q'", L0);
        set(b, "CL", L0);
        step(b);
        ok &= is(b, "CLD", L0);
    }
    ok &= is(b, "Q", L1) && is(b, "QBAR", L0) && is(b, "Q'", L1);   /* Q' half a clock later */
    set(b, "MODE", L1);   /* recirculate: Q -> stage 1 */
    set(b, "RECIRC", L0);
    step(b);
    for (k = 1; k <= 64; k++) {
        set(b, "RECIRC", bench_get(b, "Q"));
        step(b);
        pulse(b, "CL");
    }
    expect(ok && is(b, "Q", L1),
           "12 4031B 64-clock delay to Q; Q' half a clock later; CLD follows CL; MODE 1 "
           "recirculates");
    bench_destroy(b);
}

/* ---- 595 / 594 / 596 / 597 / 598 ------------------------------------------------- */

static void check_595(void) {
    static const char *const JOIN[] = {"SRCLK", "RCLK", NULL};
    Bench *b = open_fx("74_595");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "OE", L0);
    set(b, "SRCLK", L0);
    set(b, "RCLK", L0);
    step(b);
    shift_in(b, "SER", "SRCLK", 8, 0x2D);
    ok = pins_value(b, Q8, 8, 0) && is(b, "QH'", bit(0x2D, 0));
    pulse(b, "RCLK");
    ok &= pins_value(b, Q8, 8, reversed(0x2D, 8));
    set(b, "OE", L1);
    step(b);
    ok &= pins_all(b, Q8, 8, LZ) && is(b, "QH'", bit(0x2D, 0));
    set(b, "OE", L0);
    set(b, "SRCLR", L0);
    step(b);
    ok &= pins_value(b, Q8, 8, reversed(0x2D, 8)) && is(b, "QH'", L0);
    expect(ok, "13 74_595 shift and storage separate; OE high -> Z, QH' still driven; SRCLR clears "
               "the shift register only");
    bench_destroy(b);

    b = bench_create_joined(fx_part("74_595"), JOIN);
    set(b, "SRCLR", L1);
    set(b, "OE", L0);
    set(b, "SRCLK", L0);
    set(b, "SER", L1);
    step(b);
    pulse(b, "SRCLK");
    ok = pins_value(b, Q8, 8, 0x00);   /* storage took the stages before the shift */
    pulse(b, "SRCLK");
    expect(ok && pins_value(b, Q8, 8, 0x01), "14 74_595 SRCLK and RCLK tied: storage one clock behind");
    bench_destroy(b);
}

static void check_594(void) {
    Bench *b = open_fx("74_594");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "RCLR", L1);
    set(b, "SRCLK", L0);
    set(b, "RCLK", L0);
    step(b);
    shift_in(b, "SER", "SRCLK", 8, 0xFF);
    pulse(b, "RCLK");
    ok = pins_value(b, Q8, 8, 0xFF);
    set(b, "RCLR", L0);
    step(b);
    ok &= pins_value(b, Q8, 8, 0) && is(b, "QH'", L1);
    expect(ok, "15 74_594 RCLR clears the storage only");
    bench_destroy(b);
}

static void check_596(void) {
    Bench *b = open_fx("74_596");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "OE", L0);
    set(b, "SRCLK", L0);
    set(b, "RCLK", L0);
    step(b);
    shift_in(b, "SER", "SRCLK", 8, 0x0F);
    pulse(b, "RCLK");
    {
        unsigned v = reversed(0x0F, 8);
        unsigned i;

        ok = 1;
        for (i = 0; i < 8; i++) {
            ok &= is(b, Q8[i], (v >> i) & 1u ? LZ : L0);
        }
    }
    expect(ok, "16 74_596 storage bit 0 -> output 0, bit 1 -> Z (open drain)");
    bench_destroy(b);
}

static void check_597(void) {
    Bench *b = open_fx("74_597");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "SRLOAD", L1);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    set(b, "SER", L0);
    set_pins(b, D8, 8, 0x80);
    step(b);
    ok = is(b, "QH'", L0);
    pulse(b, "RCK");
    ok &= is(b, "QH'", L0);   /* only the input register */
    set(b, "SRLOAD", L0);
    step(b);
    ok &= is(b, "QH'", L1);
    set(b, "SRCLR", L0);   /* clear beats load */
    step(b);
    expect(ok && is(b, "QH'", L0),
           "17 74_597 RCK rising loads the input register; SRLOAD low copies it; clear beats load");
    bench_destroy(b);
}

static const char *const IO598[] = {"A/QA", "B/QB", "C/QC", "D/QD", "E/QE", "F/QF", "G/QG",
                                    "H/QH"};

static void check_598(void) {
    Bench *b = open_fx("74_598");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "SRLOAD", L1);
    set(b, "SRCKEN", L0);
    set(b, "G", L1);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    set(b, "SER0", L0);
    set(b, "SER1", L1);
    set(b, "DS", L1);
    step(b);
    pulse(b, "SRCK");   /* DS high: SER1 (1) */
    set(b, "DS", L0);
    step(b);
    pulse(b, "SRCK");   /* DS low: SER0 (0) */
    set(b, "SRCKEN", L1);
    step(b);
    pulse(b, "SRCK");   /* blocked */
    set(b, "G", L0);
    step(b);
    ok = pins_value(b, IO598, 8, 0x02);
    expect(ok && bench_error(b) == NULL,
           "18 74_598 DS selects SER0/SER1; SRCKEN high blocks shifting; G low drives the stages "
           "onto the pins");
    bench_destroy(b);
}

/* ---- TPIC ------------------------------------------------------------------------ */

static const char *const DRAIN[] = {"DRAIN0", "DRAIN1", "DRAIN2", "DRAIN3",
                                    "DRAIN4", "DRAIN5", "DRAIN6", "DRAIN7"};

/* DRAINn: 0 where the bit of value is 1, else Z. */
static int drains(Bench *b, unsigned value) {
    unsigned i;
    int ok = 1;

    for (i = 0; i < 8; i++) {
        ok &= is(b, DRAIN[i], (value >> i) & 1u ? L0 : LZ);
    }
    return ok;
}

static void check_tpic6b595(void) {
    Bench *b = open_fx("TPIC6B595");
    int ok;

    set(b, "SRCLR", L1);
    set(b, "G", L0);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    step(b);
    shift_in(b, "SERIN", "SRCK", 8, 0x96);
    pulse(b, "RCK");
    ok = drains(b, reversed(0x96, 8));
    set(b, "G", L1);
    step(b);
    ok &= drains(b, 0);
    set(b, "G", L0);
    set(b, "SRCLR", L0);
    step(b);
    ok &= drains(b, reversed(0x96, 8)) && is(b, "SEROUT", L0);
    expect(ok, "19 TPIC6B595 DRAIN low exactly when G is low and the storage bit is 1; SRCLR clears "
               "the shift register, not the storage");
    bench_destroy(b);
}

static void check_tpic6b596(void) {
    Bench *b = open_fx("TPIC6B596");
    int ok;
    unsigned k;

    set(b, "SRCLR", L1);
    set(b, "G", L0);
    set(b, "SRCK", L0);
    set(b, "RCK", L0);
    set(b, "SERIN", L1);
    step(b);
    for (k = 0; k < 7; k++) {
        pulse(b, "SRCK");
    }
    ok = is(b, "SEROUT", L0);
    set(b, "SRCK", L1);   /* the 8th rising edge: stage 8 becomes 1 */
    step(b);
    ok &= is(b, "SEROUT", L0);
    set(b, "SRCK", L0);   /* falling: SER OUT takes it */
    step(b);
    ok &= is(b, "SEROUT", L1);
    pulse(b, "RCK");
    ok &= drains(b, 0xFF);
    set(b, "SRCLR", L0);   /* clears the shift register, the storage and SER OUT */
    step(b);
    expect(ok && drains(b, 0) && is(b, "SEROUT", L0),
           "20 TPIC6B596 SER OUT changes on SRCK falling; SRCLR clears the storage too");
    bench_destroy(b);
}


/* ---- 4094 ------------------------------------------------------------------------ */

static void check_4094(void) {
    static const char *const Q[] = {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "Q8"};
    Bench *b = open_fx("4094B");
    int ok;
    unsigned k;

    set(b, "OE", L1);
    set(b, "STROBE", L0);
    set(b, "CLOCK", L0);
    set(b, "DATA", L1);
    step(b);
    pulse(b, "CLOCK");
    ok = pins_value(b, Q, 8, 0);   /* STROBE low: storage holds */
    set(b, "STROBE", L1);
    step(b);
    ok &= pins_value(b, Q, 8, 0x01);   /* follows */
    set(b, "DATA", L0);
    step(b);
    for (k = 0; k < 6; k++) {
        pulse(b, "CLOCK");
    }
    ok &= pins_value(b, Q, 8, 0x40) && is(b, "QS", L0);   /* 7 edges: the 1 is in stage 7 */
    set(b, "CLOCK", L1);   /* 8th rising: stage 8 -> 1 on QS; Q'S waits for the fall */
    step(b);
    ok &= is(b, "QS", L1) && is(b, "Q'S", L0);
    set(b, "CLOCK", L0);
    step(b);
    ok &= is(b, "Q'S", L1);
    set(b, "OE", L0);
    step(b);
    expect(ok && pins_all(b, Q, 8, LZ) && is(b, "QS", L1),
           "21 4094B STROBE high: storage follows; OE low -> Z; QS on CLOCK rising, Q'S on CLOCK "
           "falling");
    bench_destroy(b);
}

/* ---- plan-10-H: ls673 / ls674 ------------------------------------------------ */

static const char *const Y16[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7",
                                  "Y8", "Y9", "Y10", "Y11", "Y12", "Y13", "Y14", "Y15"};
static const char *const P16[] = {"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
                                  "P8", "P9", "P10", "P11", "P12", "P13", "P14", "P15"};

/* Write mode: shifts value's bits in through SER/Q15, LSB first, on falling clock edges. */
static void write_16(Bench *b, const char *clock, unsigned value) {
    unsigned i;

    set(b, "R/W", L0);
    step(b);
    for (i = 0; i < 16; i++) {
        set(b, "SER/Q15", bit(value, i));
        set(b, clock, L1);
        step(b);
        set(b, clock, L0);
        step(b);
    }
    set(b, "SER/Q15", LZ);
    step(b);
}

/* Read mode, MODE 0: recirculates, reading Q15 before each falling edge; returns the 16 bits
   in the order they come out (first = bit 15 of Q). */
static unsigned read_16(Bench *b, const char *clock, const char *mode) {
    unsigned i, v = 0;

    set(b, "R/W", L1);
    set(b, mode, L0);
    step(b);
    for (i = 0; i < 16; i++) {
        v |= (bench_get(b, "SER/Q15") == L1 ? 1u : 0u) << i;
        set(b, clock, L1);
        step(b);
        set(b, clock, L0);
        step(b);
    }
    return v;
}

static void check_673(void) {
    Bench *b = open_fx("74_673");
    int ok;
    unsigned v;

    set(b, "CS", L0);
    set(b, "STRCLR", L1);
    set(b, "MODE/STRCLK", L0);
    set(b, "SHCLK", L0);
    step(b);
    write_16(b, "SHCLK", 0xA5C3);
    /* The first bit in went furthest: Q15 = bit 0, so reading gives the bits back in order. */
    v = read_16(b, "SHCLK", "MODE/STRCLK");
    ok = v == 0xA5C3;
    if (!ok) {
        printf("  read 0x%04X\n", v);
    }
    expect(ok, "24 74_673 write mode shifts the SER pin in on the gated clock's falling edge");

    /* 25: store clock with R/W 0 copies Q to Y; STRCLR clears Y with CS high. */
    set(b, "R/W", L0);
    set(b, "MODE/STRCLK", L0);
    step(b);
    set(b, "MODE/STRCLK", L1);   /* store clock rising: Y <- Q */
    step(b);
    set(b, "MODE/STRCLK", L0);
    step(b);
    {
        /* After write_16(0xA5C3) and a full recirculation, stage k holds bit 15-k of the value. */
        unsigned want = 0, i;

        for (i = 0; i < 16; i++) {
            want |= ((0xA5C3u >> i) & 1u) << (15 - i);
        }
        ok = pins_value(b, Y16, 16, want);
        set(b, "CS", L1);
        set(b, "STRCLR", L0);
        step(b);
        ok &= pins_value(b, Y16, 16, 0);
        set(b, "STRCLR", L1);
        set(b, "CS", L0);
        step(b);
        expect(ok, "25 74_673 the store clock (R/W 0) copies Q to Y; STRCLR clears Y with CS high");

        /* 26: store a value, then read mode MODE 1 loads Q from Y; MODE 0 recirculates. */
        set(b, "MODE/STRCLK", L1);
        step(b);
        set(b, "MODE/STRCLK", L0);
        step(b);
        ok = pins_value(b, Y16, 16, want);
        set(b, "R/W", L1);
        set(b, "MODE/STRCLK", L1);
        step(b);
        set(b, "SHCLK", L1);
        step(b);
        set(b, "SHCLK", L0);   /* parallel load Q <- Y */
        step(b);
        ok &= is(b, "SER/Q15", bit(want, 15));
        v = read_16(b, "SHCLK", "MODE/STRCLK");
        ok &= v == 0xA5C3;
        set(b, "CS", L1);
        step(b);
        ok &= is(b, "SER/Q15", LZ);
        set(b, "CS", L0);
        set(b, "R/W", L0);
        step(b);
        expect(ok && is(b, "SER/Q15", LZ),
               "26 74_673 read mode MODE 1 loads Q from Y; MODE 0 recirculates; SER/Q15 driven only "
               "in read with CS 0");
    }
    bench_destroy(b);

    /* 27: CS rising with the shift clock high is a falling gated-clock edge. */
    b = open_fx("74_673");
    set(b, "STRCLR", L1);
    set(b, "MODE/STRCLK", L0);
    set(b, "SHCLK", L0);
    set(b, "CS", L1);
    set(b, "R/W", L0);
    set(b, "SER/Q15", L0);   /* the gated clock leaving X at power-up is a falling edge */
    step(b);
    set(b, "SER/Q15", L1);
    step(b);
    set(b, "SHCLK", L1);
    step(b);
    set(b, "CS", L0);   /* gated clock rises: no shift */
    step(b);
    set(b, "CS", L1);   /* gated clock falls: a shift of the 1 into Q0 */
    step(b);
    set(b, "SHCLK", L0);
    set(b, "SER/Q15", LZ);
    set(b, "CS", L0);
    step(b);
    set(b, "MODE/STRCLK", L1);   /* store, to see Q on Y */
    step(b);
    expect(pins_value(b, Y16, 16, 0x0001),
           "27 74_673 CS rising with the clock high is a falling gated-clock edge (the datasheet's "
           "warning)");
    bench_destroy(b);
}

static void check_674(void) {
    Bench *b = open_fx("74_674");
    unsigned v, want = 0, i;

    set(b, "CS", L0);
    set(b, "CLK", L0);
    set_pins(b, P16, 16, 0x1234);
    set(b, "R/W", L1);
    set(b, "MODE", L1);
    step(b);
    set(b, "CLK", L1);
    step(b);
    set(b, "CLK", L0);   /* parallel load */
    step(b);
    v = read_16(b, "CLK", "MODE");
    for (i = 0; i < 16; i++) {
        want |= ((0x1234u >> (15 - i)) & 1u) << i;
    }
    if (v != want) {
        printf("  read 0x%04X, want 0x%04X\n", v, want);
    }
    expect(v == want, "28 74_674 MODE 1 in read mode loads P0-P15");
    bench_destroy(b);
}

/* ---- plan-10-H: 4517B, 4034B ------------------------------------------------------ */

static void check_4517(void) {
    Bench *b = open_fx("4517B");
    int ok = 1;
    unsigned k;

    set(b, "WEA", L0);
    set(b, "CLA", L0);
    set(b, "DA", L1);
    step(b);
    pulse(b, "CLA");
    set(b, "DA", L0);
    step(b);
    for (k = 2; k <= 64; k++) {
        pulse(b, "CLA");
        ok &= is(b, "Q16A", k == 16 ? L1 : L0) && is(b, "Q64A", k == 64 ? L1 : L0);
    }
    expect(ok, "29 4517B 64 clocks from D to Q64 (and 16 to Q16)");

    set(b, "WEA", L1);
    step(b);
    ok = is(b, "Q16A", LZ) && is(b, "Q32A", LZ) && is(b, "Q48A", LZ) && is(b, "Q64A", LZ);
    set(b, "Q16A", L1);   /* the tap-16 pin feeds stage 17 */
    step(b);
    pulse(b, "CLA");
    set(b, "Q16A", LZ);
    set(b, "WEA", L0);
    step(b);
    for (k = 18; k <= 32; k++) {
        pulse(b, "CLA");
    }
    expect(ok && is(b, "Q32A", L1), "30 4517B WE 1: the taps are Z; the tap-16 pin feeds stage 17");
    bench_destroy(b);
}

static const char *const A8[] = {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"};
static const char *const B8[] = {"B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"};

static void check_4034(void) {
    Bench *b = open_fx("4034B");
    int ok;

    set(b, "AE", L1);
    set(b, "A/B", L1);
    set(b, "P/S", L0);
    set(b, "A/S", L0);
    set(b, "CLOCK", L0);
    step(b);
    shift_in(b, "SI", "CLOCK", 8, 0x8D);
    expect(pins_value(b, B8, 8, reversed(0x8D, 8)) && pins_all(b, A8, 8, LZ),
           "31 4034B serial shift into the B outputs (A/B 1)");

    set_pins(b, A8, 8, 0x5A);
    set(b, "P/S", L1);
    step(b);
    ok = pins_value(b, B8, 8, reversed(0x8D, 8));   /* synchronous: no change yet */
    pulse(b, "CLOCK");
    ok &= pins_value(b, B8, 8, 0x5A);
    set(b, "A/S", L1);   /* asynchronous: follows A */
    set_pins(b, A8, 8, 0x3C);
    step(b);
    ok &= pins_value(b, B8, 8, 0x3C);
    set(b, "AE", L0);   /* recirculation: A ignored */
    set_pins(b, A8, 8, 0xFF);
    step(b);
    pulse(b, "CLOCK");
    expect(ok && pins_value(b, B8, 8, 0x3C),
           "32 4034B sync parallel load from A on the edge; async load follows A; AE 0 recirculates");

    release_pins(b, A8, 8);
    set(b, "A/S", L0);
    set(b, "A/B", L0);   /* B is the input, A the output */
    step(b);
    ok = pins_all(b, B8, 8, LZ) && pins_all(b, A8, 8, LZ);   /* AE 0: A off */
    set(b, "AE", L1);
    step(b);
    ok &= pins_value(b, A8, 8, 0x3C);
    set_pins(b, B8, 8, 0x81);
    step(b);
    pulse(b, "CLOCK");
    expect(ok && pins_value(b, A8, 8, 0x81) && bench_error(b) == NULL,
           "33 4034B A/B 0: B is the input, A driven only with AE 1");
    bench_destroy(b);
}

/* ---- plan-10-H: FIFO --------------------------------------------------------------- */

static const char *const D4[] = {"D0", "D1", "D2", "D3"};
static const char *const Q4[] = {"Q0", "Q1", "Q2", "Q3"};

/* The fixture's fall-through is 1500 ns (100 ns per position step). */
static Bench *open_fifo(const char *device) {
    Bench *b = open_fx(device);

    set(b, "OE", L0);
    set(b, "SI", L0);
    set(b, "SO", L1);
    set(b, "MR", L1);
    step(b);
    set(b, "MR", L0);
    step(b);
    return b;
}

static void fifo_write(Bench *b, unsigned value) {
    set_pins(b, D4, 4, value);
    set(b, "SI", L1);
    bench_run(b, NS(20));
    set(b, "SI", L0);
    bench_run(b, NS(20));
}

static void fifo_unload(Bench *b) {
    set(b, "SO", L0);
    bench_run(b, NS(20));
    set(b, "SO", L1);
    bench_run(b, NS(20));
}

static void check_fifo(void) {
    Bench *b = open_fifo("40105B");
    int ok;
    unsigned k;

    ok = is(b, "DIR", L1) && is(b, "DOR", L0);
    set_pins(b, D4, 4, 0x9);
    set(b, "SI", L1);
    bench_run(b, NS(1000));
    ok &= is(b, "DOR", L0);   /* still falling through */
    bench_run(b, NS(1000));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x9),
           "34 40105B one word into an empty FIFO: DOR high and Q valid after the fall-through time");
    set(b, "SI", L0);
    step(b);

    for (k = 1; k < 16; k++) {
        fifo_write(b, k);
        bench_run(b, NS(2000));
    }
    ok = is(b, "DIR", L0);
    fifo_write(b, 0xF);   /* the 17th: ignored */
    bench_run(b, NS(2000));
    expect(ok && is(b, "DIR", L0) && pins_value(b, Q4, 4, 0x9),
           "35 40105B 16 writes fill it: DIR stays low; a 17th SI is ignored");

    set(b, "SO", L0);
    bench_run(b, NS(20));
    ok = is(b, "DOR", L0);
    set(b, "SO", L1);
    bench_run(b, NS(300));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x1),
           "36 40105B SO falling unloads; DOR low, then high when the next word arrives");

    set(b, "MR", L1);
    step(b);
    expect(is(b, "DIR", L1) && is(b, "DOR", L0), "37 40105B MR empties it: DIR 1, DOR 0");
    set(b, "MR", L0);
    step(b);

    fifo_write(b, 0x6);
    bench_run(b, NS(300));
    fifo_write(b, 0x5);
    bench_run(b, NS(3000));
    set(b, "OE", L1);   /* outputs disabled: unloading inhibited */
    step(b);
    fifo_unload(b);
    bench_run(b, NS(300));
    ok = is(b, "DOR", L1) && pins_all(b, Q4, 4, LZ);
    set(b, "SO", L0);
    step(b);
    set(b, "OE", L0);   /* enabling with SO low unloads */
    bench_run(b, NS(20));
    ok &= is(b, "DOR", L0);
    bench_run(b, NS(300));
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0x5),
           "38 40105B output disable blocks unloading; enabling with SO low unloads");
    bench_destroy(b);

    b = open_fifo("74_40105");
    fifo_write(b, 0x3);
    bench_run(b, NS(3000));
    set(b, "OE", L1);
    step(b);
    fifo_unload(b);   /* HC: SO alone unloads, even while disabled */
    bench_run(b, NS(20));
    ok = is(b, "DOR", L0);
    for (k = 0; k < 16; k++) {
        fifo_write(b, k);
        bench_run(b, NS(2000));
    }
    ok &= is(b, "DIR", L0);   /* full */
    set_pins(b, D4, 4, 0xC);
    set(b, "SI", L1);   /* held high */
    step(b);
    set(b, "OE", L0);
    fifo_unload(b);   /* position 1 frees after the ripple; SI still high loads 0xC */
    bench_run(b, NS(3000));
    set(b, "SI", L0);
    step(b);
    for (k = 1; k < 16; k++) {
        fifo_unload(b);
        bench_run(b, NS(2000));
    }
    expect(ok && is(b, "DOR", L1) && pins_value(b, Q4, 4, 0xC),
           "39 74_40105 SO unloads while disabled; SI held high loads when position 1 frees");
    bench_destroy(b);
}

/* ---- plan-10-H: SN74LV8153 --------------------------------------------------------- */

static const char *const Y8[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"};

/* One frame: ST0 0, ST1 1, A0-A2, the nibble, stop 1; period in ps. Stops after `bits` bits
   (10 = the whole frame) and leaves D at the last bit. */
static void frame(Bench *b, unsigned address, unsigned nibble, SimTime period, unsigned bits) {
    unsigned levels[10];
    unsigned k;

    levels[0] = 0;
    levels[1] = 1;
    for (k = 0; k < 3; k++) {
        levels[2 + k] = (address >> k) & 1u;
    }
    for (k = 0; k < 4; k++) {
        levels[5 + k] = (nibble >> k) & 1u;
    }
    levels[9] = 1;
    for (k = 0; k < bits; k++) {
        set(b, "D", levels[k] ? L1 : L0);
        bench_run(b, period);
    }
}

static Bench *open_8153(unsigned address) {
    static const char *const A[] = {"A0", "A1", "A2"};
    Bench *b = open_fx("74_8153");

    set(b, "D", L1);
    set(b, "OUTSEL", L1);
    set(b, "OE", L0);
    set_pins(b, A, 3, address);
    set(b, "RESET", L0);
    step(b);
    set(b, "RESET", L1);
    bench_run(b, NS(1000));
    return b;
}

static void check_8153(void) {
    const SimTime p9600 = 1000000000000ull / 9600;
    Bench *b = open_8153(5);
    int ok;

    frame(b, 5, 0x6, p9600, 10);   /* D0-D3 = 6 */
    bench_run(b, p9600);
    frame(b, 5, 0xB, p9600, 8);    /* D4-D7 = 0xB; stop just before the D7 bit */
    set(b, "D", L1);   /* D7 = 1 (0xB bit 3) */
    bench_run(b, p9600 * 4 / 10);
    ok = pins_value(b, Y8, 8, 0x00) && is(b, "SOUT", L1);
    bench_run(b, p9600 * 2 / 10);   /* past the D7 sample point */
    ok &= pins_value(b, Y8, 8, 0xB6) && is(b, "SOUT", L0);
    bench_run(b, p9600 * 8 / 10);
    ok &= is(b, "SOUT", L0);
    bench_run(b, p9600 * 4 / 10);
    expect(ok && is(b, "SOUT", L1),
           "40 74_8153 two frames at 9600 bit/s, A = 5: Y = the byte at the D7 sample point; SOUT low "
           "for one bit");

    bench_run(b, p9600 * 2);
    frame(b, 3, 0xF, p9600, 10);   /* another device's frame: ignored */
    bench_run(b, p9600);
    frame(b, 5, 0x1, p9600, 10);   /* first */
    bench_run(b, p9600);
    frame(b, 3, 0x2, p9600, 10);   /* ignored (other address): not a second frame */
    bench_run(b, p9600);
    frame(b, 5, 0x2, p9600, 10);   /* second: completes 0x21 */
    bench_run(b, p9600);
    ok = pins_value(b, Y8, 8, 0x21);
    bench_destroy(b);
    /* A pair with mismatched addresses is dropped: pins A = 5 accept both ... */
    b = open_8153(5);
    frame(b, 5, 0x7, p9600, 10);
    bench_run(b, p9600);
    frame(b, 5, 0x0, p9600, 10);   /* 0x07 */
    bench_run(b, p9600);
    ok &= pins_value(b, Y8, 8, 0x07);
    bench_destroy(b);
    expect(ok, "41 74_8153 a frame with another address is ignored; the pair completes with the next "
               "matching frame");

    b = open_8153(0);
    frame(b, 0, 0x5, p9600, 10);
    bench_run(b, p9600);
    frame(b, 0, 0xA, p9600, 10);   /* 0xA5 */
    bench_run(b, p9600);
    set(b, "OUTSEL", L0);
    step(b);
    {
        unsigned i;

        ok = 1;
        for (i = 0; i < 8; i++) {
            ok &= is(b, Y8[i], (0xA5u >> i) & 1u ? L0 : LZ);
        }
    }
    set(b, "OUTSEL", L1);
    set(b, "OE", L1);
    step(b);
    ok &= pins_all(b, Y8, 8, LZ);
    set(b, "OE", L0);
    set(b, "RESET", L0);
    step(b);
    expect(ok && pins_value(b, Y8, 8, 0x00),
           "42 74_8153 OUTSEL 0: Y = 0 for 1 bits, Z for 0 bits; OE high -> Z; RESET low -> 0");
    bench_destroy(b);

    b = open_8153(2);
    frame(b, 2, 0xC, 1000000000000ull / 2400, 10);
    bench_run(b, NS(100000));
    frame(b, 2, 0x3, 1000000000000ull / 19200, 10);   /* 0x3C */
    bench_run(b, NS(100000));
    ok = pins_value(b, Y8, 8, 0x3C);
    expect(ok, "43 74_8153 the bit rate is measured per frame (a 2400 then a 19200 bit/s frame)");
    bench_destroy(b);
}

/* ---- Loader and voltage ------------------------------------------------------------ */

static int expect_error(const char *text, const char *want) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "bad.json");
    int ok = error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n  want: %s\n", error != NULL ? error : "(none)", want);
    }
    free(error);
    part_library_destroy(lib);
    return ok;
}

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'shift','devices':[{'id':'bad'," \
    "'function':'Shift register','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'C','role':'input'}]}," \
    "'model':'shift','units':[{"
#define BAD_TAIL "}],'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static void check_loader(void) {
    int ok = expect_error(BAD_HEAD "'stages':4,'clock':{'pin':'A','edge':'rising'},'serial':['B'],"
                          "'serialJK':{'j':'B','kbar':'C'}" BAD_TAIL,
                          "exactly one of \"serial\", \"serialJK\" and \"serialSelect\"");

    ok &= expect_error(BAD_HEAD "'stages':4,'clock':{'pin':'A','edge':'rising'},'serial':['B'],"
                       "'storage':{'clock':{'pin':'C','edge':'rising'},'latch':{'pin':'C',"
                       "'active':'high'},'outputs':['Q','Q','Q','Q']}" BAD_TAIL,
                       "a storage has exactly one of \"clock\" and \"latch\"");
    ok &= expect_error(BAD_HEAD "'stages':65,'clock':{'pin':'A','edge':'rising'},'serial':['B']"
                       BAD_TAIL, "\"stages\" must be 1 to 64");
    expect(ok, "22 loader: serial with serialJK, a storage with both clock and latch, and 65 stages "
               "are errors");
}

static size_t pin_index(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    return SIZE_MAX;
}

static void check_open_drain(void) {
    const Part *tpic = part_library_find(library, "FX-TPIC6B595");
    const Part *od = part_library_find(library, "FX-74_596");
    const Part *pp = part_library_find(library, "FX-74_595");

    expect(tpic != NULL && od != NULL && pp != NULL &&
               part_pin_open_drain(tpic, pin_index(tpic, "DRAIN0")) &&
               part_pin_open_drain(od, pin_index(od, "QA")) &&
               !part_pin_open_drain(pp, pin_index(pp, "QA")) &&
               !part_pin_open_drain(tpic, pin_index(tpic, "SEROUT")),
           "23 voltage: open drain on FX-TPIC6B595 DRAIN0 and FX-74_596 QA, not FX-74_595 QA");
}

int main(void) {
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "shift_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_164();
    check_264();
    check_165();
    check_166();
    check_194();
    check_195();
    check_299();
    check_323();
    check_4014();
    check_4015();
    check_4035();
    check_4031();
    check_595();
    check_594();
    check_596();
    check_597();
    check_598();
    check_tpic6b595();
    check_tpic6b596();
    check_4094();
    check_loader();
    check_open_drain();
    check_673();
    check_674();
    check_4517();
    check_4034();
    check_fifo();
    check_8153();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("shift_test: all passed\n");
    return 0;
}
