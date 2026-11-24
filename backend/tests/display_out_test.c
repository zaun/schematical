/*
 * display_out_test (plan-56-B): the leddriver and lcddriver kinds on fixture
 * devices (models/display_out.c). The converter engine runs the bus and the
 * registers; the "display" section's driver drives the pins:
 *
 *   SPILED4   SPI, `digits`: 4 digits scanned (digit pins active low), raw and
 *             decoded bytes (per-digit decodeField), blank, test, shutdown
 *   DEC*      SPI, `digits` with the codeB, bcd and ascii7 decodes
 *   SPISTAT   SPI, `static`: two 7-segment groups, one with a decode switch
 *   SPIPORT   SPI, `ports`: direction and data fields, invert, open drain
 *   SPIMAT    SPI, `matrix`: three rows scanned, four columns
 *   I2CLCD4/3/S  I2C, `lcd` in modes 1:4, 1:3 and static, wired to the real
 *             GENERIC-LCD-8D-1:4, GENERIC-LCD-8D-1:3 and GENERIC-LCD-4D-STATIC
 *             glass: the lit cells equal the RAM bits
 *
 * and the loader's rejections of a bad "display" section.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

static int failures;
static PartLibrary *library;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static char *json(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    size_t i;

    for (i = 0; text[i] != '\0'; i++) {
        copy[i] = text[i] == '\'' ? '"' : text[i];
    }
    copy[i] = '\0';
    return copy;
}

#define REG(NAME, ADDR, RESET, FIELDS) \
    "{'name':'" NAME "','address':" #ADDR ",'width':8,'reset':" #RESET ",'fields':[" FIELDS "]}"
#define F(NAME, HI, LO) "{'name':'" NAME "','bits':[" #HI "," #LO "]}"

#define SPI_BUS "{'style':'spi','cs':'CS','sclk':'SCLK','mosi':'DIN','mode':0}"
#define SPI_WRITE \
    "'protocol':{'frame':'fixed','commands':[{'name':'W','in':'aaaaaaaa dddddddd','action':'write'," \
    "'address':'a','data':'d'}]}"
#define SPI_SPEC "CS:i SCLK:i DIN:i"
#define SEG7_NAMES "'SEGA','SEGB','SEGC','SEGD','SEGE','SEGF','SEGG'"
#define SEG7_SPEC "SEGA:o SEGB:o SEGC:o SEGD:o SEGE:o SEGF:o SEGG:o"
#define I2C_BUS "{'style':'i2c','scl':'SCL','sda':'SDA','address':[],'table':[56]}"
#define I2C_PROTOCOL \
    "'protocol':{'frame':'fixed','commands':[{'name':'P','in':'aaaaaaaa','action':'pointer','address':'a'," \
    "'increment':1}],'read':'pointer'}"
#define I2C_SPEC "SDA:b SCL:i VCC:p GND:g"

/*
 * A device as JSON text (single quotes): `spec` is the pinout, "NAME:r" per pin in order
 * (r: i input, o output, b inout, p power, g ground, n no connect); an odd count gets an
 * NC pin. The footprint is the dual DT<pins> of load_fixtures. `body` is the kind keys.
 */
static char *device(const char *id, const char *function, const char *drawer, const char *spec,
                    const char *vcc, const char *bus, const char *body) {
    size_t cap = strlen(spec) * 64 + strlen(body) + 2048, used;
    char *out = malloc(cap), *copy = malloc(strlen(spec) + 1), *tok, *save = NULL;
    int n = 0;

    snprintf(out, cap,
             "{'id':'%s','function':'%s','description':'d','drawer':[%s],'pinouts':{'p':[", id,
             function, drawer);
    used = strlen(out);
    strcpy(copy, spec);
    for (tok = strtok_r(copy, " ", &save); tok != NULL; tok = strtok_r(NULL, " ", &save)) {
        char *colon = strchr(tok, ':');
        char r = colon[1];
        const char *role = r == 'i' ? "input" : r == 'o' ? "output" : r == 'b' ? "inout" :
                           r == 'p' ? "power" : r == 'g' ? "ground" : "nc";

        *colon = '\0';
        used += (size_t)snprintf(out + used, cap - used, "%s{'number':%d,'name':'%s','role':'%s'}",
                                 n ? "," : "", n + 1, tok, role);
        n++;
    }
    if (n % 2) {
        used += (size_t)snprintf(out + used, cap - used, ",{'number':%d,'name':'NC','role':'nc'}", n + 1);
        n++;
    }
    snprintf(out + used, cap - used,
             "]},'vcc':%s,'bus':%s,%s,'parts':[{'name':'%sX','package':'SOIC','pinout':'p',"
             "'footprint':'DT%d'}]}", vcc, bus, body, id, n);
    free(copy);
    return out;
}

/* A MAX6951-shape 4-digit driver: digit pins active low, byte bit 6 = a ... bit 0 = g, bit 7 = point,
   a decode bit per digit (hex), blank, display test and shutdown. */
#define SPILED4_BODY \
    "'registers':[" REG("DIGIT1", 1, 0, "") "," REG("DIGIT2", 2, 0, "") "," REG("DIGIT3", 3, 0, "") "," \
    REG("DIGIT4", 4, 0, "") "," REG("TEST", 7, 0, F("T", 0, 0)) "," \
    REG("DECODE", 9, 0, F("D1", 0, 0) "," F("D2", 1, 1) "," F("D3", 2, 2) "," F("D4", 3, 3)) "," \
    REG("CFG", 12, 1, F("RUN", 0, 0) "," F("BLANK", 1, 1)) "]," SPI_WRITE "," \
    "'display':{'driver':'digits','digits':4,'digitPins':['DIG1','DIG2','DIG3','DIG4'],'digitActive':'low'," \
    "'segmentPins':[" SEG7_NAMES ",'SEGDP'],'segmentActive':'high','segmentBits':[6,5,4,3,2,1,0,7]," \
    "'ram':['DIGIT1','DIGIT2','DIGIT3','DIGIT4'],'decode':'hex'," \
    "'decodeField':['DECODE.D1','DECODE.D2','DECODE.D3','DECODE.D4'],'scan':{'periodUs':250}," \
    "'blank':'CFG.BLANK=1','test':'TEST.T=1','shutdown':['CFG.RUN=0','TEST.T=0']}"

/* Two digits whose byte is always decoded. */
#define DECDEV_BODY(DECODE) \
    "'registers':[" REG("DIGIT1", 1, 0, "") "," REG("DIGIT2", 2, 0, "") "]," SPI_WRITE "," \
    "'display':{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'high'," \
    "'segmentPins':[" SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','DIGIT2']," \
    "'decode':'" DECODE "','scan':{'periodUs':100}}"

