/*
 * sim_bench: how fast the engine runs a busy circuit next to idle chips.
 *
 *     sim_bench <oscillator part> <stages> <sim ms> <idle chips>
 *
 * Builds VCC, GND, the oscillator, a ripple counter of <stages> D flip-flops
 * (SN74HC74N units, each Q lighting a GENERIC-LED) clocked by the
 * oscillator, plus <idle chips> SN74HC00N with every gate pin on its own net.
 * Advances in 10 us steps to <sim ms> and prints the speed against real time.
 *
 * Not a ctest (a wall-clock check would be flaky). Build it Release:
 *     cmake -S backend -B /tmp/logic-rel -DCMAKE_BUILD_TYPE=Release
 *     cmake --build /tmp/logic-rel --target sim_bench
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

static Circuit *circuit;

static void check(int result) {
    if (result != 0) {
        fprintf(stderr, "sim_bench: %s\n", circuit_error(circuit));
        exit(1);
    }
}

static size_t add_part(const char *name) {
    size_t index = 0;

    check(circuit_add_part(circuit, name, &index));
    return index;
}

static size_t add_net(void) {
    size_t index = 0;

    check(circuit_add_net(circuit, &index));
    return index;
}

static void connect(size_t part, const char *pin, size_t net) {
    check(circuit_connect(circuit, part, pin, net));
}

static double wall_seconds(void) {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    static const char *files[] = {
        "footprints.json", "supply.json", "led.json",
        "oscillator.json", "dff.json", "gate.json"
    };
    /* SN74HC74N units: CLR, D, CLK, PRE, Q, QBAR. */
    static const char *unit_pins[2][6] = {
        { "1", "2", "3", "4", "5", "6" },
        { "13", "12", "11", "10", "9", "8" }
    };
    static const char *gate_pins[12] = {
        "1", "2", "3", "4", "5", "6", "8", "9", "10", "11", "12", "13"
    };
    const char *oscillator;
    int stages;
    double sim_ms;
    int idle;
    PartLibrary *library;
    size_t vcc_net, gnd_net, clock, osc, part_index;
    uint64_t step = 10000000ULL;
    uint64_t total;
    uint64_t done = 0;
    double start;
    double wall;
    int i;

    if (argc != 5) {
        fprintf(stderr, "usage: sim_bench <oscillator part> <stages> <sim ms> <idle chips>\n");
        return 2;
    }
    oscillator = argv[1];
    stages = atoi(argv[2]);
    sim_ms = atof(argv[3]);
    idle = atoi(argv[4]);

    library = part_library_create();
    if (load_datasheets_file(library, PARTS_DIR) != 0) {
        return 1;
    }
    for (i = 0; i < 6; i++) {
        char path[1024];
        char *text;
        char *error = NULL;
        int result;

        snprintf(path, sizeof(path), "%s/%s", PARTS_DIR, files[i]);
        text = read_file(path);
        if (text == NULL) {
            fprintf(stderr, "sim_bench: cannot read %s\n", path);
            return 1;
        }
        result = i == 0 ? load_footprints(library, text, files[i], &error)
                        : load_parts(library, text, files[i], &error);
        free(text);
        if (result != 0) {
            fprintf(stderr, "sim_bench: %s\n", error != NULL ? error : files[i]);
            return 1;
        }
    }

    circuit = circuit_create(library);
    if (circuit == NULL) {
        fprintf(stderr, "sim_bench: out of memory\n");
        return 1;
    }

    vcc_net = add_net();
    gnd_net = add_net();
    clock = add_net();
    /* 3.3 V: inside the 74HC parts' 2-6 V and a 2.5-3.3 V-option LMK6's 2.37-3.5 V
       (circuit_start refuses a supply outside a part's range). */
    connect(add_part("3V3"), "1", vcc_net);
    connect(add_part("GND"), "1", gnd_net);
    osc = add_part(oscillator);
    connect(osc, "4", vcc_net);
    connect(osc, "2", gnd_net);
    connect(osc, "3", clock);

    for (i = 0; i < stages; i += 2) {
        size_t ff = add_part("SN74HC74N");
        int unit;

        connect(ff, "14", vcc_net);
        connect(ff, "7", gnd_net);
        for (unit = 0; unit < 2 && i + unit < stages; unit++) {
            const char **pins = unit_pins[unit];
            size_t q = add_net();
            size_t qbar = add_net();
            size_t led;

            connect(ff, pins[0], vcc_net);  /* CLR */
            connect(ff, pins[3], vcc_net);  /* PRE */
            connect(ff, pins[2], clock);    /* CLK */
            connect(ff, pins[1], qbar);     /* D = QBAR */
            connect(ff, pins[4], q);
            connect(ff, pins[5], qbar);
            led = add_part("GENERIC-LED");
            connect(led, "2", q);
            connect(led, "1", gnd_net);
            clock = qbar;
        }
    }

    for (i = 0; i < idle; i++) {
        int k;

        part_index = add_part("SN74HC00N");
        connect(part_index, "14", vcc_net);
        connect(part_index, "7", gnd_net);
        for (k = 0; k < 12; k++) {
            connect(part_index, gate_pins[k], add_net());
        }
    }

    check(circuit_start(circuit));

    total = (uint64_t)(sim_ms * 1e9);
    start = wall_seconds();
    while (done < total) {
        if (!circuit_advance(circuit, step)) {
            fprintf(stderr, "sim_bench: %s\n", circuit_error(circuit));
            return 1;
        }
        done += step;
    }
    wall = wall_seconds() - start;

    printf("osc=%s stages=%d extra_chips=%d sim=%.3fms wall=%.3fs  => %.4gx real time\n",
           oscillator, stages, idle, sim_ms, wall, (sim_ms / 1000.0) / wall);

    circuit_destroy(circuit);
    part_library_destroy(library);
    return 0;
}
