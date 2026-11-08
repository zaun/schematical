/*
 * mux_test: the mux kind's models against the function tables of the
 * plan-21 devices (plan-21-B section 4, plan-21-C section 4). Fixture devices
 * copy the real devices' pinouts and model keys from
 * planning/plan-21-data/specs.md, with 10 ns delays; levels are checked
 * 50 ns after each step.
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
/* Generated from planning/plan-21-data/specs.md (the spec JSON of each device). */
static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'mux','devices':[{'id':'fx-74_151','funct"
    "ion':'Multiplexer','description':'8-line to 1-line data selectors/multiplexers','drawer':["
    "],'pinouts':{'p16':[{'number':1,'name':'D3','role':'input'},{'number':2,'name':'D2','role'"
    ":'input'},{'number':3,'name':'D1','role':'input'},{'number':4,'name':'D0','role':'input'},"
    "{'number':5,'name':'Y','role':'output'},{'number':6,'name':'W','role':'output'},{'number':"
    "7,'name':'G','role':'input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':"
    "'C','role':'input'},{'number':10,'name':'B','role':'input'},{'number':11,'name':'A','role'"
    ":'input'},{'number':12,'name':'D7','role':'input'},{'number':13,'name':'D6','role':'input'"
    "},{'number':14,'name':'D5','role':'input'},{'number':15,'name':'D4','role':'input'},{'numb"
    "er':16,'name':'VCC','role':'power'}]},'model':'select','units':[{'data':['D0','D1','D2','D"
    "3','D4','D5','D6','D7'],'select':['A','B','C'],'y':'Y','w':'W','strobe':{'pin':'G','active"
    "':'low'}}],'disabled':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'n"
    "ame':'FX-74_151','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':"
    "'fx-74_153','function':'Multiplexer','description':'Dual 4-line to 1-line data selectors/m"
    "ultiplexers','drawer':[],'pinouts':{'p16':[{'number':1,'name':'1G','role':'input'},{'numbe"
    "r':2,'name':'B','role':'input'},{'number':3,'name':'1C3','role':'input'},{'number':4,'name"
    "':'1C2','role':'input'},{'number':5,'name':'1C1','role':'input'},{'number':6,'name':'1C0',"
    "'role':'input'},{'number':7,'name':'1Y','role':'output'},{'number':8,'name':'GND','role':'"
    "ground'},{'number':9,'name':'2Y','role':'output'},{'number':10,'name':'2C0','role':'input'"
    "},{'number':11,'name':'2C1','role':'input'},{'number':12,'name':'2C2','role':'input'},{'nu"
    "mber':13,'name':'2C3','role':'input'},{'number':14,'name':'A','role':'input'},{'number':15"
    ",'name':'2G','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'select',"
    "'units':[{'data':['1C0','1C1','1C2','1C3'],'select':['A','B'],'y':'1Y','strobe':{'pin':'1G"
    "','active':'low'}},{'data':['2C0','2C1','2C2','2C3'],'select':['A','B'],'y':'2Y','strobe':"
    "{'pin':'2G','active':'low'}}],'disabled':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':["
    "2,6]}},'parts':[{'name':'FX-74_153','package':'DIP','pinout':'p16','footprint':'FX16','fam"
    "ily':'HC'}]},{'id':'fx-74_157','function':'Multiplexer','description':'Quadruple 2-line to"
    " 1-line data selectors/multiplexers','drawer':[],'pinouts':{'p16':[{'number':1,'name':'A/B"
    "','role':'input'},{'number':2,'name':'1A','role':'input'},{'number':3,'name':'1B','role':'"
    "input'},{'number':4,'name':'1Y','role':'output'},{'number':5,'name':'2A','role':'input'},{"
    "'number':6,'name':'2B','role':'input'},{'number':7,'name':'2Y','role':'output'},{'number':"
    "8,'name':'GND','role':'ground'},{'number':9,'name':'3Y','role':'output'},{'number':10,'nam"
    "e':'3B','role':'input'},{'number':11,'name':'3A','role':'input'},{'number':12,'name':'4Y',"
    "'role':'output'},{'number':13,'name':'4B','role':'input'},{'number':14,'name':'4A','role':"
    "'input'},{'number':15,'name':'G','role':'input'},{'number':16,'name':'VCC','role':'power'}"
    "]},'model':'select','units':[{'data':['1A','1B'],'select':['A/B'],'y':'1Y','strobe':{'pin'"
    ":'G','active':'low'}},{'data':['2A','2B'],'select':['A/B'],'y':'2Y','strobe':{'pin':'G','a"
    "ctive':'low'}},{'data':['3A','3B'],'select':['A/B'],'y':'3Y','strobe':{'pin':'G','active':"
    "'low'}},{'data':['4A','4B'],'select':['A/B'],'y':'4Y','strobe':{'pin':'G','active':'low'}}"
    "],'disabled':'low','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-"
    "74_157','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_15"
    "8','function':'Multiplexer','description':'Quadruple 2-line to 1-line data selectors/multi"
    "plexers','drawer':[],'pinouts':{'p16':[{'number':1,'name':'A/B','role':'input'},{'number':"
    "2,'name':'1A','role':'input'},{'number':3,'name':'1B','role':'input'},{'number':4,'name':'"
    "1Y','role':'output'},{'number':5,'name':'2A','role':'input'},{'number':6,'name':'2B','role"
    "':'input'},{'number':7,'name':'2Y','role':'output'},{'number':8,'name':'GND','role':'groun"
    "d'},{'number':9,'name':'3Y','role':'output'},{'number':10,'name':'3B','role':'input'},{'nu"
    "mber':11,'name':'3A','role':'input'},{'number':12,'name':'4Y','role':'output'},{'number':1"
    "3,'name':'4B','role':'input'},{'number':14,'name':'4A','role':'input'},{'number':15,'name'"
    ":'G','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'select','units':"
    "[{'data':['1A','1B'],'select':['A/B'],'y':'1Y','strobe':{'pin':'G','active':'low'}},{'data"
    "':['2A','2B'],'select':['A/B'],'y':'2Y','strobe':{'pin':'G','active':'low'}},{'data':['3A'"
    ",'3B'],'select':['A/B'],'y':'3Y','strobe':{'pin':'G','active':'low'}},{'data':['4A','4B'],"
    "'select':['A/B'],'y':'4Y','strobe':{'pin':'G','active':'low'}}],'disabled':'low','invert':"
    "true,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_158','packa"
    "ge':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_251','function':"
    "'Multiplexer','description':'1-of-8 data selectors/multiplexers with 3-state outputs','dra"
    "wer':[],'pinouts':{'p16':[{'number':1,'name':'D3','role':'input'},{'number':2,'name':'D2',"
    "'role':'input'},{'number':3,'name':'D1','role':'input'},{'number':4,'name':'D0','role':'in"
    "put'},{'number':5,'name':'Y','role':'output'},{'number':6,'name':'W','role':'output'},{'nu"
    "mber':7,'name':'G','role':'input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'"
    "name':'C','role':'input'},{'number':10,'name':'B','role':'input'},{'number':11,'name':'A',"
    "'role':'input'},{'number':12,'name':'D7','role':'input'},{'number':13,'name':'D6','role':'"
    "input'},{'number':14,'name':'D5','role':'input'},{'number':15,'name':'D4','role':'input'},"
    "{'number':16,'name':'VCC','role':'power'}]},'model':'select','units':[{'data':['D0','D1','"
    "D2','D3','D4','D5','D6','D7'],'select':['A','B','C'],'y':'Y','w':'W','oe':{'pin':'G','acti"
    "ve':'low'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_251"
    "','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_257','fu"
    "nction':'Multiplexer','description':'Quadruple 2-line to 1-line data selectors/multiplexer"
    "s with 3-state outputs','drawer':[],'pinouts':{'p16':[{'number':1,'name':'A/B','role':'inp"
    "ut'},{'number':2,'name':'1A','role':'input'},{'number':3,'name':'1B','role':'input'},{'num"
    "ber':4,'name':'1Y','role':'output'},{'number':5,'name':'2A','role':'input'},{'number':6,'n"
    "ame':'2B','role':'input'},{'number':7,'name':'2Y','role':'output'},{'number':8,'name':'GND"
    "','role':'ground'},{'number':9,'name':'3Y','role':'output'},{'number':10,'name':'3B','role"
    "':'input'},{'number':11,'name':'3A','role':'input'},{'number':12,'name':'4Y','role':'outpu"
    "t'},{'number':13,'name':'4B','role':'input'},{'number':14,'name':'4A','role':'input'},{'nu"
    "mber':15,'name':'G','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'s"
    "elect','units':[{'data':['1A','1B'],'select':['A/B'],'y':'1Y','strobe':{'pin':'G','active'"
    ":'low'}},{'data':['2A','2B'],'select':['A/B'],'y':'2Y','strobe':{'pin':'G','active':'low'}"
    "},{'data':['3A','3B'],'select':['A/B'],'y':'3Y','strobe':{'pin':'G','active':'low'}},{'dat"
    "a':['4A','4B'],'select':['A/B'],'y':'4Y','strobe':{'pin':'G','active':'low'}}],'disabled':"
    "'z','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_257','packag"
    "e':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4512B','function':'M"
    "ultiplexer','description':'CMOS 8-channel data selector','drawer':[],'pinouts':{'p16':[{'n"
    "umber':1,'name':'D0','role':'input'},{'number':2,'name':'D1','role':'input'},{'number':3,'"
    "name':'D2','role':'input'},{'number':4,'name':'D3','role':'input'},{'number':5,'name':'D4'"
    ",'role':'input'},{'number':6,'name':'D5','role':'input'},{'number':7,'name':'D6','role':'i"
    "nput'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'D7','role':'input'},{"
    "'number':10,'name':'INHIBIT','role':'input'},{'number':11,'name':'A','role':'input'},{'num"
    "ber':12,'name':'B','role':'input'},{'number':13,'name':'C','role':'input'},{'number':14,'n"
    "ame':'OUTPUT','role':'output'},{'number':15,'name':'DISABLE','role':'input'},{'number':16,"
    "'name':'VDD','role':'power'}]},'model':'select','units':[{'data':['D0','D1','D2','D3','D4'"
    ",'D5','D6','D7'],'select':['A','B','C'],'y':'OUTPUT','strobe':{'pin':'INHIBIT','active':'l"
    "ow'},'oe':{'pin':'DISABLE','active':'low'}}],'disabled':'low','timing':{'HC':{'rise':10,'f"
    "all':10,'vcc':[2,6]}},'parts':[{'name':'FX-4512B','package':'DIP','pinout':'p16','footprin"
    "t':'FX16','family':'HC'}]},{'id':'fx-shape_sl','function':'Multiplexer','description':'sel"
    "ect-latch shape','drawer':[],'pinouts':{'p16':[{'number':1,'name':'D0','role':'input'},{'n"
    "umber':2,'name':'D1','role':'input'},{'number':3,'name':'D2','role':'input'},{'number':4,'"
    "name':'D3','role':'input'},{'number':5,'name':'D4','role':'input'},{'number':6,'name':'D5'"
    ",'role':'input'},{'number':7,'name':'D6','role':'input'},{'number':8,'name':'GND','role':'"
    "ground'},{'number':9,'name':'D7','role':'input'},{'number':10,'name':'LE','role':'input'},"
    "{'number':11,'name':'A','role':'input'},{'number':12,'name':'B','role':'input'},{'number':"
    "13,'name':'C','role':'input'},{'number':14,'name':'Y','role':'output'},{'number':15,'name'"
    ":'W','role':'output'},{'number':16,'name':'VCC','role':'power'}]},'model':'select','units'"
    ":[{'data':['D0','D1','D2','D3','D4','D5','D6','D7'],'select':['A','B','C'],'y':'Y','w':'W'"
    "}],'selectLatch':{'pin':'LE','active':'low'},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,"
    "6]}},'parts':[{'name':'FX-shape_sl','package':'DIP','pinout':'p16','footprint':'FX16','fam"
    "ily':'HC'}]},{'id':'fx-74_2G157','function':'Multiplexer','description':'Single 2-line to "
    "1-line data selector multiplexer','drawer':[],'pinouts':{'p8':[{'number':1,'name':'A','rol"
    "e':'input'},{'number':2,'name':'B','role':'input'},{'number':3,'name':'YBAR','role':'outpu"
    "t'},{'number':4,'name':'GND','role':'ground'},{'number':5,'name':'Y','role':'output'},{'nu"
    "mber':6,'name':'A/B','role':'input'},{'number':7,'name':'G','role':'input'},{'number':8,'n"
    "ame':'VCC','role':'power'}]},'model':'select','units':[{'data':['A','B'],'select':['A/B'],"
    "'y':'Y','w':'YBAR','strobe':{'pin':'G','active':'low'}}],'timing':{'HC':{'rise':10,'fall':"
    "10,'vcc':[2,6]}},'parts':[{'name':'FX-74_2G157','package':'DIP','pinout':'p8','footprint':"
    "'FX8','family':'HC'}]},{'id':'fx-74_298','function':'Multiplexer','description':'Quadruple"
    " 2-input multiplexers with storage','drawer':[],'pinouts':{'p16':[{'number':1,'name':'B2',"
    "'role':'input'},{'number':2,'name':'A2','role':'input'},{'number':3,'name':'A1','role':'in"
    "put'},{'number':4,'name':'B1','role':'input'},{'number':5,'name':'C2','role':'input'},{'nu"
    "mber':6,'name':'D2','role':'input'},{'number':7,'name':'D1','role':'input'},{'number':8,'n"
    "ame':'GND','role':'ground'},{'number':9,'name':'C1','role':'input'},{'number':10,'name':'W"
    "S','role':'input'},{'number':11,'name':'CLK','role':'input'},{'number':12,'name':'QD','rol"
    "e':'output'},{'number':13,'name':'QC','role':'output'},{'number':14,'name':'QB','role':'ou"
    "tput'},{'number':15,'name':'QA','role':'output'},{'number':16,'name':'VCC','role':'power'}"
    "]},'model':'stored','units':[{'data':['A1','A2'],'select':['WS'],'y':'QA'},{'data':['B1','"
    "B2'],'select':['WS'],'y':'QB'},{'data':['C1','C2'],'select':['WS'],'y':'QC'},{'data':['D1'"
    ",'D2'],'select':['WS'],'y':'QD'}],'outputClock':{'pin':'CLK','edge':'falling'},'timing':{'"
    "HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_298','package':'DIP','pinou"
    "t':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_399','function':'Multiplexer','d"
    "escription':'Quadruple 2-input multiplexers with storage','drawer':[],'pinouts':{'p16':[{'"
    "number':1,'name':'WS','role':'input'},{'number':2,'name':'QA','role':'output'},{'number':3"
    ",'name':'A1','role':'input'},{'number':4,'name':'A2','role':'input'},{'number':5,'name':'B"
    "2','role':'input'},{'number':6,'name':'B1','role':'input'},{'number':7,'name':'QB','role':"
    "'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'CLK','role':'input"
    "'},{'number':10,'name':'QC','role':'output'},{'number':11,'name':'C1','role':'input'},{'nu"
    "mber':12,'name':'C2','role':'input'},{'number':13,'name':'D2','role':'input'},{'number':14"
    ",'name':'D1','role':'input'},{'number':15,'name':'QD','role':'output'},{'number':16,'name'"
    ":'VCC','role':'power'}]},'model':'stored','units':[{'data':['A1','A2'],'select':['WS'],'y'"
    ":'QA'},{'data':['B1','B2'],'select':['WS'],'y':'QB'},{'data':['C1','C2'],'select':['WS'],'"
    "y':'QC'},{'data':['D1','D2'],'select':['WS'],'y':'QD'}],'outputClock':{'pin':'CLK','edge':"
    "'rising'},'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_399','"
    "package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_354','funct"
    "ion':'Multiplexer','description':'8-line to 1-line data selector/multiplexer/register with"
    " 3-state outputs','drawer':[],'pinouts':{'p20':[{'number':1,'name':'D7','role':'input'},{'"
    "number':2,'name':'D6','role':'input'},{'number':3,'name':'D5','role':'input'},{'number':4,"
    "'name':'D4','role':'input'},{'number':5,'name':'D3','role':'input'},{'number':6,'name':'D2"
    "','role':'input'},{'number':7,'name':'D1','role':'input'},{'number':8,'name':'D0','role':'"
    "input'},{'number':9,'name':'E','role':'input'},{'number':10,'name':'GND','role':'ground'},"
    "{'number':11,'name':'LE','role':'input'},{'number':12,'name':'S2','role':'input'},{'number"
    "':13,'name':'S1','role':'input'},{'number':14,'name':'S0','role':'input'},{'number':15,'na"
    "me':'OE1','role':'input'},{'number':16,'name':'OE2','role':'input'},{'number':17,'name':'O"
    "E3','role':'input'},{'number':18,'name':'YBAR','role':'output'},{'number':19,'name':'Y','r"
    "ole':'output'},{'number':20,'name':'VCC','role':'power'}]},'model':'stored','units':[{'dat"
    "a':['D0','D1','D2','D3','D4','D5','D6','D7'],'select':['S0','S1','S2'],'y':'Y','w':'YBAR'}"
    "],'dataLatch':{'pin':'E','active':'high'},'selectLatch':{'pin':'LE','active':'high'},'oes'"
    ":[{'pin':'OE1','active':'low'},{'pin':'OE2','active':'low'},{'pin':'OE3','active':'high'}]"
    ",'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_354','package':"
    "'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_356','function':'Mul"
    "tiplexer','description':'8-input multiplexer/register, three-state','drawer':[],'pinouts':"
    "{'p20':[{'number':1,'name':'D7','role':'input'},{'number':2,'name':'D6','role':'input'},{'"
    "number':3,'name':'D5','role':'input'},{'number':4,'name':'D4','role':'input'},{'number':5,"
    "'name':'D3','role':'input'},{'number':6,'name':'D2','role':'input'},{'number':7,'name':'D1"
    "','role':'input'},{'number':8,'name':'D0','role':'input'},{'number':9,'name':'CP','role':'"
    "input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'LE','role':'input'"
    "},{'number':12,'name':'S2','role':'input'},{'number':13,'name':'S1','role':'input'},{'numb"
    "er':14,'name':'S0','role':'input'},{'number':15,'name':'OE1','role':'input'},{'number':16,"
    "'name':'OE2','role':'input'},{'number':17,'name':'OE3','role':'input'},{'number':18,'name'"
    ":'YBAR','role':'output'},{'number':19,'name':'Y','role':'output'},{'number':20,'name':'VCC"
    "','role':'power'}]},'model':'stored','units':[{'data':['D0','D1','D2','D3','D4','D5','D6',"
    "'D7'],'select':['S0','S1','S2'],'y':'Y','w':'YBAR'}],'dataClock':{'pin':'CP','edge':'risin"
    "g'},'selectLatch':{'pin':'LE','active':'high'},'oes':[{'pin':'OE1','active':'low'},{'pin':"
    "'OE2','active':'low'},{'pin':'OE3','active':'high'}],'timing':{'HC':{'rise':10,'fall':10,'"
    "vcc':[2,6]}},'parts':[{'name':'FX-74_356','package':'DIP','pinout':'p20','footprint':'FX20"
    "','family':'HC'}]},{'id':'fx-74_670','function':'Register file','description':'4-by-4 regi"
    "ster files with 3-state outputs','drawer':[],'pinouts':{'p16':[{'number':1,'name':'D2','ro"
    "le':'input'},{'number':2,'name':'D3','role':'input'},{'number':3,'name':'D4','role':'input"
    "'},{'number':4,'name':'RB','role':'input'},{'number':5,'name':'RA','role':'input'},{'numbe"
    "r':6,'name':'Q4','role':'output'},{'number':7,'name':'Q3','role':'output'},{'number':8,'na"
    "me':'GND','role':'ground'},{'number':9,'name':'Q2','role':'output'},{'number':10,'name':'Q"
    "1','role':'output'},{'number':11,'name':'GR','role':'input'},{'number':12,'name':'GW','rol"
    "e':'input'},{'number':13,'name':'WB','role':'input'},{'number':14,'name':'WA','role':'inpu"
    "t'},{'number':15,'name':'D1','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'"
    "model':'regfile','words':4,'bits':4,'files':[{'d':['D1','D2','D3','D4'],'wa':['WA','WB'],'"
    "we':{'pin':'GW','active':'low'},'ra':['RA','RB'],'re':{'pin':'GR','active':'low'},'q':['Q1"
    "','Q2','Q3','Q4']}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX"
    "-74_670','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-shap"
    "e_rf2','function':'Register file','description':'two-file shape','drawer':[],'pinouts':{'p"
    "24':[{'number':1,'name':'AD0','role':'input'},{'number':2,'name':'AD1','role':'input'},{'n"
    "umber':3,'name':'AWA0','role':'input'},{'number':4,'name':'AWA1','role':'input'},{'number'"
    ":5,'name':'AWE','role':'input'},{'number':6,'name':'ARA0','role':'input'},{'number':7,'nam"
    "e':'ARA1','role':'input'},{'number':8,'name':'ARE','role':'input'},{'number':9,'name':'AQ0"
    "','role':'output'},{'number':10,'name':'AQ1','role':'output'},{'number':11,'name':'BD0','r"
    "ole':'input'},{'number':12,'name':'BD1','role':'input'},{'number':13,'name':'BWA0','role':"
    "'input'},{'number':14,'name':'BWA1','role':'input'},{'number':15,'name':'BWE','role':'inpu"
    "t'},{'number':16,'name':'BRA0','role':'input'},{'number':17,'name':'BRA1','role':'input'},"
    "{'number':18,'name':'BRE','role':'input'},{'number':19,'name':'BQ0','role':'output'},{'num"
    "ber':20,'name':'BQ1','role':'output'},{'number':21,'name':'GND','role':'ground'},{'number'"
    ":22,'name':'VCC','role':'power'},{'number':23,'name':'NC1','role':'nc'},{'number':24,'name"
    "':'NC2','role':'nc'}]},'model':'regfile','words':4,'bits':2,'files':[{'d':['AD0','AD1'],'w"
    "a':['AWA0','AWA1'],'we':{'pin':'AWE','active':'low'},'ra':['ARA0','ARA1'],'re':{'pin':'ARE"
    "','active':'low'},'q':['AQ0','AQ1']},{'d':['BD0','BD1'],'wa':['BWA0','BWA1'],'we':{'pin':'"
    "BWE','active':'low'},'ra':['BRA0','BRA1'],'re':{'pin':'BRE','active':'low'},'q':['BQ0','BQ"
    "1']}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-shape_rf2','p"
    "ackage':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]}]}";

/* Pin counts of the fixture devices: [8, 16, 20, 24] */
/* FIXTURE END */

/* Dual-row fixture footprints FX<n>, one per pin count above. */
#define FX_FOOTPRINT(N) \
    "{'id':'FX" #N "','description':'d','style':'dual','pins':" #N ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[" FX_FOOTPRINT(8) ","
    FX_FOOTPRINT(16) "," FX_FOOTPRINT(20) "," FX_FOOTPRINT(24) "," FX_FOOTPRINT(28) ","
    FX_FOOTPRINT(56) "]}";

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
    if (part == NULL || part->kind != PART_MUX) {
        fprintf(stderr, "no mux part %s\n", name);
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

/* Sets the pins fmt % i (i < n) from value's bits. */
static void set_bus(Bench *b, const char *fmt, unsigned first, unsigned n, unsigned value) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, first + i);
        set(b, name, bit(value, i));
    }
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

/* Every pin fmt % i (i < n) shows v. */
static int bus_all(Bench *b, const char *fmt, unsigned first, unsigned n, LogicValue v) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, first + i);
        ok &= is(b, name, v);
    }
    return ok;
}

