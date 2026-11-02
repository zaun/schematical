/*
 * sc18is604: the NXP SC18IS604 SPI-to-I2C bridge (plan-12-I section 3.3;
 * plan-12-data/specs/bridges.md § SC18IS604, decisions.md § SC18IS604 and
 * the planner decisions). An SPI target (mode 3) whose messages (one per CS
 * frame) command a single-controller I2C bus: write N bytes (0x00), read N
 * bytes (0x01), read after write (0x02) and write after write (0x03) with a
 * repeated START between the two transfers, read buffer (0x06), SPI
 * configuration (0x18: 0x81 LSB first, 0x42 MSB first), register write and
 * read (0x20, 0x21), deep power-down (0x30 0x5A 0xA5; CS low wakes it) and
 * the version string (0xFE).
 *
 * The I2C controller runs from the internal 15 MHz RC oscillator (a model
 * time base on sim timers; the chip has no clock pin): SCL = 15 MHz /
 * (8 x I2CClock), 50 % duty, each quarter period a step. It drives SCL and
 * SDA open drain (0 or Z) with no internal pull-ups, waits while a target
 * holds SCL low (clock stretching) and for a free bus (a START seen with no
 * STOP yet), loses arbitration when SDA reads 0 while it sends a 1 (it then
 * waits for the bus and starts again), and ends with I2CStat 0xF0 (done),
 * 0xF1 (address NACK), 0xF2 (data NACK), 0xF8 (I2CTO timeout) or 0xF9 (a
 * byte count of 0 or larger than the bytes sent), asserting INT (open
 * drain); 0xF3 while busy. Reading I2CStat releases INT.
 *
 * Signal rules: RESET at Z reads inactive; a RESET low pulse counts after
 * tsa (125 ns; shorter ones are filtered). Inputs at Z read X (no pulls).
 * MISO is driven only for the bytes the command defines (register value,
 * read buffer), Z otherwise, as the J front ends do.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"
#include "serial_target.h"

enum { SP_GPIO = 5, SP_BUF = 255, SP_MSG = 520 };
enum { T_V, T_TR, T_START, T_SA };

static const char *const TIMING[] = {"tvq", "ttr", "tstart", "tsa", NULL};
static const char *const KEYS[] = {"bus", "i2c", "reset", "interrupt", "gpio", NULL};

/* I2CStat codes. */
enum { ST_OK = 0xF0, ST_ADDR_NACK = 0xF1, ST_DATA_NACK = 0xF2, ST_BUSY = 0xF3, ST_TIMEOUT = 0xF8,
       ST_COUNT = 0xF9 };

typedef struct {
    SerialHostDevice host;
    const char *scl, *sda, *reset, *irq;
    const char *gpio[SP_GPIO];
} SpDevice;

typedef struct {
    SerialHostPins host;
    size_t scl, sda, reset, irq;
    size_t gpio[SP_GPIO];
} SpPins;

typedef struct {
    uint8_t v;
    uint8_t k;
} SpReg;

/* One I2C transfer of a transaction: its address byte and data. */
typedef struct {
    uint8_t addr;
    uint8_t addr_k;
    int read;
    unsigned count;
    unsigned first;   /* index of its first byte in the TX buffer (writes) */
} Segment;

typedef enum {
    E_IDLE, E_WAIT_BUS, E_START_HOLD, E_LOW, E_RISE, E_HIGH, E_FALL, E_RS_RELEASE, E_RS_RISE,
    E_RS_HIGH, E_STOP_LOW, E_STOP_RISE, E_STOP_HIGH
} Step;

/* Timer ids: (generation << 3) | kind. */
enum { TM_STEP = 1, TM_TIMEOUT = 2, TM_RESET = 3, TM_BEGIN = 4 };

