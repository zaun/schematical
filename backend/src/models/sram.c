/*
 * sram: asynchronous SRAMs (ported from backend/sram.c; the engine section
 * below, "Storage" through the timer, is the old code unchanged).
 *
 * Memory: every instance has its own storage, allocated in pages as they are
 * first written. Each bit is 0, 1 or X; the chip powers up all X.
 *
 * Control (datasheet truth tables): selected while every chipEnable (active
 * low) is low and every chipEnableHigh is high. It writes while selected
 * with WE# low and the lane's byte enable low; data and address are taken at
 * the end of the write. It reads while selected with OE# low, WE# high and
 * the byte enable low. An undriven (Z) control reads inactive; X unknown.
 *
 * Read timing follows the datasheet window (tCLZ/tOLZ/tBLZ/tOW to leave Z,
 * tCHZ/tOHZ/tBHZ/tWHZ to return to Z, data valid at the latest of tAA, tACE,
 * tOE, tBA; old data held for tOH after an address change). Write timing is
 * checked at the end of each write (tWP or tWP with OE# low, tCW, tBW, tAW,
 * tAS, tDW) and after it (tDH, tWR); a violation stores X.
 *
 * Kind keys name the pins (address[i] = bit i, data[i] = bit i,
 * byteEnable[i] = lane i); grades hold the 24 timing values in ns.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define SRAM_NO_PIN ((size_t)-1)
#define SRAM_MAX_ADDRESS_BITS 24
#define SRAM_MAX_DATA_BITS 32
#define SRAM_MAX_LANES 4
#define SRAM_MAX_SELECTS 2
#define SRAM_PAGE_BITS 12
#define SRAM_PAGE_WORDS (1u << SRAM_PAGE_BITS)
#define SRAM_NEVER UINT64_MAX
#define SRAM_COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* One speed grade. Times in picoseconds; 0 where the datasheet gives none. */
typedef struct {
    /* read */
    SimTime trc, taa, tace, toe, tba, toh;
    SimTime tclz, tolz, tblz, tchz, tohz, tbhz;
    /* write */
    SimTime twc, taw, tcw, tbw, tas, twp, twp_oe_low, twr, tdw, tdh, twhz, tow;
} SramTiming;

/* ---- Instance state ------------------------------------------------------ */

typedef struct {
    uint32_t value[SRAM_PAGE_WORDS];
    uint32_t known[SRAM_PAGE_WORDS];
} SramPage;

typedef enum {
    OUT_Z,
    OUT_X,
    OUT_DATA,
    OUT_HOLD
} OutKind;

/* An input control: 1 active, 0 inactive, -1 unknown. */
typedef struct {
    int level;
    SimTime active_since;   /* last move to active */
    SimTime inactive_since; /* last move to inactive */
    SimTime changed;
} Control;

typedef struct {
    const SramTiming *t;

    unsigned address_bits;
    unsigned data_bits;
    unsigned lane_count;
    uint32_t word_count;
    uint32_t lane_mask[SRAM_MAX_LANES];
    unsigned lane_of_bit[SRAM_MAX_DATA_BITS];

    size_t addr_pin[SRAM_MAX_ADDRESS_BITS];
    size_t dq_pin[SRAM_MAX_DATA_BITS];
    size_t ce_n_pin[SRAM_MAX_SELECTS];
    size_t ce_pin[SRAM_MAX_SELECTS];
    unsigned ce_n_count;
    unsigned ce_count;
    size_t oe_pin;
    size_t we_pin;
    size_t be_pin[SRAM_MAX_LANES];
    unsigned be_count;

    /* Storage: pages of SRAM_PAGE_WORDS, NULL until first written (all X). */
    SramPage **pages;
    size_t page_count;

    /* Inputs as last seen, with the value before the latest change time. */
    LogicValue addr[SRAM_MAX_ADDRESS_BITS];
    LogicValue addr_prev[SRAM_MAX_ADDRESS_BITS];
    SimTime addr_time;
    SimTime addr_prev_time;
    LogicValue dq[SRAM_MAX_DATA_BITS];
    LogicValue dq_prev[SRAM_MAX_DATA_BITS];
    SimTime dq_time[SRAM_MAX_DATA_BITS];
    SimTime dq_prev_time[SRAM_MAX_DATA_BITS];

    Control sel;
    Control oe;
    Control we;
    Control be[SRAM_MAX_LANES];

    /* Output side, per byte lane. */
    int out_enable[SRAM_MAX_LANES];      /* 1, 0 or -1, as last evaluated */
    SimTime off_time[SRAM_MAX_LANES];
    SimTime off_hz[SRAM_MAX_LANES];
    int off_from_z[SRAM_MAX_LANES];
    OutKind out_kind[SRAM_MAX_LANES];
    SimTime hold_until[SRAM_MAX_LANES];
    LogicValue hold[SRAM_MAX_DATA_BITS];
    LogicValue driven[SRAM_MAX_DATA_BITS];

    /* Write side, per byte lane. */
    int writing[SRAM_MAX_LANES];         /* 1, 0 or -1, as last evaluated */
    SimTime write_start[SRAM_MAX_LANES];
    int write_bad[SRAM_MAX_LANES];
    int write_oe_low[SRAM_MAX_LANES];
    int last_write_valid[SRAM_MAX_LANES];
    SimTime last_write_end[SRAM_MAX_LANES];
    uint32_t last_write_addr[SRAM_MAX_LANES];

    SimTime wake;
} SramState;

