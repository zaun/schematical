/*
 * charlcd_4row_test: split from charlcd_test.c by plan-68-H; the shared helpers are
 * support/charlcd_bench.c.
 */

#include "charlcd_bench.h"

/* ---- plan-35-D cases 2-9: 4 rows, two controllers, RGB backlight ---------- */

/* The 8-bit init with Function Set `fs` (0x38: N = 1, 0x30: N = 0), then Clear. */
static void init8_fs(Lcd *l, uint8_t fs) {
    at(l, MS(100));
    cmd(l, 0x30, MS(5));
    cmd(l, 0x30, US(160));
    cmd(l, 0x30, US(160));
    cmd(l, fs, US(40));
    cmd(l, 0x10, US(40));
    cmd(l, 0x0C, US(40));
    cmd(l, 0x06, US(40));
    clear(l);
}

/* Data bytes 50 us apart. */
static void data_bytes(Lcd *l, const uint8_t *bytes, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        wr(l, 1, bytes[i]);
        bench_run(l->b, US(50));
    }
}

/* Row `row` columns 0..count-1 show ROM codes bytes[first + c]. */
static int row_shows(const LcdState *st, unsigned row, const uint8_t *bytes, unsigned first,
                     unsigned count) {
    unsigned c;

    for (c = 0; c < count; c++) {
        if (!cell_is_rom(st, row, c, bytes[first + c])) {
            return 0;
        }
    }
    return 1;
}

/* 80 distinct ROM codes, 30-7F; byte i is the i-th write. */
static void eighty(uint8_t bytes[80]) {
    unsigned i;

    for (i = 0; i < 80; i++) {
        bytes[i] = (uint8_t)(0x30 + i);
    }
}

static void check_2004_map(void) {
    Lcd l = lcd_open("FX-2004");
    LcdState st;
    uint8_t bytes[80];
    int ac;

    eighty(bytes);
    init8_fs(&l, 0x38);
    data_bytes(&l, bytes, 80);
    ac = read_ac(&l);
    lcd_state(&l, &st);
    expect(ac == 0x00 && st.ddram[0][39] == bytes[39] && st.ddram[0][40] == bytes[40] &&
               st.ddram[0][79] == bytes[79],
           "D2 20x4 map: 80 writes from 00 wrap 27 -> 40 and 67 -> 00 (AC 00 after)");
    expect(st.rows == 4 && st.controllers == 1 && row_shows(&st, 0, bytes, 0, 20),
           "D2 20x4 map: row 0 shows bytes 0-19 (addresses 00-13)");
    expect(row_shows(&st, 1, bytes, 40, 20), "D2 20x4 map: row 1 shows bytes 40-59 (addresses 40-53)");
    expect(row_shows(&st, 2, bytes, 20, 20), "D2 20x4 map: row 2 shows bytes 20-39 (addresses 14-27)");
    expect(row_shows(&st, 3, bytes, 60, 20), "D2 20x4 map: row 3 shows bytes 60-79 (addresses 54-67)");

    /* Case 3: shift the display left once. */
    cmd(&l, 0x18, US(40));
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 2, 0, bytes[0x15]) && cell_is_rom(&st, 0, 19, bytes[0x14]),
           "D3 20x4 shift: after one left shift row 2 column 0 shows address 15, row 0 column 19 address 14");
    expect(cell_is_rom(&st, 1, 0, bytes[40 + 1]) && cell_is_rom(&st, 3, 19, bytes[40]),
           "D3 20x4 shift: row 1 column 0 shows address 41, row 3 column 19 wraps to address 40");
    bench_destroy(l.b);
}

static void check_2004_one_line(void) {
    Lcd l = lcd_open("FX-2004");
    LcdState st;
    uint8_t bytes[80];

    eighty(bytes);
    init8_fs(&l, 0x30);
    data_bytes(&l, bytes, 80);
    lcd_state(&l, &st);
    expect(row_all(&st, 1, 20, LCD_DOT_OFF) && row_all(&st, 3, 20, LCD_DOT_OFF),
           "D4 20x4 1-line: N = 0: rows 1 and 3 are off");
    expect(row_shows(&st, 0, bytes, 0, 20) && row_shows(&st, 2, bytes, 20, 20),
           "D4 20x4 1-line: row 0 shows addresses 00-13, row 2 addresses 14-27");
    bench_destroy(l.b);
}

