/*
 * lcd_test: the lcd kind (models/lcd.c, plan-56-A) on a fixture glass loaded
 * next to the real part files: the loader's rules and rejections and the
 * model (a cell is lit while its segment and backplane pins are both driven
 * and differ; a backplane at Z lights nothing; two backplanes share a
 * segment pin on a multiplexed glass). Then every shipped glass is checked
 * for its first cell.
 *
 *   TESTLCD   1 S1, 2 S2, 3 BP1, 4 BP2; drive 1:2; cells A (S1,BP1), B (S1,BP2),
 *             G (S2,BP1), C (S2,BP2)
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

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

#define LX_PIN(N, NAME) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"input\"}"
#define LX_HEAD "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"lcd\",\"devices\":["
#define LX_DEVICE(ID) "{\"id\":\"" ID "\",\"function\":\"LCD\",\"description\":\"fixture\",\"drawer\":[],"
#define LX_PINS "\"pinouts\":{\"p4\":[" LX_PIN(1, "S1") "," LX_PIN(2, "S2") "," LX_PIN(3, "BP1") "," \
                LX_PIN(4, "BP2") "]},"
#define LX_FACTS "\"digits\":1,\"colonAfter\":0,\"dotAfter\":0,\"color\":\"Gray\",\"size\":\"0.5 in\","
#define LX_SIDES "\"segmentSide\":[\"S1\",\"S2\"],\"backplaneSide\":[\"BP1\",\"BP2\"],"
#define LX_CELL(D, SEG, PIN, BP) "{\"digit\":" #D ",\"segment\":\"" SEG "\",\"pin\":\"" PIN "\",\"bp\":\"" BP "\"}"
#define LX_CELLS "\"cells\":[" LX_CELL(1, "A", "S1", "BP1") "," LX_CELL(1, "B", "S1", "BP2") "," \
                 LX_CELL(1, "G", "S2", "BP1") "," LX_CELL(1, "C", "S2", "BP2") "]"
#define LX_PART(NAME) ",\"parts\":[{\"name\":\"" NAME "\",\"manufacturer\":\"Generic\",\"package\":\"LCD\"," \
                      "\"pinout\":\"p4\",\"footprint\":\"LED-DIP4\"}]}"

static const char FIXTURE[] =
    LX_HEAD LX_DEVICE("lx") LX_PINS LX_FACTS "\"drive\":\"1:2\",\"backplanes\":[\"BP1\",\"BP2\"]," LX_SIDES LX_CELLS
    LX_PART("TESTLCD") "]}";

/* A one-device file with these kind keys must be rejected with `want` in its message. */
static void rejects(const char *keys, const char *want, const char *what) {
    char json[4096];
    char *error = NULL;
    size_t before = part_library_count(library);
    int result;

    snprintf(json, sizeof json, LX_HEAD LX_DEVICE("bad") LX_PINS LX_FACTS "%s" LX_PART("BADLCD") "]}", keys);
    result = load_parts(library, json, "bad.json", &error);
    if (result == 0 || error == NULL || strstr(error, want) == NULL) {
        printf("  got: %s\n  want: %s\n", error != NULL ? error : "(loaded)", want);
    }
    expect(result != 0 && error != NULL && strstr(error, want) != NULL &&
               part_library_count(library) == before, what);
    free(error);
}

