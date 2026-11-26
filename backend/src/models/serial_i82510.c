#include "serial_i82510_int.h"
const char *const i82_TIMING[] = {"trldv", "trhdz", "ttxd", NULL};

const char *const i82_KEYS[] = {"bus", "clock", "serial", NULL};
const char *const i82_BUS_KEYS[] = {"data", "address", "read", "write", "select", "reset", NULL};
const char *const i82_CLOCK_KEYS[] = {"in", "out", NULL};
const char *const i82_SERIAL_KEYS[] = {"tx", "rx", "int", "cts", "rts", "dtr", "ri", "dsr", "dcd",
                                          NULL};
const uint8_t i82_RESET_VALUE[R_COUNT] = {
    0x02, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x1E,
    0x00, 0x00, 0x04, 0x84, 0xFC, 0x0F, 0x00, 0x00
};

static SimTime t_of(const I5State *s, unsigned i) {
    return s->grade->t[i];
}

uint8_t i82_rv(const I5State *s, unsigned i) {
    return s->r[i].v;
}

/* ---- Formats ---------------------------------------------------------------------------- */

static unsigned char_length(const I5State *s) {
    if (i82_rv(s, R_TMD) & 0x20u) {
        return 9;   /* NBCL */
    }
    return 5 + (i82_rv(s, R_LCR) & 3u);
}

static int ulan(const I5State *s) {
    return (i82_rv(s, R_IMD) >> 1) & 1u;
}

static int soft_parity(const I5State *s) {
    return (i82_rv(s, R_TMD) & 0x04u) && (i82_rv(s, R_LCR) & 0x08u) && !(i82_rv(s, R_LCR) & 0x30u);
}

static int tx_1x(const I5State *s) {
    return (i82_rv(s, R_CLCF) >> 5) & 1u;
}

static int rx_1x(const I5State *s) {
    return (i82_rv(s, R_CLCF) >> 7) & 1u;
}

/* Stop length in quarter bits (16x), or whole bits x 4 (1x); 0 = undefined. */
static unsigned stop_quarters(const I5State *s) {
    unsigned code = ((i82_rv(s, R_TMD) & 3u) << 1) | ((i82_rv(s, R_LCR) >> 2) & 1u);
    static const unsigned Q16[8] = {4, 8, 3, 4, 5, 6, 7, 8};

    if (tx_1x(s)) {
        return code < 2 ? 0 : code == 7 ? 8 : 4;
    }
    if (code == 1) {
        return char_length(s) == 5 ? 6 : 8;
    }
    return Q16[code];
}

void i82_update_format(I5State *s) {
    SerialFormat f = serial_format_default();
    uint8_t lcr = i82_rv(s, R_LCR);
    unsigned q = stop_quarters(s);

    f.data_bits = char_length(s) + (ulan(s) && char_length(s) < 9 ? 1u : 0u);
    if (!(lcr & 0x08u)) {
        f.parity = SERIAL_PARITY_NONE;
    } else if (soft_parity(s)) {
        f.parity = SERIAL_PARITY_MARK;   /* per character, from TXF.SP */
    } else {
        static const SerialParity P[4] = {
            SERIAL_PARITY_ODD, SERIAL_PARITY_EVEN, SERIAL_PARITY_MARK, SERIAL_PARITY_SPACE
        };

        f.parity = P[(lcr >> 4) & 3u];
    }
    f.factor = tx_1x(s) ? SU_ONE_X_FACTOR : 16;
    f.stop_halves = q / 2 > 0 ? q / 2 : 1;
    f.format_x = s->r[R_LCR].k != 0xFF || s->r[R_TMD].k != 0xFF || q == 0 ||
                 ((i82_rv(s, R_TMD) >> 3) & 3u) == 1;
    s->txf = f;
    f.factor = rx_1x(s) ? SU_ONE_X_FACTOR : 16;
    f.stop_halves = 2;   /* the receiver takes any stop of 3/4 bit or more */
    f.format_x = s->r[R_LCR].k != 0xFF;
    s->rxf = f;
}

/* ---- Status, GSR, GIR -------------------------------------------------------------------- */

static unsigned rx_depth(const I5State *s) {
    return (i82_rv(s, R_IMD) & 0x04u) ? 1u : FIFO_MAX;
}

int i82_rfir(const I5State *s) {
    unsigned rft = (i82_rv(s, R_FMD) >> 4) & 3u;

    return s->rx_count > rft && !s->rfir_ack;
}

