/*
 * serial_max310x_cov_test (plan-68-P): the MAX3107 of the max310x model
 * (models/serial_max310x*.c) past what serial_max310x_test covers: loader
 * rejections, the pin states (RST X, ready, loopback, 3-state, break,
 * inversion), FIFO edges (overrun, FIFORst, triggers and their inversions,
 * a full TX FIFO), software flow control in both directions, echo
 * suppression, multidrop with GPIAddr and the terminal's async rule.
 * Expected values: the model header (serial_max310x_int.h) and
 * plan-12-data/specs/maxim.md § MAX3107 (register bits p31-p40).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"

#include "cJSON.h"
#include "model_pins.h"

#define MX_HALF NS(271)   /* XIN = 1.8432 MHz */
#define BT (16u * 2 * MX_HALF)

/* ---- A variant of a fixture device ---------------------------------------------------- */

static int variant_n;
static char variant_name[96];

/* Sets (or deletes, for the string "-") the item at a dotted path. */
static void apply_edit(cJSON *root, const char *path, const cJSON *value) {
    char buf[160];
    char *key;
    char *dot;
    cJSON *at = root;
    int del = cJSON_IsString(value) && strcmp(value->valuestring, "-") == 0;

    snprintf(buf, sizeof buf, "%s", path);
    key = buf;
    while ((dot = strchr(key, '.')) != NULL) {
        *dot = '\0';
        at = cJSON_IsArray(at) ? cJSON_GetArrayItem(at, atoi(key)) : cJSON_GetObjectItemCaseSensitive(at, key);
        key = dot + 1;
    }
    if (cJSON_IsArray(at)) {
        cJSON_ReplaceItemInArray(at, atoi(key), cJSON_Duplicate(value, 1));
    } else if (del) {
        cJSON_DeleteItemFromObjectCaseSensitive(at, key);
    } else if (cJSON_GetObjectItemCaseSensitive(at, key) != NULL) {
        cJSON_ReplaceItemInObjectCaseSensitive(at, key, cJSON_Duplicate(value, 1));
    } else {
        cJSON_AddItemToObject(at, key, cJSON_Duplicate(value, 1));
    }
}

/* Loads device `id` of the Maxim fixture with `edits`; 0 or -1 with *error. */
static int mx_variant(const char *id, const char *edits, char **error) {
    cJSON *file = cJSON_Parse(SERIAL_FIXTURE_MAXIM);
    cJSON *ed = cJSON_Parse(edits);
    cJSON *dev = NULL;
    cJSON *item;
    cJSON *out;
    char new_id[64];
    char *json;
    int status;

    maxim_need();
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(file, "devices")) {
        if (strcmp(cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring, id) == 0) {
            dev = item;
        }
    }
    if (dev == NULL || ed == NULL) {
        fprintf(stderr, "mx_variant: %s %s\n", id, edits);
        exit(2);
    }
    cJSON_ArrayForEach(item, ed) {
        apply_edit(dev, item->string, item);
    }
    variant_n++;
    snprintf(new_id, sizeof new_id, "%s-V%d", id, variant_n);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(new_id));
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(dev, "parts")) {
        char name[96];

        snprintf(name, sizeof name, "%s-V%d",
                 cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring, variant_n);
        if (item == cJSON_GetArrayItem(cJSON_GetObjectItemCaseSensitive(dev, "parts"), 0)) {
            snprintf(variant_name, sizeof variant_name, "%s", name);
        }
        cJSON_ReplaceItemInObjectCaseSensitive(item, "name", cJSON_CreateString(name));
    }
    out = cJSON_CreateObject();
    cJSON_AddStringToObject(out, "format", "schematical-parts");
    cJSON_AddNumberToObject(out, "version", 1);
    cJSON_AddStringToObject(out, "kind", "serial");
    cJSON_AddItemToObject(out, "devices", cJSON_CreateArray());
    cJSON_AddItemToArray(cJSON_GetObjectItemCaseSensitive(out, "devices"), cJSON_DetachItemViaPointer(
        cJSON_GetObjectItemCaseSensitive(file, "devices"), dev));
    json = cJSON_PrintUnformatted(out);
    *error = NULL;
    status = load_parts(library, json, "max310x_variant.json", error);
    cJSON_free(json);
    cJSON_Delete(out);
    cJSON_Delete(ed);
    cJSON_Delete(file);
    return status;
}

