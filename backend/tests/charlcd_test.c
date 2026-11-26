/*
 * charlcd_test: split from charlcd_test.c by plan-68-H; the shared helpers are
 * support/charlcd_bench.c.
 */

#include "charlcd_bench.h"

/* ---- Case 20: loader ------------------------------------------------------ */

static void expect_error(const char *what, char *json, const char *message) {
    PartLibrary *lib = fixture_library();
    char *error = load_file(lib, json, "bad.json");
    char line[512];

    snprintf(line, sizeof line, "20 loader: %s -> %s (got: %s)", what, message,
             error != NULL ? error : "loaded");
    expect(error != NULL && strstr(error, message) != NULL, line);
    free(error);
    part_library_destroy(lib);
}

static void check_loader(void) {
    const Part *part = part_library_find(library, "FX-LCD");
    const PartCharLcd *l = part != NULL ? part->charlcd : NULL;

    expect(part != NULL && part->kind == PART_CHARLCD && l != NULL, "20 loader: the fixture loads");
    if (l != NULL) {
        expect(l->columns == 16 && l->rows == 2 && strcmp(l->controller, "ST7066U") == 0 &&
                   strcmp(l->rom, "A00") == 0 && strcmp(l->panel, "blue-negative") == 0 &&
                   l->backlight_color != NULL && strcmp(l->backlight_color, "White") == 0,
               "20 loader: facts (16x2, ST7066U, A00, blue-negative, White)");
        expect(l->bus_pin_count == 11 && strcmp(part->pins[l->bus_pins[0]].name, "RS") == 0 &&
                   strcmp(part->pins[l->bus_pins[2]].name, "E") == 0 &&
                   strcmp(part->pins[l->bus_pins[3]].name, "DB0") == 0 &&
                   strcmp(part->pins[l->bus_pins[10]].name, "DB7") == 0,
               "20 loader: bus pins RS, R/W, E, DB0-DB7");
        expect(part->vcc != NULL && part->vcc->min_mv == 4500 && part->vcc->max_mv == 5500,
               "20 loader: vcc 4.5-5.5 V");
    }
    part = part_library_find(library, "FX-LCD-NB");
    expect(part != NULL && part->charlcd != NULL && part->charlcd->backlight_color == NULL,
           "20 loader: a device without a backlight has no backlight color");

    expect_error("columns 41", device_file("B", "41", "2", "'A00'", "'yellow-green'", DATA, TIMING,
                                           EXEC, BACKLIGHT VCC),
                 "\"columns\" must be 1 to 40");
    expect_error("columns 0", device_file("B", "0", "2", "'A00'", "'yellow-green'", DATA, TIMING,
                                          EXEC, BACKLIGHT VCC),
                 "\"columns\" must be 1 to 40");
    expect_error("rows 3", device_file("B", "16", "3", "'A00'", "'yellow-green'", DATA, TIMING,
                                       EXEC, BACKLIGHT VCC),
                 "\"rows\" must be 1, 2 or 4");
    expect_error("rom A02", device_file("B", "16", "2", "'A02'", "'yellow-green'", DATA, TIMING,
                                        EXEC, BACKLIGHT VCC),
                 "unknown rom \"A02\"");
    expect_error("panel green", device_file("B", "16", "2", "'A00'", "'green'", DATA, TIMING,
                                            EXEC, BACKLIGHT VCC),
                 "\"panel\" must be yellow-green, blue-negative, gray or white");
    expect_error("7 data pins", device_file("B", "16", "2", "'A00'", "'yellow-green'",
                                            "['DB0','DB1','DB2','DB3','DB4','DB5','DB6']",
                                            TIMING, EXEC, BACKLIGHT VCC),
                 "\"data\" must be 8 pin names, DB0 first");
    expect_error("data pin with no pin", device_file("B", "16", "2", "'A00'", "'yellow-green'",
                                                     "['DB0','DB1','DB2','DB3','DB4','DB5','DB6','DB9']",
                                                     TIMING, EXEC, BACKLIGHT VCC),
                 "no pin named \"DB9\"");
    expect_error("backlight anode with no pin",
                 device_file("B", "16", "2", "'A00'", "'yellow-green'", DATA, TIMING, EXEC,
                             "'backlight':{'anode':'A','cathode':'LED-','color':'White'}," VCC),
                 "no pin named \"A\"");
    expect_error("exec powerOn 0",
                 device_file("B", "16", "2", "'A00'", "'yellow-green'", DATA, TIMING,
                             "{'instruction':37000,'data':37000,'clearHome':1520000,'powerOn':0}",
                             BACKLIGHT VCC),
                 "\"exec.powerOn\" must be more than 0");
    expect_error("timing tpw 0",
                 device_file("B", "16", "2", "'A00'", "'yellow-green'", DATA,
                             "{'tc':1200,'tpw':0,'tas':0,'tah':10,'tdsw':40,'th':10,'tddr':100,'tdhr':10}",
                             EXEC, BACKLIGHT VCC),
                 "\"timing.tpw\" must be more than 0");
    expect_error("timing tc 0",
                 device_file("B", "16", "2", "'A00'", "'yellow-green'", DATA,
                             "{'tc':0,'tpw':140,'tas':0,'tah':10,'tdsw':40,'th':10,'tddr':100,'tdhr':10}",
                             EXEC, BACKLIGHT VCC),
                 "\"timing.tc\" must be more than 0");
    expect_error("timing without tdhr",
                 device_file("B", "16", "2", "'A00'", "'yellow-green'", DATA,
                             "{'tc':1200,'tpw':140,'tas':0,'tah':10,'tdsw':40,'th':10,'tddr':100}",
                             EXEC, BACKLIGHT VCC),
                 "missing \"timing.tdhr\"");
    expect_error("no vcc", device_file("B", "16", "2", "'A00'", "'yellow-green'", DATA, TIMING,
                                       EXEC, BACKLIGHT),
                 "missing \"vcc\"");
}

