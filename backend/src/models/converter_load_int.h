/* models/converter_load_int.h: what the pieces of models/converter_load.c share (plan-68-I/J split it). */
#ifndef MODELS_CONVERTER_LOAD_INT_H
#define MODELS_CONVERTER_LOAD_INT_H

/*
 * Loads a converter, rtc, leddriver or lcddriver device
 * (planning/plan-24-data/schema.md, docs/parts-format.md) into a CvDevice, binds its pin names per pinout and loads its parts. Everything
 * comes from the KindContext arena.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "converter.h"
#include "display_out.h"
#include "rtc.h"

typedef struct {
    KindContext *ctx;
    CvDevice *dev;
    CvRegister *regs;
    size_t reg_count;
    const char **spaces;
    size_t space_count;
    const char **names;
    size_t name_count, name_cap;
    size_t ram_first, ram_count;    /* display.ram {"bytes", ...}: the registers it made */
    int display;                    /* a leddriver / lcddriver device */
} Ld;

/* ---- Patterns -------------------------------------------------------------------------- */

typedef struct {
    CvTok *toks;
    size_t count, cap;
} TokBuf;
/* ---- The display section (plan-56-B; docs/parts-format.md § leddriver, § lcddriver) ----- */

enum {
    DISP_MAX_USED = 512,
    DISP_MAX_LINES = 32,            /* digits, rows */
    DISP_MAX_SEGMENTS = 17,         /* data pins of one digit / row / group */
    DISP_MAX_LCD_SEGMENTS = 160
};

typedef struct {
    Ld *ld;
    int used[DISP_MAX_USED];        /* name ids the section has taken: a pin has one job */
    size_t used_count;
} Dl;

int cvl_cv_ns(KindContext *ctx, const cJSON *value, const char *what, uint64_t *ps);
const cJSON *cvl_member(const cJSON *o, const char *key);
int cvl_check_keys(Ld *ld, const cJSON *o, const char *const *keys);
uint32_t cvl_mask_of(unsigned bits);
int cvl_u32(Ld *ld, const cJSON *v, const char *what, uint32_t *out);
int cvl_u32_member(Ld *ld, const cJSON *o, const char *key, uint32_t *out);
int cvl_level_member(Ld *ld, const cJSON *o, const char *key, int dflt, int *out);
int cvl_name_id(Ld *ld, const char *name);
int cvl_pin_member(Ld *ld, const cJSON *o, const char *key, int required, int *out);
int cvl_space_index(Ld *ld, const char *name);
int cvl_field_ref(Ld *ld, const char *text, CvField *out);
int cvl_field_member(Ld *ld, const cJSON *o, const char *key, CvField *out);
int cvl_cond_text(Ld *ld, const char *s, CvCond *out);
int cvl_conds(Ld *ld, const cJSON *v, CvConds *out);
int cvl_value_of(Ld *ld, const cJSON *v, const char *key, CvValue *out);
int cvl_pattern(Ld *ld, const char *text, int in, CvPattern *out, uint32_t *caps);
int cvl_pattern_member(Ld *ld, const cJSON *o, const char *key, int in, CvPattern *out,
                          uint32_t *caps);
int cvl_load_registers(Ld *ld, const cJSON *arr);
int cvl_load_protocol(Ld *ld, const cJSON *p, CvProtocol *out);
int cvl_load_bus(Ld *ld, const cJSON *b, CvBus *out, int config);
int cvl_load_choice(Ld *ld, const cJSON *v, CvChoice *out, int formats);
int cvl_load_conversion(Ld *ld, const cJSON *c, CvConversion *out);
int cvl_load_ready(Ld *ld, const cJSON *arr, CvDevice *dev);
int cvl_load_pins(Ld *ld, const cJSON *p, CvPins *out);
int cvl_load_dac(Ld *ld, const cJSON *d, CvDac *out);
int cvl_load_clock(Ld *ld, const cJSON *c, CvDevice *dev);
int cvl_load_ram(Ld *ld, const cJSON *c);
int cvl_load_display_ram(Ld *ld, const cJSON *d);
int cvl_dl_take(Dl *dl, const char *name, int *id);
int cvl_dl_pins(Dl *dl, const cJSON *o, const char *key, size_t min, size_t max,
                   const int **out, size_t *count);
int cvl_dl_field_member(Ld *ld, const cJSON *o, const char *key, CvField *out);
int cvl_dl_ram(Ld *ld, const cJSON *o, size_t want, const char *noun, const CvField **out,
                  size_t *count);
int cvl_dl_cond(Ld *ld, const cJSON *o, const char *key, CvConds *out);
int cvl_dl_us(Ld *ld, const cJSON *v, const char *what, uint64_t *ps);
int cvl_dl_scan(Ld *ld, const cJSON *o, uint64_t *ps);
int cvl_dl_open_drain(Dl *dl, const cJSON *o, DispCfg *c);
int cvl_load_display_scan(Dl *dl, const cJSON *o, DispCfg *c, int matrix);
int cvl_load_display_static(Dl *dl, const cJSON *o, DispCfg *c);
int cvl_load_display_ports(Dl *dl, const cJSON *o, DispCfg *c);
int cvl_load_display(Ld *ld, const cJSON *d, CvDevice *dev);

#endif
