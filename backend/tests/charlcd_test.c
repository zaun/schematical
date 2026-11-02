/*
 * charlcd_test: the charlcd kind (plan-11-B section 6). An inline fixture
 * device copies the NHD-0216K1Z-NSW-BBW-L facts (plan-11-C section 4.1):
 * 16x2, the Newhaven timing and exec values, a fixture footprint. The bus is
 * driven through tests/support/bench.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "circuit.h"
#include "charlcd_font.h"
#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

static int failures = 0;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

/* ---- Fixture -------------------------------------------------------------- */

#define PINOUT \
    "'pinouts':{'p16':[{'number':1,'name':'VSS','role':'ground'}," \
    "{'number':2,'name':'VDD','role':'power'},{'number':3,'name':'V0','role':'input'}," \
    "{'number':4,'name':'RS','role':'input'},{'number':5,'name':'R/W','role':'input'}," \
    "{'number':6,'name':'E','role':'input'}," \
    "{'number':7,'name':'DB0','role':'inout','group':'data'}," \
    "{'number':8,'name':'DB1','role':'inout','group':'data'}," \
    "{'number':9,'name':'DB2','role':'inout','group':'data'}," \
    "{'number':10,'name':'DB3','role':'inout','group':'data'}," \
    "{'number':11,'name':'DB4','role':'inout','group':'data'}," \
    "{'number':12,'name':'DB5','role':'inout','group':'data'}," \
    "{'number':13,'name':'DB6','role':'inout','group':'data'}," \
    "{'number':14,'name':'DB7','role':'inout','group':'data'}," \
    "{'number':15,'name':'LED+','role':'input'},{'number':16,'name':'LED-','role':'input'}]}"

#define DATA "['DB0','DB1','DB2','DB3','DB4','DB5','DB6','DB7']"
#define TIMING "{'tc':1200,'tpw':140,'tas':0,'tah':10,'tdsw':40,'th':10,'tddr':100,'tdhr':10}"
#define EXEC "{'instruction':37000,'data':37000,'clearHome':1520000,'powerOn':40000000}"
#define BACKLIGHT "'backlight':{'anode':'LED+','cathode':'LED-','color':'White'},"
#define VCC "'vcc':[4.5,5.5],"

