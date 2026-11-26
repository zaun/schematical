/*
 * serial_hs_com_cov_test (plan-68-P): the 82C52 / HD-6406 (hs82c52) and
 * COM78C80x (com78c80x) cases serial_hs_com_test does not reach: an X
 * reset and what it leaves, an unknown address or ALE, the external
 * divide-by-1 clock, back-to-back characters, an unknown REN, the COM78C80x
 * start rule, and the loaders' rejections (on variants of the fixtures in
 * support/serial_fixture_classic.h).
 */

#include "serial_bench.h"
#include "serial_bench_classic.h"

/* ---- Variants of a fixture device ------------------------------------------------------------- */

static cJSON *fixtures;
static unsigned variant_count;

static cJSON *path_parent(cJSON *root, const char *path, char *last, size_t cap) {
    char buffer[128];
    char *key;
    char *dot;
    cJSON *at = root;

    snprintf(buffer, sizeof buffer, "%s", path);
    key = buffer;
    while ((dot = strchr(key, '.')) != NULL) {
        *dot = '\0';
        at = cJSON_IsArray(at) ? cJSON_GetArrayItem(at, atoi(key))
                               : cJSON_GetObjectItemCaseSensitive(at, key);
        if (at == NULL) {
            return NULL;
        }
        key = dot + 1;
    }
    snprintf(last, cap, "%s", key);
    return at;
}

/* Loads a copy of fixture device `id` with `edits` (dot path, JSON value or
   "-" to delete; NULL-terminated pairs); returns load_parts' result. */
static int load_variant(const char *id, const char *const *edits, char **error) {
    const cJSON *item;
    cJSON *device = NULL;
    cJSON *file;
    cJSON *part;
    char text[64];
    char *json;
    unsigned i;
    int result;

    cJSON_ArrayForEach(item, cJSON_GetObjectItemCaseSensitive(fixtures, "devices")) {
        const cJSON *v = cJSON_GetObjectItemCaseSensitive(item, "id");

        if (strcmp(v->valuestring, id) == 0) {
            device = cJSON_Duplicate(item, 1);
        }
    }
    *error = NULL;
    if (device == NULL) {
        return -2;
    }
    variant_count++;
    snprintf(text, sizeof text, "%s-V%u", id, variant_count);
    cJSON_ReplaceItemInObjectCaseSensitive(device, "id", cJSON_CreateString(text));
    cJSON_ArrayForEach(part, cJSON_GetObjectItemCaseSensitive(device, "parts")) {
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(part, "name");

        snprintf(text, sizeof text, "%s-V%u", name->valuestring, variant_count);
        cJSON_ReplaceItemInObjectCaseSensitive(part, "name", cJSON_CreateString(text));
    }
    for (i = 0; edits[i] != NULL; i += 2) {
        char last[64];
        cJSON *parent = path_parent(device, edits[i], last, sizeof last);

        if (parent == NULL) {
            continue;
        }
        if (cJSON_IsArray(parent)) {
            cJSON_ReplaceItemInArray(parent, atoi(last), cJSON_Parse(edits[i + 1]));
        } else if (strcmp(edits[i + 1], "-") == 0) {
            cJSON_DeleteItemFromObjectCaseSensitive(parent, last);
        } else if (cJSON_GetObjectItemCaseSensitive(parent, last) != NULL) {
            cJSON_ReplaceItemInObjectCaseSensitive(parent, last, cJSON_Parse(edits[i + 1]));
        } else {
            cJSON_AddItemToObject(parent, last, cJSON_Parse(edits[i + 1]));
        }
    }
    file = cJSON_CreateObject();
    cJSON_AddStringToObject(file, "format", "schematical-parts");
    cJSON_AddNumberToObject(file, "version", 1);
    cJSON_AddStringToObject(file, "kind", "serial");
    cJSON_AddItemToArray(cJSON_AddArrayToObject(file, "devices"), device);
    json = cJSON_PrintUnformatted(file);
    cJSON_Delete(file);
    result = load_parts(library, json, "classic_variant.json", error);
    cJSON_free(json);
    return result;
}

