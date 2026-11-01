#include "led7.h"

#include <stdlib.h>
#include <string.h>

/* ---- Part data ----------------------------------------------------------- */

typedef enum {
    LED7_KIND_SEG,
    LED7_KIND_COM,
    LED7_KIND_NC,
    LED7_KIND_NP   /* no pin at this position */
} Led7PinKind;

typedef struct {
    unsigned number;
    Led7PinKind kind;
    unsigned digit;     /* SEG: 0 = every digit; COM: digit or LED7_EXTRAS */
    unsigned segments;  /* SEG: bit per Led7Segment */
} Led7Pin;

typedef struct {
    const char *name;
    unsigned digits;
    const char *size;
    const char *color;
    const char *chip;
    int common_anode;
    unsigned colon_after;
    unsigned dot_after;
    const Led7Pin *pins;
    size_t pin_count;
} Led7Part;

#define LED7_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define S(segment) (1u << LED7_SEG_##segment)

#define LED7_PIN_ENTRY(number, name, kind, digit, segments) \
    {number, LED7_KIND_##kind, digit, segments},
#define LED7_ROLE_SEG PIN_ROLE_INPUT
#define LED7_ROLE_COM PIN_ROLE_INPUT
#define LED7_ROLE_NC PIN_ROLE_NO_CONNECT
#define LED7_ROLE_NP PIN_ROLE_NO_CONNECT
#define LED7_GROUP_SEG NULL
#define LED7_GROUP_COM NULL
#define LED7_GROUP_NC NULL
#define LED7_GROUP_NP CHIP_PIN_GROUP_HIDDEN
#define LED7_CHIP_PIN(number, name, kind, digit, segments) \
    {number, name, LED7_ROLE_##kind, LED7_GROUP_##kind},

#define LED7_PART(ID, NAME, DESCRIPTION, DIGITS, SIZE, COLOR, CHIP, CA, COLON, DOT, FLAGS) \
    static const Led7Pin ID##_LED7_PINS[] = { ID##_PINS(LED7_PIN_ENTRY) }; \
    static const ChipPin ID##_CHIP_PINS[] = { ID##_PINS(LED7_CHIP_PIN) }; \
    static const Led7Part ID##_PART = { \
        NAME, DIGITS, SIZE, COLOR, CHIP, CA, COLON, DOT, \
        ID##_LED7_PINS, LED7_COUNT(ID##_LED7_PINS) \
    }; \
    static const char *const ID##_NAMES[] = { NAME }; \
    static const unsigned ID##_FLAGS[] = { FLAGS }; \
    static const ChipDefinition ID##_DEFINITION = { \
        NAME, "7-Segment", DESCRIPTION, "Indicators", \
        ID##_NAMES, 1, ID##_CHIP_PINS, LED7_COUNT(ID##_CHIP_PINS), \
        led7_lookup, DIGITS, ID##_FLAGS, "LED 7-Seg" \
    };

#include "led7_parts.inc"
#undef LED7_PART

#define LED7_PART(ID, NAME, DESCRIPTION, DIGITS, SIZE, COLOR, CHIP, CA, COLON, DOT, FLAGS) &ID##_PART,
static const Led7Part *const PARTS[] = {
#include "led7_parts.inc"
};
#undef LED7_PART

#define LED7_PART(ID, NAME, DESCRIPTION, DIGITS, SIZE, COLOR, CHIP, CA, COLON, DOT, FLAGS) &ID##_DEFINITION,
static const ChipDefinition *const DEFINITIONS[] = {
#include "led7_parts.inc"
};
#undef LED7_PART

size_t led7_definition_count(void) {
    return LED7_COUNT(DEFINITIONS);
}

const ChipDefinition *led7_definition_at(size_t index) {
    return index < led7_definition_count() ? DEFINITIONS[index] : NULL;
}

const SimComponentType *led7_lookup(const char *part_number, SimLogicConfig *config) {
    size_t i;

    if (part_number == NULL) {
        return NULL;
    }
    for (i = 0; i < LED7_COUNT(PARTS); i++) {
        if (strcmp(PARTS[i]->name, part_number) == 0) {
            if (config != NULL) {
                config->package = PACKAGE_TYPE_DIP;
                config->family = LOGIC_FAMILY_LED7;
                config->model_data = PARTS[i];
                config->variant = 0;
            }
            return &LED7_TYPE;
        }
    }
    return NULL;
}

static const Led7Part *config_part(const SimLogicConfig *config) {
    if (config == NULL || config->family != LOGIC_FAMILY_LED7) {
        return NULL;
    }
    return config->model_data;
}

/* ---- Layout -------------------------------------------------------------- */

