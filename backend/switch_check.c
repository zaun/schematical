/*
 * Checks the switches: the TE DIP switch catalog entries, and contacts in the
 * simulator (values pass both ways through an ON position, OFF isolates,
 * shorts across a switch blow up, chains and loops resolve, and positions can
 * be set from a netlist and flipped while running).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catalog.h"
#include "net.h"
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

/* ---- Catalog ------------------------------------------------------------ */

/* Every tube product key in the datasheet, with its position count. */
static const struct {
    const char *name;
    size_t positions;
    const char *package;
} EXPECTED[] = {
    {"TE-GDS0204", 2, "DIP"}, {"TE-GDS02S04", 2, "SMD"}, {"TE-GDS0404", 4, "DIP"},
    {"TE-GDS04S04", 4, "SMD"}, {"TE-GDS0604", 6, "DIP"}, {"TE-GDS06S04", 6, "SMD"},
    {"TE-GDS0804", 8, "DIP"}, {"TE-GDS08S04", 8, "SMD"}, {"TE-GDS1004", 10, "DIP"},
    {"TE-GDS10S04", 10, "SMD"},
    {"TE-GDP0204", 2, "DIP"}, {"TE-GDP0404", 4, "DIP"}, {"TE-GDP0604", 6, "DIP"},
    {"TE-GDP0804", 8, "DIP"}, {"TE-GDP1004", 10, "DIP"},
    {"TE-GDR0404", 4, "DIP"}, {"TE-GDR04S04", 4, "SMD"}, {"TE-GDR0804", 8, "DIP"},
    {"TE-GDR08S04", 8, "SMD"},
    {"TE-GDH02S04", 2, "SMD"}, {"TE-GDH04S04", 4, "SMD"}, {"TE-GDH08S04", 8, "SMD"},
    {"TE-GDHL04S", 4, "SMD"}, {"TE-GDHL08S", 8, "SMD"},
    {"TE-ADE0204", 2, "DIP"}, {"TE-ADE0404", 4, "DIP"}, {"TE-ADE04S04", 4, "SMD"},
    {"TE-ADE0604", 6, "DIP"}, {"TE-ADE0804", 8, "DIP"}, {"TE-ADE08S04", 8, "SMD"},
    {"TE-ADE1004", 10, "DIP"},
    {"TE-ADF0204", 2, "DIP"}, {"TE-ADF0404", 4, "DIP"}, {"TE-ADF0604", 6, "DIP"},
    {"TE-ADF0804", 8, "DIP"}, {"TE-ADF1004", 10, "DIP"},
};

#define EXPECTED_COUNT (sizeof(EXPECTED) / sizeof(EXPECTED[0]))

/* The catalog definition that lists a part, or NULL. */
static const ChipDefinition *definition_for(const char *name) {
    size_t i;
    size_t j;

    for (i = 0; i < catalog_definition_count(); i++) {
        const ChipDefinition *chip = catalog_definition_at(i);

        for (j = 0; j < chip->part_number_count; j++) {
            if (strcmp(chip->part_numbers[j], name) == 0) {
                return chip;
            }
        }
    }
    return NULL;
}

