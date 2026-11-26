#include "serialmem_int.h"
#include "logic_alloc.h"

int smem_t_not(int a) { return a == TX ? TX : !a; }
int smem_t_and(int a, int b) {
    if (a == T0 || b == T0) {
        return T0;
    }
    return a == TX || b == TX ? TX : T1;
}
int smem_t_or(int a, int b) {
    if (a == T1 || b == T1) {
        return T1;
    }
    return a == TX || b == TX ? TX : T0;
}
const char *const smem_TYPE_NAMES[] = {"eeprom", "fram", "flash", "dataflash", "pageeeprom", NULL};
const char *const smem_PROT_NAMES[] = {"wpen", "wp", "sector", "block", NULL};
const char *const smem_PIN_KEY_NAMES[SM_PINS] = {
    "chipSelect", "clock", "dataIn", "dataOut", "writeProtect", "hold", "reset"
};

const char *const smem_DEVICE_KEYS[] = {
    "type", "size", "addressBytes", "a8InOpcode", "pageSize", "protection", "protectSize",
    "chipSelect", "clock", "dataIn", "dataOut", "writeProtect", "hold", "reset",
    "writeCycle", "pageProgram", "byteProgram", "pageErase", "sectorErase", "blockErase32",
    "blockErase64", "chipErase", "writeStatus", "outputValid", "outputDisable", "wake",
    "fastRead", "sleep", "jedecId", "pullUp", "statusOnes", "busyOnes",
    "maxClock", "endurance", "retention", "vcc", "inputMax", NULL
};
const char *const smem_PART_KEYS[] = {"vcc", "inputMax", "maxClock", NULL};

/* ---- Inputs ------------------------------------------------------------------------------- */

/* WP#, HOLD#: 1 high, 0 low, TX unknown; a pin the package leaves out is high. */
int smem_control(const SmState *s, int which) {
    size_t pin = s->pins->pin[which];
    LogicValue v;

    if (pin == SM_NONE) {
        return T1;
    }
    v = serial_level(s->sim, s->instance, pin, s->dev->pull_up ? LOGIC_1 : LOGIC_X);
    return v == LOGIC_1 ? T1 : v == LOGIC_0 ? T0 : TX;
}

SimTime smem_now_of(const SmState *s) {
    return sim_now(s->sim);
}

/* ---- Protection ---------------------------------------------------------------------------- */

uint32_t smem_quarter(const SmState *s) {
    return s->dev->size / 4;
}

/* Whether bytes [start, start + length) overlap the blocks the BP bits protect. */
static int bp_protected(const SmState *s, uint32_t start, uint32_t length) {
    uint32_t last = start + length - 1;
    int seen0 = 0, seen1 = 0;
    unsigned c;

    for (c = 0; c < 4; c++) {
        uint32_t zone;
        int p;

        if (((c ^ s->bp) & s->bp_known & 3u) != 0) {
            continue;
        }
        if (s->dev->prot == PROT_BLOCK) {
            p = (c & 1u) != 0;
        } else {
            zone = c == 0 ? s->dev->size : c == 1 ? s->dev->size - smem_quarter(s)
                   : c == 2 ? s->dev->size / 2 : 0;
            p = c != 0 && last >= zone;
        }
        seen0 |= !p;
        seen1 |= p;
    }
    return seen0 && seen1 ? TX : seen1 ? T1 : T0;
}

int smem_range_protected(const SmState *s, uint32_t start, uint32_t length) {
    int result = T0;
    uint32_t sector, last;

    if (s->dev->prot != PROT_SECTOR) {
        return bp_protected(s, start, length);
    }
    last = (start + length - 1) / s->dev->protect_size;
    for (sector = start / s->dev->protect_size; sector <= last && sector < s->sprot_count; sector++) {
        result = smem_t_or(result, s->sprot[sector]);
    }
    return result;
}

/* ---- Memory -------------------------------------------------------------------------------- */

void smem_put_byte(SmState *s, uint32_t address, uint8_t v, uint8_t k, int and_mode) {
    address &= s->mask;
    if (and_mode) {
        uint8_t ov = s->value[address], ok = s->known[address];
        uint8_t rk = (uint8_t)((ok & k) | (ok & (uint8_t)~ov) | (k & (uint8_t)~v));

        s->value[address] = (uint8_t)(ov & v & rk);
        s->known[address] = rk;
    } else {
        s->value[address] = (uint8_t)(v & k);
        s->known[address] = k;
    }
}

