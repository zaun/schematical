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
#include "voltage.h"

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
    "'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}},{'id':'FX18',"
    "'description':'d','style':'dual','pins':18,'body':{'width':1,'height':1},'pitch':1,"
    "'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}]}";

/* plan-35-D fixtures. p18-dual: the C404ALB 40x4 pin table (E1 pin 9 for
   the top two lines, E2 pin 15 for the bottom two). p18-rgb: the Midas
   MD42004A6W-FPTLRGB table (LED+ 15, K(B) 16, K(G) 17, K(R) 18). */
#define PINOUT_P18_DUAL \
    "'pinouts':{'p18-dual':[" \
    "{'number':1,'name':'DB7','role':'inout','group':'data'}," \
    "{'number':2,'name':'DB6','role':'inout','group':'data'}," \
    "{'number':3,'name':'DB5','role':'inout','group':'data'}," \
    "{'number':4,'name':'DB4','role':'inout','group':'data'}," \
    "{'number':5,'name':'DB3','role':'inout','group':'data'}," \
    "{'number':6,'name':'DB2','role':'inout','group':'data'}," \
    "{'number':7,'name':'DB1','role':'inout','group':'data'}," \
    "{'number':8,'name':'DB0','role':'inout','group':'data'}," \
    "{'number':9,'name':'E1','role':'input'},{'number':10,'name':'R/W','role':'input'}," \
    "{'number':11,'name':'RS','role':'input'},{'number':12,'name':'V0','role':'input'}," \
    "{'number':13,'name':'VSS','role':'ground'},{'number':14,'name':'VDD','role':'power'}," \
    "{'number':15,'name':'E2','role':'input'},{'number':16,'name':'NC','role':'nc'}," \
    "{'number':17,'name':'LED+','role':'input'},{'number':18,'name':'LED-','role':'input'}]}"

#define PINOUT_P18_RGB \
    "'pinouts':{'p18-rgb':[{'number':1,'name':'VSS','role':'ground'}," \
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
    "{'number':15,'name':'LED+','role':'input'},{'number':16,'name':'K(B)','role':'input'}," \
    "{'number':17,'name':'K(G)','role':'input'},{'number':18,'name':'K(R)','role':'input'}]}"

#define RGB_BACKLIGHT \
    "'backlight':[{'anode':'LED+','cathode':'K(R)','color':'Red'}," \
    "{'anode':'LED+','cathode':'K(G)','color':'Green'}," \
    "{'anode':'LED+','cathode':'K(B)','color':'Blue'}],"

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

/*
 * A plan-35-D device file: as device_file, with the pinouts text, its pinout
 * id, the footprint and the "bus".e value given; the timing and exec are the
 * fixture's; `extra` holds the backlight and vcc pairs.
 */
static char *large_file(const char *name, const char *pinouts, const char *pinout,
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
    const char *e;       /* the enable the accesses pulse: "E", or "E1" / "E2" on a 40x4 */
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
    l.e = bench_has_pin(l.b, "E") ? "E" : "E1";
    if (bench_has_pin(l.b, "E")) {
        bench_set(l.b, "E", LOGIC_0);
    } else {
        bench_set(l.b, "E1", LOGIC_0);
        bench_set(l.b, "E2", LOGIC_0);
    }
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
    lv(l, l->e, 1);
    bench_run(l->b, NS(500));
    lv(l, l->e, 0);
    bench_run(l->b, US(2));
}