static int tfir(const I5State *s) {
    unsigned tft = i82_rv(s, R_FMD) & 3u;

    return s->tx_count <= tft;
}

static int tx_idle(const I5State *s) {
    return !s->tx_enabled || (s->tx_count == 0 && !serial_tx_busy(&s->line.tx));
}

uint8_t i82_gsr_of(const I5State *s) {
    uint8_t g = 0;

    if (s->tmst & i82_rv(s, R_TMIE) & 3u) g |= 0x20;
    if (tx_idle(s)) g |= 0x10;
    if (s->msr_delta & i82_rv(s, R_MIE) & 0x0Fu) g |= 0x08;
    if (s->rst & i82_rv(s, R_RIE) & 0xFEu) g |= 0x04;
    if (tfir(s) && !s->tx_hold_state) g |= 0x02;
    if (i82_rfir(s)) g |= 0x01;
    return g;
}

/* The highest enabled pending block: 5 timer .. 0 modem; -1 none. */
int i82_gir_block(const I5State *s) {
    static const uint8_t BIT[6] = {0x20, 0x10, 0x04, 0x01, 0x02, 0x08};
    static const int CODE[6] = {5, 4, 3, 2, 1, 0};
    uint8_t pend = i82_gsr_of(s) & i82_rv(s, R_GER) & 0x3Fu;
    unsigned i;

    for (i = 0; i < 6; i++) {
        if (pend & BIT[i]) {
            return CODE[i];
        }
    }
    return -1;
}

uint8_t i82_gir_of(const I5State *s) {
    int b = i82_gir_block(s);

    return (uint8_t)((s->bank << 5) | (b >= 0 ? (unsigned)b << 1 : 0u) | (b >= 0 ? 0u : 1u));
}

static int auto_ack(const I5State *s) {
    return (i82_rv(s, R_IMD) & 0x08u) != 0;
}

static LogicValue int_level(const I5State *s) {
    int pending = i82_gir_block(s) >= 0;

    if (s->int_force_low > 0) {
        return LOGIC_0;
    }
    if (!(i82_rv(s, R_IMD) & 0x08u)) {
        return su_bool(s->int_latched || pending);   /* manual acknowledge */
    }
    return su_bool(pending);
}

uint8_t i82_lsr_of(const I5State *s) {
    uint8_t g = i82_gsr_of(s);

    return (uint8_t)(((g & 0x10) ? 0x40 : 0) | ((g & 0x02) ? 0x20 : 0) | (s->rst & 0x1Eu) |
                     (i82_rfir(s) ? 1 : 0));
}

/* ---- Pins -------------------------------------------------------------------------------- */

static int local_loop(const I5State *s) {
    return (i82_rv(s, R_MCR) & 0x10u) && !(i82_rv(s, R_IMD) & 0x01u);
}

static int remote_loop(const I5State *s) {
    return (i82_rv(s, R_MCR) & 0x10u) && (i82_rv(s, R_IMD) & 0x01u);
}

static int auto_echo(const I5State *s) {
    return !(i82_rv(s, R_MCR) & 0x10u) && (i82_rv(s, R_IMD) & 0x01u);
}

/* The modem inputs as MSR status (1 = active): CTS, DSR, RI, DCD. */
static void modem_now(const I5State *s, LogicValue now[4]) {
    uint8_t pmd = i82_rv(s, R_PMD);
    uint8_t mcr = i82_rv(s, R_MCR);

    if (local_loop(s)) {
        /* OUT1 -> RI, OUT2 -> DCD, DTR -> DSR, RTS -> CTS. */
        now[0] = su_bool(mcr & 0x02u);
        now[1] = su_bool(mcr & 0x01u);
        now[2] = su_bool(mcr & 0x04u);
        now[3] = su_bool(mcr & 0x08u);
        return;
    }
    now[0] = su_not(serial_level(s->sim, s->instance, s->pins->pin[P_CTS], LOGIC_X));
    now[1] = (pmd & 0x20u) ? su_not(serial_level(s->sim, s->instance, s->pins->pin[P_DSR], LOGIC_X))
                           : LOGIC_X;
    now[2] = (pmd & 0x08u) ? su_not(serial_level(s->sim, s->instance, s->pins->pin[P_RI], LOGIC_X))
                           : LOGIC_X;
    now[3] = (pmd & 0x80u) ? su_not(serial_level(s->sim, s->instance, s->pins->pin[P_DCD], LOGIC_X))
                           : LOGIC_X;
}

