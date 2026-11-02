#ifndef CPU_H
#define CPU_H

/*
 * The cpu kind's internal interface (plan-09-B section 4.2): the device and
 * grade data the loader builds (cpu.c), and the per-core adapter table
 * (cpu6502.c; the Z80 in plan-09-C). One SimComponentType (CPU_MODEL in
 * cpu.c) forwards every hook to the part's core.
 */

#include "cpu_state.h"
#include "kind.h"

enum {
    CPU_ADDRESS_BITS = 16,
    CPU_DATA_BITS = 8,
    CPU_MAX_CONTROLS = 16,
    CPU_MAX_TIMING = 40,
    CPU_MAX_PINS = 64
};

#define CPU_NEVER UINT64_MAX

typedef struct CpuCoreOps CpuCoreOps;

/* One speed grade: the core's timing values (picoseconds, in the order of
   CpuCoreDesc.timing), the facts, and the grade's voltage keys. */
typedef struct {
    SimTime t[CPU_MAX_TIMING];
    PartCpu info;
    const PartVoltage *vcc;   /* NULL = none */
} CpuGrade;

/* A core the "core" key can name. */
typedef struct {
    const char *name;
    const char *const *controls;  /* control keys, NULL-terminated, in the core's control order */
    const char *const *timing;    /* timing names, NULL-terminated; timing[0] is the clock period */
    size_t positive;              /* the first `positive` timing values must be more than 0 */
    const CpuCoreOps *ops;
} CpuCoreDesc;

typedef struct {
    const CpuCoreDesc *core;
    const char *const *address;
    size_t address_count;
    const char *const *data;
    size_t data_count;
    const char *controls[CPU_MAX_CONTROLS];   /* pin names, in the core's control order */
    size_t control_count;
    const char **grade_names;
    CpuGrade *grades;
    size_t grade_count;
} CpuDevice;

/* A device's pins in one pinout. */
typedef struct {
    size_t address[CPU_ADDRESS_BITS];
    size_t data[CPU_DATA_BITS];
    size_t control[CPU_MAX_CONTROLS];
    const CpuDevice *device;
} CpuPins;

struct CpuCoreOps {
    void *(*create)(const Part *part, const CpuPins *pins, const CpuGrade *grade);
    void (*destroy)(void *state);
    void (*start)(Sim *sim, SimInstance *instance, void *state);
    void (*pin_changed)(Sim *sim, SimInstance *instance, void *state, size_t pin,
                        LogicValue old_value, LogicValue new_value);
    void (*timer)(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id);
    int (*state)(const void *state, CpuState *out);
};

/* Every core's model state starts with this; CPU_MODEL forwards through it. */
typedef struct {
    const CpuCoreOps *ops;
} CpuBase;

/* ---- Inputs (cpu.c) ------------------------------------------------------ */

/* A pin's level: 0, 1 or LOGIC_X; Z reads as `inactive` (inputs left open
   count as inactive). */
LogicValue cpu_level(LogicValue value, LogicValue inactive);

/* The pin's level now, or X when it changed less than `setup` ago. `seen`
   is the value pin_changed last reported for the pin: a different value now
   is a change at this very instant whose pin_changed has not run yet (the
   engine reports a batch's nets one by one). */
LogicValue cpu_sample(const Sim *sim, const SimInstance *instance, size_t pin,
                      SimTime changed, LogicValue seen, SimTime setup, LogicValue inactive);

/* ---- The instruction tracker (cpu.c; plan-09-B section 4.6) -------------- */

/* A vendored disassembler (m6502dasm_op, w65c02dasm_op, z80dasm_op). */
typedef uint16_t (*CpuDasm)(uint16_t pc, uint8_t (*in)(void *), void (*out)(char, void *),
                            void *user_data);

typedef struct {
    CpuDasm dasm;
    int recording;
    uint16_t start;
    uint8_t bytes[4];
    size_t length;
    int has;                  /* the last completed instruction: */
    uint16_t address;
    uint8_t last[4];
    size_t last_length;
    char text[32];
} CpuTracker;

void cpu_tracker_reset(CpuTracker *tracker, CpuDasm dasm);
/* An instruction starts at address: the one being recorded is complete. */
void cpu_tracker_begin(CpuTracker *tracker, uint16_t address);
/* A read cycle's sampled byte. */
void cpu_tracker_read(CpuTracker *tracker, uint16_t address, uint8_t data);
/* The instruction being recorded is complete, and no new one starts. */
void cpu_tracker_finish(CpuTracker *tracker);
void cpu_tracker_fill(const CpuTracker *tracker, CpuState *out);

/* ---- The 6502 family (cpu6502.c) ------------------------------------------ */

extern const CpuCoreOps CPU6502_OPS;

#endif
