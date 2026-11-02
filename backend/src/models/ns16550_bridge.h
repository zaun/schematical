#ifndef NS16550_BRIDGE_H
#define NS16550_BRIDGE_H

/*
 * The I2C/SPI-to-UART bridges on the ns16550 model (plan-12-I section 3.2):
 * the device data and state shared by ns16550_bridge.c (the SC16IS7xx
 * register map, its PI7C9X SFR bank and XR20M1170 DLD, GPIO, the host port)
 * and ns16550_xr1280.c (the XR20M1280's 3-bit map).
 */

#include "ns16550_int.h"

enum { NB_GPIO = 8, NB_CHANNELS = 2 };

typedef struct NsBridgeDevice NsBridgeDevice;
typedef struct NsBridgePins NsBridgePins;
typedef struct NsBridge NsBridge;

typedef enum { NB_GPIO_NONE, NB_GPIO_SC16IS, NB_GPIO_XR1280 } NbGpioStyle;

/* The bridges' own registers (ExtReg codes from NS_REG_BRIDGE on). */
enum {
    B_TXLVL = NS_REG_BRIDGE, B_RXLVL, B_IODIR, B_IOSTATE, B_IOINTENA, B_IOCONTROL, B_EFCR,
    B_DLD,
    /* PI7C9X760/762 */
    B_SFREN, B_SFR, B_ASR, B_CPR, B_RFD, B_TFD, B_TIDLE, B_TRCTL, B_ISCR,
    /* XR20M1280 */
    X_SHR, X_SFR, X_GPIOLVL, X_EMSR_FC, X_FC_TRG, X_FCTR, X_GPIOINT, X_GPIO3T, X_GPIOINV,
    X_GPIOSEL, X_DLL, X_DLM, X_DLD, X_MCR, X_FCR
};

struct NsBridgeDevice {
    SerialHostDevice host;
    unsigned reg_bits;             /* sub-address register bits: 4 ([6:3]) or 3 ([5:3]) */
    int nack_full;                 /* I2C NACK for a THR write with the TX FIFO full */
    int nack_reset;                /* I2C NACK for the byte that sets IOControl[3] */
    uint64_t sda_timeout;          /* ps of SDA low that reset the I2C port; 0 = none */
    const char *irq;               /* "interrupt": open drain, active low */
    NbGpioStyle gpio_style;
    const char *gpio[NB_GPIO];
    unsigned gpio_count;
    int gpio_pullup;               /* GPIO inputs read Z as 1 */
    int state_reset;               /* GPIO output latch after reset; -1 = X */
    int fast_irda_all;
    const char *const *fast_irda_grades;
    size_t fast_irda_count;
    int dl_power[2];               /* DLL, DLM at power-up; -1 = X */
    const char *en485, *enir;      /* XR20M1280 power-up straps */
    const char *sleep_pin;         /* XR20M1280 SLEEP/PWRDN# */
};

struct NsBridgePins {
    SerialHostPins host;
    size_t irq;
    size_t gpio[NB_GPIO];
    int gpio_ch[NB_GPIO];          /* the channel whose modem pin it also is; -1 none */
    int gpio_role[NB_GPIO];        /* that ChannelPin */
    size_t en485, enir, sleep_pin;
};

/* Per channel: the registers beyond the core's. */
typedef struct {
    Reg efcr;
    Reg dld;
    /* PI7C9X */
    Reg sfren, sfr, cpr, tidle, trctl;
    int fc_toggle;
    /* 9-bit auto address: the receiver enabled by a matching address */
    int aa_on;
    /* XR20M1280 */
    Reg shr, xsfr, emsr, fctr, trg_rx, trg_tx;
    Reg dll_tx, dlm_tx, dld_tx, dll_rx, dlm_rx, dld_rx;
    SerialBaud rx_baud;
    unsigned tx_factor, rx_factor;
    unsigned prescale_tx, prescale_rx;
    int wake_int;                  /* FCR[3] wake-up interrupt pending */
    int ls_now;                    /* EMSR[6] = 1: an errored byte came in (until an LSR read) */
    uint8_t addr_bit[NS_MAX_FIFO]; /* SFR[7]: the byte at this TX FIFO slot has its 9th bit 1 */
    int wake_en;                   /* FCR[3] (written with EFR[4]) */
    int forced_sleep;              /* the SLEEP pin holds the channel asleep */
    int xoff_sticky;               /* EMSR[7] = 1: the Xoff interrupt waits for an ISR read */
    int rts_active;                /* auto RS-485: RTS asserted */
    unsigned rs_lead, rs_hold;     /* ticks of setup still to go / turn-around left */
    uint64_t xoff_at;              /* RX tick count when the level crossed the trigger */
    int xoff_wait;
    int holding;                   /* RS-485: the turn-around delay is running */
} NbChannel;