/* Two static 7-segment groups, outputs active low; the second group decodes while MODE.M is 1. */
#define SPISTAT_BODY \
    "'registers':[" REG("V1", 1, 0, "") "," REG("V2", 2, 0, "") "," REG("MODE", 3, 0, F("M", 0, 0)) "]," \
    SPI_WRITE "," \
    "'display':{'driver':'static','segmentActive':'low','decode':'hex','groups':[" \
    "{'pins':['G1A','G1B','G1C','G1D','G1E','G1F','G1G'],'ram':'V1'}," \
    "{'pins':['G2A','G2B','G2C','G2D','G2E','G2F','G2G'],'ram':'V2','decodeField':'MODE.M'}]}"

/* Four ports: P0 an input until DIR.D0, P1 always an output (inverted), P2 an output while
   DIR.D2 is 0 and open drain, P3 an output while MODE.M3 is 1 or 3. */
#define SPIPORT_BODY \
    "'registers':[" REG("DIR", 1, 0, F("D0", 0, 0) "," F("D2", 2, 2)) "," \
    REG("DATA", 2, 0, F("B0", 0, 0) "," F("B1", 1, 1) "," F("B2", 2, 2) "," F("B3", 3, 3)) "," \
    REG("MODE", 3, 0, F("M3", 1, 0)) "]," SPI_WRITE "," \
    "'display':{'driver':'ports','openDrain':['P2'],'ports':[" \
    "{'pin':'P0','dirField':'DIR.D0','dataField':'DATA.B0'}," \
    "{'pin':'P1','dataField':'DATA.B1','invert':true}," \
    "{'pin':'P2','dirField':'DIR.D2','dirOutput':0,'dataField':'DATA.B2'}," \
    "{'pin':'P3','dirField':'MODE.M3','dirOutput':[1,3],'dataField':'DATA.B3'}]}"

/* Three rows (active low) scanned, four columns (active high) with COL1 = bit 3. */
#define SPIMAT_BODY \
    "'registers':[" REG("ROWD1", 1, 0, "") "," REG("ROWD2", 2, 0, "") "," REG("ROWD3", 3, 0, "") "," \
    REG("CFG", 4, 1, F("RUN", 0, 0)) "]," SPI_WRITE "," \
    "'display':{'driver':'matrix','rows':['ROW1','ROW2','ROW3'],'rowActive':'low'," \
    "'columns':['COL1','COL2','COL3','COL4'],'columnActive':'high','columnBits':[3,2,1,0]," \
    "'ram':['ROWD1','ROWD2','ROWD3'],'scan':{'periodUs':200},'shutdown':'CFG.RUN=0'}"

/* PCF8576-shape I2C LCD drivers: RAM written through the pointer (auto-increment), a CTRL.ON enable. */
#define CTRL_REG REG("CTRL", 16, 1, F("ON", 0, 0))

/* Appends "S<digit><letter>" for each digit 1..digits and letter of `letters`: pins ("S1A:o ...") to
   `spec` and quoted names ("'S1A','S1B'") to `names`. */
static void segment_pins(char *spec, char *names, int digits, const char *letters) {
    int d;
    size_t i;

    for (d = 1; d <= digits; d++) {
        for (i = 0; letters[i] != '\0'; i++) {
            char one[16];

            snprintf(one, sizeof one, " S%d%c:o", d, letters[i]);
            strcat(spec, one);
            snprintf(one, sizeof one, "%s'S%d%c'", names[0] ? "," : "", d, letters[i]);
            strcat(names, one);
        }
    }
}

static char *lcd_device(const char *id, const char *bp_spec, const char *bp_names, int digits,
                        const char *letters, const char *mode, const char *ram, const char *frame,
                        const char *registers) {
    char spec[2048], names[2048], body[4096];
    char *text;

    snprintf(spec, sizeof spec, "%s %s", I2C_SPEC, bp_spec);
    names[0] = '\0';
    segment_pins(spec, names, digits, letters);
    snprintf(body, sizeof body,
             "'registers':[%s]," I2C_PROTOCOL ",'display':{'driver':'lcd','mode':'%s','backplanes':[%s],"
             "'segmentPins':[%s],%s,'frameUs':%s,'enable':'CTRL.ON=1'}",
             registers, mode, bp_names, names, ram, frame);
    text = device(id, "LCD driver", "'Displays & Indicators','LCD drivers'", spec, "[2.5,6]", I2C_BUS, body);
    return text;
}

static char *join(char **parts, size_t n) {
    size_t total = 1, i;
    char *out;

    for (i = 0; i < n; i++) {
        total += strlen(parts[i]) + 1;
    }
    out = calloc(total, 1);
    for (i = 0; i < n; i++) {
        strcat(out, i ? "," : "");
        strcat(out, parts[i]);
    }
    return out;
}

static char *led_device(const char *id, const char *spec, const char *body) {
    return device(id, "LED driver", "'Displays & Indicators','LED drivers'", spec, "[4.5,5.5]", SPI_BUS, body);
}

