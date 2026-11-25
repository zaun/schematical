#ifndef TIMER_FIXTURE_H
#define TIMER_FIXTURE_H

/*
 * Fixture parts for timer_test (plan-48-E section 6), loaded next to the
 * real part files (footprints from footprints.json):
 *
 *   TESTTIMER    bipolar 555, maxHz 500000, tpd 100 ns, vcc 4.5-16 V, pinout p8 (PDIP-8)
 *   TESTTIMER2   cmos 556 (two sections), maxHz 2000000, tpd 50 ns, vcc 2-15 V, pinout p14 (PDIP-14)
 *
 * TIMER_DEVICE builds one device for the rejection cases.
 */

#include <stdio.h>
#include <stdlib.h>

#include "parts.h"

#define TFX_PIN(N, NAME, ROLE) "{\"number\":" #N ",\"name\":\"" NAME "\",\"role\":\"" ROLE "\"}"

/* The 555-8 pinout of plan-48-B section 3.1. */
#define TFX_P8                                                                                    \
    TFX_PIN(1, "GND", "ground") "," TFX_PIN(2, "TRIG", "input") "," TFX_PIN(3, "OUT", "output")   \
    "," TFX_PIN(4, "RESET", "input") "," TFX_PIN(5, "CONT", "input") ","                         \
    TFX_PIN(6, "THRES", "input") "," TFX_PIN(7, "DISCH", "output") "," TFX_PIN(8, "VCC", "power")

/* The 556-14 pinout. */
#define TFX_P14                                                                                   \
    TFX_PIN(1, "DISCH1", "output") "," TFX_PIN(2, "THRES1", "input") ","                         \
    TFX_PIN(3, "CONT1", "input") "," TFX_PIN(4, "RESET1", "input") ","                           \
    TFX_PIN(5, "OUT1", "output") "," TFX_PIN(6, "TRIG1", "input") "," TFX_PIN(7, "GND", "ground") \
    "," TFX_PIN(8, "TRIG2", "input") "," TFX_PIN(9, "OUT2", "output") ","                        \
    TFX_PIN(10, "RESET2", "input") "," TFX_PIN(11, "CONT2", "input") ","                         \
    TFX_PIN(12, "THRES2", "input") "," TFX_PIN(13, "DISCH2", "output") ","                       \
    TFX_PIN(14, "VCC", "power")

#define TFX_SEC1                                                                                  \
    "{\"trigger\":\"TRIG\",\"threshold\":\"THRES\",\"discharge\":\"DISCH\",\"output\":\"OUT\","    \
    "\"reset\":\"RESET\",\"control\":\"CONT\"}"
#define TFX_SEC1_2                                                                                \
    "{\"trigger\":\"TRIG1\",\"threshold\":\"THRES1\",\"discharge\":\"DISCH1\",\"output\":\"OUT1\"," \
    "\"reset\":\"RESET1\",\"control\":\"CONT1\"}"
#define TFX_SEC2                                                                                  \
    "{\"trigger\":\"TRIG2\",\"threshold\":\"THRES2\",\"discharge\":\"DISCH2\",\"output\":\"OUT2\"," \
    "\"reset\":\"RESET2\",\"control\":\"CONT2\"}"

/* PROCESS, TIMERS (the array text), PINS, KEYS (more device keys, with a leading comma). */
#define TIMER_DEVICE(ID, PROCESS, TIMERS, PINS, KEYS, NAME, FOOTPRINT)                           \
    "{\"id\":\"" ID "\",\"function\":\"fixture\",\"description\":\"fixture\",\"drawer\":[],"    \
    "\"pinouts\":{\"p\":[" PINS "]},\"process\":\"" PROCESS "\",\"timers\":[" TIMERS "]" KEYS ","  \
    "\"parts\":[{\"name\":\"" NAME "\",\"package\":\"DIP\",\"pinout\":\"p\","                  \
    "\"footprint\":\"" FOOTPRINT "\"}]}"
#define TIMER_FILE(DEVICES)                                                                       \
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"timer\",\"devices\":[" DEVICES "]}"

static const char TIMER_FIXTURE[] = TIMER_FILE(
    TIMER_DEVICE("tfx-555", "bipolar", TFX_SEC1, TFX_P8,
                 ",\"maxHz\":500000,\"tpd\":100,\"vcc\":[4.5,16]", "TESTTIMER", "PDIP-8") ","
    TIMER_DEVICE("tfx-556", "cmos", TFX_SEC1_2 "," TFX_SEC2, TFX_P14,
                 ",\"maxHz\":2000000,\"tpd\":50,\"vcc\":[2,15]", "TESTTIMER2", "PDIP-14"));

/* TESTTACT: a momentary two-terminal switch (A, B), the one-shot's trigger button. */
static const char TIMER_SWITCH_FIXTURE[] =
    "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"switch\",\"devices\":["
    "{\"id\":\"tfx-tact\",\"function\":\"Normal Open\",\"description\":\"fixture\","
    "\"drawer\":[\"Switch\",\"Tactile\"],\"pinouts\":{\"p\":[{\"number\":1,\"name\":\"A\",\"role\":\"inout\"},"
    "{\"number\":2,\"name\":\"B\",\"role\":\"inout\"}]},\"actuator\":\"Standard\","
    "\"contacts\":[[\"A\",\"B\"]],\"momentary\":true,\"ratedVolts\":24,"
    "\"parts\":[{\"name\":\"TESTTACT\",\"package\":\"SMD\",\"pinout\":\"p\",\"footprint\":\"RES-0402\"}]}]}";

/* Loads the fixture parts; prints the error and returns -1 on failure. */
static int load_timer_fixture(PartLibrary *library) {
    char *error = NULL;

    if (load_parts(library, TIMER_FIXTURE, "timer-fixture.json", &error) != 0) {
        fprintf(stderr, "timer fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    if (load_parts(library, TIMER_SWITCH_FIXTURE, "timer-switch-fixture.json", &error) != 0) {
        fprintf(stderr, "timer switch fixture: %s\n", error != NULL ? error : "(none)");
        free(error);
        return -1;
    }
    return 0;
}

#endif
