/*
 * Checks the LED 7-segment displays: catalog entries (category, digits,
 * colors, sizes, LED lists) and the model (segments light on the right
 * digit for common anode and common cathode parts, doubled common pins are
 * joined, and a multiplexed digit that was ON during a step still glows).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catalog.h"
#include "led7.h"
#include "netlist_sim.h"
#include "sim.h"
#include "supply.h"
#include "switch.h"

static int failures = 0;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL %s\n", message);
        failures++;
    }
}

static const ChipDefinition *definition_for(const char *name) {
    size_t i;

    for (i = 0; i < led7_definition_count(); i++) {
        const ChipDefinition *chip = led7_definition_at(i);

        if (strcmp(chip->part_numbers[0], name) == 0) {
            return chip;
        }
    }
    return NULL;
}

/* ---- Catalog ------------------------------------------------------------ */

static const struct {
    const char *name;
    unsigned digits;
    const char *size;
    const char *color;
    int common_anode;
    size_t leds;
} EXPECTED[] = {
    {"LTC-4727JR", 4, "0.4 in", "Red", 0, 35},   /* 4 x (A-G, DP) + L1-L3 */
    {"LTC-4627JR", 4, "0.4 in", "Red", 1, 35},
    {"LTC-5723HR", 4, "0.56 in", "Red", 0, 32},
    {"LTC-5623G", 4, "0.56 in", "Green", 1, 32},
    {"LTC-2623G", 4, "0.28 in", "Green", 1, 35},
    {"LTC-4624JR", 3, "0.4 in", "Red", 1, 27},
    {"LTC-4724JR", 3, "0.4 in", "Red", 0, 27},
    {"LTD-4608JF", 2, "0.4 in", "Amber", 1, 16},
    {"LTD-2701JD", 2, "0.28 in", "Red", 0, 16},
    {"LTD-6910HR", 2, "0.56 in", "Red", 1, 16},
    {"LTD-6730R", 2, "0.56 in", "Red", 1, 13},  /* +/-1 digit: B, C, G, DP, HJ */
    {"LTD-6850Y", 2, "0.56 in", "Yellow", 0, 13},
    {"LTD-6640E", 2, "0.56 in", "Orange", 0, 16},
    {"LSHD-5503", 1, "0.56 in", "Red", 0, 8},
    {"LSHD-7801", 1, "0.3 in", "Green", 1, 8},
    {"LTS-312AHR", 1, "0.3 in", "Red", 1, 9},   /* left and right decimal points */
    {"LTS-4802BJR-H1", 1, "0.4 in", "Red", 1, 8},
};

#define EXPECTED_COUNT (sizeof(EXPECTED) / sizeof(EXPECTED[0]))

