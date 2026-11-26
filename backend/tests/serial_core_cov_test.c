/*
 * serial_core_cov_test (plan-68-P): the shared serial code every serial model
 * runs through, case by case where no model program reaches it:
 *
 *   C1  the serial kind's loader (serial.c): device, grade, usbSupply and part
 *       rejections, one per message, each on the smallest change to a device
 *       that loads (the fixture 8251A, AY-5-1013A, and the shipped XR21B1411).
 *   C2  the loader helpers of serial_util.h (pin-name lists and objects, and
 *       binding them), reached through the 8251A's "bus" and "serial".
 *   C3  the line engine (serial_line.h) driven directly: unknown data and
 *       parity bits, an unknown format, unknown samples on the receiver, the
 *       82C50A two-stop check, the terminal sender's parity bit and reset.
 *   C4  API misuse: out-of-range channels (serial.h), serial_bus_has_pin with
 *       no pin, su_mux with an X selector, su_drive_weak with no pin.
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"
#include "serial_bus.h"
#include "serial_line.h"
#include "serial_util.h"

/* ---- C1/C2 rejections ---------------------------------------------------------------- */

static cJSON *classic_doc;
static cJSON *shipped_doc;
static unsigned case_number;

static cJSON *device_of(const cJSON *doc, const char *id) {
    const cJSON *devices = cJSON_GetObjectItemCaseSensitive(doc, "devices");
    const cJSON *d;

    cJSON_ArrayForEach(d, devices) {
        const cJSON *v = cJSON_GetObjectItemCaseSensitive(d, "id");

        if (cJSON_IsString(v) && strcmp(v->valuestring, id) == 0) {
            return cJSON_Duplicate(d, 1);
        }
    }
    printf("FAIL no device %s\n", id);
    failures++;
    return cJSON_CreateObject();
}

/* Sets (or with value NULL deletes) the member at a dotted path ("a.b.0.c";
   a number indexes an array). With add = 1 the value is added even when the
   key exists (a duplicate key). */
static void edit(cJSON *dev, const char *path, const char *value, int add) {
    char buf[128];
    char *seg;
    char *next;
    cJSON *at = dev;

    snprintf(buf, sizeof buf, "%s", path);
    seg = buf;
    while ((next = strchr(seg, '.')) != NULL) {
        *next = '\0';
        at = cJSON_IsArray(at) ? cJSON_GetArrayItem(at, atoi(seg))
                               : cJSON_GetObjectItemCaseSensitive(at, seg);
        if (at == NULL) {
            printf("FAIL edit path %s\n", path);
            failures++;
            return;
        }
        seg = next + 1;
    }
    if (value == NULL) {
        if (cJSON_IsArray(at)) {
            cJSON_DeleteItemFromArray(at, atoi(seg));
        } else {
            cJSON_DeleteItemFromObjectCaseSensitive(at, seg);
        }
        return;
    }
    if (cJSON_IsArray(at)) {
        cJSON_ReplaceItemInArray(at, atoi(seg), cJSON_Parse(value));
    } else if (add || cJSON_GetObjectItemCaseSensitive(at, seg) == NULL) {
        cJSON_AddItemToObject(at, seg, cJSON_Parse(value));
    } else {
        cJSON_ReplaceItemInObjectCaseSensitive(at, seg, cJSON_Parse(value));
    }
}

/* Loads one device (renamed so it never collides with the bench's own);
   returns load_parts' result and the message. */
static int load_device(cJSON *dev, char **error) {
    char id[32];
    cJSON *doc = cJSON_CreateObject();
    cJSON *devices = cJSON_AddArrayToObject(doc, "devices");
    cJSON *parts = cJSON_GetObjectItemCaseSensitive(dev, "parts");
    cJSON *p;
    unsigned k = 0;
    char *text;
    int result;

    case_number++;
    snprintf(id, sizeof id, "CV-%u", case_number);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(id));
    cJSON_ArrayForEach(p, parts) {
        char name[48];

        snprintf(name, sizeof name, "CV-%u-%u", case_number, k++);
        if (cJSON_GetObjectItemCaseSensitive(p, "name") != NULL) {
            cJSON_ReplaceItemInObjectCaseSensitive(p, "name", cJSON_CreateString(name));
        }
    }
    cJSON_AddStringToObject(doc, "format", "schematical-parts");
    cJSON_AddNumberToObject(doc, "version", 1);
    cJSON_AddStringToObject(doc, "kind", "serial");
    cJSON_AddItemToArray(devices, dev);
    text = cJSON_PrintUnformatted(doc);
    cJSON_Delete(doc);
    *error = NULL;
    result = load_parts(library, text, "core_cov.json", error);
    cJSON_free(text);
    return result;
}

