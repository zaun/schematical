/*
 * circuit_adc_check: split from circuit_check.c by plan-68-H; the shared helpers are
 * support/circuit_bench.c.
 */

#include "circuit_bench.h"

/* ---- plan-24-G: an ADC read by a CPU ---------------------------------------- */

/* A W65C02S board (io_board's decoding: SRAM 0000-3FFF, VIA 6000, EEPROM
   8000-FFFF) with an ADS1115IDGSR on the VIA's port B: SDA = PB0, SCL = PB1,
   each pulled up, ADDR grounded (address 0x48). The program bit-bangs I2C
   (open drain by DDRB, ORB = 0), writes CONFIG = 0x85A3 (OS = 1 starts a
   single-shot conversion, MUX AIN0-AIN1, DR 250 SPS), polls CONFIG until OS
   reads 1, then reads CONVERSION. SRAM results: 0200 write ACKs ORed
   (0 = all acknowledged), 0201 the first poll's CONFIG MSB, 0202 polls that
   saw OS = 0, 0203 the last CONFIG MSB, 0204/0205 CONVERSION, 0206 read
   ACKs ORed, 0207 = AA when done. */
static const uint8_t ADC_PROGRAM[] = {
    /* RESET: */
    0x78,                /* 8000 SEI */
    0xD8,                /* 8001 CLD */
    0xA2, 0xFF,          /* 8002 LDX #$FF */
    0x9A,                /* 8004 TXS */
    0xA9, 0x00,          /* 8005 LDA #$00 */
    0x8D, 0x00, 0x60,    /* 8007 STA ORB */
    0x8D, 0x02, 0x60,    /* 800A STA DDRB */
    0x85, 0x13,          /* 800D STA $13 */
    0x85, 0x14,          /* 800F STA $14 */
    0x8D, 0x01, 0x02,    /* 8011 STA $0201 */
    0x20, 0xF3, 0x80,    /* 8014 JSR START */
    0xA9, 0x90,          /* 8017 LDA #$90 */
    0x85, 0x10,          /* 8019 STA $10 */
    0x20, 0x2D, 0x81,    /* 801B JSR SEND */
    0xA5, 0x12,          /* 801E LDA $12 */
    0x05, 0x13,          /* 8020 ORA $13 */
    0x85, 0x13,          /* 8022 STA $13 */
    0xA9, 0x01,          /* 8024 LDA #$01 */
    0x85, 0x10,          /* 8026 STA $10 */
    0x20, 0x2D, 0x81,    /* 8028 JSR SEND */
    0xA5, 0x12,          /* 802B LDA $12 */
    0x05, 0x13,          /* 802D ORA $13 */
    0x85, 0x13,          /* 802F STA $13 */
    0xA9, 0x85,          /* 8031 LDA #$85 */
    0x85, 0x10,          /* 8033 STA $10 */
    0x20, 0x2D, 0x81,    /* 8035 JSR SEND */
    0xA5, 0x12,          /* 8038 LDA $12 */
    0x05, 0x13,          /* 803A ORA $13 */
    0x85, 0x13,          /* 803C STA $13 */
    0xA9, 0xA3,          /* 803E LDA #$A3 */
    0x85, 0x10,          /* 8040 STA $10 */
    0x20, 0x2D, 0x81,    /* 8042 JSR SEND */
    0xA5, 0x12,          /* 8045 LDA $12 */
    0x05, 0x13,          /* 8047 ORA $13 */
    0x85, 0x13,          /* 8049 STA $13 */
    0x20, 0x14, 0x81,    /* 804B JSR STOP */
    0xA5, 0x13,          /* 804E LDA $13 */
    0x8D, 0x00, 0x02,    /* 8050 STA $0200 */
    0xA9, 0x00,          /* 8053 LDA #$00 */
    0x85, 0x13,          /* 8055 STA $13 */
    /* POLL: */
    0x20, 0xF3, 0x80,    /* 8057 JSR START */
    0xA9, 0x91,          /* 805A LDA #$91 */
    0x85, 0x10,          /* 805C STA $10 */
    0x20, 0x2D, 0x81,    /* 805E JSR SEND */
    0xA5, 0x12,          /* 8061 LDA $12 */
    0x05, 0x13,          /* 8063 ORA $13 */
    0x85, 0x13,          /* 8065 STA $13 */
    0xA9, 0x00,          /* 8067 LDA #$00 */
    0x85, 0x12,          /* 8069 STA $12 */
    0x20, 0x79, 0x81,    /* 806B JSR RECV */
    0xA5, 0x10,          /* 806E LDA $10 */
    0x8D, 0x03, 0x02,    /* 8070 STA $0203 */
    0xA9, 0x01,          /* 8073 LDA #$01 */
    0x85, 0x12,          /* 8075 STA $12 */
    0x20, 0x79, 0x81,    /* 8077 JSR RECV */
    0xA5, 0x10,          /* 807A LDA $10 */
    0x8D, 0x08, 0x02,    /* 807C STA $0208 */
    0x20, 0x14, 0x81,    /* 807F JSR STOP */
    0xA5, 0x14,          /* 8082 LDA $14 */
    0xD0, 0x06,          /* 8084 BNE NOTFIRST */
    0xAD, 0x03, 0x02,    /* 8086 LDA $0203 */
    0x8D, 0x01, 0x02,    /* 8089 STA $0201 */
    /* NOTFIRST: */
    0xAD, 0x03, 0x02,    /* 808C LDA $0203 */
    0x30, 0x05,          /* 808F BMI DONE */
    0xE6, 0x14,          /* 8091 INC $14 */
    0x4C, 0x57, 0x80,    /* 8093 JMP POLL */
    /* DONE: */
    0xA5, 0x14,          /* 8096 LDA $14 */
    0x8D, 0x02, 0x02,    /* 8098 STA $0202 */
    0x20, 0xF3, 0x80,    /* 809B JSR START */
    0xA9, 0x90,          /* 809E LDA #$90 */
    0x85, 0x10,          /* 80A0 STA $10 */
    0x20, 0x2D, 0x81,    /* 80A2 JSR SEND */
    0xA5, 0x12,          /* 80A5 LDA $12 */
    0x05, 0x13,          /* 80A7 ORA $13 */
    0x85, 0x13,          /* 80A9 STA $13 */
    0xA9, 0x00,          /* 80AB LDA #$00 */
    0x85, 0x10,          /* 80AD STA $10 */
    0x20, 0x2D, 0x81,    /* 80AF JSR SEND */
    0xA5, 0x12,          /* 80B2 LDA $12 */
    0x05, 0x13,          /* 80B4 ORA $13 */
    0x85, 0x13,          /* 80B6 STA $13 */
    0x20, 0x14, 0x81,    /* 80B8 JSR STOP */
    0x20, 0xF3, 0x80,    /* 80BB JSR START */
    0xA9, 0x91,          /* 80BE LDA #$91 */
    0x85, 0x10,          /* 80C0 STA $10 */
    0x20, 0x2D, 0x81,    /* 80C2 JSR SEND */
    0xA5, 0x12,          /* 80C5 LDA $12 */
    0x05, 0x13,          /* 80C7 ORA $13 */
    0x85, 0x13,          /* 80C9 STA $13 */
    0xA9, 0x00,          /* 80CB LDA #$00 */
    0x85, 0x12,          /* 80CD STA $12 */
    0x20, 0x79, 0x81,    /* 80CF JSR RECV */
    0xA5, 0x10,          /* 80D2 LDA $10 */
    0x8D, 0x04, 0x02,    /* 80D4 STA $0204 */
    0xA9, 0x01,          /* 80D7 LDA #$01 */
    0x85, 0x12,          /* 80D9 STA $12 */
    0x20, 0x79, 0x81,    /* 80DB JSR RECV */
    0xA5, 0x10,          /* 80DE LDA $10 */
    0x8D, 0x05, 0x02,    /* 80E0 STA $0205 */
    0x20, 0x14, 0x81,    /* 80E3 JSR STOP */
    0xA5, 0x13,          /* 80E6 LDA $13 */
    0x8D, 0x06, 0x02,    /* 80E8 STA $0206 */
    0xA9, 0xAA,          /* 80EB LDA #$AA */
    0x8D, 0x07, 0x02,    /* 80ED STA $0207 */
    /* HALT: */
    0x4C, 0xF0, 0x80,    /* 80F0 JMP HALT */
    /* START: */
    0xAD, 0x02, 0x60,    /* 80F3 LDA DDRB */
    0x29, 0xFE,          /* 80F6 AND #$FE */
    0x8D, 0x02, 0x60,    /* 80F8 STA DDRB */
    0xAD, 0x02, 0x60,    /* 80FB LDA DDRB */
    0x29, 0xFD,          /* 80FE AND #$FD */
    0x8D, 0x02, 0x60,    /* 8100 STA DDRB */
    0xAD, 0x02, 0x60,    /* 8103 LDA DDRB */
    0x09, 0x01,          /* 8106 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 8108 STA DDRB */
    0xAD, 0x02, 0x60,    /* 810B LDA DDRB */
    0x09, 0x02,          /* 810E ORA #$02 */
    0x8D, 0x02, 0x60,    /* 8110 STA DDRB */
    0x60,                /* 8113 RTS */
    /* STOP: */
    0xAD, 0x02, 0x60,    /* 8114 LDA DDRB */
    0x09, 0x01,          /* 8117 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 8119 STA DDRB */
    0xAD, 0x02, 0x60,    /* 811C LDA DDRB */
    0x29, 0xFD,          /* 811F AND #$FD */
    0x8D, 0x02, 0x60,    /* 8121 STA DDRB */
    0xAD, 0x02, 0x60,    /* 8124 LDA DDRB */
    0x29, 0xFE,          /* 8127 AND #$FE */
    0x8D, 0x02, 0x60,    /* 8129 STA DDRB */
    0x60,                /* 812C RTS */
    /* SEND: */
    0xA2, 0x08,          /* 812D LDX #$08 */
    /* SBIT: */
    0x06, 0x10,          /* 812F ASL $10 */
    0xB0, 0x0B,          /* 8131 BCS SONE */
    0xAD, 0x02, 0x60,    /* 8133 LDA DDRB */
    0x09, 0x01,          /* 8136 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 8138 STA DDRB */
    0x4C, 0x46, 0x81,    /* 813B JMP SCLK */
    /* SONE: */
    0xAD, 0x02, 0x60,    /* 813E LDA DDRB */
    0x29, 0xFE,          /* 8141 AND #$FE */
    0x8D, 0x02, 0x60,    /* 8143 STA DDRB */
    /* SCLK: */
    0xAD, 0x02, 0x60,    /* 8146 LDA DDRB */
    0x29, 0xFD,          /* 8149 AND #$FD */
    0x8D, 0x02, 0x60,    /* 814B STA DDRB */
    0xAD, 0x02, 0x60,    /* 814E LDA DDRB */
    0x09, 0x02,          /* 8151 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 8153 STA DDRB */
    0xCA,                /* 8156 DEX */
    0xD0, 0xD6,          /* 8157 BNE SBIT */
    0xAD, 0x02, 0x60,    /* 8159 LDA DDRB */
    0x29, 0xFE,          /* 815C AND #$FE */
    0x8D, 0x02, 0x60,    /* 815E STA DDRB */
    0xAD, 0x02, 0x60,    /* 8161 LDA DDRB */
    0x29, 0xFD,          /* 8164 AND #$FD */
    0x8D, 0x02, 0x60,    /* 8166 STA DDRB */
    0xAD, 0x00, 0x60,    /* 8169 LDA ORB */
    0x29, 0x01,          /* 816C AND #$01 */
    0x85, 0x12,          /* 816E STA $12 */
    0xAD, 0x02, 0x60,    /* 8170 LDA DDRB */
    0x09, 0x02,          /* 8173 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 8175 STA DDRB */
    0x60,                /* 8178 RTS */
    /* RECV: */
    0xAD, 0x02, 0x60,    /* 8179 LDA DDRB */
    0x29, 0xFE,          /* 817C AND #$FE */
    0x8D, 0x02, 0x60,    /* 817E STA DDRB */
    0xA9, 0x00,          /* 8181 LDA #$00 */
    0x85, 0x10,          /* 8183 STA $10 */
    0xA2, 0x08,          /* 8185 LDX #$08 */
    /* RBIT: */
    0xAD, 0x02, 0x60,    /* 8187 LDA DDRB */
    0x29, 0xFD,          /* 818A AND #$FD */
    0x8D, 0x02, 0x60,    /* 818C STA DDRB */
    0xAD, 0x00, 0x60,    /* 818F LDA ORB */
    0x4A,                /* 8192 LSR */
    0x26, 0x10,          /* 8193 ROL $10 */
    0xAD, 0x02, 0x60,    /* 8195 LDA DDRB */
    0x09, 0x02,          /* 8198 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 819A STA DDRB */
    0xCA,                /* 819D DEX */
    0xD0, 0xE7,          /* 819E BNE RBIT */
    0xA5, 0x12,          /* 81A0 LDA $12 */
    0xD0, 0x0B,          /* 81A2 BNE RNACK */
    0xAD, 0x02, 0x60,    /* 81A4 LDA DDRB */
    0x09, 0x01,          /* 81A7 ORA #$01 */
    0x8D, 0x02, 0x60,    /* 81A9 STA DDRB */
    0x4C, 0xB7, 0x81,    /* 81AC JMP RCLK */
    /* RNACK: */
    0xAD, 0x02, 0x60,    /* 81AF LDA DDRB */
    0x29, 0xFE,          /* 81B2 AND #$FE */
    0x8D, 0x02, 0x60,    /* 81B4 STA DDRB */
    /* RCLK: */
    0xAD, 0x02, 0x60,    /* 81B7 LDA DDRB */
    0x29, 0xFD,          /* 81BA AND #$FD */
    0x8D, 0x02, 0x60,    /* 81BC STA DDRB */
    0xAD, 0x02, 0x60,    /* 81BF LDA DDRB */
    0x09, 0x02,          /* 81C2 ORA #$02 */
    0x8D, 0x02, 0x60,    /* 81C4 STA DDRB */
    0xAD, 0x02, 0x60,    /* 81C7 LDA DDRB */
    0x29, 0xFE,          /* 81CA AND #$FE */
    0x8D, 0x02, 0x60,    /* 81CC STA DDRB */
    0x60,                /* 81CF RTS */
};