static void check_catalog(void) {
    char *json = catalog_json_copy();
    size_t i;

    /* 26 single datasheets + 23 more from the LTD-6000 series sheet. */
    expect(led7_definition_count() == 49, "49 displays");
    for (i = 0; i < led7_definition_count(); i++) {
        const ChipDefinition *chip = led7_definition_at(i);
        SimLogicConfig config;

        expect(strcmp(chip->category, "Indicators") == 0, "displays are Indicators");
        expect(chip->subcategory != NULL && strcmp(chip->subcategory, "LED 7-Seg") == 0,
               "displays are in the LED 7-Seg group");
        expect(led7_lookup(chip->part_numbers[0], &config) == &LED7_TYPE, "display resolves");
        expect(chip->gate_count == led7_digits(&config), "count is the number of digits");
    }

    for (i = 0; i < EXPECTED_COUNT; i++) {
        const ChipDefinition *chip = definition_for(EXPECTED[i].name);
        SimLogicConfig config;

        if (chip == NULL || led7_lookup(EXPECTED[i].name, &config) == NULL) {
            fprintf(stderr, "FAIL %s is not in the catalog\n", EXPECTED[i].name);
            failures++;
            continue;
        }
        expect(chip->gate_count == EXPECTED[i].digits, "digit count");
        expect(strcmp(led7_size_label(&config), EXPECTED[i].size) == 0, "digit height");
        expect(strcmp(led7_color_label(&config), EXPECTED[i].color) == 0, "filter color");
        expect(led7_common_anode(&config) == EXPECTED[i].common_anode, "common anode or cathode");
        if (led7_led_count(&config) != EXPECTED[i].leds) {
            fprintf(stderr, "FAIL %s has %zu LEDs, expected %zu\n", EXPECTED[i].name,
                    led7_led_count(&config), EXPECTED[i].leds);
            failures++;
        }
    }

    /* LTC-4727JR pins 10 and 12 are missing (no pin): hidden. Pin 9 is a real NC lead. */
    {
        const ChipDefinition *clock = definition_for("LTC-4727JR");

        expect(clock->pins[9].group != NULL && strcmp(clock->pins[9].group, CHIP_PIN_GROUP_HIDDEN) == 0 &&
               clock->pins[11].group != NULL && strcmp(clock->pins[11].group, CHIP_PIN_GROUP_HIDDEN) == 0,
               "missing pins are in the hidden group");
        expect(clock->pins[8].group == NULL && clock->pins[8].role == PIN_ROLE_NO_CONNECT,
               "a no-connect lead is not hidden");
    }
    expect(json != NULL && strstr(json, "{\"number\":10,\"name\":\"NP\",\"role\":\"noConnect\",\"group\":\"hidden\"}") != NULL,
           "catalog JSON puts missing pins in the hidden group");

    expect(definition_for("LTD-6710R")->part_flags[0] & PART_FLAG_OBSOLETE, "LTD-6710R is obsolete");
    expect(!(definition_for("LTD-6410G")->part_flags[0] & PART_FLAG_OBSOLETE), "LTD-6410G is current");

    expect(json != NULL && strstr(json, "\"name\":\"LTC-4727JR\",\"package\":\"DIP\",\"family\":\"Red\",\"size\":\"0.4 in\"") != NULL,
           "catalog gives a display's color and size");
    expect(json != NULL && strstr(json, "\"display\":{\"digits\":4,\"commonAnode\":false,\"colonAfter\":2,\"dotAfter\":3") != NULL,
           "catalog gives a clock display's layout");
    expect(json != NULL && strstr(json, "\"category\":\"Indicators\",\"gateCount\":2,\"subcategory\":\"LED 7-Seg\"") != NULL,
           "catalog lists displays under Indicators -> LED 7-Seg");
    free(json);
}

/* ---- Model -------------------------------------------------------------- */

/* Opens a netlist, runs one step, and returns its "glow" bits for part `part`. */
static int glow_of(const char *text, size_t part, char *out, size_t cap) {
    char *error = NULL;
    NetlistSim *sim = netlist_sim_open(text, &error);
    char *snapshot;
    char needle[32];
    const char *found;
    int ok = 0;

    if (sim == NULL) {
        fprintf(stderr, "FAIL netlist did not open: %s\n", error != NULL ? error : "?");
        failures++;
        free(error);
        return 0;
    }
    netlist_sim_advance(sim, 1000);
    snapshot = netlist_sim_snapshot(sim);
    snprintf(needle, sizeof(needle), "glow %zu ", part);
    found = snapshot != NULL ? strstr(snapshot, needle) : NULL;
    if (found != NULL) {
        size_t n = strcspn(found + strlen(needle), "\n");

        if (n < cap) {
            memcpy(out, found + strlen(needle), n);
            out[n] = '\0';
            ok = 1;
        }
    }
    free(snapshot);
    netlist_sim_close(sim);
    return ok;
}

