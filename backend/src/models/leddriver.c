/*
 * The leddriver kind (plan-56-B): LED display driver ICs (MAX6951, ICM7218,
 * AS1115, ...) on the converter engine (converter.c) with a "display" section
 * (display_out.c). The same loader as the lcddriver kind; docs/parts-format.md
 * § leddriver.
 */

#include "converter.h"

static const char *const DEVICE_KEYS[] = {
    "vcc", "inputMax", "analog", "bus", "configBus", "timing", "registers", "protocol", "pins",
    "display", NULL
};

static const char *const PART_KEYS[] = {"vcc", "inputMax", "i2cAddress", "i2c", "resets", NULL};

static void *leddriver_load_device(KindContext *ctx, const cJSON *json) {
    return cv_load_device(ctx, json, CV_MODE_DISPLAY);
}

const PartKindOps LEDDRIVER_KIND = {
    PART_LEDDRIVER, "leddriver", &CONVERTER_COMPONENT, DEVICE_KEYS, PART_KEYS,
    leddriver_load_device, cv_bind_pinout, cv_load_part, cv_pin_open_drain, cv_pin_analog
};
