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
    part_library_destroy(library);
    puts("PASS: barrel jack normally closes its sleeve switch until a plug is inserted");
    return 0;
}