/* Every byte whose address matches the known bits (ignoring the low span bits) is unknown. */
void smem_corrupt_matching(SmState *s, uint32_t addr, uint32_t known, uint32_t span) {
    uint32_t w = 1;
    uint32_t mk;
    uint64_t a;

    while (w < span) {
        w <<= 1;
    }
    mk = known & ~(w - 1) & s->mask;
    for (a = 0; a <= s->mask; a++) {
        if ((((uint32_t)a ^ addr) & mk) == 0) {
            s->known[a] = 0;
        }
    }
}

static void erase_bytes(SmState *s, uint32_t start, uint32_t length, int maybe) {
    uint32_t i;

    for (i = 0; i < length; i++) {
        uint32_t a = (start + i) & s->mask;

        if (maybe) {
            s->known[a] = (uint8_t)(s->known[a] & s->value[a]);
        } else {
            s->value[a] = 0xFF;
            s->known[a] = 0xFF;
        }
    }
}

/* ---- Status -------------------------------------------------------------------------------- */

void smem_status_byte(const SmState *s, unsigned which, uint8_t *v, uint8_t *k) {
    const SerialmemDevice *d = s->dev;
    uint8_t value = 0;
    uint8_t known = 0xFF;
    int wp;

    if (d->type == SM_FLASH) {
        value = (uint8_t)(s->busy ? 1 : 0);
        if (which == 1) {
            if (s->rste) {
                value |= 0x10;
            }
            *v = value;
            *k = 0xFF;
            return;
        }
        if (s->wel == T1) {
            value |= 2;
        } else if (s->wel == TX) {
            known &= (uint8_t)~2;
        }
        if (s->lock == T1) {
            value |= 0x80;
        } else if (s->lock == TX) {
            known &= (uint8_t)~0x80;
        }
        wp = smem_control(s, SM_WP);
        if (wp == T1) {
            value |= 0x10;
        } else if (wp == TX) {
            known &= (uint8_t)~0x10;
        }
        if (d->prot == PROT_SECTOR) {
            int any0 = 0, any1 = 0, anyx = 0;
            uint32_t i;

            for (i = 0; i < s->sprot_count; i++) {
                any0 |= s->sprot[i] == T0;
                any1 |= s->sprot[i] == T1;
                anyx |= s->sprot[i] == TX;
            }
            if (anyx) {
                known &= (uint8_t)~0x0C;
            } else if (any1 && !any0) {
                value |= 0x0C;
            } else if (any0 && any1) {
                value |= 0x04;
            }
        } else {
            if (s->bp & 1u) {
                value |= 4;
            }
            if (!(s->bp_known & 1u)) {
                known &= (uint8_t)~4;
            }
        }
        *v = value;
        *k = known;
        return;
    }
    if (s->busy) {
        value = (uint8_t)(1 | d->busy_ones);
    }
    value |= d->status_ones;
    if (s->wel == T1) {
        value |= 2;
    } else if (s->wel == TX) {
        known &= (uint8_t)~2;
    }
    value |= (uint8_t)((s->bp & 3u) << 2);
    known &= (uint8_t)~((~s->bp_known & 3u) << 2);
    if (d->prot == PROT_WPEN) {
        if (s->lock == T1) {
            value |= 0x80;
        } else if (s->lock == TX) {
            known &= (uint8_t)~0x80;
        }
    }
    *v = value;
    *k = known;
}

/* ---- Busy ---------------------------------------------------------------------------------- */