static void check_catalog(void) {
    size_t listed = 0;
    size_t i;
    char *json = catalog_json_copy();

    for (i = 0; i < switch_definition_count(); i++) {
        const ChipDefinition *chip = switch_definition_at(i);
        size_t j;

        expect(strcmp(chip->category, "Switch") == 0, "switches are in the Switch category");
        expect(chip->subcategory != NULL && strcmp(chip->subcategory, "DIP") == 0,
               "DIP switches are in the DIP subcategory");
        expect(chip->pin_count == 2 * chip->gate_count,
               "a switch has two pins per position");
        for (j = 0; j < chip->part_number_count; j++) {
            expect(strncmp(chip->part_numbers[j], "TE-", 3) == 0, "switch names start with TE-");
            expect(strstr(chip->part_numbers[j], "TR") == NULL,
                   "no tape-and-reel product keys");
        }
        listed += chip->part_number_count;
    }
    expect(listed == EXPECTED_COUNT, "every tube switch is listed once");

    for (i = 0; i < EXPECTED_COUNT; i++) {
        const ChipDefinition *chip = definition_for(EXPECTED[i].name);
        SimLogicConfig config;
        char needle[128];

        if (chip == NULL) {
            fprintf(stderr, "FAIL %s is not in the catalog\n", EXPECTED[i].name);
            failures++;
            continue;
        }
        expect(chip->gate_count == EXPECTED[i].positions, "count is the number of positions");
        expect(switch_lookup(EXPECTED[i].name, &config) == &SWITCH_TYPE, "switch resolves");
        expect(switch_config_positions(&config) == EXPECTED[i].positions,
               "model has the datasheet's position count");
        snprintf(needle, sizeof(needle), "\"name\":\"%s\",\"package\":\"%s\"",
                 EXPECTED[i].name, EXPECTED[i].package);
        expect(json != NULL && strstr(json, needle) != NULL, "catalog gives a switch's package");
    }

    expect(json != NULL &&
           strstr(json, "\"category\":\"Switch\",\"gateCount\":8,\"subcategory\":\"DIP\"") != NULL,
           "catalog JSON lists switches under Switch -> DIP with their count");
    expect(json != NULL &&
           strstr(json, "\"name\":\"TE-GDP0404\",\"package\":\"DIP\",\"family\":\"Piano actuator\"") != NULL,
           "catalog JSON gives a switch's actuator style as its family");
    expect(json != NULL && strstr(json, "\"name\":\"1A\",\"role\":\"inout\"") != NULL,
           "switch pins are in/outs");
    expect(switch_lookup("GDS0404", NULL) == NULL, "names without TE- are not switches");
    expect(switch_lookup("TE-GDS02STR04", NULL) == NULL, "tape-and-reel keys are not listed");
    free(json);
}

/* ---- Contacts ----------------------------------------------------------- */

typedef struct {
    Sim *sim;
    SimInstance *sw;
    SimNet *nets[4];
} Bench;

/*
 * One 4-position switch, TE-GDS0404, on nets[0..3]: position 1 joins nets[0]
 * (pin 1) and nets[1] (pin 8); position 2 joins nets[1] (pin 2) and nets[2]
 * (pin 7); position 3 joins nets[2] (pin 3) and nets[3] (pin 6).
 */
static int bench_open(Bench *bench) {
    SimLogicConfig config;
    const SimComponentType *type = switch_lookup("TE-GDS0404", &config);
    size_t i;
    int ok;

    memset(bench, 0, sizeof(*bench));
    bench->sim = sim_create();
    for (i = 0; i < 4; i++) {
        char name[8];

        snprintf(name, sizeof(name), "N%zu", i);
        bench->nets[i] = sim_add_net(bench->sim, name);
    }
    bench->sw = sim_add_instance(bench->sim, type, "SW1", &config);
    ok = bench->sw != NULL &&
         sim_connect(bench->sim, bench->nets[0], bench->sw, 0) &&
         sim_connect(bench->sim, bench->nets[1], bench->sw, 7) &&
         sim_connect(bench->sim, bench->nets[1], bench->sw, 1) &&
         sim_connect(bench->sim, bench->nets[2], bench->sw, 6) &&
         sim_connect(bench->sim, bench->nets[2], bench->sw, 2) &&
         sim_connect(bench->sim, bench->nets[3], bench->sw, 5);
    return ok;
}

static SimInstance *add_supply(Sim *sim, SupplyKind kind, SimNet *net) {
    SupplyConfig config = {kind};
    const SimComponentType *type = supply_lookup(kind == SUPPLY_VCC ? "VCC" : "GND", &config);
    SimInstance *instance = sim_add_instance(sim, type, kind == SUPPLY_VCC ? "V" : "G", &config);

    if (instance != NULL) {
        sim_connect(sim, net, instance, 0);
    }
    return instance;
}

