/*
 * avr_periph_test (plan-62-B): the AVR peripherals and the chip assembly
 * (src/models/avr_gpio.c, avr_clock.c, avr_timer.c, avr_chip.c) against
 * avr8js, their oracle.
 *
 *   layout     The "atmega328" register layout, printed as JSON, must equal
 *              avr8js's own ATmega328P configs (oracle.js layout-check).
 *   scenarios  tests/avr/<name>.json: a program (tests/avr/src/<name>.asm,
 *              assembled by avr8js's assembler) and pin stimuli. The C chip
 *              runs it in exact mode and prints a trace: a line per
 *              instruction (pc, cycles, SREG, SP, a hash of the data
 *              memory) and a line per port output change. The trace must
 *              hash to the scenario's `run` line in tests/avr/golden.txt
 *              and, when CMake found node, equal the trace oracle.js prints
 *              from avr8js running the same program on the same layout.
 *
 *   avr_periph_test                       run the checks
 *   avr_periph_test --layout <name> <flash bytes>   print a layout as JSON
 *   avr_periph_test --trace <scenario>    print the C side's trace
 *   avr_periph_test --print-golden        print every `run` golden line from
 *                                         the oracle's output (needs node)
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avr_chip.h"
#include "cJSON.h"
#include "memory_image.h"
#include "read_file.h"

#define RUN_HZ 16000000u

static int failures;

typedef struct {
    const char *name;
    int needs_port_events;   /* the trace must hold a port output change */
    int needs_interrupts;    /* ... and a taken interrupt */
} Scenario;

static const Scenario SCENARIOS[] = {
    {"gpio_out", 1, 0},
    {"gpio_int", 0, 1},
    {"clkpr", 0, 0},
};

#define SCENARIO_COUNT (sizeof SCENARIOS / sizeof SCENARIOS[0])

/* ---- Text buffers and hashes ------------------------------------------------ */

typedef struct {
    char *text;
    size_t length, capacity;
} Text;

static void text_add(Text *t, const char *format, ...) {
    va_list args;
    int need;

    if (t->capacity - t->length < 256) {
        t->capacity = t->capacity ? t->capacity * 2 : 65536;
        t->text = realloc(t->text, t->capacity);
        if (t->text == NULL) {
            fprintf(stderr, "out of memory\n");
            exit(2);
        }
    }
    va_start(args, format);
    need = vsnprintf(t->text + t->length, t->capacity - t->length, format, args);
    va_end(args);
    t->length += (size_t)need;
}

static uint32_t fnv32(const uint8_t *bytes, size_t count) {
    uint32_t h = 0x811c9dc5u;
    size_t i;

    for (i = 0; i < count; i++) {
        h ^= bytes[i];
        h *= 0x01000193u;
    }
    return h;
}

static uint64_t fnv64(const char *text) {
    uint64_t h = 0xcbf29ce484222325ull;

    for (; *text != '\0'; text++) {
        h ^= (uint8_t)*text;
        h *= 0x100000001b3ull;
    }
    return h;
}

static unsigned long count_lines(const char *text) {
    unsigned long lines = 0;

    for (; *text != '\0'; text++) {
        lines += *text == '\n';
    }
    return lines;
}

/* ---- The layout as JSON (avr8js's field names) ------------------------------ */

