/*
 * The rtc kind (plan-24-F; rtc.h, planning/plan-24-data/schema.md § 10): the
 * converter engine runs the bus and registers, this file keeps time.
 *
 * The clock counts simulated time from 2000-01-01 00:00:00 (a Saturday): a
 * tick each second (each 10 ms with a hundredths field) advances the
 * counters with carries and leap years (every fourth year: the parts cover
 * 2000-2099) and writes them into the time registers in each field's format
 * (BCD or binary, 12- or 24-hour). Writing a time register loads the
 * counter (and restarts the second); the latch mechanisms freeze the
 * registers or hold writes until the latch bit clears; the UIP flag rises
 * 244 us before each update; the stop bit (or inhibit bit) halts counting.
 * Alarms compare at each second, periodic interrupts and the watchdog run
 * on their own timers. Flags and pins follow schema.md § 10.
 */

#include "rtc.h"

#include <stdlib.h>
#include <string.h>

#include "converter_int.h"
#include "converter_state.h"

enum {
    RT_TICK = 0, RT_UIP = 1, RT_PERIODIC = 2, RT_WATCHDOG = 3, RT_PULSE = 4,
    RT_UIP_PS = 244000000       /* 244 us */
};

typedef struct {
    int t[RU_COUNT];        /* hundredth, second, minute, hour (0-23), weekday (0 = Sunday),
                               date, month, year (full), century (unused: from year) */
    uint32_t tick_gen;
    uint32_t wd_gen;
    uint32_t pulse_gen;
    uint32_t periodic_gen[16];
    int dirty;              /* time registers written while latched */
    uint32_t prev_latch, prev_write;
    int wd_fired;
    int pulsing;
    LogicValue kick_level;
} RtcState;

static const RtcClock *clk(const CvState *s) {
    return s->dev->clock;
}

static uint32_t mask_of(unsigned bits) {
    return bits >= 32 ? 0xFFFFFFFFu : ((1u << bits) - 1u);
}

static void put(CvState *s, const CvField *f, uint32_t v) {
    uint32_t m;

    if (f->reg < 0) {
        return;
    }
    m = mask_of(f->hi - f->lo + 1) << f->lo;
    s->regs[f->reg] = (s->regs[f->reg] & ~m) | ((v << f->lo) & m);
}

static int is_leap(int year) {
    return year % 4 == 0;
}