static void reject(const char *id, const char *edits, const char *want) {
    char *error = NULL;
    char what[600];
    int status = mx_variant(id, edits, &error);

    snprintf(what, sizeof what, "%s %s refused: \"%s\" (got: %s)", id, edits, want,
             status == 0 ? "loaded" : error != NULL ? error : "(null)");
    check(status != 0 && error != NULL && strstr(error, want) != NULL, what);
    free(error);
}

/* ---- The MAX3107 on the bench ----------------------------------------------------------- */

static MB *mx_open_part(const char *part, const char *const *ties) {
    MB *b = mb_open(part, "XIN", MX_HALF, ties, NULL);

    b->cs = "CS/A0";
    b->sclk = "SCLK/SCL";
    b->mosi = "DIN/A1";
    b->miso = "DOUT/SDA";
    b->scl = "SCLK/SCL";
    b->sda = b->miso;
    mb_set(b, "I2C/SPI", LOGIC_1);
    mb_set(b, "RST", LOGIC_1);
    mb_set(b, "CS/A0", LOGIC_1);
    mb_set(b, "SCLK/SCL", LOGIC_0);
    mb_set(b, b->mosi, LOGIC_0);
    mb_set(b, "CTS", LOGIC_0);
    if (ties == NULL) {
        mb_set(b, "RX", LOGIC_1);
    }
    mb_run(b, NS(2000));
    return b;
}

static MB *mx_open(void) {
    return mx_open_part("SFX-MAX3107-ssop24", NULL);
}

static void mx_w(MB *b, unsigned cmd, unsigned value) {
    mb_spi(b, ((0x80u | cmd) << 8) | value, 16, NULL, 0);
}

static unsigned mx_r(MB *b, unsigned cmd) {
    return mb_spi(b, cmd << 8, 16, NULL, 0) & 0xFF;
}

/* The data byte of a read as "01XZ" text. */
static void mx_r_text(MB *b, unsigned cmd, char *text) {
    char all[20];

    mb_spi(b, cmd << 8, 16, all, 0);
    memcpy(text, all + 8, 8);
    text[8] = '\0';
}

/* External clock on (ClockEn + PLLBypass), 8N1, DIV = 1: BT per bit. */
static void mx_setup(MB *b) {
    mx_w(b, 0x1E, 0x18);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x1C, 0x01);
    mx_w(b, 0x1D, 0x00);
}

static void rx_bytes(MB *b, const unsigned *v, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        mb_drive_byte(b, "RX", v[i], BT, 1);
    }
    mb_run(b, BT);
}

static void rx_byte(MB *b, unsigned v) {
    rx_bytes(b, &v, 1);
}

/* ---- Loader ------------------------------------------------------------------------------------ */

