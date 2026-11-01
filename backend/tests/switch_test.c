/*
 * switch_test: switch contacts on the part library (port of the old
 * switch_check.c check_contacts). Parts come from switch.json and
 * supply.json; instances use each part's kind model with the Part as config.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "net.h"
#include "parts.h"
#include "read_file.h"

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

static PartLibrary *library;

typedef struct {
    Sim *sim;
    SimInstance *sw;
    const Part *part;
    SimNet *nets[4];
} Bench;

static SimInstance *add_part(Sim *sim, const char *name, const Part **out) {
    const Part *part = part_library_find(library, name);

    if (out != NULL) {
        *out = part;
    }
    return part != NULL ? sim_add_instance(sim, kind_model(part), name, part) : NULL;
}

static int set_position(Sim *sim, SimInstance *instance, size_t position, int on) {
    const SimComponentType *type = sim_instance_type(instance);

    return type->set_switch(sim, instance, sim_instance_state(instance), position, on);
}

/* Whether a position (1-based) is ON: its contact pins "kA"/"kB" are joined. */
static int position_on(const Bench *bench, size_t position) {
    char a[8];
    char b[8];
    size_t pin_a = 0;
    size_t pin_b = 0;
    size_t i;

    snprintf(a, sizeof a, "%zuA", position);
    snprintf(b, sizeof b, "%zuB", position);
    for (i = 0; i < bench->part->pin_count; i++) {
        if (strcmp(bench->part->pins[i].name, a) == 0) {
            pin_a = i;
        } else if (strcmp(bench->part->pins[i].name, b) == 0) {
            pin_b = i;
        }
    }
    return sim_contact_closed(bench->sim, bench->sw, pin_a, pin_b);
}

/*
 * One 4-position switch, TE-GDS0404, on nets[0..3]: position 1 joins nets[0]
 * (pin 1) and nets[1] (pin 8); position 2 joins nets[1] (pin 2) and nets[2]
 * (pin 7); position 3 joins nets[2] (pin 3) and nets[3] (pin 6).
 */
static int bench_open(Bench *bench) {
    size_t i;

    memset(bench, 0, sizeof(*bench));
    bench->sim = sim_create();
    for (i = 0; i < 4; i++) {
        char name[8];

        snprintf(name, sizeof(name), "N%zu", i);
        bench->nets[i] = sim_add_net(bench->sim, name);
    }
    bench->sw = add_part(bench->sim, "TE-GDS0404", &bench->part);
    return bench->sw != NULL &&
           sim_connect(bench->sim, bench->nets[0], bench->sw, 0) &&
           sim_connect(bench->sim, bench->nets[1], bench->sw, 7) &&
           sim_connect(bench->sim, bench->nets[1], bench->sw, 1) &&
           sim_connect(bench->sim, bench->nets[2], bench->sw, 6) &&
           sim_connect(bench->sim, bench->nets[2], bench->sw, 2) &&
           sim_connect(bench->sim, bench->nets[3], bench->sw, 5);
}

