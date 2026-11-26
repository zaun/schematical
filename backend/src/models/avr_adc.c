/*
 * The AVR analog-to-digital converter. A C port of avr8js 0.21.1's
 * src/peripherals/adc.ts (Copyright (c) Uri Shaked and contributors, MIT;
 * see LICENSE-avr8js). plan-62-C.
 *
 * As in avr8js: single conversions started by ADSC; no auto trigger.
 */

#include <math.h>
#include <string.h>

#include "avr.h"

enum {
    ADPS_MASK = 0x07, ADIE = 0x08, ADIF = 0x10, ADSC = 0x40, ADEN = 0x80,
    MUX_MASK = 0x1f, ADLAR = 0x20, MUX5 = 0x08, REFS2 = 0x08, REFS_MASK = 0x03, REFS_SHIFT = 6
};

static int adc_prescaler(const AvrAdc *adc) {
    switch (adc->cpu->data[adc->config.ADCSRA] & ADPS_MASK) {
        case 0:
        case 1: return 2;
        case 2: return 4;
        case 3: return 8;
        case 4: return 16;
        case 5: return 32;
        case 6: return 64;
        default: return 128;
    }
}

int avr_adc_sample_cycles(const AvrAdc *adc) {
    return adc->conversion_cycles * adc_prescaler(adc);
}

static double reference_voltage(const AvrAdc *adc) {
    uint8_t admux = adc->cpu->data[adc->config.ADMUX];
    unsigned refs = (admux >> REFS_SHIFT) & REFS_MASK;
    AvrAdcReference type;

    if (adc->config.reference_count > 4 && (admux & REFS2)) {
        refs |= 4;
    }
    type = refs < adc->config.reference_count ? adc->config.adc_references[refs] : AVR_ADC_RESERVED;
    if (!adc->cpu->exact && adc->config.refs2_mask != 0 && (admux & adc->config.refs2_mask) &&
        (refs & 2)) {
        /* The ATtiny25/45/85's REFS2 (ADMUX bit 4) with REFS1: the internal
           2.56 V reference (datasheet table 17-3). avr8js looks for REFS2 in
           bit 3, which is MUX3 on these chips, so its config cannot say
           this. `exact` keeps avr8js's answer for the oracle tests. */
        return 2.56;
    }
    switch (type) {
        case AVR_ADC_AVCC: return adc->avcc;
        case AVR_ADC_AREF: return adc->aref;
        case AVR_ADC_INTERNAL_1V1: return 1.1;
        case AVR_ADC_INTERNAL_2V56: return 2.56;
        default: return adc->avcc;
    }
}

void avr_adc_complete_read(AvrAdc *adc, int value) {
    AvrCpu *cpu = adc->cpu;

    adc->converting = 0;
    adc->conversion_cycles = 13;
    if (cpu->data[adc->config.ADMUX] & ADLAR) {
        cpu->data[adc->config.ADCL] = (uint8_t)((value << 6) & 0xff);
        cpu->data[adc->config.ADCH] = (uint8_t)(value >> 2);
    } else {
        cpu->data[adc->config.ADCL] = (uint8_t)(value & 0xff);
        cpu->data[adc->config.ADCH] = (uint8_t)((value >> 8) & 0x3);
    }
    cpu->data[adc->config.ADCSRA] &= (uint8_t)~ADSC;
    avr_set_interrupt_flag(cpu, &adc->ADC);
}

static void complete_pending(void *ctx) {
    AvrAdc *adc = ctx;

    avr_adc_complete_read(adc, adc->pending_result);
}

static void complete_zero(void *ctx) {
    avr_adc_complete_read(ctx, 0);
}

static double channel_volts(const AvrAdc *adc, unsigned channel) {
    return channel < AVR_ADC_CHANNELS ? adc->channel_volts[channel] : 0;
}

/* avr8js's default onADCRead: the input's voltage against the reference. */
static void default_read(AvrAdc *adc, const AvrAdcInput *input) {
    double voltage = 0;
    double raw_value;
    double result;

    switch (input->type) {
        case AVR_ADC_CONSTANT:
            voltage = input->voltage;
            break;
        case AVR_ADC_SINGLE_ENDED:
            voltage = channel_volts(adc, input->channel);
            break;
        case AVR_ADC_DIFFERENTIAL:
            voltage = input->gain * (channel_volts(adc, input->positive_channel) -
                                     channel_volts(adc, input->negative_channel));
            break;
        case AVR_ADC_TEMPERATURE:
            voltage = 0.378125;   /* 25 degrees Celsius */
            break;
    }
    raw_value = (voltage / reference_voltage(adc)) * 1024;
    result = floor(raw_value);
    if (!(result > 0)) {
        result = 0;       /* also a NaN (0 V against a 0 V reference) */
    }
    if (result > 1023) {
        result = 1023;
    }
    adc->pending_result = (int)result;
    avr_add_clock_event(adc->cpu, complete_pending, adc, avr_adc_sample_cycles(adc));
}

static int adcsra_write(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    static const AvrAdcInput FALLBACK = {1, AVR_ADC_CONSTANT, 0, 0, 0, 0, 0};
    AvrAdc *adc = ctx;
    AvrCpu *cpu = adc->cpu;

    (void)addr;
    (void)mask;
    if ((value & ADEN) && !(old_value & ADEN)) {
        adc->conversion_cycles = 25;
    }
    cpu->data[adc->config.ADCSRA] = value;
    avr_update_interrupt_enable(cpu, &adc->ADC, value);
    if (!adc->converting && (value & ADSC)) {
        unsigned channel;
        const AvrAdcInput *input;

        if (!(value & ADEN)) {
            avr_add_clock_event(cpu, complete_zero, adc, avr_adc_sample_cycles(adc));
            return 1;
        }
        channel = cpu->data[adc->config.ADMUX] & MUX_MASK;
        if (cpu->data[adc->config.ADCSRB] & MUX5) {
            channel |= 0x20;
        }
        channel &= adc->config.mux_input_mask;
        input = channel < AVR_ADC_MUX_INPUTS && adc->config.mux_channels[channel].present
                    ? &adc->config.mux_channels[channel] : &FALLBACK;
        adc->converting = 1;
        if (adc->on_read.fn != NULL) {
            adc->on_read.fn(adc->on_read.ctx, input);
        } else {
            default_read(adc, input);
        }
        return 1;
    }
    return 0;
}

void avr_adc_reset(AvrAdc *adc) {
    adc->converting = 0;
    adc->conversion_cycles = 25;
}

void avr_adc_init(AvrAdc *adc, AvrCpu *cpu, const AvrAdcConfig *config) {
    memset(adc, 0, sizeof(*adc));
    adc->cpu = cpu;
    adc->config = *config;
    adc->avcc = 5;
    adc->aref = 5;
    adc->conversion_cycles = 25;
    adc->ADC.address = config->adc_interrupt;
    adc->ADC.flag_register = config->ADCSRA;
    adc->ADC.flag_mask = ADIF;
    adc->ADC.enable_register = config->ADCSRA;
    adc->ADC.enable_mask = ADIE;
    if (config->ADCSRA < cpu->data_size) {
        cpu->write_hooks[config->ADCSRA].fn = adcsra_write;
        cpu->write_hooks[config->ADCSRA].ctx = adc;
    }
}
