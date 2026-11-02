/*
 * latch_test: the latch kind's five models against the function tables of
 * the devices that use them (plan-10-B section 6). Fixture devices copy the
 * real devices' pinouts and keys from the spec sections (no latch part file
 * exists before plan-10-C), with 10 ns delays; levels are checked 50 ns
 * after each step. Cases 29-34 repeat a few on real parts from latch.json
 * (plan-10-C section 4.3), 1 us per step.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"
#include "model_pins.h"
#include "parts.h"
#include "read_file.h"
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

/* Generated from the spec sections (plan-10-data/latch-a.md, latch-b.md). */
static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'latch','devices':[{'id':'fx-74_373','f"
    "unction':'Latch','description':'Octal transparent D-type latches with 3-state outputs','"
    "drawer':[],'pinouts':{'p20':[{'number':1,'name':'OE','role':'input'},{'number':2,'name':"
    "'1Q','role':'output'},{'number':3,'name':'1D','role':'input'},{'number':4,'name':'2D','r"
    "ole':'input'},{'number':5,'name':'2Q','role':'output'},{'number':6,'name':'3Q','role':'o"
    "utput'},{'number':7,'name':'3D','role':'input'},{'number':8,'name':'4D','role':'input'},"
    "{'number':9,'name':'4Q','role':'output'},{'number':10,'name':'GND','role':'ground'},{'nu"
    "mber':11,'name':'LE','role':'input'},{'number':12,'name':'5Q','role':'output'},{'number'"
    ":13,'name':'5D','role':'input'},{'number':14,'name':'6D','role':'input'},{'number':15,'n"
    "ame':'6Q','role':'output'},{'number':16,'name':'7Q','role':'output'},{'number':17,'name'"
    ":'7D','role':'input'},{'number':18,'name':'8D','role':'input'},{'number':19,'name':'8Q',"
    "'role':'output'},{'number':20,'name':'VCC','role':'power'}]},'model':'transparent','unit"
    "s':[{'d':'1D','le':'LE','q':'1Q','oe':'OE'},{'d':'2D','le':'LE','q':'2Q','oe':'OE'},{'d'"
    ":'3D','le':'LE','q':'3Q','oe':'OE'},{'d':'4D','le':'LE','q':'4Q','oe':'OE'},{'d':'5D','l"
    "e':'LE','q':'5Q','oe':'OE'},{'d':'6D','le':'LE','q':'6Q','oe':'OE'},{'d':'7D','le':'LE',"
    "'q':'7Q','oe':'OE'},{'d':'8D','le':'LE','q':'8Q','oe':'OE'}],'enableActive':'high','outp"
    "utEnableActive':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name'"
    ":'FX-74_373','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'f"
    "x-74_533','function':'Latch','description':'Octal transparent D-type latches with invert"
    "ed 3-state outputs','drawer':[],'pinouts':{'p20':[{'number':1,'name':'OE','role':'input'"
    "},{'number':2,'name':'1Q','role':'output'},{'number':3,'name':'1D','role':'input'},{'num"
    "ber':4,'name':'2D','role':'input'},{'number':5,'name':'2Q','role':'output'},{'number':6,"
    "'name':'3Q','role':'output'},{'number':7,'name':'3D','role':'input'},{'number':8,'name':"
    "'4D','role':'input'},{'number':9,'name':'4Q','role':'output'},{'number':10,'name':'GND',"
    "'role':'ground'},{'number':11,'name':'LE','role':'input'},{'number':12,'name':'5Q','role"
    "':'output'},{'number':13,'name':'5D','role':'input'},{'number':14,'name':'6D','role':'in"
    "put'},{'number':15,'name':'6Q','role':'output'},{'number':16,'name':'7Q','role':'output'"
    "},{'number':17,'name':'7D','role':'input'},{'number':18,'name':'8D','role':'input'},{'nu"
    "mber':19,'name':'8Q','role':'output'},{'number':20,'name':'VCC','role':'power'}]},'model"
    "':'transparent','units':[{'d':'1D','le':'LE','qn':'1Q','oe':'OE'},{'d':'2D','le':'LE','q"
    "n':'2Q','oe':'OE'},{'d':'3D','le':'LE','qn':'3Q','oe':'OE'},{'d':'4D','le':'LE','qn':'4Q"
    "','oe':'OE'},{'d':'5D','le':'LE','qn':'5Q','oe':'OE'},{'d':'6D','le':'LE','qn':'6Q','oe'"
    ":'OE'},{'d':'7D','le':'LE','qn':'7Q','oe':'OE'},{'d':'8D','le':'LE','qn':'8Q','oe':'OE'}"
    "],'enableActive':'high','outputEnableActive':'low','timing':{'HC':{'rise':10,'fall':10,'"
    "vcc':[2,6]}},'parts':[{'name':'FX-74_533','package':'DIP','pinout':'p20','footprint':'FX"
    "20','family':'HC'}]},{'id':'fx-74_843','function':'Latch','description':'9-bit bus-inter"
    "face D-type latches with 3-state outputs','drawer':[],'pinouts':{'p24':[{'number':1,'nam"
    "e':'OE','role':'input'},{'number':2,'name':'1D','role':'input'},{'number':3,'name':'2D',"
    "'role':'input'},{'number':4,'name':'3D','role':'input'},{'number':5,'name':'4D','role':'"
    "input'},{'number':6,'name':'5D','role':'input'},{'number':7,'name':'6D','role':'input'},"
    "{'number':8,'name':'7D','role':'input'},{'number':9,'name':'8D','role':'input'},{'number"
    "':10,'name':'9D','role':'input'},{'number':11,'name':'CLR','role':'input'},{'number':12,"
    "'name':'GND','role':'ground'},{'number':13,'name':'LE','role':'input'},{'number':14,'nam"
    "e':'PRE','role':'input'},{'number':15,'name':'9Q','role':'output'},{'number':16,'name':'"
    "8Q','role':'output'},{'number':17,'name':'7Q','role':'output'},{'number':18,'name':'6Q',"
    "'role':'output'},{'number':19,'name':'5Q','role':'output'},{'number':20,'name':'4Q','rol"
    "e':'output'},{'number':21,'name':'3Q','role':'output'},{'number':22,'name':'2Q','role':'"
    "output'},{'number':23,'name':'1Q','role':'output'},{'number':24,'name':'VCC','role':'pow"
    "er'}]},'model':'transparent','units':[{'d':'1D','le':'LE','q':'1Q','oe':'OE','pre':'PRE'"
    ",'clr':'CLR'},{'d':'2D','le':'LE','q':'2Q','oe':'OE','pre':'PRE','clr':'CLR'},{'d':'3D',"
    "'le':'LE','q':'3Q','oe':'OE','pre':'PRE','clr':'CLR'},{'d':'4D','le':'LE','q':'4Q','oe':"
    "'OE','pre':'PRE','clr':'CLR'},{'d':'5D','le':'LE','q':'5Q','oe':'OE','pre':'PRE','clr':'"
    "CLR'},{'d':'6D','le':'LE','q':'6Q','oe':'OE','pre':'PRE','clr':'CLR'},{'d':'7D','le':'LE"
    "','q':'7Q','oe':'OE','pre':'PRE','clr':'CLR'},{'d':'8D','le':'LE','q':'8Q','oe':'OE','pr"
    "e':'PRE','clr':'CLR'},{'d':'9D','le':'LE','q':'9Q','oe':'OE','pre':'PRE','clr':'CLR'}],'"
    "enableActive':'high','outputEnableActive':'low','presetActive':'low','clearActive':'low'"
    ",'presetWins':true,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'F"
    "X-74_843','package':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]},{'id':'fx-7"
    "4_16373','function':'Latch','description':'16-bit transparent D-type latches with 3-stat"
    "e outputs','drawer':[],'pinouts':{'p48':[{'number':1,'name':'1OE','role':'input'},{'numb"
    "er':2,'name':'1Q1','role':'output'},{'number':3,'name':'1Q2','role':'output'},{'number':"
    "4,'name':'GND','role':'ground'},{'number':5,'name':'1Q3','role':'output'},{'number':6,'n"
    "ame':'1Q4','role':'output'},{'number':7,'name':'VCC','role':'power'},{'number':8,'name':"
    "'1Q5','role':'output'},{'number':9,'name':'1Q6','role':'output'},{'number':10,'name':'GN"
    "D','role':'ground'},{'number':11,'name':'1Q7','role':'output'},{'number':12,'name':'1Q8'"
    ",'role':'output'},{'number':13,'name':'2Q1','role':'output'},{'number':14,'name':'2Q2','"
    "role':'output'},{'number':15,'name':'GND','role':'ground'},{'number':16,'name':'2Q3','ro"
    "le':'output'},{'number':17,'name':'2Q4','role':'output'},{'number':18,'name':'VCC','role"
    "':'power'},{'number':19,'name':'2Q5','role':'output'},{'number':20,'name':'2Q6','role':'"
    "output'},{'number':21,'name':'GND','role':'ground'},{'number':22,'name':'2Q7','role':'ou"
    "tput'},{'number':23,'name':'2Q8','role':'output'},{'number':24,'name':'2OE','role':'inpu"
    "t'},{'number':25,'name':'2LE','role':'input'},{'number':26,'name':'2D8','role':'input'},"
    "{'number':27,'name':'2D7','role':'input'},{'number':28,'name':'GND','role':'ground'},{'n"
    "umber':29,'name':'2D6','role':'input'},{'number':30,'name':'2D5','role':'input'},{'numbe"
    "r':31,'name':'VCC','role':'power'},{'number':32,'name':'2D4','role':'input'},{'number':3"
    "3,'name':'2D3','role':'input'},{'number':34,'name':'GND','role':'ground'},{'number':35,'"
    "name':'2D2','role':'input'},{'number':36,'name':'2D1','role':'input'},{'number':37,'name"
    "':'1D8','role':'input'},{'number':38,'name':'1D7','role':'input'},{'number':39,'name':'G"
    "ND','role':'ground'},{'number':40,'name':'1D6','role':'input'},{'number':41,'name':'1D5'"
    ",'role':'input'},{'number':42,'name':'VCC','role':'power'},{'number':43,'name':'1D4','ro"
    "le':'input'},{'number':44,'name':'1D3','role':'input'},{'number':45,'name':'GND','role':"
    "'ground'},{'number':46,'name':'1D2','role':'input'},{'number':47,'name':'1D1','role':'in"
    "put'},{'number':48,'name':'1LE','role':'input'}]},'model':'transparent','units':[{'d':'1"
    "D1','le':'1LE','q':'1Q1','oe':'1OE'},{'d':'1D2','le':'1LE','q':'1Q2','oe':'1OE'},{'d':'1"
    "D3','le':'1LE','q':'1Q3','oe':'1OE'},{'d':'1D4','le':'1LE','q':'1Q4','oe':'1OE'},{'d':'1"
    "D5','le':'1LE','q':'1Q5','oe':'1OE'},{'d':'1D6','le':'1LE','q':'1Q6','oe':'1OE'},{'d':'1"
    "D7','le':'1LE','q':'1Q7','oe':'1OE'},{'d':'1D8','le':'1LE','q':'1Q8','oe':'1OE'},{'d':'2"
    "D1','le':'2LE','q':'2Q1','oe':'2OE'},{'d':'2D2','le':'2LE','q':'2Q2','oe':'2OE'},{'d':'2"
    "D3','le':'2LE','q':'2Q3','oe':'2OE'},{'d':'2D4','le':'2LE','q':'2Q4','oe':'2OE'},{'d':'2"
    "D5','le':'2LE','q':'2Q5','oe':'2OE'},{'d':'2D6','le':'2LE','q':'2Q6','oe':'2OE'},{'d':'2"
    "D7','le':'2LE','q':'2Q7','oe':'2OE'},{'d':'2D8','le':'2LE','q':'2Q8','oe':'2OE'}],'enabl"
    "eActive':'high','outputEnableActive':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,"
    "6]}},'parts':[{'name':'FX-74_16373','package':'DIP','pinout':'p48','footprint':'FX48','f"
    "amily':'HC'}]},{'id':'fx-74_75','function':'Latch','description':'4-bit bistable latches"
    "','drawer':[],'pinouts':{'p16':[{'number':1,'name':'1QBAR','role':'output'},{'number':2,"
    "'name':'1D','role':'input'},{'number':3,'name':'2D','role':'input'},{'number':4,'name':'"
    "3C4C','role':'input'},{'number':5,'name':'VCC','role':'power'},{'number':6,'name':'3D','"
    "role':'input'},{'number':7,'name':'4D','role':'input'},{'number':8,'name':'4QBAR','role'"
    ":'output'},{'number':9,'name':'4Q','role':'output'},{'number':10,'name':'3Q','role':'out"
    "put'},{'number':11,'name':'3QBAR','role':'output'},{'number':12,'name':'GND','role':'gro"
    "und'},{'number':13,'name':'1C2C','role':'input'},{'number':14,'name':'2QBAR','role':'out"
    "put'},{'number':15,'name':'2Q','role':'output'},{'number':16,'name':'1Q','role':'output'"
    "}]},'model':'transparent','units':[{'d':'1D','le':'1C2C','q':'1Q','qn':'1QBAR'},{'d':'2D"
    "','le':'1C2C','q':'2Q','qn':'2QBAR'},{'d':'3D','le':'3C4C','q':'3Q','qn':'3QBAR'},{'d':'"
    "4D','le':'3C4C','q':'4Q','qn':'4QBAR'}],'enableActive':'high','timing':{'HC':{'rise':10,"
    "'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_75','package':'DIP','pinout':'p16','foot"
    "print':'FX16','family':'HC'}]},{'id':'fx-4042B','function':'Latch','description':'Quad c"
    "locked D latch','drawer':[],'pinouts':{'p16':[{'number':1,'name':'Q4','role':'output'},{"
    "'number':2,'name':'Q1','role':'output'},{'number':3,'name':'Q1BAR','role':'output'},{'nu"
    "mber':4,'name':'D1','role':'input'},{'number':5,'name':'CLOCK','role':'input'},{'number'"
    ":6,'name':'POLARITY','role':'input'},{'number':7,'name':'D2','role':'input'},{'number':8"
    ",'name':'VSS','role':'ground'},{'number':9,'name':'Q2BAR','role':'output'},{'number':10,"
    "'name':'Q2','role':'output'},{'number':11,'name':'Q3','role':'output'},{'number':12,'nam"
    "e':'Q3BAR','role':'output'},{'number':13,'name':'D3','role':'input'},{'number':14,'name'"
    ":'D4','role':'input'},{'number':15,'name':'Q4BAR','role':'output'},{'number':16,'name':'"
    "VDD','role':'power'}]},'model':'transparent','units':[{'d':'D1','le':'CLOCK','q':'Q1','q"
    "n':'Q1BAR'},{'d':'D2','le':'CLOCK','q':'Q2','qn':'Q2BAR'},{'d':'D3','le':'CLOCK','q':'Q3"
    "','qn':'Q3BAR'},{'d':'D4','le':'CLOCK','q':'Q4','qn':'Q4BAR'}],'polarity':'POLARITY','ti"
    "ming':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4042B','package':'DI"
    "P','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4508B','function':'Latch"
    "','description':'Dual 4-bit latch with 3-state outputs','drawer':[],'pinouts':{'p24':[{'"
    "number':1,'name':'RESETA','role':'input'},{'number':2,'name':'STROBEA','role':'input'},{"
    "'number':3,'name':'ODA','role':'input'},{'number':4,'name':'D0A','role':'input'},{'numbe"
    "r':5,'name':'Q0A','role':'output'},{'number':6,'name':'D1A','role':'input'},{'number':7,"
    "'name':'Q1A','role':'output'},{'number':8,'name':'D2A','role':'input'},{'number':9,'name"
    "':'Q2A','role':'output'},{'number':10,'name':'D3A','role':'input'},{'number':11,'name':'"
    "Q3A','role':'output'},{'number':12,'name':'VSS','role':'ground'},{'number':13,'name':'RE"
    "SETB','role':'input'},{'number':14,'name':'STROBEB','role':'input'},{'number':15,'name':"
    "'ODB','role':'input'},{'number':16,'name':'D0B','role':'input'},{'number':17,'name':'Q0B"
    "','role':'output'},{'number':18,'name':'D1B','role':'input'},{'number':19,'name':'Q1B','"
    "role':'output'},{'number':20,'name':'D2B','role':'input'},{'number':21,'name':'Q2B','rol"
    "e':'output'},{'number':22,'name':'D3B','role':'input'},{'number':23,'name':'Q3B','role':"
    "'output'},{'number':24,'name':'VDD','role':'power'}]},'model':'transparent','units':[{'d"
    "':'D0A','le':'STROBEA','q':'Q0A','oe':'ODA','clr':'RESETA'},{'d':'D1A','le':'STROBEA','q"
    "':'Q1A','oe':'ODA','clr':'RESETA'},{'d':'D2A','le':'STROBEA','q':'Q2A','oe':'ODA','clr':"
    "'RESETA'},{'d':'D3A','le':'STROBEA','q':'Q3A','oe':'ODA','clr':'RESETA'},{'d':'D0B','le'"
    ":'STROBEB','q':'Q0B','oe':'ODB','clr':'RESETB'},{'d':'D1B','le':'STROBEB','q':'Q1B','oe'"
    ":'ODB','clr':'RESETB'},{'d':'D2B','le':'STROBEB','q':'Q2B','oe':'ODB','clr':'RESETB'},{'"
    "d':'D3B','le':'STROBEB','q':'Q3B','oe':'ODB','clr':'RESETB'}],'enableActive':'high','out"
    "putEnableActive':'low','clearActive':'high','timing':{'HC':{'rise':10,'fall':10,'vcc':[2"
    ",6]}},'parts':[{'name':'FX-4508B','package':'DIP','pinout':'p24','footprint':'FX24','fam"
    "ily':'HC'}]},{'id':'fx-74_259','function':'Latch','description':'8-bit addressable latch"
    "es','drawer':[],'pinouts':{'p16':[{'number':1,'name':'S0','role':'input'},{'number':2,'n"
    "ame':'S1','role':'input'},{'number':3,'name':'S2','role':'input'},{'number':4,'name':'Q0"
    "','role':'output'},{'number':5,'name':'Q1','role':'output'},{'number':6,'name':'Q2','rol"
    "e':'output'},{'number':7,'name':'Q3','role':'output'},{'number':8,'name':'GND','role':'g"
    "round'},{'number':9,'name':'Q4','role':'output'},{'number':10,'name':'Q5','role':'output"
    "'},{'number':11,'name':'Q6','role':'output'},{'number':12,'name':'Q7','role':'output'},{"
    "'number':13,'name':'D','role':'input'},{'number':14,'name':'G','role':'input'},{'number'"
    ":15,'name':'CLR','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'ad"
    "dressable','address':['S0','S1','S2'],'data':'D','outputs':['Q0','Q1','Q2','Q3','Q4','Q5"
    "','Q6','Q7'],'enable':{'pin':'G','active':'low'},'clear':{'pin':'CLR','active':'low'},'t"
    "iming':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_259','package':'"
    "DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4099B','function':'Lat"
    "ch','description':'8-bit addressable latch','drawer':[],'pinouts':{'p16':[{'number':1,'n"
    "ame':'Q7','role':'output'},{'number':2,'name':'RESET','role':'input'},{'number':3,'name'"
    ":'DATA','role':'input'},{'number':4,'name':'WD','role':'input'},{'number':5,'name':'A0',"
    "'role':'input'},{'number':6,'name':'A1','role':'input'},{'number':7,'name':'A2','role':'"
    "input'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'Q0','role':'output"
    "'},{'number':10,'name':'Q1','role':'output'},{'number':11,'name':'Q2','role':'output'},{"
    "'number':12,'name':'Q3','role':'output'},{'number':13,'name':'Q4','role':'output'},{'num"
    "ber':14,'name':'Q5','role':'output'},{'number':15,'name':'Q6','role':'output'},{'number'"
    ":16,'name':'VDD','role':'power'}]},'model':'addressable','address':['A0','A1','A2'],'dat"
    "a':'DATA','outputs':['Q0','Q1','Q2','Q3','Q4','Q5','Q6','Q7'],'enable':{'pin':'WD','acti"
    "ve':'low'},'clear':{'pin':'RESET','active':'high'},'timing':{'HC':{'rise':10,'fall':10,'"
    "vcc':[2,6]}},'parts':[{'name':'FX-4099B','package':'DIP','pinout':'p16','footprint':'FX1"
    "6','family':'HC'}]},{'id':'fx-TPIC6259','function':'Latch','description':'Power logic 8-"
    "bit addressable latch','drawer':[],'pinouts':{'p20':[{'number':1,'name':'PGND','role':'g"
    "round'},{'number':2,'name':'VCC','role':'power'},{'number':3,'name':'S0','role':'input'}"
    ",{'number':4,'name':'DRAIN0','role':'output'},{'number':5,'name':'DRAIN1','role':'output"
    "'},{'number':6,'name':'DRAIN2','role':'output'},{'number':7,'name':'DRAIN3','role':'outp"
    "ut'},{'number':8,'name':'S1','role':'input'},{'number':9,'name':'LGND','role':'ground'},"
    "{'number':10,'name':'PGND','role':'ground'},{'number':11,'name':'PGND','role':'ground'},"
    "{'number':12,'name':'S2','role':'input'},{'number':13,'name':'G','role':'input'},{'numbe"
    "r':14,'name':'DRAIN4','role':'output'},{'number':15,'name':'DRAIN5','role':'output'},{'n"
    "umber':16,'name':'DRAIN6','role':'output'},{'number':17,'name':'DRAIN7','role':'output'}"
    ",{'number':18,'name':'D','role':'input'},{'number':19,'name':'CLR','role':'input'},{'num"
    "ber':20,'name':'PGND','role':'ground'}]},'model':'addressable','address':['S0','S1','S2'"
    "],'data':'D','outputs':['DRAIN0','DRAIN1','DRAIN2','DRAIN3','DRAIN4','DRAIN5','DRAIN6','"
    "DRAIN7'],'enable':{'pin':'G','active':'low'},'clear':{'pin':'CLR','active':'low'},'outpu"
    "t':'open-drain','invert':true,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':"
    "[{'name':'FX-TPIC6259','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]"
    "},{'id':'fx-74_279','function':'Latch','description':'Quadruple S-R latches','drawer':[]"
    ",'pinouts':{'p16':[{'number':1,'name':'1R','role':'input'},{'number':2,'name':'1S1','rol"
    "e':'input'},{'number':3,'name':'1S2','role':'input'},{'number':4,'name':'1Q','role':'out"
    "put'},{'number':5,'name':'2R','role':'input'},{'number':6,'name':'2S','role':'input'},{'"
    "number':7,'name':'2Q','role':'output'},{'number':8,'name':'GND','role':'ground'},{'numbe"
    "r':9,'name':'3Q','role':'output'},{'number':10,'name':'3R','role':'input'},{'number':11,"
    "'name':'3S1','role':'input'},{'number':12,'name':'3S2','role':'input'},{'number':13,'nam"
    "e':'4Q','role':'output'},{'number':14,'name':'4R','role':'input'},{'number':15,'name':'4"
    "S','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'sr','units':[{'s"
    "':['1S1','1S2'],'r':'1R','q':'1Q'},{'s':['2S'],'r':'2R','q':'2Q'},{'s':['3S1','3S2'],'r'"
    ":'3R','q':'3Q'},{'s':['4S'],'r':'4R','q':'4Q'}],'inputsActive':'low','both':'set','timin"
    "g':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_279','package':'DIP'"
    ",'pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4043B','function':'Latch',"
    "'description':'Quad 3-state NOR R/S latch','drawer':[],'pinouts':{'p16':[{'number':1,'na"
    "me':'Q4','role':'output'},{'number':2,'name':'Q1','role':'output'},{'number':3,'name':'R"
    "1','role':'input'},{'number':4,'name':'S1','role':'input'},{'number':5,'name':'ENABLE','"
    "role':'input'},{'number':6,'name':'S2','role':'input'},{'number':7,'name':'R2','role':'i"
    "nput'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'Q2','role':'output'"
    "},{'number':10,'name':'Q3','role':'output'},{'number':11,'name':'R3','role':'input'},{'n"
    "umber':12,'name':'S3','role':'input'},{'number':13,'name':'NC','role':'nc'},{'number':14"
    ",'name':'S4','role':'input'},{'number':15,'name':'R4','role':'input'},{'number':16,'name"
    "':'VDD','role':'power'}]},'model':'sr','units':[{'s':['S1'],'r':'R1','q':'Q1'},{'s':['S2"
    "'],'r':'R2','q':'Q2'},{'s':['S3'],'r':'R3','q':'Q3'},{'s':['S4'],'r':'R4','q':'Q4'}],'in"
    "putsActive':'high','both':'set','outputEnable':{'pin':'ENABLE','active':'high'},'timing'"
    ":{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4043B','package':'DIP','p"
    "inout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4044B','function':'Latch','de"
    "scription':'Quad 3-state NAND R/S latch','drawer':[],'pinouts':{'p16':[{'number':1,'name"
    "':'Q4','role':'output'},{'number':2,'name':'NC','role':'nc'},{'number':3,'name':'S1','ro"
    "le':'input'},{'number':4,'name':'R1','role':'input'},{'number':5,'name':'ENABLE','role':"
    "'input'},{'number':6,'name':'R2','role':'input'},{'number':7,'name':'S2','role':'input'}"
    ",{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'Q2','role':'output'},{'nu"
    "mber':10,'name':'Q3','role':'output'},{'number':11,'name':'S3','role':'input'},{'number'"
    ":12,'name':'R3','role':'input'},{'number':13,'name':'Q1','role':'output'},{'number':14,'"
    "name':'R4','role':'input'},{'number':15,'name':'S4','role':'input'},{'number':16,'name':"
    "'VDD','role':'power'}]},'model':'sr','units':[{'s':['S1'],'r':'R1','q':'Q1'},{'s':['S2']"
    ",'r':'R2','q':'Q2'},{'s':['S3'],'r':'R3','q':'Q3'},{'s':['S4'],'r':'R4','q':'Q4'}],'inpu"
    "tsActive':'low','both':'reset','outputEnable':{'pin':'ENABLE','active':'high'},'timing':"
    "{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4044B','package':'DIP','pi"
    "nout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_4511','function':'Latch','d"
    "escription':'BCD-to-7-segment latch/decoder/driver','drawer':[],'pinouts':{'p16':[{'numb"
    "er':1,'name':'D1','role':'input'},{'number':2,'name':'D2','role':'input'},{'number':3,'n"
    "ame':'LT','role':'input'},{'number':4,'name':'BL','role':'input'},{'number':5,'name':'LE"
    "','role':'input'},{'number':6,'name':'D3','role':'input'},{'number':7,'name':'D0','role'"
    ":'input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'e','role':'outpu"
    "t'},{'number':10,'name':'d','role':'output'},{'number':11,'name':'c','role':'output'},{'"
    "number':12,'name':'b','role':'output'},{'number':13,'name':'a','role':'output'},{'number"
    "':14,'name':'g','role':'output'},{'number':15,'name':'f','role':'output'},{'number':16,'"
    "name':'VCC','role':'power'}]},'model':'bcd7seg','inputs':['D0','D1','D2','D3'],'latch':{"
    "'pin':'LE','active':'high'},'lampTest':{'pin':'LT','active':'low'},'blank':{'pin':'BL','"
    "active':'low'},'segments':['a','b','c','d','e','f','g'],'glyphs':['1111110','0110000','1"
    "101101','1111001','0110011','1011011','0011111','1110000','1111111','1110011','0000000',"
    "'0000000','0000000','0000000','0000000','0000000'],'timing':{'HC':{'rise':10,'fall':10,'"
    "vcc':[2,6]}},'parts':[{'name':'FX-74_4511','package':'DIP','pinout':'p16','footprint':'F"
    "X16','family':'HC'}]},{'id':'fx-74_16260','function':'Latch','description':'12-bit to 24"
    "-bit multiplexed D-type latch with 3-state outputs','drawer':[],'pinouts':{'p56':[{'numb"
    "er':1,'name':'OEA','role':'input'},{'number':2,'name':'LE1B','role':'input'},{'number':3"
    ",'name':'2B3','role':'inout'},{'number':4,'name':'GND','role':'ground'},{'number':5,'nam"
    "e':'2B2','role':'inout'},{'number':6,'name':'2B1','role':'inout'},{'number':7,'name':'VC"
    "C','role':'power'},{'number':8,'name':'A1','role':'inout'},{'number':9,'name':'A2','role"
    "':'inout'},{'number':10,'name':'A3','role':'inout'},{'number':11,'name':'GND','role':'gr"
    "ound'},{'number':12,'name':'A4','role':'inout'},{'number':13,'name':'A5','role':'inout'}"
    ",{'number':14,'name':'A6','role':'inout'},{'number':15,'name':'A7','role':'inout'},{'num"
    "ber':16,'name':'A8','role':'inout'},{'number':17,'name':'A9','role':'inout'},{'number':1"
    "8,'name':'GND','role':'ground'},{'number':19,'name':'A10','role':'inout'},{'number':20,'"
    "name':'A11','role':'inout'},{'number':21,'name':'A12','role':'inout'},{'number':22,'name"
    "':'VCC','role':'power'},{'number':23,'name':'1B1','role':'inout'},{'number':24,'name':'1"
    "B2','role':'inout'},{'number':25,'name':'GND','role':'ground'},{'number':26,'name':'1B3'"
    ",'role':'inout'},{'number':27,'name':'LE2B','role':'input'},{'number':28,'name':'SEL','r"
    "ole':'input'},{'number':29,'name':'OE1B','role':'input'},{'number':30,'name':'LEA1B','ro"
    "le':'input'},{'number':31,'name':'1B4','role':'inout'},{'number':32,'name':'GND','role':"
    "'ground'},{'number':33,'name':'1B5','role':'inout'},{'number':34,'name':'1B6','role':'in"
    "out'},{'number':35,'name':'VCC','role':'power'},{'number':36,'name':'1B7','role':'inout'"
    "},{'number':37,'name':'1B8','role':'inout'},{'number':38,'name':'1B9','role':'inout'},{'"
    "number':39,'name':'GND','role':'ground'},{'number':40,'name':'1B10','role':'inout'},{'nu"
    "mber':41,'name':'1B11','role':'inout'},{'number':42,'name':'1B12','role':'inout'},{'numb"
    "er':43,'name':'2B12','role':'inout'},{'number':44,'name':'2B11','role':'inout'},{'number"
    "':45,'name':'2B10','role':'inout'},{'number':46,'name':'GND','role':'ground'},{'number':"
    "47,'name':'2B9','role':'inout'},{'number':48,'name':'2B8','role':'inout'},{'number':49,'"
    "name':'2B7','role':'inout'},{'number':50,'name':'VCC','role':'power'},{'number':51,'name"
    "':'2B6','role':'inout'},{'number':52,'name':'2B5','role':'inout'},{'number':53,'name':'G"
    "ND','role':'ground'},{'number':54,'name':'2B4','role':'inout'},{'number':55,'name':'LEA2"
    "B','role':'input'},{'number':56,'name':'OE2B','role':'input'}]},'model':'bus-mux','a':['"
    "A1','A2','A3','A4','A5','A6','A7','A8','A9','A10','A11','A12'],'b1':['1B1','1B2','1B3','"
    "1B4','1B5','1B6','1B7','1B8','1B9','1B10','1B11','1B12'],'b2':['2B1','2B2','2B3','2B4','"
    "2B5','2B6','2B7','2B8','2B9','2B10','2B11','2B12'],'select':'SEL','latchEnables':{'b1':'"
    "LE1B','b2':'LE2B','a1':'LEA1B','a2':'LEA2B'},'outputEnables':{'a':'OEA','b1':'OE1B','b2'"
    ":'OE2B'},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_16260"
    "','package':'DIP','pinout':'p56','footprint':'FX56','family':'HC'}]}]}";

/* Dual-row fixture footprints FX<n>, one per pin count above. */
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[{'id':'FX16','description':"
    "'d','style':'dual','pins':16,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'wi"
    "dth':0.5,'height':0.5,'shape':'rect'}},{'id':'FX20','description':'d','style':'dual','pi"
    "ns':20,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,"
    "'shape':'rect'}},{'id':'FX24','description':'d','style':'dual','pins':24,'body':{'width'"
    ":1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id'"
    ":'FX48','description':'d','style':'dual','pins':48,'body':{'width':1,'height':1},'pitch'"
    ":1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id':'FX56','description':"
    "'d','style':'dual','pins':56,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'wi"
    "dth':0.5,'height':0.5,'shape':'rect'}}]}";

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

static Bench *open_fx(const char *device) {
    char name[64];
    const Part *part;

    snprintf(name, sizeof name, "FX-%s", device);
    part = part_library_find(library, name);
    if (part == NULL || part->kind != PART_LATCH) {
        fprintf(stderr, "no latch part %s\n", name);
        exit(1);
    }
    return bench_create(part);
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

/* Sets pins fmt % (i + base) for i < n from value's bits. */
static void set_bus(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, i + base);
        set(b, name, bit(value, i));
    }
}

/* 1 when every pin fmt % (i + base) shows want(i) (values[i]). */
static int bus_is(Bench *b, const char *fmt, unsigned base, unsigned n, const LogicValue *want) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got;

        snprintf(name, sizeof name, fmt, i + base);
        got = bench_get(b, name);
        if (got != want[i]) {
            printf("  %s = %c, want %c\n", name, "01XZ"[got], "01XZ"[want[i]]);
            ok = 0;
        }
    }
    return ok;
}

static int bus_value(Bench *b, const char *fmt, unsigned base, unsigned n, unsigned value) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = bit(value, i);
    }
    return bus_is(b, fmt, base, n, want);
}

