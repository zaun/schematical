/*
 * Checks the netlist bridge end to end: an LED lit by VCC, an LED left dark,
 * a VCC-to-GND short, and an LED blinking from a real oscillator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip_definition.h"
#include "netlist_sim.h"
#include "osc.h"

static int failures = 0;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static char *open_and_run(const char *netlist, SimTime run_ps) {
    char *error = NULL;
    NetlistSim *sim = netlist_sim_open(netlist, &error);
    char *snapshot = NULL;

    if (sim == NULL) {
        printf("  open failed: %s\n", error != NULL ? error : "(no message)");
        free(error);
        return NULL;
    }

    netlist_sim_advance(sim, run_ps);
    snapshot = netlist_sim_snapshot(sim);
    netlist_sim_close(sim);
    return snapshot;
}

static void check_lit_led(void) {
    /* VCC on the anode (pin 2), GND on the cathode (pin 1). */
    const char *netlist =
        "nets 2\n"
        "part 0 VCC\n"
        "part 1 GND\n"
        "part 2 LED-RED-0603\n"
        "pin 0 1 0\n"
        "pin 1 1 1\n"
        "pin 2 2 0\n"
        "pin 2 1 1\n";
    char *snapshot = open_and_run(netlist, 1000);

    expect(snapshot != NULL && strstr(snapshot, "lit 2 1\n") != NULL,
           "LED with anode high and cathode low is lit");
    free(snapshot);
}

static void check_dark_led(void) {
    /* The anode is left unconnected, so it reads Z and the LED stays dark. */
    const char *netlist =
        "nets 2\n"
        "part 0 VCC\n"
        "part 1 GND\n"
        "part 2 LED-GREEN-0603\n"
        "pin 0 1 0\n"
        "pin 1 1 1\n"
        "pin 2 1 1\n";
    char *snapshot = open_and_run(netlist, 1000);

    expect(snapshot != NULL && strstr(snapshot, "lit 2 0\n") != NULL,
           "LED with an unconnected anode is dark");
    free(snapshot);
}

static void check_short(void) {
    /* VCC and GND both drive net 0. */
    const char *netlist =
        "nets 1\n"
        "part 0 VCC\n"
        "part 1 GND\n"
        "pin 0 1 0\n"
        "pin 1 1 0\n";
    char *snapshot = open_and_run(netlist, 1000);

    expect(snapshot != NULL && strstr(snapshot, "shorted 0\n") != NULL &&
               strstr(snapshot, "error smoke:") != NULL,
           "VCC shorted to GND reports the net and a smoke error");
    free(snapshot);
}

static void check_unknown_part(void) {
    char *error = NULL;
    NetlistSim *sim = netlist_sim_open("nets 0\npart 0 NOT-A-PART\n", &error);

    expect(sim == NULL && error != NULL && strstr(error, "unknown part") != NULL,
           "unknown part name fails to open with a message");
    free(error);
    netlist_sim_close(sim);
}

static void check_oscillator_blinks(void) {
    size_t i;
    const char *name = NULL;
    OscConfig config;
    const ChipPin *pins;
    size_t pin_count;
    size_t j;
    unsigned out_number = 0;
    char netlist[512];
    NetlistSim *sim;
    char *error = NULL;
    int transitions = 0;
    int last = -1;
    SimTime step;

    /* Find the 2012 SiT1534 oscillator and its output pin. */
    for (i = 0; i < osc_part_count(); i++) {
        if (osc_part_config_at(i, &config) == 0 &&
            config.family == OSC_FAMILY_SIT1534 &&
            config.package == OSC_PACKAGE_2012) {
            name = osc_part_name_at(i);
            break;
        }
    }
    if (name == NULL) {
        expect(0, "find a SiT1534 2012 oscillator");
        return;
    }

    pins = osc_pins(config.family, config.package);
    pin_count = osc_pin_count(config.family, config.package);
    for (j = 0; j < pin_count; j++) {
        if (pins[j].role == PIN_ROLE_OUTPUT) {
            out_number = pins[j].number;
            break;
        }
    }

    /* Oscillator output drives the anode. The cathode goes to GND on net 1. */
    snprintf(netlist, sizeof(netlist),
             "nets 2\n"
             "part 0 %s\n"
             "part 1 GND\n"
             "part 2 LED-RED-0603\n"
             "pin 0 %u 0\n"
             "pin 1 1 1\n"
             "pin 2 2 0\n"
             "pin 2 1 1\n",
             name, out_number);

    sim = netlist_sim_open(netlist, &error);
    if (sim == NULL) {
        printf("  open failed: %s\n", error != NULL ? error : "(no message)");
        free(error);
        expect(0, "oscillator netlist opens");
        return;
    }

    /* About 1 ms of simulated time, in 1 us slices. */
    for (step = 0; step < 1000; step++) {
        char *snapshot;
        int lit;

        netlist_sim_advance(sim, 1000000);
        snapshot = netlist_sim_snapshot(sim);
        if (snapshot == NULL) {
            break;
        }
        lit = strstr(snapshot, "lit 2 1\n") != NULL;
        if (last != -1 && lit != last) {
            transitions++;
        }
        last = lit;
        free(snapshot);
    }
    netlist_sim_close(sim);

    expect(transitions >= 2, "LED blinks while the oscillator runs");
}

int main(void) {
    check_lit_led();
    check_dark_led();
    check_short();
    check_unknown_part();
    check_oscillator_blinks();

    if (failures > 0) {
        printf("%d check(s) failed\n", failures);
        return 1;
    }
    printf("all netlist checks passed\n");
    return 0;
}
