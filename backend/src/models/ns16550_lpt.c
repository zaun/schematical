/*
 * ns16550 printer port (plan-12-D section 3.2): the PC-compatible parallel
 * port of the TL16C451/452 ("style": "ti451"), TL16C552/552A ("ti552") and
 * COM92C451 ("com92c451"). It is a unit on the host bus beside the serial
 * channels (its own chip select, A1..A0), with three registers: data,
 * status and control (plan-12-data/specs/ti-a.md § TL16C451, ti-b.md
 * § TL16C552, classic-b.md § COM92C451; decisions.md for the polarities).
 *
 * Control pins STB, AFD, INIT and SLIN are open drain with an internal
 * pull-up: asserted they drive 0 strong, released they drive 1 weak.
 */

#include <stdlib.h>
#include <string.h>

#include "ns16550_int.h"

struct NsLpt {
    uint8_t data;          /* the output latch */
    uint8_t control;       /* the control register (bits 0-5) */
    int print;             /* TL16C552 status bit 2 (active low PRINT); 1 = idle */
    int irq;               /* COM92C451 IRQ0 latch */
    LogicValue ack_last;
};

static LogicValue pin_in(const NsState *s, size_t pin) {
    return serial_level(s->sim, s->instance, pin, LOGIC_X);
}

static void drive_weak(NsState *s, size_t pin, LogicValue value) {
    if (pin == SERIAL_NO_PIN || s->out[pin] == value) {
        return;
    }
    s->out[pin] = value;
    sim_schedule_drive(s->sim, s->instance, pin, value == LOGIC_Z ? LOGIC_1 : value,
                       value == LOGIC_Z ? DRIVE_WEAK : DRIVE_STRONG, 0);
}

struct NsLpt *ns_lpt_create(const NsState *s) {
    struct NsLpt *lpt = calloc(1, sizeof *lpt);

    (void)s;
    if (lpt != NULL) {
        lpt->print = 1;
        lpt->ack_last = LOGIC_X;
    }
    return lpt;
}

void ns_lpt_destroy(struct NsLpt *lpt) {
    free(lpt);
}

void ns_lpt_reset(NsState *s) {
    const NsLptDevice *d = s->dev->lpt;

    if (d == NULL || s->lpt == NULL) {
        return;
    }
    /* Control cleared by reset (all three); the COM92C451 also clears data. */
    s->lpt->control = 0;
    s->lpt->print = 1;
    s->lpt->irq = 0;
    if (d->style == LPT_COM92C451) {
        s->lpt->data = 0;
    }
}

int ns_lpt_has_pin(const NsState *s, size_t pin) {
    const NsLptPins *p = s->pins->lpt;
    unsigned i;

    if (p == NULL || pin == SERIAL_NO_PIN) {
        return 0;
    }
    for (i = 0; i < 8; i++) {
        if (p->data[i] == pin) {
            return 1;
        }
    }
    return pin == p->ack || pin == p->busy || pin == p->paper_end || pin == p->slct ||
           pin == p->error || pin == p->input_enable || pin == p->pemd || pin == p->enirq;
}

/* The PD pins drive the latch (1) or are inputs (0). */
static LogicValue data_driven(const NsState *s) {
    const NsLptDevice *d = s->dev->lpt;
    const NsLptPins *p = s->pins->lpt;

    switch (d->style) {
        case LPT_TI451:
            /* LPTOE low drives the bus; high leaves PD as inputs. */
            return not3(serial_level(s->sim, s->instance, p->input_enable, LOGIC_X));
        case LPT_TI552: {
            LogicValue pemd = serial_level(s->sim, s->instance, p->pemd, LOGIC_X);
            LogicValue dir = bool3((s->lpt->control >> 5) & 1u);

            /* PEMD low: always output; high: output while DIR = 0. */
            return or3(not3(pemd), not3(dir));
        }
        case LPT_COM92C451:
        default:
            return LOGIC_1;
    }
}

static uint8_t status_byte(const NsState *s, uint8_t *known) {
    const NsLptDevice *d = s->dev->lpt;
    const NsLptPins *p = s->pins->lpt;
    LogicValue busy = pin_in(s, p->busy);
    LogicValue bits[5];
    uint8_t v;
    unsigned i;

    bits[0] = pin_in(s, p->error);
    bits[1] = pin_in(s, p->slct);
    bits[2] = pin_in(s, p->paper_end);
    bits[3] = pin_in(s, p->ack);
    bits[4] = not3(busy);
    switch (d->style) {
        case LPT_TI451: v = 0x07; break;
        case LPT_TI552: v = (uint8_t)(0x03 | (s->lpt->print ? 0x04 : 0)); break;
        default: v = 0x00; break;
    }
    *known = 0x07;
    for (i = 0; i < 5; i++) {
        if (bits[i] == LOGIC_1) {
            v |= (uint8_t)(0x08u << i);
        }
        if (bits[i] != LOGIC_X) {
            *known |= (uint8_t)(0x08u << i);
        }
    }
    return v;
}

int ns_lpt_selected(const NsState *s) {
    return s->dev->lpt != NULL;
}