void i82_track_modem(I5State *s) {
    LogicValue now[4];
    static const uint8_t DELTA[4] = {0x01, 0x02, 0x04, 0x08};
    unsigned i;

    modem_now(s, now);
    for (i = 0; i < 4; i++) {
        LogicValue was = s->msr_level[i];

        if (was != LOGIC_X && now[i] != LOGIC_X && was != now[i]) {
            /* DRI: the RI input high to low (status 0 -> 1). */
            if (i != 2 || now[i] == LOGIC_1) {
                s->msr_delta |= DELTA[i];
            }
        }
        s->msr_level[i] = now[i];
    }
}

uint8_t i82_msr_of(const I5State *s, uint8_t *k) {
    uint8_t v = s->msr_delta & 0x0Fu;
    static const unsigned BIT[4] = {4, 5, 6, 7};
    unsigned i;

    *k = 0x0F;
    for (i = 0; i < 4; i++) {
        if (s->msr_level[i] == LOGIC_1) v |= (uint8_t)(1u << BIT[i]);
        if (s->msr_level[i] != LOGIC_X) *k |= (uint8_t)(1u << BIT[i]);
    }
    return v;
}

static int handshake_rts(const I5State *s) {
    unsigned tm = (i82_rv(s, R_TMD) >> 3) & 3u;

    if (tm == 2) {
        return s->tx_enabled;
    }
    if (tm == 3) {
        return s->tx_enabled && (s->tx_count > 0 || serial_tx_busy(&s->line.tx));
    }
    return 0;
}

static LogicValue txd_level(const I5State *s) {
    if (remote_loop(s)) {
        return serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    }
    if (local_loop(s)) {
        return LOGIC_1;
    }
    if (i82_rv(s, R_LCR) & 0x40u) {
        return LOGIC_0;   /* SBK */
    }
    return s->tsr;
}

void i82_update_outputs(I5State *s) {
    const size_t *p = s->pins->pin;
    uint8_t mcr = i82_rv(s, R_MCR);
    uint8_t pmd = i82_rv(s, R_PMD);
    int loop = local_loop(s);

    if (s->unknown || s->strap_x == LOGIC_X) {
        static const unsigned OUTS[] = {P_TX, P_INT, P_RTS, P_DTR};
        unsigned i;

        for (i = 0; i < sizeof OUTS / sizeof OUTS[0]; i++) {
            su_drive(&s->o, s->sim, s->instance, p[OUTS[i]], LOGIC_X, 0);
        }
        return;
    }
    if (s->in_reset) {
        /* RTS and DTR are strap inputs with weak pull-ups during RESET. */
        su_drive(&s->o, s->sim, s->instance, p[P_TX], LOGIC_1, 0);
        su_drive(&s->o, s->sim, s->instance, p[P_INT], LOGIC_0, 0);
        return;
    }
    su_drive(&s->o, s->sim, s->instance, p[P_TX], txd_level(s), t_of(s, T_TXD));
    if (!(auto_ack(s)) && i82_gir_block(s) >= 0) {
        s->int_latched = 1;   /* manual acknowledge: INT holds until ICM.INTA */
    }
    su_drive(&s->o, s->sim, s->instance, p[P_INT], int_level(s), 0);
    /* Modem outputs (1 in MCR = pin low); inactive in local loopback (P4). */
    su_drive(&s->o, s->sim, s->instance, p[P_RTS],
             loop ? LOGIC_1 : su_bool(!((mcr & 0x02u) || handshake_rts(s))), 0);
    if (pmd & 0x04u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DTR], loop ? LOGIC_1 : su_bool(!(mcr & 0x01u)), 0);
    } else {
        su_drive_clock(&s->o, s->sim, s->instance, p[P_DTR], s->brg[1].out, 0);   /* TB */
    }
    if (pmd & 0x20u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DSR], LOGIC_Z, 0);
    } else if (pmd & 0x10u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DSR], loop ? LOGIC_1 : su_bool(!(mcr & 0x20u)), 0);
    } else {
        su_drive_clock(&s->o, s->sim, s->instance, p[P_DSR], s->brg[0].out, 0);   /* TA */
    }
    if (pmd & 0x80u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DCD], LOGIC_Z, 0);
    } else if (pmd & 0x40u) {
        su_drive(&s->o, s->sim, s->instance, p[P_DCD], loop ? LOGIC_1 : su_bool(!(mcr & 0x04u)), 0);
    } else {
        su_drive_clock(&s->o, s->sim, s->instance, p[P_DCD], s->sysclk, 0);   /* ICLK */
    }
    if (!s->crystal) {
        su_drive(&s->o, s->sim, s->instance, s->pins->clock[K_OUT],
                 loop ? LOGIC_1 : su_bool(!(mcr & 0x08u)), 0);   /* OUT2 */
    }
}

