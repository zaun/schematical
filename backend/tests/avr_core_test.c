/*
 * avr_core_test (plan-62-A): the AVR CPU core (src/models/avr_cpu.c,
 * avr_instruction.c) against avr8js, its oracle.
 *
 *   step   Every opcode word 0..65535 in 4 random states each: one
 *          instruction, then a line of the resulting state. The lines must
 *          hash to tests/avr/golden.txt's `step` line, and, when CMake found
 *          node, equal the lines tools/avr8js-oracle/oracle.js prints from
 *          avr8js itself. The core runs in exact mode (avr8js's behavior
 *          everywhere).
 *   fixes  The 7 places where the core follows Microchip's instruction set
 *          manual instead of avr8js, each against a reference written from
 *          the manual here.
 *   unit   Clock events and the interrupt queue.
 *
 *   avr_core_test                 run the checks
 *   avr_core_test --print-golden  print the `step` golden line from the
 *                                 oracle's output (needs node)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avr.h"

#define STEP_SEED 305419896u
#define STEP_STATES 4
#define STEP_FLASH_BYTES 0x8000u
#define STEP_SRAM_BYTES 2048u

static int failures;

static void expect(int ok, const char *what) {
    if (!ok) {
        printf("FAIL: %s\n", what);
        failures++;
    }
}

/* ---- Hashes and the random source (the oracle has the same) -------------- */

static uint32_t fnv32(const uint8_t *bytes, size_t count) {
    uint32_t h = 0x811c9dc5u;
    size_t i;

    for (i = 0; i < count; i++) {
        h ^= bytes[i];
        h *= 0x01000193u;
    }
    return h;
}

static uint64_t fnv64_add(uint64_t h, const char *text) {
    for (; *text != '\0'; text++) {
        h ^= (uint8_t)*text;
        h *= 0x100000001b3ull;
    }
    return h;
}

#define FNV64_START 0xcbf29ce484222325ull

static uint32_t random_state;

