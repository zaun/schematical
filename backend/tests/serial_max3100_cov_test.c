/*
 * serial_max3100_cov_test: the MAX3100 / MAX3140 paths serial_maxim_test
 * does not reach (plan-68-P): IrDA transmit and receive, unknown inputs
 * (SHDN, DIN, a stop bit, H/F), a configuration written while a frame is
 * going out, frames of other than 16 bits, and the loader's rejections.
 * Expected results come from the model's header comment (max3100.pdf,
 * max3140.pdf; plan-12-data/specs/maxim.md § MAX3100, § MAX3140).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"

#include "cJSON.h"

#define M3_HALF NS(271)   /* X1 = 1.8432 MHz */
#define M3_BT (16u * 2 * M3_HALF)

static MB *m3_open(const char *part) {
    MB *b = mb_open(part, "X1", M3_HALF, NULL, NULL);

    b->cs = "CS";
    b->sclk = "SCLK";
    b->mosi = "DIN";
    b->miso = "DOUT";
    mb_set(b, "CS", LOGIC_1);
    mb_set(b, "SCLK", LOGIC_0);
    mb_set(b, "DIN", LOGIC_0);
    mb_set(b, "SHDN", LOGIC_1);
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_1);
    mb_run(b, NS(2000));
    return b;
}

static unsigned m3_word(MB *b, unsigned din) {
    return mb_spi(b, din, 16, NULL, 0);
}

/* One CS-low frame with DIN from `bits` ('0', '1', 'X'), MSB first; DOUT
   sampled before each rising edge into `text` ("01XZ"). */
static void m3_xframe(MB *b, const char *bits, char *text) {
    size_t i;

    mb_set(b, "CS", LOGIC_0);
    mb_run(b, SPI_HALF);
    for (i = 0; bits[i] != '\0'; i++) {
        mb_set(b, "DIN", bits[i] == '1' ? LOGIC_1 : bits[i] == '0' ? LOGIC_0 : LOGIC_X);
        mb_run(b, SPI_HALF);
        if (text != NULL) {
            text[i] = "01XZ"[mb_get(b, "DOUT")];
        }
        mb_set(b, "SCLK", LOGIC_1);
        mb_run(b, SPI_HALF);
        mb_set(b, "SCLK", LOGIC_0);
    }
    if (text != NULL) {
        text[i] = '\0';
    }
    mb_set(b, "DIN", LOGIC_0);
    mb_run(b, SPI_HALF);
    mb_set(b, "CS", LOGIC_1);
    mb_run(b, SPI_HALF);
}

static void expect_text(const char *got, const char *want, const char *what) {
    char message[200];

    snprintf(message, sizeof message, "%s: got %s, want %s", what, got, want);
    check(strcmp(got, want) == 0, message);
}

/* ---- IrDA ----------------------------------------------------------------------------------- */

static void case_irda(void) {
    MB *b = m3_open("SFX-MAX3100-qsop16");
    char frame[24];
    long long t;
    unsigned w;
    unsigned i;
    static const char bits[] = "01111000011";   /* start, 0x0F LSB first, stop */

    case_begin();
    /* IR = 1 (bit 7), 8N1, BRD 1; RX idles low in IrDA mode. */
    mb_set(b, "RX", LOGIC_0);
    m3_word(b, 0xC000 | 0x0080);
    mb_run(b, M3_BT * 24);
    m3_word(b, 0xC000 | 0x0080);   /* clears what the idle switch left */
    mb_expect_u(m3_word(b, 0x0000) & 0x8000, 0, "IrDA: RX idling low receives nothing");
    /* A 0 bit is a TX low pulse of 3/16 bit, TX high for the rest. */
    m3_word(b, 0x8000 | 0x00);
    t = mb_wait(b, "TX", LOGIC_0, M3_BT * 4, NS(10));
    check(t >= 0, "IrDA: the start bit's pulse");
    t = mb_wait(b, "TX", LOGIC_1, M3_BT, NS(10));
    check(within(t, (long long)(3 * 2 * M3_HALF), NS(30)), "IrDA: the pulse is 3/16 of a bit");
    mb_run(b, M3_BT / 2);
    mb_expect_pin(b, "TX", LOGIC_1, "IrDA: TX high after the pulse within a 0 bit");
    mb_run(b, M3_BT * 10);
    /* Receive: a high pulse of 3/16 bit is a 0 bit; low is a 1. */
    for (i = 0; bits[i] != '\0'; i++) {
        if (bits[i] == '0') {
            mb_set(b, "RX", LOGIC_1);
            mb_run(b, 3 * 2 * M3_HALF);
            mb_set(b, "RX", LOGIC_0);
            mb_run(b, M3_BT - 3 * 2 * M3_HALF);
        } else {
            mb_run(b, M3_BT);
        }
    }
    mb_run(b, M3_BT * 2);
    w = m3_word(b, 0x0000);
    mb_expect_u(w & 0x84FF, 0x800F, "IrDA: the pulses decode to 0x0F, R = 1, no FE");
    /* RX X for a whole bit (bit 0): that data bit is unknown. */
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, 3 * 2 * M3_HALF);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, M3_BT - 3 * 2 * M3_HALF);
    mb_set(b, "RX", LOGIC_X);
    mb_run(b, M3_BT);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, M3_BT * 10);
    mb_spi(b, 0x0000, 16, frame, 0);
    check(frame[0] == '1' && frame[15] == 'X' && frame[14] == '1',
          "IrDA: RX X for bit 0 gives D0 X, the 1 bits (low) known");
    case_end("max3100 IrDA: 3/16-bit TX pulses, RX high pulses decoded as 0 bits");
    mb_close(b);
}