static int load_fixtures(void) {
    char footprints[8192] = "{'format':'schematical-footprints','version':1,'footprints':[";
    char *leds[7], *lcds[3];
    char *list, *text, *fp;
    char *error = NULL;
    int ok, n;

    for (n = 4; n <= 40; n += 2) {
        char one[256];

        snprintf(one, sizeof one,
                 "%s{'id':'DT%d','description':'d','style':'dual','pins':%d,'body':{'width':1,'height':1},"
                 "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}",
                 n > 4 ? "," : "", n, n);
        strcat(footprints, one);
    }
    strcat(footprints, "]}");
    leds[0] = led_device("SPILED4", SPI_SPEC " DIG1:o DIG2:o DIG3:o DIG4:o " SEG7_SPEC " SEGDP:o VCC:p GND:g",
                         SPILED4_BODY);
    leds[1] = led_device("DECB", SPI_SPEC " DIG1:o DIG2:o " SEG7_SPEC " VCC:p GND:g", DECDEV_BODY("codeB"));
    leds[2] = led_device("DECD", SPI_SPEC " DIG1:o DIG2:o " SEG7_SPEC " VCC:p GND:g", DECDEV_BODY("bcd"));
    leds[3] = led_device("DECA", SPI_SPEC " DIG1:o DIG2:o " SEG7_SPEC " VCC:p GND:g", DECDEV_BODY("ascii7"));
    leds[4] = led_device("SPISTAT", SPI_SPEC " G1A:o G1B:o G1C:o G1D:o G1E:o G1F:o G1G:o G2A:o G2B:o G2C:o G2D:o "
                         "G2E:o G2F:o G2G:o VCC:p GND:g", SPISTAT_BODY);
    leds[5] = led_device("SPIPORT", SPI_SPEC " P0:b P1:b P2:b P3:b VCC:p GND:g", SPIPORT_BODY);
    leds[6] = led_device("SPIMAT", SPI_SPEC " ROW1:o ROW2:o ROW3:o COL1:o COL2:o COL3:o COL4:o VCC:p GND:g",
                         SPIMAT_BODY);
    lcds[0] = lcd_device("I2CLCD4", "BP1:o BP2:o BP3:o BP4:o", "'BP1','BP2','BP3','BP4'", 8, "AB", "1:4",
                         "'ram':{'bytes':8,'address':0}", "2000", CTRL_REG);
    lcds[1] = lcd_device("I2CLCD3", "BP1:o BP2:o BP3:o", "'BP1','BP2','BP3'", 8, "ABC", "1:3",
                         "'ram':{'bytes':9,'address':0},'ramOrder':'backplane','bitOrder':'msb'", "3000", CTRL_REG);
    lcds[2] = lcd_device("I2CLCDS", "COM:o", "'COM'", 4, "ABCDEFGH", "static",
                         "'ram':['R0','R1','R2','R3']", "2000",
                         CTRL_REG "," REG("R0", 0, 0, "") "," REG("R1", 1, 0, "") "," REG("R2", 2, 0, "") ","
                         REG("R3", 3, 0, ""));
    fp = json(footprints);
    list = join(leds, 7);
    text = malloc(strlen(list) + 256);
    sprintf(text, "{'format':'schematical-parts','version':1,'kind':'leddriver','devices':[%s]}", list);
    free(list);
    list = json(text);
    free(text);
    library = part_library_create();
    ok = load_parts_dir(library, PARTS_DIR) == 0 && load_footprints(library, fp, "fixtures.json", &error) == 0 &&
         load_parts(library, list, "leddriver-fixtures.json", &error) == 0;
    free(list);
    if (ok) {
        list = join(lcds, 3);
        text = malloc(strlen(list) + 256);
        sprintf(text, "{'format':'schematical-parts','version':1,'kind':'lcddriver','devices':[%s]}", list);
        free(list);
        list = json(text);
        free(text);
        ok = load_parts(library, list, "lcddriver-fixtures.json", &error) == 0;
        free(list);
    }
    if (!ok) {
        printf("FAIL loading the fixtures: %s\n", error ? error : "?");
        failures++;
    }
    free(error);
    free(fp);
    return ok;
}

/* ---- The bench: the chip, an optional glass, a test driver and pull-ups ------------------------- */

typedef struct {
    size_t count;
} PinCount;

static size_t drv_pins(const void *config) {
    return ((const PinCount *)config)->count;
}

static void *drv_create(const void *config) {
    PinCount *c = malloc(sizeof *c);

    *c = *(const PinCount *)config;
    return c;
}

static void pull_start(Sim *sim, SimInstance *instance, void *state) {
    size_t i;

    for (i = 0; i < ((PinCount *)state)->count; i++) {
        sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_WEAK, 0);
    }
}

static const SimComponentType DRIVER = {"tb", drv_pins, drv_create, free, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};
static const SimComponentType PULLUP = {"pu", drv_pins, drv_create, free, pull_start, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL};

typedef struct {
    Sim *sim;
    SimInstance *chip, *glass, *drv, *pu;
    const Part *part, *glass_part;
    PinCount pins, pulls;
} Bench;

enum { HALF = 100000 };
static const SimTime US = 1000000;

static size_t pin_of(const Part *part, const char *name) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, name) == 0) {
            return i;
        }
    }
    return SIZE_MAX;
}

static size_t bpin(const Bench *b, const char *name) {
    size_t i = pin_of(b->part, name);

    if (i == SIZE_MAX) {
        fprintf(stderr, "no pin %s\n", name);
        exit(2);
    }
    return i;
}

static void set(Bench *b, const char *name, LogicValue v) {
    sim_schedule_drive(b->sim, b->drv, bpin(b, name), v, DRIVE_STRONG, 0);
}

static LogicValue get(const Bench *b, const char *name) {
    return sim_read_pin(b->sim, b->chip, bpin(b, name));
}

static void run(Bench *b, SimTime ps) {
    if (!sim_run_until(b->sim, sim_now(b->sim) + ps)) {
        fprintf(stderr, "sim stopped: %s\n", sim_error(b->sim));
        exit(2);
    }
}

/* `glass` (or NULL): a part wired to the chip's pins of the same names (not NC). */
static Bench *bench(const char *part, const char *glass, const char *const *pulls) {
    Bench *b = calloc(1, sizeof *b);
    size_t i, k;

    b->part = part_library_find(library, part);
    if (b->part == NULL) {
        fprintf(stderr, "no part %s\n", part);
        exit(2);
    }
    b->pins.count = b->part->pin_count;
    for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
        b->pulls.count++;
    }
    b->sim = sim_create();
    b->chip = sim_add_instance(b->sim, kind_model(b->part), "U1", b->part);
    b->drv = sim_add_instance(b->sim, &DRIVER, "TB", &b->pins);
    b->pu = sim_add_instance(b->sim, &PULLUP, "PU", &b->pulls);
    if (glass != NULL) {
        b->glass_part = part_library_find(library, glass);
        if (b->glass_part == NULL) {
            fprintf(stderr, "no glass %s\n", glass);
            exit(2);
        }
        b->glass = sim_add_instance(b->sim, kind_model(b->glass_part), "G1", b->glass_part);
    }
    for (i = 0; i < b->part->pin_count; i++) {
        SimNet *net = sim_add_net(b->sim, b->part->pins[i].name);
        size_t g;

        sim_connect(b->sim, net, b->chip, i);
        sim_connect(b->sim, net, b->drv, i);
        for (k = 0; pulls != NULL && pulls[k] != NULL; k++) {
            if (strcmp(pulls[k], b->part->pins[i].name) == 0) {
                sim_connect(b->sim, net, b->pu, k);
            }
        }
        if (glass != NULL && strcmp(b->part->pins[i].name, "NC") != 0 &&
            (g = pin_of(b->glass_part, b->part->pins[i].name)) != SIZE_MAX) {
            sim_connect(b->sim, net, b->glass, g);
        }
    }
    if (!sim_start(b->sim)) {
        fprintf(stderr, "start: %s\n", sim_error(b->sim));
        exit(2);
    }
    return b;
}

static void close_bench(Bench *b) {
    sim_destroy(b->sim);
    free(b);
}

/* ---- SPI (mode 0, CS active low): 16-bit write frames ------------------------------------------ */

static void spi_start(Bench *b) {
    set(b, "CS", LOGIC_1);
    set(b, "SCLK", LOGIC_0);
    run(b, HALF);
}