static size_t board_net(Board *b, const char *name) {
    size_t i;

    for (i = 0; i < b->net_count; i++) {
        if (strcmp(b->names[i], name) == 0) {
            return b->nets[i];
        }
    }
    snprintf(b->names[b->net_count], sizeof b->names[0], "%s", name);
    b->nets[b->net_count] = net(b->c);
    return b->nets[b->net_count++];
}

/* A chip with its power and ground pins on the supplies. */
static size_t board_chip(Board *b, const char *name) {
    const Part *p = part_library_find(library, name);
    size_t index = part(b->c, name);
    size_t i;

    for (i = 0; p != NULL && i < p->pin_count; i++) {
        if (p->pins[i].role == PIN_POWER || p->pins[i].role == PIN_GROUND) {
            circuit_connect(b->c, index, p->pins[i].designator,
                            p->pins[i].role == PIN_POWER ? b->v : b->g);
        }
    }
    return index;
}

static void board_wire(Board *b, size_t chip, const char *part_name, const char *pin,
                       const char *net_name) {
    size_t n = strcmp(net_name, "ONE") == 0 ? b->v
             : strcmp(net_name, "ZERO") == 0 ? b->g
                                             : board_net(b, net_name);

    if (circuit_connect(b->c, chip, pin_named(part_name, pin), n) != 0) {
        printf("  connect %s %s: %s\n", part_name, pin, circuit_error(b->c));
    }
}

