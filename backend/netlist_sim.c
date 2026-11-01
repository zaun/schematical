#include "netlist_sim.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catalog.h"
#include "chip_definition.h"
#include "led.h"
#include "led7.h"
#include "net.h"
#include "osc.h"
#include "sim.h"
#include "sram.h"
#include "supply.h"
#include "switch.h"

#define LINE_MAX_LENGTH 512
#define PART_NUMBER_MAX_LENGTH 128

/* Config storage for any part kind. The model copies what it needs. */
typedef union {
    LedConfig led;
    OscConfig osc;
    SupplyConfig supply;
    SimLogicConfig logic;
} PartConfig;

struct NetlistSim {
    Sim *sim;
    SimNet **nets;
    size_t net_count;
    SimInstance **parts;
    int *is_led;
    PartConfig *configs;
    size_t part_count;
    /* Start of the last netlist_sim_advance: 7-segment LEDs ON since then glow. */
    SimTime step_start;
};

/* ---- Part resolution ---------------------------------------------------- */

/* Finds the simulation model for any orderable part, LED, supply, oscillator or chip. */
static const SimComponentType *resolve_part(
    const char *name,
    PartConfig *config,
    int *is_led
) {
    const SimComponentType *type;
    size_t i;

    *is_led = 0;

    type = led_lookup(name, &config->led);
    if (type != NULL) {
        *is_led = 1;
        return type;
    }

    type = supply_lookup(name, &config->supply);
    if (type != NULL) {
        return type;
    }

    type = osc_lookup(name, &config->osc);
    if (type != NULL) {
        return type;
    }

    for (i = 0; i < catalog_definition_count(); i++) {
        type = catalog_definition_at(i)->lookup(name, &config->logic);
        if (type != NULL) {
            return type;
        }
    }

    return NULL;
}

/* ---- Text output -------------------------------------------------------- */

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
    int failed;
} TextBuffer;

static void text_append(TextBuffer *text, const char *format, ...) {
    va_list args;
    int needed;
    char *grown;

    if (text->failed) {
        return;
    }

    va_start(args, format);
    needed = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (needed < 0) {
        text->failed = 1;
        return;
    }

    if (text->length + (size_t)needed + 1 > text->capacity) {
        size_t capacity = text->capacity == 0 ? 256 : text->capacity;

        while (text->length + (size_t)needed + 1 > capacity) {
            capacity *= 2;
        }

        grown = realloc(text->data, capacity);
        if (grown == NULL) {
            text->failed = 1;
            return;
        }
        text->data = grown;
        text->capacity = capacity;
    }

    va_start(args, format);
    vsnprintf(text->data + text->length, text->capacity - text->length, format, args);
    va_end(args);
    text->length += (size_t)needed;
}

static char logic_char(LogicValue value) {
    switch (value) {
        case LOGIC_0: return '0';
        case LOGIC_1: return '1';
        case LOGIC_X: return 'X';
        case LOGIC_Z: return 'Z';
    }
    return 'X';
}

static char *copy_message(const char *message) {
    size_t length = strlen(message) + 1;
    char *copy = malloc(length);

    if (copy != NULL) {
        memcpy(copy, message, length);
    }
    return copy;
}

static void fail_open(const char *message, char **error_out) {
    if (error_out != NULL) {
        *error_out = copy_message(message);
    }
}

/* ---- Open and close ----------------------------------------------------- */

static int parse_index(const char *text, size_t limit, size_t *index) {
    char *end;
    unsigned long value;

    value = strtoul(text, &end, 10);
    if (end == text || *end != '\0' || value >= limit) {
        return 0;
    }
    *index = (size_t)value;
    return 1;
}