/* ---- select ----------------------------------------------------------------- */

static const char *const ABC[] = {"A", "B", "C", "D"};

static void check_151(void) {
    Bench *b = open_fx("74_151");
    unsigned code, data;
    int ok = 1;

    set(b, "G", L0);
    for (code = 0; code < 8; code++) {
        set_pins(b, ABC, 3, code);
        for (data = 0; data < 2; data++) {
            /* only the chosen input differs from its neighbours */
            set_bus(b, "D%u", 0, 8, data ? 1u << code : 0xFFu & ~(1u << code));
            step(b);
            ok &= is(b, "Y", data ? L1 : L0) && is(b, "W", data ? L0 : L1);
        }
    }
    expect(ok, "1 '151 strobe low: Y = D[code], W = NOT Y (8 codes x data 0/1)");

    set_bus(b, "D%u", 0, 8, 0xFF);
    set(b, "G", L1);
    step(b);
    expect(is(b, "Y", L0) && is(b, "W", L1), "2 '151 strobe high: Y low, W high");

    set(b, "G", L0);
    set(b, "A", L0);
    set(b, "B", LX);
    set(b, "C", L0);
    set_bus(b, "D%u", 0, 8, 0x05);   /* D0 = D2 = 1 */
    step(b);
    ok = is(b, "Y", L1) && is(b, "W", L0);
    set_bus(b, "D%u", 0, 8, 0x01);   /* D0 = 1, D2 = 0 */
    step(b);
    ok &= is(b, "Y", LX) && is(b, "W", LX);
    set(b, "B", LZ);
    step(b);
    ok &= is(b, "Y", LX);
    expect(ok, "3 '151 X (or Z) on B: Y known when D0 = D2, else X");

    set(b, "B", L0);
    set(b, "G", LX);
    step(b);
    expect(is(b, "Y", LX) && is(b, "W", LX), "4 '151 X strobe: Y and W X");
    bench_destroy(b);
}