void i82_refresh(I5State *s) {
    i82_track_modem(s);
    i82_update_outputs(s);
    serial_bus_refresh(&s->bus, s->sim, s->instance, &s->events);
}

/* A service operation (automatic acknowledge): INT low two system clocks. */
void i82_serviced(I5State *s) {
    if (i82_rv(s, R_IMD) & 0x08u) {
        s->int_force_low = 2;
    }
}

/* ---- BRGs and timers ---------------------------------------------------------------------- */

uint32_t i82_divisor_of(const I5State *s, unsigned which) {
    unsigned lo = which == 0 ? R_BAL : R_BBL;

    return (uint32_t)i82_rv(s, lo) | ((uint32_t)i82_rv(s, lo + 1) << 8);
}

int i82_brg_mode(const I5State *s, unsigned which) {
    return (i82_rv(s, which == 0 ? R_BACF : R_BBCF) >> 2) & 1u;
}

void i82_brg_load(I5State *s, unsigned which) {
    unsigned lo = which == 0 ? R_BAL : R_BBL;
    uint32_t d = i82_divisor_of(s, which);
    int known = s->r[lo].k == 0xFF && s->r[lo + 1].k == 0xFF;

    serial_baud_load(&s->brg[which].baud, d, known);   /* divisor 0: forbidden, stopped */
    s->brg[which].timer_run = 0;
    s->brg[which].out = i82_brg_mode(s, which) ? s->brg[which].baud.baudout : LOGIC_0;
}

static void serial_tick_tx(I5State *s);
static void serial_tick_rx(I5State *s);

/* One input edge to a BRG; returns 1 on its output rising edge (a tick). */
static int brg_edge(I5State *s, unsigned which, int rising, LogicValue level) {
    I5Brg *g = &s->brg[which];
    int tick;

    if (!i82_brg_mode(s, which)) {
        return 0;
    }
    tick = serial_baud_edge(&g->baud, rising, level);
    g->out = g->baud.baudout;
    return tick;
}

/* A system clock edge on a BRG in timer mode. */
static void timer_clock(I5State *s, unsigned which) {
    I5Brg *g = &s->brg[which];
    uint8_t tmcr_gate = which == 0 ? 0x10 : 0x20;

    g->pulse = 0;
    if (i82_brg_mode(s, which) || !g->timer_run || !(s->tmcr.v & tmcr_gate)) {
        g->out = LOGIC_0;
        return;
    }
    if (g->timer_count > 1) {
        g->timer_count--;
    }
    if (g->timer_count <= 1) {
        s->tmst |= which == 0 ? 0x01 : 0x02;
        g->timer_run = 0;
        g->pulse = 1;
    }
    g->out = su_bool(g->pulse);
}

/* The 16x / 1x clocks a BRG tick feeds. */
static void brg_ticked(I5State *s, unsigned which) {
    uint8_t clcf = i82_rv(s, R_CLCF);
    unsigned tx_src = (clcf >> 4) & 1u ? 0u : 1u;   /* TxCS: 1 BRGA, 0 BRGB */
    unsigned rx_src = (clcf >> 6) & 1u ? 0u : 1u;

    if (!tx_1x(s) && tx_src == which) {
        serial_tick_tx(s);
    }
    if (!rx_1x(s) && rx_src == which) {
        serial_tick_rx(s);
    }
}

