/* gate.json devices from backend/cd4000.c (CD4000 quad 2-input gates; family C). */

#include "../../cd4000.c"

#include "export_gate.h"

static const char *op_name(CdOp op) {
    switch (op) {
        case CD_OP_NOR: return "nor";
        case CD_OP_NAND: return "nand";
        case CD_OP_XOR: return "xor";
        case CD_OP_XNOR: return "xnor";
        case CD_OP_OR: return "or";
        case CD_OP_AND: return "and";
    }
    return "?";
}

static int cd_model(const ChipDefinition *def, const char *part, int *family, int *package,
                    ExportGate *gates, size_t *gate_count, ExportTiming *timing) {
    SimLogicConfig config;
    const SimComponentType *type = def->lookup(part, &config);
    CdState *state;
    size_t i;

    if (type == NULL) {
        return -1;
    }
    state = type->create(&config);
    *family = (int)config.family;
    *package = (int)config.package;
    for (i = 0; i < 4; i++) {
        gates[i].op = op_name(state->data->op);
        gates[i].input_count = 2;
        gates[i].inputs[0] = GATES[i].input_a;
        gates[i].inputs[1] = GATES[i].input_b;
        gates[i].output = GATES[i].output;
    }
    *gate_count = 4;
    timing->rise = state->data->delay;
    timing->fall = state->data->delay;
    type->destroy(state);
    return 0;
}

int export_gate_cd(void) {
    static const ChipDefinition *const DEFS[] = {
        &CD4001B_DEFINITION, &CD4011B_DEFINITION, &CD4030B_DEFINITION, &CD4070B_DEFINITION,
        &CD4071B_DEFINITION, &CD4077B_DEFINITION, &CD4081B_DEFINITION, &CD4093B_DEFINITION,
    };
    ExportGateSource source = {0};
    size_t i;

    /* cd4000.c has no timing comments: no notes */
    source.model = cd_model;
    for (i = 0; i < sizeof DEFS / sizeof DEFS[0]; i++) {
        source.def = DEFS[i];
        if (export_gate_definition(&source) != 0) {
            return -1;
        }
    }
    return 0;
}
