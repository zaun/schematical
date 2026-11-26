/*
 * converter_load_cov_test (plan-68-M): the converter loader's rejections
 * (models/converter_load.c, converter_load_conv.c, converter_load_proto.c):
 * one malformed device per message, each the smallest change to a valid
 * SPI ADC (docs/parts-format.md § converter, planning/plan-24-data/schema.md).
 * load_parts must fail with the message and add nothing.
 */

#include "library_check_bench.h"

#define P(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define PINS6 "'pinouts':{'p':[" P(1, "VDD", "power") "," P(2, "SCLK", "input") "," \
    P(3, "SDO", "output") "," P(4, "CS", "input") "," P(5, "DIN", "input") "," P(6, "GND", "ground") "]}"
#define PART6(EXTRA) "'parts':[{'name':'C-1','package':'SOIC','pinout':'p','footprint':'T6'" EXTRA "}]"
#define DEV(BODY, PARTX) \
    HEAD("converter") "{'id':'C','function':'ADC','description':'d','drawer':['Interface']," PINS6 "," \
    BODY "," PART6(PARTX) "}" TAIL
#define ADC12 "'type':'adc','bits':12,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]"
#define SPIBUS "'bus':{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','miso':'SDO','mode':0}"
#define REGS1 "'registers':[{'name':'CFG','address':1,'width':8,'fields':[{'name':'A','bits':[3,0]}]}]"
#define CONV0 "'conversion':{'time':0,'start':['cs-fall']}"
/* A valid ADC with protocol keys PX (after 'frame') and more device keys MORE. */
#define ADC_P(PX, MORE) DEV(ADC12 "," SPIBUS "," REGS1 ",'protocol':{'frame':'stream'" PX "}," CONV0 MORE, "")
/* ... with one command C. */
#define ADC_C(C) ADC_P(",'commands':[" C "]", "")
/* ... with the registers R instead. */
#define ADC_R(R) DEV(ADC12 "," SPIBUS ",'registers':[" R "],'protocol':{'frame':'stream'}," CONV0, "")
/* ... with the conversion C instead. */
#define ADC_CONV(C) DEV(ADC12 "," SPIBUS "," REGS1 ",'protocol':{'frame':'stream'},'conversion':" C, "")
/* ... with the bus B instead. */
#define ADC_BUS(B) DEV(ADC12 ",'bus':" B "," REGS1 ",'protocol':{'frame':'stream'}," CONV0, "")
/* ... with part keys X. */
#define ADC_PART(X) DEV(ADC12 "," SPIBUS "," REGS1 ",'protocol':{'frame':'stream'}," CONV0, X)
/* ... with these device head keys H instead of ADC12. */
#define ADC_HEAD(H) DEV(H "," SPIBUS "," REGS1 ",'protocol':{'frame':'stream'}," CONV0, "")
/* A valid DAC with a 'dac' section D and more keys MORE. */
#define DAC_D(D, MORE) DEV("'type':'dac','bits':8,'channels':1,'vcc':[2.7,5.5]," SPIBUS "," \
    "'registers':[{'name':'IN','space':'input','width':8},{'name':'OUT','space':'dac','width':8}]," \
    "'protocol':{'frame':'stream'}," "'dac':" D MORE, "")
#define DAC_OK "{'input':['IN'],'output':['OUT'],'ldac':'sync'}"

#define X10 "AAAAAAAAAA"
#define X130 X10 X10 X10 X10 X10 X10 X10 X10 X10 X10 X10 X10 X10

typedef struct {
    const char *name, *json, *expect;
} Case;

