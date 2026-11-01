/*
 * eeprom: parallel EEPROMs (Microchip AT28 family, plan-04-A section 5).
 *
 * Memory: every instance has its own byte array, created erased (every byte
 * FF). Each bit is 0, 1 or X.
 *
 * Reads follow the SRAM read window (sram.c) with no low-Z delay: with CE#
 * and OE# low and WE# high the outputs drive X, then data at the latest of
 * address + tACC, CE# + tCE and OE# + tOE; old data holds tOH after an
 * address change; they return to Z tDF after the read ends.
 *
 * Writes follow the datasheets. A load starts when CE# and WE# are both low
 * with OE# high (the address is latched then) and ends when that stops (the
 * data is latched then). Loads within tBLC of each other form one window,
 * which may only touch one page; tBLC after the last load the internal write
 * cycle starts and runs for tWC. During it loads are ignored and reads
 * return DATA# polling (I/O7 = the complement of the last loaded byte's
 * bit 7) and the toggle bit (I/O6), with I/O0-5 X. A timing violation stores
 * the byte as X.
 *
 * Software data protection (SDP): a window that starts with the enable
 * prefix (A/AA, B/55, A/A0) turns protection on; the disable sequence
 * (A/AA, B/55, A/80, A/AA, B/55, A/20) turns it off, on devices whose
 * protection is "off" at power-up. Command bytes are never data. While
 * protection is on, a window without either sequence writes nothing (its
 * cycle still runs).
 *
 * Not modeled: 12 V chip erase and device ID, the power-on write inhibit,
 * the write-pulse noise filter, tCS and tCH (zero on every part).
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define EE_NO_PIN ((size_t)-1)
#define EE_MAX_ADDRESS_BITS 24
#define EE_DATA_BITS 8
#define EE_NEVER UINT64_MAX
#define EE_MAX_UNKNOWN 32
#define EE_COMMAND_MAX 6
#define EE_COUNT(array) (sizeof(array) / sizeof((array)[0]))

enum { TIMER_REFRESH, TIMER_WINDOW, TIMER_CYCLE };

/* One speed grade. Times in picoseconds; 0 where the datasheet gives none. */
typedef struct {
    SimTime tacc, tce, toe, tdf, toh;
    SimTime tas, tah, tcs, tch, twp, twph, tds, tdh, toes, toeh, tblc, twc;
} EepromTiming;

typedef enum { PROTECTION_OFF, PROTECTION_ALWAYS } Protection;

/* ---- Instance state ------------------------------------------------------ */

typedef enum { OUT_Z, OUT_X, OUT_DATA, OUT_HOLD } OutKind;

typedef enum { MODE_IDLE, MODE_LOADING, MODE_BUSY } Mode;

/* How the window's first loads compare with the SDP sequences. */
typedef enum {
    COMMAND_MATCHING, /* every load so far is a step of a sequence */
    COMMAND_NONE,     /* the window is data */
    COMMAND_ENABLE,   /* the enable prefix is done; the rest is data */
    COMMAND_DISABLE   /* the disable sequence is done; the rest is data */
} CommandState;

/* Where the last load went, so a hold violation can turn it X. */
typedef enum { LAST_NONE, LAST_BUFFER, LAST_COMMAND } LastTarget;

typedef struct {
    uint32_t address;
    uint8_t value;
    uint8_t known;
} Load;

/* A write to every word matching the known address bits, stored as X. */
typedef struct {
    uint32_t address;
    uint32_t known;
} Unknown;

/* An input control: 1 active, 0 inactive, -1 unknown. */
typedef struct {
    int level;
    SimTime active_since;
    SimTime inactive_since;
    SimTime changed;
} Control;

