/*
 * mcu_test (plan-36-F): the mcu kind. The fixture device file
 * (support/mcu_fixture.h) loads next to the real part files; the PartMcu
 * facts read back, with the part's clock over the device's; the listing
 * line ends "|unsimulated"; and a circuit refuses the part: an mcu with no
 * "model" has no simulation model.
 *
 * plan-62-E: a device with a "model" runs. The fixtures TESTAVR and
 * TESTAVR48 (model atmega328) run firmware from tests/avr/<name>.hex in
 * circuits of library parts: startup, the two clocks (times in
 * picoseconds), reset, GPIO, PWM, an external interrupt, the ADC, flash
 * through the memory calls and circuit_cpu_state.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avr.h"
#include "circuit.h"
#include "cpu_state.h"
#include "listing.h"
#include "mcu_fixture.h"
#include "memory_image.h"
#include "parts.h"
#include "read_file.h"

static int failures;
static PartLibrary *library;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static int ends_with(const char *text, const char *tail) {
    size_t n = strlen(text), m = strlen(tail);

    return n >= m && strcmp(text + n - m, tail) == 0;
}

static void fact_cases(void) {
    const Part *m = part_library_find(library, "TESTMCU");

    expect(m != NULL && m->kind == PART_MCU && m->mcu != NULL && m->pin_count == 8,
           "TESTMCU loads as an mcu with 8 pins");
    expect(m != NULL && m->mcu != NULL && strcmp(m->mcu->core, "avr") == 0 &&
               m->mcu->max_hz == 10000000ULL && m->mcu->flash == 32768 && m->mcu->ram == 2048 &&
               m->mcu->eeprom == 1024 && m->mcu->psram == 0,
           "TESTMCU: core avr, clock 10000000 (the part's, over the device's 20 MHz), flash 32768, "
           "ram 2048, eeprom 1024, no psram");
    expect(m != NULL && m->vcc != NULL && m->vcc->min_mv == 1800 && m->vcc->max_mv == 5500,
           "TESTMCU: vcc 1.8-5.5 V");
    expect(m != NULL && m->simulated == 0, "TESTMCU: simulated 0");
}

static void listing_cases(void) {
    const Part *m = part_library_find(library, "TESTMCU");
    char *line = m != NULL ? listing_line(m) : NULL;

    expect(line != NULL && strstr(line, "|mcu|") != NULL &&
               strstr(line, "core=avr clock=10000000 flash=32768 ram=2048 eeprom=1024") != NULL &&
               ends_with(line, "|unsimulated"),
           "the listing line names the facts and ends |unsimulated");
    if (line != NULL) {
        printf("     %s\n", line);
    }
    free(line);
}

static void refusal_cases(void) {
    Circuit *c = circuit_create(library);
    size_t index = 0;

    expect(circuit_add_part(c, "TESTMCU", &index) == -1 &&
               strcmp(circuit_error(c), "part \"TESTMCU\" is not simulated") == 0,
           "circuit_add_part refuses TESTMCU: part \"TESTMCU\" is not simulated");
    printf("     %s\n", circuit_error(c));
    expect(circuit_add_part(c, "5V", &index) == 0, "the circuit still takes a simulated part");
    circuit_destroy(c);
}

/* ---- plan-62-E: a model device on the engine -------------------------------- */

#define US 1000000ull            /* picoseconds */
#define MS 1000000000ull

/* A TESTAVR (pinout p28) with 5V on VCC and AVCC, GND on both grounds, and a
   net of its own on every other pin. */
