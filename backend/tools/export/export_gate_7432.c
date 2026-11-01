/* gate.json devices from backend/74_32.c. */

#include "../../74_32.c"

#define HAND_STATE Sn74_32State
#define HAND_OP "or"
#define HAND_OPEN_DRAIN 0
#define HAND_XOR 0
#define HAND_NOTE "74_32.c"
#define HAND_FN export_gate_7432
#define HAND_BASE SN74_32_DEFINITION
#define HAND_FK SN74_32_FK_DEFINITION

#include "export_gate_hand.inc"
