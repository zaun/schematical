/*
 * buffer_test: the buffer kind's models against the function tables of the
 * devices that use them (plan-20-D section 3). Fixture devices copy the
 * real devices' pin names and keys with 10 ns delays; levels are checked
 * 50 ns after each step unless a case says otherwise.
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

/* Generated from the device keys of plan-20-D section 3 (a script, not committed). */
static const char FIXTURE[] =
    "{'format':'schematical-parts','version':1,'kind':'buffer','devices':[{'id':'fx-74_244','fu"
    "nction':'Buffer','description':'d','drawer':[],'pinouts':{'p20':[{'number':1,'name':'1OE',"
    "'role':'input'},{'number':2,'name':'1A1','role':'input'},{'number':3,'name':'2Y4','role':'"
    "output'},{'number':4,'name':'1A2','role':'input'},{'number':5,'name':'2Y3','role':'output'"
    "},{'number':6,'name':'1A3','role':'input'},{'number':7,'name':'2Y2','role':'output'},{'num"
    "ber':8,'name':'1A4','role':'input'},{'number':9,'name':'2Y1','role':'output'},{'number':10"
    ",'name':'GND','role':'ground'},{'number':11,'name':'2A1','role':'input'},{'number':12,'nam"
    "e':'1Y4','role':'output'},{'number':13,'name':'2A2','role':'input'},{'number':14,'name':'1"
    "Y3','role':'output'},{'number':15,'name':'2A3','role':'input'},{'number':16,'name':'1Y2','"
    "role':'output'},{'number':17,'name':'2A4','role':'input'},{'number':18,'name':'1Y1','role'"
    ":'output'},{'number':19,'name':'2OE','role':'input'},{'number':20,'name':'VCC','role':'pow"
    "er'}]},'model':'tristate','units':[{'a':['1A1','1A2','1A3','1A4'],'y':['1Y1','1Y2','1Y3','"
    "1Y4'],'oe':{'pin':'1OE','active':'low'}},{'a':['2A1','2A2','2A3','2A4'],'y':['2Y1','2Y2','"
    "2Y3','2Y4'],'oe':{'pin':'2OE','active':'low'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':"
    "[2,6]}},'parts':[{'name':'FX-74_244','package':'DIP','pinout':'p20','footprint':'FX20','fa"
    "mily':'HC'}]},{'id':'fx-74_240','function':'Buffer','description':'d','drawer':[],'pinouts"
    "':{'p20':[{'number':1,'name':'1OE','role':'input'},{'number':2,'name':'1A1','role':'input'"
    "},{'number':3,'name':'2Y4','role':'output'},{'number':4,'name':'1A2','role':'input'},{'num"
    "ber':5,'name':'2Y3','role':'output'},{'number':6,'name':'1A3','role':'input'},{'number':7,"
    "'name':'2Y2','role':'output'},{'number':8,'name':'1A4','role':'input'},{'number':9,'name':"
    "'2Y1','role':'output'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'2A1"
    "','role':'input'},{'number':12,'name':'1Y4','role':'output'},{'number':13,'name':'2A2','ro"
    "le':'input'},{'number':14,'name':'1Y3','role':'output'},{'number':15,'name':'2A3','role':'"
    "input'},{'number':16,'name':'1Y2','role':'output'},{'number':17,'name':'2A4','role':'input"
    "'},{'number':18,'name':'1Y1','role':'output'},{'number':19,'name':'2OE','role':'input'},{'"
    "number':20,'name':'VCC','role':'power'}]},'model':'tristate','units':[{'a':['1A1','1A2','1"
    "A3','1A4'],'y':['1Y1','1Y2','1Y3','1Y4'],'oe':{'pin':'1OE','active':'low'}},{'a':['2A1','2"
    "A2','2A3','2A4'],'y':['2Y1','2Y2','2Y3','2Y4'],'oe':{'pin':'2OE','active':'low'}}],'invert"
    "':true,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_240','pac"
    "kage':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_541','function"
    "':'Buffer','description':'d','drawer':[],'pinouts':{'p20':[{'number':1,'name':'OE1','role'"
    ":'input'},{'number':2,'name':'A1','role':'input'},{'number':3,'name':'A2','role':'input'},"
    "{'number':4,'name':'A3','role':'input'},{'number':5,'name':'A4','role':'input'},{'number':"
    "6,'name':'A5','role':'input'},{'number':7,'name':'A6','role':'input'},{'number':8,'name':'"
    "A7','role':'input'},{'number':9,'name':'A8','role':'input'},{'number':10,'name':'GND','rol"
    "e':'ground'},{'number':11,'name':'Y8','role':'output'},{'number':12,'name':'Y7','role':'ou"
    "tput'},{'number':13,'name':'Y6','role':'output'},{'number':14,'name':'Y5','role':'output'}"
    ",{'number':15,'name':'Y4','role':'output'},{'number':16,'name':'Y3','role':'output'},{'num"
    "ber':17,'name':'Y2','role':'output'},{'number':18,'name':'Y1','role':'output'},{'number':1"
    "9,'name':'OE2','role':'input'},{'number':20,'name':'VCC','role':'power'}]},'model':'trista"
    "te','units':[{'a':['A1','A2','A3','A4','A5','A6','A7','A8'],'y':['Y1','Y2','Y3','Y4','Y5',"
    "'Y6','Y7','Y8'],'oe':[{'pin':'OE1','active':'low'},{'pin':'OE2','active':'low'}]}],'timing"
    "':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_541','package':'DIP','p"
    "inout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-1G125','function':'Buffer','des"
    "cription':'d','drawer':[],'pinouts':{'p5':[{'number':1,'name':'OE','role':'input'},{'numbe"
    "r':2,'name':'A','role':'input'},{'number':3,'name':'GND','role':'ground'},{'number':4,'nam"
    "e':'Y','role':'output'},{'number':5,'name':'VCC','role':'power'}]},'model':'tristate','uni"
    "ts':[{'a':['A'],'y':['Y'],'oe':{'pin':'OE','active':'low'}}],'timing':{'HC':{'rise':10,'fa"
    "ll':10,'vcc':[2,6]}},'parts':[{'name':'FX-1G125','package':'DIP','pinout':'p5','footprint'"
    ":'FX5','family':'HC'}]},{'id':'fx-1G126','function':'Buffer','description':'d','drawer':[]"
    ",'pinouts':{'p5':[{'number':1,'name':'OE','role':'input'},{'number':2,'name':'A','role':'i"
    "nput'},{'number':3,'name':'GND','role':'ground'},{'number':4,'name':'Y','role':'output'},{"
    "'number':5,'name':'VCC','role':'power'}]},'model':'tristate','units':[{'a':['A'],'y':['Y']"
    ",'oe':{'pin':'OE','active':'high'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'pa"
    "rts':[{'name':'FX-1G126','package':'DIP','pinout':'p5','footprint':'FX5','family':'HC'}]},"
    "{'id':'fx-1G125_OD','function':'Buffer','description':'d','drawer':[],'pinouts':{'p5':[{'n"
    "umber':1,'name':'OE','role':'input'},{'number':2,'name':'A','role':'input'},{'number':3,'n"
    "ame':'GND','role':'ground'},{'number':4,'name':'Y','role':'output'},{'number':5,'name':'VC"
    "C','role':'power'}]},'model':'tristate','units':[{'a':['A'],'y':['Y'],'oe':{'pin':'OE','ac"
    "tive':'low'}}],'output':'open-drain','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'pa"
    "rts':[{'name':'FX-1G125_OD','package':'DIP','pinout':'p5','footprint':'FX5','family':'HC'}"
    "]},{'id':'fx-74_245','function':'Buffer','description':'d','drawer':[],'pinouts':{'p20':[{"
    "'number':1,'name':'DIR','role':'input'},{'number':2,'name':'A1','role':'inout'},{'number':"
    "3,'name':'A2','role':'inout'},{'number':4,'name':'A3','role':'inout'},{'number':5,'name':'"
    "A4','role':'inout'},{'number':6,'name':'A5','role':'inout'},{'number':7,'name':'A6','role'"
    ":'inout'},{'number':8,'name':'A7','role':'inout'},{'number':9,'name':'A8','role':'inout'},"
    "{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'B8','role':'inout'},{'numb"
    "er':12,'name':'B7','role':'inout'},{'number':13,'name':'B6','role':'inout'},{'number':14,'"
    "name':'B5','role':'inout'},{'number':15,'name':'B4','role':'inout'},{'number':16,'name':'B"
    "3','role':'inout'},{'number':17,'name':'B2','role':'inout'},{'number':18,'name':'B1','role"
    "':'inout'},{'number':19,'name':'OE','role':'input'},{'number':20,'name':'VCC','role':'powe"
    "r'}]},'model':'transceiver','units':[{'a':['A1','A2','A3','A4','A5','A6','A7','A8'],'b':['"
    "B1','B2','B3','B4','B5','B6','B7','B8'],'dir':{'pin':'DIR','ab':'high'},'oe':{'pin':'OE','"
    "active':'low'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74"
    "_245','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_640'"
    ",'function':'Buffer','description':'d','drawer':[],'pinouts':{'p20':[{'number':1,'name':'D"
    "IR','role':'input'},{'number':2,'name':'A1','role':'inout'},{'number':3,'name':'A2','role'"
    ":'inout'},{'number':4,'name':'A3','role':'inout'},{'number':5,'name':'A4','role':'inout'},"
    "{'number':6,'name':'A5','role':'inout'},{'number':7,'name':'A6','role':'inout'},{'number':"
    "8,'name':'A7','role':'inout'},{'number':9,'name':'A8','role':'inout'},{'number':10,'name':"
    "'GND','role':'ground'},{'number':11,'name':'B8','role':'inout'},{'number':12,'name':'B7','"
    "role':'inout'},{'number':13,'name':'B6','role':'inout'},{'number':14,'name':'B5','role':'i"
    "nout'},{'number':15,'name':'B4','role':'inout'},{'number':16,'name':'B3','role':'inout'},{"
    "'number':17,'name':'B2','role':'inout'},{'number':18,'name':'B1','role':'inout'},{'number'"
    ":19,'name':'OE','role':'input'},{'number':20,'name':'VCC','role':'power'}]},'model':'trans"
    "ceiver','units':[{'a':['A1','A2','A3','A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','"
    "B5','B6','B7','B8'],'dir':{'pin':'DIR','ab':'high'},'oe':{'pin':'OE','active':'low'}}],'in"
    "vert':true,'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_640',"
    "'package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_641','func"
    "tion':'Buffer','description':'d','drawer':[],'pinouts':{'p20':[{'number':1,'name':'DIR','r"
    "ole':'input'},{'number':2,'name':'A1','role':'inout'},{'number':3,'name':'A2','role':'inou"
    "t'},{'number':4,'name':'A3','role':'inout'},{'number':5,'name':'A4','role':'inout'},{'numb"
    "er':6,'name':'A5','role':'inout'},{'number':7,'name':'A6','role':'inout'},{'number':8,'nam"
    "e':'A7','role':'inout'},{'number':9,'name':'A8','role':'inout'},{'number':10,'name':'GND',"
    "'role':'ground'},{'number':11,'name':'B8','role':'inout'},{'number':12,'name':'B7','role':"
    "'inout'},{'number':13,'name':'B6','role':'inout'},{'number':14,'name':'B5','role':'inout'}"
    ",{'number':15,'name':'B4','role':'inout'},{'number':16,'name':'B3','role':'inout'},{'numbe"
    "r':17,'name':'B2','role':'inout'},{'number':18,'name':'B1','role':'inout'},{'number':19,'n"
    "ame':'OE','role':'input'},{'number':20,'name':'VCC','role':'power'}]},'model':'transceiver"
    "','units':[{'a':['A1','A2','A3','A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B5','B"
    "6','B7','B8'],'dir':{'pin':'DIR','ab':'high'},'oe':{'pin':'OE','active':'low'}}],'output':"
    "'open-drain','timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_641"
    "','package':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_623','fu"
    "nction':'Buffer','description':'d','drawer':[],'pinouts':{'p20':[{'number':1,'name':'OEAB'"
    ",'role':'input'},{'number':2,'name':'A1','role':'inout'},{'number':3,'name':'A2','role':'i"
    "nout'},{'number':4,'name':'A3','role':'inout'},{'number':5,'name':'A4','role':'inout'},{'n"
    "umber':6,'name':'A5','role':'inout'},{'number':7,'name':'A6','role':'inout'},{'number':8,'"
    "name':'A7','role':'inout'},{'number':9,'name':'A8','role':'inout'},{'number':10,'name':'GN"
    "D','role':'ground'},{'number':11,'name':'B8','role':'inout'},{'number':12,'name':'B7','rol"
    "e':'inout'},{'number':13,'name':'B6','role':'inout'},{'number':14,'name':'B5','role':'inou"
    "t'},{'number':15,'name':'B4','role':'inout'},{'number':16,'name':'B3','role':'inout'},{'nu"
    "mber':17,'name':'B2','role':'inout'},{'number':18,'name':'B1','role':'inout'},{'number':19"
    ",'name':'OEBA','role':'input'},{'number':20,'name':'VCC','role':'power'}]},'model':'transc"
    "eiver','units':[{'a':['A1','A2','A3','A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B"
    "5','B6','B7','B8'],'oeab':{'pin':'OEAB','active':'high'},'oeba':{'pin':'OEBA','active':'lo"
    "w'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_623','pack"
    "age':'DIP','pinout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-74_16245','functio"
    "n':'Buffer','description':'d','drawer':[],'pinouts':{'p14':[{'number':1,'name':'1DIR','rol"
    "e':'input'},{'number':2,'name':'1B1','role':'inout'},{'number':3,'name':'1B2','role':'inou"
    "t'},{'number':4,'name':'GND','role':'ground'},{'number':5,'name':'2B1','role':'inout'},{'n"
    "umber':6,'name':'2B2','role':'inout'},{'number':7,'name':'2DIR','role':'input'},{'number':"
    "8,'name':'2OE','role':'input'},{'number':9,'name':'2A2','role':'inout'},{'number':10,'name"
    "':'2A1','role':'inout'},{'number':11,'name':'VCC','role':'power'},{'number':12,'name':'1A2"
    "','role':'inout'},{'number':13,'name':'1A1','role':'inout'},{'number':14,'name':'1OE','rol"
    "e':'input'}]},'model':'transceiver','units':[{'a':['1A1','1A2'],'b':['1B1','1B2'],'dir':{'"
    "pin':'1DIR','ab':'high'},'oe':{'pin':'1OE','active':'low'}},{'a':['2A1','2A2'],'b':['2B1',"
    "'2B2'],'dir':{'pin':'2DIR','ab':'high'},'oe':{'pin':'2OE','active':'low'}}],'timing':{'HC'"
    ":{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_16245','package':'DIP','pinout"
    "':'p14','footprint':'FX14','family':'HC'}]},{'id':'fx-74_646','function':'Buffer','descrip"
    "tion':'d','drawer':[],'pinouts':{'p24':[{'number':1,'name':'CLKAB','role':'input'},{'numbe"
    "r':2,'name':'SAB','role':'input'},{'number':3,'name':'DIR','role':'input'},{'number':4,'na"
    "me':'A1','role':'inout'},{'number':5,'name':'A2','role':'inout'},{'number':6,'name':'A3','"
    "role':'inout'},{'number':7,'name':'A4','role':'inout'},{'number':8,'name':'A5','role':'ino"
    "ut'},{'number':9,'name':'A6','role':'inout'},{'number':10,'name':'A7','role':'inout'},{'nu"
    "mber':11,'name':'A8','role':'inout'},{'number':12,'name':'GND','role':'ground'},{'number':"
    "13,'name':'B8','role':'inout'},{'number':14,'name':'B7','role':'inout'},{'number':15,'name"
    "':'B6','role':'inout'},{'number':16,'name':'B5','role':'inout'},{'number':17,'name':'B4','"
    "role':'inout'},{'number':18,'name':'B3','role':'inout'},{'number':19,'name':'B2','role':'i"
    "nout'},{'number':20,'name':'B1','role':'inout'},{'number':21,'name':'OE','role':'input'},{"
    "'number':22,'name':'SBA','role':'input'},{'number':23,'name':'CLKBA','role':'input'},{'num"
    "ber':24,'name':'VCC','role':'power'}]},'model':'registered','units':[{'a':['A1','A2','A3',"
    "'A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B5','B6','B7','B8'],'dir':{'pin':'DIR'"
    ",'ab':'high'},'oe':{'pin':'OE','active':'low'},'ab':{'store':'register','clk':{'pin':'CLKA"
    "B','edge':'rising'},'select':{'pin':'SAB','active':'high'}},'ba':{'store':'register','clk'"
    ":{'pin':'CLKBA','edge':'rising'},'select':{'pin':'SBA','active':'high'}}}],'timing':{'HC':"
    "{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_646','package':'DIP','pinout':'"
    "p24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_652','function':'Buffer','descriptio"
    "n':'d','drawer':[],'pinouts':{'p24':[{'number':1,'name':'CLKAB','role':'input'},{'number':"
    "2,'name':'SAB','role':'input'},{'number':3,'name':'OEAB','role':'input'},{'number':4,'name"
    "':'A1','role':'inout'},{'number':5,'name':'A2','role':'inout'},{'number':6,'name':'A3','ro"
    "le':'inout'},{'number':7,'name':'A4','role':'inout'},{'number':8,'name':'A5','role':'inout"
    "'},{'number':9,'name':'A6','role':'inout'},{'number':10,'name':'A7','role':'inout'},{'numb"
    "er':11,'name':'A8','role':'inout'},{'number':12,'name':'GND','role':'ground'},{'number':13"
    ",'name':'B8','role':'inout'},{'number':14,'name':'B7','role':'inout'},{'number':15,'name':"
    "'B6','role':'inout'},{'number':16,'name':'B5','role':'inout'},{'number':17,'name':'B4','ro"
    "le':'inout'},{'number':18,'name':'B3','role':'inout'},{'number':19,'name':'B2','role':'ino"
    "ut'},{'number':20,'name':'B1','role':'inout'},{'number':21,'name':'OEBA','role':'input'},{"
    "'number':22,'name':'SBA','role':'input'},{'number':23,'name':'CLKBA','role':'input'},{'num"
    "ber':24,'name':'VCC','role':'power'}]},'model':'registered','units':[{'a':['A1','A2','A3',"
    "'A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B5','B6','B7','B8'],'ab':{'store':'reg"
    "ister','clk':{'pin':'CLKAB','edge':'rising'},'oe':{'pin':'OEAB','active':'high'},'select':"
    "{'pin':'SAB','active':'high'}},'ba':{'store':'register','clk':{'pin':'CLKBA','edge':'risin"
    "g'},'oe':{'pin':'OEBA','active':'low'},'select':{'pin':'SBA','active':'high'}}}],'timing':"
    "{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_652','package':'DIP','pin"
    "out':'p24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_543','function':'Buffer','desc"
    "ription':'d','drawer':[],'pinouts':{'p24':[{'number':1,'name':'LEBA','role':'input'},{'num"
    "ber':2,'name':'OEBA','role':'input'},{'number':3,'name':'A1','role':'inout'},{'number':4,'"
    "name':'A2','role':'inout'},{'number':5,'name':'A3','role':'inout'},{'number':6,'name':'A4'"
    ",'role':'inout'},{'number':7,'name':'A5','role':'inout'},{'number':8,'name':'A6','role':'i"
    "nout'},{'number':9,'name':'A7','role':'inout'},{'number':10,'name':'A8','role':'inout'},{'"
    "number':11,'name':'CEAB','role':'input'},{'number':12,'name':'GND','role':'ground'},{'numb"
    "er':13,'name':'CEBA','role':'input'},{'number':14,'name':'B8','role':'inout'},{'number':15"
    ",'name':'B7','role':'inout'},{'number':16,'name':'B6','role':'inout'},{'number':17,'name':"
    "'B5','role':'inout'},{'number':18,'name':'B4','role':'inout'},{'number':19,'name':'B3','ro"
    "le':'inout'},{'number':20,'name':'B2','role':'inout'},{'number':21,'name':'B1','role':'ino"
    "ut'},{'number':22,'name':'LEAB','role':'input'},{'number':23,'name':'OEAB','role':'input'}"
    ",{'number':24,'name':'VCC','role':'power'}]},'model':'registered','units':[{'a':['A1','A2'"
    ",'A3','A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B5','B6','B7','B8'],'ab':{'store"
    "':'latch','le':{'pin':'LEAB','active':'low'},'ce':{'pin':'CEAB','active':'low'},'oe':{'pin"
    "':'OEAB','active':'low'}},'ba':{'store':'latch','le':{'pin':'LEBA','active':'low'},'ce':{'"
    "pin':'CEBA','active':'low'},'oe':{'pin':'OEBA','active':'low'}}}],'timing':{'HC':{'rise':1"
    "0,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_543','package':'DIP','pinout':'p24','foo"
    "tprint':'FX24','family':'HC'}]},{'id':'fx-74_2952','function':'Buffer','description':'d','"
    "drawer':[],'pinouts':{'p24':[{'number':1,'name':'CLKAB','role':'input'},{'number':2,'name'"
    ":'CLKENAB','role':'input'},{'number':3,'name':'OEAB','role':'input'},{'number':4,'name':'C"
    "LKBA','role':'input'},{'number':5,'name':'CLKENBA','role':'input'},{'number':6,'name':'OEB"
    "A','role':'input'},{'number':7,'name':'A1','role':'inout'},{'number':8,'name':'A2','role':"
    "'inout'},{'number':9,'name':'A3','role':'inout'},{'number':10,'name':'A4','role':'inout'},"
    "{'number':11,'name':'A5','role':'inout'},{'number':12,'name':'A6','role':'inout'},{'number"
    "':13,'name':'A7','role':'inout'},{'number':14,'name':'A8','role':'inout'},{'number':15,'na"
    "me':'GND','role':'ground'},{'number':16,'name':'B1','role':'inout'},{'number':17,'name':'B"
    "2','role':'inout'},{'number':18,'name':'B3','role':'inout'},{'number':19,'name':'B4','role"
    "':'inout'},{'number':20,'name':'B5','role':'inout'},{'number':21,'name':'B6','role':'inout"
    "'},{'number':22,'name':'B7','role':'inout'},{'number':23,'name':'B8','role':'inout'},{'num"
    "ber':24,'name':'VCC','role':'power'}]},'model':'registered','units':[{'a':['A1','A2','A3',"
    "'A4','A5','A6','A7','A8'],'b':['B1','B2','B3','B4','B5','B6','B7','B8'],'ab':{'store':'reg"
    "ister','clk':{'pin':'CLKAB','edge':'rising'},'ce':{'pin':'CLKENAB','active':'low'},'oe':{'"
    "pin':'OEAB','active':'low'}},'ba':{'store':'register','clk':{'pin':'CLKBA','edge':'rising'"
    "},'ce':{'pin':'CLKENBA','active':'low'},'oe':{'pin':'OEBA','active':'low'}}}],'timing':{'H"
    "C':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_2952','package':'DIP','pinou"
    "t':'p24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_16952','function':'Buffer','desc"
    "ription':'d','drawer':[],'pinouts':{'p22':[{'number':1,'name':'1CLKAB','role':'input'},{'n"
    "umber':2,'name':'1CLKENAB','role':'input'},{'number':3,'name':'1OEAB','role':'input'},{'nu"
    "mber':4,'name':'1CLKBA','role':'input'},{'number':5,'name':'1CLKENBA','role':'input'},{'nu"
    "mber':6,'name':'1OEBA','role':'input'},{'number':7,'name':'1A1','role':'inout'},{'number':"
    "8,'name':'1A2','role':'inout'},{'number':9,'name':'1B1','role':'inout'},{'number':10,'name"
    "':'1B2','role':'inout'},{'number':11,'name':'2CLKAB','role':'input'},{'number':12,'name':'"
    "2CLKENAB','role':'input'},{'number':13,'name':'2OEAB','role':'input'},{'number':14,'name':"
    "'2CLKBA','role':'input'},{'number':15,'name':'2CLKENBA','role':'input'},{'number':16,'name"
    "':'2OEBA','role':'input'},{'number':17,'name':'2A1','role':'inout'},{'number':18,'name':'2"
    "A2','role':'inout'},{'number':19,'name':'2B1','role':'inout'},{'number':20,'name':'2B2','r"
    "ole':'inout'},{'number':21,'name':'GND','role':'ground'},{'number':22,'name':'VCC','role':"
    "'power'}]},'model':'registered','units':[{'a':['1A1','1A2'],'b':['1B1','1B2'],'ab':{'store"
    "':'register','clk':{'pin':'1CLKAB','edge':'rising'},'ce':{'pin':'1CLKENAB','active':'low'}"
    ",'oe':{'pin':'1OEAB','active':'low'}},'ba':{'store':'register','clk':{'pin':'1CLKBA','edge"
    "':'rising'},'ce':{'pin':'1CLKENBA','active':'low'},'oe':{'pin':'1OEBA','active':'low'}}},{"
    "'a':['2A1','2A2'],'b':['2B1','2B2'],'ab':{'store':'register','clk':{'pin':'2CLKAB','edge':"
    "'rising'},'ce':{'pin':'2CLKENAB','active':'low'},'oe':{'pin':'2OEAB','active':'low'}},'ba'"
    ":{'store':'register','clk':{'pin':'2CLKBA','edge':'rising'},'ce':{'pin':'2CLKENBA','active"
    "':'low'},'oe':{'pin':'2OEBA','active':'low'}}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':"
    "[2,6]}},'parts':[{'name':'FX-74_16952','package':'DIP','pinout':'p22','footprint':'FX22','"
    "family':'HC'}]},{'id':'fx-1T45','function':'Buffer','description':'d','drawer':[],'pinouts"
    "':{'p6':[{'number':1,'name':'VCCA','role':'power'},{'number':2,'name':'GND','role':'ground"
    "'},{'number':3,'name':'A','role':'inout'},{'number':4,'name':'B','role':'inout'},{'number'"
    ":5,'name':'DIR','role':'input'},{'number':6,'name':'VCCB','role':'power'}]},'model':'trans"
    "lator','units':[{'a':['A'],'b':['B'],'dir':{'pin':'DIR','ab':'high'},'oe':[]}],'domains':["
    "{'power':'VCCA','pins':['A','DIR']},{'power':'VCCB','pins':['B']}],'timing':{'LVC':{'rise'"
    ":10,'fall':10,'vcc':[1.65,5.5],'vccB':[1.65,5.5]}},'parts':[{'name':'FX-1T45','package':'D"
    "IP','pinout':'p6','footprint':'FX6','family':'LVC'}]},{'id':'fx-74_8T245','function':'Buff"
    "er','description':'d','drawer':[],'pinouts':{'p10':[{'number':1,'name':'VCCA','role':'powe"
    "r'},{'number':2,'name':'DIR','role':'input'},{'number':3,'name':'A1','role':'inout'},{'num"
    "ber':4,'name':'A2','role':'inout'},{'number':5,'name':'GND','role':'ground'},{'number':6,'"
    "name':'GND','role':'ground'},{'number':7,'name':'B2','role':'inout'},{'number':8,'name':'B"
    "1','role':'inout'},{'number':9,'name':'OE','role':'input'},{'number':10,'name':'VCCB','rol"
    "e':'power'}]},'model':'translator','units':[{'a':['A1','A2'],'b':['B1','B2'],'dir':{'pin':"
    "'DIR','ab':'high'},'oe':{'pin':'OE','active':'low'}}],'domains':[{'power':'VCCA','pins':['"
    "A1','A2','DIR','OE']},{'power':'VCCB','pins':['B1','B2']}],'timing':{'LVC':{'rise':10,'fal"
    "l':10,'vcc':[1.65,5.5],'vccB':[1.65,5.5]}},'parts':[{'name':'FX-74_8T245','package':'DIP',"
    "'pinout':'p10','footprint':'FX10','family':'LVC'}]},{'id':'fx-TXU0104','function':'Buffer'"
    ",'description':'d','drawer':[],'pinouts':{'p8':[{'number':1,'name':'VCCA','role':'power'},"
    "{'number':2,'name':'A1','role':'input'},{'number':3,'name':'A2','role':'input'},{'number':"
    "4,'name':'OE','role':'input'},{'number':5,'name':'GND','role':'ground'},{'number':6,'name'"
    ":'Y2','role':'output'},{'number':7,'name':'Y1','role':'output'},{'number':8,'name':'VCCB',"
    "'role':'power'}]},'model':'translator','units':[{'a':['A1','A2'],'y':['Y1','Y2'],'oe':{'pi"
    "n':'OE','active':'high'}}],'domains':[{'power':'VCCA','pins':['A1','A2','OE']},{'power':'V"
    "CCB','pins':['Y1','Y2']}],'timing':{'TXU':{'rise':10,'fall':10,'vcc':[1.1,5.5],'vccB':[1.1"
    ",5.5]}},'parts':[{'name':'FX-TXU0104','package':'DIP','pinout':'p8','footprint':'FX8','fam"
    "ily':'TXU'}]}]}"
    ;