void i82_sysclk_edge(I5State *s, LogicValue level) {
    int rising = s->sysclk == LOGIC_0 && level == LOGIC_1;
    uint8_t bacf = i82_rv(s, R_BACF);
    uint8_t bbcf = i82_rv(s, R_BBCF);
    unsigned i;

    s->sysclk = level;
    if (s->in_reset || s->unknown || s->powered_down) {
        return;
    }
    if (rising) {
        if (s->int_force_low > 0) {
            s->int_force_low--;
        }
        for (i = 0; i < 2; i++) {
            timer_clock(s, i);
        }
    }
    /* BRGA from the system clock (BACS = 0). */
    if (!(bacf & 0x40u) && brg_edge(s, 0, rising, level)) {
        brg_ticked(s, 0);
        if (((bbcf >> 6) & 3u) == 2u && brg_edge(s, 1, 1, LOGIC_1)) {
            brg_ticked(s, 1);
        }
    } else if (!(bacf & 0x40u) && !rising && ((bbcf >> 6) & 3u) == 2u) {
        brg_edge(s, 1, 0, LOGIC_0);   /* BRGA's output fell: BRGB's input */
    }
    if (((bbcf >> 6) & 3u) == 0u && brg_edge(s, 1, rising, level)) {
        brg_ticked(s, 1);
    }
    i82_update_outputs(s);
}

void i82_sclk_edge(I5State *s, LogicValue level) {
    int rising = s->sclk == LOGIC_0 && level == LOGIC_1;
    int falling = s->sclk == LOGIC_1 && level == LOGIC_0;
    uint8_t bacf = i82_rv(s, R_BACF);
    uint8_t bbcf = i82_rv(s, R_BBCF);

    s->sclk = level;
    if (s->in_reset || s->unknown || s->powered_down || (!rising && !falling) ||
        (i82_rv(s, R_PMD) & 0x08u)) {
        return;   /* the pin is RI, not SCLK */
    }
    if ((bacf & 0x40u) && brg_edge(s, 0, rising, level)) {
        brg_ticked(s, 0);
    }
    if (((bbcf >> 6) & 3u) == 1u && brg_edge(s, 1, rising, level)) {
        brg_ticked(s, 1);
    }
    /* 1x clocks: TXD changes after SCLK falls, RXD is sampled as it rises (p40). */
    if (tx_1x(s) && falling) {
        serial_tick_tx(s);
        serial_tick_tx(s);
    }
    if (rx_1x(s) && rising) {
        serial_tick_rx(s);
        serial_tick_rx(s);
    }
    i82_refresh(s);
}

/* ---- Transmitter ------------------------------------------------------------------------- */

static int cts_ok(const I5State *s) {
    unsigned tm = (i82_rv(s, R_TMD) >> 3) & 3u;

    if (tm < 2) {
        return 1;   /* manual: CTS is ignored by the transmitter */
    }
    return s->msr_level[0] == LOGIC_1;
}

static int tx_fetch(void *context, uint16_t *data, uint16_t *known) {
    I5State *s = context;
    I5TxChar c;
    unsigned n = char_length(s);

    if (s->tx_count == 0 || !s->tx_enabled || s->tx_stop_hold > 0 || !cts_ok(s) || remote_loop(s)) {
        return 0;
    }
    c = s->tx[0];
    memmove(&s->tx[0], &s->tx[1], (s->tx_count - 1) * sizeof s->tx[0]);
    s->tx_count--;
    *data = c.data;
    *known = c.known;
    if (ulan(s) && n < 9) {
        /* uLAN: the address marker is the bit after the data. */
        *data = (uint16_t)((*data & ((1u << n) - 1)) | ((c.txf & 0x80u) ? 1u << n : 0));
        *known |= (uint16_t)(1u << n);
    }
    if (soft_parity(s)) {
        s->txf.parity = (c.txf & 0x40u) ? SERIAL_PARITY_MARK : SERIAL_PARITY_SPACE;
    }
    return 1;
}

static void tx_done(void *context, uint16_t data, uint16_t known) {
    I5State *s = context;
    unsigned q = stop_quarters(s);

    (void)known;
    serial_line_log(&s->line, s->sim, data);
    /* Odd quarter stops: the engine sends whole halves; the last quarter
       is held as mark before the next start. */
    if (!tx_1x(s) && q % 2 == 1) {
        s->tx_stop_hold = 4;
    }
}

static void echo_drain(I5State *s);

static void serial_tick_tx(I5State *s) {
    SerialTxEvents events = {s, tx_fetch, NULL, tx_done};
    unsigned before = s->tx_count;

    /* No reset / unknown / power-down check: both callers (i82_sysclk_edge,
       i82_sclk_edge, directly or through brg_ticked) return before ticking. */
    if (s->tx_stop_hold > 0) {
        s->tx_stop_hold--;
    }
    s->tsr = serial_tx_tick(&s->line.tx, &s->txf, &events);
    if (s->tx_count != before) {
        echo_drain(s);
    }
}