/* ---- plan-35-D case 1: loader ---------------------------------------------- */

static int bus_is(const Part *part, const char *const *names, size_t count) {
    const PartCharLcd *l = part->charlcd;
    size_t i;

    if (l->bus_pin_count != count) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (strcmp(part->pins[l->bus_pins[i]].name, names[i]) != 0) {
            return 0;
        }
    }
    return 1;
}

static void check_loader_large(void) {
    static const char *const BUS_ONE[] = {"RS", "R/W", "E", "DB0", "DB1", "DB2", "DB3",
                                          "DB4", "DB5", "DB6", "DB7"};
    static const char *const BUS_TWO[] = {"RS", "R/W", "E1", "E2", "DB0", "DB1", "DB2",
                                          "DB3", "DB4", "DB5", "DB6", "DB7"};
    const Part *part;
    const PartCharLcd *l;

    part = part_library_find(library, "FX-2004");
    l = part != NULL ? part->charlcd : NULL;
    expect(l != NULL && l->columns == 20 && l->rows == 4 && l->enables == 1 &&
               strcmp(l->panel, "gray") == 0 && l->backlight_count == 1 &&
               strcmp(l->backlight_colors[0], "White") == 0 &&
               l->backlight_color == l->backlight_colors[0] && bus_is(part, BUS_ONE, 11),
           "D1 loader: 20x4 on one E loads (rows 4, panel gray, enables 1, 1 LED, bus of 11)");
    part = part_library_find(library, "FX-4004");
    l = part != NULL ? part->charlcd : NULL;
    expect(l != NULL && l->columns == 40 && l->rows == 4 && l->enables == 2 &&
               strcmp(l->panel, "white") == 0 && l->backlight_count == 1 &&
               bus_is(part, BUS_TWO, 12),
           "D1 loader: 40x4 with two enables loads (enables 2, panel white, bus RS R/W E1 E2 DB0-7: 12)");
    part = part_library_find(library, "FX-RGB");
    l = part != NULL ? part->charlcd : NULL;
    expect(l != NULL && l->rows == 4 && l->enables == 1 && l->backlight_count == 3 &&
               l->backlight_colors != NULL && strcmp(l->backlight_colors[0], "Red") == 0 &&
               strcmp(l->backlight_colors[1], "Green") == 0 &&
               strcmp(l->backlight_colors[2], "Blue") == 0 &&
               l->backlight_color == l->backlight_colors[0] && bus_is(part, BUS_ONE, 11),
           "D1 loader: a 3-LED backlight loads (backlight_count 3: Red, Green, Blue; color = [0])");
    part = part_library_find(library, "FX-LCD");
    l = part != NULL ? part->charlcd : NULL;
    expect(l != NULL && l->enables == 1 && l->backlight_count == 1 &&
               strcmp(l->backlight_colors[0], "White") == 0 && bus_is(part, BUS_ONE, 11),
           "D1 loader: the 16x2 fixture has enables 1, one LED, the same 11 bus pins");
    part = part_library_find(library, "FX-LCD-NB");
    expect(part != NULL && part->charlcd->backlight_count == 0 &&
               part->charlcd->backlight_colors == NULL,
           "D1 loader: no backlight: backlight_count 0, backlight_colors NULL");

    expect_error("rows 4, one E, 21 columns",
                 large_file("B", PINOUT, "p16", "FX16", "21", "4", "'white'", "'E'", BACKLIGHT VCC),
                 "4 rows on one controller need \"columns\" of 20 or less");
    expect_error("rows 3", large_file("B", PINOUT, "p16", "FX16", "20", "3", "'white'", "'E'",
                                      BACKLIGHT VCC),
                 "\"rows\" must be 1, 2 or 4");
    expect_error("two enables on 2 rows",
                 large_file("B", PINOUT_P18_DUAL, "p18-dual", "FX18", "40", "2", "'white'",
                            "['E1','E2']", VCC),
                 "two enables need \"rows\": 4");
    expect_error("three enables",
                 large_file("B", PINOUT_P18_DUAL, "p18-dual", "FX18", "40", "4", "'white'",
                            "['E1','E2','E1']", VCC),
                 "\"e\" must be a pin name or 2 pin names");
    expect_error("one enable in an array",
                 large_file("B", PINOUT_P18_DUAL, "p18-dual", "FX18", "40", "4", "'white'",
                            "['E1']", VCC),
                 "\"e\" must be a pin name or 2 pin names");
    expect_error("e a number",
                 large_file("B", PINOUT_P18_DUAL, "p18-dual", "FX18", "40", "4", "'white'", "9",
                            VCC),
                 "\"e\" must be a pin name or 2 pin names");
    expect_error("E2 with no pin",
                 large_file("B", PINOUT_P18_DUAL, "p18-dual", "FX18", "40", "4", "'white'",
                            "['E1','E3']", VCC),
                 "no pin named \"E3\"");
    expect_error("panel black", large_file("B", PINOUT, "p16", "FX16", "20", "4", "'black'", "'E'",
                                           BACKLIGHT VCC),
                 "\"panel\" must be yellow-green, blue-negative, gray or white");
    expect_error("backlight of 4 LEDs",
                 large_file("B", PINOUT_P18_RGB, "p18-rgb", "FX18", "20", "4", "'white'", "'E'",
                            "'backlight':[{'anode':'LED+','cathode':'K(R)','color':'Red'},"
                            "{'anode':'LED+','cathode':'K(G)','color':'Green'},"
                            "{'anode':'LED+','cathode':'K(B)','color':'Blue'},"
                            "{'anode':'LED+','cathode':'K(B)','color':'Blue'}]," VCC),
                 "\"backlight\" must be an object or 1 to 3 objects");
    expect_error("backlight empty array",
                 large_file("B", PINOUT_P18_RGB, "p18-rgb", "FX18", "20", "4", "'white'", "'E'",
                            "'backlight':[]," VCC),
                 "\"backlight\" must be an object or 1 to 3 objects");
    expect_error("backlight a string",
                 large_file("B", PINOUT_P18_RGB, "p18-rgb", "FX18", "20", "4", "'white'", "'E'",
                            "'backlight':'White'," VCC),
                 "\"backlight\" must be an object or 1 to 3 objects");
    expect_error("backlight array of a string",
                 large_file("B", PINOUT_P18_RGB, "p18-rgb", "FX18", "20", "4", "'white'", "'E'",
                            "'backlight':['White']," VCC),
                 "\"backlight\" must be an object or 1 to 3 objects");
    expect_error("backlight LED without a color",
                 large_file("B", PINOUT_P18_RGB, "p18-rgb", "FX18", "20", "4", "'white'", "'E'",
                            "'backlight':[{'anode':'LED+','cathode':'K(R)','color':'Red'},"
                            "{'anode':'LED+','cathode':'K(G)'}]," VCC),
                 "backlight[1]");
    expect_error("backlight cathode with no pin",
                 large_file("B", PINOUT_P18_RGB, "p18-rgb", "FX18", "20", "4", "'white'", "'E'",
                            "'backlight':[{'anode':'LED+','cathode':'K(R)','color':'Red'},"
                            "{'anode':'LED+','cathode':'K(W)','color':'White'}]," VCC),
                 "no pin named \"K(W)\"");
}

