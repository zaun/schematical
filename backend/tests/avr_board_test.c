/*
 * avr_board_test (plan-62-G): the shipped ATmega48/88/168/328 parts run.
 *
 *   An ATMEGA328P-PU with a GENERIC-CRYSTAL-16M on PB6/PB7 and a GENERIC-LED
 *   on PB5 blinks: the LED is lit half of the time (blink toggles PB5 every
 *   1000 cycles of 62.5 ns).
 *   An ATMEGA48PA-PU (4 KB of flash: one-word vectors) with no crystal runs
 *   blink48 on its internal clock: PB5 changes every 1000 us on average.
 *   An ATMEGA328PB-AU toggles PE0, a pin of port E.
 *   Every mcu part with a model, alone with its supply, loads blink (the
 *   build for its vector size), starts and runs 2 ms without an error:
 *   126 parts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

static PartLibrary *library;
static int failures;

#include "avr_rig.h"

static void check(int ok, const char *name) {
    printf("%s %s\n", ok ? "ok" : "FAIL:", name);
    if (!ok) {
        failures++;
    }
}

static void uno_case(void) {
    Rig r;
    size_t led;
    uint64_t on[1] = {0}, before;

    rig_new(&r, "ATMEGA328P-PU", "blink");
    rig_two_pin(&r, "GENERIC-CRYSTAL-16M", rig_net(&r, "PB6"), rig_net(&r, "PB7"));
    led = rig_two_pin(&r, "GENERIC-LED", r.n_gnd, rig_net(&r, "PB5"));   /* pin 1 K, pin 2 A */
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 2 * MS);
    circuit_part_light_time(r.c, led, on, 1);
    before = on[0];
    circuit_advance(r.c, 20 * MS);
    circuit_part_light_time(r.c, led, on, 1);
    check(on[0] - before >= 9900 * US && on[0] - before <= 10100 * US && circuit_error(r.c) == NULL,
          "ATMEGA328P-PU on a 16 MHz crystal blinks an LED on PB5 (lit 10 ms of 20 ms)");
    if (!(on[0] - before >= 9900 * US && on[0] - before <= 10100 * US)) {
        printf("     lit %llu us\n", (unsigned long long)((on[0] - before) / US));
    }
    rig_to(&r, 23 * MS);
    check(toggle_period(&r, rig_net(&r, "PB5"), 62500, 2 * MS, 62500) == 62500000ull,
          "its PB5 changes every 62500000 ps on average (1000 cycles of 62.5 ns)");
    circuit_destroy(r.c);
}

static void small_case(void) {
    Rig r;

    rig_new(&r, "ATMEGA48PA-PU", "blink48");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    check(toggle_period(&r, rig_net(&r, "PB5"), US, 10 * MS, US) == 1000 * US,
          "ATMEGA48PA-PU with no crystal runs blink48 on its internal clock (PB5 every 1000 us on average)");
    circuit_destroy(r.c);
}

static void pb_case(void) {
    Rig r;
    uint64_t changes;

    rig_new(&r, "ATMEGA328PB-AU", "pe0");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 2 * MS);
    changes = circuit_net_changes(r.c, rig_net(&r, "PE0"));
    circuit_advance(r.c, MS);
    /* sbi (2 cycles) and rjmp (2): a change every 4 us. */
    check(circuit_net_changes(r.c, rig_net(&r, "PE0")) - changes == 250,
          "ATMEGA328PB-AU toggles PE0 (250 changes in 1 ms)");
    circuit_destroy(r.c);
}

/* Every part with a model runs. */
static void every_part_case(void) {
    size_t count = part_library_count(library), i, ran = 0, bad = 0;

    for (i = 0; i < count; i++) {
        const Part *part = part_library_at(library, i);
        Rig r;
        CpuState state;

        if (part->kind != PART_MCU || part->mcu == NULL || part->mcu->model == NULL) {
            continue;
        }
        rig_new(&r, part->name, part->mcu->flash <= 8192 ? "blink48" : "blink");
        if (circuit_start(r.c) != 0 || circuit_advance(r.c, 2 * MS) == 0 || circuit_error(r.c) != NULL ||
            circuit_cpu_state(r.c, r.avr, &state) != 0 || state.status != CPU_RUNNING ||
            state.cycles == 0 || circuit_net_value(r.c, rig_net(&r, "PB5")) == CIRCUIT_Z) {
            printf("FAIL: %s: %s\n", part->name,
                   circuit_error(r.c) != NULL ? circuit_error(r.c) : "did not run");
            bad++;
        }
        ran++;
        circuit_destroy(r.c);
    }
    failures += (int)bad;
    check(ran == 126 && bad == 0, "126 parts run");
    if (ran != 126) {
        printf("     %zu parts have a model\n", ran);
    }
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    uno_case();
    small_case();
    pb_case();
    every_part_case();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("avr_board_test: all passed\n");
    return 0;
}