/* The variant is refused with a message containing `want`. */
static void reject(const char *id, const char *const *edits, const char *want) {
    char *error = NULL;
    int result = load_variant(id, edits, &error);
    char message[400];

    snprintf(message, sizeof message, "%s refused (%s): %s", id, want,
             error != NULL ? error : "(loaded)");
    check(result != 0 && error != NULL && strstr(error, want) != NULL, message);
    free(error);
}

/* ---- The 82C52 / HD-6406 bench (as serial_hs_com_test) --------------------------------------- */

static const char *const HS_D[8] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"};

#define HS_XHALF AC_XHALF   /* IX 1.8432 MHz */

static void hs_set_addr(Bench *b, unsigned a) {
    set(b, "A0", a & 1u ? LOGIC_1 : LOGIC_0);
    set(b, "A1", a & 2u ? LOGIC_1 : LOGIC_0);
}

/* A register write; the data bits in `xmask` are driven X. */
static void hs_write_x(Bench *b, unsigned a, unsigned v, unsigned xmask) {
    unsigned i;

    hs_set_addr(b, a);
    h_set_bus(b, HS_D, 8, 1, v);
    for (i = 0; i < 8; i++) {
        if ((xmask >> i) & 1u) {
            set(b, HS_D[i], LOGIC_X);
        }
    }
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(200));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "CS0", LOGIC_1);
    h_set_bus(b, HS_D, 8, 0, 0);
    run(b, NS(200));
}

static void hs_write(Bench *b, unsigned a, unsigned v) {
    hs_write_x(b, a, v, 0);
}

static unsigned hs_read(Bench *b, unsigned a, char *text) {
    unsigned v;

    hs_set_addr(b, a);
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    v = h_get_bus(b, HS_D, 8, text);
    set(b, "RD", LOGIC_1);
    run(b, NS(50));
    set(b, "CS0", LOGIC_1);
    run(b, NS(200));
    return v;
}

static void hs_expect(Bench *b, unsigned a, const char *want, const char *what) {
    char text[16];

    hs_read(b, a, text);
    h_expect_text(text, want, what);
}

/* Time of 3 CO periods (falling edge to the third falling edge after it). */
static SimTime hs_co_3(Bench *b, SimTime limit) {
    SimTime spent = 0;
    SimTime t0 = 0;
    LogicValue last = get(b, "CO");
    int falls = 0;
    SimTime step = NS(30);

    while (spent < limit) {
        LogicValue now;

        run(b, step);
        spent += step;
        now = get(b, "CO");
        if (last == LOGIC_1 && now == LOGIC_0) {
            if (falls == 0) {
                t0 = sim_now(b->sim);
            } else if (falls == 3) {
                return sim_now(b->sim) - t0;
            }
            falls++;
        }
        last = now;
    }
    return 0;
}

static HBench *hs_open(const char *part, int hd) {
    static const HClockPin CLOCKS[1] = {{"IX", 0}};
    SimTime half = HS_XHALF;
    HBench *h = hb_open(part, CLOCKS, 1, &half, 1);
    Bench *b = &h->b;

    set(b, "RST", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "RD", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "SDI", LOGIC_1);
    set(b, "CTS", LOGIC_0);
    set(b, "DSR", LOGIC_0);
    hs_set_addr(b, 0);
    if (hd) {
        set(b, "CS1", LOGIC_1);
        set(b, "ALE", LOGIC_1);
        set(b, "TXDACK", LOGIC_1);
        set(b, "RXDACK", LOGIC_1);
        set(b, "SIE", LOGIC_0);
        set(b, "SFD", LOGIC_0);
        set(b, "RLSD", LOGIC_0);
        set(b, "RI", LOGIC_1);
    }
    run(b, NS(3000));
    set(b, "RST", LOGIC_0);
    run(b, NS(1000));
    return h;
}

