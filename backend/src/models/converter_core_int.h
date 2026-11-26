/* models/converter_core_int.h: what the pieces of models/converter.c share (plan-68-I/J split it). */
#ifndef MODELS_CONVERTER_CORE_INT_H
#define MODELS_CONVERTER_CORE_INT_H

/*
 * The converter kind (plan-24-D): runs a CvDevice (converter.h,
 * planning/plan-24-data/schema.md) on the engine.
 *
 *   Protocol engine   bits in from the bus are matched against the
 *                     device's command patterns (fixed or stream frames,
 *                     end-aligned decoding, commit at the end); bits out are
 *                     shifted from the output pattern, a command's reply, or
 *                     the pending `next` pattern.
 *   SPI front end     CS (optional, either polarity), SCLK in modes 0-3, MOSI
 *                     and MISO (either optional, or one SDIO pin), MISO
 *                     doubling as a ready pin. Bit level, so frames of any
 *                     length work. CS-less 2-wire parts frame on conversions.
 *   I2C front end     the plan-12 SerialI2c target, unchanged: the device
 *                     claims its address (strap table, a part's fixed
 *                     address, the general call), write bytes feed the
 *                     engine, reads shift its output.
 *   Conversions       start triggers, timing by setting, single/continuous,
 *                     result registers, busy/done fields, ready pins.
 *   DAC registers     input -> DAC copies on LDAC (pin, command or sync),
 *                     clear codes.
 *   Control pins      reset, convst, start, powerdown, ldac, clear, level
 *                     copies, strap-selected reset values.
 *
 * Signal rules: an X or Z on a data input arrives as an unknown bit, which
 * makes the rest of the transaction X (nothing is executed, MISO drives X).
 * CS and control pins at Z read inactive. The result code is the 0 V code
 * (plan-24-A §3.3). Parallel and clocked buses: converter_par.c (plan-24-E).
 */

#include "converter.h"

#include <stdlib.h>
#include <string.h>

#include "converter_int.h"
#include "converter_state.h"
#include "model_pins.h"

void cvc_field_put(CvState *s, const CvField *f, uint32_t value);
int cvc_reg_at(const CvState *s, int space, uint32_t address);
int cvc_reg_next(const CvState *s, int reg);
void cvc_ready_clear(CvState *s, unsigned why);
int cvc_continuous(const CvState *s);
void cvc_conv_stop(CvState *s);
void cvc_conv_done(CvState *s);
void cvc_dac_update(CvState *s, int ch);
void cvc_dac_clear(CvState *s);
void cvc_mark_read(CvState *s, int reg);
void cvc_set_out(CvState *s, CvSeg *seg, const CvPattern *pat, const uint32_t *caps,
                    const int *at, size_t at_count, unsigned at_word, int sdio);
size_t cvc_reg_run(const CvState *s, int first, size_t count, int *out);
void cvc_run_action(CvState *s, uint8_t action, const CvCommand *cmd, const uint32_t *caps);
void cvc_execute(CvState *s, const CvCommand *cmd, const uint32_t *caps);
const CvBus *cvc_spi_bus(const CvState *s);
void cvc_spi_select(CvState *s, const CvBus *b);
void cvc_spi_pin(CvState *s, const CvBus *b, size_t pin);
const CvBus *cvc_i2c_bus(const CvState *s);
void cvc_i2c_begin(void *ctx, int repeated);
LogicValue cvc_i2c_address(void *ctx, uint8_t byte, uint8_t known);
LogicValue cvc_i2c_received(void *ctx, unsigned index, uint8_t data, uint8_t known);
void cvc_i2c_next(void *ctx, unsigned index, uint8_t *data, uint8_t *known);
void cvc_i2c_end(void *ctx, int stop);

#endif