static void expect_loads(cJSON *dev, const char *what) {
    char *error;
    int result = load_device(dev, &error);
    char message[256];

    snprintf(message, sizeof message, "%s loads (%s)", what, error != NULL ? error : "no error");
    check(result == 0, message);
    free(error);
}

static void reject(cJSON *dev, const char *want, const char *what) {
    size_t before = part_library_count(library);
    char *error;
    int result = load_device(dev, &error);
    char message[400];

    snprintf(message, sizeof message, "%s: rejected with \"%s\" (got %d \"%s\")", what, want, result,
             error != NULL ? error : "(none)");
    check(result == -1 && error != NULL && strstr(error, want) != NULL &&
          part_library_count(library) == before, message);
    free(error);
}

/* One rejection: device `id` of `doc` with `path` set to `value` (NULL deletes). */
static void reject_edit(const cJSON *doc, const char *id, const char *path, const char *value,
                        const char *want, const char *what) {
    cJSON *dev = device_of(doc, id);

    edit(dev, path, value, 0);
    reject(dev, want, what);
}

static void case_c1_loader(void) {
    cJSON *dev;

    case_begin();
    /* The bases load unchanged: each rejection below comes from its edit. */
    expect_loads(device_of(classic_doc, "SFX-8251A"), "8251A base");
    expect_loads(device_of(classic_doc, "SFX-AY-5-1013A"), "AY-5-1013A base");
    expect_loads(device_of(shipped_doc, "XR21B1411"), "XR21B1411 base");

    /* Device keys. */
    reject_edit(classic_doc, "SFX-8251A", "model", NULL, "missing \"model\"", "no model");
    reject_edit(classic_doc, "SFX-8251A", "fifo", "16",
                "\"fifo\" is not a key of model \"i8251\"", "an ns16550 key on an i8251");
    reject_edit(classic_doc, "SFX-8251A", "grades", "5", "\"grades\" must be a non-empty object",
                "grades a number");
    reject_edit(classic_doc, "SFX-8251A", "grades", "{}", "\"grades\" must be a non-empty object",
                "grades empty");
    dev = device_of(classic_doc, "SFX-8251A");
    edit(dev, "grades.C", "{\"trd\":200}", 1);
    reject(dev, "duplicate key \"C\"", "a grade named twice");
    /* A grade. */
    reject_edit(classic_doc, "SFX-8251A", "grades.C", "5", "a grade is an object of timing values",
                "grade a number");
    reject_edit(classic_doc, "SFX-8251A", "grades.C.tbogus", "1", "tbogus", "grade unknown timing");
    reject_edit(classic_doc, "SFX-8251A", "grades.C.trd", "\"slow\"", "trd", "timing a string");
    reject_edit(classic_doc, "SFX-8251A", "grades.C.note", "5", "\"note\" must be a string",
                "grade note a number");
    /* A part. */
    reject_edit(classic_doc, "SFX-8251A", "parts.0.fiveBitStops", "2",
                "\"fiveBitStops\" is not a part key of model \"i8251\"",
                "an ay1013 part key on an i8251 part");
    reject_edit(classic_doc, "SFX-8251A", "parts.0.grade", NULL, "missing \"grade\"",
                "part without a grade");
    reject_edit(classic_doc, "SFX-8251A", "parts.0.vcc", "\"high\"", "vcc", "part vcc a string");
    reject_edit(classic_doc, "SFX-AY-5-1013A", "parts.0.fiveBitStops", "3",
                "\"fiveBitStops\" must be 1.5 or 2", "the model's part loader refuses");

    /* usbSupply (XR21B1411: VCC is the USB core supply, VIO_REF the I/O one). */
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.bogus", "1", "bogus", "usbSupply unknown key");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.power", "5", "power", "usbSupply power a number");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.power", "[]",
                "\"power\" needs at least one pin name", "usbSupply power empty");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.pins", "5", "pins", "usbSupply pins a number");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.vcc", "\"5V\"", "vcc", "usbSupply vcc a string");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.power", "[\"NOPE\"]", "NOPE",
                "usbSupply power not in the pinout");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.power", "[\"TX\"]",
                "\"usbSupply\": \"TX\" is not a power pin", "usbSupply power on a signal pin");
    reject_edit(shipped_doc, "XR21B1411", "usbSupply.power", "[\"VCC\",\"VIO_REF\"]",
                "\"usbSupply\": no power pin is left for the I/O supply",
                "usbSupply takes every power pin");
    case_end("C1 serial loader rejections");
}