static int bus_all(Bench *b, const char *fmt, unsigned base, unsigned n, LogicValue v) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = v;
    }
    return bus_is(b, fmt, base, n, want);
}

static int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

/* ---- transparent ---------------------------------------------------------- */

static void check_373(void) {
    Bench *b = open_fx("74_373");
    int ok;

    set(b, "OE", L0);
    set(b, "LE", L0);
    set_bus(b, "%uD", 1, 8, 0xFF);
    step(b);
    expect(bus_value(b, "%uQ", 1, 8, 0x00), "1 74_373 power-on: all Q = 0 (LE low, D high)");

    set(b, "LE", L1);
    step(b);
    ok = bus_value(b, "%uQ", 1, 8, 0xFF);
    set_bus(b, "%uD", 1, 8, 0xA5);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 8, 0xA5);
    set_bus(b, "%uD", 1, 8, 0x5A);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 8, 0x5A), "2 74_373 LE high: Q follows D (each bit)");

    set(b, "LE", L0);
    step(b);
    set_bus(b, "%uD", 1, 8, 0x0F);
    step(b);
    expect(bus_value(b, "%uQ", 1, 8, 0x5A), "3 74_373 LE falls: Q holds while D changes");

    set(b, "OE", L1);
    step(b);
    ok = bus_all(b, "%uQ", 1, 8, LZ);
    set(b, "LE", L1);
    step(b);
    set(b, "LE", L0);
    step(b);
    ok &= bus_all(b, "%uQ", 1, 8, LZ);
    set(b, "OE", L0);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 8, 0x0F),
           "4 74_373 OE high: Q = Z; LE high meanwhile loads; OE low shows the new data");

    /* stored 0x0F; D = 0x3C differs in bits 2, 3 (now 1 -> 0 ... ) and 4, 5. */
    set_bus(b, "%uD", 1, 8, 0x3C);
    step(b);
    set(b, "LE", LX);
    step(b);
    {
        LogicValue want[8] = {L1, L1, L1, L1, LX, LX, L0, L0};

        /* bits 0,1: stored 1, D 0 -> X; bits 2,3: 1 = 1; bits 4,5: 0 vs 1 -> X; 6,7: 0 = 0 */
        want[0] = LX;
        want[1] = LX;
        expect(bus_is(b, "%uQ", 1, 8, want),
               "5 74_373 LE = X: Q = X where D differs from the stored bit, unchanged where equal");
    }
    set(b, "LE", L0);
    set_bus(b, "%uD", 1, 8, 0x00);
    step(b);
    set(b, "LE", L1);
    step(b);
    set(b, "LE", L0);
    step(b);
    set(b, "OE", LX);
    step(b);
    ok = bus_all(b, "%uQ", 1, 8, LX);
    set(b, "OE", LZ);
    step(b);
    expect(ok && bus_all(b, "%uQ", 1, 8, LX), "6 74_373 OE = X -> Q = X; OE = Z -> Q = X");
    bench_destroy(b);
}

