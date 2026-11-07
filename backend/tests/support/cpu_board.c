#include "cpu_board.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define NS(value) ((SimTime)((value) * 1000ull))

enum { MAX_NETS = 96, BENCH_CLK, BENCH_RESET, BENCH_ONE };
enum { T_RISE, T_FALL, T_RELEASE };

typedef struct {
    char name[16];
    SimNet *net;
} Net;

struct CpuBoard {
    Sim *sim;
    SimInstance *cpu, *rom, *ram, *nand, *tb, *nand2, *latch;
    const Part *cpu_part, *rom_part, *ram_part, *nand_part, *nand2_part, *latch_part;
    Net nets[MAX_NETS];
    size_t net_count;
    int failed;
};

/* ---- The bench: CLK (pin 0), RESET (pin 1, low for 4 cycles), ONE (pin 2),
   ZERO (pin 3) ---------------------------------------------------------------- */

static size_t bench_pins(const void *config) {
    (void)config;
    return 4;
}

static void *bench_create(const void *config) {
    return (void *)config;
}

static void bench_destroy(void *state) {
    (void)state;
}

static void bench_start(Sim *sim, SimInstance *instance, void *state) {
    (void)state;
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, 1, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, 2, LOGIC_1, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, 3, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_timer(sim, instance, T_RISE, NS(500));
    sim_schedule_timer(sim, instance, T_RELEASE, NS(4200));
}

static void bench_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    (void)state;
    switch (id) {
        case T_RISE:
            sim_schedule_drive(sim, instance, 0, LOGIC_1, DRIVE_STRONG, 0);
            sim_schedule_timer(sim, instance, T_FALL, NS(500));
            break;
        case T_FALL:
            sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
            sim_schedule_timer(sim, instance, T_RISE, NS(500));
            break;
        case T_RELEASE:
            sim_schedule_drive(sim, instance, 1, LOGIC_1, DRIVE_STRONG, 0);
            break;
    }
}

static const SimComponentType BENCH = {
    "board-bench", bench_pins, bench_create, bench_destroy, bench_start, NULL, bench_timer,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/* ---- Wiring ------------------------------------------------------------ */

static SimNet *net(CpuBoard *b, const char *name) {
    size_t i;

    for (i = 0; i < b->net_count; i++) {
        if (strcmp(b->nets[i].name, name) == 0) {
            return b->nets[i].net;
        }
    }
    if (b->net_count == MAX_NETS) {
        fprintf(stderr, "cpu_board: too many nets\n");
        b->failed = 1;
        return NULL;
    }
    snprintf(b->nets[b->net_count].name, sizeof b->nets[0].name, "%s", name);
    b->nets[b->net_count].net = sim_add_net(b->sim, name);
    return b->nets[b->net_count++].net;
}

static void wire(CpuBoard *b, SimInstance *instance, const Part *part, const char *pin,
                 const char *net_name) {
    SimNet *n = net(b, net_name);
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, pin) == 0) {
            if (n != NULL) {
                sim_connect(b->sim, n, instance, i);
            }
            return;
        }
    }
    fprintf(stderr, "cpu_board: no pin %s on %s\n", pin, part->name);
    b->failed = 1;
}

static void wire_bench(CpuBoard *b, size_t pin, const char *net_name) {
    SimNet *n = net(b, net_name);

    if (n != NULL) {
        sim_connect(b->sim, n, b->tb, pin);
    }
}

static SimInstance *add(CpuBoard *b, const PartLibrary *library, const char *name,
                        const Part **part) {
    *part = part_library_find(library, name);
    if (*part == NULL) {
        fprintf(stderr, "cpu_board: no part %s\n", name);
        b->failed = 1;
        return NULL;
    }
    return sim_add_instance(b->sim, kind_model(*part), name, *part);
}