/* ---- Receiver ---------------------------------------------------------------------------- */

static int is_control(const I5State *s, unsigned c) {
    unsigned mode = ((i82_rv(s, R_RMD) >> 6) & 1u) << 1 | ((i82_rv(s, R_RMD) >> 7) & 1u);

    if (mode == 1) {
        return c <= 0x1F || c == 0x7F;   /* ASCII */
    }
    if (mode == 3) {
        return c <= 0x3F;   /* EBCDIC */
    }
    return 0;
}

static void rx_push(I5State *s, const I5RxChar *c) {
    unsigned depth = rx_depth(s);

    if (s->rx_locked) {
        return;   /* locked: discarded, status still reported */
    }
    if (s->rx_count >= depth) {
        /* Overrun: the oldest character goes, the new one comes in. */
        memmove(&s->rx[0], &s->rx[1], (depth - 1) * sizeof s->rx[0]);
        s->rx_count = depth - 1;
        s->rst |= S_OE;
    }
    s->rx[s->rx_count++] = *c;
    s->rfir_ack = 0;
}

static void echo_drain(I5State *s) {
    if (!auto_echo(s)) {
        return;
    }
    /* Automatic echo: characters leave the RX FIFO into the TX FIFO. */
    while (s->rx_count > 0 && s->tx_count < FIFO_MAX) {
        I5RxChar c = s->rx[0];
        int errors = !(c.flags & F_ROK);
        int control = (c.flags & F_ACR) != 0;

        memmove(&s->rx[0], &s->rx[1], (s->rx_count - 1) * sizeof s->rx[0]);
        s->rx_count--;
        if ((errors && (i82_rv(s, R_TMD) & 0x80u)) || (control && (i82_rv(s, R_TMD) & 0x40u))) {
            continue;   /* EED / CED: not echoed */
        }
        s->tx[s->tx_count].data = c.data & 0x1FFu;
        s->tx[s->tx_count].known = c.known;
        s->tx[s->tx_count].txf = (uint8_t)((c.flags & F_ACR) && ulan(s) ? 0x80 : 0);
        s->tx_count++;
    }
}

static void on_received(void *context, const SerialRxChar *rc) {
    I5State *s = context;
    unsigned n = char_length(s);
    unsigned bits = s->rxf.data_bits;
    uint16_t mask = (uint16_t)((1u << bits) - 1);
    uint16_t data = rc->data & mask;
    I5RxChar c;
    int pe = (rc->flags & SERIAL_RX_PE) != 0;
    int fe = (rc->flags & SERIAL_RX_FE) != 0;
    int brk = (rc->flags & SERIAL_RX_BI) != 0;
    int addr = ulan(s) && n < 9 && ((data >> n) & 1u);
    unsigned v8 = data & 0xFFu;
    unsigned mode = ((i82_rv(s, R_RMD) >> 6) & 1u) << 1 | ((i82_rv(s, R_RMD) >> 7) & 1u);
    int match = 0;

    memset(&c, 0, sizeof c);
    if (fe && !brk) {
        s->resync = 1;
    }
    c.data = ulan(s) && n < 9 ? (uint16_t)(data & ((1u << n) - 1)) : data;
    c.known = s->rxf.format_x ? 0 : (uint16_t)(rc->known | ~mask);
    if (soft_parity(s)) {
        if (!pe) c.flags |= F_RPE;   /* the received parity bit as is (mark = 1) */
        pe = 0;
    } else if (pe) {
        c.flags |= F_RPE;
    }
    if (rc->flags & SERIAL_RX_FE) c.flags |= F_RFE;
    if (brk) c.flags |= F_BKF | F_RFE;
    if (s->noisy && !rx_1x(s)) c.flags |= F_RXN;
    if (n == 9) c.flags |= (uint8_t)((data >> 8) & 1u);   /* RND */
    if (!pe && !fe) c.flags |= F_ROK;
    c.flags_known = 0xFF;
    /* Programmed characters (ACR0/ACR1) and the standard control sets. */
    if (!ulan(s) || addr) {
        uint8_t cmp = (uint8_t)(c.data & 0xFFu);

        match = cmp == i82_rv(s, R_ACR0) || cmp == i82_rv(s, R_ACR1);
    }
    if (ulan(s)) {
        if (addr) {
            c.flags |= F_ACR;
            s->rst |= S_CRF;
            if (match) {
                s->rst |= S_PCRF;
            }
            /* Semi-automatic: any address opens; automatic: a match opens,
               a mismatch locks. */
            if (mode == 1) {
                s->rx_locked = 0;
            } else if (mode == 2) {
                s->rx_locked = !match;
            }
        }
    } else {
        if (match) {
            c.flags |= F_ACR;
            s->rst |= S_PCRF;
        }
        if (is_control(s, v8)) {
            c.flags |= F_ACR;
            s->rst |= S_CRF;
        }
    }
    if (pe) s->rst |= S_PE;
    if (fe) s->rst |= S_FE;
    if (brk) {
        s->rst |= S_BKD | S_FE;
        s->breaking = 1;
        c.data = 0;
    }
    if (s->rx_enabled) {
        rx_push(s, &c);
        echo_drain(s);
    }
    s->noisy = 0;
}