/* models/converter_load.c: values, names, conditions, patterns, the device, parts. */
static const Case LOAD[] = {
    /* plan-68-L: once checked only for a device with a display, so a converter's lcdload loaded and
       reached disp_lcd_byte with no display. */
    {"lcdload without a display", ADC_C("{'in':'0101','action':'lcdload'}"),
     "action \"lcdload\" needs an lcd \"display\" with a \"load\" section"},
    {"time out of range", ADC_P("", ",'timing':{'tValid':-1}"), "tValid must be a time in ns from 0 to 1e13"},
    {"timing unknown key", ADC_P("", ",'timing':{'tHold':1}"), "unknown key \"tHold\""},
    {"u32 not whole", ADC_R("{'name':'R','width':8,'reset':1.5}"), "reset must be a whole number 0-4294967295"},
    {"level not a number", ADC_BUS("{'style':'spi','sclk':'SCLK','csActive':'x'}"),
     "csActive must be a whole number"},
    {"level 2", ADC_BUS("{'style':'spi','sclk':'SCLK','csActive':2}"), "\"csActive\" must be 0 or 1"},
    {"empty pin name", ADC_BUS("{'style':'spi','sclk':''}"), "\"sclk\" must be a pin name"},
    {"long register name", ADC_C("{'in':'0101','action':'wreg','register':'" X130 "'}"), "is too long"},
    {"channel field not a string", ADC_CONV("{'time':0,'channel':5}"), "\"channel\" must be a string"},
    {"long condition", ADC_C("{'in':'0101','when':'" X130 X10 X10 X10 X10 "=1'}"), "is too long"},
    {"condition without =", ADC_C("{'in':'0101','when':'CFG.A'}"), "want REG.FIELD=v or REG.FIELD!=v"},
    {"condition with = at the end", ADC_C("{'in':'0101','when':'CFG.A='}"), "want REG.FIELD=v"},
    {"condition value", ADC_C("{'in':'0101','when':'CFG.A=zz'}"), "the value must be a number"},
    {"condition not a string", ADC_C("{'in':'0101','when':5}"), "a condition must be a string or an array"},
    {"condition array item", ADC_C("{'in':'0101','when':[5]}"), "a condition must be a string"},
    {"condition array bad field", ADC_C("{'in':'0101','when':['CFG.Q=1']}"), "has no field \"Q\""},
    {"capture letter", ADC_C("{'in':'0101aaaa','action':'wreg','address':'xx'}"),
     "\"address\": a capture letter a-y (not x) or a number"},
    {"capture x", ADC_C("{'in':'0101aaaa','action':'wreg','address':'x'}"), "a capture letter a-y (not x)"},
    {"pattern bad bracket", ADC_C("{'in':'0101 [CFG'}"), "bad [...]"},
    {"pattern bracket hi:lo", ADC_C("{'in':'0101 [CFG:3]'}"), "[NAME:hi:lo]"},
    {"pattern R bracket in input", ADC_C("{'in':'0101 [R]'}"), "[R] in an input pattern"},
    {"pattern CH bracket in input", ADC_C("{'in':'0101 [CH]'}"), "[CH] in an input pattern"},
    {"pattern slice outside", ADC_C("{'in':'0101','reply':'[CFG:9:0]'}"), "[CFG:9:0] is outside it"},
    {"pattern slice reversed", ADC_C("{'in':'0101','reply':'[CFG:0:3]'}"), "[CFG:0:3] is outside it"},
    {"pattern z in input", ADC_C("{'in':'01z'}"), "'z' in an input pattern"},
    {"pattern x in output", ADC_C("{'in':'01','reply':'0x'}"), "'x' in an output pattern"},
    {"pattern R in input", ADC_C("{'in':'01R'}"), "'R' in an input pattern"},
    {"pattern @ in input", ADC_C("{'in':'01@'}"), "'@' in an input pattern"},
    {"pattern bad character", ADC_C("{'in':'01Q'}"), "bad character 'Q'"},
    {"pattern {0}", ADC_C("{'in':'0{0}'}"), "bad {n}"},
    {"pattern {n unclosed", ADC_C("{'in':'0{3'}"), "bad {n}"},
    {"pattern R run too long", ADC_C("{'in':'01','reply':'R{13}'}"), "a run of R longer than \"bits\" (12)"},
    {"pattern too long", ADC_C("{'in':'0{4096}1'}"), "is longer than 4096 bits"},
    {"pattern capture too long", ADC_C("{'in':'a{33}'}"), "a capture longer than 32 bits"},
    {"type missing", ADC_HEAD("'bits':12,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]"), "missing \"type\""},
    {"type unknown", ADC_HEAD("'type':'pga','bits':12,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]"),
     "\"type\" is \"adc\" or \"dac\""},
    {"bits too many", ADC_HEAD("'type':'adc','bits':33,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]"),
     "bits"},
    {"bits 0", ADC_HEAD("'type':'adc','bits':0,'channels':1,'codeFormat':'straight','vcc':[2.7,3.6]"),
     "\"bits\" and \"channels\" must be at least 1"},
    {"ADC without codeFormat", ADC_HEAD("'type':'adc','bits':12,'channels':1,'vcc':[2.7,3.6]"),
     "an ADC needs \"codeFormat\""},
    {"codeFormat unknown", ADC_HEAD("'type':'adc','bits':12,'channels':1,'codeFormat':'gray','vcc':[2.7,3.6]"),
     "\"codeFormat\" is straight, twos or offset"},
    {"analog not an array", ADC_P("", ",'analog':'AIN'"), "\"analog\" must be an array of pin names"},
    {"analog item", ADC_P("", ",'analog':[5]"), "\"analog\" must be an array of pin names"},
    {"bus missing", DEV(ADC12 "," REGS1 ",'protocol':{'frame':'stream'}," CONV0, ""), "\"bus\" must be an object"},
    {"configBus parallel", ADC_P("", ",'configBus':{'style':'parallel'}"), "\"configBus\" is \"spi\" or \"i2c\""},
    {"configBus i2c-spi", ADC_P("", ",'configBus':{'style':'i2c-spi'}"), "\"configBus\" is \"spi\" or \"i2c\""},
    {"configBus none", ADC_P("", ",'configBus':{'style':'none'}"), "\"none\" is a bus style of a display driver only"},
    {"dac on an ADC", ADC_P("", ",'dac':{}"), "\"dac\" is for type \"dac\""},
    {"ADC without conversion",
     DEV(ADC12 "," SPIBUS "," REGS1 ",'protocol':{'frame':'stream'}", ""), "an ADC needs \"conversion\""},
    {"DAC ldac pin without pins.ldac", DAC_D("{'input':['IN'],'output':['OUT'],'ldac':'pin'}", ""),
     "\"dac.ldac\" \"pin\" needs \"pins.ldac\""},
    {"vcc bad", DEV("'type':'adc','bits':12,'channels':1,'codeFormat':'straight','vcc':'x'," SPIBUS ","
                    REGS1 ",'protocol':{'frame':'stream'}," CONV0, ""), "vcc"},
    {"part i2cAddress", ADC_PART(",'i2cAddress':200"), "\"i2cAddress\" is a 7-bit address"},
    {"part i2cAddress not a number", ADC_PART(",'i2cAddress':'x'"), "\"i2cAddress\" is a 7-bit address"},
    {"part i2c unknown key", ADC_PART(",'i2c':{'scl':'SCLK'}"), "unknown key \"scl\""},
    {"part i2c address not an array", ADC_PART(",'i2c':{'address':'CS','table':[1]}"),
     "\"i2c\": \"address\" is an array of strap pins"},
    {"part i2c address unknown pin", ADC_PART(",'i2c':{'address':['A9'],'table':[1,2,3,4]}"), "A9"},
    {"part i2c address item", ADC_PART(",'i2c':{'address':[5],'table':[1,2,3,4]}"), "\"i2c\": strap pin names"},
    {"part i2c table count", ADC_PART(",'i2c':{'address':['CS'],'table':[1]}"), "\"i2c\": \"table\" needs 4 entries"},
    {"part resets not an object", ADC_PART(",'resets':[1]"), "\"resets\" is an object of register values"},
    {"part resets unknown register", ADC_PART(",'resets':{'NOPE':1}"), "unknown register \"NOPE\""},
    {"part resets value", ADC_PART(",'resets':{'CFG':-1}"), "a reset value must be a whole number"},
    {"part resets R", ADC_PART(",'resets':{'R':1}"), "\"resets\" names registers"},
};

