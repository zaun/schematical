/* models/pio_via6522_int.h: what the pieces of models/pio_via6522.c share (plan-68-I/J split it). */
#ifndef MODELS_PIO_VIA6522_INT_H
#define MODELS_PIO_VIA6522_INT_H

/*
 * via6522: the WDC W65C22 versatile interface adapter (plan-14-C, D;
 * datasheets/pio/w65c22.pdf).
 *
 * 16 registers (Table 2-1): ORB/IRB, ORA/IRA (with handshake), DDRB, DDRA,
 * T1 counter and latches, T2 counter and low latch, SR, ACR, PCR, IFR, IER,
 * ORA/IRA without handshake. Writes act at the PHI2 fall (bus65); port
 * outputs follow tcpw later. Read side effects (flag clears, the CA2 read
 * strobe) act at the PHI2 fall of the read.
 *
 * Ports (section 2.1): DDR 1 drives from OR, DDR 0 floats. IRA reads the PA
 * pins (every bit); IRB reads ORB for outputs and the pins for inputs. With
 * latching (ACR bits 0, 1) a read returns the pins as they were at the last
 * active CA1/CB1 edge; once read, the register is transparent again until
 * the next edge (p.8). A pin that changed less than tpcr before the read's
 * rise reads X; a latched pin that changed less than til before the edge,
 * or within tpdh after it, latches X. With ACR bit 7, T1 drives PB7 and IRB
 * bit 7 reads its level.
 *
 * Control lines (PCR, Table 2-5): CA1/CB1 set IFR1/IFR4 on the active edge.
 * CA2/CB2: input (IFR0/IFR3 on the selected edge; cleared by an ORA/ORB
 * access unless "independent"), handshake output, pulse output, low, high.
 *
 * Timers (sections 2.5-2.10): each counter decrements at every PHI2 fall
 * but the one that loaded it. A count that wraps from 0 times out at the
 * next PHI2 rise: N + 1.5 cycles after the start write's fall. T1 one-shot
 * sets IFR6 once per start and keeps rolling; T1 free-run reloads from the
 * latch at the next fall (a period of L + 2 cycles) and toggles PB7. T2
 * one-shot as T1 one-shot; T2 pulse counting decrements once per PB6 low
 * sampled at a PHI2 rise after a high one, and sets IFR5 when it reaches 0.
 * PB6 pulses narrower than, or closer than, 2 PHI2 cycles make it X.
 *
 * Shift register (sections 2.11-2.13, ACR bits 4-2): bit 7 shifts out
 * first on CB2 and rotates into bit 0; bits shift in at bit 0. With T2
 * (001, 100, 101) CB1 toggles every N + 2 PHI2 cycles (N = the T2 low
 * latch); with PHI2 (010, 110) it is low one cycle and high the next. An
 * output bit leaves tsr1 after the PHI2 fall where CB1 falls; an input bit
 * is sampled at the PHI2 rise after CB1 rises (X if CB2 changed less than
 * tsr2 before it) and shifted in at that cycle's fall. 001, 010, 101, 110
 * stop after 8 bits and set IFR2; 100 never stops and never sets IFR2.
 * External clock (011 in on CB1 rises, 111 out on CB1 falls) shifts at the
 * PHI2 fall after the edge, sets IFR2 every 8 bits and keeps going; an edge
 * less than tpcr before that fall makes the SR and its count X. An SR
 * access restarts the count, clears IFR2 and starts the internal clock.
 *
 * IRQB is asserted while IFR & IER & 0x7F is not 0, with no delay; open
 * drain ("irqOpenDrain": W65C22N) or totem pole (W65C22S).
 *
 * RESB low clears every register but the T1 and T2 counters and latches and
 * the SR, makes every pin an input and stops the timers (they restart on
 * their start write). RESB X makes everything X. Registers power up X.
 * "busHold" (W65C22S): an input at Z reads the last level it had.
 */

#include <stdlib.h>
#include <string.h>

#include "pio.h"

enum {
    T_TCYC, T_TPWH, T_TPWL, T_TACR, T_TCAR, T_TCDR, T_THR, T_TPCR, T_TACW, T_TCAW, T_TDCW, T_THW,
    T_TCPW, T_TCA2, T_TRS1, T_TRS2, T_TWHS, T_TDS, T_TRS3, T_TRS4, T_T21, T_TIL, T_TSR1, T_TSR2,
    T_TAL, T_TPDH, T_COUNT
};

enum { MAX_PINS = 64 };

typedef struct {
    PioNames names;
    const char *irq;
    int irq_open_drain;
    int bus_hold;
} ViaDevice;

typedef struct {
    PioPins p;
    size_t irq;
    const ViaDevice *device;
} ViaPins;

enum {
    R_ORB, R_ORA, R_DDRB, R_DDRA, R_T1CL, R_T1CH, R_T1LL, R_T1LH, R_T2CL, R_T2CH, R_SR, R_ACR,
    R_PCR, R_IFR, R_IER, R_ORA_NH
};

enum {
    F_CA2 = 1 << 0, F_CA1 = 1 << 1, F_SR = 1 << 2, F_CB2 = 1 << 3, F_CB1 = 1 << 4,
    F_T2 = 1 << 5, F_T1 = 1 << 6
};

/* C2 modes from PCR bits 3-1 (CA2) / 7-5 (CB2). */
enum { C2_IN_NEG, C2_IND_NEG, C2_IN_POS, C2_IND_POS, C2_HANDSHAKE, C2_PULSE, C2_LOW, C2_HIGH,
       C2_UNKNOWN };