static const char FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[{'id':'FX16',"
    "'description':'d','style':'dual','pins':16,'body':{'width':1,'height':1},'pitch':1,"
    "'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}]}";

static char *dequote(const char *text) {
    size_t n = strlen(text);
    char *copy = malloc(n + 1);
    size_t i;

    for (i = 0; i <= n; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    return copy;
}

/*
 * One charlcd device file; each argument is the text of that key's value
 * (or, for `extra`, whole "key":value, pairs ending in a comma).
 */
static char *device_file(const char *name, const char *columns, const char *rows,
                         const char *rom, const char *panel, const char *data,
                         const char *timing, const char *exec, const char *extra) {
    static char text[8192];

    snprintf(text, sizeof text,
             "{'format':'schematical-parts','version':1,'kind':'charlcd','devices':[{"
             "'id':'%s','function':'16x2','description':'d','drawer':[]," PINOUT ","
             "'columns':%s,'rows':%s,'controller':'ST7066U','rom':%s,'panel':%s,"
             "'bus':{'rs':'RS','rw':'R/W','e':'E','data':%s},'contrast':'V0',%s"
             "'timing':%s,'exec':%s,"
             "'parts':[{'name':'%s','package':'Module','pinout':'p16','footprint':'FX16'}]}]}",
             name, columns, rows, rom, panel, data, extra, timing, exec, name);
    return dequote(text);
}

static char *load_file(PartLibrary *lib, char *json, const char *source) {
    char *error = NULL;

    if (load_parts(lib, json, source, &error) == 0) {
        free(error);
        error = NULL;
    } else if (error == NULL) {
        error = strdup("(no message)");
    }
    free(json);
    return error;
}

static PartLibrary *fixture_library(void) {
    PartLibrary *lib = part_library_create();
    char *json = dequote(FOOTPRINTS);
    char *error = NULL;

    if (lib == NULL || load_footprints(lib, json, "fx-footprints.json", &error) != 0) {
        fprintf(stderr, "fixture footprints: %s\n", error != NULL ? error : "(no message)");
        exit(1);
    }
    free(json);
    free(error);
    return lib;
}

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
                 "\"rows\" must be 1 or 2");
    expect_error("rom A02", device_file("B", "16", "2", "'A02'", "'yellow-green'", DATA, TIMING,
                                        EXEC, BACKLIGHT VCC),
                 "unknown rom \"A02\"");
    expect_error("panel green", device_file("B", "16", "2", "'A00'", "'green'", DATA, TIMING,
                                            EXEC, BACKLIGHT VCC),
                 "\"panel\" must be yellow-green or blue-negative");
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

/* ---- Character ROM (section 5.2) ------------------------------------------ */

static int glyph_is(unsigned code, const uint8_t rows[8]) {
    return memcmp(CHARLCD_ROM_A00[code], rows, 8) == 0;
}

static void check_rom(void) {
    static const uint8_t A[8] = {0x0E, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00};
    static const uint8_t ZERO[8] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E, 0x00};
    static const uint8_t YEN[8] = {0x11, 0x0A, 0x1F, 0x04, 0x1F, 0x04, 0x04, 0x00};
    static const uint8_t RIGHT[8] = {0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00, 0x00};
    static const uint8_t LEFT[8] = {0x00, 0x04, 0x08, 0x1F, 0x08, 0x04, 0x00, 0x00};
    static const uint8_t BLANK[8] = {0};
    static const uint8_t FULL[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
    unsigned k;
    int blank = 1;

    expect(glyph_is(0x41, A), "rom: A (41) = 0E 11 11 11 1F 11 11 00");
    expect(glyph_is(0x30, ZERO), "rom: 0 (30) = 0E 11 13 15 19 11 0E 00");
    expect(glyph_is(0x5C, YEN), "rom: yen (5C) = 11 0A 1F 04 1F 04 04 00");
    expect(glyph_is(0x7E, RIGHT), "rom: right arrow (7E) = 00 04 02 1F 02 04 00 00");
    expect(glyph_is(0x7F, LEFT), "rom: left arrow (7F) = 00 04 08 1F 08 04 00 00");
    expect(glyph_is(0x20, BLANK), "rom: space (20) is blank");
    expect(glyph_is(0xFF, FULL), "rom: FF is all 1F");
    for (k = 0; k < 0x20; k++) {
        blank = blank && glyph_is(k, BLANK);
    }
    for (k = 0x80; k <= 0xA0; k++) {
        blank = blank && glyph_is(k, BLANK);
    }
    expect(blank, "rom: 00-1F and 80-A0 are blank");
}

/* The 16 x 16 glyph sheet, laid out as the datasheet table: rows are the
   code's high nibble, columns the low nibble. */
static void print_font(void) {
    unsigned high, low, y, x;

    for (high = 0; high < 16; high++) {
        printf("%X_:\n", high);
        for (y = 0; y < 8; y++) {
            for (low = 0; low < 16; low++) {
                for (x = 0; x < 5; x++) {
                    putchar(CHARLCD_ROM_A00[high * 16 + low][y] & (0x10 >> x) ? '#' : '.');
                }
                putchar(' ');
            }
            putchar('\n');
        }
    }
}

/* ---- Bus helpers (section 6) ---------------------------------------------- */

#define NS(v) ((SimTime)(v) * 1000ull)
#define US(v) ((SimTime)(v) * 1000000ull)
#define MS(v) ((SimTime)(v) * 1000000000ull)

typedef struct {
    Bench *b;
    const Part *part;
    int four;            /* 4-bit mode: bytes go as two nibbles */
    int db_low_z;        /* every 4-bit read saw DB0-3 at Z */
} Lcd;

static const char *const DB[8] = {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"};

static Lcd lcd_open(const char *name) {
    Lcd l;

    memset(&l, 0, sizeof l);
    l.part = part_library_find(library, name);
    if (l.part == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(1);
    }
    l.b = bench_create(l.part);
    l.db_low_z = 1;
    bench_set(l.b, "E", LOGIC_0);
    bench_set(l.b, "RS", LOGIC_0);
    bench_set(l.b, "R/W", LOGIC_0);
    bench_run(l.b, 1);
    return l;
}

static SimTime now_ps(Lcd *l) {
    return sim_now(bench_sim(l->b));
}

static void at(Lcd *l, SimTime t) {
    if (t > now_ps(l)) {
        bench_run(l->b, t - now_ps(l));
    }
}

static void lv(Lcd *l, const char *pin, int bit) {
    bench_set(l->b, pin, bit ? LOGIC_1 : LOGIC_0);
}

/* Drives DB pins in mask with v's bits; the others are released (Z). */
static void put_db(Lcd *l, uint8_t v, uint8_t mask) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        if ((mask >> i) & 1u) {
            lv(l, DB[i], (v >> i) & 1u);
        } else {
            bench_set(l->b, DB[i], LOGIC_Z);
        }
    }
}

/* One write access: E high 500 ns, every setup and hold met, 2 us after. */
static void access_write(Lcd *l, int rs, uint8_t v, uint8_t mask) {
    lv(l, "RS", rs);
    lv(l, "R/W", 0);
    put_db(l, v, mask);
    bench_run(l->b, NS(50));
    lv(l, "E", 1);
    bench_run(l->b, NS(500));
    lv(l, "E", 0);
    bench_run(l->b, US(2));
}

/* One read access; the DB values sampled just before E falls. */
static void access_read(Lcd *l, int rs, uint8_t *v, uint8_t *k) {
    unsigned i;

    lv(l, "RS", rs);
    lv(l, "R/W", 1);
    put_db(l, 0, 0);
    bench_run(l->b, NS(50));
    lv(l, "E", 1);
    bench_run(l->b, NS(500));
    *v = *k = 0;
    for (i = 0; i < 8; i++) {
        LogicValue x = bench_get(l->b, DB[i]);

        if (x == LOGIC_0 || x == LOGIC_1) {
            *k |= (uint8_t)(1u << i);
            *v |= (uint8_t)((x == LOGIC_1) << i);
        }
        if (l->four && i < 4 && x != LOGIC_Z) {
            l->db_low_z = 0;
        }
    }
    lv(l, "E", 0);
    bench_run(l->b, US(2));
    lv(l, "R/W", 0);
}

static void wr(Lcd *l, int rs, uint8_t byte) {
    if (l->four) {
        access_write(l, rs, byte & 0xF0, 0xF0);
        access_write(l, rs, (uint8_t)(byte << 4), 0xF0);
    } else {
        access_write(l, rs, byte, 0xFF);
    }
}

/* A byte write, then gap. */
static void cmd(Lcd *l, uint8_t byte, SimTime gap) {
    wr(l, 0, byte);
    bench_run(l->b, gap);
}

static void text(Lcd *l, const char *s) {
    for (; *s != '\0'; s++) {
        wr(l, 1, (uint8_t)*s);
        bench_run(l->b, US(40));
    }
}

/* A read; k = known bits. In 4-bit mode two nibbles, high first. */
static void rd(Lcd *l, int rs, uint8_t *v, uint8_t *k) {
    if (l->four) {
        uint8_t v1, k1, v2, k2;

        access_read(l, rs, &v1, &k1);
        access_read(l, rs, &v2, &k2);
        *v = (uint8_t)((v1 & 0xF0) | (v2 >> 4));
        *k = (uint8_t)((k1 & 0xF0) | (k2 >> 4));
    } else {
        access_read(l, rs, v, k);
    }
}

/* AC from a BF/AC read, or -1 when BF is not a known 0 or AC is not known. */
static int read_ac(Lcd *l) {
    uint8_t v, k;

    rd(l, 0, &v, &k);
    return k == 0xFF && !(v & 0x80) ? v & 0x7F : -1;
}

static void lcd_state(Lcd *l, LcdState *st) {
    kind_model(l->part)->lcd_state(bench_sim(l->b), bench_state(l->b), st);
}

/* The NHD 8-bit initialization example (PDF p.11), from 100 ms. */
static void init8(Lcd *l) {
    at(l, MS(100));
    cmd(l, 0x30, MS(5));
    cmd(l, 0x30, US(160));
    cmd(l, 0x30, US(160));
    cmd(l, 0x38, US(40));
    cmd(l, 0x10, US(40));
    cmd(l, 0x0C, US(40));
    cmd(l, 0x06, US(40));
}

static void clear(Lcd *l) {
    cmd(l, 0x01, MS(2));
}

/* ---- Image checks ---- */

static uint8_t dot(const LcdState *st, unsigned row, unsigned column, unsigned y, unsigned x) {
    return st->dots[row * 8 + y][column * 5 + x];
}

/* The cell shows ROM glyph `code` (rows 0-6 always; row 7 when with_row7). */
static int cell_is_rom(const LcdState *st, unsigned row, unsigned column, unsigned code) {
    unsigned y, x;

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 5; x++) {
            uint8_t want = CHARLCD_ROM_A00[code][y] & (0x10u >> x) ? LCD_DOT_ON : LCD_DOT_OFF;

            if (dot(st, row, column, y, x) != want) {
                return 0;
            }
        }
    }
    return 1;
}

