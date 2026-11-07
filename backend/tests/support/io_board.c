#include "io_board.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define NS(value) ((SimTime)((value) * 1000ull))

enum { MAX_NETS = 160 };
enum { T_RISE, T_FALL, T_RELEASE };

typedef struct {
    char name[16];
    SimNet *net;
} Net;

typedef struct {
    SimInstance *instance;
    const Part *part;
} Chip;

struct IoBoard {
    Sim *sim;
    SimInstance *tb;
    Chip cpu, rom, ram, via, pia, acia, osc, nand[3], pullup, vcc, gnd;
    Net nets[MAX_NETS];
    size_t net_count;
    int failed;
};

/* ---- The bench: CLK (pin 0), RES (pin 1, low for 4 cycles) ------------------- */

static size_t tb_pins(const void *config) {
    (void)config;
    return 2;
}

static void *tb_create(const void *config) {
    return (void *)config;
}

static void tb_destroy(void *state) {
    (void)state;
}

static void tb_start(Sim *sim, SimInstance *instance, void *state) {
    (void)state;
    sim_schedule_drive(sim, instance, 0, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_drive(sim, instance, 1, LOGIC_0, DRIVE_STRONG, 0);
    sim_schedule_timer(sim, instance, T_RISE, NS(500));
    sim_schedule_timer(sim, instance, T_RELEASE, NS(4200));
}

static void tb_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
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

static const SimComponentType TB = {
    "io-bench", tb_pins, tb_create, tb_destroy, tb_start, NULL, tb_timer
};

/* ---- Wiring ------------------------------------------------------------------ */

static SimNet *net(IoBoard *b, const char *name) {
    size_t i;

    for (i = 0; i < b->net_count; i++) {
        if (strcmp(b->nets[i].name, name) == 0) {
            return b->nets[i].net;
        }
    }
    if (b->net_count == MAX_NETS) {
        fprintf(stderr, "io_board: too many nets\n");
        b->failed = 1;
        return NULL;
    }
    snprintf(b->nets[b->net_count].name, sizeof b->nets[0].name, "%s", name);
    b->nets[b->net_count].net = sim_add_net(b->sim, name);
    return b->nets[b->net_count++].net;
}

static int pin_of(const Part *part, const char *pin) {
    size_t i;

    for (i = 0; i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, pin) == 0) {
            return (int)i;
        }
    }
    return -1;
}

static void wire(IoBoard *b, const Chip *c, const char *pin, const char *net_name) {
    int i = pin_of(c->part, pin);
    SimNet *n;

    if (i < 0) {
        fprintf(stderr, "io_board: no pin %s on %s\n", pin, c->part->name);
        b->failed = 1;
        return;
    }
    n = net(b, net_name);
    if (n != NULL) {
        sim_connect(b->sim, n, c->instance, (size_t)i);
    }
}

/* PREFIX0..PREFIX(n-1) of a chip onto nets NET0..NET(n-1). */
static void wire_bus(IoBoard *b, const Chip *c, const char *prefix, const char *net_prefix,
                     int n) {
    char pin[16], name[16];
    int i;

    for (i = 0; i < n; i++) {
        snprintf(pin, sizeof pin, "%s%d", prefix, i);
        snprintf(name, sizeof name, "%s%d", net_prefix, i);
        wire(b, c, pin, name);
    }
}

static void add(IoBoard *b, const PartLibrary *library, const char *name, const char *label,
                Chip *c) {
    c->part = part_library_find(library, name);
    if (c->part == NULL) {
        fprintf(stderr, "io_board: no part %s\n", name);
        b->failed = 1;
        return;
    }
    c->instance = sim_add_instance(b->sim, kind_model(c->part), label, c->part);
}

/* NAND gate g (1-10): chip (g - 1) / 4, gate (g - 1) % 4 + 1. */
static void gate(IoBoard *b, int g, const char *a, const char *bb, const char *y) {
    const Chip *c = &b->nand[(g - 1) / 4];
    int unit = (g - 1) % 4 + 1;
    char pin[8];

    snprintf(pin, sizeof pin, "%dA", unit);
    wire(b, c, pin, a);
    snprintf(pin, sizeof pin, "%dB", unit);
    wire(b, c, pin, bb);
    snprintf(pin, sizeof pin, "%dY", unit);
    wire(b, c, pin, y);
}