static void check_533(void) {
    Bench *b = open_fx("74_533");

    set(b, "OE", L0);
    set(b, "LE", L1);
    set_bus(b, "%uD", 1, 8, 0x3C);
    step(b);
    expect(bus_value(b, "%uQ", 1, 8, 0xC3), "7 74_533 outputs are NOT D");
    bench_destroy(b);
}

static void check_843(void) {
    Bench *b = open_fx("74_843");
    int ok;

    set(b, "OE", L0);
    set(b, "PRE", L1);
    set(b, "CLR", L1);
    set(b, "LE", L1);
    set_bus(b, "%uD", 1, 9, 0x1FF);
    step(b);
    ok = bus_value(b, "%uQ", 1, 9, 0x1FF);
    set(b, "CLR", L0);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x000);
    set(b, "LE", L0);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x000);
    set(b, "CLR", L1);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x000);
    set(b, "PRE", L0);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x1FF);
    set(b, "PRE", L1);
    set_bus(b, "%uD", 1, 9, 0x000);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 9, 0x1FF);
    set(b, "PRE", L0);
    set(b, "CLR", L0);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 9, 0x1FF),
           "8 74_843 CLR low -> 0 regardless of LE; PRE low -> 1; both low -> 1 (presetWins)");
    bench_destroy(b);
}

