#include "charlcd_int.h"
#include "logic_alloc.h"

static const char *const DEVICE_KEYS[] = {
    "columns", "rows", "controller", "rom", "panel", "bus", "contrast", "backlight", "timing",
    "exec", "vcc", "inputMax", NULL
};
static const char *const PART_KEYS[] = {"vcc", "inputMax", NULL};
const char *const clcd_BUS_KEYS[] = {"rs", "rw", "e", "data", NULL};
const char *const clcd_BACKLIGHT_KEYS[] = {"anode", "cathode", "color", NULL};
const char *const clcd_TIMING_KEYS[] = {
    "tc", "tpw", "tas", "tah", "tdsw", "th", "tddr", "tdhr", NULL
};
const char *const clcd_EXEC_KEYS[] = {"instruction", "data", "clearHome", "powerOn", NULL};

static int level(const Sim *sim, const SimInstance *instance, size_t pin) {
    LogicValue v = sim_read_pin(sim, instance, pin);

    return v == LOGIC_0 ? 0 : v == LOGIC_1 ? 1 : -1;   /* Z reads as X */
}

static int bit_of(uint8_t value, uint8_t known, unsigned bit) {
    return (known >> bit) & 1u ? (int)((value >> bit) & 1u) : -1;
}

static int busy_flag(const CharLcdCtl *ctl, SimTime now) {
    if (now < ctl->bf_x_until) {
        return -1;
    }
    return now < ctl->busy_until;
}

static int in_power_on(const CharLcdState *st, SimTime now) {
    return now < st->t->power_on;
}

/* The DB pins the current mode reads and drives: DB4-7 in 4-bit mode,
   all eight in 8-bit mode or when DL is X. */
static uint8_t mode_mask(const CharLcdCtl *ctl) {
    return ctl->dl == 0 ? 0xF0 : 0xFF;
}

static void power_up(CharLcdState *st, CharLcdCtl *ctl) {
    ctl->dl = 1;
    ctl->n = ctl->f = ctl->d = ctl->c = ctl->b = ctl->s = 0;
    ctl->id = 1;
    ctl->ac = 0;
    ctl->ram = 0;
    ctl->shift = 0;
    ctl->phase = 0;
    memset(ctl->ddram, 0x20, sizeof ctl->ddram);
    memset(ctl->ddram_known, 0xFF, sizeof ctl->ddram_known);
    memset(ctl->cgram, 0, sizeof ctl->cgram);
    memset(ctl->cgram_known, 0, sizeof ctl->cgram_known);
    ctl->busy_until = st->t->power_on;
    ctl->bf_x_until = 0;
}

/* LOST: every register field, every RAM cell and BF X. */
static void lose(CharLcdCtl *ctl) {
    ctl->dl = ctl->n = ctl->f = ctl->d = ctl->c = ctl->b = ctl->id = ctl->s = -1;
    ctl->ac = ctl->ram = ctl->shift = ctl->phase = -1;
    memset(ctl->ddram_known, 0, sizeof ctl->ddram_known);
    memset(ctl->cgram_known, 0, sizeof ctl->cgram_known);
    ctl->bf_x_until = LCD_NEVER;
}

static void start_busy(CharLcdCtl *ctl, SimTime now, SimTime length) {
    ctl->busy_until = now + length;
    if (ctl->bf_x_until > now) {
        ctl->bf_x_until = ctl->busy_until;   /* BF is known again when this ends */
    }
}

/* ---- Addresses (section 4.4) ---- */

/* DDRAM physical index of an address for a known N, or -1 (no cell). */
static int ddram_index_n(int n, int address) {
    if (n == 1) {
        if (address < 0x28) {
            return address;
        }
        if (address >= 0x40 && address < 0x68) {
            return address - 0x40 + 40;
        }
        return -1;
    }
    return address < 0x50 ? address : -1;
}

/* -2 when N is X and the two modes disagree. */
static int ddram_index(const CharLcdCtl *ctl, int address) {
    int a, b;

    if (ctl->n >= 0) {
        return ddram_index_n(ctl->n, address);
    }
    a = ddram_index_n(0, address);
    b = ddram_index_n(1, address);
    return a == b ? a : -2;
}