static void spi_frame(Bench *b, unsigned word, unsigned bits) {
    unsigned i;

    set(b, "CS", LOGIC_0);
    run(b, HALF);
    for (i = 0; i < bits; i++) {
        set(b, "DIN", (word >> (bits - 1 - i)) & 1u ? LOGIC_1 : LOGIC_0);
        run(b, HALF);
        set(b, "SCLK", LOGIC_1);
        run(b, HALF);
        set(b, "SCLK", LOGIC_0);
    }
    run(b, HALF);
    set(b, "CS", LOGIC_1);
    run(b, HALF);
}

static void spi_write(Bench *b, unsigned reg, unsigned value) {
    spi_frame(b, (reg << 8) | value, 16);
}

/* ---- I2C (address 0x38): pointer, then bytes with auto-increment --------------------------------- */

static void line(Bench *b, const char *pin, int level) {
    set(b, pin, level ? LOGIC_Z : LOGIC_0);
}

static void i2c_bit(Bench *b, int out) {
    line(b, "SDA", out);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SCL", 0);
    run(b, HALF);
}

static void i2c_byte(Bench *b, unsigned v) {
    int i;

    for (i = 7; i >= 0; i--) {
        i2c_bit(b, (v >> i) & 1u);
    }
    i2c_bit(b, 1);
}

static void i2c_write(Bench *b, unsigned reg, const unsigned *v, size_t n) {
    size_t i;

    line(b, "SDA", 1);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 0);
    run(b, HALF);
    i2c_byte(b, 0x38u << 1);
    i2c_byte(b, reg);
    for (i = 0; i < n; i++) {
        i2c_byte(b, v[i]);
    }
    line(b, "SDA", 0);
    run(b, HALF);
    line(b, "SCL", 1);
    run(b, HALF);
    line(b, "SDA", 1);
    run(b, HALF);
}

/* ---- The digits driver ------------------------------------------------------------------------- */

/* One sample of a scan: the active digit (-1 none, -2 several) and the segment pins as bits
   0-7 (a..g, point). Digit pins are active low. */
static int sample_digit(Bench *b, const char *const *digits, size_t count, unsigned *segments,
                        const char *const *segs, size_t seg_count, int segment_high) {
    int active = -1;
    size_t i;

    for (i = 0; i < count; i++) {
        if (get(b, digits[i]) == LOGIC_0) {
            active = active == -1 ? (int)i : -2;
        }
    }
    *segments = 0;
    for (i = 0; i < seg_count; i++) {
        LogicValue v = get(b, segs[i]);

        if ((v == LOGIC_1) == (segment_high != 0) && (v == LOGIC_1 || v == LOGIC_0)) {
            *segments |= 1u << i;
        }
    }
    return active;
}

static const char *const LED4_DIGITS[] = {"DIG1", "DIG2", "DIG3", "DIG4"};
static const char *const LED4_SEGS[] = {"SEGA", "SEGB", "SEGC", "SEGD", "SEGE", "SEGF", "SEGG", "SEGDP"};

/* Runs 2.2 ms (two frames and more) sampling every 25 us: every sample has exactly one active
   digit, its segments equal expect[digit], and every digit is seen. */
static int scan_is(Bench *b, const unsigned *expect, size_t digits) {
    unsigned seen = 0;
    int ok = 1, t;

    for (t = 0; t < 88; t++) {
        unsigned seg;
        int d = sample_digit(b, LED4_DIGITS, digits, &seg, LED4_SEGS, 8, 1);

        if (d < 0 || seg != expect[d]) {
            ok = 0;
        } else {
            seen |= 1u << d;
        }
        run(b, 25 * US);
    }
    return ok && seen == (1u << digits) - 1u;
}

/* All digit pins inactive (high) and no segment lit, over a few samples. */
static int scan_off(Bench *b) {
    int ok = 1, t;

    for (t = 0; t < 40; t++) {
        unsigned seg;

        ok &= sample_digit(b, LED4_DIGITS, 4, &seg, LED4_SEGS, 8, 1) == -1 && seg == 0;
        run(b, 25 * US);
    }
    return ok;
}

static void led4(void) {
    Bench *b = bench("SPILED4X", NULL, NULL);
    unsigned zero[4] = {0, 0, 0, 0};
    unsigned raw[4] = {0x01, 0x40, 0x80, 0x7F};
    unsigned dec[4] = {0x6D, 0xF7, 0x71, 0x06};
    unsigned mixed[4] = {0x6D, 0xA8, 0x71, 0x06};
    unsigned all[4] = {0xFF, 0xFF, 0xFF, 0xFF};
    int last = -1, order_ok = 1, step_ok = 1, have_time = 0;
    SimTime changed = 0, t;

    spi_start(b);
    check(b->part->kind == PART_LEDDRIVER && b->part->converter == NULL, "SPILED4 is a leddriver part");
    /* Slot timing and order: the digit pin changes every 250 us, 1, 2, 3, 4, 1, ... (slot 1 from t = 0). */
    for (t = 0; t < 1400 * US; t += 10 * US) {
        unsigned seg;
        int d = sample_digit(b, LED4_DIGITS, 4, &seg, LED4_SEGS, 8, 1);

        if (d != last) {
            if (last >= 0) {
                if (d != (last + 1) % 4) {
                    order_ok = 0;
                }
                if (have_time && sim_now(b->sim) - changed != 250 * US) {
                    step_ok = 0;
                }
                changed = sim_now(b->sim);
                have_time = 1;
            }
            last = d;
        }
        run(b, 10 * US);
    }
    check(order_ok, "digits: the digit pins go active in digit order");
    check(step_ok, "digits: a slot lasts the scan period (250 us)");
    check(scan_is(b, zero, 4), "digits: after reset every digit is scanned with no segment lit");
    spi_write(b, 1, 0x40);
    spi_write(b, 2, 0x01);
    spi_write(b, 3, 0x80);
    spi_write(b, 4, 0x7F);
    check(scan_is(b, raw, 4), "digits: raw bytes (bit 6 = a ... bit 0 = g, bit 7 = point) show in each digit's slot");
    spi_write(b, 9, 0x0F);
    spi_write(b, 1, 0x05);
    spi_write(b, 2, 0x8A);
    spi_write(b, 3, 0x0F);
    spi_write(b, 4, 0x01);
    check(scan_is(b, dec, 4), "digits: hex decode of 5, A (+ point), F and 1");
    spi_write(b, 9, 0x0D);
    check(scan_is(b, mixed, 4), "digits: decodeField off for digit 2 shows its byte raw (d, f, point)");
    spi_write(b, 12, 0x03);
    check(scan_is(b, zero, 4), "digits: blank lights no segment while the scan runs");
    spi_write(b, 7, 0x01);
    check(scan_is(b, all, 4), "digits: test lights every segment (over blank)");
    spi_write(b, 12, 0x00);
    check(scan_is(b, all, 4), "digits: test shows through shutdown when the shutdown condition says so");
    spi_write(b, 7, 0x00);
    check(scan_off(b), "digits: shutdown drives every digit pin and segment pin inactive");
    spi_write(b, 12, 0x01);
    check(scan_is(b, mixed, 4), "digits: leaving shutdown restores the display");
    close_bench(b);
}