/* ---- Cases 1-13 ---- */

static void check_power_up(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    uint8_t v, k;
    unsigned i;
    int spaces = 1;

    at(&l, MS(1));
    wr(&l, 1, 'A');
    rd(&l, 0, &v, &k);
    expect((k & 0x80) && (v & 0x80), "1 power-up: at 1 ms BF = 1");
    at(&l, MS(41));
    rd(&l, 0, &v, &k);
    expect(k == 0xFF && v == 0x00, "1 power-up: at 41 ms BF = 0 and AC = 0");
    lcd_state(&l, &st);
    for (i = 0; i < 80; i++) {
        spaces = spaces && st.ddram[0][i] == 0x20 && st.ddram_known[0][i] == 0xFF;
    }
    expect(spaces, "1 power-up: the write at 1 ms was not accepted (DDRAM all 20)");
    expect(image_all(&st, LCD_DOT_OFF), "1 power-up: display off (D = 0): every dot off");
    bench_destroy(l.b);
}

static void check_init8(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;

    init8(&l);
    expect(read_ac(&l) == 0x67, "2 8-bit init: AC = 67 after the example (10 wraps from 00)");
    clear(&l);
    text(&l, "HELLO");
    lcd_state(&l, &st);
    expect(memcmp(st.ddram[0], "HELLO", 5) == 0, "2 8-bit init: ddram[0..4] = 48 45 4C 4C 4F");
    expect(read_ac(&l) == 0x05, "2 8-bit init: AC = 05");
    expect(cell_is_rom(&st, 0, 0, 0x48) && cell_is_rom(&st, 0, 4, 0x4F),
           "2 8-bit init: cell 0 shows ROM 48, cell 4 ROM 4F");
    expect(row_all(&st, 1, 16, LCD_DOT_OFF), "2 8-bit init: row 1 all off (spaces)");
    bench_destroy(l.b);
}