/* Dual-row fixture footprints FX<n>, one per pin count above. */
static const char FIXTURE_FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[{'id':'FX5','description':'d'"
    ",'style':'dual','pins':5,'perSide':3,'slots':[1,2,3,4,6],'body':{'width':1,'height':1},'pi"
    "tch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id':'FX6','description'"
    ":'d','style':'dual','pins':6,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'widt"
    "h':0.5,'height':0.5,'shape':'rect'}},{'id':'FX8','description':'d','style':'dual','pins':8"
    ",'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':"
    "'rect'}},{'id':'FX10','description':'d','style':'dual','pins':10,'body':{'width':1,'height"
    "':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id':'FX14','des"
    "cription':'d','style':'dual','pins':14,'body':{'width':1,'height':1},'pitch':1,'span':2,'p"
    "ad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id':'FX20','description':'d','style':'dua"
    "l','pins':20,'body':{'width':1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':"
    "0.5,'shape':'rect'}},{'id':'FX22','description':'d','style':'dual','pins':22,'body':{'widt"
    "h':1,'height':1},'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id'"
    ":'FX24','description':'d','style':'dual','pins':24,'body':{'width':1,'height':1},'pitch':1"
    ",'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}]}"
    ;

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
    if (part == NULL || part->kind != PART_BUFFER) {
        fprintf(stderr, "no buffer part %s\n", name);
        exit(1);
    }
    return part;
}

static Bench *open_fx(const char *device) {
    return bench_create(fx_part(device));
}

/* Runs 50 ns; an engine error (a short between drivers) fails the test. */
static void step(Bench *b) {
    bench_run(b, NS(50));
    if (bench_error(b) != NULL) {
        printf("FAIL: engine error: %s\n", bench_error(b));
        failures++;
    }
}

static void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

static LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Sets pins fmt % (i + 1) for i < n from value's bits. */
static void set_bus(Bench *b, const char *fmt, unsigned n, unsigned value) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, i + 1);
        set(b, name, bit(value, i));
    }
}