static void check_153(void) {
    Bench *b = open_fx("74_153");
    int ok;

    set(b, "1G", L0);
    set(b, "2G", L0);
    set_bus(b, "1C%u", 0, 4, 0x4);   /* 1C2 = 1 */
    set_bus(b, "2C%u", 0, 4, 0xB);   /* 2C2 = 0 */
    set(b, "A", L0);
    set(b, "B", L1);
    step(b);
    ok = is(b, "1Y", L1) && is(b, "2Y", L0);
    set(b, "1G", L1);
    step(b);
    ok &= is(b, "1Y", L0) && is(b, "2Y", L0);
    set(b, "A", L1);   /* code 3: 1C3 = 0, 2C3 = 1 */
    step(b);
    ok &= is(b, "1Y", L0) && is(b, "2Y", L1);
    set(b, "1G", L0);
    set(b, "2G", L1);
    set_bus(b, "1C%u", 0, 4, 0x8);
    step(b);
    ok &= is(b, "1Y", L1) && is(b, "2Y", L0);
    expect(ok, "5 '153: two units on shared selects, each with its own strobe");
    bench_destroy(b);
}

static void check_157(const char *device, int inverted, int n) {
    Bench *b = open_fx(device);
    LogicValue t = inverted ? L0 : L1, f = inverted ? L1 : L0;
    char what[96];
    int ok;

    set(b, "G", L0);
    set_bus(b, "%uA", 1, 4, 0x5);
    set_bus(b, "%uB", 1, 4, 0xA);
    set(b, "A/B", L0);
    step(b);
    ok = is(b, "1Y", t) && is(b, "2Y", f) && is(b, "3Y", t) && is(b, "4Y", f);
    set(b, "A/B", L1);
    step(b);
    ok &= is(b, "1Y", f) && is(b, "2Y", t) && is(b, "3Y", f) && is(b, "4Y", t);
    snprintf(what, sizeof what, "%d %s: select low -> A, high -> B, every unit%s", n, device,
             inverted ? " (Y = NOT data)" : "");
    expect(ok, what);

    set(b, "G", L1);
    step(b);
    snprintf(what, sizeof what, "%d %s: strobe high -> every Y %s", n + 1, device,
             inverted ? "high" : "low");
    expect(bus_all(b, "%uY", 1, 4, f), what);
    bench_destroy(b);
}