void ns_lpt_peek(NsState *s, unsigned address, uint8_t *v, uint8_t *k) {
    const NsLptDevice *d = s->dev->lpt;
    const NsLptPins *p = s->pins->lpt;
    unsigned i;

    *v = 0;
    *k = 0;
    switch (address & 3u) {
        case 0:
            /* TL16C552 with the port as output: the latch; otherwise the pins. */
            if (d->style == LPT_TI552 && data_driven(s) == LOGIC_1) {
                *v = s->lpt->data;
                *k = 0xFF;
                break;
            }
            for (i = 0; i < 8; i++) {
                LogicValue b = pin_in(s, p->data[i]);

                if (b == LOGIC_1) *v |= (uint8_t)(1u << i);
                if (b != LOGIC_X) *k |= (uint8_t)(1u << i);
            }
            break;
        case 1:
            *v = status_byte(s, k);
            break;
        case 2:
            if (d->style == LPT_TI552) {
                *v = s->lpt->control & 0x3F;
                *k = 0xFF;
            } else {
                /* The control lines' pin levels, through the register's
                   polarity (STB, AFD, SLIN inverted; INIT true); b7-5 = 1. */
                LogicValue stb = not3(pin_in(s, p->strobe));
                LogicValue afd = not3(pin_in(s, p->auto_feed));
                LogicValue init = pin_in(s, p->init);
                LogicValue slin = not3(pin_in(s, p->select_in));
                const LogicValue b[4] = {stb, afd, init, slin};

                *v = (uint8_t)(0xE0 | (s->lpt->control & 0x10));
                *k = 0xF0;
                for (i = 0; i < 4; i++) {
                    if (b[i] == LOGIC_1) *v |= (uint8_t)(1u << i);
                    if (b[i] != LOGIC_X) *k |= (uint8_t)(1u << i);
                }
            }
            break;
        default:
            break;   /* invalid: X (P2) */
    }
}

/* A status read: the TL16C552 sets PRINT again; the COM92C451 clears IRQ0. */
static void status_read(NsState *s) {
    s->lpt->print = 1;
    s->lpt->irq = 0;
}

void ns_lpt_write(NsState *s, unsigned address, uint8_t v, uint8_t k) {
    switch (address & 3u) {
        case 0:
            s->lpt->data = v;
            (void)k;
            break;
        case 2:
            s->lpt->control = (uint8_t)(v & (s->dev->lpt->style == LPT_TI552 ? 0x3F : 0x1F));
            break;
        default:
            break;   /* invalid addresses: ignored (P2) */
    }
    ns_lpt_pins(s);
}

/* Called for a read that ended. */
void ns_lpt_read_done(NsState *s, unsigned address) {
    if ((address & 3u) == 1) {
        status_read(s);
        ns_lpt_pins(s);
    }
}

void ns_lpt_pins(NsState *s) {
    const NsLptDevice *d = s->dev->lpt;
    const NsLptPins *p = s->pins->lpt;
    struct NsLpt *lpt = s->lpt;
    LogicValue ack;
    LogicValue drive_data;
    LogicValue irq;
    unsigned i;

    if (d == NULL || lpt == NULL || s->sim == NULL) {
        return;
    }
    ack = pin_in(s, p->ack);
    /* Edges of ACK: PRINT clears on the rising edge (TL16C552, with INT2 EN);
       IRQ0 latches on the falling edge (COM92C451, when enabled). */
    if (lpt->ack_last == LOGIC_0 && ack == LOGIC_1 && (lpt->control & 0x10)) {
        lpt->print = 0;
    }
    if (lpt->ack_last == LOGIC_1 && ack == LOGIC_0 && (lpt->control & 0x10)) {
        lpt->irq = 1;
    }
    lpt->ack_last = ack;

    drive_data = s->unknown ? LOGIC_X : data_driven(s);
    for (i = 0; i < 8; i++) {
        LogicValue bit = (lpt->data >> i) & 1u ? LOGIC_1 : LOGIC_0;

        ns_drive(s, p->data[i], drive_data == LOGIC_1 ? bit : drive_data == LOGIC_0 ? LOGIC_Z
                                                                                   : LOGIC_X, 0);
    }
    /* Open-drain control lines: asserted -> 0. STB, AFD, SLIN assert for a
       1 bit; INIT asserts (low) for a 0 bit. */
    drive_weak(s, p->strobe, lpt->control & 0x01 ? LOGIC_0 : LOGIC_Z);
    drive_weak(s, p->auto_feed, lpt->control & 0x02 ? LOGIC_0 : LOGIC_Z);
    drive_weak(s, p->init, lpt->control & 0x04 ? LOGIC_Z : LOGIC_0);
    drive_weak(s, p->select_in, lpt->control & 0x08 ? LOGIC_0 : LOGIC_Z);

    /* The port's interrupt pin, 3-state unless enabled (control bit 4). */
    if (!(lpt->control & 0x10)) {
        irq = LOGIC_Z;
    } else if (d->style == LPT_COM92C451) {
        irq = bool3(lpt->irq);
    } else if (d->style == LPT_TI552 &&
               serial_level(s->sim, s->instance, p->enirq, LOGIC_X) != LOGIC_0) {
        LogicValue en = serial_level(s->sim, s->instance, p->enirq, LOGIC_X);

        irq = en == LOGIC_1 ? bool3(!lpt->print) : LOGIC_X;
    } else {
        irq = ack;   /* INT2 follows ACK (decisions.md TL16C451/452; TL16C552 ENIRQ low) */
    }
    ns_drive(s, p->irq, irq, 0);
}