/* Releases pins fmt % (i + 1) for i < n. */
static void release_bus(Bench *b, const char *fmt, unsigned n) {
    char name[32];
    unsigned i;

    for (i = 0; i < n; i++) {
        snprintf(name, sizeof name, fmt, i + 1);
        set(b, name, LZ);
    }
}

static int bus_is(Bench *b, const char *fmt, unsigned n, const LogicValue *want) {
    char name[32];
    unsigned i;
    int ok = 1;

    for (i = 0; i < n; i++) {
        LogicValue got;

        snprintf(name, sizeof name, fmt, i + 1);
        got = bench_get(b, name);
        if (got != want[i]) {
            printf("  %s = %c, want %c\n", name, "01XZ"[got], "01XZ"[want[i]]);
            ok = 0;
        }
    }
    return ok;
}

static int bus_value(Bench *b, const char *fmt, unsigned n, unsigned value) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = bit(value, i);
    }
    return bus_is(b, fmt, n, want);
}

static int bus_all(Bench *b, const char *fmt, unsigned n, LogicValue v) {
    LogicValue want[64];
    unsigned i;

    for (i = 0; i < n; i++) {
        want[i] = v;
    }
    return bus_is(b, fmt, n, want);
}

static int is(Bench *b, const char *pin, LogicValue v) {
    LogicValue got = bench_get(b, pin);

    if (got != v) {
        printf("  %s = %c, want %c\n", pin, "01XZ"[got], "01XZ"[v]);
        return 0;
    }
    return 1;
}

