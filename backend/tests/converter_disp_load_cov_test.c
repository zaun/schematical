/*
 * converter_disp_load_cov_test (plan-68-M): the rejections of the display
 * sections of the leddriver / lcddriver kinds (models/converter_load_display.c,
 * converter_load_lcd.c; docs/parts-format.md § leddriver, § lcddriver) and of
 * the rtc kind's clock (models/converter_load_rtc.c; planning/plan-24-data/
 * schema.md § 10). One malformed device per message; load_parts must fail
 * with it and add nothing.
 */

#include "library_check_bench.h"

#define DD(KIND, DISPLAY) \
    HEAD(KIND) "{'id':'L','function':'driver','description':'d','drawer':['Displays & Indicators']," \
    "'pinouts':{'p':[{'number':1,'name':'VCC','role':'power'},{'number':2,'name':'CS','role':'input'}," \
    "{'number':3,'name':'SCLK','role':'input'},{'number':4,'name':'DIN','role':'input'}," \
    "{'number':5,'name':'O1','role':'output'},{'number':6,'name':'O2','role':'output'}]},'vcc':[4.5,5.5]," \
    "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','mode':0}," \
    "'registers':[{'name':'V','address':1,'width':8,'fields':[{'name':'F','bits':[3,0]}]}," \
    "{'name':'W','address':2,'width':8}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'aaaaaaaa dddddddd','action':'write'," \
    "'address':'a','data':'d'}]},'display':" DISPLAY \
    ",'parts':[{'name':'L-1','package':'SOIC','pinout':'p','footprint':'T6'}]}" TAIL
#define LED(D) DD("leddriver", D)
#define LCD(D) DD("lcddriver", D)
#define SEG7 "'segmentPins':['a','b','c','d','e','f','g'],'segmentActive':'high'"
/* digits: one digit, the keys X added. */
#define DIG(X) LED("{'driver':'digits','digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 \
    ",'ram':['V'],'scan':{'periodUs':100}" X "}")
/* digits without the given key (the caller lists the rest). */
#define DIGX(X) LED("{'driver':'digits'" X "}")
#define STAT(X) LED("{'driver':'static','segmentActive':'high','groups':[{'pins':['O1','O2'],'ram':'V'}]" X "}")
#define STATG(G) LED("{'driver':'static','segmentActive':'high','groups':[" G "]}")
#define SH9 "'sharedPins':['S0','S1','S2','S3','S4','S5','S6','S7','S8'],'digitActive':'low','segmentActive':'high'"
#define LINES8 "'segmentLines':['a','b','c','d','e','f','g','dp']"
#define SHARED(X) LED("{'driver':'digits','digits':1," SH9 "," LINES8 ",'ram':['V'],'scan':{'periodUs':100}" X "}")
#define PORT(P) LED("{'driver':'ports','ports':[" P "]}")
#define LCDS(X) LCD("{'driver':'lcd','mode':'static','backplanes':['BP'],'segmentPins':['S0'],'ram':['V'],'frameUs':1000" X "}")
#define LCDM(X) LCD("{'driver':'lcd','mode':{'field':'V.F','values':{'0':'static'}},'backplanes':['BP']," \
    "'segmentPins':['S0'],'ram':['V'],'frameUs':1000,'slots':{'static':[['BP']]}" X "}")
#define DIGIT "{'value':'hourUnits','cells':[null,null,null,null,null,null,null],'glyphs':['1111110']}"
#define CLOCKD(X) LCD("{'driver':'clock','backplanes':['B0','B1'],'segmentPins':['S0','S1'],'frameUs':1000" X)
#define CLOCK(X) CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[" DIGIT "]" X)
#define CCSI(X) LED("{'driver':'ccsi'" X "}")

typedef struct {
    const char *name, *json, *expect;
} Case;

