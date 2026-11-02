/*
 * counter_test: the counter kind's sync and ripple models against the
 * function tables of the devices that use them (plan-10-D section 6).
 * Fixture devices copy the real devices' pinouts and keys from the spec
 * sections (no counter part file exists before plan-10-F), with 10 ns
 * delays (ripple: 5 ns per stage); levels are checked 50 ns after each step.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "kind.h"
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
    "{'format':'schematical-parts','version':1,'kind':'counter','devices':[{'id':'fx-74_161',"
    "'function':'Counter','description':'Synchronous 4-bit binary counter, asynchronous clear"
    "','drawer':[],'pinouts':{'p16':[{'number':1,'name':'CLR','role':'input'},{'number':2,'na"
    "me':'CLK','role':'input'},{'number':3,'name':'A','role':'input'},{'number':4,'name':'B',"
    "'role':'input'},{'number':5,'name':'C','role':'input'},{'number':6,'name':'D','role':'in"
    "put'},{'number':7,'name':'ENP','role':'input'},{'number':8,'name':'GND','role':'ground'}"
    ",{'number':9,'name':'LOAD','role':'input'},{'number':10,'name':'ENT','role':'input'},{'n"
    "umber':11,'name':'QD','role':'output'},{'number':12,'name':'QC','role':'output'},{'numbe"
    "r':13,'name':'QB','role':'output'},{'number':14,'name':'QA','role':'output'},{'number':1"
    "5,'name':'RCO','role':'output'},{'number':16,'name':'VCC','role':'power'}]},'model':'syn"
    "c','units':[{'stages':4,'code':'binary','q':['QA','QB','QC','QD'],'clock':{'pin':'CLK','"
    "edge':'rising'},'count':'up','enables':[{'pin':'ENP','active':'high'},{'pin':'ENT','acti"
    "ve':'high'}],'clears':[{'pin':'CLR','active':'low','when':'async','to':'zero'}],'loads':"
    "[{'pin':'LOAD','active':'low','when':'clock','from':'data'}],'data':['A','B','C','D'],'c"
    "arries':[{'pin':'RCO','active':'high','at':'max','gates':[{'pin':'ENT','active':'high'}]"
    "}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_161','pac"
    "kage':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_163','functi"
    "on':'Counter','description':'Synchronous 4-bit binary counter, synchronous clear','drawe"
    "r':[],'pinouts':{'p16':[{'number':1,'name':'CLR','role':'input'},{'number':2,'name':'CLK"
    "','role':'input'},{'number':3,'name':'A','role':'input'},{'number':4,'name':'B','role':'"
    "input'},{'number':5,'name':'C','role':'input'},{'number':6,'name':'D','role':'input'},{'"
    "number':7,'name':'ENP','role':'input'},{'number':8,'name':'GND','role':'ground'},{'numbe"
    "r':9,'name':'LOAD','role':'input'},{'number':10,'name':'ENT','role':'input'},{'number':1"
    "1,'name':'QD','role':'output'},{'number':12,'name':'QC','role':'output'},{'number':13,'n"
    "ame':'QB','role':'output'},{'number':14,'name':'QA','role':'output'},{'number':15,'name'"
    ":'RCO','role':'output'},{'number':16,'name':'VCC','role':'power'}]},'model':'sync','unit"
    "s':[{'stages':4,'code':'binary','q':['QA','QB','QC','QD'],'clock':{'pin':'CLK','edge':'r"
    "ising'},'count':'up','enables':[{'pin':'ENP','active':'high'},{'pin':'ENT','active':'hig"
    "h'}],'clears':[{'pin':'CLR','active':'low','when':'clock','to':'zero'}],'loads':[{'pin':"
    "'LOAD','active':'low','when':'clock','from':'data'}],'data':['A','B','C','D'],'carries':"
    "[{'pin':'RCO','active':'high','at':'max','gates':[{'pin':'ENT','active':'high'}]}]}],'ti"
    "ming':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_163','package':'D"
    "IP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_169','function':'Cou"
    "nter','description':'Synchronous 4-bit up/down binary counter','drawer':[],'pinouts':{'p"
    "16':[{'number':1,'name':'U/D','role':'input'},{'number':2,'name':'CLK','role':'input'},{"
    "'number':3,'name':'A','role':'input'},{'number':4,'name':'B','role':'input'},{'number':5"
    ",'name':'C','role':'input'},{'number':6,'name':'D','role':'input'},{'number':7,'name':'E"
    "NP','role':'input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'LOAD',"
    "'role':'input'},{'number':10,'name':'ENT','role':'input'},{'number':11,'name':'QD','role"
    "':'output'},{'number':12,'name':'QC','role':'output'},{'number':13,'name':'QB','role':'o"
    "utput'},{'number':14,'name':'QA','role':'output'},{'number':15,'name':'RCO','role':'outp"
    "ut'},{'number':16,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages':4,'co"
    "de':'binary','q':['QA','QB','QC','QD'],'clock':{'pin':'CLK','edge':'rising'},'direction'"
    ":{'pin':'U/D','up':'high'},'enables':[{'pin':'ENP','active':'low'},{'pin':'ENT','active'"
    ":'low'}],'loads':[{'pin':'LOAD','active':'low','when':'clock','from':'data'}],'data':['A"
    "','B','C','D'],'carries':[{'pin':'RCO','active':'low','at':'terminal','gates':[{'pin':'E"
    "NT','active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'na"
    "me':'FX-74_169','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id'"
    ":'fx-74_190','function':'Counter','description':'Synchronous up/down BCD counter','drawe"
    "r':[],'pinouts':{'p16':[{'number':1,'name':'B','role':'input'},{'number':2,'name':'QB','"
    "role':'output'},{'number':3,'name':'QA','role':'output'},{'number':4,'name':'CTEN','role"
    "':'input'},{'number':5,'name':'D/U','role':'input'},{'number':6,'name':'QC','role':'outp"
    "ut'},{'number':7,'name':'QD','role':'output'},{'number':8,'name':'GND','role':'ground'},"
    "{'number':9,'name':'D','role':'input'},{'number':10,'name':'C','role':'input'},{'number'"
    ":11,'name':'LOAD','role':'input'},{'number':12,'name':'MAX/MIN','role':'output'},{'numbe"
    "r':13,'name':'RCO','role':'output'},{'number':14,'name':'CLK','role':'input'},{'number':"
    "15,'name':'A','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'sync'"
    ",'units':[{'stages':4,'code':'bcd','q':['QA','QB','QC','QD'],'clock':{'pin':'CLK','edge'"
    ":'rising'},'direction':{'pin':'D/U','up':'low'},'enables':[{'pin':'CTEN','active':'low'}"
    "],'loads':[{'pin':'LOAD','active':'low','when':'async','from':'data'}],'data':['A','B','"
    "C','D'],'carries':[{'pin':'MAX/MIN','active':'high','at':'terminal'},{'pin':'RCO','activ"
    "e':'low','at':'terminal','gates':[{'pin':'CTEN','active':'low'}],'clockLow':'CLK'}],'ill"
    "egal':{'up':[1,2,3,4,5,6,7,8,9,0,9,4,9,0,9,0],'down':[9,0,1,2,3,4,5,6,7,8,9,10,3,4,5,6]}"
    "}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_190','packa"
    "ge':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_191','function"
    "':'Counter','description':'Synchronous up/down 4-bit binary counter','drawer':[],'pinout"
    "s':{'p16':[{'number':1,'name':'B','role':'input'},{'number':2,'name':'QB','role':'output"
    "'},{'number':3,'name':'QA','role':'output'},{'number':4,'name':'CTEN','role':'input'},{'"
    "number':5,'name':'D/U','role':'input'},{'number':6,'name':'QC','role':'output'},{'number"
    "':7,'name':'QD','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,'"
    "name':'D','role':'input'},{'number':10,'name':'C','role':'input'},{'number':11,'name':'L"
    "OAD','role':'input'},{'number':12,'name':'MAX/MIN','role':'output'},{'number':13,'name':"
    "'RCO','role':'output'},{'number':14,'name':'CLK','role':'input'},{'number':15,'name':'A'"
    ",'role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'sync','units':[{'s"
    "tages':4,'code':'binary','q':['QA','QB','QC','QD'],'clock':{'pin':'CLK','edge':'rising'}"
    ",'direction':{'pin':'D/U','up':'low'},'enables':[{'pin':'CTEN','active':'low'}],'loads':"
    "[{'pin':'LOAD','active':'low','when':'async','from':'data'}],'data':['A','B','C','D'],'c"
    "arries':[{'pin':'MAX/MIN','active':'high','at':'terminal'},{'pin':'RCO','active':'low','"
    "at':'terminal','gates':[{'pin':'CTEN','active':'low'}],'clockLow':'CLK'}]}],'timing':{'H"
    "C':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_191','package':'DIP','pino"
    "ut':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_192','function':'Counter','de"
    "scription':'Synchronous up/down BCD counter with dual clock and clear','drawer':[],'pino"
    "uts':{'p16':[{'number':1,'name':'B','role':'input'},{'number':2,'name':'QB','role':'outp"
    "ut'},{'number':3,'name':'QA','role':'output'},{'number':4,'name':'DOWN','role':'input'},"
    "{'number':5,'name':'UP','role':'input'},{'number':6,'name':'QC','role':'output'},{'numbe"
    "r':7,'name':'QD','role':'output'},{'number':8,'name':'GND','role':'ground'},{'number':9,"
    "'name':'D','role':'input'},{'number':10,'name':'C','role':'input'},{'number':11,'name':'"
    "LOAD','role':'input'},{'number':12,'name':'CO','role':'output'},{'number':13,'name':'BO'"
    ",'role':'output'},{'number':14,'name':'CLR','role':'input'},{'number':15,'name':'A','rol"
    "e':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages"
    "':4,'code':'bcd','q':['QA','QB','QC','QD'],'clockUp':'UP','clockDown':'DOWN','clears':[{"
    "'pin':'CLR','active':'high','when':'async','to':'zero'}],'loads':[{'pin':'LOAD','active'"
    ":'low','when':'async','from':'data'}],'data':['A','B','C','D'],'carries':[{'pin':'CO','a"
    "ctive':'low','at':'max','clockLow':'UP'},{'pin':'BO','active':'low','at':'min','clockLow"
    "':'DOWN'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_1"
    "92','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_193'"
    ",'function':'Counter','description':'Synchronous up/down 4-bit binary counter with dual "
    "clock and clear','drawer':[],'pinouts':{'p16':[{'number':1,'name':'B','role':'input'},{'"
    "number':2,'name':'QB','role':'output'},{'number':3,'name':'QA','role':'output'},{'number"
    "':4,'name':'DOWN','role':'input'},{'number':5,'name':'UP','role':'input'},{'number':6,'n"
    "ame':'QC','role':'output'},{'number':7,'name':'QD','role':'output'},{'number':8,'name':'"
    "GND','role':'ground'},{'number':9,'name':'D','role':'input'},{'number':10,'name':'C','ro"
    "le':'input'},{'number':11,'name':'LOAD','role':'input'},{'number':12,'name':'CO','role':"
    "'output'},{'number':13,'name':'BO','role':'output'},{'number':14,'name':'CLR','role':'in"
    "put'},{'number':15,'name':'A','role':'input'},{'number':16,'name':'VCC','role':'power'}]"
    "},'model':'sync','units':[{'stages':4,'code':'binary','q':['QA','QB','QC','QD'],'clockUp"
    "':'UP','clockDown':'DOWN','clears':[{'pin':'CLR','active':'high','when':'async','to':'ze"
    "ro'}],'loads':[{'pin':'LOAD','active':'low','when':'async','from':'data'}],'data':['A','"
    "B','C','D'],'carries':[{'pin':'CO','active':'low','at':'max','clockLow':'UP'},{'pin':'BO"
    "','active':'low','at':'min','clockLow':'DOWN'}]}],'timing':{'HC':{'rise':10,'fall':10,'v"
    "cc':[2,6]}},'parts':[{'name':'FX-74_193','package':'DIP','pinout':'p16','footprint':'FX1"
    "6','family':'HC'}]},{'id':'fx-74_561','function':'Counter','description':'Synchronous 4-"
    "bit counter with 3-state outputs','drawer':[],'pinouts':{'p20':[{'number':1,'name':'ALOA"
    "D','role':'input'},{'number':2,'name':'CLK','role':'input'},{'number':3,'name':'A','role"
    "':'input'},{'number':4,'name':'B','role':'input'},{'number':5,'name':'C','role':'input'}"
    ",{'number':6,'name':'D','role':'input'},{'number':7,'name':'ENP','role':'input'},{'numbe"
    "r':8,'name':'ACLR','role':'input'},{'number':9,'name':'SCLR','role':'input'},{'number':1"
    "0,'name':'GND','role':'ground'},{'number':11,'name':'SLOAD','role':'input'},{'number':12"
    ",'name':'ENT','role':'input'},{'number':13,'name':'QD','role':'output'},{'number':14,'na"
    "me':'QC','role':'output'},{'number':15,'name':'QB','role':'output'},{'number':16,'name':"
    "'QA','role':'output'},{'number':17,'name':'OE','role':'input'},{'number':18,'name':'CCO'"
    ",'role':'output'},{'number':19,'name':'RCO','role':'output'},{'number':20,'name':'VCC','"
    "role':'power'}]},'model':'sync','units':[{'stages':4,'code':'binary','q':['QA','QB','QC'"
    ",'QD'],'clock':{'pin':'CLK','edge':'rising'},'count':'up','enables':[{'pin':'ENP','activ"
    "e':'high'},{'pin':'ENT','active':'high'}],'clears':[{'pin':'ACLR','active':'low','when':"
    "'async','to':'zero'},{'pin':'SCLR','active':'low','when':'clock','to':'zero'}],'loads':["
    "{'pin':'ALOAD','active':'low','when':'async','from':'data'},{'pin':'SLOAD','active':'low"
    "','when':'clock','from':'data'}],'data':['A','B','C','D'],'outputEnable':{'pin':'OE','ac"
    "tive':'low'},'carries':[{'pin':'RCO','active':'high','at':'max','gates':[{'pin':'ENT','a"
    "ctive':'high'}]},{'pin':'CCO','active':'high','at':'max','gates':[{'pin':'ENT','active':"
    "'high'},{'pin':'ENP','active':'high'}],'clockLow':'CLK'}]}],'timing':{'HC':{'rise':10,'f"
    "all':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_561','package':'DIP','pinout':'p20','footp"
    "rint':'FX20','family':'HC'}]},{'id':'fx-74_569','function':'Counter','description':'Sync"
    "hronous 4-bit up/down binary counter with 3-state outputs','drawer':[],'pinouts':{'p20':"
    "[{'number':1,'name':'U/D','role':'input'},{'number':2,'name':'CLK','role':'input'},{'num"
    "ber':3,'name':'A','role':'input'},{'number':4,'name':'B','role':'input'},{'number':5,'na"
    "me':'C','role':'input'},{'number':6,'name':'D','role':'input'},{'number':7,'name':'ENP',"
    "'role':'input'},{'number':8,'name':'ACLR','role':'input'},{'number':9,'name':'SCLR','rol"
    "e':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'LOAD','role'"
    ":'input'},{'number':12,'name':'ENT','role':'input'},{'number':13,'name':'QD','role':'out"
    "put'},{'number':14,'name':'QC','role':'output'},{'number':15,'name':'QB','role':'output'"
    "},{'number':16,'name':'QA','role':'output'},{'number':17,'name':'OE','role':'input'},{'n"
    "umber':18,'name':'CCO','role':'output'},{'number':19,'name':'RCO','role':'output'},{'num"
    "ber':20,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages':4,'code':'binar"
    "y','q':['QA','QB','QC','QD'],'clock':{'pin':'CLK','edge':'rising'},'direction':{'pin':'U"
    "/D','up':'high'},'enables':[{'pin':'ENP','active':'low'},{'pin':'ENT','active':'low'}],'"
    "clears':[{'pin':'ACLR','active':'low','when':'async','to':'zero'},{'pin':'SCLR','active'"
    ":'low','when':'clock','to':'zero'}],'loads':[{'pin':'LOAD','active':'low','when':'clock'"
    ",'from':'data'}],'data':['A','B','C','D'],'outputEnable':{'pin':'OE','active':'low'},'ca"
    "rries':[{'pin':'RCO','active':'low','at':'terminal','gates':[{'pin':'ENT','active':'low'"
    "}]},{'pin':'CCO','active':'low','at':'terminal','gates':[{'pin':'ENT','active':'low'},{'"
    "pin':'ENP','active':'low'}],'clockLow':'CLK'}]}],'timing':{'HC':{'rise':10,'fall':10,'vc"
    "c':[2,6]}},'parts':[{'name':'FX-74_569','package':'DIP','pinout':'p20','footprint':'FX20"
    "','family':'HC'}]},{'id':'fx-74_867','function':'Counter','description':'Synchronous 8-b"
    "it up/down counter, asynchronous clear','drawer':[],'pinouts':{'p24':[{'number':1,'name'"
    ":'S0','role':'input'},{'number':2,'name':'S1','role':'input'},{'number':3,'name':'A','ro"
    "le':'input'},{'number':4,'name':'B','role':'input'},{'number':5,'name':'C','role':'input"
    "'},{'number':6,'name':'D','role':'input'},{'number':7,'name':'E','role':'input'},{'numbe"
    "r':8,'name':'F','role':'input'},{'number':9,'name':'G','role':'input'},{'number':10,'nam"
    "e':'H','role':'input'},{'number':11,'name':'ENT','role':'input'},{'number':12,'name':'GN"
    "D','role':'ground'},{'number':13,'name':'RCO','role':'output'},{'number':14,'name':'CLK'"
    ",'role':'input'},{'number':15,'name':'QH','role':'output'},{'number':16,'name':'QG','rol"
    "e':'output'},{'number':17,'name':'QF','role':'output'},{'number':18,'name':'QE','role':'"
    "output'},{'number':19,'name':'QD','role':'output'},{'number':20,'name':'QC','role':'outp"
    "ut'},{'number':21,'name':'QB','role':'output'},{'number':22,'name':'QA','role':'output'}"
    ",{'number':23,'name':'ENP','role':'input'},{'number':24,'name':'VCC','role':'power'}]},'"
    "model':'sync','units':[{'stages':8,'code':'binary','q':['QA','QB','QC','QD','QE','QF','Q"
    "G','QH'],'clock':{'pin':'CLK','edge':'rising'},'data':['A','B','C','D','E','F','G','H'],"
    "'modes':{'pins':['S0','S1'],'table':['clear','down','load','up'],'clear':'async'},'enabl"
    "es':[{'pin':'ENP','active':'low'},{'pin':'ENT','active':'low'}],'carries':[{'pin':'RCO',"
    "'active':'low','at':'terminal','gates':[{'pin':'ENT','active':'low'}]}]}],'timing':{'HC'"
    ":{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_867','package':'DIP','pinout"
    "':'p24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_869','function':'Counter','desc"
    "ription':'Synchronous 8-bit up/down counter, synchronous clear','drawer':[],'pinouts':{'"
    "p24':[{'number':1,'name':'S0','role':'input'},{'number':2,'name':'S1','role':'input'},{'"
    "number':3,'name':'A','role':'input'},{'number':4,'name':'B','role':'input'},{'number':5,"
    "'name':'C','role':'input'},{'number':6,'name':'D','role':'input'},{'number':7,'name':'E'"
    ",'role':'input'},{'number':8,'name':'F','role':'input'},{'number':9,'name':'G','role':'i"
    "nput'},{'number':10,'name':'H','role':'input'},{'number':11,'name':'ENT','role':'input'}"
    ",{'number':12,'name':'GND','role':'ground'},{'number':13,'name':'RCO','role':'output'},{"
    "'number':14,'name':'CLK','role':'input'},{'number':15,'name':'QH','role':'output'},{'num"
    "ber':16,'name':'QG','role':'output'},{'number':17,'name':'QF','role':'output'},{'number'"
    ":18,'name':'QE','role':'output'},{'number':19,'name':'QD','role':'output'},{'number':20,"
    "'name':'QC','role':'output'},{'number':21,'name':'QB','role':'output'},{'number':22,'nam"
    "e':'QA','role':'output'},{'number':23,'name':'ENP','role':'input'},{'number':24,'name':'"
    "VCC','role':'power'}]},'model':'sync','units':[{'stages':8,'code':'binary','q':['QA','QB"
    "','QC','QD','QE','QF','QG','QH'],'clock':{'pin':'CLK','edge':'rising'},'data':['A','B','"
    "C','D','E','F','G','H'],'modes':{'pins':['S0','S1'],'table':['clear','down','load','up']"
    ",'clear':'clock'},'enables':[{'pin':'ENP','active':'low'},{'pin':'ENT','active':'low'}],"
    "'carries':[{'pin':'RCO','active':'low','at':'terminal','gates':[{'pin':'ENT','active':'l"
    "ow'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_869'"
    ",'package':'DIP','pinout':'p24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_867_AS'"
    ",'function':'Counter','description':'Synchronous 8-bit up/down counter, asynchronous cle"
    "ar','drawer':[],'pinouts':{'p24':[{'number':1,'name':'S0','role':'input'},{'number':2,'n"
    "ame':'S1','role':'input'},{'number':3,'name':'A','role':'input'},{'number':4,'name':'B',"
    "'role':'input'},{'number':5,'name':'C','role':'input'},{'number':6,'name':'D','role':'in"
    "put'},{'number':7,'name':'E','role':'input'},{'number':8,'name':'F','role':'input'},{'nu"
    "mber':9,'name':'G','role':'input'},{'number':10,'name':'H','role':'input'},{'number':11,"
    "'name':'ENT','role':'input'},{'number':12,'name':'GND','role':'ground'},{'number':13,'na"
    "me':'RCO','role':'output'},{'number':14,'name':'CLK','role':'input'},{'number':15,'name'"
    ":'QH','role':'output'},{'number':16,'name':'QG','role':'output'},{'number':17,'name':'QF"
    "','role':'output'},{'number':18,'name':'QE','role':'output'},{'number':19,'name':'QD','r"
    "ole':'output'},{'number':20,'name':'QC','role':'output'},{'number':21,'name':'QB','role'"
    ":'output'},{'number':22,'name':'QA','role':'output'},{'number':23,'name':'ENP','role':'i"
    "nput'},{'number':24,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages':8,'"
    "code':'binary','q':['QA','QB','QC','QD','QE','QF','QG','QH'],'clock':{'pin':'CLK','edge'"
    ":'rising'},'data':['A','B','C','D','E','F','G','H'],'modes':{'pins':['S0','S1'],'table':"
    "['clear','down','load','up'],'clear':'async'},'enables':[{'pin':'ENP','active':'low'},{'"
    "pin':'ENT','active':'low'}],'carries':[{'pin':'RCO','active':'low','at':'terminal','gate"
    "s':[{'pin':'ENT','active':'low'},{'pin':'ENP','active':'low'}]}]}],'timing':{'HC':{'rise"
    "':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_867_AS','package':'DIP','pinout':'p"
    "24','footprint':'FX24','family':'HC'}]},{'id':'fx-74_869_AS','function':'Counter','descr"
    "iption':'Synchronous 8-bit up/down counter, synchronous clear','drawer':[],'pinouts':{'p"
    "24':[{'number':1,'name':'S0','role':'input'},{'number':2,'name':'S1','role':'input'},{'n"
    "umber':3,'name':'A','role':'input'},{'number':4,'name':'B','role':'input'},{'number':5,'"
    "name':'C','role':'input'},{'number':6,'name':'D','role':'input'},{'number':7,'name':'E',"
    "'role':'input'},{'number':8,'name':'F','role':'input'},{'number':9,'name':'G','role':'in"
    "put'},{'number':10,'name':'H','role':'input'},{'number':11,'name':'ENT','role':'input'},"
    "{'number':12,'name':'GND','role':'ground'},{'number':13,'name':'RCO','role':'output'},{'"
    "number':14,'name':'CLK','role':'input'},{'number':15,'name':'QH','role':'output'},{'numb"
    "er':16,'name':'QG','role':'output'},{'number':17,'name':'QF','role':'output'},{'number':"
    "18,'name':'QE','role':'output'},{'number':19,'name':'QD','role':'output'},{'number':20,'"
    "name':'QC','role':'output'},{'number':21,'name':'QB','role':'output'},{'number':22,'name"
    "':'QA','role':'output'},{'number':23,'name':'ENP','role':'input'},{'number':24,'name':'V"
    "CC','role':'power'}]},'model':'sync','units':[{'stages':8,'code':'binary','q':['QA','QB'"
    ",'QC','QD','QE','QF','QG','QH'],'clock':{'pin':'CLK','edge':'rising'},'data':['A','B','C"
    "','D','E','F','G','H'],'modes':{'pins':['S0','S1'],'table':['clear','down','load','up'],"
    "'clear':'clock'},'enables':[{'pin':'ENP','active':'low'},{'pin':'ENT','active':'low'}],'"
    "carries':[{'pin':'RCO','active':'low','at':'terminal','gates':[{'pin':'ENT','active':'lo"
    "w'},{'pin':'ENP','active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},"
    "'parts':[{'name':'FX-74_869_AS','package':'DIP','pinout':'p24','footprint':'FX24','famil"
    "y':'HC'}]},{'id':'fx-74_590','function':'Counter','description':'8-bit binary counter wi"
    "th 3-state output register','drawer':[],'pinouts':{'p16':[{'number':1,'name':'QB','role'"
    ":'output'},{'number':2,'name':'QC','role':'output'},{'number':3,'name':'QD','role':'outp"
    "ut'},{'number':4,'name':'QE','role':'output'},{'number':5,'name':'QF','role':'output'},{"
    "'number':6,'name':'QG','role':'output'},{'number':7,'name':'QH','role':'output'},{'numbe"
    "r':8,'name':'GND','role':'ground'},{'number':9,'name':'RCO','role':'output'},{'number':1"
    "0,'name':'CCLR','role':'input'},{'number':11,'name':'CCK','role':'input'},{'number':12,'"
    "name':'CCKEN','role':'input'},{'number':13,'name':'RCK','role':'input'},{'number':14,'na"
    "me':'G','role':'input'},{'number':15,'name':'QA','role':'output'},{'number':16,'name':'V"
    "CC','role':'power'}]},'model':'sync','units':[{'stages':8,'code':'binary','clock':{'pin'"
    ":'CCK','edge':'rising'},'count':'up','enables':[{'pin':'CCKEN','active':'low'}],'clears'"
    ":[{'pin':'CCLR','active':'low','when':'async','to':'zero'}],'register':{'clock':{'pin':'"
    "RCK','edge':'rising'},'outputs':['QA','QB','QC','QD','QE','QF','QG','QH'],'outputEnable'"
    ":{'pin':'G','active':'low'}},'carries':[{'pin':'RCO','active':'low','at':'max'}]}],'timi"
    "ng':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_590','package':'DIP"
    "','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_592','function':'Count"
    "er','description':'8-bit binary counter with input register','drawer':[],'pinouts':{'p16"
    "':[{'number':1,'name':'B','role':'input'},{'number':2,'name':'C','role':'input'},{'numbe"
    "r':3,'name':'D','role':'input'},{'number':4,'name':'E','role':'input'},{'number':5,'name"
    "':'F','role':'input'},{'number':6,'name':'G','role':'input'},{'number':7,'name':'H','rol"
    "e':'input'},{'number':8,'name':'GND','role':'ground'},{'number':9,'name':'RCO','role':'o"
    "utput'},{'number':10,'name':'CCLR','role':'input'},{'number':11,'name':'CCK','role':'inp"
    "ut'},{'number':12,'name':'CCKEN','role':'input'},{'number':13,'name':'RCK','role':'input"
    "'},{'number':14,'name':'CLOAD','role':'input'},{'number':15,'name':'A','role':'input'},{"
    "'number':16,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages':8,'code':'b"
    "inary','clock':{'pin':'CCK','edge':'rising'},'count':'up','enables':[{'pin':'CCKEN','act"
    "ive':'low'}],'clears':[{'pin':'CCLR','active':'low','when':'async','to':'zero'}],'loads'"
    ":[{'pin':'CLOAD','active':'low','when':'async','from':'register'}],'inputRegister':{'clo"
    "ck':{'pin':'RCK','edge':'rising'},'data':['A','B','C','D','E','F','G','H']},'carries':[{"
    "'pin':'RCO','active':'low','at':'max'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6"
    "]}},'parts':[{'name':'FX-74_592','package':'DIP','pinout':'p16','footprint':'FX16','fami"
    "ly':'HC'}]},{'id':'fx-74_593','function':'Counter','description':'8-bit binary counter w"
    "ith input register and 3-state I/O','drawer':[],'pinouts':{'p20':[{'number':1,'name':'A/"
    "QA','role':'inout'},{'number':2,'name':'B/QB','role':'inout'},{'number':3,'name':'C/QC',"
    "'role':'inout'},{'number':4,'name':'D/QD','role':'inout'},{'number':5,'name':'E/QE','rol"
    "e':'inout'},{'number':6,'name':'F/QF','role':'inout'},{'number':7,'name':'G/QG','role':'"
    "inout'},{'number':8,'name':'H/QH','role':'inout'},{'number':9,'name':'CLOAD','role':'inp"
    "ut'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name':'RCO','role':'output"
    "'},{'number':12,'name':'CCLR','role':'input'},{'number':13,'name':'CCK','role':'input'},"
    "{'number':14,'name':'CCKENBAR','role':'input'},{'number':15,'name':'CCKEN','role':'input"
    "'},{'number':16,'name':'RCK','role':'input'},{'number':17,'name':'RCKEN','role':'input'}"
    ",{'number':18,'name':'GBAR','role':'input'},{'number':19,'name':'G','role':'input'},{'nu"
    "mber':20,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages':8,'code':'bina"
    "ry','clock':{'pin':'CCK','edge':'rising'},'count':'up','enablesAny':[{'pin':'CCKEN','act"
    "ive':'high'},{'pin':'CCKENBAR','active':'low'}],'clears':[{'pin':'CCLR','active':'low','"
    "when':'async','to':'zero'}],'loads':[{'pin':'CLOAD','active':'low','when':'async','from'"
    ":'register'}],'inputRegister':{'clock':{'pin':'RCK','edge':'rising'},'enable':{'pin':'RC"
    "KEN','active':'low'}},'bus':{'pins':['A/QA','B/QB','C/QC','D/QD','E/QE','F/QF','G/QG','H"
    "/QH'],'outputEnables':[{'pin':'G','active':'high'},{'pin':'GBAR','active':'low'}]},'carr"
    "ies':[{'pin':'RCO','active':'low','at':'max'}]}],'timing':{'HC':{'rise':10,'fall':10,'vc"
    "c':[2,6]}},'parts':[{'name':'FX-74_593','package':'DIP','pinout':'p20','footprint':'FX20"
    "','family':'HC'}]},{'id':'fx-74_697','function':'Counter','description':'Synchronous 4-b"
    "it up/down binary counter with output register and multiplexed 3-state outputs','drawer'"
    ":[],'pinouts':{'p20':[{'number':1,'name':'U/D','role':'input'},{'number':2,'name':'CCK',"
    "'role':'input'},{'number':3,'name':'A','role':'input'},{'number':4,'name':'B','role':'in"
    "put'},{'number':5,'name':'C','role':'input'},{'number':6,'name':'D','role':'input'},{'nu"
    "mber':7,'name':'ENP','role':'input'},{'number':8,'name':'CCLR','role':'input'},{'number'"
    ":9,'name':'RCK','role':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,"
    "'name':'R/C','role':'input'},{'number':12,'name':'G','role':'input'},{'number':13,'name'"
    ":'LOAD','role':'input'},{'number':14,'name':'ENT','role':'input'},{'number':15,'name':'Q"
    "D','role':'output'},{'number':16,'name':'QC','role':'output'},{'number':17,'name':'QB','"
    "role':'output'},{'number':18,'name':'QA','role':'output'},{'number':19,'name':'RCO','rol"
    "e':'output'},{'number':20,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stage"
    "s':4,'code':'binary','clock':{'pin':'CCK','edge':'rising'},'direction':{'pin':'U/D','up'"
    ":'high'},'enables':[{'pin':'ENP','active':'low'},{'pin':'ENT','active':'low'}],'clears':"
    "[{'pin':'CCLR','active':'low','when':'async','to':'zero'}],'loads':[{'pin':'LOAD','activ"
    "e':'low','when':'clock','from':'data'}],'data':['A','B','C','D'],'register':{'clock':{'p"
    "in':'RCK','edge':'rising'},'outputs':['QA','QB','QC','QD'],'outputEnable':{'pin':'G','ac"
    "tive':'low'},'select':{'pin':'R/C','register':'high'}},'carries':[{'pin':'RCO','active':"
    "'low','at':'terminal','gates':[{'pin':'ENT','active':'low'}]}]}],'timing':{'HC':{'rise':"
    "10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_697','package':'DIP','pinout':'p20','"
    "footprint':'FX20','family':'HC'}]},{'id':'fx-74_8154','function':'Counter','description'"
    ":'Dual 16-bit binary counter with 3-state output register','drawer':[],'pinouts':{'p20':"
    "[{'number':1,'name':'CLKA','role':'input'},{'number':2,'name':'CLKB','role':'input'},{'n"
    "umber':3,'name':'GAL','role':'input'},{'number':4,'name':'GAU','role':'input'},{'number'"
    ":5,'name':'GBL','role':'input'},{'number':6,'name':'GBU','role':'input'},{'number':7,'na"
    "me':'RCLK','role':'input'},{'number':8,'name':'RCOA','role':'output'},{'number':9,'name'"
    ":'CLKBEN','role':'input'},{'number':10,'name':'GND','role':'ground'},{'number':11,'name'"
    ":'CCLR','role':'input'},{'number':12,'name':'Y7','role':'output'},{'number':13,'name':'Y"
    "6','role':'output'},{'number':14,'name':'Y5','role':'output'},{'number':15,'name':'Y4','"
    "role':'output'},{'number':16,'name':'Y3','role':'output'},{'number':17,'name':'Y2','role"
    "':'output'},{'number':18,'name':'Y1','role':'output'},{'number':19,'name':'Y0','role':'o"
    "utput'},{'number':20,'name':'VCC','role':'power'}]},'model':'sync','units':[{'stages':16"
    ",'code':'binary','clock':{'pin':'CLKA','edge':'rising'},'count':'up','clears':[{'pin':'C"
    "CLR','active':'low','when':'async','to':'zero'}],'register':{'clock':{'pin':'RCLK','edge"
    "':'rising'}},'carries':[{'pin':'RCOA','active':'low','at':'max'}]},{'stages':16,'code':'"
    "binary','clock':{'pin':'CLKB','edge':'rising'},'count':'up','enables':[{'pin':'CLKBEN','"
    "active':'low'}],'clears':[{'pin':'CCLR','active':'low','when':'async','to':'zero'}],'reg"
    "ister':{'clock':{'pin':'RCLK','edge':'rising'}}}],'byteBus':{'outputs':['Y0','Y1','Y2','"
    "Y3','Y4','Y5','Y6','Y7'],'selects':[{'pin':'GAL','unit':0,'byte':0},{'pin':'GAU','unit':"
    "0,'byte':1},{'pin':'GBL','unit':1,'byte':0},{'pin':'GBU','unit':1,'byte':1}]},'timing':{"
    "'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_8154','package':'DIP','p"
    "inout':'p20','footprint':'FX20','family':'HC'}]},{'id':'fx-4029B','function':'Counter','"
    "description':'Presettable up/down counter, binary or BCD-decade','drawer':[],'pinouts':{"
    "'p16':[{'number':1,'name':'PE','role':'input'},{'number':2,'name':'Q4','role':'output'},"
    "{'number':3,'name':'JAM4','role':'input'},{'number':4,'name':'JAM1','role':'input'},{'nu"
    "mber':5,'name':'CARRYIN','role':'input'},{'number':6,'name':'Q1','role':'output'},{'numb"
    "er':7,'name':'CARRYOUT','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'num"
    "ber':9,'name':'BINARY/DECADE','role':'input'},{'number':10,'name':'UP/DOWN','role':'inpu"
    "t'},{'number':11,'name':'Q2','role':'output'},{'number':12,'name':'JAM2','role':'input'}"
    ",{'number':13,'name':'JAM3','role':'input'},{'number':14,'name':'Q3','role':'output'},{'"
    "number':15,'name':'CLOCK','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'m"
    "odel':'sync','units':[{'stages':4,'codeSelect':{'pin':'BINARY/DECADE','binary':'high'},'"
    "q':['Q1','Q2','Q3','Q4'],'clock':{'pin':'CLOCK','edge':'rising'},'direction':{'pin':'UP/"
    "DOWN','up':'high'},'enables':[{'pin':'CARRYIN','active':'low'}],'loads':[{'pin':'PE','ac"
    "tive':'high','when':'async','from':'data'}],'data':['JAM1','JAM2','JAM3','JAM4'],'carrie"
    "s':[{'pin':'CARRYOUT','active':'low','at':'terminal','gates':[{'pin':'CARRYIN','active':"
    "'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4029B"
    "','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4510B','f"
    "unction':'Counter','description':'Presettable BCD up/down counter','drawer':[],'pinouts'"
    ":{'p16':[{'number':1,'name':'PE','role':'input'},{'number':2,'name':'Q4','role':'output'"
    "},{'number':3,'name':'P4','role':'input'},{'number':4,'name':'P1','role':'input'},{'numb"
    "er':5,'name':'CARRYIN','role':'input'},{'number':6,'name':'Q1','role':'output'},{'number"
    "':7,'name':'CARRYOUT','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'numbe"
    "r':9,'name':'RESET','role':'input'},{'number':10,'name':'UP/DOWN','role':'input'},{'numb"
    "er':11,'name':'Q2','role':'output'},{'number':12,'name':'P2','role':'input'},{'number':1"
    "3,'name':'P3','role':'input'},{'number':14,'name':'Q3','role':'output'},{'number':15,'na"
    "me':'CLOCK','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'sync','"
    "units':[{'stages':4,'code':'bcd','q':['Q1','Q2','Q3','Q4'],'clock':{'pin':'CLOCK','edge'"
    ":'rising'},'direction':{'pin':'UP/DOWN','up':'high'},'enables':[{'pin':'CARRYIN','active"
    "':'low'}],'clears':[{'pin':'RESET','active':'high','when':'async','to':'zero'}],'loads':"
    "[{'pin':'PE','active':'high','when':'async','from':'data'}],'data':['P1','P2','P3','P4']"
    ",'carries':[{'pin':'CARRYOUT','active':'low','at':'terminal','gates':[{'pin':'CARRYIN','"
    "active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'"
    "FX-4510B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4"
    "516B','function':'Counter','description':'Presettable binary up/down counter','drawer':["
    "],'pinouts':{'p16':[{'number':1,'name':'PE','role':'input'},{'number':2,'name':'Q4','rol"
    "e':'output'},{'number':3,'name':'P4','role':'input'},{'number':4,'name':'P1','role':'inp"
    "ut'},{'number':5,'name':'CARRYIN','role':'input'},{'number':6,'name':'Q1','role':'output"
    "'},{'number':7,'name':'CARRYOUT','role':'output'},{'number':8,'name':'VSS','role':'groun"
    "d'},{'number':9,'name':'RESET','role':'input'},{'number':10,'name':'UP/DOWN','role':'inp"
    "ut'},{'number':11,'name':'Q2','role':'output'},{'number':12,'name':'P2','role':'input'},"
    "{'number':13,'name':'P3','role':'input'},{'number':14,'name':'Q3','role':'output'},{'num"
    "ber':15,'name':'CLOCK','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'mode"
    "l':'sync','units':[{'stages':4,'code':'binary','q':['Q1','Q2','Q3','Q4'],'clock':{'pin':"
    "'CLOCK','edge':'rising'},'direction':{'pin':'UP/DOWN','up':'high'},'enables':[{'pin':'CA"
    "RRYIN','active':'low'}],'clears':[{'pin':'RESET','active':'high','when':'async','to':'ze"
    "ro'}],'loads':[{'pin':'PE','active':'high','when':'async','from':'data'}],'data':['P1','"
    "P2','P3','P4'],'carries':[{'pin':'CARRYOUT','active':'low','at':'terminal','gates':[{'pi"
    "n':'CARRYIN','active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'par"
    "ts':[{'name':'FX-4516B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}"
    "]},{'id':'fx-4518B','function':'Counter','description':'Dual BCD up counter','drawer':[]"
    ",'pinouts':{'p16':[{'number':1,'name':'CLOCKA','role':'input'},{'number':2,'name':'ENABL"
    "EA','role':'input'},{'number':3,'name':'Q1A','role':'output'},{'number':4,'name':'Q2A','"
    "role':'output'},{'number':5,'name':'Q3A','role':'output'},{'number':6,'name':'Q4A','role"
    "':'output'},{'number':7,'name':'RESETA','role':'input'},{'number':8,'name':'VSS','role':"
    "'ground'},{'number':9,'name':'CLOCKB','role':'input'},{'number':10,'name':'ENABLEB','rol"
    "e':'input'},{'number':11,'name':'Q1B','role':'output'},{'number':12,'name':'Q2B','role':"
    "'output'},{'number':13,'name':'Q3B','role':'output'},{'number':14,'name':'Q4B','role':'o"
    "utput'},{'number':15,'name':'RESETB','role':'input'},{'number':16,'name':'VDD','role':'p"
    "ower'}]},'model':'sync','units':[{'stages':4,'code':'bcd','q':['Q1A','Q2A','Q3A','Q4A'],"
    "'clock':{'pin':'CLOCKA','edge':'rising'},'clockGate':{'pin':'ENABLEA','op':'or-not'},'co"
    "unt':'up','clears':[{'pin':'RESETA','active':'high','when':'async','to':'zero'}]},{'stag"
    "es':4,'code':'bcd','q':['Q1B','Q2B','Q3B','Q4B'],'clock':{'pin':'CLOCKB','edge':'rising'"
    "},'clockGate':{'pin':'ENABLEB','op':'or-not'},'count':'up','clears':[{'pin':'RESETB','ac"
    "tive':'high','when':'async','to':'zero'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2"
    ",6]}},'parts':[{'name':'FX-4518B','package':'DIP','pinout':'p16','footprint':'FX16','fam"
    "ily':'HC'}]},{'id':'fx-4520B','function':'Counter','description':'Dual binary up counter"
    "','drawer':[],'pinouts':{'p16':[{'number':1,'name':'CLOCKA','role':'input'},{'number':2,"
    "'name':'ENABLEA','role':'input'},{'number':3,'name':'Q1A','role':'output'},{'number':4,'"
    "name':'Q2A','role':'output'},{'number':5,'name':'Q3A','role':'output'},{'number':6,'name"
    "':'Q4A','role':'output'},{'number':7,'name':'RESETA','role':'input'},{'number':8,'name':"
    "'VSS','role':'ground'},{'number':9,'name':'CLOCKB','role':'input'},{'number':10,'name':'"
    "ENABLEB','role':'input'},{'number':11,'name':'Q1B','role':'output'},{'number':12,'name':"
    "'Q2B','role':'output'},{'number':13,'name':'Q3B','role':'output'},{'number':14,'name':'Q"
    "4B','role':'output'},{'number':15,'name':'RESETB','role':'input'},{'number':16,'name':'V"
    "DD','role':'power'}]},'model':'sync','units':[{'stages':4,'code':'binary','q':['Q1A','Q2"
    "A','Q3A','Q4A'],'clock':{'pin':'CLOCKA','edge':'rising'},'clockGate':{'pin':'ENABLEA','o"
    "p':'or-not'},'count':'up','clears':[{'pin':'RESETA','active':'high','when':'async','to':"
    "'zero'}]},{'stages':4,'code':'binary','q':['Q1B','Q2B','Q3B','Q4B'],'clock':{'pin':'CLOC"
    "KB','edge':'rising'},'clockGate':{'pin':'ENABLEB','op':'or-not'},'count':'up','clears':["
    "{'pin':'RESETB','active':'high','when':'async','to':'zero'}]}],'timing':{'HC':{'rise':10"
    ",'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4520B','package':'DIP','pinout':'p16','foo"
    "tprint':'FX16','family':'HC'}]},{'id':'fx-74_4518','function':'Counter','description':'D"
    "ual BCD up counter','drawer':[],'pinouts':{'p16':[{'number':1,'name':'1CP','role':'input"
    "'},{'number':2,'name':'1E','role':'input'},{'number':3,'name':'1Q0','role':'output'},{'n"
    "umber':4,'name':'1Q1','role':'output'},{'number':5,'name':'1Q2','role':'output'},{'numbe"
    "r':6,'name':'1Q3','role':'output'},{'number':7,'name':'1MR','role':'input'},{'number':8,"
    "'name':'GND','role':'ground'},{'number':9,'name':'2CP','role':'input'},{'number':10,'nam"
    "e':'2E','role':'input'},{'number':11,'name':'2Q0','role':'output'},{'number':12,'name':'"
    "2Q1','role':'output'},{'number':13,'name':'2Q2','role':'output'},{'number':14,'name':'2Q"
    "3','role':'output'},{'number':15,'name':'2MR','role':'input'},{'number':16,'name':'VCC',"
    "'role':'power'}]},'model':'sync','units':[{'stages':4,'code':'bcd','q':['1Q0','1Q1','1Q2"
    "','1Q3'],'clock':{'pin':'1CP','edge':'rising'},'clockGate':{'pin':'1E','op':'or-not'},'c"
    "ount':'up','clears':[{'pin':'1MR','active':'high','when':'async','to':'zero'}]},{'stages"
    "':4,'code':'bcd','q':['2Q0','2Q1','2Q2','2Q3'],'clock':{'pin':'2CP','edge':'rising'},'cl"
    "ockGate':{'pin':'2E','op':'or-not'},'count':'up','clears':[{'pin':'2MR','active':'high',"
    "'when':'async','to':'zero'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts'"
    ":[{'name':'FX-74_4518','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]"
    "},{'id':'fx-74_4520','function':'Counter','description':'Dual binary up counter','drawer"
    "':[],'pinouts':{'p16':[{'number':1,'name':'1CP','role':'input'},{'number':2,'name':'1E',"
    "'role':'input'},{'number':3,'name':'1Q0','role':'output'},{'number':4,'name':'1Q1','role"
    "':'output'},{'number':5,'name':'1Q2','role':'output'},{'number':6,'name':'1Q3','role':'o"
    "utput'},{'number':7,'name':'1MR','role':'input'},{'number':8,'name':'GND','role':'ground"
    "'},{'number':9,'name':'2CP','role':'input'},{'number':10,'name':'2E','role':'input'},{'n"
    "umber':11,'name':'2Q0','role':'output'},{'number':12,'name':'2Q1','role':'output'},{'num"
    "ber':13,'name':'2Q2','role':'output'},{'number':14,'name':'2Q3','role':'output'},{'numbe"
    "r':15,'name':'2MR','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':'"
    "sync','units':[{'stages':4,'code':'binary','q':['1Q0','1Q1','1Q2','1Q3'],'clock':{'pin':"
    "'1CP','edge':'rising'},'clockGate':{'pin':'1E','op':'or-not'},'count':'up','clears':[{'p"
    "in':'1MR','active':'high','when':'async','to':'zero'}]},{'stages':4,'code':'binary','q':"
    "['2Q0','2Q1','2Q2','2Q3'],'clock':{'pin':'2CP','edge':'rising'},'clockGate':{'pin':'2E',"
    "'op':'or-not'},'count':'up','clears':[{'pin':'2MR','active':'high','when':'async','to':'"
    "zero'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-74_4520"
    "','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-40102B','"
    "function':'Counter','description':'Presettable 2-decade BCD down counter','drawer':[],'p"
    "inouts':{'p16':[{'number':1,'name':'CLOCK','role':'input'},{'number':2,'name':'CLR','rol"
    "e':'input'},{'number':3,'name':'CI/CE','role':'input'},{'number':4,'name':'J0','role':'i"
    "nput'},{'number':5,'name':'J1','role':'input'},{'number':6,'name':'J2','role':'input'},{"
    "'number':7,'name':'J3','role':'input'},{'number':8,'name':'VSS','role':'ground'},{'numbe"
    "r':9,'name':'APE','role':'input'},{'number':10,'name':'J4','role':'input'},{'number':11,"
    "'name':'J5','role':'input'},{'number':12,'name':'J6','role':'input'},{'number':13,'name'"
    ":'J7','role':'input'},{'number':14,'name':'CO/ZD','role':'output'},{'number':15,'name':'"
    "SPE','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'sync','units':"
    "[{'stages':8,'code':'bcd','clock':{'pin':'CLOCK','edge':'rising'},'count':'down','enable"
    "s':[{'pin':'CI/CE','active':'low'}],'clears':[{'pin':'CLR','active':'low','when':'async'"
    ",'to':'max'}],'loads':[{'pin':'APE','active':'low','when':'async','from':'data'},{'pin':"
    "'SPE','active':'low','when':'clock','from':'data'}],'data':['J0','J1','J2','J3','J4','J5"
    "','J6','J7'],'carries':[{'pin':'CO/ZD','active':'low','at':'min','gates':[{'pin':'CI/CE'"
    ",'active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name'"
    ":'FX-40102B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'f"
    "x-40103B','function':'Counter','description':'Presettable 8-bit binary down counter','dr"
    "awer':[],'pinouts':{'p16':[{'number':1,'name':'CLOCK','role':'input'},{'number':2,'name'"
    ":'CLR','role':'input'},{'number':3,'name':'CI/CE','role':'input'},{'number':4,'name':'J0"
    "','role':'input'},{'number':5,'name':'J1','role':'input'},{'number':6,'name':'J2','role'"
    ":'input'},{'number':7,'name':'J3','role':'input'},{'number':8,'name':'VSS','role':'groun"
    "d'},{'number':9,'name':'APE','role':'input'},{'number':10,'name':'J4','role':'input'},{'"
    "number':11,'name':'J5','role':'input'},{'number':12,'name':'J6','role':'input'},{'number"
    "':13,'name':'J7','role':'input'},{'number':14,'name':'CO/ZD','role':'output'},{'number':"
    "15,'name':'SPE','role':'input'},{'number':16,'name':'VDD','role':'power'}]},'model':'syn"
    "c','units':[{'stages':8,'code':'binary','clock':{'pin':'CLOCK','edge':'rising'},'count':"
    "'down','enables':[{'pin':'CI/CE','active':'low'}],'clears':[{'pin':'CLR','active':'low',"
    "'when':'async','to':'max'}],'loads':[{'pin':'APE','active':'low','when':'async','from':'"
    "data'},{'pin':'SPE','active':'low','when':'clock','from':'data'}],'data':['J0','J1','J2'"
    ",'J3','J4','J5','J6','J7'],'carries':[{'pin':'CO/ZD','active':'low','at':'min','gates':["
    "{'pin':'CI/CE','active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'p"
    "arts':[{'name':'FX-40103B','package':'DIP','pinout':'p16','footprint':'FX16','family':'H"
    "C'}]},{'id':'fx-74_40103','function':'Counter','description':'Presettable 8-bit binary d"
    "own counter','drawer':[],'pinouts':{'p16':[{'number':1,'name':'CP','role':'input'},{'num"
    "ber':2,'name':'MR','role':'input'},{'number':3,'name':'TE','role':'input'},{'number':4,'"
    "name':'P0','role':'input'},{'number':5,'name':'P1','role':'input'},{'number':6,'name':'P"
    "2','role':'input'},{'number':7,'name':'P3','role':'input'},{'number':8,'name':'GND','rol"
    "e':'ground'},{'number':9,'name':'PL','role':'input'},{'number':10,'name':'P4','role':'in"
    "put'},{'number':11,'name':'P5','role':'input'},{'number':12,'name':'P6','role':'input'},"
    "{'number':13,'name':'P7','role':'input'},{'number':14,'name':'TC','role':'output'},{'num"
    "ber':15,'name':'PE','role':'input'},{'number':16,'name':'VCC','role':'power'}]},'model':"
    "'sync','units':[{'stages':8,'code':'binary','clock':{'pin':'CP','edge':'rising'},'count'"
    ":'down','enables':[{'pin':'TE','active':'low'}],'clears':[{'pin':'MR','active':'low','wh"
    "en':'async','to':'max'}],'loads':[{'pin':'PL','active':'low','when':'async','from':'data"
    "'},{'pin':'PE','active':'low','when':'clock','from':'data'}],'data':['P0','P1','P2','P3'"
    ",'P4','P5','P6','P7'],'carries':[{'pin':'TC','active':'low','at':'min','gates':[{'pin':'"
    "TE','active':'low'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'na"
    "me':'FX-74_40103','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'i"
    "d':'fx-40160B','function':'Counter','description':'Synchronous programmable decade count"
    "er, asynchronous clear','drawer':[],'pinouts':{'p16':[{'number':1,'name':'CLEAR','role':"
    "'input'},{'number':2,'name':'CLOCK','role':'input'},{'number':3,'name':'P1','role':'inpu"
    "t'},{'number':4,'name':'P2','role':'input'},{'number':5,'name':'P3','role':'input'},{'nu"
    "mber':6,'name':'P4','role':'input'},{'number':7,'name':'PE','role':'input'},{'number':8,"
    "'name':'VSS','role':'ground'},{'number':9,'name':'LOAD','role':'input'},{'number':10,'na"
    "me':'TE','role':'input'},{'number':11,'name':'Q4','role':'output'},{'number':12,'name':'"
    "Q3','role':'output'},{'number':13,'name':'Q2','role':'output'},{'number':14,'name':'Q1',"
    "'role':'output'},{'number':15,'name':'COUT','role':'output'},{'number':16,'name':'VDD','"
    "role':'power'}]},'model':'sync','units':[{'stages':4,'code':'bcd','q':['Q1','Q2','Q3','Q"
    "4'],'clock':{'pin':'CLOCK','edge':'rising'},'count':'up','enables':[{'pin':'PE','active'"
    ":'high'},{'pin':'TE','active':'high'}],'clears':[{'pin':'CLEAR','active':'low','when':'a"
    "sync','to':'zero'}],'loads':[{'pin':'LOAD','active':'low','when':'clock','from':'data'}]"
    ",'data':['P1','P2','P3','P4'],'carries':[{'pin':'COUT','active':'high','at':'max','gates"
    "':[{'pin':'TE','active':'high'}]}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'"
    "parts':[{'name':'FX-40160B','package':'DIP','pinout':'p16','footprint':'FX16','family':'"
    "HC'}]},{'id':'fx-40161B','function':'Counter','description':'Synchronous programmable 4-"
    "bit binary counter, asynchronous clear','drawer':[],'pinouts':{'p16':[{'number':1,'name'"
    ":'CLEAR','role':'input'},{'number':2,'name':'CLOCK','role':'input'},{'number':3,'name':'"
    "P1','role':'input'},{'number':4,'name':'P2','role':'input'},{'number':5,'name':'P3','rol"
    "e':'input'},{'number':6,'name':'P4','role':'input'},{'number':7,'name':'PE','role':'inpu"
    "t'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'LOAD','role':'input'},"
    "{'number':10,'name':'TE','role':'input'},{'number':11,'name':'Q4','role':'output'},{'num"
    "ber':12,'name':'Q3','role':'output'},{'number':13,'name':'Q2','role':'output'},{'number'"
    ":14,'name':'Q1','role':'output'},{'number':15,'name':'COUT','role':'output'},{'number':1"
    "6,'name':'VDD','role':'power'}]},'model':'sync','units':[{'stages':4,'code':'binary','q'"
    ":['Q1','Q2','Q3','Q4'],'clock':{'pin':'CLOCK','edge':'rising'},'count':'up','enables':[{"
    "'pin':'PE','active':'high'},{'pin':'TE','active':'high'}],'clears':[{'pin':'CLEAR','acti"
    "ve':'low','when':'async','to':'zero'}],'loads':[{'pin':'LOAD','active':'low','when':'clo"
    "ck','from':'data'}],'data':['P1','P2','P3','P4'],'carries':[{'pin':'COUT','active':'high"
    "','at':'max','gates':[{'pin':'TE','active':'high'}]}]}],'timing':{'HC':{'rise':10,'fall'"
    ":10,'vcc':[2,6]}},'parts':[{'name':'FX-40161B','package':'DIP','pinout':'p16','footprint"
    "':'FX16','family':'HC'}]},{'id':'fx-40192B','function':'Counter','description':'Presetta"
    "ble BCD up/down counter, dual clock','drawer':[],'pinouts':{'p16':[{'number':1,'name':'J"
    "2','role':'input'},{'number':2,'name':'Q2','role':'output'},{'number':3,'name':'Q1','rol"
    "e':'output'},{'number':4,'name':'CLOCKDOWN','role':'input'},{'number':5,'name':'CLOCKUP'"
    ",'role':'input'},{'number':6,'name':'Q3','role':'output'},{'number':7,'name':'Q4','role'"
    ":'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'J4','role':'inp"
    "ut'},{'number':10,'name':'J3','role':'input'},{'number':11,'name':'PE','role':'input'},{"
    "'number':12,'name':'CARRY','role':'output'},{'number':13,'name':'BORROW','role':'output'"
    "},{'number':14,'name':'RESET','role':'input'},{'number':15,'name':'J1','role':'input'},{"
    "'number':16,'name':'VDD','role':'power'}]},'model':'sync','units':[{'stages':4,'code':'b"
    "cd','q':['Q1','Q2','Q3','Q4'],'clockUp':'CLOCKUP','clockDown':'CLOCKDOWN','clears':[{'pi"
    "n':'RESET','active':'high','when':'async','to':'zero'}],'loads':[{'pin':'PE','active':'l"
    "ow','when':'async','from':'data'}],'data':['J1','J2','J3','J4'],'carries':[{'pin':'CARRY"
    "','active':'low','at':'max','clockLow':'CLOCKUP'},{'pin':'BORROW','active':'low','at':'m"
    "in','clockLow':'CLOCKDOWN'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts'"
    ":[{'name':'FX-40192B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]}"
    ",{'id':'fx-40193B','function':'Counter','description':'Presettable binary up/down counte"
    "r, dual clock','drawer':[],'pinouts':{'p16':[{'number':1,'name':'J2','role':'input'},{'n"
    "umber':2,'name':'Q2','role':'output'},{'number':3,'name':'Q1','role':'output'},{'number'"
    ":4,'name':'CLOCKDOWN','role':'input'},{'number':5,'name':'CLOCKUP','role':'input'},{'num"
    "ber':6,'name':'Q3','role':'output'},{'number':7,'name':'Q4','role':'output'},{'number':8"
    ",'name':'VSS','role':'ground'},{'number':9,'name':'J4','role':'input'},{'number':10,'nam"
    "e':'J3','role':'input'},{'number':11,'name':'PE','role':'input'},{'number':12,'name':'CA"
    "RRY','role':'output'},{'number':13,'name':'BORROW','role':'output'},{'number':14,'name':"
    "'RESET','role':'input'},{'number':15,'name':'J1','role':'input'},{'number':16,'name':'VD"
    "D','role':'power'}]},'model':'sync','units':[{'stages':4,'code':'binary','q':['Q1','Q2',"
    "'Q3','Q4'],'clockUp':'CLOCKUP','clockDown':'CLOCKDOWN','clears':[{'pin':'RESET','active'"
    ":'high','when':'async','to':'zero'}],'loads':[{'pin':'PE','active':'low','when':'async',"
    "'from':'data'}],'data':['J1','J2','J3','J4'],'carries':[{'pin':'CARRY','active':'low','a"
    "t':'max','clockLow':'CLOCKUP'},{'pin':'BORROW','active':'low','at':'min','clockLow':'CLO"
    "CKDOWN'}]}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-40193"
    "B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-4522B','"
    "function':'Divider','description':'Programmable BCD divide-by-N counter','drawer':[],'pi"
    "nouts':{'p16':[{'number':1,'name':'Q3','role':'output'},{'number':2,'name':'P3','role':'"
    "input'},{'number':3,'name':'PE','role':'input'},{'number':4,'name':'CI','role':'input'},"
    "{'number':5,'name':'P0','role':'input'},{'number':6,'name':'CL','role':'input'},{'number"
    "':7,'name':'Q0','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'"
    "name':'Q1','role':'output'},{'number':10,'name':'MR','role':'input'},{'number':11,'name'"
    ":'P1','role':'input'},{'number':12,'name':'ZERO','role':'output'},{'number':13,'name':'C"
    "F','role':'input'},{'number':14,'name':'P2','role':'input'},{'number':15,'name':'Q2','ro"
    "le':'output'},{'number':16,'name':'VDD','role':'power'}]},'model':'sync','units':[{'stag"
    "es':4,'code':'bcd','q':['Q0','Q1','Q2','Q3'],'clock':{'pin':'CL','edge':'rising'},'clock"
    "Gate':{'pin':'CI','op':'and-not'},'count':'down','clears':[{'pin':'MR','active':'high','"
    "when':'async','to':'zero'}],'loads':[{'pin':'PE','active':'high','when':'async','from':'"
    "data'}],'data':['P0','P1','P2','P3'],'zeroLatch':{'pin':'ZERO','cascade':'CF'}}],'timing"
    "':{'HC':{'rise':10,'fall':10,'vcc':[2,6]}},'parts':[{'name':'FX-4522B','package':'DIP','"
    "pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_90','function':'Counter',"
    "'description':'Decade counter','drawer':[],'pinouts':{'p14':[{'number':1,'name':'CKB','r"
    "ole':'input'},{'number':2,'name':'R0(1)','role':'input'},{'number':3,'name':'R0(2)','rol"
    "e':'input'},{'number':4,'name':'NC','role':'nc'},{'number':5,'name':'VCC','role':'power'"
    "},{'number':6,'name':'R9(1)','role':'input'},{'number':7,'name':'R9(2)','role':'input'},"
    "{'number':8,'name':'QC','role':'output'},{'number':9,'name':'QB','role':'output'},{'numb"
    "er':10,'name':'GND','role':'ground'},{'number':11,'name':'QD','role':'output'},{'number'"
    ":12,'name':'QA','role':'output'},{'number':13,'name':'NC','role':'nc'},{'number':14,'nam"
    "e':'CKA','role':'input'}]},'model':'ripple','units':1,'sections':[{'clock':'CKA','struct"
    "ure':'div2','q':['QA'],'reset':{'pins':['R0(1)','R0(2)'],'active':'high'},'set':{'pins':"
    "['R9(1)','R9(2)'],'active':'high','value':1}},{'clock':'CKB','structure':'div5','q':['QB"
    "','QC','QD'],'reset':{'pins':['R0(1)','R0(2)'],'active':'high'},'set':{'pins':['R9(1)','"
    "R9(2)'],'active':'high','value':4}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'st"
    "age':5}},'parts':[{'name':'FX-74_90','package':'DIP','pinout':'p14','footprint':'FX14','"
    "family':'HC'}]},{'id':'fx-74_92','function':'Counter','description':'Divide-by-12 counte"
    "r','drawer':[],'pinouts':{'p14':[{'number':1,'name':'CKB','role':'input'},{'number':2,'n"
    "ame':'NC','role':'nc'},{'number':3,'name':'NC','role':'nc'},{'number':4,'name':'NC','rol"
    "e':'nc'},{'number':5,'name':'VCC','role':'power'},{'number':6,'name':'R0(1)','role':'inp"
    "ut'},{'number':7,'name':'R0(2)','role':'input'},{'number':8,'name':'QD','role':'output'}"
    ",{'number':9,'name':'QC','role':'output'},{'number':10,'name':'GND','role':'ground'},{'n"
    "umber':11,'name':'QB','role':'output'},{'number':12,'name':'QA','role':'output'},{'numbe"
    "r':13,'name':'NC','role':'nc'},{'number':14,'name':'CKA','role':'input'}]},'model':'ripp"
    "le','units':1,'sections':[{'clock':'CKA','structure':'div2','q':['QA'],'reset':{'pins':["
    "'R0(1)','R0(2)'],'active':'high'}},{'clock':'CKB','structure':'div6','q':['QB','QC','QD'"
    "],'reset':{'pins':['R0(1)','R0(2)'],'active':'high'}}],'timing':{'HC':{'rise':10,'fall':"
    "10,'vcc':[2,6],'stage':5}},'parts':[{'name':'FX-74_92','package':'DIP','pinout':'p14','f"
    "ootprint':'FX14','family':'HC'}]},{'id':'fx-74_93','function':'Counter','description':'4"
    "-bit binary counter','drawer':[],'pinouts':{'p14':[{'number':1,'name':'CKB','role':'inpu"
    "t'},{'number':2,'name':'R0(1)','role':'input'},{'number':3,'name':'R0(2)','role':'input'"
    "},{'number':4,'name':'NC','role':'nc'},{'number':5,'name':'VCC','role':'power'},{'number"
    "':6,'name':'NC','role':'nc'},{'number':7,'name':'NC','role':'nc'},{'number':8,'name':'QC"
    "','role':'output'},{'number':9,'name':'QB','role':'output'},{'number':10,'name':'GND','r"
    "ole':'ground'},{'number':11,'name':'QD','role':'output'},{'number':12,'name':'QA','role'"
    ":'output'},{'number':13,'name':'NC','role':'nc'},{'number':14,'name':'CKA','role':'input"
    "'}]},'model':'ripple','units':1,'sections':[{'clock':'CKA','structure':'div2','q':['QA']"
    ",'reset':{'pins':['R0(1)','R0(2)'],'active':'high'}},{'clock':'CKB','structure':'binary'"
    ",'stages':3,'q':['QB','QC','QD'],'reset':{'pins':['R0(1)','R0(2)'],'active':'high'}}],'t"
    "iming':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'stage':5}},'parts':[{'name':'FX-74_93','p"
    "ackage':'DIP','pinout':'p14','footprint':'FX14','family':'HC'}]},{'id':'fx-74_293','func"
    "tion':'Counter','description':'4-bit binary counter','drawer':[],'pinouts':{'p14':[{'num"
    "ber':1,'name':'NC','role':'nc'},{'number':2,'name':'NC','role':'nc'},{'number':3,'name':"
    "'NC','role':'nc'},{'number':4,'name':'QC','role':'output'},{'number':5,'name':'QB','role"
    "':'output'},{'number':6,'name':'NC','role':'nc'},{'number':7,'name':'GND','role':'ground"
    "'},{'number':8,'name':'QD','role':'output'},{'number':9,'name':'QA','role':'output'},{'n"
    "umber':10,'name':'CKA','role':'input'},{'number':11,'name':'CKB','role':'input'},{'numbe"
    "r':12,'name':'R0(1)','role':'input'},{'number':13,'name':'R0(2)','role':'input'},{'numbe"
    "r':14,'name':'VCC','role':'power'}]},'model':'ripple','units':1,'sections':[{'clock':'CK"
    "A','structure':'div2','q':['QA'],'reset':{'pins':['R0(1)','R0(2)'],'active':'high'}},{'c"
    "lock':'CKB','structure':'binary','stages':3,'q':['QB','QC','QD'],'reset':{'pins':['R0(1)"
    "','R0(2)'],'active':'high'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'stage':5}"
    "},'parts':[{'name':'FX-74_293','package':'DIP','pinout':'p14','footprint':'FX14','family"
    "':'HC'}]},{'id':'fx-74_390','function':'Counter','description':'Dual 4-bit decade counte"
    "r','drawer':[],'pinouts':{'p16':[{'number':1,'name':'1CKA','role':'input'},{'number':2,'"
    "name':'1CLR','role':'input'},{'number':3,'name':'1QA','role':'output'},{'number':4,'name"
    "':'1CKB','role':'input'},{'number':5,'name':'1QB','role':'output'},{'number':6,'name':'1"
    "QC','role':'output'},{'number':7,'name':'1QD','role':'output'},{'number':8,'name':'GND',"
    "'role':'ground'},{'number':9,'name':'2QD','role':'output'},{'number':10,'name':'2QC','ro"
    "le':'output'},{'number':11,'name':'2QB','role':'output'},{'number':12,'name':'2CKB','rol"
    "e':'input'},{'number':13,'name':'2QA','role':'output'},{'number':14,'name':'2CLR','role'"
    ":'input'},{'number':15,'name':'2CKA','role':'input'},{'number':16,'name':'VCC','role':'p"
    "ower'}]},'model':'ripple','units':2,'sections':[{'clock':'1CKA','structure':'div2','q':["
    "'1QA'],'reset':{'pins':['1CLR'],'active':'high'}},{'clock':'1CKB','structure':'div5','q'"
    ":['1QB','1QC','1QD'],'reset':{'pins':['1CLR'],'active':'high'}},{'clock':'2CKA','structu"
    "re':'div2','q':['2QA'],'reset':{'pins':['2CLR'],'active':'high'}},{'clock':'2CKB','struc"
    "ture':'div5','q':['2QB','2QC','2QD'],'reset':{'pins':['2CLR'],'active':'high'}}],'timing"
    "':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'stage':5}},'parts':[{'name':'FX-74_390','packa"
    "ge':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-74_393','function"
    "':'Counter','description':'Dual 4-bit binary counter','drawer':[],'pinouts':{'p14':[{'nu"
    "mber':1,'name':'1A','role':'input'},{'number':2,'name':'1CLR','role':'input'},{'number':"
    "3,'name':'1QA','role':'output'},{'number':4,'name':'1QB','role':'output'},{'number':5,'n"
    "ame':'1QC','role':'output'},{'number':6,'name':'1QD','role':'output'},{'number':7,'name'"
    ":'GND','role':'ground'},{'number':8,'name':'2QD','role':'output'},{'number':9,'name':'2Q"
    "C','role':'output'},{'number':10,'name':'2QB','role':'output'},{'number':11,'name':'2QA'"
    ",'role':'output'},{'number':12,'name':'2CLR','role':'input'},{'number':13,'name':'2A','r"
    "ole':'input'},{'number':14,'name':'VCC','role':'power'}]},'model':'ripple','units':2,'se"
    "ctions':[{'clock':'1A','structure':'binary','stages':4,'q':['1QA','1QB','1QC','1QD'],'re"
    "set':{'pins':['1CLR'],'active':'high'}},{'clock':'2A','structure':'binary','stages':4,'q"
    "':['2QA','2QB','2QC','2QD'],'reset':{'pins':['2CLR'],'active':'high'}}],'timing':{'HC':{"
    "'rise':10,'fall':10,'vcc':[2,6],'stage':5}},'parts':[{'name':'FX-74_393','package':'DIP'"
    ",'pinout':'p14','footprint':'FX14','family':'HC'}]},{'id':'fx-4020B','function':'Counter"
    "','description':'14-stage ripple-carry binary counter/divider','drawer':[],'pinouts':{'p"
    "16':[{'number':1,'name':'Q12','role':'output'},{'number':2,'name':'Q13','role':'output'}"
    ",{'number':3,'name':'Q14','role':'output'},{'number':4,'name':'Q6','role':'output'},{'nu"
    "mber':5,'name':'Q5','role':'output'},{'number':6,'name':'Q7','role':'output'},{'number':"
    "7,'name':'Q4','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'na"
    "me':'Q1','role':'output'},{'number':10,'name':'CLK','role':'input'},{'number':11,'name':"
    "'RESET','role':'input'},{'number':12,'name':'Q9','role':'output'},{'number':13,'name':'Q"
    "8','role':'output'},{'number':14,'name':'Q10','role':'output'},{'number':15,'name':'Q11'"
    ",'role':'output'},{'number':16,'name':'VDD','role':'power'}]},'model':'ripple','units':1"
    ",'sections':[{'clock':'CLK','structure':'binary','stages':14,'q':['Q1',null,null,'Q4','Q"
    "5','Q6','Q7','Q8','Q9','Q10','Q11','Q12','Q13','Q14'],'reset':{'pins':['RESET'],'active'"
    ":'high'}}],'timing':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'stage':5}},'parts':[{'name':"
    "'FX-4020B','package':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]},{'id':'fx-"
    "4024B','function':'Counter','description':'7-stage ripple-carry binary counter/divider',"
    "'drawer':[],'pinouts':{'p14':[{'number':1,'name':'CLK','role':'input'},{'number':2,'name"
    "':'RESET','role':'input'},{'number':3,'name':'Q7','role':'output'},{'number':4,'name':'Q"
    "6','role':'output'},{'number':5,'name':'Q5','role':'output'},{'number':6,'name':'Q4','ro"
    "le':'output'},{'number':7,'name':'VSS','role':'ground'},{'number':8,'name':'NC','role':'"
    "nc'},{'number':9,'name':'Q3','role':'output'},{'number':10,'name':'NC','role':'nc'},{'nu"
    "mber':11,'name':'Q2','role':'output'},{'number':12,'name':'Q1','role':'output'},{'number"
    "':13,'name':'NC','role':'nc'},{'number':14,'name':'VDD','role':'power'}]},'model':'rippl"
    "e','units':1,'sections':[{'clock':'CLK','structure':'binary','stages':7,'q':['Q1','Q2','"
    "Q3','Q4','Q5','Q6','Q7'],'reset':{'pins':['RESET'],'active':'high'}}],'timing':{'HC':{'r"
    "ise':10,'fall':10,'vcc':[2,6],'stage':5}},'parts':[{'name':'FX-4024B','package':'DIP','p"
    "inout':'p14','footprint':'FX14','family':'HC'}]},{'id':'fx-4040B','function':'Counter','"
    "description':'12-stage ripple-carry binary counter/divider','drawer':[],'pinouts':{'p16'"
    ":[{'number':1,'name':'Q12','role':'output'},{'number':2,'name':'Q6','role':'output'},{'n"
    "umber':3,'name':'Q5','role':'output'},{'number':4,'name':'Q7','role':'output'},{'number'"
    ":5,'name':'Q4','role':'output'},{'number':6,'name':'Q3','role':'output'},{'number':7,'na"
    "me':'Q2','role':'output'},{'number':8,'name':'VSS','role':'ground'},{'number':9,'name':'"
    "Q1','role':'output'},{'number':10,'name':'CLK','role':'input'},{'number':11,'name':'RESE"
    "T','role':'input'},{'number':12,'name':'Q9','role':'output'},{'number':13,'name':'Q8','r"
    "ole':'output'},{'number':14,'name':'Q10','role':'output'},{'number':15,'name':'Q11','rol"
    "e':'output'},{'number':16,'name':'VDD','role':'power'}]},'model':'ripple','units':1,'sec"
    "tions':[{'clock':'CLK','structure':'binary','stages':12,'q':['Q1','Q2','Q3','Q4','Q5','Q"
    "6','Q7','Q8','Q9','Q10','Q11','Q12'],'reset':{'pins':['RESET'],'active':'high'}}],'timin"
    "g':{'HC':{'rise':10,'fall':10,'vcc':[2,6],'stage':5}},'parts':[{'name':'FX-4040B','packa"
    "ge':'DIP','pinout':'p16','footprint':'FX16','family':'HC'}]}]}";

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
    if (part == NULL || part->kind != PART_COUNTER) {
        fprintf(stderr, "no counter part %s\n", name);
        exit(1);
    }
    return part;
}

static Bench *open_fx(const char *device) {
    return bench_create(fx_part(device));
}

static Bench *open_joined(const char *device, const char *const *joins) {
    return bench_create_joined(fx_part(device), joins);
}

static void step(Bench *b) {
    bench_run(b, NS(50));
}

static void set(Bench *b, const char *pin, LogicValue v) {
    bench_set(b, pin, v);
}

/* A full clock pulse on pin: up, step, down, step. */
static void pulse(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
    set(b, pin, L0);
    step(b);
}