static void check_init4(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    uint8_t v1, k1, v2, k2;
    int ok;

    at(&l, MS(100));
    access_write(&l, 0, 0x30, 0xF0);
    bench_run(l.b, MS(5));
    access_write(&l, 0, 0x30, 0xF0);
    bench_run(l.b, US(160));
    access_write(&l, 0, 0x30, 0xF0);
    bench_run(l.b, US(160));
    access_write(&l, 0, 0x20, 0xF0);
    bench_run(l.b, US(40));
    l.four = 1;
    cmd(&l, 0x28, US(40));
    cmd(&l, 0x10, US(40));
    cmd(&l, 0x0F, US(40));
    cmd(&l, 0x06, US(40));
    clear(&l);
    text(&l, "Hi");
    access_read(&l, 0, &v1, &k1);
    access_read(&l, 0, &v2, &k2);
    expect((k1 & 0xF0) == 0xF0 && (v1 >> 4) == 0x0 && (k2 & 0xF0) == 0xF0 && (v2 >> 4) == 0x2,
           "3 4-bit init: BF/AC read in two nibbles gives 0 then 2");
    expect(l.db_low_z, "3 4-bit init: DB0-3 stay Z through every read");
    lcd_state(&l, &st);
    expect(memcmp(st.ddram[0], "Hi", 2) == 0 && cell_is_rom(&st, 0, 0, 'H') &&
               cell_is_rom(&st, 0, 1, 'i'),
           "3 4-bit init: \"Hi\" in ddram[0..1] and on the face");
    at(&l, blink_time(now_ps(&l), 1));
    lcd_state(&l, &st);
    ok = cell_all(&st, 0, 2, LCD_DOT_ON);
    at(&l, blink_time(now_ps(&l), 0));
    lcd_state(&l, &st);
    {
        static const uint8_t CURSOR[8] = {0, 0, 0, 0, 0, 0, 0, 0x1F};

        ok = ok && cell_rows_are(&st, 0, 2, CURSOR);
    }
    expect(ok, "3 4-bit init: 0F made cell 2 blink (all on, then the cursor line)");
    bench_destroy(l.b);
}

