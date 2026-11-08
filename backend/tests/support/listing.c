#include "listing.h"

#include <stdio.h>
#include <stdlib.h>

const char *listing_kind_name(PartKind kind) {
    switch (kind) {
        case PART_GATE: return "gate";
        case PART_DFF: return "dff";
        case PART_JKFF: return "jkff";
        case PART_SRAM: return "sram";
        case PART_SWITCH: return "switch";
        case PART_LED: return "led";
        case PART_7SEG: return "7seg";
        case PART_OSCILLATOR: return "oscillator";
        case PART_SUPPLY: return "supply";
        case PART_EEPROM: return "eeprom";
        case PART_LATCH: return "latch";
        case PART_COUNTER: return "counter";
        case PART_SHIFT: return "shift";
        case PART_CPU: return "cpu";
        case PART_CHARLCD: return "charlcd";
        case PART_SERIAL: return "serial";
        case PART_CONNECTOR: return "connector";
        case PART_PIO: return "pio";
        case PART_DECODER: return "decoder";
        case PART_BUFFER: return "buffer";
        case PART_MUX: return "mux";
        case PART_HEADER: return "header";
        case PART_ARITH: return "arith";
        case PART_CONVERTER: return "converter";
        case PART_RTC: return "rtc";
    }
    return "?";
}

static const char *role_name(PinRole role) {
    static const char *const NAMES[] = {
        "input", "output", "inout", "power", "ground", "nc", "absent"
    };

    return (size_t)role < sizeof NAMES / sizeof NAMES[0] ? NAMES[role] : "?";
}

/* Prints the designators of the pins at these indices. */
static void print_numbers(FILE *out, const PartPin *pins, const size_t *indices, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) {
        fprintf(out, i > 0 ? ",%s" : "%s", pins[indices[i]].designator);
    }
}

static void print_details(FILE *out, const Part *part) {
    switch (part->kind) {
        case PART_GATE:
        case PART_DFF:
        case PART_JKFF:
        case PART_LATCH:
        case PART_COUNTER:
        case PART_SHIFT:
        case PART_DECODER:
        case PART_BUFFER:
        case PART_MUX:
        case PART_ARITH:
            fprintf(out, "units=%u", part->logic->units);
            break;
        case PART_HEADER: {
            size_t k;

            fprintf(out, "rows=%u genders=", part->header->rows);
            for (k = 0; k < part->header->gender_count; k++) {
                fprintf(out, "%s%s", k > 0 ? "," : "", part->header->genders[k]);
            }
            break;
        }
        case PART_CONVERTER:
            fprintf(out, "type=%s bits=%u channels=%u", part->converter->type,
                    part->converter->bits, part->converter->channels);
            break;
        case PART_RTC:
            fputs("clock", out);
            break;
        case PART_SRAM:
            fprintf(out, "words=%u bits=%u access_ps=%llu", (unsigned)part->sram->words,
                    part->sram->bits, (unsigned long long)part->sram->access_ps);
            break;
        case PART_SWITCH:
            fprintf(out, "positions=%u actuator=%s", part->dip_switch->positions,
                    part->dip_switch->actuator);
            if (part->dip_switch->rated_mv > 0) {
                fprintf(out, " rated=%u", (unsigned)part->dip_switch->rated_mv);
            }
            break;
        case PART_LED: {
            size_t k;

            fprintf(out, "colors=");
            for (k = 0; k < part->led->color_count; k++) {
                fprintf(out, "%s%s", k > 0 ? "," : "", part->led->colors[k]);
            }
            break;
        }
        case PART_7SEG: {
            const PartDisplay *d = part->display;
            size_t i;

            fprintf(out, "digits=%u commonAnode=%d colonAfter=%u dotAfter=%u color=%s size=%s chip=%s leds=",
                    d->digits, d->common_anode ? 1 : 0, d->colon_after, d->dot_after, d->color,
                    d->size, d->chip);
            for (i = 0; i < d->led_count; i++) {
                fprintf(out, "%s%u:%s", i > 0 ? "," : "", d->leds[i].digit, d->leds[i].segment);
            }
            fputs(" segmentPins=", out);
            print_numbers(out, part->pins, d->segment_pins, d->segment_pin_count);
            fputs(" commonPins=", out);
            print_numbers(out, part->pins, d->common_pins, d->common_pin_count);
            if (d->vf_max_mv > 0) {
                fprintf(out, " vf=%u-%u", (unsigned)d->vf_min_mv, (unsigned)d->vf_max_mv);
            }
            break;
        }
        case PART_OSCILLATOR: {
            const PartOscillator *o = part->oscillator;

            fprintf(out, "family=%s minHz=%llu maxHz=%llu defaultHz=%llu enablePin=", o->family,
                    (unsigned long long)o->min_hz, (unsigned long long)o->max_hz,
                    (unsigned long long)o->default_hz);
            if (o->enable_pin != NULL) {
                fputs(o->enable_pin, out);
            } else {
                fputs("none", out);
            }
            break;
        }
        case PART_SUPPLY:
            fprintf(out, "level=%d mv=%u", part->supply->level, (unsigned)part->supply->mv);
            if (part->supply->pull) {
                fputs(" pull", out);
            }
            break;
        case PART_EEPROM:
            fprintf(out, "words=%u bits=%u access_ps=%llu page=%u twc_ps=%llu",
                    (unsigned)part->eeprom->words, part->eeprom->bits,
                    (unsigned long long)part->eeprom->access_ps, part->eeprom->page_bytes,
                    (unsigned long long)part->eeprom->write_cycle_ps);
            break;
        case PART_CPU:
            fprintf(out, "core=%s grade=%s max_hz=%llu", part->cpu->core, part->cpu->grade,
                    (unsigned long long)part->cpu->max_hz);
            break;
        case PART_CHARLCD: {
            const PartCharLcd *l = part->charlcd;

            fprintf(out, "columns=%u rows=%u controller=%s rom=%s panel=%s backlight=%s bus=",
                    l->columns, l->rows, l->controller, l->rom, l->panel,
                    l->backlight_color != NULL ? l->backlight_color : "none");
            print_numbers(out, part->pins, l->bus_pins, l->bus_pin_count);
            break;
        }
        case PART_SERIAL:
            fprintf(out, "model=%s channels=%u grade=%s", part->serial->model,
                    part->serial->channels, part->serial->grade);
            break;
        case PART_CONNECTOR:
            fprintf(out, "center_um=%u rated=%uV/%umA contacts=%u",
                    (unsigned)part->connector->center_diameter_um,
                    (unsigned)(part->connector->rated_mv / 1000),
                    (unsigned)part->connector->rated_ma, part->connector->contacts);
            break;
        case PART_PIO:
            fprintf(out, "model=%s grade=%s max_hz=%llu", part->pio->model, part->pio->grade,
                    (unsigned long long)part->pio->max_hz);
            break;
    }
}