static void check_16373(void) {
    Bench *b = open_fx("74_16373");
    int ok;

    set(b, "1OE", L0);
    set(b, "2OE", L0);
    set(b, "1LE", L1);
    set(b, "2LE", L1);
    set_bus(b, "1D%u", 1, 8, 0x00);
    set_bus(b, "2D%u", 1, 8, 0xFF);
    step(b);
    set(b, "2LE", L0);
    step(b);
    set_bus(b, "1D%u", 1, 8, 0x96);
    set_bus(b, "2D%u", 1, 8, 0x00);
    step(b);
    ok = bus_value(b, "1Q%u", 1, 8, 0x96) && bus_value(b, "2Q%u", 1, 8, 0xFF);
    set(b, "1OE", L1);
    step(b);
    expect(ok && bus_all(b, "1Q%u", 1, 8, LZ) && bus_value(b, "2Q%u", 1, 8, 0xFF),
           "9 74_16373 bank 1 LE (and OE) do not move bank 2");
    bench_destroy(b);
}

static void check_75(void) {
    Bench *b = open_fx("74_75");
    int ok;

    set(b, "1C2C", L1);
    set(b, "3C4C", L1);
    set_bus(b, "%uD", 1, 4, 0x5);
    step(b);
    ok = bus_value(b, "%uQ", 1, 4, 0x5) && bus_value(b, "%uQBAR", 1, 4, 0xA);
    set(b, "3C4C", L0);
    step(b);
    set_bus(b, "%uD", 1, 4, 0xA);
    step(b);
    ok &= bus_value(b, "%uQ", 1, 4, 0x6) && bus_value(b, "%uQBAR", 1, 4, 0x9);
    set(b, "1C2C", L0);
    set(b, "3C4C", L1);
    step(b);
    set_bus(b, "%uD", 1, 4, 0x0);
    step(b);
    expect(ok && bus_value(b, "%uQ", 1, 4, 0x2) && bus_value(b, "%uQBAR", 1, 4, 0xD),
           "10 74_75 Q and QBAR complementary; pair enables independent");
    bench_destroy(b);
}

