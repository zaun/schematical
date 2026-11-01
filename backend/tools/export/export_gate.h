#ifndef EXPORT_GATE_H
#define EXPORT_GATE_H

/*
 * Shared part of the gate exporters (plan-01-C section 3.3). Each
 * export_gate_<file>.c includes one old gate model and reports, per part,
 * the gates and timing that model really runs (from its own lookup and
 * create); export_gate_definition() turns a definition into a device,
 * merging an _FK definition into its base device as an "lccc20" pinout when
 * the gates (by pin name), output mode and timing agree.
 */

#include "export.h"

#define EXPORT_GATE_MAX 8
#define EXPORT_GATE_INPUTS 13

typedef struct {
    const char *op;                       /* "and", "nand", ... */
    size_t inputs[EXPORT_GATE_INPUTS];    /* 0-based pin indices */
    size_t input_count;
    size_t output;
} ExportGate;

typedef struct {
    int split;                            /* other-input timing (74x86) */
    unsigned long long rise, fall;
    unsigned long long low_rise, low_fall, high_rise, high_fall;
} ExportTiming;

typedef struct {
    const ChipDefinition *def;
    int open_drain;
    /* "74_00.c" for family_timing() notes; NULL for "def:<id>" notes */
    const char *note_file;
    int device_note;                      /* write "@def:<id>/" */
    /* The part's family, gates and timing; 0 on success. */
    int (*model)(const ChipDefinition *def, const char *part, int *family, int *package,
                 ExportGate *gates, size_t *gate_count, ExportTiming *timing);
} ExportGateSource;

int export_gate_definition(const ExportGateSource *source);

int export_gate_7400(void);
int export_gate_7402(void);
int export_gate_7408(void);
int export_gate_7432(void);
int export_gate_7486(void);
int export_gate_74266(void);
int export_gate_cd(void);
int export_gate_gt(void);
int export_gate(void);

#endif
