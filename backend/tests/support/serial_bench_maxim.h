#ifndef SERIAL_BENCH_MAXIM_H
#define SERIAL_BENCH_MAXIM_H

#include "serial_bench.h"

/* ==== plan-12-J: the Maxim SPI/I2C UARTs and the SPI/I2C target front ends ==========
 *
 * JF1/JF2 drive the front ends of serial_target.h directly (a test target
 * that logs its events); J1-J6 run the max3100 and max310x models on the
 * shipped device objects of support/serial_fixture_maxim.h. The bench: one
 * net per pin with a driver on each, an optional clock on XIN/X1, weak
 * pull-ups on the I2C lines, and pins tied onto another pin's net (I2C
 * address straps). SPI is bit-banged in mode 0, I2C at 400 kHz with the
 * bench's SDA/SCL open drain (0 or Z).
 */

#include "serial_fixture_maxim.h"
#include "serial_target.h"

typedef struct {
    size_t count;
} PullConfig;
size_t pull_pins(const void *config);
void pull_start(Sim *sim, SimInstance *instance, void *state);
void *pull_create(const void *config);
extern const SimComponentType PULLUP;

typedef struct {
    Sim *sim;
    SimInstance *chip;
    SimInstance *drv;
    SimInstance *clk;
    SimInstance *pu;
    const Part *part;
    size_t pins;
    SimTime half;
    SimTime period;
    PullConfig pull;
    const char *cs, *sclk, *mosi, *miso, *scl, *sda;
} MB;
extern int maxim_loaded;
void maxim_need(void);
size_t mb_pin(const MB *b, const char *name);
void mb_set(MB *b, const char *name, LogicValue v);
LogicValue mb_get(const MB *b, const char *name);
void mb_run(MB *b, SimTime ps);
MB *mb_open(const char *part_name, const char *clock, SimTime half,
                   const char *const *ties, const char *const *pulls);
void mb_close(MB *b);
void mb_expect_pin(MB *b, const char *pin, LogicValue want, const char *what);
void mb_expect_u(unsigned got, unsigned want, const char *what);
size_t mb_sent(MB *b, unsigned channel, uint16_t *frames, size_t cap);
int mb_send(MB *b, unsigned channel, const char *text);

/* ---- SPI master (mode 0) ----------------------------------------------------------- */

#define SPI_HALF NS(250)
uint32_t mb_spi(MB *b, uint32_t out, unsigned n, char *text, int keep);

/* ---- I2C master (bench SDA/SCL open drain at 400 kHz) ---------------------------------------- */

#define I2C_Q NS(625)
void i2c_line(MB *b, const char *pin, int level);
void i2c_start(MB *b);
void i2c_restart(MB *b);
void i2c_stop(MB *b);
LogicValue i2c_bit(MB *b, int out);
LogicValue i2c_write(MB *b, unsigned byte);
unsigned i2c_read(MB *b, int ack, char *text);
long long mb_wait(MB *b, const char *pin, LogicValue level, SimTime limit, SimTime step);
void mb_frame(MB *b, const char *pin, SimTime bt, unsigned n, char *out);
void mb_drive_bits(MB *b, const char *pin, const char *bits, SimTime bt, LogicValue idle);
void mb_drive_byte(MB *b, const char *pin, unsigned v, SimTime bt, int stop);

/* ---- JF: the front ends, direct ----------------------------------------------------------- */

enum { TG_CS, TG_SCLK, TG_MOSI, TG_MISO, TG_SCL, TG_SDA, TG_A0, TG_A1, TG_PINS };

typedef struct {
    SerialSpiPins sp;
    SerialI2cPins ip;
    SerialSpi spi;
    SerialI2c i2c;
    SerialSpiEvents se;
    SerialI2cEvents ie;
    char log[1024];
    uint8_t mem;
    int ties[2];
} Tgt;
void tg_log(Tgt *t, const char *format, unsigned a, unsigned b);
void tg_spi_begin(void *c);
SerialSpiByte tg_spi_next(void *c, unsigned index);
void tg_spi_received(void *c, unsigned index, uint8_t data, uint8_t known);
void tg_spi_end(void *c, unsigned bits);
void tg_i2c_begin(void *c, int repeated);
LogicValue tg_i2c_address(void *c, uint8_t byte, uint8_t known);
LogicValue tg_i2c_received(void *c, unsigned index, uint8_t data, uint8_t known);
void tg_i2c_next(void *c, unsigned index, uint8_t *data, uint8_t *known);
void tg_i2c_acked(void *c, unsigned index, LogicValue ack);
void tg_i2c_end(void *c, int stop);
size_t tg_pin_count(const void *config);
void *tg_create(const void *config);
void tg_start(Sim *sim, SimInstance *instance, void *state);
void tg_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                           LogicValue old_value, LogicValue new_value);
extern const SimComponentType TARGET;

/* A bench around the test target: SCL/SDA pulled up, A0 tied to SCL, A1 to SDA. */
typedef struct {
    MB b;
    PartPin pin_table[TG_PINS];
    Part part;
} TgBench;
TgBench *tg_open(void);
Tgt *tg_state(TgBench *t);
long long m3_start_bit(MB *b, const char *pin, SimTime limit);
int within(long long got, long long want, long long tolerance);

#endif