static void check_4042(void) {
    Bench *b = open_fx("4042B");
    int ok;

    set(b, "POLARITY", L0);
    set(b, "CLOCK", L0);
    set_bus(b, "D%u", 1, 4, 0x9);
    step(b);
    ok = bus_value(b, "Q%u", 1, 4, 0x9) && bus_value(b, "Q%uBAR", 1, 4, 0x6);
    set(b, "CLOCK", L1);
    step(b);
    set_bus(b, "D%u", 1, 4, 0x6);
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0x9);
    set(b, "POLARITY", L1);   /* CLOCK steady at 1: the latch opens */
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0x6);
    set(b, "CLOCK", L0);
    step(b);
    set_bus(b, "D%u", 1, 4, 0xF);
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0x6);
    set(b, "CLOCK", L1);
    step(b);
    ok &= bus_value(b, "Q%u", 1, 4, 0xF);
    set(b, "POLARITY", L0);   /* CLOCK steady at 1: the latch closes */
    step(b);
    set_bus(b, "D%u", 1, 4, 0x0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 1, 4, 0xF),
           "11 4042B POLARITY 0: transparent at CLOCK 0; POLARITY 1 at CLOCK 1; "
           "POLARITY change with CLOCK steady opens/closes");
    bench_destroy(b);
}

