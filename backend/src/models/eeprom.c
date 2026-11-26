#include "eeprom_int.h"

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

/* A hold time after the latest load was missed: that byte is X. Only hold_checks calls it, after
   it checked last_hold_valid. */
static void spoil_last(EepromState *state) {
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

void eep_eeprom_start(Sim *sim, SimInstance *instance, void *raw) {
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

void eep_eeprom_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    update(sim, instance, state);
}

void eep_eeprom_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    (void)timer_id;
    update(sim, instance, state);
}

/* ---- Inspection hooks ---------------------------------------------------- */

int eep_eeprom_memory_size(const void *raw, uint32_t *words, unsigned *bits) {
    const EepromState *state = raw;

    if (words != NULL) {
        *words = state->word_count;
    }
    if (bits != NULL) {
        *bits = EE_DATA_BITS;
    }
    return 1;
}

size_t eep_eeprom_memory_read(const void *raw, uint32_t start, size_t count,
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

void eep_eeprom_memory_load(void *raw, uint32_t start, const uint32_t *values, size_t count) {
    EepromState *state = raw;
    size_t i;

    for (i = 0; i < count; i++) {
        state->value[start + i] = (uint8_t)values[i];
        state->known[start + i] = 0xFF;
    }
}

