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
 *   dasm       avr_dasm's text for every opcode word must assemble back to
 *              the same word(s) in avr8js's assembler (oracle.js dasm-check),
 *              and hash to the `dasm` line of golden.txt.
 *
 *   avr_periph_test --dasm                print the disassembly of every word
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
    int needs_interrupts;    /* ... a taken interrupt */
    int needs_serial;        /* ... a USART, SPI or TWI event */
    const char *vectors;     /* the vector numbers that must be taken, e.g. "1,16", or NULL */
} Scenario;

static const Scenario SCENARIOS[] = {
    {"gpio_out", 1, 0, 0, NULL},
    {"gpio_int", 0, 1, 0, NULL},
    {"clkpr", 0, 0, 0, NULL},
    {"timer0", 1, 1, 0, NULL},
    {"timer1", 1, 1, 0, NULL},
    {"timer2", 1, 1, 0, NULL},
    {"timer_ext", 1, 1, 0, NULL},
    {"usart", 0, 1, 1, "18,19,20"},
    {"spi", 0, 1, 1, "17"},
    {"twi", 0, 1, 1, "24"},
    {"adc", 0, 1, 0, NULL},
    {"eeprom", 0, 1, 0, NULL},
    {"watchdog", 0, 1, 0, "6"},
    {"m48_vectors", 1, 1, 1, "1,16,18"},
    {"m168pb_porte", 1, 0, 0, NULL},
    {"m328pb_extras", 1, 1, 1, "27,28,30,33,35,39,40,42,44"},
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
    fprintf(out, "],\n \"usarts\":[");
    for (i = 0; i < layout.usart_count; i++) {
        const AvrUsartConfig *u = &layout.usarts[i];

        fprintf(out,
                "%s\n  {\"rxCompleteInterrupt\":%u,\"dataRegisterEmptyInterrupt\":%u,"
                "\"txCompleteInterrupt\":%u,\"UCSRA\":%u,\"UCSRB\":%u,\"UCSRC\":%u,"
                "\"UBRRL\":%u,\"UBRRH\":%u,\"UDR\":%u}",
                i ? "," : "", u->rx_complete_interrupt, u->data_register_empty_interrupt,
                u->tx_complete_interrupt, u->UCSRA, u->UCSRB, u->UCSRC, u->UBRRL, u->UBRRH, u->UDR);
    }
    fprintf(out, "],\n \"spis\":[");
    for (i = 0; i < layout.spi_count; i++) {
        const AvrSpiConfig *sp = &layout.spis[i];

        fprintf(out, "%s\n  {\"spiInterrupt\":%u,\"SPCR\":%u,\"SPSR\":%u,\"SPDR\":%u}",
                i ? "," : "", sp->spi_interrupt, sp->SPCR, sp->SPSR, sp->SPDR);
    }
    fprintf(out, "],\n \"twis\":[");
    for (i = 0; i < layout.twi_count; i++) {
        const AvrTwiConfig *tw = &layout.twis[i];

        fprintf(out,
                "%s\n  {\"twiInterrupt\":%u,\"TWBR\":%u,\"TWSR\":%u,\"TWAR\":%u,\"TWDR\":%u,"
                "\"TWCR\":%u,\"TWAMR\":%u}",
                i ? "," : "", tw->twi_interrupt, tw->TWBR, tw->TWSR, tw->TWAR, tw->TWDR, tw->TWCR,
                tw->TWAMR);
    }
    fprintf(out,
            "],\n \"adc\":{\"ADMUX\":%u,\"ADCSRA\":%u,\"ADCSRB\":%u,\"ADCL\":%u,\"ADCH\":%u,"
            "\"DIDR0\":%u,\"adcInterrupt\":%u,\"numChannels\":%u,\"muxInputMask\":%u,"
            "\"muxChannels\":{",
            layout.adc.ADMUX, layout.adc.ADCSRA, layout.adc.ADCSRB, layout.adc.ADCL, layout.adc.ADCH,
            layout.adc.DIDR0, layout.adc.adc_interrupt, layout.adc.num_channels,
            layout.adc.mux_input_mask);
    for (k = 0, last = 0; k < AVR_ADC_MUX_INPUTS; k++) {
        const AvrAdcInput *input = &layout.adc.mux_channels[k];

        if (!input->present) {
            continue;
        }
        fprintf(out, "%s\"%d\":{\"type\":%d", last++ ? "," : "", k, (int)input->type);
        if (input->type == AVR_ADC_SINGLE_ENDED) {
            fprintf(out, ",\"channel\":%u", input->channel);
        } else if (input->type == AVR_ADC_CONSTANT) {
            fprintf(out, ",\"voltage\":%.17g", input->voltage);
        } else if (input->type == AVR_ADC_DIFFERENTIAL) {
            fprintf(out, ",\"positiveChannel\":%u,\"negativeChannel\":%u,\"gain\":%.17g",
                    input->positive_channel, input->negative_channel, input->gain);
        }
        fprintf(out, "}");
    }
    fprintf(out, "},\"adcReferences\":[");
    for (k = 0; k < layout.adc.reference_count; k++) {
        fprintf(out, "%s%d", k ? "," : "", (int)layout.adc.adc_references[k]);
    }
    fprintf(out,
            "]},\n \"eeprom\":{\"eepromReadyInterrupt\":%u,\"EECR\":%u,\"EEDR\":%u,\"EEARL\":%u,"
            "\"EEARH\":%u,\"eraseCycles\":%u,\"writeCycles\":%u},\n"
            " \"watchdog\":{\"watchdogInterrupt\":%u,\"MCUSR\":%u,\"WDTCSR\":%u}}\n",
            layout.eeprom.eeprom_ready_interrupt, layout.eeprom.EECR, layout.eeprom.EEDR,
            layout.eeprom.EEARL, layout.eeprom.EEARH, (unsigned)avr_eeprom_cycles(RUN_HZ),
            (unsigned)avr_eeprom_cycles(RUN_HZ), layout.watchdog.watchdog_interrupt,
            layout.watchdog.MCUSR, layout.watchdog.WDTCSR);
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

/* The scripted outside of a serial peripheral: the same on both sides
   (oracle.js run), so the traces can be compared. */
typedef struct {
    Text *out;
    AvrChip *chip;
    int index;
    /* TWI: the answer the handler gives 40 cycles after the request. */
    int twi_kind;        /* 's'tart, sto'p', 'c'onnect, 'w'rite, 'r'ead */
    int twi_ack;
    uint8_t twi_value;
    uint8_t twi_next;    /* the next byte a read returns: 0x10, 0x11, ... */
    uint8_t spi_reply;
} SerialTrace;

static void usart_transmit(void *ctx, uint8_t value) {
    SerialTrace *trace = ctx;

    text_add(trace->out, "u %llx %d %02x\n", (unsigned long long)trace->chip->cpu->cycles,
             trace->index, value);
}

static void spi_complete(void *ctx) {
    SerialTrace *trace = ctx;

    avr_spi_complete_transfer(&trace->chip->spis[trace->index], trace->spi_reply);
}

static void spi_byte(void *ctx, uint8_t value) {
    SerialTrace *trace = ctx;
    AvrSpi *spi = &trace->chip->spis[trace->index];

    text_add(trace->out, "s %llx %d %02x\n", (unsigned long long)trace->chip->cpu->cycles,
             trace->index, value);
    trace->spi_reply = value ^ 0xa5;
    avr_add_clock_event(trace->chip->cpu, spi_complete, trace, avr_spi_transfer_cycles(spi));
}

static void twi_answer(void *ctx) {
    SerialTrace *trace = ctx;
    AvrTwi *twi = &trace->chip->twis[trace->index];

    switch (trace->twi_kind) {
        case 's': avr_twi_complete_start(twi); break;
        case 'p': avr_twi_complete_stop(twi); break;
        case 'c': avr_twi_complete_connect(twi, trace->twi_ack); break;
        case 'w': avr_twi_complete_write(twi, 1); break;
        case 'r': avr_twi_complete_read(twi, trace->twi_value); break;
    }
}

static void twi_request(SerialTrace *trace, int kind, const char *name, unsigned argument) {
    text_add(trace->out, "t %llx %d %s %02x\n", (unsigned long long)trace->chip->cpu->cycles,
             trace->index, name, argument);
    trace->twi_kind = kind;
    avr_add_clock_event(trace->chip->cpu, twi_answer, trace, 40);
}

static void twi_start(void *ctx, int repeated) {
    twi_request(ctx, 's', "start", (unsigned)repeated);
}

static void twi_stop(void *ctx) {
    twi_request(ctx, 'p', "stop", 0);
}

static void twi_connect(void *ctx, uint8_t address, int write) {
    SerialTrace *trace = ctx;

    trace->twi_ack = address == 0x50;
    twi_request(trace, 'c', "connect", (unsigned)((address << 1) | (write ? 0 : 1)));
}

static void twi_write(void *ctx, uint8_t value) {
    twi_request(ctx, 'w', "write", value);
}

static void twi_read(void *ctx, int ack) {
    SerialTrace *trace = ctx;

    trace->twi_value = trace->twi_next++;
    twi_request(trace, 'r', "read", (unsigned)ack);
}

typedef struct {
    unsigned long port_events, interrupts, serial_events;
    unsigned char vector_taken[AVR_MAX_INTERRUPTS];   /* by vector number */
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
    SerialTrace usarts[AVR_LAYOUT_SERIALS], spis[AVR_LAYOUT_SERIALS], twis[AVR_LAYOUT_SERIALS];
    uint32_t flash, sram, eeprom, *image;
    char *error = NULL;
    const char *program;
    long steps, step;
    int stimulus_count, next = 0;
    size_t i;

    counts->port_events = 0;
    counts->interrupts = 0;
    counts->serial_events = 0;
    memset(counts->vector_taken, 0, sizeof counts->vector_taken);
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
    for (i = 0; i < chip->usart_count; i++) {
        memset(&usarts[i], 0, sizeof usarts[i]);
        usarts[i].out = out;
        usarts[i].chip = chip;
        usarts[i].index = (int)i;
        chip->usarts[i].on_transmit.fn = usart_transmit;
        chip->usarts[i].on_transmit.ctx = &usarts[i];
    }
    for (i = 0; i < chip->spi_count; i++) {
        memset(&spis[i], 0, sizeof spis[i]);
        spis[i].out = out;
        spis[i].chip = chip;
        spis[i].index = (int)i;
        chip->spis[i].on_byte.fn = spi_byte;
        chip->spis[i].on_byte.ctx = &spis[i];
    }
    for (i = 0; i < chip->twi_count; i++) {
        memset(&twis[i], 0, sizeof twis[i]);
        twis[i].out = out;
        twis[i].chip = chip;
        twis[i].index = (int)i;
        twis[i].twi_next = 0x10;
        chip->twis[i].handler.start = twi_start;
        chip->twis[i].handler.stop = twi_stop;
        chip->twis[i].handler.connect = twi_connect;
        chip->twis[i].handler.write_byte = twi_write;
        chip->twis[i].handler.read_byte = twi_read;
        chip->twis[i].handler.ctx = &twis[i];
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
            } else if (kind != NULL && strcmp(kind, "rx") == 0) {
                size_t usart = (size_t)cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 2));

                if (usart < chip->usart_count) {
                    avr_usart_write_byte(&chip->usarts[usart],
                                         (uint8_t)cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 3)),
                                         0);
                }
            } else if (kind != NULL && strcmp(kind, "adc") == 0) {
                size_t channel = (size_t)cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 2));

                if (channel < AVR_ADC_CHANNELS) {
                    chip->adc.channel_volts[channel] =
                        cJSON_GetNumberValue(cJSON_GetArrayItem(stimulus, 3)) / 1000;
                }
            } else {
                printf("FAIL: %s: unknown stimulus %d\n", name, next - 1);
                failures++;
            }
        }
        avr_instruction(cpu);
        pc_before_tick = cpu->pc;
        avr_cpu_tick(cpu);
        if (cpu->pc != pc_before_tick) {
            /* An interrupt was taken: pc is its vector's word address. */
            counts->interrupts++;
            if (cpu->pc / avr_vector_words(flash) < AVR_MAX_INTERRUPTS) {
                counts->vector_taken[cpu->pc / avr_vector_words(flash)] = 1;
            }
        }
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
            counts->serial_events += *p == 'u' || *p == 's' || *p == 't';
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
    if (scenario->vectors != NULL) {
        const char *p = scenario->vectors;

        while (*p != '\0') {
            char *end;
            unsigned long vector = strtoul(p, &end, 10);

            if (vector >= AVR_MAX_INTERRUPTS || !counts.vector_taken[vector]) {
                printf("FAIL: %s: vector %lu was never taken\n", name, vector);
                failures++;
            }
            p = *end == ',' ? end + 1 : end;
        }
    }
    if (scenario->needs_serial && counts.serial_events == 0) {
        printf("FAIL: %s: the trace has no USART, SPI or TWI event\n", name);
        failures++;
    }
    if (failures == before) {
        printf("ok %s (%lu lines, %lu port events, %lu interrupts, %lu serial events%s)\n", name,
               lines, counts.port_events, counts.interrupts, counts.serial_events,
               live ? "" : "; golden hash only, node not found");
    }
    free(ours.text);
}

