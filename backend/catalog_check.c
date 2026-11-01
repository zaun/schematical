/*
 * Checks every chip definition against the models:
 *   - the pin table numbers 1..pin_count exactly once,
 *   - every listed part number resolves and creates a model instance whose
 *     pin count matches the definition's pin table.
 *
 * A part number whose family/package combination the model does not support
 * aborts inside the model, so each part is checked in a child process.
 */

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "catalog.h"

static int check_pin_table(const ChipDefinition *chip) {
    size_t number;
    int ok = 1;

    for (number = 1; number <= chip->pin_count; number++) {
        size_t i;
        size_t matches = 0;

        for (i = 0; i < chip->pin_count; i++) {
            if (chip->pins[i].number == number) {
                matches++;
            }
        }
        if (matches != 1) {
            fprintf(stderr, "FAIL %s: pin %zu appears %zu times\n",
                    chip->id, number, matches);
            ok = 0;
        }
    }

    return ok;
}

static int check_part(const ChipDefinition *chip, const char *part_number) {
    pid_t pid = fork();
    int status = 0;

    if (pid < 0) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        SimLogicConfig config;
        const SimComponentType *type = chip->lookup(part_number, &config);
        Sim *sim;
        SimInstance *instance;

        if (type == NULL) {
            _exit(2);
        }
        if (type->pin_count(&config) != chip->pin_count) {
            _exit(3);
        }

        sim = sim_create();
        if (sim == NULL) {
            _exit(4);
        }
        instance = sim_add_instance(sim, type, "U1", &config);
        sim_destroy(sim);
        _exit(instance != NULL ? 0 : 5);
    }

    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 0;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 1;
    }

    if (WIFSIGNALED(status)) {
        fprintf(stderr, "FAIL %s %s: model rejected the configuration (signal %d)\n",
                chip->id, part_number, WTERMSIG(status));
    } else {
        fprintf(stderr, "FAIL %s %s: exit status %d\n",
                chip->id, part_number, WEXITSTATUS(status));
    }
    return 0;
}

int main(void) {
    size_t parts = 0;
    size_t failures = 0;
    size_t i;

    for (i = 0; i < catalog_definition_count(); i++) {
        const ChipDefinition *chip = catalog_definition_at(i);
        size_t j;

        if (!check_pin_table(chip)) {
            failures++;
        }
        for (j = 0; j < chip->part_number_count; j++) {
            parts++;
            if (!check_part(chip, chip->part_numbers[j])) {
                failures++;
            }
        }
    }

    printf("%zu part numbers checked, %zu failures\n", parts, failures);
    return failures == 0 ? 0 : 1;
}