typedef struct {
    SerialBase base;
    const SpDevice *dev;
    const SpPins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SerialHost host;
    SerialSpiEvents spi;
    LogicValue *out;
    size_t pin_count;
    int in_reset;
    int unknown;
    /* registers */
    SpReg ioconfig, iolatch, clock, timeout, stat, adr;
    int int_active;
    int int_x;
    /* buffers */
    uint8_t tx[SP_BUF];
    uint8_t tx_k[SP_BUF];
    uint8_t rx[SP_BUF];
    uint8_t rx_k[SP_BUF];
    /* the SPI message */
    uint8_t msg[SP_MSG];
    uint8_t msg_k[SP_MSG];
    unsigned msg_len;
    int lsb_first;
    int power_down;
    int ignoring;              /* the frame that woke the device */
    /* the I2C engine */
    Segment seg[2];
    unsigned seg_count;
    unsigned seg_index;
    unsigned byte_index;       /* byte of the segment; 0 is the address byte */
    int bit;                   /* 7..0 data, -1 the ACK slot */
    uint8_t shift;
    uint8_t shift_k;
    unsigned rx_pos;
    Step step;
    uint32_t gen;              /* invalidates stale step and timeout timers */
    SimTime q;                 /* a quarter SCL period */
    int busy;                  /* a transaction is in progress */
    int bus_busy;              /* a START was seen on the bus, no STOP yet */
    LogicValue scl_seen, sda_seen;
    LogicValue sda_out;
    LogicValue scl_out;
    uint32_t reset_gen;
} SpState;

const SerialModel SC18IS604_MODEL;

static SimTime t_of(const SpState *s, unsigned i) {
    return s->grade->t[i];
}

static LogicValue level(const SpState *s, size_t pin, LogicValue z_reads) {
    return serial_level(s->sim, s->instance, pin, z_reads);
}

static void drive(SpState *s, size_t pin, LogicValue v, SimTime delay) {
    if (!SERIAL_REAL_PIN(pin) || s->out[pin] == v) {
        return;
    }
    s->out[pin] = v;
    sim_schedule_inertial_drive(s->sim, s->instance, pin, v, DRIVE_STRONG, delay);
}

static uint8_t reverse(uint8_t b) {
    b = (uint8_t)((b & 0xF0u) >> 4 | (b & 0x0Fu) << 4);
    b = (uint8_t)((b & 0xCCu) >> 2 | (b & 0x33u) << 2);
    b = (uint8_t)((b & 0xAAu) >> 1 | (b & 0x55u) << 1);
    return b;
}

static void schedule(SpState *s, unsigned kind, uint32_t gen, SimTime delay) {
    sim_schedule_timer(s->sim, s->instance, (gen << 3) | kind, delay);
}

/* ---- Outputs ------------------------------------------------------------------------- */

static void update_int(SpState *s) {
    LogicValue v;

    if (s->unknown) {
        v = LOGIC_X;
    } else if (s->in_reset || !s->int_active) {
        v = LOGIC_Z;
    } else {
        v = s->int_x ? LOGIC_X : LOGIC_0;
    }
    drive(s, s->pins->irq, v, 0);
}

static void set_lines(SpState *s, LogicValue scl, LogicValue sda) {
    s->scl_out = scl;
    s->sda_out = sda;
    drive(s, s->pins->scl, scl, 0);
    drive(s, s->pins->sda, sda, 0);
}

/* GPIO0-3 per IOConfig (00/01 input, 10 push-pull, 11 open drain); GPIO4
   is an input. */
static void update_gpio(SpState *s) {
    unsigned i;

    for (i = 0; i < 4; i++) {
        unsigned mode = (s->ioconfig.v >> (2 * i)) & 3u;
        int mode_known = ((s->ioconfig.k >> (2 * i)) & 3u) == 3u;
        LogicValue latch = (s->iolatch.k >> i) & 1u ? ((s->iolatch.v >> i) & 1u ? LOGIC_1 : LOGIC_0)
                                                    : LOGIC_X;
        LogicValue v;

        if (s->unknown) {
            v = LOGIC_X;
        } else if (s->in_reset) {
            v = LOGIC_Z;
        } else if (!mode_known) {
            v = LOGIC_X;
        } else if (mode < 2) {
            v = LOGIC_Z;
        } else if (mode == 2) {
            v = latch;
        } else {
            v = latch == LOGIC_1 ? LOGIC_Z : latch;   /* open drain: 0 or released */
        }
        drive(s, s->pins->gpio[i], v, 0);
    }
}

/* ---- I2C engine ------------------------------------------------------------------------ */

static void engine_end(SpState *s, uint8_t status, uint8_t known) {
    s->busy = 0;
    s->step = E_IDLE;
    s->gen++;
    set_lines(s, LOGIC_Z, LOGIC_Z);
    s->stat = (SpReg){status, known};
    s->int_active = 1;
    s->int_x = 0;
    update_int(s);
}

static void step_later(SpState *s, Step next, unsigned quarters) {
    s->step = next;
    if (s->q == 0) {
        return;   /* I2CClock = 0: the SCL generator is stopped (decisions.md) */
    }
    schedule(s, TM_STEP, s->gen, s->q * quarters);
}