static void case_c2_helpers(void) {
    case_begin();
    /* su_load_names: bus.data. */
    reject_edit(classic_doc, "SFX-8251A", "bus.data", NULL, "missing \"data\"", "bus.data missing");
    reject_edit(classic_doc, "SFX-8251A", "bus.data", "5", "data", "bus.data a number");
    reject_edit(classic_doc, "SFX-8251A", "bus.data.7", NULL, "\"data\" needs exactly 8 pin names",
                "bus.data with 7 names");
    /* su_load_pin_object: serial. */
    reject_edit(classic_doc, "SFX-8251A", "serial", NULL, "missing \"serial\"", "serial missing");
    reject_edit(classic_doc, "SFX-8251A", "serial", "5", "must be an object of pin names",
                "serial a number");
    reject_edit(classic_doc, "SFX-8251A", "serial.bogus", "\"TXD\"", "bogus", "serial unknown key");
    reject_edit(classic_doc, "SFX-8251A", "serial.tx", "5", "tx", "serial.tx a number");
    reject_edit(classic_doc, "SFX-8251A", "serial.tx", NULL, "missing \"tx\"", "serial.tx missing");
    /* su_bind_names / su_bind_object / serial_bind_pin: a name the pinout lacks. */
    reject_edit(classic_doc, "SFX-8251A", "bus.data.0", "\"NOPE\"", "bus.data[0]",
                "bus.data[0] not a pin");
    reject_edit(classic_doc, "SFX-8251A", "serial.tx", "\"NOPE\"", "serial.tx", "serial.tx not a pin");
    case_end("C2 serial_util loader helpers");
}

/* ---- C3 the line engine ---------------------------------------------------------------- */

typedef struct {
    uint16_t data, known;
    int pending;
    unsigned started;
    unsigned got;
    SerialRxChar c;
} Ctx;

static int fetch(void *context, uint16_t *data, uint16_t *known) {
    Ctx *x = context;

    if (!x->pending) {
        return 0;
    }
    x->pending = 0;
    *data = x->data;
    *known = x->known;
    return 1;
}

static void received(void *context, const SerialRxChar *c) {
    Ctx *x = context;

    x->got++;
    x->c = *c;
}

static void started(void *context) {
    ((Ctx *)context)->started++;
}

static char level_char(LogicValue v) {
    return v == LOGIC_0 ? '0' : v == LOGIC_1 ? '1' : v == LOGIC_Z ? 'Z' : 'X';
}

/* Sends one character at factor 1 and returns the levels, one per tick. */
static void tx_levels(SerialFormat f, uint16_t data, uint16_t known, unsigned ticks, char *out) {
    SerialTx tx;
    Ctx x;
    SerialTxEvents ev;
    unsigned i;

    memset(&x, 0, sizeof x);
    x.data = data;
    x.known = known;
    x.pending = 1;
    memset(&ev, 0, sizeof ev);
    ev.context = &x;
    ev.fetch = fetch;
    serial_tx_reset(&tx);
    for (i = 0; i < ticks; i++) {
        out[i] = level_char(serial_tx_tick(&tx, &f, &ev));
    }
    out[ticks] = '\0';
}

/* Feeds a line pattern ('0' '1' 'X', each one bit of `factor` ticks) to a receiver. */
static void rx_feed(SerialRx *rx, const SerialFormat *f, const char *bits, Ctx *x) {
    SerialRxEvents ev;
    unsigned i, t;

    memset(&ev, 0, sizeof ev);
    ev.context = x;
    ev.received = received;
    ev.started = started;
    for (i = 0; bits[i] != '\0'; i++) {
        LogicValue v = bits[i] == '0' ? LOGIC_0 : bits[i] == '1' ? LOGIC_1 : LOGIC_X;

        for (t = 0; t < f->factor; t++) {
            serial_rx_tick(rx, v, f, &ev);
        }
    }
}