static SimInstance *add_supply(Sim *sim, const char *name, SimNet *net) {
    SimInstance *instance = add_part(sim, name, NULL);

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
    if (!bench_open(&bench) || add_supply(bench.sim, "5V", bench.nets[0]) == NULL ||
        !sim_start(bench.sim) || !sim_run_until(bench.sim, 1000)) {
        expect(0, "switch bench setup");
    } else {
        expect(net_value(&bench, 0) == LOGIC_1, "VCC drives its net");
        expect(net_value(&bench, 1) == LOGIC_Z, "an OFF position isolates");

        expect(set_position(bench.sim, bench.sw, 1, 1), "position 1 turns ON");
        expect(sim_run_until(bench.sim, 2000), "run after turning ON");
        expect(net_value(&bench, 1) == LOGIC_1, "an ON position passes a 1");
        expect(position_on(&bench, 1), "position 1 reads ON");
        expect(net_value(&bench, 2) == LOGIC_Z, "position 2 is still OFF");

        /* Chain: positions 1 and 2 in series. */
        set_position(bench.sim, bench.sw, 2, 1);
        sim_run_until(bench.sim, 3000);
        expect(net_value(&bench, 2) == LOGIC_1, "two ON positions in series pass a 1");

        set_position(bench.sim, bench.sw, 1, 0);
        sim_run_until(bench.sim, 4000);
        expect(net_value(&bench, 1) == LOGIC_Z && net_value(&bench, 2) == LOGIC_Z,
               "turning OFF the first position lets the chain float, without latching");
        expect(!position_on(&bench, 1), "position 1 reads OFF");
        expect(!set_position(bench.sim, bench.sw, 5, 1), "position 5 is out of range");
        expect(!set_position(bench.sim, bench.sw, 0, 1), "position 0 is out of range");
    }
    sim_destroy(bench.sim);

    /* GND on the far side (net 1) pulls net 0 low: values pass both ways. */
    if (!bench_open(&bench) || add_supply(bench.sim, "GND", bench.nets[1]) == NULL ||
        !set_position(bench.sim, bench.sw, 1, 1) ||
        !sim_start(bench.sim) || !sim_run_until(bench.sim, 1000)) {
        expect(0, "reverse bench setup");
    } else {
        expect(net_value(&bench, 0) == LOGIC_0, "an ON position passes a 0 the other way");
        expect(net_value(&bench, 3) == LOGIC_Z, "an untouched net floats");
    }
    sim_destroy(bench.sim);

    /* VCC on net 0 and GND on net 2, joined by turning ON positions 1 and 2. */
    if (!bench_open(&bench) || add_supply(bench.sim, "5V", bench.nets[0]) == NULL ||
        add_supply(bench.sim, "GND", bench.nets[2]) == NULL ||
        !sim_start(bench.sim) || !sim_run_until(bench.sim, 1000)) {
        expect(0, "short bench setup");
    } else {
        set_position(bench.sim, bench.sw, 1, 1);
        expect(sim_run_until(bench.sim, 2000), "one ON position does not short");
        expect(net_value(&bench, 1) == LOGIC_1, "net 1 follows VCC");
        set_position(bench.sim, bench.sw, 2, 1);
        expect(!sim_run_until(bench.sim, 3000), "VCC to GND through a switch stops the run");
        expect(sim_error(bench.sim) != NULL && strncmp(sim_error(bench.sim), "smoke:", 6) == 0,
               "a short through a switch is smoke");
    }
    sim_destroy(bench.sim);

    /* A loop: positions 1-3 on, and position 4 joins net 3 back to net 0. */
    if (!bench_open(&bench)) {
        expect(0, "loop bench setup");
    } else {
        SimInstance *loop = add_part(bench.sim, "TE-GDS0204", NULL);

        if (loop == NULL || !sim_connect(bench.sim, bench.nets[3], loop, 0) ||
            !sim_connect(bench.sim, bench.nets[0], loop, 3) ||
            add_supply(bench.sim, "5V", bench.nets[3]) == NULL) {
            expect(0, "loop bench wiring");
        } else {
            set_position(bench.sim, bench.sw, 1, 1);
            set_position(bench.sim, bench.sw, 2, 1);
            set_position(bench.sim, loop, 1, 1);
            expect(sim_start(bench.sim) && sim_run_until(bench.sim, 1000), "loop runs");
            expect(net_value(&bench, 0) == LOGIC_1 && net_value(&bench, 1) == LOGIC_1 &&
                       net_value(&bench, 2) == LOGIC_1,
                   "VCC reaches every net around a loop of switches");
            expect(net_value(&bench, 3) == LOGIC_1, "the driven net stays high");
        }
    }
    sim_destroy(bench.sim);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 1;
    }
    check_contacts();
    part_library_destroy(library);
    if (failures > 0) {
        printf("switch_test: %d failures\n", failures);
        return 1;
    }
    printf("switch_test: all passed\n");
    return 0;
}
