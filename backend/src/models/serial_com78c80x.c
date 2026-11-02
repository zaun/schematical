/*
 * com78c80x: the SMC scanner UARTs COM78C804 (4 lines), COM78C802 (2 lines)
 * and COM78804 (the NMOS 4-line part) (plan-12-H section 3.6; facts in
 * plan-12-data/specs/classic-b.md sections COM78C804, COM78C802, COM78804;
 * decisions.md section COM78C804 / COM78804).
 *
 * Bus: CS selects, DS strobes, WR gives the direction (low = write into the
 * UART). A read drives DL while CS, DS are low and WR is high; a write is
 * taken on the end of the strobe (DS rising). ADD4..3 (4 lines) or ADD3 (2
 * lines) pick the line, ADD2..0 the register (Table 2); 100 and 101 are the
 * Interrupt Summary and Data Set Change Summary for every line. RDY (open
 * drain) is low from tRDL after CS falls to tRDH after it rises. A pin name
 * used twice in a pinout (DS on the DIP-48) is one input: the pins must
 * agree, else it reads X.
 *
 * Each line: MR1/MR2 behind an alternating pointer (reset by RESET or a
 * Command read), the Command register, the Status register, a two-entry
 * receive FIFO and the THR. MR2 picks independent TX and RX rates from CLK
 * (decisions.md: the internal divisor is CLK / (16 x the printed rate), e.g.
 * 19200 -> 16, 110 -> 2816). The interrupt scanner (p9) steps one position
 * per CLK: the receivers of lines 0..n-1, then the transmitters; it stops
 * on an interrupting condition (IRQ low, IRQLN and IRQTXRX show it), and
 * runs on when the condition goes, from the next position after a receiver
 * and from position 0 after a transmitter.
 *
 * Signal rules (plan-12-A section 3.5): power-on is the RESET state; CS,
 * DS, RESET read inactive at Z. COM78804 differs only in timing (its grade:
 * tDSU 0 ns), so no device key separates it.
 */

#include <stdlib.h>
#include <string.h>

#include "serial_util.h"

enum { T_DD, T_DDZ, T_ID, T_RDL, T_RDH };
static const char *const TIMING[] = {"tdd", "tddz", "tid", "trdl", "trdh", NULL};

static const char *const KEYS[] = {"bus", "clock", "lines", "channels", "interrupt", NULL};

static const char *const BUS_KEYS[] = {
    "data", "address", "select", "strobe", "write", "ready", "reset", NULL
};
enum { L_TX, L_RX, L_DSR, L_DCD, L_COUNT };
static const char *const LINE_KEYS[] = {"tx", "rx", "dsr", "dcd", NULL};
static const char *const INT_KEYS[] = {"irq", "line", "txrx", NULL};

enum { MAX_LINES = 4, DUP = 2 };

/* MR2 rate codes: the CLK divisor of the 16x clock (4.9152 MHz). */
static const uint32_t RATE_DIV[16] = {
    6144, 4096, 2816, 2304, 2048, 1024, 512, 256, 176, 152, 128, 88, 64, 44, 32, 16
};

typedef struct {
    const char *data[8], *addr[5];
    unsigned addr_count;
    const char *select, *strobe, *write, *ready, *reset;
    const char *clock;
    unsigned lines;
    const char *line[MAX_LINES][L_COUNT];
    const char *irq, *irqln[2], *txrx;
} CmDevice;

/* A named input: up to two pins with the name. */
typedef struct {
    size_t pin[DUP];
    unsigned count;
} CmIn;

typedef struct {
    size_t data[8];
    CmIn addr[5], select, strobe, write, reset;
    size_t ready, clock, irq, irqln[2], txrx;
    size_t line[MAX_LINES][L_COUNT];
} CmPins;

typedef struct {
    uint16_t data, known;
    uint8_t flags, flags_known;
} CmChar;

typedef struct {
    SerialLine line;
    SerialBaud txb, rxb;
    SerialFormat format;
    SuReg mr1, mr2, cmd;
    int mr_ptr;                  /* 0: the next access is MR1 */
    CmChar fifo[2];
    unsigned count;
    LogicValue fer, orr, per;
    LogicValue txrdy, txemt;
    SuReg thr;
    int thr_full;
    LogicValue tsr;
    int brk;                     /* TxD held spacing */
    LogicValue dsr, dcd;         /* pin levels */
    int dschng;
    unsigned long mark_clk;      /* CLK cycles of receiver input high so far */
    unsigned long mark_before;   /* ... before its last 1 -> 0 */
    LogicValue in_level;         /* the receiver input at the last CLK */
    int resync;
} CmLine;