static void check_loader(void) {
    const Part *part = part_library_find(library, "TESTLCD");

    expect(part != NULL && part->kind == PART_LCD && part->display != NULL &&
               part->display->led_count == 4 && part->display->segment_pin_count == 2 &&
               part->display->common_pin_count == 2 && strcmp(part->display->type, "1:2") == 0,
           "TESTLCD loads: 4 cells, 2 segment pins, 2 backplanes, drive 1:2");
    rejects("\"drive\":\"1:5\",\"backplanes\":[\"BP1\"]," LX_SIDES LX_CELLS, "\"drive\" must be",
            "a drive that is not static, 1:2, 1:3 or 1:4 is rejected");
    rejects("\"drive\":\"static\",\"backplanes\":[\"BP1\"]," LX_SIDES LX_CELLS,
            "is not one of \"backplanes\"", "a cell on a backplane the device does not list is rejected");
    rejects("\"drive\":\"1:2\",\"backplanes\":[\"BP1\",\"BP2\"]," LX_SIDES "\"cells\":["
            LX_CELL(1, "A", "S1", "BP1") "," LX_CELL(1, "A", "S2", "BP2") "]",
            "is given twice", "the same digit and segment twice is rejected");
    rejects("\"drive\":\"1:2\",\"backplanes\":[\"BP1\",\"BP2\"]," LX_SIDES "\"cells\":["
            LX_CELL(1, "Q", "S1", "BP1") "]",
            "unknown segment", "an unknown segment name is rejected");
    rejects("\"drive\":\"1:2\",\"backplanes\":[\"BP1\",\"BP2\"],\"segmentSide\":[\"S1\",\"S9\"],"
            "\"backplaneSide\":[\"BP1\",\"BP2\"]," LX_CELLS,
            "S9", "a side that names a pin the cells do not use is rejected");
}

/* ---- Model ------------------------------------------------------------------ */

static SimInstance *add(Sim *sim, const char *name) {
    const Part *part = part_library_find(library, name);

    return part != NULL ? sim_add_instance(sim, kind_model(part), name, part) : NULL;
}

static void glow(const Sim *sim, const SimInstance *instance, SimTime since, char *out, size_t cap) {
    const SimComponentType *type = sim_instance_type(instance);
    unsigned char values[160];
    size_t count = type->lights(sim, instance, sim_instance_state(instance), since, values,
                                sizeof values);
    size_t i;

    for (i = 0; i < count && i + 1 < cap; i++) {
        out[i] = values[i] ? '1' : '0';
    }
    out[i] = '\0';
}

/*
 * One glass, VCC on net 1 and GND on net 0. `wires` is one char per pin: '1' VCC,
 * '0' GND, '-' not connected. Runs 1 ns and returns the lights.
 */
static int glow_of(const char *glass, const char *wires, char *out, size_t cap) {
    Sim *sim = sim_create();
    SimNet *gnd_net = sim_add_net(sim, "GND");
    SimNet *vcc_net = sim_add_net(sim, "VCC");
    SimInstance *part = add(sim, glass);
    SimInstance *vcc = add(sim, "5V");
    SimInstance *gnd = add(sim, "GND");
    size_t i;
    int ok = part != NULL && vcc != NULL && gnd != NULL && sim_connect(sim, vcc_net, vcc, 0) &&
             sim_connect(sim, gnd_net, gnd, 0);

    for (i = 0; ok && wires[i] != '\0'; i++) {
        if (wires[i] != '-') {
            ok = sim_connect(sim, wires[i] == '1' ? vcc_net : gnd_net, part, i);
        }
    }
    ok = ok && sim_start(sim) && sim_run_until(sim, 1000);
    if (ok) {
        glow(sim, part, 0, out, cap);
    }
    sim_destroy(sim);
    return ok;
}

static void check_model(void) {
    char lights[160];

    /* S1 1, S2 -, BP1 0, BP2 -: only A is driven on both sides and differs. */
    expect(glow_of("TESTLCD", "1-0-", lights, sizeof lights) && strcmp(lights, "1000") == 0,
           "segment 1 against a backplane 0 lights that cell; the Z backplane's cells stay dark");
    /* S1 1, S2 1, BP1 0, BP2 1: A and G differ from BP1; B and C equal BP2. */
    expect(glow_of("TESTLCD", "1101", lights, sizeof lights) && strcmp(lights, "1010") == 0,
           "in phase with the backplane is off, out of phase is on");
    /* S1 0, S2 0, BP1 0, BP2 1: B and C differ from BP2. */
    expect(glow_of("TESTLCD", "0001", lights, sizeof lights) && strcmp(lights, "0101") == 0,
           "a segment at 0 lights the cells whose backplane is 1");
    expect(glow_of("TESTLCD", "----", lights, sizeof lights) && strcmp(lights, "0000") == 0,
           "nothing connected lights nothing");
}

