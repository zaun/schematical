/*
 * export_parts <kind> <out.json>
 * Writes one part file from the old backend's tables; then run
 * tools/export/notes.py on it to fill in the datasheet notes.
 */

#include <stdio.h>
#include <string.h>

#include "export.h"
#include "export_gate.h"
#include "export_ff.h"

typedef struct {
    const char *kind;
    int (*run)(void);
} Exporter;

static const Exporter EXPORTERS[] = {
    {"supply", export_supply},
    {"led", export_led},
    {"oscillator", export_osc},
    {"gate", export_gate},
    {"dff", export_dff},
    {"jkff", export_jkff},
    {"switch", export_switch},
};

int main(int argc, char **argv) {
    size_t i;

    if (argc != 3) {
        fprintf(stderr, "usage: export_parts <kind> <out.json>\n");
        return 2;
    }
    for (i = 0; i < sizeof EXPORTERS / sizeof EXPORTERS[0]; i++) {
        if (strcmp(EXPORTERS[i].kind, argv[1]) == 0) {
            export_begin(argv[1]);
            if (EXPORTERS[i].run() != 0) {
                fprintf(stderr, "export_parts: %s failed\n", argv[1]);
                return 1;
            }
            return export_write(argv[2]) == 0 ? 0 : 1;
        }
    }
    fprintf(stderr, "export_parts: unknown kind %s\n", argv[1]);
    return 2;
}