static int ddram_step_n(int n, int address, int up) {
    if (n == 1) {
        if (up) {
            return address == 0x27 ? 0x40 : address == 0x67 ? 0x00 : (address + 1) & 0x7F;
        }
        return address == 0x00 ? 0x67 : address == 0x40 ? 0x27 : (address - 1) & 0x7F;
    }
    if (up) {
        return address == 0x4F ? 0x00 : (address + 1) & 0x7F;
    }
    return address == 0x00 ? 0x4F : (address - 1) & 0x7F;
}

/* Moves AC one step; dir 1 up, 0 down, -1 X. */
static void ac_step(CharLcdCtl *ctl, int dir) {
    if (ctl->ac < 0 || ctl->ram < 0 || dir < 0) {
        ctl->ac = -1;
        return;
    }
    if (ctl->ram == 1) {
        ctl->ac = (ctl->ac + (dir ? 1 : 63)) & 0x3F;
        return;
    }
    if (ctl->n >= 0) {
        ctl->ac = ddram_step_n(ctl->n, ctl->ac, dir);
    } else {
        int a = ddram_step_n(0, ctl->ac, dir);
        int b = ddram_step_n(1, ctl->ac, dir);

        ctl->ac = a == b ? a : -1;
    }
}

/* Shifts the display one step; dir 1 = +1 (content moves left), 0 = -1, -1 = X. */
static void shift_step(CharLcdCtl *ctl, int dir) {
    if (ctl->shift < 0 || dir < 0) {
        ctl->shift = -1;
        return;
    }
    ctl->shift = (ctl->shift + (dir ? 1 : 79)) % 80;
}

/* ---- Instructions and data (sections 4.4, 4.5) ---- */

static void write_data(CharLcdState *st, CharLcdCtl *ctl, SimTime now, uint8_t value,
                       uint8_t known, int violated) {
    start_busy(ctl, now, st->t->data);
    if (violated) {
        known = 0;
    }
    if (ctl->ram < 0 || ctl->ac < 0) {
        /* Every cell of the RAM it could have hit. */
        if (ctl->ram != 1) {
            memset(ctl->ddram_known, 0, sizeof ctl->ddram_known);
            if (ctl->s != 0) {
                ctl->shift = -1;
            }
        }
        if (ctl->ram != 0) {
            memset(ctl->cgram_known, 0, sizeof ctl->cgram_known);
        }
        ctl->ac = -1;
        return;
    }
    if (ctl->ram == 1) {
        ctl->cgram[ctl->ac & 0x3F] = value;
        ctl->cgram_known[ctl->ac & 0x3F] = known;
    } else {
        int index = ddram_index(ctl, ctl->ac);

        if (index == -2) {
            memset(ctl->ddram_known, 0, sizeof ctl->ddram_known);
        } else if (index >= 0) {
            ctl->ddram[index] = value;
            ctl->ddram_known[index] = known;
        }
        if (ctl->s == 1) {
            shift_step(ctl, ctl->id);
        } else if (ctl->s < 0) {
            ctl->shift = -1;
        }
    }
    ac_step(ctl, ctl->id);
}

static void write_instruction(CharLcdState *st, CharLcdCtl *ctl, SimTime now, uint8_t v,
                              uint8_t k) {
    int which = -1;
    int i;

    for (i = 7; i >= 0; i--) {
        int bit = bit_of(v, k, (unsigned)i);

        if (bit < 0) {
            lose(ctl);   /* the instruction is not known */
            return;
        }
        if (bit == 1) {
            which = i;
            break;
        }
    }
    if (which < 0) {
        return;   /* 0000 0000: no instruction, no busy time */
    }
    start_busy(ctl, now, which <= 1 ? st->t->clear_home : st->t->instruction);
    switch (which) {
        case 0:   /* Clear display */
            memset(ctl->ddram, 0x20, sizeof ctl->ddram);
            memset(ctl->ddram_known, 0xFF, sizeof ctl->ddram_known);
            ctl->ac = 0;
            ctl->ram = 0;
            ctl->shift = 0;
            ctl->id = 1;
            break;
        case 1:   /* Return home */
            ctl->ac = 0;
            ctl->ram = 0;
            ctl->shift = 0;
            break;
        case 2:   /* Entry mode set */
            ctl->id = bit_of(v, k, 1);
            ctl->s = bit_of(v, k, 0);
            break;
        case 3:   /* Display on/off */
            ctl->d = bit_of(v, k, 2);
            ctl->c = bit_of(v, k, 1);
            ctl->b = bit_of(v, k, 0);
            break;
        case 4: { /* Cursor/display shift */
            int sc = bit_of(v, k, 3);
            int rl = bit_of(v, k, 2);

            if (sc < 0) {
                ctl->ac = -1;
                ctl->shift = -1;
            } else if (sc == 1) {
                shift_step(ctl, rl < 0 ? -1 : !rl);   /* right: shift - 1 */
            } else {
                ac_step(ctl, rl);                     /* right: AC + 1 */
            }
            break;
        }
        case 5:   /* Function set */
            ctl->dl = bit_of(v, k, 4);
            ctl->n = bit_of(v, k, 3);
            ctl->f = bit_of(v, k, 2);
            ctl->phase = 0;
            break;
        case 6:   /* Set CGRAM address */
            ctl->ac = (k & 0x3F) == 0x3F ? (v & 0x3F) : -1;
            ctl->ram = 1;
            break;
        default:  /* Set DDRAM address */
            ctl->ac = (k & 0x7F) == 0x7F ? (v & 0x7F) : -1;
            ctl->ram = 0;
            break;
    }
}