static void case_c3_line(void) {
    SerialFormat f = serial_format_default();
    SerialRx rx;
    SerialLine line;
    Ctx x;
    char text[64];
    unsigned i;

    case_begin();
    /* Transmitter, 8E1 at factor 1: data bit 0 unknown -> that bit and the
       parity bit are X (serial_line.h: data is a (data, known) pair). */
    f.factor = 1;
    f.parity = SERIAL_PARITY_EVEN;
    tx_levels(f, 0x00, 0xFE, 12, text);
    h_expect_text(text, "0X0000000X11", "TX 8E1: unknown bit 0 and parity send X");
    /* Stick parity: MARK sends 1, SPACE sends 0, whatever the data. */
    f.parity = SERIAL_PARITY_MARK;
    tx_levels(f, 0x00, 0xFF, 11, text);
    h_expect_text(text, "00000000011", "TX 8M1: the parity bit is 1");
    f.parity = SERIAL_PARITY_SPACE;
    tx_levels(f, 0xFF, 0xFF, 11, text);
    h_expect_text(text, "01111111101", "TX 8S1: the parity bit is 0");
    /* format_x: every bit of the frame is X. */
    f.parity = SERIAL_PARITY_NONE;
    f.format_x = 1;
    tx_levels(f, 0x55, 0xFF, 10, text);
    h_expect_text(text, "XXXXXXXXXX", "TX with an unknown format: every bit X");
    f.format_x = 0;

    /* Receiver, 8E1 at 16x: bit 0 X, parity 1, stop X -> bit 0 unknown, PE and
       FE unknown, BI known 0 (a 1 was seen). */
    f = serial_format_default();
    f.parity = SERIAL_PARITY_EVEN;
    serial_rx_reset(&rx);
    memset(&x, 0, sizeof x);
    rx_feed(&rx, &f, "110X00000001X11", &x);
    check(x.started == 1 && x.got == 1, "RX: one start validated (started event), one character");
    check(x.c.known == 0xFE && x.c.data == 0, "RX: an X data sample leaves its bit unknown");
    check(x.c.flags_known == SERIAL_RX_BI && x.c.flags == 0,
          "RX: X parity (unknown data) and an X stop: PE, FE unknown; BI known clear");
    /* 8N1: zeros and one X, stop 0 -> FE, and BI unknown (it may be a break). */
    f.parity = SERIAL_PARITY_NONE;
    serial_rx_reset(&rx);
    memset(&x, 0, sizeof x);
    rx_feed(&rx, &f, "1100000000X0", &x);
    check(x.got == 1 && (x.c.flags & SERIAL_RX_FE) != 0 &&
          (x.c.flags_known & (SERIAL_RX_FE | SERIAL_RX_BI | SERIAL_RX_PE)) ==
              (SERIAL_RX_FE | SERIAL_RX_PE),
          "RX: all 0 but one X with stop 0: FE set, BI unknown");
    /* Majority voting: an X bit has no majority -> unknown. */
    f.majority = 1;
    serial_rx_reset(&rx);
    memset(&x, 0, sizeof x);
    rx_feed(&rx, &f, "110X10101011111", &x);
    check(x.got == 1 && x.c.known == 0xFE && x.c.data == 0xAA,
          "RX majority: three X samples give no majority (bit unknown)");
    f.majority = 0;
    /* 82C50A: both stop bits checked. Good stops -> idle; an all-0 frame with
       both stops 0 is a break and waits for mark. */
    f.check_both_stops = 1;
    f.stop_halves = 4;
    serial_rx_reset(&rx);
    memset(&x, 0, sizeof x);
    rx_feed(&rx, &f, "11001010101111", &x);
    check(x.got == 1 && x.c.data == 0xAA && x.c.flags == 0 && !serial_rx_busy(&rx) &&
              rx.phase == SERIAL_RX_IDLE,
          "RX two stops checked: both 1 -> character, receiver idle");
    rx_feed(&rx, &f, "00000000000", &x);
    check(x.got == 2 && (x.c.flags & (SERIAL_RX_BI | SERIAL_RX_FE)) == (SERIAL_RX_BI | SERIAL_RX_FE) &&
              rx.phase == SERIAL_RX_WAIT_MARK,
          "RX two stops checked: an all-0 frame is a break; the receiver waits for mark");

    /* The terminal sender, 8O1 at factor 1: idle mark, start, 0x01 LSB first,
       odd parity 0, stop, then the pin is released (back to the pin's level). */
    f = serial_format_default();
    f.factor = 1;
    f.parity = SERIAL_PARITY_ODD;
    serial_line_init(&line, SIZE_MAX);
    check(serial_line_queue(&line, (const uint8_t *)"\x01", 1) == 0, "sender: queued");
    for (i = 0; i < 13; i++) {
        text[i] = level_char(serial_line_rx_input(&line, NULL, NULL, LOGIC_X, &f));
    }
    text[13] = '\0';
    h_expect_text(text, "101000000001X", "sender 8O1: odd parity bit 0 for 0x01");
    /* A reset mid-frame drops the queue and releases the pin. */
    check(serial_line_queue(&line, (const uint8_t *)"ab", 2) == 0, "sender: queued 2");
    for (i = 0; i < 4; i++) {
        serial_line_rx_input(&line, NULL, NULL, LOGIC_X, &f);
    }
    check(line.sender.active, "sender: sending");
    serial_line_sender_reset(&line, NULL, NULL);
    check(!line.sender.active && line.sender.level == LOGIC_Z && line.sender.count == 0,
          "sender reset mid-frame: released (Z), queue empty");
    check(serial_line_rx_input(&line, NULL, NULL, LOGIC_0, &f) == LOGIC_0,
          "sender after reset: the receiver sees the pin");
    serial_line_free(&line);
    case_end("C3 line engine");
}