static void layout_json(const AvrLayout *base, uint32_t flash_bytes, FILE *out) {
    AvrLayout layout;
    size_t i;
    int pin, last, k;

    avr_layout_scale(base, flash_bytes, &layout);
    fprintf(out, "{\"name\":\"%s\",\"flash\":%u,\n \"ports\":[", layout.name, (unsigned)flash_bytes);
    for (i = 0; i < layout.port_count; i++) {
        const AvrPortConfig *port = &layout.ports[i].config;

        fprintf(out, "%s\n  {\"letter\":\"%c\",\"PIN\":%u,\"DDR\":%u,\"PORT\":%u,", i ? "," : "",
                layout.ports[i].letter, port->PIN, port->DDR, port->PORT);
        if (port->has_pin_change) {
            const AvrPinChangeConfig *pc = &port->pin_change;

            fprintf(out,
                    "\"pinChange\":{\"PCIE\":%u,\"PCICR\":%u,\"PCIFR\":%u,\"PCMSK\":%u,"
                    "\"pinChangeInterrupt\":%u,\"mask\":%u,\"offset\":%u},",
                    pc->PCIE, pc->PCICR, pc->PCIFR, pc->PCMSK, pc->pin_change_interrupt, pc->mask,
                    pc->offset);
        }
        for (last = -1, pin = 0; pin < 8; pin++) {
            if (port->external[pin].present) {
                last = pin;
            }
        }
        fprintf(out, "\"externalInterrupts\":[");
        for (pin = 0; pin <= last; pin++) {
            const AvrExternalInterruptConfig *e = &port->external[pin];

            if (!e->present) {
                fprintf(out, "%snull", pin ? "," : "");
            } else {
                fprintf(out,
                        "%s{\"EICR\":%u,\"EIMSK\":%u,\"EIFR\":%u,\"iscOffset\":%u,\"index\":%u,"
                        "\"interrupt\":%u}",
                        pin ? "," : "", e->EICR, e->EIMSK, e->EIFR, e->isc_offset, e->index,
                        e->interrupt);
            }
        }
        fprintf(out, "]}");
    }
    fprintf(out, "],\n \"clock\":{\"CLKPR\":%u},\n \"timers\":[", layout.clock.CLKPR);
    for (i = 0; i < layout.timer_count; i++) {
        const AvrTimerConfig *t = &layout.timers[i];

        fprintf(out, "%s\n  {\"bits\":%d,\"dividers\":[", i ? "," : "", t->bits);
        for (k = 0; k < 8; k++) {
            fprintf(out, "%s%u", k ? "," : "", t->dividers[k]);
        }
        fprintf(out,
                "],\"captureInterrupt\":%u,\"compAInterrupt\":%u,\"compBInterrupt\":%u,"
                "\"compCInterrupt\":%u,\"ovfInterrupt\":%u,"
                "\"TIFR\":%u,\"OCRA\":%u,\"OCRB\":%u,\"OCRC\":%u,\"ICR\":%u,\"TCNT\":%u,"
                "\"TCCRA\":%u,\"TCCRB\":%u,\"TCCRC\":%u,\"TIMSK\":%u,"
                "\"TOV\":%u,\"OCFA\":%u,\"OCFB\":%u,\"OCFC\":%u,"
                "\"TOIE\":%u,\"OCIEA\":%u,\"OCIEB\":%u,\"OCIEC\":%u,"
                "\"compPortA\":%u,\"compPinA\":%u,\"compPortB\":%u,\"compPinB\":%u,"
                "\"compPortC\":%u,\"compPinC\":%u,\"externalClockPort\":%u,\"externalClockPin\":%u}",
                t->capture_interrupt, t->comp_a_interrupt, t->comp_b_interrupt, t->comp_c_interrupt,
                t->ovf_interrupt, t->TIFR, t->OCRA, t->OCRB, t->OCRC, t->ICR, t->TCNT, t->TCCRA,
                t->TCCRB, t->TCCRC, t->TIMSK, t->TOV, t->OCFA, t->OCFB, t->OCFC, t->TOIE, t->OCIEA,
                t->OCIEB, t->OCIEC, t->comp_port_a, t->comp_pin_a, t->comp_port_b, t->comp_pin_b,
                t->comp_port_c, t->comp_pin_c, t->external_clock_port, t->external_clock_pin);
    }
    fprintf(out, "]}\n");
}

/* ---- Running a scenario on the C chip ----------------------------------------- */

typedef struct {
    Text *out;
    AvrChip *chip;
    char letter;
    uint16_t ddr;
} PortTrace;

static void port_listener(void *ctx, uint8_t value, uint8_t old_value) {
    PortTrace *trace = ctx;

    (void)old_value;
    text_add(trace->out, "p %llx %c %02x %02x\n", (unsigned long long)trace->chip->cpu->cycles,
             trace->letter, value, trace->chip->cpu->data[trace->ddr]);
}

typedef struct {
    unsigned long port_events, interrupts;
} RunCounts;

static char *scenario_path(const char *name, const char *suffix, char *out, size_t cap) {
    snprintf(out, cap, "%s/%s%s", AVR_DIR, name, suffix);
    return out;
}