static void check_251(void) {
    Bench *b = open_fx("74_251");
    int ok;

    set_pins(b, ABC, 3, 6);
    set_bus(b, "D%u", 0, 8, 0x40);
    set(b, "G", L1);
    step(b);
    ok = is(b, "Y", LZ) && is(b, "W", LZ);
    set(b, "G", L0);
    step(b);
    ok &= is(b, "Y", L1) && is(b, "W", L0);
    set(b, "G", LX);
    step(b);
    ok &= is(b, "Y", LX) && is(b, "W", LX);
    expect(ok, "10 '251: OE high -> Y and W Z; low -> driven; X -> X");
    bench_destroy(b);
}

static void check_257(void) {
    Bench *b = open_fx("74_257");
    int ok;

    set_bus(b, "%uA", 1, 4, 0xF);
    set(b, "A/B", L0);
    set(b, "G", L1);
    step(b);
    ok = bus_all(b, "%uY", 1, 4, LZ);
    set(b, "G", L0);
    step(b);
    ok &= bus_all(b, "%uY", 1, 4, L1);
    expect(ok, "11 '257: strobe high -> every Y Z (disabled z); low -> driven");
    bench_destroy(b);
}

static void check_4512(void) {
    Bench *b = open_fx("4512B");
    unsigned code;
    int ok = 1;

    set(b, "INHIBIT", L0);
    set(b, "DISABLE", L0);
    for (code = 0; code < 8; code++) {
        set_pins(b, ABC, 3, code);
        set_bus(b, "D%u", 0, 8, 1u << code);
        step(b);
        ok &= is(b, "OUTPUT", L1);
        set_bus(b, "D%u", 0, 8, 0xFFu & ~(1u << code));
        step(b);
        ok &= is(b, "OUTPUT", L0);
    }
    expect(ok, "12 CD4512B: OUTPUT = D[code] (8 codes)");
    set_bus(b, "D%u", 0, 8, 0xFF);
    set(b, "INHIBIT", L1);
    step(b);
    ok = is(b, "OUTPUT", L0);
    set(b, "DISABLE", L1);
    step(b);
    ok &= is(b, "OUTPUT", LZ);
    set(b, "INHIBIT", L0);
    step(b);
    ok &= is(b, "OUTPUT", LZ);
    expect(ok, "13 CD4512B: INHIBIT high -> 0; DISABLE high -> Z (whatever INHIBIT)");
    bench_destroy(b);
}