/* ---- tristate ------------------------------------------------------------- */

static void check_244(void) {
    Bench *b = open_fx("74_244");

    set(b, "1OE", L0);
    set(b, "2OE", L1);
    set_bus(b, "1A%u", 4, 0x5);
    set_bus(b, "2A%u", 4, 0xF);
    step(b);
    expect(bus_value(b, "1Y%u", 4, 0x5) && bus_all(b, "2Y%u", 4, LZ),
           "'244: 1OE low drives 1Y = 1A; 2OE high leaves 2Y at Z");
    set_bus(b, "1A%u", 4, 0xA);
    set(b, "2OE", L0);
    step(b);
    expect(bus_value(b, "1Y%u", 4, 0xA) && bus_value(b, "2Y%u", 4, 0xF),
           "'244: both units enabled, each Y follows its A");
    set(b, "1A1", LX);
    set(b, "1A2", LZ);
    step(b);
    expect(is(b, "1Y1", LX) && is(b, "1Y2", LX) && is(b, "1Y3", L0) && is(b, "1Y4", L1),
           "'244: A = X gives X; a floating A gives X; other bits unchanged");
    set(b, "1OE", LX);
    step(b);
    expect(bus_all(b, "1Y%u", 4, LX) && bus_value(b, "2Y%u", 4, 0xF),
           "'244: X on 1OE makes every 1Y X, 2Y unaffected");
    set(b, "1OE", L1);
    step(b);
    expect(bus_all(b, "1Y%u", 4, LZ), "'244: 1OE high again: 1Y back to Z");
    bench_destroy(b);
}