static void case_loader(void) {
    case_begin();
    reject("SFX-MAX3107", "{\"variant\":\"-\"}", "\"variant\"");
    reject("SFX-MAX3107", "{\"variant\":\"max9999\"}", "unknown \"variant\" \"max9999\"");
    reject("SFX-MAX3107", "{\"bus\":\"-\"}", "\"bus\"");
    reject("SFX-MAX3107",
           "{\"bus\":{\"style\":\"spi\",\"cs\":\"CS/A0\",\"sclk\":\"SCLK/SCL\","
           "\"mosi\":\"DIN/A1\",\"miso\":\"DOUT/SDA\",\"mode\":0}}", "the max310x host port is \"i2c-spi\" with SPI mode 0");
    reject("SFX-MAX3107", "{\"bus.spi.mode\":1}", "the max310x host port is \"i2c-spi\" with SPI mode 0");
    reject("SFX-MAX3107", "{\"bus.spi.mode\":9}", "\"mode\"");
    reject("SFX-MAX3107", "{\"bus.uartOffsets\":[0,1]}", "\"uartOffsets\" needs one number per UART");
    reject("SFX-MAX3107", "{\"bus.uartOffsets\":5}", "\"uartOffsets\" needs one number per UART");
    reject("SFX-MAX3107", "{\"bus.uartOffsets\":[0.5]}",
           "\"uartOffsets\" entries are whole numbers from -127 to 127");
    reject("SFX-MAX3107", "{\"bus.uartOffsets\":[\"a\"]}",
           "\"uartOffsets\" entries are whole numbers from -127 to 127");
    reject("SFX-MAX3107", "{\"bus.uartOffsets\":[-128]}",
           "\"uartOffsets\" entries are whole numbers from -127 to 127");
    reject("SFX-MAX3107", "{\"bus.uartOffsets\":[128]}",
           "\"uartOffsets\" entries are whole numbers from -127 to 127");
    reject("SFX-MAX3109", "{\"bus.uartOffsets\":\"-\"}", "missing \"uartOffsets\"");
    reject("SFX-MAX3107", "{\"clock\":\"-\"}", "\"clock\"");
    reject("SFX-MAX3107", "{\"clock.speed\":1}", "\"speed\"");
    reject("SFX-MAX3107", "{\"clock.in\":\"-\"}", "\"in\"");
    reject("SFX-MAX3107", "{\"clock.out\":5}", "\"out\"");
    reject("SFX-MAX3107", "{\"reset\":\"-\"}", "\"reset\"");
    reject("SFX-MAX3107", "{\"interrupt\":\"-\"}", "\"interrupt\"");
    reject("SFX-MAX3107", "{\"channels\":\"-\"}", "\"channels\"");
    reject("SFX-MAX3107", "{\"channels\":[]}", "\"channels\" must be an array of 1 channel(s) for max3107");
    reject("SFX-MAX3107", "{\"channels\":{}}", "\"channels\" must be an array of 1 channel(s) for max3107");
    reject("SFX-MAX3107", "{\"channels.0.dcd\":\"CTS\"}", "\"dcd\"");
    reject("SFX-MAX3107", "{\"channels.0.tx\":\"-\"}", "\"tx\"");
    reject("SFX-MAX3107", "{\"gpio\":5}", "\"gpio\"");
    reject("SFX-MAX3107", "{\"gpio\":[\"GPIO0\"]}", "\"gpio\" needs 4 pin names (4 per UART)");
    reject("SFX-MAX3107", "{\"bus.spi.cs\":\"NOPE\"}", "NOPE");
    reject("SFX-MAX3107", "{\"reset\":\"NOPE\"}", "NOPE");
    reject("SFX-MAX3107", "{\"interrupt\":\"NOPE\"}", "NOPE");
    reject("SFX-MAX3107", "{\"clock.in\":\"NOPE\"}", "NOPE");
    reject("SFX-MAX3107", "{\"clock.out\":\"NOPE\"}", "NOPE");
    reject("SFX-MAX3107", "{\"channels.0.cts\":\"NOPE\"}", "NOPE");
    reject("SFX-MAX3107", "{\"gpio.3\":\"NOPE\"}", "NOPE");
    case_end("max310x loader: variant, host port, uartOffsets, clock, channels, gpio and pin names refused");
}

/* ---- Pins: RST X, ready, 3-state, break, inversion ----------------------------------------- */