static void check_busy(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    uint8_t v, k;
    SimTime fall;

    init8(&l);
    clear(&l);
    wr(&l, 1, 'A');
    bench_run(l.b, US(10));
    wr(&l, 1, 'B');
    bench_run(l.b, US(40));
    lcd_state(&l, &st);
    expect(st.ddram[0][0] == 'A' && st.ddram[0][1] == 0x20,
           "4 busy: a data write 10 us after another is not accepted");
    wr(&l, 0, 0x01);
    fall = now_ps(&l) - US(2);
    at(&l, fall + US(1500) - NS(50));
    rd(&l, 0, &v, &k);
    expect((k & 0x80) && (v & 0x80), "4 busy: Clear: BF = 1 at + 1.5 ms");
    at(&l, fall + US(1530) - NS(50));
    rd(&l, 0, &v, &k);
    expect((k & 0x80) && !(v & 0x80), "4 busy: Clear: BF = 0 at + 1.53 ms");
    bench_destroy(l.b);
}

static void check_clear_home(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    unsigned i;
    int spaces = 1;

    init8(&l);
    text(&l, "xyz");
    cmd(&l, 0x04, US(40));   /* I/D = 0 */
    clear(&l);
    lcd_state(&l, &st);
    for (i = 0; i < 80; i++) {
        spaces = spaces && st.ddram[0][i] == 0x20 && st.ddram_known[0][i] == 0xFF;
    }
    expect(spaces && read_ac(&l) == 0, "5 clear: DDRAM all 20, AC 0");
    text(&l, "AB");
    expect(read_ac(&l) == 2, "5 clear: I/D = 1 again (two writes go right, AC 2)");
    cmd(&l, 0x18, US(40));   /* shift the display left */
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 0, 0, 'B'), "5 home: after a shift, column 0 shows address 01");
    cmd(&l, 0x02, MS(2));
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 0, 0, 'A') && cell_is_rom(&st, 0, 1, 'B') && st.ddram[0][0] == 'A' &&
               st.ddram[0][1] == 'B' && read_ac(&l) == 0,
           "5 home: shift back to 0, AC 0, DDRAM unchanged");
    bench_destroy(l.b);
}

static void check_entry_shift(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;

    init8(&l);
    clear(&l);
    cmd(&l, 0x04, US(40));
    cmd(&l, 0x85, US(40));
    text(&l, "AB");
    lcd_state(&l, &st);
    expect(st.ddram[0][5] == 'A' && st.ddram[0][4] == 'B' && read_ac(&l) == 0x03,
           "6 entry mode: I/D = 0 writes go leftward (05, 04; AC 03)");
    clear(&l);
    cmd(&l, 0x06, US(40));
    text(&l, "ABCD");
    cmd(&l, 0x90, US(40));
    cmd(&l, 0x07, US(40));
    text(&l, "xyz");
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 0, 0, 'D'), "6 entry mode: S = 1, 3 writes: column 0 shows address 03");
    bench_destroy(l.b);
}

static void check_wrap(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;

    init8(&l);
    clear(&l);
    cmd(&l, 0xA7, US(40));
    text(&l, "PQ");
    lcd_state(&l, &st);
    expect(st.ddram[0][39] == 'P' && st.ddram[0][40] == 'Q' && read_ac(&l) == 0x41,
           "7 2-line wrap: 27 then 40 (indices 39, 40), AC 41");
    cmd(&l, 0x80, US(40));
    cmd(&l, 0x10, US(40));
    expect(read_ac(&l) == 0x67, "7 2-line wrap: decrement from 00 lands at 67");
    bench_destroy(l.b);
}

static void check_no_cell(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState before, after;
    uint8_t v, k;

    init8(&l);
    clear(&l);
    lcd_state(&l, &before);
    cmd(&l, 0xA8, US(40));
    text(&l, "Z");
    lcd_state(&l, &after);
    expect(memcmp(before.ddram[0], after.ddram[0], 80) == 0 &&
               memcmp(before.ddram_known[0], after.ddram_known[0], 80) == 0,
           "8 no-cell address: a write at 28 changes no cell");
    expect(read_ac(&l) == 0x29, "8 no-cell address: AC = 29");
    cmd(&l, 0xA8, US(40));
    rd(&l, 1, &v, &k);
    expect(k == 0, "8 no-cell address: a data read there drives X");
    bench_destroy(l.b);
}