typedef struct {
    const EepromTiming *t;
    unsigned address_bits;
    uint32_t word_count;
    uint32_t page_size;
    uint32_t command_a;
    uint32_t command_b;
    Protection protection;

    size_t addr_pin[EE_MAX_ADDRESS_BITS];
    size_t dq_pin[EE_DATA_BITS];
    size_t ce_pin;
    size_t oe_pin;
    size_t we_pin;

    /* Committed contents. */
    uint8_t *value;
    uint8_t *known;

    /* Inputs as last seen, with the value before the latest change time. */
    LogicValue addr[EE_MAX_ADDRESS_BITS];
    LogicValue addr_prev[EE_MAX_ADDRESS_BITS];
    SimTime addr_time;
    SimTime addr_prev_time;
    LogicValue dq[EE_DATA_BITS];
    LogicValue dq_prev[EE_DATA_BITS];
    SimTime dq_time[EE_DATA_BITS];
    SimTime dq_prev_time[EE_DATA_BITS];
    Control ce;
    Control oe;
    Control we;

    /* Read side. */
    int out_enable;          /* read_on as last evaluated: 1, 0 or -1 */
    SimTime off_time;
    int off_from_z;
    OutKind out_kind;
    SimTime hold_until;
    LogicValue hold[EE_DATA_BITS];
    LogicValue driven[EE_DATA_BITS];
    uint8_t poll_value;      /* what a read during the write cycle returns */
    uint8_t poll_known;
    unsigned poll_reads;
    SimTime wake;

    /* Write side. */
    int writing;             /* write_on as last evaluated: 1, 0 or -1 */
    int load_active;         /* a load started outside the write cycle */
    SimTime load_start;
    uint32_t load_addr;
    uint32_t load_addr_known;
    int load_bad;

    Mode mode;
    int protected_on;
    SimTime last_end;        /* end of the window's latest load */
    SimTime cycle_end;
    int writes_allowed;      /* set when the cycle starts */

    /* The window. */
    CommandState command_state;
    Load command[EE_COMMAND_MAX];
    unsigned command_count;
    int has_page;
    uint32_t page_base;
    int mixed;
    uint8_t *buf_value;      /* page_size each */
    uint8_t *buf_known;
    uint8_t *buf_loaded;
    Unknown unknown[EE_MAX_UNKNOWN];
    unsigned unknown_count;

    /* The latest load, for DATA# polling and the tDH / tOEH checks. */
    uint8_t last_value;
    uint8_t last_known;
    int last_hold_valid;
    SimTime last_hold_end;
    LastTarget last_target;
    uint32_t last_index;
} EepromState;

/* ---- Inputs -------------------------------------------------------------- */