static void check_select_latch(void) {
    Bench *b = open_fx("shape_sl");
    int ok;

    set(b, "LE", L1);   /* transparent */
    set_bus(b, "D%u", 0, 8, 0x08);
    set_pins(b, ABC, 3, 3);
    step(b);
    ok = is(b, "Y", L1) && is(b, "W", L0);
    set(b, "LE", L0);   /* holds code 3 */
    set_pins(b, ABC, 3, 5);
    step(b);
    ok &= is(b, "Y", L1);
    set_bus(b, "D%u", 0, 8, 0x20);   /* D3 = 0: the held code still picks D3 */
    step(b);
    ok &= is(b, "Y", L0);
    set(b, "LE", L1);   /* follows code 5 again */
    step(b);
    ok &= is(b, "Y", L1);
    set(b, "LE", LX);   /* X latch, input equal to the held code: kept */
    step(b);
    ok &= is(b, "Y", L1);
    set(b, "C", L0);    /* X latch, input differs: that bit becomes X (D1 = 0, D5 = 1) */
    step(b);
    ok &= is(b, "Y", LX);
    expect(ok, "14 selectLatch: select held while asserted, followed while not; X latch rules");
    bench_destroy(b);
}

static void check_2g157(void) {
    Bench *b = open_fx("74_2G157");
    int ok;

    set(b, "G", L0);
    set(b, "A", L1);
    set(b, "B", L0);
    set(b, "A/B", L0);
    step(b);
    ok = is(b, "Y", L1) && is(b, "YBAR", L0);
    set(b, "A/B", L1);
    step(b);
    ok &= is(b, "Y", L0) && is(b, "YBAR", L1);
    set(b, "B", L1);
    step(b);
    ok &= is(b, "Y", L1) && is(b, "YBAR", L0);
    set(b, "G", L1);
    step(b);
    ok &= is(b, "Y", L0) && is(b, "YBAR", L1);
    expect(ok, "15 2G157: A/B low -> A, high -> B, YBAR = NOT Y; G high -> Y low, YBAR high");
    bench_destroy(b);
}

