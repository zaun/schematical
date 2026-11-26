/* models/ns16550_bridge_int.h: what the pieces of models/ns16550_bridge.c share (plan-68-I/J split it). */
#ifndef MODELS_NS16550_BRIDGE_INT_H
#define MODELS_NS16550_BRIDGE_INT_H

/*
 * ns16550 I2C/SPI-to-UART bridges (plan-12-I section 3.2): the SC16IS740,
 * SC16IS750/760 and SC16IS752/762 (NXP), the PI7C9X760/762 (Diodes, the
 * SC16IS map plus the SFR bank) and the XR20M1170 (MaxLinear, plus DLD);
 * the XR20M1280's own map is in ns16550_xr1280.c.
 *
 * The host port is serial_target.c's I2C/SPI target, with the NXP sub-address
 * byte (I2C) or command byte (SPI): register bits [6:3] (or [5:3]), channel
 * bits [2:1], SPI bit 7 = 1 read. No auto-increment: every data byte of a
 * transfer goes to the same register (the FIFO burst), except with PI7C9X
 * SFR[4] = 1. Registers 8-15: TXLVL, RXLVL, IODir, IOState, IOIntEna,
 * reserved, IOControl, EFCR; the 650-style bank at LCR = 0xBF; TCR/TLR at 6/7
 * with EFR[4] and MCR[2]; with LCR[7] = 1 (not 0xBF) only DLL, DLH, LCR (and
 * the XR20M1170's DLD) answer, other addresses read X (decisions.md).
 *
 * Sources: plan-12-data/specs/bridges.md, matrix.md Table A rows 54-62 and
 * Table B, decisions.md § bridges.md and the planner decisions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ns16550_bridge.h"

int brs_is_xr(const NsDevice *d);
int brs_enhanced(const NsChannel *c);
void brs_gpio_levels(const NsState *s, uint8_t *v, uint8_t *k);
LogicValue brs_gpio_is_modem(const NsState *s, unsigned b);
void brs_reset_registers(NsState *s, int power_on);
LogicValue brs_sc_gpio_pending(const NsState *s);
void brs_sc_gpio_changed(NsState *s, unsigned b);
void brs_i2c_begin(void *context, int repeated);
LogicValue brs_i2c_address(void *context, uint8_t byte, uint8_t known);
LogicValue brs_i2c_received(void *context, unsigned index, uint8_t data, uint8_t known);
void brs_i2c_next(void *context, unsigned index, uint8_t *data, uint8_t *known);
void brs_i2c_acked(void *context, unsigned index, LogicValue ack);
void brs_spi_begin(void *context);
SerialSpiByte brs_spi_next(void *context, unsigned index);
void brs_spi_received(void *context, unsigned index, uint8_t data, uint8_t known);

#endif