static void check_240(void) {
    Bench *b = open_fx("74_240");

    set(b, "1OE", L0);
    set(b, "2OE", L0);
    set_bus(b, "1A%u", 4, 0x3);
    set_bus(b, "2A%u", 4, 0x9);
    step(b);
    expect(bus_value(b, "1Y%u", 4, 0xC) && bus_value(b, "2Y%u", 4, 0x6), "'240: Y = NOT A");
    bench_destroy(b);
}

static void check_541(void) {
    Bench *b = open_fx("74_541");
    static const LogicValue oe[4][2] = {{LOGIC_0, LOGIC_0}, {LOGIC_0, LOGIC_1},
                                        {LOGIC_1, LOGIC_0}, {LOGIC_1, LOGIC_1}};
    int ok = 1;
    unsigned i;

    set_bus(b, "A%u", 8, 0xA5);
    for (i = 0; i < 4; i++) {
        set(b, "OE1", oe[i][0]);
        set(b, "OE2", oe[i][1]);
        step(b);
        if (i == 0) {
            ok &= bus_value(b, "Y%u", 8, 0xA5);
        } else {
            ok &= bus_all(b, "Y%u", 8, LZ);
        }
    }
    expect(ok, "'541: Y = A only while OE1 and OE2 are both low");
    set(b, "OE1", L1);
    set(b, "OE2", LX);
    step(b);
    expect(bus_all(b, "Y%u", 8, LZ), "'541: one OE high disables even with X on the other");
    set(b, "OE1", L0);
    step(b);
    expect(bus_all(b, "Y%u", 8, LX), "'541: OE1 low and OE2 X: every Y X");
    bench_destroy(b);
}

static void check_1g125_126(void) {
    Bench *b = open_fx("1G125");
    Bench *c = open_fx("1G126");

    set(b, "A", L1);
    set(c, "A", L1);
    set(b, "OE", L0);
    set(c, "OE", L0);
    step(b);
    step(c);
    expect(is(b, "Y", L1) && is(c, "Y", LZ), "1G125 enabled by OE low; 1G126 off at OE low");
    set(b, "OE", L1);
    set(c, "OE", L1);
    step(b);
    step(c);
    expect(is(b, "Y", LZ) && is(c, "Y", L1), "1G125 off at OE high; 1G126 enabled by OE high");
    bench_destroy(b);
    bench_destroy(c);
}

static void check_open_drain(void) {
    const Part *part = fx_part("1G125_OD");
    Bench *b = bench_create(part);

    set(b, "OE", L0);
    set(b, "A", L0);
    step(b);
    expect(is(b, "Y", L0), "open-drain 1G125: A = 0 drives Y low");
    set(b, "A", L1);
    step(b);
    expect(is(b, "Y", LZ), "open-drain 1G125: A = 1 releases Y (Z)");
    expect(part_pin_open_drain(part, 3) && !part_pin_open_drain(part, 1),
           "open-drain 1G125: Y is open drain, A is not");
    expect(!part_pin_open_drain(fx_part("1G125"), 3), "push-pull 1G125: Y is not open drain");
    bench_destroy(b);
}

/* ---- transceiver ---------------------------------------------------------- */