/* ---- stored ------------------------------------------------------------------ */

static void check_298(void) {
    Bench *b = open_fx("74_298");
    int ok;

    set(b, "CLK", L1);
    set_bus(b, "A%u", 1, 2, 0x1);   /* A1 = 1, A2 = 0 */
    set_bus(b, "B%u", 1, 2, 0x2);   /* B1 = 0, B2 = 1 */
    set_bus(b, "C%u", 1, 2, 0x1);
    set_bus(b, "D%u", 1, 2, 0x2);
    set(b, "WS", L0);
    step(b);
    ok = is(b, "QA", L0) && is(b, "QB", L0);   /* power-on 0, no edge yet */
    set(b, "CLK", L0);   /* falling: word 1 */
    step(b);
    ok &= is(b, "QA", L1) && is(b, "QB", L0) && is(b, "QC", L1) && is(b, "QD", L0);
    set(b, "WS", L1);
    step(b);
    ok &= is(b, "QA", L1) && is(b, "QB", L0);   /* held between edges */
    set(b, "CLK", L1);   /* rising: nothing */
    step(b);
    ok &= is(b, "QA", L1) && is(b, "QB", L0);
    set(b, "CLK", L0);   /* falling: word 2 */
    step(b);
    ok &= is(b, "QA", L0) && is(b, "QB", L1) && is(b, "QC", L0) && is(b, "QD", L1);
    expect(ok, "16 '298: the falling edge loads word 1 (WS low) or word 2 (WS high); rising does nothing");

    set(b, "CLK", L1);
    set(b, "WS", LX);
    step(b);
    set(b, "CLK", L0);
    step(b);
    expect(is(b, "QA", LX) && is(b, "QB", LX), "17 '298: X WS at the edge (words differ) -> X");

    set(b, "WS", L0);
    set(b, "CLK", L1);
    step(b);
    set(b, "CLK", L0);
    step(b);
    set(b, "CLK", LX);   /* the clock leaves its idle level for X: no edge */
    step(b);
    ok = is(b, "QA", L1);
    set(b, "CLK", L1);
    step(b);
    set(b, "WS", L1);
    set(b, "CLK", LX);   /* from high to X: it could be the falling edge */
    step(b);
    ok &= is(b, "QA", LX) && is(b, "QC", LX);
    expect(ok, "18 '298: a clock moving from high to X makes X the bits a fall would change");
    bench_destroy(b);
}

