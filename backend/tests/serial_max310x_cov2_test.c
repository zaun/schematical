/*
 * serial_max310x_cov2_test (plan-68-P): the max310x model's multi-UART and
 * clocking paths past serial_max310x_test: MODE2 software reset per UART,
 * transmit triggers (TrigDelay, TxAutoDis), XOFF2Int on the own multidrop
 * address, the PLL (fractional period, CLKtoGPIO, CLKtoRTS, a changing or
 * stopped XIN), sleep (forced per UART, AutoSleep and waking), the I2C
 * address and pointer edges, the MAX3108 TxDisabl read and the MAX14830
 * RTSInvert. Expected values: serial_max310x_int.h and
 * plan-12-data/specs/maxim.md (§ MAX3107 p22-p40, § MAX3109 p21-p56,
 * § MAX14830 p43).
 */

#include "serial_bench.h"
#include "serial_bench_maxim.h"

#include "cJSON.h"

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

static const char *const PULLS_B[] = {"SCLK/SCL", "MISO/SDA", NULL};
static const char *const PULLS_3107[] = {"SCLK/SCL", "DOUT/SDA", NULL};

/* A max310x part: SPI (spi = 1) or I2C, XIN from the bench clock (clock = 1)
   or left to the test (clock = 0). */
static MB *mx_open(const char *part, int spi, int clock) {
    int b_names = strstr(part, "MAX3107") == NULL;
    MB *b = mb_open(part, clock ? "XIN" : NULL, MX_HALF, NULL,
                    spi ? NULL : b_names ? PULLS_B : PULLS_3107);
    unsigned u;

    b->cs = "CS/A0";
    b->sclk = "SCLK/SCL";
    b->mosi = b_names ? "MOSI/A1" : "DIN/A1";
    b->miso = b_names ? "MISO/SDA" : "DOUT/SDA";
    b->scl = "SCLK/SCL";
    b->sda = b->miso;
    mb_set(b, b_names ? "SPI/I2C" : "I2C/SPI", spi ? LOGIC_1 : LOGIC_0);
    mb_set(b, "RST", LOGIC_1);
    if (spi) {
        mb_set(b, "CS/A0", LOGIC_1);
        mb_set(b, "SCLK/SCL", LOGIC_0);
        mb_set(b, b->mosi, LOGIC_0);
    }
    if (!clock) {
        mb_set(b, "XIN", LOGIC_0);
    }
    for (u = 0; b_names && u < 4; u++) {
        char rx[8];
        char cts[8];
        size_t i;

        snprintf(rx, sizeof rx, "RX%u", u);
        snprintf(cts, sizeof cts, "CTS%u", u);
        for (i = 0; i < b->part->pin_count; i++) {
            if (strcmp(b->part->pins[i].name, rx) == 0) {
                mb_set(b, rx, LOGIC_1);
                mb_set(b, cts, LOGIC_0);
            }
        }
    }
    mb_run(b, NS(2000));
    return b;
}

static void mx_w(MB *b, unsigned cmd, unsigned value) {
    mb_spi(b, ((0x80u | cmd) << 8) | value, 16, NULL, 0);
}

static unsigned mx_r(MB *b, unsigned cmd) {
    return mb_spi(b, cmd << 8, 16, NULL, 0) & 0xFF;
}

/* UART u of a MAX3109 (bit 5 of the command). */
#define U1 0x20u

/* ---- MODE2 RST, extended map ----------------------------------------------------------------- */