/* A rising edge only (the pin was low). */
static void rise(Bench *b, const char *pin) {
    set(b, pin, L1);
    step(b);
}

static void fall(Bench *b, const char *pin) {
    set(b, pin, L0);
    step(b);
}

static LogicValue bit(unsigned value, unsigned i) {
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

/* Drives the named pins from value's bits (names[0] = bit 0). */
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
            printf("  %s = %c, want %c (value %u)\n", names[i], "01XZ"[got],
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

static const char *const QAD[] = {"QA", "QB", "QC", "QD"};
static const char *const ABCD[] = {"A", "B", "C", "D"};
static const char *const Q8[] = {"QA", "QB", "QC", "QD", "QE", "QF", "QG", "QH"};
static const char *const D8[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
static const char *const Q14[] = {"Q1", "Q2", "Q3", "Q4"};

/* ---- 74_161 / 163 / 169 -------------------------------------------------- */

static void check_161(void) {
    Bench *b = open_fx("74_161");
    int ok;
    unsigned k;

    set(b, "CLR", L1);
    set(b, "LOAD", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set_pins(b, ABCD, 4, 0);
    set(b, "CLK", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0) && is(b, "RCO", L0);
    for (k = 1; k <= 16; k++) {
        pulse(b, "CLK");
        ok &= pins_value(b, QAD, 4, k % 16) && is(b, "RCO", k % 16 == 15 ? L1 : L0);
    }
    expect(ok, "1 74_161 power-on 0; counts 0->15->0 on CLK rising with ENP = ENT = 1; RCO high only "
               "at 15");

    for (k = 0; k < 15; k++) {
        pulse(b, "CLK");
    }
    ok = pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    set(b, "CLR", L0);
    step(b);
    expect(ok && pins_value(b, QAD, 4, 0) && is(b, "RCO", L0),
           "2 74_161 CLR low clears at once (no clock); RCO falls with it");

    set(b, "CLR", L1);
    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set_pins(b, ABCD, 4, 0xA);
    set(b, "LOAD", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0xA),
           "3 74_161 LOAD low loads at the edge regardless of ENP/ENT");

    set_pins(b, ABCD, 4, 15);
    pulse(b, "CLK");
    set(b, "LOAD", L1);
    set(b, "ENT", L1);
    step(b);
    ok = pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L0);
    step(b);
    ok &= is(b, "RCO", L0);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 15) && is(b, "RCO", L0),
           "4 74_161 ENP low holds, RCO still follows ENT; ENT low holds and RCO low");
    bench_destroy(b);
}