/* ---- Shipped glass ---------------------------------------------------------- */

static void check_parts(void) {
    size_t i;
    size_t glasses = 0;
    int ok = 1;

    for (i = 0; i < part_library_count(library); i++) {
        const Part *part = part_library_at(library, i);

        if (part->kind != PART_LCD) {
            continue;
        }
        glasses++;
        ok &= part->display != NULL && part->display->led_count > 0 && part->manufacturer != NULL;
        if (part->display != NULL && part->display->led_count > 0) {
            /* every cell is reachable by name: segment and backplane pins exist */
            ok &= part->display->segment_pin_count > 0 && part->display->common_pin_count > 0;
        }
    }
    expect(ok, "every shipped glass has cells, segment pins, backplanes and a manufacturer");
    printf("note: %zu shipped glass parts\n", glasses);
}

/* A wiring string for a 40-pin glass: '-' everywhere but the given pins (1-based). */
static void wire40(char *out, const char *pins_and_levels) {
    size_t i;

    memset(out, '-', 40);
    out[40] = '\0';
    for (i = 0; pins_and_levels[i] != '\0'; i += 3) {
        int pin = (pins_and_levels[i] - '0') * 10 + (pins_and_levels[i + 1] - '0');

        out[pin - 1] = pins_and_levels[i + 2];
    }
}

static void check_real_glass(void) {
    char lights[160];
    char wires[41];

    /* LCD-S401C39TR: COM (pin 1) 0, 1A (pin 35) 1 -> cell 0 (digit 1 A) only. */
    wire40(wires, "010" "351");
    expect(glow_of("LCD-S401C39TR", wires, lights, sizeof lights) &&
               strncmp(lights, "10000000", 8) == 0 && strchr(lights + 1, '1') == NULL,
           "LCD-S401C39TR: 1A against COM 0 lights digit 1 segment A and nothing else");
    /* COM is on pins 1 and 40: driving only the far COM works the same. */
    wire40(wires, "400" "351");
    expect(glow_of("LCD-S401C39TR", wires, lights, sizeof lights) && lights[0] == '1',
           "LCD-S401C39TR: the second COM pin (40) is the same backplane");
    /* The colon (pin 28) lights its two cells at the end of the list. */
    wire40(wires, "010" "281");
    expect(glow_of("LCD-S401C39TR", wires, lights, sizeof lights) && strlen(lights) == 33 &&
               strcmp(lights + 31, "11") == 0 && strchr(lights, '1') == lights + 31,
           "LCD-S401C39TR: COL lights the colon's two dots");
    /* A 1:3 glass: BP1 (pin 1) 0, S1A (pin 4) 1: A of digit 1 on BP1 lights; F on BP2 (floating) does not. */
    wire40(wires, "010" "041");
    expect(glow_of("GENERIC-LCD-8D-1:3", wires, lights, sizeof lights) && lights[0] == '1' &&
               lights[1] == '0',
           "GENERIC-LCD-8D-1:3: a cell lights only on its own, driven backplane");
    /* Same pin, BP2 (pin 2) at 0 and BP1 at Z: F (the next cell on this pin) lights. */
    wire40(wires, "020" "041");
    expect(glow_of("GENERIC-LCD-8D-1:3", wires, lights, sizeof lights) && lights[0] == '0' &&
               lights[1] == '1',
           "GENERIC-LCD-8D-1:3: the next slot's cell on the same segment pin lights with BP2");
}

int main(void) {
    char *error = NULL;

    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    if (load_parts(library, FIXTURE, "lcd-fixture.json", &error) != 0) {
        fprintf(stderr, "lcd fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return 1;
    }
    check_loader();
    check_model();
    check_parts();
    check_real_glass();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d checks failed\n", failures);
        return 1;
    }
    printf("lcd_test: all checks passed\n");
    return 0;
}
