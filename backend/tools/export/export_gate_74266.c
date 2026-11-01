/* gate.json devices from backend/74_266.c. */

#include "../../74_266.c"

#define HAND_STATE Sn74_266State
#define HAND_OP "xnor"
#define HAND_OPEN_DRAIN 1
#define HAND_XOR 0
#define HAND_NOTE "74_266.c"
#define HAND_FN export_gate_74266
#define HAND_BASE SN74_266_DEFINITION

#include "export_gate_hand.inc"