/* A write takes effect (E fall + the hold window). */
static void apply_write(CharLcdState *st, CharLcdCtl *ctl, SimTime now) {
    uint8_t v = ctl->pend_value, k = ctl->pend_known;
    int rs = ctl->pend_rs;
    int violated = ctl->pend_violated;

    ctl->pending = 0;
    if (busy_flag(ctl, now) == 1) {
        return;   /* not accepted while busy; the nibble phase does not move */
    }
    if (rs < 0) {
        lose(ctl);
        return;
    }
    if (ctl->dl < 0 || (ctl->dl == 0 && ctl->phase < 0)) {
        /* Resync: RS = 0, DB7-4 = 0011 is Function Set with DL = 1. */
        if (rs == 0 && (k & 0xF0) == 0xF0 && (v & 0xF0) == 0x30 && !violated) {
            start_busy(ctl, now, st->t->instruction);
            ctl->dl = 1;
            ctl->phase = 0;
            ctl->n = bit_of(v, k, 3);
            ctl->f = bit_of(v, k, 2);
        } else {
            lose(ctl);
        }
        return;
    }
    if (ctl->dl == 0) {
        if (ctl->phase == 0) {
            ctl->high = (uint8_t)(v >> 4);
            ctl->high_known = (uint8_t)(k >> 4);
            ctl->high_violated = violated;
            ctl->phase = 1;
            return;
        }
        v = (uint8_t)((ctl->high << 4) | (v >> 4));
        k = (uint8_t)((ctl->high_known << 4) | (k >> 4));
        violated |= ctl->high_violated;
        ctl->phase = 0;
    }
    if (rs == 1) {
        write_data(st, ctl, now, v, k, violated);
    } else if (violated) {
        lose(ctl);
    } else {
        write_instruction(st, ctl, now, v, k);
    }
}

/* ---- Reads ---- */

/* What a read of this bus state returns now: BF and AC, or RAM[AC]. */
static void read_byte(const CharLcdCtl *ctl, SimTime now, int rs, uint8_t *value,
                      uint8_t *known) {
    *value = 0;
    *known = 0;
    if (rs == 0) {
        int bf = busy_flag(ctl, now);

        *known = 0xFF;
        if (bf < 0) {
            *known &= 0x7F;
        } else if (bf) {
            *value |= 0x80;
        }
        if (ctl->ac < 0) {
            *known &= 0x80;
        } else {
            *value |= (uint8_t)(ctl->ac & 0x7F);
            if (ctl->ram < 0) {
                *known &= (uint8_t)~0x40;
            }
        }
    } else if (rs == 1) {
        if (busy_flag(ctl, now) != 0 || ctl->ram < 0 || ctl->ac < 0) {
            return;
        }
        if (ctl->ram == 1) {
            *value = ctl->cgram[ctl->ac & 0x3F];
            *known = ctl->cgram_known[ctl->ac & 0x3F];
        } else {
            int index = ddram_index(ctl, ctl->ac);

            if (index >= 0) {
                *value = ctl->ddram[index];
                *known = ctl->ddram_known[index];
            }
        }
    }
}