/* ---- Storage ------------------------------------------------------------- */

static void store(SramState *state, uint32_t address, uint32_t mask, uint32_t value, uint32_t known) {
    size_t page_index = address >> SRAM_PAGE_BITS;
    size_t offset = address & (SRAM_PAGE_WORDS - 1);
    SramPage *page;

    if (address >= state->word_count) {
        return;
    }
    page = state->pages[page_index];
    if (page == NULL) {
        if ((known & mask) == 0) {
            return; /* storing X in a page that is still all X */
        }
        page = calloc(1, sizeof(*page));
        if (page == NULL) {
            return; /* out of memory: the word stays X */
        }
        state->pages[page_index] = page;
    }
    page->value[offset] = (page->value[offset] & ~mask) | (value & known & mask);
    page->known[offset] = (page->known[offset] & ~mask) | (known & mask);
}

static void fetch(const SramState *state, uint32_t address, uint32_t *value, uint32_t *known) {
    const SramPage *page;

    *value = 0;
    *known = 0;
    if (address >= state->word_count) {
        return;
    }
    page = state->pages[address >> SRAM_PAGE_BITS];
    if (page != NULL) {
        *value = page->value[address & (SRAM_PAGE_WORDS - 1)];
        *known = page->known[address & (SRAM_PAGE_WORDS - 1)];
    }
}

/*
 * Makes the masked bits X at every address that matches the known address
 * bits. With every address bit known this is one word.
 */
static void corrupt(SramState *state, uint32_t address, uint32_t address_known, uint32_t mask) {
    uint32_t full = state->word_count - 1;
    size_t p;

    if ((address_known & full) == full) {
        store(state, address & full, mask, 0, 0);
        return;
    }
    for (p = 0; p < state->page_count; p++) {
        SramPage *page = state->pages[p];
        size_t offset;

        if (page == NULL) {
            continue;
        }
        for (offset = 0; offset < SRAM_PAGE_WORDS; offset++) {
            uint32_t candidate = (uint32_t)((p << SRAM_PAGE_BITS) | offset);

            if (((candidate ^ address) & address_known & full) == 0) {
                page->known[offset] &= ~mask;
            }
        }
    }
}

/* ---- Inputs -------------------------------------------------------------- */

static int level_of(LogicValue value, int active_high) {
    if (value == LOGIC_Z) {
        return 0; /* undriven control: inactive */
    }
    if (value == LOGIC_X) {
        return -1;
    }
    return (value == LOGIC_1) == (active_high != 0);
}

/* Three-valued AND of control levels. */
static int all_of(const int *levels, size_t count) {
    int result = 1;
    size_t i;

    for (i = 0; i < count; i++) {
        if (levels[i] == 0) {
            return 0;
        }
        if (levels[i] < 0) {
            result = -1;
        }
    }
    return result;
}

static int not_level(int level) {
    return level < 0 ? -1 : !level;
}

static void update_control(Control *control, int level, SimTime now) {
    if (control->level == level) {
        return;
    }
    if (level == 1) {
        control->active_since = now;
    } else if (level == 0) {
        control->inactive_since = now;
    }
    control->level = level;
    control->changed = now;
}