static int days_in(int month, int year) {
    static const int D[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    return month == 2 && is_leap(year) ? 29 : D[(month - 1 + 12) % 12];
}

static int binary(const CvState *s, const RtcTimeField *f) {
    if (f->format.has_field) {
        return cv_field_get(s, &f->format.field) == f->format.binary_value;
    }
    return f->format.fixed_binary;
}

static uint32_t to_bcd(int v) {
    uint32_t out = 0, shift = 0;

    if (v < 0) {
        v = 0;
    }
    do {
        out |= (uint32_t)(v % 10) << shift;
        v /= 10;
        shift += 4;
    } while (v > 0 && shift < 32);
    return out;
}

static int from_bcd(uint32_t v) {
    int out = 0, mult = 1;

    while (v) {
        out += (int)(v & 0xFu) * mult;
        v >>= 4;
        mult *= 10;
    }
    return out;
}

static int twelve_hour(const CvState *s, const RtcTimeField *f) {
    return f->has_h12 && (int)cv_field_get(s, &f->h12) == f->h12_level;
}

/* The value a field holds for the current time (before BCD/binary coding). */
static int unit_value(const CvState *s, const RtcState *r, const RtcTimeField *f) {
    unsigned width = f->field.hi - f->field.lo + 1;

    switch (f->unit) {
    case RU_HOUR:
        if (twelve_hour(s, f)) {
            int h = r->t[RU_HOUR] % 12;

            return h == 0 ? 12 : h;
        }
        return r->t[RU_HOUR];
    case RU_WEEKDAY:
        return r->t[RU_WEEKDAY] + f->first;
    case RU_YEAR:
        return ((r->t[RU_YEAR] - f->base) % 100 + 100) % 100;
    case RU_CENTURY:
        return width == 1 ? ((r->t[RU_YEAR] - f->base) / 100) & 1 : r->t[RU_YEAR] / 100;
    default:
        return r->t[f->unit];
    }
}

static uint32_t encoded(const CvState *s, const RtcState *r, const RtcTimeField *f) {
    int v = unit_value(s, r, f);

    return binary(s, f) ? (uint32_t)v : to_bcd(v);
}

/* Writes the counters into the time registers (unless the registers are frozen). */
static void publish(CvState *s, RtcState *r) {
    const RtcClock *c = clk(s);
    size_t i;

    if (c->update.kind == RTC_UPDATE_LATCH && cv_field_get(s, &c->update.latch)) {
        return;
    }
    for (i = 0; i < c->field_count; i++) {
        const RtcTimeField *f = &c->fields[i];

        put(s, &f->field, encoded(s, r, f));
        if (f->unit == RU_HOUR && f->has_pm && twelve_hour(s, f)) {
            put(s, &f->pm, r->t[RU_HOUR] >= 12);
        }
    }
}

/* Loads the counters from the time registers. */
static void load(CvState *s, RtcState *r) {
    const RtcClock *c = clk(s);
    size_t i;
    int century = -1;

    for (i = 0; i < c->field_count; i++) {
        const RtcTimeField *f = &c->fields[i];
        uint32_t raw = cv_field_get(s, &f->field);
        int v = binary(s, f) ? (int)raw : from_bcd(raw);

        switch (f->unit) {
        case RU_HOUR:
            if (twelve_hour(s, f)) {
                int pm = f->has_pm ? (int)cv_field_get(s, &f->pm) : 0;

                v = v % 12 + (pm ? 12 : 0);
            }
            r->t[RU_HOUR] = v % 24;
            break;
        case RU_WEEKDAY:
            r->t[RU_WEEKDAY] = ((v - f->first) % 7 + 7) % 7;
            break;
        case RU_YEAR:
            r->t[RU_YEAR] = (r->t[RU_YEAR] / 100) * 100 + v % 100;
            if (century < 0) {
                r->t[RU_YEAR] = f->base / 100 * 100 + v % 100;
            }
            break;
        case RU_CENTURY:
            century = v;
            break;
        default:
            r->t[f->unit] = v;
            break;
        }
    }
    if (century >= 0) {
        const RtcTimeField *cf = NULL;

        for (i = 0; i < c->field_count; i++) {
            if (c->fields[i].unit == RU_CENTURY) {
                cf = &c->fields[i];
            }
        }
        if (cf != NULL && cf->field.hi == cf->field.lo) {
            r->t[RU_YEAR] = cf->base + century * 100 + r->t[RU_YEAR] % 100;
        } else {
            r->t[RU_YEAR] = century * 100 + r->t[RU_YEAR] % 100;
        }
    }
    if (r->t[RU_MONTH] < 1 || r->t[RU_MONTH] > 12) {
        r->t[RU_MONTH] = 1;
    }
    if (r->t[RU_DATE] < 1) {
        r->t[RU_DATE] = 1;
    }
}

static int running(const CvState *s) {
    const RtcClock *c = clk(s);

    if (c->has_stop) {
        uint32_t v = cv_field_get(s, &c->stop);

        if (c->stop_running ? v != c->stop_value : v == c->stop_value) {
            return 0;
        }
    }
    if (c->update.kind == RTC_UPDATE_UIP && c->update.has_inhibit &&
        cv_field_get(s, &c->update.inhibit)) {
        return 0;
    }
    if (c->update.kind == RTC_UPDATE_LATCH && c->update.write_halts && c->update.has_write &&
        cv_field_get(s, &c->update.write)) {
        return 0;
    }
    return 1;
}

static void advance(RtcState *r, int hundredths) {
    if (hundredths && ++r->t[RU_HUNDREDTH] < 100) {
        return;
    }
    r->t[RU_HUNDREDTH] = 0;
    if (++r->t[RU_SECOND] < 60) {
        return;
    }
    r->t[RU_SECOND] = 0;
    if (++r->t[RU_MINUTE] < 60) {
        return;
    }
    r->t[RU_MINUTE] = 0;
    if (++r->t[RU_HOUR] < 24) {
        return;
    }
    r->t[RU_HOUR] = 0;
    r->t[RU_WEEKDAY] = (r->t[RU_WEEKDAY] + 1) % 7;
    if (++r->t[RU_DATE] <= days_in(r->t[RU_MONTH], r->t[RU_YEAR])) {
        return;
    }
    r->t[RU_DATE] = 1;
    if (++r->t[RU_MONTH] <= 12) {
        return;
    }
    r->t[RU_MONTH] = 1;
    r->t[RU_YEAR]++;
}

/* ---- Pins ------------------------------------------------------------------------------- */

static int source_on(const CvState *s, int has_flag, const CvField *flag, int has_enable,
                     const CvField *enable) {
    return (!has_flag || cv_field_get(s, flag)) && (!has_enable || cv_field_get(s, enable)) &&
           has_flag;
}

/* The attributes of a pin named by several sources: the first that gives them. */
static RtcPin attrs(const RtcClock *c, int pin) {
    RtcPin out = {pin, 0, 1};
    size_t i;

    for (i = 0; i < c->alarm_count; i++) {
        if (c->alarms[i].pin.pin == pin && c->alarms[i].pin.open_drain >= 0) {
            return c->alarms[i].pin;
        }
    }
    for (i = 0; i < c->periodic_count; i++) {
        if (c->periodic[i].pin.pin == pin && c->periodic[i].pin.open_drain >= 0) {
            return c->periodic[i].pin;
        }
    }
    if (c->watchdog.pin.pin == pin && c->watchdog.pin.open_drain >= 0) {
        return c->watchdog.pin;
    }
    if (c->watchdog.reset_pin.pin == pin && c->watchdog.reset_pin.open_drain >= 0) {
        return c->watchdog.reset_pin;
    }
    return out;
}

static void drive_pin(CvState *s, const RtcClock *c, int pin, int asserted) {
    RtcPin a = attrs(c, pin);
    LogicValue v;

    if (pin == CV_NONE) {
        return;
    }
    if (asserted) {
        v = a.active ? LOGIC_1 : LOGIC_0;
    } else {
        v = a.open_drain ? LOGIC_Z : (a.active ? LOGIC_0 : LOGIC_1);
    }
    if (a.open_drain && v == LOGIC_1) {
        v = LOGIC_Z;
    }
    cv_drive(s, cv_pin(s, pin), v, 0);
}

void rtc_refresh(CvState *s) {
    const RtcClock *c = clk(s);
    RtcState *r = s->rtc;
    int pins[64];
    int on[64];
    size_t n = 0, i, k;

    if (c == NULL || r == NULL) {
        return;
    }
#define ADD(PIN, ON)                                       \
    do {                                                   \
        if ((PIN) != CV_NONE) {                            \
            for (k = 0; k < n && pins[k] != (PIN); k++) {  \
            }                                              \
            if (k == n && n < 64) {                        \
                pins[n] = (PIN);                           \
                on[n++] = 0;                               \
            }                                              \
            if (k < 64) {                                  \
                on[k] |= (ON);                             \
            }                                              \
        }                                                  \
    } while (0)
    for (i = 0; i < c->alarm_count; i++) {
        const RtcAlarm *a = &c->alarms[i];

        ADD(a->pin.pin, source_on(s, a->has_flag, &a->flag, a->has_enable, &a->enable));
    }
    for (i = 0; i < c->periodic_count; i++) {
        const RtcPeriodic *p = &c->periodic[i];

        ADD(p->pin.pin, source_on(s, p->has_flag, &p->flag, p->has_enable, &p->enable));
    }
    if (c->watchdog.present) {
        ADD(c->watchdog.pin.pin, r->wd_fired);
        ADD(c->watchdog.reset_pin.pin, r->pulsing);
    }
#undef ADD
    for (i = 0; i < n; i++) {
        drive_pin(s, c, pins[i], on[i]);
    }
}

/* ---- Timers ------------------------------------------------------------------------------- */

static uint32_t tid(unsigned kind, unsigned index, uint32_t gen) {
    return CV_TM_RTC | ((uint32_t)kind << 24) | ((uint32_t)(index & 0xFFu) << 16) | (gen & 0xFFFFu);
}

static SimTime tick_ps(const CvState *s) {
    return clk(s)->hundredths ? 10000000000ull : 1000000000000ull;
}

static void schedule_tick(CvState *s, RtcState *r) {
    const RtcClock *c = clk(s);

    r->tick_gen++;
    sim_schedule_timer(s->sim, s->inst, tid(RT_TICK, 0, r->tick_gen), tick_ps(s));
    if (c->update.kind == RTC_UPDATE_UIP && !c->hundredths) {
        sim_schedule_timer(s->sim, s->inst, tid(RT_UIP, 0, r->tick_gen),
                           tick_ps(s) - RT_UIP_PS);
    }
}

static int lookup(const uint32_t *keys, const uint64_t *ps, size_t count, uint32_t key,
                  uint64_t *out) {
    size_t i;

    for (i = 0; i < count; i++) {
        if (keys[i] == key) {
            *out = ps[i];
            return 1;
        }
    }
    return 0;
}

static void schedule_periodic(CvState *s, RtcState *r, size_t i) {
    const RtcPeriodic *p = &clk(s)->periodic[i];
    uint64_t period = p->period_ps;

    r->periodic_gen[i & 15]++;
    if (!p->fixed && !lookup(p->keys, p->ps, p->count, cv_field_get(s, &p->rate), &period)) {
        return;
    }
    if (period > 0) {
        sim_schedule_timer(s->sim, s->inst, tid(RT_PERIODIC, (unsigned)i, r->periodic_gen[i & 15]),
                           period);
    }
}

static void schedule_watchdog(CvState *s, RtcState *r) {
    const RtcWatchdog *w = &clk(s)->watchdog;
    uint64_t period;

    r->wd_gen++;
    if (!w->present || !lookup(w->keys, w->ps, w->count, cv_field_get(s, &w->field), &period) ||
        period == 0) {
        return;
    }
    sim_schedule_timer(s->sim, s->inst, tid(RT_WATCHDOG, 0, r->wd_gen), period);
}

static void check_alarms(CvState *s, RtcState *r) {
    const RtcClock *c = clk(s);
    size_t i, j, k;

    for (i = 0; i < c->alarm_count; i++) {
        const RtcAlarm *a = &c->alarms[i];
        int hit = 1;

        for (j = 0; j < a->match_count && hit; j++) {
            const RtcMatch *m = &a->match[j];
            uint32_t alarm = cv_field_get(s, &m->field);
            const RtcTimeField *tf = NULL;

            if (m->has_mask && (int)cv_field_get(s, &m->mask) == m->mask_level) {
                continue;
            }
            if (m->has_dont_care && (alarm & m->dc_mask) == m->dc_value) {
                continue;
            }
            for (k = 0; k < c->field_count; k++) {
                if (c->fields[k].unit == m->unit) {
                    tf = &c->fields[k];
                    break;
                }
            }
            if (tf == NULL) {
                hit = 0;
                break;
            }
            {
                uint32_t want = encoded(s, r, tf);
                unsigned w = tf->field.hi - tf->field.lo + 1;

                if (m->unit == RU_HOUR && tf->has_pm && twelve_hour(s, tf) &&
                    tf->pm.reg == tf->field.reg && tf->pm.lo > tf->field.hi) {
                    /* the alarm byte carries the PM bit beside the hour */
                    want |= (uint32_t)(r->t[RU_HOUR] >= 12) << (tf->pm.lo - tf->field.lo);
                    w = tf->pm.lo - tf->field.lo + 1;
                }
                hit = (alarm & mask_of(w)) == (want & mask_of(w));
            }
        }
        if (hit && a->has_flag) {
            put(s, &a->flag, 1);
        }
    }
}

void rtc_timer(CvState *s, uint32_t id) {
    RtcState *r = s->rtc;
    const RtcClock *c = clk(s);
    unsigned kind = (id >> 24) & 0xFu, index = (id >> 16) & 0xFFu;
    uint32_t gen = id & 0xFFFFu;

    if (r == NULL) {
        return;
    }
    if (kind == RT_TICK) {
        int second_changed;

        if (gen != (r->tick_gen & 0xFFFFu)) {
            return;
        }
        second_changed = !c->hundredths || r->t[RU_HUNDREDTH] == 99;
        if (running(s)) {
            advance(r, c->hundredths);
            publish(s, r);
            if (second_changed) {
                check_alarms(s, r);
            }
        }
        if (c->update.kind == RTC_UPDATE_UIP) {
            put(s, &c->update.uip, 0);
        }
        schedule_tick(s, r);
    } else if (kind == RT_UIP) {
        if (gen == (r->tick_gen & 0xFFFFu) && running(s)) {
            put(s, &c->update.uip, 1);
        }
    } else if (kind == RT_PERIODIC) {
        if (index < c->periodic_count && gen == (r->periodic_gen[index & 15] & 0xFFFFu)) {
            const RtcPeriodic *p = &c->periodic[index];

            if (p->has_flag) {
                put(s, &p->flag, 1);
            }
            if (p->has_summary) {
                put(s, &p->summary, 1);
            }
            schedule_periodic(s, r, index);
        }
    } else if (kind == RT_WATCHDOG) {
        const RtcWatchdog *w = &c->watchdog;
        uint64_t pulse;

        if (gen != (r->wd_gen & 0xFFFFu)) {
            return;
        }
        r->wd_fired = 1;
        if (w->has_flag) {
            put(s, &w->flag, 1);
        }
        if (w->reset_pin.pin != CV_NONE &&
            lookup(w->pulse_keys, w->pulse_ps, w->pulse_count, cv_field_get(s, &w->field), &pulse)) {
            r->pulsing = 1;
            r->pulse_gen++;
            sim_schedule_timer(s->sim, s->inst, tid(RT_PULSE, 0, r->pulse_gen), pulse);
        }
    } else if (kind == RT_PULSE) {
        if (gen == (r->pulse_gen & 0xFFFFu)) {
            r->pulsing = 0;
            schedule_watchdog(s, r);
        }
    }
    cv_ready_refresh(s);
}

/* ---- Hooks from the engine ----------------------------------------------------------------- */

static int holds_time(const RtcClock *c, int reg) {
    size_t i;

    for (i = 0; i < c->field_count; i++) {
        if (c->fields[i].field.reg == reg) {
            return 1;
        }
    }
    return 0;
}

void rtc_after_write(CvState *s, int reg) {
    RtcState *r = s->rtc;
    const RtcClock *c = clk(s);
    size_t i;

    if (r == NULL) {
        return;
    }
    if (c->update.kind == RTC_UPDATE_LATCH) {
        uint32_t latch = cv_field_get(s, &c->update.latch);
        uint32_t write = c->update.has_write ? cv_field_get(s, &c->update.write) : latch;

        if (holds_time(c, reg)) {
            if (latch || write) {
                r->dirty = 1;
            } else {
                load(s, r);
                schedule_tick(s, r);
            }
        }
        if ((r->prev_write && !write) || (r->prev_latch && !latch)) {
            if (r->dirty) {
                load(s, r);
                r->dirty = 0;
                schedule_tick(s, r);
            }
            publish(s, r);
        }
        r->prev_latch = latch;
        r->prev_write = write;
    } else if (holds_time(c, reg)) {
        load(s, r);
        schedule_tick(s, r);
    }
    for (i = 0; i < c->periodic_count; i++) {
        if (!c->periodic[i].fixed && c->periodic[i].rate.reg == reg) {
            schedule_periodic(s, r, i);
        }
    }
    if (c->watchdog.present && c->watchdog.field.reg == reg) {
        r->wd_fired = 0;
        schedule_watchdog(s, r);
    }
}

void rtc_pin(CvState *s, size_t pin) {
    RtcState *r = s->rtc;
    const RtcWatchdog *w;
    LogicValue v;

    if (r == NULL) {
        return;
    }
    w = &clk(s)->watchdog;
    if (!w->present || !w->has_kick || cv_pin(s, w->kick_pin) != pin) {
        return;
    }
    v = cv_level(s, pin, LOGIC_X);
    if (v != r->kick_level && v != LOGIC_X &&
        (w->kick_edge == 2 || (w->kick_edge == 1) == (v == LOGIC_1))) {
        r->wd_fired = 0;
        schedule_watchdog(s, r);
        cv_ready_refresh(s);
    }
    r->kick_level = v;
}

void rtc_start(CvState *s) {
    RtcState *r = calloc(1, sizeof *r);
    const RtcClock *c = clk(s);
    size_t i;

    if (r == NULL) {
        return;
    }
    s->rtc = r;
    r->t[RU_SECOND] = 0;
    r->t[RU_MINUTE] = 0;
    r->t[RU_HOUR] = 0;
    r->t[RU_WEEKDAY] = 6;           /* 2000-01-01 is a Saturday */
    r->t[RU_DATE] = 1;
    r->t[RU_MONTH] = 1;
    r->t[RU_YEAR] = 2000;
    if (c->update.kind == RTC_UPDATE_LATCH) {
        r->prev_latch = cv_field_get(s, &c->update.latch);
        r->prev_write = c->update.has_write ? cv_field_get(s, &c->update.write) : r->prev_latch;
    }
    publish(s, r);
    schedule_tick(s, r);
    for (i = 0; i < c->periodic_count; i++) {
        schedule_periodic(s, r, i);
    }
    schedule_watchdog(s, r);
    if (c->watchdog.present && c->watchdog.has_kick) {
        r->kick_level = cv_level(s, cv_pin(s, c->watchdog.kick_pin), LOGIC_X);
    }
}

void rtc_free(CvState *s) {
    free(s->rtc);
    s->rtc = NULL;
}

int rtc_snapshot(const CvState *s, RtcTime *out) {
    const RtcState *r = s->rtc;

    if (r == NULL || s->dev->clock == NULL) {
        return 0;
    }
    out->year = r->t[RU_YEAR];
    out->month = r->t[RU_MONTH];
    out->day = r->t[RU_DATE];
    out->weekday = r->t[RU_WEEKDAY];
    out->hour = r->t[RU_HOUR];
    out->minute = r->t[RU_MINUTE];
    out->second = r->t[RU_SECOND];
    out->hundredth = r->t[RU_HUNDREDTH];
    out->hundredths = s->dev->clock->hundredths;
    out->running = running(s);
    return 1;
}

/* ---- The kind ------------------------------------------------------------------------------- */

static const char *const DEVICE_KEYS[] = {
    "vcc", "inputMax", "analog", "bus", "configBus", "timing", "registers", "protocol", "ready",
    "pins", "clock", NULL
};

static const char *const PART_KEYS[] = {"vcc", "inputMax", "i2cAddress", "i2c", "resets", NULL};

static void *rtc_load_device(KindContext *ctx, const cJSON *json) {
    return cv_load_device(ctx, json, 1);
}

static int rtc_pin_open_drain(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;
    const CvDevice *d = b->dev;
    const RtcClock *c = d->clock;
    size_t i;

    if (pin < b->pin_count && b->open_drain[pin]) {
        return 1;
    }
    for (i = 0; i < d->name_count; i++) {
        if (b->pin[i] == pin) {
            RtcPin a = attrs(c, (int)i);

            if (a.open_drain > 0 && attrs(c, (int)i).pin == (int)i) {
                size_t k;

                for (k = 0; k < c->alarm_count; k++) {
                    if (c->alarms[k].pin.pin == (int)i) {
                        return 1;
                    }
                }
                for (k = 0; k < c->periodic_count; k++) {
                    if (c->periodic[k].pin.pin == (int)i) {
                        return 1;
                    }
                }
                if (c->watchdog.reset_pin.pin == (int)i || c->watchdog.pin.pin == (int)i) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int rtc_pin_analog(const Part *part, size_t pin) {
    const CvBound *b = part_internal(part)->bound;

    return pin < b->pin_count && b->analog[pin];
}

const PartKindOps RTC_KIND = {
    PART_RTC, "rtc", &CONVERTER_COMPONENT, DEVICE_KEYS, PART_KEYS,
    rtc_load_device, cv_bind_pinout, cv_load_part, rtc_pin_open_drain, rtc_pin_analog
};