/* The byte the current slot sends (address or data). */
static void load_byte(SpState *s) {
    const Segment *g = &s->seg[s->seg_index];

    if (s->byte_index == 0) {
        s->shift = g->addr;
        s->shift_k = g->addr_k;
    } else if (!g->read) {
        unsigned at = g->first + s->byte_index - 1;

        s->shift = s->tx[at];
        s->shift_k = s->tx_k[at];
    } else {
        s->shift = 0;
        s->shift_k = 0xFF;
    }
    s->bit = 7;
}

static int reading(const SpState *s) {
    return s->seg[s->seg_index].read && s->byte_index > 0;
}

/* SDA for the current bit while SCL is low. */
static LogicValue sda_for_bit(const SpState *s) {
    const Segment *g = &s->seg[s->seg_index];

    if (s->bit < 0) {
        if (reading(s)) {
            /* The controller ACKs each byte read and NACKs the last. */
            return s->byte_index < g->count ? LOGIC_0 : LOGIC_Z;
        }
        return LOGIC_Z;   /* the target's ACK slot */
    }
    if (reading(s)) {
        return LOGIC_Z;
    }
    if (!((s->shift_k >> s->bit) & 1u)) {
        return LOGIC_X;
    }
    return (s->shift >> s->bit) & 1u ? LOGIC_Z : LOGIC_0;
}

static void begin_start(SpState *s) {
    s->seg_index = 0;
    s->rx_pos = 0;
    step_later(s, E_WAIT_BUS, 1);
}

/* After the ACK slot: what comes next. */
static void after_byte(SpState *s) {
    const Segment *g = &s->seg[s->seg_index];

    if (s->byte_index < g->count) {
        s->byte_index++;
        load_byte(s);
        step_later(s, E_LOW, 1);
        return;
    }
    if (s->seg_index + 1 < s->seg_count) {
        s->seg_index++;
        step_later(s, E_RS_RELEASE, 1);   /* repeated START (planner decision) */
        return;
    }
    s->stat = (SpReg){ST_OK, 0xFF};
    step_later(s, E_STOP_LOW, 1);
}

static void engine_step(SpState *s) {
    LogicValue scl = level(s, s->pins->scl, LOGIC_X);
    LogicValue sda = level(s, s->pins->sda, LOGIC_X);

    switch (s->step) {
        case E_WAIT_BUS:
            if (s->bus_busy || scl != LOGIC_1 || sda != LOGIC_1) {
                step_later(s, E_WAIT_BUS, 1);   /* wait for a free bus */
                return;
            }
            set_lines(s, LOGIC_Z, LOGIC_0);     /* START */
            s->byte_index = 0;
            load_byte(s);
            step_later(s, E_START_HOLD, 2);
            return;
        case E_START_HOLD:
            set_lines(s, LOGIC_0, s->sda_out);
            step_later(s, E_LOW, 1);
            return;
        case E_LOW:
            set_lines(s, LOGIC_0, sda_for_bit(s));
            step_later(s, E_RISE, 1);
            return;
        case E_RISE:
            set_lines(s, LOGIC_Z, s->sda_out);
            step_later(s, E_HIGH, 1);
            return;
        case E_HIGH: {
            if (scl != LOGIC_1) {
                step_later(s, E_HIGH, 1);   /* a target stretches the clock */
                return;
            }
            if (s->bit >= 0) {
                if (reading(s)) {
                    uint8_t m = (uint8_t)(1u << s->bit);

                    if (sda == LOGIC_1) {
                        s->shift |= m;
                    }
                    if (sda == LOGIC_X) {
                        s->shift_k &= (uint8_t)~m;
                    }
                } else if (s->sda_out == LOGIC_Z && sda != LOGIC_1) {
                    if (sda == LOGIC_0) {
                        /* Arbitration lost: release, wait for the bus, start again. */
                        set_lines(s, LOGIC_Z, LOGIC_Z);
                        s->rx_pos = 0;
                        s->seg_index = 0;
                        step_later(s, E_WAIT_BUS, 1);
                        return;
                    }
                    engine_end(s, 0xF0, 0xF0);   /* X on SDA: the outcome is X */
                    return;
                }
            } else if (!reading(s)) {
                if (sda == LOGIC_X) {
                    engine_end(s, 0xF0, 0xF0);
                    return;
                }
                if (sda == LOGIC_1) {
                    s->stat = (SpReg){s->byte_index == 0 ? ST_ADDR_NACK : ST_DATA_NACK, 0xFF};
                    step_later(s, E_FALL, 1);
                    s->bit = -2;   /* stop after the fall */
                    return;
                }
            }
            step_later(s, E_FALL, 1);
            return;
        }
        case E_FALL:
            set_lines(s, LOGIC_0, s->sda_out);
            if (s->bit == -2) {
                step_later(s, E_STOP_LOW, 1);
                return;
            }
            if (s->bit > 0) {
                s->bit--;
                step_later(s, E_LOW, 1);
                return;
            }
            if (s->bit == 0) {
                s->bit = -1;
                if (reading(s) && s->rx_pos < SP_BUF) {
                    s->rx[s->rx_pos] = s->shift;
                    s->rx_k[s->rx_pos] = s->shift_k;
                    s->rx_pos++;
                }
                step_later(s, E_LOW, 1);
                return;
            }
            after_byte(s);
            return;
        case E_RS_RELEASE:
            set_lines(s, LOGIC_0, LOGIC_Z);
            step_later(s, E_RS_RISE, 1);
            return;
        case E_RS_RISE:
            set_lines(s, LOGIC_Z, LOGIC_Z);
            step_later(s, E_RS_HIGH, 1);
            return;
        case E_RS_HIGH:
            if (scl != LOGIC_1) {
                step_later(s, E_RS_HIGH, 1);
                return;
            }
            set_lines(s, LOGIC_Z, LOGIC_0);   /* repeated START */
            s->byte_index = 0;
            load_byte(s);
            step_later(s, E_START_HOLD, 2);
            return;
        case E_STOP_LOW:
            set_lines(s, LOGIC_0, LOGIC_0);
            step_later(s, E_STOP_RISE, 1);
            return;
        case E_STOP_RISE:
            set_lines(s, LOGIC_Z, LOGIC_0);
            step_later(s, E_STOP_HIGH, 1);
            return;
        case E_STOP_HIGH:
            if (scl != LOGIC_1) {
                step_later(s, E_STOP_HIGH, 1);
                return;
            }
            set_lines(s, LOGIC_Z, LOGIC_Z);   /* STOP */
            engine_end(s, s->stat.v, s->stat.k);
            return;
        default:
            return;
    }
}

