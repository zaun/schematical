/*
 * cpu_bench: how fast the engine runs a CPU board (plan-09-D).
 *
 *     cpu_bench <sim ms>
 *
 * Builds the W65C02S board of cpu_test (W65C02S6TPG-14, AT28C256-15PU,
 * AS6C62256-55PCN, SN74HC00N; tests/support/cpu_board.c) with the counting
 * program looping forever (JMP $8007 in place of the STP), runs <sim ms> at
 * 1 MHz in 100 us steps and prints the speed against real time.
 *
 * Not a ctest (a wall-clock check would be flaky). Build it Release:
 *     cmake -S backend -B /tmp/logic-rel -DCMAKE_BUILD_TYPE=Release
 *     cmake --build /tmp/logic-rel --target cpu_bench
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cpu_board.h"
#include "cpu_state.h"
#include "parts.h"
#include "read_file.h"

static double wall_seconds(void) {
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    static const uint8_t PROGRAM[] = {0xA2, 0xFF, 0x9A, 0xA9, 0x00, 0x85, 0x10, 0xE6, 0x10, 0xA5,
                                      0x10, 0x8D, 0x00, 0x02, 0xC9, 0x10, 0xD0, 0xF5, 0x4C, 0x07,
                                      0x80};
    PartLibrary *library = part_library_create();
    CpuBoard *board;
    CpuState state;
    long ms = argc > 1 ? atol(argv[1]) : 100;
    SimTime step = 100000000ull;   /* 100 us */
    SimTime end;
    SimTime t;
    double start, seconds;

    if (ms <= 0) {
        fprintf(stderr, "usage: cpu_bench <sim ms>\n");
        return 2;
    }
    if (load_parts_dir(library, PARTS_DIR) != 0) {
        return 2;
    }
    board = cpu_board_create(library, "W65C02S6TPG-14", PROGRAM, sizeof PROGRAM, 0x8000);
    if (board == NULL) {
        return 2;
    }
    end = (SimTime)ms * 1000000000ull;
    start = wall_seconds();
    for (t = step; t <= end; t += step) {
        if (!cpu_board_run(board, t)) {
            return 1;
        }
    }
    seconds = wall_seconds() - start;
    cpu_board_state(board, &state);
    printf("W65C02S board, %ld ms simulated in %.3f s: %.2fx real time (status %d, %llu cycles)\n",
           ms, seconds, (double)ms / 1000.0 / seconds, (int)state.status,
           (unsigned long long)state.cycles);
    cpu_board_destroy(board);
    part_library_destroy(library);
    return 0;
}
