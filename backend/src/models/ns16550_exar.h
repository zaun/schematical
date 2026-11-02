#ifndef NS16550_EXAR_H
#define NS16550_EXAR_H

/*
 * plan-12-E: the ns16550 model's Exar/MaxLinear extensions (ns16550_exar.c).
 * Included by ns16550_int.h after Reg; the types here hang off NsDevice
 * (`x`), NsPins (`xp`), NsChannel (`xc`) and NsState (`xs`).
 */

#include <stddef.h>
#include <stdint.h>

/* The register style, from "extended". */
typedef enum { XS_NONE, XS_IER5, XS_650, XS_854, XS_M752, XS_5G } NsExarStyle;

/* "mfPins" functions for AFR[2:1] = 00..11. */
typedef enum { MF_OP2, MF_BAUDOUT, MF_RXRDY, MF_X } NsMfFunction;

typedef struct NsExarDevice {
    NsExarStyle style;
    /* Bus ("intel-motorola"). */
    int motorola_bus;
    const char *mode_pin;          /* 16/68: high = Intel */
    const char *rw;                /* Motorola R/W (an Intel strobe pin) */
    const char *strobe;            /* Motorola CS (default: the first select) */
    const char *chan_bit[2];       /* Motorola channel bits A3, A4 */
    unsigned chan_bits;
    const char *irq;               /* Motorola IRQ (open drain), an INT pin */
    int irq_others_low;            /* the other INT pins drive 0 (else Z) */
    const char *interrupt;         /* 5G: the device INT# (open drain) */
    /* Registers. */
    int dld;                       /* DLD: fraction n/16 (+ 16/8/4x on 650/752) */
    int dld_sampling;              /* DLD[5:4] picks the sampling rate */
    unsigned rx_table[3][4], tx_table[3][4];
    int has_tables;                /* "triggerTables": A-C, D programmable */
    int fc_flvl;                   /* "fifoCounters": "fcFlvl" */
    int start_75;                  /* "startSample": 7.5 */
    int thr_ier1;                  /* "thrIntClear": "ier1Rewrite" */
    int isr_inverted;              /* "isrStatus": "inverted" (ier5) */
    const char *reset_out;         /* "resetOut": RST = RESET OR (IER[5] AND MCR[2]) */
    int divisor_power_up;          /* "divisorPowerUp": DLL at power-up (DLM 0); -1 none */
    int sleep_divisor;             /* "sleepNeedsDivisor" */
    int wake_indicator;            /* "wakeIndicator" */
    const char *pwrsave;           /* "powerSave": {"pin"} */
    int mcr2_enhanced;             /* "mcr2Enhanced": EFR[4] also gates MCR[2] */
    int xon_write_only;            /* "xonWriteOnly": Xon/Xoff reads follow the normal bank */
    int flow_loopback;             /* "autoFlowLoopback": auto RTS/CTS work in loopback */
    int break_hold;                /* "lsrBreakHold": LSR[4] holds while RX stays low */
    int status_read_only;          /* "statusWrite": false */
    int dlab_hides_status;         /* "dlabHidesStatus": LSR/MSR read X with LCR[7] = 1 */
    int afr0_per_channel;          /* "broadcastWrite": "afr0-perChannel" */
    NsMfFunction mf[4];            /* "mfPins" */
    int has_mf;
    int xon_any;                   /* "xonAny": "mcr5" */
    int irda_polarity;             /* "irda": "mcr6-exar": MCR[2] picks the RX polarity */
    const char *fstat;             /* "fstat": the FSRS pin */
    const char *dack[4], *aen, *tc;/* "dmaHost" (XR16C864) */
    int dma_host;
    const char *chc_clock;         /* "chcClock": channel C's clock pin */
    const char *bclk[8], *irtx[8]; /* channel "bclk" / "irtx" pins */
    const char *enir;              /* 5G: the ENIR strap (MCR[6] preset) */
    const char *tmrck;             /* 5G: "timer": {"clock"} */
    int timer_commands;            /* "timer": {"style": "commands"} (XR16V798) */
    int v798;                      /* "variant5g": "xr16v798" (else XR16L784) */
} NsExarDevice;

typedef struct NsExarPins {
    size_t mode, rw, strobe, chan_bit[2], irq, interrupt, reset_out, pwrsave, fstat;
    size_t dack[4], aen, tc, chc_clock, enir, tmrck;
    size_t bclk[8], irtx[8];
} NsExarPins;

/* Part options ("intPin" part key). */
typedef struct NsExarPart {
    int int_pin;                   /* an NsIntPin value; -1 = the device's */
} NsExarPart;

typedef struct NsExarChannel {
    Reg dld, fctr, emsr, msrw, xchar;
    unsigned trg_rx, trg_tx;       /* TRG / RXTRG / TXTRG */
    int trg_rx_bad, trg_tx_bad;    /* TRG written 0 or out of range: X */
    int prescale4;                 /* the ÷4 prescaler is on */
    int filled;                    /* the TX FIFO reached its trigger since it last emptied */
    unsigned xoff_delay;           /* ticks until an owed Xoff goes out */
    int wake_int;                  /* wake-up indicator */
    int ls_new;                    /* 5G: an errored byte came in */
    int powered_down;              /* ier5: MCR[7] power-down */
    int rs485;                     /* the RS-485 direction pin is in transmit */
    unsigned rs485_hold;           /* ticks left of the turnaround delay */
    int imm_armed, imm_valid;      /* 5G send-immediate */
    uint8_t imm_data, imm_known;
    int flvl_tx_next;              /* EMSR[1:0] = 11: the next FLVL read shows TX */
    int rx_allow;                  /* 5G RX disable: characters still allowed */
    int drq_tx, drq_rx;            /* XR16C864 DMA requests */
    int tc_tx, tc_rx;              /* TC ended the request until the level re-arms */
    LogicValue dsr_last;
    LogicValue clock_level;        /* own clock pin (CHCCLK) */
} NsExarChannel;

typedef struct NsExarState {
    int motorola;                  /* 1 Motorola, 0 Intel, -1 X */
    int soft;                      /* a 5G soft reset (one channel) is in progress */
    int isolated;                  /* PowerSave isolation */
    const NsExarPart *part;
    /* 5G global block. */
    uint8_t mode8x, sleep, regb;
    uint16_t timer_reload;
    uint32_t timer_count;
    int timer_running, timer_oneshot, timer_ext, timer_int_en;
    LogicValue timer_int;          /* X: the reload value is 0 or 1 (decisions.md) */
    uint8_t timercntl;             /* XR16L784: the written bits */
    LogicValue tmrck_level;
    LogicValue tc_level;
} NsExarState;

#endif