static void check_245(void) {
    Bench *b = open_fx("74_245");

    set(b, "OE", L0);
    set(b, "DIR", L1);
    set_bus(b, "A%u", 8, 0x3C);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x3C), "'245: DIR high: B = A");
    set_bus(b, "A%u", 8, 0x81);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x81), "'245: an external drive on A passes through to B");
    /* Turn around: release A, DIR low. B is still the part's own drive for
       10 ns; A must not pick that up (it shows X: B is floating). */
    release_bus(b, "A%u", 8);
    set(b, "DIR", L0);
    bench_run(b, NS(15));
    expect(bus_all(b, "A%u", 8, LX),
           "'245: after DIR goes low, A never shows the part's own old B drive");
    step(b);
    expect(bus_all(b, "B%u", 8, LZ) && bus_all(b, "A%u", 8, LX),
           "'245: DIR low: B released, A follows the floating B (X)");
    set_bus(b, "B%u", 8, 0x5A);
    step(b);
    expect(bus_value(b, "A%u", 8, 0x5A), "'245: DIR low: A = B");
    set(b, "OE", L1);
    release_bus(b, "B%u", 8);
    step(b);
    expect(bus_all(b, "A%u", 8, LZ) && bus_all(b, "B%u", 8, LZ), "'245: OE high: both ports Z");
    set(b, "OE", L0);
    set(b, "DIR", LX);
    set_bus(b, "A%u", 8, 0xFF);
    step(b);
    expect(bus_all(b, "B%u", 8, LX), "'245: X on DIR: B (which could be driven) is X");
    release_bus(b, "A%u", 8);
    step(b);
    expect(bus_all(b, "A%u", 8, LX) && bus_all(b, "B%u", 8, LX),
           "'245: X on DIR: both ports X");
    bench_destroy(b);
}

static void check_640(void) {
    Bench *b = open_fx("74_640");

    set(b, "OE", L0);
    set(b, "DIR", L1);
    set_bus(b, "A%u", 8, 0x0F);
    step(b);
    expect(bus_value(b, "B%u", 8, 0xF0), "'640: A to B inverted");
    release_bus(b, "A%u", 8);
    set(b, "DIR", L0);
    step(b);
    set_bus(b, "B%u", 8, 0x33);
    step(b);
    expect(bus_value(b, "A%u", 8, 0xCC), "'640: B to A inverted");
    bench_destroy(b);
}

static void check_641(void) {
    static const LogicValue want[8] = {LOGIC_Z, LOGIC_Z, LOGIC_Z, LOGIC_Z,
                                       LOGIC_0, LOGIC_0, LOGIC_0, LOGIC_0};
    const Part *part = fx_part("74_641");
    Bench *b = bench_create(part);

    set(b, "OE", L0);
    set(b, "DIR", L1);
    set_bus(b, "A%u", 8, 0x0F);
    step(b);
    expect(bus_is(b, "B%u", 8, want),
           "'641 (open drain): A to B drives the 0s and releases the 1s");
    expect(part_pin_open_drain(part, 1) && part_pin_open_drain(part, 17) &&
               !part_pin_open_drain(part, 0),
           "'641: both ports are open drain, DIR is not");
    bench_destroy(b);
}

static void check_623(void) {
    Bench *b = open_fx("74_623");

    set(b, "OEAB", L1);
    set(b, "OEBA", L1);
    set_bus(b, "A%u", 8, 0x96);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x96), "'623: OEAB high alone: B = A");
    set(b, "OEAB", L0);
    release_bus(b, "A%u", 8);
    step(b);
    expect(bus_all(b, "A%u", 8, LZ) && bus_all(b, "B%u", 8, LZ), "'623: both off: isolation");
    set(b, "OEBA", L0);
    set_bus(b, "B%u", 8, 0x69);
    step(b);
    expect(bus_value(b, "A%u", 8, 0x69), "'623: OEBA low alone: A = B");
    set(b, "OEAB", L1);
    step(b);
    release_bus(b, "B%u", 8);
    step(b);
    expect(bus_value(b, "A%u", 8, 0x69) && bus_value(b, "B%u", 8, 0x69),
           "'623: both enabled: the loop holds the value present when the second enable came");
    bench_destroy(b);
}

static void check_16245(void) {
    Bench *b = open_fx("74_16245");

    set(b, "1OE", L0);
    set(b, "1DIR", L1);
    set(b, "2OE", L0);
    set(b, "2DIR", L0);
    set(b, "1A1", L1);
    set(b, "1A2", L0);
    set(b, "2B1", L0);
    set(b, "2B2", L1);
    step(b);
    expect(is(b, "1B1", L1) && is(b, "1B2", L0) && is(b, "2A1", L0) && is(b, "2A2", L1),
           "16245: unit 1 A to B, unit 2 B to A at the same time");
    set(b, "2OE", L1);
    step(b);
    expect(is(b, "1B1", L1) && is(b, "2A1", LZ), "16245: disabling unit 2 leaves unit 1 driving");
    bench_destroy(b);
}

/* ---- registered (plan-20-E) ----------------------------------------------- */

/* A rising edge on pin (0 -> 1, then back to 0). */
static void pulse(Bench *b, const char *pin) {
    set(b, pin, L0);
    step(b);
    set(b, pin, L1);
    step(b);
    set(b, pin, L0);
    step(b);
}

static void check_646(void) {
    Bench *b = open_fx("74_646");

    set(b, "CLKAB", L0);
    set(b, "CLKBA", L0);
    set(b, "OE", L1);
    set(b, "DIR", L1);
    set(b, "SAB", L0);
    set(b, "SBA", L0);
    set_bus(b, "A%u", 8, 0x3C);
    step(b);
    expect(bus_all(b, "B%u", 8, LZ), "'646: OE high: isolation, B is Z");
    pulse(b, "CLKAB");
    set(b, "OE", L0);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x3C), "'646: OE low, DIR high, SAB low: real-time A to B");
    set_bus(b, "A%u", 8, 0x81);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x81), "'646: real time follows A");
    set(b, "SAB", L1);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x3C),
           "'646: SAB high: B shows the A register (stored while OE was high)");
    /* Turn around: A released and DIR low first, then B driven (no contention). */
    set(b, "SAB", L0);
    release_bus(b, "A%u", 8);
    set(b, "DIR", L0);
    step(b);
    set_bus(b, "B%u", 8, 0x5A);
    step(b);
    expect(bus_value(b, "A%u", 8, 0x5A), "'646: DIR low, SBA low: real-time B to A");
    /* Store A in the A register while the part drives A from B. */
    pulse(b, "CLKAB");
    set(b, "OE", L1);
    step(b);
    release_bus(b, "B%u", 8);
    set(b, "DIR", L1);
    set(b, "SAB", L1);
    set(b, "OE", L0);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x5A),
           "'646: the A register stored the bus the part was driving (0x5A), stored transfer to B");
    bench_destroy(b);
}

static void check_652(void) {
    Bench *b = open_fx("74_652");

    set(b, "CLKAB", L0);
    set(b, "CLKBA", L0);
    set(b, "SAB", L0);
    set(b, "SBA", L0);
    set(b, "OEAB", L1);
    set(b, "OEBA", L1);
    set_bus(b, "A%u", 8, 0x5A);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x5A), "'652: OEAB high, OEBA high (off): real-time A to B");
    pulse(b, "CLKAB");
    set(b, "OEAB", L0);
    release_bus(b, "A%u", 8);
    step(b);
    set_bus(b, "B%u", 8, 0xA5);
    step(b);
    expect(bus_all(b, "A%u", 8, LZ), "'652: OEAB low, OEBA high: neither direction drives");
    pulse(b, "CLKBA");
    release_bus(b, "B%u", 8);
    set(b, "SAB", L1);
    set(b, "SBA", L1);
    set(b, "OEAB", L1);
    set(b, "OEBA", L0);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x5A) && bus_value(b, "A%u", 8, 0xA5),
           "'652: both enabled, stored select: B shows the A register, A the B register");
    bench_destroy(b);
}