typedef struct {
    Circuit *c;
    size_t avr, vcc, gnd;
    size_t n_vcc, n_gnd;
    size_t net[29];              /* by pin number 1..28 (the supply pins: n_vcc / n_gnd) */
} Rig;

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
    char pin[8];
    int n;

    memset(r, 0, sizeof *r);
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
    for (n = 1; n <= 28; n++) {
        snprintf(pin, sizeof pin, "%d", n);
        if (n == 7 || n == 20) {
            r->net[n] = r->n_vcc;
        } else if (n == 8 || n == 22) {
            r->net[n] = r->n_gnd;
        } else {
            circuit_add_net(r->c, &r->net[n]);
        }
        circuit_connect(r->c, r->avr, pin, r->net[n]);
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

static void avr_fact_cases(void) {
    const Part *a = part_library_find(library, "TESTAVR");
    const Part *m = part_library_find(library, "TESTMCU");
    char *line = a != NULL ? listing_line(a) : NULL;

    expect(a != NULL && a->simulated == 1 && a->mcu != NULL && a->mcu->model != NULL &&
               strcmp(a->mcu->model, "atmega328") == 0 && a->pin_count == 28,
           "TESTAVR: simulated, model atmega328, 28 pins");
    expect(m != NULL && m->mcu->model == NULL, "TESTMCU: no model");
    expect(line != NULL && strstr(line, " model=atmega328") != NULL &&
               strstr(line, "|unsimulated") == NULL,
           "TESTAVR's listing line names the model and has no |unsimulated");
    free(line);
}

static void avr_startup_and_blink(void) {
    Rig r;
    size_t led;
    uint64_t on[1] = {0}, on_before, first, period;
    int n, quiet = 1;

    rig_new(&r, "TESTAVR", "blink");
    led = rig_two_pin(&r, "GENERIC-LED", r.n_gnd, r.net[P_PB5]);   /* pin 1 K, pin 2 A */
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS / 2);
    for (n = 1; n <= 28; n++) {
        if (n != 7 && n != 8 && n != 20 && n != 22 && n != P_PC6) {
            quiet &= circuit_net_value(r.c, r.net[n]) == CIRCUIT_Z;
        }
    }
    expect(quiet, "during startup (0.5 ms) every port pin is Z");
    expect(circuit_net_value(r.c, r.net[P_PC6]) == CIRCUIT_1, "the reset pin PC6 reads 1 (its pull-up)");
    rig_to(&r, MS);
    first = next_change(&r, r.net[P_PB5], US, 10 * MS);
    expect(first > MS && first < 3 * MS, "blink: PB5 first changes after the 1 ms startup");
    period = toggle_period(&r, r.net[P_PB5], US, 10 * MS, US);
    expect(period == 1000 * US,
           "blink on the internal clock: PB5 changes every 1000000000 ps on average (1000 cycles of 1 us)");
    expect(instructions_run(&r) > 0, "the chip ran instructions");
    expect(circuit_net_value(r.c, r.net[P_PB6]) == CIRCUIT_Z, "PB6 is a port pin on the internal clock (an input: Z)");
    /* The LED is lit half of the time. */
    circuit_part_light_time(r.c, led, on, 1);
    on_before = on[0];
    circuit_advance(r.c, 100 * MS);
    circuit_part_light_time(r.c, led, on, 1);
    expect(on[0] - on_before >= 49 * MS && on[0] - on_before <= 51 * MS,
           "an LED on PB5 is lit 50 ms of 100 ms");
    circuit_destroy(r.c);
}

static void avr_crystal_clock(void) {
    Rig r;
    size_t xtal;
    uint64_t period, changes;

    rig_new(&r, "TESTAVR", "blink");
    xtal = rig_two_pin(&r, "GENERIC-CRYSTAL-16M", r.net[P_PB6], r.net[P_PB7]);
    (void)xtal;
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    period = toggle_period(&r, r.net[P_PB5], 62500, 2 * MS, 62500);
    expect(period == 62500000ull,
           "blink on a 16 MHz crystal: PB5 changes every 62500000 ps on average (1000 cycles of 62.5 ns)");
    changes = circuit_net_changes(r.c, r.net[P_PB6]);
    circuit_advance(r.c, 10 * US);
    expect(circuit_net_changes(r.c, r.net[P_PB6]) - changes == 320 && circuit_error(r.c) == NULL,
           "the crystal keeps clocking PB6 (320 changes in 10 us): the chip does not drive it");
    circuit_destroy(r.c);
}