/* The memories' address and data pins on the CPU's buses. */
static void wire_memories(CpuBoard *b) {
    char pin[8], bus[8];
    int i;

    for (i = 0; i < 15; i++) {
        snprintf(pin, sizeof pin, "A%d", i);
        wire(b, b->rom, b->rom_part, pin, pin);
        wire(b, b->ram, b->ram_part, pin, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(bus, sizeof bus, "D%d", i);
        snprintf(pin, sizeof pin, "I/O%d", i);
        wire(b, b->rom, b->rom_part, pin, bus);
        snprintf(pin, sizeof pin, "DQ%d", i);
        wire(b, b->ram, b->ram_part, pin, bus);
    }
}

static void wire_cpu_buses(CpuBoard *b) {
    char pin[8];
    int i;

    for (i = 0; i < 16; i++) {
        snprintf(pin, sizeof pin, "A%d", i);
        wire(b, b->cpu, b->cpu_part, pin, pin);
    }
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "D%d", i);
        wire(b, b->cpu, b->cpu_part, pin, pin);
    }
}

static void wire_6502(CpuBoard *b, int wdc) {
    wire_cpu_buses(b);
    wire(b, b->cpu, b->cpu_part, wdc ? "PHI2" : "PHI0IN", "CLK");
    wire(b, b->cpu, b->cpu_part, wdc ? "RESB" : "RES#", "RES");
    wire(b, b->cpu, b->cpu_part, wdc ? "RWB" : "R/W", "RW");
    /* gate 1: OE# = NAND(R/W, R/W); gate 2: SRAM WE# = NAND(clock, gate 1);
       gate 3: EEPROM CE# = NAND(A15, A15). */
    wire(b, b->nand, b->nand_part, "1A", "RW");
    wire(b, b->nand, b->nand_part, "1B", "RW");
    wire(b, b->nand, b->nand_part, "1Y", "G1Y");
    wire(b, b->nand, b->nand_part, "2A", "CLK");
    wire(b, b->nand, b->nand_part, "2B", "G1Y");
    wire(b, b->nand, b->nand_part, "2Y", "G2Y");
    wire(b, b->nand, b->nand_part, "3A", "A15");
    wire(b, b->nand, b->nand_part, "3B", "A15");
    wire(b, b->nand, b->nand_part, "3Y", "G3Y");
    wire(b, b->rom, b->rom_part, "CE#", "G3Y");
    wire(b, b->rom, b->rom_part, "OE#", "G1Y");
    wire(b, b->rom, b->rom_part, "WE#", "ONE");
    wire(b, b->ram, b->ram_part, "CE#", "A15");
    wire(b, b->ram, b->ram_part, "OE#", "G1Y");
    wire(b, b->ram, b->ram_part, "WE#", "G2Y");
}