/* models/converter_load_display.c */
static const Case DISPLAY[] = {
    {"display ram without bytes", STAT(",'ram':{'address':0}"), "missing \"bytes\""},
    {"empty pin name", STATG("{'pins':[''],'ram':'V'}"), "a pin name is empty"},
    {"group without pins", STATG("{'ram':'V'}"), "missing \"pins\""},
    {"pins not an array", STATG("{'pins':5,'ram':'V'}"), "\"pins\" must be an array of pin names"},
    {"pins item", STATG("{'pins':[5],'ram':'V'}"), "\"pins\" must be an array of pin names"},
    {"static without segmentActive", LED("{'driver':'static','groups':[]}"), "missing \"segmentActive\""},
    {"group ram R", STATG("{'pins':['O1'],'ram':'R'}"), "\"R\" is not a register or field"},
    {"digits without ram", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'scan':{'periodUs':1}"),
     "missing \"ram\""},
    {"ram a number", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'ram':5"),
     "\"ram\" is an array of register names or {\"bytes\": n}"},
    {"ram empty", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'ram':[]"), "\"ram\" is empty"},
    {"ram item", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'ram':[5]"),
     "\"ram\" is an array of register names"},
    {"segmentBits count", DIG(",'segmentBits':[0]"), "\"segmentBits\" must list 7 bit numbers, one per pin"},
    {"segmentBits range", DIG(",'segmentBits':[0,1,2,3,4,5,40]"), "\"segmentBits\" holds bit numbers 0-31"},
    {"periodUs too short", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'ram':['V'],"
                                "'scan':{'periodUs':1e-7}"), "\"periodUs\" is shorter than 1 ps"},
    {"scan unknown key", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'ram':['V'],"
                              "'scan':{'bad':1}"), "unknown key \"bad\""},
    {"scan without periodUs", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low'," SEG7 ",'ram':['V'],"
                                   "'scan':{}"), "missing \"periodUs\""},
    {"decode a number", DIG(",'decode':5"), "\"decode\" must be a string"},
    {"decodeActive 2", DIG(",'decode':'hex','decodeActive':2"), "\"decodeActive\" is 0 or 1 and \"dpBit\" is 0-31"},
    {"ascii16 on 7 pins", DIG(",'decode':'ascii16'"), "\"decode\" \"ascii16\" needs 16 segment pins"},
    {"decodeField unknown", DIG(",'decode':'hex','decodeField':'NOPE'"), "unknown register \"NOPE\""},
    {"decodeField a number", DIG(",'decode':'hex','decodeField':5"), "\"decodeField\" is a field name or an array of 1"},
    {"openDrain not an array", DIG(",'openDrain':5"), "\"openDrain\" must be an array of pin names"},
    {"shared digits 0", LED("{'driver':'digits','digits':0," SH9 "}"), "\"digits\" must be 1 to 32"},
    {"shared digits over the pins", LED("{'driver':'digits','digits':10," SH9 "}"),
     "\"digits\" is at most the 9 shared pins"},
    {"shared segmentLines missing", LED("{'driver':'digits','digits':1," SH9 "}"),
     "\"segmentLines\" names 8 segments (a-g, dp), one per data line"},
    {"shared segmentLines twice", LED("{'driver':'digits','digits':1," SH9 ",'segmentLines':['a','a','c','d','e','f','g','dp']}"),
     "\"segmentLines\" lists each of a b c d e f g dp once"},
    {"shared segmentBits", SHARED(",'segmentBits':[1]"), "\"segmentBits\" must list 8 bit numbers"},
    {"shared without scan", LED("{'driver':'digits','digits':1," SH9 "," LINES8 ",'ram':['V']}"), "missing \"scan\""},
    {"shared scanLimit", SHARED(",'scanLimit':'NOPE'"), "unknown register \"NOPE\""},
    {"blink unknown key", SHARED(",'blink':{'bad':1}"), "unknown key \"bad\""},
    {"blink without ram", SHARED(",'blink':{'enable':'V.F=1'}"), "\"blink\" needs \"enable\" and \"ram\""},
    {"blink ram count", SHARED(",'blink':{'enable':'V.F=1','ram':['V','W']}"), "\"ram\" has 2 registers for 1 digits"},
    {"blink halfPeriodUs negative", SHARED(",'blink':{'enable':'V.F=1','ram':['W'],'halfPeriodUs':-1}"),
     "\"halfPeriodUs\" must be a time in microseconds"},
    {"blink halfPeriodUs choice", SHARED(",'blink':{'enable':'V.F=1','ram':['W'],'halfPeriodUs':{}}"),
     "a choice needs \"pin\" or \"field\""},
    {"blink halfPeriodUs a string", SHARED(",'blink':{'enable':'V.F=1','ram':['W'],'halfPeriodUs':'x'}"),
     "\"halfPeriodUs\" is a time in microseconds or {\"field\", \"values\"}"},
    {"digits 0", DIGX(",'digits':0"), "\"digits\" must be 1 to 32"},
    {"segmentPins 9", DIGX(",'digits':1,'digitPins':['D0'],'digitActive':'low','segmentActive':'high',"
                           "'segmentPins':['a','b','c','d','e','f','g','h','i']"), "\"segmentPins\" names 7 pins"},
    {"static decode a number", STAT(",'decode':5"), "\"decode\" must be a string"},
    {"groups empty", STATG(""), "\"groups\" must be an array of 1 to 32 groups"},
    {"group decodeField without decode", STATG("{'pins':['O1'],'ram':'V','decodeField':'V.F'}"),
     "\"decodeField\" needs a \"decode\" other than \"none\""},
    {"static segmentBits", STAT(",'segmentBits':[0]"), "\"segmentBits\" must list 2 bit numbers"},
    {"sinkValues", PORT("{'pin':'O1','dataField':'V.F','dirField':'W','sinkValues':-1}"), "sinkValues must be a whole number"},
    {"sinkValues item", PORT("{'pin':'O1','dataField':'V.F','dirField':'W','sinkValues':[-1]}"), "sinkValues must be"},
    {"ports empty", LED("{'driver':'ports','ports':[]}"), "\"ports\" must be an array of 1 to 256 ports"},
    {"port without dataField", PORT("{'pin':'O1'}"), "missing \"dataField\""},
    {"port dirField", PORT("{'pin':'O1','dataField':'V.F','dirField':'NOPE'}"), "unknown register \"NOPE\""},
    {"dirOutput", PORT("{'pin':'O1','dataField':'V.F','dirField':'W','dirOutput':-1}"), "\"dirOutput\" must be a whole number"},
    {"dirOutput item", PORT("{'pin':'O1','dataField':'V.F','dirField':'W','dirOutput':[-1]}"), "\"dirOutput\" must be"},
    {"dirOutput a string", PORT("{'pin':'O1','dataField':'V.F','dirField':'W','dirOutput':'x'}"),
     "\"dirOutput\" is a value or an array of 1 to 64 values"},
    {"port invert", PORT("{'pin':'O1','dataField':'V.F','invert':5}"), "\"invert\" must be"},
};