/* ---- Unknown inputs ------------------------------------------------------------------------- */

static void case_unknown(void) {
    MB *b = m3_open("SFX-MAX3100-qsop16");
    char text[32];
    char frame[24];
    unsigned i;

    case_begin();
    /* SHDN X: the whole device is unknown: IRQ, TX X; DOUT reads X. */
    mb_set(b, "SHDN", LOGIC_X);
    mb_run(b, NS(500));
    mb_expect_pin(b, "IRQ", LOGIC_X, "SHDN X: IRQ X");
    mb_expect_pin(b, "TX", LOGIC_X, "SHDN X: TX X");
    m3_xframe(b, "0100000000000000", text);
    expect_text(text, "XXXXXXXXXXXXXXXX", "SHDN X: Read Config DOUT all X");
    mb_set(b, "SHDN", LOGIC_1);
    mb_run(b, NS(500));
    mb_expect_pin(b, "TX", LOGIC_1, "SHDN high again: TX idles high");
    /* Write Config with TM (bit 11) unknown: T = 1 after it, so IRQ = T and TM = X. */
    m3_xframe(b, "1100X00000000000", NULL);
    mb_expect_pin(b, "IRQ", LOGIC_X, "TM unknown with T = 1: IRQ X");
    m3_word(b, 0xC000);
    mb_expect_pin(b, "IRQ", LOGIC_Z, "TM = 0 again: IRQ released");
    /* An unknown operation (DIN bit 15 X): DOUT X after the op bits, and every
       register it could have written goes X (RTS among them). */
    m3_xframe(b, "X100000000000000", text);
    check(text[2] == 'X' && text[7] == 'X', "unknown op: DOUT bits 13..8 X");
    mb_expect_pin(b, "RTS", LOGIC_X, "unknown op: RTS X");
    m3_xframe(b, "0100000000000000", text);
    /* Bit 12 is SHDNo, the device's own shutdown state: still known. */
    expect_text(text + 2, "X0XXXXXXXXXXXX", "unknown op: Read Config bits 13, 11..0 X");
    m3_word(b, 0xC000);
    m3_word(b, 0x8000 | 0x0400);   /* TE = 1: only RTS (bit 9 = 0: pin high) */
    mb_expect_pin(b, "RTS", LOGIC_1, "Write Data TE = 1, RTS bit 0: RTS high again");
    mb_run(b, M3_BT * 3);
    mb_expect_pin(b, "TX", LOGIC_1, "TE = 1: nothing sent");
    /* Write Data with TE unknown: the buffer may have been loaded, so it is
       full of an unknown word: T = 0 and the frame's data bits are X. */
    m3_xframe(b, "10000X0001000001", NULL);
    check(mb_wait(b, "TX", LOGIC_0, M3_BT * 4, NS(20)) >= 0, "TE unknown: a frame starts");
    mb_frame(b, "TX", M3_BT, 9, frame);
    expect_text(frame, "0XXXXXXXX1", "TE unknown: the data bits are X");
    /* A stop bit that is X: FE unknown, so RA/FE (DOUT bit 10) reads X. */
    mb_run(b, M3_BT * 2);
    m3_word(b, 0xC000);
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, M3_BT);
    for (i = 0; i < 8; i++) {
        mb_set(b, "RX", (0x5Au >> i) & 1u ? LOGIC_1 : LOGIC_0);
        mb_run(b, M3_BT);
    }
    mb_set(b, "RX", LOGIC_X);
    mb_run(b, M3_BT);
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, M3_BT * 2);
    mb_spi(b, 0x0000, 16, text, 0);
    check(text[5] == 'X', "stop bit X: RA/FE (bit 10) reads X");
    case_end("max3100 unknowns: SHDN X, TM X, unknown op, TE X, a stop bit X");
    mb_close(b);
}

