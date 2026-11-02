/*
 * charlcd: HD44780-compatible character LCD modules (plan-11-B). Each module
 * has its controller, character ROM, glass and backlight.
 *
 * Loader: the device keys of docs/parts-format.md § charlcd.
 *
 * Model (plan-11-B sections 4.3-4.7): the 8- and 4-bit bus on E edges, the
 * instruction set, the busy flag and execution times, power-on reset, the
 * bus timing checks and the X rules (an access whose effect is unknown makes
 * everything it could change X; "LOST" is every field X). The backend
 * renders the dot image (circuit_lcd_state); the backlight is one light.
 * Not modeled: V0 and contrast, the 5x10 font (F is stored), ROM A02, tR/tF,
 * serial interfaces.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "charlcd_font.h"
#include "kind.h"
#include "lcd_state.h"
#include "lights.h"

#define LCD_DATA_BITS 8
#define LCD_BUS_PINS (3 + LCD_DATA_BITS)

static const char *const DEVICE_KEYS[] = {
    "columns", "rows", "controller", "rom", "panel", "bus", "contrast", "backlight", "timing",
    "exec", "vcc", "inputMax", NULL
};
static const char *const PART_KEYS[] = {"vcc", "inputMax", NULL};
static const char *const BUS_KEYS[] = {"rs", "rw", "e", "data", NULL};
static const char *const BACKLIGHT_KEYS[] = {"anode", "cathode", "color", NULL};
static const char *const TIMING_KEYS[] = {
    "tc", "tpw", "tas", "tah", "tdsw", "th", "tddr", "tdhr", NULL
};
static const char *const EXEC_KEYS[] = {"instruction", "data", "clearHome", "powerOn", NULL};

/* Bus timing and execution times, in picoseconds. */
typedef struct {
    SimTime tc, tpw, tas, tah, tdsw, th, tddr, tdhr;
    SimTime instruction, data, clear_home, power_on;
} CharLcdTiming;

typedef struct {
    unsigned columns, rows;
    const char *controller, *rom, *panel, *backlight_color;
    const char *rs, *rw, *e;
    const char *const *data;
    const char *contrast;
    const char *anode, *cathode;      /* NULL without a backlight */
    CharLcdTiming t;
    const PartVoltage *vcc;
} CharLcdDevice;

/* Pin indices for one pinout. */
typedef struct {
    size_t rs, rw, e, db[LCD_DATA_BITS], contrast, anode, cathode;
    int has_backlight;
    size_t bus[LCD_BUS_PINS];
    PartCharLcd info;
    const CharLcdDevice *device;
} CharLcdPins;

/* ---- Loader --------------------------------------------------------------- */