static void check_cgram(void) {
    static const uint8_t GLYPH[8] = {0x1F, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x1F, 0x00};
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    unsigned i;

    init8(&l);
    clear(&l);
    cmd(&l, 0x40, US(40));
    for (i = 0; i < 8; i++) {
        wr(&l, 1, GLYPH[i]);
        bench_run(l.b, US(40));
    }
    cmd(&l, 0x80, US(40));
    wr(&l, 1, 0x00);
    bench_run(l.b, US(40));
    wr(&l, 1, 0x08);
    bench_run(l.b, US(40));
    lcd_state(&l, &st);
    expect(cell_rows_are(&st, 0, 0, GLYPH) && cell_rows_are(&st, 0, 1, GLYPH),
           "9 CGRAM: codes 00 and 08 both render CGRAM glyph 0");
    cmd(&l, 0x41, US(40));
    access_write(&l, 1, 0x0E, 0xFE);   /* row 1 = 0111X: DB0 released */
    bench_run(l.b, US(40));
    lcd_state(&l, &st);
    expect(dot(&st, 0, 0, 1, 4) == LCD_DOT_X && dot(&st, 0, 0, 1, 0) == LCD_DOT_OFF &&
               dot(&st, 0, 0, 1, 1) == LCD_DOT_ON && dot(&st, 0, 0, 0, 4) == LCD_DOT_ON,
           "9 CGRAM: a row with an X bit renders that dot X, the rest as written");
    bench_destroy(l.b);
}

static void check_cursor_blink(void) {
    static const uint8_t CURSOR[8] = {0, 0, 0, 0, 0, 0, 0, 0x1F};
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    int ok;

    init8(&l);
    clear(&l);
    text(&l, "A");
    cmd(&l, 0x0E, US(40));
    lcd_state(&l, &st);
    expect(cell_rows_are(&st, 0, 1, CURSOR) && cell_is_rom(&st, 0, 0, 'A'),
           "10 cursor: C = 1: cell AC's row 7 is on");
    cmd(&l, 0x0D, US(40));   /* C = 0, B = 1 */
    at(&l, blink_time(now_ps(&l), 1));
    lcd_state(&l, &st);
    ok = cell_all(&st, 0, 1, LCD_DOT_ON) && cell_is_rom(&st, 0, 0, 'A');
    at(&l, blink_time(now_ps(&l), 0));
    lcd_state(&l, &st);
    ok = ok && cell_all(&st, 0, 1, LCD_DOT_OFF);
    expect(ok, "10 blink: B = 1: all dots on in the even phase, the glyph in the odd");
    bench_destroy(l.b);
}

static void check_display_off(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;

    init8(&l);
    clear(&l);
    text(&l, "A");
    cmd(&l, 0x08, US(40));
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_OFF) && st.ddram[0][0] == 'A',
           "11 display off: every dot off, DDRAM kept");
    bench_destroy(l.b);
}

static void check_one_line(void) {
    Lcd l = lcd_open("FX-LCD");
    LcdState st;
    unsigned c;
    int ok = 1;

    at(&l, MS(100));
    cmd(&l, 0x30, MS(5));
    cmd(&l, 0x30, US(160));
    cmd(&l, 0x30, US(160));
    cmd(&l, 0x30, US(40));   /* N = 0 */
    cmd(&l, 0x0C, US(40));
    cmd(&l, 0x06, US(40));
    clear(&l);
    text(&l, "0123456789ABCDEFG");
    lcd_state(&l, &st);
    for (c = 0; c < 16; c++) {
        ok = ok && cell_is_rom(&st, 0, c, (unsigned)"0123456789ABCDEF"[c]);
    }
    expect(ok, "12 1-line: row 0 shows 16 cells from address 00");
    expect(row_all(&st, 1, 16, LCD_DOT_OFF), "12 1-line: row 1 off");
    cmd(&l, 0xCF, US(40));
    text(&l, "Z");
    lcd_state(&l, &st);
    expect(st.ddram[0][79] == 'Z' && read_ac(&l) == 0x00, "12 1-line: AC wraps 4F -> 00");
    bench_destroy(l.b);
}

static void check_read_timing(void) {
    Lcd l = lcd_open("FX-LCD");

    init8(&l);
    clear(&l);
    lv(&l, "RS", 0);
    lv(&l, "R/W", 1);
    put_db(&l, 0, 0);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(50));
    expect(db_all(&l, LOGIC_X), "13 read timing: DB X from E rise (at + 50 ns)");
    bench_run(l.b, NS(60));
    expect(db_all(&l, LOGIC_0), "13 read timing: BF/AC data from + 100 ns (00 at + 110 ns)");
    bench_run(l.b, NS(390));
    lv(&l, "E", 0);
    bench_run(l.b, NS(5));
    expect(db_all(&l, LOGIC_0), "13 read timing: data held at E fall + 5 ns");
    bench_run(l.b, NS(10));
    expect(db_all(&l, LOGIC_Z), "13 read timing: Z after E fall + 10 ns");
    bench_destroy(l.b);
}