/* ---- C4 API misuse --------------------------------------------------------------------- */

static void case_c4_api(void) {
    Bench *b = bench_open("SFX-16550", HALF_18432);
    const SimComponentType *type = sim_instance_type(b->chip);
    void *state = sim_instance_state(b->chip);
    SerialPortSettings port = {9600, 8, SERIAL_PARITY_NONE, 2, 0};
    uint16_t frames[4];
    SerialBusPins pins;
    SerialBusTiming timing;
    SerialBus bus;
    LogicValue out[2] = {LOGIC_1, LOGIC_0};
    SuOutputs o = {out, 2};
    unsigned i;

    case_begin();
    check(type->serial_channels(state) == 1, "SFX-16550 has one channel");
    check(type->serial_sent(state, 1, 0, frames, 4) == 0, "sent on a channel that does not exist: 0");
    check(serial_configure(NULL, NULL, state, 1, &port) == -1,
          "serial_configure on a channel that does not exist: -1");
    check(serial_configure(NULL, NULL, state, 0, &port) == 0,
          "serial_configure on a UART whose port is its own: 0");
    check(serial_rx_pin(state, 1) == SIZE_MAX, "serial_rx_pin on a channel that does not exist");
    check(serial_rx_pin(state, 0) == pin_of(b, "SIN"), "serial_rx_pin of channel 0 is SIN");
    bench_close(b);

    /* serial_bus_has_pin: no pin is never a bus pin, even with an unused slot. */
    memset(&pins, 0, sizeof pins);
    memset(&timing, 0, sizeof timing);
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        pins.data[i] = i;
    }
    pins.data[SERIAL_BUS_DATA - 1] = SERIAL_NO_PIN;
    serial_bus_init(&bus, &pins, &timing);
    check(serial_bus_has_pin(&bus, 3) == 1, "has_pin: a data pin");
    check(serial_bus_has_pin(&bus, SERIAL_NO_PIN) == 0, "has_pin: SERIAL_NO_PIN is not a bus pin");

    /* su_mux: an X selector keeps equal candidates, else X. */
    check(su_mux(LOGIC_1, LOGIC_1, LOGIC_0) == LOGIC_1 && su_mux(LOGIC_0, LOGIC_1, LOGIC_0) == LOGIC_0,
          "su_mux: a known selector picks its input");
    check(su_mux(LOGIC_X, LOGIC_1, LOGIC_1) == LOGIC_1, "su_mux X select, equal inputs: that level");
    check(su_mux(LOGIC_X, LOGIC_1, LOGIC_0) == LOGIC_X, "su_mux X select, different inputs: X");
    /* su_drive_weak: no pin, or a pin past the outputs, does nothing. */
    su_drive_weak(&o, NULL, NULL, SERIAL_NO_PIN, LOGIC_0);
    su_drive_weak(&o, NULL, NULL, 2, LOGIC_0);
    check(out[0] == LOGIC_1 && out[1] == LOGIC_0, "su_drive_weak without a pin leaves the outputs");
    case_end("C4 API misuse");
}

int main(void) {
    char path[512];
    char *text;

    if (serial_bench_load() != 0) {
        return 1;
    }
    classic_doc = cJSON_Parse(SERIAL_FIXTURE_CLASSIC);
    snprintf(path, sizeof path, "%s/serial.json", PARTS_DIR);
    text = read_file(path);
    shipped_doc = text != NULL ? cJSON_Parse(text) : NULL;
    free(text);
    if (classic_doc == NULL || shipped_doc == NULL) {
        printf("FAIL: fixtures\n");
        return 1;
    }
    case_c1_loader();
    case_c2_helpers();
    case_c3_line();
    case_c4_api();
    cJSON_Delete(classic_doc);
    cJSON_Delete(shipped_doc);
    return serial_bench_finish();
}
