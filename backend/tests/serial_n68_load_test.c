/*
 * serial_n68_load_test (plan-68-N): the ns16550 loader's rejections — the
 * core keys (models/ns16550_load.c, ns16550_kind.c), the printer port, and
 * the Exar keys (ns16550_exar_load.c, ns_x_bind, ns_x_load_part). Each case
 * is one edit of a shipped fixture device (tests/fixtures/serial/ns16550.json)
 * and the message the loader must give for it (support/serial_bench_n68.c).
 * Edits and messages are written with ' for " (swapped before use).
 */

#include "serial_bench_n68.h"

static void swap_quotes(char *text) {
    for (; *text != '\0'; text++) {
        if (*text == '\'') {
            *text = '"';
        }
    }
}

/* One rejection: base device, edits and wanted message, ' for ". */
static void rj(const char *base, const char *edits, const char *want) {
    char e[600];
    char w[300];

    snprintf(e, sizeof e, "%s", edits);
    snprintf(w, sizeof w, "%s", want);
    swap_quotes(e);
    swap_quotes(w);
    n68_reject(base, e, w);
}

#define PC "SFX-PC16550D"

/* ---- The bus, clock and channels (nsc_load_bus, nsc_load_clock, nsc_load_channels) ---- */

static void case_bus(void) {
    case_begin();
    rj(PC, "{'bus':'-'}", "missing 'bus'");
    rj(PC, "{'bus.style':5}", "'style' must be a string");
    rj(PC, "{'bus.width':8}", "unknown key 'width'");
    rj(PC, "{'bus.style':'motorola'}", "unknown bus style 'motorola'");
    rj(PC, "{'bus.data':'-'}", "missing 'data'");
    rj(PC, "{'bus.data':5}", "'data' must be an array of strings");
    rj(PC, "{'bus.data':['D0','D1']}", "'data' needs exactly 8 pin names");
    rj(PC, "{'bus.select':['CS2','A0','A1','A2','D0']}", "'select' has at most 4 pin names");
    rj(PC, "{'bus.read':5}", "'read' must be a string");
    rj(PC, "{'bus.address':['A0','A1']}", "'address' needs 3 pin names (8 for '5g')");
    rj(PC, "{'bus.read':'-','bus.readHigh':'-'}", "a bus needs 'read' or 'readHigh'");
    rj(PC, "{'bus.write':'-','bus.writeHigh':'-'}", "a bus needs 'write' or 'writeHigh'");
    rj(PC, "{'bus.resetActiveHigh':3}", "'resetActiveHigh' must be true or false");
    rj(PC, "{'bus.iowOnly':{'modePin':'MR'}}", "missing 'rw'");
    rj(PC, "{'clock':'-'}", "missing 'clock'");
    rj(PC, "{'clock.speed':1}", "unknown key 'speed'");
    rj(PC, "{'clock.single':'XIN'}", "'single' replaces 'in' and 'receive'");
    rj(PC, "{'clock.in':'-'}", "missing 'in'");
    rj(PC, "{'clock.baudOutShape':5}", "'baudOutShape' must be a string");
    rj(PC, "{'clock.baudOutShape':'square'}", "'baudOutShape' must be 'half' or 'low2'");
    rj(PC, "{'clock.out':5}", "'out' must be a string");
    rj(PC, "{'channels':'-'}", "missing 'channels'");
    rj(PC, "{'channels':[]}", "'channels' must be an array of 1 to");
    rj(PC, "{'channels.0.speed':1}", "unknown key 'speed'");
    rj(PC, "{'channels.0.rts':5}", "'rts' must be a string");
    rj(PC, "{'channels.0.tx':'-'}", "a channel needs 'tx' and 'rx'");
    rj(PC, "{'channels.0.op':5}", "'op' must be a string");
    rj(PC, "{'channels.0.bclk':5}", "'bclk' must be a string");
    rj(PC, "{'channels.0.select':5}", "'select' must be an array of strings");
    rj("SFX-TL16C554", "{'channels.1.select':'-'}", "with per-channel selects every channel needs one");
    case_end("N1 bus, clock and channel keys: every rejection");
}

/* ---- The device keys (ns_load_device) ------------------------------------------------------- */

