/* jkff.json from backend/ff-jk.c (definitions in the old catalog's order). */

#include "../../ff-jk.c"

#define FF_JK 1
#define FF_FN export_jkff
#define FF_DEFS \
    &FF_74_73_DEFINITION, &FF_74_76_DEFINITION, &FF_74_107_DEFINITION, \
    &FF_74_109_DEFINITION, &FF_74_112_DEFINITION, &FF_4027B_DEFINITION, \
    &FF_74_109_FK_DEFINITION, &FF_74_112_FK_DEFINITION

#include "export_ff.inc"