static int read_select(Sim *sim, SimInstance *instance, const SramState *state) {
    int levels[2 * SRAM_MAX_SELECTS];
    size_t count = 0;
    size_t i;

    for (i = 0; i < state->ce_n_count; i++) {
        levels[count++] = level_of(sim_read_pin(sim, instance, state->ce_n_pin[i]), 0);
    }
    for (i = 0; i < state->ce_count; i++) {
        levels[count++] = level_of(sim_read_pin(sim, instance, state->ce_pin[i]), 1);
    }
    return all_of(levels, count);
}

static int read_low_active(Sim *sim, SimInstance *instance, size_t pin, int missing) {
    if (pin == SRAM_NO_PIN) {
        return missing;
    }
    return level_of(sim_read_pin(sim, instance, pin), 0);
}

/* Reads every input. Returns 1 when any address bit changed. */
static int sync_inputs(Sim *sim, SimInstance *instance, SramState *state) {
    SimTime now = sim_now(sim);
    int address_changed = 0;
    size_t i;

    for (i = 0; i < state->address_bits; i++) {
        LogicValue value = state->addr_pin[i] == SRAM_NO_PIN
            ? LOGIC_X
            : sim_read_pin(sim, instance, state->addr_pin[i]);

        if (value != state->addr[i]) {
            if (!address_changed && state->addr_time != now) {
                memcpy(state->addr_prev, state->addr, sizeof(state->addr));
                state->addr_prev_time = state->addr_time;
            }
            address_changed = 1;
            state->addr[i] = value;
        }
    }
    if (address_changed) {
        state->addr_time = now;
    }

    for (i = 0; i < state->data_bits; i++) {
        LogicValue value = state->dq_pin[i] == SRAM_NO_PIN
            ? LOGIC_X
            : sim_read_pin(sim, instance, state->dq_pin[i]);

        if (value != state->dq[i]) {
            if (state->dq_time[i] != now) {
                state->dq_prev[i] = state->dq[i];
                state->dq_prev_time[i] = state->dq_time[i];
            }
            state->dq[i] = value;
            state->dq_time[i] = now;
        }
    }

    update_control(&state->sel, read_select(sim, instance, state), now);
    update_control(&state->oe, read_low_active(sim, instance, state->oe_pin, 1), now);
    update_control(&state->we, read_low_active(sim, instance, state->we_pin, 0), now);
    for (i = 0; i < state->lane_count; i++) {
        int level = state->be_count > 0
            ? read_low_active(sim, instance, state->be_pin[i], 0)
            : 1;
        update_control(&state->be[i], level, now);
    }
    return address_changed;
}

/* An address from pin values: the bits, and a mask of the bits that are known. */
static void address_of(const SramState *state, const LogicValue *bits, uint32_t *value, uint32_t *known) {
    size_t i;

    *value = 0;
    *known = 0;
    for (i = 0; i < state->address_bits; i++) {
        if (bits[i] == LOGIC_0 || bits[i] == LOGIC_1) {
            *known |= (uint32_t)1 << i;
            if (bits[i] == LOGIC_1) {
                *value |= (uint32_t)1 << i;
            }
        }
    }
}

static int address_known(const SramState *state, uint32_t known) {
    return known == state->word_count - 1;
}

/* ---- Writes -------------------------------------------------------------- */

static int lane_write_level(const SramState *state, size_t lane) {
    int levels[3];

    levels[0] = state->sel.level;
    levels[1] = state->we.level;
    levels[2] = state->be[lane].level;
    return all_of(levels, 3);
}

static int too_short(SimTime now, SimTime since, SimTime minimum) {
    return minimum > 0 && now - since < minimum;
}

static void write_begin(SramState *state, size_t lane, SimTime now) {
    state->write_start[lane] = now;
    state->write_bad[lane] = too_short(now, state->addr_time, state->t->tas);
    state->write_oe_low[lane] = state->oe.level != 0;
    state->last_write_valid[lane] = 0;
}