#define CF(X) ADC_HEAD("'type':'adc','bits':12,'channels':1,'codeFormat':" X ",'vcc':[2.7,3.6]")
#define PINS(X) ADC_P("", ",'pins':" X)
#define CLEAR(X) DAC_D("{'input':['IN'],'output':['OUT'],'ldac':'sync','clearCode':" X "}", "")

/* models/converter_load_conv.c: choices, conversion, ready, pins, dac. */
static const Case CONV[] = {
    {"choice unknown key", CF("{'pin':'CS','values':{'0':'twos'},'bad':1}"), "unknown key \"bad\""},
    {"choice empty pin", CF("{'pin':'','values':{'0':'twos'}}"), "\"pin\" must be a pin name"},
    {"choice unknown field", CF("{'field':'NOPE','values':{'0':'twos'}}"), "unknown register \"NOPE\""},
    {"choice without pin or field", CF("{'values':{'0':'twos'}}"), "a choice needs \"pin\" or \"field\""},
    {"choice without values", CF("{'pin':'CS'}"), "a choice needs a non-empty \"values\" object"},
    {"choice empty values", CF("{'pin':'CS','values':{}}"), "a choice needs a non-empty \"values\" object"},
    {"choice format value", CF("{'pin':'CS','values':{'0':'gray'}}"), "\"codeFormat\" values are straight, twos or offset"},
    {"choice format not a string", CF("{'pin':'CS','values':{'0':1}}"), "\"codeFormat\" values are"},
    {"codeFormat a number", CF("5"), "bad \"codeFormat\""},
    {"clearCode value", CLEAR("{'pin':'CS','values':{'0':-1}}"), "a \"clearCode\" value must be a whole number"},
    {"clearCode a string", CLEAR("'mid'"), "bad \"clearCode\""},
    {"clearCode a number too big", CLEAR("-1"), "\"clearCode\" must be a whole number"},
    {"busy not an object", ADC_CONV("{'time':0,'busy':5}"), "\"busy\" must be an object of field values"},
    {"busy unknown register", ADC_CONV("{'time':0,'busy':{'NOPE':1}}"), "unknown register \"NOPE\""},
    {"done names R", ADC_CONV("{'time':0,'done':{'R':1}}"), "\"done\" names fields"},
    {"onRead value", ADC_CONV("{'time':0,'onRead':{'CFG.A':-1}}"), "onRead must be a whole number"},
    {"time negative", ADC_CONV("{'time':-1}"), "\"time\" must be a time in ns"},
    {"time unknown key", ADC_CONV("{'time':{'field':'CFG.A','values':{'0':1},'bad':1}}"), "unknown key \"bad\""},
    {"time empty values", ADC_CONV("{'time':{'field':'CFG.A','values':{}}}"), "\"time\" is ns or {field(s), values}"},
    {"time no fields", ADC_CONV("{'time':{'fields':[],'values':{'0':1}}}"), "\"time\" is ns or {field(s), values}"},
    {"time five fields", ADC_CONV("{'time':{'fields':['CFG','CFG','CFG','CFG','CFG'],'values':{'0':1}}}"),
     "\"time\" is ns or {field(s), values}"},
    {"time fields item", ADC_CONV("{'time':{'fields':[5],'values':{'0':1}}}"), "\"fields\" are names"},
    {"time fields unknown", ADC_CONV("{'time':{'fields':['NOPE'],'values':{'0':1}}}"), "unknown register \"NOPE\""},
    {"time field unknown", ADC_CONV("{'time':{'field':'NOPE','values':{'0':1}}}"), "unknown register \"NOPE\""},
    {"time key count", ADC_CONV("{'time':{'fields':['CFG.A','CFG.A'],'values':{'1':5}}}"),
     "\"time\" key \"1\" needs 2 values"},
    {"time key not a number", ADC_CONV("{'time':{'field':'CFG.A','values':{'q':5}}}"), "\"time\" key \"q\" needs 1 values"},
    {"time key trailing", ADC_CONV("{'time':{'field':'CFG.A','values':{'1,2':5}}}"), "needs 1 values"},
    {"time value", ADC_CONV("{'time':{'field':'CFG.A','values':{'1':-5}}}"), "a time must be a time in ns"},
    {"time a string", ADC_CONV("{'time':'fast'}"), "\"time\" is ns or {field(s), values}"},
    {"conversion unknown key", ADC_CONV("{'time':0,'bad':1}"), "unknown key \"bad\""},
    {"start not an array", ADC_CONV("{'time':0,'start':'cs-fall'}"), "\"start\" must be an array"},
    {"start write unknown", ADC_CONV("{'time':0,'start':['write:NOPE']}"), "unknown register \"NOPE\""},
    {"start write R", ADC_CONV("{'time':0,'start':['write:R']}"), "\"write:REG\""},
    {"start write condition", ADC_CONV("{'time':0,'start':['write:CFG.A=zz']}"), "the value must be a number"},
    {"start unknown", ADC_CONV("{'time':0,'start':['sometimes']}"), "unknown start trigger \"sometimes\""},
    {"start not a string", ADC_CONV("{'time':0,'start':[5]}"), "unknown start trigger \"\""},
    {"mode unknown", ADC_CONV("{'time':0,'mode':'burst'}"), "\"mode\" is single, continuous or {field}"},
    {"mode field unknown", ADC_CONV("{'time':0,'mode':{'field':'NOPE','single':0}}"), "unknown register \"NOPE\""},
    {"mode continuous value", ADC_CONV("{'time':0,'mode':{'field':'CFG.A','continuous':-1}}"),
     "continuous must be a whole number"},
    {"mode single value", ADC_CONV("{'time':0,'mode':{'field':'CFG.A','single':-1}}"), "single must be a whole number"},
    {"mode without a value", ADC_CONV("{'time':0,'mode':{'field':'CFG.A'}}"), "\"mode\" needs \"continuous\" or \"single\""},
    {"mode a number", ADC_CONV("{'time':0,'mode':5}"), "\"mode\" is single, continuous or {field}"},
    {"channel unknown", ADC_CONV("{'time':0,'channel':'NOPE'}"), "unknown register \"NOPE\""},
    {"ready not an array", ADC_P("", ",'ready':5"), "\"ready\" must be an array"},
    {"ready without asserts", ADC_P("", ",'ready':[{'pin':'CS'}]"), "missing \"asserts\""},
    {"ready asserts", ADC_P("", ",'ready':[{'pin':'CS','asserts':'idle'}]"), "\"asserts\" is \"busy\" or \"done\""},
    {"ready clears time", ADC_P("", ",'ready':[{'pin':'CS','asserts':'done','clears':-1}]"), "\"clears\" must be a time"},
    {"ready clears unknown", ADC_P("", ",'ready':[{'pin':'CS','asserts':'done','clears':['never']}]"),
     "\"clears\" is read, start, cs-fall or ns"},
    {"pins unknown key", PINS("{'bad':1}"), "unknown key \"bad\""},
    {"pins empty pin", PINS("{'reset':{'pin':''}}"), "\"pin\" must be a pin name"},
    {"pins edge unknown", PINS("{'reset':{'pin':'CS','edge':'up'}}"), "\"edge\" is rise, fall or level"},
    {"pins edge not a string", PINS("{'reset':{'pin':'CS','edge':5}}"), "\"edge\" must be a string"},
    {"pins convst level", PINS("{'convst':{'pin':'CS','edge':'level'}}"), "\"convst\" needs an edge"},
    {"levels not an array", PINS("{'levels':5}"), "\"levels\" must be an array"},
    {"levels unknown field", PINS("{'levels':[{'pin':'CS','field':'NOPE'}]}"), "unknown register \"NOPE\""},
    {"levels R", PINS("{'levels':[{'pin':'CS','field':'R'}]}"), "\"levels\": a field"},
    {"resetSelect not an array", PINS("{'resetSelect':5}"), "\"resetSelect\" must be an array"},
    {"resetSelect without resets", PINS("{'resetSelect':[{'pin':'CS'}]}"), "\"resetSelect\" entries"},
    {"resetSelect empty pin", PINS("{'resetSelect':[{'pin':'','resets':{}}]}"), "\"pin\" must be a pin name"},
    {"resetSelect unknown register", PINS("{'resetSelect':[{'pin':'CS','resets':{'NOPE':1}}]}"),
     "unknown register \"NOPE\""},
    {"resetSelect R", PINS("{'resetSelect':[{'pin':'CS','resets':{'R':1}}]}"), "\"resets\" names registers"},
    {"dac input not an array", DAC_D("{'input':5,'output':['OUT'],'ldac':'sync'}", ""),
     "\"input\" must be an array of register names"},
    {"dac input unknown", DAC_D("{'input':['NOPE'],'output':['OUT'],'ldac':'sync'}", ""), "unknown register \"NOPE\""},
    {"dac output R", DAC_D("{'input':['IN'],'output':['R'],'ldac':'sync'}", ""), "\"output\" names registers"},
    {"dac without ldac", DAC_D("{'input':['IN'],'output':['OUT']}", ""), "missing \"ldac\""},
    {"dac channel count", DAC_D("{'input':['IN','OUT'],'output':['OUT'],'ldac':'sync'}", ""),
     "\"input\" and \"output\" need one register per channel (1)"},
    {"dac ldac unknown", DAC_D("{'input':['IN'],'output':['OUT'],'ldac':'never'}", ""), "\"ldac\" is pin, command or sync"},
    {"DAC without dac", DEV("'type':'dac','bits':8,'channels':1,'vcc':[2.7,5.5]," SPIBUS ",'protocol':{'frame':'stream'}", ""),
     "a DAC needs \"dac\""},
};