static void check_4004(void) {
    Lcd l = lcd_open("FX-4004");
    LcdState st;
    uint8_t v, k;
    unsigned i;
    int spaces;

    l.e = "E1";
    init8_fs(&l, 0x38);
    text(&l, "TOP");
    lcd_state(&l, &st);
    expect(st.rows == 4 && st.columns == 40 && cell_is_rom(&st, 0, 0, 'T') &&
               cell_is_rom(&st, 0, 1, 'O') && cell_is_rom(&st, 0, 2, 'P') &&
               row_all(&st, 1, 40, LCD_DOT_OFF),
           "D5 40x4: E1 init + TOP shows on row 0");
    expect(row_all(&st, 2, 40, LCD_DOT_OFF) && row_all(&st, 3, 40, LCD_DOT_OFF),
           "D5 40x4: rows 2-3 stay blank while E2 is not initialised (its display is off)");
    l.e = "E2";
    init8_fs(&l, 0x38);
    text(&l, "LOW");
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 2, 0, 'L') && cell_is_rom(&st, 2, 1, 'O') && cell_is_rom(&st, 2, 2, 'W') &&
               row_all(&st, 3, 40, LCD_DOT_OFF) && cell_is_rom(&st, 0, 0, 'T'),
           "D5 40x4: then E2 init + LOW shows on row 2; row 0 keeps TOP");
    expect(st.controllers == 2 && st.ddram[1][0] == 'L' && st.ddram[0][0] == 'T' &&
               st.ddram[1][3] == 0x20 && st.ddram[0][3] == 0x20,
           "D5 40x4: LcdState.controllers 2, ddram[0][0] = T, ddram[1][0] = L");
    wr(&l, 0, 0x01);   /* Clear on E2: busy 1.52 ms */
    lcd_state(&l, &st);
    expect(st.busy[1] == 1 && st.busy[0] == 0,
           "D5 40x4: busy[] are separate: E2 busy after its Clear, E1 not");
    l.e = "E1";
    expect(read_ac(&l) == 0x03, "D5 40x4: a BF/AC read through E1 meanwhile: BF 0, AC 03");
    l.e = "E2";
    rd(&l, 0, &v, &k);
    expect((k & 0x80) && (v & 0x80), "D5 40x4: a BF read through E2 meanwhile: BF 1");
    bench_destroy(l.b);

    /* E2 never initialised: during its power-on BF = 1 and writes are ignored. */
    l = lcd_open("FX-4004");
    l.e = "E2";
    at(&l, MS(1));
    rd(&l, 0, &v, &k);
    expect((k & 0x80) && (v & 0x80), "D5 40x4: at 1 ms a BF read through E2: BF 1 (power-on)");
    text(&l, "XY");
    l.e = "E1";
    init8_fs(&l, 0x38);
    text(&l, "TOP");
    lcd_state(&l, &st);
    spaces = 1;
    for (i = 0; i < 80; i++) {
        spaces = spaces && st.ddram[1][i] == 0x20 && st.ddram_known[1][i] == 0xFF;
    }
    expect(spaces && row_all(&st, 2, 40, LCD_DOT_OFF) && row_all(&st, 3, 40, LCD_DOT_OFF) &&
               cell_is_rom(&st, 0, 0, 'T'),
           "D5 40x4: E2 never initialised: its writes at 1 ms were ignored (DDRAM spaces), rows 2-3 blank");
    bench_destroy(l.b);
}

/* A 40x4 with both controllers initialised; "TOP" through E1, "LOW" through E2. */
static Lcd ready_4004(void) {
    Lcd l = lcd_open("FX-4004");

    l.e = "E1";
    init8_fs(&l, 0x38);
    text(&l, "TOP");
    l.e = "E2";
    init8_fs(&l, 0x38);
    text(&l, "LOW");
    return l;
}

/* A read access with E1 and E2 both high; DB sampled before they fall. */
static void read_both(Lcd *l, int rs, uint8_t *v, uint8_t *k) {
    unsigned i;

    lv(l, "RS", rs);
    lv(l, "R/W", 1);
    put_db(l, 0, 0);
    bench_run(l->b, NS(50));
    lv(l, "E1", 1);
    lv(l, "E2", 1);
    bench_run(l->b, NS(500));
    *v = *k = 0;
    for (i = 0; i < 8; i++) {
        LogicValue x = bench_get(l->b, DB[i]);

        if (x == LOGIC_0 || x == LOGIC_1) {
            *k |= (uint8_t)(1u << i);
            *v |= (uint8_t)((x == LOGIC_1) << i);
        }
    }
    lv(l, "E1", 0);
    lv(l, "E2", 0);
    bench_run(l->b, US(2));
    lv(l, "R/W", 0);
}