/* The write ended now: check its timing and store the lane. */
static void write_end(SramState *state, size_t lane, SimTime now) {
    const SramTiming *t = state->t;
    const LogicValue *address_bits = state->addr_time == now ? state->addr_prev : state->addr;
    SimTime address_stable = state->addr_time == now ? state->addr_prev_time : state->addr_time;
    SimTime pulse = state->write_oe_low[lane] && t->twp_oe_low > 0 ? t->twp_oe_low : t->twp;
    uint32_t mask = state->lane_mask[lane];
    uint32_t address;
    uint32_t known_address;
    uint32_t value = 0;
    uint32_t known = 0;
    int bad = state->write_bad[lane];
    size_t i;

    bad |= too_short(now, state->we.active_since, pulse);
    bad |= too_short(now, state->sel.active_since, t->tcw);
    if (state->be_count > 0) {
        bad |= too_short(now, state->be[lane].active_since, t->tbw);
    }
    bad |= too_short(now, address_stable, t->taw);

    address_of(state, address_bits, &address, &known_address);
    if (!address_known(state, known_address)) {
        corrupt(state, address, known_address, mask);
        return;
    }

    for (i = 0; i < state->data_bits; i++) {
        uint32_t bit = (uint32_t)1 << i;
        LogicValue v = state->dq_time[i] == now ? state->dq_prev[i] : state->dq[i];
        SimTime stable = state->dq_time[i] == now ? state->dq_prev_time[i] : state->dq_time[i];

        if (!(mask & bit) || too_short(now, stable, t->tdw)) {
            continue;
        }
        if (v == LOGIC_0 || v == LOGIC_1) {
            known |= bit;
            if (v == LOGIC_1) {
                value |= bit;
            }
        }
    }
    if (bad) {
        known = 0;
    }
    store(state, address, mask, value, known);

    state->last_write_valid[lane] = 1;
    state->last_write_end[lane] = now;
    state->last_write_addr[lane] = address;
}

static void evaluate_writes(SramState *state, int address_changed, SimTime now) {
    uint32_t address;
    uint32_t known_address;
    uint32_t prev_address;
    uint32_t prev_known;
    size_t lane;
    size_t i;

    address_of(state, state->addr, &address, &known_address);
    address_of(state, state->addr_prev, &prev_address, &prev_known);

    for (lane = 0; lane < state->lane_count; lane++) {
        uint32_t mask = state->lane_mask[lane];
        int level = lane_write_level(state, lane);
        int was = state->writing[lane];

        /* Hold checks for the write that ended last. */
        if (state->last_write_valid[lane]) {
            SimTime end = state->last_write_end[lane];
            uint32_t written = state->last_write_addr[lane];

            if (address_changed && state->t->twr > 0 && now < end + state->t->twr) {
                corrupt(state, written, state->word_count - 1, mask);
                corrupt(state, address, known_address, mask);
            }
            if (state->t->tdh > 0 && now < end + state->t->tdh) {
                for (i = 0; i < state->data_bits; i++) {
                    if ((mask >> i & 1u) && state->dq_time[i] == now) {
                        corrupt(state, written, state->word_count - 1, (uint32_t)1 << i);
                    }
                }
            }
        }

        if (was == 1 && state->oe.level != 0) {
            state->write_oe_low[lane] = 1;
        }

        if (was == 1 && level == 1) {
            /* Address moved inside the write: both locations are undefined. */
            if (address_changed && now != state->write_start[lane]) {
                corrupt(state, prev_address, prev_known, mask);
                corrupt(state, address, known_address, mask);
                state->write_bad[lane] = 1;
            }
        } else if (was != 1 && level == 1) {
            write_begin(state, lane, now);
        } else if (was == 1 && level == 0) {
            write_end(state, lane, now);
        } else if (was == 1 && level < 0) {
            corrupt(state, prev_address, prev_known, mask);
            corrupt(state, address, known_address, mask);
        }

        /* Unknown write enable: whatever it may have hit is undefined. */
        if (level < 0) {
            corrupt(state, address, known_address, mask);
            state->last_write_valid[lane] = 0;
        }
        state->writing[lane] = level;
    }
}

/* ---- Reads --------------------------------------------------------------- */

static int lane_output_level(const SramState *state, size_t lane) {
    int levels[4];

    levels[0] = state->sel.level;
    levels[1] = state->oe.level;
    levels[2] = not_level(state->we.level);
    levels[3] = state->be[lane].level;
    return all_of(levels, 4);
}

static SimTime max_time(SimTime a, SimTime b) {
    return a > b ? a : b;
}