static LogicValue net_value(const Bench *bench, size_t net) {
    return sim_net_value(bench->nets[net]);
}

static void check_contacts(void) {
    Bench bench;

    /* VCC on net 0. OFF: net 1 floats. ON: it follows. OFF again: floats. */
    if (!bench_open(&bench) || add_supply(bench.sim, SUPPLY_VCC, bench.nets[0]) == NULL ||
        !sim_start(bench.sim) || !sim_run_until(bench.sim, 1000)) {
        expect(0, "switch bench setup");
    } else {
        expect(net_value(&bench, 0) == LOGIC_1, "VCC drives its net");
        expect(net_value(&bench, 1) == LOGIC_Z, "an OFF position isolates");

        expect(switch_set_position(bench.sim, bench.sw, 1, 1), "position 1 turns ON");
        expect(sim_run_until(bench.sim, 2000), "run after turning ON");
        expect(net_value(&bench, 1) == LOGIC_1, "an ON position passes a 1");
        expect(switch_position_on(bench.sim, bench.sw, 1), "position 1 reads ON");
        expect(net_value(&bench, 2) == LOGIC_Z, "position 2 is still OFF");

        /* Chain: positions 1 and 2 in series. */
        switch_set_position(bench.sim, bench.sw, 2, 1);
        sim_run_until(bench.sim, 3000);
        expect(net_value(&bench, 2) == LOGIC_1, "two ON positions in series pass a 1");

        switch_set_position(bench.sim, bench.sw, 1, 0);
        sim_run_until(bench.sim, 4000);
        expect(net_value(&bench, 1) == LOGIC_Z && net_value(&bench, 2) == LOGIC_Z,
               "turning OFF the first position lets the chain float, without latching");
        expect(!switch_position_on(bench.sim, bench.sw, 1), "position 1 reads OFF");
        expect(!switch_set_position(bench.sim, bench.sw, 5, 1), "position 5 is out of range");
        expect(!switch_set_position(bench.sim, bench.sw, 0, 1), "position 0 is out of range");
    }
    sim_destroy(bench.sim);

    /* GND on the far side (net 1) pulls net 0 low: values pass both ways. */
    if (!bench_open(&bench) || add_supply(bench.sim, SUPPLY_GND, bench.nets[1]) == NULL ||
        !switch_set_position(bench.sim, bench.sw, 1, 1) ||
        !sim_start(bench.sim) || !sim_run_until(bench.sim, 1000)) {
        expect(0, "reverse bench setup");
    } else {
        expect(net_value(&bench, 0) == LOGIC_0, "an ON position passes a 0 the other way");
        expect(net_value(&bench, 3) == LOGIC_Z, "an untouched net floats");
    }
    sim_destroy(bench.sim);

    /* VCC on net 0 and GND on net 2, joined by turning ON positions 1 and 2. */
    if (!bench_open(&bench) || add_supply(bench.sim, SUPPLY_VCC, bench.nets[0]) == NULL ||
        add_supply(bench.sim, SUPPLY_GND, bench.nets[2]) == NULL ||
        !sim_start(bench.sim) || !sim_run_until(bench.sim, 1000)) {
        expect(0, "short bench setup");
    } else {
        switch_set_position(bench.sim, bench.sw, 1, 1);
        expect(sim_run_until(bench.sim, 2000), "one ON position does not short");
        expect(net_value(&bench, 1) == LOGIC_1, "net 1 follows VCC");
        switch_set_position(bench.sim, bench.sw, 2, 1);
        expect(!sim_run_until(bench.sim, 3000), "VCC to GND through a switch stops the run");
        expect(sim_error(bench.sim) != NULL && strncmp(sim_error(bench.sim), "smoke:", 6) == 0,
               "a short through a switch is smoke");
    }
    sim_destroy(bench.sim);

    /* A loop: positions 1-3 on, and position 4 joins net 3 back to net 0. */
    {
        SimLogicConfig config;
        const SimComponentType *type = switch_lookup("TE-GDS0204", &config);
        SimInstance *loop;

        if (!bench_open(&bench)) {
            expect(0, "loop bench setup");
        } else {
            loop = sim_add_instance(bench.sim, type, "SW2", &config);
            if (loop == NULL || !sim_connect(bench.sim, bench.nets[3], loop, 0) ||
                !sim_connect(bench.sim, bench.nets[0], loop, 3) ||
                add_supply(bench.sim, SUPPLY_VCC, bench.nets[3]) == NULL) {
                expect(0, "loop bench wiring");
            } else {
                switch_set_position(bench.sim, bench.sw, 1, 1);
                switch_set_position(bench.sim, bench.sw, 2, 1);
                switch_set_position(bench.sim, loop, 1, 1);
                expect(sim_start(bench.sim) && sim_run_until(bench.sim, 1000), "loop runs");
                expect(net_value(&bench, 0) == LOGIC_1 && net_value(&bench, 1) == LOGIC_1 &&
                       net_value(&bench, 2) == LOGIC_1,
                       "VCC reaches every net around a loop of switches");
                expect(net_value(&bench, 3) == LOGIC_1, "the driven net stays high");
            }
        }
        sim_destroy(bench.sim);
    }
}