void smem_apply_pending(SmState *s) {
    Pending *p = &s->pend;
    uint32_t i;

    switch (p->kind) {
        case P_PAGE: {
            int and_mode = s->dev->type == SM_FLASH;

            for (i = 0; i < s->dev->page_size; i++) {
                if (s->buf_loaded[i]) {
                    smem_put_byte(s, s->page_base + i, s->buf_value[i], p->maybe ? 0 : s->buf_known[i],
                             and_mode);
                }
            }
            break;
        }
        case P_CORRUPT:
            smem_corrupt_matching(s, p->addr, p->known, p->length);
            break;
        case P_ERASE:
            erase_bytes(s, p->addr, p->length, p->maybe);
            break;
        case P_WRSR:
        case P_WRSR_FLASH:
            if (p->kind == P_WRSR_FLASH && s->dev->prot == PROT_BLOCK) {
                if (p->maybe) {
                    s->bp_known = 0;
                    s->lock = TX;
                } else {
                    s->bp = (uint8_t)((p->value >> 2) & 1u);
                    s->bp_known = (uint8_t)((p->value_known >> 2) & 1u);
                    s->lock = !(p->value_known & 0x80) ? TX : (p->value & 0x80) ? T1 : T0;
                }
            } else if (p->kind == P_WRSR) {
                if (p->maybe) {
                    s->bp_known = 0;
                    if (s->dev->prot == PROT_WPEN) {
                        s->lock = TX;
                    }
                } else {
                    s->bp = (uint8_t)((p->value >> 2) & 3u);
                    s->bp_known = (uint8_t)((p->value_known >> 2) & 3u);
                    if (s->dev->prot == PROT_WPEN) {
                        s->lock = !(p->value_known & 0x80) ? TX : (p->value & 0x80) ? T1 : T0;
                    }
                }
            }
            break;
        case P_NONE:
            break;
    }
    p->kind = P_NONE;
}

static void finish_busy(SmState *s) {
    smem_apply_pending(s);
    s->busy = 0;
    if (s->dev->type == SM_EEPROM) {
        s->wel = T0;
    }
}

/* Starts an operation taking `duration`; 0 applies it at once. */
void smem_start_op(SmState *s, SimTime duration) {
    if (duration == 0) {
        smem_apply_pending(s);
        if (s->dev->type == SM_EEPROM) {
            s->wel = T0;
        }
        return;
    }
    s->busy = 1;
    s->busy_until = smem_now_of(s) + duration;
    sim_schedule_timer(s->sim, s->instance, TIMER_BUSY, duration);
}

/* ---- Pins ---------------------------------------------------------------------------------- */

/* HOLD#: `release` before the front end sees an SCK edge, else after. */
static void hold_eval(SmState *s, int release) {
    int h;
    LogicValue sck;

    if (s->pins->pin[SM_HOLD] == SM_NONE || !s->spi.active) {
        return;
    }
    h = smem_control(s, SM_HOLD);
    if (h == TX) {
        serial_spi_hold(&s->spi, s->sim, s->instance, 2, 0);
        return;
    }
    sck = serial_level(s->sim, s->instance, s->pins->pin[SM_SCK], LOGIC_X);
    if (sck != LOGIC_0) {
        return;
    }
    if (release && s->spi.held && h == T1) {
        serial_spi_hold(&s->spi, s->sim, s->instance, 0, s->dev->t_valid);
    } else if (!release && !s->spi.held && h == T0) {
        serial_spi_hold(&s->spi, s->sim, s->instance, 1, s->dev->t_disable);
    }
}

static void sm_start(Sim *sim, SimInstance *instance, void *raw) {
    SmState *s = raw;

    s->sim = sim;
    s->instance = instance;
    if (s->passive) {
        return;
    }
    serial_spi_start(&s->spi, sim, instance);
}

static void sm_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    SmState *s = raw;
    const size_t *p;

    (void)old_value;
    (void)new_value;
    s->sim = sim;
    s->instance = instance;
    if (s->passive) {
        return;
    }
    p = s->pins->pin;
    if (pin == p[SM_WP]) {
        int level = smem_control(s, SM_WP);

        if (s->spi.active && level != T1) {
            s->wp_fell = 1;
        }
        return;
    }
    if (pin == p[SM_HOLD]) {
        hold_eval(s, 1);
        hold_eval(s, 0);
        return;
    }
    if (pin == p[SM_CS]) {
        if (!s->spi.active) {
            LogicValue sck = serial_level(sim, instance, p[SM_SCK], LOGIC_0);

            s->spi_pins.mode = sck == LOGIC_1 ? 3u : 0u;
        }
        serial_spi_pin(&s->spi, sim, instance, pin, &s->events);
        hold_eval(s, 0);
        return;
    }
    if (pin == p[SM_SCK]) {
        hold_eval(s, 1);
        serial_spi_pin(&s->spi, sim, instance, pin, &s->events);
        hold_eval(s, 0);
    }
}

static void sm_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    SmState *s = raw;

    (void)timer_id;
    s->sim = sim;
    s->instance = instance;
    if (s->passive || !s->busy) {
        return;
    }
    if (sim_now(sim) >= s->busy_until) {
        finish_busy(s);
    }
}

/* ---- Inspection hooks ---------------------------------------------------------------------- */