/* Runs tests/avr/<name>.json on the C chip; the trace into out. 0 on success. */
static int run_scenario(const char *name, Text *out, RunCounts *counts, char *layout_name,
                        size_t layout_cap, uint32_t *flash_out) {
    char path[1024];
    char *json_text = read_file(scenario_path(name, ".json", path, sizeof path));
    cJSON *json = json_text != NULL ? cJSON_Parse(json_text) : NULL;
    const cJSON *item, *stimuli, *stimulus;
    const AvrLayout *layout;
    AvrChip *chip;
    PortTrace traces[AVR_LAYOUT_PORTS];
    uint32_t flash, sram, eeprom, *image;
    char *error = NULL;
    const char *program;
    long steps, step;
    int stimulus_count, next = 0;
    size_t i;

    counts->port_events = 0;
    counts->interrupts = 0;
    if (json == NULL) {
        printf("FAIL: %s: cannot read or parse %s\n", name, path);
        free(json_text);
        return -1;
    }
    item = cJSON_GetObjectItemCaseSensitive(json, "layout");
    layout = cJSON_IsString(item) ? avr_layout_find(item->valuestring) : NULL;
    flash = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "flash"));
    sram = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "sram"));
    eeprom = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "eeprom"));
    steps = (long)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(json, "steps"));
    program = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "program"));
    stimuli = cJSON_GetObjectItemCaseSensitive(json, "stimuli");
    stimulus_count = cJSON_IsArray(stimuli) ? cJSON_GetArraySize(stimuli) : 0;
    if (layout == NULL || program == NULL || flash == 0 || steps <= 0) {
        printf("FAIL: %s: the scenario needs layout, flash, steps and program\n", name);
        cJSON_Delete(json);
        free(json_text);
        return -1;
    }
    snprintf(layout_name, layout_cap, "%s", layout->name);
    *flash_out = flash;

    chip = avr_chip_create(layout, flash, sram, eeprom, RUN_HZ, 0);
    chip->cpu->exact = 1;
    image = malloc(flash * sizeof *image);
    if (memory_image_parse((const unsigned char *)program, strlen(program), MEMORY_IMAGE_INTEL_HEX,
                           flash, 8, image, &error) != 0) {
        printf("FAIL: %s: program: %s\n", name, error);
        free(error);
        free(image);
        avr_chip_destroy(chip);
        cJSON_Delete(json);
        free(json_text);
        return -1;
    }
    for (i = 0; i < flash; i++) {
        chip->cpu->flash[i] = (uint8_t)image[i];
    }
    free(image);
    for (i = 0; i < chip->port_count; i++) {
        traces[i].out = out;
        traces[i].chip = chip;
        traces[i].letter = chip->layout.ports[i].letter;
        traces[i].ddr = chip->layout.ports[i].config.DDR;
        chip->ports[i].listener.fn = port_listener;
        chip->ports[i].listener.ctx = &traces[i];
    }

    for (step = 0; step < steps; step++) {
        AvrCpu *cpu = chip->cpu;
        uint32_t pc_before_tick;

        while (next < stimulus_count) {
            const char *kind;

            stimulus = cJSON_GetArrayItem(stimuli, next);
            if ((uint64_t)cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 0)) > cpu->cycles) {
                break;
            }
            next++;
            kind = cJSON_GetStringValue(cJSON_GetArrayItem(stimulus, 1));
            if (kind != NULL && strcmp(kind, "pin") == 0) {
                const char *letter = cJSON_GetStringValue(cJSON_GetArrayItem(stimulus, 2));
                AvrPort *port = letter != NULL ? avr_chip_port(chip, letter[0]) : NULL;

                if (port == NULL) {
                    printf("FAIL: %s: stimulus %d names no port\n", name, next - 1);
                    failures++;
                    continue;
                }
                avr_port_set_pin(port, (int)cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 3)),
                                 cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 4)) != 0);
            } else {
                printf("FAIL: %s: unknown stimulus %d\n", name, next - 1);
                failures++;
            }
        }
        avr_instruction(cpu);
        pc_before_tick = cpu->pc;
        avr_cpu_tick(cpu);
        counts->interrupts += cpu->pc != pc_before_tick;
        text_add(out, "i %x %llx %02x %04x %08x\n", (unsigned)cpu->pc, (unsigned long long)cpu->cycles,
                 cpu->data[AVR_SREG], avr_get16(cpu, AVR_SP), fnv32(cpu->data, cpu->data_size));
    }
    avr_chip_destroy(chip);
    cJSON_Delete(json);
    free(json_text);
    {
        const char *p;

        for (p = out->text; p != NULL && *p != '\0'; p = strchr(p, '\n'), p = p ? p + 1 : NULL) {
            counts->port_events += *p == 'p';
        }
    }
    return 0;
}

/* ---- The golden file ----------------------------------------------------------- */

