#include "trace_core.h"

#include <stdlib.h>
#include <string.h>

#define TRACE_DRIVES 40
#define TRACE_END_PS 3000000u
#define TRACE_STEP_PS 100000u
#define TRACE_FLIP_PS 1500000u
#define FNV_OFFSET 0xcbf29ce484222325ull
#define FNV_PRIME 0x100000001b3ull

static uint64_t fold_bytes(uint64_t hash, const void *data, size_t length) {
    const unsigned char *p = data;
    size_t i;

    for (i = 0; i < length; i++) {
        hash ^= p[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

/* Folds a value as 8 little-endian bytes, so the hash does not depend on the host. */
static uint64_t fold_u64(uint64_t hash, uint64_t value) {
    unsigned char bytes[8];
    int i;

    for (i = 0; i < 8; i++) {
        bytes[i] = (unsigned char)(value >> (8 * i));
    }
    return fold_bytes(hash, bytes, sizeof bytes);
}

uint64_t trace_fnv1a64(const char *text) {
    return fold_bytes(FNV_OFFSET, text, strlen(text));
}

static uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

/* One run: the subject, the shared hash and the verbose stream. */
typedef struct {
    const TraceSubject *subject;
    uint64_t hash;
    FILE *verbose;
} TraceRun;

/* ---- Stimulus: one pin per part pin, driving inputs and supplies -------- */

static size_t trace_pin_count(const void *config) {
    return ((const TraceRun *)config)->subject->pin_count;
}

static void *trace_create(const void *config) {
    return (void *)config;
}

static void stimulus_start(Sim *sim, SimInstance *instance, void *state) {
    const TraceSubject *subject = ((TraceRun *)state)->subject;
    uint64_t name_hash = trace_fnv1a64(subject->name);
    size_t i;

    for (i = 0; i < subject->pin_count; i++) {
        TraceRole role = subject->roles[i];

        if (role == TRACE_POWER) {
            sim_schedule_drive(sim, instance, i, LOGIC_1, DRIVE_STRONG, 0);
        } else if (role == TRACE_GROUND) {
            sim_schedule_drive(sim, instance, i, LOGIC_0, DRIVE_STRONG, 0);
        } else if (role == TRACE_INPUT || role == TRACE_INOUT) {
            DriveStrength strength = role == TRACE_INPUT ? DRIVE_STRONG : DRIVE_WEAK;
            uint64_t seed = name_hash ^ ((uint64_t)(i + 1) * 0x9E3779B97F4A7C15ull);
            SimTime t = 1000;
            LogicValue value = LOGIC_0;
            int n;

            for (n = 0; n < TRACE_DRIVES; n++) {
                sim_schedule_drive(sim, instance, i, value, strength, t);
                t += 2000 + xorshift64(&seed) % 150000;
                value = value == LOGIC_0 ? LOGIC_1 : LOGIC_0;
            }
        }
    }
}

static const SimComponentType STIMULUS_TYPE = {
    "trace-stimulus", trace_pin_count, trace_create, NULL,
    stimulus_start, NULL, NULL
};

/* ---- Probe: sees every change on every net ------------------------------- */

static const char VALUE_CHARS[] = "01XZ";

static void probe_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    TraceRun *run = state;
    SimTime now = sim_now(sim);

    (void)instance;
    (void)old_value;
    run->hash = fold_u64(run->hash, now);
    run->hash = fold_u64(run->hash, pin);
    run->hash = fold_u64(run->hash, (uint64_t)new_value);
    if (run->verbose != NULL) {
        fprintf(run->verbose, "%llu pin %zu -> %c\n", (unsigned long long)now, pin + 1,
                VALUE_CHARS[new_value]);
    }
}

static const SimComponentType PROBE_TYPE = {
    "trace-probe", trace_pin_count, trace_create, NULL,
    NULL, probe_pin_changed, NULL
};

/* ---- The run ------------------------------------------------------------- */

static void flip_switches(const TraceSubject *subject, Sim *sim, SimInstance *part,
                          uint64_t name_hash, int flipped) {
    size_t p;

    for (p = 1; p <= subject->switch_positions; p++) {
        int on = (int)((name_hash >> p) & 1u);

        subject->set_switch(sim, part, p, flipped ? !on : on);
    }
}

uint64_t trace_run(const TraceSubject *subject, FILE *verbose) {
    TraceRun run = {subject, FNV_OFFSET, verbose};
    Sim *sim = sim_create();
    SimInstance *part;
    SimInstance *stimulus;
    SimInstance *probe;
    uint64_t name_hash = trace_fnv1a64(subject->name);
    char lights[256];
    SimTime t;
    size_t i;
    const char *error;

    if (sim == NULL) {
        fprintf(stderr, "trace: out of memory\n");
        exit(2);
    }

    part = sim_add_instance(sim, subject->type, subject->name, subject->config);
    stimulus = sim_add_instance(sim, &STIMULUS_TYPE, "stimulus", &run);
    probe = sim_add_instance(sim, &PROBE_TYPE, "probe", &run);

    for (i = 0; i < subject->pin_count && probe != NULL; i++) {
        char net_name[32];
        SimNet *net;

        snprintf(net_name, sizeof net_name, "n%zu", i + 1);
        net = sim_add_net(sim, net_name);
        sim_connect(sim, net, part, i);
        if (subject->roles[i] != TRACE_NONE) {
            sim_connect(sim, net, stimulus, i);
        }
        sim_connect(sim, net, probe, i);
    }

    if (subject->set_switch != NULL && part != NULL) {
        flip_switches(subject, sim, part, name_hash, 0);
    }

    sim_start(sim);
    for (t = 0; t < TRACE_END_PS && sim_error(sim) == NULL; t += TRACE_STEP_PS) {
        if (t == TRACE_FLIP_PS && subject->set_switch != NULL) {
            flip_switches(subject, sim, part, name_hash, 1);
        }
        sim_run_until(sim, t + TRACE_STEP_PS);
        if (subject->lights != NULL && sim_error(sim) == NULL) {
            size_t count = subject->lights(sim, part, t, lights, sizeof lights);

            if (count > sizeof lights) {
                count = sizeof lights;
            }
            run.hash = fold_bytes(run.hash, lights, count);
            if (verbose != NULL) {
                fprintf(verbose, "%llu lights %.*s\n", (unsigned long long)(t + TRACE_STEP_PS),
                        (int)count, lights);
            }
        }
    }

    error = sim_error(sim);
    if (error == NULL) {
        error = "ok";
    }
    run.hash = fold_bytes(run.hash, error, strlen(error));
    run.hash = fold_u64(run.hash, sim_now(sim));
    if (verbose != NULL) {
        fprintf(verbose, "end %llu %s\n", (unsigned long long)sim_now(sim), error);
    }

    sim_destroy(sim);
    return run.hash;
}