/* The two-digit decoders: digit pins are active high. */
static unsigned dec_digit(Bench *b, int digit) {
    const char *dig = digit == 0 ? "DIG1" : "DIG2";
    static const char *const SEGS[] = {"SEGA", "SEGB", "SEGC", "SEGD", "SEGE", "SEGF", "SEGG"};
    int t;

    for (t = 0; t < 80; t++) {
        if (get(b, dig) == LOGIC_1 && get(b, digit == 0 ? "DIG2" : "DIG1") == LOGIC_0) {
            unsigned seg = 0;
            size_t i;

            for (i = 0; i < 7; i++) {
                if (get(b, SEGS[i]) == LOGIC_1) {
                    seg |= 1u << i;
                }
            }
            return seg;
        }
        run(b, 10 * US);
    }
    return 0xFFFFu;
}

static void decoders(void) {
    Bench *b = bench("DECBX", NULL, NULL);

    spi_start(b);
    spi_write(b, 1, 0x0A);
    spi_write(b, 2, 0x0C);
    check(dec_digit(b, 0) == 0x40 && dec_digit(b, 1) == 0x76, "codeB: 10 is a dash, 12 is H");
    spi_write(b, 1, 0x0B);
    spi_write(b, 2, 0x0F);
    check(dec_digit(b, 0) == 0x79 && dec_digit(b, 1) == 0x00, "codeB: 11 is E, 15 is blank");
    close_bench(b);

    b = bench("DECDX", NULL, NULL);
    spi_start(b);
    spi_write(b, 1, 0x09);
    spi_write(b, 2, 0x0B);
    check(dec_digit(b, 0) == 0x6F && dec_digit(b, 1) == 0x00, "bcd: 9 shows, 11 is blank");
    close_bench(b);

    b = bench("DECAX", NULL, NULL);
    spi_start(b);
    spi_write(b, 1, 'A');
    spi_write(b, 2, 'a');
    check(dec_digit(b, 0) == 0x77 && dec_digit(b, 1) == 0x77, "ascii7: A and a read the same");
    spi_write(b, 1, '-');
    spi_write(b, 2, '8');
    check(dec_digit(b, 0) == 0x40 && dec_digit(b, 1) == 0x7F, "ascii7: - and 8");
    spi_write(b, 1, ' ');
    spi_write(b, 2, 0x00);
    check(dec_digit(b, 0) == 0x00 && dec_digit(b, 1) == 0x00, "ascii7: space and a control code are blank");
    close_bench(b);
}

/* ---- static, ports, matrix --------------------------------------------------------------------- */

/* The lit segments (outputs active low) of a group of seven pins named PREFIX + A..G. */
static unsigned lit_low(Bench *b, const char *prefix) {
    unsigned seg = 0;
    size_t i;

    for (i = 0; i < 7; i++) {
        char name[8];

        snprintf(name, sizeof name, "%s%c", prefix, 'A' + (int)i);
        if (get(b, name) == LOGIC_0) {
            seg |= 1u << i;
        }
    }
    return seg;
}

static void static_driver(void) {
    Bench *b = bench("SPISTATX", NULL, NULL);

    spi_start(b);
    run(b, 50 * US);
    check(lit_low(b, "G1") == 0x3F && lit_low(b, "G2") == 0x00,
          "static: group 1 (always decoded) shows 0 after reset; group 2 (raw, V2 = 0) is dark");
    spi_write(b, 1, 0x0A);
    spi_write(b, 2, 0x0A);
    check(lit_low(b, "G1") == 0x77 && lit_low(b, "G2") == 0x0A,
          "static: V1 = 0xA decodes to A; V2 = 0xA is raw, bits 1 and 3 light pins B and D");
    spi_write(b, 3, 0x01);
    check(lit_low(b, "G2") == 0x77, "static: MODE.M = 1 decodes group 2");
    check(get(b, "G1A") == LOGIC_0 && get(b, "G1D") == LOGIC_1, "static: segmentActive low drives lit pins 0, dark pins 1");
    close_bench(b);
}

static void ports_driver(void) {
    Bench *b = bench("SPIPORTX", NULL, NULL);

    spi_start(b);
    run(b, 20 * US);
    check(get(b, "P0") == LOGIC_Z && get(b, "P3") == LOGIC_Z, "ports: an input port (direction 0) is released");
    check(get(b, "P1") == LOGIC_1, "ports: P1 (always an output, inverted) drives 1 for data 0");
    check(get(b, "P2") == LOGIC_0, "ports: P2 (output while its field is 0) drives data 0");
    spi_write(b, 1, 0x01);
    spi_write(b, 2, 0x0F);
    check(get(b, "P0") == LOGIC_1 && get(b, "P1") == LOGIC_0, "ports: DIR.D0 = 1 drives P0 with its data; inverted P1 drives 0");
    check(get(b, "P2") == LOGIC_Z, "ports: an open-drain port never drives 1");
    check(get(b, "P3") == LOGIC_Z, "ports: P3 stays released while its mode (0) is not 1 or 3");
    spi_write(b, 3, 0x03);
    check(get(b, "P3") == LOGIC_1, "ports: mode 3 is one of dirOutput's values: P3 drives its data");
    spi_write(b, 3, 0x02);
    check(get(b, "P3") == LOGIC_Z, "ports: mode 2 releases P3 again");
    spi_write(b, 1, 0x04);
    check(get(b, "P2") == LOGIC_Z && get(b, "P0") == LOGIC_Z, "ports: DIR.D2 = 1 makes P2 an input");
    spi_write(b, 1, 0x00);
    spi_write(b, 2, 0x00);
    check(get(b, "P2") == LOGIC_0, "ports: an open-drain port drives 0");
    close_bench(b);
}