static void check_399(void) {
    Bench *b = open_fx("74_399");
    int ok;

    set(b, "CLK", L0);
    set_bus(b, "A%u", 1, 2, 0x2);   /* A1 = 0, A2 = 1 */
    set(b, "WS", L1);
    step(b);
    ok = is(b, "QA", L0);
    set(b, "CLK", L1);   /* rising: word 2 */
    step(b);
    ok &= is(b, "QA", L1);
    set(b, "WS", L0);
    set(b, "CLK", L0);   /* falling: nothing */
    step(b);
    ok &= is(b, "QA", L1);
    set(b, "CLK", L1);
    step(b);
    ok &= is(b, "QA", L0);
    expect(ok, "19 '399: the rising edge loads the word WS selects");
    bench_destroy(b);
}

static const char *const S012[] = {"S0", "S1", "S2"};

static void enable_354(Bench *b) {
    set(b, "OE1", L0);
    set(b, "OE2", L0);
    set(b, "OE3", L1);
}

static void check_354(void) {
    Bench *b = open_fx("74_354");
    int ok;

    enable_354(b);
    set(b, "E", L0);    /* data latches transparent */
    set(b, "LE", L0);   /* select latches transparent */
    set_pins(b, S012, 3, 6);
    set_bus(b, "D%u", 0, 8, 0x40);
    step(b);
    ok = is(b, "Y", L1) && is(b, "YBAR", L0);
    set(b, "E", L1);    /* data held */
    set_bus(b, "D%u", 0, 8, 0x00);
    step(b);
    ok &= is(b, "Y", L1);
    expect(ok, "20 '354: E low passes the data, E high holds it");

    set(b, "LE", L1);   /* select held at 6 */
    set_pins(b, S012, 3, 1);
    step(b);
    ok = is(b, "Y", L1);
    set(b, "LE", L0);   /* select 1: the held data has D1 = 0 */
    step(b);
    ok &= is(b, "Y", L0) && is(b, "YBAR", L1);
    expect(ok, "21 '354: LE high holds the select, LE low passes it");

    set(b, "OE1", L1);
    step(b);
    ok = is(b, "Y", LZ) && is(b, "YBAR", LZ);
    set(b, "OE1", L0);
    set(b, "OE2", L1);
    step(b);
    ok &= is(b, "Y", LZ);
    set(b, "OE2", L0);
    set(b, "OE3", L0);
    step(b);
    ok &= is(b, "Y", LZ);
    set(b, "OE3", LX);
    step(b);
    ok &= is(b, "Y", LX);
    set(b, "OE1", L1);   /* a deasserted enable wins over an X one */
    step(b);
    ok &= is(b, "Y", LZ);
    enable_354(b);
    step(b);
    ok &= is(b, "Y", L0);
    expect(ok, "22 '354: any output enable deasserted -> Y and YBAR Z; X -> X");
    bench_destroy(b);
}

