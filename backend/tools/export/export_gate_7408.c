/* gate.json devices from backend/74_08.c. */

#include "../../74_08.c"

#define HAND_STATE Sn74_08State
#define HAND_OP "and"
#define HAND_OPEN_DRAIN 0
#define HAND_XOR 0
#define HAND_NOTE "74_08.c"
#define HAND_FN export_gate_7408
#define HAND_BASE SN74_08_DEFINITION
#define HAND_FK SN74_08_FK_DEFINITION

#include "export_gate_hand.inc"