/* ---- Configuration while sending; frames of other than 16 bits ------------------------------- */

static void case_pending(void) {
    MB *b = m3_open("SFX-MAX3100-qsop16");
    char frame[24];
    char text[32];
    long long len;
    unsigned w;

    case_begin();
    m3_word(b, 0xC000);
    m3_word(b, 0x8000 | 0x55);
    check(mb_wait(b, "TX", LOGIC_0, M3_BT * 4, NS(20)) >= 0, "frame of 0x55 starts");
    /* Write Config B = 0001 (BRD 2) mid-frame: it waits for the transmitter. */
    m3_word(b, 0xC000 | 0x0001);
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x3FFF, 0x0001, "Read Config shows the written configuration at once");
    mb_frame(b, "TX", M3_BT, 9, frame);
    check(frame[1] == '1' && frame[2] == '0' && frame[9] == '1',
          "the frame in flight keeps its BRD 1 bit time");
    mb_run(b, M3_BT * 2);
    m3_word(b, 0x8000 | 0x01);
    len = m3_start_bit(b, "TX", M3_BT * 8);
    check(within(len, (long long)(2 * M3_BT), (long long)(2 * M3_HALF)),
          "after it: B = 0001 bit time 16 x 2 X1 periods");
    mb_run(b, M3_BT * 2 * 11);
    /* A 24-bit frame is aborted: no Write Data, the 3rd byte of DOUT undefined. */
    mb_spi(b, ((uint32_t)0x80 << 16) | ((uint32_t)0x41 << 8), 24, text, 0);
    check(text[16] == 'X', "past the 16-bit word: DOUT undefined (X)");
    check(mb_wait(b, "TX", LOGIC_0, M3_BT * 2 * 3, NS(50)) < 0, "24-bit Write Data: nothing sent");
    w = m3_word(b, 0x4000);
    mb_expect_u(w & 0x4000, 0x4000, "24-bit Write Data: T stays 1");
    case_end("max3100: Write Config waits for the frame in flight; a 24-bit frame is aborted");
    mb_close(b);
}

/* ---- MAX3140 H/F unknown -------------------------------------------------------------------- */

static void case_hf_x(void) {
    MB *b = m3_open("SFX-MAX3140-qsop28");

    case_begin();
    mb_set(b, "DE", LOGIC_0);
    mb_set(b, "RE", LOGIC_0);
    mb_set(b, "H/F", LOGIC_X);
    mb_set(b, "A", LOGIC_1);
    mb_set(b, "B", LOGIC_0);
    mb_run(b, NS(5000));
    mb_expect_pin(b, "RO", LOGIC_1, "H/F X: A/B = 1 and Y/Z open (1) agree: RO 1");
    mb_set(b, "A", LOGIC_0);
    mb_set(b, "B", LOGIC_1);
    mb_run(b, NS(5000));
    mb_expect_pin(b, "RO", LOGIC_X, "H/F X: A/B = 0 and Y/Z open (1) differ: RO X");
    case_end("max3140 H/F X: RO is the pair reading when both pairs agree, else X");
    mb_close(b);
}

/* ---- Loader rejections ---------------------------------------------------------------------- */

static int cov_counter;

static int apply_edit(cJSON *root, const char *path, const cJSON *value) {
    char buf[160];
    char *key;
    char *dot;
    cJSON *at = root;
    int del = cJSON_IsString(value) && strcmp(value->valuestring, "-") == 0;

    snprintf(buf, sizeof buf, "%s", path);
    key = buf;
    while ((dot = strchr(key, '.')) != NULL) {
        *dot = '\0';
        at = cJSON_IsArray(at) ? cJSON_GetArrayItem(at, atoi(key))
                               : cJSON_GetObjectItemCaseSensitive(at, key);
        if (at == NULL) {
            return -1;
        }
        key = dot + 1;
    }
    if (del) {
        cJSON_DeleteItemFromObjectCaseSensitive(at, key);
        return 0;
    }
    if (cJSON_GetObjectItemCaseSensitive(at, key) != NULL) {
        return cJSON_ReplaceItemInObjectCaseSensitive(at, key, cJSON_Duplicate(value, 1)) ? 0 : -1;
    }
    cJSON_AddItemToObject(at, key, cJSON_Duplicate(value, 1));
    return 0;
}