typedef struct {
    SerialBase base;
    const CmDevice *dev;
    const CmPins *pins;
    const SerialGrade *grade;
    Sim *sim;
    SimInstance *instance;
    SuOutputs o;
    CmLine ln[MAX_LINES];
    unsigned lines;
    int in_reset;
    int unknown;
    LogicValue clk;
    /* Scanner. */
    unsigned pos;
    int stopped;
    /* Bus. */
    int cycle;                   /* 0 none, 1 read, 2 write, 3 X */
    unsigned cyc_line, cyc_reg;
    int cyc_known;
    LogicValue cs_level;
} CmState;

static SimTime t_of(const CmState *s, unsigned i) {
    return s->grade->t[i];
}

static LogicValue in_level(const CmState *s, const CmIn *in, LogicValue z_reads) {
    LogicValue v = LOGIC_X;
    unsigned i;

    if (in->count == 0) {
        return z_reads;
    }
    for (i = 0; i < in->count; i++) {
        LogicValue a = serial_level(s->sim, s->instance, in->pin[i], z_reads);

        if (i == 0) {
            v = a;
        } else if (a != v) {
            v = LOGIC_X;   /* the tied pins disagree */
        }
    }
    return v;
}

static int in_has(const CmIn *in, size_t pin) {
    unsigned i;

    for (i = 0; i < in->count; i++) {
        if (in->pin[i] == pin) {
            return 1;
        }
    }
    return 0;
}

/* ---- Line format and state ---------------------------------------------------------------- */

static unsigned op_mode(const CmLine *l) {
    return (l->cmd.v >> 6) & 3u;
}

static void update_format(CmLine *l) {
    SerialFormat f = serial_format_default();
    unsigned stop = (l->mr1.v >> 6) & 3u;
    unsigned par = (l->mr1.v >> 4) & 3u;

    f.data_bits = 5 + ((l->mr1.v >> 2) & 3u);
    f.parity = !(par & 1u) ? SERIAL_PARITY_NONE : par == 3 ? SERIAL_PARITY_EVEN : SERIAL_PARITY_ODD;
    f.stop_halves = stop == 1 ? 2 : stop == 2 ? 3 : 4;
    f.format_x = (l->mr1.k & 0xFCu) != 0xFCu || stop == 0;
    l->format = f;
}

static void load_rates(CmLine *l) {
    int known_tx = (l->mr2.k & 0xF0u) == 0xF0u;
    int known_rx = (l->mr2.k & 0x0Fu) == 0x0Fu;

    serial_baud_load(&l->txb, RATE_DIV[(l->mr2.v >> 4) & 15u], known_tx);
    serial_baud_load(&l->rxb, RATE_DIV[l->mr2.v & 15u], known_rx);
}

static LogicValue cmd_bit(const CmLine *l, unsigned bit) {
    return su_bit(l->cmd, bit);
}

static void clear_rx_errors(CmLine *l) {
    l->fer = l->orr = l->per = LOGIC_0;
}

/* The top character's errors show in FER / PER (FER sticky, PER cleared
   when that character is read). */
static void show_top(CmLine *l) {
    if (l->count == 0 || cmd_bit(l, 4) == LOGIC_1) {
        return;
    }
    if ((l->fifo[0].flags & SERIAL_RX_FE) && (l->fifo[0].flags_known & SERIAL_RX_FE)) {
        l->fer = LOGIC_1;
    }
    if ((l->fifo[0].flags & SERIAL_RX_PE) && (l->fifo[0].flags_known & SERIAL_RX_PE)) {
        l->per = LOGIC_1;
    }
}

static void reset_line(CmLine *l) {
    l->mr1 = su_reg(0, 0xFF);
    l->mr2 = su_reg(0, 0xFF);
    l->cmd = su_reg(0, 0xFF);
    l->mr_ptr = 0;
    l->count = 0;
    clear_rx_errors(l);
    l->txrdy = LOGIC_0;
    l->txemt = LOGIC_0;
    l->thr_full = 0;
    l->tsr = LOGIC_1;
    l->brk = 0;
    l->dschng = 0;
    l->mark_clk = 0;
    l->mark_before = 0;
    l->in_level = LOGIC_X;
    serial_tx_reset(&l->line.tx);
    serial_rx_reset(&l->line.rx);
    update_format(l);
    load_rates(l);
}

/* ---- Scanner -------------------------------------------------------------------------------- */

static int rx_condition(const CmLine *l) {
    int rxie = cmd_bit(l, 5) == LOGIC_1;
    int mcie = su_bit(l->mr1, 0) == LOGIC_1;

    return rxie && (l->count > 0 || (l->dschng && mcie));
}

static int tx_condition(const CmLine *l) {
    return cmd_bit(l, 1) == LOGIC_1 && l->txrdy == LOGIC_1;
}