static void case_pins(void) {
    MB *b = mx_open();
    char *error = NULL;
    const char *name;

    case_begin();
    check(part_pin_open_drain(b->part, mb_pin(b, "IRQ")), "IRQ is open drain");
    check(!part_pin_open_drain(b->part, mb_pin(b, "TX")), "TX is push-pull");
    mx_w(b, 0x18, 0x01);   /* GPIO0 a push-pull output */
    mb_expect_pin(b, "GPIO0", LOGIC_0, "GPIO0 output 0");
    /* RST X: an unknown reset state drives every output X (plan-12-A). */
    mb_set(b, "RST", LOGIC_X);
    mb_run(b, NS(100));
    mb_expect_pin(b, "IRQ", LOGIC_X, "RST X: IRQ X");
    mb_expect_pin(b, "TX", LOGIC_X, "RST X: TX X");
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_X, "RST X: RTS X");
    mb_expect_pin(b, "GPIO0", LOGIC_X, "RST X: GPIO X");
    mb_set(b, "RST", LOGIC_1);
    mb_run(b, NS(100));
    mb_expect_pin(b, "TX", LOGIC_1, "RST high again: TX idles high");
    mb_expect_pin(b, "IRQ", LOGIC_Z, "RST high again: IRQ released (ready)");
    mx_setup(b);
    /* TxBreak holds TX low (LCR[6]). */
    mx_w(b, 0x0B, 0x43);
    mb_expect_pin(b, "TX", LOGIC_0, "TxBreak: TX low");
    mx_w(b, 0x0B, 0x03);
    mb_expect_pin(b, "TX", LOGIC_1, "TxBreak off: TX high");
    /* IrDA[5] TxInv inverts TX (independent of IrDA). */
    mx_w(b, 0x0E, 0x20);
    mb_expect_pin(b, "TX", LOGIC_0, "TxInv: the idle line reads low");
    mx_w(b, 0x0E, 0x00);
    /* MODE1[2] TxHiZ, [3] RTSHiZ. */
    mx_w(b, 0x09, 0x04);
    mb_expect_pin(b, "TX", LOGIC_Z, "TxHiZ: TX 3-state");
    mx_w(b, 0x09, 0x08);
    mb_expect_pin(b, "TX", LOGIC_1, "TxHiZ off: TX driven");
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_Z, "RTSHiZ: RTS 3-state");
    mx_w(b, 0x09, 0x00);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "RTSHiZ off: RTS from LCR[7] = 0");
    /* ClockReady (STSInt[5]): the crystal oscillator with a running reference. */
    mx_w(b, 0x1E, 0x1A);
    mb_run(b, NS(2000));
    mb_expect_u(mx_r(b, 0x08) & 0x20, 0x20, "CrystalEn: STSInt ClockReady");
    mx_w(b, 0x07, 0x20);
    mb_expect_u(mx_r(b, 0x02) & 0x04, 0x04, "ClockReady through STSIntEn: ISR[2]");
    mx_w(b, 0x07, 0x00);
    mx_w(b, 0x1E, 0x18);
    mb_run(b, NS(2000));
    mb_expect_u(mx_r(b, 0x08) & 0x20, 0x00, "external clock: no ClockReady");
    mb_close(b);

    /* IRQSel = 0: IRQ held low until the device is ready (p22): tready 5 us. */
    check(mx_variant("SFX-MAX3107", "{\"grades.E.tready\":5000}", &error) == 0, "tready variant loads");
    free(error);
    name = variant_name;
    b = mx_open_part(name, NULL);
    mb_expect_pin(b, "IRQ", LOGIC_0, "2 us after RST: IRQ held low (not ready)");
    mb_run(b, NS(3500));
    mb_expect_pin(b, "IRQ", LOGIC_Z, "after tready: IRQ released");
    mb_set(b, "RST", LOGIC_0);
    mb_run(b, NS(500));
    mb_set(b, "RST", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_pin(b, "IRQ", LOGIC_0, "after a reset pulse: IRQ low again until ready");
    mb_run(b, NS(5000));
    mb_expect_pin(b, "IRQ", LOGIC_Z, "ready again");
    case_end("max310x pins: RST X, IRQ until ready, break, TxInv, TxHiZ, RTSHiZ, ClockReady");
    mb_close(b);
}

/* ---- Loopback, RX inversion, receiver off, the terminal ------------------------------------- */