/* Starts a transaction now (after the message's tstart delay). */
static void engine_begin(SpState *s) {
    uint8_t clk = s->clock.v;

    s->busy = 1;
    s->gen++;
    s->stat = (SpReg){ST_BUSY, 0xFF};
    /* SCL = 15 MHz / (8 x I2CClock): a quarter period is 2 x I2CClock / 15 MHz. */
    s->q = s->clock.k == 0xFF ? (SimTime)((clk * 400000ull + 1) / 3) : 0;
    if ((s->timeout.k & 1u) && (s->timeout.v & 1u)) {
        /* ((I2CClock + 1) / 15 MHz) x ((1 + TO) x 512) */
        uint64_t to = (uint64_t)(s->timeout.v >> 1);
        SimTime t = (SimTime)(((uint64_t)clk + 1) * (1 + to) * 512ull * 200000ull / 3);

        schedule(s, TM_TIMEOUT, s->gen, t);
    }
    begin_start(s);
}

/* ---- Commands --------------------------------------------------------------------------- */

static void count_error(SpState *s) {
    s->stat = (SpReg){ST_COUNT, 0xFF};
    s->int_active = 1;
    s->int_x = 0;
    update_int(s);
}

static void version(SpState *s) {
    static const char TEXT[16] = "SC18IS604 1.0.0";
    unsigned i;

    for (i = 0; i < 16; i++) {
        s->rx[i] = (uint8_t)TEXT[i];
        s->rx_k[i] = 0xFF;
    }
    s->int_active = 1;
    s->int_x = 0;
    update_int(s);
}

static int msg_known(const SpState *s, unsigned from, unsigned to) {
    unsigned i;

    for (i = from; i < to && i < s->msg_len; i++) {
        if (s->msg_k[i] != 0xFF) {
            return 0;
        }
    }
    return 1;
}

