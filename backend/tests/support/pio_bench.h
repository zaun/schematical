#ifndef PIO_BENCH_H
#define PIO_BENCH_H

/*
 * pio_test: the pio kind's models and the 65xx bus helper (plan-14).
 *
 * A1-A8 drive bus65 through a test-only component (16 registers in an
 * array) on the shared bench, with the W65C22S 14 MHz timing (tacr 10, tcar
 * 10, tcdr 20, thr 10, tacw 10, tcaw 10, tdcw 10, thw 10 ns; tcyc 70, tpwh
 * 35, tpwl 35; w65c22.pdf p.41).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "read_file.h"
#include "bus65.h"
#include "kind.h"
#include "model_pins.h"

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
            failures++;                                                    \
        }                                                                  \
    } while (0)

#define NS 1000ull
#define HALF (500 * NS)   /* 1 MHz PHI2 */

/* ---- The test component ----------------------------------------------------- */

enum {
    P_PHI2, P_RWB, P_CS1, P_CS2B, P_RS0, P_RS1, P_RS2, P_RS3,
    P_D0, P_D1, P_D2, P_D3, P_D4, P_D5, P_D6, P_D7, P_COUNT
};

extern int failures;
extern const PartPin TEST_PINS[P_COUNT];
extern const Bus65Pins TEST_BUS_PINS;
extern const Bus65Timing TEST_TIMING;

typedef struct {
    Bus65 bus;
    uint8_t regs[16], known[16];
    unsigned reads, writes, cycles, unknown_reads;
} TestRegs;
int t_read(void *model, unsigned reg, uint8_t *value, uint8_t *known);
void t_write(void *model, unsigned reg, uint8_t value, uint8_t known);
void t_cycle(void *model);
void t_read_unknown(void *model);
extern const Bus65Ops TEST_OPS;
size_t t_pin_count(const void *config);
void *t_create(const void *config);
void t_destroy(void *state);
void t_start(Sim *sim, SimInstance *instance, void *state);
void t_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                          LogicValue old_value, LogicValue new_value);
void t_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id);
extern const SimComponentType TEST_TYPE;
extern const PartKindOps TEST_KIND;
extern const PartInternal TEST_INTERNAL;
Part test_part(void);
extern const char *const DATA_NAMES[8];
extern const char *const RS_NAMES[4];
LogicValue bit_value(unsigned v, int i);
void set_rs(Bench *b, unsigned reg);
void set_data(Bench *b, unsigned v);
void release_data(Bench *b);
char data_state(Bench *b, unsigned *value);
TestRegs *regs_of(Bench *b);
Bench *new_bench(const Part *part);
void bus_write(Bench *b, unsigned reg, unsigned value);
extern PartLibrary *shipped_library;
const Part *shipped_part(const char *name);
extern const char *const PA_NAMES[8];
extern const char *const PB_NAMES[8];
unsigned pins_value(Bench *b, const char *const *names, unsigned *x);
void idle_begin(Bench *b);
void idle_end(Bench *b, SimTime since_fall);
void idle_to_rise(Bench *b);
void finish_high(Bench *b, SimTime elapsed);

#endif