/* The high-Z time for whichever enable turned the outputs off just now. */
static SimTime turn_off_time(const SramState *state, size_t lane, SimTime now) {
    const SramTiming *t = state->t;
    SimTime hz = 0;

    if (state->sel.changed == now && state->sel.level != 1) {
        hz = max_time(hz, t->tchz);
    }
    if (state->oe.changed == now && state->oe.level != 1) {
        hz = max_time(hz, t->tohz);
    }
    if (state->we.changed == now && state->we.level != 0) {
        hz = max_time(hz, t->twhz);
    }
    if (state->be_count > 0 && state->be[lane].changed == now && state->be[lane].level != 1) {
        hz = max_time(hz, t->tbhz);
    }
    return hz;
}

static void evaluate_outputs(SramState *state, int address_changed, SimTime now) {
    size_t lane;
    size_t i;

    for (lane = 0; lane < state->lane_count; lane++) {
        int level = lane_output_level(state, lane);

        if (address_changed && state->out_kind[lane] == OUT_DATA) {
            state->hold_until[lane] = now + state->t->toh;
            for (i = 0; i < state->data_bits; i++) {
                state->hold[i] = state->driven[i];
            }
        }
        if (level == state->out_enable[lane]) {
            continue;
        }
        if (level == 0) {
            state->off_time[lane] = now;
            state->off_hz[lane] = turn_off_time(state, lane, now);
            state->off_from_z[lane] = state->out_kind[lane] == OUT_Z;
        }
        state->out_enable[lane] = level;
    }
}

/* What a lane drives now, and the next time that changes (SRAM_NEVER if not). */
static OutKind lane_output(const SramState *state, size_t lane, SimTime now, SimTime *next) {
    const SramTiming *t = state->t;
    SimTime low_z;
    SimTime valid;

    *next = SRAM_NEVER;
    switch (state->out_enable[lane]) {
        case 0: {
            SimTime high_z = state->off_time[lane] + state->off_hz[lane];

            if (!state->off_from_z[lane] && now < high_z) {
                *next = high_z;
                return OUT_X;
            }
            return OUT_Z;
        }
        case 1:
            break;
        default:
            return OUT_X;
    }

    low_z = max_time(state->sel.active_since + t->tclz, state->oe.active_since + t->tolz);
    low_z = max_time(low_z, state->we.inactive_since + t->tow);
    valid = max_time(state->addr_time + t->taa, state->sel.active_since + t->tace);
    valid = max_time(valid, state->oe.active_since + t->toe);
    if (state->be_count > 0) {
        low_z = max_time(low_z, state->be[lane].active_since + t->tblz);
        valid = max_time(valid, state->be[lane].active_since + t->tba);
    }
    valid = max_time(valid, low_z);

    if (now < low_z) {
        *next = low_z;
        return OUT_Z;
    }
    if (now >= valid) {
        return OUT_DATA;
    }
    if (now < state->hold_until[lane]) {
        *next = state->hold_until[lane] < valid ? state->hold_until[lane] : valid;
        return OUT_HOLD;
    }
    *next = valid;
    return OUT_X;
}

static void drive(Sim *sim, SimInstance *instance, SramState *state, size_t bit, LogicValue value) {
    if (state->dq_pin[bit] == SRAM_NO_PIN || state->driven[bit] == value) {
        return;
    }
    state->driven[bit] = value;
    sim_schedule_inertial_drive(sim, instance, state->dq_pin[bit], value, DRIVE_STRONG, 0);
}

static void refresh_outputs(Sim *sim, SimInstance *instance, SramState *state) {
    SimTime now = sim_now(sim);
    SimTime wake = SRAM_NEVER;
    uint32_t address;
    uint32_t known_address;
    uint32_t word = 0;
    uint32_t word_known = 0;
    size_t lane;
    size_t i;

    address_of(state, state->addr, &address, &known_address);
    if (address_known(state, known_address)) {
        fetch(state, address, &word, &word_known);
    }

    for (lane = 0; lane < state->lane_count; lane++) {
        SimTime next;
        OutKind kind = lane_output(state, lane, now, &next);

        state->out_kind[lane] = kind;
        if (next < wake) {
            wake = next;
        }
        for (i = 0; i < state->data_bits; i++) {
            LogicValue value;

            if (state->lane_of_bit[i] != lane) {
                continue;
            }
            switch (kind) {
                case OUT_Z:
                    value = LOGIC_Z;
                    break;
                case OUT_DATA:
                    value = !(word_known >> i & 1u) ? LOGIC_X
                          : (word >> i & 1u) ? LOGIC_1 : LOGIC_0;
                    break;
                case OUT_HOLD:
                    value = state->hold[i];
                    break;
                default:
                    value = LOGIC_X;
                    break;
            }
            drive(sim, instance, state, i, value);
        }
    }

    /* Earlier wake-ups still pending just refresh again, which is harmless. */
    if (wake != SRAM_NEVER && (state->wake <= now || wake < state->wake)) {
        state->wake = wake;
        sim_schedule_timer(sim, instance, 0, wake - now);
    }
}

