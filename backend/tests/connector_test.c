#include <stdio.h>

#include "circuit.h"
#include "parts.h"
#include "read_file.h"

int main(void) {
    PartLibrary *library = part_library_create();
    Circuit *circuit;
    size_t vcc, gnd, jack, positive, negative;

    if (load_parts_dir(library, PARTS_DIR) != 0) return 1;
    circuit = circuit_create(library);
    if (circuit == NULL || circuit_add_part(circuit, "5V", &vcc) != 0 ||
        circuit_add_part(circuit, "GND", &gnd) != 0 ||
        circuit_add_part(circuit, "PJ-002A", &jack) != 0 ||
        circuit_add_net(circuit, &positive) != 0 || circuit_add_net(circuit, &negative) != 0 ||
        circuit_connect(circuit, vcc, "1", positive) != 0 ||
        circuit_connect(circuit, jack, "2", positive) != 0 ||
        circuit_connect(circuit, gnd, "1", negative) != 0 ||
        circuit_connect(circuit, jack, "3", negative) != 0) {
        fprintf(stderr, "setup: %s\n", circuit != NULL ? circuit_error(circuit) : "out of memory");
        return 1;
    }
    if (circuit_start(circuit) != 0 || circuit_advance(circuit, 1) != 0 ||
        !circuit_shorted_net(circuit, &positive)) {
        fprintf(stderr, "normally-closed connector contact did not join sleeve and switch: %s\n",
                circuit_error(circuit));
        return 1;
    }
    circuit_destroy(circuit);
    puts("PASS: barrel jack normally closes its sleeve switch until a plug is inserted");

    /* A USB connector is passive: its terminals join nothing to anything. */
    {
        static const char USB[] =
            "{\"format\":\"schematical-parts\",\"version\":1,\"kind\":\"connector\",\"devices\":["
            "{\"id\":\"usb-test\",\"function\":\"USB-A receptacle\",\"description\":\"d\","
            "\"drawer\":[\"Connector\",\"USB\"],\"usb\":\"A\",\"ratedAmps\":1.5,"
            "\"pinouts\":{\"p\":[{\"number\":1,\"name\":\"VBUS\",\"role\":\"inout\"},"
            "{\"number\":2,\"name\":\"D-\",\"role\":\"inout\"},{\"number\":3,\"name\":\"D+\",\"role\":\"inout\"},"
            "{\"number\":4,\"name\":\"GND\",\"role\":\"inout\"}]},"
            "\"parts\":[{\"name\":\"USB-TEST\",\"package\":\"x\",\"pinout\":\"p\",\"footprint\":\"LED-DIP4\"}]}]}";
        char *error = NULL;
        size_t usb, a, b;
        const Part *part;

        if (load_parts(library, USB, "usb-test.json", &error) != 0) {
            fprintf(stderr, "usb fixture: %s\n", error != NULL ? error : "?");
            return 1;
        }
        part = part_library_find(library, "USB-TEST");
        circuit = circuit_create(library);
        if (part == NULL || part->connector == NULL || part->connector->usb != 'A' ||
            part->connector->rated_ma != 1500 || circuit == NULL ||
            circuit_add_part(circuit, "5V", &vcc) != 0 || circuit_add_part(circuit, "GND", &gnd) != 0 ||
            circuit_add_part(circuit, "USB-TEST", &usb) != 0 ||
            circuit_add_net(circuit, &a) != 0 || circuit_add_net(circuit, &b) != 0 ||
            circuit_connect(circuit, vcc, "1", a) != 0 || circuit_connect(circuit, usb, "1", a) != 0 ||
            circuit_connect(circuit, gnd, "1", b) != 0 || circuit_connect(circuit, usb, "4", b) != 0 ||
            circuit_start(circuit) != 0 || (circuit_advance(circuit, 1), circuit_shorted_net(circuit, &a))) {
            fprintf(stderr, "USB connector: %s\n", circuit != NULL ? circuit_error(circuit) : "no part");
            return 1;
        }
        circuit_destroy(circuit);
        puts("PASS: a USB connector loads and joins nothing");
    }
    part_library_destroy(library);
    return 0;
}