/* 19200 8N1 (/3 prescaler, divisor 2), INTEN, REN, DTR, RTS. */
static void hs_setup(Bench *b) {
    hs_write(b, 1, 0x3C);
    hs_write(b, 3, 0x81);
    hs_write(b, 2, 0x27);
    hs_read(b, 1, NULL);
}

static size_t hs_sent_count(Bench *b) {
    const SimComponentType *type = sim_instance_type(b->chip);

    return type->serial_sent(sim_instance_state(b->chip), 0, 0, NULL, 0);
}

#define HS_BT (3 * 2 * 16 * 2 * HS_XHALF)   /* 19200 */

/* RST at X: every output X, no cycle, no baud tick; RST low after it: the
   registers RST does not set stay unknown (plan-12-A section 3.5). */
static void case_hs_reset_x(void) {
    HBench *h = hs_open("SFX-HD-6406-dip40", 1);
    Bench *b = &h->b;
    static const char *const OUTS[] = {"SDO", "DTR", "RTS", "INTR", "TBRE", "DR", "CO",
                                       "TC", "OE", "FE", "PE", NULL};
    char message[80];
    char text[16];
    unsigned i;

    case_begin();
    hs_setup(b);
    hs_write(b, 0, 'A');
    /* A read in progress when RST goes X: the cycle ends, the bus is released. */
    hs_set_addr(b, 2);
    set(b, "CS0", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(200));
    set(b, "RST", LOGIC_X);
    run(b, NS(300));
    h_expect_bus(b, HS_D, 8, "ZZZZZZZZ", "RST X: no read cycle, the data bus released");
    set(b, "RD", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    run(b, 12 * HS_BT);
    for (i = 0; OUTS[i] != NULL; i++) {
        snprintf(message, sizeof message, "RST X: %s is X", OUTS[i]);
        expect_pin(b, OUTS[i], LOGIC_X, message);
    }
    check(hs_sent_count(b) == 0, "RST X: no baud ticks, the TBR character is not sent");
    hs_read(b, 2, text);
    h_expect_text(text, "ZZZZZZZZ", "RST X: a read is not answered");
    set(b, "RST", LOGIC_0);
    run(b, NS(1000));
    hs_expect(b, 2, "XXXXXXXX", "after an X reset the MCR is unknown");
    expect_pin(b, "SDO", LOGIC_X, "MCR mode bits unknown: SDO X");
    expect_pin(b, "RTS", LOGIC_X, "MCR RTS bit unknown: RTS X");
    expect_pin(b, "DTR", LOGIC_X, "MCR DTR bit unknown: DTR X");
    expect_pin(b, "CO", LOGIC_X, "BRSR unknown: CO X");
    expect_pin(b, "TBRE", LOGIC_1, "TBRE set as by RST");
    expect_pin(b, "DR", LOGIC_0, "DR cleared as by RST");
    hs_write(b, 2, 0x27);
    expect_pin(b, "SDO", LOGIC_1, "MCR written: SDO marks again");
    case_end("hs82c52: RST X and what it leaves");
    hb_close(h);
}

/* BRSR 11111 with prescaler /1: external, divide by 1, CO = IX (Fig 2). */
static void case_hs_external_div1(void) {
    HBench *h = hs_open("SFX-82C52-plcc28", 0);
    Bench *b = &h->b;
    const SimTime ix = 2 * HS_XHALF;
    SimTime got3;
    char message[96];

    case_begin();
    hs_write(b, 3, 0x80 | (31u << 2));
    got3 = hs_co_3(b, 20 * ix);
    snprintf(message, sizeof message, "external /1: 3 CO periods %llu ps, want %llu",
             (unsigned long long)got3, (unsigned long long)(3 * ix));
    check(got3 + NS(60) >= 3 * ix && got3 <= 3 * ix + NS(60), message);
    case_end("hs82c52: external divide-by-1 clock");
    hb_close(h);
}

/* An unknown register address with the part selected: the cycle is X (P2). */
static void case_hs_unknown_address(void) {
    HBench *h = hs_open("SFX-82C52-plcc28", 0);
    Bench *b = &h->b;

    case_begin();
    hs_setup(b);
    set(b, "A0", LOGIC_X);
    set(b, "CS0", LOGIC_0);
    run(b, NS(50));
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    h_expect_bus(b, HS_D, 8, "XXXXXXXX", "A0 X: the read drives X");
    set(b, "RD", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    run(b, NS(300));
    hb_close(h);

    /* HD-6406: ALE X and the address changing: the latch is unknown. */
    h = hs_open("SFX-HD-6406-dip40", 1);
    b = &h->b;
    hs_setup(b);
    hs_set_addr(b, 2);
    run(b, NS(50));
    set(b, "ALE", LOGIC_X);
    hs_set_addr(b, 0);
    run(b, NS(50));
    set(b, "CS0", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    h_expect_bus(b, HS_D, 8, "XXXXXXXX", "ALE X while A1A0 change: the latched address is X");
    set(b, "RD", LOGIC_1);
    set(b, "CS0", LOGIC_1);
    set(b, "ALE", LOGIC_1);
    run(b, NS(300));
    hs_expect(b, 2, "00100111", "ALE high again: the address follows, MCR reads");
    /* The DMA acknowledges the other way round: TXDACK with RD drives X
       (decisions.md); RXDACK with WR stores nothing. */
    set(b, "TXDACK", LOGIC_0);
    set(b, "RD", LOGIC_0);
    run(b, NS(300));
    h_expect_bus(b, HS_D, 8, "XXXXXXXX", "TXDACK read: X");
    set(b, "RD", LOGIC_1);
    set(b, "TXDACK", LOGIC_1);
    run(b, NS(300));
    hs_set_addr(b, 2);
    set(b, "RXDACK", LOGIC_0);
    h_set_bus(b, HS_D, 8, 1, 0x00);
    run(b, NS(50));
    set(b, "WR", LOGIC_0);
    run(b, NS(200));
    set(b, "WR", LOGIC_1);
    run(b, NS(50));
    set(b, "RXDACK", LOGIC_1);
    h_set_bus(b, HS_D, 8, 0, 0);
    run(b, NS(300));
    expect_pin(b, "TBRE", LOGIC_1, "RXDACK write: the TBR is not loaded");
    hs_expect(b, 2, "00100111", "RXDACK write: the MCR is not written either");
    check(hs_sent_count(b) == 0, "RXDACK write: nothing sent");
    case_end("hs82c52: unknown address, unknown ALE");
    hb_close(h);
}

/* A TBR write while the TR sends: the character follows back to back, its
   start bit right after the stop bit (p14). */
static void case_hs_back_to_back(void) {
    HBench *h = hs_open("SFX-82C52-plcc28", 0);
    Bench *b = &h->b;
    char got[24];
    char want[24];

    case_begin();
    hs_setup(b);
    hs_write(b, 0, 'A');
    check(h_wait(b, "SDO", LOGIC_0, 8 * HS_BT), "A starts");
    run(b, HS_BT / 2);   /* mid start bit */
    hs_write(b, 0, 'B');
    expect_pin(b, "TBRE", LOGIC_0, "B waits in the TBR");
    /* A's data and stop bits, then (with no gap) B's start, data and stop. */
    {
        unsigned i;

        for (i = 0; i < 19; i++) {
            run(b, HS_BT);
            got[i] = "01XZ"[get(b, "SDO")];
            if (i == 9) {   /* mid B start bit */
                expect_pin(b, "TBRE", LOGIC_1, "B moved to the TR: TBRE");
            }
        }
        got[19] = '\0';
    }
    h_frame_bits('A', 8, 0, 1, want);
    h_frame_bits('B', 8, 0, 1, want + strlen(want));
    h_expect_text(got, want + 1, "A then B back to back: B's start right after A's stop");
    run(b, HS_BT);
    h_expect_sent(b, 0, 'B', "B logged after A");
    check(hs_sent_count(b) == 2, "two characters sent");
    case_end("hs82c52: back-to-back characters");
    hb_close(h);
}

/* REN unknown (an MCR write with D5 X): the input is gated by an unknown
   level, so a space reads X and the character received is not known. */
static void case_hs_ren_x(void) {
    HBench *h = hs_open("SFX-82C52-plcc28", 0);
    Bench *b = &h->b;
    char want[24];
    char text[16];

    case_begin();
    hs_setup(b);
    hs_write_x(b, 2, 0x27, 0x20);
    hs_expect(b, 2, "00X00111", "MCR with REN X");
    h_frame_bits(0x55, 8, 0, 1, want);
    h_drive_bits(b, "SDI", want, HS_BT);
    run(b, 2 * HS_BT);
    hs_read(b, 0, text);
    check(strcmp(text, "01010101") != 0 && strchr(text, 'X') != NULL,
          "REN X: no known character is received");
    case_end("hs82c52: REN unknown");
    hb_close(h);
}

/* ---- The COM78C80x bench (as serial_hs_com_test) ------------------------------------------------ */

static const char *const CM_DL[8] = {"DL0", "DL1", "DL2", "DL3", "DL4", "DL5", "DL6", "DL7"};

#define CM_HALF ((SimTime)101725)
#define CM_BIT19200 (256 * 2 * CM_HALF)

static void cm_set_addr(Bench *b, unsigned line, unsigned reg) {
    unsigned a = (line << 3) | reg;
    char name[8];
    unsigned i;

    for (i = 0; i < 5; i++) {
        snprintf(name, sizeof name, "ADD%u", i);
        set(b, name, (a >> i) & 1u ? LOGIC_1 : LOGIC_0);
    }
}

static void cm_write(Bench *b, unsigned line, unsigned reg, unsigned v) {
    cm_set_addr(b, line, reg);
    set(b, "WR", LOGIC_0);
    h_set_bus(b, CM_DL, 8, 1, v);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "DS", LOGIC_0);
    run(b, NS(200));
    set(b, "DS", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    set(b, "WR", LOGIC_1);
    h_set_bus(b, CM_DL, 8, 0, 0);
    run(b, NS(500));
}

static void cm_expect(Bench *b, unsigned line, unsigned reg, const char *want, const char *what) {
    char text[16];

    cm_set_addr(b, line, reg);
    set(b, "WR", LOGIC_1);
    set(b, "CS", LOGIC_0);
    run(b, NS(50));
    set(b, "DS", LOGIC_0);
    run(b, NS(250));
    h_get_bus(b, CM_DL, 8, text);
    set(b, "DS", LOGIC_1);
    run(b, NS(50));
    set(b, "CS", LOGIC_1);
    run(b, NS(500));
    h_expect_text(text, want, what);
}

static HBench *cm_open(void) {
    static const HClockPin CLOCKS[1] = {{"CLK", 0}};
    SimTime half = CM_HALF;
    HBench *h = hb_open("SFX-COM78C804-plcc44", CLOCKS, 1, &half, 1);
    Bench *b = &h->b;
    char name[8];
    unsigned i;

    set(b, "RESET", LOGIC_0);
    set(b, "CS", LOGIC_1);
    set(b, "DS", LOGIC_1);
    set(b, "WR", LOGIC_1);
    set(b, "MRST", LOGIC_0);
    for (i = 0; i < 4; i++) {
        snprintf(name, sizeof name, "RXD%u", i);
        set(b, name, LOGIC_1);
        snprintf(name, sizeof name, "DSR%u", i);
        set(b, name, LOGIC_0);
        snprintf(name, sizeof name, "DCD%u", i);
        set(b, name, LOGIC_1);
    }
    cm_set_addr(b, 0, 0);
    run(b, NS(2000));
    set(b, "RESET", LOGIC_1);
    run(b, NS(2000));
    return h;
}

/* RESET X: TXD, IRQ, IRQLN, IRQTXRX X; RESET released after it: MR1, MR2
   and the Command register unknown (plan-12-A section 3.5). */
static void case_cm_reset_x(void) {
    HBench *h = cm_open();
    Bench *b = &h->b;
    static const char *const OUTS[] = {"TXD0", "TXD1", "TXD2", "TXD3", "IRQ", "IRQLN0", "IRQLN1",
                                       "IRQTXRX", NULL};
    char message[80];
    unsigned i;

    case_begin();
    set(b, "RESET", LOGIC_X);
    run(b, NS(2000));
    for (i = 0; OUTS[i] != NULL; i++) {
        snprintf(message, sizeof message, "RESET X: %s is X", OUTS[i]);
        expect_pin(b, OUTS[i], LOGIC_X, message);
    }
    set(b, "RESET", LOGIC_1);
    run(b, NS(2000));
    cm_expect(b, 0, 3, "XXXXXXXX", "after an X reset: Command unknown");
    cm_expect(b, 1, 2, "XXXXXXXX", "after an X reset: MR1 unknown");
    cm_write(b, 0, 3, 0x05);
    cm_expect(b, 0, 3, "00000101", "Command written: known again");
    case_end("com78c80x: RESET X and what it leaves");
    hb_close(h);
}

/* A start needs RXD high for half a bit before the 1 -> 0 (p4). */
static void case_cm_start_rule(void) {
    HBench *h = cm_open();
    Bench *b = &h->b;
    char want[24];

    case_begin();
    cm_expect(b, 0, 3, "00000000", "pointer to MR1");
    cm_write(b, 0, 2, 0x4C);
    cm_write(b, 0, 2, 0xFF);
    cm_write(b, 0, 3, 0x05);
    /* RXD spacing long enough to end any frame, then a short mark. */
    set(b, "RXD0", LOGIC_0);
    run(b, 24 * CM_BIT19200);
    cm_write(b, 0, 3, 0x15);   /* RERR: clear what the spacing left */
    cm_write(b, 0, 3, 0x05);
    cm_expect(b, 0, 0, "XXXXXXXX", "empty FIFO reads X");
    cm_write(b, 0, 3, 0x15);
    cm_write(b, 0, 3, 0x05);
    cm_expect(b, 0, 1, "10000001", "no character waiting, no error (TxRDY)");
    /* 0xFF: no fall after the start bit, so an ignored start leaves nothing. */
    set(b, "RXD0", LOGIC_1);
    run(b, CM_BIT19200 / 8);
    h_frame_bits(0xFF, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, 3 * CM_BIT19200);
    cm_expect(b, 0, 1, "10000001", "a fall after an eighth of a bit of mark is not a start");
    h_frame_bits(0x41, 8, 0, 1, want);
    h_drive_bits(b, "RXD0", want, CM_BIT19200);
    run(b, CM_BIT19200);
    cm_expect(b, 0, 1, "10000011", "after a long mark a frame is received (RxRDY)");
    cm_expect(b, 0, 0, "01000001", "the character");
    case_end("com78c80x: the start rule");
    hb_close(h);
}

/* ---- Loader rejections ------------------------------------------------------------------------ */

#define E1(name, path, value) static const char *const name[] = {path, value, NULL}

static void case_hs_rejections(void) {
    E1(NO_BUS, "bus", "-");
    E1(BUS_KEY, "bus.strobe", "\"RD\"");
    E1(NO_CLOCK, "clock", "-");
    E1(HD_NOT_OBJECT, "hd6406", "3");
    E1(UNBOUND_READ, "bus.read", "\"NOPE\"");
    E1(UNBOUND_TC, "hd6406.tc", "\"NOPE\"");

    case_begin();
    reject("SFX-82C52", NO_BUS, "missing \"bus\"");
    reject("SFX-82C52", BUS_KEY, "unknown key \"strobe\"");
    reject("SFX-82C52", NO_CLOCK, "missing \"clock\"");
    reject("SFX-HD-6406", HD_NOT_OBJECT, "must be an object of pin names");
    reject("SFX-82C52", UNBOUND_READ, "no pin named \"NOPE\"");
    reject("SFX-HD-6406", UNBOUND_TC, "no pin named \"NOPE\"");
    case_end("hs82c52: loader rejections");
}

static void case_cm_rejections(void) {
    E1(LINES_TEXT, "lines", "\"four\"");
    E1(LINES_3, "lines", "3");
    E1(NO_BUS, "bus", "-");
    E1(NO_STROBE, "bus.strobe", "-");
    E1(NO_CLOCK, "clock", "-");
    E1(NO_CHANNELS, "channels", "-");
    E1(CHANNELS_2, "channels", "[{},{}]");
    E1(CHANNEL_KEY, "channels.0.rts", "\"RTS\"");
    E1(CHANNEL_NO_TX, "channels.0.tx", "-");
    E1(NO_INTERRUPT, "interrupt", "-");
    E1(NO_IRQ, "interrupt.irq", "-");
    E1(UNBOUND_SELECT, "bus.select", "\"NOPE\"");
    E1(UNBOUND_ADDRESS, "bus.address.0", "\"NOPE\"");
    E1(UNBOUND_LINE, "interrupt.line.0", "\"NOPE\"");
    E1(UNBOUND_DSR, "channels.0.dsr", "\"NOPE\"");

    case_begin();
    reject("SFX-COM78C804", LINES_TEXT, "lines");
    reject("SFX-COM78C804", LINES_3, "\"lines\" must be 2 or 4");
    reject("SFX-COM78C804", NO_BUS, "missing \"bus\"");
    reject("SFX-COM78C804", NO_STROBE, "strobe");
    reject("SFX-COM78C804", NO_CLOCK, "clock");
    reject("SFX-COM78C804", NO_CHANNELS, "missing \"channels\"");
    reject("SFX-COM78C804", CHANNELS_2, "\"channels\" must be an array of 4 lines");
    reject("SFX-COM78C804", CHANNEL_KEY, "unknown key \"rts\"");
    reject("SFX-COM78C804", CHANNEL_NO_TX, "tx");
    reject("SFX-COM78C804", NO_INTERRUPT, "missing \"interrupt\"");
    reject("SFX-COM78C804", NO_IRQ, "irq");
    reject("SFX-COM78C804", UNBOUND_SELECT, "bus.select: no pin named \"NOPE\"");
    reject("SFX-COM78C804", UNBOUND_ADDRESS, "bus.address: no pin named \"NOPE\"");
    reject("SFX-COM78C804", UNBOUND_LINE, "no pin named \"NOPE\"");
    reject("SFX-COM78C804", UNBOUND_DSR, "no pin named \"NOPE\"");
    case_end("com78c80x: loader rejections");
}

int main(void) {
    if (serial_bench_load() != 0) {
        return 1;
    }
    fixtures = cJSON_Parse(SERIAL_FIXTURE_CLASSIC);
    case_hs_reset_x();
    case_hs_external_div1();
    case_hs_unknown_address();
    case_hs_back_to_back();
    case_hs_ren_x();
    case_cm_reset_x();
    case_cm_start_rule();
    case_hs_rejections();
    case_cm_rejections();
    cJSON_Delete(fixtures);
    return serial_bench_finish();
}
