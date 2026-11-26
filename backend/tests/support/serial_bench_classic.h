#ifndef SERIAL_BENCH_CLASSIC_H
#define SERIAL_BENCH_CLASSIC_H

#include "serial_bench.h"

/* ==== plan-12-H: the classic UARTs (cases H1-H8) ================================== */

/*
 * The H bench: every pin of the part on its own net with a bench driver,
 * except clock pins, which a free-running clock drives instead (pins in one
 * group share one clock). Fixtures: support/serial_fixture_classic.h.
 */
typedef struct {
    Bench b;
    SimInstance *clocks[4];
    SimTime halves[4];
    unsigned clock_count;
} HBench;

typedef struct {
    const char *pin;
    unsigned group;
} HClockPin;
HBench *hb_open(const char *part_name, const HClockPin *clock_pins, unsigned n_pins,
                       const SimTime *halves, unsigned n_clocks);
void hb_close(HBench *h);
void h_set_bus(Bench *b, const char *const *names, unsigned n, int drive, unsigned v);
unsigned h_get_bus(Bench *b, const char *const *names, unsigned n, char *text);
void h_expect_bus(Bench *b, const char *const *names, unsigned n, const char *want,
                         const char *what);
int h_wait(Bench *b, const char *pin, LogicValue level, SimTime limit);
void h_frame(Bench *b, const char *pin, SimTime bt, unsigned n, char *out);
int h_catch(Bench *b, const char *pin, SimTime bt, unsigned n, SimTime limit, char *out);
void h_drive_bits(Bench *b, const char *pin, const char *bits, SimTime bt);
void h_frame_bits(unsigned v, unsigned n, char parity, unsigned stops, char *out);
int h_send(Bench *b, unsigned channel, const char *text);
void h_expect_text(const char *got, const char *want, const char *what);
void h_expect_sent(Bench *b, unsigned channel, unsigned v, const char *what);

#define AC_XHALF ((SimTime)271267)

#endif