static void matrix_driver(void) {
    Bench *b = bench("SPIMATX", NULL, NULL);
    static const char *const ROWS[] = {"ROW1", "ROW2", "ROW3"};
    static const char *const COLS[] = {"COL1", "COL2", "COL3", "COL4"};
    /* ROWD1 = 0x09: bits 3 and 0 -> COL1, COL4; ROWD3 = 0x03: bits 1, 0 -> COL3, COL4 (COL n = bit 4 - n) */
    unsigned want[3] = {0x1 | 0x8, 0, 0x4 | 0x8};
    unsigned seen = 0;
    int ok = 1, t, shut_ok = 1;

    spi_start(b);
    spi_write(b, 1, 0x09);
    spi_write(b, 3, 0x03);
    for (t = 0; t < 60; t++) {
        int active = -1;
        unsigned cols = 0;
        size_t i;

        for (i = 0; i < 3; i++) {
            if (get(b, ROWS[i]) == LOGIC_0) {
                active = active == -1 ? (int)i : -2;
            }
        }
        for (i = 0; i < 4; i++) {
            if (get(b, COLS[i]) == LOGIC_1) {
                cols |= 1u << i;
            }
        }
        if (active < 0 || cols != want[active]) {
            ok = 0;
        } else {
            seen |= 1u << active;
        }
        run(b, 20 * US);
    }
    check(ok && seen == 7, "matrix: each row in turn carries its bits on the columns (COL1 = bit 3)");
    spi_write(b, 4, 0x00);
    for (t = 0; t < 30; t++) {
        size_t i;

        for (i = 0; i < 3; i++) {
            shut_ok &= get(b, ROWS[i]) == LOGIC_1;
        }
        for (i = 0; i < 4; i++) {
            shut_ok &= get(b, COLS[i]) == LOGIC_0;
        }
        run(b, 20 * US);
    }
    check(shut_ok, "matrix: shutdown drives every row and column inactive");
    close_bench(b);
}

/* ---- The lcd driver against real glass ---------------------------------------------------------- */

static size_t glass_lights(Bench *b, SimTime since, char *out, size_t cap) {
    const SimComponentType *type = sim_instance_type(b->glass);
    unsigned char values[200];
    size_t n = type->lights(b->sim, b->glass, sim_instance_state(b->glass), since, values, sizeof values);
    size_t i;

    for (i = 0; i < n && i + 1 < cap; i++) {
        out[i] = values[i] ? '1' : '0';
    }
    out[i] = '\0';
    return n;
}

/* The expected lights for RAM bytes: bit n of the stream (byte 0 bit 0 first, or bit 7 first)
   is cell n. */
static void expect_bits(const unsigned *bytes, size_t n, size_t cells, int msb, char *out) {
    size_t i;

    for (i = 0; i < cells; i++) {
        unsigned byte = bytes[i / 8], bit = (unsigned)(i % 8);

        out[i] = ((byte >> (msb ? 7 - bit : bit)) & 1u) ? '1' : '0';
    }
    out[cells] = '\0';
    (void)n;
}

/* Waveform: over `us` microseconds sampling every `step`, which of the backplanes was driven low and
   high, and was ever more than one driven at once. */
static void waveform(Bench *b, const char *const *bps, size_t count, unsigned us, unsigned step,
                     unsigned *low, unsigned *high, int *overlap) {
    unsigned t;

    *low = *high = 0;
    *overlap = 0;
    for (t = 0; t < us; t += step) {
        unsigned driven = 0;
        size_t i;

        for (i = 0; i < count; i++) {
            LogicValue v = get(b, bps[i]);

            if (v == LOGIC_0 || v == LOGIC_1) {
                driven++;
                if (v == LOGIC_0) {
                    *low |= 1u << i;
                } else {
                    *high |= 1u << i;
                }
            }
        }
        if (driven > 1) {
            *overlap = 1;
        }
        run(b, step * US);
    }
}

static void lcd_case(const char *chip, const char *glass, const char *label, unsigned address_bytes,
                     size_t cells, int msb, const char *const *bps, size_t bp_count, unsigned frame_us,
                     const unsigned *pattern, const unsigned *second) {
    static const char *const PULLS[] = {"SDA", "SCL", NULL};
    Bench *b = bench(chip, glass, PULLS);
    char lights[200], want[200], text[160];
    unsigned low, high;
    int overlap;
    SimTime window = (SimTime)frame_us * 2 * US;

    run(b, 10 * US);
    i2c_write(b, 0, pattern, address_bytes);
    run(b, (SimTime)frame_us * 4 * US);
    snprintf(text, sizeof text, "%s: the lit cells equal the RAM bits", label);
    check(glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights) == cells &&
              (expect_bits(pattern, address_bytes, cells, msb, want), strcmp(lights, want) == 0),
          text);
    if (strcmp(lights, want) != 0) {
        printf("  lit  %s\n  want %s\n", lights, want);
    }
    waveform(b, bps, bp_count, frame_us * 3, 50, &low, &high, &overlap);
    snprintf(text, sizeof text, "%s: every backplane is driven low and high over two frames, one at a time", label);
    check(low == (1u << bp_count) - 1u && high == (1u << bp_count) - 1u && !overlap, text);
    i2c_write(b, 0, second, address_bytes);
    run(b, (SimTime)frame_us * 4 * US);
    expect_bits(second, address_bytes, cells, msb, want);
    glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights);
    snprintf(text, sizeof text, "%s: a second write replaces the picture", label);
    check(strcmp(lights, want) == 0, text);
    {
        unsigned off[1] = {0};

        i2c_write(b, 16, off, 1);
    }
    run(b, (SimTime)frame_us * 4 * US);
    glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights);
    snprintf(text, sizeof text, "%s: CTRL.ON = 0 releases every pin and the glass goes dark", label);
    check(strchr(lights, '1') == NULL && get(b, bps[0]) == LOGIC_Z && get(b, "S1A") == LOGIC_Z, text);
    {
        unsigned on[1] = {1};

        i2c_write(b, 16, on, 1);
    }
    run(b, (SimTime)frame_us * 4 * US);
    glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights);
    snprintf(text, sizeof text, "%s: CTRL.ON = 1 brings the picture back", label);
    check(strcmp(lights, want) == 0, text);
    close_bench(b);
}