static void check_163(void) {
    Bench *b = open_fx("74_163");
    int ok;

    set(b, "CLR", L1);
    set(b, "LOAD", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set(b, "CLK", L0);
    step(b);
    pulse(b, "CLK");
    pulse(b, "CLK");
    pulse(b, "CLK");
    set(b, "CLR", L0);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 9);
    step(b);
    ok = pins_value(b, QAD, 4, 3);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0), "5 74_163 CLR low clears only at the edge; beats LOAD");
    bench_destroy(b);
}

static void check_169(void) {
    Bench *b = open_fx("74_169");
    int ok;

    set(b, "LOAD", L1);
    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set(b, "U/D", L0);
    set(b, "CLK", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0) && is(b, "RCO", L0);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1);
    set(b, "U/D", L1);
    step(b);
    ok &= is(b, "RCO", L0);
    set(b, "ENT", L1);
    step(b);
    expect(ok && is(b, "RCO", L1),
           "6 74_169 down count 0 -> 15; RCO low at 0 down, at 15 up, only with ENT low");
    bench_destroy(b);
}

/* ---- 74_190 / 191 / 193 --------------------------------------------------- */

static void check_190(void) {
    Bench *b = open_fx("74_190");
    int ok = 1;
    unsigned k;

    set(b, "LOAD", L1);
    set(b, "CTEN", L0);
    set(b, "D/U", L0);
    set(b, "CLK", L0);
    step(b);
    for (k = 1; k <= 10; k++) {
        pulse(b, "CLK");
        ok &= pins_value(b, QAD, 4, k % 10);
    }
    set(b, "D/U", L1);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 9);
    set(b, "D/U", L0);
    set_pins(b, ABCD, 4, 12);
    set(b, "LOAD", L0);
    step(b);
    set(b, "LOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 12);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 9);
    pulse(b, "CLK");
    expect(ok && pins_value(b, QAD, 4, 0),
           "7 74_190 9 -> 0 up, 0 -> 9 down; loaded 12, counts up to 9 then 0 (the illegal table)");
    bench_destroy(b);
}