/* Loads a scenario's program into a chip's flash. 0 on success. */
static int load_program(const char *name, AvrChip *chip) {
    char path[1024];
    char *json_text = read_file(scenario_path(name, ".json", path, sizeof path));
    cJSON *json = json_text != NULL ? cJSON_Parse(json_text) : NULL;
    const char *program = json != NULL
        ? cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "program")) : NULL;
    uint32_t flash = chip->cpu->flash_bytes;
    uint32_t *image = malloc(flash * sizeof *image);
    char *error = NULL;
    int result = -1;
    uint32_t i;

    if (program != NULL && memory_image_parse((const unsigned char *)program, strlen(program),
                                              MEMORY_IMAGE_INTEL_HEX, flash, 8, image,
                                              &error) == 0) {
        for (i = 0; i < flash; i++) {
            chip->cpu->flash[i] = (uint8_t)image[i];
        }
        result = 0;
    }
    free(error);
    free(image);
    cJSON_Delete(json);
    free(json_text);
    return result;
}

/*
 * Outside exact mode a watchdog reset restarts the whole chip, where avr8js
 * resets only the CPU. The `watchdog` program runs until it has started
 * twice; registers of other peripherals set before must be back at their
 * reset values, and the program must have seen WDRF.
 */
static void full_reset_case(void) {
    AvrChip *chip = avr_chip_create(avr_layout_find("atmega328"), 32768, 2048, 1024, RUN_HZ, 0);
    AvrCpu *cpu = chip->cpu;
    long step;
    int ok;

    if (load_program("watchdog", chip) != 0) {
        printf("FAIL: watchdog_full_reset: cannot load the program\n");
        failures++;
        avr_chip_destroy(chip);
        return;
    }
    avr_write_data(cpu, 0xc1, 0x18, 0xff);   /* UCSR0B: RXEN, TXEN */
    avr_write_data(cpu, 0x24, 0xff, 0xff);   /* DDRB: outputs */
    avr_write_data(cpu, 0x25, 0xaa, 0xff);   /* PORTB */
    avr_write_data(cpu, 0x44, 0x42, 0xff);   /* TCCR0A: CTC, OC0A toggles */
    avr_write_data(cpu, 0x45, 0x01, 0xff);   /* TCCR0B: running */
    for (step = 0; step < 200000 && cpu->data[21] < 2; step++) {
        avr_chip_step(chip);
    }
    /* A few instructions into the second start: r20 holds MCUSR as it was. */
    for (step = 0; step < 4; step++) {
        avr_chip_step(chip);
    }
    ok = cpu->data[21] == 2 && (cpu->data[20] & 0x08) != 0 && cpu->data[0xc1] == 0 &&
         cpu->data[0x24] == 0 && cpu->data[0x25] == 0 && cpu->data[0x44] == 0 &&
         cpu->data[0x45] == 0 && chip->ports[0].last_value == 0 &&
         avr_port_pin_state(&chip->ports[0], 1) == AVR_PIN_INPUT &&
         chip->timers[0].divider == 0 && cpu->data[0xc0] == 0x20;
    if (ok) {
        printf("ok watchdog_full_reset\n");
    } else {
        printf("FAIL: watchdog_full_reset: starts %u, MCUSR seen %02x, UCSR0B %02x, DDRB %02x, "
               "PORTB %02x, TCCR0A %02x, TCCR0B %02x, UCSR0A %02x\n",
               cpu->data[21], cpu->data[20], cpu->data[0xc1], cpu->data[0x24], cpu->data[0x25],
               cpu->data[0x44], cpu->data[0x45], cpu->data[0xc0]);
        failures++;
    }
    avr_chip_destroy(chip);
}