/* Queues the I2C transaction of message command 0x00-0x03. */
static void i2c_command(SpState *s) {
    unsigned cmd = s->msg[0];
    unsigned n1;
    unsigned n2;
    unsigned i;

    if (s->busy) {
        return;   /* 0xF3: the host must not start another (ignored, P2) */
    }
    memset(s->seg, 0, sizeof s->seg);
    s->seg_count = 1;
    switch (cmd) {
        case 0x00:   /* N, addr, data x N */
            n1 = s->msg[1];
            if (s->msg_len < 3 || n1 == 0 || s->msg_len < 3 + n1) {
                count_error(s);
                return;
            }
            s->seg[0] = (Segment){(uint8_t)(s->msg[2] & 0xFE), (uint8_t)(s->msg_k[2] | 1u), 0, n1,
                                  0};
            for (i = 0; i < n1; i++) {
                s->tx[i] = s->msg[3 + i];
                s->tx_k[i] = s->msg_k[3 + i];
            }
            break;
        case 0x01:   /* N, addr */
            n1 = s->msg[1];
            if (s->msg_len < 3 || n1 == 0) {
                count_error(s);
                return;
            }
            s->seg[0] = (Segment){(uint8_t)(s->msg[2] | 1u), (uint8_t)(s->msg_k[2] | 1u), 1, n1, 0};
            break;
        case 0x02:   /* Nw, Nr, addr+W, data x Nw, addr+R */
            n1 = s->msg[1];
            n2 = s->msg[2];
            if (s->msg_len < 4 || n1 == 0 || n2 == 0 || s->msg_len < 5 + n1) {
                count_error(s);
                return;
            }
            s->seg[0] = (Segment){(uint8_t)(s->msg[3] & 0xFE), (uint8_t)(s->msg_k[3] | 1u), 0, n1,
                                  0};
            for (i = 0; i < n1; i++) {
                s->tx[i] = s->msg[4 + i];
                s->tx_k[i] = s->msg_k[4 + i];
            }
            s->seg[1] = (Segment){(uint8_t)(s->msg[4 + n1] | 1u), (uint8_t)(s->msg_k[4 + n1] | 1u), 1,
                                  n2, 0};
            s->seg_count = 2;
            break;
        case 0x03:   /* N1, N2, addr1, data x N1, addr2, data x N2 */
            n1 = s->msg[1];
            n2 = s->msg[2];
            if (s->msg_len < 4 || n1 == 0 || n2 == 0 || s->msg_len < 5 + n1 + n2 ||
                n1 + n2 > SP_BUF) {
                count_error(s);
                return;
            }
            s->seg[0] = (Segment){(uint8_t)(s->msg[3] & 0xFE), (uint8_t)(s->msg_k[3] | 1u), 0, n1,
                                  0};
            for (i = 0; i < n1; i++) {
                s->tx[i] = s->msg[4 + i];
                s->tx_k[i] = s->msg_k[4 + i];
            }
            s->seg[1] = (Segment){(uint8_t)(s->msg[4 + n1] & 0xFE), (uint8_t)(s->msg_k[4 + n1] | 1u),
                                  0, n2, n1};
            for (i = 0; i < n2; i++) {
                s->tx[n1 + i] = s->msg[5 + n1 + i];
                s->tx_k[n1 + i] = s->msg_k[5 + n1 + i];
            }
            s->seg_count = 2;
            break;
        default:
            return;
    }
    /* The I2C START follows the message after td (35 us). */
    s->stat = (SpReg){ST_BUSY, 0xFF};
    s->busy = 1;
    s->gen++;
    schedule(s, TM_BEGIN, s->gen, t_of(s, T_START));
}

static void register_write(SpState *s, uint8_t reg, uint8_t v, uint8_t k) {
    switch (reg) {
        case 0x00: s->ioconfig = (SpReg){v, k}; break;
        case 0x01: s->iolatch = (SpReg){(uint8_t)(v & 0x0Fu), (uint8_t)(k | 0xF0u)}; break;
        case 0x02: s->clock = (SpReg){v, k}; break;
        case 0x03: s->timeout = (SpReg){v, k}; break;
        case 0x05: s->adr = (SpReg){(uint8_t)(v & 0xFEu), (uint8_t)(k | 1u)}; break;
        default: break;   /* I2CStat is read-only; other addresses undefined (P2) */
    }
    update_gpio(s);
}

static void register_peek(const SpState *s, uint8_t reg, uint8_t *v, uint8_t *k) {
    unsigned i;

    *v = 0;
    *k = 0xFF;
    switch (reg) {
        case 0x00: *v = s->ioconfig.v; *k = s->ioconfig.k; break;
        case 0x01:
            for (i = 0; i < SP_GPIO; i++) {
                LogicValue l = level(s, s->pins->gpio[i], LOGIC_X);

                if (l == LOGIC_1) {
                    *v |= (uint8_t)(1u << i);
                } else if (l == LOGIC_X) {
                    *k &= (uint8_t)~(1u << i);
                }
            }
            break;
        case 0x02: *v = s->clock.v; *k = s->clock.k; break;
        case 0x03: *v = s->timeout.v; *k = s->timeout.k; break;
        case 0x04: *v = s->stat.v; *k = s->stat.k; break;
        case 0x05: *v = s->adr.v; *k = s->adr.k; break;
        default: *k = 0; break;
    }
}

