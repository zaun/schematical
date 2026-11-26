/* charlcd_bench: shared by the programs split from charlcd_test.c (plan-68-H). */
#include "charlcd_bench.h"


int failures = 0;
PartLibrary *library;

void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

const char FOOTPRINTS[] =
    "{'format':'schematical-footprints','version':1,'footprints':[{'id':'FX16',"
    "'description':'d','style':'dual','pins':16,'body':{'width':1,'height':1},'pitch':1,"
    "'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id':'FX18',"
    "'description':'d','style':'dual','pins':18,'body':{'width':1,'height':1},'pitch':1,"
    "'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}]}";

char *dequote(const char *text) {
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
char *device_file(const char *name, const char *columns, const char *rows,
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

/*
 * A plan-35-D device file: as device_file, with the pinouts text, its pinout
 * id, the footprint and the "bus".e value given; the timing and exec are the
 * fixture's; `extra` holds the backlight and vcc pairs.
 */
char *large_file(const char *name, const char *pinouts, const char *pinout,
                        const char *footprint, const char *columns, const char *rows,
                        const char *panel, const char *e, const char *extra) {
    static char text[8192];

    snprintf(text, sizeof text,
             "{'format':'schematical-parts','version':1,'kind':'charlcd','devices':[{"
             "'id':'%s','function':'%sx%s','description':'d','drawer':[],%s,"
             "'columns':%s,'rows':%s,'controller':'ST7066U','rom':'A00','panel':%s,"
             "'bus':{'rs':'RS','rw':'R/W','e':%s,'data':" DATA "},'contrast':'V0',%s"
             "'timing':" TIMING ",'exec':" EXEC ","
             "'parts':[{'name':'%s','package':'Module','pinout':'%s','footprint':'%s'}]}]}",
             name, columns, rows, pinouts, columns, rows, panel, e, extra, name, pinout,
             footprint);
    return dequote(text);
}

char *load_file(PartLibrary *lib, char *json, const char *source) {
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

PartLibrary *fixture_library(void) {
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

/* ---- Character ROM (section 5.2) ------------------------------------------ */

int glyph_is(unsigned code, const uint8_t rows[8]) {
    return memcmp(CHARLCD_ROM_A00[code], rows, 8) == 0;
}

void check_rom(void) {
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
void print_font(void) {
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

const char *const DB[8] = {"DB0", "DB1", "DB2", "DB3", "DB4", "DB5", "DB6", "DB7"};

/* The bus pin names from part->charlcd->bus_pins: RS, R/W, E (E1, E2),
   DB0-DB7. pins_ok: the counts are right and each bus pin is the first pin
   of its name (the one the bench drives). */
void lcd_pins(Lcd *l) {
    const PartCharLcd *c = l->part->charlcd;
    const char **names[12];
    size_t i, n = 0, j;

    names[n++] = &l->rs;
    names[n++] = &l->rw;
    l->enables = c != NULL && c->enables == 2 ? 2 : 1;
    names[n++] = &l->en[0];
    if (l->enables == 2) {
        names[n++] = &l->en[1];
    }
    for (i = 0; i < 8; i++) {
        names[n++] = &l->db[i];
    }
    l->pins_ok = c != NULL && c->bus_pin_count == n;
    for (i = 0; i < n; i++) {
        size_t index = l->pins_ok ? c->bus_pins[i] : 0;

        if (!l->pins_ok || index >= l->part->pin_count) {
            fprintf(stderr, "%s: bad bus_pins\n", l->part->name);
            exit(1);
        }
        *names[i] = l->part->pins[index].name;
        for (j = 0; j < index; j++) {
            l->pins_ok = l->pins_ok && strcmp(l->part->pins[j].name, *names[i]) != 0;
        }
    }
    if (l->enables == 1) {
        l->en[1] = NULL;
    }
}

Lcd lcd_open(const char *name) {
    Lcd l;
    unsigned i;

    memset(&l, 0, sizeof l);
    l.part = part_library_find(library, name);
    if (l.part == NULL) {
        fprintf(stderr, "no part %s\n", name);
        exit(1);
    }
    l.b = bench_create(l.part);
    l.db_low_z = 1;
    l.setup = NS(50);
    l.high = NS(500);
    l.after = US(2);
    lcd_pins(&l);
    l.e = l.en[0];
    for (i = 0; i < l.enables; i++) {
        bench_set(l.b, l.en[i], LOGIC_0);
    }
    bench_set(l.b, l.rs, LOGIC_0);
    bench_set(l.b, l.rw, LOGIC_0);
    bench_run(l.b, 1);
    return l;
}

SimTime now_ps(Lcd *l) {
    return sim_now(bench_sim(l->b));
}

void at(Lcd *l, SimTime t) {
    if (t > now_ps(l)) {
        bench_run(l->b, t - now_ps(l));
    }
}

void lv(Lcd *l, const char *pin, int bit) {
    bench_set(l->b, pin, bit ? LOGIC_1 : LOGIC_0);
}

/* Drives DB pins in mask with v's bits; the others are released (Z). */
void put_db(Lcd *l, uint8_t v, uint8_t mask) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        if ((mask >> i) & 1u) {
            lv(l, l->db[i], (v >> i) & 1u);
        } else {
            bench_set(l->b, l->db[i], LOGIC_Z);
        }
    }
}

/* One write access: setup, E high, after (by default 50 ns, 500 ns, 2 us:
   every setup and hold met). */
void access_write(Lcd *l, int rs, uint8_t v, uint8_t mask) {
    lv(l, l->rs, rs);
    lv(l, l->rw, 0);
    put_db(l, v, mask);
    bench_run(l->b, l->setup);
    lv(l, l->e, 1);
    bench_run(l->b, l->high);
    lv(l, l->e, 0);
    bench_run(l->b, l->after);
}

/* One read access; the DB values sampled just before E falls. */
void access_read(Lcd *l, int rs, uint8_t *v, uint8_t *k) {
    unsigned i;

    lv(l, l->rs, rs);
    lv(l, l->rw, 1);
    put_db(l, 0, 0);
    bench_run(l->b, l->setup);
    lv(l, l->e, 1);
    bench_run(l->b, l->high);
    *v = *k = 0;
    for (i = 0; i < 8; i++) {
        LogicValue x = bench_get(l->b, l->db[i]);

        if (x == LOGIC_0 || x == LOGIC_1) {
            *k |= (uint8_t)(1u << i);
            *v |= (uint8_t)((x == LOGIC_1) << i);
        }
        if (l->four && i < 4 && x != LOGIC_Z) {
            l->db_low_z = 0;
        }
    }
    lv(l, l->e, 0);
    bench_run(l->b, l->after);
    lv(l, l->rw, 0);
}

void wr(Lcd *l, int rs, uint8_t byte) {
    if (l->four) {
        access_write(l, rs, byte & 0xF0, 0xF0);
        access_write(l, rs, (uint8_t)(byte << 4), 0xF0);
    } else {
        access_write(l, rs, byte, 0xFF);
    }
}

/* A byte write, then gap. */
void cmd(Lcd *l, uint8_t byte, SimTime gap) {
    wr(l, 0, byte);
    bench_run(l->b, gap);
}

/* Data writes 50 us apart: longer than every device's data exec time
   (37 us Newhaven, 43 us Displaytech and Orient). */
void text(Lcd *l, const char *s) {
    for (; *s != '\0'; s++) {
        wr(l, 1, (uint8_t)*s);
        bench_run(l->b, US(50));
    }
}

/* A read; k = known bits. In 4-bit mode two nibbles, high first. */
void rd(Lcd *l, int rs, uint8_t *v, uint8_t *k) {
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
int read_ac(Lcd *l) {
    uint8_t v, k;

    rd(l, 0, &v, &k);
    return k == 0xFF && !(v & 0x80) ? v & 0x7F : -1;
}

void lcd_state(Lcd *l, LcdState *st) {
    kind_model(l->part)->lcd_state(bench_sim(l->b), bench_state(l->b), st);
}

/* The NHD 8-bit initialization example (PDF p.11), from 100 ms. */
void init8(Lcd *l) {
    at(l, MS(100));
    cmd(l, 0x30, MS(5));
    cmd(l, 0x30, US(160));
    cmd(l, 0x30, US(160));
    cmd(l, 0x38, US(40));
    cmd(l, 0x10, US(40));
    cmd(l, 0x0C, US(40));
    cmd(l, 0x06, US(40));
}

void clear(Lcd *l) {
    cmd(l, 0x01, MS(2));
}

/* ---- Image checks ---- */

uint8_t dot(const LcdState *st, unsigned row, unsigned column, unsigned y, unsigned x) {
    return st->dots[row * 8 + y][column * 5 + x];
}

/* The cell shows ROM glyph `code` (rows 0-6 always; row 7 when with_row7). */
int cell_is_rom(const LcdState *st, unsigned row, unsigned column, unsigned code) {
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

int cell_rows_are(const LcdState *st, unsigned row, unsigned column, const uint8_t rows[8]) {
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

int cell_all(const LcdState *st, unsigned row, unsigned column, uint8_t value) {
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

int row_all(const LcdState *st, unsigned row, unsigned columns, uint8_t value) {
    unsigned c;

    for (c = 0; c < columns; c++) {
        if (!cell_all(st, row, c, value)) {
            return 0;
        }
    }
    return 1;
}

int image_all(const LcdState *st, uint8_t value) {
    return row_all(st, 0, st->columns, value) && row_all(st, 1, st->columns, value);
}

/* A time at or after `from` in the middle of a blink half-period: on (even) or off. */
SimTime blink_time(SimTime from, int on) {
    SimTime p = 379259259ull;
    SimTime m = from / p + 1;

    if ((m % 2 == 0) != on) {
        m++;
    }
    return m * p + p / 2;
}


int db_all(Lcd *l, LogicValue v) {
    unsigned i;

    for (i = 0; i < 8; i++) {
        if (bench_get(l->b, DB[i]) != v) {
            return 0;
        }
    }
    return 1;
}

/* ---- plan-35-E section 4.1: every charlcd part in the library ---- */

PartLibrary *real_library;

/* Row `row` from `column` shows the ROM glyphs of s. */
int cells_show(const LcdState *st, unsigned row, unsigned column, const char *s) {
    for (; *s != '\0'; s++, column++) {
        if (!cell_is_rom(st, row, column, (unsigned char)*s)) {
            return 0;
        }
    }
    return 1;
}
