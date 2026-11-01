/* gate.json devices from backend/74_02.c. */

#include "../../74_02.c"

#define HAND_STATE Sn74_02State
#define HAND_OP "nor"
#define HAND_OPEN_DRAIN 0
#define HAND_XOR 0
#define HAND_NOTE "74_02.c"
#define HAND_FN export_gate_7402
#define HAND_BASE SN74_02_DEFINITION
#define HAND_FK SN74_02_FK_DEFINITION

#include "export_gate_hand.inc"