/* ---- Cases 14-16: violations, X, LOST ---- */

/* An LCD after the 8-bit init and Clear, AC 0. */
static Lcd ready_lcd(void) {
    Lcd l = lcd_open("FX-LCD");

    init8(&l);
    clear(&l);
    return l;
}

/* Cell 0 became X (all 8 bits, every dot) and AC moved to 01. */
static int cell0_lost_ac1(Lcd *l) {
    LcdState st;

    bench_run(l->b, US(40));
    lcd_state(l, &st);
    return st.ddram_known[0][0] == 0 && cell_all(&st, 0, 0, LCD_DOT_X) && read_ac(l) == 1;
}

static void check_write_violations(void) {
    Lcd l;
    uint8_t v, k;

    l = ready_lcd();
    lv(&l, "RS", 1);
    put_db(&l, 'A', 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(100));
    lv(&l, "E", 0);
    bench_run(l.b, US(2));
    expect(cell0_lost_ac1(&l), "14 violation: E high 100 ns (< tpw 140): the cell is X, AC moves");
    bench_destroy(l.b);

    l = ready_lcd();
    lv(&l, "RS", 1);
    put_db(&l, 'A', 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(480));
    put_db(&l, 'B', 0xFF);
    bench_run(l.b, NS(20));
    lv(&l, "E", 0);
    bench_run(l.b, US(2));
    expect(cell0_lost_ac1(&l), "14 violation: data changing 20 ns before E fall (< tdsw 40): the cell is X");
    bench_destroy(l.b);

    l = ready_lcd();
    lv(&l, "RS", 1);
    put_db(&l, 'A', 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E", 0);
    bench_run(l.b, NS(5));
    put_db(&l, 'B', 0xFF);
    bench_run(l.b, US(2));
    expect(cell0_lost_ac1(&l), "14 violation: data changing 5 ns after E fall (< th 10): the cell is X");
    bench_destroy(l.b);

    l = ready_lcd();
    lv(&l, "RS", 1);
    put_db(&l, 'A', 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E", 0);
    bench_run(l.b, NS(5));
    lv(&l, "RS", 0);
    bench_run(l.b, US(2));
    expect(cell0_lost_ac1(&l), "14 violation: RS changing 5 ns after E fall (< tah 10): the cell is X");
    bench_destroy(l.b);

    /* E rising 1000 ns after the last rise; the last access is a read, so
       the write is not refused as busy. */
    l = ready_lcd();
    lv(&l, "RS", 0);
    lv(&l, "R/W", 1);
    put_db(&l, 0, 0);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E", 0);
    bench_run(l.b, NS(450));
    lv(&l, "RS", 1);
    lv(&l, "R/W", 0);
    put_db(&l, 'A', 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E", 0);
    bench_run(l.b, US(2));
    expect(cell0_lost_ac1(&l), "14 violation: E rising 1000 ns after the last rise (< tc 1200): the cell is X");
    (void)v;
    (void)k;
    bench_destroy(l.b);
}

static void check_lost(void) {
    Lcd l = ready_lcd();
    LcdState st;
    uint8_t v, k;

    /* An instruction write with RS X. */
    bench_set(l.b, "RS", LOGIC_Z);
    lv(&l, "R/W", 0);
    put_db(&l, 0x0F, 0xFF);
    bench_run(l.b, NS(50));
    lv(&l, "E", 1);
    bench_run(l.b, NS(500));
    lv(&l, "E", 0);
    bench_run(l.b, US(40));
    lcd_state(&l, &st);
    rd(&l, 0, &v, &k);
    expect(image_all(&st, LCD_DOT_X) && st.busy[0] == -1 && !(k & 0x80),
           "15 LOST: an instruction write with RS X: every dot X, BF reads X");
    bench_run(l.b, MS(1));
    cmd(&l, 0x30, MS(5));
    cmd(&l, 0x30, US(160));
    cmd(&l, 0x30, US(160));
    cmd(&l, 0x38, US(40));
    cmd(&l, 0x10, US(40));
    cmd(&l, 0x0C, US(40));
    cmd(&l, 0x06, US(40));
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_X) && st.busy[0] == 0,
           "15 resync: after the init sequence BF is known; the X DDRAM cells show X");
    clear(&l);
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_OFF), "15 resync: after Clear every dot is off");
    text(&l, "HELLO");
    lcd_state(&l, &st);
    expect(memcmp(st.ddram[0], "HELLO", 5) == 0 && cell_is_rom(&st, 0, 0, 'H') &&
               cell_is_rom(&st, 0, 4, 'O'),
           "15 resync: HELLO shows");
    bench_destroy(l.b);

    /* E reading X after the power-on reset: LOST. */
    l = ready_lcd();
    bench_set(l.b, "E", LOGIC_Z);
    bench_run(l.b, US(1));
    bench_set(l.b, "E", LOGIC_0);
    bench_run(l.b, US(1));
    lcd_state(&l, &st);
    expect(st.busy[0] == -1 && st.ddram_known[0][0] == 0, "15 LOST: E reading X makes every field X");
    bench_destroy(l.b);

    /* E reading X with R/W 1 is a possible read: only AC goes X. */
    l = ready_lcd();
    text(&l, "AB");
    lv(&l, "R/W", 1);
    bench_set(l.b, "RS", LOGIC_Z);
    bench_run(l.b, NS(50));
    bench_set(l.b, "E", LOGIC_Z);
    bench_run(l.b, US(1));
    bench_set(l.b, "E", LOGIC_0);
    bench_run(l.b, US(1));
    lcd_state(&l, &st);
    {
        uint8_t v, k;

        rd(&l, 0, &v, &k);
        expect(st.busy[0] == 0 && st.ddram[0][0] == 'A' && st.ddram[0][1] == 'B' && st.ddram_known[0][0] == 0xFF &&
                   cell_is_rom(&st, 0, 0, 'A') && (k & 0x80) && !(v & 0x80) && (k & 0x7F) == 0,
               "15 E X with R/W 1 (a possible read): BF and RAM known, only AC X");
        cmd(&l, 0x80, US(40));
        expect(read_ac(&l) == 0, "15 E X with R/W 1: Set DDRAM Address makes AC known again");
    }
    bench_destroy(l.b);

    /* E reading X during the power-on reset is ignored (see Corrections). */
    l = lcd_open("FX-LCD");
    bench_set(l.b, "E", LOGIC_Z);
    bench_run(l.b, MS(1));
    bench_set(l.b, "E", LOGIC_0);
    at(&l, MS(41));
    lcd_state(&l, &st);
    expect(st.busy[0] == 0 && st.ddram_known[0][0] == 0xFF && read_ac(&l) == 0,
           "15 power-on: E X before exec.powerOn does not make the controller LOST");
    bench_destroy(l.b);
}