static const char *const SEGMENT_NAMES[LED7_SEG_COUNT] = {
    "A", "B", "C", "D", "E", "F", "G", "DP", "LDP", "HJ", "L1", "L2", "L3",
};

const char *led7_segment_name(Led7Segment segment) {
    return segment < LED7_SEG_COUNT ? SEGMENT_NAMES[segment] : "?";
}

unsigned led7_digits(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->digits : 0;
}

int led7_common_anode(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL && part->common_anode;
}

const char *led7_size_label(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->size : "unknown";
}

const char *led7_color_label(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->color : "unknown";
}

const char *led7_chip_label(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->chip : "unknown";
}

unsigned led7_colon_after(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->colon_after : 0;
}

unsigned led7_dot_after(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->dot_after : 0;
}

/* ---- LEDs ---------------------------------------------------------------- */

#define LED7_MAX_LEDS 64
#define LED7_MAX_COMMONS 2

typedef struct {
    unsigned digit;
    Led7Segment segment;
    size_t segment_pin;                  /* 0-based pin index */
    size_t common_pins[LED7_MAX_COMMONS];
    size_t common_count;
} Led7Led;

/* The common pins of a digit (or of L1-L3), up to LED7_MAX_COMMONS. */
static size_t digit_commons(const Led7Part *part, unsigned digit, size_t *pins) {
    size_t count = 0;
    size_t i;

    for (i = 0; i < part->pin_count && count < LED7_MAX_COMMONS; i++) {
        if (part->pins[i].kind == LED7_KIND_COM && part->pins[i].digit == digit) {
            pins[count++] = part->pins[i].number - 1;
        }
    }
    return count;
}

/* The pin driving one segment of one digit, or -1. */
static long segment_pin(const Led7Part *part, unsigned digit, Led7Segment segment) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        const Led7Pin *pin = &part->pins[i];

        if (pin->kind == LED7_KIND_SEG && (pin->segments & (1u << segment)) &&
            (pin->digit == 0 || pin->digit == digit)) {
            return (long)pin->number - 1;
        }
    }
    return -1;
}

static void add_led(const Led7Part *part, unsigned digit, Led7Segment segment,
                    Led7Led *out, size_t *count) {
    long seg = segment_pin(part, digit, segment);
    Led7Led led;

    if (seg < 0 || *count >= LED7_MAX_LEDS) {
        return;
    }
    led.digit = digit;
    led.segment = segment;
    led.segment_pin = (size_t)seg;
    led.common_count = digit_commons(part, digit, led.common_pins);
    if (led.common_count > 0) {
        out[(*count)++] = led;
    }
}

/* Every LED of a part, in led7_led_at order. */
static size_t build_leds(const Led7Part *part, Led7Led *out) {
    size_t count = 0;
    unsigned digit;
    unsigned segment;

    for (digit = 1; digit <= part->digits; digit++) {
        for (segment = LED7_SEG_A; segment <= LED7_SEG_HJ; segment++) {
            add_led(part, digit, (Led7Segment)segment, out, &count);
        }
    }
    for (segment = LED7_SEG_L1; segment <= LED7_SEG_L3; segment++) {
        add_led(part, LED7_EXTRAS, (Led7Segment)segment, out, &count);
    }
    return count;
}

size_t led7_led_count(const SimLogicConfig *config) {
    const Led7Part *part = config_part(config);
    Led7Led leds[LED7_MAX_LEDS];

    return part != NULL ? build_leds(part, leds) : 0;
}

int led7_led_at(const SimLogicConfig *config, size_t index, unsigned *digit, Led7Segment *segment) {
    const Led7Part *part = config_part(config);
    Led7Led leds[LED7_MAX_LEDS];

    if (part == NULL || index >= build_leds(part, leds)) {
        return 0;
    }
    if (digit != NULL) {
        *digit = leds[index].digit;
    }
    if (segment != NULL) {
        *segment = leds[index].segment;
    }
    return 1;
}

/* Lowest segment a pin drives, for ordering. */
static unsigned first_segment(unsigned segments) {
    unsigned segment = 0;

    while (segment < LED7_SEG_COUNT && !(segments & (1u << segment))) {
        segment++;
    }
    return segment;
}

static unsigned side_key(const Led7Pin *pin) {
    return pin->kind == LED7_KIND_COM ? pin->digit * 1000u + pin->number
                                      : pin->digit * 1000u + first_segment(pin->segments) * 100u + pin->number;
}