static void case_loop(void) {
    MB *b = mx_open();
    char text[12];

    case_begin();
    mx_setup(b);
    /* Loopback: TX 3-state, RTS pin from LCR[7], TX -> RX and RTS -> CTS inside. */
    mx_w(b, 0x0A, 0x20);
    mb_expect_pin(b, "TX", LOGIC_Z, "loopback: TX pin 3-state");
    mx_w(b, 0x0B, 0x83);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_1, "loopback: RTS pin = LCR[7] = 1");
    mx_w(b, 0x13, 0x02);   /* AutoCTS: the looped RTS is CTS */
    mx_w(b, 0x00, 0x5A);
    mb_run(b, BT * 14);
    mb_expect_u(mx_r(b, 0x12), 0, "loopback + AutoCTS: RTS high holds the transmitter");
    mx_w(b, 0x0B, 0x03);
    mb_expect_pin(b, "RTS/CLKOUT", LOGIC_0, "loopback: RTS pin = LCR[7] = 0");
    mb_run(b, BT * 14);
    mb_expect_u(mx_r(b, 0x12), 1, "loopback: RTS low lets it send; the word comes back");
    mb_expect_u(mx_r(b, 0x00), 0x5A, "loopback: the word read back");
    mx_r(b, 0x02);
    mb_set(b, "CTS", LOGIC_1);
    mb_run(b, NS(500));
    mb_set(b, "CTS", LOGIC_0);
    mb_run(b, NS(500));
    mb_expect_u(mx_r(b, 0x02) & 0x80, 0x00, "loopback: the CTS pin is disconnected (no CTSInt)");
    mx_w(b, 0x13, 0x00);
    mx_w(b, 0x0A, 0x00);
    mb_expect_pin(b, "TX", LOGIC_1, "loopback off: TX driven again");
    /* IrDA[4] RxInv: RX idles low, a frame arrives inverted. */
    mx_w(b, 0x0E, 0x10);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, BT * 12);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    check(mb_send(b, 0, "A") == 0, "RxInv: the terminal does not send (not its NRZ waveform)");
    mb_drive_bits(b, "RX", "010110101", BT, LOGIC_0);
    mb_run(b, BT * 2);
    mb_expect_u(mx_r(b, 0x12), 1, "RxInv: one word");
    mb_expect_u(mx_r(b, 0x00), 0x5A, "RxInv: 0x5A received inverted");
    mx_w(b, 0x0E, 0x00);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, BT * 12);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    /* The terminal sends while the line is plain NRZ (the bench lets go of RX). */
    mb_set(b, "RX", LOGIC_Z);
    check(mb_send(b, 0, "AB") == 1, "the terminal sends with IrDA off");
    mb_run(b, BT * 24);
    mb_set(b, "RX", LOGIC_1);
    mb_expect_u(mx_r(b, 0x12), 2, "the terminal's two words arrive");
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x0E, 0x01);
    check(mb_send(b, 0, "A") == 0, "IrDAEn: the terminal does not send");
    mx_w(b, 0x0E, 0x00);
    mb_run(b, BT * 12);   /* IrDA reads the high RX as pulses: flush that */
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    /* MODE1[0] RxDisabl: the receiver is off. */
    mx_w(b, 0x09, 0x01);
    rx_byte(b, 0x31);
    mb_expect_u(mx_r(b, 0x12), 0, "RxDisabl: nothing received");
    mx_w(b, 0x09, 0x00);
    rx_byte(b, 0x32);
    mb_expect_u(mx_r(b, 0x12), 1, "RxDisabl off: received");
    mb_expect_u(mx_r(b, 0x00), 0x32, "RxDisabl off: the word");
    /* A frame with an X data bit: the word keeps the X. */
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, BT);
    {
        static const LogicValue bits[] = {LOGIC_1, LOGIC_0, LOGIC_0, LOGIC_X, LOGIC_0, LOGIC_0, LOGIC_0,
                                          LOGIC_0, LOGIC_1};
        unsigned i;

        for (i = 0; i < 9; i++) {
            mb_set(b, "RX", bits[i]);
            mb_run(b, BT);
        }
    }
    mb_run(b, BT);
    mb_expect_u(mx_r(b, 0x12), 1, "X data bit: the word is stored");
    mx_r_text(b, 0x00, text);
    check(strcmp(text, "0000X001") == 0, "X data bit: RHR reads bit 3 X");
    case_end("max310x loopback, RxInv, RxDisabl, the terminal's async rule, an X data bit");
    mb_close(b);
}

/* ---- FIFOs ------------------------------------------------------------------------------------ */

