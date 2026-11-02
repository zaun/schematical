/*
 * shift_test: the shift kind's shift model against the function tables of
 * the devices that use it (plan-10-G section 5). Fixture devices copy the
 * real devices' pinouts and keys from the spec sections (no shift part file
 * exists before plan-10-I), with 10 ns delays; levels are checked 50 ns
 * after each step.
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
    "ckage':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]}]}";

/* Dual-row fixture footprints FX<n>, one per pin count above. */
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[{'id':'FX14','description':"
    "'d','style':'dual','pins':14,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'wi"
    "dth':0.5,'height':0.5,'shape':'rect'}},{'id':'FX16','description':'d','style':'dual','pi"
    "ns':16,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,"
    "'shape':'rect'}},{'id':'FX20','description':'d','style':'dual','pins':20,'body':{'width'"
    ":1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}]}";

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
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("shift_test: all passed\n");
    return 0;
}
