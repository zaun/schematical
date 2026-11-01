/*
 * display7_test: the 7-segment model on the part library (port of the old
 * led7_check.c check_model and check_persistence). Parts come from the JSON
 * files; lights are read through the model's lights hook. The catalog
 * checks are covered by listing_check.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"

static int failures;
static PartLibrary *library;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL %s\n", message);
        failures++;
    } else {
        printf("ok   %s\n", message);
    }
}

static SimInstance *add(Sim *sim, const char *name) {
    const Part *part = part_library_find(library, name);

    return part != NULL ? sim_add_instance(sim, kind_model(part), name, part) : NULL;
}

/* The lights of an instance as '0'/'1' characters, lit since `since`. */
static void glow(const Sim *sim, const SimInstance *instance, SimTime since, char *out, size_t cap) {
    const SimComponentType *type = sim_instance_type(instance);
    unsigned char values[64];
    size_t count = type->lights(sim, instance, sim_instance_state(instance), since, values,
                                sizeof values);
    size_t i;

    for (i = 0; i < count && i + 1 < cap; i++) {
        out[i] = values[i] ? '1' : '0';
    }
    out[i] = '\0';
}

/*
 * One display, VCC on net 0 and GND on net 1. Each wire is (display pin
 * number, net). Runs one 1 ns step and returns the lights.
 */
static int glow_of(const char *display, const unsigned (*wires)[2], size_t wire_count,
                   int with_gnd, char *out, size_t cap) {
    Sim *sim = sim_create();
    SimNet *nets[2] = {sim_add_net(sim, "VCC"), sim_add_net(sim, "GND")};
    SimInstance *part = add(sim, display);
    SimInstance *vcc = add(sim, "VCC");
    SimInstance *gnd = with_gnd ? add(sim, "GND") : NULL;
    size_t i;
    int ok = part != NULL && vcc != NULL && (gnd != NULL || !with_gnd) &&
             sim_connect(sim, nets[0], vcc, 0) && (!with_gnd || sim_connect(sim, nets[1], gnd, 0));

    for (i = 0; ok && i < wire_count; i++) {
        ok = sim_connect(sim, nets[wires[i][1]], part, wires[i][0] - 1);
    }
    ok = ok && sim_start(sim) && sim_run_until(sim, 1000);
    if (ok) {
        glow(sim, part, 0, out, cap);
    }
    sim_destroy(sim);
    return ok;
}

static void check_model(void) {
    char lights[64];

    /* LTS-4301JR, common cathode: GND on pin 3, VCC on segment A (pin 10). */
    {
        static const unsigned W[][2] = {{10, 0}, {3, 1}};
        expect(glow_of("LTS-4301JR", W, 2, 1, lights, sizeof lights) &&
                   strcmp(lights, "10000000") == 0,
               "common cathode: segment A lights");
    }
    /* Same with GND on the other common pin (8): the two are joined inside. */
    {
        static const unsigned W[][2] = {{10, 0}, {8, 1}};
        expect(glow_of("LTS-4301JR", W, 2, 1, lights, sizeof lights) &&
                   strcmp(lights, "10000000") == 0,
               "doubled common pins are joined");
    }
    /* No common driven: nothing lights. */
    {
        static const unsigned W[][2] = {{10, 0}};
        expect(glow_of("LTS-4301JR", W, 1, 0, lights, sizeof lights) &&
                   strcmp(lights, "00000000") == 0,
               "a floating common lights nothing");
    }
    /* LTD-4608JR, common anode: VCC on digit 1's anode (pin 9), GND on cathode G (pin 7). */
    {
        static const unsigned W[][2] = {{9, 0}, {7, 1}};
        expect(glow_of("LTD-4608JR", W, 2, 1, lights, sizeof lights) &&
                   strcmp(lights, "0000001000000000") == 0,
               "common anode: segment G of digit 1 lights, digit 2 stays dark");
    }
    /*
     * LTC-4727JR (clock, common cathode): VCC on A/L1 (pin 14), GND on
     * digit 3 (pin 6) and on the L common (pin 4): digit 3's A and L1 light.
     */
    {
        static const unsigned W[][2] = {{14, 0}, {6, 1}, {4, 1}};
        expect(glow_of("LTC-4727JR", W, 3, 1, lights, sizeof lights) &&
                   strcmp(lights, "00000000" "00000000" "10000000" "00000000" "100") == 0,
               "clock display: segment A of digit 3 and colon dot L1 light");
    }
    /* LTD-6730R +/-1 digit: GND on HJ1 (pin 2), VCC on digit 1's anode (pin 14). */
    {
        static const unsigned W[][2] = {{14, 0}, {2, 1}};
        expect(glow_of("LTD-6730R", W, 2, 1, lights, sizeof lights) &&
                   strcmp(lights, "00001" "00000000") == 0,
               "overflow digit: the HJ bar lights");
    }
}

static int set_position(Sim *sim, SimInstance *instance, size_t position, int on) {
    const SimComponentType *type = sim_instance_type(instance);

    return type->set_switch(sim, instance, sim_instance_state(instance), position, on);
}

/*
 * A multiplexed digit lit only for part of a step still glows for that step:
 * digit 1's common is switched to GND, then away, within one step.
 */
static void check_persistence(void) {
    Sim *sim = sim_create();
    SimNet *segment = sim_add_net(sim, "SEG");
    SimNet *common = sim_add_net(sim, "COM");
    SimNet *ground = sim_add_net(sim, "GND");
    SimInstance *display = add(sim, "LTS-4301JR");
    SimInstance *sw = add(sim, "TE-GDS0204");
    SimInstance *vcc = add(sim, "VCC");
    SimInstance *gnd = add(sim, "GND");
    char lights[16];
    int ok = display && sw && vcc && gnd &&
             sim_connect(sim, segment, display, 9) && sim_connect(sim, segment, vcc, 0) &&
             sim_connect(sim, common, display, 2) && sim_connect(sim, common, sw, 0) &&
             sim_connect(sim, ground, sw, 3) && sim_connect(sim, ground, gnd, 0) &&
             sim_start(sim) && sim_run_until(sim, 1000);

    expect(ok, "persistence bench setup");
    if (ok) {
        set_position(sim, sw, 1, 1);
        sim_run_until(sim, 2000);
        set_position(sim, sw, 1, 0);
        sim_run_until(sim, 3000);

        glow(sim, display, 1500, lights, sizeof lights);
        expect(lights[0] == '1', "a segment ON during the step glows");
        glow(sim, display, 2500, lights, sizeof lights);
        expect(lights[0] == '0', "a segment OFF for the whole step is dark");
    }
    sim_destroy(sim);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    check_model();
    check_persistence();
    part_library_destroy(library);
    if (failures != 0) {
        fprintf(stderr, "%d 7-segment check(s) failed\n", failures);
        return 1;
    }
    printf("display7_test: all passed\n");
    return 0;
}