static void case_soft_reset(void) {
    MB *b = mx_open("SFX-MAX3109-tqfn32", 1, 1);

    case_begin();
    mx_w(b, 0x0B, 0x03);
    mx_w(b, U1 | 0x0B, 0x03);
    /* MODE2[0] RST on UART1 only (MAX3109: "the addressed UART"). */
    mx_w(b, U1 | 0x0A, 0x01);
    mb_expect_u(mx_r(b, U1 | 0x0B), 0x05, "UART1 RST: LCR back to 0x05");
    mb_expect_u(mx_r(b, 0x0B), 0x03, "UART0 untouched");
    mb_expect_u(mx_r(b, U1 | 0x0A), 0x01, "UART1 MODE2 reads RST");
    mx_w(b, U1 | 0x0B, 0x07);
    mb_expect_u(mx_r(b, U1 | 0x0B), 0x05, "held in reset: register writes are ignored");
    mx_w(b, U1 | 0x0A, 0x01);
    mb_expect_u(mx_r(b, U1 | 0x0A), 0x01, "RST written again: still held");
    mx_w(b, U1 | 0x0A, 0x00);
    mb_expect_u(mx_r(b, U1 | 0x0A), 0x00, "RST written back to 0: released");
    mx_w(b, U1 | 0x0B, 0x07);
    mb_expect_u(mx_r(b, U1 | 0x0B), 0x07, "released: LCR writes again");
    /* RevID (0x25) is read only. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x05, 0x00);
    mb_expect_u(mx_r(b, 0x05), 0xC2, "RevID ignores a write");
    mx_w(b, 0x1F, 0xCD);
    mb_close(b);

    /* MAX3107: MODE2[0] resets the (only) UART. */
    b = mx_open("SFX-MAX3107-ssop24", 1, 1);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x0A, 0x01);
    mb_expect_u(mx_r(b, 0x0B), 0x05, "MAX3107 RST: LCR 0x05");
    mx_w(b, 0x0A, 0x00);
    mb_expect_u(mx_r(b, 0x0A), 0x00, "MAX3107 RST released");
    case_end("max310x MODE2 RST: per UART on the MAX3109, writes ignored while held, RevID read only");
    mb_close(b);
}

/* ---- Triggers, auto disable, multidrop XOFF2Int, the fast read with X --------------------- */

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

static void case_sync(void) {
    MB *b = mx_open("SFX-MAX3109-tqfn32", 1, 1);
    char text[24];
    long long t;

    case_begin();
    mx_w(b, 0x0B, 0x03);
    mx_w(b, U1 | 0x0B, 0x03);
    /* The fast read with an X R/W bit: the IRQ bits are X (it may be a read). */
    spi_bits(b, "X0000000", text);
    check(strcmp(text, "ZZZZXXXX") == 0, "X R/W bit: clocks 5-8 X");
    /* TxAutoDis (TxSynch[6]): TxDisabl set when the TX FIFO drains. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x00, 0x40);
    mx_w(b, 0x1F, 0xCD);
    mx_w(b, 0x00, 0x31);
    mb_run(b, BT * 12);
    mb_expect_u(mx_r(b, 0x09) & 0x02, 0x02, "TxAutoDis: TxDisabl after the FIFO drained");
    mx_w(b, 0x00, 0x32);
    check(mb_wait(b, "TX0", LOGIC_0, BT * 3, NS(50)) < 0, "TxAutoDis: the next word waits");
    mb_expect_u(mx_r(b, 0x11), 1, "TxAutoDis: the word stays in the FIFO");
    mx_w(b, 0x09, 0x00);
    check(mb_wait(b, "TX0", LOGIC_0, BT * 3, NS(50)) >= 0, "TxDisabl cleared: it goes");
    mb_run(b, BT * 12);
    /* TrigDelay (TxSynch[5]): SynchDelay = 4 bit times after the trigger. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x00, 0x00);
    mx_w(b, U1 | 0x00, 0x33);
    mx_w(b, U1 | 0x01, 0x04);
    mx_w(b, U1 | 0x02, 0x00);
    mx_w(b, 0x1F, 0xCD);
    mx_w(b, U1 | 0x00, 0x44);
    mx_w(b, 0x1F, 0xE3);
    t = mb_wait(b, "TX1", LOGIC_0, BT * 8, NS(50));
    check(t >= (long long)(4 * BT) && t <= (long long)(5 * BT),
          "TrigDelay: the start bit SynchDelay (4) bit times after the trigger");
    mb_run(b, BT * 12);
    /* Multidrop address filter: the MAX3109 also raises XOFF2Int on its address. */
    mx_w(b, 0x17, 0x42);
    mx_w(b, 0x0B, 0x2B);
    mx_w(b, 0x0A, 0x50);
    mb_drive_bits(b, "RX0", "0100001011", BT, LOGIC_1);
    mb_run(b, BT);
    mb_expect_u(mx_r(b, 0x06) & 0x28, 0x28, "own address: MultiDropInt and XOFF2Int");
    case_end("max310x MAX3109: fast read with X, TxAutoDis, TrigDelay, XOFF2Int on the own address");
    mb_close(b);
}