static int load_times(KindContext *ctx, const cJSON *json, const char *key,
                      const char *const *names, SimTime *const *fields) {
    const cJSON *object = kind_member(ctx, json, key, 1);
    char what[48];
    size_t i;

    if (object == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(object)) {
        return kind_error(ctx, "\"%s\" must be an object of times in ns", key);
    }
    if (kind_check_keys(ctx, object, names, NULL) != 0) {
        return -1;
    }
    for (i = 0; names[i] != NULL; i++) {
        snprintf(what, sizeof what, "%s.%s", key, names[i]);
        if (kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(object, names[i]), what, fields[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

static int more_than_zero(KindContext *ctx, SimTime value, const char *what) {
    return value > 0 ? 0 : kind_error(ctx, "\"%s\" must be more than 0", what);
}

static void *charlcd_load_device(KindContext *ctx, const cJSON *json) {
    CharLcdDevice *device = kind_alloc(ctx, sizeof(*device));
    CharLcdTiming *t;
    const cJSON *bus;
    const cJSON *backlight;
    uint64_t columns, rows;
    size_t data_count;

    if (device == NULL) {
        return NULL;
    }
    t = &device->t;
    if (kind_uint(ctx, json, "columns", 1000000, &columns) != 0) {
        return NULL;
    }
    if (columns < 1 || columns > 40) {
        kind_error(ctx, "\"columns\" must be 1 to 40");
        return NULL;
    }
    if (kind_uint(ctx, json, "rows", 1000000, &rows) != 0) {
        return NULL;
    }
    if (rows < 1 || rows > 2) {
        kind_error(ctx, "\"rows\" must be 1 or 2");
        return NULL;
    }
    device->columns = (unsigned)columns;
    device->rows = (unsigned)rows;
    if (kind_string(ctx, json, "controller", &device->controller) != 0 ||
        kind_string(ctx, json, "rom", &device->rom) != 0) {
        return NULL;
    }
    if (strcmp(device->rom, "A00") != 0) {
        kind_error(ctx, "unknown rom \"%s\"", device->rom);
        return NULL;
    }
    if (kind_string(ctx, json, "panel", &device->panel) != 0) {
        return NULL;
    }
    if (strcmp(device->panel, "yellow-green") != 0 && strcmp(device->panel, "blue-negative") != 0) {
        kind_error(ctx, "\"panel\" must be yellow-green or blue-negative");
        return NULL;
    }

    bus = kind_member(ctx, json, "bus", 1);
    if (bus == NULL) {
        return NULL;
    }
    if (!cJSON_IsObject(bus)) {
        kind_error(ctx, "\"bus\" must be an object of pin names");
        return NULL;
    }
    if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 ||
        kind_string(ctx, bus, "rs", &device->rs) != 0 ||
        kind_string(ctx, bus, "rw", &device->rw) != 0 ||
        kind_string(ctx, bus, "e", &device->e) != 0) {
        return NULL;
    }
    {
        const cJSON *data = cJSON_GetObjectItemCaseSensitive(bus, "data");

        if (data == NULL || !cJSON_IsArray(data) || cJSON_GetArraySize(data) != LCD_DATA_BITS ||
            kind_names(ctx, bus, "data", &device->data, &data_count) != 0 ||
            data_count != LCD_DATA_BITS) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "\"data\" must be 8 pin names, DB0 first");
            }
            return NULL;
        }
    }
    if (kind_string(ctx, json, "contrast", &device->contrast) != 0) {
        return NULL;
    }

    backlight = cJSON_GetObjectItemCaseSensitive(json, "backlight");
    if (backlight != NULL) {
        if (!cJSON_IsObject(backlight)) {
            kind_error(ctx, "\"backlight\" must be an object");
            return NULL;
        }
        if (kind_check_keys(ctx, backlight, BACKLIGHT_KEYS, NULL) != 0 ||
            kind_string(ctx, backlight, "anode", &device->anode) != 0 ||
            kind_string(ctx, backlight, "cathode", &device->cathode) != 0 ||
            kind_string(ctx, backlight, "color", &device->backlight_color) != 0) {
            return NULL;
        }
    }

    {
        SimTime *const timing[] = {&t->tc, &t->tpw, &t->tas, &t->tah, &t->tdsw, &t->th,
                                   &t->tddr, &t->tdhr};
        SimTime *const exec[] = {&t->instruction, &t->data, &t->clear_home, &t->power_on};
        size_t i;

        if (load_times(ctx, json, "timing", TIMING_KEYS, timing) != 0 ||
            more_than_zero(ctx, t->tc, "timing.tc") != 0 ||
            more_than_zero(ctx, t->tpw, "timing.tpw") != 0 ||
            load_times(ctx, json, "exec", EXEC_KEYS, exec) != 0) {
            return NULL;
        }
        for (i = 0; EXEC_KEYS[i] != NULL; i++) {
            char what[32];

            snprintf(what, sizeof what, "exec.%s", EXEC_KEYS[i]);
            if (more_than_zero(ctx, *exec[i], what) != 0) {
                return NULL;
            }
        }
    }
    if (kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

static void *charlcd_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const CharLcdDevice *device = raw;
    CharLcdPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i;

    if (bound == NULL ||
        kind_pin(ctx, pins, pin_count, device->rs, &bound->rs) != 0 ||
        kind_pin(ctx, pins, pin_count, device->rw, &bound->rw) != 0 ||
        kind_pin(ctx, pins, pin_count, device->e, &bound->e) != 0 ||
        kind_pin(ctx, pins, pin_count, device->contrast, &bound->contrast) != 0) {
        return NULL;
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (kind_pin(ctx, pins, pin_count, device->data[i], &bound->db[i]) != 0) {
            return NULL;
        }
    }
    if (device->anode != NULL) {
        if (kind_pin(ctx, pins, pin_count, device->anode, &bound->anode) != 0 ||
            kind_pin(ctx, pins, pin_count, device->cathode, &bound->cathode) != 0) {
            return NULL;
        }
        bound->has_backlight = 1;
    }
    bound->bus[0] = bound->rs;
    bound->bus[1] = bound->rw;
    bound->bus[2] = bound->e;
    for (i = 0; i < LCD_DATA_BITS; i++) {
        bound->bus[3 + i] = bound->db[i];
    }
    bound->info.columns = device->columns;
    bound->info.rows = device->rows;
    bound->info.controller = device->controller;
    bound->info.rom = device->rom;
    bound->info.panel = device->panel;
    bound->info.backlight_color = device->backlight_color;
    bound->info.bus_pins = bound->bus;
    bound->info.bus_pin_count = LCD_BUS_PINS;
    bound->device = device;
    return bound;
}

static int charlcd_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CharLcdDevice *device = raw;

    out->charlcd = &((const CharLcdPins *)bound)->info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

/* ---- Model ---------------------------------------------------------------- */

/*
 * One state machine driven by E edges, input changes and timers. Registers
 * are tracked one field at a time, each known or X (-1). Both bus widths
 * share one byte path; 4-bit mode only assembles nibbles in front of it.
 */

#define LCD_NEVER UINT64_MAX
#define LCD_BLINK_PS 379259259ull   /* 409.6 ms x 250 kHz / 270 kHz */

enum { TIMER_READ_DATA, TIMER_READ_OFF, TIMER_WRITE };

typedef struct {
    CharLcdPins pins;
    const CharLcdTiming *t;

    /* Registers: 0/1 (or a value), -1 = X. */
    int dl, n, f, d, c, b, id, s;
    int ac;                  /* 0-127 */
    int ram;                 /* 0 DDRAM, 1 CGRAM */
    int shift;               /* 0-79 */
    int phase;               /* 4-bit: 0 = the high nibble comes next, 1 = the low */
    uint8_t high, high_known;   /* a 4-bit write's first nibble */
    int high_violated;
    SimTime busy_until;
    SimTime bf_x_until;      /* BF reads X before this (after LOST) */
    uint8_t ddram[80], ddram_known[80];
    uint8_t cgram[64], cgram_known[64];

    /* Inputs. */
    int e;                   /* E as last seen: 0, 1, -1 */
    SimTime rs_time, rw_time, db_time[LCD_DATA_BITS];
    SimTime rise_time, prev_rise;
    int have_rise;

    /* The access in progress (E high). */
    int acc_rs, acc_rw, acc_violated;
    uint32_t seq;

    /* Read drive. */
    int driving, drive_x;
    uint8_t drive_mask, read_value, read_known;
    uint8_t read_byte, read_byte_known;   /* 4-bit: the byte whose nibbles a read returns */

    /* A write waiting out its hold window. */
    int pending, pend_rs, pend_violated;
    SimTime pend_fall;
    uint8_t pend_value, pend_known, pend_mask;

    Light light;
} CharLcdState;

static int level(const Sim *sim, const SimInstance *instance, size_t pin) {
    LogicValue v = sim_read_pin(sim, instance, pin);

    return v == LOGIC_0 ? 0 : v == LOGIC_1 ? 1 : -1;   /* Z reads as X */
}

static int bit_of(uint8_t value, uint8_t known, unsigned bit) {
    return (known >> bit) & 1u ? (int)((value >> bit) & 1u) : -1;
}

static int busy_flag(const CharLcdState *st, SimTime now) {
    if (now < st->bf_x_until) {
        return -1;
    }
    return now < st->busy_until;
}

static int in_power_on(const CharLcdState *st, SimTime now) {
    return now < st->t->power_on;
}

/* The DB pins the current mode reads and drives: DB4-7 in 4-bit mode,
   all eight in 8-bit mode or when DL is X. */
static uint8_t mode_mask(const CharLcdState *st) {
    return st->dl == 0 ? 0xF0 : 0xFF;
}

static void power_up(CharLcdState *st) {
    st->dl = 1;
    st->n = st->f = st->d = st->c = st->b = st->s = 0;
    st->id = 1;
    st->ac = 0;
    st->ram = 0;
    st->shift = 0;
    st->phase = 0;
    memset(st->ddram, 0x20, sizeof st->ddram);
    memset(st->ddram_known, 0xFF, sizeof st->ddram_known);
    memset(st->cgram, 0, sizeof st->cgram);
    memset(st->cgram_known, 0, sizeof st->cgram_known);
    st->busy_until = st->t->power_on;
    st->bf_x_until = 0;
}

/* LOST: every register field, every RAM cell and BF X. */
static void lose(CharLcdState *st) {
    st->dl = st->n = st->f = st->d = st->c = st->b = st->id = st->s = -1;
    st->ac = st->ram = st->shift = st->phase = -1;
    memset(st->ddram_known, 0, sizeof st->ddram_known);
    memset(st->cgram_known, 0, sizeof st->cgram_known);
    st->bf_x_until = LCD_NEVER;
}

static void start_busy(CharLcdState *st, SimTime now, SimTime length) {
    st->busy_until = now + length;
    if (st->bf_x_until > now) {
        st->bf_x_until = st->busy_until;   /* BF is known again when this ends */
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
static int ddram_index(const CharLcdState *st, int address) {
    int a, b;

    if (st->n >= 0) {
        return ddram_index_n(st->n, address);
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
static void ac_step(CharLcdState *st, int dir) {
    if (st->ac < 0 || st->ram < 0 || dir < 0) {
        st->ac = -1;
        return;
    }
    if (st->ram == 1) {
        st->ac = (st->ac + (dir ? 1 : 63)) & 0x3F;
        return;
    }
    if (st->n >= 0) {
        st->ac = ddram_step_n(st->n, st->ac, dir);
    } else {
        int a = ddram_step_n(0, st->ac, dir);
        int b = ddram_step_n(1, st->ac, dir);

        st->ac = a == b ? a : -1;
    }
}

/* Shifts the display one step; dir 1 = +1 (content moves left), 0 = -1, -1 = X. */
static void shift_step(CharLcdState *st, int dir) {
    if (st->shift < 0 || dir < 0) {
        st->shift = -1;
        return;
    }
    st->shift = (st->shift + (dir ? 1 : 79)) % 80;
}

/* ---- Instructions and data (sections 4.4, 4.5) ---- */

static void write_data(CharLcdState *st, SimTime now, uint8_t value, uint8_t known, int violated) {
    start_busy(st, now, st->t->data);
    if (violated) {
        known = 0;
    }
    if (st->ram < 0 || st->ac < 0) {
        /* Every cell of the RAM it could have hit. */
        if (st->ram != 1) {
            memset(st->ddram_known, 0, sizeof st->ddram_known);
            if (st->s != 0) {
                st->shift = -1;
            }
        }
        if (st->ram != 0) {
            memset(st->cgram_known, 0, sizeof st->cgram_known);
        }
        st->ac = -1;
        return;
    }
    if (st->ram == 1) {
        st->cgram[st->ac & 0x3F] = value;
        st->cgram_known[st->ac & 0x3F] = known;
    } else {
        int index = ddram_index(st, st->ac);

        if (index == -2) {
            memset(st->ddram_known, 0, sizeof st->ddram_known);
        } else if (index >= 0) {
            st->ddram[index] = value;
            st->ddram_known[index] = known;
        }
        if (st->s == 1) {
            shift_step(st, st->id);
        } else if (st->s < 0) {
            st->shift = -1;
        }
    }
    ac_step(st, st->id);
}

static void write_instruction(CharLcdState *st, SimTime now, uint8_t v, uint8_t k) {
    int which = -1;
    int i;

    for (i = 7; i >= 0; i--) {
        int bit = bit_of(v, k, (unsigned)i);

        if (bit < 0) {
            lose(st);   /* the instruction is not known */
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
    start_busy(st, now, which <= 1 ? st->t->clear_home : st->t->instruction);
    switch (which) {
        case 0:   /* Clear display */
            memset(st->ddram, 0x20, sizeof st->ddram);
            memset(st->ddram_known, 0xFF, sizeof st->ddram_known);
            st->ac = 0;
            st->ram = 0;
            st->shift = 0;
            st->id = 1;
            break;
        case 1:   /* Return home */
            st->ac = 0;
            st->ram = 0;
            st->shift = 0;
            break;
        case 2:   /* Entry mode set */
            st->id = bit_of(v, k, 1);
            st->s = bit_of(v, k, 0);
            break;
        case 3:   /* Display on/off */
            st->d = bit_of(v, k, 2);
            st->c = bit_of(v, k, 1);
            st->b = bit_of(v, k, 0);
            break;
        case 4: { /* Cursor/display shift */
            int sc = bit_of(v, k, 3);
            int rl = bit_of(v, k, 2);

            if (sc < 0) {
                st->ac = -1;
                st->shift = -1;
            } else if (sc == 1) {
                shift_step(st, rl < 0 ? -1 : !rl);   /* right: shift - 1 */
            } else {
                ac_step(st, rl);                     /* right: AC + 1 */
            }
            break;
        }
        case 5:   /* Function set */
            st->dl = bit_of(v, k, 4);
            st->n = bit_of(v, k, 3);
            st->f = bit_of(v, k, 2);
            st->phase = 0;
            break;
        case 6:   /* Set CGRAM address */
            st->ac = (k & 0x3F) == 0x3F ? (v & 0x3F) : -1;
            st->ram = 1;
            break;
        default:  /* Set DDRAM address */
            st->ac = (k & 0x7F) == 0x7F ? (v & 0x7F) : -1;
            st->ram = 0;
            break;
    }
}

/* A write takes effect (E fall + the hold window). */
static void apply_write(CharLcdState *st, SimTime now) {
    uint8_t v = st->pend_value, k = st->pend_known;
    int rs = st->pend_rs;
    int violated = st->pend_violated;

    st->pending = 0;
    if (busy_flag(st, now) == 1) {
        return;   /* not accepted while busy; the nibble phase does not move */
    }
    if (rs < 0) {
        lose(st);
        return;
    }
    if (st->dl < 0 || (st->dl == 0 && st->phase < 0)) {
        /* Resync: RS = 0, DB7-4 = 0011 is Function Set with DL = 1. */
        if (rs == 0 && (k & 0xF0) == 0xF0 && (v & 0xF0) == 0x30 && !violated) {
            start_busy(st, now, st->t->instruction);
            st->dl = 1;
            st->phase = 0;
            st->n = bit_of(v, k, 3);
            st->f = bit_of(v, k, 2);
        } else {
            lose(st);
        }
        return;
    }
    if (st->dl == 0) {
        if (st->phase == 0) {
            st->high = (uint8_t)(v >> 4);
            st->high_known = (uint8_t)(k >> 4);
            st->high_violated = violated;
            st->phase = 1;
            return;
        }
        v = (uint8_t)((st->high << 4) | (v >> 4));
        k = (uint8_t)((st->high_known << 4) | (k >> 4));
        violated |= st->high_violated;
        st->phase = 0;
    }
    if (rs == 1) {
        write_data(st, now, v, k, violated);
    } else if (violated) {
        lose(st);
    } else {
        write_instruction(st, now, v, k);
    }
}

/* ---- Reads ---- */

/* What a read of this bus state returns now: BF and AC, or RAM[AC]. */
static void read_byte(const CharLcdState *st, SimTime now, int rs, uint8_t *value,
                      uint8_t *known) {
    *value = 0;
    *known = 0;
    if (rs == 0) {
        int bf = busy_flag(st, now);

        *known = 0xFF;
        if (bf < 0) {
            *known &= 0x7F;
        } else if (bf) {
            *value |= 0x80;
        }
        if (st->ac < 0) {
            *known &= 0x80;
        } else {
            *value |= (uint8_t)(st->ac & 0x7F);
            if (st->ram < 0) {
                *known &= (uint8_t)~0x40;
            }
        }
    } else if (rs == 1) {
        if (busy_flag(st, now) != 0 || st->ram < 0 || st->ac < 0) {
            return;
        }
        if (st->ram == 1) {
            *value = st->cgram[st->ac & 0x3F];
            *known = st->cgram_known[st->ac & 0x3F];
        } else {
            int index = ddram_index(st, st->ac);

            if (index >= 0) {
                *value = st->ddram[index];
                *known = st->ddram_known[index];
            }
        }
    }
}

static void drive(Sim *sim, SimInstance *instance, CharLcdState *st, int z) {
    unsigned i;

    for (i = 0; i < LCD_DATA_BITS; i++) {
        LogicValue v;

        if (!((st->drive_mask >> i) & 1u)) {
            continue;
        }
        if (z) {
            v = LOGIC_Z;
        } else if (st->drive_x || !((st->read_known >> i) & 1u)) {
            v = LOGIC_X;
        } else {
            v = (st->read_value >> i) & 1u ? LOGIC_1 : LOGIC_0;
        }
        sim_schedule_drive(sim, instance, st->pins.db[i], v, DRIVE_STRONG, 0);
    }
}

static void schedule(Sim *sim, SimInstance *instance, CharLcdState *st, unsigned kind,
                     SimTime delay) {
    sim_schedule_timer(sim, instance, (uint32_t)(kind | (st->seq << 2)), delay);
}

static void start_read(Sim *sim, SimInstance *instance, CharLcdState *st, SimTime now) {
    uint8_t v = 0, k = 0;

    st->drive_mask = mode_mask(st);
    st->driving = 1;
    st->drive_x = 0;
    if (st->acc_rw == 1 && !st->acc_violated) {
        if (st->dl == 1) {
            read_byte(st, now, st->acc_rs, &v, &k);
        } else if (st->dl == 0 && st->phase == 0) {
            read_byte(st, now, st->acc_rs, &st->read_byte, &st->read_byte_known);
            v = st->read_byte;
            k = st->read_byte_known;
        } else if (st->dl == 0 && st->phase == 1) {
            v = (uint8_t)(st->read_byte << 4);
            k = (uint8_t)(st->read_byte_known << 4);
        }
    }
    st->read_value = v;
    st->read_known = k;
    /* X from E rise; the data at + tddr. */
    st->drive_x = 1;
    drive(sim, instance, st, 0);
    st->drive_x = 0;
    schedule(sim, instance, st, TIMER_READ_DATA, st->t->tddr);
}

/* ---- Bus events (section 4.3) ---- */

static void e_rise(Sim *sim, SimInstance *instance, CharLcdState *st, SimTime now) {
    if (st->pending) {
        apply_write(st, now);
    }
    st->seq++;
    st->acc_violated = 0;
    if (st->have_rise && now - st->prev_rise < st->t->tc) {
        st->acc_violated = 1;
    }
    if (now - st->rs_time < st->t->tas || now - st->rw_time < st->t->tas) {
        st->acc_violated = 1;
    }
    st->prev_rise = now;
    st->rise_time = now;
    st->have_rise = 1;
    st->acc_rs = level(sim, instance, st->pins.rs);
    st->acc_rw = level(sim, instance, st->pins.rw);
    if (st->driving && st->acc_rw == 0) {
        drive(sim, instance, st, 1);
        st->driving = 0;
    }
    if (st->acc_rw == 1) {
        start_read(sim, instance, st, now);
    } else if (st->acc_rw < 0) {
        /* An unknown access: it may be a read, so the bus goes X. */
        if (!in_power_on(st, now)) {
            lose(st);
        }
        start_read(sim, instance, st, now);
    }
}

static void e_fall(Sim *sim, SimInstance *instance, CharLcdState *st, SimTime now) {
    if (now - st->rise_time < st->t->tpw) {
        st->acc_violated = 1;
    }
    if (st->acc_rw != 0) {
        /* A read (or an unknown access) ends: Z at + tdhr. */
        schedule(sim, instance, st, TIMER_READ_OFF, st->t->tdhr);
        if (st->acc_rw == 1) {
            int data_end = st->dl == 1 || (st->dl == 0 && st->phase == 1);

            if (st->dl == 0 && st->phase >= 0) {
                st->phase = !st->phase;
            }
            if (st->acc_rs == 1 && data_end) {
                ac_step(st, st->id);   /* AC moves after a data read (not while LOST) */
            } else if (st->acc_rs < 0 || (st->acc_rs == 1 && st->dl < 0)) {
                st->ac = -1;
            }
        }
        return;
    }
    {
        int rw = level(sim, instance, st->pins.rw);
        uint8_t mask = mode_mask(st);
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
                st->acc_violated = 1;
            }
        }
        st->pending = 1;
        st->pend_fall = now;
        st->pend_rs = level(sim, instance, st->pins.rs);
        if (rw != 0) {
            st->pend_rs = -1;   /* R/W X at a write: LOST when it takes effect */
        }
        st->pend_value = v;
        st->pend_known = k;
        st->pend_mask = mask;
        st->pend_violated = st->acc_violated;
        {
            SimTime hold = st->t->th > st->t->tah ? st->t->th : st->t->tah;

            if (hold == 0) {
                apply_write(st, now);
            } else {
                schedule(sim, instance, st, TIMER_WRITE, hold);
            }
        }
    }
}

static void update_light(Sim *sim, SimInstance *instance, CharLcdState *st) {
    if (st->pins.has_backlight) {
        int on = sim_read_pin(sim, instance, st->pins.anode) == LOGIC_1 &&
                 sim_read_pin(sim, instance, st->pins.cathode) == LOGIC_0;

        lights_set(&st->light, 0, on, sim_now(sim));
    }
}

static size_t charlcd_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *charlcd_create(const void *config) {
    const CharLcdPins *pins = part_internal(config)->bound;
    CharLcdState *st = calloc(1, sizeof(*st));

    if (st != NULL) {
        st->pins = *pins;
        st->t = &pins->device->t;
        st->e = -1;
        power_up(st);
    }
    return st;
}

static void charlcd_destroy(void *state) {
    free(state);
}

static void charlcd_start(Sim *sim, SimInstance *instance, void *raw) {
    CharLcdState *st = raw;

    st->e = level(sim, instance, st->pins.e);
    update_light(sim, instance, st);
}

static void charlcd_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    CharLcdState *st = raw;
    SimTime now = sim_now(sim);
    unsigned i;

    (void)old_value;
    (void)new_value;
    if (st->pins.has_backlight && (pin == st->pins.anode || pin == st->pins.cathode)) {
        update_light(sim, instance, st);
    }
    if (pin == st->pins.e) {
        int e = level(sim, instance, st->pins.e);
        int was = st->e;

        st->e = e;
        if (e == was) {
            return;
        }
        if (was == 1) {
            e_fall(sim, instance, st, now);
        }
        if (e == 1) {
            e_rise(sim, instance, st, now);
        } else if (e < 0 && !in_power_on(st, now)) {
            lose(st);
            st->pending = 0;
            if (st->driving) {
                st->drive_x = 1;
                drive(sim, instance, st, 0);
            }
        }
        return;
    }
    if (pin == st->pins.rs || pin == st->pins.rw) {
        if (pin == st->pins.rs) {
            st->rs_time = now;
        } else {
            st->rw_time = now;
        }
        if (st->pending && now - st->pend_fall < st->t->tah) {
            st->pend_violated = 1;
        }
        return;
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (pin == st->pins.db[i]) {
            st->db_time[i] = now;
            if (st->pending && ((st->pend_mask >> i) & 1u) && now - st->pend_fall < st->t->th) {
                st->pend_violated = 1;
            }
            return;
        }
    }
}

static void charlcd_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    CharLcdState *st = raw;
    unsigned kind = id & 3u;

    if ((id >> 2) != (st->seq & 0x3FFFFFFFu)) {
        return;   /* from an earlier access */
    }
    switch (kind) {
        case TIMER_READ_DATA:
            if (st->driving) {
                drive(sim, instance, st, 0);
            }
            break;
        case TIMER_READ_OFF:
            if (st->driving) {
                drive(sim, instance, st, 1);
                st->driving = 0;
            }
            break;
        case TIMER_WRITE:
            if (st->pending) {
                apply_write(st, sim_now(sim));
            }
            break;
    }
}

/* ---- Lights ---- */

/* One light, the backlight; a module without one has no lights. */
static size_t charlcd_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                             SimTime since, unsigned char *out, size_t cap) {
    const CharLcdState *st = raw;

    (void)sim;
    (void)instance;
    return st->pins.has_backlight ? lights_read(&st->light, 1, since, out, cap) : 0;
}

static size_t charlcd_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                                 uint64_t *out, size_t cap) {
    const CharLcdState *st = raw;

    (void)instance;
    return st->pins.has_backlight ? lights_time(&st->light, 1, sim_now(sim), out, cap) : 0;
}

/* ---- The image (section 4.7) ---- */

static int t_or(int a, int b) {
    return a == 1 || b == 1 ? 1 : a < 0 || b < 0 ? -1 : 0;
}

static int t_and(int a, int b) {
    return a == 0 || b == 0 ? 0 : a < 0 || b < 0 ? -1 : 1;
}

/* One dot for a known N: 1, 0 or -1. */
static int image_dot(const CharLcdState *st, int n, int blink_on, unsigned row, unsigned column,
                     unsigned y, unsigned x) {
    int address = -1, index = -1, glyph, here, extra;

    if (n == 0 && row == 1) {
        return 0;
    }
    if (st->shift >= 0) {
        if (n == 1) {
            unsigned pos = ((unsigned)st->shift + column) % 40;

            address = (int)(row * 0x40 + pos);
            index = (int)(row * 40 + pos);
        } else {
            address = index = (int)(((unsigned)st->shift + column) % 80);
        }
    }
    if (index < 0 || st->ddram_known[index] != 0xFF) {
        glyph = -1;
    } else {
        unsigned code = st->ddram[index];
        unsigned mask = 0x10u >> x;

        if (code < 0x10) {
            unsigned cell = (code & 7u) * 8 + y;

            glyph = (st->cgram_known[cell] & mask) ? (st->cgram[cell] & mask ? 1 : 0) : -1;
        } else {
            glyph = CHARLCD_ROM_A00[code][y] & mask ? 1 : 0;
        }
    }
    /* Is AC's address shown here? */
    if (st->ram == 1) {
        here = 0;
    } else if (st->ac >= 0 && address >= 0) {
        here = address == st->ac ? (st->ram == 0 ? 1 : -1) : 0;
    } else {
        here = -1;
    }
    extra = y == 7 ? t_and(st->c, here) : 0;
    if (blink_on) {
        extra = t_or(extra, t_and(st->b, here));
    }
    return t_and(st->d, t_or(glyph, extra));
}

static int charlcd_lcd_state(const Sim *sim, const void *raw, LcdState *out) {
    const CharLcdState *st = raw;
    const PartCharLcd *info = &st->pins.info;
    SimTime now = sim_now(sim);
    int blink_on = (now / LCD_BLINK_PS) % 2 == 0;
    unsigned row, column, y, x;

    memset(out, 0, sizeof(*out));
    out->columns = info->columns;
    out->rows = info->rows;
    out->busy = busy_flag(st, now);
    memcpy(out->ddram, st->ddram, sizeof out->ddram);
    memcpy(out->ddram_known, st->ddram_known, sizeof out->ddram_known);
    memcpy(out->cgram, st->cgram, sizeof out->cgram);
    memcpy(out->cgram_known, st->cgram_known, sizeof out->cgram_known);
    for (row = 0; row < info->rows; row++) {
        for (column = 0; column < info->columns; column++) {
            for (y = 0; y < LCD_STATE_CELL_HEIGHT; y++) {
                for (x = 0; x < LCD_STATE_CELL_WIDTH; x++) {
                    int dot;

                    if (st->n >= 0) {
                        dot = image_dot(st, st->n, blink_on, row, column, y, x);
                    } else {
                        int a = image_dot(st, 0, blink_on, row, column, y, x);
                        int b = image_dot(st, 1, blink_on, row, column, y, x);

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
    charlcd_load_device, charlcd_bind_pinout, charlcd_load_part
};
