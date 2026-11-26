/*
 * circuit_misuse_test (plan-68-L): the circuit API on a part index or net
 * index it does not have, on a part of the wrong kind, before and after
 * start, and past the end of time: each returns what circuit.h documents
 * and, where it has one, sets its error.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

static int failures;

static void expect(int ok, const char *what) {
    printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        failures++;
    }
}

static int error_has(const Circuit *circuit, const char *text) {
    return strstr(circuit_error(circuit), text) != NULL;
}

/* A circuit holding one part `name`; its index in *index. */
static Circuit *with(const PartLibrary *library, const char *name, size_t *index) {
    Circuit *circuit = circuit_create(library);

    if (circuit_add_part(circuit, name, index) != 0) {
        printf("FAIL: %s: %s\n", name, circuit_error(circuit));
        failures++;
    }
    return circuit;
}

static void check_indexes(const PartLibrary *library) {
    size_t vcc;
    Circuit *circuit = with(library, "5V", &vcc);
    unsigned char lights[1];
    uint64_t on_ps[1];
    uint32_t words, values[1], known[1];
    unsigned bits;
    CpuState cpu;
    LcdState lcd;
    TimerState timer;
    RtcTime rtc;
    uint16_t frames[1];
    size_t net;

    expect(strcmp(circuit_error(NULL), "no circuit") == 0, "no circuit says so");
    expect(circuit_set_switch(circuit, 99, 1, 1) == -1 && error_has(circuit, "99"), "a switch that is not there");
    expect(circuit_part_lights(circuit, 99, lights, 1) == 0 && circuit_part_light_time(circuit, 99, on_ps, 1) == 0,
           "no lights on a part that is not there");
    expect(circuit_memory_size(circuit, 99, &words, &bits) == 0 &&
               circuit_memory_read(circuit, 99, 0, 1, values, known) == 0,
           "no memory on a part that is not there");
    expect(circuit_cpu_state(circuit, 99, &cpu) == -1 && circuit_lcd_state(circuit, 99, &lcd) == -1 &&
               circuit_timer_state(circuit, 99, 0, &timer) == -1 && circuit_rtc_time(circuit, 99, &rtc) == -1,
           "no state of a part that is not there");
    expect(circuit_serial_sent_count(circuit, 99, 0) == 0 && circuit_serial_sent(circuit, 99, 0, 0, frames, 1) == 0,
           "nothing sent by a part that is not there");
    expect(circuit_serial_sent(circuit, vcc, 0, 0, NULL, 1) == 0 && circuit_serial_sent(circuit, vcc, 0, 0, frames, 0) == 0,
           "nothing copied into no room");
    expect(circuit_serial_configure(circuit, 99, 0, 9600, 8, 0, 2, 0) == -1, "no port on a part that is not there");
    expect(circuit_serial_configure(circuit, vcc, 0, 9600, 8, 0, 2, 0) == -1 &&
               error_has(circuit, "is not a serial part"),
           "a supply has no port");
    expect(circuit_serial_send(circuit, 99, 0, (const uint8_t *)"x", 1) == -1, "nothing typed into a part that is not there");
    expect(circuit_add_net(circuit, &net) == 0 && circuit_connect(circuit, vcc, "1", net) == 0 && circuit_start(circuit) == 0,
           "the supply starts");
    expect(circuit_net_value(circuit, 99) == CIRCUIT_X, "a net that is not there reads X");
    expect(circuit_shorted_net(circuit, &net) == 0, "nothing shorted");
    expect(circuit_start(circuit) == -1, "it starts once");
    expect(circuit_advance(circuit, 1000) == 1 && circuit_advance(circuit, UINT64_MAX) == 0 &&
               error_has(circuit, "time overflow"),
           "time does not run past its end");
    circuit_destroy(circuit);
}

static void check_kinds(const PartLibrary *library) {
    size_t vcc, jack, uart, net;
    Circuit *circuit = with(library, "5V", &vcc);

    expect(circuit_set_connector_plug(circuit, vcc, 1) == -1 && error_has(circuit, "is not a connector"),
           "a supply has no plug");
    circuit_destroy(circuit);
    circuit = with(library, "PJ-037A", &jack);
    expect(circuit_set_connector_plug(circuit, jack, 1) == -1 && error_has(circuit, "has no switched contact"),
           "a jack with no switch has no plug state");
    circuit_destroy(circuit);
    circuit = with(library, "N82510", &uart);
    expect(circuit_add_net(circuit, &net) == 0 && circuit_start(circuit) == 0, "the UART starts");
    expect(circuit_serial_send(circuit, uart, 0, (const uint8_t *)"x", 0) == 0 &&
               circuit_serial_sent_count(circuit, uart, 0) == 0,
           "typing nothing sends nothing");
    circuit_destroy(circuit);
}

/* A switched jack (PJ-002A: Sleeve 2 and Switch 3 joined while unplugged)
   with 5 V on Sleeve and 3.3 V on Switch starts with a plug in; pulling
   the plug joins the two supplies, and the run stops on the clash. */
static void check_unplug_stops(const PartLibrary *library) {
    size_t jack, v5, v3, sleeve, sw;
    Circuit *circuit = with(library, "PJ-002A", &jack);
    VoltageProblem problem;

    circuit_add_part(circuit, "5V", &v5);
    circuit_add_part(circuit, "3V3", &v3);
    circuit_add_net(circuit, &sleeve);
    circuit_add_net(circuit, &sw);
    circuit_connect(circuit, v5, "1", sleeve);
    circuit_connect(circuit, v3, "1", sw);
    circuit_connect(circuit, jack, "2", sleeve);
    circuit_connect(circuit, jack, "3", sw);
    expect(circuit_set_connector_plug(circuit, jack, 1) == 0 && circuit_start(circuit) == 0 &&
               circuit_advance(circuit, 1000) == 1,
           "with a plug in, the two supplies are apart and it runs");
    expect(circuit_set_connector_plug(circuit, jack, 0) == 0 && circuit_advance(circuit, 1000) == 0 &&
               circuit_voltage_problem(circuit, &problem) == 1 && problem.kind == VOLTAGE_SUPPLY_CLASH,
           "pulling the plug joins 5 V and 3.3 V: the run stops on the clash");
    circuit_destroy(circuit);
}

int main(void) {
    PartLibrary *library = part_library_create();

    if (load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: the library does not load\n");
        part_library_destroy(library);
        return 1;
    }
    check_indexes(library);
    check_kinds(library);
    check_unplug_stops(library);
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