static void lcd_driver(void) {
    static const char *const BP4[] = {"BP1", "BP2", "BP3", "BP4"};
    static const char *const BP3[] = {"BP1", "BP2", "BP3"};
    static const char *const COM[] = {"COM"};
    const unsigned a[8] = {0xA5, 0x3C, 0xFF, 0x00, 0x81, 0x7E, 0x12, 0xC3};
    const unsigned b2[8] = {0x01, 0x80, 0x00, 0xFF, 0x0F, 0xF0, 0x55, 0xAA};
    const unsigned c[9] = {0xA5, 0x3C, 0xFF, 0x00, 0x81, 0x7E, 0x12, 0xC3, 0x5A};
    const unsigned c2[9] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
    const unsigned d[4] = {0xA5, 0x3C, 0xFF, 0x81};
    const unsigned d2[4] = {0x00, 0x00, 0x00, 0x01};

    check(part_library_find(library, "I2CLCD4X")->kind == PART_LCDDRIVER, "the lcd fixtures load as lcddriver parts");
    lcd_case("I2CLCD4X", "GENERIC-LCD-8D-1:4", "lcd 1:4", 8, 64, 0, BP4, 4, 2000, a, b2);
    /* 1:3 with backplane-major RAM, most significant bit first. The glass has 8 cells per digit:
       digit d (0-7) cell j (0-7) is on segment pin 3d + j / 3 and backplane j % 3, which is bit
       bp * 24 + pin of the RAM stream (byte k / 8, bit 7 - k % 8). */
    {
        static const char *const PULLS[] = {"SDA", "SCL", NULL};
        Bench *b = bench("I2CLCD3X", "GENERIC-LCD-8D-1:3", PULLS);
        char lights[200], want[200];
        const unsigned *ram[2] = {c, c2};
        size_t pass, cell;
        SimTime window = (SimTime)6000 * US;

        run(b, 10 * US);
        for (pass = 0; pass < 2; pass++) {
            i2c_write(b, 0, ram[pass], 9);
            run(b, (SimTime)12000 * US);
            glass_lights(b, sim_now(b->sim) - window, lights, sizeof lights);
            for (cell = 0; cell < 64; cell++) {
                size_t pin = 3 * (cell / 8) + (cell % 8) / 3, bp = (cell % 8) % 3, k = bp * 24 + pin;

                want[cell] = ((ram[pass][k / 8] >> (7 - k % 8)) & 1u) ? '1' : '0';
            }
            want[64] = '\0';
            check(strcmp(lights, want) == 0, pass == 0
                      ? "lcd 1:3: backplane-major RAM, MSB first: the lit cells follow the RAM bits"
                      : "lcd 1:3: a second write replaces the picture");
            if (strcmp(lights, want) != 0) {
                printf("  lit  %s\n  want %s\n", lights, want);
            }
            if (pass == 0) {
                unsigned low, high;
                int overlap;

                waveform(b, BP3, 3, 9000, 50, &low, &high, &overlap);
                check(low == 7 && high == 7 && !overlap,
                      "lcd 1:3: every backplane is driven low and high, one at a time");
            }
        }
        close_bench(b);
    }
    lcd_case("I2CLCDSX", "GENERIC-LCD-4D-STATIC", "lcd static", 4, 32, 0, COM, 1, 2000, d, d2);
    /* Static: one backplane toggling every frame: both levels seen, the segment is its inverse. */
    {
        static const char *const PULLS[] = {"SDA", "SCL", NULL};
        Bench *b = bench("I2CLCDSX", "GENERIC-LCD-4D-STATIC", PULLS);
        unsigned low = 0, high = 0, t;
        int pair_ok = 1;
        const unsigned one[1] = {0x01};

        run(b, 10 * US);
        i2c_write(b, 0, one, 1);
        run(b, (SimTime)6000 * US);
        for (t = 0; t < 8000; t += 100) {
            LogicValue com = get(b, "COM"), seg = get(b, "S1A"), dark = get(b, "S1B");

            low |= com == LOGIC_0;
            high |= com == LOGIC_1;
            pair_ok &= (seg != com) && (dark == com);
            run(b, 100 * US);
        }
        check(low && high && pair_ok,
              "lcd static: COM alternates 0/1 each frame; a set bit drives the inverse of COM, a clear bit COM's level");
        close_bench(b);
    }
}

/* ---- Loader rejections --------------------------------------------------------------------------- */

#define REJ_SPEC SPI_SPEC " DIG1:o DIG2:o " SEG7_SPEC " VCC:p GND:g"
#define REJ_BODY(DISPLAY) \
    "'registers':[" REG("DIGIT1", 1, 0, "") "," REG("DIGIT2", 2, 0, "") "," REG("CFG", 3, 0, F("X", 0, 0)) "]," \
    SPI_WRITE ",'display':" DISPLAY
#define REJ_DIGITS(EXTRA) \
    "{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'high','segmentPins':[" SEG7_NAMES "]," \
    "'segmentActive':'high','ram':['DIGIT1','DIGIT2'],'scan':{'periodUs':100}" EXTRA "}"

/* One device in a file of this kind: loaded (and then unloaded again by the test's own library
   copy? no: a failing load adds nothing, a passing one stays) and compared with `want`. */
static void rejects(const char *kind, const char *device, const char *want, const char *what) {
    char *text = malloc(strlen(device) + 256), *copy, *error = NULL;
    size_t before = part_library_count(library);
    int result;

    sprintf(text, "{'format':'schematical-parts','version':1,'kind':'%s','devices':[%s]}", kind, device);
    copy = json(text);
    result = load_parts(library, copy, "reject.json", &error);
    if (result == 0 || error == NULL || strstr(error, want) == NULL) {
        printf("  got:  %s\n  want: %s\n", error != NULL ? error : "(loaded)", want);
    }
    check(result != 0 && error != NULL && strstr(error, want) != NULL && part_library_count(library) == before,
          what);
    free(error);
    free(copy);
    free(text);
}

static char *led_with(const char *id, const char *body) {
    return led_device(id, REJ_SPEC, body);
}

/* A leddriver device with this display. */
static void rejects_led(const char *display, const char *want, const char *what) {
    char body[8192];
    char *dev;

    snprintf(body, sizeof body, REJ_BODY("%s"), display);
    dev = led_with("REJ", body);
    rejects("leddriver", dev, want, what);
    free(dev);
}

static void accepts_led(const char *id, const char *display, const char *what) {
    char body[8192];
    char *dev, *text, *copy, *error = NULL;
    int result;

    snprintf(body, sizeof body, REJ_BODY("%s"), display);
    dev = led_with(id, body);
    text = malloc(strlen(dev) + 256);
    sprintf(text, "{'format':'schematical-parts','version':1,'kind':'leddriver','devices':[%s]}", dev);
    copy = json(text);
    result = load_parts(library, copy, "ok.json", &error);
    if (result != 0) {
        printf("  error: %s\n", error != NULL ? error : "?");
    }
    check(result == 0, what);
    free(error);
    free(copy);
    free(text);
    free(dev);
}

static void rejects_lcd(const char *display, const char *want, const char *what) {
    char body[8192];
    char *dev;

    snprintf(body, sizeof body, "'registers':[]," I2C_PROTOCOL ",'display':%s", display);
    dev = device("REJL", "LCD driver", "", I2C_SPEC " BP1:o BP2:o S1:o S2:o", "[2.5,6]", I2C_BUS, body);
    rejects("lcddriver", dev, want, what);
    free(dev);
}