static int pos_condition(const CmState *s, unsigned pos) {
    return pos < s->lines ? rx_condition(&s->ln[pos]) : tx_condition(&s->ln[pos - s->lines]);
}

static unsigned pos_line(const CmState *s, unsigned pos) {
    return pos < s->lines ? pos : pos - s->lines;
}

static void scanner_clock(CmState *s) {
    unsigned n = 2 * s->lines;

    if (s->stopped) {
        if (pos_condition(s, s->pos)) {
            return;
        }
        /* The condition went: run on (a receiver: from the next position;
           a transmitter: from position 0, so receivers come first). */
        s->stopped = 0;
        s->pos = s->pos < s->lines ? (s->pos + 1) % n : 0;
    } else {
        s->pos = (s->pos + 1) % n;
    }
    if (pos_condition(s, s->pos)) {
        s->stopped = 1;
    }
}

/* ---- Outputs -------------------------------------------------------------------------------- */

static LogicValue line_txd(const CmState *s, const CmLine *l, unsigned i) {
    LogicValue rx = serial_level(s->sim, s->instance, s->pins->line[i][L_RX], LOGIC_X);

    if (s->in_reset) {
        return LOGIC_1;   /* decisions.md: TxD marks during RESET */
    }
    if ((l->cmd.k & 0xC0u) != 0xC0u) {
        return LOGIC_X;
    }
    switch (op_mode(l)) {
        case 1: return rx;          /* automatic echo */
        case 2: return LOGIC_1;     /* local loopback: TXD high */
        case 3: return rx;          /* remote loopback */
        default: break;
    }
    if (l->brk) {
        return LOGIC_0;
    }
    return l->tsr;
}

static void update_outputs(CmState *s, SimTime delay) {
    const CmPins *p = s->pins;
    unsigned i;
    unsigned line;

    if (s->unknown) {
        for (i = 0; i < s->lines; i++) {
            su_drive(&s->o, s->sim, s->instance, p->line[i][L_TX], LOGIC_X, 0);
        }
        su_drive(&s->o, s->sim, s->instance, p->irq, LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, p->irqln[0], LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, p->irqln[1], LOGIC_X, 0);
        su_drive(&s->o, s->sim, s->instance, p->txrx, LOGIC_X, 0);
        return;
    }
    for (i = 0; i < s->lines; i++) {
        su_drive(&s->o, s->sim, s->instance, p->line[i][L_TX], line_txd(s, &s->ln[i], i), 0);
    }
    line = pos_line(s, s->pos);
    su_drive_od(&s->o, s->sim, s->instance, p->irq,
                s->in_reset ? LOGIC_1 : su_not(su_bool(s->stopped)), delay);
    /* IRQLN / IRQTXRX are valid only while IRQ is asserted (p9): X otherwise. */
    su_drive(&s->o, s->sim, s->instance, p->irqln[0], s->stopped ? su_bool(line & 1u) : LOGIC_X, 0);
    su_drive(&s->o, s->sim, s->instance, p->irqln[1], s->stopped ? su_bool(line & 2u) : LOGIC_X, 0);
    su_drive(&s->o, s->sim, s->instance, p->txrx, s->stopped ? su_bool(s->pos >= s->lines) : LOGIC_X, 0);
}

/* ---- Registers ---------------------------------------------------------------------------------- */

static void status_of(const CmState *s, const CmLine *l, unsigned i, uint8_t *v, uint8_t *k) {
    LogicValue bits[8];
    unsigned b;

    bits[7] = su_not(serial_level(s->sim, s->instance, s->pins->line[i][L_DSR], LOGIC_X));
    bits[6] = su_not(serial_level(s->sim, s->instance, s->pins->line[i][L_DCD], LOGIC_X));
    bits[5] = l->fer;
    bits[4] = l->orr;
    bits[3] = l->per;
    bits[2] = l->txemt;
    bits[1] = su_bool(l->count > 0);
    bits[0] = l->txrdy;
    *v = 0;
    *k = 0;
    for (b = 0; b < 8; b++) {
        if (bits[b] == LOGIC_1) *v |= (uint8_t)(1u << b);
        if (bits[b] != LOGIC_X) *k |= (uint8_t)(1u << b);
    }
}

static uint8_t dschng_summary(const CmState *s) {
    static const unsigned BIT4[4] = {0, 1, 6, 7};
    uint8_t v = 0;
    unsigned i;

    for (i = 0; i < s->lines; i++) {
        if (s->ln[i].dschng) {
            v |= (uint8_t)(1u << (s->lines == 4 ? BIT4[i] : i));
        }
    }
    return v;
}