static void check_4004_clash(void) {
    Lcd l = ready_4004();
    uint8_t v, k;

    /* Both AC are 03. Set E1's to 00, then Clear E2 (AC 00, busy 1.52 ms):
       the AC values agree and only BF differs. */
    l.e = "E1";
    cmd(&l, 0x80, US(40));
    l.e = "E2";
    wr(&l, 0, 0x01);
    read_both(&l, 0, &v, &k);
    expect(k == 0x7F && (v & 0x7F) == 0x00,
           "D6 40x4 read clash: BF read with E1 and E2 high, busy flags differ: DB7 X, DB6-0 = AC 00");
    bench_run(l.b, MS(2));
    read_both(&l, 0, &v, &k);
    expect(k == 0xFF && v == 0x00, "D6 40x4 read clash: both BF 0 and AC 00: every DB bit known (00)");
    bench_run(l.b, US(1));
    expect(db_all(&l, LOGIC_Z), "D6 40x4 read clash: after both reads end, DB is released (Z)");
    l.e = "E1";
    cmd(&l, 0x83, US(40));
    read_both(&l, 0, &v, &k);
    expect(k == 0xFC && (v & 0xFC) == 0x00,
           "D6 40x4 read clash: AC 03 and 00: DB1-0 X, the agreeing bits known");
    bench_destroy(l.b);
}

static void check_4004_timing(void) {
    Lcd l = ready_4004();
    LcdState st;

    /* Each controller has its own access count for its timers: an E2 access
       starting inside an E1 write's hold window leaves that write pending. */
    lv(&l, "RS", 1);
    lv(&l, "R/W", 0);
    put_db(&l, 'A', 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E1", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E1", 0);
    bench_run(l.b, NS(5));
    lv(&l, "E2", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E2", 0);
    bench_run(l.b, US(50));
    lcd_state(&l, &st);
    expect(st.ddram[0][3] == 'A' && st.ddram_known[0][3] == 0xFF && st.ddram[1][3] == 'A' &&
               st.ddram_known[1][3] == 0xFF,
           "D7 40x4 timing: an E2 write rising 5 ns after an E1 write's fall: both take effect");

    /* An instruction write through E2 with E2 high 100 ns (< tpw 140). */
    lv(&l, "RS", 0);
    lv(&l, "R/W", 0);
    put_db(&l, 0x01, 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E2", 1);
    bench_run(l.b, NS(100));
    lv(&l, "E2", 0);
    bench_run(l.b, US(40));
    lcd_state(&l, &st);
    expect(st.busy[1] == -1 && st.ddram_known[1][0] == 0 && row_all(&st, 2, 40, LCD_DOT_X) &&
               row_all(&st, 3, 40, LCD_DOT_X),
           "D7 40x4 timing: an E2 pulse shorter than tpw makes controller 2 LOST (BF, RAM, rows 2-3 X)");
    expect(st.busy[0] == 0 && st.ddram_known[0][0] == 0xFF && cell_is_rom(&st, 0, 0, 'T') &&
               cell_is_rom(&st, 0, 2, 'P') && row_all(&st, 1, 40, LCD_DOT_OFF),
           "D7 40x4 timing: controller 1 stays known (BF 0, TOP on row 0)");
    l.e = "E1";
    expect(read_ac(&l) == 0x04, "D7 40x4 timing: E1 reads BF 0, AC 04");
    bench_destroy(l.b);
}

/* FX-RGB in a circuit: VSS, V0 on GND, VDD and LED+ on 5V, and each
   cathode K(R) 18, K(G) 17, K(B) 16 on 5V (1) or GND (0). */
static Circuit *rgb_circuit(int red, int green, int blue, size_t *part) {
    Circuit *c = circuit_create(library);
    size_t p5, pg, n5, ng;

    circuit_add_part(c, "FX-RGB", part);
    circuit_add_part(c, "5V", &p5);
    circuit_add_part(c, "GND", &pg);
    circuit_add_net(c, &n5);
    circuit_add_net(c, &ng);
    circuit_connect(c, p5, "1", n5);
    circuit_connect(c, pg, "1", ng);
    circuit_connect(c, *part, "1", ng);
    circuit_connect(c, *part, "2", n5);
    circuit_connect(c, *part, "3", ng);
    circuit_connect(c, *part, "15", n5);
    circuit_connect(c, *part, "18", red ? n5 : ng);
    circuit_connect(c, *part, "17", green ? n5 : ng);
    circuit_connect(c, *part, "16", blue ? n5 : ng);
    return c;
}

static void check_rgb_lights(void) {
    Circuit *c;
    size_t part;
    unsigned char on[4] = {9, 9, 9, 9};
    uint64_t time[4] = {0};
    size_t lights = 0, times = 0;
    int started;

    c = rgb_circuit(1, 0, 1, &part);
    started = circuit_start(c) == 0 && circuit_advance(c, US(10));
    if (started) {
        lights = circuit_part_lights(c, part, on, 4);
    }
    expect(started && lights == 3 && on[0] == 0 && on[1] == 1 && on[2] == 0,
           "D8 RGB lights: three lights; K(G) low, K(R) and K(B) high: lights R, G, B = 0/1/0");
    started = started && circuit_advance(c, US(10));
    if (started) {
        times = circuit_part_light_time(c, part, time, 4);
    }
    expect(started && times == 3 && time[0] == 0 && time[2] == 0 && time[1] >= US(19) &&
               time[1] <= US(20),
           "D8 RGB lights: light times per LED: green on ~20 us, red and blue 0");
    circuit_destroy(c);
    c = rgb_circuit(0, 1, 0, &part);
    started = circuit_start(c) == 0 && circuit_advance(c, US(10));
    if (started) {
        lights = circuit_part_lights(c, part, on, 4);
    }
    expect(started && lights == 3 && on[0] == 1 && on[1] == 0 && on[2] == 1,
           "D8 RGB lights: K(R) and K(B) low, K(G) high: 1/0/1");
    circuit_destroy(c);
}

static size_t pin_number(const Part *part, unsigned number) {
    char text[8];
    size_t i;

    snprintf(text, sizeof text, "%u", number);
    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].designator, text) == 0) {
            return i;
        }
    }
    fprintf(stderr, "%s has no pin %u\n", part->name, number);
    exit(1);
}

