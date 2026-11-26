#ifndef AVR_RIG_H
#define AVR_RIG_H

/*
 * A test rig for a simulated AVR (plan-62-E, F), shared by mcu_test,
 * avr_serial_test and avr_board_test: an mcu part (a fixture of
 * mcu_fixture.h or a shipped ATmega, any pinout) with 5V on every power pin,
 * GND on every ground pin and a net of its own on every other pin; firmware
 * from tests/avr/<name>.hex. A pin's net is found by its name
 * (rig_net(&r, "PB5")) or, for a numbered pinout, by its number
 * (r.net[19]; the P_ names below are the p28 pinout's).
 *
 * The including test defines `static PartLibrary *library;` and
 * `static int failures;` first, and AVR_DIR comes from CMake.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "cpu_state.h"
#include "memory_image.h"
#include "parts.h"
#include "read_file.h"

#define US 1000000ull            /* picoseconds */
#define MS 1000000000ull

enum { RIG_PINS = 128 };

typedef struct {
    Circuit *c;
    const Part *part;
    size_t avr, vcc, gnd;
    size_t n_vcc, n_gnd;
    size_t pin_net[RIG_PINS];    /* by index into part->pins */
    size_t net[RIG_PINS];        /* by pin number, for a pinout numbered 1..N */
} Rig;

/* The net of the pin with this name ("PB5", "ADC6"); the ground net when
   the part has no such pin (and a failure). */
static size_t rig_net(const Rig *r, const char *name) {
    size_t i;

    for (i = 0; r->part != NULL && i < r->part->pin_count && i < RIG_PINS; i++) {
        if (r->part->pins[i].name != NULL && strcmp(r->part->pins[i].name, name) == 0) {
            return r->pin_net[i];
        }
    }
    printf("FAIL: the part has no pin %s\n", name);
    failures++;
    return r->n_gnd;
}

/* p28 pin numbers. */
enum { P_PC6 = 1, P_PD0 = 2, P_PD1 = 3, P_PD2 = 4, P_PD3 = 5, P_PD4 = 6, P_PB6 = 9, P_PB7 = 10,
       P_PD5 = 11, P_PD6 = 12, P_PD7 = 13, P_PB0 = 14, P_PB1 = 15, P_PB2 = 16, P_PB3 = 17,
       P_PB4 = 18, P_PB5 = 19, P_PC0 = 23, P_PC5 = 28 };

static int rig_load(Rig *r, const char *firmware) {
    char path[1024];
    char *text;
    uint32_t words = 0;
    unsigned bits = 0;
    uint32_t *image;
    char *error = NULL;
    int result;

    snprintf(path, sizeof path, "%s/%s.hex", AVR_DIR, firmware);
    text = read_file(path);
    if (text == NULL || !circuit_memory_size(r->c, r->avr, &words, &bits)) {
        printf("FAIL: cannot read %s\n", path);
        free(text);
        return -1;
    }
    image = malloc(words * sizeof *image);
    result = memory_image_parse((const unsigned char *)text, strlen(text), MEMORY_IMAGE_INTEL_HEX,
                                words, bits, image, &error);
    if (result == 0) {
        result = circuit_memory_load(r->c, r->avr, 0, image, words);
    } else {
        printf("FAIL: %s: %s\n", path, error);
    }
    free(error);
    free(image);
    free(text);
    return result;
}

/* Builds the rig; firmware NULL leaves the flash erased. */
static void rig_new(Rig *r, const char *part, const char *firmware) {
    size_t i;

    memset(r, 0, sizeof *r);
    r->part = part_library_find(library, part);
    r->c = circuit_create(library);
    circuit_add_part(r->c, "5V", &r->vcc);
    circuit_add_part(r->c, "GND", &r->gnd);
    if (circuit_add_part(r->c, part, &r->avr) != 0) {
        printf("FAIL: %s: %s\n", part, circuit_error(r->c));
        failures++;
        return;
    }
    circuit_add_net(r->c, &r->n_vcc);
    circuit_add_net(r->c, &r->n_gnd);
    circuit_connect(r->c, r->vcc, "1", r->n_vcc);
    circuit_connect(r->c, r->gnd, "1", r->n_gnd);
    for (i = 0; r->part != NULL && i < r->part->pin_count && i < RIG_PINS; i++) {
        const PartPin *pin = &r->part->pins[i];
        long number = strtol(pin->designator, NULL, 10);

        if (pin->role == PIN_POWER) {
            r->pin_net[i] = r->n_vcc;
        } else if (pin->role == PIN_GROUND) {
            r->pin_net[i] = r->n_gnd;
        } else if (pin->role == PIN_NC || pin->role == PIN_ABSENT) {
            continue;
        } else {
            circuit_add_net(r->c, &r->pin_net[i]);
        }
        circuit_connect(r->c, r->avr, pin->designator, r->pin_net[i]);
        if (number > 0 && number < RIG_PINS) {
            r->net[number] = r->pin_net[i];
        }
    }
    if (firmware != NULL && rig_load(r, firmware) != 0) {
        failures++;
    }
}