static void reg_peek(const CmState *s, unsigned line, unsigned reg, uint8_t *v, uint8_t *k) {
    const CmLine *l = &s->ln[line < s->lines ? line : 0];

    *v = 0;
    *k = 0;
    switch (reg) {
        case 0:
            if (l->count > 0) {
                *v = (uint8_t)l->fifo[0].data;
                *k = (uint8_t)l->fifo[0].known;
            }
            /* empty: the buffer contents are undefined (X) */
            break;
        case 1: status_of(s, l, line, v, k); break;
        case 2:
            *v = l->mr_ptr ? l->mr2.v : l->mr1.v;
            *k = l->mr_ptr ? l->mr2.k : l->mr1.k;
            break;
        case 3: *v = l->cmd.v; *k = l->cmd.k; break;
        case 4: {
            unsigned ln = pos_line(s, s->pos);

            /* 7 IRQ; 3..1 the line (bit 3 reads 0); 0 Tx/Rx. Bits 3..0 run
               free while bit 7 is 0. */
            *v = (uint8_t)((s->stopped ? 0x80 : 0) | (ln << 1) | (s->pos >= s->lines ? 1 : 0));
            if (s->lines == 2) {
                *v &= 0x83;
            }
            /* Bits 3..0 run free, valid only while bit 7 is 1: X otherwise (P2). */
            *k = s->stopped ? 0xFF : 0xF0;
            break;
        }
        case 5: *v = dschng_summary(s); *k = 0xFF; break;
        default: break;   /* 110, 111: not listed: X (P2) */
    }
}

static void pop_rx(CmLine *l) {
    if (l->count == 0) {
        return;
    }
    l->fifo[0] = l->fifo[1];
    l->count--;
    l->per = LOGIC_0;   /* reading the current character clears PER */
    show_top(l);
}

static void read_done(CmState *s, unsigned line, unsigned reg) {
    CmLine *l = &s->ln[line < s->lines ? line : 0];

    switch (reg) {
        case 0: pop_rx(l); break;
        case 2: l->mr_ptr ^= 1; break;
        case 3: l->mr_ptr = 0; break;
        default: break;
    }
}

static void write_command(CmLine *l, uint8_t v, uint8_t k) {
    LogicValue rx_was = cmd_bit(l, 2);
    LogicValue tx_was = cmd_bit(l, 0);
    LogicValue brk_was = cmd_bit(l, 3);

    l->cmd = su_reg(v, k);
    if (op_mode(l) == 3) {
        /* Remote loopback: TxEN and RxEN held cleared. */
        l->cmd.v &= (uint8_t)~0x05u;
    }
    if (cmd_bit(l, 4) == LOGIC_1) {
        clear_rx_errors(l);   /* RERR (not self-clearing) */
    }
    if (cmd_bit(l, 2) == LOGIC_0 && rx_was != LOGIC_0) {
        /* RxEN cleared: assembly stops, FIFO lost, errors and RxRDY cleared. */
        l->count = 0;
        clear_rx_errors(l);
        serial_rx_reset(&l->line.rx);
    }
    if (cmd_bit(l, 0) == LOGIC_0) {
        /* TxEN cleared: TxRDY and TxEMT cleared; the THR keeps its byte. */
        l->txrdy = LOGIC_0;
        l->txemt = LOGIC_0;
    } else if (cmd_bit(l, 0) == LOGIC_1 && tx_was != LOGIC_1 && !l->thr_full &&
               op_mode(l) != 1 && op_mode(l) != 3) {
        l->txrdy = LOGIC_1;
    }
    if (cmd_bit(l, 3) == LOGIC_0 && brk_was == LOGIC_1) {
        l->brk = 0;
    }
    if (op_mode(l) == 1 || op_mode(l) == 3) {
        l->txrdy = LOGIC_0;
        l->txemt = LOGIC_0;
    }
}

static void write_reg(CmState *s, unsigned line, unsigned reg, uint8_t v, uint8_t k) {
    CmLine *l = &s->ln[line < s->lines ? line : 0];
    unsigned i;

    switch (reg) {
        case 0:
            l->thr = su_reg(v, k);
            l->thr_full = 1;
            l->txrdy = LOGIC_0;
            l->txemt = LOGIC_0;
            break;
        case 2:
            if (l->mr_ptr) {
                l->mr2 = su_reg(v, k);
                load_rates(l);
            } else {
                l->mr1 = su_reg(v, k);
                update_format(l);
            }
            l->mr_ptr ^= 1;
            break;
        case 3:
            write_command(l, v, k);
            break;
        case 5:
            /* Data Set Change Summary: a 1 clears that line's bit. */
            for (i = 0; i < s->lines; i++) {
                static const unsigned BIT4[4] = {0, 1, 6, 7};
                unsigned b = s->lines == 4 ? BIT4[i] : i;

                if (((k >> b) & 1u) && ((v >> b) & 1u)) {
                    s->ln[i].dschng = 0;
                }
            }
            break;
        default:
            break;   /* status / summaries / unlisted: writes ignored */
    }
}