static void board_bus(Board *b, size_t chip, const char *part_name, const char *prefix,
                      const char *net_prefix, int n) {
    char pin[16];
    char name[8];
    int i;

    for (i = 0; i < n; i++) {
        snprintf(pin, sizeof pin, "%s%d", prefix, i);
        snprintf(name, sizeof name, "%s%d", net_prefix, i);
        board_wire(b, chip, part_name, pin, name);
    }
}

static void board_pullup(Board *b, const char *net_name) {
    circuit_connect(b->c, part(b->c, "PULLUP-5V"), "1", board_net(b, net_name));
}

static void check_adc_from_cpu(void) {
    static const char *const GATES[10][3] = {
        {"A15", "A15", "G1"}, {"A14", "A14", "G2"}, {"G1", "G2", "G3"}, {"G1", "A14", "G4"},
        {"A13", "A13", "G5"}, {"A12", "A12", "G6"}, {"G5", "G6", "G7"}, {"G7", "G7", "G8"},
        {"RW", "RW", "G9"}, {"CLK", "G9", "G10"},
    };
    static uint32_t words[32768];
    static Board board;
    Board *b = &board;
    size_t cpu, rom, ram, via, adc, osc, sw, nand[3];
    uint32_t values[8];
    uint32_t known[8];
    CpuState state;
    size_t i;
    int step;

    memset(b, 0, sizeof *b);
    b->c = circuit_create(library);
    circuit_connect(b->c, part(b->c, "5V"), "1", b->v = net(b->c));
    circuit_connect(b->c, part(b->c, "GND"), "1", b->g = net(b->c));
    cpu = board_chip(b, "W65C02S6TPG-14");
    rom = board_chip(b, "AT28C256-15PU");
    ram = board_chip(b, "AS6C62256-55PCN");
    via = board_chip(b, "W65C22S6TPG-14");
    adc = board_chip(b, "ADS1115IDGSR");
    osc = board_chip(b, "ECS-100A-010");
    for (i = 0; i < 3; i++) {
        nand[i] = board_chip(b, "SN74HC00N");
    }
    sw = part(b->c, "TE-GDS0604");

    /* The clock (1 MHz) and the reset: a switch to GND on a pulled-up RES. */
    board_wire(b, osc, "ECS-100A-010", "OUTPUT", "CLK");
    board_pullup(b, "RES");
    circuit_connect(b->c, sw, pin_named("TE-GDS0604", "1A"), board_net(b, "RES"));
    circuit_connect(b->c, sw, pin_named("TE-GDS0604", "1B"), b->g);

    board_bus(b, cpu, "W65C02S6TPG-14", "A", "A", 16);
    board_bus(b, cpu, "W65C02S6TPG-14", "D", "D", 8);
    board_wire(b, cpu, "W65C02S6TPG-14", "PHI2", "CLK");
    board_wire(b, cpu, "W65C02S6TPG-14", "RESB", "RES");
    board_wire(b, cpu, "W65C02S6TPG-14", "RWB", "RW");
    board_wire(b, cpu, "W65C02S6TPG-14", "IRQB", "IRQ");
    board_wire(b, cpu, "W65C02S6TPG-14", "NMIB", "ONE");
    board_wire(b, cpu, "W65C02S6TPG-14", "RDY", "ONE");
    board_wire(b, cpu, "W65C02S6TPG-14", "SOB", "ONE");
    board_wire(b, cpu, "W65C02S6TPG-14", "BE", "ONE");
    for (i = 0; i < 10; i++) {
        char pin[4];
        const char *const name = "SN74HC00N";

        snprintf(pin, sizeof pin, "%zuA", i % 4 + 1);
        board_wire(b, nand[i / 4], name, pin, GATES[i][0]);
        snprintf(pin, sizeof pin, "%zuB", i % 4 + 1);
        board_wire(b, nand[i / 4], name, pin, GATES[i][1]);
        snprintf(pin, sizeof pin, "%zuY", i % 4 + 1);
        board_wire(b, nand[i / 4], name, pin, GATES[i][2]);
    }
    board_bus(b, rom, "AT28C256-15PU", "A", "A", 15);
    board_bus(b, rom, "AT28C256-15PU", "I/O", "D", 8);
    board_wire(b, rom, "AT28C256-15PU", "CE#", "G1");
    board_wire(b, rom, "AT28C256-15PU", "OE#", "G9");
    board_wire(b, rom, "AT28C256-15PU", "WE#", "ONE");
    board_bus(b, ram, "AS6C62256-55PCN", "A", "A", 15);
    board_bus(b, ram, "AS6C62256-55PCN", "DQ", "D", 8);
    board_wire(b, ram, "AS6C62256-55PCN", "CE#", "G3");
    board_wire(b, ram, "AS6C62256-55PCN", "OE#", "G9");
    board_wire(b, ram, "AS6C62256-55PCN", "WE#", "G10");
    board_bus(b, via, "W65C22S6TPG-14", "D", "D", 8);
    board_bus(b, via, "W65C22S6TPG-14", "RS", "A", 4);
    board_wire(b, via, "W65C22S6TPG-14", "PHI2", "CLK");
    board_wire(b, via, "W65C22S6TPG-14", "RESB", "RES");
    board_wire(b, via, "W65C22S6TPG-14", "RWB", "RW");
    board_wire(b, via, "W65C22S6TPG-14", "CS1", "A13");
    board_wire(b, via, "W65C22S6TPG-14", "CS2B", "G4");
    board_wire(b, via, "W65C22S6TPG-14", "IRQB", "IRQ");
    board_wire(b, via, "W65C22S6TPG-14", "PB0", "SDA");
    board_wire(b, via, "W65C22S6TPG-14", "PB1", "SCL");
    /* PB2-PB7 are read with PB0: pulled low so the byte is known (a pull,
       not GND: before reset the VIA drives its ports X). */
    for (i = 2; i < 8; i++) {
        char pin[4];

        snprintf(pin, sizeof pin, "PB%zu", i);
        board_wire(b, via, "W65C22S6TPG-14", pin, "PBX");
    }
    circuit_connect(b->c, part(b->c, "PULLDOWN"), "1", board_net(b, "PBX"));

    /* The ADC on the bus, with the pull-ups. */
    board_wire(b, adc, "ADS1115IDGSR", "SDA", "SDA");
    board_wire(b, adc, "ADS1115IDGSR", "SCL", "SCL");
    board_wire(b, adc, "ADS1115IDGSR", "ADDR", "ZERO");
    board_wire(b, adc, "ADS1115IDGSR", "ALERT/RDY", "ALERT");
    board_pullup(b, "SDA");
    board_pullup(b, "SCL");
    board_pullup(b, "ALERT");

    for (i = 0; i < 32768; i++) {
        words[i] = 0xFF;
    }
    for (i = 0; i < sizeof ADC_PROGRAM; i++) {
        words[i] = ADC_PROGRAM[i];
    }
    words[0x7FFC] = 0x00; /* RESET vector: 8000 */
    words[0x7FFD] = 0x80;
    expect(circuit_memory_load(b->c, rom, 0, words, 32768) == 0, "adc board: the program loads");
    circuit_set_switch(b->c, sw, 1, 1);
    expect(circuit_start(b->c) == 0, "adc board: the circuit starts");
    circuit_advance(b->c, 10000000); /* RES low for 10 cycles */
    circuit_set_switch(b->c, sw, 1, 0);
    for (step = 0; step < 40; step++) {
        if (!circuit_advance(b->c, 500000000)) {
            printf("  stopped: %s\n", circuit_error(b->c) ? circuit_error(b->c) : "?");
            break;
        }
        if (circuit_memory_read(b->c, ram, 0x0207, 1, values, known) == 1 && known[0] == 0xFF &&
            values[0] == 0xAA) {
            break;
        }
    }
    expect(circuit_cpu_state(b->c, cpu, &state) == 0 && state.status == CPU_RUNNING,
           "adc board: the CPU runs");
    printf("  %llu cycles, at %04X %s\n", (unsigned long long)state.cycles,
           (unsigned)state.instruction_address, state.instruction_text);
    expect(circuit_memory_read(b->c, ram, 0x0200, 8, values, known) == 8,
           "adc board: the results read back");
    for (i = 0; i < 8; i++) {
        if (known[i] != 0xFF) {
            values[i] = 0x100;
        }
    }
    printf("  results: %02X %02X %02X %02X %02X %02X %02X %02X\n", values[0], values[1],
           values[2], values[3], values[4], values[5], values[6], values[7]);
    expect(values[7] == 0xAA, "adc board: the program finishes");
    expect(values[0] == 0x00 && values[6] == 0x00, "adc board: the ADS1115 acknowledges every byte");
    expect(values[1] == 0x05, "adc board: the first poll reads CONFIG 05xx (OS = 0, converting)");
    expect(values[2] >= 1, "adc board: the program polls while OS = 0");
    expect(values[3] == 0x85, "adc board: OS reads 1 when the conversion is done (CONFIG 85xx)");
    expect(values[4] == 0x00 && values[5] == 0x00, "adc board: CONVERSION reads 0x0000 (0 V)");

    /* plan-24-I: the register window's snapshot of the ADC. */
    {
        ConverterRegister regs[8];
        int n = circuit_converter_registers(b->c, adc, regs, 8);

        expect(n == 4 && strcmp(regs[0].name, "CONVERSION") == 0 && regs[0].address == 0 &&
                   regs[0].width == 16 && regs[0].value == 0,
               "registers: 4, CONVERSION at 0, 16 bits, 0x0000");
        expect(n == 4 && strcmp(regs[1].name, "CONFIG") == 0 && regs[1].address == 1 &&
                   regs[1].value == 0x85A3,
               "registers: CONFIG holds the written 0x85A3 (OS = 1: done)");
        if (n == 4) {
            printf("  registers: %s=%04X %s=%04X %s=%04X %s=%04X\n", regs[0].name, regs[0].value,
                   regs[1].name, regs[1].value, regs[2].name, regs[2].value, regs[3].name,
                   regs[3].value);
        }
        expect(n == 4 && strcmp(regs[2].name, "LO_THRESH") == 0 && regs[2].value == 0x8000 &&
                   regs[3].value == 0x7FFF,
               "registers: the thresholds keep their reset values");
        expect(circuit_converter_registers(b->c, adc, NULL, 0) == 4 &&
                   circuit_converter_registers(b->c, adc, regs, 1) == 4,
               "registers: the count with no room, or room for one");
        expect_error(b->c, circuit_converter_registers(b->c, cpu, regs, 8), "part 2 has no registers",
                     "registers: a CPU has none");
        expect_error(b->c, circuit_converter_registers(b->c, 999, regs, 8), "no part 999",
                     "registers: no part 999");
        expect_error(b->c, circuit_rtc_time(b->c, adc, &(RtcTime){0}), "part 6 is not an RTC",
                     "rtc time: an ADC is not an RTC");
    }
    circuit_destroy(b->c);
}