static int parse_line(NetlistSim *netlist, char *line, char **error_out) {
    char command[16];
    char name[PART_NUMBER_MAX_LENGTH];
    char a[32];
    char b[32];
    char c[32];
    size_t index;
    size_t pin_number;
    size_t net_index;
    const SimComponentType *type;
    int is_led;

    if (sscanf(line, "%15s", command) != 1) {
        return 1; /* blank line */
    }

    if (strcmp(command, "nets") == 0) {
        return 1; /* handled before parsing parts */
    }

    if (strcmp(command, "part") == 0) {
        if (sscanf(line, "%*s %31s %127s", a, name) != 2) {
            fail_open("malformed part line", error_out);
            return 0;
        }
        if (!parse_index(a, (size_t)-1, &index) || index != netlist->part_count) {
            fail_open("parts must be declared in order from 0", error_out);
            return 0;
        }

        type = resolve_part(name, &netlist->configs[index], &is_led);
        if (type == NULL) {
            char message[LINE_MAX_LENGTH];
            snprintf(message, sizeof(message), "unknown part %s", name);
            fail_open(message, error_out);
            return 0;
        }

        netlist->is_led[index] = is_led;
        netlist->parts[index] = sim_add_instance(
            netlist->sim,
            type,
            name,
            &netlist->configs[index]
        );
        netlist->part_count++;

        if (netlist->parts[index] == NULL) {
            fail_open(sim_error(netlist->sim), error_out);
            return 0;
        }
        return 1;
    }

    if (strcmp(command, "pin") == 0) {
        if (sscanf(line, "%*s %31s %31s %31s", a, b, c) != 3) {
            fail_open("malformed pin line", error_out);
            return 0;
        }
        if (!parse_index(a, netlist->part_count, &index) ||
            !parse_index(b, (size_t)-1, &pin_number) ||
            pin_number == 0 ||
            !parse_index(c, netlist->net_count, &net_index)) {
            fail_open("pin line refers to an unknown part, pin or net", error_out);
            return 0;
        }

        /* Pin numbers are 1-based; the simulator indexes pins from 0. */
        if (!sim_connect(netlist->sim, netlist->nets[net_index],
                         netlist->parts[index], pin_number - 1)) {
            fail_open(sim_error(netlist->sim), error_out);
            return 0;
        }
        return 1;
    }

    if (strcmp(command, "switch") == 0) {
        if (sscanf(line, "%*s %31s %31s %31s", a, b, c) != 3) {
            fail_open("malformed switch line", error_out);
            return 0;
        }
        if (!parse_index(a, netlist->part_count, &index) ||
            !parse_index(b, (size_t)-1, &pin_number) ||
            (strcmp(c, "0") != 0 && strcmp(c, "1") != 0) ||
            !switch_set_position(netlist->sim, netlist->parts[index], pin_number,
                                 strcmp(c, "1") == 0)) {
            fail_open("switch line refers to an unknown switch or position", error_out);
            return 0;
        }
        return 1;
    }

    fail_open("unknown directive", error_out);
    return 0;
}

NetlistSim *netlist_sim_open(const char *text, char **error_out) {
    NetlistSim *netlist;
    const char *cursor;
    size_t net_count = 0;
    size_t i;
    char line[LINE_MAX_LENGTH];
    int ok = 1;

    if (error_out != NULL) {
        *error_out = NULL;
    }
    if (text == NULL) {
        fail_open("no netlist", error_out);
        return NULL;
    }

    /* The nets line sizes the tables, so find it first. */
    cursor = text;
    while (*cursor != '\0') {
        const char *end = strchr(cursor, '\n');
        size_t length = end != NULL ? (size_t)(end - cursor) : strlen(cursor);

        if (length >= sizeof(line)) {
            fail_open("netlist line too long", error_out);
            return NULL;
        }
        memcpy(line, cursor, length);
        line[length] = '\0';

        if (strncmp(line, "nets ", 5) == 0) {
            if (sscanf(line, "nets %zu", &net_count) != 1) {
                fail_open("malformed nets line", error_out);
                return NULL;
            }
            break;
        }

        cursor = end != NULL ? end + 1 : cursor + length;
    }

    netlist = calloc(1, sizeof(*netlist));
    if (netlist == NULL) {
        fail_open("out of memory", error_out);
        return NULL;
    }

    /* Every line can declare at most one part, so the line count bounds the parts. */
    {
        size_t lines = 1;
        const char *scan;

        for (scan = text; *scan != '\0'; scan++) {
            if (*scan == '\n') {
                lines++;
            }
        }

        netlist->sim = sim_create();
        netlist->net_count = net_count;
        netlist->nets = calloc(net_count + 1, sizeof(*netlist->nets));
        netlist->parts = calloc(lines, sizeof(*netlist->parts));
        netlist->is_led = calloc(lines, sizeof(*netlist->is_led));
        netlist->configs = calloc(lines, sizeof(*netlist->configs));

        if (netlist->sim == NULL || netlist->nets == NULL ||
            netlist->parts == NULL || netlist->is_led == NULL ||
            netlist->configs == NULL) {
            netlist_sim_close(netlist);
            fail_open("out of memory", error_out);
            return NULL;
        }
    }

    for (i = 0; i < net_count; i++) {
        char name[32];
        snprintf(name, sizeof(name), "n%zu", i);
        netlist->nets[i] = sim_add_net(netlist->sim, name);
        if (netlist->nets[i] == NULL) {
            fail_open(sim_error(netlist->sim), error_out);
            netlist_sim_close(netlist);
            return NULL;
        }
    }

    cursor = text;
    while (ok && *cursor != '\0') {
        const char *end = strchr(cursor, '\n');
        size_t length = end != NULL ? (size_t)(end - cursor) : strlen(cursor);

        if (length >= sizeof(line)) {
            fail_open("netlist line too long", error_out);
            ok = 0;
            break;
        }
        memcpy(line, cursor, length);
        line[length] = '\0';

        if (!parse_line(netlist, line, error_out)) {
            ok = 0;
            break;
        }

        cursor = end != NULL ? end + 1 : cursor + length;
    }

    if (!ok) {
        netlist_sim_close(netlist);
        return NULL;
    }

    if (!sim_start(netlist->sim)) {
        fail_open(sim_error(netlist->sim), error_out);
        netlist_sim_close(netlist);
        return NULL;
    }

    return netlist;
}