static int read_run_golden(const char *name, unsigned long *lines, unsigned long long *hash) {
    char path[1024], line[256], found_name[128];
    FILE *file = fopen(scenario_path("golden", ".txt", path, sizeof path), "r");
    int found = 0;

    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof line, file) != NULL) {
        if (sscanf(line, "run %127s %lu %llx", found_name, lines, hash) == 3 &&
            strcmp(found_name, name) == 0) {
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}

/* ---- The oracle ------------------------------------------------------------------- */

#ifdef AVR_NODE
/* The oracle's whole output for a command line (after the script's name). */
static char *oracle_output(const char *arguments, int *status) {
    char command[4096];
    Text text = {0};
    char chunk[65536];
    FILE *pipe;
    size_t n;

    snprintf(command, sizeof command, "\"%s\" \"%s\" %s", AVR_NODE, AVR_ORACLE, arguments);
    pipe = popen(command, "r");
    if (pipe == NULL) {
        *status = -1;
        return NULL;
    }
    text_add(&text, "%s", "");
    while ((n = fread(chunk, 1, sizeof chunk - 1, pipe)) > 0) {
        chunk[n] = '\0';
        while (text.capacity - text.length < n + 1) {
            text.capacity *= 2;
            text.text = realloc(text.text, text.capacity);
        }
        memcpy(text.text + text.length, chunk, n + 1);
        text.length += n;
    }
    *status = pclose(pipe);
    return text.text;
}

/* Writes a layout's JSON to a file in the build directory; its path in out. */
static void write_layout_file(const char *layout_name, uint32_t flash, char *out, size_t cap) {
    FILE *file;

    snprintf(out, cap, "%s/avr-layout-%s-%u.json", AVR_TMP, layout_name, (unsigned)flash);
    file = fopen(out, "w");
    if (file == NULL) {
        fprintf(stderr, "cannot write %s\n", out);
        exit(2);
    }
    layout_json(avr_layout_find(layout_name), flash, file);
    fclose(file);
}

static char *oracle_run(const char *name, const char *layout_name, uint32_t flash, int *status) {
    char scenario[1024], layout_file[1024], arguments[2200];

    write_layout_file(layout_name, flash, layout_file, sizeof layout_file);
    snprintf(arguments, sizeof arguments, "run \"%s\" \"%s\"",
             scenario_path(name, ".json", scenario, sizeof scenario), layout_file);
    return oracle_output(arguments, status);
}

/* Prints the first line where two traces differ. */
static void print_difference(const char *name, const char *ours, const char *theirs) {
    unsigned long line = 1;

    while (*ours != '\0' && *theirs != '\0') {
        size_t a = strcspn(ours, "\n"), b = strcspn(theirs, "\n");

        if (a != b || strncmp(ours, theirs, a) != 0) {
            printf("FAIL: %s: line %lu: avr8js \"%.*s\" ours \"%.*s\"\n", name, line, (int)b, theirs,
                   (int)a, ours);
            return;
        }
        ours += a + (ours[a] == '\n');
        theirs += b + (theirs[b] == '\n');
        line++;
    }
    printf("FAIL: %s: line %lu: one trace ends (avr8js %s, ours %s)\n", name, line,
           *theirs ? "goes on" : "ends", *ours ? "goes on" : "ends");
}
#endif

/* ---- Checks ------------------------------------------------------------------------ */

static void layout_case(const char *layout_name, uint32_t flash) {
#ifdef AVR_NODE
    char layout_file[1024], arguments[1100], expected[128];
    char *output;
    int status;

    write_layout_file(layout_name, flash, layout_file, sizeof layout_file);
    snprintf(arguments, sizeof arguments, "layout-check \"%s\" 2>&1", layout_file);
    output = oracle_output(arguments, &status);
    snprintf(expected, sizeof expected, "layout %s equal\n", layout_name);
    if (output == NULL || status != 0 || strcmp(output, expected) != 0) {
        printf("FAIL: layout %s: %s", layout_name, output != NULL ? output : "(the oracle did not run)\n");
        failures++;
    } else {
        printf("layout %s equal\n", layout_name);
    }
    free(output);
#else
    printf("layout %s: not checked (node not found: avr8js was not run)\n", layout_name);
    (void)flash;
#endif
}

#ifdef AVR_NODE
/* tests/avr/<name>.json must be what oracle.js build makes of its sources. */
static void build_case(const char *name) {
    char asm_path[1024], meta_path[1024], json_path[1024], arguments[2200];
    char *built, *committed;
    int status;

    snprintf(asm_path, sizeof asm_path, "%s/src/%s.asm", AVR_DIR, name);
    snprintf(meta_path, sizeof meta_path, "%s/src/%s.meta.json", AVR_DIR, name);
    snprintf(arguments, sizeof arguments, "build \"%s\" \"%s\"", asm_path, meta_path);
    built = oracle_output(arguments, &status);
    committed = read_file(scenario_path(name, ".json", json_path, sizeof json_path));
    if (built == NULL || status != 0 || committed == NULL || strcmp(built, committed) != 0) {
        printf("FAIL: %s: %s is not `oracle.js build` of src/%s.asm and src/%s.meta.json\n", name,
               json_path, name, name);
        failures++;
    }
    free(built);
    free(committed);
}
#endif

static void scenario_case(const Scenario *scenario) {
    const char *name = scenario->name;
    Text ours = {0};
    RunCounts counts;
    char layout_name[64];
    uint32_t flash = 0;
    unsigned long lines, golden_lines = 0;
    unsigned long long golden_hash = 0;
    uint64_t hash;
    int before = failures;
    int live = 0;

    text_add(&ours, "%s", "");
    if (run_scenario(name, &ours, &counts, layout_name, sizeof layout_name, &flash) != 0) {
        failures++;
        free(ours.text);
        return;
    }
    lines = count_lines(ours.text);
    hash = fnv64(ours.text);
#ifdef AVR_NODE
    {
        int status;
        char *theirs = oracle_run(name, layout_name, flash, &status);

        live = 1;
        if (theirs == NULL || status != 0) {
            printf("FAIL: %s: the oracle did not run\n", name);
            failures++;
        } else if (strcmp(ours.text, theirs) != 0) {
            print_difference(name, ours.text, theirs);
            failures++;
        }
        free(theirs);
        build_case(name);
    }
#endif
    if (!read_run_golden(name, &golden_lines, &golden_hash)) {
        printf("FAIL: %s: no `run %s` line in golden.txt\n", name, name);
        failures++;
    } else if (golden_lines != lines || golden_hash != hash) {
        printf("FAIL: %s: %lu lines hash %016llx; the golden says %lu lines hash %016llx\n", name,
               lines, (unsigned long long)hash, golden_lines, golden_hash);
        failures++;
    }
    if (scenario->needs_port_events && counts.port_events == 0) {
        printf("FAIL: %s: the trace has no port output change\n", name);
        failures++;
    }
    if (scenario->needs_interrupts && counts.interrupts == 0) {
        printf("FAIL: %s: the trace has no taken interrupt\n", name);
        failures++;
    }
    if (failures == before) {
        printf("ok %s (%lu lines, %lu port events, %lu interrupts%s)\n", name, lines,
               counts.port_events, counts.interrupts,
               live ? "" : "; golden hash only, node not found");
    }
    free(ours.text);
}

static int print_golden(void) {
#ifdef AVR_NODE
    size_t i;

    for (i = 0; i < SCENARIO_COUNT; i++) {
        Text ours = {0};
        RunCounts counts;
        char layout_name[64];
        uint32_t flash = 0;
        char *theirs;
        int status;

        text_add(&ours, "%s", "");
        if (run_scenario(SCENARIOS[i].name, &ours, &counts, layout_name, sizeof layout_name,
                         &flash) != 0) {
            return 1;
        }
        theirs = oracle_run(SCENARIOS[i].name, layout_name, flash, &status);
        if (theirs == NULL || status != 0) {
            fprintf(stderr, "%s: the oracle did not run\n", SCENARIOS[i].name);
            return 1;
        }
        printf("run %s %lu %016llx\n", SCENARIOS[i].name, count_lines(theirs),
               (unsigned long long)fnv64(theirs));
        free(theirs);
        free(ours.text);
    }
    return 0;
#else
    fprintf(stderr, "--print-golden needs node (CMake did not find it)\n");
    return 1;
#endif
}

int main(int argc, char **argv) {
    size_t i;

    if (argc == 4 && strcmp(argv[1], "--layout") == 0) {
        const AvrLayout *layout = avr_layout_find(argv[2]);

        if (layout == NULL) {
            fprintf(stderr, "unknown layout %s\n", argv[2]);
            return 2;
        }
        layout_json(layout, (uint32_t)strtoul(argv[3], NULL, 0), stdout);
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--trace") == 0) {
        Text ours = {0};
        RunCounts counts;
        char layout_name[64];
        uint32_t flash = 0;

        text_add(&ours, "%s", "");
        if (run_scenario(argv[2], &ours, &counts, layout_name, sizeof layout_name, &flash) != 0) {
            return 1;
        }
        fputs(ours.text, stdout);
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--print-golden") == 0) {
        return print_golden();
    }

    layout_case("atmega328", 32768);
    for (i = 0; i < SCENARIO_COUNT; i++) {
        scenario_case(&SCENARIOS[i]);
    }
    if (failures != 0) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    return 0;
}