/* ---- Bus ------------------------------------------------------------------------------------------- */

static void address_of(const CmState *s, unsigned *line, unsigned *reg, int *known) {
    unsigned v = 0;
    unsigned k = 0;
    unsigned i;
    unsigned n = s->dev->addr_count;

    for (i = 0; i < n; i++) {
        LogicValue a = in_level(s, &s->pins->addr[i], LOGIC_X);

        if (a == LOGIC_1) v |= 1u << i;
        if (a != LOGIC_X) k |= 1u << i;
    }
    *reg = v & 7u;
    *line = v >> 3;
    /* The summaries ignore the line bits. */
    *known = (k & 7u) == 7u && (((v & 7u) == 4 || (v & 7u) == 5) || k == (1u << n) - 1);
}

static void drive_data(CmState *s) {
    uint8_t v = 0;
    uint8_t k = 0;
    int release = s->cycle != 1 && s->cycle != 3;

    if (s->cycle == 1 && s->cyc_known) {
        reg_peek(s, s->cyc_line, s->cyc_reg, &v, &k);
    }
    su_drive_bus(&s->o, s->sim, s->instance, s->pins->data, 8, v, k, release,
                 release ? t_of(s, T_DDZ) : t_of(s, T_DD));
}

static void bus_update(CmState *s) {
    LogicValue cs = in_level(s, &s->pins->select, LOGIC_1);
    LogicValue ds = in_level(s, &s->pins->strobe, LOGIC_1);
    LogicValue wr = in_level(s, &s->pins->write, LOGIC_X);
    LogicValue active = su_and(su_not(cs), su_not(ds));
    unsigned line;
    unsigned reg;
    int known;
    int cycle = 0;

    /* RDY: low after CS falls, released after it rises. */
    if (cs != s->cs_level) {
        su_drive_od(&s->o, s->sim, s->instance, s->pins->ready, cs,
                    cs == LOGIC_0 ? t_of(s, T_RDL) : t_of(s, T_RDH));
        s->cs_level = cs;
    }
    if (s->in_reset || s->unknown) {
        s->cycle = 0;
        drive_data(s);
        return;
    }
    address_of(s, &line, &reg, &known);
    if (active == LOGIC_1 && wr == LOGIC_1) {
        cycle = 1;
    } else if (active == LOGIC_1 && wr == LOGIC_0) {
        cycle = 2;
    } else if (active != LOGIC_0) {
        cycle = 3;
    }
    if (cycle != s->cycle || (cycle != 0 && (line != s->cyc_line || reg != s->cyc_reg))) {
        /* The previous cycle ended. */
        if (s->cycle == 1 && s->cyc_known && cycle == 0) {
            read_done(s, s->cyc_line, s->cyc_reg);
        } else if (s->cycle == 2) {
            unsigned v;
            unsigned k;

            su_read_bus(s->sim, s->instance, s->pins->data, 8, &v, &k);
            if (cycle == 3 || !s->cyc_known) {
                k = 0;
            }
            if (s->cyc_known) {
                write_reg(s, s->cyc_line, s->cyc_reg, (uint8_t)v, (uint8_t)k);
            }
        }
        s->cycle = cycle;
        s->cyc_line = line;
        s->cyc_reg = reg;
        s->cyc_known = known;
    }
    drive_data(s);
    update_outputs(s, t_of(s, T_ID));
}

/* ---- Serial ------------------------------------------------------------------------------------------ */

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    CmLine *l = context;

    if (!l->thr_full || cmd_bit(l, 0) != LOGIC_1 || l->brk || cmd_bit(l, 3) == LOGIC_1 ||
        op_mode(l) == 1 || op_mode(l) == 3) {
        return 0;
    }
    *data = l->thr.v;
    *known = l->thr.k;
    l->thr_full = 0;
    l->txrdy = LOGIC_1;
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    CmLine *l = context;

    (void)known;
    serial_line_log(&l->line, data);
}

static void tx_tick(CmState *s, CmLine *l) {
    SerialTxEvents events = {l, tx_fetch, NULL, tx_done};
    int busy = serial_tx_busy(&l->line.tx);

    l->tsr = serial_tx_tick(&l->line.tx, &l->format, &events);
    if (!serial_tx_busy(&l->line.tx)) {
        /* TxBRK: spacing at the end of the character being sent. */
        if (cmd_bit(l, 3) == LOGIC_1) {
            l->brk = 1;
        }
        if (busy && !l->thr_full && cmd_bit(l, 0) == LOGIC_1 && op_mode(l) != 1 &&
            op_mode(l) != 3) {
            l->txemt = LOGIC_1;
        }
    }
    (void)s;
}