size_t led7_side_pins(const SimLogicConfig *config, int commons, unsigned *numbers, size_t cap) {
    const Led7Part *part = config_part(config);
    const Led7Pin *chosen[64];
    size_t count = 0;
    size_t i;
    size_t j;

    if (part == NULL) {
        return 0;
    }
    for (i = 0; i < part->pin_count && count < LED7_COUNT(chosen); i++) {
        Led7PinKind want = commons ? LED7_KIND_COM : LED7_KIND_SEG;

        if (part->pins[i].kind == want) {
            chosen[count++] = &part->pins[i];
        }
    }
    /* Insertion sort: the lists are short. */
    for (i = 1; i < count; i++) {
        const Led7Pin *pin = chosen[i];

        for (j = i; j > 0 && side_key(chosen[j - 1]) > side_key(pin); j--) {
            chosen[j] = chosen[j - 1];
        }
        chosen[j] = pin;
    }
    for (i = 0; i < count && i < cap; i++) {
        numbers[i] = chosen[i]->number;
    }
    return count < cap ? count : cap;
}

/* ---- Simulation model ------------------------------------------------------ */

typedef struct {
    const Led7Part *part;
    Led7Led leds[LED7_MAX_LEDS];
    size_t led_count;
    unsigned char on[LED7_MAX_LEDS];
    unsigned char ever_on[LED7_MAX_LEDS];
    SimTime last_on[LED7_MAX_LEDS]; /* when the LED last went OFF */
} Led7State;

static size_t led7_pin_count(const void *config) {
    const Led7Part *part = config_part(config);
    return part != NULL ? part->pin_count : 0;
}

static void *led7_create(const void *config) {
    const Led7Part *part = config_part(config);
    Led7State *state;

    if (part == NULL) {
        return NULL;
    }
    state = calloc(1, sizeof(*state));
    if (state != NULL) {
        state->part = part;
        state->led_count = build_leds(part, state->leds);
    }
    return state;
}

static void led7_destroy(void *state) {
    free(state);
}

/* A digit's common reads as the first of its common pins that is not Z. */
static LogicValue common_value(const Sim *sim, const SimInstance *instance, const Led7Led *led) {
    size_t i;

    for (i = 0; i < led->common_count; i++) {
        LogicValue value = sim_read_pin(sim, instance, led->common_pins[i]);

        if (value != LOGIC_Z) {
            return value;
        }
    }
    return LOGIC_Z;
}

static void update_leds(Sim *sim, SimInstance *instance, Led7State *state) {
    SimTime now = sim_now(sim);
    size_t i;

    for (i = 0; i < state->led_count; i++) {
        const Led7Led *led = &state->leds[i];
        LogicValue common = common_value(sim, instance, led);
        LogicValue segment = sim_read_pin(sim, instance, led->segment_pin);
        LogicValue anode = state->part->common_anode ? common : segment;
        LogicValue cathode = state->part->common_anode ? segment : common;
        int on = anode == LOGIC_1 && cathode == LOGIC_0;

        if (state->on[i] && !on) {
            state->last_on[i] = now;
        }
        if (on) {
            state->ever_on[i] = 1;
        }
        state->on[i] = (unsigned char)on;
    }
}

/* Joins the common pins the datasheet lists twice, then reads the LEDs. */
static void led7_start(Sim *sim, SimInstance *instance, void *raw) {
    Led7State *state = raw;
    unsigned digit;

    for (digit = 1; digit <= LED7_EXTRAS; digit++) {
        size_t pins[LED7_MAX_COMMONS];

        if (digit_commons(state->part, digit, pins) == 2) {
            sim_set_contact(sim, instance, pins[0], pins[1], 1);
        }
    }
    update_leds(sim, instance, state);
}

static void led7_pin_changed(Sim *sim, SimInstance *instance, void *state,
                             size_t pin, LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    update_leds(sim, instance, state);
}

const SimComponentType LED7_TYPE = {
    "LED7",
    led7_pin_count,
    led7_create,
    led7_destroy,
    led7_start,
    led7_pin_changed,
    NULL,
};

size_t led7_instance_led_count(const SimInstance *instance) {
    const Led7State *state;

    if (sim_instance_type(instance) != &LED7_TYPE) {
        return 0;
    }
    state = sim_instance_state(instance);
    return state != NULL ? state->led_count : 0;
}

size_t led7_instance_glow(const Sim *sim, const SimInstance *instance, SimTime since,
                          char *out, size_t cap) {
    const Led7State *state;
    size_t i;

    (void)sim;
    if (led7_instance_led_count(instance) == 0) {
        return 0;
    }
    state = sim_instance_state(instance);
    for (i = 0; i < state->led_count && i < cap; i++) {
        int glow = state->on[i] || (state->ever_on[i] && state->last_on[i] >= since);
        out[i] = glow ? '1' : '0';
    }
    return state->led_count;
}