/* models/converter_load_lcd.c (lcd, clock, ccsi) and the display entry points. */
static const Case LCDC[] = {
    {"mode field unknown", LCD("{'driver':'lcd','mode':{'field':'NOPE'},'backplanes':['BP'],'segmentPins':['S0'],"
                               "'ram':['V'],'frameUs':1000}"), "unknown register \"NOPE\""},
    {"lcd with a mode field: unknown key", LCDM(",'bad':0") , "unknown key \"bad\""},
    {"mode values none", LCD("{'driver':'lcd','mode':{'field':'V.F','values':{}},'backplanes':['BP'],"
                             "'segmentPins':['S0'],'ram':['V'],'frameUs':1000,'slots':{}}"),
     "\"mode\" \"values\" maps 1 to"},
    {"mode values key", LCD("{'driver':'lcd','mode':{'field':'V.F','values':{'x':'static'}},'backplanes':['BP'],"
                            "'segmentPins':['S0'],'ram':['V'],'frameUs':1000,'slots':{}}"),
     "\"mode\" \"values\" is field value -> \"static\""},
    {"mode values twice", LCD("{'driver':'lcd','mode':{'field':'V.F','values':{'0':'static','00':'static'}},"
                              "'backplanes':['BP'],'segmentPins':['S0'],'ram':['V'],'frameUs':1000,"
                              "'slots':{'static':[['BP']]}}"), "\"mode\" \"values\" names field value 0 twice"},
    {"slot not an array", LCD("{'driver':'lcd','mode':{'field':'V.F','values':{'0':'static'}},'backplanes':['BP'],"
                              "'segmentPins':['S0'],'ram':['V'],'frameUs':1000,'slots':{'static':[5]}}"),
     "a slot of \"slots\" \"static\" is an array of backplane pin names"},
    {"slots extra mode", LCD("{'driver':'lcd','mode':{'field':'V.F','values':{'0':'static'}},'backplanes':['BP'],"
                             "'segmentPins':['S0'],'ram':['V'],'frameUs':1000,'slots':{'static':[['BP']],'1:2':[]}}"),
     "\"slots\" names a mode that \"mode\" \"values\" does not"},
    {"load step", LCDM(",'load':{'pointer':'W','step':5}"), "\"step\" maps each mode name"},
    {"load pointer", LCDM(",'load':{'pointer':'NOPE','step':{'static':1}}"), "unknown register \"NOPE\""},
    {"load bank", LCDM(",'load':{'pointer':'W','step':{'static':1},'bank':'NOPE'}"), "unknown register \"NOPE\""},
    {"load hardware", LCDM(",'load':{'pointer':'W','step':{'static':1},'subaddress':'V.F','hardware':'NOPE'}"),
     "unknown register \"NOPE\""},
    {"load subaddresses alone", LCDM(",'load':{'pointer':'W','step':{'static':1},'subaddresses':4}"),
     "\"subaddresses\" needs \"subaddress\""},
    {"lcd unknown key", LCDS(",'bad':1"), "unknown key \"bad\""},
    {"lcd without mode", LCD("{'driver':'lcd','backplanes':['BP']}"), "missing \"mode\""},
    {"ramOrder a number", LCDS(",'ramOrder':5"), "\"ramOrder\" must be a string"},
    {"ramOrder unknown", LCDS(",'ramOrder':'row'"), "\"ramOrder\" must be \"segment\" or \"backplane\""},
    {"bitOrder a number", LCDS(",'bitOrder':5"), "\"bitOrder\" must be a string"},
    {"bitOrder unknown", LCDS(",'bitOrder':'mid'"), "\"bitOrder\" must be \"lsb\" or \"msb\""},
    {"lcd without frameUs", LCD("{'driver':'lcd','mode':'static','backplanes':['BP'],'segmentPins':['S0'],'ram':['V']}"),
     "missing \"frameUs\""},
    {"frameUs under 1 ps per backplane",
     LCD("{'driver':'lcd','mode':'1:2','backplanes':['B0','B1'],'segmentPins':['S0'],'ram':['V'],'frameUs':1e-6}"),
     "\"frameUs\" is shorter than 1 ps per backplane"},
    {"outputBank", LCDM(",'outputBank':'NOPE'"), "unknown register \"NOPE\""},
    {"clock cell a number", CLOCK(",'colon':5}"), "a cell is null or {pin, bp}"},
    {"clock cell without bp", CLOCK(",'colon':{'pin':'S0'}}"), "missing \"bp\""},
    {"clock cell pin", CLOCK(",'colon':{'pin':'S9','bp':'B0'}}"), "cell pin \"S9\" is not one of \"segmentPins\""},
    {"clock cell bp", CLOCK(",'colon':{'pin':'S0','bp':'B9'}}"), "cell bp \"B9\" is not one of \"backplanes\""},
    {"clock debounceMs", CLOCK(",'debounceMs':-1}"), "\"debounceMs\" must be a time in milliseconds above 0"},
    {"clock unknown key", CLOCK(",'bad':1}"), "unknown key \"bad\""},
    {"clock one backplane", LCD("{'driver':'clock','backplanes':['B0'],'segmentPins':['S0'],'frameUs':1000}"),
     "\"backplanes\" must name 2 to"},
    {"clock without frameUs", LCD("{'driver':'clock','backplanes':['B0','B1'],'segmentPins':['S0']}"),
     "missing \"frameUs\""},
    {"clock frameUs", LCD("{'driver':'clock','backplanes':['B0','B1'],'segmentPins':['S0'],'frameUs':-1}"),
     "\"frameUs\" must be a time in microseconds"},
    {"clock mode pin", CLOCK(",'mode':''}"), "\"mode\" must be a pin name"},
    {"clock fastDivisor 0", CLOCK(",'fastDivisor':0}"), "\"fastDivisor\" is 1 or more"},
    {"clock fastDivisor -1", CLOCK(",'fastDivisor':-1}"), "fastDivisor must be a whole number"},
    {"clock without keys", CLOCKD("}"), "\"keys\" is an array of 1-2 {pin, unit}"},
    {"clock key without unit", CLOCKD(",'keys':[{'pin':'K'}]}"), "missing \"unit\""},
    {"clock key unit", CLOCKD(",'keys':[{'pin':'K','unit':'day'}]}"), "\"unit\" is \"hour\" or \"minute\""},
    {"clock without digits", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}]}"), "\"digits\" is an array of 1-"},
    {"clock digit without value", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{}]}"), "missing \"value\""},
    {"clock digit value", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{'value':'seconds'}]}"),
     "\"value\" is hourTens, hourUnits, minuteTens or minuteUnits"},
    {"clock digit cells", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{'value':'hourUnits','cells':[]}]}"),
     "\"cells\" is an array of 7 cells"},
    {"clock digit cell", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{'value':'hourUnits',"
                                "'cells':[5,null,null,null,null,null,null]}]}"), "a cell is null or {pin, bp}"},
    {"clock digit glyphs", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{'value':'hourUnits',"
                                  "'cells':[null,null,null,null,null,null,null]}]}"), "\"glyphs\" is an array of 1-"},
    {"clock glyph length", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{'value':'hourUnits',"
                                  "'cells':[null,null,null,null,null,null,null],'glyphs':['11']}]}"),
     "a glyph is 7 characters, 0 or 1 for segments a-g"},
    {"clock glyph character", CLOCKD(",'keys':[{'pin':'K','unit':'hour'}],'digits':[{'value':'hourUnits',"
                                     "'cells':[null,null,null,null,null,null,null],'glyphs':['111111x']}]}"),
     "a glyph is 7 characters, 0 or 1 for segments a-g"},
    {"ccsi unknown key", CCSI(",'bad':1"), "unknown key \"bad\""},
    {"ccsi without sclk", CCSI(""), "missing \"sclk\""},
    {"ccsi lines", CCSI(",'sclk':'K','sin':'I','sout':'O','lines':['L0']"), "\"lines\" must name 16 pins"},
    {"display not an object", LED("5"), "\"display\" must be an object"},
    {"lcdload without a load section", HEAD("lcddriver") "{'id':'L','function':'driver','description':'d',"
     "'drawer':['Displays & Indicators'],'pinouts':{'p':[{'number':1,'name':'VCC','role':'power'}]},'vcc':[4.5,5.5],"
     "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','mode':0},"
     "'registers':[{'name':'V','address':1,'width':8,'fields':[{'name':'F','bits':[3,0]}]}],"
     "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'0000 dddd','action':'nop','then':['nop','lcdload']}]},"
     "'display':{'driver':'lcd','mode':'static','backplanes':['BP'],'segmentPins':['S0'],'ram':['V'],'frameUs':1000},"
     "'parts':[{'name':'L-1','package':'SOIC','pinout':'p','footprint':'T1'}]}" TAIL,
     "action \"lcdload\" needs an lcd \"display\" with a \"load\" section"},
};