static void on_received(void *context, const SerialRxChar *c) {
    CmLine *l = context;
    uint8_t mask = (uint8_t)((1u << l->format.data_bits) - 1);

    if ((c->flags & SERIAL_RX_FE) && !(c->flags & SERIAL_RX_BI)) {
        l->resync = 1;
    }
    if (l->count >= 2) {
        /* The FIFO is full: the character is lost (ORR). */
        if (cmd_bit(l, 4) != LOGIC_1) {
            l->orr = LOGIC_1;
        }
        return;
    }
    l->fifo[l->count].data = (uint16_t)(c->data & mask);
    l->fifo[l->count].known = l->format.format_x ? 0 : (uint16_t)((c->known & mask) | (uint16_t)~mask);
    l->fifo[l->count].flags = c->flags;
    l->fifo[l->count].flags_known = c->flags_known;
    l->count++;
    if (l->count == 1) {
        show_top(l);
    }
}

static void rx_tick(CmState *s, CmLine *l, unsigned i) {
    SerialRxEvents events = {l, on_received, NULL};
    LogicValue pin = serial_level(s->sim, s->instance, s->pins->line[i][L_RX], LOGIC_X);
    LogicValue level = serial_line_rx_input(&l->line, s->sim, s->instance, pin, &l->format);
    SerialRxPhase before = l->line.rx.phase;
    unsigned mode = op_mode(l);

    if (mode == 2) {
        level = l->tsr;   /* local loopback: the transmitter feeds the receiver */
    }
    if (mode == 3 || cmd_bit(l, 2) != LOGIC_1) {
        l->line.rx.last = level;
        return;
    }
    l->resync = 0;
    serial_rx_tick(&l->line.rx, level, &l->format, &events);
    /* A start needs RXD high for half a bit before the 1 -> 0 (p4). */
    if (before == SERIAL_RX_IDLE && l->line.rx.phase == SERIAL_RX_START &&
        l->mark_before < (unsigned long)l->rxb.divisor * l->format.factor / 2) {
        su_rx_no_resync(&l->line.rx, level);
    }
    if (l->resync) {
        su_rx_no_resync(&l->line.rx, level);
    }
}

/* The receiver input's high time, in CLK cycles (for the start rule). */
static void track_mark(CmState *s, CmLine *l, unsigned i) {
    LogicValue level = op_mode(l) == 2
        ? l->tsr : serial_level(s->sim, s->instance, s->pins->line[i][L_RX], LOGIC_X);

    if (level == LOGIC_1) {
        l->mark_clk++;
    } else if (l->in_level == LOGIC_1) {
        l->mark_before = l->mark_clk;
        l->mark_clk = 0;
    } else {
        l->mark_clk = 0;
    }
    l->in_level = level;
}

static void clk_edge(CmState *s, LogicValue level) {
    int rising = s->clk == LOGIC_0 && level == LOGIC_1;
    unsigned i;

    s->clk = level;
    if (!rising || s->in_reset || s->unknown) {
        return;
    }
    for (i = 0; i < s->lines; i++) {
        CmLine *l = &s->ln[i];

        track_mark(s, l, i);
        if (serial_baud_edge(&l->txb, 1, LOGIC_1)) {
            tx_tick(s, l);
        }
        if (serial_baud_edge(&l->rxb, 1, LOGIC_1)) {
            rx_tick(s, l, i);
        }
    }
    scanner_clock(s);
    update_outputs(s, 0);
    drive_data(s);
}

/* ---- Reset, modem inputs -------------------------------------------------------------------------- */

static void apply_reset(CmState *s) {
    LogicValue r = su_not(in_level(s, &s->pins->reset, LOGIC_1));
    unsigned i;

    if (r == LOGIC_X) {
        s->unknown = 1;
        s->in_reset = 0;
    } else if (r == LOGIC_1) {
        s->unknown = 0;
        s->in_reset = 1;
        for (i = 0; i < s->lines; i++) {
            reset_line(&s->ln[i]);
            serial_line_sender_reset(&s->ln[i].line, s->sim, s->instance);
        }
        s->pos = 0;
        s->stopped = 0;
        s->cycle = 0;
    } else {
        if (s->unknown) {
            for (i = 0; i < s->lines; i++) {
                reset_line(&s->ln[i]);
                s->ln[i].mr1 = s->ln[i].mr2 = s->ln[i].cmd = su_reg(0, 0);
                update_format(&s->ln[i]);
                load_rates(&s->ln[i]);
            }
        }
        s->unknown = 0;
        s->in_reset = 0;
    }
    bus_update(s);
    update_outputs(s, 0);
}