static void check_543(void) {
    Bench *b = open_fx("74_543");

    set(b, "CEBA", L1);
    set(b, "LEBA", L1);
    set(b, "OEBA", L1);
    set(b, "CEAB", L0);
    set(b, "LEAB", L0);
    set(b, "OEAB", L0);
    set_bus(b, "A%u", 8, 0x96);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x96), "'543: CEAB, LEAB, OEAB low: transparent A to B");
    set(b, "LEAB", L1);
    step(b);
    set_bus(b, "A%u", 8, 0x0F);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x96), "'543: LEAB high: the latch holds while A changes");
    set(b, "CEAB", L1);
    step(b);
    expect(bus_all(b, "B%u", 8, LZ), "'543: CEAB high: B outputs off (CE gates OE)");
    set(b, "CEAB", L0);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x96), "'543: CEAB low again: the held value is back on B");
    set(b, "LEAB", L0);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x0F), "'543: LEAB low: transparent again");
    bench_destroy(b);
}

static void check_2952(void) {
    Bench *b = open_fx("74_2952");

    set(b, "CLKAB", L0);
    set(b, "CLKBA", L0);
    set(b, "OEBA", L1);
    set(b, "OEAB", L0);
    set(b, "CLKENAB", L1);
    set_bus(b, "A%u", 8, 0xC3);
    step(b);
    expect(bus_value(b, "B%u", 8, 0x00), "'2952: B shows the register (0 at power-on)");
    pulse(b, "CLKAB");
    expect(bus_value(b, "B%u", 8, 0x00), "'2952: CLKENAB high: the clock edge is ignored");
    set(b, "CLKENAB", L0);
    pulse(b, "CLKAB");
    expect(bus_value(b, "B%u", 8, 0xC3), "'2952: CLKENAB low: the edge loads A");
    set_bus(b, "A%u", 8, 0x3C);
    step(b);
    expect(bus_value(b, "B%u", 8, 0xC3), "'2952: no real-time path: B keeps the register");
    set(b, "CLKAB", LX);
    step(b);
    expect(bus_all(b, "B%u", 8, LX), "X on a clock (from 0): every bit that would change is X");
    bench_destroy(b);
}

static void check_16952(void) {
    Bench *b = open_fx("74_16952");

    set(b, "1CLKAB", L0);
    set(b, "2CLKAB", L0);
    set(b, "1OEAB", L0);
    set(b, "2OEAB", L0);
    set(b, "1OEBA", L1);
    set(b, "2OEBA", L1);
    set(b, "1CLKENAB", L0);
    set(b, "2CLKENAB", L0);
    set(b, "1A1", L1);
    set(b, "1A2", L1);
    set(b, "2A1", L1);
    set(b, "2A2", L1);
    step(b);
    pulse(b, "1CLKAB");
    expect(is(b, "1B1", L1) && is(b, "1B2", L1) && is(b, "2B1", L0) && is(b, "2B2", L0),
           "16952: clocking unit 1 leaves unit 2's register alone");
    bench_destroy(b);
}

/* ---- translator (plan-20-F) ----------------------------------------------- */

static void check_translators(void) {
    Bench *b = open_fx("1T45");
    Bench *c = open_fx("74_8T245");
    Bench *d = open_fx("TXU0104");
    const Part *t45 = fx_part("1T45");

    expect(t45->domain_count == 2 && t45->domains[0].power_pin_count == 1 &&
               t45->domains[0].power_pins[0] == 0 && t45->domains[0].pin_count == 2 &&
               t45->domains[1].power_pins[0] == 5 && t45->domains[1].pin_count == 1 &&
               t45->domains[1].pins[0] == 3 && t45->domains[1].vcc != NULL &&
               t45->domains[1].vcc->max_mv == 5500 && t45->domains[0].vcc == t45->vcc,
           "1T45: two domains (VCCA: A, DIR; VCCB: B), domain A's range is Part.vcc");
    set(b, "DIR", L1);
    set(b, "A", L1);
    step(b);
    expect(is(b, "B", L1), "1T45: DIR high: B = A (no OE)");
    set(b, "A", LZ);
    set(b, "DIR", L0);
    step(b);
    set(b, "B", L0);
    step(b);
    expect(is(b, "A", L0), "1T45: DIR low: A = B");

    set(c, "DIR", L1);
    set(c, "OE", L1);
    set(c, "A1", L1);
    set(c, "A2", L0);
    step(c);
    expect(is(c, "B1", LZ) && is(c, "B2", LZ), "8T245: OE high: B is Z");
    set(c, "OE", L0);
    step(c);
    expect(is(c, "B1", L1) && is(c, "B2", L0), "8T245: OE low, DIR high: B = A");

    set(d, "OE", L1);
    set(d, "A1", L0);
    set(d, "A2", L1);
    step(d);
    expect(is(d, "Y1", L0) && is(d, "Y2", L1), "TXU0104: OE high: fixed direction, Y = A");
    set(d, "OE", L0);
    step(d);
    expect(is(d, "Y1", LZ) && is(d, "Y2", LZ), "TXU0104: OE low: Y is Z");
    bench_destroy(b);
    bench_destroy(c);
    bench_destroy(d);
}

/* ---- Loader errors -------------------------------------------------------- */

#define HEAD "{'format':'schematical-parts','version':1,'kind':'buffer','devices':["
#define DEV(MODEL, KEYS)                                                                     \
    "{'id':'e','function':'Buffer','description':'d','drawer':[],'pinouts':{'p5':["          \
    "{'number':1,'name':'OE','role':'input'},{'number':2,'name':'A','role':'inout'},"       \
    "{'number':3,'name':'GND','role':'ground'},{'number':4,'name':'Y','role':'inout'},"     \
    "{'number':5,'name':'VCC','role':'power'}]},'model':'" MODEL "'," KEYS ","              \
    "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},"                                       \
    "'parts':[{'name':'E1','package':'DIP','pinout':'p5','footprint':'FX5','family':'HC'}]}]}"

#define DEV6(MODEL, KEYS, TIMING, PARTX)                                                     \
    "{'id':'e','function':'Level shifter','description':'d','drawer':[],'pinouts':{'p6':["   \
    "{'number':1,'name':'VCCA','role':'power'},{'number':2,'name':'GND','role':'ground'},"  \
    "{'number':3,'name':'A','role':'inout'},{'number':4,'name':'B','role':'inout'},"        \
    "{'number':5,'name':'DIR','role':'input'},{'number':6,'name':'VCCB','role':'power'}]}," \
    "'model':'" MODEL "'," KEYS ",'timing':{'LVC':{'rise':1,'fall':1,'vcc':[1.65,5.5]" TIMING "}}," \
    "'parts':[{'name':'E1','package':'SOT-23','pinout':'p6','footprint':'FX6','family':'LVC'" PARTX "}]}]}"
#define XUNITS "'units':[{'a':['A'],'b':['B'],'dir':{'pin':'DIR','ab':'high'}}]"
#define DOMAINS(APINS) ",'domains':[{'power':'VCCA','pins':[" APINS "]},{'power':'VCCB','pins':['B']}]"