static void check_191(void) {
    Bench *b = open_fx("74_191");
    int ok;

    set(b, "CTEN", L0);
    set(b, "D/U", L0);
    set(b, "CLK", L0);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 5);
    step(b);
    ok = pins_value(b, QAD, 4, 5);
    set_pins(b, ABCD, 4, 6);
    step(b);
    ok &= pins_value(b, QAD, 4, 6);
    set_pins(b, ABCD, 4, 15);
    step(b);
    rise(b, "CLK");   /* LOAD still low: no count */
    set(b, "LOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1) && is(b, "MAX/MIN", L1);
    fall(b, "CLK");
    ok &= is(b, "RCO", L0);
    set(b, "CTEN", L1);
    step(b);
    expect(ok && is(b, "RCO", L1),
           "8 74_191 LOAD low is transparent (data changes follow); RCO low only during CLK low at "
           "max with CTEN low");
    bench_destroy(b);
}

static void check_dual_clock(const char *device, const char *up, const char *down,
                             const char *loadpin, LogicValue load_on, const char *clr,
                             const char *co, const char *bo, const char *const *q,
                             const char *const *data, const char *what) {
    Bench *b = open_fx(device);
    LogicValue load_off = load_on == L0 ? L1 : L0;
    int ok;

    /* DOWN high before UP rises (an UP edge with DOWN floating is rightly X), then a clear. */
    set(b, loadpin, load_off);
    set(b, down, L1);
    step(b);
    set(b, up, L1);
    step(b);
    set(b, clr, L1);
    step(b);
    set(b, clr, L0);
    step(b);
    fall(b, up);
    rise(b, up);
    ok = pins_value(b, q, 4, 1);
    fall(b, up);
    rise(b, up);
    ok &= pins_value(b, q, 4, 2);
    set_pins(b, data, 4, 15);
    set(b, loadpin, load_on);
    step(b);
    set(b, loadpin, load_off);
    step(b);
    ok &= pins_value(b, q, 4, 15) && is(b, co, L1);
    fall(b, up);
    ok &= is(b, co, L0) && pins_value(b, q, 4, 15);
    rise(b, up);
    ok &= pins_value(b, q, 4, 0) && is(b, co, L1) && is(b, bo, L1);
    fall(b, down);
    ok &= is(b, bo, L0);
    rise(b, down);
    ok &= pins_value(b, q, 4, 15);
    set_pins(b, data, 4, 7);
    set(b, loadpin, load_on);
    set(b, clr, L1);
    step(b);
    expect(ok && pins_value(b, q, 4, 0), what);
    bench_destroy(b);
}