static void check_rgb_voltage(void) {
    /* FX-RGB with LED+ on 5V and K(R), K(G), K(B) on three nets with no
       other driver; each net goes to an input of a 3.6 V-max probe. */
    const Part *rgb = part_library_find(library, "FX-RGB");
    const Part *probe = part_library_find(library, "FX-LCD-3V");
    const Part *five = part_library_find(library, "5V");
    const Part *parts[3];
    VoltageConnection conn[8];
    VoltageProblem problems[8];
    size_t count, n = 0, i;
    int seen[3] = {0, 0, 0};
    int ok;

    if (rgb == NULL || probe == NULL || five == NULL) {
        expect(0, "D9 RGB voltage: fixture parts");
        return;
    }
    parts[0] = rgb;
    parts[1] = probe;
    parts[2] = five;
    /* net 0: 5V; nets 1-3: K(R), K(G), K(B). */
    conn[n++] = (VoltageConnection){2, 0, 0};
    conn[n++] = (VoltageConnection){0, pin_number(rgb, 15), 0};
    conn[n++] = (VoltageConnection){0, pin_number(rgb, 18), 1};
    conn[n++] = (VoltageConnection){0, pin_number(rgb, 17), 2};
    conn[n++] = (VoltageConnection){0, pin_number(rgb, 16), 3};
    conn[n++] = (VoltageConnection){1, pin_number(probe, 4), 1};   /* RS */
    conn[n++] = (VoltageConnection){1, pin_number(probe, 5), 2};   /* R/W */
    conn[n++] = (VoltageConnection){1, pin_number(probe, 6), 3};   /* E */
    count = voltage_check(parts, 3, 4, conn, n, NULL, 0, NULL, problems, 8);
    ok = count == 3;
    for (i = 0; ok && i < count; i++) {
        ok = problems[i].kind == VOLTAGE_INPUT_OVER && problems[i].part == 1 &&
             problems[i].mv == 5000 && problems[i].net >= 1 && problems[i].net <= 3;
        if (ok) {
            seen[problems[i].net - 1] = 1;
        }
    }
    expect(ok && seen[0] && seen[1] && seen[2],
           "D9 RGB voltage: LED+ on 5V carries 5 V to all three cathode nets (3 INPUT_OVER on the probe)");
    /* The same with K(G) also on GND: that net is 0 V. */
    {
        const Part *ground = part_library_find(library, "GND");

        parts[2] = five;
        if (ground != NULL) {
            const Part *more[4] = {rgb, probe, five, ground};

            conn[n] = (VoltageConnection){3, 0, 2};
            count = voltage_check(more, 4, 4, conn, n + 1, NULL, 0, NULL, problems, 8);
            expect(count == 2 && problems[0].net == 1 && problems[1].net == 3,
                   "D9 RGB voltage: GND on K(G)'s net overrides the carried level (2 problems: K(R), K(B))");
        }
    }
}

