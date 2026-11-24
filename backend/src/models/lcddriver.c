/*
 * The lcddriver kind (plan-56-B): LCD driver ICs (PCF8576, PCF8553, ...) on
 * the converter engine (converter.c) with a "display" section (display_out.c),
 * usually the "lcd" driver. The same loader as the leddriver kind;
 * docs/parts-format.md § lcddriver.
 */

#include "converter.h"

static const char *const DEVICE_KEYS[] = {
    "vcc", "inputMax", "analog", "bus", "configBus", "timing", "registers", "protocol", "pins",
    "display", NULL
};

static const char *const PART_KEYS[] = {"vcc", "inputMax", "i2cAddress", "i2c", "resets", NULL};

static void *lcddriver_load_device(KindContext *ctx, const cJSON *json) {
    return cv_load_device(ctx, json, CV_MODE_DISPLAY);
}

const PartKindOps LCDDRIVER_KIND = {
    PART_LCDDRIVER, "lcddriver", &CONVERTER_COMPONENT, DEVICE_KEYS, PART_KEYS,
    lcddriver_load_device, cv_bind_pinout, cv_load_part, cv_pin_open_drain, cv_pin_analog
};