static int cell_rows_are(const LcdState *st, unsigned row, unsigned column, const uint8_t rows[8]) {
    unsigned y, x;

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 5; x++) {
            if (dot(st, row, column, y, x) != (rows[y] & (0x10u >> x) ? LCD_DOT_ON : LCD_DOT_OFF)) {
                return 0;
            }
        }
    }
    return 1;
}

static int cell_all(const LcdState *st, unsigned row, unsigned column, uint8_t value) {
    unsigned y, x;

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 5; x++) {
            if (dot(st, row, column, y, x) != value) {
                return 0;
            }
        }
    }
    return 1;
}

static int row_all(const LcdState *st, unsigned row, unsigned columns, uint8_t value) {
    unsigned c;

    for (c = 0; c < columns; c++) {
        if (!cell_all(st, row, c, value)) {
            return 0;
        }
    }
    return 1;
}

static int image_all(const LcdState *st, uint8_t value) {
    return row_all(st, 0, st->columns, value) && row_all(st, 1, st->columns, value);
}

/* A time at or after `from` in the middle of a blink half-period: on (even) or off. */
static SimTime blink_time(SimTime from, int on) {
    SimTime p = 379259259ull;
    SimTime m = from / p + 1;

    if ((m % 2 == 0) != on) {
        m++;
    }
    return m * p + p / 2;
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
        spaces = spaces && st.ddram[i] == 0x20 && st.ddram_known[i] == 0xFF;
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
    expect(memcmp(st.ddram, "HELLO", 5) == 0, "2 8-bit init: ddram[0..4] = 48 45 4C 4C 4F");
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
    expect(memcmp(st.ddram, "Hi", 2) == 0 && cell_is_rom(&st, 0, 0, 'H') &&
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
    expect(st.ddram[0] == 'A' && st.ddram[1] == 0x20,
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
        spaces = spaces && st.ddram[i] == 0x20 && st.ddram_known[i] == 0xFF;
    }
    expect(spaces && read_ac(&l) == 0, "5 clear: DDRAM all 20, AC 0");
    text(&l, "AB");
    expect(read_ac(&l) == 2, "5 clear: I/D = 1 again (two writes go right, AC 2)");
    cmd(&l, 0x18, US(40));   /* shift the display left */
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 0, 0, 'B'), "5 home: after a shift, column 0 shows address 01");
    cmd(&l, 0x02, MS(2));
    lcd_state(&l, &st);
    expect(cell_is_rom(&st, 0, 0, 'A') && cell_is_rom(&st, 0, 1, 'B') && st.ddram[0] == 'A' &&
               st.ddram[1] == 'B' && read_ac(&l) == 0,
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
    expect(st.ddram[5] == 'A' && st.ddram[4] == 'B' && read_ac(&l) == 0x03,
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
    expect(st.ddram[39] == 'P' && st.ddram[40] == 'Q' && read_ac(&l) == 0x41,
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
    expect(memcmp(before.ddram, after.ddram, 80) == 0 &&
               memcmp(before.ddram_known, after.ddram_known, 80) == 0,
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
    expect(image_all(&st, LCD_DOT_OFF) && st.ddram[0] == 'A',
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
    expect(st.ddram[79] == 'Z' && read_ac(&l) == 0x00, "12 1-line: AC wraps 4F -> 00");
    bench_destroy(l.b);
}

static int db_all(Lcd *l, LogicValue v) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        if (bench_get(l->b, DB[i]) != v) {
            return 0;
        }
    }
    return 1;
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
    return st.ddram_known[0] == 0 && cell_all(&st, 0, 0, LCD_DOT_X) && read_ac(l) == 1;
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
    expect(image_all(&st, LCD_DOT_X) && st.busy == -1 && !(k & 0x80),
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
    expect(image_all(&st, LCD_DOT_X) && st.busy == 0,
           "15 resync: after the init sequence BF is known; the X DDRAM cells show X");
    clear(&l);
    lcd_state(&l, &st);
    expect(image_all(&st, LCD_DOT_OFF), "15 resync: after Clear every dot is off");
    text(&l, "HELLO");
    lcd_state(&l, &st);
    expect(memcmp(st.ddram, "HELLO", 5) == 0 && cell_is_rom(&st, 0, 0, 'H') &&
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
    expect(st.busy == -1 && st.ddram_known[0] == 0, "15 LOST: E reading X makes every field X");
    bench_destroy(l.b);

    /* E reading X during the power-on reset is ignored (see Corrections). */
    l = lcd_open("FX-LCD");
    bench_set(l.b, "E", LOGIC_Z);
    bench_run(l.b, MS(1));
    bench_set(l.b, "E", LOGIC_0);
    at(&l, MS(41));
    lcd_state(&l, &st);
    expect(st.busy == 0 && st.ddram_known[0] == 0xFF && read_ac(&l) == 0,
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
    expect(circuit_lcd_state(c, 0, &st) == 0 && st.columns == 16 && st.rows == 2 && st.busy == 1,
           "19 api: circuit_lcd_state on the LCD: 0, 16x2, busy at power-up");
    circuit_destroy(c);
}

/* ---- main ----------------------------------------------------------------- */

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
    check_backlight();
    check_voltage();
    check_api();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_test: all passed\n");
    return 0;
}