static LogicValue window_vote(const I5State *s, unsigned w) {
    unsigned ones = 0;
    unsigned zeros = 0;
    unsigned i;

    for (i = 0; i < w; i++) {
        LogicValue v = s->raw[(s->raw_at + 7 - 1 - i) % 7];

        ones += v == LOGIC_1;
        zeros += v == LOGIC_0;
    }
    if (ones * 2 > w) return LOGIC_1;
    if (zeros * 2 > w) return LOGIC_0;
    return LOGIC_X;
}

static int window_same(const I5State *s, unsigned w, LogicValue v) {
    unsigned i;

    for (i = 0; i < w; i++) {
        if (s->raw[(s->raw_at + 7 - 1 - i) % 7] != v) {
            return 0;
        }
    }
    return 1;
}

static void serial_tick_rx(I5State *s) {
    SerialRxEvents events = {s, on_received, NULL};
    LogicValue pin = serial_level(s->sim, s->instance, s->pins->pin[P_RX], LOGIC_X);
    LogicValue level = serial_line_rx_input(&s->line, s->sim, s->instance, pin, &s->rxf);
    SerialRx *rx = &s->line.rx;
    SerialRxPhase before = rx->phase;
    unsigned bit_before = rx->bit;
    unsigned w = (i82_rv(s, R_RMD) & 0x10u) ? 7u : 3u;
    LogicValue in;

    /* As serial_tick_tx: the callers never tick in reset, unknown or power down. */
    if (local_loop(s)) {
        level = s->tsr;
    }
    if (remote_loop(s)) {
        return;   /* nothing reaches the receiver */
    }
    if (rx_1x(s)) {
        in = level;
    } else {
        /* The voting filter: the majority of the window. */
        s->raw[s->raw_at] = level;
        s->raw_at = (s->raw_at + 1) % 7;
        in = window_vote(s, w);
        /* The DPLL: an edge inside a character moves the bit counter one
           tick toward the bit boundary (count 8). */
        if (!(i82_rv(s, R_RMD) & 0x20u) && in != s->filtered && in != LOGIC_X &&
            s->filtered != LOGIC_X &&
            (rx->phase == SERIAL_RX_DATA || rx->phase == SERIAL_RX_PARITY ||
             rx->phase == SERIAL_RX_STOP)) {
            if (rx->count < 8 && rx->count > 0) {
                rx->count++;
            } else if (rx->count > 8) {
                rx->count--;
            }
        }
        s->filtered = in;
    }
    if (s->breaking && in == LOGIC_1) {
        s->breaking = 0;
        s->rst |= S_BKT;   /* break terminated at the first mark */
    }
    s->resync = 0;
    serial_rx_tick(rx, in, &s->rxf, &events);
    if (!rx_1x(s)) {
        /* The start check: all samples 0 when RMD.SSM = 1 (else majority). */
        if (before == SERIAL_RX_START && rx->phase == SERIAL_RX_DATA && (i82_rv(s, R_RMD) & 0x08u) &&
            !window_same(s, w, LOGIC_0)) {
            su_rx_no_resync(rx, in);
        }
        /* The noisy flag: samples that differ in a bit's window. */
        if ((rx->phase != before || rx->bit != bit_before) && before != SERIAL_RX_IDLE &&
            before != SERIAL_RX_START && !window_same(s, w, s->raw[(s->raw_at + 6) % 7])) {
            s->noisy = 1;
        }
    }
    if (s->resync) {
        su_rx_no_resync(rx, in);
    }
}