/* A 16-bit counter with its known bits. */
typedef struct {
    uint16_t v, k;
} Word;

/* One side's port and control lines (0 = A, 1 = B). */
typedef struct {
    uint8_t out, out_k;       /* OR */
    uint8_t ddr, ddr_k;
    uint8_t latch, latch_k;   /* IRA/IRB as latched at the C1 edge */
    int latched;              /* latch holds a value (else transparent) */
    SimTime latch_at;
    LogicValue c1, c2;        /* input levels last seen */
    LogicValue c2_out;        /* handshake/pulse output level */
    int arm;                  /* written: the write strobe acts at the next rise */
    int pulse_fall;           /* CA2 read pulse: high at the next fall */
    int pulse_rise;           /* write pulse: high at the next rise */
    int strobe_read;          /* the low came from a read handshake (CA2 only) */
    int read_or;              /* RS 1 (A) / RS 0 (B) read this cycle: clears flags at the fall */
    SimTime port_at[PIO_PORT_BITS];
    int port_seen[PIO_PORT_BITS];
} ViaSide;

typedef struct {
    PioBase base;
    Bus65 bus;
    const ViaPins *pins;
    const SimTime *t;
    Sim *sim;
    SimInstance *instance;
    LogicValue reset;
    LogicValue phi2;
    SimTime last_rise, period;  /* the measured PHI2 period */

    ViaSide side[2];
    uint8_t acr, acr_k, pcr, pcr_k;
    uint8_t ifr, ifr_k, ier, ier_k;   /* bits 0-6 */
    uint8_t sr, sr_k;
    int sr_run;                /* the internal shift clock runs */
    unsigned sr_div;           /* PHI2 falls since the last T2-rate toggle */
    int sr_count;              /* bits shifted since the access; -1 = X */
    LogicValue cb1_out;        /* the shift clock CB1 drives */
    LogicValue cb2_sr;         /* the serial output CB2 drives */
    int sr_sample_pending;     /* input: sample CB2 at the next rise */
    int sr_shift_pending;      /* input: shift the sample in at the next fall */
    LogicValue sr_sample;
    int sr_access;             /* the SR was read or written this cycle */
    int sr_ext;                /* an external CB1 edge waits for the next fall */
    SimTime sr_ext_at;
    SimTime cb2_at;
    int cb2_seen;
    uint8_t pending_clear, pending_x; /* IFR bits a read clears (or makes X) at the fall */

    Word t1c, t1l, t2c;
    uint8_t t2ll, t2ll_k;
    int t1_run, t2_run;        /* counting: 1, 0, or -1 unknown */
    int t1_loaded, t2_loaded;  /* loaded at this fall: no decrement */
    int t1_armed, t2_armed;    /* the next time-out sets the flag: 1, 0, -1 X */
    int t1_timeout;            /* wrapped: times out at the next rise */
    int t1_reload;             /* free-run: reload at the next fall */
    int t2_timeout;
    LogicValue pb7;            /* T1's PB7 output */
    LogicValue pb6_rise;       /* PB6 at the last rise (pulse counting) */
    SimTime pb6_fall_at, pb6_high_at;   /* PB6's last falling / rising edge */
    int pb6_seen_fall, pb6_seen_high;

    LogicValue held[MAX_PINS];  /* bus hold */
} Via;

/* Shift register modes (ACR bits 4-2). */
enum { SR_OFF, SR_IN_T2, SR_IN_PHI2, SR_IN_EXT, SR_OUT_FREE, SR_OUT_T2, SR_OUT_PHI2, SR_OUT_EXT };

LogicValue v22_bitv(uint8_t v, uint8_t k, int bit);
void v22_set_bit(uint8_t *v, uint8_t *k, int bit, LogicValue x);
void v22_flag_set(Via *s, uint8_t mask, int x);
void v22_flag_clear(Via *s, uint8_t mask);
void v22_flag_maybe_clear(Via *s, uint8_t mask);
LogicValue v22_pin_level(const Via *s, size_t pin);
const size_t *v22_port_pins(const Via *s, int side);
int v22_c2_mode(const Via *s, int side);
void v22_update_irq(Via *s);
LogicValue v22_irq_bit(const Via *s);
LogicValue v22_acr_bit(const Via *s, int bit);
void v22_drive_ports(Via *s, int side, SimTime delay);
void v22_drive_pb7(Via *s, SimTime delay);
int v22_sr_mode(const Via *s);
int v22_sr_internal(int m);
int v22_sr_t2(int m);
void v22_drive_cb1(Via *s, SimTime delay);
void v22_drive_c2(Via *s, int side, SimTime delay);
void v22_set_c2_out(Via *s, int side, LogicValue v, SimTime delay);
int v22_recent(const ViaSide *p, int bit, SimTime now, SimTime window);
int v22_via_read(void *model, unsigned reg, uint8_t *value, uint8_t *known);
void v22_via_read_unknown(void *model);
uint8_t v22_or_access_flags(const Via *s, int side);
void v22_via_write(void *model, unsigned reg, uint8_t value, uint8_t known);
void v22_t1_fall(Via *s);
void v22_t1_rise(Via *s);
void v22_t2_fall(Via *s);
void v22_t2_rise(Via *s);
void v22_pb6_edge(Via *s, LogicValue v);
void v22_sr_fall(Via *s);
void v22_sr_start(Via *s);
void v22_sr_rise(Via *s);

#endif
