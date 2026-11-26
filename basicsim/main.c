#include "net.h"
#include "sim.h"
#include "kind.h"
#include "parts.h"

#include <stdio.h>
#include <stdlib.h>

typedef int (*Loader)(PartLibrary *, const char *, const char *, char **);

/* Reads the file at `path` and loads it into `library` with `load`. A short
   read leaves a truncated text, which the loader reports. */
static int load_file(PartLibrary *library, const char *path, Loader load) {
    FILE *file = fopen(path, "rb");
    char *text = NULL;
    char *error = NULL;
    long length = 0;
    int result;

    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        text = malloc((size_t)length + 1);
        if (text != NULL) {
            text[fread(text, 1, (size_t)length, file)] = '\0';
        }
        fclose(file);
    }
    if (text == NULL) {
        fprintf(stderr, "Could not read %s\n", path);
        return -1;
    }
    result = load(library, text, path, &error);
    if (result != 0) {
        fprintf(stderr, "%s\n", error);
        free(error);
    }
    free(text);
    return result;
}

/*
 * The gates from `gates` (the app's gate.json unless the command line names
 * another file). The app's datasheets and footprints load first: gate.json
 * names both.
 */
static PartLibrary *load_gates(const char *gates) {
    PartLibrary *library = part_library_create();

    if (library == NULL || load_file(library, PARTS_DIR "/datasheets.json", load_datasheets) != 0 ||
        load_file(library, PARTS_DIR "/footprints.json", load_footprints) != 0 ||
        load_file(library, gates, load_parts) != 0) {
        part_library_destroy(library);
        return NULL;
    }
    return library;
}

static size_t source_pin_count(const void *config) {
    (void)config;
    return 2;
}

static void source_start(
    Sim *sim,
    SimInstance *instance,
    void *state
) {
    (void)state;

    sim_schedule_drive(sim, instance, 0, LOGIC_1, DRIVE_STRONG, 20000);
    sim_schedule_drive(sim, instance, 1, LOGIC_1, DRIVE_STRONG, 20000);
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 40000);
}

static const SimComponentType SOURCE_TYPE = {
    "test source",
    source_pin_count,
    NULL,
    NULL,
    source_start,
    NULL,
    NULL
};

/*
 * Expected outputs of the AND (SN74LS08N) and NAND (SN74LS00N) gates, both
 * fed by the same A/B source. Timing follows the LS delays: the NAND's first
 * evaluation of unknown inputs drives X after 11 ns, so its output is still
 * Z at 10 ns; it falls at 28 ns (7 ns fall delay after A and B go high at
 * 20 ns) and rises 11 ns after A returns low at 40 ns, so it is still 0 at
 * 50 ns and settles to 1 at 55 ns.
 */
typedef struct {
    SimTime time;
    LogicValue and_value;
    LogicValue nand_value;
} Checkpoint;

static const Checkpoint CHECKPOINTS[] = {
    {10000, LOGIC_X, LOGIC_Z},
    {28000, LOGIC_1, LOGIC_0},
    {50000, LOGIC_0, LOGIC_0},
    {55000, LOGIC_0, LOGIC_1},
};

/* LOGIC_0, LOGIC_1, LOGIC_X, LOGIC_Z in sim.h's order. */
static char logic_char(LogicValue value) {
    return "01XZ"[value];
}

/* Builds the circuit and checks every checkpoint: 1 when all match. */
static int run(Sim *sim, PartLibrary *library) {
    SimInstance *source;
    SimInstance *and_chip;
    SimInstance *nand_chip;
    const SimComponentType *and_type;
    const SimComponentType *nand_type;
    SimNet *a;
    SimNet *b;
    SimNet *and_out;
    SimNet *nand_out;
    const Part *and_part;
    const Part *nand_part;
    size_t i;

    source = sim_add_instance(sim, &SOURCE_TYPE, "inputs", NULL);

    /* The gates come from the part library; the source stays engine-level. */
    and_part = part_library_find(library, "SN74LS08N");
    and_type = and_part != NULL ? kind_model(and_part) : NULL;
    and_chip = and_type != NULL
        ? sim_add_instance(sim, and_type, "U1", and_part)
        : NULL;

    nand_part = part_library_find(library, "SN74LS00N");
    nand_type = nand_part != NULL ? kind_model(nand_part) : NULL;
    nand_chip = nand_type != NULL
        ? sim_add_instance(sim, nand_type, "U2", nand_part)
        : NULL;

    a = sim_add_net(sim, "A");
    b = sim_add_net(sim, "B");
    and_out = sim_add_net(sim, "AND_OUT");
    nand_out = sim_add_net(sim, "NAND_OUT");

    /* Pin indices are 0-based: pins 1A, 1B, 1Y of each gate. */
    if (source == NULL || and_chip == NULL || nand_chip == NULL || a == NULL || b == NULL || and_out == NULL ||
        nand_out == NULL || !sim_connect(sim, a, source, 0) || !sim_connect(sim, b, source, 1) ||
        !sim_connect(sim, a, and_chip, 0) || !sim_connect(sim, b, and_chip, 1) ||
        !sim_connect(sim, and_out, and_chip, 2) || !sim_connect(sim, a, nand_chip, 0) ||
        !sim_connect(sim, b, nand_chip, 1) || !sim_connect(sim, nand_out, nand_chip, 2) || !sim_start(sim)) {
        return 0;
    }

    for (i = 0; i < sizeof(CHECKPOINTS) / sizeof(CHECKPOINTS[0]); i++) {
        const Checkpoint *checkpoint = &CHECKPOINTS[i];
        /* A failed run leaves the nets as they were: the check below reports it. */
        int ran = sim_run_until(sim, checkpoint->time);
        LogicValue and_actual = sim_net_value(and_out);
        LogicValue nand_actual = sim_net_value(nand_out);

        printf("%llu ns: AND = %c, NAND = %c\n", (unsigned long long)(checkpoint->time / 1000),
               logic_char(and_actual), logic_char(nand_actual));
        if (!ran || and_actual != checkpoint->and_value || nand_actual != checkpoint->nand_value) {
            fprintf(stderr, "FAIL at %llu ns: expected AND = %c, NAND = %c\n",
                    (unsigned long long)(checkpoint->time / 1000), logic_char(checkpoint->and_value),
                    logic_char(checkpoint->nand_value));
            return 0;
        }
    }
    return 1;
}

/* basicsim [gate file]: an AND and a NAND gate from the part library, driven
   by an engine-level source, checked at four times. */
int main(int argc, char **argv) {
    Sim *sim = sim_create();
    PartLibrary *library = load_gates(argc > 1 ? argv[1] : PARTS_DIR "/gate.json");
    int result = sim != NULL && library != NULL && run(sim, library) ? 0 : 1;

    if (result != 0) {
        const char *error = sim != NULL ? sim_error(sim) : NULL;

        fprintf(stderr, "Simulation error: %s\n", error != NULL ? error : "unknown");
    }
    sim_destroy(sim);
    part_library_destroy(library);
    return result;
}
