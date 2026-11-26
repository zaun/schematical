#ifndef SERIAL_BENCH_BRIDGE_H
#define SERIAL_BENCH_BRIDGE_H

#include "serial_bench.h"
#include "serial_bench_maxim.h"

/*
 * plan-12-I cases I1-I9: the I2C/SPI-to-UART bridges on the ns16550 model
 * and the sc18is604 SPI-to-I2C bridge, on the shipped device objects
 * (tests/fixtures/serial/bridges.json, merge.py). Included by serial_test.c
 * after the plan-12-J block (it uses its MB bench, SPI and I2C masters and
 * the JF test target).
 *
 * The bridges run on a 10 MHz XTAL1 (bit time 1.6 us x divisor). I2C is the
 * bench master at 400 kHz with weak pull-ups on SDA, SCL and IRQ; SPI is
 * bit-banged in mode 0 (mode 3 for the SC18IS604).
 */

#define XT_HALF NS(50)
#define BIT1 NS(1600)

typedef struct {
    MB *b;
    int spi;
    unsigned addr;        /* 7-bit I2C address */
    const char *a0, *a1;
} Br;
extern const char *const BR_PULLS_SC[];
extern const char *const BR_PULLS_XR[];
void br_reset(Br *r);
Br br_open(const char *part, int spi, int xr, const char *const *ties);
unsigned br_sub(unsigned reg, unsigned ch);
LogicValue br_w(Br *r, unsigned ch, unsigned reg, unsigned v);
unsigned br_r(Br *r, unsigned ch, unsigned reg, char *text);
void br_expect(Br *r, unsigned ch, unsigned reg, unsigned want, const char *what);
void br_expect_text(Br *r, unsigned ch, unsigned reg, const char *want, const char *what);
void br_program(Br *r, unsigned ch, unsigned d);
void br_spi_frame(MB *b, const uint8_t *out, unsigned n, uint8_t *in, char *text);
long long br_width(MB *b, const char *pin, LogicValue level, SimTime limit);
int near(long long got, long long want, long long tol);
void expect_near(long long got, long long want, long long tol, const char *what);
long long br_bit_time(MB *b, const char *pin);
void br_frame(MB *b, const char *pin, SimTime bt, char *out, SimTime limit);
unsigned nxp_address(int a1, int a0);
unsigned xr_address(int a1, int a0);
void address_table(const char *part, int xr);

#endif