/* ---- The PLL ------------------------------------------------------------------------------------- */

/* Rising edges of `pin` in `span` (sampled every `step`). */
static unsigned rises(MB *b, const char *pin, SimTime span, SimTime step) {
    LogicValue last = mb_get(b, pin);
    unsigned n = 0;
    SimTime t;

    for (t = 0; t < span; t += step) {
        LogicValue v;

        mb_run(b, step);
        v = mb_get(b, pin);
        n += last == LOGIC_0 && v == LOGIC_1;
        last = v;
    }
    return n;
}

static void case_pll(void) {
    MB *b = mx_open("SFX-MAX3109-tqfn32", 1, 1);
    SimTime p = 2 * MX_HALF;
    long long t;
    unsigned n;
    char message[160];

    case_begin();
    /* CLKtoGPIO set before the PLL: the reference starts once XIN is measured. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x00, 0x80);
    mx_w(b, 0x1F, 0xCD);
    /* PLL x6, PreDiv 1: fREF = 6 x XIN; DIV 1: 16 ticks per bit (not whole ps). */
    mx_w(b, 0x1A, 0x01);
    mx_w(b, 0x1E, 0x04);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x1C, 0x01);
    mx_w(b, U1 | 0x1C, 0x00);   /* UART1 DIV = 0: stopped */
    mx_w(b, U1 | 0x0B, 0x03);
    mb_run(b, 4 * p);
    n = rises(b, "GPIO0", 10 * p, NS(5));
    snprintf(message, sizeof message, "CLKtoGPIO: %u reference cycles in 10 XIN periods, want 60", n);
    check(n >= 59 && n <= 61, message);
    mx_w(b, 0x00, 0x01);
    t = m3_start_bit(b, "TX0", 40 * p);
    snprintf(message, sizeof message, "PLL x6 / 1: bit %lld ps, want %llu", t, (unsigned long long)(16 * p / 6));
    check(within(t, (long long)(16 * p / 6), NS(15)), message);
    mx_w(b, U1 | 0x00, 0x01);
    check(mb_wait(b, "TX1", LOGIC_0, 40 * p, NS(20)) < 0, "UART1 DIV = 0: no clock, nothing sent");
    /* CLKtoGPIO off, then on again while the PLL runs. */
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x00, 0x00);
    mb_run(b, 2 * p);
    n = rises(b, "GPIO0", 4 * p, NS(5));
    check(n == 0, "CLKtoGPIO off: GPIO0 still");
    mx_w(b, 0x00, 0x80);
    mx_w(b, 0x1F, 0xCD);
    n = rises(b, "GPIO0", 10 * p, NS(5));
    check(n >= 59 && n <= 61, "CLKtoGPIO on again: the reference on GPIO0");
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x00, 0x00);
    mx_w(b, 0x1F, 0xCD);
    /* CLKtoRTS with the PLL (x6 / 6: fREF = XIN), DIV 2: RTS0 at fREF / 2. */
    mx_w(b, 0x1A, 0x06);
    mx_w(b, 0x1C, 0x02);
    mx_w(b, 0x1E, 0x84);
    mb_run(b, 4 * p);
    n = rises(b, "RTS0", 20 * p, NS(10));
    snprintf(message, sizeof message, "CLKtoRTS with the PLL: %u cycles in 20 XIN periods, want 10", n);
    check(n >= 9 && n <= 11, message);
    mx_w(b, 0x1E, 0x04);
    case_end("max310x PLL: fractional tick, CLKtoGPIO before and while it runs, CLKtoRTS, DIV 0");
    mb_close(b);
}

/* XIN driven by the test: a changing period and a stopped clock. */
static void xin_cycles(MB *b, SimTime half, unsigned n) {
    unsigned i;

    for (i = 0; i < n; i++) {
        mb_set(b, "XIN", LOGIC_1);
        mb_run(b, half);
        mb_set(b, "XIN", LOGIC_0);
        mb_run(b, half);
    }
}