#define RTC(CLOCK) \
    HEAD("rtc") "{'id':'R','function':'RTC','description':'d','drawer':['Interface']," \
    "'pinouts':{'p':[{'number':1,'name':'VCC','role':'power'},{'number':2,'name':'SCL','role':'input'}," \
    "{'number':3,'name':'SDA','role':'inout'},{'number':4,'name':'GND','role':'ground'}]},'vcc':[2,5.5]," \
    "'bus':{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[104]}," \
    "'registers':[{'name':'SEC','address':0,'width':8},{'name':'MIN','address':1,'width':8}," \
    "{'name':'HR','address':2,'width':8},{'name':'CTL','address':3,'width':8,'fields':[{'name':'A','bits':[0,0]}]}]," \
    "'protocol':{'frame':'fixed','commands':[{'name':'P','in':'aaaaaaaa','action':'pointer','address':'a'}],'read':'pointer'}," \
    "'clock':" CLOCK ",'parts':[{'name':'R-1','package':'SOIC','pinout':'p','footprint':'T4'}]}" TAIL
#define TF "{'field':'SEC','unit':'second'},{'field':'MIN','unit':'minute'},{'field':'HR','unit':'hour'}"
#define CLK(X) RTC("{'fields':[" TF "]" X "}")
#define ALARM(M) CLK(",'alarms':[{'match':[" M "]}]")