/* Loads an edited copy of fixture device `id`; expects the loader to refuse it
   with a message containing `want`. */
static void reject(const char *id, const char *edits, const char *want) {
    cJSON *file = cJSON_Parse(SERIAL_FIXTURE_MAXIM);
    cJSON *ed = cJSON_Parse(edits);
    cJSON *dev = NULL;
    cJSON *item;
    cJSON *out;
    char name[64];
    char what[600];
    char *json;
    char *error = NULL;
    int status;

    maxim_need();
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(file, "devices")) {
        if (strcmp(cJSON_GetObjectItemCaseSensitive(item, "id")->valuestring, id) == 0) {
            dev = cJSON_Duplicate(item, 1);
        }
    }
    if (dev == NULL || ed == NULL) {
        fprintf(stderr, "reject: no device %s or bad edits %s\n", id, edits);
        exit(2);
    }
    cJSON_ArrayForEach(item, ed) {
        if (apply_edit(dev, item->string, item) != 0) {
            fprintf(stderr, "reject: cannot apply %s\n", item->string);
            exit(2);
        }
    }
    snprintf(name, sizeof name, "%s-P%d", id, cov_counter);
    cJSON_ReplaceItemInObjectCaseSensitive(dev, "id", cJSON_CreateString(name));
    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(dev, "parts")) {
        snprintf(name, sizeof name, "%s-P%d", cJSON_GetObjectItemCaseSensitive(item, "name")->valuestring,
                 cov_counter);
        cJSON_ReplaceItemInObjectCaseSensitive(item, "name", cJSON_CreateString(name));
    }
    cov_counter++;
    out = cJSON_CreateObject();
    cJSON_AddStringToObject(out, "format", "schematical-parts");
    cJSON_AddNumberToObject(out, "version", 1);
    cJSON_AddStringToObject(out, "kind", "serial");
    cJSON_AddItemToArray(cJSON_AddArrayToObject(out, "devices"), dev);
    json = cJSON_PrintUnformatted(out);
    status = load_parts(library, json, "max3100_cov.json", &error);
    snprintf(what, sizeof what, "%s %s: refused with \"%s\" (got: %s)", id, edits, want,
             status == 0 ? "loaded" : (error != NULL ? error : "(null)"));
    check(status != 0 && error != NULL && strstr(error, want) != NULL, what);
    free(error);
    cJSON_free(json);
    cJSON_Delete(out);
    cJSON_Delete(ed);
    cJSON_Delete(file);
}

static void case_loader(void) {
    static const char *const M3 = "SFX-MAX3100";
    static const char *const M4 = "SFX-MAX3140";

    case_begin();
    reject(M3, "{\"bus\":\"-\"}", "\"bus\"");
    reject(M3, "{\"bus\":{\"style\":\"bogus\"}}", "unknown bus style");
    reject(M3, "{\"bus.mode\":1}", "the max3100 host port is \"spi\" mode 0");
    reject(M3, "{\"clock\":\"-\"}", "\"clock\"");
    reject(M3, "{\"clock.bogus\":\"X1\"}", "bogus");
    reject(M3, "{\"channels\":\"-\"}", "\"channels\"");
    reject(M3, "{\"channels\":[]}", "\"channels\" must be an array of 1 channel");
    reject(M3, "{\"channels.0.bogus\":\"TX\"}", "bogus");
    reject(M3, "{\"channels.0.tx\":\"-\"}", "\"tx\"");
    reject(M3, "{\"shutdown\":\"-\"}", "\"shutdown\"");
    reject(M3, "{\"testMode\":\"bogus\"}", "\"testMode\" is \"loopback\" or \"txLow\"");
    reject(M4, "{\"transceiver.bogus\":\"Y\"}", "bogus");
    reject(M4, "{\"transceiver.y\":\"-\"}", "\"y\"");
    reject(M3, "{\"bus.cs\":\"NOPE\"}", "NOPE");
    reject(M3, "{\"clock.in\":\"NOPE\"}", "NOPE");
    reject(M3, "{\"channels.0.tx\":\"NOPE\"}", "NOPE");
    reject(M4, "{\"transceiver.y\":\"NOPE\"}", "NOPE");
    case_end("max3100 loader: every rejection");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_irda();
    case_unknown();
    case_pending();
    case_hf_x();
    case_loader();
    return serial_bench_finish();
}