static void case_xin(void) {
    MB *b;
    uint16_t log[4];
    size_t n;

    b = mb_open("SFX-MAX3109-tqfn32", NULL, MX_HALF, NULL, NULL);
    b->cs = "CS/A0";
    b->sclk = "SCLK/SCL";
    b->mosi = "MOSI/A1";
    b->miso = "MISO/SDA";
    mb_set(b, "SPI/I2C", LOGIC_1);
    mb_set(b, "RST", LOGIC_1);
    mb_set(b, "CS/A0", LOGIC_1);
    mb_set(b, "SCLK/SCL", LOGIC_0);
    mb_set(b, "MOSI/A1", LOGIC_0);
    mb_set(b, "RX0", LOGIC_1);
    mb_set(b, "RX1", LOGIC_1);
    mb_set(b, "CTS0", LOGIC_0);
    mb_set(b, "CTS1", LOGIC_0);
    mb_set(b, "XIN", LOGIC_0);
    mb_run(b, NS(2000));
    case_begin();
    /* PLL x6 / 6 (fREF = XIN), DIV 1; CLKtoGPIO on. */
    mx_w(b, 0x1A, 0x06);
    mx_w(b, 0x1E, 0x04);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x1F, 0xCE);
    mx_w(b, 0x00, 0x80);
    mx_w(b, 0x1F, 0xCD);
    xin_cycles(b, MX_HALF, 4);
    mx_w(b, 0x00, 0x55);
    /* Half a frame at 1.8432 MHz, then XIN at half the rate: the frame
       stretches (the tick in flight is rescaled), it still completes. */
    xin_cycles(b, MX_HALF, 16 * 5);
    xin_cycles(b, 2 * MX_HALF, 16 * 6);
    n = mb_sent(b, 0, log, 4);
    check(n == 1 && log[0] == 0x55, "XIN slowed mid-frame: the frame completes");
    /* XIN stops mid-frame: the PLL stops with it, then resumes with XIN. */
    mx_w(b, 0x00, 0x66);
    xin_cycles(b, MX_HALF, 16 * 4);
    mb_run(b, BT * 20);
    check(mb_sent(b, 0, log, 4) == 1, "XIN stopped: the frame does not finish");
    check(rises(b, "GPIO0", 4 * BT, NS(50)) == 0, "XIN stopped: the CLKtoGPIO reference stops too");
    xin_cycles(b, MX_HALF, 16 * 12);
    n = mb_sent(b, 0, log, 4);
    check(n == 2 && log[1] == 0x66, "XIN back: the frame finishes");
    case_end("max310x PLL on a test-driven XIN: rescale when the period changes, stop with XIN");
    mb_close(b);
}

/* ---- Sleep ------------------------------------------------------------------------------------------ */