static void modem_changed(CmState *s, unsigned i) {
    CmLine *l = &s->ln[i];
    LogicValue dsr = serial_level(s->sim, s->instance, s->pins->line[i][L_DSR], LOGIC_X);
    LogicValue dcd = serial_level(s->sim, s->instance, s->pins->line[i][L_DCD], LOGIC_X);

    if ((dsr != l->dsr || dcd != l->dcd) && !s->in_reset && !s->unknown) {
        l->dschng = 1;
    }
    l->dsr = dsr;
    l->dcd = dcd;
    drive_data(s);
}

/* ---- Engine callbacks ---------------------------------------------------------------------------- */

static void cm_start(Sim *sim, SimInstance *instance, void *raw) {
    CmState *s = raw;
    unsigned i;

    s->sim = sim;
    s->instance = instance;
    s->clk = serial_level(sim, instance, s->pins->clock, LOGIC_X);
    s->cs_level = LOGIC_Z;
    for (i = 0; i < s->lines; i++) {
        s->ln[i].dsr = serial_level(sim, instance, s->pins->line[i][L_DSR], LOGIC_X);
        s->ln[i].dcd = serial_level(sim, instance, s->pins->line[i][L_DCD], LOGIC_X);
    }
    apply_reset(s);
}

static void cm_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    CmState *s = raw;
    const CmPins *p = s->pins;
    unsigned i;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (in_has(&p->reset, pin)) {
        apply_reset(s);
        return;
    }
    if (pin == p->clock) {
        clk_edge(s, serial_level(sim, instance, pin, LOGIC_X));
        return;
    }
    for (i = 0; i < s->lines; i++) {
        if (pin == p->line[i][L_DSR] || pin == p->line[i][L_DCD]) {
            modem_changed(s, i);
            return;
        }
        if (pin == p->line[i][L_RX]) {
            update_outputs(s, 0);   /* echo and remote loopback follow RXD */
            return;
        }
    }
    bus_update(s);
}

/* ---- Terminal ------------------------------------------------------------------------------------------ */

static unsigned cm_channels(const void *raw) {
    return ((const CmState *)raw)->lines;
}

static SerialLine *cm_line(void *raw, unsigned channel) {
    return &((CmState *)raw)->ln[channel].line;
}

static int cm_async(const void *raw, unsigned channel) {
    (void)raw;
    (void)channel;
    return 1;
}

/* ---- Create / destroy / loader --------------------------------------------------------------------- */

static void *cm_create(const Part *part, const void *data, const void *pins,
                       const SerialGrade *grade, const void *options) {
    CmState *s = calloc(1, sizeof *s);
    size_t i;

    (void)options;
    if (s == NULL) {
        return NULL;
    }
    s->base.model = &COM78C80X_MODEL;
    s->dev = data;
    s->pins = pins;
    s->grade = grade;
    s->lines = s->dev->lines;
    s->o.count = part->pin_count;
    s->o.out = malloc(part->pin_count * sizeof *s->o.out);
    if (s->o.out == NULL) {
        free(s);
        return NULL;
    }
    for (i = 0; i < part->pin_count; i++) {
        s->o.out[i] = LOGIC_Z;
    }
    for (i = 0; i < s->lines; i++) {
        serial_line_init(&s->ln[i].line, s->pins->line[i][L_RX]);
        s->ln[i].thr = su_reg(0, 0);
        reset_line(&s->ln[i]);
        s->ln[i].dsr = s->ln[i].dcd = LOGIC_X;
    }
    s->clk = LOGIC_X;
    s->cs_level = LOGIC_Z;
    return s;
}

static void cm_destroy(void *raw) {
    CmState *s = raw;
    unsigned i;

    for (i = 0; i < s->lines; i++) {
        serial_line_free(&s->ln[i].line);
    }
    free(s->o.out);
    free(s);
}

