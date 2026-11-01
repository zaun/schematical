#include "catalog_validate.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "catalog.h"
#include "led.h"
#include "sim.h"
#include "supply.h"

/* ---- Report building ---------------------------------------------------- */

typedef struct {
    CatalogItemErrors *items;
    size_t count;
    size_t capacity;
} ReportBuilder;

static char *copy_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);

    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}

/*
 * Starts an entry for one item. The returned pointer stays valid only until
 * the next begin_item() call, so add errors to it before starting another.
 */
static CatalogItemErrors *begin_item(ReportBuilder *builder, const char *name) {
    CatalogItemErrors *item;

    if (builder->count == builder->capacity) {
        size_t capacity = builder->capacity == 0 ? 64 : builder->capacity * 2;
        CatalogItemErrors *items = realloc(builder->items,
                                           capacity * sizeof(*items));

        if (items == NULL) {
            return NULL;
        }
        builder->items = items;
        builder->capacity = capacity;
    }

    item = &builder->items[builder->count++];
    item->item = copy_text(name);
    item->errors = NULL;
    item->error_count = 0;
    return item;
}

static void add_error(CatalogItemErrors *item, const char *format, ...) {
    char message[512];
    char **errors;
    va_list args;

    if (item == NULL) {
        return;
    }

    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    errors = realloc(item->errors, (item->error_count + 1) * sizeof(*errors));
    if (errors == NULL) {
        return;
    }
    item->errors = errors;
    item->errors[item->error_count] = copy_text(message);
    if (item->errors[item->error_count] != NULL) {
        item->error_count++;
    }
}

static const char *value_name(LogicValue value) {
    switch (value) {
        case LOGIC_0: return "0";
        case LOGIC_1: return "1";
        case LOGIC_X: return "X";
        case LOGIC_Z: return "Z";
    }
    return "?";
}

/* ---- Pin tables --------------------------------------------------------- */

/* Pins must be numbered 1..count, each exactly once. */
static void check_pin_table(CatalogItemErrors *item, const ChipPin *pins, size_t count) {
    size_t i;
    size_t number;

    for (i = 0; i < count; i++) {
        if (pins[i].number < 1 || pins[i].number > count) {
            add_error(item, "pin %u is outside 1..%zu", pins[i].number, count);
        }
    }

    for (number = 1; number <= count; number++) {
        size_t matches = 0;

        for (i = 0; i < count; i++) {
            if (pins[i].number == number) {
                matches++;
            }
        }
        if (matches != 1) {
            add_error(item, "pin %zu appears %zu times", number, matches);
        }
    }
}

/* ---- Child-process checks ------------------------------------------------
 * A part number whose configuration the model does not support can abort
 * inside the model, so resolving and instantiating runs in a child process.
 */

typedef int (*ChildCheck)(const void *context);

typedef struct {
    int exit_code;   /* valid when signal_number == 0 */
    int signal_number;
    int forked;      /* 0 when fork() itself failed */
} ChildResult;