static void check_model(void) {
    char glow[64];

    /* LTS-4301JR, common cathode: GND on pin 3, VCC on segment A (pin 10). */
    expect(glow_of("nets 2\npart 0 LTS-4301JR\npart 1 VCC\npart 2 GND\n"
                   "pin 0 10 0\npin 1 1 0\npin 0 3 1\npin 2 1 1\n", 0, glow, sizeof(glow)) &&
           strcmp(glow, "10000000") == 0,
           "common cathode: segment A lights");

    /* Same with GND on the other common pin (8): the two are joined inside. */
    expect(glow_of("nets 2\npart 0 LTS-4301JR\npart 1 VCC\npart 2 GND\n"
                   "pin 0 10 0\npin 1 1 0\npin 0 8 1\npin 2 1 1\n", 0, glow, sizeof(glow)) &&
           strcmp(glow, "10000000") == 0,
           "doubled common pins are joined");

    /* No common driven: nothing lights. */
    expect(glow_of("nets 1\npart 0 LTS-4301JR\npart 1 VCC\npin 0 10 0\npin 1 1 0\n",
                   0, glow, sizeof(glow)) && strcmp(glow, "00000000") == 0,
           "a floating common lights nothing");

    /* LTD-4608JR, common anode: VCC on digit 1's anode (pin 9), GND on cathode G (pin 7). */
    expect(glow_of("nets 2\npart 0 LTD-4608JR\npart 1 VCC\npart 2 GND\n"
                   "pin 0 9 0\npin 1 1 0\npin 0 7 1\npin 2 1 1\n", 0, glow, sizeof(glow)) &&
           strcmp(glow, "0000001000000000") == 0,
           "common anode: segment G of digit 1 lights, digit 2 stays dark");

    /*
     * LTC-4727JR (clock, common cathode): VCC on A/L1 (pin 14), GND on
     * digit 3 (pin 6) and on the L common (pin 4): digit 3's A and L1 light.
     */
    expect(glow_of("nets 2\npart 0 LTC-4727JR\npart 1 VCC\npart 2 GND\n"
                   "pin 0 14 0\npin 1 1 0\npin 0 6 1\npin 0 4 1\npin 2 1 1\n", 0, glow, sizeof(glow)) &&
           strcmp(glow, "00000000" "00000000" "10000000" "00000000" "100") == 0,
           "clock display: segment A of digit 3 and colon dot L1 light");

    /* LTD-6730R +/-1 digit: GND on HJ1 (pin 2), VCC on digit 1's anode (pin 14). */
    expect(glow_of("nets 2\npart 0 LTD-6730R\npart 1 VCC\npart 2 GND\n"
                   "pin 0 14 0\npin 1 1 0\npin 0 2 1\npin 2 1 1\n", 0, glow, sizeof(glow)) &&
           strcmp(glow, "00001" "00000000") == 0,
           "overflow digit: the HJ bar lights");
}

/*
 * A multiplexed digit lit only for part of a step still glows for that step:
 * digit 1's common is switched to GND, then away, within one step.
 */
static void check_persistence(void) {
    SimLogicConfig display_config;
    SimLogicConfig switch_config;
    SupplyConfig vcc_config = {SUPPLY_VCC};
    SupplyConfig gnd_config = {SUPPLY_GND};
    Sim *sim = sim_create();
    SimNet *segment = sim_add_net(sim, "SEG");
    SimNet *common = sim_add_net(sim, "COM");
    SimNet *ground = sim_add_net(sim, "GND");
    SimInstance *display = sim_add_instance(sim, led7_lookup("LTS-4301JR", &display_config), "D1", &display_config);
    SimInstance *sw = sim_add_instance(sim, switch_lookup("TE-GDS0204", &switch_config), "S1", &switch_config);
    SimInstance *vcc = sim_add_instance(sim, supply_lookup("VCC", &vcc_config), "V", &vcc_config);
    SimInstance *gnd = sim_add_instance(sim, supply_lookup("GND", &gnd_config), "G", &gnd_config);
    char glow[16];
    int ok = display && sw && vcc && gnd &&
             sim_connect(sim, segment, display, 9) && sim_connect(sim, segment, vcc, 0) &&
             sim_connect(sim, common, display, 2) && sim_connect(sim, common, sw, 0) &&
             sim_connect(sim, ground, sw, 3) && sim_connect(sim, ground, gnd, 0) &&
             sim_start(sim) && sim_run_until(sim, 1000);

    expect(ok, "persistence bench setup");
    if (ok) {
        switch_set_position(sim, sw, 1, 1);
        sim_run_until(sim, 2000);
        switch_set_position(sim, sw, 1, 0);
        sim_run_until(sim, 3000);

        led7_instance_glow(sim, display, 1500, glow, sizeof(glow));
        expect(glow[0] == '1', "a segment ON during the step glows");
        led7_instance_glow(sim, display, 2500, glow, sizeof(glow));
        expect(glow[0] == '0', "a segment OFF for the whole step is dark");
    }
    sim_destroy(sim);
}

int main(void) {
    check_catalog();
    check_model();
    check_persistence();

    if (failures != 0) {
        fprintf(stderr, "%d LED 7-segment check(s) failed\n", failures);
        return 1;
    }
    printf("LED 7-segment checks passed\n");
    return 0;
}