static void check_4508(void) {
    Bench *b = open_fx("4508B");
    int ok;

    set(b, "ODA", L0);
    set(b, "ODB", L0);
    set(b, "RESETA", L0);
    set(b, "RESETB", L0);
    set(b, "STROBEA", L1);
    set(b, "STROBEB", L1);
    set_bus(b, "D%uA", 0, 4, 0xF);
    set_bus(b, "D%uB", 0, 4, 0xF);
    step(b);
    set(b, "STROBEA", L0);
    set(b, "STROBEB", L0);
    step(b);
    set(b, "RESETA", L1);
    step(b);
    ok = bus_value(b, "Q%uA", 0, 4, 0x0) && bus_value(b, "Q%uB", 0, 4, 0xF);
    set(b, "RESETA", L0);
    step(b);
    ok &= bus_value(b, "Q%uA", 0, 4, 0x0);   /* the stored bits were cleared */
    set(b, "ODB", L1);
    step(b);
    ok &= bus_all(b, "Q%uB", 0, 4, LZ);
    set(b, "ODB", L0);
    step(b);
    expect(ok && bus_value(b, "Q%uB", 0, 4, 0xF),
           "12 4508B RESET high -> 0 and clears the stored bit; DISABLE high -> Z, storage intact");
    bench_destroy(b);
}

/* ---- addressable ---------------------------------------------------------- */

static void address(Bench *b, const char *fmt, unsigned a) {
    set_bus(b, fmt, 0, 3, a);
}

static void check_259(void) {
    Bench *b = open_fx("74_259");
    int ok = 1;
    unsigned i;

    set(b, "CLR", L1);
    set(b, "G", L1);
    set(b, "D", L0);
    address(b, "S%u", 0);
    step(b);
    for (i = 0; i < 8; i += 2) {   /* write 1 to latches 0, 2, 4, 6 */
        address(b, "S%u", i);
        step(b);
        set(b, "D", L1);
        set(b, "G", L0);
        step(b);
        set(b, "G", L1);
        step(b);
        set(b, "D", L0);
    }
    ok = bus_value(b, "Q%u", 0, 8, 0x55);
    address(b, "S%u", 3);
    set(b, "G", L0);
    set(b, "D", L1);
    step(b);
    ok &= bus_value(b, "Q%u", 0, 8, 0x5D);
    set(b, "D", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x55),
           "13 74_259 addressable mode: only the addressed latch follows D");

    set(b, "G", L1);
    step(b);
    set(b, "D", L1);
    address(b, "S%u", 7);
    step(b);
    expect(bus_value(b, "Q%u", 0, 8, 0x55), "14 74_259 memory mode: G high holds all");

    set(b, "G", L0);
    set(b, "CLR", L0);
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x80);
    set(b, "D", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x00),
           "15 74_259 demux mode: CLR low, G low -> addressed = D, others 0");

    set(b, "CLR", L1);
    set(b, "D", L1);
    step(b);
    set(b, "G", L1);
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x80);
    set(b, "CLR", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x00), "16 74_259 clear mode: CLR low, G high -> all 0");

    set(b, "CLR", L1);
    address(b, "S%u", 1);
    set(b, "D", L1);
    step(b);
    set(b, "G", L0);
    step(b);
    address(b, "S%u", 5);   /* G low: the transparent latch moves */
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x22);
    set(b, "D", L0);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x02),
           "17 74_259 address change with G low moves the transparent latch; the old one keeps "
           "its value");
    bench_destroy(b);
}