/* ---- SPI ----------------------------------------------------------------------------------- */

static void spi_begin(void *context) {
    SpState *s = context;

    s->msg_len = 0;
    if (s->power_down) {
        s->power_down = 0;   /* CS low wakes it; this frame is not a message */
        s->ignoring = 1;
    }
}

static SerialSpiByte spi_next(void *context, unsigned index) {
    SpState *s = context;
    SerialSpiByte out = {0, 0, 0};

    if (index == 0 || s->ignoring || s->unknown || s->in_reset || s->msg_len == 0) {
        return out;
    }
    if (s->msg_k[0] != 0xFF) {
        out.drive = 0xFF;   /* an unknown command: X */
        return out;
    }
    if (s->msg[0] == 0x06 && index - 1 < SP_BUF) {
        out.drive = 0xFF;
        out.data = s->rx[index - 1];
        out.known = s->rx_k[index - 1];
    } else if (s->msg[0] == 0x21 && index == 2) {
        out.drive = 0xFF;
        if (s->msg_k[1] == 0xFF) {
            register_peek(s, s->msg[1], &out.data, &out.known);
        }
    }
    if (out.drive && s->lsb_first) {
        out.data = reverse(out.data);
        out.known = reverse(out.known);
    }
    return out;
}

static void spi_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    SpState *s = context;

    if (s->ignoring || s->unknown || s->in_reset) {
        return;
    }
    if (s->lsb_first) {
        data = reverse(data);
        known = reverse(known);
    }
    if (s->msg_len < SP_MSG) {
        s->msg[s->msg_len] = data;
        s->msg_k[s->msg_len] = known;
        s->msg_len++;
    }
    if (index != 2 || s->msg_k[0] != 0xFF) {
        return;
    }
    /* Register access is processed as its data byte completes. */
    if (s->msg[0] == 0x20) {
        if (s->msg_k[1] != 0xFF) {
            s->ioconfig = s->iolatch = s->clock = s->timeout = s->adr = (SpReg){0, 0};
            update_gpio(s);
        } else {
            register_write(s, s->msg[1], data, known);
        }
    } else if (s->msg[0] == 0x21 && s->msg_k[1] == 0xFF && s->msg[1] == 0x04) {
        s->int_active = 0;   /* reading I2CStat releases INT */
        update_int(s);
    }
}

static void spi_end(void *context, unsigned bits) {
    SpState *s = context;

    if (s->ignoring) {
        s->ignoring = 0;
        return;
    }
    if (s->unknown || s->in_reset || s->msg_len == 0) {
        return;
    }
    if (bits % 8 != 0) {
        /* A partial byte: the message is malformed; nothing more happens. */
    }
    if (s->msg_k[0] != 0xFF) {
        /* An unknown command could have been a register write: X there (P2). */
        if (((s->msg[0] ^ 0x20u) & s->msg_k[0]) == 0 && s->msg_len >= 3) {
            s->ioconfig = s->iolatch = s->clock = s->timeout = s->adr = (SpReg){0, 0};
            update_gpio(s);
        }
        return;
    }
    switch (s->msg[0]) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            if (!msg_known(s, 1, s->msg[0] >= 0x02 ? 3 : 2)) {
                s->stat = (SpReg){0xF0, 0xF0};   /* an unknown byte count */
                s->int_active = 1;
                s->int_x = 1;
                update_int(s);
                return;
            }
            i2c_command(s);
            return;
        case 0x18:
            if (s->msg_len >= 2 && s->msg_k[1] == 0xFF) {
                if (s->msg[1] == 0x81) {
                    s->lsb_first = 1;
                } else if (s->msg[1] == 0x42) {
                    s->lsb_first = 0;
                }
            }
            return;
        case 0x30:
            if (s->msg_len == 3 && msg_known(s, 1, 3) && s->msg[1] == 0x5A && s->msg[2] == 0xA5 &&
                !s->busy) {
                s->power_down = 1;
            }
            return;
        case 0xFE:
            version(s);
            return;
        default:
            return;
    }
}

