/* models/serial_i82510_int.h: what the pieces of models/serial_i82510.c share (plan-68-I/J split it). */
#ifndef MODELS_SERIAL_I82510_INT_H
#define MODELS_SERIAL_I82510_INT_H

/*
 * i82510: the Intel 82510 asynchronous serial controller (plan-12-H section
 * 3.3; facts in plan-12-data/specs/classic-b.md section 82510,
 * decisions.md section 82510).
 *
 * Bus: Intel RD/WR with CS low (serial_bus.c); A2..A0 and the bank pointer
 * (GIR/BANK bits 6:5) pick one of the 35 registers of Table 9; DLAB (LCR
 * bit 7) switches bank 0 A0/A1 to BAL/BAH and bank 3 A0/A1 to BBL/BBH.
 * Addresses Table 9 leaves empty read X and ignore writes (P2).
 *
 * Clocks: CLK is counted. The strap sampled as RESET falls picks the mode
 * (RTS low: crystal, pin 8 is X2; else external clock, pin 8 is OUT2) and,
 * with an external clock, DTR low disables the divide by 2 (p8). The
 * system clock feeds BRGA and BRGB (16-bit, BRG or timer mode; BRGB can run
 * from BRGA's output or SCLK). TX and RX take their 16x clock from either
 * BRG, or a 1x clock from SCLK (CLCF). Wake-up defaults (Table 4) give the
 * 8250 path: BRGA / 2 -> BRGB / 5 -> 16x.
 *
 * Receiver: a voting filter over a 3/16 or 7/16 window (RMD.SWM) in front
 * of the line engine, the start check by majority or by all samples
 * (RMD.SSM), the DPLL that nudges the bit counter on each edge (unless
 * RMD.DPD), and the noisy-character flag; then control-character and
 * uLAN address recognition, the FIFO lock, the 1- or 4-deep RX FIFO with
 * per-character flags (RXF) and the overrun rule (the oldest goes).
 * Transmitter: the 4-deep TX FIFO (11 bits per entry), 5-9 bit characters,
 * software parity, quarter-bit stop lengths (Table 13), the manual,
 * semi-automatic and automatic CTS/RTS handshakes.
 *
 * Interrupts: six blocks with source enables (RIE, MIE, TMIE), GER, the
 * GSR, GIR's priority code and IPN, automatic or manual acknowledge (INT
 * low two system clocks), and the TX FIFO hold state (GIR read).
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_RLDV, T_RHDZ, T_TXD };
enum { K_IN, K_OUT, K_COUNT };
enum { P_TX, P_RX, P_INT, P_CTS, P_RTS, P_DTR, P_RI, P_DSR, P_DCD, P_COUNT };

enum { FIFO_MAX = 4 };

/* Register indices. */
enum {
    R_BAL, R_BAH, R_BBL, R_BBH, R_GER, R_LCR, R_MCR, R_ACR0, R_ACR1, R_FMD, R_TMD, R_IMD, R_RIE,
    R_RMD, R_CLCF, R_BACF, R_BBCF, R_PMD, R_MIE, R_TMIE, R_TXF, R_COUNT
};

/* RST / LSR bits. */
enum { S_CRF = 0x80, S_PCRF = 0x40, S_BKT = 0x20, S_BKD = 0x10, S_FE = 0x08, S_PE = 0x04, S_OE = 0x02 };
/* RXF bits. */
enum { F_ROK = 0x40, F_RXN = 0x20, F_RPE = 0x10, F_ACR = 0x08, F_BKF = 0x04, F_RFE = 0x02, F_RND = 0x01 };

typedef struct {
    const char *data[8], *addr[3], *read, *write, *select, *reset;
    const char *clock[K_COUNT];
    const char *pin[P_COUNT];
} I5Device;

typedef struct {
    SerialBusPins bus;
    size_t reset;
    size_t clock[K_COUNT];
    size_t pin[P_COUNT];
} I5Pins;

typedef struct {
    uint16_t data, known;
    uint8_t flags, flags_known;
} I5RxChar;

typedef struct {
    uint16_t data, known;   /* 9 data bits */
    uint8_t txf;            /* bit 7 address marker, bit 6 software parity */
} I5TxChar;

/* A BRG / timer. */
typedef struct {
    SerialBaud baud;
    int timer_run;          /* timer mode: counting */
    uint32_t timer_count;
    LogicValue out;         /* TA / TB level */
    int pulse;              /* timer mode: the output pulse (one system clock) */
} I5Brg;

typedef struct {
    SerialBase base;
    const I5Device *dev;
    const I5Pins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    SerialBus bus;
    SerialBusEvents events;
    SerialLine line;
    SerialFormat txf, rxf;

    int in_reset;
    int unknown;
    int crystal;            /* strap: crystal oscillator mode */
    int div2;               /* strap: system clock = CLK / 2 */
    LogicValue strap_x;     /* the strap read X */
    SuReg r[R_COUNT];
    unsigned bank;
    SuReg tmcr;             /* TMCR: the timer gates (bits 5:4) */
    /* Status. */
    uint8_t rst;            /* RST 7:1 (LSR 4:1 shares 4:1) */
    uint8_t rst_k;
    int rfir_ack;
    uint8_t msr_delta;
    LogicValue msr_level[4]; /* CTS DSR RI DCD complements (1 = active) */
    uint8_t tmst;           /* TBEx bit 1, TAEx bit 0 */
    /* FIFOs. */
    I5RxChar rx[FIFO_MAX];
    unsigned rx_count;
    I5TxChar tx[FIFO_MAX];
    unsigned tx_count;
    int rx_enabled, rx_locked, tx_enabled;
    int tx_hold_state;      /* the TX FIFO hold state (GIR read) */
    unsigned tx_stop_hold;  /* extra stop ticks (quarter-bit stop lengths) */
    LogicValue tsr;
    /* Interrupt pin. */
    int int_force_low;      /* system clocks of INT forced low */
    int int_latched;        /* manual acknowledge: INT held */
    /* Clocks. */
    LogicValue clk, sysclk, sclk;
    I5Brg brg[2];
    /* Receiver front end. */
    LogicValue raw[7];
    unsigned raw_at;
    LogicValue filtered;
    int noisy;
    int resync;
    int breaking;           /* RX saw a break, BkT on the first mark */
    int powered_down;
    int pd_sleep;
} I5State;

extern const char *const i82_TIMING[];
extern const char *const i82_KEYS[];
extern const char *const i82_BUS_KEYS[];
extern const char *const i82_CLOCK_KEYS[];
extern const char *const i82_SERIAL_KEYS[];
extern const uint8_t i82_RESET_VALUE[R_COUNT];
uint8_t i82_rv(const I5State *s, unsigned i);
void i82_update_format(I5State *s);
int i82_rfir(const I5State *s);
uint8_t i82_gsr_of(const I5State *s);
int i82_gir_block(const I5State *s);
uint8_t i82_gir_of(const I5State *s);
uint8_t i82_lsr_of(const I5State *s);
void i82_track_modem(I5State *s);
uint8_t i82_msr_of(const I5State *s, uint8_t *k);
void i82_update_outputs(I5State *s);
void i82_refresh(I5State *s);
void i82_serviced(I5State *s);
uint32_t i82_divisor_of(const I5State *s, unsigned which);
int i82_brg_mode(const I5State *s, unsigned which);
void i82_brg_load(I5State *s, unsigned which);
void i82_sysclk_edge(I5State *s, LogicValue level);
void i82_sclk_edge(I5State *s, LogicValue level);

#endif