static void check_partial_x(void) {
    static const uint8_t X_CURSOR[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    Lcd l = ready_lcd();
    LcdState st;
    unsigned x;
    int ok;

    text(&l, "A");
    access_write(&l, 0, 0x0C, 0xFD);   /* 0000 11X0: DB1 released */
    bench_run(l.b, US(40));
    lcd_state(&l, &st);
    ok = cell_is_rom(&st, 0, 0, 'A') && row_all(&st, 1, 16, LCD_DOT_OFF);
    for (x = 0; x < 5; x++) {
        ok = ok && dot(&st, 0, 1, 7, x) == LCD_DOT_X;
    }
    {
        LcdState copy = st;
        unsigned y;

        for (y = 0; y < 7; y++) {
            for (x = 0; x < 5; x++) {
                ok = ok && dot(&copy, 0, 1, y, x) == LCD_DOT_OFF;
            }
        }
    }
    for (x = 2; x < 16; x++) {
        ok = ok && cell_rows_are(&st, 0, x, X_CURSOR);
    }
    expect(ok, "16 partial X: 0000 11X0 makes only C X: the cursor cell's row 7 is X, every other dot right");
    bench_destroy(l.b);
}

int main(int argc, char **argv) {
    char *error;

    if (argc > 1 && strcmp(argv[1], "--font") == 0) {
        print_font();
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
    check_loader();
    check_loader_large();
    check_rom();
    check_power_up();
    check_init8();
    check_init4();
    check_busy();
    check_clear_home();
    check_entry_shift();
    check_wrap();
    check_no_cell();
    check_cgram();
    check_cursor_blink();
    check_display_off();
    check_one_line();
    check_read_timing();
    check_write_violations();
    check_lost();
    check_partial_x();
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    part_library_destroy(real_library);
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_test: all passed\n");
    return 0;
}