/* ---- 74_561 / 569 / 867 / 869 ------------------------------------------- */

static void check_561(void) {
    Bench *b = open_fx("74_561");
    int ok;

    set(b, "ACLR", L1);
    set(b, "ALOAD", L1);
    set(b, "SCLR", L1);
    set(b, "SLOAD", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set(b, "OE", L0);
    set(b, "CLK", L0);
    step(b);
    pulse(b, "CLK");
    pulse(b, "CLK");
    pulse(b, "CLK");
    set_pins(b, ABCD, 4, 9);
    set(b, "ACLR", L0);
    set(b, "ALOAD", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 0);
    set(b, "ACLR", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);
    set(b, "ALOAD", L1);
    set(b, "SCLR", L0);
    set(b, "SLOAD", L0);
    set_pins(b, ABCD, 4, 5);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0);
    set(b, "SCLR", L1);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 5);
    set(b, "SLOAD", L1);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 6);
    set(b, "OE", L1);
    step(b);
    ok &= pins_all(b, QAD, 4, LZ) && is(b, "RCO", L0);
    set(b, "OE", L0);
    set_pins(b, ABCD, 4, 15);
    set(b, "CLK", L1);
    step(b);
    set(b, "ALOAD", L0);
    step(b);
    set(b, "ALOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 15) && is(b, "RCO", L1) && is(b, "CCO", L0);
    fall(b, "CLK");
    expect(ok && is(b, "CCO", L1) && is(b, "RCO", L1),
           "10 74_561 ACLR > ALOAD > SCLR > SLOAD > count; OE high -> Q = Z, RCO driven; CCO high only "
           "during CLK low");
    bench_destroy(b);
}