/* ---- Model callbacks ----------------------------------------------------- */

static void update(Sim *sim, SimInstance *instance, SramState *state) {
    SimTime now = sim_now(sim);
    int address_changed = sync_inputs(sim, instance, state);

    evaluate_writes(state, address_changed, now);
    evaluate_outputs(state, address_changed, now);
    refresh_outputs(sim, instance, state);
}

static void sram_start(Sim *sim, SimInstance *instance, void *raw) {
    SramState *state = raw;
    size_t i;

    for (i = 0; i < SRAM_MAX_ADDRESS_BITS; i++) {
        state->addr[i] = LOGIC_Z;
        state->addr_prev[i] = LOGIC_Z;
    }
    for (i = 0; i < SRAM_MAX_DATA_BITS; i++) {
        state->dq[i] = LOGIC_Z;
        state->dq_prev[i] = LOGIC_Z;
        state->driven[i] = LOGIC_Z;
    }
    for (i = 0; i < SRAM_MAX_LANES; i++) {
        state->out_kind[i] = OUT_Z;
        state->out_enable[i] = 0;
        state->off_from_z[i] = 1;
    }
    update(sim, instance, state);
}

static void sram_pin_changed(
    Sim *sim,
    SimInstance *instance,
    void *state,
    size_t pin,
    LogicValue old_value,
    LogicValue new_value
) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    update(sim, instance, state);
}

static void sram_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    (void)timer_id;
    refresh_outputs(sim, instance, raw);
}

/* ---- Inspection hooks ---------------------------------------------------- */

static int sram_memory_size(const void *raw, uint32_t *words, unsigned *bits) {
    const SramState *state = raw;

    if (words != NULL) {
        *words = state->word_count;
    }
    if (bits != NULL) {
        *bits = state->data_bits;
    }
    return 1;
}

static size_t sram_memory_read(const void *raw, uint32_t start, size_t count,
                               uint32_t *values, uint32_t *known) {
    const SramState *state = raw;
    size_t i;

    if (start >= state->word_count) {
        return 0;
    }
    if (count > state->word_count - start) {
        count = state->word_count - start;
    }
    for (i = 0; i < count; i++) {
        uint32_t value;
        uint32_t bits;

        fetch(state, start + (uint32_t)i, &value, &bits);
        if (values != NULL) {
            values[i] = value;
        }
        if (known != NULL) {
            known[i] = bits;
        }
    }
    return count;
}

/* ---- Kind: devices, pinouts, parts --------------------------------------- */

static const char *const DEVICE_KEYS[] = {
    "address", "data", "chipEnable", "chipEnableHigh", "outputEnable", "writeEnable",
    "byteEnable", "grades", NULL
};
static const char *const PART_KEYS[] = {"grade", NULL};

/* The 24 timing names, in SramTiming order. */
static const char *const TIMING_NAMES[] = {
    "trc", "taa", "tace", "toe", "tba", "toh", "tclz", "tolz", "tblz", "tchz", "tohz", "tbhz",
    "twc", "taw", "tcw", "tbw", "tas", "twp", "twpOeLow", "twr", "tdw", "tdh", "twhz", "tow",
    "note", NULL
};

typedef struct {
    SramTiming timing;
    PartSram info;
} SramGrade;

typedef struct {
    const char *const *address;
    size_t address_count;
    const char *const *data;
    size_t data_count;
    const char *const *ce_n;
    size_t ce_n_count;
    const char *const *ce;
    size_t ce_count;
    const char *oe;
    const char *we;
    const char *const *be;
    size_t be_count;
    const char **grade_names;
    SramGrade *grades;
    size_t grade_count;
} SramDevice;

typedef struct {
    size_t addr[SRAM_MAX_ADDRESS_BITS];
    size_t dq[SRAM_MAX_DATA_BITS];
    size_t ce_n[SRAM_MAX_SELECTS];
    size_t ce[SRAM_MAX_SELECTS];
    size_t oe;
    size_t we;
    size_t be[SRAM_MAX_LANES];
    const SramDevice *device;
} SramPins;

