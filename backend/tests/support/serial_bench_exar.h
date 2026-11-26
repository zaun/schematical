#ifndef SERIAL_BENCH_EXAR_H
#define SERIAL_BENCH_EXAR_H

#include "serial_bench.h"
#include "serial_bench_d.h"

extern const BenchConfig E_ST450_BENCH;
extern const BenchConfig E_ONE_BENCH;
extern const BenchConfig E_L580_BENCH;
extern const BenchConfig E_DUAL_MOTO_BENCH;
extern const BenchConfig E_DUAL_68_BENCH;
extern const BenchConfig E_DUAL_CLKSEL_BENCH;
extern const BenchConfig E_DUAL_QFN_BENCH;
extern const BenchConfig E_QUAD_V_BENCH;
extern const BenchConfig E_QUAD_CLKSEL_BENCH;
extern const BenchConfig E_C854_BENCH;
extern const BenchConfig E_C864_BENCH;
extern const BenchConfig E_V798_BENCH;
extern const BenchConfig E_L784_BENCH;
void e_efr(Bench *b, unsigned efr);
void e_before_rise(Bench *b, SimTime before);
void m_write(Bench *b, const char *cs, unsigned a, unsigned v);
unsigned m_read_text(Bench *b, const char *cs, unsigned a, char text[9]);
void m_expect(Bench *b, const char *cs, unsigned a, const char *want, const char *what);
SimTime e_low_time(Bench *b, const char *pin, SimTime limit, SimTime step);
void e_motorola_dual(Bench *b);
void e_powersave(Bench *b, int indicator);

#endif
