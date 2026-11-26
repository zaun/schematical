#ifndef CONVERTER_BENCH_H
#define CONVERTER_BENCH_H

/*
 * converter_bench (plan-68-M): fixture loading and a pin-by-pin bench for
 * converter-kind parts: a strong driver on every pin, weak pull-ups and
 * pull-downs from time 0, SPI frames (modes 0 and 1), I2C transactions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "parts.h"

#define CB_DUAL(ID, PINS) \
    "{'id':'" ID "','description':'d','style':'dual','pins':" PINS ",'body':{'width':1,'height':1}," \
    "'pitch':1,'span':2,'pad':{'width':0.5,'height':0.5,'shape':'rect'}}"
#define PIN(N, NAME, ROLE) "{'number':" #N ",'name':'" NAME "','role':'" ROLE "'}"
#define HEADX(ID, FUNC) "{'id':'" ID "','function':'" FUNC "','description':'d','drawer':['Interface'],'pinouts':{'p':["
#define PART(NAME, FP) "'parts':[{'name':'" NAME "','package':'SOIC','pinout':'p','footprint':'" FP "'}]}"
#define FILE_OF(DEVICES) "{'format':'schematical-parts','version':1,'kind':'converter','devices':[" DEVICES "]}"

typedef struct {
    size_t count;
    LogicValue level;
} PinCount;

typedef struct {
    Sim *sim;
    SimInstance *chip, *drv, *pu, *pd;
    const Part *part;
    PinCount pins, ups, downs;
    const char *cs, *sclk, *mosi, *miso, *scl, *sda;
} Bench;

enum { HALF = 100000 };     /* 100 ns: a 5 MHz bus */

extern int failures;
extern PartLibrary *library;

void check(int ok, const char *what);
char *json(const char *text);
int load_converters(const char *devices);
size_t bpin(const Bench *b, const char *name);
void set(Bench *b, const char *name, LogicValue v);
LogicValue get(const Bench *b, const char *name);
void run(Bench *b, SimTime ps);
/* ups / downs: pins given a weak pull-up / pull-down (from time 0); NULL-terminated or NULL. */
Bench *bench(const char *part, const char *const *ups, const char *const *downs);
void close_bench(Bench *b);
/* One SPI frame of n bits in mode 0 or 1 (MSB first) on cs/sclk/mosi/miso; the MISO bits. */
uint32_t spi(Bench *b, unsigned mode, uint32_t out, unsigned n, int deselect);
/* I2C on scl/sda: 1 releases the line, 0 drives it low. */
void line(Bench *b, const char *pin, int level);
void i2c_start(Bench *b);
void i2c_stop(Bench *b);
LogicValue i2c_bit(Bench *b, int out);
int i2c_write(Bench *b, unsigned byte);
unsigned i2c_read(Bench *b, int ack);
int i2c_tx(Bench *b, unsigned address, const unsigned *bytes, size_t count);
int i2c_rx(Bench *b, unsigned address, unsigned *bytes, size_t count);
/* Four data pins, MSB first; *z: all of them Z. */
uint32_t bus4(Bench *b, const char *const *pins, int *z);

#endif