/*
 * The controller drives its read (z = 0: X while drive_x, else the read
 * value; z = 1: releases DB). The pins of its drive_mask get what every
 * controller drives there: Z when none does; one controller's bit; with
 * two (E1 and E2 both reading), X where they disagree or either is X.
 */
static void drive(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl, int z) {
    unsigned i, k;

    if (z) {
        ctl->out_mask = 0;
    } else {
        ctl->out_mask = ctl->drive_mask;
        ctl->out_known = ctl->drive_x ? 0 : ctl->read_known;
        ctl->out_value = ctl->read_value;
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        LogicValue v = LOGIC_Z;

        if (!((ctl->drive_mask >> i) & 1u)) {
            continue;
        }
        for (k = 0; k < st->pins.enables; k++) {
            const CharLcdCtl *one = &st->ctl[k];
            LogicValue mine;

            if (!((one->out_mask >> i) & 1u)) {
                continue;
            }
            mine = !((one->out_known >> i) & 1u) ? LOGIC_X
                   : (one->out_value >> i) & 1u  ? LOGIC_1
                                                 : LOGIC_0;
            v = v == LOGIC_Z || v == mine ? mine : LOGIC_X;
        }
        sim_schedule_drive(sim, instance, st->pins.db[i], v, DRIVE_STRONG, 0);
    }
}

static void schedule(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl,
                     unsigned kind, SimTime delay) {
    uint32_t which = (uint32_t)(ctl - st->ctl);
    uint32_t id = (uint32_t)(kind | ((ctl->seq & TIMER_SEQ_MASK) << 2) | (which << 31));

    sim_schedule_timer(sim, instance, id, delay);
}

static void start_read(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl,
                       SimTime now) {
    uint8_t v = 0, k = 0;

    ctl->drive_mask = mode_mask(ctl);
    ctl->driving = 1;
    ctl->drive_x = 0;
    if (ctl->acc_rw == 1 && !ctl->acc_violated) {
        if (ctl->dl == 1) {
            read_byte(ctl, now, ctl->acc_rs, &v, &k);
        } else if (ctl->dl == 0 && ctl->phase == 0) {
            read_byte(ctl, now, ctl->acc_rs, &ctl->read_byte, &ctl->read_byte_known);
            v = ctl->read_byte;
            k = ctl->read_byte_known;
        } else if (ctl->dl == 0 && ctl->phase == 1) {
            v = (uint8_t)(ctl->read_byte << 4);
            k = (uint8_t)(ctl->read_byte_known << 4);
        }
    }
    ctl->read_value = v;
    ctl->read_known = k;
    /* X from E rise; the data at + tddr. */
    ctl->drive_x = 1;
    drive(sim, instance, st, ctl, 0);
    ctl->drive_x = 0;
    schedule(sim, instance, st, ctl, TIMER_READ_DATA, st->t->tddr);
}

/* ---- Bus events (section 4.3) ---- */

static void e_rise(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl,
                   SimTime now) {
    if (ctl->pending) {
        apply_write(st, ctl, now);
    }
    ctl->seq++;
    ctl->acc_violated = 0;
    if (ctl->have_rise && now - ctl->prev_rise < st->t->tc) {
        ctl->acc_violated = 1;
    }
    if (now - st->rs_time < st->t->tas || now - st->rw_time < st->t->tas) {
        ctl->acc_violated = 1;
    }
    ctl->prev_rise = now;
    ctl->rise_time = now;
    ctl->have_rise = 1;
    ctl->acc_rs = level(sim, instance, st->pins.rs);
    ctl->acc_rw = level(sim, instance, st->pins.rw);
    if (ctl->driving && ctl->acc_rw == 0) {
        drive(sim, instance, st, ctl, 1);
        ctl->driving = 0;
    }
    if (ctl->acc_rw == 1) {
        start_read(sim, instance, st, ctl, now);
    } else if (ctl->acc_rw < 0) {
        /* An unknown access: it may be a read, so the bus goes X. */
        if (!in_power_on(st, now)) {
            lose(ctl);
        }
        start_read(sim, instance, st, ctl, now);
    }
}