#define FIELDS(X) ADC_R("{'name':'CFG','width':8,'fields':" X "}")
#define PROTO(X) DEV(ADC12 "," SPIBUS "," REGS1 ",'protocol':" X "," CONV0, "")
#define PB(X) ADC_BUS("{'style':'parallel','data':['SCLK','SDO','CS','DIN']" X "}")
#define CB(X) ADC_BUS("{'style':'clocked','clock':{'pin':'SCLK'},'data':['SDO']" X "}")
#define I2CB(X) ADC_BUS("{'style':'i2c','scl':'SCLK','sda':'SDO'" X "}")

/* models/converter_load_proto.c: registers, commands, protocol, buses. */
static const Case PROTOCOL[] = {
    {"fields not an array", FIELDS("5"), "\"fields\" must be an array"},
    {"field without a name", FIELDS("[{'bits':[1,0]}]"), "missing \"name\""},
    {"field bits", FIELDS("[{'name':'A','bits':[1]}]"), "field \"A\": \"bits\" is [hi, lo]"},
    {"field negative bit", FIELDS("[{'name':'A','bits':[1,-1]}]"), "field \"A\" is outside the register's 8 bits"},
    {"field reversed", FIELDS("[{'name':'A','bits':[0,1]}]"), "field \"A\" is outside the register's 8 bits"},
    {"duplicate field", FIELDS("[{'name':'A','bits':[0,0]},{'name':'A','bits':[1,1]}]"), "duplicate field \"A\""},
    {"registers not an array", DEV(ADC12 "," SPIBUS ",'registers':5,'protocol':{'frame':'stream'}," CONV0, ""),
     "\"registers\" must be an array"},
    {"register without width", ADC_R("{'name':'R1'}"), "missing \"width\""},
    {"register width 0", ADC_R("{'name':'R1','width':0}"), "\"width\" must be 1-32"},
    {"register address", ADC_R("{'name':'R1','width':8,'address':-1}"), "\"address\" must be a whole number"},
    {"register space", ADC_R("{'name':'R1','width':8,'space':5}"), "\"space\" must be a string"},
    {"register writeOneClears", ADC_R("{'name':'R1','width':8,'writeOneClears':-1}"), "writeOneClears must be"},
    {"register readOnly", ADC_R("{'name':'R1','width':8,'readOnly':-1}"), "\"readOnly\" must be a whole number"},
    {"register reset too wide", ADC_R("{'name':'R1','width':8,'reset':256}"), "a value is wider than the register's 8 bits"},
    {"register readOnly too wide", ADC_R("{'name':'R1','width':4,'readOnly':16}"), "a value is wider"},
    {"register result field", ADC_R("{'name':'R1','width':8,'result':'NOPE'}"), "\"result\": no field \"NOPE\""},
    {"register result a number", ADC_R("{'name':'R1','width':8,'result':5}"), "\"result\" is true or a field name"},
    {"register resultBits", ADC_R("{'name':'R1','width':8,'resultBits':[1]}"), "\"resultBits\" is [hi, lo]"},
    {"register resultBits not an array", ADC_R("{'name':'R1','width':8,'resultBits':5}"), "\"resultBits\" is [hi, lo]"},
    {"register resultBits reversed", ADC_R("{'name':'R1','width':8,'resultBits':[0,3]}"), "hi >= lo"},
    {"register resultBits past 31", ADC_R("{'name':'R1','width':8,'resultBits':[32,0]}"), "hi >= lo"},
    {"register channel", ADC_R("{'name':'R1','width':8,'channel':-1}"), "\"channel\" must be a whole number"},
    {"duplicate register", ADC_R("{'name':'R1','width':8},{'name':'R1','width':8}"), "duplicate register \"R1\""},
    {"command in empty", ADC_C("{'in':' '}"), "\"in\" is empty"},
    {"command action not a string", ADC_C("{'in':'01','action':5}"), "\"action\" must be a string"},
    {"command space not a string", ADC_C("{'in':'01','space':5}"), "\"space\" must be a string"},
    {"command space unknown", ADC_C("{'in':'01','space':'dac'}"), "no register has space \"dac\""},
    {"command register a number", ADC_C("{'in':'01','register':5}"), "\"register\" names a register"},
    {"command register R", ADC_C("{'in':'01','register':'R'}"), "\"register\" names a register"},
    {"command register unknown", ADC_C("{'in':'01','register':'NOPE'}"), "unknown register \"NOPE\""},
    {"command count", ADC_C("{'in':'01','count':'xx'}"), "\"count\": a capture letter"},
    {"command count not captured", ADC_C("{'in':'01','count':'b'}"), "\"count\" 'b' is not a capture of \"in\""},
    {"command countOffset", ADC_C("{'in':'01','countOffset':'x'}"), "\"countOffset\" must be a number"},
    {"command data", ADC_C("{'in':'01','data':'xx'}"), "\"data\": a capture letter"},
    {"command channel", ADC_C("{'in':'01','channel':'xx'}"), "\"channel\": a capture letter"},
    {"command increment", ADC_C("{'in':'01','increment':'xx'}"), "\"increment\": a capture letter"},
    {"command increment not captured", ADC_C("{'in':'01','increment':'q'}"), "\"increment\" 'q' is not a capture"},
    {"channelMap not an object", ADC_C("{'in':'01','channelMap':5}"), "\"channelMap\" must be an object"},
    {"channelMap value", ADC_C("{'in':'01','channelMap':{'0':5}}"), "\"channelMap\" values are arrays of channels"},
    {"channelMap channel", ADC_C("{'in':'01','channelMap':{'0':[300]}}"), "\"channelMap\": a channel is 0-255"},
    {"channelMap channel a string", ADC_C("{'in':'01','channelMap':{'0':['a']}}"), "a channel is 0-255"},
    {"set not an object", ADC_C("{'in':'01','set':5}"), "\"set\" must be an object"},
    {"set unknown", ADC_C("{'in':'01','set':{'NOPE':1}}"), "unknown register \"NOPE\""},
    {"set R", ADC_C("{'in':'01','set':{'R':1}}"), "\"set\" names fields"},
    {"set not captured", ADC_C("{'in':'01','set':{'CFG.A':'b'}}"), "\"set\" 'b' is not a capture of"},
    {"next pattern", ADC_C("{'in':'01','next':'0x'}"), "'x' in an output pattern"},
    {"commit", ADC_C("{'in':'01','commit':'later'}"), "\"commit\" is \"now\" or \"end\""},
    {"then not an array", ADC_C("{'in':'01','then':5}"), "\"then\" must be an array of actions"},
    {"then unknown", ADC_C("{'in':'01','then':['fly']}"), "unknown action \"fly\""},
    {"then not a string", ADC_C("{'in':'01','then':[5]}"), "\"then\": action names"},
    {"wordBits", ADC_C("{'in':'01','wordBits':-1}"), "\"wordBits\" must be a whole number"},
    {"wreg without an address", ADC_C("{'in':'0101','action':'wreg'}"), "action \"wreg\" needs \"address\" or \"register\""},
    {"setbits without data", ADC_C("{'in':'0101','action':'setbits','register':'CFG'}"), "action \"setbits\" needs \"data\""},
    {"write without data on an ADC", ADC_C("{'in':'0101','action':'write'}"), "action \"write\" needs \"data\""},
    {"no register at the address", ADC_C("{'in':'0101','action':'wreg','address':7}"), "no register at address 7"},
    {"read key", ADC_P(",'read':{'foo':'0'}", ""), "\"read\" keys are addresses or \"high\"/\"low\""},
    {"read value not a string", ADC_P(",'read':{'0':5}", ""), "\"read\" values are patterns"},
    {"read value pattern", ADC_P(",'read':{'0':'0x'}", ""), "'x' in an output pattern"},
    {"read a number", ADC_P(",'read':5", ""), "\"read\" is a pattern, \"pointer\" or an address map"},
    {"read pattern", ADC_P(",'read':'0x'", ""), "'x' in an output pattern"},
    {"protocol missing", DEV(ADC12 "," SPIBUS "," REGS1 "," CONV0, ""), "missing \"protocol\""},
    {"protocol unknown key", PROTO("{'frame':'stream','bad':1}"), "unknown key \"bad\""},
    {"frame", PROTO("{'frame':'burst'}"), "\"frame\" is \"fixed\" or \"stream\""},
    {"frame missing", PROTO("{}"), "\"frame\" is \"fixed\" or \"stream\""},
    {"align", ADC_P(",'align':'middle'", ""), "\"align\" is \"start\" or \"end\""},
    {"out pattern", ADC_P(",'out':'0x'", ""), "'x' in an output pattern"},
    {"idle a number", ADC_P(",'idle':5", ""), "\"idle\" is \"0\", \"1\", \"z\" or \"repeat\""},
    {"idle unknown", ADC_P(",'idle':'q'", ""), "\"idle\" is \"0\", \"1\", \"z\" or \"repeat\""},
    {"generalCall", ADC_P(",'generalCall':'wake'", ""), "\"generalCall\" is \"reset\""},
    {"commands not an array", ADC_P(",'commands':5", ""), "\"commands\" must be an array"},
    {"outs not an array", ADC_P(",'outs':5", ""), "\"outs\" must be an array"},
    {"outs unknown key", ADC_P(",'outs':[{'bad':1}]", ""), "unknown key \"bad\""},
    {"outs pattern", ADC_P(",'outs':[{'out':'0x'}]", ""), "'x' in an output pattern"},
    {"reads read", ADC_P(",'reads':[{'read':5}]", ""), "\"read\" is a pattern"},
    {"reads without read", ADC_P(",'reads':[{'when':'CFG.A=1'}]", ""), "missing \"read\""},
    {"triggers not an array", ADC_P(",'triggers':5", ""), "\"triggers\" must be an array"},
    {"triggers action", ADC_P(",'triggers':[{'write':'CFG.A=1','action':'fly'}]", ""), "unknown action \"fly\""},
    {"parallel cs", PB(",'cs':{'pin':''}"), "\"pin\" must be a pin name"},
    {"parallel data not an array", ADC_BUS("{'style':'parallel','data':5}"), "\"data\" is an array of at most 24 pin names"},
    {"parallel data item", ADC_BUS("{'style':'parallel','data':[5]}"), "\"data\" is an array of pin names"},
    {"parallel ale", PB(",'ale':{'pin':''}"), "\"pin\" must be a pin name"},
    {"parallel ale edge", PB(",'ale':{'pin':'GND','edge':5}"), "\"edge\" must be a string"},
    {"i2c address", I2CB(",'address':5,'table':[1]"), "\"address\" is an array of at most"},
    {"i2c table count", I2CB(",'address':[],'table':[1,2]"), "\"table\" needs 1 entries (4 ^ address pins)"},
    {"i2c table entry", I2CB(",'address':[],'table':[200]"), "\"table\" entries are 7-bit addresses or null"},
    {"bus unknown key", ADC_BUS("{'style':'spi','sclk':'SCLK','bad':1}"), "unknown key \"bad\""},
    {"bus none on a converter", ADC_BUS("{'style':'none'}"), "\"none\" is a bus style of a display driver only"},
    {"spi mode", ADC_BUS("{'style':'spi','sclk':'SCLK','mode':4}"), "\"mode\" is 0-3"},
    {"i2c-spi without spi", ADC_BUS("{'style':'i2c-spi','selectPin':'CS','i2c':{}}"), "i2c-spi needs spi, i2c, selectPin"},
    {"i2c-spi i2c table", ADC_BUS("{'style':'i2c-spi','selectPin':'CS','spi':{'sclk':'SCLK'},"
                                  "'i2c':{'scl':'SCLK','sda':'SDO','table':[300]}}"), "\"table\" entries are 7-bit"},
    {"clocked oe", CB(",'oe':{'pin':''}"), "\"pin\" must be a pin name"},
    {"clocked no data", ADC_BUS("{'style':'clocked','clock':{'pin':'SCLK'},'data':[]}"), "\"data\" needs 1-24 pins"},
    {"clocked clock edge", ADC_BUS("{'style':'clocked','clock':{'pin':'SCLK','edge':5},'data':['SDO']}"),
     "\"edge\" must be a string"},
    {"clocked clockOut", CB(",'clockOut':[{'pin':''}]"), "\"pin\" must be a pin name"},
    {"bus style unknown", ADC_BUS("{'style':'can'}"), "unknown bus style \"can\""},
};

int main(void) {
    size_t i;

    for (i = 0; i < sizeof LOAD / sizeof LOAD[0]; i++) {
        expect_fail(LOAD[i].name, LOAD[i].json, LOAD[i].expect);
    }
    for (i = 0; i < sizeof CONV / sizeof CONV[0]; i++) {
        expect_fail(CONV[i].name, CONV[i].json, CONV[i].expect);
    }
    for (i = 0; i < sizeof PROTOCOL / sizeof PROTOCOL[0]; i++) {
        expect_fail(PROTOCOL[i].name, PROTOCOL[i].json, PROTOCOL[i].expect);
    }
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("converter_load_cov_test: all passed\n");
    return 0;
}