static int level_of(LogicValue value) {
    if (value == LOGIC_Z) {
        return 0; /* undriven control: inactive */
    }
    if (value == LOGIC_X) {
        return -1;
    }
    return value == LOGIC_0; /* every control is active low */
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

/* Reads every input. Returns 1 when any address bit changed. */
static int sync_inputs(Sim *sim, SimInstance *instance, EepromState *state) {
    SimTime now = sim_now(sim);
    int address_changed = 0;
    size_t i;

    for (i = 0; i < state->address_bits; i++) {
        LogicValue value = sim_read_pin(sim, instance, state->addr_pin[i]);

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
    for (i = 0; i < EE_DATA_BITS; i++) {
        LogicValue value = sim_read_pin(sim, instance, state->dq_pin[i]);

        if (value != state->dq[i]) {
            if (state->dq_time[i] != now) {
                state->dq_prev[i] = state->dq[i];
                state->dq_prev_time[i] = state->dq_time[i];
            }
            state->dq[i] = value;
            state->dq_time[i] = now;
        }
    }
    update_control(&state->ce, level_of(sim_read_pin(sim, instance, state->ce_pin)), now);
    update_control(&state->oe, level_of(sim_read_pin(sim, instance, state->oe_pin)), now);
    update_control(&state->we, level_of(sim_read_pin(sim, instance, state->we_pin)), now);
    return address_changed;
}

/* The current address: its bits, and a mask of the bits that are known. */
static void current_address(const EepromState *state, uint32_t *value, uint32_t *known) {
    size_t i;

    *value = 0;
    *known = 0;
    for (i = 0; i < state->address_bits; i++) {
        if (state->addr[i] == LOGIC_0 || state->addr[i] == LOGIC_1) {
            *known |= (uint32_t)1 << i;
            if (state->addr[i] == LOGIC_1) {
                *value |= (uint32_t)1 << i;
            }
        }
    }
}

static int too_short(SimTime now, SimTime since, SimTime minimum) {
    return minimum > 0 && now - since < minimum;
}

static SimTime max_time(SimTime a, SimTime b) {
    return a > b ? a : b;
}

/* ---- The load window ------------------------------------------------------ */

static void window_open(EepromState *state) {
    if (state->mode != MODE_IDLE) {
        return;
    }
    state->mode = MODE_LOADING;
    state->command_state = COMMAND_MATCHING;
    state->command_count = 0;
    state->has_page = 0;
    state->mixed = 0;
    state->unknown_count = 0;
    memset(state->buf_loaded, 0, state->page_size);
}

static void add_unknown(EepromState *state, uint32_t address, uint32_t known) {
    uint32_t full = state->word_count - 1;

    if (state->unknown_count == EE_MAX_UNKNOWN) {
        /* Too many to track: every word is undefined. */
        state->unknown[0].address = 0;
        state->unknown[0].known = 0;
        state->unknown_count = 1;
        return;
    }
    state->unknown[state->unknown_count].address = address & full;
    state->unknown[state->unknown_count].known = known & full;
    state->unknown_count++;
}

/* A data byte for the page buffer. The window's page is its first data byte's. */
static void apply_data(EepromState *state, const Load *load) {
    uint32_t page_mask = state->page_size - 1;
    uint32_t offset = load->address & page_mask;

    if (!state->has_page) {
        state->has_page = 1;
        state->page_base = load->address & ~page_mask;
    }
    if ((load->address & ~page_mask) != state->page_base) {
        state->mixed = 1;
        state->last_target = LAST_NONE;
        return;
    }
    state->buf_value[offset] = load->value;
    state->buf_known[offset] = load->known;
    state->buf_loaded[offset] = 1;
    state->last_target = LAST_BUFFER;
    state->last_index = offset;
}

/* The loads held as a possible command are data after all. */
static void flush_commands(EepromState *state) {
    unsigned count = state->command_count;
    unsigned i;

    state->command_count = 0;
    state->command_state = COMMAND_NONE;
    for (i = 0; i < count; i++) {
        apply_data(state, &state->command[i]);
    }
}

/*
 * Whether this load is step n of a sequence (the steps so far matched).
 * *done gets COMMAND_ENABLE or COMMAND_DISABLE when it completes one.
 */
static int command_step(const EepromState *state, unsigned n, const Load *load, CommandState *done) {
    static const struct { int use_b; uint8_t data; } DISABLE[EE_COMMAND_MAX] = {
        {0, 0xAA}, {1, 0x55}, {0, 0x80}, {0, 0xAA}, {1, 0x55}, {0, 0x20}
    };
    int disable_allowed = state->protection == PROTECTION_OFF;

    *done = COMMAND_MATCHING;
    if (load->known != 0xFF) {
        return 0;
    }
    if (n == 2 && load->address == state->command_a && load->value == 0xA0) {
        *done = COMMAND_ENABLE;
        return 1;
    }
    if (n >= EE_COMMAND_MAX || (n >= 2 && !disable_allowed)) {
        return 0;
    }
    if (load->address != (DISABLE[n].use_b ? state->command_b : state->command_a) ||
        load->value != DISABLE[n].data) {
        return 0;
    }
    if (n == EE_COMMAND_MAX - 1) {
        *done = COMMAND_DISABLE;
    }
    return 1;
}

static void schedule_window(Sim *sim, SimInstance *instance, const EepromState *state) {
    sim_schedule_timer(sim, instance, TIMER_WINDOW, state->t->tblc);
}

/* A load ended now (address, data and their known masks). */
static void record_load(Sim *sim, SimInstance *instance, EepromState *state, uint32_t address,
                        uint32_t address_known, uint8_t value, uint8_t known) {
    SimTime now = sim_now(sim);
    Load load;
    CommandState done;

    window_open(state);
    state->last_end = now;
    state->last_value = value;
    state->last_known = known;
    state->last_hold_valid = 1;
    state->last_hold_end = now;
    schedule_window(sim, instance, state);

    if (address_known != state->word_count - 1) {
        if (state->command_state == COMMAND_MATCHING) {
            flush_commands(state);
        }
        add_unknown(state, address, address_known);
        state->last_target = LAST_NONE;
        return;
    }
    load.address = address;
    load.value = value;
    load.known = known;
    if (state->command_state != COMMAND_MATCHING) {
        apply_data(state, &load);
        return;
    }
    if (command_step(state, state->command_count, &load, &done)) {
        state->last_index = state->command_count;
        state->command[state->command_count++] = load;
        state->last_target = LAST_COMMAND;
        if (done != COMMAND_MATCHING) {
            state->command_state = done;
        }
        return;
    }
    flush_commands(state);
    apply_data(state, &load);
}

/* WE#, CE# or OE# unknown: whatever may have been written is X. */
static void record_unknown_write(Sim *sim, SimInstance *instance, EepromState *state) {
    uint32_t address;
    uint32_t known;

    window_open(state);
    state->last_end = sim_now(sim);
    state->last_known = 0;
    state->last_hold_valid = 0;
    state->last_target = LAST_NONE;
    schedule_window(sim, instance, state);
    if (state->command_state == COMMAND_MATCHING) {
        flush_commands(state);
    }
    current_address(state, &address, &known);
    add_unknown(state, address, known);
    if (state->load_active) {
        add_unknown(state, state->load_addr, state->load_addr_known);
        state->load_active = 0;
    }
}

/* A hold time after the latest load was missed: that byte is X. */
static void spoil_last(EepromState *state) {
    if (!state->last_hold_valid) {
        return;
    }
    state->last_known = 0;
    switch (state->last_target) {
        case LAST_BUFFER:
            state->buf_known[state->last_index] = 0;
            break;
        case LAST_COMMAND:
            /* It no longer matches its step: the sequence is data. */
            state->command[state->last_index].known = 0;
            flush_commands(state);
            break;
        case LAST_NONE:
            break;
    }
}

/* ---- The write cycle ------------------------------------------------------ */

static void start_poll(EepromState *state) {
    state->poll_value = (uint8_t)((~state->last_value & 0x80u) | ((state->poll_reads & 1u) << 6));
    state->poll_known = (uint8_t)((state->last_known & 0x80u) | 0x40u);
    state->poll_reads++;
}

static void begin_cycle(Sim *sim, SimInstance *instance, EepromState *state, SimTime at) {
    if (state->command_state == COMMAND_MATCHING) {
        flush_commands(state); /* a sequence that never finished is data */
    }
    state->writes_allowed = !state->protected_on || state->command_state == COMMAND_ENABLE ||
                            state->command_state == COMMAND_DISABLE;
    state->mode = MODE_BUSY;
    state->cycle_end = at + state->t->twc;
    state->poll_reads = 0;
    state->last_hold_valid = 0;
    sim_schedule_timer(sim, instance, TIMER_CYCLE, state->cycle_end - sim_now(sim));
    if (state->out_enable == 1) {
        start_poll(state);
    }
}

static void corrupt(EepromState *state, const Unknown *unknown) {
    uint32_t full = state->word_count - 1;
    uint32_t word;

    if (unknown->known == full) {
        state->known[unknown->address] = 0;
        return;
    }
    for (word = 0; word < state->word_count; word++) {
        if (((word ^ unknown->address) & unknown->known) == 0) {
            state->known[word] = 0;
        }
    }
}

static void commit(EepromState *state) {
    uint32_t offset;
    unsigned i;

    if (state->writes_allowed) {
        for (offset = 0; offset < state->page_size; offset++) {
            uint32_t word = state->page_base + offset;

            if (!state->buf_loaded[offset]) {
                continue;
            }
            if (state->mixed) {
                state->known[word] = 0;
            } else {
                state->value[word] = state->buf_value[offset] & state->buf_known[offset];
                state->known[word] = state->buf_known[offset];
            }
        }
        for (i = 0; i < state->unknown_count; i++) {
            corrupt(state, &state->unknown[i]);
        }
    }
    if (state->command_state == COMMAND_ENABLE) {
        state->protected_on = 1;
    } else if (state->command_state == COMMAND_DISABLE) {
        state->protected_on = 0;
    }
    state->mode = MODE_IDLE;
}

/* The tBLC timeout and the end of tWC, each at its own time. */
static void service(Sim *sim, SimInstance *instance, EepromState *state, SimTime now) {
    if (state->mode == MODE_LOADING && state->writing != 1 &&
        now >= state->last_end + state->t->tblc) {
        begin_cycle(sim, instance, state, state->last_end + state->t->tblc);
    }
    if (state->mode == MODE_BUSY && now >= state->cycle_end) {
        commit(state);
    }
}

/* ---- Writes -------------------------------------------------------------- */

static void load_begin(EepromState *state, SimTime now) {
    const EepromTiming *t = state->t;
    int bad = too_short(now, state->addr_time, t->tas);

    current_address(state, &state->load_addr, &state->load_addr_known);
    state->load_start = now;
    state->load_active = 1;
    bad |= too_short(now, state->oe.inactive_since, t->toes);
    if (state->mode == MODE_LOADING) {
        bad |= too_short(now, state->last_end, t->twph);
    }
    state->load_bad = bad;
}

/* The checks that run after a load: data and OE# held tDH / tOEH. */
static void hold_checks(EepromState *state, SimTime now) {
    const EepromTiming *t = state->t;
    size_t i;

    if (!state->last_hold_valid) {
        return;
    }
    if (t->tdh > 0 && now < state->last_hold_end + t->tdh) {
        for (i = 0; i < EE_DATA_BITS; i++) {
            if (state->dq_time[i] == now) {
                spoil_last(state);
                return;
            }
        }
    }
    if (t->toeh > 0 && now < state->last_hold_end + t->toeh && state->oe.changed == now) {
        spoil_last(state);
    }
}

static void load_end(Sim *sim, SimInstance *instance, EepromState *state, SimTime now) {
    const EepromTiming *t = state->t;
    int bad = state->load_bad || too_short(now, state->load_start, t->twp);
    uint8_t value = 0;
    uint8_t known = 0;
    size_t i;

    for (i = 0; i < EE_DATA_BITS; i++) {
        LogicValue v = state->dq_time[i] == now ? state->dq_prev[i] : state->dq[i];
        SimTime stable = state->dq_time[i] == now ? state->dq_prev_time[i] : state->dq_time[i];

        bad |= too_short(now, stable, t->tds);
        if (v == LOGIC_0 || v == LOGIC_1) {
            known |= (uint8_t)(1u << i);
            if (v == LOGIC_1) {
                value |= (uint8_t)(1u << i);
            }
        }
    }
    if (bad) {
        known = 0;
    }
    state->load_active = 0;
    record_load(sim, instance, state, state->load_addr, state->load_addr_known, value, known);
    hold_checks(state, now); /* data or OE# that changed at this same instant */
}

static void evaluate_writes(Sim *sim, SimInstance *instance, EepromState *state,
                            int address_changed, SimTime now) {
    int levels[3];
    int level;
    int was = state->writing;

    levels[0] = state->ce.level;
    levels[1] = state->we.level;
    levels[2] = not_level(state->oe.level);
    level = all_of(levels, 3);

    if (state->mode != MODE_BUSY) {
        hold_checks(state, now);
        if (was != 1 && level == 1) {
            load_begin(state, now);
        } else if (was == 1 && level == 1 && state->load_active && address_changed) {
            if (now == state->load_start) {
                /* The address settled at the same instant: latch the final value. */
                current_address(state, &state->load_addr, &state->load_addr_known);
            } else if (now < state->load_start + state->t->tah) {
                state->load_bad = 1;
            }
        } else if (was == 1 && level == 0 && state->load_active) {
            load_end(sim, instance, state, now);
        }
        if (level < 0 && (was != -1 || address_changed)) {
            record_unknown_write(sim, instance, state);
        }
    }
    if (level != 1) {
        state->load_active = 0;
    }
    state->writing = level;
}

/* ---- Reads --------------------------------------------------------------- */

static void evaluate_outputs(EepromState *state, int address_changed, SimTime now) {
    int levels[3];
    int level;
    size_t i;

    levels[0] = state->ce.level;
    levels[1] = state->oe.level;
    levels[2] = not_level(state->we.level);
    level = all_of(levels, 3);

    if (address_changed && state->out_kind == OUT_DATA) {
        state->hold_until = now + state->t->toh;
        for (i = 0; i < EE_DATA_BITS; i++) {
            state->hold[i] = state->driven[i];
        }
    }
    if (level == state->out_enable) {
        return;
    }
    if (level == 0) {
        state->off_time = now;
        state->off_from_z = state->out_kind == OUT_Z;
    }
    if (level == 1 && state->mode == MODE_BUSY) {
        start_poll(state);
    }
    state->out_enable = level;
}

/* What the outputs drive now, and the next time that changes (EE_NEVER if not). */
static OutKind output_kind(const EepromState *state, SimTime now, SimTime *next) {
    const EepromTiming *t = state->t;
    SimTime valid;

    *next = EE_NEVER;
    switch (state->out_enable) {
        case 0: {
            SimTime high_z = state->off_time + t->tdf;

            if (!state->off_from_z && now < high_z) {
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
    valid = max_time(state->addr_time + t->tacc, state->ce.active_since + t->tce);
    valid = max_time(valid, state->oe.active_since + t->toe);
    if (now >= valid) {
        return OUT_DATA;
    }
    if (now < state->hold_until) {
        *next = state->hold_until < valid ? state->hold_until : valid;
        return OUT_HOLD;
    }
    *next = valid;
    return OUT_X;
}

static void drive(Sim *sim, SimInstance *instance, EepromState *state, size_t bit, LogicValue value) {
    if (state->driven[bit] == value) {
        return;
    }
    state->driven[bit] = value;
    sim_schedule_inertial_drive(sim, instance, state->dq_pin[bit], value, DRIVE_STRONG, 0);
}

static void refresh_outputs(Sim *sim, SimInstance *instance, EepromState *state) {
    SimTime now = sim_now(sim);
    SimTime next;
    OutKind kind = output_kind(state, now, &next);
    uint8_t word = 0;
    uint8_t word_known = 0;
    size_t i;

    if (state->mode == MODE_BUSY) {
        word = state->poll_value;
        word_known = state->poll_known;
    } else {
        uint32_t address;
        uint32_t known;

        current_address(state, &address, &known);
        if (known == state->word_count - 1) {
            word = state->value[address];
            word_known = state->known[address];
        }
    }
    state->out_kind = kind;
    for (i = 0; i < EE_DATA_BITS; i++) {
        LogicValue value;

        switch (kind) {
            case OUT_Z:
                value = LOGIC_Z;
                break;
            case OUT_DATA:
                value = !(word_known >> i & 1u) ? LOGIC_X : (word >> i & 1u) ? LOGIC_1 : LOGIC_0;
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
    /* Earlier wake-ups still pending just refresh again, which is harmless. */
    if (next != EE_NEVER && (state->wake <= now || next < state->wake)) {
        state->wake = next;
        sim_schedule_timer(sim, instance, TIMER_REFRESH, next - now);
    }
}

/* ---- Model callbacks ----------------------------------------------------- */

static void update(Sim *sim, SimInstance *instance, EepromState *state) {
    SimTime now = sim_now(sim);
    int address_changed = sync_inputs(sim, instance, state);

    service(sim, instance, state, now);
    evaluate_writes(sim, instance, state, address_changed, now);
    evaluate_outputs(state, address_changed, now);
    refresh_outputs(sim, instance, state);
}

static void eeprom_start(Sim *sim, SimInstance *instance, void *raw) {
    EepromState *state = raw;
    size_t i;

    for (i = 0; i < EE_MAX_ADDRESS_BITS; i++) {
        state->addr[i] = LOGIC_Z;
        state->addr_prev[i] = LOGIC_Z;
    }
    for (i = 0; i < EE_DATA_BITS; i++) {
        state->dq[i] = LOGIC_Z;
        state->dq_prev[i] = LOGIC_Z;
        state->driven[i] = LOGIC_Z;
    }
    state->out_kind = OUT_Z;
    state->out_enable = 0;
    state->off_from_z = 1;
    state->mode = MODE_IDLE;
    /* Power-up: protection as the part ships. */
    state->protected_on = state->protection == PROTECTION_ALWAYS;
    update(sim, instance, state);
}

static void eeprom_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    update(sim, instance, state);
}

static void eeprom_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    (void)timer_id;
    update(sim, instance, state);
}

/* ---- Inspection hooks ---------------------------------------------------- */

static int eeprom_memory_size(const void *raw, uint32_t *words, unsigned *bits) {
    const EepromState *state = raw;

    if (words != NULL) {
        *words = state->word_count;
    }
    if (bits != NULL) {
        *bits = EE_DATA_BITS;
    }
    return 1;
}

static size_t eeprom_memory_read(const void *raw, uint32_t start, size_t count,
                                 uint32_t *values, uint32_t *known) {
    const EepromState *state = raw;
    size_t i;

    if (start >= state->word_count) {
        return 0;
    }
    if (count > state->word_count - start) {
        count = state->word_count - start;
    }
    for (i = 0; i < count; i++) {
        if (values != NULL) {
            values[i] = state->value[start + i];
        }
        if (known != NULL) {
            known[i] = state->known[start + i];
        }
    }
    return count;
}

static void eeprom_memory_load(void *raw, uint32_t start, const uint32_t *values, size_t count) {
    EepromState *state = raw;
    size_t i;

    for (i = 0; i < count; i++) {
        state->value[start + i] = (uint8_t)values[i];
        state->known[start + i] = 0xFF;
    }
}

/* ---- Kind: devices, pinouts, parts --------------------------------------- */

static const char *const DEVICE_KEYS[] = {
    "address", "data", "chipEnable", "outputEnable", "writeEnable", "pageSize",
    "commandAddresses", "protection", "grades", NULL
};
static const char *const PART_KEYS[] = {"grade", NULL};

/* The timing names, in EepromTiming order. */
static const char *const TIMING_NAMES[] = {
    "tacc", "tce", "toe", "tdf", "toh", "tas", "tah", "tcs", "tch", "twp", "twph", "tds", "tdh",
    "toes", "toeh", "tblc", "twc", "note", NULL
};

typedef struct {
    EepromTiming timing;
    PartEeprom info;
} EepromGrade;

typedef struct {
    const char *const *address;
    size_t address_count;
    const char *const *data;
    size_t data_count;
    const char *ce;
    const char *oe;
    const char *we;
    uint32_t page_size;
    uint32_t command[2];
    Protection protection;
    const char **grade_names;
    EepromGrade *grades;
    size_t grade_count;
} EepromDevice;

typedef struct {
    size_t addr[EE_MAX_ADDRESS_BITS];
    size_t dq[EE_DATA_BITS];
    size_t ce;
    size_t oe;
    size_t we;
    const EepromDevice *device;
} EepromPins;

static int load_grade(KindContext *ctx, const cJSON *json, EepromTiming *t) {
    SimTime *fields[] = {
        &t->tacc, &t->tce, &t->toe, &t->tdf, &t->toh, &t->tas, &t->tah, &t->tcs, &t->tch,
        &t->twp, &t->twph, &t->tds, &t->tdh, &t->toes, &t->toeh, &t->tblc, &t->twc
    };
    const cJSON *note = cJSON_GetObjectItemCaseSensitive(json, "note");
    size_t i;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "a grade is an object of timing values");
    }
    if (kind_check_keys(ctx, json, TIMING_NAMES, NULL) != 0) {
        return -1;
    }
    for (i = 0; i < EE_COUNT(fields); i++) {
        const cJSON *value = cJSON_GetObjectItemCaseSensitive(json, TIMING_NAMES[i]);

        *fields[i] = 0;
        if (value != NULL && kind_ns(ctx, value, TIMING_NAMES[i], fields[i]) != 0) {
            return -1;
        }
    }
    if (t->tblc == 0) {
        return kind_error(ctx, "\"tblc\" must be more than 0");
    }
    if (t->twc == 0) {
        return kind_error(ctx, "\"twc\" must be more than 0");
    }
    if (note != NULL && !cJSON_IsString(note)) {
        return kind_error(ctx, "\"note\" must be a string");
    }
    return 0;
}

/* A hex address of [0-9A-F]+ below words. */
static int load_command_address(KindContext *ctx, const cJSON *item, size_t index, uint32_t words,
                                uint32_t *out) {
    const char *text = cJSON_IsString(item) ? item->valuestring : NULL;
    uint64_t value = 0;
    size_t i;

    if (text == NULL || text[0] == '\0') {
        return kind_error(ctx, "commandAddresses[%zu] must be a hex string", index);
    }
    for (i = 0; text[i] != '\0'; i++) {
        char c = text[i];

        if (c >= '0' && c <= '9') {
            value = value * 16 + (uint64_t)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            value = value * 16 + (uint64_t)(c - 'A' + 10);
        } else {
            return kind_error(ctx, "commandAddresses[%zu] \"%s\" is not a hex address (0-9, A-F)",
                              index, text);
        }
        if (value >= words) {
            break;
        }
    }
    if (value >= words) {
        return kind_error(ctx, "commandAddresses[%zu] \"%s\" is past the last address (%X)", index,
                          text, (unsigned)(words - 1));
    }
    *out = (uint32_t)value;
    return 0;
}

static void *eeprom_load_device(KindContext *ctx, const cJSON *json) {
    EepromDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *grades = kind_member(ctx, json, "grades", 1);
    const cJSON *commands;
    const char *protection;
    uint64_t page_size;
    uint32_t words;
    const cJSON *item;
    size_t n = 0;

    if (device == NULL || grades == NULL ||
        kind_names(ctx, json, "address", &device->address, &device->address_count) != 0) {
        return NULL;
    }
    if (device->address_count < 1 || device->address_count > EE_MAX_ADDRESS_BITS) {
        kind_error(ctx, "\"address\" needs 1 to %d pin names", EE_MAX_ADDRESS_BITS);
        return NULL;
    }
    words = (uint32_t)1 << device->address_count;
    if (kind_names(ctx, json, "data", &device->data, &device->data_count) != 0) {
        return NULL;
    }
    if (device->data_count != EE_DATA_BITS) {
        kind_error(ctx, "\"data\" needs exactly %d pin names", EE_DATA_BITS);
        return NULL;
    }
    if (kind_string(ctx, json, "chipEnable", &device->ce) != 0 ||
        kind_string(ctx, json, "outputEnable", &device->oe) != 0 ||
        kind_string(ctx, json, "writeEnable", &device->we) != 0 ||
        kind_uint(ctx, json, "pageSize", (uint64_t)1 << EE_MAX_ADDRESS_BITS, &page_size) != 0) {
        return NULL;
    }
    if (page_size == 0 || (page_size & (page_size - 1)) != 0) {
        kind_error(ctx, "\"pageSize\" must be a power of two");
        return NULL;
    }
    if (page_size > words) {
        kind_error(ctx, "\"pageSize\" %llu is more than the %u words",
                   (unsigned long long)page_size, (unsigned)words);
        return NULL;
    }
    device->page_size = (uint32_t)page_size;

    commands = kind_member(ctx, json, "commandAddresses", 1);
    if (commands == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(commands) || cJSON_GetArraySize(commands) != 2) {
        kind_error(ctx, "\"commandAddresses\" needs two hex strings");
        return NULL;
    }
    if (load_command_address(ctx, cJSON_GetArrayItem(commands, 0), 0, words, &device->command[0]) != 0 ||
        load_command_address(ctx, cJSON_GetArrayItem(commands, 1), 1, words, &device->command[1]) != 0) {
        return NULL;
    }

    if (kind_string(ctx, json, "protection", &protection) != 0) {
        return NULL;
    }
    if (strcmp(protection, "off") == 0) {
        device->protection = PROTECTION_OFF;
    } else if (strcmp(protection, "always") == 0) {
        device->protection = PROTECTION_ALWAYS;
    } else {
        kind_error(ctx, "\"protection\" must be \"off\" or \"always\", not \"%s\"", protection);
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
        EepromGrade *grade = &device->grades[n];
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
        grade->info.words = words;
        grade->info.bits = EE_DATA_BITS;
        grade->info.access_ps = grade->timing.tacc;
        grade->info.grade = device->grade_names[n];
        grade->info.page_bytes = device->page_size;
        grade->info.write_cycle_ps = grade->timing.twc;
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

static int bind_one(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *key,
                    const char *name, size_t *out) {
    kind_push(ctx, "%s", key);
    if (kind_pin(ctx, pins, pin_count, name, out) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static void *eeprom_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const EepromDevice *device = raw;
    EepromPins *bound = kind_alloc(ctx, sizeof(*bound));

    if (bound == NULL ||
        bind_list(ctx, pins, pin_count, "address", device->address, device->address_count, bound->addr) != 0 ||
        bind_list(ctx, pins, pin_count, "data", device->data, device->data_count, bound->dq) != 0 ||
        bind_one(ctx, pins, pin_count, "chipEnable", device->ce, &bound->ce) != 0 ||
        bind_one(ctx, pins, pin_count, "outputEnable", device->oe, &bound->oe) != 0 ||
        bind_one(ctx, pins, pin_count, "writeEnable", device->we, &bound->we) != 0) {
        return NULL;
    }
    bound->device = device;
    return bound;
}

static int eeprom_load_part(KindContext *ctx, const void *raw, const void *bound,
                            const cJSON *json, Part *out) {
    const EepromDevice *device = raw;
    const char *grade;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "grade", &grade) != 0) {
        return -1;
    }
    for (i = 0; i < device->grade_count; i++) {
        if (strcmp(device->grade_names[i], grade) == 0) {
            out->eeprom = &device->grades[i].info;
            kind_set_part_data(out, &device->grades[i]);
            return 0;
        }
    }
    return kind_error(ctx, "grade \"%s\" is not in \"grades\"", grade);
}

/* ---- Model creation ------------------------------------------------------ */

static size_t eeprom_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void eeprom_destroy(void *raw);

static void *eeprom_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const EepromPins *pins = internal->bound;
    const EepromDevice *device = pins->device;
    const EepromGrade *grade = internal->data;
    EepromState *state = calloc(1, sizeof(*state));
    size_t i;

    if (state == NULL) {
        return NULL;
    }
    state->t = &grade->timing;
    state->address_bits = (unsigned)device->address_count;
    state->word_count = (uint32_t)1 << device->address_count;
    state->page_size = device->page_size;
    state->command_a = device->command[0];
    state->command_b = device->command[1];
    state->protection = device->protection;
    for (i = 0; i < EE_MAX_ADDRESS_BITS; i++) {
        state->addr_pin[i] = i < device->address_count ? pins->addr[i] : EE_NO_PIN;
    }
    for (i = 0; i < EE_DATA_BITS; i++) {
        state->dq_pin[i] = pins->dq[i];
    }
    state->ce_pin = pins->ce;
    state->oe_pin = pins->oe;
    state->we_pin = pins->we;

    state->value = malloc(state->word_count);
    state->known = malloc(state->word_count);
    state->buf_value = calloc(state->page_size, 1);
    state->buf_known = calloc(state->page_size, 1);
    state->buf_loaded = calloc(state->page_size, 1);
    if (state->value == NULL || state->known == NULL || state->buf_value == NULL ||
        state->buf_known == NULL || state->buf_loaded == NULL) {
        eeprom_destroy(state);
        return NULL;
    }
    /* Erased: every byte FF. */
    memset(state->value, 0xFF, state->word_count);
    memset(state->known, 0xFF, state->word_count);
    return state;
}

static void eeprom_destroy(void *raw) {
    EepromState *state = raw;

    if (state == NULL) {
        return;
    }
    free(state->value);
    free(state->known);
    free(state->buf_value);
    free(state->buf_known);
    free(state->buf_loaded);
    free(state);
}

static const SimComponentType EEPROM_MODEL = {
    "eeprom", eeprom_pin_count, eeprom_create, eeprom_destroy, eeprom_start, eeprom_pin_changed,
    eeprom_timer, NULL, NULL, eeprom_memory_size, eeprom_memory_read, NULL, eeprom_memory_load
};

const PartKindOps EEPROM_KIND = {
    PART_EEPROM, "eeprom", &EEPROM_MODEL, DEVICE_KEYS, PART_KEYS,
    eeprom_load_device, eeprom_bind_pinout, eeprom_load_part
};