static void avr_clkpr(void) {
    Rig r;

    rig_new(&r, "TESTAVR", "blink_fast");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    expect(toggle_period(&r, r.net[P_PB5], 125000, 2 * MS, 125000) == 125000000ull,
           "after CLKPR = 0 the internal clock is 8 MHz: PB5 changes every 125000000 ps on average");
    circuit_destroy(r.c);
}

static void avr_button(void) {
    Rig r;
    size_t sw;

    rig_new(&r, "TESTAVR", "button");
    sw = rig_two_pin(&r, "AS11AH", r.net[P_PD2], r.n_gnd);   /* SPST, open at rest */
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 2 * MS);
    expect(circuit_net_value(r.c, r.net[P_PD2]) == CIRCUIT_1,
           "button: PD2's net reads 1 with the switch open (the pull-up, a weak 1)");
    expect(circuit_net_value(r.c, r.net[P_PB0]) == CIRCUIT_1, "button: PB0 is 1");
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, 100 * US);
    expect(circuit_net_value(r.c, r.net[P_PD2]) == CIRCUIT_0 &&
               circuit_net_value(r.c, r.net[P_PB0]) == CIRCUIT_0 && circuit_error(r.c) == NULL,
           "button: the switch closed to GND reads 0 with no short, and PB0 follows to 0");
    circuit_set_switch(r.c, sw, 1, 0);
    circuit_advance(r.c, 100 * US);
    expect(circuit_net_value(r.c, r.net[P_PB0]) == CIRCUIT_1, "button: open again, PB0 is 1");
    circuit_destroy(r.c);
}

static void avr_pwm(void) {
    Rig r;
    uint64_t t[5];
    int i, ok = 1;

    rig_new(&r, "TESTAVR", "pwm");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 2 * MS);
    /* Start on a rising edge. */
    do {
        t[0] = next_change(&r, r.net[P_PD6], US, 10 * MS);
    } while (t[0] != 0 && circuit_net_value(r.c, r.net[P_PD6]) != CIRCUIT_1);
    for (i = 1; i < 5; i++) {
        t[i] = next_change(&r, r.net[P_PD6], US, 10 * MS);
        ok &= t[i] != 0;
    }
    expect(ok && t[1] - t[0] == 64 * US && t[2] - t[1] == 192 * US && t[3] - t[2] == 64 * US &&
               t[4] - t[3] == 192 * US,
           "pwm: OC0A (PD6) is high 64 and low 192 of every 256 cycles");
    if (!(ok && t[1] - t[0] == 64 * US)) {
        printf("     high %llu us, low %llu us\n", (unsigned long long)((t[1] - t[0]) / US),
               (unsigned long long)((t[2] - t[1]) / US));
    }
    circuit_destroy(r.c);
}

static void avr_int0(void) {
    Rig r;
    size_t sw;

    rig_new(&r, "TESTAVR", "int0");
    sw = rig_two_pin(&r, "AS11AH", r.net[P_PD2], r.n_gnd);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 2 * MS);
    expect(circuit_net_value(r.c, r.net[P_PB1]) == CIRCUIT_0, "int0: PB1 starts at 0");
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, 100 * US);
    expect(circuit_net_value(r.c, r.net[P_PB1]) == CIRCUIT_1, "int0: a falling edge on PD2 toggles PB1 to 1");
    circuit_set_switch(r.c, sw, 1, 0);
    circuit_advance(r.c, 100 * US);
    expect(circuit_net_value(r.c, r.net[P_PB1]) == CIRCUIT_1, "int0: the rising edge does nothing");
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, 100 * US);
    expect(circuit_net_value(r.c, r.net[P_PB1]) == CIRCUIT_0, "int0: the next falling edge toggles it back");
    circuit_destroy(r.c);
}