/* ---- Reset ------------------------------------------------------------------------------------ */

static void reset_state(SpState *s) {
    s->ioconfig = (SpReg){0, 0xFF};
    s->iolatch = (SpReg){0, 0xF0};   /* IOState reset 0xXX */
    s->clock = (SpReg){0x19, 0xFF};
    s->timeout = (SpReg){0, 0xFF};
    s->stat = (SpReg){ST_OK, 0xFF};
    s->adr = (SpReg){0, 0xFF};
    s->int_active = 0;
    s->int_x = 0;
    s->lsb_first = 0;
    s->power_down = 0;
    s->ignoring = 0;
    s->busy = 0;
    s->step = E_IDLE;
    s->gen++;
    s->msg_len = 0;
}

static void outputs(SpState *s) {
    update_int(s);
    update_gpio(s);
    if (s->unknown) {
        set_lines(s, LOGIC_X, LOGIC_X);
    } else if (s->in_reset || !s->busy) {
        set_lines(s, LOGIC_Z, LOGIC_Z);
    }
}

static void apply_reset(SpState *s) {
    LogicValue r = level(s, s->pins->reset, LOGIC_1);

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
        s->gen++;
        s->busy = 0;
        serial_host_hold(&s->host, s->sim, s->instance, 1);
    } else if (r == LOGIC_0) {
        /* Counted after tsa (glitch filter). */
        s->reset_gen++;
        schedule(s, TM_RESET, s->reset_gen, t_of(s, T_SA));
        return;
    } else {
        s->reset_gen++;
        if (s->in_reset || s->unknown) {
            s->in_reset = 0;
            s->unknown = 0;
            serial_host_hold(&s->host, s->sim, s->instance, 0);
        }
    }
    outputs(s);
}

/* ---- Engine callbacks ------------------------------------------------------------------------- */

static void sp_start(Sim *sim, SimInstance *instance, void *raw) {
    SpState *s = raw;

    s->sim = sim;
    s->instance = instance;
    serial_host_start(&s->host, sim, instance);
    s->scl_seen = level(s, s->pins->scl, LOGIC_X);
    s->sda_seen = level(s, s->pins->sda, LOGIC_X);
    s->in_reset = 0;
    if (level(s, s->pins->reset, LOGIC_1) == LOGIC_0) {
        s->in_reset = 1;   /* low at power-on: held from the start */
        serial_host_hold(&s->host, sim, instance, 1);
    } else {
        apply_reset(s);
    }
    outputs(s);
}

static void sp_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    SpState *s = raw;
    unsigned kind = id & 7u;
    uint32_t gen = id >> 3;

    s->sim = sim;
    s->instance = instance;
    if (kind == TM_RESET) {
        if (gen == (s->reset_gen & 0x1FFFFFFFu) && level(s, s->pins->reset, LOGIC_1) == LOGIC_0) {
            s->in_reset = 1;
            s->unknown = 0;
            reset_state(s);
            serial_host_hold(&s->host, sim, instance, 1);
            outputs(s);
        }
        return;
    }
    if (gen != (s->gen & 0x1FFFFFFFu) || s->in_reset || s->unknown || s->power_down) {
        return;
    }
    switch (kind) {
        case TM_BEGIN: engine_begin(s); break;
        case TM_STEP: engine_step(s); break;
        case TM_TIMEOUT:
            if (s->busy) {
                engine_end(s, ST_TIMEOUT, 0xFF);
            }
            break;
        default: break;
    }
}

static void sp_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    SpState *s = raw;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (pin == s->pins->reset) {
        apply_reset(s);
        return;
    }
    if (pin == s->pins->scl || pin == s->pins->sda) {
        /* Bus monitor: START (SDA falls with SCL high) and STOP (SDA rises). */
        LogicValue scl = level(s, s->pins->scl, LOGIC_X);
        LogicValue sda = level(s, s->pins->sda, LOGIC_X);

        if (pin == s->pins->sda && scl == LOGIC_1 && s->scl_seen == LOGIC_1) {
            if (s->sda_seen == LOGIC_1 && sda == LOGIC_0) {
                s->bus_busy = 1;
            } else if (s->sda_seen == LOGIC_0 && sda == LOGIC_1) {
                s->bus_busy = 0;
            }
        }
        s->scl_seen = scl;
        s->sda_seen = sda;
    }
    if (serial_host_has_pin(&s->dev->host, &s->pins->host, pin)) {
        serial_host_pin(&s->host, sim, instance, pin, &s->spi, NULL);
    }
}

