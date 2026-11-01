/*
 * Template for the hand-written quad 2-input gate files (74_00.c ...
 * 74_266.c). Before including, define:
 *   HAND_STATE      the file's state struct (gates + timing)
 *   HAND_OP         "nand", "nor", ...
 *   HAND_OPEN_DRAIN 1 for 74_266
 *   HAND_XOR        1 when the timing is an XorTiming (74_86)
 *   HAND_NOTE       "74_00.c"
 *   HAND_FN         the exporter function name
 *   HAND_BASE       the base ChipDefinition
 *   HAND_FK         the _FK ChipDefinition, if any
 * and include the old model .c file.
 */

#include "export_gate.h"

static int hand_model(const ChipDefinition *def, const char *part, int *family, int *package,
                      ExportGate *gates, size_t *gate_count, ExportTiming *timing) {
    SimLogicConfig config;
    const SimComponentType *type = def->lookup(part, &config);
    HAND_STATE *state;
    size_t i;

    if (type == NULL) {
        return -1;
    }
    state = type->create(&config);
    *family = (int)config.family;
    *package = (int)config.package;
    for (i = 0; i < 4; i++) {
        gates[i].op = HAND_OP;
        gates[i].input_count = 2;
        gates[i].inputs[0] = state->gates[i].input_a;
        gates[i].inputs[1] = state->gates[i].input_b;
        gates[i].output = state->gates[i].output;
    }
    *gate_count = 4;
#if HAND_XOR
    timing->split = 1;
    timing->low_rise = state->timing.other_low.rise_delay;
    timing->low_fall = state->timing.other_low.fall_delay;
    timing->high_rise = state->timing.other_high.rise_delay;
    timing->high_fall = state->timing.other_high.fall_delay;
#else
    timing->rise = state->timing.rise_delay;
    timing->fall = state->timing.fall_delay;
#endif
    type->destroy(state);
    return 0;
}

int HAND_FN(void) {
    ExportGateSource source = {0};

    source.open_drain = HAND_OPEN_DRAIN;
    source.note_file = HAND_NOTE;
    source.model = hand_model;
    source.def = &HAND_BASE;
    if (export_gate_definition(&source) != 0) {
        return -1;
    }
#ifdef HAND_FK
    source.def = &HAND_FK;
    if (export_gate_definition(&source) != 0) {
        return -1;
    }
#endif
    return 0;
}