/* The W65C816S board (plan-14-I section 3.2). */
static void wire_65816(CpuBoard *b) {
    char pin[8], bus[8];
    int i;

    wire_cpu_buses(b);
    wire(b, b->cpu, b->cpu_part, "PHI2", "CLK");
    wire(b, b->cpu, b->cpu_part, "RESB", "RES");
    wire(b, b->cpu, b->cpu_part, "RWB", "RW");
    wire(b, b->cpu, b->cpu_part, "IRQB", "ONE");
    wire(b, b->cpu, b->cpu_part, "NMIB", "ONE");
    wire(b, b->cpu, b->cpu_part, "ABORTB", "ONE");
    wire(b, b->cpu, b->cpu_part, "BE", "ONE");
    /* g1 = NAND(A15, A15): EEPROM CE#; g2 = NAND(PHI2, RWB): OE#;
       g3 = NAND(RWB, RWB); g4 = NAND(PHI2, g3): SRAM WE#; g5 = NAND(PHI2,
       PHI2): the latch's LE. */
    wire(b, b->nand, b->nand_part, "1A", "A15");
    wire(b, b->nand, b->nand_part, "1B", "A15");
    wire(b, b->nand, b->nand_part, "1Y", "G1Y");
    wire(b, b->nand, b->nand_part, "2A", "CLK");
    wire(b, b->nand, b->nand_part, "2B", "RW");
    wire(b, b->nand, b->nand_part, "2Y", "G2Y");
    wire(b, b->nand, b->nand_part, "3A", "RW");
    wire(b, b->nand, b->nand_part, "3B", "RW");
    wire(b, b->nand, b->nand_part, "3Y", "G3Y");
    wire(b, b->nand, b->nand_part, "4A", "CLK");
    wire(b, b->nand, b->nand_part, "4B", "G3Y");
    wire(b, b->nand, b->nand_part, "4Y", "G4Y");
    wire(b, b->nand2, b->nand2_part, "1A", "CLK");
    wire(b, b->nand2, b->nand2_part, "1B", "CLK");
    wire(b, b->nand2, b->nand2_part, "1Y", "G5Y");
    /* The bank latch. */
    for (i = 0; i < 8; i++) {
        snprintf(pin, sizeof pin, "%dD", i + 1);
        snprintf(bus, sizeof bus, "D%d", i);
        wire(b, b->latch, b->latch_part, pin, bus);
    }
    wire(b, b->latch, b->latch_part, "LE", "G5Y");
    wire(b, b->latch, b->latch_part, "OE", "ZERO");
    wire(b, b->latch, b->latch_part, "1Q", "BA0");
    /* Memories: the CPU's A0-A14 and D0-D7 (wire_memories); the SRAM's A15
       is BA0, A16 0. */
    wire(b, b->ram, b->ram_part, "A15", "BA0");
    wire(b, b->ram, b->ram_part, "A16", "ZERO");
    wire(b, b->ram, b->ram_part, "CE#", "A15");
    wire(b, b->ram, b->ram_part, "CE2", "ONE");
    wire(b, b->ram, b->ram_part, "OE#", "G2Y");
    wire(b, b->ram, b->ram_part, "WE#", "G4Y");
    wire(b, b->rom, b->rom_part, "CE#", "G1Y");
    wire(b, b->rom, b->rom_part, "OE#", "G2Y");
    wire(b, b->rom, b->rom_part, "WE#", "ONE");
}

static void wire_z80(CpuBoard *b) {
    wire_cpu_buses(b);
    wire(b, b->cpu, b->cpu_part, "CLK", "CLK");
    wire(b, b->cpu, b->cpu_part, "RESET#", "RES");
    wire(b, b->cpu, b->cpu_part, "MREQ#", "MREQ");
    wire(b, b->cpu, b->cpu_part, "RD#", "RD");
    wire(b, b->cpu, b->cpu_part, "WR#", "WR");
    /* gate 1 = MREQ; gate 2 = !A15; EEPROM CE# = NAND(gate 1, gate 2); SRAM
       CE# = NAND(gate 1, A15). */
    wire(b, b->nand, b->nand_part, "1A", "MREQ");
    wire(b, b->nand, b->nand_part, "1B", "MREQ");
    wire(b, b->nand, b->nand_part, "1Y", "G1Y");
    wire(b, b->nand, b->nand_part, "2A", "A15");
    wire(b, b->nand, b->nand_part, "2B", "A15");
    wire(b, b->nand, b->nand_part, "2Y", "G2Y");
    wire(b, b->nand, b->nand_part, "3A", "G1Y");
    wire(b, b->nand, b->nand_part, "3B", "G2Y");
    wire(b, b->nand, b->nand_part, "3Y", "G3Y");
    wire(b, b->nand, b->nand_part, "4A", "G1Y");
    wire(b, b->nand, b->nand_part, "4B", "A15");
    wire(b, b->nand, b->nand_part, "4Y", "G4Y");
    wire(b, b->rom, b->rom_part, "CE#", "G3Y");
    wire(b, b->rom, b->rom_part, "OE#", "RD");
    wire(b, b->rom, b->rom_part, "WE#", "ONE");
    wire(b, b->ram, b->ram_part, "CE#", "G4Y");
    wire(b, b->ram, b->ram_part, "OE#", "RD");
    wire(b, b->ram, b->ram_part, "WE#", "WR");
}

