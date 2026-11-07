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

/*
 * The reset input sampled at an edge. A reset that changed less than
 * `setup` before the edge, to a known level, takes effect at the next edge:
 * the result is the level it had before (0 while in_reset, else 1). The
 * chip then leaves or enters reset one cycle later, which is all an
 * asynchronous reset switch can do to it. A reset that is X itself stays X.
 */
LogicValue cpu_sample_reset(const Sim *sim, const SimInstance *instance, size_t pin,
                            SimTime changed, LogicValue seen, SimTime setup, int in_reset);

/* The pin's level now, or X when it changed less than `setup` ago. `seen`
   is the value pin_changed last reported for the pin: a different value now
   is a change at this very instant whose pin_changed has not run yet (the
   engine reports a batch's nets one by one). */
LogicValue cpu_sample(const Sim *sim, const SimInstance *instance, size_t pin,
                      SimTime changed, LogicValue seen, SimTime setup, LogicValue inactive);

/* ---- The instruction tracker (cpu.c; plan-09-B section 4.6) -------------- */

/* A disassembler (plan-14-H section 3.5): the instruction at pc from its
   recorded bytes (count of them; missing bytes read 0) into out (cap bytes,
   NUL-terminated). Returns the instruction's length in bytes. `user` is the
   value cpu_tracker_begin was given for this instruction. */
typedef size_t (*CpuDasm)(uint32_t pc, const uint8_t *bytes, size_t count, void *user, char *out,
                          size_t cap);

/* A floooh/chips disassembler (m6502dasm_op, w65c02dasm_op, z80dasm_op). */
typedef uint16_t (*CpuDasmFloooh)(uint16_t pc, uint8_t (*in)(void *), void (*out)(char, void *),
                                  void *user_data);

/* Runs a floooh disassembler as a CpuDasm. */
size_t cpu_dasm_floooh(CpuDasmFloooh dasm, uint32_t pc, const uint8_t *bytes, size_t count,
                       char *out, size_t cap);

typedef struct {
    CpuDasm dasm;
    int recording;
    uint32_t start;
    void *user;
    uint8_t bytes[4];
    size_t length;
    int has;                  /* the last completed instruction: */
    uint32_t address;
    uint8_t last[4];
    size_t last_length;
    char text[32];
} CpuTracker;

void cpu_tracker_reset(CpuTracker *tracker, CpuDasm dasm);
/* An instruction starts at address (bank in bits 16-23 on the 65816): the
   one being recorded is complete. `user` goes to the disassembler. */
void cpu_tracker_begin(CpuTracker *tracker, uint32_t address, void *user);
/* A read cycle's sampled byte. Bytes follow the start within its bank. */
void cpu_tracker_read(CpuTracker *tracker, uint32_t address, uint8_t data);
/* The instruction being recorded is complete, and no new one starts. */
void cpu_tracker_finish(CpuTracker *tracker);
void cpu_tracker_fill(const CpuTracker *tracker, CpuState *out);

/* ---- The 6502 family (cpu6502.c) ------------------------------------------ */

extern const CpuCoreOps CPU6502_OPS;

/* ---- The Z80 (cpuz80.c) ---------------------------------------------------- */

extern const CpuCoreOps CPUZ80_OPS;

/* ---- The W65C816S (cpu65816.c, plan-14-H) ---------------------------------- */

extern const CpuCoreOps CPU65816_OPS;

#endif