static void loader(void) {
    char *dev;

    check(part_library_find(library, "SPILED4X")->kind == PART_LEDDRIVER, "the fixtures load as leddriver parts");
    accepts_led("OKD", REJ_DIGITS(""), "a minimal digits driver loads");
    /* keys of other kinds */
    dev = led_with("REJ", REJ_BODY(REJ_DIGITS("")));
    rejects("converter", dev, "unknown key \"display\"", "a converter device may not have \"display\"");
    rejects("rtc", dev, "unknown key \"display\"", "an rtc device may not have \"display\"");
    free(dev);
    dev = led_with("REJ", "'registers':[],'conversion':{}");
    rejects("leddriver", dev, "unknown key \"conversion\"", "a leddriver device may not have \"conversion\"");
    free(dev);
    dev = led_with("REJ", "'registers':[]");
    rejects("leddriver", dev, "a display driver needs \"display\"", "a leddriver device needs \"display\"");
    free(dev);
    rejects_led("{'driver':'blink'}", "unknown driver \"blink\"", "an unknown driver is rejected");
    rejects_led("{'digits':2}", "missing \"driver\"", "a display without a driver is rejected");
    rejects_led(REJ_DIGITS(",'blink':1"), "unknown key \"blink\"", "an unknown display key is rejected");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'high','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','NOPE'],'scan':{'periodUs':100}}",
                "unknown register \"NOPE\"", "an unknown register in ram is rejected");
    rejects_led(REJ_DIGITS(",'decode':'hex','decodeField':['CFG.NOPE','CFG.X']"), "has no field \"NOPE\"",
                "an unknown field in decodeField is rejected");
    rejects_led(REJ_DIGITS(",'blank':'CFG.NOPE=1'"), "has no field \"NOPE\"",
                "an unknown field in a condition is rejected");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG9'],'digitActive':'high','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','DIGIT2'],'scan':{'periodUs':100}}",
                "no pin \"DIG9\" in this pinout", "a pin the pinout lacks is rejected");
    rejects_led("{'driver':'digits','digits':3,'digitPins':['DIG1','DIG2'],'digitActive':'high','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','DIGIT2'],'scan':{'periodUs':100}}",
                "\"digitPins\" must name 3 pins", "digits and digitPins must agree");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'high','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1'],'scan':{'periodUs':100}}",
                "\"ram\" has 1 registers for 2 digits", "digits and ram must agree");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG1'],'digitActive':'high','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','DIGIT2'],'scan':{'periodUs':100}}",
                "pin \"DIG1\" is used twice", "a pin used twice is rejected");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'up','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','DIGIT2'],'scan':{'periodUs':100}}",
                "\"digitActive\" must be \"high\" or \"low\"", "a bad level word is rejected");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'high','segmentPins':["
                SEG7_NAMES "],'segmentActive':'high','ram':['DIGIT1','DIGIT2']}",
                "missing \"scan\"", "a scanning driver needs scan");
    rejects_led(REJ_DIGITS(",'decode':'rot13'"), "\"decode\" must be", "an unknown decode is rejected");
    rejects_led("{'driver':'digits','digits':2,'digitPins':['DIG1','DIG2'],'digitActive':'high','segmentPins':["
                SEG7_NAMES ",'SEGH','SEGI','SEGJ','SEGK','SEGL','SEGM','SEGN'],'segmentActive':'high',"
                "'ram':['DIGIT1','DIGIT2'],'decode':'hex','scan':{'periodUs':100}}",
                "\"decode\" needs 7 segment pins", "a decode needs 7 or 8 segment pins");
    rejects_led(REJ_DIGITS(",'decodeField':'CFG.X'"), "needs a \"decode\" other than \"none\"",
                "decodeField without a decode is rejected");
    rejects_led(REJ_DIGITS(",'segmentBits':[0,1,2,3,4,5,5]"), "names bit 5 twice", "segmentBits must be unique");
    rejects_led(REJ_DIGITS(",'openDrain':['DIG2','VCC']"),
                "\"openDrain\": \"VCC\" is not a pin this driver drives", "openDrain must name driven pins");
    rejects_led("{'driver':'ports','ports':[{'pin':'SEGA','dataField':'CFG.X','dirOutput':1}]}",
                "\"dirOutput\" needs \"dirField\"", "dirOutput needs dirField");
    rejects_led("{'driver':'static','segmentActive':'high','groups':[{'pins':['SEGA','SEGB'],'ram':'DIGIT1'},"
                "{'pins':['SEGC'],'ram':'DIGIT2'}]}",
                "every group has the same number of pins", "static groups have equal width");
    rejects_led("{'driver':'matrix','rows':['DIG1'],'rowActive':'low','columns':['SEGA'],'columnActive':'high',"
                "'ram':{'bytes':1,'address':40},'scan':{'periodUs':0}}",
                "must be a time in microseconds", "a zero scan period is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:5','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000}", "\"mode\" must be", "an unknown lcd mode is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:4','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000}", "\"backplanes\" must name 4 pins",
                "the backplane count must fit the mode");
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':{'bytes':0,'address':0},'frameUs':1000}", "\"bytes\" is 1-65536",
                "a ram of 0 bytes is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':{'bytes':1,'address':0,'size':3},'frameUs':1000}", "unknown key \"size\"",
                "an unknown ram key is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000,'enable':'NOPE.X=1'}", "unknown register \"NOPE\"",
                "an unknown register in enable is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':['CTRL'],'frameUs':1000}", "unknown register \"CTRL\"", "an unknown ram register is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],'segmentPins':['S1','S2','S1'],"
                "'ram':{'bytes':1,'address':0},'frameUs':1000}", "pin \"S1\" is used twice",
                "a segment pin used twice is rejected");
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],'segmentPins':['S1','S2'],"
                "'ram':{'bytes':1,'address':0},'frameUs':0}", "\"frameUs\" must be a time in microseconds",
                "a zero frame time is rejected");
    /* 5 segments x 2 backplanes need 10 bits, one byte has 8 */
    rejects_lcd("{'driver':'lcd','mode':'1:2','backplanes':['BP1','BP2'],"
                "'segmentPins':['S1','S2','S3','S4','S5'],'ram':{'bytes':1,'address':0},'frameUs':1000}",
                "\"ram\" holds 8 bits", "a ram that is too small is rejected");
}

int main(void) {
    if (!load_fixtures()) {
        return 1;
    }
    led4();
    decoders();
    static_driver();
    ports_driver();
    matrix_driver();
    lcd_driver();
    loader();
    part_library_destroy(library);
    if (failures) {
        printf("%d FAILED\n", failures);
        return 1;
    }
    printf("display_out_test: all passed\n");
    return 0;
}
