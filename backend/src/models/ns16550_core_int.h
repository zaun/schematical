/* models/ns16550_core_int.h: what the pieces of models/ns16550.c share (plan-68-I/J split it). */
#ifndef MODELS_NS16550_CORE_INT_H
#define MODELS_NS16550_CORE_INT_H

/*
 * ns16550: the 16450/16550 UART family (plan-12-C section 3.6). C's core is
 * the PC16550D (plan-12-data/specs/ti-a.md section PC16550D, with
 * decisions.md section PC16550D); "fifo": 0 makes it a 16450 (no FCR,
 * IIR[7:6] = 00, no timeout). D and E extend it with option keys.
 *
 * Signal rules (plan-12-A section 3.5): power-on is the MR state, then the
 * reset pin applies at start; DLL, DLM and RBR, which MR does not reset,
 * power on X. Inputs at Z read X, except MR and the chip selects (inactive).
 * Every register holds (value, known); a bus read drives X for unknown bits.
 *
 * Timing: bus delays from the grade (trvd, thz, trdd); BAUDOUT tbld after
 * the clock edge; interrupt and status pins after the bus access that
 * changed them (trint, thr, tir, trim, tmdo, twxi, twrr) or tsim after a
 * modem input; serial events act at the 16x tick they happen on. A received
 * character reaches the RBR/FIFO 1 RCLK after its stop-bit sample (3 in FIFO
 * mode, PC16550D p4 note 2); the timeout fires 4 character times plus 8
 * RCLKs after the last character or read.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ns16550_int.h"

/* ---- Interrupts ------------------------------------------------------------------------ */

/* Each source: pending AND enabled, as 0 / 1 / X. */
typedef struct {
    LogicValue ls, rda, timeout, thre, modem, xoff, ctsrts, gpio;
    LogicValue wake;   /* plan-12-E: the wake-up indicator (INT only; IIR reads none) */
} Sources;

extern const char *const nsc_TIMING[];
extern const char *const nsc_KEYS[];
extern const char *const nsc_BRIDGE_KEYS[];
extern const char *const nsc_CHANNEL_KEYS[];
extern const char *const nsc_CHANNEL_EXTRA_KEYS[];
void nsc_drive_baudout(NsState *s, LogicValue value, SimTime delay);
int nsc_loopback(const NsChannel *c);
void nsc_update_format(NsChannel *c);
void nsc_load_divisor(NsChannel *c);
int nsc_char_has_error(const SerialRxChar *c);
void nsc_char_lsr_bits(const SerialRxChar *rc, uint8_t *v, uint8_t *k);
void nsc_show_errors(NsChannel *c, const SerialRxChar *rc);
unsigned nsc_tx_free(const NsChannel *c);
void nsc_iir_of(const NsChannel *c, uint8_t *v, uint8_t *k);
void nsc_update_channel_outputs(NsState *s, NsChannel *c, Cause cause);
void nsc_update_shared_dma(NsState *s, Cause cause);
void nsc_update_outputs(NsState *s, Cause cause);
void nsc_update_dma_latches(NsChannel *c);
void nsc_set_modem(NsChannel *c, const LogicValue now[4], uint8_t record);
void nsc_modem_now(const NsState *s, const NsChannel *c, LogicValue now[4]);
void nsc_reset_all(NsState *s);
unsigned nsc_ns_channels(const void *raw);
SerialLine *nsc_ns_line(void *raw, unsigned channel);
int nsc_ns_async(const void *raw, unsigned channel);
void *nsc_ns_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options);
void nsc_ns_destroy(void *raw);
void nsc_rx_tick(NsState *s, NsChannel *c);
void nsc_tx_tick(NsState *s, NsChannel *c);
void nsc_reg_peek(NsChannel *c, unsigned address, uint8_t *v, uint8_t *k);
Cause nsc_reg_read_done(NsChannel *c, unsigned address);
Cause nsc_reg_write(NsState *s, NsChannel *c, unsigned address, uint8_t data, uint8_t known);
LogicValue nsc_bus_select(void *context, const Sim *sim, const SimInstance *instance,
                             unsigned address, unsigned address_known, uint32_t *units);
void nsc_bus_peek(void *context, unsigned unit, unsigned address, uint8_t *data, uint8_t *known);
void nsc_bus_read_done(void *context, unsigned unit, unsigned address);
void nsc_bus_write(void *context, unsigned unit, unsigned address, uint8_t data, uint8_t known);
int nsc_load_bus(KindContext *ctx, const cJSON *json, NsDevice *d);
int nsc_load_clock(KindContext *ctx, const cJSON *json, NsDevice *d);
int nsc_load_channels(KindContext *ctx, const cJSON *json, NsDevice *d);
int nsc_load_choice(KindContext *ctx, const cJSON *json, const char *key,
                       const char *const *values, int fallback, int *out);
int nsc_load_optional_bool(KindContext *ctx, const cJSON *json, const char *key, int *out);
int nsc_load_levels(KindContext *ctx, const cJSON *json, const char *key, unsigned max,
                       unsigned out[4]);
int nsc_load_byte(KindContext *ctx, const cJSON *json, const char *key, int fallback, int *out);
int nsc_load_phase2(KindContext *ctx, const cJSON *json, NsDevice *d);

#endif
