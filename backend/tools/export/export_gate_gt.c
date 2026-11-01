/* gate.json devices from backend/gates.c + gates_defs.inc (generated N-input gates). */

#include "../../gates.c"

#include "export_gate.h"

static const char *op_name(GtOp op) {
    switch (op) {
        case GT_AND: return "and";
        case GT_NAND: return "nand";
        case GT_OR: return "or";
        case GT_NOR: return "nor";
        case GT_XOR: return "xor";
        case GT_XNOR: return "xnor";
    }
    return "?";
}

static int gt_model(const ChipDefinition *def, const char *part, int *family, int *package,
                    ExportGate *gates, size_t *gate_count, ExportTiming *timing) {
    SimLogicConfig config;
    const SimComponentType *type = def->lookup(part, &config);
    GtState *state;
    size_t g;
    size_t i;

    if (type == NULL) {
        return -1;
    }
    state = type->create(&config);
    *family = (int)config.family;
    *package = (int)config.package;
    if (state->layout->gate_count > EXPORT_GATE_MAX) {
        fprintf(stderr, "%s: too many gates\n", def->id);
        exit(2);
    }
    for (g = 0; g < state->layout->gate_count; g++) {
        const GtGate *gate = &state->layout->gates[g];

        gates[g].op = op_name(gate->op);
        gates[g].input_count = gate->input_count;
        for (i = 0; i < gate->input_count; i++) {
            gates[g].inputs[i] = gate->inputs[i];
        }
        gates[g].output = gate->output;
    }
    *gate_count = state->layout->gate_count;
    timing->rise = state->timing.rise_delay;
    timing->fall = state->timing.fall_delay;
    type->destroy(state);
    return 0;
}

int export_gate_gt(void) {
    static const ChipDefinition *const DEFS[] = {
#include "../../gates_catalog.inc"
    };
    ExportGateSource source = {0};
    size_t i;

    source.device_note = 1;
    source.model = gt_model;
    for (i = 0; i < sizeof DEFS / sizeof DEFS[0]; i++) {
        source.def = DEFS[i];
        if (export_gate_definition(&source) != 0) {
            return -1;
        }
    }
    return 0;
}