static void case_device(void) {
    case_begin();
    rj(PC, "{'fifo':'-'}", "missing 'fifo'");
    rj(PC, "{'rxTrigger':[1,2]}", "'rxTrigger' must be 4 numbers");
    rj(PC, "{'rxTrigger':[1,4,8,99]}", "'rxTrigger' values must be whole numbers from 1 to the FIFO size");
    rj(PC, "{'rxTrigger64':[1,4,8,14]}", "'rxTrigger64' needs 'fifo': '750'");
    rj(PC, "{'tlr':300}", "'tlr' must be a whole number from 0 to 255");
    rj(PC, "{'tlr':5}", "'tlr' must be 4 or 8");
    rj(PC, "{'intPin':5}", "'intPin' must be a string");
    rj(PC, "{'intPin':'pulsed'}", "unknown 'intPin' 'pulsed'");
    rj(PC, "{'intControl':5}", "'intControl' must be a string");
    rj(PC, "{'intControl':'INTR'}", "'intControl' names the INTN/INTSEL pin");
    rj(PC, "{'dmaClass':'Q'}", "unknown 'dmaClass' 'Q'");
    rj(PC, "{'dmaPins':'Q'}", "unknown 'dmaPins' 'Q'");
    rj(PC, "{'dma':{'ack':'A0'}}", "unknown key 'ack'");
    rj(PC, "{'dmaPins':'sharedAnd'}", "shared 'dmaPins' and the 'dma' pins go together");
    rj(PC, "{'broadcastWrite':'Q'}", "unknown 'broadcastWrite' 'Q'");
    rj("SFX-TL16C2552", "{'channelSelect.pin2':1}", "unknown key 'pin2'");
    rj("SFX-TL16C2552", "{'channels.1':'-'}", "'channelSelect' is for 2 channels sharing one chip select");
    rj("SFX-TL16C554", "{'channels.0.select':'-','channels.1.select':'-','channels.2.select':'-',"
                       "'channels.3.select':'-'}",
       "more than one channel needs per-channel selects or 'channelSelect'");
    rj(PC, "{'lcrReset':300}", "'lcrReset' must be a whole number from 0 to 255");
    rj(PC, "{'sprReset':300}", "'sprReset' must be a whole number from 0 to 255");
    rj(PC, "{'timeout':'Q'}", "unknown 'timeout' 'Q'");
    rj(PC, "{'iirPriority':'Q'}", "unknown 'iirPriority' 'Q'");
    rj(PC, "{'lsr7Clear':'Q'}", "unknown 'lsr7Clear' 'Q'");
    rj(PC, "{'stopCheck':'Q'}", "unknown 'stopCheck' 'Q'");
    rj(PC, "{'statusReadDefer':3}", "'statusReadDefer' must be true or false");
    rj(PC, "{'gpio':{}}", "'gpio' needs an I2C/SPI 'bus'");
    rj(PC, "{'extended':'sc16is'}", "the bridge 'extended' styles need an I2C/SPI 'bus'");
    rj("SFX-SC16IS750", "{'bus.spi.mode':1}", "SPI port is mode 0");
    case_end("N2 device keys: every rejection");
}

/* ---- The plan-12-D Phase 2 keys (nsc_load_phase2) and the printer port --------------------- */