static void e_fall(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl,
                   SimTime now) {
    if (now - ctl->rise_time < st->t->tpw) {
        ctl->acc_violated = 1;
    }
    if (ctl->acc_rw != 0) {
        /* A read (or an unknown access) ends: Z at + tdhr. */
        schedule(sim, instance, st, ctl, TIMER_READ_OFF, st->t->tdhr);
        if (ctl->acc_rw == 1) {
            int data_end = ctl->dl == 1 || (ctl->dl == 0 && ctl->phase == 1);

            if (ctl->dl == 0 && ctl->phase >= 0) {
                ctl->phase = !ctl->phase;
            }
            if (ctl->acc_rs == 1 && data_end) {
                ac_step(ctl, ctl->id);   /* AC moves after a data read (not while LOST) */
            } else if (ctl->acc_rs < 0 || (ctl->acc_rs == 1 && ctl->dl < 0)) {
                ctl->ac = -1;
            }
        }
        return;
    }
    {
        int rw = level(sim, instance, st->pins.rw);
        uint8_t mask = mode_mask(ctl);
        uint8_t v = 0, k = 0;
        unsigned i;

        for (i = 0; i < LCD_DATA_BITS; i++) {
            int bit;

            if (!((mask >> i) & 1u)) {
                continue;
            }
            bit = level(sim, instance, st->pins.db[i]);
            if (bit >= 0) {
                k |= (uint8_t)(1u << i);
                v |= (uint8_t)(bit << i);
            }
            if (now - st->db_time[i] < st->t->tdsw) {
                ctl->acc_violated = 1;
            }
        }
        ctl->pending = 1;
        ctl->pend_fall = now;
        ctl->pend_rs = level(sim, instance, st->pins.rs);
        if (rw != 0) {
            ctl->pend_rs = -1;   /* R/W X at a write: LOST when it takes effect */
        }
        ctl->pend_value = v;
        ctl->pend_known = k;
        ctl->pend_mask = mask;
        ctl->pend_violated = ctl->acc_violated;
        {
            SimTime hold = st->t->th > st->t->tah ? st->t->th : st->t->tah;

            if (hold == 0) {
                apply_write(st, ctl, now);
            } else {
                schedule(sim, instance, st, ctl, TIMER_WRITE, hold);
            }
        }
    }
}

/* Light i: LED i, on while its anode reads 1 and its cathode 0. */
static void update_light(Sim *sim, SimInstance *instance, CharLcdState *st) {
    size_t i;

    for (i = 0; i < st->pins.leds; i++) {
        int on = sim_read_pin(sim, instance, st->pins.anode[i]) == LOGIC_1 &&
                 sim_read_pin(sim, instance, st->pins.cathode[i]) == LOGIC_0;

        lights_set(st->light, i, on, sim_now(sim));
    }
}

static size_t charlcd_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *charlcd_create(const void *config) {
    const CharLcdPins *pins = part_internal(config)->bound;
    CharLcdState *st = logic_calloc(1, sizeof(*st));

    if (st != NULL) {
        unsigned i;

        st->pins = *pins;
        st->t = &pins->device->t;
        for (i = 0; i < LCD_MAX_ENABLES; i++) {
            st->ctl[i].e = -1;
            power_up(st, &st->ctl[i]);
        }
    }
    return st;
}

static void charlcd_destroy(void *state) {
    logic_free(state);
}

static void charlcd_start(Sim *sim, SimInstance *instance, void *raw) {
    CharLcdState *st = raw;
    unsigned k;

    for (k = 0; k < st->pins.enables; k++) {
        st->ctl[k].e = level(sim, instance, st->pins.e[k]);
    }
    update_light(sim, instance, st);
}

/* A controller's enable pin changed. */
static void e_changed(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl,
                      size_t pin, SimTime now) {
    int e = level(sim, instance, pin);
    int was = ctl->e;

    ctl->e = e;
    if (e == was) {
        return;
    }
    if (was == 1) {
        e_fall(sim, instance, st, ctl, now);
    }
    if (e == 1) {
        e_rise(sim, instance, st, ctl, now);
    } else if (e < 0 && !in_power_on(st, now)) {
        /* E reading X may be an access. With R/W reading 1 it can only be
           a read, which changes no more than AC (a data read moves it)
           and, in 4-bit mode, the nibble phase. Anything else may be a
           write: LOST. */
        int rw = was == 1 ? ctl->acc_rw : level(sim, instance, st->pins.rw);

        if (rw == 1) {
            if (level(sim, instance, st->pins.rs) != 0) {
                ctl->ac = -1;
            }
            if (ctl->dl != 1) {
                ctl->phase = -1;
            }
        } else {
            lose(ctl);
            ctl->pending = 0;
        }
        if (ctl->driving) {
            ctl->drive_x = 1;
            drive(sim, instance, st, ctl, 0);
        }
    }
}