/* ---- Terminal: none (no UART) ---------------------------------------------------------------------- */

static unsigned sp_channels(const void *raw) {
    (void)raw;
    return 0;
}

static SerialLine *sp_line(void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return NULL;
}

static int sp_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 0;
}

/* ---- Create / destroy ------------------------------------------------------------------------------ */

static void *sp_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    SpState *s = calloc(1, sizeof *s);
    size_t i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &SC18IS604_MODEL;
    s->dev = data;
    s->pins = pins;
    s->grade = grade;
    s->pin_count = part->pin_count;
    s->out = malloc(part->pin_count * sizeof *s->out);
    if (s->out == NULL) {
        free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->out[i] = LOGIC_Z;
    }
    serial_host_init(&s->host, &s->dev->host, &s->pins->host);
    s->host.spi.t_out = grade->t[T_V];
    s->host.spi.t_float = grade->t[T_TR];
    s->spi.context = s;
    s->spi.begin = spi_begin;
    s->spi.next = spi_next;
    s->spi.received = spi_received;
    s->spi.end = spi_end;
    for (i = 0; i < SP_BUF; i++) {
        s->rx_k[i] = 0;   /* buffer contents at power-on: not stated (X) */
        s->tx_k[i] = 0;
    }
    reset_state(s);
    s->scl_out = s->sda_out = LOGIC_Z;
    return s;
}

static void sp_destroy(void *raw) {
    SpState *s = raw;

    free(s->out);
    free(s);
}

static int sp_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const SpPins *p = pins;

    (void)data;
    (void)options;
    return pin != SERIAL_NO_PIN && (pin == p->scl || pin == p->sda || pin == p->irq);
}

/* ---- Loader ----------------------------------------------------------------------------------------- */

static void *sp_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    static const char *const I2C_KEYS[] = {"scl", "sda", NULL};
    SpDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;
    const cJSON *i2c;
    const char *const *names;
    size_t n;
    size_t i;

    if (d == NULL) {
        return NULL;
    }
    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    kind_push(ctx, "bus");
    if (serial_host_load(ctx, bus, NULL, &d->host) != 0) {
        return NULL;
    }
    if (d->host.style != SERIAL_HOST_SPI || d->host.mode != 3) {
        kind_error(ctx, "the sc18is604 host port is \"spi\" mode 3");
        return NULL;
    }
    kind_pop(ctx);
    i2c = kind_member(ctx, json, "i2c", 1);
    if (i2c == NULL) {
        return NULL;
    }
    kind_push(ctx, "i2c");
    if (kind_check_keys(ctx, i2c, I2C_KEYS, NULL) != 0 || kind_string(ctx, i2c, "scl", &d->scl) != 0 ||
        kind_string(ctx, i2c, "sda", &d->sda) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if (kind_string(ctx, json, "reset", &d->reset) != 0 ||
        kind_string(ctx, json, "interrupt", &d->irq) != 0 ||
        kind_names(ctx, json, "gpio", &names, &n) != 0) {
        return NULL;
    }
    if (n != SP_GPIO) {
        kind_error(ctx, "\"gpio\" needs 5 pin names (GPIO0 first; GPIO4 is an input)");
        return NULL;
    }
    for (i = 0; i < n; i++) {
        d->gpio[i] = names[i];
    }
    *channels = 0;
    return d;
}

static void *sp_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const SpDevice *d = raw;
    SpPins *p = kind_alloc(ctx, sizeof *p);
    char key[32];
    unsigned i;

    if (p == NULL || serial_host_bind(ctx, &d->host, pins, pin_count, &p->host) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "i2c.scl", d->scl, &p->scl) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "i2c.sda", d->sda, &p->sda) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "reset", d->reset, &p->reset) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "interrupt", d->irq, &p->irq) != 0) {
        return NULL;
    }
    for (i = 0; i < SP_GPIO; i++) {
        snprintf(key, sizeof key, "gpio[%u]", i);
        if (serial_bind_pin(ctx, pins, pin_count, key, d->gpio[i], &p->gpio[i]) != 0) {
            return NULL;
        }
    }
    return p;
}

const SerialModel SC18IS604_MODEL = {
    "sc18is604", KEYS, TIMING, NULL,
    sp_load_device, sp_bind, NULL, sp_open_drain,
    sp_create, sp_destroy, sp_start, sp_pin_changed, sp_timer,
    sp_channels, sp_line, sp_async
};