/* ---- Cases 17-18: backlight and voltage ---- */

/* A circuit of `lcd` with 5V, 3V3 and GND; VSS and LED- on GND, V0 on GND,
   VDD on `vdd` (a supply name), LED+ on `anode` ("5V" or "GND"). */
static Circuit *lcd_circuit(const char *lcd, const char *vdd, const char *anode, size_t *part) {
    Circuit *c = circuit_create(library);
    size_t p5, p3, pg, n5, n3, ng;

    circuit_add_part(c, lcd, part);
    circuit_add_part(c, "5V", &p5);
    circuit_add_part(c, "3V3", &p3);
    circuit_add_part(c, "GND", &pg);
    circuit_add_net(c, &n5);
    circuit_add_net(c, &n3);
    circuit_add_net(c, &ng);
    circuit_connect(c, p5, "1", n5);
    circuit_connect(c, p3, "1", n3);
    circuit_connect(c, pg, "1", ng);
    circuit_connect(c, *part, "1", ng);
    circuit_connect(c, *part, "2", strcmp(vdd, "3V3") == 0 ? n3 : n5);
    circuit_connect(c, *part, "3", ng);
    circuit_connect(c, *part, "15", strcmp(anode, "5V") == 0 ? n5 : ng);
    circuit_connect(c, *part, "16", ng);
    return c;
}

static void check_backlight(void) {
    Circuit *c;
    size_t part;
    unsigned char on[4] = {0};

    c = lcd_circuit("FX-LCD", "5V", "5V", &part);
    expect(circuit_start(c) == 0 && circuit_advance(c, US(10)) &&
               circuit_part_lights(c, part, on, 4) == 1 && on[0] == 1,
           "17 backlight: anode 1, cathode 0: 1 light, on");
    circuit_destroy(c);
    c = lcd_circuit("FX-LCD", "5V", "GND", &part);
    on[0] = 1;
    expect(circuit_start(c) == 0 && circuit_advance(c, US(10)) &&
               circuit_part_lights(c, part, on, 4) == 1 && on[0] == 0,
           "17 backlight: anode 0: off");
    circuit_destroy(c);
    c = lcd_circuit("FX-LCD-NB", "5V", "5V", &part);
    expect(circuit_start(c) == 0 && circuit_advance(c, US(10)) &&
               circuit_part_lights(c, part, on, 4) == 0 &&
               circuit_part_light_time(c, part, NULL, 0) == 0,
           "17 backlight: a module without a backlight has 0 lights");
    circuit_destroy(c);
}

static void check_voltage(void) {
    Circuit *c;
    size_t part;
    const char *want = "voltage: 3.3 V on pin 2 (VDD) of FX-LCD; it needs 4.5-5.5 V";
    char line[256];

    c = lcd_circuit("FX-LCD", "3V3", "GND", &part);
    snprintf(line, sizeof line, "18 voltage: VDD on 3V3 is refused: %s (got: %s)", want,
             circuit_start(c) == -1 && circuit_error(c) != NULL ? circuit_error(c) : "started");
    expect(circuit_error(c) != NULL && strcmp(circuit_error(c), want) == 0, line);
    circuit_destroy(c);
    c = lcd_circuit("FX-LCD", "5V", "GND", &part);
    expect(circuit_start(c) == 0, "18 voltage: VDD on 5V starts");
    circuit_destroy(c);
    c = lcd_circuit("FX-LCD", "5V", "5V", &part);
    expect(circuit_start(c) == 0 && circuit_advance(c, US(10)) && !circuit_voltage_problem(c, NULL),
           "18 voltage: backlight anode on 5V, cathode on GND starts (no voltage problem)");
    circuit_destroy(c);
}

/* ---- Case 19: API ---- */