static void case_fifo(void) {
    MB *b = mx_open();
    unsigned many[12];
    unsigned i;

    case_begin();
    /* FIFORst with words in the TX FIFO sets TxEmpty (clock off: they wait). */
    mx_w(b, 0x00, 0x41);
    mx_w(b, 0x00, 0x42);
    mx_r(b, 0x02);
    mb_expect_u(mx_r(b, 0x02), 0x00, "ISR clear");
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    mb_expect_u(mx_r(b, 0x02), 0x20, "FIFORst of a full TX FIFO: TxEmptyInt only");
    mb_expect_u(mx_r(b, 0x11), 0, "FIFORst: TxFIFOLvl 0");
    /* A full TX FIFO drops what is written after 128 words. */
    for (i = 0; i < 130; i++) {
        mx_w(b, 0x00, i & 0xFF);
    }
    mb_expect_u(mx_r(b, 0x11), 128, "TX FIFO full: TxFIFOLvl 128");
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    /* TxTrig (FIFOTrgLvl[3:0] x 8 = 8): ISR[4] at 8 words; it clears below 8. */
    mx_w(b, 0x10, 0xF1);
    mx_r(b, 0x02);
    for (i = 0; i < 8; i++) {
        mx_w(b, 0x00, 0x30 + i);
    }
    mx_w(b, 0x01, 0x10);
    mx_w(b, 0x09, 0x80);
    mb_expect_pin(b, "IRQ", LOGIC_0, "TxTrig: 8 words in the TX FIFO");
    mx_setup(b);
    check(mb_wait(b, "IRQ", LOGIC_Z, BT * 2, NS(50)) >= 0, "TxTrig clears once the first word is taken");
    mb_expect_u(mx_r(b, 0x11) >= 6, 1, "and the FIFO still holds words");
    mx_w(b, 0x01, 0x00);
    mx_w(b, 0x09, 0x00);
    mb_run(b, BT * 90);
    /* FIFORst held: received words are not stored. */
    mx_w(b, 0x0A, 0x02);
    rx_byte(b, 0x51);
    mx_w(b, 0x0A, 0x00);
    mb_expect_u(mx_r(b, 0x12), 0, "FIFORst held: the received word is dropped");
    /* RxEmtyInv: ISR[6] when the first word arrives. */
    mx_w(b, 0x0A, 0x08);
    mx_r(b, 0x02);
    rx_byte(b, 0x52);
    mb_expect_u(mx_r(b, 0x02) & 0x40, 0x40, "RxEmtyInv: ISR[6] with one word");
    mx_r(b, 0x00);
    mb_expect_u(mx_r(b, 0x02) & 0x40, 0x00, "RxEmtyInv: no ISR[6] when it empties");
    mx_w(b, 0x0A, 0x00);
    /* RxTrig = 8 (FIFOTrgLvl 0x1F): ISR[3] when the 8th word arrives. */
    mx_w(b, 0x10, 0x1F);
    mx_r(b, 0x02);
    for (i = 0; i < 9; i++) {
        many[i] = 0x60 + i;
    }
    rx_bytes(b, many, 7);
    mb_expect_u(mx_r(b, 0x02) & 0x08, 0x00, "RxTrig: not at 7 words");
    rx_byte(b, 0x67);
    mb_expect_u(mx_r(b, 0x02) & 0x08, 0x08, "RxTrig: ISR[3] at 8 words");
    /* RxTrigInv: ISR[3] when reading brings the level down to the trigger. */
    mx_w(b, 0x0A, 0x04);
    rx_byte(b, 0x68);
    mb_expect_u(mx_r(b, 0x02) & 0x08, 0x00, "RxTrigInv: no ISR[3] on receiving");
    mx_r(b, 0x00);
    mb_expect_u(mx_r(b, 0x02) & 0x08, 0x08, "RxTrigInv: ISR[3] at 8 words when reading down");
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x10, 0xFF);
    /* Overrun: the 129th word is lost, LSR[1] until LSR is read. */
    for (i = 0; i < 129; i++) {
        mb_drive_byte(b, "RX", i & 0x7F, BT, 1);
    }
    mb_run(b, BT);
    mb_expect_u(mx_r(b, 0x12), 128, "overrun: RxFIFOLvl 128");
    mb_expect_u(mx_r(b, 0x04) & 0x02, 0x02, "overrun: LSR RxOverrun");
    mb_expect_u(mx_r(b, 0x04) & 0x02, 0x00, "reading LSR clears it");
    mb_expect_u(mx_r(b, 0x00), 0x00, "the FIFO kept the first word");
    /* A break: BREAKInt, an all-zero word with LSR[4]; ISR[1] and ISR[0]. */
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x05, 0x10);
    mx_w(b, 0x03, 0x10);
    mx_r(b, 0x02);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, BT * 24);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, BT * 2);
    mb_expect_u(mx_r(b, 0x02) & 0x03, 0x02, "break: ISR[1] SpclCharInt (BREAKInt enabled)");
    mb_expect_u(mx_r(b, 0x06) & 0x10, 0x10, "break: SpclCharInt BREAKInt");
    mb_expect_u(mx_r(b, 0x12) >= 1, 1, "break: a word in the FIFO");
    mb_expect_u(mx_r(b, 0x00), 0x00, "break: the word is all zero");
    mb_expect_u(mx_r(b, 0x04) & 0x10, 0x10, "break: LSR RxBreak for that word");
    mb_expect_u(mx_r(b, 0x02) & 0x01, 0x01, "break: ISR[0] through LSRIntEn");
    case_end("max310x FIFOs: FIFORst, TX full, TxTrig, RxEmtyInv, RxTrig(Inv), overrun, break");
    mb_close(b);
}

