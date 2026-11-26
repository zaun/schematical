#ifndef SERIAL_BENCH_D_H
#define SERIAL_BENCH_D_H

#include "serial_bench.h"

extern const BenchConfig TI550_BENCH;
extern const BenchConfig TI550_BGA_BENCH;
extern const BenchConfig TI450_BENCH;
extern const BenchConfig QUAD_BENCH;
extern const BenchConfig QUAD_CD_BENCH;
extern const BenchConfig QUAD_C_BENCH;
extern const BenchConfig DUAL_SYNC_BENCH;
extern const BenchConfig DUAL_CD_BENCH;
extern const BenchConfig CHSEL_BENCH;
extern const BenchConfig NS_BENCH;
extern const BenchConfig TI45X_BENCH;
extern const BenchConfig TI451_BENCH;
extern const BenchConfig TI552_BENCH;
extern const BenchConfig COM92_BENCH;
extern const BenchConfig NXP650_BENCH;
extern const BenchConfig TI750E_BENCH;
void program_unit(Bench *b, unsigned unit, unsigned divisor, unsigned lcr);
void drive_bits_on(Bench *b, const char *pin, const char *bits, SimTime bt);
void drive_named(Bench *b, const char *rx, unsigned v, SimTime bt);
int part_exists(const char *name);
int wait_level(Bench *b, const char *pin, LogicValue level, SimTime limit);

#endif