int netlist_sim_advance(NetlistSim *netlist, SimTime picoseconds) {
    if (netlist == NULL || netlist->sim == NULL) {
        return 0;
    }
    netlist->step_start = sim_now(netlist->sim);
    return sim_run_until(netlist->sim, sim_now(netlist->sim) + picoseconds);
}

char *netlist_sim_snapshot(const NetlistSim *netlist) {
    TextBuffer text = {NULL, 0, 0, 0};
    size_t i;
    const SimNet *shorted;

    if (netlist == NULL || netlist->sim == NULL) {
        return NULL;
    }

    text_append(&text, "time %llu\n", (unsigned long long)sim_now(netlist->sim));

    for (i = 0; i < netlist->net_count; i++) {
        text_append(&text, "net %zu %c\n", i, logic_char(sim_net_value(netlist->nets[i])));
    }

    for (i = 0; i < netlist->part_count; i++) {
        SimInstance *part = netlist->parts[i];

        if (netlist->is_led[i]) {
            /* Pin 1 is the cathode (K), pin 2 the anode (A). */
            LogicValue cathode = sim_read_pin(netlist->sim, part, 0);
            LogicValue anode = sim_read_pin(netlist->sim, part, 1);
            int lit = anode == LOGIC_1 && cathode == LOGIC_0;
            text_append(&text, "lit %zu %d\n", i, lit ? 1 : 0);
        } else if (led7_instance_led_count(part) > 0) {
            char glow[64];
            size_t count = led7_instance_glow(netlist->sim, part, netlist->step_start,
                                              glow, sizeof(glow));

            text_append(&text, "glow %zu %.*s\n", i, (int)count, glow);
        }
    }

    shorted = sim_shorted_net(netlist->sim);
    if (shorted != NULL) {
        for (i = 0; i < netlist->net_count; i++) {
            if (netlist->nets[i] == shorted) {
                text_append(&text, "shorted %zu\n", i);
            }
        }
    }

    if (sim_error(netlist->sim) != NULL) {
        text_append(&text, "error %s\n", sim_error(netlist->sim));
    }

    if (text.failed) {
        free(text.data);
        return NULL;
    }

    if (text.data == NULL) {
        text.data = copy_message("");
    }
    return text.data;
}

static const SimInstance *part_at(const NetlistSim *netlist, size_t part) {
    if (netlist == NULL || part >= netlist->part_count) {
        return NULL;
    }
    return netlist->parts[part];
}

int netlist_sim_memory_size(
    const NetlistSim *netlist,
    size_t part,
    uint32_t *word_count,
    unsigned *word_bits
) {
    const SimInstance *instance = part_at(netlist, part);

    return instance != NULL && sram_instance_size(instance, word_count, word_bits);
}

size_t netlist_sim_inspect_memory(
    const NetlistSim *netlist,
    size_t part,
    uint32_t start,
    size_t count,
    uint32_t *values,
    uint32_t *known
) {
    const SimInstance *instance = part_at(netlist, part);

    return instance != NULL ? sram_inspect(instance, start, count, values, known) : 0;
}

int netlist_sim_set_switch(NetlistSim *netlist, size_t part, size_t position, int on) {
    if (netlist == NULL || part >= netlist->part_count) {
        return 0;
    }
    return switch_set_position(netlist->sim, netlist->parts[part], position, on);
}

void netlist_sim_close(NetlistSim *netlist) {
    if (netlist == NULL) {
        return;
    }
    sim_destroy(netlist->sim);
    free(netlist->nets);
    free(netlist->parts);
    free(netlist->is_led);
    free(netlist->configs);
    free(netlist);
}