static void check_569(void) {
    Bench *b = open_fx("74_569");
    int ok;
    unsigned k;

    set(b, "ACLR", L1);
    set(b, "SCLR", L1);
    set(b, "LOAD", L1);
    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set(b, "U/D", L1);
    set(b, "OE", L0);
    set(b, "CLK", L0);
    step(b);
    for (k = 0; k < 5; k++) {
        pulse(b, "CLK");
    }
    set(b, "SCLR", L0);
    step(b);
    ok = pins_value(b, QAD, 4, 5);
    pulse(b, "CLK");
    ok &= pins_value(b, QAD, 4, 0);
    set(b, "SCLR", L1);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 15);
    step(b);
    rise(b, "CLK");
    set(b, "LOAD", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 15) && is(b, "CCO", L1) && is(b, "RCO", L0);
    fall(b, "CLK");
    ok &= is(b, "CCO", L0);
    set(b, "ENP", L1);
    step(b);
    expect(ok && is(b, "CCO", L1) && is(b, "RCO", L0),
           "11 74_569 SCLR at the edge; CCO low pulse at terminal with both enables low");
    bench_destroy(b);
}

static void check_867(void) {
    Bench *b = open_fx("74_867");
    Bench *c = open_fx("74_869");
    Bench *as = open_fx("74_867_AS");
    int ok;

    set(b, "ENP", L0);
    set(b, "ENT", L0);
    set(b, "CLK", L0);
    set(b, "S1", L1);
    set(b, "S0", L0);
    set_pins(b, D8, 8, 0x5A);
    step(b);
    pulse(b, "CLK");
    ok = pins_value(b, Q8, 8, 0x5A);
    set(b, "S0", L1);   /* 11: up */
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, Q8, 8, 0x5B);
    set(b, "S1", L0);   /* 01: down */
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, Q8, 8, 0x5A);
    set(b, "S0", L0);   /* 00: clear at once */
    step(b);
    ok &= pins_value(b, Q8, 8, 0x00);
    set(b, "S1", L1);   /* 10: load FF */
    set_pins(b, D8, 8, 0xFF);
    step(b);
    pulse(b, "CLK");
    ok &= pins_value(b, Q8, 8, 0xFF) && is(b, "RCO", L1);
    set(b, "S0", L1);   /* 11: up, at max */
    step(b);
    ok &= is(b, "RCO", L0);
    set(b, "ENP", L1);
    step(b);
    ok &= is(b, "RCO", L0);

    set(c, "ENP", L0);
    set(c, "ENT", L0);
    set(c, "CLK", L0);
    set(c, "S1", L1);
    set(c, "S0", L0);
    set_pins(c, D8, 8, 0x33);
    step(c);
    pulse(c, "CLK");
    set(c, "S1", L0);   /* 00 on the 869: clear only at the edge */
    step(c);
    ok &= pins_value(c, Q8, 8, 0x33);
    pulse(c, "CLK");
    ok &= pins_value(c, Q8, 8, 0x00);

    set(as, "ENP", L0);
    set(as, "ENT", L0);
    set(as, "CLK", L0);
    set(as, "S1", L1);
    set(as, "S0", L0);
    set_pins(as, D8, 8, 0xFF);
    step(as);
    pulse(as, "CLK");
    set(as, "S0", L1);
    step(as);
    ok &= is(as, "RCO", L0);
    set(as, "ENP", L1);
    step(as);
    expect(ok && is(as, "RCO", L1),
           "12 74_867 modes: 00 clears at once (869: at the edge); 10 loads; 01/11 count; RCO only in "
           "count modes; AS: RCO needs ENP low too");
    bench_destroy(b);
    bench_destroy(c);
    bench_destroy(as);
}