static ChildResult run_in_child(ChildCheck check, const void *context) {
    ChildResult result = {0, 0, 0};
    pid_t pid = fork();
    int status = 0;

    if (pid < 0) {
        return result;
    }
    if (pid == 0) {
        _exit(check(context));
    }
    if (waitpid(pid, &status, 0) < 0) {
        return result;
    }

    result.forked = 1;
    if (WIFSIGNALED(status)) {
        result.signal_number = WTERMSIG(status);
    } else {
        result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    return result;
}

/* Exit codes shared by the child checks below. */
enum {
    CHILD_OK = 0,
    CHILD_NOT_RESOLVED = 2,
    CHILD_PIN_COUNT_MISMATCH = 3,
    CHILD_NO_SIM = 4,
    CHILD_NO_INSTANCE = 5,
};

static void report_child(CatalogItemErrors *item, ChildResult result) {
    if (!result.forked) {
        add_error(item, "could not run the model check");
        return;
    }
    if (result.signal_number != 0) {
        add_error(item, "model rejected the configuration (signal %d)",
                  result.signal_number);
        return;
    }
    switch (result.exit_code) {
        case CHILD_OK:
            return;
        case CHILD_NOT_RESOLVED:
            add_error(item, "part number does not resolve to a model");
            return;
        case CHILD_PIN_COUNT_MISMATCH:
            add_error(item, "model pin count does not match the pin table");
            return;
        case CHILD_NO_SIM:
            add_error(item, "simulation could not be created");
            return;
        case CHILD_NO_INSTANCE:
            add_error(item, "model instance could not be added");
            return;
        default:
            add_error(item, "model check exited with status %d", result.exit_code);
            return;
    }
}

/* Chip part number: lookup, pin count, and instance creation. */
typedef struct {
    const ChipDefinition *chip;
    const char *part_number;
} ChipPartContext;

static int check_chip_part(const void *raw) {
    const ChipPartContext *context = raw;
    SimLogicConfig config;
    const SimComponentType *type = context->chip->lookup(context->part_number, &config);
    Sim *sim;
    SimInstance *instance;

    if (type == NULL) {
        return CHILD_NOT_RESOLVED;
    }
    if (type->pin_count(&config) != context->chip->pin_count) {
        return CHILD_PIN_COUNT_MISMATCH;
    }
    sim = sim_create();
    if (sim == NULL) {
        return CHILD_NO_SIM;
    }
    instance = sim_add_instance(sim, type, "U1", &config);
    sim_destroy(sim);
    return instance != NULL ? CHILD_OK : CHILD_NO_INSTANCE;
}

typedef struct {
    const char *part_number;
} LedPartContext;

static int check_led_part(const void *raw) {
    const LedPartContext *context = raw;
    LedConfig config;
    const SimComponentType *type = led_lookup(context->part_number, &config);
    Sim *sim;
    SimInstance *instance;

    if (type == NULL) {
        return CHILD_NOT_RESOLVED;
    }
    if (type->pin_count(&config) != led_pin_count(config.package)) {
        return CHILD_PIN_COUNT_MISMATCH;
    }
    sim = sim_create();
    if (sim == NULL) {
        return CHILD_NO_SIM;
    }
    instance = sim_add_instance(sim, type, "D1", &config);
    sim_destroy(sim);
    return instance != NULL ? CHILD_OK : CHILD_NO_INSTANCE;
}

/* ---- Chips -------------------------------------------------------------- */

static void validate_chips(ReportBuilder *builder) {
    size_t i;

    for (i = 0; i < catalog_definition_count(); i++) {
        const ChipDefinition *chip = catalog_definition_at(i);
        CatalogItemErrors *item;
        size_t j;

        item = begin_item(builder, chip->id);
        check_pin_table(item, chip->pins, chip->pin_count);

        for (j = 0; j < chip->part_number_count; j++) {
            char name[128];
            ChipPartContext context;
            ChildResult result;

            snprintf(name, sizeof(name), "%s/%s", chip->id, chip->part_numbers[j]);
            context.chip = chip;
            context.part_number = chip->part_numbers[j];

            item = begin_item(builder, name);
            result = run_in_child(check_chip_part, &context);
            report_child(item, result);
        }
    }
}

/* ---- Generic LEDs ------------------------------------------------------- */

static void validate_leds(ReportBuilder *builder) {
    size_t i;

    for (i = 0; i < led_part_count(); i++) {
        const char *name = led_part_name_at(i);
        LedConfig config;
        CatalogItemErrors *item;
        LedPartContext context;
        ChildResult result;

        item = begin_item(builder, name);
        if (led_part_config_at(i, &config) == 0) {
            check_pin_table(item, led_pins(config.package), led_pin_count(config.package));
        }

        context.part_number = name;
        result = run_in_child(check_led_part, &context);
        report_child(item, result);
    }
}

/* ---- Supplies ----------------------------------------------------------- */

static void check_supply_level(CatalogItemErrors *item, const SimComponentType *type,
                               const SupplyConfig *config, LogicValue expected) {
    Sim *sim = sim_create();
    SimNet *net;
    SimInstance *instance;
    LogicValue value;

    if (sim == NULL) {
        add_error(item, "simulation could not be created");
        return;
    }

    net = sim_add_net(sim, "N");
    instance = sim_add_instance(sim, type, "S1", config);

    /* The sim API returns 1 on success and 0 on failure. */
    if (net == NULL || instance == NULL || sim_connect(sim, net, instance, 0) == 0 ||
        sim_start(sim) == 0 || sim_run_until(sim, 1000) == 0) {
        add_error(item, "simulation setup or run failed");
        sim_destroy(sim);
        return;
    }

    value = sim_read_pin(sim, instance, 0);
    if (value != expected) {
        add_error(item, "pin reads %s, expected %s",
                  value_name(value), value_name(expected));
    }
    sim_destroy(sim);
}

static void validate_supplies(ReportBuilder *builder) {
    size_t i;

    for (i = 0; i < supply_part_count(); i++) {
        const char *name = supply_part_name_at(i);
        SupplyConfig config;
        const SimComponentType *type;
        CatalogItemErrors *item;

        item = begin_item(builder, name);
        if (supply_part_config_at(i, &config) != 0 ||
            (type = supply_lookup(name, &config)) == NULL) {
            add_error(item, "does not resolve to a model");
            continue;
        }

        check_pin_table(item, supply_pins(config.kind), 1);
        if (type->pin_count(&config) != 1) {
            add_error(item, "model pin count is not 1");
            continue;
        }

        check_supply_level(item, type, &config,
                           config.kind == SUPPLY_VCC ? LOGIC_1 : LOGIC_0);
    }
}

/* ---- Public API --------------------------------------------------------- */

CatalogErrorReport catalog_validate(void) {
    ReportBuilder builder = {NULL, 0, 0};
    CatalogErrorReport report = {NULL, 0};

    validate_chips(&builder);
    validate_leds(&builder);
    validate_supplies(&builder);

    report.items = builder.items;
    report.item_count = builder.count;
    return report;
}

size_t catalog_report_error_total(const CatalogErrorReport *report) {
    size_t total = 0;
    size_t i;

    if (report == NULL) {
        return 0;
    }
    for (i = 0; i < report->item_count; i++) {
        total += report->items[i].error_count;
    }
    return total;
}

void catalog_report_free(CatalogErrorReport *report) {
    size_t i;
    size_t j;

    if (report == NULL || report->items == NULL) {
        return;
    }
    for (i = 0; i < report->item_count; i++) {
        CatalogItemErrors *item = &report->items[i];

        for (j = 0; j < item->error_count; j++) {
            free(item->errors[j]);
        }
        free(item->errors);
        free(item->item);
    }
    free(report->items);
    report->items = NULL;
    report->item_count = 0;
}
