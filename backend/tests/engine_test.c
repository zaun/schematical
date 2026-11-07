/*
 * engine_test: event-engine timing with a real gate model.
 *
 * An SN74HC32N (OR, 10 ns) gets its inputs from an engine-level source
 * (transport drives at set times); a probe records every value of 1Y. The
 * inertial rule must keep an output edge that is already pending when a
 * later input change asks for the same value again (plan-05-B), while a
 * pulse shorter than the delay is still swallowed.
 *
 * A one-input "buf" gate (plan-20-C) passes 0 and 1 through and turns an X
 * or a floating (Z) input into X.
 */

#include <stdio.h>
#include <stdlib.h>

#include "kind.h"
#include "net.h"
#include "parts.h"
#include "read_file.h"
#include "sim.h"

/* SN74HC32N pins, 0-based: 1A, 1B, 1Y, GND (pin 7), VCC (pin 14). */
enum { PIN_1A = 0, PIN_1B = 1, PIN_1Y = 2, PIN_GND = 6, PIN_VCC = 13 };

/* Source pins. */
enum { SRC_A, SRC_B, SRC_VCC, SRC_GND, SRC_PINS };

typedef struct {
    size_t pin;
    SimTime time;
    LogicValue value;
} Edge;

typedef struct {
    const Edge *edges;
    size_t count;
} Script;

#define MAX_SAMPLES 64

typedef struct {
    SimTime time[MAX_SAMPLES];
    LogicValue value[MAX_SAMPLES];
    size_t count;
} Probe;

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static size_t source_pin_count(const void *config) {
    (void)config;
    return SRC_PINS;
}

static void *pass_config(const void *config) {
    return (void *)config;
}

static void source_start(Sim *sim, SimInstance *instance, void *state) {
    const Script *script = state;
    size_t i;

    sim_schedule_drive(sim, instance, SRC_VCC, LOGIC_1, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, SRC_GND, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, SRC_A, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, SRC_B, LOGIC_0, DRIVE_STRONG, 0);
    for (i = 0; i < script->count; i++) {
        const Edge *edge = &script->edges[i];

        sim_schedule_drive(sim, instance, edge->pin, edge->value, DRIVE_STRONG, edge->time);
    }
}

static const SimComponentType SOURCE_TYPE = {
    "test source", source_pin_count, pass_config, NULL, source_start, NULL, NULL
};

static size_t probe_pin_count(const void *config) {
    (void)config;
    return 1;
}

static void probe_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    Probe *probe = state;

    (void)instance;
    (void)pin;
    (void)old_value;
    if (probe->count < MAX_SAMPLES) {
        probe->time[probe->count] = sim_now(sim);
        probe->value[probe->count] = new_value;
        probe->count++;
    }
}

static const SimComponentType PROBE_TYPE = {
    "probe", probe_pin_count, pass_config, NULL, NULL, probe_pin_changed, NULL
};

static const Part *or_part;
static const Part *buf_part;

