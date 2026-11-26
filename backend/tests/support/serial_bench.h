#ifndef SERIAL_BENCH_H
#define SERIAL_BENCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"
#include "read_file.h"
#include "serial_fixture.h"
#include "net.h"
#include "serial_fixture_classic.h"
#include "serial_fixture_maxim.h"
#include "sim.h"

#define NS(x) ((SimTime)(x) * 1000u)
extern int failures;
extern int case_failures;
extern const char *c_part;
extern PartLibrary *library;
void check(int ok, const char *what);
void case_begin(void);
void case_end(const char *name);
size_t driver_pins(const void *config);
extern const SimComponentType DRIVER;

typedef struct {
    SimTime half;
    LogicValue level;
} ClockState;
size_t clock_pins(const void *config);
void *clock_create(const void *config);
void clock_destroy(void *state);
void clock_start(Sim *sim, SimInstance *instance, void *raw);
void sb_clock_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id);
extern const SimComponentType CLOCK;

/* ---- The bench ------------------------------------------------------------------ */

/*
 * How a device sits on the bench: its strobe, data, address, reset and clock
 * pins, the pins tied at fixed levels, and how each unit (channel, printer
 * port) is selected for a bus access.
 */
typedef struct {
    const char *read;              /* active-low read strobe */
    const char *write;             /* active-low write strobe */
    const char *data[8];
    const char *address[8];
    unsigned address_count;
    const char *ties[32];          /* "NAME=1" / "NAME=0" / "NAME=Z" */
    const char *reset;             /* NULL = none */
    int reset_high;
    const char *clock;
    const char *baudout;           /* joined to rclk when both are set */
    const char *rclk;
    /* Per unit: the select pins to make active ("NAME=0" etc.; each unit
       sets these and the other units' to their inactive levels). */
    const char *unit_select[8][3];
    unsigned units;
    unsigned recovery_ns;          /* after a strobe ends (0 = 160 ns); longer for
                                      clock-synchronous strobes */
} BenchConfig;
extern const BenchConfig PC16550D_BENCH;

typedef struct {
    Sim *sim;
    SimInstance *chip;
    SimInstance *drv;
    SimInstance *clk;
    const Part *part;
    const BenchConfig *cfg;
    unsigned unit;
    size_t pins;
    SimTime half;
    SimTime period;
} Bench;
size_t pin_of(const Bench *b, const char *name);
int has_pin(const Bench *b, const char *name);
void set(Bench *b, const char *name, LogicValue v);
void set_tie(Bench *b, const char *tie, int invert);
void run(Bench *b, SimTime ps);
LogicValue get(const Bench *b, const char *name);
void select_unit(Bench *b, unsigned u);
void pulse_reset(Bench *b);
Bench *bench_open_cfg(const char *part_name, SimTime half, const BenchConfig *cfg);
Bench *bench_open(const char *part_name, SimTime half);
void bench_close(Bench *b);
void set_address(Bench *b, unsigned a);
void set_data(Bench *b, int drive, unsigned v);
void bus_write(Bench *b, unsigned a, unsigned v);
unsigned bus_read_text(Bench *b, unsigned a, char text[9]);
unsigned bus_read(Bench *b, unsigned a);
void expect_reg(Bench *b, unsigned a, const char *want, const char *what);
void expect_pin(Bench *b, const char *pin, LogicValue want, const char *what);
void program(Bench *b, unsigned divisor, unsigned lcr);
SimTime bit_time(const Bench *b, unsigned divisor);
int wait_start(Bench *b, SimTime limit);
void sample_frame(Bench *b, unsigned divisor, unsigned n, char *out);
void drive_bits(Bench *b, const char *bits, SimTime bt);
void drive_byte(Bench *b, unsigned v, SimTime bt, int stop);
size_t sent(Bench *b, uint16_t *frames, size_t cap);
int sb_send(Bench *b, const char *text);

/* 1.8432 MHz: 542.5 ns period. */
#define HALF_18432 NS(271)
void strip_active_low(cJSON *device);
int compare_shipped(const char *fixture_text, const cJSON *shipped, const char *what);
char *read_fixture(const char *file);
int load_new_footprints(void);
int load_file(const char *name, int footprints);
int serial_bench_load(void);
int serial_bench_finish(void);

#endif