static void wire_board(IoBoard *b) {
    /* The bench, the supplies, the pull-up. */
    sim_connect(b->sim, net(b, "CLK"), b->tb, 0);
    sim_connect(b->sim, net(b, "RES"), b->tb, 1);
    wire(b, &b->vcc, "VCC", "ONE");
    wire(b, &b->gnd, "GND", "ZERO");
    wire(b, &b->pullup, "PU", "IRQX");

    /* CPU. */
    wire_bus(b, &b->cpu, "A", "A", 16);
    wire_bus(b, &b->cpu, "D", "D", 8);
    wire(b, &b->cpu, "PHI2", "CLK");
    wire(b, &b->cpu, "RESB", "RES");
    wire(b, &b->cpu, "RWB", "RW");
    wire(b, &b->cpu, "IRQB", "IRQ");
    wire(b, &b->cpu, "NMIB", "ONE");
    wire(b, &b->cpu, "RDY", "ONE");
    wire(b, &b->cpu, "SOB", "ONE");
    wire(b, &b->cpu, "BE", "ONE");

    /* Decoding (plan-14-E section 3.4). */
    gate(b, 1, "A15", "A15", "G1");     /* !A15: EEPROM CE# */
    gate(b, 2, "A14", "A14", "G2");     /* !A14 */
    gate(b, 3, "G1", "G2", "G3");       /* A15 | A14: SRAM CE# */
    gate(b, 4, "G1", "A14", "G4");      /* I/O select, low at 4000-7FFF */
    gate(b, 5, "A13", "A13", "G5");     /* !A13: PIA CS1 */
    gate(b, 6, "A12", "A12", "G6");     /* !A12 */
    gate(b, 7, "G5", "G6", "G7");       /* A13 | A12 */
    gate(b, 8, "G7", "G7", "G8");       /* !(A13 | A12): ACIA CS0 */
    gate(b, 9, "RW", "RW", "G9");       /* !RWB: memory OE# */
    gate(b, 10, "CLK", "G9", "G10");    /* SRAM WE# */

    /* Memories. */
    wire_bus(b, &b->rom, "A", "A", 15);
    wire_bus(b, &b->rom, "I/O", "D", 8);
    wire(b, &b->rom, "CE#", "G1");
    wire(b, &b->rom, "OE#", "G9");
    wire(b, &b->rom, "WE#", "ONE");
    wire_bus(b, &b->ram, "A", "A", 15);
    wire_bus(b, &b->ram, "DQ", "D", 8);
    wire(b, &b->ram, "CE#", "G3");
    wire(b, &b->ram, "OE#", "G9");
    wire(b, &b->ram, "WE#", "G10");

    /* VIA at 6000. */
    wire_bus(b, &b->via, "D", "D", 8);
    wire_bus(b, &b->via, "RS", "A", 4);
    wire(b, &b->via, "PHI2", "CLK");
    wire(b, &b->via, "RESB", "RES");
    wire(b, &b->via, "RWB", "RW");
    wire(b, &b->via, "CS1", "A13");
    wire(b, &b->via, "CS2B", "G4");
    wire(b, &b->via, "IRQB", "IRQ");
    wire_bus(b, &b->via, "PA", "VPA", 8);
    wire_bus(b, &b->via, "PB", "VPB", 8);

    /* PIA at 5000. */
    wire_bus(b, &b->pia, "D", "D", 8);
    wire_bus(b, &b->pia, "RS", "A", 2);
    wire(b, &b->pia, "PHI2", "CLK");
    wire(b, &b->pia, "RESB", "RES");
    wire(b, &b->pia, "RWB", "RW");
    wire(b, &b->pia, "CS0", "A12");
    wire(b, &b->pia, "CS1", "G5");
    wire(b, &b->pia, "CS2B", "G4");
    wire(b, &b->pia, "IRQAB", "IRQX");
    wire(b, &b->pia, "IRQBB", "IRQX");
    wire_bus(b, &b->pia, "PA", "PPA", 8);
    wire_bus(b, &b->pia, "PB", "PPB", 8);

    /* ACIA at 4000, TxD looped to RxD. */
    wire_bus(b, &b->acia, "D", "D", 8);
    wire_bus(b, &b->acia, "RS", "A", 2);
    wire(b, &b->acia, "PHI2", "CLK");
    wire(b, &b->acia, "RESB", "RES");
    wire(b, &b->acia, "RWB", "RW");
    wire(b, &b->acia, "CS0", "G8");
    wire(b, &b->acia, "CS1B", "G4");
    wire(b, &b->acia, "IRQB", "IRQX");
    wire(b, &b->acia, "XTLI", "XTAL");
    wire(b, &b->acia, "CTSB", "ZERO");
    wire(b, &b->acia, "DCDB", "ZERO");
    wire(b, &b->acia, "DSRB", "ZERO");
    wire(b, &b->acia, "TxD", "SER");
    wire(b, &b->acia, "RxD", "SER");
    wire(b, &b->osc, "OUTPUT", "XTAL");
}

