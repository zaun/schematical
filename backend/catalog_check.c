/*
 * Runs catalog_validate() and prints the errors for every item that has any.
 * Also checks that VCC shorted to GND blows up the simulation, which spans two
 * items and so is not part of the per-item report.
 */

#include <stdio.h>
#include <string.h>

#include "catalog_validate.h"
#include "sim.h"
#include "supply.h"

static size_t check_short_circuit(void) {
    SupplyConfig vcc_config = {SUPPLY_VCC};
    SupplyConfig gnd_config = {SUPPLY_GND};
    const SimComponentType *vcc = supply_lookup("VCC", &vcc_config);
    const SimComponentType *gnd = supply_lookup("GND", &gnd_config);
    size_t failures = 0;

    /* Shorted: both sources on one net. The run must stop with smoke. */
    {
        Sim *sim = sim_create();
        SimNet *net = sim_add_net(sim, "SHORT");
        SimInstance *v = sim_add_instance(sim, vcc, "V1", &vcc_config);
        SimInstance *g = sim_add_instance(sim, gnd, "G1", &gnd_config);
        int ok = sim && net && v && g && sim_connect(sim, net, v, 0) &&
                 sim_connect(sim, net, g, 0) && sim_start(sim);
        int run = ok ? sim_run_until(sim, 1000) : 1;

        /* The sim API returns 1 on success and 0 on failure. */
        if (run != 0 || sim_error(sim) == NULL ||
            strncmp(sim_error(sim), "smoke:", 6) != 0 ||
            sim_shorted_net(sim) != net) {
            fprintf(stderr, "FAIL short: VCC on GND did not blow up\n");
            failures++;
        }
        sim_destroy(sim);
    }

    /* Not shorted: each source on its own net. The run must succeed. */
    {
        Sim *sim = sim_create();
        SimNet *a = sim_add_net(sim, "A");
        SimNet *b = sim_add_net(sim, "B");
        SimInstance *v = sim_add_instance(sim, vcc, "V1", &vcc_config);
        SimInstance *g = sim_add_instance(sim, gnd, "G1", &gnd_config);
        int ok = sim && a && b && v && g && sim_connect(sim, a, v, 0) &&
                 sim_connect(sim, b, g, 0) && sim_start(sim);
        int run = ok ? sim_run_until(sim, 1000) : 0;

        if (run != 1 || sim_shorted_net(sim) != NULL) {
            fprintf(stderr, "FAIL separate VCC and GND nets blew up\n");
            failures++;
        }
        sim_destroy(sim);
    }

    return failures;
}

int main(void) {
    CatalogErrorReport report = catalog_validate();
    size_t total_errors = catalog_report_error_total(&report);
    size_t items_with_errors = 0;
    size_t short_failures;
    size_t i;
    size_t j;

    for (i = 0; i < report.item_count; i++) {
        const CatalogItemErrors *item = &report.items[i];

        if (item->error_count == 0) {
            continue;
        }
        items_with_errors++;
        printf("%s:\n", item->item);
        for (j = 0; j < item->error_count; j++) {
            printf("  - %s\n", item->errors[j]);
        }
    }

    short_failures = check_short_circuit();
    total_errors += short_failures;

    printf("%zu items checked, %zu with errors, %zu total errors\n",
           report.item_count, items_with_errors, total_errors);

    catalog_report_free(&report);
    return total_errors == 0 ? 0 : 1;
}