static void expect_fail(const char *what, const char *text, const char *message) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "e.json");
    int ok = error != NULL && strstr(error, message) != NULL;

    if (!ok) {
        printf("  got: %s\n  want: %s\n", error != NULL ? error : "(loaded)", message);
    }
    expect(ok, what);
    free(error);
    part_library_destroy(lib);
}

static void check_loader(void) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib,
                       HEAD DEV("tristate",
                                "'units':[{'a':['A'],'y':['Y'],'oe':{'pin':'OE','active':'low'}}]"),
                       "ok.json");

    expect(error == NULL, "a minimal tristate device loads");
    free(error);
    part_library_destroy(lib);
    expect_fail("unknown model", HEAD DEV("tri", "'units':[]"), "unknown model \"tri\"");
    expect_fail("unknown device key",
                HEAD DEV("tristate", "'units':[{'a':['A'],'y':['Y']}],'dir':1"),
                "unknown key \"dir\"");
    expect_fail("no units", HEAD DEV("tristate", "'units':[]"),
                "\"units\" must be an array of 1 to 8 units");
    expect_fail("a and y differ", HEAD DEV("tristate", "'units':[{'a':['A','OE'],'y':['Y']}]"),
                "\"a\" and \"y\" must have the same number of names");
    expect_fail("unknown unit key",
                HEAD DEV("tristate", "'units':[{'a':['A'],'y':['Y'],'b':['Y']}]"),
                "unknown key \"b\"");
    expect_fail("enable not a control",
                HEAD DEV("tristate", "'units':[{'a':['A'],'y':['Y'],'oe':'OE'}]"),
                "oe: must be a control or an array of controls");
    expect_fail("enable with a bad level",
                HEAD DEV("tristate",
                         "'units':[{'a':['A'],'y':['Y'],'oe':{'pin':'OE','active':'lo'}}]"),
                "\"active\" must be \"high\" or \"low\", not \"lo\"");
    expect_fail("pin name not in the pinout",
                HEAD DEV("tristate", "'units':[{'a':['A'],'y':['Q']}]"), "no pin named \"Q\"");
    expect_fail("bad output style",
                HEAD DEV("tristate", "'units':[{'a':['A'],'y':['Y']}],'output':'od'"),
                "\"output\" must be \"push-pull\" or \"open-drain\"");
    expect_fail("transceiver a and b differ",
                HEAD DEV("transceiver",
                         "'units':[{'a':['A','OE'],'b':['Y'],'oeab':{'pin':'OE','active':'high'},"
                         "'oeba':{'pin':'OE','active':'low'}}]"),
                "\"a\" and \"b\" must have the same number of names");
    expect_fail("transceiver separate form needs oeba",
                HEAD DEV("transceiver",
                         "'units':[{'a':['A'],'b':['Y'],'oeab':{'pin':'OE','active':'high'}}]"),
                "missing \"oeba\"");
    expect_fail("transceiver dir without ab",
                HEAD DEV("transceiver", "'units':[{'a':['A'],'b':['Y'],'dir':{'pin':'OE'}}]"),
                "missing \"ab\"");
    expect_fail("transceiver dir and oeab together",
                HEAD DEV("transceiver",
                         "'units':[{'a':['A'],'b':['Y'],'dir':{'pin':'OE','ab':'high'},"
                         "'oeab':{'pin':'OE','active':'high'}}]"),
                "unknown key \"oeab\"");
    expect_fail("registered store neither latch nor register",
                HEAD DEV("registered",
                         "'units':[{'a':['A'],'b':['Y'],'ab':{'store':'flop'},'ba':{'store':'flop'}}]"),
                "\"store\" must be \"latch\" or \"register\", not \"flop\"");
    expect_fail("registered latch without le",
                HEAD DEV("registered",
                         "'units':[{'a':['A'],'b':['Y'],'ab':{'store':'latch','oe':[]},"
                         "'ba':{'store':'latch','oe':[]}}]"),
                "missing \"le\"");
    expect_fail("registered register without clk",
                HEAD DEV("registered",
                         "'units':[{'a':['A'],'b':['Y'],'ab':{'store':'register','oe':[]},"
                         "'ba':{'store':'register','oe':[]}}]"),
                "missing \"clk\"");
    expect_fail("registered latch with a clock",
                HEAD DEV("registered",
                         "'units':[{'a':['A'],'b':['Y'],'ab':{'store':'latch','le':{'pin':'OE','active':'low'},"
                         "'clk':{'pin':'OE','edge':'rising'},'oe':[]},'ba':{'store':'latch','le':{'pin':'OE','active':'low'},'oe':[]}}]"),
                "unknown key \"clk\"");
    expect_fail("registered dir form with a direction oe",
                HEAD DEV("registered",
                         "'units':[{'a':['A'],'b':['Y'],'dir':{'pin':'OE','ab':'high'},"
                         "'ab':{'store':'register','clk':{'pin':'OE','edge':'rising'},'oe':[]},"
                         "'ba':{'store':'register','clk':{'pin':'OE','edge':'rising'}}}]"),
                "\"oe\" is on the unit when the unit has \"dir\"");
    expect_fail("registered direction missing",
                HEAD DEV("registered",
                         "'units':[{'a':['A'],'b':['Y'],'ab':{'store':'register','clk':{'pin':'OE','edge':'rising'},'oe':[]}}]"),
                "missing \"ba\"");
    expect_fail("domains: a pin in no domain",
                HEAD DEV6("translator", XUNITS DOMAINS("'A'"), ",'vccB':[1.65,5.5]", ""),
                "\"domains\": pin \"DIR\" is in no domain");
    expect_fail("domains: a pin in two domains",
                HEAD DEV6("translator", XUNITS DOMAINS("'A','DIR','B'"), ",'vccB':[1.65,5.5]", ""),
                "\"domains\": pin \"B\" is in two domains");
    expect_fail("domains: power is not a power pin",
                HEAD DEV6("translator", XUNITS ",'domains':[{'power':'DIR','pins':['A']},"
                          "{'power':'VCCB','pins':['B']}]", ",'vccB':[1.65,5.5]", ""),
                "\"domains\": \"DIR\" is not a power pin");
    expect_fail("vccB on a part without domains",
                HEAD DEV6("transceiver", XUNITS, "", ",'vccB':[1.65,5.5]"),
                "\"vccB\" without \"domains\"");
    expect_fail("vccB in timing without domains",
                HEAD DEV6("transceiver", XUNITS, ",'vccB':[1.65,5.5]", ""),
                "unknown key \"vccB\"");
    expect_fail("domains on a model that has none",
                HEAD DEV6("transceiver", XUNITS DOMAINS("'A','DIR'"), ",'vccB':[1.65,5.5]", ""),
                "\"domains\" is not a key of model \"transceiver\"");
    expect_fail("translator without domains",
                HEAD DEV6("translator", XUNITS, ",'vccB':[1.65,5.5]", ""),
                "missing \"domains\"");
    expect_fail("translator timing without vccB",
                HEAD DEV6("translator", XUNITS DOMAINS("'A','DIR'"), "", ""),
                "timing.LVC: missing \"vccB\"");
    expect_fail("inputMaxB below the vccB max",
                HEAD DEV6("translator", XUNITS DOMAINS("'A','DIR'"), ",'vccB':[1.65,5.5],'inputMaxB':3", ""),
                "\"inputMaxB\" below the \"vccB\" max");
}

int main(void) {
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "buffer_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_244();
    check_240();
    check_541();
    check_1g125_126();
    check_open_drain();
    check_245();
    check_640();
    check_641();
    check_623();
    check_16245();
    check_646();
    check_652();
    check_543();
    check_2952();
    check_16952();
    check_translators();
    check_loader();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("buffer_test: all passed\n");
    return 0;
}