IoBoard *io_board_create(const PartLibrary *library, const uint8_t *image, size_t count) {
    IoBoard *b = calloc(1, sizeof(*b));
    const SimComponentType *rom_type;
    static uint32_t words[32768];
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    b->sim = sim_create();
    add(b, library, "W65C02S6TPG-14", "CPU", &b->cpu);
    add(b, library, "AT28C256-15PU", "ROM", &b->rom);
    add(b, library, "AS6C62256-55PCN", "RAM", &b->ram);
    add(b, library, "W65C22S6TPG-14", "VIA", &b->via);
    add(b, library, "W65C21N6TPG-14", "PIA", &b->pia);
    add(b, library, "W65C51N6TPG-14", "ACIA", &b->acia);
    add(b, library, "ECS-2100A-018", "OSC", &b->osc);
    add(b, library, "SN74HC00N", "U1", &b->nand[0]);
    add(b, library, "SN74HC00N", "U2", &b->nand[1]);
    add(b, library, "SN74HC00N", "U3", &b->nand[2]);
    add(b, library, "PULLUP-5V", "PU", &b->pullup);
    add(b, library, "5V", "VCC", &b->vcc);
    add(b, library, "GND", "GND", &b->gnd);
    b->tb = sim_add_instance(b->sim, &TB, "TB", b);
    if (b->failed || count > 32768) {
        io_board_destroy(b);
        return NULL;
    }
    wire_board(b);
    if (b->failed) {
        io_board_destroy(b);
        return NULL;
    }
    for (i = 0; i < 32768; i++) {
        words[i] = i < count ? image[i] : 0xFF;
    }
    rom_type = sim_instance_type(b->rom.instance);
    rom_type->memory_load(sim_instance_state(b->rom.instance), 0, words, 32768);
    if (!sim_start(b->sim)) {
        fprintf(stderr, "io_board: %s\n", sim_error(b->sim));
        io_board_destroy(b);
        return NULL;
    }
    return b;
}

void io_board_destroy(IoBoard *board) {
    if (board != NULL) {
        sim_destroy(board->sim);
        free(board);
    }
}

int io_board_run(IoBoard *board, SimTime time) {
    if (!sim_run_until(board->sim, time)) {
        fprintf(stderr, "io_board: %s\n", sim_error(board->sim));
        return 0;
    }
    return 1;
}

int io_board_sram(const IoBoard *board, uint32_t address, uint8_t *value) {
    const SimComponentType *type = sim_instance_type(board->ram.instance);
    uint32_t v = 0;
    uint32_t known = 0;

    if (type->memory_read(sim_instance_state(board->ram.instance), address, 1, &v, &known) != 1 ||
        known != 0xFF) {
        return 0;
    }
    *value = (uint8_t)v;
    return 1;
}

int io_board_state(const IoBoard *board, CpuState *state) {
    const SimComponentType *type = sim_instance_type(board->cpu.instance);

    return type->cpu_state != NULL && type->cpu_state(sim_instance_state(board->cpu.instance), state);
}

int io_board_port(const IoBoard *board, const char *chip, char port, uint8_t *value) {
    const Chip *c = strcmp(chip, "VIA") == 0 ? &board->via : &board->pia;
    char pin[8];
    uint8_t v = 0;
    int i;

    for (i = 0; i < 8; i++) {
        int index;
        LogicValue l;

        snprintf(pin, sizeof pin, "P%c%d", port, i);
        index = pin_of(c->part, pin);
        if (index < 0) {
            return 0;
        }
        l = sim_read_pin(board->sim, c->instance, (size_t)index);
        if (l != LOGIC_0 && l != LOGIC_1) {
            return 0;
        }
        if (l == LOGIC_1) v |= (uint8_t)(1u << i);
    }
    *value = v;
    return 1;
}

size_t io_board_sent(const IoBoard *board, uint16_t *frames, size_t cap) {
    const SimComponentType *type = sim_instance_type(board->acia.instance);

    return type->serial_sent(sim_instance_state(board->acia.instance), 0, 0, frames, cap);
}