/* The test buffer (SOT-23-5: NC, A, GND, Y, VCC), 4 ns each way. */
static const char BUF_JSON[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"gate\",\"devices\":["
    "{\"id\":\"TBUF\",\"function\":\"Buffer\",\"description\":\"test buffer\",\"drawer\":[],"
    "\"pinouts\":{\"p5\":[{\"number\":1,\"name\":\"NC\",\"role\":\"nc\"},"
    "{\"number\":2,\"name\":\"A\",\"role\":\"input\"},{\"number\":3,\"name\":\"GND\",\"role\":\"ground\"},"
    "{\"number\":4,\"name\":\"Y\",\"role\":\"output\"},{\"number\":5,\"name\":\"VCC\",\"role\":\"power\"}]},"
    "\"gates\":[{\"op\":\"buf\",\"inputs\":[\"A\"],\"output\":\"Y\"}],"
    "\"timing\":{\"LVC\":{\"rise\":4,\"fall\":4,\"vcc\":[1.65,5.5]}},"
    "\"parts\":[{\"name\":\"TEST-BUF\",\"package\":\"SOT-23\",\"pinout\":\"p5\","
    "\"footprint\":\"SOT23-5\",\"family\":\"LVC\"}]}]}";

/* Test buffer pins, 0-based. */
enum { BUF_A = 1, BUF_GND = 2, BUF_Y = 3, BUF_VCC = 4 };

/* Y of the test buffer at time t, with A driven by the script (A starts at 0). */
static LogicValue buf_y_at(const Edge *edges, size_t count, SimTime t) {
    Script script = { edges, count };
    Probe probe = { {0}, {0}, 0 };
    Sim *sim = sim_create();
    SimInstance *source = sim_add_instance(sim, &SOURCE_TYPE, "source", &script);
    SimInstance *buf = sim_add_instance(sim, kind_model(buf_part), "U1", buf_part);
    SimInstance *probe_instance = sim_add_instance(sim, &PROBE_TYPE, "probe", &probe);
    SimNet *a = sim_add_net(sim, "A");
    SimNet *y = sim_add_net(sim, "Y");
    SimNet *vcc = sim_add_net(sim, "VCC");
    SimNet *gnd = sim_add_net(sim, "GND");
    LogicValue v = LOGIC_Z;

    sim_connect(sim, a, source, SRC_A);
    sim_connect(sim, vcc, source, SRC_VCC);
    sim_connect(sim, gnd, source, SRC_GND);
    sim_connect(sim, a, buf, BUF_A);
    sim_connect(sim, y, buf, BUF_Y);
    sim_connect(sim, vcc, buf, BUF_VCC);
    sim_connect(sim, gnd, buf, BUF_GND);
    sim_connect(sim, y, probe_instance, 0);
    if (!sim_start(sim) || !sim_run_until(sim, t)) {
        printf("  engine error: %s\n", sim_error(sim));
        failures++;
    } else {
        v = sim_read_pin(sim, probe_instance, 0);
    }
    sim_destroy(sim);
    return v;
}

/*
 * Runs one script to 200 ns and returns the time 1Y first became 1, or 0
 * when it never did.
 */
static SimTime first_high(const Edge *edges, size_t count) {
    Script script = { edges, count };
    Probe probe = { {0}, {0}, 0 };
    Sim *sim = sim_create();
    SimInstance *source = sim_add_instance(sim, &SOURCE_TYPE, "source", &script);
    SimInstance *gate = sim_add_instance(sim, kind_model(or_part), "U1", or_part);
    SimInstance *probe_instance = sim_add_instance(sim, &PROBE_TYPE, "probe", &probe);
    SimNet *a = sim_add_net(sim, "A");
    SimNet *b = sim_add_net(sim, "B");
    SimNet *y = sim_add_net(sim, "Y");
    SimNet *vcc = sim_add_net(sim, "VCC");
    SimNet *gnd = sim_add_net(sim, "GND");
    SimTime result = 0;
    size_t i;

    sim_connect(sim, a, source, SRC_A);
    sim_connect(sim, b, source, SRC_B);
    sim_connect(sim, vcc, source, SRC_VCC);
    sim_connect(sim, gnd, source, SRC_GND);
    sim_connect(sim, a, gate, PIN_1A);
    sim_connect(sim, b, gate, PIN_1B);
    sim_connect(sim, y, gate, PIN_1Y);
    sim_connect(sim, vcc, gate, PIN_VCC);
    sim_connect(sim, gnd, gate, PIN_GND);
    sim_connect(sim, y, probe_instance, 0);

    if (!sim_start(sim) || !sim_run_until(sim, 200000)) {
        printf("  engine error: %s\n", sim_error(sim));
        failures++;
        sim_destroy(sim);
        return 0;
    }
    for (i = 0; i < probe.count; i++) {
        printf("  1Y = %d at %llu ps\n", (int)probe.value[i],
               (unsigned long long)probe.time[i]);
    }
    for (i = 0; i < probe.count; i++) {
        if (probe.value[i] == LOGIC_1) {
            result = probe.time[i];
            break;
        }
    }
    sim_destroy(sim);
    return result;
}

int main(void) {
    static const Edge both_rise[] = {
        { SRC_A, 100000, LOGIC_1 },
        { SRC_B, 105000, LOGIC_1 },
    };
    static const Edge short_pulse[] = {
        { SRC_A, 100000, LOGIC_1 },
        { SRC_A, 105000, LOGIC_0 },
    };
    static const Edge one_rise[] = {
        { SRC_A, 100000, LOGIC_1 },
    };
    PartLibrary *library = part_library_create();
    char *text = read_file(PARTS_DIR "/footprints.json");
    char *error = NULL;
    SimTime t;

    /* gate.json names footprints, so footprints.json loads first. */
    if (text == NULL || load_footprints(library, text, "footprints.json", &error) != 0) {
        fprintf(stderr, "engine_test: cannot load footprints.json: %s\n",
                error ? error : "read failed");
        return 1;
    }
    free(text);
    text = read_file(PARTS_DIR "/gate.json");
    if (text == NULL || load_parts(library, text, "gate.json", &error) != 0) {
        fprintf(stderr, "engine_test: cannot load gate.json: %s\n", error ? error : "read failed");
        return 1;
    }
    free(text);
    or_part = part_library_find(library, "SN74HC32N");
    if (or_part == NULL || kind_model(or_part) == NULL) {
        fprintf(stderr, "engine_test: SN74HC32N not found\n");
        return 1;
    }

    t = first_high(both_rise, 2);
    printf("  first 1Y high: %llu ps\n", (unsigned long long)t);
    expect(t == 110000, "A up at 100 ns, B up at 105 ns: 1Y rises at 110 ns, not 115 ns");

    t = first_high(short_pulse, 2);
    expect(t == 0, "a 5 ns pulse on A (shorter than the 10 ns delay) is swallowed");

    t = first_high(one_rise, 1);
    expect(t == 110000, "A up at 100 ns alone: 1Y rises at 110 ns");

    if (load_parts(library, BUF_JSON, "buf.json", &error) != 0 ||
        (buf_part = part_library_find(library, "TEST-BUF")) == NULL) {
        fprintf(stderr, "engine_test: cannot load the test buffer: %s\n", error ? error : "?");
        return 1;
    }
    {
        static const Edge up[] = { { SRC_A, 100000, LOGIC_1 } };
        static const Edge unknown[] = { { SRC_A, 100000, LOGIC_X } };
        static const Edge floating[] = { { SRC_A, 100000, LOGIC_Z } };

        expect(buf_y_at(up, 0, 50000) == LOGIC_0, "buf: A = 0 gives Y = 0");
        expect(buf_y_at(up, 1, 103000) == LOGIC_0, "buf: Y still 0 at 3 ns after A rises (4 ns delay)");
        expect(buf_y_at(up, 1, 105000) == LOGIC_1, "buf: A = 1 gives Y = 1 after 4 ns");
        expect(buf_y_at(unknown, 1, 110000) == LOGIC_X, "buf: A = X gives Y = X");
        expect(buf_y_at(floating, 1, 110000) == LOGIC_X, "buf: A floating (Z) gives Y = X");
    }

    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