static void case_sleep(void) {
    MB *b = mx_open("SFX-MAX3109-tqfn32", 1, 1);

    case_begin();
    mx_w(b, 0x0B, 0x03);
    mx_w(b, U1 | 0x0B, 0x03);
    /* UART1 ForcedSleep: its clock stops (UART0 keeps the reference). */
    mx_w(b, U1 | 0x09, 0x20);
    mb_set(b, "GPIO4", LOGIC_1);
    mb_run(b, BT);
    mb_expect_u(mx_r(b, U1 | 0x19) & 0x10, 0x00, "UART1 asleep: its GPIO inputs are not sampled");
    mx_w(b, U1 | 0x00, 0x41);
    check(mb_wait(b, "TX1", LOGIC_0, BT * 3, NS(50)) < 0, "UART1 asleep: nothing sent");
    mx_w(b, 0x00, 0x42);
    check(mb_wait(b, "TX0", LOGIC_0, BT * 3, NS(50)) >= 0, "UART0 awake: it sends");
    mb_run(b, BT * 12);
    mx_w(b, U1 | 0x09, 0x00);
    check(mb_wait(b, "TX1", LOGIC_0, BT * 3, NS(50)) >= 0, "UART1 awake: it sends");
    mb_run(b, BT * 12);
    mb_set(b, "GPIO4", LOGIC_0);
    mb_run(b, BT);
    mb_set(b, "GPIO4", LOGIC_1);
    mb_run(b, BT);
    mb_expect_u(mx_r(b, U1 | 0x19) & 0x10, 0x10, "UART1 awake: GPIO4 sampled");
    mb_close(b);

    /* AutoSleep (MAX3107): quiet for 65 536 character times, then asleep;
       activity on RX wakes it (p22). PLL x144 / 1 keeps the run short:
       4x mode, 5N1 (7 bits): 65 536 x 7 x 4 ticks. */
    b = mx_open("SFX-MAX3107-ssop24", 1, 1);
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_0);
    mx_w(b, 0x1A, 0xC1);
    mx_w(b, 0x1E, 0x14);
    mx_w(b, 0x1B, 0x20);
    mx_w(b, 0x0B, 0x00);
    mx_w(b, 0x1C, 0x01);
    mx_w(b, 0x09, 0x40);
    mb_run(b, NS(3000000));
    mb_expect_u(mx_r(b, 0x09) & 0x20, 0x00, "AutoSleep: awake before 65 536 character times");
    mb_run(b, NS(5000000));
    mb_expect_u(mx_r(b, 0x09) & 0x20, 0x20, "AutoSleep: asleep after 65 536 quiet character times");
    mb_expect_u(mx_r(b, 0x08) & 0x40, 0x40, "AutoSleep: SleepInt");
    mb_set(b, "RX", LOGIC_0);
    mb_run(b, NS(1000));
    mb_set(b, "RX", LOGIC_1);
    mb_run(b, NS(1000));
    mb_expect_u(mx_r(b, 0x09) & 0x20, 0x00, "RX activity wakes it");
    mb_expect_u(mx_r(b, 0x08) & 0x40, 0x00, "awake: no SleepInt");
    case_end("max310x sleep: one MAX3109 UART forced asleep, MAX3107 AutoSleep and wake-up on RX");
    mb_close(b);
}

/* ---- I2C edges -------------------------------------------------------------------------------------- */

/* One I2C bit with the master's SDA at `level` (0, Z = 1, or X). */
static LogicValue i2c_bit_v(MB *b, LogicValue level) {
    LogicValue v;

    mb_set(b, b->sda, level);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 1);
    mb_run(b, I2C_Q);
    v = mb_get(b, b->sda);
    mb_run(b, I2C_Q);
    i2c_line(b, b->scl, 0);
    mb_run(b, I2C_Q);
    return v;
}

/* A byte from text ("01X", MSB first); the ninth bit as the target left it. */
static LogicValue i2c_write_text(MB *b, const char *bits) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        i2c_bit_v(b, bits[i] == '1' ? LOGIC_Z : bits[i] == '0' ? LOGIC_0 : LOGIC_X);
    }
    return i2c_bit_v(b, LOGIC_Z);
}