static int load_list(KindContext *ctx, const cJSON *json, const char *key, size_t min, size_t max,
                     const char *const **out, size_t *count) {
    if (kind_names(ctx, json, key, out, count) != 0) {
        return -1;
    }
    if (*count < min || *count > max) {
        return kind_error(ctx, "\"%s\" needs %zu to %zu pin names", key, min, max);
    }
    return 0;
}

static int load_grade(KindContext *ctx, const cJSON *json, SramTiming *t) {
    SimTime *fields[] = {
        &t->trc, &t->taa, &t->tace, &t->toe, &t->tba, &t->toh, &t->tclz, &t->tolz, &t->tblz,
        &t->tchz, &t->tohz, &t->tbhz, &t->twc, &t->taw, &t->tcw, &t->tbw, &t->tas, &t->twp,
        &t->twp_oe_low, &t->twr, &t->tdw, &t->tdh, &t->twhz, &t->tow
    };
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a grade is an object of timing values");
    }
    if (kind_check_keys(ctx, json, TIMING_NAMES, NULL) != 0) {
        return -1;
    }
    for (i = 0; i < SRAM_COUNT(fields); i++) {
        const cJSON *value = cJSON_GetObjectItemCaseSensitive(json, TIMING_NAMES[i]);

        *fields[i] = 0;   /* a missing name is 0, as the old designated initializers */
        if (value != NULL && kind_ns(ctx, value, TIMING_NAMES[i], fields[i]) != 0) {
            return -1;
        }
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return 0;
}

static void *sram_load_device(KindContext *ctx, const cJSON *json) {
    SramDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *grades = kind_member(ctx, json, "grades", 1);
    const cJSON *item;
    size_t n = 0;

    if (device == NULL || grades == NULL ||
        load_list(ctx, json, "address", 1, SRAM_MAX_ADDRESS_BITS, &device->address,
                  &device->address_count) != 0 ||
        load_list(ctx, json, "data", 1, SRAM_MAX_DATA_BITS, &device->data, &device->data_count) != 0 ||
        load_list(ctx, json, "chipEnable", 0, SRAM_MAX_SELECTS, &device->ce_n, &device->ce_n_count) != 0 ||
        load_list(ctx, json, "chipEnableHigh", 0, SRAM_MAX_SELECTS, &device->ce, &device->ce_count) != 0 ||
        load_list(ctx, json, "byteEnable", 0, SRAM_MAX_LANES, &device->be, &device->be_count) != 0 ||
        kind_string(ctx, json, "outputEnable", &device->oe) != 0 ||
        kind_string(ctx, json, "writeEnable", &device->we) != 0) {
        return NULL;
    }
    if (!cJSON_IsObject(grades) || grades->child == NULL) {
        kind_error(ctx, "\"grades\" must be a non-empty object");
        return NULL;
    }
    device->grade_count = (size_t)cJSON_GetArraySize(grades);
    device->grade_names = kind_alloc(ctx, device->grade_count * sizeof(*device->grade_names));
    device->grades = kind_alloc(ctx, device->grade_count * sizeof(*device->grades));
    if (device->grade_names == NULL || device->grades == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, grades) {
        SramGrade *grade = &device->grades[n];
        const cJSON *other;

        for (other = grades->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                kind_error(ctx, "duplicate key \"%s\"", item->string);
                return NULL;
            }
        }
        kind_push(ctx, "grades.%s", item->string);
        device->grade_names[n] = kind_strdup(ctx, item->string);
        if (device->grade_names[n] == NULL || load_grade(ctx, item, &grade->timing) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        grade->info.words = (uint32_t)1 << device->address_count;
        grade->info.bits = (unsigned)device->data_count;
        grade->info.access_ps = grade->timing.taa;
        grade->info.grade = device->grade_names[n];
        n++;
    }
    return device;
}

static int bind_list(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                     const char *const *names, size_t count, size_t *out) {
    size_t i;

    for (i = 0; i < count; i++) {
        kind_push(ctx, "%s[%zu]", key, i);
        if (kind_pin(ctx, pins, pin_count, names[i], &out[i]) != 0) {
            return -1;
        }
        kind_pop(ctx);
    }
    return 0;
}