struct NsBridge {
    SerialHost host;
    SerialSpiEvents spi;
    SerialI2cEvents i2c;
    NbChannel ch[NB_CHANNELS];
    /* SC16IS GPIO */
    Reg iodir, latch, iointena, iocontrol;
    uint8_t ref;                   /* input levels at the last IOState read (IOLATCH = 0) */
    uint8_t ref_k;
    uint8_t latched;               /* IOLATCH = 1: bits holding a latched change */
    uint8_t latched_v;
    uint8_t latched_k;
    int latch_pending;
    /* XR20M1280 GPIO */
    Reg gpioint, gpio3t, gpioinv, gpiosel;
    uint8_t xr_last;               /* input levels last seen */
    uint8_t xr_last_k;
    int xr_gpio_pending;
    int en485_low, enir_low;       /* straps as sampled */
    /* host transaction */
    int have_sub;
    int sub_x;
    unsigned sub_ch, sub_reg;
    uint8_t sub_byte, sub_known;
    int spi_rw;                    /* 1 read, 0 write, -1 unknown */
    int fast_irda;
    LogicValue sda_level;
    uint32_t sda_gen;
    int raw;                       /* XR20M1280: a register write passed on to the core */
};

/* Small helpers shared by the two files. */
static inline int nb_known1(Reg r, unsigned bit) {
    return bit_of(r, bit) == LOGIC_1;
}

static inline NbChannel *nb_ch(const NsChannel *c) {
    return &c->owner->bridge->ch[c->index];
}

void nb_drive(NsState *s, size_t pin, LogicValue value, SimTime delay);
SimTime nb_cause_delay(const NsState *s, Cause cause);
LogicValue nb_gpio_level(const NsState *s, unsigned bit);

/* ns16550_xr1280.c */
int xr_decode(const NsChannel *c, unsigned address, int write);
void xr_peek_reg(NsChannel *c, int reg, uint8_t *v, uint8_t *k);
void xr_write_reg(NsState *s, NsChannel *c, int reg, uint8_t v, uint8_t k);
int xr_read_done(NsChannel *c, int reg, Cause *cause);
void xr_reset(NsState *s, int power_on);
void xr_gpio_outputs(NsState *s);
void xr_gpio_changed(NsState *s);
int xr_modem_on(const NsState *s);
LogicValue xr_irq_extra(const NsState *s);
LogicValue xr_gpio_int(const NsState *s);
int xr_rx_char(NsState *s, NsChannel *c, const SerialRxChar *rc);
int xr_tx_blocked(NsState *s, NsChannel *c);
int xr_levels(NsState *s, NsChannel *c);
LogicValue xr_rts(const NsChannel *c, LogicValue normal);
int xr_clock(NsState *s, NsChannel *c, int rising);
void xr_load_divisors(NsChannel *c);
unsigned xr_rx_trigger(const NsChannel *c);
unsigned xr_tx_threshold(const NsChannel *c);
void xr_thr_written(NsChannel *c, unsigned at);
int xr_tx_fetch(NsChannel *c);
int xr_sleep_check(NsState *s, NsChannel *c);
void xr_sleep_changed(NsState *s, NsChannel *c);
void xr_sleep_pin(NsState *s);
void xr_straps(NsState *s);

#endif