CpuBoard *cpu_board_create(const PartLibrary *library, const char *cpu, const uint8_t *image,
                           size_t count, int vector) {
    CpuBoard *b = calloc(1, sizeof(*b));
    const SimComponentType *rom_type;
    uint32_t words[32768];
    size_t i;
    int w816;

    if (b == NULL) {
        return NULL;
    }
    b->sim = sim_create();
    b->cpu = add(b, library, cpu, &b->cpu_part);
    if (b->failed || b->cpu_part->kind != PART_CPU || count > 32768) {
        cpu_board_destroy(b);
        return NULL;
    }
    w816 = strcmp(b->cpu_part->cpu->core, "65816") == 0;
    b->rom = add(b, library, "AT28C256-15PU", &b->rom_part);
    b->ram = add(b, library, w816 ? "AS6C1008-55PCN" : "AS6C62256-55PCN", &b->ram_part);
    b->nand = add(b, library, "SN74HC00N", &b->nand_part);
    if (w816) {
        b->nand2 = add(b, library, "SN74HC00N", &b->nand2_part);
        b->latch = add(b, library, "SN74HC573AN", &b->latch_part);
    }
    b->tb = sim_add_instance(b->sim, &BENCH, "TB", b);
    if (b->failed) {
        cpu_board_destroy(b);
        return NULL;
    }
    wire_bench(b, 0, "CLK");
    wire_bench(b, 1, "RES");
    wire_bench(b, 2, "ONE");
    wire_bench(b, 3, "ZERO");
    wire_memories(b);
    if (w816) {
        wire_65816(b);
    } else if (strcmp(b->cpu_part->cpu->core, "z80") == 0) {
        wire_z80(b);
    } else {
        wire_6502(b, strcmp(b->cpu_part->cpu->core, "65c02") == 0);
    }
    if (b->failed) {
        cpu_board_destroy(b);
        return NULL;
    }
    /* The program, the rest erased (FF). */
    for (i = 0; i < 32768; i++) {
        words[i] = i < count ? image[i] : 0xFF;
    }
    if (vector >= 0) {
        words[0x7FFC] = (uint32_t)vector & 0xFF;
        words[0x7FFD] = ((uint32_t)vector >> 8) & 0xFF;
    }
    rom_type = sim_instance_type(b->rom);
    rom_type->memory_load(sim_instance_state(b->rom), 0, words, 32768);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "cpu_board: %s\n", sim_error(b->sim));
        cpu_board_destroy(b);
        return NULL;
    }
    return b;
}

void cpu_board_destroy(CpuBoard *board) {
    if (board != NULL) {
        sim_destroy(board->sim);
        free(board);
    }
}

int cpu_board_run(CpuBoard *board, SimTime time) {
    if (!sim_run_until(board->sim, time)) {
        fprintf(stderr, "cpu_board: %s\n", sim_error(board->sim));
        return 0;
    }
    return 1;
}

int cpu_board_sram(const CpuBoard *board, uint32_t address, uint8_t *value) {
    const SimComponentType *type = sim_instance_type(board->ram);
    uint32_t v = 0;
    uint32_t known = 0;

    if (type->memory_read(sim_instance_state(board->ram), address, 1, &v, &known) != 1 ||
        known != 0xFF) {
        return 0;
    }
    *value = (uint8_t)v;
    return 1;
}

int cpu_board_state(const CpuBoard *board, CpuState *state) {
    const SimComponentType *type = sim_instance_type(board->cpu);

    return type->cpu_state != NULL && type->cpu_state(sim_instance_state(board->cpu), state);
}