/* ---- The disassembler ---------------------------------------------------------- */

#define DASM_NEXT 0x1234
#define DASM_ADDRESS 0x100

/* One line per opcode word: the word, the next word, the text. */
static void dasm_text(Text *out) {
    unsigned word;
    char text[64];

    for (word = 0; word < 0x10000; word++) {
        avr_dasm((uint16_t)word, DASM_NEXT, DASM_ADDRESS, text, sizeof text);
        text_add(out, "%04x %04x %s\n", word, DASM_NEXT, text);
    }
}

static int read_dasm_golden(unsigned long *words, unsigned long long *hash) {
    char path[1024], line[256];
    FILE *file = fopen(scenario_path("golden", ".txt", path, sizeof path), "r");
    int found = 0;

    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof line, file) != NULL) {
        if (sscanf(line, "dasm %lu %llx", words, hash) == 2) {
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}

#ifdef AVR_NODE
/* The oracle's answer for our disassembly: "dasm: N words round-trip ..." */
static char *dasm_oracle(const char *ours, int *status) {
    char file_name[1024], arguments[1100];
    FILE *file;

    snprintf(file_name, sizeof file_name, "%s/avr-dasm.txt", AVR_TMP);
    file = fopen(file_name, "w");
    if (file == NULL) {
        *status = -1;
        return NULL;
    }
    fputs(ours, file);
    fclose(file);
    snprintf(arguments, sizeof arguments, "dasm-check \"%s\" 2>&1", file_name);
    return oracle_output(arguments, status);
}
#endif

static void dasm_case(void) {
    Text ours = {0};
    unsigned long golden_words = 0, words = 0;
    unsigned long long golden_hash = 0;
    int before = failures;
    int live = 0;
    char sample[64];

    text_add(&ours, "%s", "");
    dasm_text(&ours);
#ifdef AVR_NODE
    {
        int status;
        char *answer = dasm_oracle(ours.text, &status);

        live = 1;
        if (answer == NULL || status != 0 || sscanf(answer, "dasm: %lu words round-trip", &words) != 1) {
            printf("FAIL: dasm: %s", answer != NULL ? answer : "the oracle did not run\n");
            failures++;
        }
        free(answer);
    }
#endif
    if (!read_dasm_golden(&golden_words, &golden_hash)) {
        printf("FAIL: dasm: no `dasm` line in golden.txt\n");
        failures++;
    } else if (golden_hash != fnv64(ours.text) || (live && golden_words != words)) {
        printf("FAIL: dasm: %lu words hash %016llx; the golden says %lu words hash %016llx\n", words,
               (unsigned long long)fnv64(ours.text), golden_words, golden_hash);
        failures++;
    }
    /* A few fixed texts, so a reader sees the format. */
    avr_dasm(0xe5fa, 0, 0, sample, sizeof sample);
    if (strcmp(sample, "LDI R31, 0x5A") != 0) {
        printf("FAIL: dasm: e5fa is \"%s\"\n", sample);
        failures++;
    }
    avr_dasm(0xcffd, 0, 0, sample, sizeof sample);
    if (strcmp(sample, "RJMP .-6") != 0) {
        printf("FAIL: dasm: cffd is \"%s\"\n", sample);
        failures++;
    }
    if (avr_dasm(0x940c, 0x0080, 0, sample, sizeof sample) != 2 || strcmp(sample, "JMP 0x0100") != 0) {
        printf("FAIL: dasm: 940c 0080 is \"%s\"\n", sample);
        failures++;
    }
    avr_dasm(0xfff7, 0, 0, sample, sizeof sample);
    if (strcmp(sample, "SBRS R31, 7") != 0) {
        printf("FAIL: dasm: fff7 is \"%s\"\n", sample);
        failures++;
    }
    /* Erased flash (0xFFFF) is no instruction: the core runs it as a no-op. */
    avr_dasm(0xffff, 0, 0, sample, sizeof sample);
    if (strcmp(sample, ".DW 0xFFFF") != 0) {
        printf("FAIL: dasm: ffff is \"%s\"\n", sample);
        failures++;
    }
    if (failures == before) {
        printf("dasm: %lu words round-trip%s\n", live ? words : golden_words,
               live ? "" : " (golden hash only, node not found)");
    }
    free(ours.text);
}

/* The firmware the engine tests load (tests/avr/<name>.hex) must be what
   avr8js's assembler makes of tests/avr/src/<name>.asm. */
static void firmware_case(void) {
#ifdef AVR_NODE
    static const char *const NAMES[] = {"blink", "blink48", "blink_fast", "button", "pwm", "int0",
                                        "adc_port", "usart_hi", "usart_7e2", "spi_eeprom",
                                        "spi_mode3", "spi_noddr", "twi_rtc", "twi_nack", "twi1_rtc"};
    size_t i, bad = 0;

    for (i = 0; i < sizeof NAMES / sizeof NAMES[0]; i++) {
        char asm_path[1024], hex_path[1024], arguments[1100];
        char *built, *committed;
        int status;

        snprintf(asm_path, sizeof asm_path, "%s/src/%s.asm", AVR_DIR, NAMES[i]);
        snprintf(arguments, sizeof arguments, "asm \"%s\"", asm_path);
        built = oracle_output(arguments, &status);
        committed = read_file(scenario_path(NAMES[i], ".hex", hex_path, sizeof hex_path));
        if (built == NULL || status != 0 || committed == NULL || strcmp(built, committed) != 0) {
            printf("FAIL: firmware: %s is not `oracle.js asm` of src/%s.asm\n", hex_path, NAMES[i]);
            failures++;
            bad++;
        }
        free(built);
        free(committed);
    }
    if (bad == 0) {
        printf("ok firmware (%zu files equal their sources)\n", sizeof NAMES / sizeof NAMES[0]);
    }
#else
    printf("firmware: not checked (node not found)\n");
#endif
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
    {
        Text ours = {0};
        unsigned long words = 0;
        char *answer;
        int status;

        text_add(&ours, "%s", "");
        dasm_text(&ours);
        answer = dasm_oracle(ours.text, &status);
        if (answer == NULL || status != 0 || sscanf(answer, "dasm: %lu words round-trip", &words) != 1) {
            fprintf(stderr, "dasm: %s", answer != NULL ? answer : "the oracle did not run\n");
            return 1;
        }
        /* The count is the oracle's; the hash is of the text it accepted. */
        printf("dasm %lu %016llx\n", words, (unsigned long long)fnv64(ours.text));
        free(answer);
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
    if (argc == 2 && strcmp(argv[1], "--dasm") == 0) {
        Text ours = {0};

        text_add(&ours, "%s", "");
        dasm_text(&ours);
        fputs(ours.text, stdout);
        return 0;
    }

    layout_case("atmega328", 32768);
    for (i = 0; i < SCENARIO_COUNT; i++) {
        scenario_case(&SCENARIOS[i]);
    }
    full_reset_case();
    dasm_case();
    firmware_case();
    if (failures != 0) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    return 0;
}