/* E (E1, E2) goes to its controller; RS, R/W and DB go to every controller,
   each checking its own hold windows. */
static void charlcd_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    CharLcdState *st = raw;
    SimTime now = sim_now(sim);
    unsigned i, k;

    (void)old_value;
    (void)new_value;
    for (k = 0; k < st->pins.leds; k++) {
        if (pin == st->pins.anode[k] || pin == st->pins.cathode[k]) {
            update_light(sim, instance, st);
            break;
        }
    }
    for (k = 0; k < st->pins.enables; k++) {
        if (pin == st->pins.e[k]) {
            e_changed(sim, instance, st, &st->ctl[k], pin, now);
            return;
        }
    }
    if (pin == st->pins.rs || pin == st->pins.rw) {
        if (pin == st->pins.rs) {
            st->rs_time = now;
        } else {
            st->rw_time = now;
        }
        for (k = 0; k < st->pins.enables; k++) {
            CharLcdCtl *ctl = &st->ctl[k];

            if (ctl->pending && now - ctl->pend_fall < st->t->tah) {
                ctl->pend_violated = 1;
            }
        }
        return;
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (pin == st->pins.db[i]) {
            st->db_time[i] = now;
            for (k = 0; k < st->pins.enables; k++) {
                CharLcdCtl *ctl = &st->ctl[k];

                if (ctl->pending && ((ctl->pend_mask >> i) & 1u) &&
                    now - ctl->pend_fall < st->t->th) {
                    ctl->pend_violated = 1;
                }
            }
            return;
        }
    }
}

static void charlcd_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    CharLcdState *st = raw;
    CharLcdCtl *ctl = &st->ctl[id >> 31];
    unsigned kind = id & 3u;

    if (((id >> 2) & TIMER_SEQ_MASK) != (ctl->seq & TIMER_SEQ_MASK)) {
        return;   /* from an earlier access */
    }
    switch (kind) {
        case TIMER_READ_DATA:
            if (ctl->driving) {
                drive(sim, instance, st, ctl, 0);
            }
            break;
        case TIMER_READ_OFF:
            if (ctl->driving) {
                drive(sim, instance, st, ctl, 1);
                ctl->driving = 0;
            }
            break;
        case TIMER_WRITE:
            if (ctl->pending) {
                apply_write(st, ctl, sim_now(sim));
            }
            break;
    }
}

/* ---- Lights ---- */

/* One light per backlight LED, in "backlight" order; a module without a
   backlight has no lights. */
static size_t charlcd_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                             SimTime since, unsigned char *out, size_t cap) {
    const CharLcdState *st = raw;

    (void)sim;
    (void)instance;
    return st->pins.leds > 0 ? lights_read(st->light, st->pins.leds, since, out, cap) : 0;
}

static size_t charlcd_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                                 uint64_t *out, size_t cap) {
    const CharLcdState *st = raw;

    (void)instance;
    return st->pins.leds > 0 ? lights_time(st->light, st->pins.leds, sim_now(sim), out, cap) : 0;
}

/* ---- The image (section 4.7) ---- */

static int t_or(int a, int b) {
    return a == 1 || b == 1 ? 1 : a < 0 || b < 0 ? -1 : 0;
}

static int t_and(int a, int b) {
    return a == 0 || b == 0 ? 0 : a < 0 || b < 0 ? -1 : 1;
}

/* One dot for a known N: 1, 0 or -1. `line` is the controller's line
   (0 or 1) the row shows, `offset` the row's first cell within that line
   (section 4.4: 20 for rows 2-3 of a 20x4 on one controller, else 0). */