static int sm_memory_size(const void *raw, uint32_t *words, unsigned *bits) {
    const SmState *s = raw;

    if (s->passive) {
        return 0;
    }
    if (words != NULL) {
        *words = s->dev->size;
    }
    if (bits != NULL) {
        *bits = 8;
    }
    return 1;
}

static size_t sm_memory_read(const void *raw, uint32_t start, size_t count, uint32_t *values,
                             uint32_t *known) {
    const SmState *s = raw;
    size_t i;

    if (s->passive || start >= s->dev->size) {
        return 0;
    }
    if (count > s->dev->size - start) {
        count = s->dev->size - start;
    }
    for (i = 0; i < count; i++) {
        if (values != NULL) {
            values[i] = s->value[start + i];
        }
        if (known != NULL) {
            known[i] = s->known[start + i];
        }
    }
    return count;
}

static void sm_memory_load(void *raw, uint32_t start, const uint32_t *values, size_t count) {
    SmState *s = raw;
    size_t i;

    if (s->passive) {
        return;
    }
    for (i = 0; i < count; i++) {
        s->value[start + i] = (uint8_t)values[i];
        s->known[start + i] = 0xFF;
    }
}

/* ---- Create / destroy ------------------------------------------------------------------------ */

static size_t sm_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void sm_destroy(void *raw) {
    SmState *s = raw;

    if (s == NULL) {
        return;
    }
    logic_free(s->value);
    logic_free(s->known);
    logic_free(s->sprot);
    logic_free(s->buf_value);
    logic_free(s->buf_known);
    logic_free(s->buf_loaded);
    logic_free(s);
}

static void power_up(SmState *s) {
    s->bp = 0;
    s->bp_known = 3;
    s->lock = T0;
    s->wel = T0;
    s->rste = 0;
    s->busy = 0;
    s->pend.kind = P_NONE;
    s->asleep = 0;
    s->wake_until = 0;
    s->dpd = 0;
    if (s->dev->prot == PROT_SECTOR) {
        smem_set_all_sectors(s, T1);
    }
}

static void *sm_create(const void *config) {
    const Part *part = config;
    const PartInternal *internal = part_internal(part);
    const SerialmemPins *pins = internal->bound;
    const SerialmemDevice *d = pins->device;
    SmState *s = logic_calloc(1, sizeof *s);

    if (s == NULL) {
        return NULL;
    }
    s->dev = d;
    s->pins = pins;
    if (!d->simulated) {
        s->passive = 1;
        return s;
    }
    s->mask = d->size - 1;
    s->value = logic_calloc(d->size, 1);
    s->known = logic_calloc(d->size, 1);
    s->buf_value = logic_calloc(d->page_size, 1);
    s->buf_known = logic_calloc(d->page_size, 1);
    s->buf_loaded = logic_calloc(d->page_size, 1);
    if (d->prot == PROT_SECTOR) {
        s->sprot_count = d->size / d->protect_size;
        s->sprot = logic_calloc(s->sprot_count, 1);
    }
    if (s->value == NULL || s->known == NULL || s->buf_value == NULL || s->buf_known == NULL ||
        s->buf_loaded == NULL || (d->prot == PROT_SECTOR && s->sprot == NULL)) {
        sm_destroy(s);
        return NULL;
    }
    if (d->type != SM_FRAM) {
        memset(s->value, 0xFF, d->size);
        memset(s->known, 0xFF, d->size);
    }
    s->spi_pins.cs = pins->pin[SM_CS];
    s->spi_pins.sclk = pins->pin[SM_SCK];
    s->spi_pins.mosi = pins->pin[SM_SI];
    s->spi_pins.miso = pins->pin[SM_SO];
    s->spi_pins.mode = 0;
    serial_spi_init(&s->spi, &s->spi_pins);
    s->spi.t_valid = d->t_valid;
    s->spi.t_out = d->t_valid;
    s->spi.t_float = d->t_disable;
    s->events.context = s;
    s->events.begin = smem_sm_begin;
    s->events.next = smem_sm_next;
    s->events.received = smem_sm_received;
    s->events.end = smem_sm_end;
    power_up(s);
    return s;
}

const SimComponentType smem_SERIALMEM_MODEL = {
    "serialmem", sm_pin_count, sm_create, sm_destroy, sm_start, sm_pin_changed, sm_timer,
    NULL, NULL, sm_memory_size, sm_memory_read, NULL, sm_memory_load
};