static uint32_t draw(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

/* ---- step ----------------------------------------------------------------- */

typedef struct {
    AvrCpu *cpu;
    unsigned opcode;
    unsigned state;
} StepSource;

static void step_begin(StepSource *source) {
    source->cpu = avr_cpu_create(STEP_FLASH_BYTES, STEP_SRAM_BYTES);
    source->cpu->exact = 1;
    /* avr8js's progMem starts as zeros (a new Uint16Array). */
    memset(source->cpu->flash, 0, STEP_FLASH_BYTES);
    source->opcode = 0;
    source->state = 0;
    random_state = STEP_SEED;
}

static void set_flash_word(AvrCpu *cpu, uint32_t index, uint16_t word) {
    cpu->flash[index * 2] = (uint8_t)word;
    cpu->flash[index * 2 + 1] = (uint8_t)(word >> 8);
}

/* The next vector's line (with its newline) into out; 0 at the end. */
static int step_next(StepSource *source, char *out, size_t cap) {
    AvrCpu *cpu = source->cpu;
    uint32_t i, pc;

    if (source->opcode > 0xffff) {
        return 0;
    }
    for (i = 0; i < cpu->data_size; i += 4) {
        uint32_t r = draw();

        cpu->data[i] = (uint8_t)r;
        cpu->data[i + 1] = (uint8_t)(r >> 8);
        cpu->data[i + 2] = (uint8_t)(r >> 16);
        cpu->data[i + 3] = (uint8_t)(r >> 24);
    }
    pc = 0x1000 + draw() % 0x2000;
    cpu->pc = pc;
    set_flash_word(cpu, pc, (uint16_t)source->opcode);
    set_flash_word(cpu, pc + 1, (uint16_t)(draw() & 0xffff));
    set_flash_word(cpu, pc + 2, (uint16_t)(draw() & 0xffff));
    avr_set16(cpu, AVR_SP, 0x108 + draw() % 0x7f0);
    cpu->cycles = 0;
    avr_instruction(cpu);
    snprintf(out, cap, "%04x %04x %llx %02x %04x %08x\n", source->opcode, (unsigned)cpu->pc,
             (unsigned long long)cpu->cycles, cpu->data[AVR_SREG], avr_get16(cpu, AVR_SP),
             fnv32(cpu->data, cpu->data_size));
    if (++source->state == STEP_STATES) {
        source->state = 0;
        source->opcode++;
    }
    return 1;
}

/* The golden `step` line's fields; 0 when the file has none. */
static int read_step_golden(unsigned *seed, unsigned *states, unsigned long *lines,
                            unsigned long long *hash) {
    char path[1024];
    char line[256];
    FILE *file;
    int found = 0;

    snprintf(path, sizeof path, "%s/golden.txt", AVR_DIR);
    file = fopen(path, "r");
    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof line, file) != NULL) {
        if (sscanf(line, "step %u %u %lu %llx", seed, states, lines, hash) == 4) {
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}

#ifdef AVR_NODE
static FILE *open_oracle_step(void) {
    char command[2048];

    snprintf(command, sizeof command, "\"%s\" \"%s\" step %u %d", AVR_NODE, AVR_ORACLE, STEP_SEED,
             STEP_STATES);
    return popen(command, "r");
}
#endif

static void step_cases(void) {
    StepSource source;
    char ours[128];
    uint64_t hash = FNV64_START;
    unsigned long lines = 0;
    unsigned seed = 0, states = 0;
    unsigned long golden_lines = 0;
    unsigned long long golden_hash = 0;
    uint32_t coverage[AVR_BRANCHES + 1] = {0};
    int covered = 0, i;
    int live = 0;
#ifdef AVR_NODE
    FILE *oracle = open_oracle_step();
    char theirs[128];
    uint64_t oracle_hash = FNV64_START;
    int differs = 0;

    live = oracle != NULL;
#endif

    step_begin(&source);
    source.cpu->coverage = coverage;
    while (1) {
        unsigned opcode = source.opcode, state = source.state;

        if (!step_next(&source, ours, sizeof ours)) {
            break;
        }
        hash = fnv64_add(hash, ours);
        lines++;
#ifdef AVR_NODE
        if (live && !differs) {
            if (fgets(theirs, sizeof theirs, oracle) == NULL) {
                printf("FAIL: step: the oracle stopped after %lu lines\n", lines - 1);
                failures++;
                differs = 1;
            } else {
                oracle_hash = fnv64_add(oracle_hash, theirs);
                if (strcmp(ours, theirs) != 0) {
                    ours[strcspn(ours, "\n")] = '\0';
                    theirs[strcspn(theirs, "\n")] = '\0';
                    printf("FAIL: step: opcode %04x state %u: avr8js \"%s\" ours \"%s\"\n", opcode,
                           state, theirs, ours);
                    failures++;
                    differs = 1;
                }
            }
        }
#else
        (void)opcode;
        (void)state;
#endif
    }
    avr_cpu_destroy(source.cpu);
#ifdef AVR_NODE
    if (live) {
        if (!differs && fgets(theirs, sizeof theirs, oracle) != NULL) {
            printf("FAIL: step: the oracle printed more than %lu lines\n", lines);
            failures++;
            differs = 1;
        }
        if (pclose(oracle) != 0 && !differs) {
            printf("FAIL: step: the oracle exited with an error\n");
            failures++;
            differs = 1;
        }
        if (!differs && oracle_hash != hash) {
            printf("FAIL: step: the hashes of equal outputs differ\n");
            failures++;
        }
    }
#endif

    if (!read_step_golden(&seed, &states, &golden_lines, &golden_hash)) {
        printf("FAIL: step: no `step` line in %s/golden.txt\n", AVR_DIR);
        failures++;
    } else {
        expect(seed == STEP_SEED && states == STEP_STATES, "step: the golden's seed and states");
        if (golden_lines != lines || golden_hash != hash) {
            printf("FAIL: step: %lu lines hash %016llx; the golden says %lu lines hash %016llx\n",
                   lines, (unsigned long long)hash, golden_lines, golden_hash);
            failures++;
        }
        /* The comparison itself: a wrong hash must not compare equal. */
        expect((golden_hash ^ 0x10ull) != hash, "step: a changed golden hash is noticed");
    }

    for (i = 1; i <= AVR_BRANCHES; i++) {
        covered += coverage[i] != 0;
    }
    if (covered != AVR_BRANCHES) {
        printf("FAIL: step: %d of %d decode branches ran\n", covered, AVR_BRANCHES);
        failures++;
    }
    if (failures == 0) {
        if (live) {
            printf("step: %lu vectors match avr8js\n", lines);
        } else {
            printf("step: %lu vectors match the golden hash (node not found: avr8js was not run)\n",
                   lines);
        }
        printf("step: %d of %d decode branches ran\n", covered, AVR_BRANCHES);
    }
}

static int print_golden(void) {
#ifdef AVR_NODE
    FILE *oracle = open_oracle_step();
    char line[128];
    uint64_t hash = FNV64_START;
    unsigned long lines = 0;

    if (oracle == NULL) {
        fprintf(stderr, "cannot run the oracle\n");
        return 1;
    }
    while (fgets(line, sizeof line, oracle) != NULL) {
        hash = fnv64_add(hash, line);
        lines++;
    }
    if (pclose(oracle) != 0) {
        fprintf(stderr, "the oracle exited with an error\n");
        return 1;
    }
    printf("step %u %d %lu %016llx\n", STEP_SEED, STEP_STATES, lines, (unsigned long long)hash);
    return 0;
#else
    fprintf(stderr, "--print-golden needs node (CMake did not find it)\n");
    return 1;
#endif
}

/* ---- fixes ---------------------------------------------------------------- */

enum { F_C = 1, F_Z = 2, F_N = 4, F_V = 8, F_S = 16, F_H = 32 };

static AvrCpu *fix_cpu;

/* Runs one instruction (one or two words at address 0) and returns SREG. */
static uint8_t run_one(uint16_t opcode, uint8_t sreg) {
    set_flash_word(fix_cpu, 0, opcode);
    fix_cpu->pc = 0;
    fix_cpu->cycles = 0;
    fix_cpu->data[AVR_SREG] = sreg;
    avr_instruction(fix_cpu);
    return fix_cpu->data[AVR_SREG];
}

static uint16_t op_rr(uint16_t base, int d, int r) {
    return (uint16_t)(base | ((r & 0x10) << 5) | (d << 4) | (r & 0xf));
}

static uint16_t op_imm(uint16_t base, int d, int k) {
    return (uint16_t)(base | ((k & 0xf0) << 4) | ((d - 16) << 4) | (k & 0xf));
}

/* 1. H is the carry or borrow out of the low nibble. */
static int fix_half_carry(void) {
    static const struct { const char *name; uint16_t base; int subtract, carry, immediate; } OPS[] = {
        {"ADD", 0x0c00, 0, 0, 0}, {"ADC", 0x1c00, 0, 1, 0}, {"SUB", 0x1800, 1, 0, 0},
        {"SBC", 0x0800, 1, 1, 0}, {"CP", 0x1400, 1, 0, 0},  {"CPC", 0x0400, 1, 1, 0},
        {"SUBI", 0x5000, 1, 0, 1}, {"SBCI", 0x4000, 1, 1, 1}, {"CPI", 0x3000, 1, 0, 1},
    };
    size_t op;
    int a, b, c, bad = 0;

    for (op = 0; op < sizeof OPS / sizeof OPS[0]; op++) {
        for (a = 0; a < 256; a++) {
            for (b = 0; b < 256; b++) {
                for (c = 0; c < 2; c++) {
                    int carry = OPS[op].carry ? c : 0;
                    int want = OPS[op].subtract ? (a & 15) < (b & 15) + carry
                                                : (a & 15) + (b & 15) + carry > 15;
                    uint8_t sreg;

                    fix_cpu->data[16] = (uint8_t)a;
                    fix_cpu->data[17] = (uint8_t)b;
                    sreg = run_one(OPS[op].immediate ? op_imm(OPS[op].base, 16, b)
                                                     : op_rr(OPS[op].base, 16, 17),
                                   (uint8_t)c);
                    if (((sreg & F_H) != 0) != want && !bad) {
                        printf("FAIL: fix 1: %s 0x%02x, 0x%02x, C=%d: H %d, the manual says %d\n",
                               OPS[op].name, a, b, c, (sreg & F_H) != 0, want);
                        bad = 1;
                    }
                }
            }
        }
    }
    for (a = 0; a < 256; a++) {
        uint8_t sreg;

        fix_cpu->data[16] = (uint8_t)a;
        sreg = run_one(0x9401 | (16 << 4), 0);
        if (((sreg & F_H) != 0) != ((a & 15) != 0) && !bad) {
            printf("FAIL: fix 1: NEG 0x%02x: H %d\n", a, (sreg & F_H) != 0);
            bad = 1;
        }
    }
    return !bad;
}

/* 2. SBIW leaves H alone. */
static int fix_sbiw(void) {
    int value, k, h, bad = 0;

    for (value = 0; value < 0x10000; value++) {
        for (k = 0; k < 64; k++) {
            for (h = 0; h < 2; h++) {
                uint8_t sreg;
                int result = (value - k) & 0xffff;

                avr_set16(fix_cpu, 24, (uint32_t)value);
                sreg = run_one((uint16_t)(0x9700 | ((k & 0x30) << 2) | (k & 0xf)),
                               (uint8_t)(h ? F_H : 0));
                if ((((sreg & F_H) != 0) != h || avr_get16(fix_cpu, 24) != result ||
                     ((sreg & F_Z) != 0) != (result == 0) || ((sreg & F_C) != 0) != (k > value)) &&
                    !bad) {
                    printf("FAIL: fix 2: SBIW 0x%04x, %d, H=%d: SREG %02x\n", value, k, h, sreg);
                    bad = 1;
                }
            }
        }
    }
    return !bad;
}

/* 3. CPC, SBC, SBCI: Z stays set only when the 8-bit result is zero. */
static int fix_zero(void) {
    static const struct { const char *name; uint16_t base; int immediate; } OPS[] = {
        {"CPC", 0x0400, 0}, {"SBC", 0x0800, 0}, {"SBCI", 0x4000, 1},
    };
    size_t op;
    int a, b, c, z, bad = 0;

    for (op = 0; op < 3; op++) {
        for (a = 0; a < 256; a++) {
            for (b = 0; b < 256; b++) {
                for (c = 0; c < 2; c++) {
                    for (z = 0; z < 2; z++) {
                        int want = z && ((a - b - c) & 0xff) == 0;
                        uint8_t sreg;

                        fix_cpu->data[16] = (uint8_t)a;
                        fix_cpu->data[17] = (uint8_t)b;
                        sreg = run_one(OPS[op].immediate ? op_imm(OPS[op].base, 16, b)
                                                         : op_rr(OPS[op].base, 16, 17),
                                       (uint8_t)(c | (z ? F_Z : 0)));
                        if (((sreg & F_Z) != 0) != want && !bad) {
                            printf("FAIL: fix 3: %s 0x%02x, 0x%02x, C=%d, Z=%d: Z %d\n", OPS[op].name,
                                   a, b, c, z, (sreg & F_Z) != 0);
                            bad = 1;
                        }
                    }
                }
            }
        }
    }
    return !bad;
}

/* 4. NEG sets V when the result is 0x80. */
static int fix_neg_overflow(void) {
    int a, bad = 0;

    for (a = 0; a < 256; a++) {
        uint8_t sreg;

        fix_cpu->data[16] = (uint8_t)a;
        sreg = run_one(0x9401 | (16 << 4), 0);
        if ((((sreg & F_V) != 0) != (a == 0x80) || fix_cpu->data[16] != (uint8_t)(0 - a)) && !bad) {
            printf("FAIL: fix 4: NEG 0x%02x: V %d\n", a, (sreg & F_V) != 0);
            bad = 1;
        }
    }
    return !bad;
}

/* 5. FMULSU sets Z when the result is zero. */
static int fix_fmulsu(void) {
    int a, b, bad = 0;

    for (a = 0; a < 256; a++) {
        for (b = 0; b < 256; b++) {
            int product = ((int8_t)a * b * 2) & 0xffff;
            uint8_t sreg;

            fix_cpu->data[16] = (uint8_t)a;
            fix_cpu->data[17] = (uint8_t)b;
            sreg = run_one(0x0388 | (0 << 4) | 1, 0);
            if ((((sreg & F_Z) != 0) != (product == 0) || avr_get16(fix_cpu, 0) != product) && !bad) {
                printf("FAIL: fix 5: FMULSU 0x%02x, 0x%02x: Z %d, R1:R0 %04x\n", a, b,
                       (sreg & F_Z) != 0, avr_get16(fix_cpu, 0));
                bad = 1;
            }
        }
    }
    return !bad;
}

/* 6. CBI takes 2 cycles (and SBI too). */
static int fix_cbi(void) {
    int ok;

    fix_cpu->data[0x25] = 0xff;
    run_one(0x9800 | (5 << 3) | 2, 0);   /* CBI 0x05, 2 */
    ok = fix_cpu->cycles == 2 && fix_cpu->data[0x25] == 0xfb;
    run_one(0x9a00 | (5 << 3) | 2, 0);   /* SBI 0x05, 2 */
    ok = ok && fix_cpu->cycles == 2 && fix_cpu->data[0x25] == 0xff;
    if (!ok) {
        printf("FAIL: fix 6: CBI/SBI cycles or result\n");
    }
    return ok;
}

/* 7. A taken interrupt costs 4 cycles, pushes the return address, clears I. */
static int fix_interrupt(void) {
    static const AvrInterrupt INTERRUPT = {0x1c, 0x6e, 0x02, 0x35, 0x02, 0, 0};
    int ok;

    avr_cpu_reset(fix_cpu);
    fix_cpu->data[AVR_SREG] = 0x80;
    fix_cpu->data[0x6e] = 0x02;
    fix_cpu->pc = 0x123;
    fix_cpu->cycles = 100;
    avr_set16(fix_cpu, AVR_SP, 0x8ff);
    avr_set_interrupt_flag(fix_cpu, &INTERRUPT);
    avr_cpu_tick(fix_cpu);
    ok = fix_cpu->cycles == 104 && fix_cpu->pc == 0x1c && fix_cpu->data[0x8ff] == 0x23 &&
         fix_cpu->data[0x8fe] == 0x01 && avr_get16(fix_cpu, AVR_SP) == 0x8fd &&
         (fix_cpu->data[AVR_SREG] & 0x80) == 0 && (fix_cpu->data[0x35] & 0x02) == 0 &&
         fix_cpu->next_interrupt == -1;
    if (!ok) {
        printf("FAIL: fix 7: interrupt: cycles %llu pc %x\n", (unsigned long long)fix_cpu->cycles,
               (unsigned)fix_cpu->pc);
    }
    return ok;
}

static void fix_cases(void) {
    int ok = 0;

    fix_cpu = avr_cpu_create(STEP_FLASH_BYTES, STEP_SRAM_BYTES);
    ok += fix_half_carry();
    ok += fix_sbiw();
    ok += fix_zero();
    ok += fix_neg_overflow();
    ok += fix_fmulsu();
    ok += fix_cbi();
    ok += fix_interrupt();
    avr_cpu_destroy(fix_cpu);
    if (ok == 7) {
        printf("fixes: 7 ok\n");
    } else {
        printf("FAIL: fixes: %d of 7 ok\n", ok);
        failures++;
    }
}

/* ---- unit ----------------------------------------------------------------- */

static char event_log[32];

static void log_event(void *ctx) {
    size_t n = strlen(event_log);

    if (n + 1 < sizeof event_log) {
        event_log[n] = *(const char *)ctx;
        event_log[n + 1] = '\0';
    }
}

static void run_ticks(AvrCpu *cpu, int count) {
    int i;

    for (i = 0; i < count; i++) {
        cpu->cycles++;
        avr_cpu_tick(cpu);
    }
}

static void unit_cases(void) {
    static const AvrInterrupt LOW = {4, 0x40, 1, 0x41, 1, 0, 0};
    static const AvrInterrupt HIGH = {9, 0x40, 2, 0x41, 2, 0, 0};
    static const AvrInterrupt LEVEL = {2, 0x40, 4, 0x41, 4, 1, 0};
    AvrCpu *cpu = avr_cpu_create(STEP_FLASH_BYTES, STEP_SRAM_BYTES);
    int before = failures;

    /* Clock events fire in cycle order; of two for one cycle the later
       added fires first (avr8js inserts before an equal cycle). */
    event_log[0] = '\0';
    avr_add_clock_event(cpu, log_event, "a", 5);
    avr_add_clock_event(cpu, log_event, "b", 3);
    avr_add_clock_event(cpu, log_event, "c", 5);
    avr_add_clock_event(cpu, log_event, "d", 0);   /* at least 1 cycle */
    run_ticks(cpu, 10);
    expect(strcmp(event_log, "dbca") == 0, "unit: clock events in cycle order, later first on a tie");

    /* One event per tick, even when several are due. */
    event_log[0] = '\0';
    avr_add_clock_event(cpu, log_event, "a", 1);
    avr_add_clock_event(cpu, log_event, "b", 1);
    cpu->cycles += 5;
    avr_cpu_tick(cpu);
    expect(strcmp(event_log, "b") == 0, "unit: one clock event per tick");
    avr_cpu_tick(cpu);
    expect(strcmp(event_log, "ba") == 0, "unit: the next due event on the next tick");

    /* Update moves an event; clear removes one; both report a miss. */
    event_log[0] = '\0';
    avr_add_clock_event(cpu, log_event, "a", 2);
    avr_add_clock_event(cpu, log_event, "b", 4);
    expect(avr_update_clock_event(cpu, log_event, "a", 6) == 1, "unit: update finds the event");
    expect(avr_update_clock_event(cpu, log_event, "x", 6) == 0, "unit: update misses an unknown one");
    expect(avr_clear_clock_event(cpu, log_event, "x") == 0, "unit: clear misses an unknown one");
    run_ticks(cpu, 5);
    expect(strcmp(event_log, "b") == 0, "unit: the moved event has not fired yet");
    expect(avr_clear_clock_event(cpu, log_event, "a") == 1, "unit: clear finds the event");
    run_ticks(cpu, 5);
    expect(strcmp(event_log, "b") == 0, "unit: a cleared event never fires");
    expect(cpu->next_event == NULL, "unit: no event left");

    /* Many events: more than the pool of 10 go round without loss. */
    {
        static const char NAMES[] = "0123456789ABCDEFGHIJ";
        int i, round;

        for (round = 0; round < 3; round++) {
            event_log[0] = '\0';
            for (i = 0; i < 20; i++) {
                avr_add_clock_event(cpu, log_event, (void *)&NAMES[i], i + 1);
            }
            run_ticks(cpu, 25);
            expect(strcmp(event_log, NAMES) == 0, "unit: 20 events fire in order");
        }
    }

    /* The lower vector first; a constant interrupt stays queued. */
    avr_cpu_reset(cpu);
    cpu->data[0x40] = 7;
    cpu->data[AVR_SREG] = 0x80;
    avr_set16(cpu, AVR_SP, 0x8ff);
    avr_set_interrupt_flag(cpu, &HIGH);
    avr_set_interrupt_flag(cpu, &LOW);
    avr_cpu_tick(cpu);
    expect(cpu->pc == 4 && cpu->next_interrupt == 9 && (cpu->data[0x41] & 1) == 0,
           "unit: the lower vector is taken first and its flag cleared");
    cpu->data[AVR_SREG] = 0x80;
    avr_cpu_tick(cpu);
    expect(cpu->pc == 9 && cpu->next_interrupt == -1, "unit: then the higher one");
    cpu->data[AVR_SREG] = 0x80;
    avr_set_interrupt_flag(cpu, &LEVEL);
    avr_cpu_tick(cpu);
    expect(cpu->pc == 2 && cpu->next_interrupt == 2 && (cpu->data[0x41] & 4) != 0,
           "unit: a constant interrupt stays queued with its flag");
    avr_clear_interrupt(cpu, &LEVEL, 1);
    expect(cpu->next_interrupt == -1 && (cpu->data[0x41] & 4) == 0, "unit: until it is cleared");
    cpu->data[0x40] = 0;
    avr_set_interrupt_flag(cpu, &LOW);
    expect(cpu->next_interrupt == -1 && (cpu->data[0x41] & 1) != 0,
           "unit: a disabled interrupt sets its flag and is not queued");
    avr_update_interrupt_enable(cpu, &LOW, 1);
    expect(cpu->next_interrupt == 4, "unit: enabling it queues it");

    /* Memory past the end reads 0 and drops writes. */
    expect(avr_read_data(cpu, (int32_t)cpu->data_size) == 0 && avr_read_data(cpu, -1) == 0,
           "unit: a read outside the data memory gives 0");
    avr_write_data(cpu, (int32_t)cpu->data_size + 5, 0xaa, 0xff);
    avr_write_data(cpu, -1, 0xaa, 0xff);

    avr_cpu_destroy(cpu);
    if (failures == before) {
        printf("unit: ok\n");
    }
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--print-golden") == 0) {
        return print_golden();
    }
    step_cases();
    fix_cases();
    unit_cases();
    if (failures != 0) {
        printf("%d failure(s)\n", failures);
        return 1;
    }
    return 0;
}
