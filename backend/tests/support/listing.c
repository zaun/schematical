#include "listing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        case PART_DIODE: return "diode";
        case PART_CAPACITOR: return "capacitor";
        case PART_REGISTER: return "register";
        case PART_MCU: return "mcu";
        case PART_RESISTOR: return "resistor";
        case PART_TRANSISTOR: return "transistor";
        case PART_GENERIC: return "generic";
        case PART_SERIALMEM: return "serialmem";
        case PART_NIXIE: return "nixie";
        case PART_REGULATOR: return "regulator";
        case PART_CRYSTAL: return "crystal";
        case PART_DEKATRON: return "dekatron";
        case PART_LCD: return "lcd";
        case PART_LEDDRIVER: return "leddriver";
        case PART_LCDDRIVER: return "lcddriver";
        case PART_POWERSWITCH: return "powerswitch";
        case PART_PLD: return "pld";
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
/* ":low" when every alternative of the name is active low, ":low=A;B" (the
   flagged alternatives, name order) when some are, nothing when none. */
static void print_active_low(FILE *out, const PartPin *pin) {
    const char *start = pin->name;
    uint32_t all = 0;
    size_t i = 0;
    int first = 1;

    if (pin->active_low == 0) {
        return;
    }
    for (const char *c = pin->name; ; c++) {
        if (*c == '/' || *c == '\0') {
            all |= i < 32 ? 1u << i : 0;
            i++;
        }
        if (*c == '\0') {
            break;
        }
    }
    if ((pin->active_low & all) == all) {
        fputs(":low", out);
        return;
    }
    fputs(":low=", out);
    for (i = 0;; i++) {
        const char *slash = strchr(start, '/');
        size_t length = slash != NULL ? (size_t)(slash - start) : strlen(start);

        if (i < 32 && (pin->active_low & (1u << i))) {
            fprintf(out, "%s%.*s", first ? "" : ";", (int)length, start);
            first = 0;
        }
        if (slash == NULL) {
            break;
        }
        start = slash + 1;
    }
}

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
        case PART_REGISTER:
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
        case PART_LEDDRIVER:
        case PART_LCDDRIVER:
            fputs("display", out);
            break;
        case PART_DIODE:
            fprintf(out, "type=%s diodes=%zu", part->diode->type, part->diode->diode_count);
            if (part->diode->vz_mv > 0) {
                fprintf(out, " vz=%u pd=%u", (unsigned)part->diode->vz_mv,
                        (unsigned)part->diode->pd_mw);
            } else {
                fprintf(out, " vr=%u io=%u", (unsigned)part->diode->vr_mv,
                        (unsigned)part->diode->io_ma);
            }
            break;
        case PART_CAPACITOR:
            fprintf(out, "type=%s ff=%llu rated=%u", part->capacitor->type,
                    (unsigned long long)part->capacitor->capacitance_ff,
                    (unsigned)part->capacitor->rated_mv);
            if (part->capacitor->dielectric != NULL) {
                fprintf(out, " dielectric=%s", part->capacitor->dielectric);
            }
            if (part->capacitor->tolerance != NULL) {
                fprintf(out, " tol=%s", part->capacitor->tolerance);
            }
            break;
        case PART_MCU:
            fprintf(out, "core=%s clock=%llu flash=%u ram=%u", part->mcu->core,
                    (unsigned long long)part->mcu->max_hz, (unsigned)part->mcu->flash,
                    (unsigned)part->mcu->ram);
            if (part->mcu->eeprom != 0) {
                fprintf(out, " eeprom=%u", (unsigned)part->mcu->eeprom);
            }
            if (part->mcu->psram != 0) {
                fprintf(out, " psram=%u", (unsigned)part->mcu->psram);
            }
            break;
        case PART_RESISTOR:
            fprintf(out, "resistors=%zu milliohms=%llu tol=%u power=%u",
                    part->resistor->resistor_count,
                    (unsigned long long)part->resistor->milliohms,
                    (unsigned)part->resistor->tolerance_ppm, (unsigned)part->resistor->power_mw);
            break;
        case PART_GENERIC:
            /* No facts: a generic part is its pins and its footprint. */
            break;
        case PART_SERIALMEM:
            fprintf(out, "type=%s size=%u", part->serialmem->type, (unsigned)part->serialmem->size);
            if (part->serialmem->address_bytes != 0) {
                fprintf(out, " addr=%u", part->serialmem->address_bytes);
            }
            if (part->serialmem->page_bytes != 0) {
                fprintf(out, " page=%u", (unsigned)part->serialmem->page_bytes);
            }
            if (part->serialmem->write_ps != 0) {
                fprintf(out, " write_ps=%llu", (unsigned long long)part->serialmem->write_ps);
            }
            if (part->serialmem->jedec_id != NULL) {
                fprintf(out, " id=%s", part->serialmem->jedec_id);
            }
            if (part->serialmem->max_clock_khz != 0) {
                fprintf(out, " clock_khz=%u", (unsigned)part->serialmem->max_clock_khz);
            }
            break;
        case PART_NIXIE: {
            const PartNixie *n = part->nixie;
            size_t i;

            fprintf(out, "color=%s size=%s view=%s strike=%u sustain=%u current=%u-%u glyphs=",
                    n->color, n->size, n->view, (unsigned)n->strike_mv, (unsigned)n->sustain_mv,
                    (unsigned)n->current_min_ua, (unsigned)n->current_max_ua);
            for (i = 0; i < n->glyph_count; i++) {
                fprintf(out, "%s%s:%s", i > 0 ? "," : "", n->glyphs[i],
                        part->pins[n->cathode_pins[i]].designator);
            }
            fputs(" anodePins=", out);
            print_numbers(out, part->pins, n->anode_pins, n->anode_pin_count);
            break;
        }
        case PART_REGULATOR: {
            const PartRegulator *r = part->regulator;
            size_t i;

            fprintf(out, "regulators=%zu type=%s mixed=%u adjustable=%u vout=", r->regulator_count, r->type,
                    (unsigned)r->mixed, (unsigned)r->adjustable);
            for (i = 0; i < r->regulator_count; i++) {
                fprintf(out, "%s%u", i > 0 ? "," : "", (unsigned)r->vout_mv[i]);
            }
            fputs(" vref=", out);
            for (i = 0; i < r->regulator_count; i++) {
                fprintf(out, "%s%u", i > 0 ? "," : "", (unsigned)r->vref_mv[i]);
            }
            fprintf(out, " voutMax=%u vinMin=%u vinMax=%u dropout=%u iout=%u iq=%u fsw=%llu",
                    (unsigned)r->vout_max_mv, (unsigned)r->vin_min_mv, (unsigned)r->vin_max_mv,
                    (unsigned)r->dropout_mv, (unsigned)r->iout_ma, (unsigned)r->iq_ua,
                    (unsigned long long)r->fsw_hz);
            break;
        }
        case PART_DEKATRON: {
            const PartDekatron *d = part->dekatron;
            size_t i;

            fprintf(out, "drive=%s color=%s size=%s view=%s strike=%u sustain=%u current=%u-%u rate=%u anode=%s cathodes=",
                    d->two_guide ? "double" : "single", d->color, d->size, d->view,
                    (unsigned)d->strike_mv, (unsigned)d->sustain_mv, (unsigned)d->current_min_ua,
                    (unsigned)d->current_max_ua, (unsigned)d->rate_hz,
                    part->pins[d->anode_pin].designator);
            for (i = 0; i < 10; i++) {
                fprintf(out, "%s%zu:%s", i > 0 ? "," : "", i, part->pins[d->position_pin[i]].designator);
            }
            fputs(" guides=", out);
            for (i = 0; i < d->guide_count; i++) {
                fprintf(out, "%s%s", i > 0 ? "," : "", part->pins[d->guide_pins[i]].designator);
            }
            break;
        }
        case PART_POWERSWITCH: {
            const PartPowerSwitch *w = part->powerswitch;

            fprintf(out, "switches=%zu type=%s vinMin=%u vinMax=%u iout=%u ilimit=%u ron=%u iq=%u", w->switch_count,
                    w->type, (unsigned)w->vin_min_mv, (unsigned)w->vin_max_mv, (unsigned)w->iout_ma,
                    (unsigned)w->ilimit_ma, (unsigned)w->ron_mohm, (unsigned)w->iq_ua);
            break;
        }
        case PART_PLD:
            fprintf(out, "macrocells=%u tpd=%llu", (unsigned)part->pld->macrocells,
                    (unsigned long long)part->pld->tpd_ps);
            break;
        case PART_CRYSTAL:
            fprintf(out, "type=%s hz=%llu load=%llu tol=%u stab=%u esr=%llu cap=%d",
                    part->crystal->type, (unsigned long long)part->crystal->hz,
                    (unsigned long long)part->crystal->load_ff,
                    (unsigned)part->crystal->tolerance_ppm, (unsigned)part->crystal->stability_ppm,
                    (unsigned long long)part->crystal->esr_mohm, part->crystal->capacitors);
            break;
        case PART_TRANSISTOR:
            fprintf(out,
                    "elements=%zu type=%s mixed=%u vmax=%u imax=%u power=%u hfe=%u rdson=%llu "
                    "vgsth=%u vgs=%u logic=%u r1=%llu r2=%llu",
                    part->transistor->element_count, part->transistor->type,
                    (unsigned)part->transistor->mixed, (unsigned)part->transistor->vmax_mv,
                    (unsigned)part->transistor->imax_ma, (unsigned)part->transistor->power_mw,
                    (unsigned)part->transistor->hfe,
                    (unsigned long long)part->transistor->rdson_mohm,
                    (unsigned)part->transistor->vgsth_mv, (unsigned)part->transistor->vgs_mv,
                    (unsigned)part->transistor->logic_level,
                    (unsigned long long)part->transistor->r1_mohm,
                    (unsigned long long)part->transistor->r2_mohm);
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
            if (part->dip_switch->momentary) fprintf(out, " momentary");
            if (part->dip_switch->normally_closed) fprintf(out, " nc");
            if (part->dip_switch->joined > 0) fprintf(out, " joined=%u", part->dip_switch->joined);
            if (part->dip_switch->body_um[0] > 0) {
                fprintf(out, " body=%ux%uum", (unsigned)part->dip_switch->body_um[0],
                        (unsigned)part->dip_switch->body_um[1]);
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
        case PART_LCD:
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
            if (d->type != NULL && strcmp(d->type, "led") != 0) {
                fprintf(out, " type=%s", d->type);
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

            size_t k;

            fprintf(out, "columns=%u rows=%u", l->columns, l->rows);
            if (l->enables == 2) {
                fprintf(out, " enables=2");
            }
            fprintf(out, " controller=%s rom=%s panel=%s backlight=", l->controller, l->rom,
                    l->panel);
            if (l->backlight_count == 0) {
                fprintf(out, "none");
            }
            for (k = 0; k < l->backlight_count; k++) {
                fprintf(out, "%s%s", k > 0 ? "," : "", l->backlight_colors[k]);
            }
            fprintf(out, " bus=");
            print_numbers(out, part->pins, l->bus_pins, l->bus_pin_count);
            break;
        }
        case PART_SERIAL:
            fprintf(out, "model=%s channels=%u grade=%s", part->serial->model,
                    part->serial->channels, part->serial->grade);
            break;
        case PART_CONNECTOR:
            if (part->connector->test_point) {
                fprintf(out, "testpoint=1");
                break;
            }
            if (part->connector->card != NULL) {
                fprintf(out, "card=%s pitch_um=%u", part->connector->card, (unsigned)part->connector->card_pitch_um);
                break;
            }
            if (part->connector->usb != 0) {
                fprintf(out, "usb=%c rated=%uV/%umA", part->connector->usb,
                        (unsigned)(part->connector->rated_mv / 1000), (unsigned)part->connector->rated_ma);
                break;
            }
            if (part->connector->terminal != 0) {   /* a PCB terminal block (plan-45) */
                const char terminal = part->connector->terminal;
                fprintf(out, "terminal=%s pitch_um=%u rated=%uV/%umA",
                        terminal == 's' ? "screw" : terminal == 'c' ? "spring" : terminal == 'p' ? "push-in" : "pluggable",
                        (unsigned)part->connector->pitch_um, (unsigned)(part->connector->rated_mv / 1000),
                        (unsigned)part->connector->rated_ma);
                break;
            }
            if (part->connector->jst != 0) {         /* a JST board header (plan-45): the series' first letter + "H" */
                fprintf(out, "jst=%cH pitch_um=%u rated=%uV/%umA", part->connector->jst,
                        (unsigned)part->connector->pitch_um, (unsigned)(part->connector->rated_mv / 1000),
                        (unsigned)part->connector->rated_ma);
                break;
            }
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
                part->kind == PART_ARITH || part->kind == PART_REGISTER;

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

        fprintf(out, "%s%s:%s:%s:%s%s", i > 0 ? "," : "", pin->designator, pin->name,
                role_name(pin->role), pin->group,
                pin->side == PIN_SIDE_IN ? ":in" : pin->side == PIN_SIDE_OUT ? ":out" : "");
        print_active_low(out, pin);
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
    /* Last field: a device with "simulated": false (plan-36-F). */
    if (!part->simulated) {
        fprintf(out, "|unsimulated");
    }
    fclose(out);
    return text;
}