/* ---- Software flow control ------------------------------------------------------------------ */

static void case_flow(void) {
    MB *b = mx_open();
    unsigned many[9];
    uint16_t log[8];
    unsigned i;
    size_t n;

    case_begin();
    mx_setup(b);
    mx_w(b, 0x14, 0x11);   /* XON1 */
    mx_w(b, 0x15, 0x91);   /* XON2 */
    mx_w(b, 0x16, 0x13);   /* XOFF1 */
    mx_w(b, 0x17, 0x93);   /* XOFF2 */
    /* Receive flow control, two-byte (SwFlow[3:2] = 11): XOFF1 XOFF2 above
       Halt, XON1 XON2 below Resume (FlowLvl 0x11: 8 and 8). */
    mx_w(b, 0x0F, 0x11);
    mx_w(b, 0x13, 0xC8);
    for (i = 0; i < 9; i++) {
        many[i] = 0x40 + i;
    }
    rx_bytes(b, many, 9);
    mb_run(b, BT * 22);
    n = mb_sent(b, 0, log, 8);
    check(n == 2 && log[0] == 0x13 && log[1] == 0x93, "9 words > Halt: XOFF1 then XOFF2 sent");
    mx_r(b, 0x00);
    mx_r(b, 0x00);
    mb_run(b, BT * 22);
    n = mb_sent(b, 0, log, 8);
    check(n == 4 && log[2] == 0x11 && log[3] == 0x91, "7 words < Resume: XON1 then XON2 sent");
    mx_w(b, 0x13, 0x00);
    mx_w(b, 0x0A, 0x02);
    mx_w(b, 0x0A, 0x00);
    /* Transmit flow control, two-byte (SwFlow[1:0] = 11). */
    mx_w(b, 0x13, 0x38);
    rx_byte(b, 0x13);
    rx_byte(b, 0x93);
    mx_w(b, 0x00, 0x21);
    check(mb_wait(b, "TX", LOGIC_0, BT * 3, NS(50)) < 0, "XOFF1 XOFF2 received: the transmitter holds");
    rx_byte(b, 0x11);
    rx_byte(b, 0x91);
    check(mb_wait(b, "TX", LOGIC_0, BT * 3, NS(50)) >= 0, "XON1 XON2 received: it sends");
    mb_run(b, BT * 12);
    mb_expect_u(mx_r(b, 0x12), 0, "the flow pairs are not stored");
    rx_byte(b, 0x11);
    rx_byte(b, 0x41);
    mb_expect_u(mx_r(b, 0x12), 2, "XON1 then another word: both stored");
    mb_expect_u(mx_r(b, 0x00), 0x11, "the held XON1");
    mb_expect_u(mx_r(b, 0x00), 0x41, "then the word");
    rx_byte(b, 0x13);
    rx_byte(b, 0x11);
    rx_byte(b, 0x91);
    mx_w(b, 0x00, 0x22);
    check(mb_wait(b, "TX", LOGIC_0, BT * 3, NS(50)) >= 0, "XOFF1 broken by XON1 XON2: still sending");
    mb_run(b, BT * 12);
    mb_expect_u(mx_r(b, 0x12), 1, "the broken XOFF1 is stored");
    mx_r(b, 0x00);
    /* One-byte XON2/XOFF2 (SwFlow[1:0] = 01). */
    mx_w(b, 0x13, 0x18);
    rx_byte(b, 0x93);
    mx_w(b, 0x00, 0x23);
    check(mb_wait(b, "TX", LOGIC_0, BT * 3, NS(50)) < 0, "XOFF2 received: the transmitter holds");
    rx_byte(b, 0x91);
    check(mb_wait(b, "TX", LOGIC_0, BT * 3, NS(50)) >= 0, "XON2 received: it sends");
    mb_run(b, BT * 12);
    mb_expect_u(mx_r(b, 0x12), 0, "XON2/XOFF2 are not stored");
    case_end("max310x XON/XOFF: two-byte sending at Halt/Resume, two-byte and XON2/XOFF2 receiving");
    mb_close(b);
}

/* ---- Echo suppression, multidrop transmit and GPIAddr -------------------------------------- */