static void check_4099(void) {
    Bench *b = open_fx("4099B");
    int ok;

    set(b, "RESET", L0);
    set(b, "WD", L1);
    address(b, "A%u", 2);
    set(b, "DATA", L1);
    step(b);
    set(b, "WD", L0);
    step(b);
    set(b, "WD", L1);
    address(b, "A%u", 6);
    step(b);
    set(b, "WD", L0);
    step(b);
    set(b, "WD", L1);
    step(b);
    ok = bus_value(b, "Q%u", 0, 8, 0x44);
    set(b, "RESET", L1);
    step(b);
    ok &= bus_value(b, "Q%u", 0, 8, 0x00);
    address(b, "A%u", 1);
    set(b, "WD", L0);
    step(b);
    ok &= bus_value(b, "Q%u", 0, 8, 0x02);
    address(b, "A%u", 3);
    step(b);
    expect(ok && bus_value(b, "Q%u", 0, 8, 0x08),
           "18 4099B RESET high + WD high clears all; RESET high + WD low = demux");
    bench_destroy(b);
}

static void check_tpic6259(void) {
    Bench *b = open_fx("TPIC6259");
    LogicValue want[8] = {LZ, L0, LZ, LZ, LZ, LZ, LZ, L0};

    set(b, "CLR", L1);
    set(b, "G", L1);
    set(b, "D", L1);
    address(b, "S%u", 1);
    step(b);
    set(b, "G", L0);
    step(b);
    set(b, "G", L1);
    address(b, "S%u", 7);
    step(b);
    set(b, "G", L0);
    step(b);
    set(b, "G", L1);
    step(b);
    expect(bus_is(b, "DRAIN%u", 0, 8, want),
           "19 TPIC6259 bit 1 -> DRAIN 0; bit 0 -> DRAIN Z (open drain, inverted)");
    bench_destroy(b);
}

/* ---- sr ------------------------------------------------------------------- */

static void check_279(void) {
    Bench *b = open_fx("74_279");
    int ok;

    set(b, "1S1", L1);
    set(b, "1S2", L1);
    set(b, "1R", L1);
    step(b);
    ok = is(b, "1Q", L0);
    set(b, "1S2", L0);   /* set through the second S */
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1S2", L1);
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1R", L0);   /* reset */
    step(b);
    ok &= is(b, "1Q", L0);
    set(b, "1S1", L0);   /* both low -> 1 */
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1S1", L1);   /* release S first -> 0 */
    step(b);
    ok &= is(b, "1Q", L0);
    set(b, "1S1", L0);
    step(b);
    set(b, "1R", L1);   /* release R first -> 1 */
    step(b);
    ok &= is(b, "1Q", L1);
    set(b, "1S1", L1);
    step(b);
    expect(ok && is(b, "1Q", L1),
           "20 74_279 set, reset, both low -> 1, release S first -> 0, release R first -> 1");
    bench_destroy(b);
}

static void check_4043(void) {
    Bench *b = open_fx("4043B");
    int ok;

    set(b, "ENABLE", L1);
    set(b, "S1", L1);
    set(b, "R1", L1);
    set(b, "S2", L0);
    set(b, "R2", L0);
    step(b);
    ok = is(b, "Q1", L1) && is(b, "Q2", L0);
    set(b, "ENABLE", L0);
    set(b, "S1", L0);
    step(b);   /* R1 alone now resets latch 1 while the outputs are off */
    ok &= is(b, "Q1", LZ) && is(b, "Q2", LZ);
    set(b, "R1", L0);
    set(b, "S2", L1);
    step(b);
    set(b, "S2", L0);
    step(b);
    set(b, "ENABLE", L1);
    step(b);
    expect(ok && is(b, "Q1", L0) && is(b, "Q2", L1),
           "21 4043B S=R=1 -> 1; ENABLE 0 -> Z while S/R still act");
    bench_destroy(b);
}

static void check_4044(void) {
    Bench *b = open_fx("4044B");
    int ok;

    set(b, "ENABLE", L1);
    set(b, "S1", L0);
    set(b, "R1", L1);
    step(b);
    ok = is(b, "Q1", L1);
    set(b, "R1", L0);
    step(b);
    expect(ok && is(b, "Q1", L0), "22 4044B S=R=0 -> 0");
    bench_destroy(b);
}

/* ---- bcd7seg -------------------------------------------------------------- */

static const char *const SEGMENTS = "abcdefg";

static int segments_are(Bench *b, const char *glyph) {
    char name[2] = {0, 0};
    unsigned j;
    int ok = 1;

    for (j = 0; j < 7; j++) {
        name[0] = SEGMENTS[j];
        ok &= is(b, name, glyph[j] == '1' ? L1 : L0);
    }
    return ok;
}

static void check_4511(void) {
    static const char *const GLYPHS[16] = {
        "1111110", "0110000", "1101101", "1111001", "0110011", "1011011", "0011111", "1110000",
        "1111111", "1110011", "0000000", "0000000", "0000000", "0000000", "0000000", "0000000"
    };
    Bench *b = open_fx("74_4511");
    unsigned code;
    int ok = 1;

    set(b, "LE", L0);
    set(b, "LT", L1);
    set(b, "BL", L1);
    for (code = 0; code < 16; code++) {
        set_bus(b, "D%u", 0, 4, code);
        step(b);
        if (!segments_are(b, GLYPHS[code])) {
            printf("  code %u\n", code);
            ok = 0;
        }
    }
    set_bus(b, "D%u", 0, 4, 2);
    set(b, "LT", L0);
    step(b);
    ok &= segments_are(b, "1111111");
    set(b, "LT", L1);
    set(b, "BL", L0);
    step(b);
    ok &= segments_are(b, "0000000");
    set(b, "LT", L0);
    step(b);
    expect(ok && segments_are(b, "1111111"),
           "23 74_4511 codes 0-15 -> the glyph table; LT low -> all 1; BL low -> all 0; LT beats BL");

    set(b, "LT", L1);
    set(b, "BL", L1);
    set_bus(b, "D%u", 0, 4, 7);
    step(b);
    set(b, "LE", L1);
    step(b);
    set_bus(b, "D%u", 0, 4, 3);
    step(b);
    ok = segments_are(b, GLYPHS[7]);
    set(b, "LE", L0);
    step(b);
    expect(ok && segments_are(b, GLYPHS[3]), "24 74_4511 LE high holds the code while inputs change");
    bench_destroy(b);
}

/* ---- bus-mux -------------------------------------------------------------- */

static void check_16260(void) {
    Bench *b = open_fx("74_16260");
    int ok;

    set(b, "OEA", L1);
    set(b, "OE1B", L1);
    set(b, "OE2B", L1);
    set(b, "LE1B", L0);
    set(b, "LE2B", L0);
    set(b, "LEA2B", L0);
    set(b, "SEL", L0);
    set(b, "LEA1B", L1);
    set_bus(b, "A%u", 1, 12, 0xA5C);
    step(b);
    set(b, "LEA1B", L0);
    step(b);
    set_bus(b, "A%u", 1, 12, 0x000);
    step(b);
    ok = bus_all(b, "1B%u", 1, 12, LZ);
    set(b, "OE1B", L0);
    step(b);
    expect(ok && bus_value(b, "1B%u", 1, 12, 0xA5C) && bus_all(b, "2B%u", 1, 12, LZ),
           "25 74_16260 A->B: LEA1B high loads the 1B latch from A; OE1B low drives 1B");

    /* B -> A: release A and the B outputs, load 1B and 2B latches from the B pins. */
    set(b, "OE1B", L1);
    step(b);   /* the part releases 1B before the bench drives it */
    {
        char name[8];
        unsigned i;

        for (i = 1; i <= 12; i++) {
            snprintf(name, sizeof name, "A%u", i);
            set(b, name, LZ);
        }
    }
    set_bus(b, "1B%u", 1, 12, 0x123);
    set_bus(b, "2B%u", 1, 12, 0xFED);
    set(b, "LE1B", L1);
    set(b, "LE2B", L1);
    step(b);
    set(b, "LE1B", L0);
    set(b, "LE2B", L0);
    step(b);
    set(b, "OEA", L0);
    set(b, "SEL", L1);
    step(b);
    ok = bus_value(b, "A%u", 1, 12, 0x123);
    set(b, "SEL", L0);
    step(b);
    expect(ok && bus_value(b, "A%u", 1, 12, 0xFED) && bench_error(b) == NULL,
           "26 74_16260 B->A: SEL selects the 1B / 2B latch onto A when OEA low");
    bench_destroy(b);
}