static void check_api(void) {
    PartLibrary *real = part_library_create();
    Circuit *c;
    LcdState st;
    size_t index;

    if (real == NULL || load_parts_dir(real, PARTS_DIR) != 0) {
        expect(0, "19 api: load the part library");
        return;
    }
    c = circuit_create(real);
    circuit_add_part(c, "SN74HC00N", &index);
    expect(circuit_lcd_state(c, 0, &st) == -1 && circuit_error(c) != NULL &&
               strcmp(circuit_error(c), "part 0 is not a character LCD") == 0,
           "19 api: circuit_lcd_state on a gate: -1, \"part 0 is not a character LCD\"");
    circuit_destroy(c);
    part_library_destroy(real);

    c = circuit_create(library);
    circuit_add_part(c, "FX-LCD", &index);
    expect(circuit_lcd_state(c, 0, &st) == 0 && st.columns == 16 && st.rows == 2 && st.busy[0] == 1,
           "19 api: circuit_lcd_state on the LCD: 0, 16x2, busy at power-up");
    circuit_destroy(c);
}

static cJSON *charlcd_json;

static int json_ns(const cJSON *object, const char *key, SimTime *out) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, key);

    if (!cJSON_IsNumber(item) || item->valuedouble < 0.0) {
        return 0;
    }
    *out = (SimTime)(item->valuedouble * 1000.0 + 0.5);
    return 1;
}

/* 1 when part_name's device was found and every time read. */
static int real_timing(const char *part_name, RealTiming *t) {
    cJSON *devices = cJSON_GetObjectItemCaseSensitive(charlcd_json, "devices");
    cJSON *device, *p;

    cJSON_ArrayForEach(device, devices) {
        cJSON_ArrayForEach(p, cJSON_GetObjectItemCaseSensitive(device, "parts")) {
            const cJSON *name = cJSON_GetObjectItemCaseSensitive(p, "name");
            const cJSON *tm = cJSON_GetObjectItemCaseSensitive(device, "timing");
            const cJSON *ex = cJSON_GetObjectItemCaseSensitive(device, "exec");

            if (!cJSON_IsString(name) || strcmp(name->valuestring, part_name) != 0) {
                continue;
            }
            return json_ns(tm, "tc", &t->tc) && json_ns(tm, "tpw", &t->tpw) &&
                   json_ns(tm, "tas", &t->tas) && json_ns(tm, "tah", &t->tah) &&
                   json_ns(tm, "tdsw", &t->tdsw) && json_ns(tm, "th", &t->th) &&
                   json_ns(tm, "tddr", &t->tddr) && json_ns(tm, "tdhr", &t->tdhr) &&
                   json_ns(ex, "instruction", &t->instruction) && json_ns(ex, "data", &t->data) &&
                   json_ns(ex, "clearHome", &t->clear_home) && json_ns(ex, "powerOn", &t->power_on);
        }
    }
    return 0;
}

static SimTime max_time(SimTime a, SimTime b) {
    return a > b ? a : b;
}

/* The access shape from the part's own limits, each with 20 ns to spare:
   setup tAS, E high max(tPW, tDSW, tDDR), E low after max(tH, tAH, tDHR),
   and an E cycle of at least tC. */
static void real_bus(Lcd *l, const RealTiming *t) {
    l->setup = t->tas + NS(20);
    l->high = max_time(max_time(t->tpw, t->tdsw), t->tddr) + NS(20);
    l->after = max_time(max_time(t->th, t->tah), t->tdhr) + NS(20);
    if (l->setup + l->high + l->after < t->tc) {
        l->after = t->tc - l->setup - l->high;
    }
}

/* A write, then the part's exec time + 1 us. */
static void real_write(Lcd *l, int rs, uint8_t byte, SimTime exec) {
    wr(l, rs, byte);
    bench_run(l->b, exec + US(1));
}

/* After the part's power-on wait (+1 ms; no wait once past it): the 8-bit
   init of init8, then Clear, through l->e. */
static void real_init(Lcd *l, const RealTiming *t) {
    static const uint8_t INIT[] = {0x30, 0x30, 0x30, 0x38, 0x10, 0x0C, 0x06};
    size_t i;

    at(l, t->power_on + MS(1));
    for (i = 0; i < sizeof INIT; i++) {
        real_write(l, 0, INIT[i], t->instruction);
    }
    real_write(l, 0, 0x01, t->clear_home);
}

static void real_text(Lcd *l, const char *s, const RealTiming *t) {
    for (; *s != '\0'; s++) {
        real_write(l, 1, (uint8_t)*s, t->data);
    }
}