static void case_echo_multidrop(void) {
    static const char *const RX_ON_TX[] = {"RX", "TX", NULL};
    MB *b = mx_open_part("SFX-MAX3107-ssop24", RX_ON_TX);
    char frame[16];

    case_begin();
    mx_setup(b);
    mx_w(b, 0x00, 0x55);
    mb_run(b, BT * 14);
    mb_expect_u(mx_r(b, 0x12), 1, "RX on TX's net: the word comes back");
    mx_r(b, 0x00);
    mx_w(b, 0x0D, 0x02);   /* Hold 2: the blanking extension */
    mx_w(b, 0x0A, 0x80);
    mx_w(b, 0x00, 0x55);
    mb_run(b, BT * 14);
    mb_expect_u(mx_r(b, 0x12), 0, "EchoSuprs: the receiver discards it");
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x0D, 0x00);
    /* Multidrop without ForceParity: the 9th bit is not defined (X). */
    mx_w(b, 0x0A, 0x40);
    mx_w(b, 0x0B, 0x0B);
    mx_w(b, 0x00, 0x42);
    check(mb_wait(b, "TX", LOGIC_0, BT * 3, NS(20)) >= 0, "multidrop: frame");
    mb_frame(b, "TX", BT, 10, frame);
    check(strcmp(frame, "001000010X1") == 0, "multidrop without ForceParity: 9th bit X");
    mb_run(b, BT * 4);
    mx_w(b, 0x0A, 0x00);
    mx_w(b, 0x0B, 0x03);
    mb_close(b);

    /* GPIAddr: the station address is XOFF2[7:4] with GPIO3..0. */
    b = mx_open();
    mx_setup(b);
    mb_set(b, "GPIO1", LOGIC_1);
    mx_w(b, 0x17, 0x40);
    mx_w(b, 0x13, 0x04);
    mx_w(b, 0x0B, 0x2B);
    mx_w(b, 0x0A, 0x50);
    mb_run(b, BT);
    mb_drive_bits(b, "RX", "0000001011", BT, LOGIC_1);   /* address 0x40: XOFF2 itself */
    mb_drive_bits(b, "RX", "1000100001", BT, LOGIC_1);   /* data 0x11 */
    mb_drive_bits(b, "RX", "0100001011", BT, LOGIC_1);   /* address 0x42 = 0x4 : GPIO 0010 */
    mb_drive_bits(b, "RX", "0100010001", BT, LOGIC_1);   /* data 0x22 */
    mb_run(b, BT);
    mb_expect_u(mx_r(b, 0x12), 1, "GPIAddr: only data after 0x42 is stored");
    mb_expect_u(mx_r(b, 0x00), 0x22, "GPIAddr: the data word");
    mb_expect_u(mx_r(b, 0x06) & 0x08, 0x00, "MAX3107: no XOFF2Int on its own address");
    case_end("max310x echo suppression, the multidrop 9th bit without ForceParity, GPIAddr");
    mb_close(b);
}

/* ---- SPI: an X address reads X ------------------------------------------------------------- */

static void spi_bits(MB *b, const char *bits, char *text) {
    unsigned i;

    mb_set(b, b->cs, LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; bits[i] != '\0'; i++) {
        mb_set(b, b->mosi, bits[i] == '1' ? LOGIC_1 : bits[i] == '0' ? LOGIC_0 : LOGIC_X);
        mb_run(b, SPI_HALF);
        text[i] = "01XZ"[mb_get(b, b->miso)];
        mb_set(b, b->sclk, LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, b->sclk, LOGIC_0);
    }
    text[i] = '\0';
    mb_run(b, SPI_HALF);
    mb_set(b, b->cs, LOGIC_1);
    mb_run(b, SPI_HALF);
}

static void case_spi_x(void) {
    MB *b = mx_open();
    char text[24];

    case_begin();
    spi_bits(b, "0000101X00000000", text);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "a read with an X address bit reads X");
    mb_expect_u(mx_r(b, 0x0B), 0x05, "an X-address read has no side effect (LCR 0x05)");
    spi_bits(b, "X000101100000000", text);
    check(strcmp(text + 8, "XXXXXXXX") == 0, "an X R/W bit reads X");
    mx_r_text(b, 0x0B, text);
    check(strcmp(text, "XXXXXXXX") == 0, "an X R/W bit may be a write: LCR holds X");
    case_end("max310x SPI: X command bits read X");
    mb_close(b);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_loader();
    case_pins();
    case_loop();
    case_fifo();
    case_flow();
    case_echo_multidrop();
    case_spi_x();
    return serial_bench_finish();
}