static void case_phase2(void) {
    case_begin();
    rj(PC, "{'extended':'what'}", "unknown 'extended' 'what'");
    rj(PC, "{'afr':'what'}", "unknown 'afr' 'what'");
    rj(PC, "{'afrReset':999}", "'afrReset' must be a whole number from 0 to 255");
    rj(PC, "{'dlf':3}", "'dlf' must be true or false");
    rj(PC, "{'rtsTable':5}", "'rtsTable' must be 4 [level, level] pairs");
    rj("SFX-NS16C2552", "{'rtsTable.0':[1,200]}", "'rtsTable' must be 4 [level, level] pairs of 0 to 128");
    rj(PC, "{'autoFlow':'efrTable'}", "'rtsTable' goes with 'autoFlow': 'efrTable'");
    rj(PC, "{'swFlow':'what'}", "unknown 'swFlow' 'what'");
    rj(PC, "{'specialChar':3}", "'specialChar' must be true or false");
    rj(PC, "{'ctsRtsClear':'what'}", "unknown 'ctsRtsClear' 'what'");
    rj(PC, "{'sleep':'what'}", "unknown 'sleep' 'what'");
    rj(PC, "{'irda':'what'}", "unknown 'irda' 'what'");
    rj(PC, "{'rs485':'what'}", "unknown 'rs485' 'what'");
    rj(PC, "{'prescaler':'what'}", "unknown 'prescaler' 'what'");
    rj(PC, "{'prescaler':'mcr7-clksel'}", "'mcr7-clksel' and 'clksel' need 'clock'.'select'");
    rj(PC, "{'lsrErrors':'what'}", "unknown 'lsrErrors' 'what'");
    rj(PC, "{'mcrReset':999}", "'mcrReset' must be a whole number from 0 to 255");
    rj(PC, "{'xonReset':[1]}", "'xonReset' must be 4 bytes (Xon1, Xon2, Xoff1, Xoff2)");
    rj(PC, "{'xonReset':[1,2,3,300]}", "'xonReset' must be 4 bytes (Xon1, Xon2, Xoff1, Xoff2)");
    rj(PC, "{'idRegs':{'rev':1}}", "unknown key 'rev'");
    rj(PC, "{'timeoutDelay':999}", "'timeoutDelay' must be a whole number from 0 to 255");
    rj(PC, "{'divisorZero':999}", "'divisorZero' must be a whole number from 0 to 255");
    rj(PC, "{'tristate':5}", "'tristate' must be a string");
    rj(PC, "{'absentTies':5}", "'absentTies' must be an object of pin: 0 or 1");
    rj(PC, "{'absentTies':{'RI':2}}", "'absentTies' must be an object of pin: 0 or 1");
    rj(PC, "{'optionalPins':5}", "'optionalPins' must be an array of strings");
    rj(PC, "{'sprPowerUp':999}", "'sprPowerUp' must be a whole number from 0 to 255");
    rj(PC, "{'dmaMode1':3}", "'dmaMode1' must be true or false");
    rj("SFX-TL16C451", "{'printerPort.speed':1}", "unknown key 'speed'");
    rj("SFX-TL16C451", "{'printerPort.style':'-'}", "missing 'style'");
    rj("SFX-TL16C451", "{'printerPort.select':'-'}", "missing 'select'");
    rj("SFX-TL16C451", "{'printerPort.inputEnable':'-'}",
       "'inputEnable' is for ti451; 'pemd' and 'enirq' for ti552");
    rj("SFX-TL16C452", "{'channels.0.select':'-','channels.1.select':'-',"
                       "'channelSelect':{'pin':'CS0','aIsHigh':true}}",
       "a printer port needs per-channel or common selects");
    case_end("N3 Phase 2 keys and the printer port: every rejection");
}

/* ---- Binding: every pin name must be on the pinout (ns_bind, ns_x_bind) -------------------- */

static void case_bind(void) {
    case_begin();
    rj(PC, "{'bus.data.0':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'bus.address.0':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'bus.select.0':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'bus.selectHigh.0':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'bus.read':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'channels.0.tx':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-TL16C554", "{'channels.0.select.0':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-TL16C554", "{'channels.0.select':'-','channels.0.selectHigh':['NOPE']}",
       "no pin named 'NOPE'");
    rj("SFX-TL16C554", "{'dma.txrdy':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'channels.0.op':'NOPE'}", "no pin named 'NOPE'");
    rj(PC, "{'channels.0.mf':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-TL16C451", "{'printerPort.data.0':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-TL16C451", "{'printerPort.ack':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-SC16IS750", "{'bus.spi.miso':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-XR16L2551", "{'powerSave.pin':'NOPE'}", "no pin named 'NOPE'");
    rj("SFX-XR16L2551", "{'parts.0.intPin':'intn'}", "part 'intPin' must be 'driven' or 'out2-gated'");
    case_end("N4 binding: unknown pin names and the part key");
}

/* ---- The Exar keys (ns_x_load_bus, ns_x_load) ------------------------------------------------ */

