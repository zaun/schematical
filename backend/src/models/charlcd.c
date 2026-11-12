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
#include "model_pins.h"

#define LCD_DATA_BITS 8
#define LCD_MAX_BUS_PINS (4 + LCD_DATA_BITS)   /* RS, R/W, E1, E2, DB0-DB7 */
#define LCD_MAX_ENABLES 2
#define LCD_MAX_LEDS 3

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
    unsigned columns, rows, enables;
    const char *controller, *rom, *panel, *backlight_color;
    const char *rs, *rw, *e[LCD_MAX_ENABLES];   /* e[1]: NULL with one enable */
    const char *const *data;
    const char *contrast;
    size_t leds;                                 /* backlight LEDs, 0-3 */
    const char *anode[LCD_MAX_LEDS], *cathode[LCD_MAX_LEDS];
    const char *colors[LCD_MAX_LEDS];
    CharLcdTiming t;
    const PartVoltage *vcc;
} CharLcdDevice;

/* Pin indices for one pinout. */
typedef struct {
    size_t rs, rw, e[LCD_MAX_ENABLES], db[LCD_DATA_BITS], contrast;
    unsigned enables;
    size_t anode[LCD_MAX_LEDS], cathode[LCD_MAX_LEDS];
    size_t leds;
    size_t bus[LCD_MAX_BUS_PINS];
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

/* "bus".e: one pin name, or two (E1, E2). */
static int load_enables(KindContext *ctx, const cJSON *bus, CharLcdDevice *device) {
    const cJSON *e = kind_member(ctx, bus, "e", 1);

    if (e == NULL) {
        return -1;
    }
    if (cJSON_IsString(e)) {
        device->enables = 1;
        return kind_string(ctx, bus, "e", &device->e[0]);
    }
    if (cJSON_IsArray(e) && cJSON_GetArraySize(e) == LCD_MAX_ENABLES) {
        const char *const *names;
        size_t count;

        if (kind_names(ctx, bus, "e", &names, &count) != 0) {
            return -1;
        }
        if (count == LCD_MAX_ENABLES) {
            device->enables = LCD_MAX_ENABLES;
            device->e[0] = names[0];
            device->e[1] = names[1];
            return 0;
        }
    }
    return kind_error(ctx, "\"e\" must be a pin name or 2 pin names");
}

/* "backlight": one LED object, or an array of 1-3. */
static int load_backlight(KindContext *ctx, const cJSON *backlight, CharLcdDevice *device) {
    int array = cJSON_IsArray(backlight);
    int count = array ? cJSON_GetArraySize(backlight) : cJSON_IsObject(backlight) ? 1 : 0;
    int i;

    if (count < 1 || count > LCD_MAX_LEDS) {
        return kind_error(ctx, "\"backlight\" must be an object or 1 to 3 objects");
    }
    for (i = 0; i < count; i++) {
        const cJSON *led = array ? cJSON_GetArrayItem(backlight, i) : backlight;
        int failed;

        if (!cJSON_IsObject(led)) {
            return kind_error(ctx, "\"backlight\" must be an object or 1 to 3 objects");
        }
        if (array) {
            kind_push(ctx, "backlight[%d]", i);
        }
        failed = kind_check_keys(ctx, led, BACKLIGHT_KEYS, NULL) != 0 ||
                 kind_string(ctx, led, "anode", &device->anode[i]) != 0 ||
                 kind_string(ctx, led, "cathode", &device->cathode[i]) != 0 ||
                 kind_string(ctx, led, "color", &device->colors[i]) != 0;
        if (array) {
            kind_pop(ctx);
        }
        if (failed) {
            return -1;
        }
    }
    device->leds = (size_t)count;
    device->backlight_color = device->colors[0];
    return 0;
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
    if (rows != 1 && rows != 2 && rows != 4) {
        kind_error(ctx, "\"rows\" must be 1, 2 or 4");
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
    if (strcmp(device->panel, "yellow-green") != 0 && strcmp(device->panel, "blue-negative") != 0 &&
        strcmp(device->panel, "gray") != 0 && strcmp(device->panel, "white") != 0) {
        kind_error(ctx, "\"panel\" must be yellow-green, blue-negative, gray or white");
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
        load_enables(ctx, bus, device) != 0) {
        return NULL;
    }
    if (device->enables == 2 && device->rows != 4) {
        kind_error(ctx, "two enables need \"rows\": 4");
        return NULL;
    }
    if (device->enables == 1 && device->rows == 4 && device->columns > 20) {
        kind_error(ctx, "4 rows on one controller need \"columns\" of 20 or less");
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
    if (backlight != NULL && load_backlight(ctx, backlight, device) != 0) {
        return NULL;
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
    size_t i, n;

    if (bound == NULL ||
        kind_pin(ctx, pins, pin_count, device->rs, &bound->rs) != 0 ||
        kind_pin(ctx, pins, pin_count, device->rw, &bound->rw) != 0) {
        return NULL;
    }
    for (i = 0; i < device->enables; i++) {
        if (kind_pin(ctx, pins, pin_count, device->e[i], &bound->e[i]) != 0) {
            return NULL;
        }
    }
    if (kind_pin(ctx, pins, pin_count, device->contrast, &bound->contrast) != 0) {
        return NULL;
    }
    bound->enables = device->enables;
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (kind_pin(ctx, pins, pin_count, device->data[i], &bound->db[i]) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < device->leds; i++) {
        if (kind_pin(ctx, pins, pin_count, device->anode[i], &bound->anode[i]) != 0 ||
            kind_pin(ctx, pins, pin_count, device->cathode[i], &bound->cathode[i]) != 0) {
            return NULL;
        }
    }
    bound->leds = device->leds;
    /* RS, R/W, E (or E1, E2), DB0-DB7. */
    n = 0;
    bound->bus[n++] = bound->rs;
    bound->bus[n++] = bound->rw;
    for (i = 0; i < device->enables; i++) {
        bound->bus[n++] = bound->e[i];
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        bound->bus[n++] = bound->db[i];
    }
    bound->info.columns = device->columns;
    bound->info.rows = device->rows;
    bound->info.controller = device->controller;
    bound->info.rom = device->rom;
    bound->info.panel = device->panel;
    bound->info.backlight_color = device->backlight_color;
    bound->info.bus_pins = bound->bus;
    bound->info.bus_pin_count = n;
    bound->info.enables = device->enables;
    bound->info.backlight_count = device->leds;
    bound->info.backlight_colors = device->leds > 0 ? device->colors : NULL;
    bound->device = device;
    return bound;
}

static int charlcd_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CharLcdDevice *device = raw;

    out->charlcd = &((const CharLcdPins *)bound)->info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

int charlcd_backlight_pins(const Part *part, size_t index, size_t *anode, size_t *cathode) {
    const CharLcdPins *pins;

    if (part == NULL || part->kind != PART_CHARLCD) {
        return 0;
    }
    pins = part_internal(part)->bound;
    if (index >= pins->leds) {
        return 0;
    }
    *anode = pins->anode[index];
    *cathode = pins->cathode[index];
    return 1;
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

/* One controller: an HD44780 with its own registers, RAM, busy flag,
   enable pin and access record (a 40x4 has two on one bus). */
typedef struct {
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

    /* Its enable. */
    int e;                   /* E as last seen: 0, 1, -1 */
    SimTime rise_time, prev_rise;
    int have_rise;

    /* The access in progress (E high). */
    int acc_rs, acc_rw, acc_violated;
    uint32_t seq;            /* counts this controller's accesses; stale timers carry an old one */

    /* Read drive. */
    int driving, drive_x;
    uint8_t drive_mask, read_value, read_known;
    uint8_t read_byte, read_byte_known;   /* 4-bit: the byte whose nibbles a read returns */

    /* A write waiting out its hold window. */
    int pending, pend_rs, pend_violated;
    SimTime pend_fall;
    uint8_t pend_value, pend_known, pend_mask;
} CharLcdCtl;

/* The module: the shared bus inputs, the controllers and the backlight. */
typedef struct {
    CharLcdPins pins;
    const CharLcdTiming *t;

    /* Shared inputs: RS, R/W and DB change times. */
    SimTime rs_time, rw_time, db_time[LCD_DATA_BITS];

    CharLcdCtl ctl[LCD_MAX_ENABLES];   /* ctl[1] only with two enables */
    Light light;
} CharLcdState;

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

static void drive(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl, int z) {
    unsigned i;

    for (i = 0; i < LCD_DATA_BITS; i++) {
        LogicValue v;

        if (!((ctl->drive_mask >> i) & 1u)) {
            continue;
        }
        if (z) {
            v = LOGIC_Z;
        } else if (ctl->drive_x || !((ctl->read_known >> i) & 1u)) {
            v = LOGIC_X;
        } else {
            v = (ctl->read_value >> i) & 1u ? LOGIC_1 : LOGIC_0;
        }
        sim_schedule_drive(sim, instance, st->pins.db[i], v, DRIVE_STRONG, 0);
    }
}

static void schedule(Sim *sim, SimInstance *instance, CharLcdState *st, CharLcdCtl *ctl,
                     unsigned kind, SimTime delay) {
    sim_schedule_timer(sim, instance, (uint32_t)(kind | (ctl->seq << 2)), delay);
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

static void update_light(Sim *sim, SimInstance *instance, CharLcdState *st) {
    if (st->pins.leds > 0) {
        int on = sim_read_pin(sim, instance, st->pins.anode[0]) == LOGIC_1 &&
                 sim_read_pin(sim, instance, st->pins.cathode[0]) == LOGIC_0;

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
    free(state);
}

static void charlcd_start(Sim *sim, SimInstance *instance, void *raw) {
    CharLcdState *st = raw;

    st->ctl[0].e = level(sim, instance, st->pins.e[0]);
    update_light(sim, instance, st);
}

static void charlcd_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    CharLcdState *st = raw;
    CharLcdCtl *ctl = &st->ctl[0];
    SimTime now = sim_now(sim);
    unsigned i;

    (void)old_value;
    (void)new_value;
    if (st->pins.leds > 0 && (pin == st->pins.anode[0] || pin == st->pins.cathode[0])) {
        update_light(sim, instance, st);
    }
    if (pin == st->pins.e[0]) {
        int e = level(sim, instance, st->pins.e[0]);
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
        return;
    }
    if (pin == st->pins.rs || pin == st->pins.rw) {
        if (pin == st->pins.rs) {
            st->rs_time = now;
        } else {
            st->rw_time = now;
        }
        if (ctl->pending && now - ctl->pend_fall < st->t->tah) {
            ctl->pend_violated = 1;
        }
        return;
    }
    for (i = 0; i < LCD_DATA_BITS; i++) {
        if (pin == st->pins.db[i]) {
            st->db_time[i] = now;
            if (ctl->pending && ((ctl->pend_mask >> i) & 1u) && now - ctl->pend_fall < st->t->th) {
                ctl->pend_violated = 1;
            }
            return;
        }
    }
}

static void charlcd_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t id) {
    CharLcdState *st = raw;
    CharLcdCtl *ctl = &st->ctl[0];
    unsigned kind = id & 3u;

    if ((id >> 2) != (ctl->seq & 0x3FFFFFFFu)) {
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

/* One light, the backlight; a module without one has no lights. */
static size_t charlcd_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                             SimTime since, unsigned char *out, size_t cap) {
    const CharLcdState *st = raw;

    (void)sim;
    (void)instance;
    return st->pins.leds > 0 ? lights_read(&st->light, 1, since, out, cap) : 0;
}

static size_t charlcd_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                                 uint64_t *out, size_t cap) {
    const CharLcdState *st = raw;

    (void)instance;
    return st->pins.leds > 0 ? lights_time(&st->light, 1, sim_now(sim), out, cap) : 0;
}

/* ---- The image (section 4.7) ---- */

static int t_or(int a, int b) {
    return a == 1 || b == 1 ? 1 : a < 0 || b < 0 ? -1 : 0;
}

static int t_and(int a, int b) {
    return a == 0 || b == 0 ? 0 : a < 0 || b < 0 ? -1 : 1;
}

/* One dot for a known N: 1, 0 or -1. */
static int image_dot(const CharLcdCtl *ctl, int n, int blink_on, unsigned row, unsigned column,
                     unsigned y, unsigned x) {
    int address = -1, index = -1, glyph, here, extra;

    if (n == 0 && row == 1) {
        return 0;
    }
    if (ctl->shift >= 0) {
        if (n == 1) {
            unsigned pos = ((unsigned)ctl->shift + column) % 40;

            address = (int)(row * 0x40 + pos);
            index = (int)(row * 40 + pos);
        } else {
            address = index = (int)(((unsigned)ctl->shift + column) % 80);
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
    const CharLcdCtl *ctl = &st->ctl[0];
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
        for (column = 0; column < info->columns; column++) {
            for (y = 0; y < LCD_STATE_CELL_HEIGHT; y++) {
                for (x = 0; x < LCD_STATE_CELL_WIDTH; x++) {
                    int dot;

                    if (ctl->n >= 0) {
                        dot = image_dot(ctl, ctl->n, blink_on, row, column, y, x);
                    } else {
                        int a = image_dot(ctl, 0, blink_on, row, column, y, x);
                        int b = image_dot(ctl, 1, blink_on, row, column, y, x);

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