/* ---- Real parts (plan-10-C section 4.3) ----------------------------------- */

static PartLibrary *real_library;

static Bench *open_real(const char *name) {
    const Part *part = part_library_find(real_library, name);

    if (part == NULL || part->kind != PART_LATCH) {
        fprintf(stderr, "no latch part %s\n", name);
        exit(1);
    }
    return bench_create(part);
}

/* Real parts have real delays (up to hundreds of ns): 1 us per step. */
static void slow(Bench *b) {
    bench_run(b, NS(1000));
}

static void check_real(void) {
    Bench *b;
    int ok;

    b = open_real("SN74HC373N");
    set(b, "OE", L0);
    set(b, "LE", L1);
    set_bus(b, "%uD", 1, 8, 0xA5);
    slow(b);
    ok = bus_value(b, "%uQ", 1, 8, 0xA5);
    set(b, "LE", L0);
    slow(b);
    set_bus(b, "%uD", 1, 8, 0x0F);
    slow(b);
    ok &= bus_value(b, "%uQ", 1, 8, 0xA5);
    set(b, "OE", L1);
    set(b, "LE", L1);
    slow(b);
    ok &= bus_all(b, "%uQ", 1, 8, LZ);
    set(b, "LE", L0);
    set(b, "OE", L0);
    slow(b);
    expect(ok && bus_value(b, "%uQ", 1, 8, 0x0F),
           "29 SN74HC373N: LE high follows D, LE low holds, OE high Z while loading");
    bench_destroy(b);

    b = open_real("SN74LVC1G373DBVR");
    set(b, "OE", L0);
    set(b, "LE", L1);
    set(b, "D", L1);
    slow(b);
    ok = is(b, "Q", L1);
    set(b, "LE", L0);
    slow(b);
    set(b, "D", L0);
    slow(b);
    expect(ok && is(b, "Q", L1), "30 SN74LVC1G373DBVR: transparent, then holds");
    bench_destroy(b);

    b = open_real("SN74HC259N");
    set(b, "CLR", L0);
    set(b, "G", L0);
    set(b, "D", L1);
    address(b, "S%u", 6);
    slow(b);
    expect(bus_value(b, "Q%u", 0, 8, 0x40), "31 SN74HC259N demux mode: addressed = D, others 0");
    bench_destroy(b);

    b = open_real("TPIC6B259N");
    {
        LogicValue want[8] = {LZ, LZ, LZ, L0, LZ, LZ, LZ, LZ};

        set(b, "CLR", L1);
        set(b, "G", L1);
        set(b, "D", L1);
        address(b, "S%u", 3);
        slow(b);
        set(b, "G", L0);
        slow(b);
        set(b, "G", L1);
        slow(b);
        expect(bus_is(b, "DRAIN%u", 0, 8, want),
               "32 TPIC6B259N: stored 1 pulls DRAIN low, stored 0 releases it");
    }
    bench_destroy(b);

    b = open_real("CD74HC4511E");
    set(b, "LE", L0);
    set(b, "LT", L1);
    set(b, "BL", L1);
    set_bus(b, "D%u", 0, 4, 7);
    slow(b);
    expect(segments_are(b, "1110000"), "33 CD74HC4511E: code 7 -> segments 1110000");
    bench_destroy(b);

    b = open_real("SN74ALVCH16260DL");
    set(b, "OEA", L1);
    set(b, "OE1B", L1);
    set(b, "OE2B", L1);
    set(b, "LEA1B", L0);
    set(b, "LEA2B", L0);
    set(b, "LE1B", L1);
    set(b, "LE2B", L1);
    set_bus(b, "1B%u", 1, 12, 0x5A3);
    set_bus(b, "2B%u", 1, 12, 0x0FF);
    slow(b);
    set(b, "LE1B", L0);
    set(b, "LE2B", L0);
    slow(b);
    set(b, "SEL", L1);
    set(b, "OEA", L0);
    slow(b);
    ok = bus_value(b, "A%u", 1, 12, 0x5A3);
    set(b, "SEL", L0);
    slow(b);
    expect(ok && bus_value(b, "A%u", 1, 12, 0x0FF) && bench_error(b) == NULL,
           "34 SN74ALVCH16260DL: SEL selects the 1B latch onto A (and the 2B latch when low)");
    bench_destroy(b);
}

/* ---- Loader and voltage --------------------------------------------------- */

static void expect_error(const char *what, const char *text, const char *want) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "bad.json");
    int ok = error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n  want: %s\n", error != NULL ? error : "(none)", want);
    }
    if (!ok) {
        printf("  %s\n", what);
        failures++;
    }
    free(error);
    part_library_destroy(lib);
}

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'latch','devices':[{'id':'bad'," \
    "'function':'Latch','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'D','role':'input'},{'number':2,'name':'LE','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'POL','role':'input'}]},"
#define BAD_TAIL "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static void check_loader(void) {
    int before = failures;
    int ok;

    expect_error("27a loader: unknown model", BAD_HEAD "'model':'nope'," BAD_TAIL,
                 "unknown model \"nope\"");
    expect_error("27b loader: a key of another model",
                 BAD_HEAD "'model':'transparent','units':[{'d':'D','le':'LE','q':'Q'}],"
                 "'enableActive':'high','address':['D']," BAD_TAIL,
                 "\"address\" is not a key of model \"transparent\"");
    expect_error("27c loader: enableActive with polarity",
                 BAD_HEAD "'model':'transparent','units':[{'d':'D','le':'LE','q':'Q'}],"
                 "'enableActive':'high','polarity':'POL'," BAD_TAIL,
                 "\"enableActive\" and \"polarity\" cannot both be given");
    ok = failures == before;
    failures = before;
    expect(ok,
           "27 loader: unknown model, another model's key, enableActive with polarity are errors");
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
    const Part *tpic = part_library_find(library, "FX-TPIC6259");
    const Part *ls = part_library_find(library, "FX-74_259");

    expect(tpic != NULL && ls != NULL && part_pin_open_drain(tpic, pin_index(tpic, "DRAIN0")) &&
               !part_pin_open_drain(ls, pin_index(ls, "Q0")) &&
               !part_pin_open_drain(tpic, pin_index(tpic, "D")),
           "28 voltage: FX-TPIC6259 DRAIN0 is open drain; FX-74_259 Q0 is not");
}

int main(void) {
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "latch_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_373();
    check_533();
    check_843();
    check_16373();
    check_75();
    check_4042();
    check_4508();
    check_259();
    check_4099();
    check_tpic6259();
    check_279();
    check_4043();
    check_4044();
    check_4511();
    check_16260();
    check_loader();
    check_open_drain();
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_real();
    part_library_destroy(real_library);
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("latch_test: all passed\n");
    return 0;
}