/* models/converter_load_rtc.c */
static const Case CLOCKS[] = {
    {"field without unit", RTC("{'fields':[{'field':'SEC'}]}"), "missing \"unit\""},
    {"alarm pin", CLK(",'alarms':[{'match':[],'pin':{'pin':''}}]"), "\"pin\" must be a pin name"},
    {"alarm pin a number", CLK(",'alarms':[{'match':[],'pin':5}]"), "a pin is a name or {pin, active, openDrain}"},
    {"periodic periods empty", CLK(",'periodic':{'field':'CTL.A','periods':{}}"),
     "\"periods\" is a non-empty object value -> ns"},
    {"periodic period", CLK(",'periodic':{'field':'CTL.A','periods':{'0':-1}}"), "periods must be a time in ns"},
    {"periodic unknown key", CLK(",'periodic':{'bad':1}"), "unknown key \"bad\""},
    {"periodic array item", CLK(",'periodic':[{'bad':1}]"), "unknown key \"bad\""},
    {"clock unknown key", CLK(",'bad':1"), "unknown key \"bad\""},
    {"fields empty", RTC("{'fields':[]}"), "\"fields\" must be a non-empty array"},
    {"format unknown", RTC("{'fields':[{'field':'SEC','unit':'second','format':'gray'}]}"),
     "\"format\" is bcd, binary or {field, binary}"},
    {"format field", RTC("{'fields':[{'field':'SEC','unit':'second','format':{'field':'NOPE'}}]}"),
     "unknown register \"NOPE\""},
    {"format a number", RTC("{'fields':[{'field':'SEC','unit':'second','format':5}]}"),
     "\"format\" is bcd, binary or {field, binary}"},
    {"stop unknown key", CLK(",'stop':{'bad':1}"), "unknown key \"bad\""},
    {"stop running", CLK(",'stop':{'field':'CTL.A','running':-1}"), "running must be a whole number"},
    {"stop stopped", CLK(",'stop':{'field':'CTL.A','stopped':-1}"), "stopped must be a whole number"},
    {"update a number", CLK(",'update':5"), "\"update\" is none, {uip}, {latch}"},
    {"update uip", CLK(",'update':{'uip':'NOPE'}"), "unknown register \"NOPE\""},
    {"update latch", CLK(",'update':{'latch':'NOPE'}"), "unknown register \"NOPE\""},
    {"update empty", CLK(",'update':{}"), "\"update\" needs \"uip\" or \"latch\""},
    {"alarms not an array", CLK(",'alarms':5"), "\"alarms\" must be an array"},
    {"alarm match", CLK(",'alarms':[{'match':5}]"), "\"match\" must be an array"},
    {"dontCare empty", ALARM("{'field':'SEC','unit':'second','dontCare':''}"),
     "\"dontCare\" is a bit pattern like \"11xxxxxx\""},
    {"dontCare character", ALARM("{'field':'SEC','unit':'second','dontCare':'1q'}"), "\"dontCare\" is 0, 1 and x"},
    {"watchdog unknown key", CLK(",'watchdog':{'bad':1}"), "unknown key \"bad\""},
    {"watchdog kick", CLK(",'watchdog':{'field':'CTL','periods':{'0':1},'kick':{'pin':''}}"),
     "\"pin\" must be a pin name"},
    {"watchdog kick edge", CLK(",'watchdog':{'field':'CTL','periods':{'0':1},'kick':{'pin':'K','edge':5}}"),
     "\"edge\" must be a string"},
    {"watchdog reset pulse", CLK(",'watchdog':{'field':'CTL','periods':{'0':1},'resetPin':{'pin':'RST','pulse':{}}}"),
     "\"pulse\" is a non-empty object value -> ns"},
    {"ram bytes", CLK(",'ram':{'bytes':-1}"), "bytes must be a whole number"},
    {"ram bytes 0", CLK(",'ram':{'bytes':0}"), "clock.ram: \"bytes\" is 1-65536"},
    {"stop field R", CLK(",'stop':{'field':'R'}"), "\"field\" names a register or a field, not R or CH"},
    {"alarm flag CH", CLK(",'alarms':[{'match':[],'flag':'CH'}]"), "\"flag\" names a register or a field, not R or CH"},
};