static void *sram_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                              size_t pin_count) {
    const SramDevice *device = raw;
    SramPins *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL ||
        bind_list(ctx, pins, pin_count, "address", device->address, device->address_count, bound->addr) != 0 ||
        bind_list(ctx, pins, pin_count, "data", device->data, device->data_count, bound->dq) != 0 ||
        bind_list(ctx, pins, pin_count, "chipEnable", device->ce_n, device->ce_n_count, bound->ce_n) != 0 ||
        bind_list(ctx, pins, pin_count, "chipEnableHigh", device->ce, device->ce_count, bound->ce) != 0 ||
        bind_list(ctx, pins, pin_count, "byteEnable", device->be, device->be_count, bound->be) != 0 ||
        bind_list(ctx, pins, pin_count, "outputEnable", &device->oe, 1, &bound->oe) != 0 ||
        bind_list(ctx, pins, pin_count, "writeEnable", &device->we, 1, &bound->we) != 0) {
        return NULL;
    }
    bound->device = device;
    return bound;
}

static int sram_load_part(KindContext *ctx, const void *raw, const void *bound,
                          const cJSON *json, Part *out) {
    const SramDevice *device = raw;
    const char *grade;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "grade", &grade) != 0) {
        return -1;
    }
    for (i = 0; i < device->grade_count; i++) {
        if (strcmp(device->grade_names[i], grade) == 0) {
            out->sram = &device->grades[i].info;
            kind_set_part_data(out, &device->grades[i]);
            return 0;
        }
    }
    return kind_error(ctx, "grade \"%s\" is not in \"grades\"", grade);
}

/* ---- Model creation ------------------------------------------------------ */

static size_t sram_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *sram_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const SramPins *pins = internal->bound;
    const SramDevice *device = pins->device;
    const SramGrade *grade = internal->data;
    SramState *state;
    size_t i;

    state = calloc(1, sizeof(*state));
    if (state == NULL) {
        return NULL;
    }
    state->t = &grade->timing;
    state->address_bits = (unsigned)device->address_count;
    state->data_bits = (unsigned)device->data_count;
    state->word_count = (uint32_t)1 << device->address_count;
    state->oe_pin = pins->oe;
    state->we_pin = pins->we;
    for (i = 0; i < SRAM_MAX_ADDRESS_BITS; i++) {
        state->addr_pin[i] = i < device->address_count ? pins->addr[i] : SRAM_NO_PIN;
    }
    for (i = 0; i < SRAM_MAX_DATA_BITS; i++) {
        state->dq_pin[i] = i < device->data_count ? pins->dq[i] : SRAM_NO_PIN;
    }
    for (i = 0; i < device->ce_n_count; i++) {
        state->ce_n_pin[state->ce_n_count++] = pins->ce_n[i];
    }
    for (i = 0; i < device->ce_count; i++) {
        state->ce_pin[state->ce_count++] = pins->ce[i];
    }
    for (i = 0; i < device->be_count; i++) {
        state->be_pin[i] = pins->be[i];
    }
    state->be_count = (unsigned)device->be_count;

    /* Byte lanes: one per byte enable, or the whole word when there are none. */
    state->lane_count = state->be_count > 0 ? state->be_count : 1;
    for (i = 0; i < state->data_bits; i++) {
        unsigned lane = state->be_count > 0 ? (unsigned)(i / 8) : 0;

        if (lane >= state->lane_count) {
            lane = state->lane_count - 1;
        }
        state->lane_of_bit[i] = lane;
        state->lane_mask[lane] |= (uint32_t)1 << i;
    }

    state->page_count = (state->word_count + SRAM_PAGE_WORDS - 1) / SRAM_PAGE_WORDS;
    state->pages = calloc(state->page_count, sizeof(*state->pages));
    if (state->pages == NULL) {
        free(state);
        return NULL;
    }
    return state;
}

static void sram_destroy(void *raw) {
    SramState *state = raw;
    size_t i;

    if (state == NULL) {
        return;
    }
    for (i = 0; i < state->page_count; i++) {
        free(state->pages[i]);
    }
    free(state->pages);
    free(state);
}

static const SimComponentType SRAM_MODEL = {
    "sram", sram_pin_count, sram_create, sram_destroy, sram_start, sram_pin_changed, sram_timer,
    NULL, NULL, sram_memory_size, sram_memory_read
};

const PartKindOps SRAM_KIND = {
    PART_SRAM, "sram", &SRAM_MODEL, DEVICE_KEYS, PART_KEYS,
    sram_load_device, sram_bind_pinout, sram_load_part
};