static void case_exar(void) {
    case_begin();
    rj("SFX-XR16L2551", "{'bus.modePin':'-'}", "missing 'modePin'");
    rj("SFX-XR16L2551", "{'bus.irqOthers':'high'}", "'irqOthers' must be 'low' or 'z'");
    rj("SFX-XR16L2551", "{'bus.channelBits':5}", "'channelBits' must be an array of strings");
    rj("SFX-XR16L2551", "{'bus.channelBits':['CSB','A0','A1']}",
       "'channelBits' has at most 2 pin names (A3, A4)");
    rj("SFX-XR16C864", "{'triggerTables.rx':5}", "'triggerTables'.'rx' must be 3 tables of 4 levels and 'trg'");
    rj("SFX-XR16C864", "{'triggerTables.rx.3':'x'}", "table D of 'rx' must be 'trg'");
    rj("SFX-XR16C864", "{'triggerTables.rx.0':[1,2]}", "'triggerTables'.'rx' tables are 4 levels");
    rj("SFX-XR16C864", "{'triggerTables.rx.0':[1,2,3,999]}",
       "trigger levels are whole numbers from 1 to the FIFO size");
    rj("SFX-XR16C864", "{'triggerTables.mid':1}", "unknown key 'mid'");
    rj("SFX-XR16C864", "{'triggerTables':'-'}", "'triggerTables' goes with 'exar854' and '5g' (and only they)");
    rj("SFX-XR16L2552", "{'mfPins':5}", "'mfPins' must be the 4 functions of AFR[2:1] = 00..11");
    rj("SFX-XR16L2552", "{'mfPins.0':5}", "'mfPins' entries are 'op2', 'baudout', 'rxrdy' or 'x'");
    rj("SFX-XR16L2552", "{'mfPins.0':'clock'}", "'mfPins' entries are 'op2', 'baudout', 'rxrdy' or 'x'");
    rj(PC, "{'bus.address':['A0','A1','A2','D0','D1','D2','D3','D4']}",
       "'bus'.'address' has 8 pin names (A0-A7) with '5g', else 3");
    rj(PC, "{'extended':'afe','isrStatus':'high'}", "Exar keys go with 'extended': absent");
    rj("SFX-XR16C864", "{'fifoCounters':5}", "'fifoCounters' must be a string");
    rj("SFX-XR16C864", "{'fifoCounters':'flvl'}", "unknown 'fifoCounters' 'flvl'");
    rj("SFX-XR16L2551", "{'fifoCounters':'fcFlvl'}", "'fifoCounters': 'fcFlvl' is the exar854 FC/FLVL pair");
    rj("SFX-XR16L2551", "{'dld':3}", "'dld' must be true or false");
    rj("SFX-XR16C864", "{'dld':true}", "'dld' goes with 'exar650', 'm752' or '5g'");
    rj("SFX-XR16L2551", "{'sampling':5}", "'sampling' must be a string");
    rj("SFX-XR16L2551", "{'sampling':'8xmode'}", "'sampling': '8xmode' goes with '5g' (and only it)");
    rj("SFX-XR16L2551", "{'xonAny':5}", "'xonAny' must be a string");
    rj("SFX-XR16L2551", "{'powerSave.level':1}", "unknown key 'level'");
    rj("SFX-XR16L2551", "{'startSample':7}", "'startSample' must be 7.5 or 8");
    rj("SFX-XR16L2551", "{'thrIntClear':5}", "'thrIntClear' must be a string");
    rj("SFX-XR16L2551", "{'isrStatus':5}", "'isrStatus' must be a string");
    rj("SFX-XR16L2551", "{'isrStatus':'high'}", "'isrStatus' goes with 'ier5'");
    rj("SFX-XR16L2551", "{'resetOut':5}", "'resetOut' must be a string");
    rj("SFX-XR16L2551", "{'divisorPowerUp':999}", "'divisorPowerUp' must be a whole number from 0 to 255");
    rj("SFX-XR16L2551", "{'wakeIndicator':3}", "'wakeIndicator' must be true or false");
    rj("SFX-XR16L2551", "{'fstat':5}", "'fstat' must be a string");
    rj("SFX-XR16C864", "{'dmaHost.ack':1}", "unknown key 'ack'");
    rj("SFX-XR16C864", "{'dmaHost.dack':['DACKA']}", "'dack' names one DACK pin per channel");
    rj("SFX-XR16C864", "{'dmaClass':'N'}", "'dmaHost' goes with 'dmaClass': 'X'");
    rj("SFX-XR16V798", "{'timer.rate':1}", "unknown key 'rate'");
    rj("SFX-XR16V798", "{'timer.style':'-'}", "missing 'style'");
    rj("SFX-XR16V798", "{'variant5g':5}", "'variant5g' must be a string");
    rj("SFX-XR16V798", "{'interrupt':'-'}", "'5g' needs 'variant5g', 'interrupt' and 'timer'");
    rj("SFX-XR16V798", "{'irdaStrap':5}", "'irdaStrap' must be a string");
    rj("SFX-XR16L2551", "{'irda':'dld7'}", "'dld7' / 'dld6' go with 'm752' and 'dld'");
    rj("SFX-XR16C864", "{'afr':'exar'}", "'afr': 'exar' goes with 'exar650'");
    rj("SFX-XR16L2551", "{'broadcastWrite':'afr0-perChannel'}", "'afr0-perChannel' needs 'afr': 'exar'");
    case_end("N5 Exar keys: every rejection");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_bus();
    case_device();
    case_phase2();
    case_bind();
    case_exar();
    return serial_bench_finish();
}