static void case_i2c(void) {
    MB *b = mx_open("SFX-MAX3107-tqfn24-3.5", 0, 1);
    char text[12];
    unsigned a;
    unsigned acks;
    char *error = NULL;

    case_begin();
    /* A1 = VL, A0 = DGND: 0x2E (serial_max310x_test J3). */
    mb_set(b, "DIN/A1", LOGIC_1);
    mb_set(b, "CS/A0", LOGIC_0);
    mb_run(b, NS(2000));
    /* An address byte with an X bit that could be ours: the ACK is X. */
    i2c_start(b);
    check(i2c_write_text(b, "0101110X") == LOGIC_X, "X R/W bit on our address: ACK X");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_write_text(b, "010111X0") == LOGIC_X, "an X address bit that could match: ACK X");
    i2c_stop(b);
    i2c_start(b);
    check(i2c_write_text(b, "1X011100") == LOGIC_1, "an X address bit, a known mismatch: NACK");
    i2c_stop(b);
    /* A register address byte with X bits: ACK X. */
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    check(i2c_write_text(b, "000X0100") == LOGIC_X, "an X register address: ACK X");
    i2c_stop(b);
    /* A burst write running past the last register (0x1F): NACK. */
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    check(i2c_write(b, 0x1F) == LOGIC_0, "register 0x1F ACKs");
    check(i2c_write(b, 0x00) == LOGIC_0, "a write to 0x1F (RevID) ACKs");
    check(i2c_write(b, 0x00) == LOGIC_1, "the next register (0x20) does not exist: NACK");
    i2c_stop(b);
    /* An invalid pointer, then Sr: writes NACK, reads are X. */
    i2c_start(b);
    i2c_write(b, 0x2E << 1);
    check(i2c_write(b, 0x20) == LOGIC_1, "register 0x20 NACKs");
    i2c_restart(b);
    i2c_write(b, 0x2E << 1);
    check(i2c_write(b, 0x55) == LOGIC_1, "after Sr with no valid pointer: data NACK");
    i2c_restart(b);
    i2c_write(b, (0x2E << 1) | 1);
    i2c_read(b, 1, text);
    check(strcmp(text, "XXXXXXXX") == 0, "a read with no valid pointer is X");
    i2c_read(b, 0, text);
    i2c_stop(b);
    /* A0 X: no address table entry, the address ACK is X. */
    mb_set(b, "CS/A0", LOGIC_X);
    mb_run(b, NS(2000));
    i2c_start(b);
    check(i2c_write(b, 0x2E << 1) == LOGIC_X, "A0 X: the address ACK is X");
    i2c_stop(b);
    mb_close(b);

    /* A UART whose offset leaves the 7-bit range never answers. */
    check(mx_variant("SFX-MAX3109", "{\"bus.uartOffsets\":[0,-120]}", &error) == 0, "MAX3109 with offsets [0, -120] loads");
    free(error);
    b = mx_open(variant_name, 0, 1);
    mb_set(b, "MOSI/A1", LOGIC_0);
    mb_set(b, "CS/A0", LOGIC_0);
    mb_run(b, NS(2000));
    acks = 0;
    for (a = 0; a < 128; a++) {
        i2c_start(b);
        if (i2c_write(b, a << 1) == LOGIC_0) {
            acks++;
            check(a == 0x6C, "only UART0's 0x6C answers");
        }
        i2c_stop(b);
    }
    mb_expect_u(acks, 1, "one address answers (UART1's 0x6C - 120 is out of range)");
    case_end("max310x I2C: X address and register bytes, a burst past the end, no pointer, A0 X, offsets");
    mb_close(b);
}

/* ---- MAX3108 TxDisabl read, MAX14830 RTSInvert ---------------------------------------------- */

static void case_variants(void) {
    MB *b = mx_open("SFX-MAX3108-wlp25", 1, 1);
    char text[24];

    case_begin();
    mb_set(b, "RX", LOGIC_1);
    mb_set(b, "CTS", LOGIC_0);
    mb_run(b, NS(500));
    mx_w(b, 0x09, 0x10);
    mb_expect_u(mx_r(b, 0x09), 0x12, "MAX3108 TrnscvCtrl with nothing to send: TxDisabl reads 1");
    mx_w(b, 0x09, 0x00);
    mb_expect_u(mx_r(b, 0x09), 0x00, "TrnscvCtrl off: TxDisabl reads 0");
    mb_close(b);

    b = mx_open("SFX-MAX14830-tqfn48", 1, 1);
    mb_expect_pin(b, "RTS0", LOGIC_0, "MAX14830 RTS0 = LCR[7] = 0");
    mx_w(b, 0x0E, 0x04);
    mb_expect_pin(b, "RTS0", LOGIC_1, "IrDA[2] RTSInvert: RTS0 inverted");
    mx_w(b, 0x0E, 0x00);
    mb_expect_pin(b, "RTS0", LOGIC_0, "RTSInvert off");
    /* IrDA with RX X: the decoded line is X; nothing known is stored. */
    mx_w(b, 0x1E, 0x18);
    mx_w(b, 0x0B, 0x03);
    mx_w(b, 0x0E, 0x01);
    mb_set(b, "RX0", LOGIC_X);
    mb_run(b, BT * 12);
    mb_spi(b, 0x0000, 16, text, 0);
    check(strchr(text + 8, '1') == NULL && strchr(text + 8, '0') == NULL,
          "IrDA with RX X: RHR holds no known bit");
    case_end("max310x MAX3108 TxDisabl in auto direction mode, MAX14830 RTSInvert, IrDA RX X");
    mb_close(b);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (serial_bench_load() != 0) {
        return 1;
    }
    case_soft_reset();
    case_sync();
    case_pll();
    case_xin();
    case_sleep();
    case_i2c();
    case_variants();
    return serial_bench_finish();
}