static void check_356(void) {
    Bench *b = open_fx("74_356");
    int ok;

    enable_354(b);
    set(b, "LE", L0);
    set(b, "CP", L0);
    set_pins(b, S012, 3, 2);
    set_bus(b, "D%u", 0, 8, 0x04);
    step(b);
    ok = is(b, "Y", L0);   /* power-on 0 until the clock */
    set(b, "CP", L1);
    step(b);
    ok &= is(b, "Y", L1) && is(b, "YBAR", L0);
    set_bus(b, "D%u", 0, 8, 0x00);
    set(b, "CP", L0);
    step(b);
    ok &= is(b, "Y", L1);
    set(b, "CP", L1);
    step(b);
    ok &= is(b, "Y", L0);
    expect(ok, "23 '356: data registered on the rising edge of CP");
    bench_destroy(b);
}

/* ---- regfile ----------------------------------------------------------------- */

static void check_670(void) {
    Bench *b = open_fx("74_670");
    int ok;

    set(b, "GW", L1);
    set(b, "GR", L0);
    set(b, "RA", L0);
    set(b, "RB", L1);   /* read word 2 */
    step(b);
    ok = bus_is(b, "Q%u", 1, 4, 0x0);   /* power-on 0 */
    set(b, "WA", L1);
    set(b, "WB", L0);
    set_bus(b, "D%u", 1, 4, 0x9);
    set(b, "GW", L0);   /* write word 1 */
    step(b);
    set(b, "GW", L1);
    step(b);
    ok &= bus_is(b, "Q%u", 1, 4, 0x0);   /* word 2 still read */
    set(b, "WA", L0);
    set(b, "WB", L1);
    set_bus(b, "D%u", 1, 4, 0x5);
    set(b, "GW", L0);
    step(b);
    ok &= bus_is(b, "Q%u", 1, 4, 0x5);   /* writing the word being read shows at once */
    set_bus(b, "D%u", 1, 4, 0x6);
    step(b);
    ok &= bus_is(b, "Q%u", 1, 4, 0x6);   /* transparent while GW low */
    set(b, "GW", L1);
    step(b);
    set_bus(b, "D%u", 1, 4, 0xF);
    step(b);
    ok &= bus_is(b, "Q%u", 1, 4, 0x6);   /* held after GW rises */
    set(b, "RA", L1);
    set(b, "RB", L0);   /* read word 1 */
    step(b);
    ok &= bus_is(b, "Q%u", 1, 4, 0x9);
    expect(ok, "24 '670: writing the word being read shows; other words unchanged");

    set(b, "GR", L1);
    step(b);
    ok = bus_all(b, "Q%u", 1, 4, LZ);
    set(b, "GR", L0);
    set(b, "RB", LX);   /* word 1 = 1001 vs word 3 = 0000 */
    step(b);
    ok &= is(b, "Q1", LX) && is(b, "Q2", L0) && is(b, "Q3", L0) && is(b, "Q4", LX);
    expect(ok, "25 '670: GR high -> Q Z; an X read address bit makes X only the bits that differ");

    set(b, "RB", L1);
    set(b, "RA", L1);   /* read word 3 = 0000 */
    set(b, "WA", L1);
    set(b, "WB", LX);   /* word 1 or word 3 */
    set_bus(b, "D%u", 1, 4, 0x3);
    set(b, "GW", L0);
    step(b);
    set(b, "GW", L1);
    step(b);
    ok = is(b, "Q1", LX) && is(b, "Q2", LX) && is(b, "Q3", L0) && is(b, "Q4", L0);
    set(b, "RB", L0);   /* word 1 = 1001: bits 2 and 4 could change */
    step(b);
    ok &= is(b, "Q1", L1) && is(b, "Q2", LX) && is(b, "Q3", L0) && is(b, "Q4", LX);
    expect(ok, "26 '670: an X write address bit makes X the bits of both words that could change");
    bench_destroy(b);
}

static void check_rf2(void) {
    Bench *b = open_fx("shape_rf2");
    int ok;

    set(b, "AWE", L1);
    set(b, "BWE", L1);
    set(b, "ARE", L0);
    set(b, "BRE", L0);
    set(b, "ARA0", L1);
    set(b, "ARA1", L0);
    set(b, "BRA0", L1);
    set(b, "BRA1", L0);
    set(b, "AWA0", L1);
    set(b, "AWA1", L0);
    set(b, "AD0", L1);
    set(b, "AD1", L1);
    set(b, "AWE", L0);
    step(b);
    set(b, "AWE", L1);
    step(b);
    ok = is(b, "AQ0", L1) && is(b, "AQ1", L1) && is(b, "BQ0", L0) && is(b, "BQ1", L0);
    set(b, "BRE", L1);
    step(b);
    ok &= is(b, "AQ0", L1) && is(b, "BQ0", LZ);
    expect(ok, "27 regfile with two files: each file has its own array, write and read port");
    bench_destroy(b);
}


int main(void) {
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "mux_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    (void)pin_index;
    check_151();
    check_153();
    check_157("74_157", 0, 6);
    check_157("74_158", 1, 8);
    check_251();
    check_257();
    check_4512();
    check_select_latch();
    check_2g157();
    check_298();
    check_399();
    check_354();
    check_356();
    check_670();
    check_rf2();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("mux_test: all passed\n");
    return 0;
}
