/* gate.json devices from backend/74_00.c. */

#include "../../74_00.c"

#define HAND_STATE Sn74_00State
#define HAND_OP "nand"
#define HAND_OPEN_DRAIN 0
#define HAND_XOR 0
#define HAND_NOTE "74_00.c"
#define HAND_FN export_gate_7400
#define HAND_BASE SN74_00_DEFINITION
#define HAND_FK SN74_00_FK_DEFINITION

#include "export_gate_hand.inc"