/* One read access; the DB values sampled just before E falls. */
static void access_read(Lcd *l, int rs, uint8_t *v, uint8_t *k) {
    unsigned i;

    lv(l, "RS", rs);
    lv(l, "R/W", 1);
    put_db(l, 0, 0);
    bench_run(l->b, NS(50));
    lv(l, l->e, 1);
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
    lv(l, l->e, 0);
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

/* Data writes 50 us apart: longer than every device's data exec time
   (37 us Newhaven, 43 us Displaytech and Orient). */
static void text(Lcd *l, const char *s) {
    for (; *s != '\0'; s++) {
        wr(l, 1, (uint8_t)*s);
        bench_run(l->b, US(50));
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
    count = voltage_check(parts, 3, 4, conn, n, NULL, 0, problems, 8);
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
            count = voltage_check(more, 4, 4, conn, n + 1, NULL, 0, problems, 8);
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

/* ---- plan-11-C section 4.5: the real parts ---- */

static PartLibrary *real_library;

static void check_real_parts(void) {
    static const char *const NAMES[] = {
        "NHD-0216BZ-FL-YBW", "NHD-0216K1Z-FL-YBW", "NHD-0216K1Z-NSW-BBW-L", "162H BC BW",
        "AMC1602GR-B-Y6WFDY"
    };
    PartLibrary *fixture = library;
    const Part *part;
    size_t i;
    unsigned pins1 = 0;

    library = real_library;
    for (i = 0; i < 5; i++) {
        Lcd l = lcd_open(NAMES[i]);
        LcdState st;
        char line[160];

        init8(&l);
        clear(&l);
        text(&l, "HELLO");
        lcd_state(&l, &st);
        snprintf(line, sizeof line, "C 4.5 %s: 8-bit init, HELLO in ddram[0..4], cell 0 = ROM 48",
                 NAMES[i]);
        expect(memcmp(st.ddram[0], "HELLO", 5) == 0 && cell_is_rom(&st, 0, 0, 0x48), line);
        bench_destroy(l.b);
    }
    /* A second data write 40 us after the first (E fall to E fall). */
    for (i = 0; i < 2; i++) {
        Lcd l = lcd_open(i == 0 ? "NHD-0216K1Z-NSW-BBW-L" : "162H BC BW");
        LcdState st;

        init8(&l);
        clear(&l);
        wr(&l, 1, 'A');
        bench_run(l.b, US(40) - US(2) - NS(550));
        wr(&l, 1, 'B');
        bench_run(l.b, US(50));
        lcd_state(&l, &st);
        if (i == 0) {
            expect(st.ddram[0][1] == 'B', "C 4.5 exec: a data write 40 us after another is accepted on "
                                       "NHD-0216K1Z-NSW-BBW-L (data 37 us)");
        } else {
            expect(st.ddram[0][1] == 0x20, "C 4.5 exec: and not accepted on 162H BC BW (data 43 us)");
        }
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

/* ---- plan-11-C section 4.6: the CPU board ---- */

enum { BOARD_NETS = 64 };

typedef struct {
    Circuit *c;
    const char *names[BOARD_NETS];
    size_t nets[BOARD_NETS];
    size_t count;
    int failed;
} Board;

static size_t board_net(Board *b, const char *name) {
    size_t i;

    for (i = 0; i < b->count; i++) {
        if (strcmp(b->names[i], name) == 0) {
            return b->nets[i];
        }
    }
    b->names[b->count] = name;
    circuit_add_net(b->c, &b->nets[b->count]);
    return b->nets[b->count++];
}

/* Connects every pin of `part` named `pin` to the net `net`. */
static void board_wire(Board *b, size_t index, const char *part_name, const char *pin,
                       const char *net) {
    const Part *part = part_library_find(real_library, part_name);
    size_t i, n = 0;

    for (i = 0; part != NULL && i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, pin) == 0) {
            if (circuit_connect(b->c, index, part->pins[i].designator, board_net(b, net)) != 0) {
                b->failed = 1;
            }
            n++;
        }
    }
    if (n == 0) {
        fprintf(stderr, "board: no pin %s on %s\n", pin, part_name);
        b->failed = 1;
    }
}

static size_t board_part(Board *b, const char *name) {
    size_t index = 0;

    if (circuit_add_part(b->c, name, &index) != 0) {
        fprintf(stderr, "board: %s\n", circuit_error(b->c));
        b->failed = 1;
    }
    return index;
}

/* The plan-11-C section 4.6 program, at EEPROM offset 0 (address 8000). It
   starts with an unpolled Set DDRAM Address (80): while the CPU is held in
   reset its address bus is X, so E pulses X and AC may be X (a possible
   read); 80 makes AC known before the first BF poll, and is ignored when
   the LCD is still busy. */
static const uint8_t HELLO_PROGRAM[] = {
    0xA9, 0x80, 0x8D, 0x00, 0x40,
    0xA2, 0x00, 0xBD, 0x2C, 0x80, 0xF0, 0x0B, 0x2C, 0x00, 0x40, 0x30, 0xFB, 0x8D, 0x00, 0x40,
    0xE8, 0xD0, 0xF0, 0xA2, 0x00, 0xBD, 0x31, 0x80, 0xF0, 0x0B, 0x2C, 0x00, 0x40, 0x30, 0xFB,
    0x8D, 0x01, 0x40, 0xE8, 0xD0, 0xF0, 0x4C, 0x29, 0x80, 0x38, 0x0C, 0x01, 0x06, 0x00, 0x48,
    0x45, 0x4C, 0x4C, 0x4F, 0x00
};

/* Builds the board, holds reset for `reset_ps`, then runs 50 ms; wdc picks
   the W65C02S. */
static void run_board(int wdc, SimTime reset_ps, LcdState *lcd, CpuState *cpu, int *ok) {
    static uint32_t image[32768];
    const char *cpu_name = wdc ? "W65C02S6TPG-14" : "MPS6502";
    Board b;
    size_t lcd_i, cpu_i, ee, hc00, hc11, osc, sw, p5, pg;
    unsigned i;
    char name[8];
    static char addr_names[16][8], data_names[8][8];

    memset(&b, 0, sizeof b);
    b.c = circuit_create(real_library);
    p5 = board_part(&b, "5V");
    pg = board_part(&b, "GND");
    osc = board_part(&b, "ECS-2100A-010");
    cpu_i = board_part(&b, cpu_name);
    ee = board_part(&b, "AT28C256-15PU");
    hc00 = board_part(&b, "SN74HC00N");
    hc11 = board_part(&b, "SN74HC11N");
    sw = board_part(&b, "TE-GDS0204");
    lcd_i = board_part(&b, "NHD-0216K1Z-NSW-BBW-L");

    board_wire(&b, p5, "5V", "VCC", "5V");
    board_wire(&b, pg, "GND", "GND", "GND");
    /* Power. */
    board_wire(&b, cpu_i, cpu_name, wdc ? "VDD" : "VCC", "5V");
    board_wire(&b, cpu_i, cpu_name, "VSS", "GND");
    board_wire(&b, ee, "AT28C256-15PU", "VCC", "5V");
    board_wire(&b, ee, "AT28C256-15PU", "GND", "GND");
    board_wire(&b, ee, "AT28C256-15PU", "WE#", "5V");
    board_wire(&b, hc00, "SN74HC00N", "VCC", "5V");
    board_wire(&b, hc00, "SN74HC00N", "GND", "GND");
    board_wire(&b, hc11, "SN74HC11N", "VCC", "5V");
    board_wire(&b, hc11, "SN74HC11N", "GND", "GND");
    board_wire(&b, osc, "ECS-2100A-010", "+5V DC", "5V");
    board_wire(&b, osc, "ECS-2100A-010", "GND", "GND");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "VSS", "GND");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "VDD", "5V");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "V0", "GND");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "LED+", "5V");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "LED-", "GND");
    /* Address and data. */
    for (i = 0; i < 16; i++) {
        snprintf(addr_names[i], sizeof addr_names[i], "A%u", i);
        board_wire(&b, cpu_i, cpu_name, addr_names[i], addr_names[i]);
        if (i < 15) {
            board_wire(&b, ee, "AT28C256-15PU", addr_names[i], addr_names[i]);
        }
    }
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "RS", "A0");
    for (i = 0; i < 8; i++) {
        snprintf(data_names[i], sizeof data_names[i], "D%u", i);
        board_wire(&b, cpu_i, cpu_name, data_names[i], data_names[i]);
        snprintf(name, sizeof name, "I/O%u", i);
        board_wire(&b, ee, "AT28C256-15PU", name, data_names[i]);
        snprintf(name, sizeof name, "DB%u", i);
        board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", name, data_names[i]);
    }
    /* Decode. */
    board_wire(&b, hc00, "SN74HC00N", "1A", "A15");
    board_wire(&b, hc00, "SN74HC00N", "1B", "A15");
    board_wire(&b, hc00, "SN74HC00N", "1Y", "NA15");
    board_wire(&b, ee, "AT28C256-15PU", "CE#", "NA15");
    board_wire(&b, cpu_i, cpu_name, wdc ? "RWB" : "R/W", "RW");
    board_wire(&b, hc00, "SN74HC00N", "2A", "RW");
    board_wire(&b, hc00, "SN74HC00N", "2B", "RW");
    board_wire(&b, hc00, "SN74HC00N", "2Y", "OE");
    board_wire(&b, ee, "AT28C256-15PU", "OE#", "OE");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "R/W", "RW");
    board_wire(&b, hc11, "SN74HC11N", "1A", "PHI2");
    board_wire(&b, hc11, "SN74HC11N", "1B", "A14");
    board_wire(&b, hc11, "SN74HC11N", "1C", "NA15");
    board_wire(&b, hc11, "SN74HC11N", "1Y", "E");
    board_wire(&b, lcd_i, "NHD-0216K1Z-NSW-BBW-L", "E", "E");
    /* Clock and reset. */
    if (wdc) {
        board_wire(&b, osc, "ECS-2100A-010", "OUTPUT", "PHI2");
        board_wire(&b, cpu_i, cpu_name, "PHI2", "PHI2");
        board_wire(&b, cpu_i, cpu_name, "RESB", "RES");
    } else {
        board_wire(&b, osc, "ECS-2100A-010", "OUTPUT", "PHI0");
        board_wire(&b, cpu_i, cpu_name, "PHI0IN", "PHI0");
        board_wire(&b, cpu_i, cpu_name, "PHI2OUT", "PHI2");
        board_wire(&b, cpu_i, cpu_name, "RES#", "RES");
    }
    board_wire(&b, sw, "TE-GDS0204", "1A", "RES");
    board_wire(&b, sw, "TE-GDS0204", "1B", "GND");

    for (i = 0; i < 32768; i++) {
        image[i] = i < sizeof HELLO_PROGRAM ? HELLO_PROGRAM[i] : 0xFF;
    }
    image[0x7FFA] = 0x29;   /* NMI -> 8029 */
    image[0x7FFB] = 0x80;
    image[0x7FFC] = 0x00;   /* RESET -> 8000 */
    image[0x7FFD] = 0x80;
    image[0x7FFE] = 0x29;   /* IRQ -> 8029 */
    image[0x7FFF] = 0x80;
    *ok = !b.failed && circuit_memory_load(b.c, ee, 0, image, 32768) == 0 &&
          circuit_set_switch(b.c, sw, 1, 1) == 0 && circuit_start(b.c) == 0;
    if (!*ok) {
        fprintf(stderr, "board: %s\n", circuit_error(b.c) != NULL ? circuit_error(b.c) : "build failed");
    }
    if (*ok) {
        *ok = circuit_advance(b.c, reset_ps) && circuit_set_switch(b.c, sw, 1, 0) == 0;
        for (i = 0; *ok && i < 50; i++) {
            *ok = circuit_advance(b.c, MS(1));
        }
        if (!*ok) {
            fprintf(stderr, "board: %s\n", circuit_error(b.c) != NULL ? circuit_error(b.c) : "stopped");
        }
    }
    memset(lcd, 0, sizeof *lcd);
    memset(cpu, 0, sizeof *cpu);
    if (*ok) {
        *ok = circuit_lcd_state(b.c, lcd_i, lcd) == 0 && circuit_cpu_state(b.c, cpu_i, cpu) == 0;
    }
    circuit_destroy(b.c);
}