/* plan-24-I: an RTC's clock follows simulated time from 2000-01-01
   00:00:00 (a Saturday); its registers show the counters in BCD. */
/* A real-part RTC on a 3V3 I2C bus: `name` with its data, clock and interrupt pins; `sec` and `min`
   are the addresses of its seconds and minutes registers. */
static void check_rtc_time(const char *name, const char *sda, const char *scl, const char *irq, unsigned sec,
                           unsigned min) {
    Board board;
    Board *b = &board;
    size_t rtc;
    RtcTime t;
    ConverterRegister regs[32];
    int n, i, si = -1, mi = -1;

    memset(b, 0, sizeof *b);
    b->c = circuit_create(library);
    circuit_connect(b->c, part(b->c, "3V3"), "1", b->v = net(b->c));
    circuit_connect(b->c, part(b->c, "GND"), "1", b->g = net(b->c));
    printf("  rtc part %s\n", name);
    rtc = board_chip(b, name);
    board_wire(b, rtc, name, sda, "SDA");
    board_wire(b, rtc, name, scl, "SCL");
    board_wire(b, rtc, name, irq, "IRQ");
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SDA"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SCL"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "IRQ"));
    expect_error(b->c, circuit_rtc_time(b->c, rtc, &t), "the circuit is not running",
                 "rtc time: refused before start");
    expect(circuit_start(b->c) == 0, "rtc: the circuit starts");
    expect(circuit_rtc_time(b->c, rtc, &t) == 0 && t.year == 2000 && t.month == 1 && t.day == 1 &&
               t.weekday == 6 && t.hour == 0 && t.minute == 0 && t.second == 0 && t.running,
           "rtc time: 2000-01-01 00:00:00, a Saturday, running");
    circuit_advance(b->c, 2500000000000ull);
    expect(circuit_rtc_time(b->c, rtc, &t) == 0 && t.second == 2 && t.minute == 0,
           "rtc time: 2 s later at 2.5 s");
    circuit_advance(b->c, 60000000000000ull);
    expect(circuit_rtc_time(b->c, rtc, &t) == 0 && t.second == 2 && t.minute == 1,
           "rtc time: a minute later 00:01:02");
    printf("  rtc %04d-%02d-%02d %02d:%02d:%02d weekday %d\n", t.year, t.month, t.day, t.hour,
           t.minute, t.second, t.weekday);
    n = circuit_converter_registers(b->c, rtc, regs, 32);
    for (i = 0; i < n && i < 32; i++) {
        if (regs[i].address == sec) {
            si = i;
        }
        if (regs[i].address == min) {
            mi = i;
        }
    }
    expect(si >= 0 && mi >= 0 && (regs[si].value & 0x7F) == 0x02 && (regs[mi].value & 0x7F) == 0x01,
           "rtc registers: seconds 02, minutes 01 (BCD)");
    if (si >= 0 && mi >= 0) {
        printf("  rtc registers: %d, %s=%02X %s=%02X\n", n, regs[si].name, regs[si].value, regs[mi].name,
               regs[mi].value);
    }
    circuit_destroy(b->c);
}