/* plan-35-E section 4.1: each charlcd part through its own bus pins and
   timing: init, clear, HELLO through E (E1); a 4-row part then sets DDRAM
   14 and writes ROW3 there; a two-enable part does init, clear, WORLD
   through E2. One enable: ROW3 is row 2. Two enables: E1's 14 is its line 0,
   column 20 (row 0), and WORLD is row 2 (E2's line 0). */
static void check_every_part(void) {
    PartLibrary *fixture = library;
    char *json = read_file(PARTS_DIR "/charlcd.json");
    size_t i, n = 0;

    charlcd_json = json != NULL ? cJSON_Parse(json) : NULL;
    free(json);
    library = real_library;
    for (i = 0; i < part_library_count(real_library); i++) {
        const Part *part = part_library_at(real_library, i);
        RealTiming t;
        LcdState st;
        Lcd l;
        char line[320];
        int ok, two, four;

        if (part->kind != PART_CHARLCD) {
            continue;
        }
        n++;
        memset(&t, 0, sizeof t);
        ok = real_timing(part->name, &t);
        l = lcd_open(part->name);
        two = l.enables == 2;
        four = part->charlcd->rows == 4;
        if (ok) {
            real_bus(&l, &t);
            real_init(&l, &t);
            real_text(&l, "HELLO", &t);
            if (four) {
                real_write(&l, 0, 0x80 | 0x14, t.instruction);
                real_text(&l, "ROW3", &t);
            }
            if (two) {
                l.e = l.en[1];
                real_init(&l, &t);
                real_text(&l, "WORLD", &t);
            }
        }
        lcd_state(&l, &st);
        ok = ok && l.pins_ok && memcmp(st.ddram[0], "HELLO", 5) == 0 && cells_show(&st, 0, 0, "HELLO") &&
             st.busy[0] == 0;
        if (four && !two) {
            ok = ok && cells_show(&st, 2, 0, "ROW3");
        }
        if (two) {
            ok = ok && st.controllers == 2 && cells_show(&st, 0, 20, "ROW3") &&
                 memcmp(st.ddram[1], "WORLD", 5) == 0 && cells_show(&st, 2, 0, "WORLD") &&
                 st.busy[1] == 0;
        }
        snprintf(line, sizeof line,
                 "E 4.1 every part: %s (%ux%u, enable %s%s%s, tPW %llu ns): init, clear, HELLO at (0,0)%s%s",
                 part->name, part->charlcd->columns, part->charlcd->rows, l.en[0], two ? "+" : "",
                 two ? l.en[1] : "", (unsigned long long)(t.tpw / 1000u),
                 four && !two ? ", ROW3 at DDRAM 14 = (2,0)" : "",
                 two ? ", ROW3 at E1 DDRAM 14 = (0,20), WORLD through E2 at (2,0)" : "");
        expect(ok, line);
        bench_destroy(l.b);
    }
    {
        char line[96];

        snprintf(line, sizeof line, "E 4.1 every part: %zu charlcd parts run (40 at plan-35-E)", n);
        expect(n >= 40, line);
    }
    cJSON_Delete(charlcd_json);
    charlcd_json = NULL;
    library = fixture;
}

/* ---- plan-11-C section 4.5, plan-35-E section 4.2: exec, facts ---- */