static void avr_reset_pin(void) {
    Rig r;
    size_t sw;
    uint64_t first, released, again;
    CpuState state;

    rig_new(&r, "TESTAVR", "blink");
    sw = rig_two_pin(&r, "AS11AH", r.net[P_PC6], r.n_gnd);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    first = next_change(&r, r.net[P_PB5], US, 10 * MS);
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, 10 * US);
    expect(circuit_net_value(r.c, r.net[P_PC6]) == CIRCUIT_0 &&
               circuit_net_value(r.c, r.net[P_PB5]) == CIRCUIT_Z,
           "reset: PC6 pulled to GND releases every port pin (PB5 is Z)");
    expect(circuit_cpu_state(r.c, r.avr, &state) == 0 && state.status == CPU_RESET &&
               state.register_count == 0,
           "reset: circuit_cpu_state says CPU_RESET with no registers");
    circuit_advance(r.c, 5 * MS);
    expect(circuit_net_value(r.c, r.net[P_PB5]) == CIRCUIT_Z, "reset: nothing runs while it is held");
    circuit_set_switch(r.c, sw, 1, 0);
    released = circuit_time(r.c);
    again = next_change(&r, r.net[P_PB5], US, 10 * MS);   /* Z to 0: DDRB5 set */
    again = next_change(&r, r.net[P_PB5], US, 10 * MS);   /* the first toggle */
    expect(first != 0 && again != 0 && again - released < 3 * MS,
           "reset: released, the program starts over and PB5 toggles again");
    circuit_destroy(r.c);
}

static void avr_small_chip(void) {
    Rig r;

    rig_new(&r, "TESTAVR48", "blink48");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    expect(toggle_period(&r, r.net[P_PB5], US, 10 * MS, US) == 1000 * US,
           "TESTAVR48 (4 KB of flash, one-word vectors) runs blink48: PB5 changes every 1000 us on average");
    circuit_destroy(r.c);
}

static void avr_memory(void) {
    Rig r;
    uint32_t words = 0, values[4] = {0}, known[4] = {0}, two[2] = {1, 2};
    unsigned bits = 0;
    size_t part5v = 0;

    rig_new(&r, "TESTAVR", NULL);
    expect(circuit_memory_size(r.c, r.avr, &words, &bits) == 1 && words == 32768 && bits == 8,
           "the flash is the part's memory: 32768 words of 8 bits");
    expect(circuit_memory_read(r.c, r.avr, 0, 4, values, known) == 4 && values[0] == 0xff &&
               values[3] == 0xff && known[0] == 0xff,
           "an unloaded flash reads FF");
    expect(circuit_memory_load(r.c, r.avr, 32767, two, 2) == -1 && circuit_error(r.c) != NULL &&
               strstr(circuit_error(r.c), "past the end") != NULL,
           "a load past the end of the flash is refused");
    expect(rig_load(&r, "blink") == 0 &&
               circuit_memory_read(r.c, r.avr, 0, 4, values, known) == 4 && values[0] == 0x0c &&
               values[1] == 0x94 && values[2] == 0x18 && values[3] == 0x00,
           "blink.hex loads: the flash starts 0C 94 18 00 (JMP 0x0030)");
    expect(circuit_memory_size(r.c, r.vcc, &words, &bits) == 0, "a supply has no memory");
    (void)part5v;
    circuit_destroy(r.c);

    /* An erased chip runs (it executes nothing but FF words) without an error. */
    rig_new(&r, "TESTAVR", NULL);
    if (rig_start(&r) == 0) {
        expect(circuit_advance(r.c, 10 * MS) != 0 && circuit_error(r.c) == NULL &&
                   instructions_run(&r) > 0,
               "a chip with no firmware runs 10 ms without an error");
    }
    circuit_destroy(r.c);
}

/* PORTB's eight pins as a number; -1 when one is not 0 or 1. */
static int port_b(Rig *r) {
    static const int PINS[8] = {P_PB0, P_PB1, P_PB2, P_PB3, P_PB4, P_PB5, P_PB6, P_PB7};
    int value = 0, bit;

    for (bit = 0; bit < 8; bit++) {
        CircuitValue v = circuit_net_value(r->c, r->net[PINS[bit]]);

        if (v != CIRCUIT_0 && v != CIRCUIT_1) {
            return -1;
        }
        value |= (v == CIRCUIT_1) << bit;
    }
    return value;
}

/* The ADC reads a pin's logic level: 1 is 5 V (1023 against AVCC), anything
   else 0 V. adc_port shows the high 8 bits of channel 0 on PORTB. */