static void check_cpu_board(void) {
    LcdState lcd;
    CpuState cpu;
    int ok;
    unsigned c;
    int cells = 1, all_x = 1, y, x;

    run_board(0, US(10), &lcd, &cpu, &ok);
    for (c = 0; c < 5; c++) {
        cells = cells && cell_is_rom(&lcd, 0, c, (unsigned)"HELLO"[c]);
    }
    expect(ok && memcmp(lcd.ddram[0], "HELLO", 5) == 0 && lcd.ddram[0][5] == 0x20 && cells &&
               lcd.busy[0] == 0,
           "C 4.6 MPS6502 board: HELLO in ddram[0..4], ddram[5] = 20, cells 0-4 = ROM, BF 0");
    printf("     cpu: status %d, last \"%s\"\n", (int)cpu.status, cpu.instruction_text);
    expect(ok && cpu.status == CPU_RUNNING && cpu.has_instruction &&
               strcmp(cpu.instruction_text, "JMP $8029") == 0,
           "C 4.6 MPS6502 board: the CPU is RUNNING, last instruction JMP $8029");

    /* The app's flow: the reset switch opened by hand long after the LCD's
       40 ms power-on; E pulses X all that time (the CPU's address bus is X
       in reset). */
    run_board(0, MS(100), &lcd, &cpu, &ok);
    cells = 1;
    for (c = 0; c < 5; c++) {
        cells = cells && cell_is_rom(&lcd, 0, c, (unsigned)"HELLO"[c]);
    }
    expect(ok && memcmp(lcd.ddram[0], "HELLO", 5) == 0 && cells && lcd.busy[0] == 0 &&
               cpu.status == CPU_RUNNING && strcmp(cpu.instruction_text, "JMP $8029") == 0,
           "C 4.6 MPS6502 board, reset held 100 ms: HELLO, BF 0, the CPU RUNNING at JMP $8029");

    run_board(1, US(10), &lcd, &cpu, &ok);
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 80; x++) {
            all_x = all_x && lcd.dots[y][x] == LCD_DOT_X;
        }
    }
    expect(ok && all_x && lcd.busy[0] == -1,
           "C 4.6 W65C02S board: the first STA $4000 broke the hold time: every dot X, BF X (LOST)");
    printf("     cpu: status %d, last \"%s\"\n", (int)cpu.status, cpu.instruction_text);
    expect(ok && cpu.status == CPU_LOST,
           "C 4.6 W65C02S board: the CPU is LOST (its BIT $4000 read an X busy flag)");
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
    check_real_parts();
    check_cpu_board();
    part_library_destroy(real_library);
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_test: all passed\n");
    return 0;
}
