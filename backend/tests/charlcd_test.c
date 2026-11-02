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
#include "kind.h"
#include "parts.h"
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

/* ---- main ----------------------------------------------------------------- */

int main(void) {
    char *error;

    library = fixture_library();
    error = load_file(library, device_file("FX-LCD", "16", "2", "'A00'", "'blue-negative'", DATA,
                                           TIMING, EXEC, BACKLIGHT VCC),
                      "fx-lcd.json");
    if (error == NULL) {
        error = load_file(library, device_file("FX-LCD-NB", "16", "2", "'A00'", "'yellow-green'",
                                               DATA, TIMING, EXEC, VCC),
                          "fx-lcd-nb.json");
    }
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    check_loader();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_test: all passed\n");
    return 0;
}