static void avr_adc(void) {
    Rig r;
    size_t sw_high, sw_low;

    rig_new(&r, "TESTAVR", "adc_port");
    sw_high = rig_two_pin(&r, "AS11AH", r.net[P_PC0], r.n_vcc);
    sw_low = rig_two_pin(&r, "AS11AH", r.net[P_PC0], r.n_gnd);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 3 * MS);
    expect(port_b(&r) == 0x00, "adc: PC0 left open converts to 0");
    circuit_set_switch(r.c, sw_high, 1, 1);
    circuit_advance(r.c, MS);
    expect(port_b(&r) == 0xff, "adc: PC0 on 5 V converts to 1023 (PORTB shows FF)");
    circuit_set_switch(r.c, sw_high, 1, 0);
    circuit_set_switch(r.c, sw_low, 1, 1);
    circuit_advance(r.c, MS);
    expect(port_b(&r) == 0x00, "adc: PC0 on GND converts to 0");
    circuit_destroy(r.c);
}

static void avr_state(void) {
    Rig r;
    CpuState state;
    char text[32];
    size_t i;
    int named = 1;

    rig_new(&r, "TESTAVR", "blink");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS / 2);
    expect(circuit_cpu_state(r.c, r.avr, &state) == 0 && state.status == CPU_RESET &&
               state.register_count == 0 && !state.has_instruction,
           "state: during startup the chip is in reset, with no registers");
    rig_to(&r, 5 * MS);
    expect(circuit_cpu_state(r.c, r.avr, &state) == 0 && state.status == CPU_RUNNING &&
               state.register_count == 35 && state.cycles > 0,
           "state: running, with 35 registers and a cycle count");
    expect(strcmp(state.registers[0].name, "PC") == 0 && state.registers[0].bits == 16 &&
               state.registers[0].value % 2 == 0 && state.registers[0].value < 32768,
           "state: PC is a byte address (even, inside the flash)");
    expect(strcmp(state.registers[1].name, "SP") == 0 && state.registers[1].value >= 0x100 &&
               state.registers[1].value <= 0x8ff,
           "state: SP is inside the SRAM");
    expect(strcmp(state.registers[2].name, "SREG") == 0 && state.registers[2].bits == 8 &&
               (state.registers[2].value & 0x80) != 0,
           "state: SREG has I set (the program enabled interrupts)");
    for (i = 0; i < 32; i++) {
        char want[8];

        snprintf(want, sizeof want, "R%zu", i);
        named &= strcmp(state.registers[3 + i].name, want) == 0 && state.registers[3 + i].bits == 8;
    }
    expect(named && state.registers[3 + 16].value == 0x09,
           "state: R0 to R31 follow, and R16 holds the last LDI's 0x09");
    avr_dasm((uint16_t)(state.instruction_bytes[0] | (state.instruction_bytes[1] << 8)),
             (uint16_t)(state.instruction_bytes[2] | (state.instruction_bytes[3] << 8)),
             state.instruction_address, text, sizeof text);
    expect(state.has_instruction && state.instruction_length >= 2 &&
               strcmp(state.instruction_text, text) == 0 && state.instruction_address % 2 == 0,
           "state: the last instruction's text is the disassembly of its bytes");
    printf("     %04X  %s\n", (unsigned)state.instruction_address, state.instruction_text);
    expect(circuit_cpu_state(r.c, r.vcc, &state) == -1 &&
               strstr(circuit_error(r.c), "is not a processor") != NULL,
           "state: a part that is no processor is still refused");
    circuit_destroy(r.c);
}

static void avr_cases(void) {
    avr_fact_cases();
    avr_startup_and_blink();
    avr_crystal_clock();
    avr_clkpr();
    avr_button();
    avr_pwm();
    avr_int0();
    avr_reset_pin();
    avr_small_chip();
    avr_memory();
    avr_adc();
    avr_state();
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_mcu_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    listing_cases();
    refusal_cases();
    avr_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("mcu_test: all passed\n");
    return 0;
}
