/* gate.json devices from backend/74_86.c. */

#include "../../74_86.c"

#define HAND_STATE Sn74_86State
#define HAND_OP "xor"
#define HAND_OPEN_DRAIN 0
#define HAND_XOR 1
#define HAND_NOTE "74_86.c"
#define HAND_FN export_gate_7486
#define HAND_BASE SN74_86_DEFINITION
#define HAND_FK SN74_86_FK_DEFINITION

#include "export_gate_hand.inc"