/* ---- 74_590 / 592 / 593 / 697 / 8154 ------------------------------------- */

static void check_590(void) {
    Bench *b = open_fx("74_590");
    int ok;

    set(b, "CCKEN", L0);
    set(b, "CCLR", L1);
    set(b, "G", L0);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    step(b);
    pulse(b, "CCK");
    pulse(b, "CCK");
    pulse(b, "CCK");
    ok = pins_value(b, Q8, 8, 0);
    pulse(b, "RCK");
    ok &= pins_value(b, Q8, 8, 3);
    set(b, "CCK", L1);
    set(b, "RCK", L1);
    step(b);
    ok &= pins_value(b, Q8, 8, 3);
    set(b, "CCK", L0);
    set(b, "RCK", L0);
    step(b);
    set(b, "CCK", L1);
    set(b, "RCK", L1);
    step(b);
    ok &= pins_value(b, Q8, 8, 4);
    set(b, "CCK", L0);
    set(b, "RCK", L0);
    set(b, "G", L1);
    step(b);
    ok &= pins_all(b, Q8, 8, LZ);
    set(b, "G", L0);
    set(b, "CCLR", L0);
    step(b);
    ok &= pins_value(b, Q8, 8, 4);
    set(b, "CCLR", L1);
    step(b);
    pulse(b, "RCK");
    expect(ok && pins_value(b, Q8, 8, 0),
           "13 74_590 count and register separate; RCK rising copies; tied clocks -> register one "
           "behind; G high -> Z; CCLR clears the counter only");
    bench_destroy(b);
}

static void check_592(void) {
    Bench *b = open_fx("74_592");
    int ok;

    set(b, "CCKEN", L0);
    set(b, "CCLR", L1);
    set(b, "CLOAD", L1);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    set_pins(b, D8, 8, 0xFE);
    step(b);
    pulse(b, "RCK");
    set(b, "CLOAD", L0);
    step(b);
    ok = is(b, "RCO", L1);
    set_pins(b, D8, 8, 0xFF);
    step(b);
    ok &= is(b, "RCO", L1);   /* the input register has not taken FF yet */
    pulse(b, "RCK");
    ok &= is(b, "RCO", L0);   /* RCK during CLOAD low passes through */
    set(b, "CCLR", L0);
    step(b);
    ok &= is(b, "RCO", L1);   /* CCLR wins over CLOAD */
    set(b, "CCLR", L1);
    step(b);
    ok &= is(b, "RCO", L0);
    set(b, "CLOAD", L1);
    step(b);
    pulse(b, "CCK");
    expect(ok && is(b, "RCO", L1),
           "14 74_592 CLOAD low copies the input register continuously; RCK rising during CLOAD low "
           "passes through; CCLR wins over CLOAD");
    bench_destroy(b);
}

static const char *const BUS593[] = {"A/QA", "B/QB", "C/QC", "D/QD",
                                     "E/QE", "F/QF", "G/QG", "H/QH"};

static void check_593(void) {
    Bench *b = open_fx("74_593");
    int ok;

    set(b, "CCKEN", L0);
    set(b, "CCKENBAR", L1);
    set(b, "CCLR", L1);
    set(b, "CLOAD", L1);
    set(b, "RCKEN", L1);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    set(b, "G", L1);
    set(b, "GBAR", L0);
    step(b);
    ok = pins_value(b, BUS593, 8, 0);
    set(b, "G", L0);
    step(b);
    ok &= pins_all(b, BUS593, 8, LZ);
    set(b, "G", L1);
    set(b, "GBAR", L1);
    step(b);
    ok &= pins_all(b, BUS593, 8, LZ);
    set_pins(b, BUS593, 8, 0x42);
    set(b, "RCKEN", L0);
    step(b);
    pulse(b, "RCK");
    set(b, "RCKEN", L1);
    release_pins(b, BUS593, 8);
    set(b, "CLOAD", L0);
    step(b);
    set(b, "CLOAD", L1);
    set(b, "GBAR", L0);
    step(b);
    ok &= pins_value(b, BUS593, 8, 0x42);
    set(b, "CCKEN", L1);
    step(b);
    pulse(b, "CCK");
    ok &= pins_value(b, BUS593, 8, 0x43);
    set(b, "CCKEN", L0);
    set(b, "CCKENBAR", L0);
    step(b);
    pulse(b, "CCK");
    ok &= pins_value(b, BUS593, 8, 0x44);
    set(b, "CCKENBAR", L1);
    step(b);
    pulse(b, "CCK");
    expect(ok && pins_value(b, BUS593, 8, 0x44) && bench_error(b) == NULL,
           "15 74_593 bus driven only with G high and GBAR low; register loads from the pins with "
           "RCKEN low; counts with CCKEN high or CCKENBAR low");
    bench_destroy(b);
}

static void check_697(void) {
    Bench *b = open_fx("74_697");
    int ok;

    set(b, "CCLR", L1);
    set(b, "ENP", L1);
    set(b, "ENT", L1);
    set(b, "U/D", L1);
    set(b, "G", L0);
    set(b, "R/C", L0);
    set(b, "RCK", L0);
    set(b, "CCK", L0);
    set(b, "LOAD", L0);
    set_pins(b, ABCD, 4, 9);
    step(b);
    pulse(b, "CCK");
    set(b, "LOAD", L1);
    step(b);
    ok = pins_value(b, QAD, 4, 9);
    set(b, "R/C", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 0);
    pulse(b, "RCK");
    expect(ok && pins_value(b, QAD, 4, 9),
           "16 74_697 R/C high shows the register, low the counter; LOAD at the edge ignores the "
           "enables");
    bench_destroy(b);
}

static const char *const Y8[] = {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"};

static void selects(Bench *b, int which) {
    static const char *const G[] = {"GAL", "GAU", "GBL", "GBU"};
    int i;

    for (i = 0; i < 4; i++) {
        set(b, G[i], i == which ? L0 : L1);
    }
    step(b);
}

static void check_8154(void) {
    static const char *const JOIN[] = {"RCOA", "CLKBEN", "CLKA", "CLKB", NULL};
    Bench *b = open_fx("74_8154");
    Bench *c;
    int ok;
    unsigned k;

    set(b, "CCLR", L1);
    set(b, "CLKBEN", L0);
    set(b, "CLKA", L0);
    set(b, "CLKB", L0);
    set(b, "RCLK", L0);
    selects(b, -1);
    ok = pins_all(b, Y8, 8, LZ);
    for (k = 0; k < 3; k++) {
        pulse(b, "CLKA");
    }
    for (k = 0; k < 0x0102; k++) {
        pulse(b, "CLKB");
    }
    pulse(b, "RCLK");
    selects(b, 0);
    ok &= pins_value(b, Y8, 8, 3);
    selects(b, 1);
    ok &= pins_value(b, Y8, 8, 0);
    selects(b, 2);
    ok &= pins_value(b, Y8, 8, 0x02);
    selects(b, 3);
    ok &= pins_value(b, Y8, 8, 0x01);
    bench_destroy(b);

    c = open_joined("74_8154", JOIN);
    set(c, "CCLR", L0);
    set(c, "CLKA", L0);
    set(c, "RCLK", L0);
    selects(c, 0);
    set(c, "CCLR", L1);
    step(c);
    for (k = 0; k < 0xFFFF; k++) {
        pulse(c, "CLKA");
    }
    ok &= is(c, "RCOA", L0);
    for (k = 0; k < 6; k++) {
        pulse(c, "CLKA");
    }
    ok &= is(c, "RCOA", L1);
    pulse(c, "RCLK");
    ok &= pins_value(c, Y8, 8, 5);
    selects(c, 2);
    ok &= pins_value(c, Y8, 8, 1);   /* B took the carry: a 32-bit count of 0x10005 */
    set(c, "CCLR", L0);
    step(c);
    ok &= pins_value(c, Y8, 8, 1);   /* CCLR clears the counters, not the register */
    set(c, "CCLR", L1);
    step(c);
    pulse(c, "RCLK");
    expect(ok && pins_value(c, Y8, 8, 0),
           "17 74_8154 GAL shows A low byte, GBU B upper; RCOA low at FFFF; RCOA -> CLKBEN cascade "
           "counts 32 bits; CCLR clears both counters, not the register");
    bench_destroy(c);
}

/* ---- CD4000 sync counters --------------------------------------------------- */

static const char *const JAM[] = {"JAM1", "JAM2", "JAM3", "JAM4"};
static const char *const P14[] = {"P1", "P2", "P3", "P4"};

static void check_4029(void) {
    Bench *b = open_fx("4029B");
    int ok;

    set(b, "PE", L0);
    set(b, "CARRYIN", L0);
    set(b, "UP/DOWN", L1);
    set(b, "BINARY/DECADE", L1);
    set(b, "CLOCK", L0);
    set_pins(b, JAM, 4, 9);
    step(b);
    set(b, "PE", L1);
    step(b);
    ok = pins_value(b, Q14, 4, 9);
    set(b, "PE", L0);
    step(b);
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q14, 4, 10);   /* binary */
    set(b, "BINARY/DECADE", L0);
    set_pins(b, JAM, 4, 9);
    set(b, "PE", L1);
    step(b);
    set(b, "PE", L0);
    step(b);
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q14, 4, 0);    /* decade */
    set(b, "UP/DOWN", L0);
    step(b);
    ok &= is(b, "CARRYOUT", L0);       /* 0 down: terminal */
    pulse(b, "CLOCK");
    ok &= pins_value(b, Q14, 4, 9) && is(b, "CARRYOUT", L1);
    set(b, "BINARY/DECADE", L1);
    set(b, "UP/DOWN", L1);
    set_pins(b, JAM, 4, 15);
    set(b, "PE", L1);
    step(b);
    set(b, "PE", L0);
    step(b);
    ok &= is(b, "CARRYOUT", L0);
    set(b, "CARRYIN", L1);
    step(b);
    expect(ok && is(b, "CARRYOUT", L1),
           "18 4029B binary/decade by the pin; up/down; PRESET ENABLE async; CARRY OUT low at "
           "terminal with CARRY IN low");
    bench_destroy(b);
}