/* ---- Netlist ------------------------------------------------------------ */

static int snapshot_has(NetlistSim *sim, const char *line) {
    char *text = netlist_sim_snapshot(sim);
    int found = text != NULL && strstr(text, line) != NULL;

    free(text);
    return found;
}

static void check_netlist(void) {
    /*
     * VCC -> switch position 2 -> LED anode; LED cathode -> GND. Position 2
     * of a 4-position switch is pins 2 and 7.
     */
    const char *text =
        "nets 3\n"
        "part 0 VCC\n"
        "part 1 TE-GDS0404\n"
        "part 2 LED-RED-0603\n"
        "part 3 GND\n"
        "pin 0 1 0\n"
        "pin 1 2 0\n"
        "pin 1 7 1\n"
        "pin 2 2 1\n"
        "pin 2 1 2\n"
        "pin 3 1 2\n"
        "switch 1 2 1\n";
    char *error = NULL;
    NetlistSim *sim = netlist_sim_open(text, &error);

    if (sim == NULL) {
        fprintf(stderr, "FAIL netlist with a switch did not open: %s\n", error ? error : "?");
        failures++;
        free(error);
        return;
    }

    expect(netlist_sim_advance(sim, 1000), "netlist runs");
    expect(snapshot_has(sim, "lit 2 1\n"), "a switch that starts ON lights the LED");

    expect(netlist_sim_set_switch(sim, 1, 2, 0), "switch turns OFF from the netlist API");
    expect(netlist_sim_advance(sim, 1000), "netlist runs after turning OFF");
    expect(snapshot_has(sim, "lit 2 0\n") && snapshot_has(sim, "net 1 Z\n"),
           "turning the switch OFF puts the LED out");

    expect(netlist_sim_set_switch(sim, 1, 2, 1), "switch turns back ON");
    expect(netlist_sim_advance(sim, 1000), "netlist runs after turning ON");
    expect(snapshot_has(sim, "lit 2 1\n"), "turning it back ON lights the LED");

    expect(!netlist_sim_set_switch(sim, 2, 1, 1), "an LED is not a switch");
    expect(!netlist_sim_set_switch(sim, 1, 9, 1), "position 9 is out of range");
    netlist_sim_close(sim);

    sim = netlist_sim_open("nets 1\npart 0 TE-GDS0204\nswitch 0 3 1\n", &error);
    expect(sim == NULL && error != NULL, "a switch line for a missing position is rejected");
    free(error);
    netlist_sim_close(sim);
}

int main(void) {
    check_catalog();
    check_contacts();
    check_netlist();

    if (failures != 0) {
        fprintf(stderr, "%d switch check(s) failed\n", failures);
        return 1;
    }
    printf("switch checks passed\n");
    return 0;
}
