/*
 * decoder_test: the decoder kind's three models (binary, priority, seg7)
 * against the function tables of the plan-19 devices (plan-19-B section 5).
 * Fixture devices copy the real devices' pinouts and model keys from
 * planning/plan-19-data/specs.md, with 10 ns delays; levels are checked
 * 50 ns after each step. fx-74_131 (clocked address) and fx-4056B (strobe
 * latch, display phase) are shapes with no part in the library.
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

/* Generated from planning/plan-19-data/specs.md (the spec JSON of each device). */
static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'decoder','devices':[{'id':'fx-74_138','f"
    "unction':'Decoder','description':'3-line to 8-line decoders/demultiplexers','drawer':[],'p"
    "inouts':{'p16':[{'number':1,'name':'A','role':'input'},{'number':2,'name':'B','role':'inpu"
    "t'},{'number':3,'name':'C','role':'input'},{'number':4,'name':'G2A','role':'input'},{'numb"
    "er':5,'name':'G2B','role':'input'},{'number':6,'name':'G1','role':'input'},{'number':7,'na"
    "me':'Y7','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'Y6"
    "','role':'output'},{'number':10,'name':'Y5','role':'output'},{'number':11,'name':'Y4','rol"
    "e':'output'},{'number':12,'name':'Y3','role':'output'},{'number':13,'name':'Y2','role':'ou"
    "tput'},{'number':14,'name':'Y1','role':'output'},{'number':15,'name':'Y0','role':'output'}"
    ",{'number':16,'name':'VCC','role':'power'}]},'model':'binary','units':[{'address':['A','B'"
    ",'C'],'outputs':['Y0','Y1','Y2','Y3','Y4','Y5','Y6','Y7'],'enables':[{'pin':'G1','active':"
    "'high'},{'pin':'G2A','active':'low'},{'pin':'G2B','active':'low'}]}],'outputsActive':'low'"
    ",'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_138','package':"
    "'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_139','function':'Dec"
    "oder','description':'Dual 2-line to 4-line decoders/demultiplexers','drawer':[],'pinouts':"
    "{'p16':[{'number':1,'name':'1G','role':'input'},{'number':2,'name':'1A','role':'input'},{'"
    "number':3,'name':'1B','role':'input'},{'number':4,'name':'1Y0','role':'output'},{'number':"
    "5,'name':'1Y1','role':'output'},{'number':6,'name':'1Y2','role':'output'},{'number':7,'nam"
    "e':'1Y3','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'2Y"
    "3','role':'output'},{'number':10,'name':'2Y2','role':'output'},{'number':11,'name':'2Y1','"
    "role':'output'},{'number':12,'name':'2Y0','role':'output'},{'number':13,'name':'2B','role'"
    ":'input'},{'number':14,'name':'2A','role':'input'},{'number':15,'name':'2G','role':'input'"
    "},{'number':16,'name':'VCC','role':'power'}]},'model':'binary','units':[{'address':['1A','"
    "1B'],'outputs':['1Y0','1Y1','1Y2','1Y3'],'enables':[{'pin':'1G','active':'low'}]},{'addres"
    "s':['2A','2B'],'outputs':['2Y0','2Y1','2Y2','2Y3'],'enables':[{'pin':'2G','active':'low'}]"
    "}],'outputsActive':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name"
    "':'FX-74_139','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx"
    "-74_42','function':'Decoder','description':'4-line BCD to 10-line decimal decoder','drawer"
    "':[],'pinouts':{'p16':[{'number':1,'name':'Y0','role':'output'},{'number':2,'name':'Y1','r"
    "ole':'output'},{'number':3,'name':'Y2','role':'output'},{'number':4,'name':'Y3','role':'ou"
    "tput'},{'number':5,'name':'Y4','role':'output'},{'number':6,'name':'Y5','role':'output'},{"
    "'number':7,'name':'Y6','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number"
    "':9,'name':'Y7','role':'output'},{'number':10,'name':'Y8','role':'output'},{'number':11,'n"
    "ame':'Y9','role':'output'},{'number':12,'name':'D','role':'input'},{'number':13,'name':'C'"
    ",'role':'input'},{'number':14,'name':'B','role':'input'},{'number':15,'name':'A','role':'i"
    "nput'},{'number':16,'name':'VCC','role':'power'}]},'model':'binary','units':[{'address':['"
    "A','B','C','D'],'outputs':['Y0','Y1','Y2','Y3','Y4','Y5','Y6','Y7','Y8','Y9']}],'outputsAc"
    "tive':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_42',"
    "'package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_137','func"
    "tion':'Decoder','description':'3-line to 8-line decoders/demultiplexers with address latch"
    "es','drawer':[],'pinouts':{'p16':[{'number':1,'name':'A','role':'input'},{'number':2,'name"
    "':'B','role':'input'},{'number':3,'name':'C','role':'input'},{'number':4,'name':'LE','role"
    "':'input'},{'number':5,'name':'G2','role':'input'},{'number':6,'name':'G1','role':'input'}"
    ",{'number':7,'name':'Y7','role':'output'},{'number':8,'name':'GND','role':'ground'},{'numb"
    "er':9,'name':'Y6','role':'output'},{'number':10,'name':'Y5','role':'output'},{'number':11,"
    "'name':'Y4','role':'output'},{'number':12,'name':'Y3','role':'output'},{'number':13,'name'"
    ":'Y2','role':'output'},{'number':14,'name':'Y1','role':'output'},{'number':15,'name':'Y0',"
    "'role':'output'},{'number':16,'name':'VCC','role':'power'}]},'model':'binary','units':[{'a"
    "ddress':['A','B','C'],'outputs':['Y0','Y1','Y2','Y3','Y4','Y5','Y6','Y7'],'enables':[{'pin"
    "':'G1','active':'high'},{'pin':'G2','active':'low'}],'latch':'LE'}],'outputsActive':'low',"
    "'latchActive':'high','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'F"
    "X-74_137','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-451"
    "4B','function':'Decoder','description':'4-bit latch/4-to-16 line decoder, output high on s"
    "elect','drawer':[],'pinouts':{'p24':[{'number':1,'name':'STROBE','role':'input'},{'number'"
    ":2,'name':'DATA1','role':'input'},{'number':3,'name':'DATA2','role':'input'},{'number':4,'"
    "name':'S7','role':'output'},{'number':5,'name':'S6','role':'output'},{'number':6,'name':'S"
    "5','role':'output'},{'number':7,'name':'S4','role':'output'},{'number':8,'name':'S3','role"
    "':'output'},{'number':9,'name':'S1','role':'output'},{'number':10,'name':'S2','role':'outp"
    "ut'},{'number':11,'name':'S0','role':'output'},{'number':12,'name':'VSS','role':'ground'},"
    "{'number':13,'name':'S13','role':'output'},{'number':14,'name':'S12','role':'output'},{'nu"
    "mber':15,'name':'S15','role':'output'},{'number':16,'name':'S14','role':'output'},{'number"
    "':17,'name':'S9','role':'output'},{'number':18,'name':'S8','role':'output'},{'number':19,'"
    "name':'S11','role':'output'},{'number':20,'name':'S10','role':'output'},{'number':21,'name"
    "':'DATA3','role':'input'},{'number':22,'name':'DATA4','role':'input'},{'number':23,'name':"
    "'INHIBIT','role':'input'},{'number':24,'name':'VDD','role':'power'}]},'model':'binary','un"
    "its':[{'address':['DATA1','DATA2','DATA3','DATA4'],'outputs':['S0','S1','S2','S3','S4','S5"
    "','S6','S7','S8','S9','S10','S11','S12','S13','S14','S15'],'enables':[{'pin':'INHIBIT','ac"
    "tive':'low'}],'latch':'STROBE'}],'outputsActive':'high','latchActive':'low','timing':{'HC'"
    ":{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4514B','package':'DIP','pinout':'"
    "p24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_1G18','function':'Decoder','descript"
    "ion':'1-of-2 noninverting demultiplexer with 3-state deselected output','drawer':[],'pinou"
    "ts':{'p6':[{'number':1,'name':'S','role':'input'},{'number':2,'name':'GND','role':'ground'"
    "},{'number':3,'name':'A','role':'input'},{'number':4,'name':'Y1','role':'output'},{'number"
    "':5,'name':'VCC','role':'power'},{'number':6,'name':'Y0','role':'output'}]},'model':'binar"
    "y','units':[{'address':['S'],'outputs':['Y0','Y1'],'enables':[{'pin':'A','active':'high'}]"
    "}],'outputsActive':'high','unselected':'z','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]"
    "}},'parts':[{'name':'FX-74_1G18','package':'DIP','pinout':'p6','footprint':'FX6','family':"
    "'HC'}]},{'id':'fx-74_145','function':'Decoder','description':'BCD-to-decimal decoder/drive"
    "r','drawer':[],'pinouts':{'p16':[{'number':1,'name':'Y0','role':'output'},{'number':2,'nam"
    "e':'Y1','role':'output'},{'number':3,'name':'Y2','role':'output'},{'number':4,'name':'Y3',"
    "'role':'output'},{'number':5,'name':'Y4','role':'output'},{'number':6,'name':'Y5','role':'"
    "output'},{'number':7,'name':'Y6','role':'output'},{'number':8,'name':'GND','role':'ground'"
    "},{'number':9,'name':'Y7','role':'output'},{'number':10,'name':'Y8','role':'output'},{'num"
    "ber':11,'name':'Y9','role':'output'},{'number':12,'name':'D','role':'input'},{'number':13,"
    "'name':'C','role':'input'},{'number':14,'name':'B','role':'input'},{'number':15,'name':'A'"
    ",'role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'binary','units':[{'a"
    "ddress':['A','B','C','D'],'outputs':['Y0','Y1','Y2','Y3','Y4','Y5','Y6','Y7','Y8','Y9']}],"
    "'outputsActive':'low','output':'open-drain','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6"
    "]}},'parts':[{'name':'FX-74_145','package':'DIP','pinout':'p16','footprint':'FX16','family"
    "':'HC'}]},{'id':'fx-74_2414','function':'Decoder','description':'Memory decoder with on-ch"
    "ip supply voltage monitor','drawer':[],'pinouts':{'p20':[{'number':1,'name':'VS','role':'o"
    "utput'},{'number':2,'name':'SD','role':'input'},{'number':3,'name':'1A','role':'input'},{'"
    "number':4,'name':'2A','role':'input'},{'number':5,'name':'1B','role':'input'},{'number':6,"
    "'name':'2B','role':'input'},{'number':7,'name':'1G','role':'input'},{'number':8,'name':'2G"
    "','role':'input'},{'number':9,'name':'G','role':'input'},{'number':10,'name':'GND','role':"
    "'ground'},{'number':11,'name':'2Y3','role':'output'},{'number':12,'name':'2Y2','role':'out"
    "put'},{'number':13,'name':'2Y1','role':'output'},{'number':14,'name':'2Y0','role':'output'"
    "},{'number':15,'name':'1Y3','role':'output'},{'number':16,'name':'1Y2','role':'output'},{'"
    "number':17,'name':'1Y1','role':'output'},{'number':18,'name':'1Y0','role':'output'},{'numb"
    "er':19,'name':'VBAT','role':'power'},{'number':20,'name':'VCC','role':'power'}]},'model':'"
    "binary','units':[{'address':['1A','1B'],'outputs':['1Y0','1Y1','1Y2','1Y3'],'enables':[{'p"
    "in':'G','active':'low'},{'pin':'1G','active':'low'},{'pin':'SD','active':'high'}]},{'addre"
    "ss':['2A','2B'],'outputs':['2Y0','2Y1','2Y2','2Y3'],'enables':[{'pin':'G','active':'low'},"
    "{'pin':'2G','active':'high'},{'pin':'SD','active':'high'}]}],'outputsActive':'low','monito"
    "r':'VS','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_2414','p"
    "ackage':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_1G29','funct"
    "ion':'Decoder','description':'2-of-3 decoder/demultiplexer','drawer':[],'pinouts':{'p8':[{"
    "'number':1,'name':'G','role':'input'},{'number':2,'name':'Y1','role':'output'},{'number':3"
    ",'name':'A0','role':'input'},{'number':4,'name':'GND','role':'ground'},{'number':5,'name':"
    "'Y2','role':'output'},{'number':6,'name':'A1','role':'input'},{'number':7,'name':'Y0','rol"
    "e':'output'},{'number':8,'name':'VCC','role':'power'}]},'model':'binary','units':[{'addres"
    "s':['A1'],'outputs':['Y0'],'enables':[{'pin':'G','active':'low'}]},{'address':['A0'],'outp"
    "uts':['Y1','Y2'],'enables':[{'pin':'G','active':'low'},{'pin':'A1','active':'high'}]}],'ou"
    "tputsActive':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-"
    "74_1G29','package':'DIP','pinout':'p8','footprint':'FX8','family':'HC'}]},{'id':'fx-4028B'"
    ",'function':'Decoder','description':'BCD-to-decimal decoder','drawer':[],'pinouts':{'p16':"
    "[{'number':1,'name':'Q4','role':'output'},{'number':2,'name':'Q2','role':'output'},{'numbe"
    "r':3,'name':'Q0','role':'output'},{'number':4,'name':'Q7','role':'output'},{'number':5,'na"
    "me':'Q9','role':'output'},{'number':6,'name':'Q5','role':'output'},{'number':7,'name':'Q6'"
    ",'role':'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'Q8','role'"
    ":'output'},{'number':10,'name':'A','role':'input'},{'number':11,'name':'D','role':'input'}"
    ",{'number':12,'name':'C','role':'input'},{'number':13,'name':'B','role':'input'},{'number'"
    ":14,'name':'Q1','role':'output'},{'number':15,'name':'Q3','role':'output'},{'number':16,'n"
    "ame':'VDD','role':'power'}]},'model':'binary','units':[{'address':['A','B','C','D'],'outpu"
    "ts':['Q0','Q1','Q2','Q3','Q4','Q5','Q6','Q7','Q8','Q9']}],'outputsActive':'high','timing':"
    "{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4028B','package':'DIP','pino"
    "ut':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_148','function':'Encoder','desc"
    "ription':'8-line to 3-line priority encoder','drawer':[],'pinouts':{'p16':[{'number':1,'na"
    "me':'I4','role':'input'},{'number':2,'name':'I5','role':'input'},{'number':3,'name':'I6','"
    "role':'input'},{'number':4,'name':'I7','role':'input'},{'number':5,'name':'EI','role':'inp"
    "ut'},{'number':6,'name':'A2','role':'output'},{'number':7,'name':'A1','role':'output'},{'n"
    "umber':8,'name':'GND','role':'ground'},{'number':9,'name':'A0','role':'output'},{'number':"
    "10,'name':'I0','role':'input'},{'number':11,'name':'I1','role':'input'},{'number':12,'name"
    "':'I2','role':'input'},{'number':13,'name':'I3','role':'input'},{'number':14,'name':'GS','"
    "role':'output'},{'number':15,'name':'EO','role':'output'},{'number':16,'name':'VCC','role'"
    ":'power'}]},'model':'priority','inputs':['I0','I1','I2','I3','I4','I5','I6','I7'],'inputsA"
    "ctive':'low','code':['A0','A1','A2'],'codeActive':'low','enableIn':{'pin':'EI','active':'l"
    "ow'},'groupSelect':{'pin':'GS','active':'low'},'enableOut':{'pin':'EO','active':'low'},'ti"
    "ming':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_148','package':'DIP"
    "','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_348','function':'Encoder"
    "','description':'8-line to 3-line priority encoder with 3-state outputs','drawer':[],'pino"
    "uts':{'p16':[{'number':1,'name':'I4','role':'input'},{'number':2,'name':'I5','role':'input"
    "'},{'number':3,'name':'I6','role':'input'},{'number':4,'name':'I7','role':'input'},{'numbe"
    "r':5,'name':'EI','role':'input'},{'number':6,'name':'A2','role':'output'},{'number':7,'nam"
    "e':'A1','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'A0'"
    ",'role':'output'},{'number':10,'name':'I0','role':'input'},{'number':11,'name':'I1','role'"
    ":'input'},{'number':12,'name':'I2','role':'input'},{'number':13,'name':'I3','role':'input'"
    "},{'number':14,'name':'GS','role':'output'},{'number':15,'name':'EO','role':'output'},{'nu"
    "mber':16,'name':'VCC','role':'power'}]},'model':'priority','inputs':['I0','I1','I2','I3','"
    "I4','I5','I6','I7'],'inputsActive':'low','code':['A0','A1','A2'],'codeActive':'low','enabl"
    "eIn':{'pin':'EI','active':'low'},'groupSelect':{'pin':'GS','active':'low'},'enableOut':{'p"
    "in':'EO','active':'low'},'disabled':'z','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},"
    "'parts':[{'name':'FX-74_348','package':'DIP','pinout':'p16','footprint':'FX16','family':'H"
    "C'}]},{'id':'fx-74_147','function':'Encoder','description':'10- to 4-line priority encoder"
    "','drawer':[],'pinouts':{'p16':[{'number':1,'name':'I4','role':'input'},{'number':2,'name'"
    ":'I5','role':'input'},{'number':3,'name':'I6','role':'input'},{'number':4,'name':'I7','rol"
    "e':'input'},{'number':5,'name':'I8','role':'input'},{'number':6,'name':'Y2','role':'output"
    "'},{'number':7,'name':'Y1','role':'output'},{'number':8,'name':'GND','role':'ground'},{'nu"
    "mber':9,'name':'Y0','role':'output'},{'number':10,'name':'I9','role':'input'},{'number':11"
    ",'name':'I1','role':'input'},{'number':12,'name':'I2','role':'input'},{'number':13,'name':"
    "'I3','role':'input'},{'number':14,'name':'Y3','role':'output'},{'number':15,'name':'NC','r"
    "ole':'nc'},{'number':16,'name':'VCC','role':'power'}]},'model':'priority','inputs':['I1','"
    "I2','I3','I4','I5','I6','I7','I8','I9'],'inputsActive':'low','code':['Y0','Y1','Y2','Y3'],"
    "'codeActive':'low','offset':1,'idle':0,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'"
    "parts':[{'name':'FX-74_147','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC"
    "'}]},{'id':'fx-40147B','function':'Encoder','description':'10-line to 4-line BCD priority "
    "encoder','drawer':[],'pinouts':{'p16':[{'number':1,'name':'I4','role':'input'},{'number':2"
    ",'name':'I5','role':'input'},{'number':3,'name':'I6','role':'input'},{'number':4,'name':'I"
    "7','role':'input'},{'number':5,'name':'I8','role':'input'},{'number':6,'name':'C','role':'"
    "output'},{'number':7,'name':'B','role':'output'},{'number':8,'name':'VSS','role':'ground'}"
    ",{'number':9,'name':'A','role':'output'},{'number':10,'name':'I9','role':'input'},{'number"
    "':11,'name':'I1','role':'input'},{'number':12,'name':'I2','role':'input'},{'number':13,'na"
    "me':'I3','role':'input'},{'number':14,'name':'D','role':'output'},{'number':15,'name':'I0'"
    ",'role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'priority','inputs':["
    "'I0','I1','I2','I3','I4','I5','I6','I7','I8','I9'],'inputsActive':'low','code':['A','B','C"
    "','D'],'codeActive':'low','offset':0,'idle':15,'timing':{'HC':{'rise':10,'fall':10,'vcc':["
    "2,6]}},'parts':[{'name':'FX-40147B','package':'DIP','pinout':'p16','footprint':'FX16','fam"
    "ily':'HC'}]},{'id':'fx-4532B','function':'Encoder','description':'8-bit priority encoder',"
    "'drawer':[],'pinouts':{'p16':[{'number':1,'name':'D4','role':'input'},{'number':2,'name':'"
    "D5','role':'input'},{'number':3,'name':'D6','role':'input'},{'number':4,'name':'D7','role'"
    ":'input'},{'number':5,'name':'EI','role':'input'},{'number':6,'name':'Q2','role':'output'}"
    ",{'number':7,'name':'Q1','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'numb"
    "er':9,'name':'Q0','role':'output'},{'number':10,'name':'D0','role':'input'},{'number':11,'"
    "name':'D1','role':'input'},{'number':12,'name':'D2','role':'input'},{'number':13,'name':'D"
    "3','role':'input'},{'number':14,'name':'GS','role':'output'},{'number':15,'name':'EO','rol"
    "e':'output'},{'number':16,'name':'VDD','role':'power'}]},'model':'priority','inputs':['D0'"
    ",'D1','D2','D3','D4','D5','D6','D7'],'inputsActive':'high','code':['Q0','Q1','Q2'],'codeAc"
    "tive':'high','enableIn':{'pin':'EI','active':'high'},'groupSelect':{'pin':'GS','active':'h"
    "igh'},'enableOut':{'pin':'EO','active':'high'},'timing':{'HC':{'rise':10,'fall':10,'vcc':["
    "2,6]}},'parts':[{'name':'FX-4532B','package':'DIP','pinout':'p16','footprint':'FX16','fami"
    "ly':'HC'}]},{'id':'fx-74_47','function':'7-segment decoder','description':'BCD-to-seven-se"
    "gment decoder/driver, open-collector active-low outputs','drawer':[],'pinouts':{'p16':[{'n"
    "umber':1,'name':'B','role':'input'},{'number':2,'name':'C','role':'input'},{'number':3,'na"
    "me':'LT','role':'input'},{'number':4,'name':'BI/RBO','role':'inout'},{'number':5,'name':'R"
    "BI','role':'input'},{'number':6,'name':'D','role':'input'},{'number':7,'name':'A','role':'"
    "input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'e','role':'output'},"
    "{'number':10,'name':'d','role':'output'},{'number':11,'name':'c','role':'output'},{'number"
    "':12,'name':'b','role':'output'},{'number':13,'name':'a','role':'output'},{'number':14,'na"
    "me':'g','role':'output'},{'number':15,'name':'f','role':'output'},{'number':16,'name':'VCC"
    "','role':'power'}]},'model':'seg7','inputs':['A','B','C','D'],'segments':['a','b','c','d',"
    "'e','f','g'],'segmentsActive':'low','glyphs':['1111110','0110000','1101101','1111001','011"
    "0011','1011011','0011111','1110000','1111111','1110011','0001101','0011001','0100011','100"
    "1011','0001111','0000000'],'lampTest':{'pin':'LT','active':'low'},'rbi':'RBI','rbo':'BI/RB"
    "O','output':'open-drain','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name"
    "':'FX-74_47','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-"
    "74_131','function':'Decoder','description':'d','drawer':[],'pinouts':{'p16':[{'number':1,'"
    "name':'A','role':'input'},{'number':2,'name':'B','role':'input'},{'number':3,'name':'C','r"
    "ole':'input'},{'number':4,'name':'CLK','role':'input'},{'number':5,'name':'G2','role':'inp"
    "ut'},{'number':6,'name':'G1','role':'input'},{'number':7,'name':'Y7','role':'output'},{'nu"
    "mber':8,'name':'GND','role':'ground'},{'number':9,'name':'Y6','role':'output'},{'number':1"
    "0,'name':'Y5','role':'output'},{'number':11,'name':'Y4','role':'output'},{'number':12,'nam"
    "e':'Y3','role':'output'},{'number':13,'name':'Y2','role':'output'},{'number':14,'name':'Y1"
    "','role':'output'},{'number':15,'name':'Y0','role':'output'},{'number':16,'name':'VCC','ro"
    "le':'power'}]},'model':'binary','units':[{'address':['A','B','C'],'outputs':['Y0','Y1','Y2"
    "','Y3','Y4','Y5','Y6','Y7'],'enables':[{'pin':'G1','active':'high'},{'pin':'G2','active':'"
    "low'}],'clock':'CLK'}],'outputsActive':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,"
    "6]}},'parts':[{'name':'FX-74_131','package':'DIP','pinout':'p16','footprint':'FX16','famil"
    "y':'HC'}]},{'id':'fx-4056B','function':'7-segment decoder','description':'d','drawer':[],'"
    "pinouts':{'p16':[{'number':1,'name':'ST','role':'input'},{'number':2,'name':'C','role':'in"
    "put'},{'number':3,'name':'B','role':'input'},{'number':4,'name':'D','role':'input'},{'numb"
    "er':5,'name':'A','role':'input'},{'number':6,'name':'DF','role':'input'},{'number':7,'name"
    "':'VEE','role':'power'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'a','"
    "role':'output'},{'number':10,'name':'b','role':'output'},{'number':11,'name':'c','role':'o"
    "utput'},{'number':12,'name':'d','role':'output'},{'number':13,'name':'e','role':'output'},"
    "{'number':14,'name':'g','role':'output'},{'number':15,'name':'f','role':'output'},{'number"
    "':16,'name':'VCC','role':'power'}]},'model':'seg7','inputs':['A','B','C','D'],'segments':["
    "'a','b','c','d','e','f','g'],'segmentsActive':'high','glyphs':['1111110','0110000','110110"
    "1','1111001','0110011','1011011','0011111','1110000','1111111','1110011','0001101','001100"
    "1','0100011','1001011','0001111','0000000'],'latch':{'pin':'ST','active':'low'},'phase':'D"
    "F','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4056B','package'"
    ":'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]}]}";

/* Dual-row fixture footprints FX<n>, one per pin count above. */
#define FX_FOOTPRINT(N) \
    "{'id':'FX" #N "','description':'d','style':'dual','pins':" #N ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[" FX_FOOTPRINT(6) ","
    FX_FOOTPRINT(8) "," FX_FOOTPRINT(14) "," FX_FOOTPRINT(16) "," FX_FOOTPRINT(20) ","
    FX_FOOTPRINT(24) "]}";

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
    if (part == NULL || part->kind != PART_DECODER) {
        fprintf(stderr, "no decoder part %s\n", name);
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

/* Sets the named pins (LSB first) from value's bits. */
static void set_pins(Bench *b, const char *const *pins, unsigned n, unsigned value) {
    unsigned i;

    for (i = 0; i < n; i++) {
        set(b, pins[i], bit(value, i));
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

/* Every pin fmt % i (i < n) shows: `active` at index sel, else `other`. */
static int one_of(Bench *b, const char *fmt, unsigned n, int sel, LogicValue active,
                  LogicValue other) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, i);
        ok &= is(b, name, (int)i == sel ? active : other);
    }
    return ok;
}

/* Pins named by want: 'H' 1, 'L' 0, 'Z', 'X'; ' ' skips. */
static int pins_are(Bench *b, const char *const *pins, const char *want) {
    int ok = 1;
    size_t i;

    for (i = 0; pins[i] != NULL; i++) {
        LogicValue v = want[i] == 'H' ? L1 : want[i] == 'L' ? L0 : want[i] == 'Z' ? LZ : LX;

        if (want[i] != ' ') {
            ok &= is(b, pins[i], v);
        }
    }
    return ok;
}

/* ---- binary --------------------------------------------------------------- */

static const char *const ABC[] = {"A", "B", "C", "D"};

static void check_138(void) {
    Bench *b = open_fx("74_138");
    unsigned code;
    int ok = 1;

    set(b, "G1", L1);
    set(b, "G2A", L0);
    set(b, "G2B", L0);
    for (code = 0; code < 8; code++) {
        set_pins(b, ABC, 3, code);
        step(b);
        ok &= one_of(b, "Y%u", 8, (int)code, L0, L1);
    }
    expect(ok, "1 '138 enabled: code n makes Yn low, every other Y high (8 codes)");

    set_pins(b, ABC, 3, 5);
    set(b, "G1", L0);
    step(b);
    ok = one_of(b, "Y%u", 8, -1, L0, L1);
    set(b, "G1", L1);
    set(b, "G2A", L1);
    step(b);
    ok &= one_of(b, "Y%u", 8, -1, L0, L1);
    set(b, "G2A", L0);
    set(b, "G2B", L1);
    step(b);
    ok &= one_of(b, "Y%u", 8, -1, L0, L1);
    expect(ok, "2 '138: G1 low, G2A high or G2B high alone makes every Y high");

    set(b, "G2B", L0);
    set(b, "G1", LX);
    step(b);
    expect(one_of(b, "Y%u", 8, -1, L0, LX), "3 '138: X on G1 makes every Y X");

    set(b, "G1", L1);
    set(b, "A", L0);
    set(b, "B", LX);
    set(b, "C", L0);
    step(b);
    ok = is(b, "Y0", LX) && is(b, "Y2", LX);
    ok &= is(b, "Y1", L1) && is(b, "Y3", L1) && is(b, "Y4", L1) && is(b, "Y5", L1) &&
          is(b, "Y6", L1) && is(b, "Y7", L1);
    set(b, "B", LZ);
    step(b);
    ok &= is(b, "Y0", LX) && is(b, "Y2", LX) && is(b, "Y1", L1);
    expect(ok, "4 '138: X (or Z) on B with A = C = 0: only Y0 and Y2 X");
    bench_destroy(b);
}

static void check_139(void) {
    Bench *b = open_fx("74_139");
    static const char *const A1[] = {"1A", "1B"};
    static const char *const A2[] = {"2A", "2B"};
    int ok;

    set(b, "1G", L0);
    set(b, "2G", L1);
    set_pins(b, A1, 2, 2);
    set_pins(b, A2, 2, 1);
    step(b);
    ok = one_of(b, "1Y%u", 4, 2, L0, L1) && one_of(b, "2Y%u", 4, -1, L0, L1);
    set(b, "1G", L1);
    set(b, "2G", L0);
    step(b);
    ok &= one_of(b, "1Y%u", 4, -1, L0, L1) && one_of(b, "2Y%u", 4, 1, L0, L1);
    set_pins(b, A1, 2, 3);
    step(b);
    ok &= one_of(b, "2Y%u", 4, 1, L0, L1);
    expect(ok, "5 '139: two independent units, each with its own address and enable");
    bench_destroy(b);
}

static void check_42(void) {
    Bench *b = open_fx("74_42");
    unsigned code;
    int ok = 1;

    for (code = 0; code < 16; code++) {
        set_pins(b, ABC, 4, code);
        step(b);
        ok &= one_of(b, "Y%u", 10, code < 10 ? (int)code : -1, L0, L1);
    }
    expect(ok, "6 '42: codes 0-9 select Y0-Y9 (low), codes 10-15 leave every Y high");
    bench_destroy(b);
}

static void check_4028(void) {
    Bench *b = open_fx("4028B");
    unsigned code;
    int ok = 1;

    for (code = 0; code < 16; code++) {
        set_pins(b, ABC, 4, code);
        step(b);
        ok &= one_of(b, "Q%u", 10, code < 10 ? (int)code : -1, L1, L0);
    }
    expect(ok, "7 4028B: codes 0-9 select Q0-Q9 (high), codes 10-15 leave every Q low");
    bench_destroy(b);
}

static void check_137(void) {
    Bench *b = open_fx("74_137");
    int ok;

    set(b, "G1", L1);
    set(b, "G2", L0);
    set(b, "LE", L0);
    set_pins(b, ABC, 3, 3);
    step(b);
    ok = one_of(b, "Y%u", 8, 3, L0, L1);
    set_pins(b, ABC, 3, 6);
    step(b);
    ok &= one_of(b, "Y%u", 8, 6, L0, L1);
    expect(ok, "8 '137: LE low, the address passes (transparent)");

    set(b, "LE", L1);
    step(b);
    set_pins(b, ABC, 3, 1);
    step(b);
    ok = one_of(b, "Y%u", 8, 6, L0, L1);
    set(b, "G1", L0);
    step(b);
    ok &= one_of(b, "Y%u", 8, -1, L0, L1);
    set(b, "G1", L1);
    step(b);
    ok &= one_of(b, "Y%u", 8, 6, L0, L1);
    set(b, "LE", L0);
    step(b);
    ok &= one_of(b, "Y%u", 8, 1, L0, L1);
    expect(ok, "9 '137: LE high holds the address across a change; enables still act");
    bench_destroy(b);
}

static void check_131(void) {
    Bench *b = open_fx("74_131");
    int ok;

    set(b, "G1", L1);
    set(b, "G2", L0);
    set(b, "CLK", L0);
    set_pins(b, ABC, 3, 5);
    step(b);
    ok = one_of(b, "Y%u", 8, 0, L0, L1);   /* power-on address 0 */
    set(b, "CLK", L1);
    step(b);
    ok &= one_of(b, "Y%u", 8, 5, L0, L1);
    set_pins(b, ABC, 3, 2);
    step(b);
    ok &= one_of(b, "Y%u", 8, 5, L0, L1);
    set(b, "CLK", L0);
    step(b);
    ok &= one_of(b, "Y%u", 8, 5, L0, L1);
    set(b, "CLK", L1);
    step(b);
    ok &= one_of(b, "Y%u", 8, 2, L0, L1);
    expect(ok, "10 '131: the address is registered on the rising clock edge only");
    bench_destroy(b);
}

static void check_4514(void) {
    Bench *b = open_fx("4514B");
    static const char *const DATA[] = {"DATA1", "DATA2", "DATA3", "DATA4"};
    int ok;

    set(b, "INHIBIT", L0);
    set(b, "STROBE", L1);
    set_pins(b, DATA, 4, 12);
    step(b);
    ok = one_of(b, "S%u", 16, 12, L1, L0);
    set(b, "STROBE", L0);
    step(b);
    set_pins(b, DATA, 4, 3);
    step(b);
    ok &= one_of(b, "S%u", 16, 12, L1, L0);
    set(b, "INHIBIT", L1);
    step(b);
    ok &= one_of(b, "S%u", 16, -1, L1, L0);
    set(b, "INHIBIT", L0);
    set(b, "STROBE", L1);
    step(b);
    ok &= one_of(b, "S%u", 16, 3, L1, L0);
    expect(ok, "11 4514B: STROBE high passes, low holds; INHIBIT high makes every S low");
    bench_destroy(b);
}

static void check_1g18(void) {
    Bench *b = open_fx("74_1G18");
    int ok;

    set(b, "S", L0);
    set(b, "A", L1);
    step(b);
    ok = is(b, "Y0", L1) && is(b, "Y1", LZ);
    set(b, "A", L0);
    step(b);
    ok &= is(b, "Y0", L0) && is(b, "Y1", LZ);
    set(b, "S", L1);
    step(b);
    ok &= is(b, "Y0", LZ) && is(b, "Y1", L0);
    set(b, "A", L1);
    step(b);
    ok &= is(b, "Y0", LZ) && is(b, "Y1", L1);
    set(b, "A", LX);
    step(b);
    ok &= is(b, "Y0", LZ) && is(b, "Y1", LX);
    expect(ok, "12 '1G18: the selected output follows A, the other is Z");
    bench_destroy(b);
}

static void check_1g29(void) {
    Bench *b = open_fx("74_1G29");
    static const char *const Y[] = {"Y0", "Y1", "Y2", NULL};
    static const char *const ROWS[5][2] = {
        {"HXX", "HHH"}, {"LLL", "LHH"}, {"LLH", "LHH"}, {"LHL", "HLH"}, {"LHH", "HHL"},
    };
    int ok = 1;
    size_t r;

    /* Rows: G A1 A0 -> Y0 Y1 Y2 (sn74lvc1g29 function table). */
    for (r = 0; r < 5; r++) {
        const char *in = ROWS[r][0];

        set(b, "G", in[0] == 'H' ? L1 : L0);
        set(b, "A1", in[1] == 'H' ? L1 : L0);
        set(b, "A0", in[2] == 'H' ? L1 : L0);
        step(b);
        ok &= pins_are(b, Y, ROWS[r][1]);
    }
    expect(ok, "13 '1G29: every function-table row (A1 low selects Y0 whatever A0)");
    bench_destroy(b);
}

static void check_145(void) {
    const Part *part = fx_part("74_145");
    Bench *b = bench_create(part);
    int ok;

    set_pins(b, ABC, 4, 7);
    step(b);
    ok = one_of(b, "Y%u", 10, 7, L0, LZ);
    set_pins(b, ABC, 4, 12);
    step(b);
    ok &= one_of(b, "Y%u", 10, -1, L0, LZ);
    ok &= part_pin_open_drain(part, pin_index(part, "Y9")) == 1 &&
          part_pin_open_drain(part, pin_index(part, "A")) == 0;
    expect(ok, "14 '145 open collector: the selected Y 0, the others Z; part_pin_open_drain");
    bench_destroy(b);
}

static void check_2414(void) {
    const Part *part = fx_part("74_2414");
    Bench *b = bench_create(part);
    static const char *const A1[] = {"1A", "1B"};
    static const char *const A2[] = {"2A", "2B"};
    int ok;

    set(b, "G", L0);
    set(b, "SD", L1);
    set(b, "1G", L0);
    set(b, "2G", L1);
    set_pins(b, A1, 2, 1);
    set_pins(b, A2, 2, 3);
    step(b);
    ok = one_of(b, "1Y%u", 4, 1, L0, L1) && one_of(b, "2Y%u", 4, 3, L0, L1);
    set(b, "2G", L0);
    step(b);
    ok &= one_of(b, "1Y%u", 4, 1, L0, L1) && one_of(b, "2Y%u", 4, -1, L0, L1);
    set(b, "SD", L0);
    step(b);
    ok &= one_of(b, "1Y%u", 4, -1, L0, L1);
    set(b, "SD", L1);
    set(b, "G", L1);
    step(b);
    ok &= one_of(b, "1Y%u", 4, -1, L0, L1);
    ok &= is(b, "VS", L0) && part_pin_open_drain(part, pin_index(part, "VS")) == 1 &&
          part_pin_open_drain(part, pin_index(part, "1Y0")) == 0;
    expect(ok, "15 SN74BCT2414: dual decode gated by G, SD and 1G/2G; VS low (open collector)");
    bench_destroy(b);
}

/* ---- priority ------------------------------------------------------------- */

/* Each row: the inputs (as printed, H/L/X), then the outputs. X inputs are
   driven low, which must not change the result (a higher input wins). */
static int rows_ok(Bench *b, const char *const *in, const char *const *out, const char *const *rows,
                   size_t count) {
    int ok = 1;
    size_t r, i;

    for (r = 0; r < count; r++) {
        const char *row = rows[2 * r];

        for (i = 0; in[i] != NULL; i++) {
            set(b, in[i], row[i] == 'H' ? L1 : L0);
        }
        step(b);
        if (!pins_are(b, out, rows[2 * r + 1])) {
            printf("  row %zu (%s)\n", r, row);
            ok = 0;
        }
    }
    return ok;
}

static const char *const IN148[] = {"EI", "I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7", NULL};
static const char *const OUT148[] = {"A2", "A1", "A0", "GS", "EO", NULL};

/* SN74LS148 function table: EI 0..7 -> A2 A1 A0 GS EO. */
static const char *const ROWS148[] = {
    "HXXXXXXXX", "HHHHH",
    "LHHHHHHHH", "HHHHL",
    "LXXXXXXXL", "LLLLH",
    "LXXXXXXLH", "LLHLH",
    "LXXXXXLHH", "LHLLH",
    "LXXXXLHHH", "LHHLH",
    "LXXXLHHHH", "HLLLH",
    "LXXLHHHHH", "HLHLH",
    "LXLHHHHHH", "HHLLH",
    "LLHHHHHHH", "HHHLH",
};

static void check_148(void) {
    Bench *b = open_fx("74_148");

    expect(rows_ok(b, IN148, OUT148, ROWS148, 10), "16 '148: every function-table row (10)");
    bench_destroy(b);
}

static void check_348(void) {
    static const char *const ROWS348[] = {
        "HXXXXXXXX", "ZZZHH",
        "LHHHHHHHH", "ZZZHL",
        "LXXXXXXXL", "LLLLH",
        "LXXXXLHHH", "LHHLH",
        "LLHHHHHHH", "HHHLH",
    };
    Bench *b = open_fx("74_348");

    expect(rows_ok(b, IN148, OUT148, ROWS348, 5),
           "17 '348: code outputs Z when disabled or idle; GS and EO driven");
    bench_destroy(b);
}

static void check_147(void) {
    static const char *const IN[] = {"I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8", "I9", NULL};
    static const char *const OUT[] = {"Y3", "Y2", "Y1", "Y0", NULL};
    /* CD74HC147 truth table: 1..9 -> D C B A (Y3..Y0), active low. */
    static const char *const ROWS[] = {
        "HHHHHHHHH", "HHHH",
        "XXXXXXXXL", "LHHL",
        "XXXXXXXLH", "LHHH",
        "XXXXXXLHH", "HLLL",
        "XXXXXLHHH", "HLLH",
        "XXXXLHHHH", "HLHL",
        "XXXLHHHHH", "HLHH",
        "XXLHHHHHH", "HHLL",
        "XLHHHHHHH", "HHLH",
        "LHHHHHHHH", "HHHL",
    };
    Bench *b = open_fx("74_147");

    expect(rows_ok(b, IN, OUT, ROWS, 10), "18 '147: every truth-table row (offset 1, idle all high)");
    bench_destroy(b);
}

static void check_40147(void) {
    static const char *const IN[] = {"I0", "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8", "I9", NULL};
    static const char *const OUT[] = {"D", "C", "B", "A", NULL};
    static const char *const ROWS[] = {
        "HHHHHHHHHH", "LLLL",
        "LHHHHHHHHH", "HHHH",
        "XXXXXLHHHH", "HLHL",
        "XXXXXXXXXL", "LHHL",
    };
    Bench *b = open_fx("40147B");

    expect(rows_ok(b, IN, OUT, ROWS, 4), "19 40147B: idle 15 (all outputs low), input n -> NOT n");
    bench_destroy(b);
}

static void check_4532(void) {
    static const char *const IN[] = {"EI", "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", NULL};
    static const char *const OUT[] = {"Q2", "Q1", "Q0", "GS", "EO", NULL};
    static const char *const ROWS[] = {
        "LHHHHHHHH", "LLLLL",
        "HLLLLLLLL", "LLLLH",
        "HXXXXXXXH", "HHHHL",
        "HXXXHLLLL", "LHHHL",
        "HHLLLLLLL", "LLLHL",
    };
    Bench *b = open_fx("4532B");
    int ok = rows_ok(b, IN, OUT, ROWS, 5);

    set(b, "EI", L1);
    set(b, "D7", L0);
    set(b, "D5", LX);
    set(b, "D3", L1);
    step(b);
    /* D3 and D5 (X) compete: 3 = 011, 5 = 101; Q0 is 1 either way. */
    ok &= is(b, "Q2", LX) && is(b, "Q1", LX) && is(b, "Q0", L1) && is(b, "GS", L1) &&
          is(b, "EO", L0);
    set(b, "EI", LX);
    step(b);
    ok &= is(b, "Q0", LX) && is(b, "GS", LX) && is(b, "EO", LX);
    expect(ok, "20 4532B: active-high rows; X input above the winner makes the differing bits X; X EI");
    bench_destroy(b);
}

/* ---- seg7 ----------------------------------------------------------------- */

static const char *const SEGS[] = {"a", "b", "c", "d", "e", "f", "g", NULL};

/* glyph: 7 chars a..g, '1' = segment on; active: the pin level of "on". */
static int glyph_is(Bench *b, const char *glyph, LogicValue on, LogicValue off) {
    int ok = 1;
    size_t j;

    for (j = 0; j < 7; j++) {
        ok &= is(b, SEGS[j], glyph[j] == '1' ? on : off);
    }
    return ok;
}

static const char *const GLYPHS47[16] = {
    "1111110", "0110000", "1101101", "1111001", "0110011", "1011011", "0011111", "1110000",
    "1111111", "1110011", "0001101", "0011001", "0100011", "1001011", "0001111", "0000000",
};

static void check_47(void) {
    const Part *part = fx_part("74_47");
    Bench *b = bench_create(part);
    unsigned code;
    int ok = 1;

    set(b, "LT", L1);
    set(b, "RBI", L1);
    for (code = 0; code < 16; code++) {
        set_pins(b, ABC, 4, code);
        step(b);
        ok &= glyph_is(b, GLYPHS47[code], L0, LZ) && is(b, "BI/RBO", LZ);
    }
    expect(ok, "21 SN7447A: codes 0-15 show the datasheet glyphs (on = low, off = released)");

    set_pins(b, ABC, 4, 5);
    set(b, "LT", L0);
    step(b);
    ok = glyph_is(b, "1111111", L0, LZ);
    set(b, "BI/RBO", L0);
    step(b);
    ok &= glyph_is(b, "0000000", L0, LZ);
    expect(ok, "22 SN7447A: LT low lights every segment; BI low blanks even over LT");

    set(b, "BI/RBO", LZ);
    set(b, "LT", L1);
    set(b, "RBI", L0);
    set_pins(b, ABC, 4, 0);
    step(b);
    ok = glyph_is(b, "0000000", L0, LZ) && is(b, "BI/RBO", L0);
    set_pins(b, ABC, 4, 5);
    step(b);
    ok &= glyph_is(b, GLYPHS47[5], L0, LZ) && is(b, "BI/RBO", LZ);
    set(b, "LT", L0);
    set_pins(b, ABC, 4, 0);
    step(b);
    ok &= glyph_is(b, "1111111", L0, LZ) && is(b, "BI/RBO", LZ);
    expect(ok, "23 SN7447A: RBI low with code 0 blanks and pulls RBO low; code 5 or LT releases it");

    expect(part_pin_open_drain(part, pin_index(part, "BI/RBO")) == 1 &&
               part_pin_open_drain(part, pin_index(part, "a")) == 1 &&
               part_pin_open_drain(part, pin_index(part, "LT")) == 0,
           "24 SN7447A: BI/RBO and the segments are open drain (part_pin_open_drain)");
    bench_destroy(b);
}

static void check_4056(void) {
    Bench *b = open_fx("4056B");
    int ok;

    set(b, "DF", L0);
    set(b, "ST", L1);
    set_pins(b, ABC, 4, 2);
    step(b);
    ok = glyph_is(b, GLYPHS47[2], L1, L0);
    set(b, "ST", L0);
    step(b);
    set_pins(b, ABC, 4, 7);
    step(b);
    ok &= glyph_is(b, GLYPHS47[2], L1, L0);
    set(b, "DF", L1);
    step(b);
    ok &= glyph_is(b, GLYPHS47[2], L0, L1);
    set(b, "ST", L1);
    step(b);
    ok &= glyph_is(b, GLYPHS47[7], L0, L1);
    expect(ok, "25 CD4056B shape: strobe low holds the code; DF high inverts every segment");
    bench_destroy(b);
}

int main(void) {
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "decoder_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_138();
    check_139();
    check_42();
    check_4028();
    check_137();
    check_131();
    check_4514();
    check_1g18();
    check_1g29();
    check_145();
    check_2414();
    check_148();
    check_348();
    check_147();
    check_40147();
    check_4532();
    check_47();
    check_4056();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("decoder_test: all passed\n");
    return 0;
}