static void check_4510(void) {
    Bench *b = open_fx("4510B");
    int ok;

    set(b, "CARRYIN", L0);
    set(b, "UP/DOWN", L1);
    set(b, "CLOCK", L0);
    set_pins(b, P14, 4, 5);
    set(b, "PE", L1);
    set(b, "RESET", L1);
    step(b);
    ok = pins_value(b, Q14, 4, 0);
    set(b, "RESET", L0);
    step(b);
    expect(ok && pins_value(b, Q14, 4, 5), "19 4510B RESET beats PRESET ENABLE");
    bench_destroy(b);
}

static const char *const Q4A[] = {"Q1A", "Q2A", "Q3A", "Q4A"};

static void check_4518(void) {
    Bench *b = open_fx("4518B");
    int ok;

    set(b, "RESETA", L0);
    set(b, "ENABLEA", L1);
    set(b, "CLOCKA", L0);
    step(b);
    rise(b, "CLOCKA");
    ok = pins_value(b, Q4A, 4, 1);
    fall(b, "CLOCKA");
    fall(b, "ENABLEA");   /* ENABLE falling with CLOCK low */
    ok &= pins_value(b, Q4A, 4, 2);
    rise(b, "CLOCKA");    /* CLOCK rising with ENABLE low */
    ok &= pins_value(b, Q4A, 4, 2);
    fall(b, "CLOCKA");
    rise(b, "ENABLEA");
    ok &= pins_value(b, Q4A, 4, 2);
    set(b, "RESETA", L1);
    step(b);
    expect(ok && pins_value(b, Q4A, 4, 0),
           "20 4518B counts on CLOCK rising with ENABLE high and on ENABLE falling with CLOCK low; "
           "not on CLOCK rising with ENABLE low");
    bench_destroy(b);
}

static const char *const J8[] = {"J0", "J1", "J2", "J3", "J4", "J5", "J6", "J7"};

/* Clocks until CO/ZD goes low; returns the number of clocks (or 1000 if never). */
static unsigned clocks_to_zero(Bench *b) {
    unsigned k;

    for (k = 0; k < 1000 && bench_get(b, "CO/ZD") != L0; k++) {
        pulse(b, "CLOCK");
    }
    return k;
}

static void check_40103(void) {
    Bench *b = open_fx("40103B");
    int ok;
    unsigned n;

    set(b, "CI/CE", L0);
    set(b, "APE", L1);
    set(b, "SPE", L1);
    set(b, "CLOCK", L0);
    set(b, "CLR", L0);
    step(b);
    set(b, "CLR", L1);
    step(b);
    ok = is(b, "CO/ZD", L1);
    n = clocks_to_zero(b);
    ok &= n == 255;
    if (n != 255) {
        printf("  %u clocks from 255 to 0\n", n);
    }
    set_pins(b, J8, 8, 1);
    set(b, "APE", L0);
    step(b);
    ok &= is(b, "CO/ZD", L1);
    set(b, "APE", L1);
    step(b);
    pulse(b, "CLOCK");
    ok &= is(b, "CO/ZD", L0);
    set_pins(b, J8, 8, 3);
    set(b, "SPE", L0);
    step(b);
    ok &= is(b, "CO/ZD", L0);
    pulse(b, "CLOCK");
    set(b, "SPE", L1);
    step(b);
    ok &= is(b, "CO/ZD", L1) && clocks_to_zero(b) == 3;
    pulse(b, "CLOCK");   /* 0 -> 255 */
    ok &= is(b, "CO/ZD", L1);
    expect(ok && clocks_to_zero(b) == 255,
           "21 40103B CLR -> 255; APE async load; SPE at the edge; CO/ZD low at 0 with CI/CE low; "
           "wraps 0 -> 255");
    bench_destroy(b);
}

static void check_40102(void) {
    Bench *b = open_fx("40102B");
    int ok;

    set(b, "CI/CE", L0);
    set(b, "APE", L1);
    set(b, "SPE", L1);
    set(b, "CLOCK", L0);
    set(b, "CLR", L0);
    step(b);
    set(b, "CLR", L1);
    step(b);
    ok = clocks_to_zero(b) == 99;
    set_pins(b, J8, 8, 0x10);   /* BCD 10 */
    set(b, "APE", L0);
    step(b);
    set(b, "APE", L1);
    step(b);
    expect(ok && clocks_to_zero(b) == 10, "22 40102B CLR -> 99; BCD down count 10 -> 09 -> ... -> 0");
    bench_destroy(b);
}

/* ---- 4522B ---------------------------------------------------------------- */

static void check_4522(void) {
    static const char *const JOIN[] = {"ZERO", "PE", NULL};
    static const char *const P[] = {"P0", "P1", "P2", "P3"};
    Bench *b = open_joined("4522B", JOIN);
    unsigned k, pulses = 0;
    int ok = 1;
    LogicValue last = LOGIC_0;

    set(b, "MR", L1);
    set(b, "CF", L1);
    set(b, "CI", L0);
    set(b, "CL", L0);
    set_pins(b, P, 4, 5);
    step(b);
    set(b, "MR", L0);
    step(b);
    step(b);
    last = bench_get(b, "ZERO");
    for (k = 0; k < 20; k++) {
        LogicValue high, low;

        set(b, "CL", L1);
        bench_run(b, NS(200));
        high = bench_get(b, "ZERO");
        set(b, "CL", L0);
        bench_run(b, NS(200));
        low = bench_get(b, "ZERO");
        if (high == L1 && last != L1) {
            pulses++;
            ok &= (k + 1) % 5 == 0;
        }
        ok &= low == L0;
        last = low;
    }
    if (pulses != 4) {
        printf("  %u pulses in 20 clocks\n", pulses);
    }
    expect(ok && pulses == 4,
           "24 4522B N = 5 with ZERO tied to PE: one ZERO pulse per 5 clocks, each during the clock's "
           "high time");
    bench_destroy(b);
}

/* ---- ripple ----------------------------------------------------------------- */

static void check_393(void) {
    static const char *const Q[] = {"1QA", "1QB", "1QC", "1QD"};
    Bench *b = open_fx("74_393");
    int ok = 1;
    unsigned k;

    set(b, "1CLR", L1);
    set(b, "1A", L1);
    step(b);
    set(b, "1CLR", L0);
    step(b);
    for (k = 1; k <= 7; k++) {
        pulse(b, "1A");
        ok &= pins_value(b, Q, 4, k);
    }
    set(b, "1A", L1);
    step(b);
    set(b, "1A", L0);   /* 7 -> 8: QA falls at 10 ns, QD rises at 10 + 3 x 5 ns */
    bench_run(b, NS(15));
    ok &= is(b, "1QA", L0) && is(b, "1QD", L0);
    bench_run(b, NS(15));
    ok &= is(b, "1QD", L1);
    step(b);
    set(b, "1CLR", L1);
    step(b);
    expect(ok && pins_value(b, Q, 4, 0),
           "25 74_393 falling-edge ripple; Q3 changes 3 x stage after Q0; CLR high async");
    bench_destroy(b);
}

static void check_90(void) {
    static const char *const JOIN[] = {"QA", "CKB", NULL};
    Bench *b = open_joined("74_90", JOIN);
    int ok = 1;
    unsigned k;

    set(b, "R0(1)", L1);
    set(b, "R0(2)", L1);
    set(b, "R9(1)", L0);
    set(b, "R9(2)", L0);
    set(b, "CKA", L1);
    step(b);
    set(b, "R0(1)", L0);
    step(b);
    for (k = 1; k <= 10; k++) {
        pulse(b, "CKA");
        set(b, "CKA", L1);
        step(b);
        ok &= pins_value(b, QAD, 4, k % 10);
    }
    set(b, "R9(1)", L1);
    set(b, "R9(2)", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);
    set(b, "R0(1)", L1);
    step(b);
    ok &= pins_value(b, QAD, 4, 9);   /* R9 beats R0 */
    set(b, "R9(1)", L0);
    step(b);
    expect(ok && pins_value(b, QAD, 4, 0),
           "26 74_90 BCD with QA -> CKB (an external wire); R9 sets 9; R9 beats R0");
    bench_destroy(b);
}

static void check_92(void) {
    static const unsigned SEQ[12] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13};
    static const char *const JOIN[] = {"QA", "CKB", NULL};
    Bench *b = open_joined("74_92", JOIN);
    int ok = 1;
    unsigned k;

    set(b, "R0(1)", L1);
    set(b, "R0(2)", L1);
    set(b, "CKA", L1);
    step(b);
    set(b, "R0(1)", L0);
    step(b);
    for (k = 1; k <= 12; k++) {
        fall(b, "CKA");
        rise(b, "CKA");
        ok &= pins_value(b, QAD, 4, SEQ[k % 12]);
    }
    expect(ok, "27 74_92 divide-by-12 sequence (0-5, 8-13 in QD QC QB QA)");
    bench_destroy(b);
}

static void check_4020(void) {
    Bench *b = open_fx("4020B");
    int ok = 1;
    unsigned k;

    set(b, "RESET", L1);
    set(b, "CLK", L1);
    step(b);
    set(b, "RESET", L0);
    step(b);
    for (k = 1; k <= 32; k++) {
        fall(b, "CLK");
        rise(b, "CLK");
        ok &= is(b, "Q4", (k >> 3) & 1u ? L1 : L0) && is(b, "Q1", (k & 1u) ? L1 : L0);
    }
    expect(ok, "28 4020B Q2/Q3 have no pins: Q4 toggles every 8 clocks");
    bench_destroy(b);
}

/* ---- Loader ----------------------------------------------------------------- */

#define BAD_HEAD \
    "{'format':'schematical-parts','version':1,'kind':'counter','devices':[{'id':'bad'," \
    "'function':'Counter','description':'d','drawer':[],'pinouts':{'p4':[" \
    "{'number':1,'name':'CLK','role':'input'},{'number':2,'name':'S','role':'input'}," \
    "{'number':3,'name':'Q','role':'output'},{'number':4,'name':'VCC','role':'power'}]}," \
    "'model':'sync','units':[{"
#define BAD_TAIL "'clock':{'pin':'CLK','edge':'rising'},'count':'up'}]," \
    "'timing':{'HC':{'rise':1,'fall':1,'vcc':[2,6]}},'parts':[]}]}"

static int expect_error(const char *text, const char *want) {
    PartLibrary *lib = fixture_library();
    char *error = load(lib, text, "bad.json");
    int ok = want == NULL ? error == NULL : error != NULL && strstr(error, want) != NULL;

    if (!ok) {
        printf("  error: %s\n  want: %s\n", error != NULL ? error : "(none)",
               want != NULL ? want : "(none)");
    }
    free(error);
    part_library_destroy(lib);
    return ok;
}

static void check_loader(void) {
    int ok = expect_error(BAD_HEAD "'stages':0,'code':'binary'," BAD_TAIL,
                          "\"stages\" must be 1 to 32");

    ok &= expect_error(BAD_HEAD "'stages':6,'code':'bcd'," BAD_TAIL,
                       "a bcd unit's \"stages\" must be a multiple of 4");
    ok &= expect_error(BAD_HEAD "'stages':4,'code':'binary','illegal':{'up':[],'down':[]}," BAD_TAIL,
                       "\"illegal\" is only for a bcd unit");
    ok &= expect_error(BAD_HEAD "'stages':4,'codeSelect':{'pin':'S','binary':'high'}," BAD_TAIL,
                       NULL);
    expect(ok, "29 loader: stages 0, bcd with 6 stages and illegal on a binary unit are errors; "
               "codeSelect with no code loads");
}

int main(void) {
    static const char *const Q193[] = {"QA", "QB", "QC", "QD"};
    static const char *const J14[] = {"J1", "J2", "J3", "J4"};
    char *error;

    library = fixture_library();
    error = load(library, FIXTURE, "counter_fixture.json");
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_161();
    check_163();
    check_169();
    check_190();
    check_191();
    check_dual_clock("74_193", "UP", "DOWN", "LOAD", L0, "CLR", "CO", "BO", Q193, ABCD,
                     "9 74_193 UP rising counts with DOWN high; CO low while UP low at 15; BO low "
                     "while DOWN low at 0; CLR high beats LOAD");
    check_561();
    check_569();
    check_867();
    check_590();
    check_592();
    check_593();
    check_697();
    check_8154();
    check_4029();
    check_4510();
    check_4518();
    check_40103();
    check_40102();
    check_dual_clock("40193B", "CLOCKUP", "CLOCKDOWN", "PE", L0, "RESET", "CARRY", "BORROW", Q14,
                     J14, "23 40193B as 74_193 with an active-low PRESET ENABLE");
    check_4522();
    check_393();
    check_90();
    check_92();
    check_4020();
    check_loader();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("counter_test: all passed\n");
    return 0;
}