/* 31 static groups of 17 pins: 527 pins, more than the 512 one display section may use. */
static void too_many_pins(void) {
    static char text[40000];
    size_t g, p, n;

    n = (size_t)snprintf(text, sizeof text, "%s",
                         HEAD("leddriver") "{'id':'L','function':'driver','description':'d','drawer':[],"
                         "'pinouts':{'p':[{'number':1,'name':'VCC','role':'power'}]},'vcc':[4.5,5.5],"
                         "'bus':{'style':'none'},'registers':[{'name':'V','width':8}],"
                         "'display':{'driver':'static','segmentActive':'high','groups':[");
    for (g = 0; g < 31; g++) {
        n += (size_t)snprintf(text + n, sizeof text - n, "%s{'ram':'V','pins':[", g ? "," : "");
        for (p = 0; p < 17; p++) {
            n += (size_t)snprintf(text + n, sizeof text - n, "%s'P%zu_%zu'", p ? "," : "", g, p);
        }
        n += (size_t)snprintf(text + n, sizeof text - n, "]}");
    }
    snprintf(text + n, sizeof text - n, "%s",
             "]},'parts':[{'name':'L-1','package':'SOIC','pinout':'p','footprint':'T1'}]}" TAIL);
    expect_fail("too many pins in a display", text, "too many pins in \"display\"");
}

static void run(const Case *cases, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        expect_fail(cases[i].name, cases[i].json, cases[i].expect);
    }
}

int main(void) {
    run(DISPLAY, sizeof DISPLAY / sizeof DISPLAY[0]);
    run(LCDC, sizeof LCDC / sizeof LCDC[0]);
    run(CLOCKS, sizeof CLOCKS / sizeof CLOCKS[0]);
    too_many_pins();
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("converter_disp_load_cov_test: all passed\n");
    return 0;
}
