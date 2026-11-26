/* models/ns16550_exar_int.h: what the pieces of models/ns16550_exar.c share (plan-68-I/J split it). */
#ifndef MODELS_NS16550_EXAR_INT_H
#define MODELS_NS16550_EXAR_INT_H

/*
 * ns16550 Exar/MaxLinear extensions (plan-12-E): the ST16C/XR16C/XR16L/
 * XR16M/XR16V devices. Every ns_ext_* hook of ns16550_ext.c hands a device
 * with Exar keys (dev->x) to its ns_x_* twin here, and the core calls the
 * other ns_x_* hooks directly. Styles ("extended"):
 *
 *   absent    16550 core plus the Exar keys below (ST16C450/550/2550/554,
 *             XR16C2550, XR16V554)
 *   "ier5"    ST16C1450/1550: IER[5] special mode (ISR[5:4] status, MCR[2] to
 *             the RST pin, MCR[7] power-down, FCR[5:4] TX trigger)
 *   "exar650" the LCR = 0xBF bank (EFR, Xon1/2, Xoff1/2) with Exar's DLD, AFR
 *             and DREV/DVID (XR16L580/2551/2552, XR16M25xx/26xx,
 *             XR16V25xx/26xx, XR16V564/654)
 *   "exar854" the XR16C854/864 bank: FC/TRG, FCTR, EFR, Xon/Xoff; FLVL/EMSR
 *   "m752"    XR16M752: the 752 bank (TCR, TLR, FIFO Rdy) plus DLD
 *   "5g"      XR16V798/XR16L784: the flat A7..A0 map with the global block
 *
 * Sources: plan-12-data/specs/exar-a.md, exar-b.md, exar-c.md, exar-d.md,
 * with decisions.md and plan-12-E-ns16550-exar.md (Corrections).
 *
 * Exar TX trigger levels count the bytes in the TX FIFO: the THR interrupt
 * comes when the count falls below the level, or when the FIFO empties if it
 * never reached the level since the last reload. The core works in free
 * spaces, so ns_x_tx_threshold returns depth - level + 1 (count < level is
 * free >= depth - level + 1).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ns16550_int.h"

/* ---- Register decode --------------------------------------------------------------- */

typedef enum {
    XR_CORE, XR_X, XR_EFR, XR_XON1, XR_XON2, XR_XOFF1, XR_XOFF2, XR_DLD, XR_AFR, XR_DREV, XR_DVID, XR_TCR,
    XR_TLR, XR_FIFORDY, XR_FCTR, XR_FC, XR_TRG, XR_FLVL, XR_EMSR, XR_TXCNT, XR_TXTRG, XR_RXCNT, XR_RXTRG,
    XR_XCHAR, XR_MSRW, XR_THR_IMM
} XReg;

const NsExarDevice *exs_xd(const NsChannel *c);
NsExarStyle exs_style_of(const NsChannel *c);
int exs_kb(Reg r, unsigned bit);
uint8_t exs_word_mask(const NsChannel *c);
int exs_has_bank(const NsChannel *c);
int exs_loopback(const NsChannel *c);
unsigned exs_tx_free(const NsChannel *c);
int exs_mode1(const NsChannel *c);
LogicValue exs_rxrdy_pin(const NsChannel *c);
LogicValue exs_txrdy_pin(const NsChannel *c);
void exs_set_format_rate(NsChannel *c);
void exs_sample_straps(NsState *s, NsChannel *c);
XReg exs_decode(const NsChannel *c, unsigned address, int write);
void exs_queue_flow_pair(NsChannel *c, unsigned txmode, int xon);
int exs_table_of(const NsChannel *c);
int exs_rx_trigger_x(const NsChannel *c);
unsigned exs_tx_level_bytes(const NsChannel *c);
int exs_opt_choice(KindContext *ctx, const cJSON *json, const char *key,
                      const char *const *values, int *out);
int exs_auto_rts_on(const NsChannel *c);
int exs_flow_on_dtr(const NsChannel *c);
void exs_dma_requests(NsChannel *c);
int exs_rs485_enabled(const NsChannel *c);
void exs_timer_step(NsState *s);
void exs_update_isolation(NsState *s);
uint8_t exs_fstat(const NsState *s);

#endif