static int image_dot(const CharLcdCtl *ctl, int n, int blink_on, unsigned line, unsigned offset,
                     unsigned column, unsigned y, unsigned x) {
    int address = -1, index = -1, glyph, here, extra;

    if (n == 0 && line == 1) {
        return 0;
    }
    if (ctl->shift >= 0) {
        if (n == 1) {
            unsigned pos = ((unsigned)ctl->shift + offset + column) % 40;

            address = (int)(line * 0x40 + pos);
            index = (int)(line * 40 + pos);
        } else {
            address = index = (int)(((unsigned)ctl->shift + offset + column) % 80);
        }
    }
    if (index < 0 || ctl->ddram_known[index] != 0xFF) {
        glyph = -1;
    } else {
        unsigned code = ctl->ddram[index];
        unsigned mask = 0x10u >> x;

        if (code < 0x10) {
            unsigned cell = (code & 7u) * 8 + y;

            glyph = (ctl->cgram_known[cell] & mask) ? (ctl->cgram[cell] & mask ? 1 : 0) : -1;
        } else {
            glyph = CHARLCD_ROM_A00[code][y] & mask ? 1 : 0;
        }
    }
    /* Is AC's address shown here? */
    if (ctl->ram == 1) {
        here = 0;
    } else if (ctl->ac >= 0 && address >= 0) {
        here = address == ctl->ac ? (ctl->ram == 0 ? 1 : -1) : 0;
    } else {
        here = -1;
    }
    extra = y == 7 ? t_and(ctl->c, here) : 0;
    if (blink_on) {
        extra = t_or(extra, t_and(ctl->b, here));
    }
    return t_and(ctl->d, t_or(glyph, extra));
}

static int charlcd_lcd_state(const Sim *sim, const void *raw, LcdState *out) {
    const CharLcdState *st = raw;
    const PartCharLcd *info = &st->pins.info;
    SimTime now = sim_now(sim);
    int blink_on = (now / LCD_BLINK_PS) % 2 == 0;
    unsigned row, column, y, x, k;

    memset(out, 0, sizeof(*out));
    out->columns = info->columns;
    out->rows = info->rows;
    out->controllers = info->enables;
    for (k = 0; k < info->enables; k++) {
        const CharLcdCtl *one = &st->ctl[k];

        out->busy[k] = busy_flag(one, now);
        memcpy(out->ddram[k], one->ddram, sizeof out->ddram[k]);
        memcpy(out->ddram_known[k], one->ddram_known, sizeof out->ddram_known[k]);
        memcpy(out->cgram[k], one->cgram, sizeof out->cgram[k]);
        memcpy(out->cgram_known[k], one->cgram_known, sizeof out->cgram_known[k]);
    }
    for (row = 0; row < info->rows; row++) {
        /* Section 4.4: two enables: controller row / 2; one: controller 0,
           rows 2-3 continuing rows 0-1's lines. */
        const CharLcdCtl *ctl = &st->ctl[info->enables == 2 ? row / 2 : 0];
        unsigned line = row % 2;
        unsigned offset = info->enables == 2 ? 0 : (row / 2) * info->columns;

        for (column = 0; column < info->columns; column++) {
            for (y = 0; y < LCD_STATE_CELL_HEIGHT; y++) {
                for (x = 0; x < LCD_STATE_CELL_WIDTH; x++) {
                    int dot;

                    if (ctl->n >= 0) {
                        dot = image_dot(ctl, ctl->n, blink_on, line, offset, column, y, x);
                    } else {
                        int a = image_dot(ctl, 0, blink_on, line, offset, column, y, x);
                        int b = image_dot(ctl, 1, blink_on, line, offset, column, y, x);

                        dot = a == b ? a : -1;
                    }
                    out->dots[row * LCD_STATE_CELL_HEIGHT + y][column * LCD_STATE_CELL_WIDTH + x] =
                        dot == 1 ? LCD_DOT_ON : dot == 0 ? LCD_DOT_OFF : LCD_DOT_X;
                }
            }
        }
    }
    return 1;
}

static const SimComponentType CHARLCD_MODEL = {
    .name = "charlcd", .pin_count = charlcd_pin_count, .create = charlcd_create,
    .destroy = charlcd_destroy, .start = charlcd_start, .pin_changed = charlcd_pin_changed,
    .timer = charlcd_timer, .lights = charlcd_lights, .light_time = charlcd_light_time,
    .lcd_state = charlcd_lcd_state
};

const PartKindOps CHARLCD_KIND = {
    PART_CHARLCD, "charlcd", &CHARLCD_MODEL, DEVICE_KEYS, PART_KEYS,
    clcd_charlcd_load_device, clcd_charlcd_bind_pinout, clcd_charlcd_load_part
};