static void *cm_load_device(KindContext *ctx, const cJSON *json, unsigned *channels) {
    CmDevice *d = kind_alloc(ctx, sizeof *d);
    const cJSON *bus;
    const cJSON *list;
    const cJSON *item;
    uint64_t lines;
    unsigned n = 0;
    unsigned ln_count = 0;

    if (d == NULL || kind_uint(ctx, json, "lines", 4, &lines) != 0) {
        return NULL;
    }
    if (lines != 2 && lines != 4) {
        kind_error(ctx, "\"lines\" must be 2 or 4");
        return NULL;
    }
    d->lines = (unsigned)lines;
    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    kind_push(ctx, "bus");
    if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 ||
        su_load_names(ctx, bus, "data", 1, d->data, 8, 8, NULL) != 0 ||
        su_load_names(ctx, bus, "address", 1, d->addr, 5, d->lines == 4 ? 5 : 4, &d->addr_count) != 0 ||
        kind_string(ctx, bus, "select", &d->select) != 0 ||
        kind_string(ctx, bus, "strobe", &d->strobe) != 0 ||
        kind_string(ctx, bus, "write", &d->write) != 0 ||
        kind_string(ctx, bus, "ready", &d->ready) != 0 ||
        kind_string(ctx, bus, "reset", &d->reset) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    if (kind_string(ctx, json, "clock", &d->clock) != 0) {
        return NULL;
    }
    list = kind_member(ctx, json, "channels", 1);
    if (list == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) != (int)d->lines) {
        kind_error(ctx, "\"channels\" must be an array of %u lines", d->lines);
        return NULL;
    }
    cJSON_ArrayForEach(item, list) {
        unsigned k;

        kind_push(ctx, "channels[%u]", n);
        if (kind_check_keys(ctx, item, LINE_KEYS, NULL) != 0) {
            return NULL;
        }
        for (k = 0; k < L_COUNT; k++) {
            if (kind_string(ctx, item, LINE_KEYS[k], &d->line[n][k]) != 0) {
                return NULL;
            }
        }
        kind_pop(ctx);
        n++;
    }
    {
        const cJSON *irq = kind_member(ctx, json, "interrupt", 1);

        if (irq == NULL) {
            return NULL;
        }
        kind_push(ctx, "interrupt");
        if (kind_check_keys(ctx, irq, INT_KEYS, NULL) != 0 ||
            kind_string(ctx, irq, "irq", &d->irq) != 0 ||
            kind_string(ctx, irq, "txrx", &d->txrx) != 0 ||
            su_load_names(ctx, irq, "line", 1, d->irqln, 2, d->lines == 4 ? 2 : 1, &ln_count) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    *channels = d->lines;
    return d;
}

static int bind_in(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                   const char *name, CmIn *out) {
    kind_push(ctx, "%s", key);
    out->count = (unsigned)kind_pins(ctx, pins, pin_count, name, out->pin, DUP);
    if (out->count == 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static void *cm_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const CmDevice *d = raw;
    CmPins *p = kind_alloc(ctx, sizeof *p);
    unsigned i;

    if (p == NULL ||
        su_bind_names(ctx, pins, pin_count, "bus.data", d->data, p->data, 8) != 0 ||
        bind_in(ctx, pins, pin_count, "bus.select", d->select, &p->select) != 0 ||
        bind_in(ctx, pins, pin_count, "bus.strobe", d->strobe, &p->strobe) != 0 ||
        bind_in(ctx, pins, pin_count, "bus.write", d->write, &p->write) != 0 ||
        bind_in(ctx, pins, pin_count, "bus.reset", d->reset, &p->reset) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "bus.ready", d->ready, &p->ready) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "clock", d->clock, &p->clock) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "interrupt.irq", d->irq, &p->irq) != 0 ||
        serial_bind_pin(ctx, pins, pin_count, "interrupt.txrx", d->txrx, &p->txrx) != 0) {
        return NULL;
    }
    for (i = 0; i < d->addr_count; i++) {
        if (bind_in(ctx, pins, pin_count, "bus.address", d->addr[i], &p->addr[i]) != 0) {
            return NULL;
        }
    }
    p->irqln[0] = p->irqln[1] = SERIAL_NO_PIN;
    if (su_bind_names(ctx, pins, pin_count, "interrupt.line", d->irqln, p->irqln,
                      d->lines == 4 ? 2 : 1) != 0) {
        return NULL;
    }
    for (i = 0; i < MAX_LINES; i++) {
        unsigned k;

        for (k = 0; k < L_COUNT; k++) {
            p->line[i][k] = SERIAL_NO_PIN;
        }
    }
    for (i = 0; i < d->lines; i++) {
        char key[32];

        snprintf(key, sizeof key, "channels[%u]", i);
        if (su_bind_object(ctx, pins, pin_count, key, LINE_KEYS, d->line[i], p->line[i]) != 0) {
            return NULL;
        }
    }
    return p;
}

static int cm_pin_open_drain(const void *data, const void *pins, const void *options, size_t pin) {
    const CmPins *p = pins;

    (void)data;
    (void)options;
    return pin == p->irq || pin == p->ready;
}

const SerialModel COM78C80X_MODEL = {
    "com78c80x", KEYS, TIMING, NULL,
    cm_load_device, cm_bind, NULL, cm_pin_open_drain,
    cm_create, cm_destroy, cm_start, cm_pin_changed, NULL,
    cm_channels, cm_line, cm_async
};