/* The value of the register called `name` (or at `address` when name is NULL); -1 = not found. */
static int converter_register(Circuit *c, size_t chip, const char *name, unsigned address) {
    ConverterRegister regs[32];
    int n = circuit_converter_registers(c, chip, regs, 32);
    int i;

    for (i = 0; i < n && i < 32; i++) {
        if (name != NULL ? strcmp(regs[i].name, name) == 0 : regs[i].address == address) {
            return (int)regs[i].value;
        }
    }
    return -1;
}

/* plan-43-D: the reference boards' converters start, with their registers at the power-on values. */
static void check_reference_converters(void) {
    Board board;
    Board *b = &board;
    size_t adc, idac, dac;
    int config;

    memset(b, 0, sizeof *b);
    b->c = circuit_create(library);
    circuit_connect(b->c, part(b->c, "3V3"), "1", b->v = net(b->c));
    circuit_connect(b->c, part(b->c, "GND"), "1", b->g = net(b->c));
    adc = board_chip(b, "MCP3428-E/ST");
    board_wire(b, adc, "MCP3428-E/ST", "SDA", "SDA");
    board_wire(b, adc, "MCP3428-E/ST", "SCL", "SCL");
    board_wire(b, adc, "MCP3428-E/ST", "Adr0", "ZERO");
    board_wire(b, adc, "MCP3428-E/ST", "Adr1", "ZERO");
    idac = board_chip(b, "DS4432U+");
    board_wire(b, idac, "DS4432U+", "SDA", "SDA");
    board_wire(b, idac, "DS4432U+", "SCL", "SCL");
    dac = board_chip(b, "AD5621AKSZ-REEL7");
    board_wire(b, dac, "AD5621AKSZ-REEL7", "SYNC", "SYNC");
    board_wire(b, dac, "AD5621AKSZ-REEL7", "SCLK", "SCLK");
    board_wire(b, dac, "AD5621AKSZ-REEL7", "SDIN", "SDIN");
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SDA"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SCL"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SYNC"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SCLK"));
    circuit_connect(b->c, part(b->c, "PULLUP-3V3"), "1", board_net(b, "SDIN"));
    expect(circuit_start(b->c) == 0, "reference converters: the circuit starts (no smoke, no voltage problem)");
    if (circuit_error(b->c) != NULL) {
        printf("  %s\n", circuit_error(b->c));
    }
    config = converter_register(b->c, adc, "CONFIG", 0);
    printf("  MCP3428 CONFIG=%02X DS4432 F8=%02X F9=%02X AD5621 DAC=%03X\n", (unsigned)config,
           (unsigned)converter_register(b->c, idac, NULL, 0xF8), (unsigned)converter_register(b->c, idac, NULL, 0xF9),
           (unsigned)converter_register(b->c, dac, "DAC", 0));
    expect(config >= 0 && (config & 0x7F) == 0x10,
           "MCP3428: channel 1, continuous, 240 SPS, gain 1 at power on (90h; RDY is the status bit)");
    expect(converter_register(b->c, idac, NULL, 0xF8) == 0 && converter_register(b->c, idac, NULL, 0xF9) == 0,
           "DS4432: F8h and F9h are 00h at power on (zero current)");
    expect(converter_register(b->c, dac, "DAC", 0) == 0, "AD5621: the DAC register is zero at power on");
    circuit_destroy(b->c);
}

int main(void) {
    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_voltage_fixture(library) != 0 ||
        load_serial_fixture(library) != 0 || load_serial_fixture_classic(library) != 0) {
        return 1;
    }
    check_adc_from_cpu();
    check_rtc_time("BQ32000DR", "SDA", "SCL", "IRQ", 0, 1);
    check_rtc_time("DS3231MZ+TRL", "SDA", "SCL", "INT/SQW", 0, 1);
    check_rtc_time("PCF8563T/5,518", "SDA", "SCL", "INT", 2, 3);
    check_reference_converters();
    part_library_destroy(library);
    printf("%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