static void check_real_parts(void) {
    /* A second data write 40 us after the first (E fall to E fall):
       accepted where the data time is 37 us, refused at 43 us. */
    static const struct {
        const char *name, *data;
        int accepted;
    } EXEC40[] = {
        {"NHD-0216K1Z-NSW-BBW-L", "data 37 us", 1},
        {"162H BC BW", "data 43 us", 0},
        {"NHD-0420DZ-FL-YBW", "20x4, data 37 us", 1},
        {"MC44005A6W-FPTLW-V2", "40x4 through E1, ST7066U data 37 us", 1},
    };
    PartLibrary *fixture = library;
    const Part *part;
    size_t i;
    unsigned pins1 = 0;

    library = real_library;
    for (i = 0; i < sizeof EXEC40 / sizeof EXEC40[0]; i++) {
        Lcd l = lcd_open(EXEC40[i].name);
        LcdState st;
        char line[200];

        init8(&l);
        clear(&l);
        wr(&l, 1, 'A');
        bench_run(l.b, US(40) - US(2) - NS(550));
        wr(&l, 1, 'B');
        bench_run(l.b, US(50));
        lcd_state(&l, &st);
        snprintf(line, sizeof line, "C 4.5 exec: a data write 40 us after another is %s on %s (%s)",
                 EXEC40[i].accepted ? "accepted" : "not accepted", EXEC40[i].name, EXEC40[i].data);
        expect(st.ddram[0][0] == 'A' && st.ddram[0][1] == (EXEC40[i].accepted ? 'B' : 0x20), line);
        bench_destroy(l.b);
    }
    library = fixture;

    part = part_library_find(real_library, "NHD-0216K1Z-NSW-BBW-L");
    if (part != NULL && part->footprint != NULL) {
        for (i = 0; i < part->footprint->pad_count; i++) {
            const char *d = part->footprint->pads[i].designator;

            pins1 += d != NULL && strcmp(d, "1") == 0;
        }
    }
    expect(part != NULL && part->charlcd != NULL && strcmp(part->charlcd->panel, "blue-negative") == 0 &&
               part->charlcd->backlight_color != NULL &&
               strcmp(part->charlcd->backlight_color, "White") == 0 && part->footprint != NULL &&
               strcmp(part->footprint->id, "LCM-NHD-0216K1Z") == 0 &&
               part->connector_footprint != NULL &&
               strcmp(part->connector_footprint->id, "HDR-1X16-P2.54") == 0 && pins1 == 2,
           "C 4.5 facts: NHD-0216K1Z-NSW-BBW-L: blue-negative, White, LCM-NHD-0216K1Z with two "
           "pads \"1\", connector HDR-1X16-P2.54");
}

int main(int argc, char **argv) {
    char *error;

    if (argc > 1 && strcmp(argv[1], "--font") == 0) {
        return 0;
    }

    library = fixture_library();
    error = load_file(library, device_file("FX-LCD", "16", "2", "'A00'", "'blue-negative'", DATA,
                                           TIMING, EXEC, BACKLIGHT VCC),
                      "fx-lcd.json");
    if (error == NULL) {
        error = load_file(library, device_file("FX-LCD-NB", "16", "2", "'A00'", "'yellow-green'",
                                               DATA, TIMING, EXEC, VCC),
                          "fx-lcd-nb.json");
    }
    /* plan-35-D fixtures: the NHD-0420DZ 20x4 facts on p16, the C404ALB
       40x4 on p18-dual, a Midas RGB 20x4 on p18-rgb. */
    if (error == NULL) {
        error = load_file(library, large_file("FX-2004", PINOUT, "p16", "FX16", "20", "4",
                                              "'gray'", "'E'", BACKLIGHT VCC),
                          "fx-2004.json");
    }
    if (error == NULL) {
        error = load_file(library, large_file("FX-4004", PINOUT_P18_DUAL, "p18-dual", "FX18", "40",
                                              "4", "'white'", "['E1','E2']", BACKLIGHT VCC),
                          "fx-4004.json");
    }
    if (error == NULL) {
        error = load_file(library, large_file("FX-RGB", PINOUT_P18_RGB, "p18-rgb", "FX18", "20",
                                              "4", "'white'", "'E'", RGB_BACKLIGHT VCC),
                          "fx-rgb.json");
    }
    if (error == NULL) {
        /* A 3.6 V-max probe for case 9: its inputs flag a carried 5 V. */
        error = load_file(library, device_file("FX-LCD-3V", "16", "2", "'A00'", "'yellow-green'",
                                               DATA, TIMING, EXEC, "'vcc':[2.7,3.6],"),
                          "fx-lcd-3v.json");
    }
    if (error == NULL) {
        /* The pull parts name resistor footprints (plan-26-A). */
        char *footprints = read_file(PARTS_DIR "/footprints.json");

        if (footprints == NULL) {
            error = strdup("cannot read footprints.json");
        } else if (load_footprints(library, footprints, "footprints.json", &error) != 0 &&
                   error == NULL) {
            error = strdup("(no message)");
        }
        free(footprints);
    }
    if (error == NULL) {
        char *supplies = read_file(PARTS_DIR "/supply.json");

        error = supplies == NULL ? strdup("cannot read supply.json")
                                 : load_file(library, supplies, "supply.json");
    }
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_2004_map();
    check_2004_one_line();
    check_4004();
    check_4004_clash();
    check_4004_timing();
    check_rgb_lights();
    check_rgb_voltage();
    check_backlight();
    check_voltage();
    check_api();
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_every_part();
    check_real_parts();
    part_library_destroy(real_library);
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_test: all passed\n");
    return 0;
}