static int rig_start(Rig *r) {
    if (circuit_start(r->c) != 0) {
        printf("FAIL: start: %s\n", circuit_error(r->c));
        failures++;
        return -1;
    }
    return 0;
}

/* Adds a part with two pins "1" and "2" between two nets. */
static size_t rig_two_pin(Rig *r, const char *part, size_t net1, size_t net2) {
    size_t index = 0;

    if (circuit_add_part(r->c, part, &index) != 0) {
        printf("FAIL: %s: %s\n", part, circuit_error(r->c));
        failures++;
    }
    circuit_connect(r->c, index, "1", net1);
    circuit_connect(r->c, index, "2", net2);
    return index;
}

static void rig_to(Rig *r, uint64_t time_ps) {
    uint64_t now = circuit_time(r->c);

    if (time_ps > now) {
        circuit_advance(r->c, time_ps - now);
    }
}

/* Advances in steps until the net's value changes; the time it was first
   seen changed, or 0 after `limit` picoseconds. */
static uint64_t next_change(Rig *r, size_t net, uint64_t step_ps, uint64_t limit_ps) {
    CircuitValue before = circuit_net_value(r->c, net);
    uint64_t end = circuit_time(r->c) + limit_ps;

    while (circuit_time(r->c) < end) {
        if (circuit_advance(r->c, step_ps) == 0) {
            printf("FAIL: the run stopped: %s\n", circuit_error(r->c));
            failures++;
            return 0;
        }
        if (circuit_net_value(r->c, net) != before) {
            return circuit_time(r->c);
        }
    }
    return 0;
}

static uint64_t instructions_run(Rig *r) {
    CpuState state;

    return circuit_cpu_state(r->c, r->avr, &state) == 0 ? state.cycles : 0;
}

/*
 * The average gap between changes of a net that a timer interrupt toggles:
 * half of the time from one change to the change after the next, measured
 * twice (it must be the same both times). The first change seen is skipped
 * (it may be the pin turning from Z to 0). `step` must divide the gap.
 *
 * Two changes are measured, not one, because a single gap is off by one CPU
 * cycle either way: the interrupt is taken between instructions, and avr8js
 * brings its timers up to date between instructions too. In avr8js itself
 * blink's PB5 changes at cycles 1021, 2022, 3021, 4022 (gaps 1001, 999,
 * 1001; `oracle.js run`), and the C core equals it. A whole period is exact.
 * Each gap must be within `cycle_ps` of the average.
 */
static uint64_t toggle_period(Rig *r, size_t net, uint64_t step_ps, uint64_t limit_ps,
                              uint64_t cycle_ps) {
    uint64_t t[6];
    int i;

    for (i = 0; i < 6; i++) {
        t[i] = next_change(r, net, step_ps, limit_ps);
        if (t[i] == 0) {
            printf("     only %d changes\n", i);
            return 0;
        }
    }
    if (t[3] - t[1] != t[5] - t[3]) {
        printf("     changes at %llu %llu %llu %llu %llu ps\n", (unsigned long long)t[1],
               (unsigned long long)t[2], (unsigned long long)t[3], (unsigned long long)t[4],
               (unsigned long long)t[5]);
        return 0;
    }
    for (i = 2; i < 6; i++) {
        uint64_t gap = t[i] - t[i - 1], average = (t[3] - t[1]) / 2;

        if (gap + cycle_ps < average || gap > average + cycle_ps) {
            printf("     a gap of %llu ps, the average is %llu\n", (unsigned long long)gap,
                   (unsigned long long)average);
            return 0;
        }
    }
    return (t[3] - t[1]) / 2;
}

#endif