char *listing_line(const Part *part) {
    static const char *const FLAGS[] = {
        "obsolete", "military", "automotive", "enhanced", "ceramic"
    };
    char *text = NULL;
    size_t length = 0;
    FILE *out = open_memstream(&text, &length);
    size_t i;
    int first = 1;
    int logic = part->kind == PART_GATE || part->kind == PART_DFF || part->kind == PART_JKFF ||
                part->kind == PART_LATCH || part->kind == PART_COUNTER ||
                part->kind == PART_SHIFT || part->kind == PART_BUFFER || part->kind == PART_MUX ||
                part->kind == PART_ARITH;

    if (out == NULL) {
        return NULL;
    }
    fprintf(out, "%s|%s|%s|", part->name, listing_kind_name(part->kind), part->package);
    for (i = 0; i < part->drawer_count; i++) {
        fprintf(out, "%s%s", i > 0 ? "/" : "", part->drawer[i]);
    }
    fprintf(out, "|%s|%s|", part->function, logic ? part->logic->family : "");
    for (i = 0; i < sizeof FLAGS / sizeof FLAGS[0]; i++) {
        if (part->flags & (1u << i)) {
            fprintf(out, "%s%s", first ? "" : ",", FLAGS[i]);
            first = 0;
        }
    }
    fputc('|', out);
    print_details(out, part);
    fputc('|', out);
    for (i = 0; i < part->pin_count; i++) {
        const PartPin *pin = &part->pins[i];

        fprintf(out, "%s%s:%s:%s:%s", i > 0 ? "," : "", pin->designator, pin->name,
                role_name(pin->role), pin->group);
    }
    if (part->manufacturer != NULL) {
        fprintf(out, "|mfr=%s", part->manufacturer);
    }
    if (part->datasheet != NULL) {
        fprintf(out, "|datasheet=%s", part->datasheet->id);
    }
    if (part->footprint != NULL) {
        fprintf(out, "|footprint=%s", part->footprint->id);
    }
    if (part->connector_footprint != NULL) {
        fprintf(out, "|connector=%s", part->connector_footprint->id);
    }
    if (part->footprint_choice_count > 0) {
        size_t k;

        fprintf(out, "|choices=");
        for (k = 0; k < part->footprint_choice_count; k++) {
            fprintf(out, "%s%s:%s", k > 0 ? "," : "", part->footprint_choices[k].package,
                    part->footprint_choices[k].footprint->id);
        }
    }
    if (part->vcc != NULL) {
        fprintf(out, "|vcc=%u-%u,in=%u", (unsigned)part->vcc->min_mv, (unsigned)part->vcc->max_mv,
                (unsigned)part->vcc->input_max_mv);
    }
    /* Dual-supply parts (plan-20-F): the B side's range. */
    if (part->domain_count == 2 && part->domains[1].vcc != NULL) {
        const PartVoltage *b = part->domains[1].vcc;

        fprintf(out, "|vccb=%u-%u,in=%u", (unsigned)b->min_mv, (unsigned)b->max_mv,
                (unsigned)b->input_max_mv);
    }
    fclose(out);
    return text;
}
