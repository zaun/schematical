/*
 * charlcd_board_test: split from charlcd_test.c by plan-68-H; the shared helpers are
 * support/charlcd_bench.c.
 */

#include "charlcd_bench.h"


static size_t board_net(Board *b, const char *name) {
    size_t i;

    for (i = 0; i < b->count; i++) {
        if (strcmp(b->names[i], name) == 0) {
            return b->nets[i];
        }
    }
    b->names[b->count] = name;
    circuit_add_net(b->c, &b->nets[b->count]);
    return b->nets[b->count++];
}

/* Connects every pin of `part` named `pin` to the net `net`. */
static void board_wire(Board *b, size_t index, const char *part_name, const char *pin,
                       const char *net) {
    const Part *part = part_library_find(real_library, part_name);
    size_t i, n = 0;

    for (i = 0; part != NULL && i < part->pin_count; i++) {
        if (strcmp(part->pins[i].name, pin) == 0) {
            if (circuit_connect(b->c, index, part->pins[i].designator, board_net(b, net)) != 0) {
                b->failed = 1;
            }
            n++;
        }
    }
    if (n == 0) {
        fprintf(stderr, "board: no pin %s on %s\n", pin, part_name);
        b->failed = 1;
    }
}

static size_t board_part(Board *b, const char *name) {
    size_t index = 0;

    if (circuit_add_part(b->c, name, &index) != 0) {
        fprintf(stderr, "board: %s\n", circuit_error(b->c));
        b->failed = 1;
    }
    return index;
}

/* The plan-11-C section 4.6 program, at EEPROM offset 0 (address 8000). It
   starts with an unpolled Set DDRAM Address (80): while the CPU is held in
   reset its address bus is X, so E pulses X and AC may be X (a possible
   read); 80 makes AC known before the first BF poll, and is ignored when
   the LCD is still busy. */
static const uint8_t HELLO_PROGRAM[] = {
    0xA9, 0x80, 0x8D, 0x00, 0x40,
    0xA2, 0x00, 0xBD, 0x2C, 0x80, 0xF0, 0x0B, 0x2C, 0x00, 0x40, 0x30, 0xFB, 0x8D, 0x00, 0x40,
    0xE8, 0xD0, 0xF0, 0xA2, 0x00, 0xBD, 0x31, 0x80, 0xF0, 0x0B, 0x2C, 0x00, 0x40, 0x30, 0xFB,
    0x8D, 0x01, 0x40, 0xE8, 0xD0, 0xF0, 0x4C, 0x29, 0x80, 0x38, 0x0C, 0x01, 0x06, 0x00, 0x48,
    0x45, 0x4C, 0x4C, 0x4F, 0x00
};

static const BoardSpec HELLO_BOARD = {"NHD-0216K1Z-NSW-BBW-L", 0, HELLO_PROGRAM,
                                      sizeof HELLO_PROGRAM, 0x8029};

/* Connects the pin with this designator to the net `net`. */
static void board_wire_pin(Board *b, size_t index, const char *designator, const char *net) {
    if (circuit_connect(b->c, index, designator, board_net(b, net)) != 0) {
        b->failed = 1;
    }
}

/* The LCD: power and ground pins by role, V0 on GND, LED+ on 5V, LED- on
   GND, and its bus by bus_pins: RS = A0, R/W, E (or E1, E2), DB = D0-D7. */
static void board_wire_lcd(Board *b, size_t index, const char *name, const char *const *data_names) {
    const Part *part = part_library_find(real_library, name);
    const PartCharLcd *c = part != NULL ? part->charlcd : NULL;
    size_t i, n;

    if (c == NULL || c->bus_pin_count != 2 + c->enables + 8) {
        fprintf(stderr, "board: %s is not a charlcd\n", name);
        b->failed = 1;
        return;
    }
    for (i = 0; i < part->pin_count; i++) {
        const PartPin *pin = &part->pins[i];

        if (pin->role == PIN_GROUND || strcmp(pin->name, "V0") == 0 || strcmp(pin->name, "LED-") == 0) {
            board_wire_pin(b, index, pin->designator, "GND");
        } else if (pin->role == PIN_POWER || strcmp(pin->name, "LED+") == 0) {
            board_wire_pin(b, index, pin->designator, "5V");
        }
    }
    n = 0;
    board_wire_pin(b, index, part->pins[c->bus_pins[n++]].designator, "A0");
    board_wire_pin(b, index, part->pins[c->bus_pins[n++]].designator, "RW");
    if (c->enables == 2) {
        board_wire_pin(b, index, part->pins[c->bus_pins[n++]].designator, "E1");
        board_wire_pin(b, index, part->pins[c->bus_pins[n++]].designator, "E2");
    } else {
        board_wire_pin(b, index, part->pins[c->bus_pins[n++]].designator, "E");
    }
    for (i = 0; i < 8; i++) {
        board_wire_pin(b, index, part->pins[c->bus_pins[n++]].designator, data_names[i]);
    }
}

/* Builds the board, holds reset for `reset_ps`, then runs 50 ms; wdc picks
   the W65C02S. */
static void run_board(int wdc, const BoardSpec *spec, SimTime reset_ps, LcdState *lcd, CpuState *cpu,
                      int *ok) {
    static uint32_t image[32768];
    const char *cpu_name = wdc ? "W65C02S6TPG-14" : "MPS6502";
    Board b;
    size_t lcd_i, cpu_i, ee, hc00, hc11, osc, sw, p5, pg;
    unsigned i;
    char name[8];
    static char addr_names[16][8], data_names[8][8];
    const char *data_list[8];

    memset(&b, 0, sizeof b);
    b.c = circuit_create(real_library);
    p5 = board_part(&b, "5V");
    pg = board_part(&b, "GND");
    osc = board_part(&b, "ECS-2100A-010");
    cpu_i = board_part(&b, cpu_name);
    ee = board_part(&b, "AT28C256-15PU");
    hc00 = board_part(&b, "SN74HC00N");
    hc11 = board_part(&b, "SN74HC11N");
    sw = board_part(&b, "TE-GDS0204");
    lcd_i = board_part(&b, spec->lcd);

    board_wire(&b, p5, "5V", "VCC", "5V");
    board_wire(&b, pg, "GND", "GND", "GND");
    /* Power. */
    board_wire(&b, cpu_i, cpu_name, wdc ? "VDD" : "VCC", "5V");
    board_wire(&b, cpu_i, cpu_name, "VSS", "GND");
    board_wire(&b, ee, "AT28C256-15PU", "VCC", "5V");
    board_wire(&b, ee, "AT28C256-15PU", "GND", "GND");
    board_wire(&b, ee, "AT28C256-15PU", "WE#", "5V");
    board_wire(&b, hc00, "SN74HC00N", "VCC", "5V");
    board_wire(&b, hc00, "SN74HC00N", "GND", "GND");
    board_wire(&b, hc11, "SN74HC11N", "VCC", "5V");
    board_wire(&b, hc11, "SN74HC11N", "GND", "GND");
    board_wire(&b, osc, "ECS-2100A-010", "+5V DC", "5V");
    board_wire(&b, osc, "ECS-2100A-010", "GND", "GND");
    /* Address and data. */
    for (i = 0; i < 16; i++) {
        snprintf(addr_names[i], sizeof addr_names[i], "A%u", i);
        board_wire(&b, cpu_i, cpu_name, addr_names[i], addr_names[i]);
        if (i < 15) {
            board_wire(&b, ee, "AT28C256-15PU", addr_names[i], addr_names[i]);
        }
    }
    for (i = 0; i < 8; i++) {
        snprintf(data_names[i], sizeof data_names[i], "D%u", i);
        data_list[i] = data_names[i];
        board_wire(&b, cpu_i, cpu_name, data_names[i], data_names[i]);
        snprintf(name, sizeof name, "I/O%u", i);
        board_wire(&b, ee, "AT28C256-15PU", name, data_names[i]);
    }
    /* The LCD: power, RS = A0, R/W, E (E1, E2), DB0-7 on D0-7. */
    board_wire_lcd(&b, lcd_i, spec->lcd, data_list);
    /* Decode. */
    board_wire(&b, hc00, "SN74HC00N", "1A", "A15");
    board_wire(&b, hc00, "SN74HC00N", "1B", "A15");
    board_wire(&b, hc00, "SN74HC00N", "1Y", "NA15");
    board_wire(&b, ee, "AT28C256-15PU", "CE#", "NA15");
    board_wire(&b, cpu_i, cpu_name, wdc ? "RWB" : "R/W", "RW");
    board_wire(&b, hc00, "SN74HC00N", "2A", "RW");
    board_wire(&b, hc00, "SN74HC00N", "2B", "RW");
    board_wire(&b, hc00, "SN74HC00N", "2Y", "OE");
    board_wire(&b, ee, "AT28C256-15PU", "OE#", "OE");
    if (spec->two) {
        /* plan-35-E section 4.3: E1 = PHI2 . A14 . /A15 . /A13 and
           E2 = PHI2 . A14 . /A15 . A13. The address terms are gated first
           (SEL = A14 . /A15, /A13 from HC00 gate 3), so PHI2 still reaches
           E1 and E2 through one gate (10 ns), as on the one-enable board;
           SEL and /A13 settle while PHI2 is low. */
        board_wire(&b, hc00, "SN74HC00N", "3A", "A13");
        board_wire(&b, hc00, "SN74HC00N", "3B", "A13");
        board_wire(&b, hc00, "SN74HC00N", "3Y", "NA13");
        board_wire(&b, hc11, "SN74HC11N", "1A", "A14");
        board_wire(&b, hc11, "SN74HC11N", "1B", "NA15");
        board_wire(&b, hc11, "SN74HC11N", "1C", "5V");
        board_wire(&b, hc11, "SN74HC11N", "1Y", "SEL");
        board_wire(&b, hc11, "SN74HC11N", "2A", "PHI2");
        board_wire(&b, hc11, "SN74HC11N", "2B", "SEL");
        board_wire(&b, hc11, "SN74HC11N", "2C", "NA13");
        board_wire(&b, hc11, "SN74HC11N", "2Y", "E1");
        board_wire(&b, hc11, "SN74HC11N", "3A", "PHI2");
        board_wire(&b, hc11, "SN74HC11N", "3B", "SEL");
        board_wire(&b, hc11, "SN74HC11N", "3C", "A13");
        board_wire(&b, hc11, "SN74HC11N", "3Y", "E2");
    } else {
        board_wire(&b, hc11, "SN74HC11N", "1A", "PHI2");
        board_wire(&b, hc11, "SN74HC11N", "1B", "A14");
        board_wire(&b, hc11, "SN74HC11N", "1C", "NA15");
        board_wire(&b, hc11, "SN74HC11N", "1Y", "E");
    }
    /* Clock and reset. */
    if (wdc) {
        board_wire(&b, osc, "ECS-2100A-010", "OUTPUT", "PHI2");
        board_wire(&b, cpu_i, cpu_name, "PHI2", "PHI2");
        board_wire(&b, cpu_i, cpu_name, "RESB", "RES");
    } else {
        board_wire(&b, osc, "ECS-2100A-010", "OUTPUT", "PHI0");
        board_wire(&b, cpu_i, cpu_name, "PHI0IN", "PHI0");
        board_wire(&b, cpu_i, cpu_name, "PHI2OUT", "PHI2");
        board_wire(&b, cpu_i, cpu_name, "RES#", "RES");
    }
    board_wire(&b, sw, "TE-GDS0204", "1A", "RES");
    board_wire(&b, sw, "TE-GDS0204", "1B", "GND");

    for (i = 0; i < 32768; i++) {
        image[i] = i < spec->size ? spec->program[i] : 0xFF;
    }
    image[0x7FFA] = spec->halt & 0xFFu;   /* NMI -> halt */
    image[0x7FFB] = spec->halt >> 8;
    image[0x7FFC] = 0x00;                  /* RESET -> 8000 */
    image[0x7FFD] = 0x80;
    image[0x7FFE] = spec->halt & 0xFFu;   /* IRQ -> halt */
    image[0x7FFF] = spec->halt >> 8;
    *ok = !b.failed && circuit_memory_load(b.c, ee, 0, image, 32768) == 0 &&
          circuit_set_switch(b.c, sw, 1, 1) == 0 && circuit_start(b.c) == 0;
    if (!*ok) {
        fprintf(stderr, "board: %s\n", circuit_error(b.c) != NULL ? circuit_error(b.c) : "build failed");
    }
    if (*ok) {
        *ok = circuit_advance(b.c, reset_ps) && circuit_set_switch(b.c, sw, 1, 0) == 0;
        for (i = 0; *ok && i < 50; i++) {
            *ok = circuit_advance(b.c, MS(1));
        }
        if (!*ok) {
            fprintf(stderr, "board: %s\n", circuit_error(b.c) != NULL ? circuit_error(b.c) : "stopped");
        }
    }
    memset(lcd, 0, sizeof *lcd);
    memset(cpu, 0, sizeof *cpu);
    if (*ok) {
        *ok = circuit_lcd_state(b.c, lcd_i, lcd) == 0 && circuit_cpu_state(b.c, cpu_i, cpu) == 0;
    }
    circuit_destroy(b.c);
}

static void asm_byte(Asm *a, unsigned byte) {
    a->bytes[a->pc++ - 0x8000u] = (uint8_t)byte;
}

static void asm_abs(Asm *a, unsigned opcode, unsigned address) {
    asm_byte(a, opcode);
    asm_byte(a, address & 0xFFu);
    asm_byte(a, address >> 8);
}

enum { LCD_LOOP_SIZE = 29 };

/* Sends the list at `list` to the LCD at `base` (instructions; base + 1
   data). The list is pairs (0, instruction) / (1, data byte), ended by FF.
   Each pair waits for BF 0 with BIT base. No indexed store touches the LCD
   (STA abs,Y would first read base + Y: a data read moves AC).
       LDX #0
   P:  BIT base ; BMI P
       LDA list+1,X ; LDY list,X ; BMI END ; BNE DATA
       STA base ; BEQ NEXT
   DATA: STA base+1
   NEXT: INX ; INX ; BNE P
   END: */
static void asm_lcd_loop(Asm *a, unsigned base, unsigned list) {
    unsigned p;

    asm_byte(a, 0xA2);
    asm_byte(a, 0x00);
    p = a->pc;
    asm_abs(a, 0x2C, base);
    asm_byte(a, 0x30);
    asm_byte(a, (uint8_t)(p - (a->pc + 1)));
    asm_abs(a, 0xBD, list + 1);
    asm_abs(a, 0xBC, list);
    asm_byte(a, 0x30);
    asm_byte(a, 0x0E);       /* BMI END: +14 */
    asm_byte(a, 0xD0);
    asm_byte(a, 0x05);       /* BNE DATA: +5 */
    asm_abs(a, 0x8D, base);
    asm_byte(a, 0xF0);
    asm_byte(a, 0x03);       /* BEQ NEXT: +3 (Z from LDY #0) */
    asm_abs(a, 0x8D, base + 1);
    asm_byte(a, 0xE8);
    asm_byte(a, 0xE8);
    asm_byte(a, 0xD0);
    asm_byte(a, (uint8_t)(p - (a->pc + 1)));
}

static void asm_command(Asm *a, unsigned byte) {
    asm_byte(a, 0x00);
    asm_byte(a, byte);
}

static void asm_text(Asm *a, const char *s) {
    for (; *s != '\0'; s++) {
        asm_byte(a, 0x01);
        asm_byte(a, (unsigned char)*s);
    }
}

static void asm_end(Asm *a) {
    asm_byte(a, 0xFF);
    asm_byte(a, 0x00);
}

/* Function set (8-bit, 2 lines), display on, clear, entry mode. */
static void asm_init(Asm *a) {
    asm_command(a, 0x38);
    asm_command(a, 0x0C);
    asm_command(a, 0x01);
    asm_command(a, 0x06);
}

/* Board 1: an unpolled 80 (as HELLO_PROGRAM), then LINE1 at 00, LINE2 at
   40, LINE3 at 14, LINE4 at 54. */
static BoardSpec board_2004(Asm *a) {
    unsigned list = 0x8000 + 5 + LCD_LOOP_SIZE + 3, halt;
    BoardSpec spec;

    a->pc = 0x8000;
    asm_byte(a, 0xA9);
    asm_byte(a, 0x80);
    asm_abs(a, 0x8D, 0x4000);
    asm_lcd_loop(a, 0x4000, list);
    halt = a->pc;
    asm_abs(a, 0x4C, halt);
    asm_init(a);
    asm_text(a, "LINE1");
    asm_command(a, 0x80 | 0x40);
    asm_text(a, "LINE2");
    asm_command(a, 0x80 | 0x14);
    asm_text(a, "LINE3");
    asm_command(a, 0x80 | 0x54);
    asm_text(a, "LINE4");
    asm_end(a);
    spec.lcd = "NHD-0420DZ-FL-YBW";
    spec.two = 0;
    spec.program = a->bytes;
    spec.size = a->pc - 0x8000;
    spec.halt = halt;
    return list == halt + 3 ? spec : (spec.size = 0, spec);
}

/* Board 2: an unpolled 80 to each controller, then E1 init + TOP, then
   E2 init + BOTTOM. */
static BoardSpec board_4004(Asm *a) {
    unsigned list1 = 0x8000 + 8 + 2 * LCD_LOOP_SIZE + 3, list2, halt;
    BoardSpec spec;

    list2 = list1 + 2 * (4 + 3) + 2;
    a->pc = 0x8000;
    asm_byte(a, 0xA9);
    asm_byte(a, 0x80);
    asm_abs(a, 0x8D, 0x4000);
    asm_abs(a, 0x8D, 0x6000);
    asm_lcd_loop(a, 0x4000, list1);
    asm_lcd_loop(a, 0x6000, list2);
    halt = a->pc;
    asm_abs(a, 0x4C, halt);
    asm_init(a);
    asm_text(a, "TOP");
    asm_end(a);
    asm_init(a);
    asm_text(a, "BOTTOM");
    asm_end(a);
    spec.lcd = "NHD-0440WH-ATFH-JT#";
    spec.two = 1;
    spec.program = a->bytes;
    spec.size = a->pc - 0x8000;
    spec.halt = halt;
    return list1 == halt + 3 ? spec : (spec.size = 0, spec);
}

static void check_cpu_board(void) {
    LcdState lcd;
    CpuState cpu;
    int ok;
    unsigned c;
    int cells = 1, all_x = 1, y, x;
    Asm a;
    BoardSpec spec;
    char jmp[16];

    run_board(0, &HELLO_BOARD, US(10), &lcd, &cpu, &ok);
    for (c = 0; c < 5; c++) {
        cells = cells && cell_is_rom(&lcd, 0, c, (unsigned)"HELLO"[c]);
    }
    expect(ok && memcmp(lcd.ddram[0], "HELLO", 5) == 0 && lcd.ddram[0][5] == 0x20 && cells &&
               lcd.busy[0] == 0,
           "C 4.6 MPS6502 board: HELLO in ddram[0..4], ddram[5] = 20, cells 0-4 = ROM, BF 0");
    printf("     cpu: status %d, last \"%s\"\n", (int)cpu.status, cpu.instruction_text);
    expect(ok && cpu.status == CPU_RUNNING && cpu.has_instruction &&
               strcmp(cpu.instruction_text, "JMP $8029") == 0,
           "C 4.6 MPS6502 board: the CPU is RUNNING, last instruction JMP $8029");

    /* The app's flow: the reset switch opened by hand long after the LCD's
       40 ms power-on; E pulses X all that time (the CPU's address bus is X
       in reset). */
    run_board(0, &HELLO_BOARD, MS(100), &lcd, &cpu, &ok);
    cells = 1;
    for (c = 0; c < 5; c++) {
        cells = cells && cell_is_rom(&lcd, 0, c, (unsigned)"HELLO"[c]);
    }
    expect(ok && memcmp(lcd.ddram[0], "HELLO", 5) == 0 && cells && lcd.busy[0] == 0 &&
               cpu.status == CPU_RUNNING && strcmp(cpu.instruction_text, "JMP $8029") == 0,
           "C 4.6 MPS6502 board, reset held 100 ms: HELLO, BF 0, the CPU RUNNING at JMP $8029");

    run_board(1, &HELLO_BOARD, US(10), &lcd, &cpu, &ok);
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 80; x++) {
            all_x = all_x && lcd.dots[y][x] == LCD_DOT_X;
        }
    }
    expect(ok && all_x && lcd.busy[0] == -1,
           "C 4.6 W65C02S board: the first STA $4000 broke the hold time: every dot X, BF X (LOST)");
    printf("     cpu: status %d, last \"%s\"\n", (int)cpu.status, cpu.instruction_text);
    expect(ok && cpu.status == CPU_LOST,
           "C 4.6 W65C02S board: the CPU is LOST (its BIT $4000 read an X busy flag)");

    /* plan-35-E section 4.3, board 1: the 20x4 on the HELLO board's wiring. */
    memset(&a, 0, sizeof a);
    spec = board_2004(&a);
    snprintf(jmp, sizeof jmp, "JMP $%04X", spec.halt);
    run_board(0, &spec, US(10), &lcd, &cpu, &ok);
    printf("     cpu: status %d, last \"%s\"\n", (int)cpu.status, cpu.instruction_text);
    expect(spec.size > 0 && ok && lcd.rows == 4 && memcmp(lcd.ddram[0], "LINE1", 5) == 0 &&
               memcmp(lcd.ddram[0] + 40, "LINE2", 5) == 0 && memcmp(lcd.ddram[0] + 20, "LINE3", 5) == 0 &&
               memcmp(lcd.ddram[0] + 60, "LINE4", 5) == 0 && cells_show(&lcd, 0, 0, "LINE1") &&
               cells_show(&lcd, 1, 0, "LINE2") && cells_show(&lcd, 2, 0, "LINE3") &&
               cells_show(&lcd, 3, 0, "LINE4") && lcd.busy[0] == 0 && cpu.status == CPU_RUNNING &&
               strcmp(cpu.instruction_text, jmp) == 0,
           "E 4.3 MPS6502 + NHD-0420DZ-FL-YBW board: DDRAM 00/40/14/54 = LINE1-4 on rows 0-3 "
           "(ROM cells), BF 0, the CPU RUNNING at its JMP");

    /* Board 2: the 40x4, E1 at 4000 and E2 at 6000 (A13). */
    memset(&a, 0, sizeof a);
    spec = board_4004(&a);
    snprintf(jmp, sizeof jmp, "JMP $%04X", spec.halt);
    run_board(0, &spec, US(10), &lcd, &cpu, &ok);
    printf("     cpu: status %d, last \"%s\"\n", (int)cpu.status, cpu.instruction_text);
    expect(spec.size > 0 && ok && lcd.controllers == 2 && memcmp(lcd.ddram[0], "TOP ", 4) == 0 &&
               memcmp(lcd.ddram[1], "BOTTOM ", 7) == 0 && cells_show(&lcd, 0, 0, "TOP") &&
               cells_show(&lcd, 0, 3, " ") && row_all(&lcd, 1, 40, LCD_DOT_OFF) &&
               cells_show(&lcd, 2, 0, "BOTTOM") && row_all(&lcd, 3, 40, LCD_DOT_OFF) &&
               lcd.busy[0] == 0 && lcd.busy[1] == 0 && cpu.status == CPU_RUNNING &&
               strcmp(cpu.instruction_text, jmp) == 0,
           "E 4.3 MPS6502 + NHD-0440WH-ATFH-JT# board: TOP through E1 on row 0, BOTTOM through E2 "
           "on row 2, rows 1 and 3 blank, both BF 0, the CPU RUNNING at its JMP");
}

/* ---- main ----------------------------------------------------------------- */

int main(int argc, char **argv) {
    char *error;

    if (argc > 1 && strcmp(argv[1], "--font") == 0) {
        return 0;
    }

    library = fixture_library();
    error = load_file(library, device_file("FX-LCD", "16", "2", "'A00'", "'blue-negative'", DATA,
                                           TIMING, EXEC, BACKLIGHT VCC),
                      "fx-lcd.json");
    if (error == NULL) {
        error = load_file(library, device_file("FX-LCD-NB", "16", "2", "'A00'", "'yellow-green'",
                                               DATA, TIMING, EXEC, VCC),
                          "fx-lcd-nb.json");
    }
    /* plan-35-D fixtures: the NHD-0420DZ 20x4 facts on p16, the C404ALB
       40x4 on p18-dual, a Midas RGB 20x4 on p18-rgb. */
    if (error == NULL) {
        error = load_file(library, large_file("FX-2004", PINOUT, "p16", "FX16", "20", "4",
                                              "'gray'", "'E'", BACKLIGHT VCC),
                          "fx-2004.json");
    }
    if (error == NULL) {
        error = load_file(library, large_file("FX-4004", PINOUT_P18_DUAL, "p18-dual", "FX18", "40",
                                              "4", "'white'", "['E1','E2']", BACKLIGHT VCC),
                          "fx-4004.json");
    }
    if (error == NULL) {
        error = load_file(library, large_file("FX-RGB", PINOUT_P18_RGB, "p18-rgb", "FX18", "20",
                                              "4", "'white'", "'E'", RGB_BACKLIGHT VCC),
                          "fx-rgb.json");
    }
    if (error == NULL) {
        /* A 3.6 V-max probe for case 9: its inputs flag a carried 5 V. */
        error = load_file(library, device_file("FX-LCD-3V", "16", "2", "'A00'", "'yellow-green'",
                                               DATA, TIMING, EXEC, "'vcc':[2.7,3.6],"),
                          "fx-lcd-3v.json");
    }
    if (error == NULL) {
        /* The pull parts name resistor footprints (plan-26-A). */
        char *footprints = read_file(PARTS_DIR "/footprints.json");

        if (footprints == NULL) {
            error = strdup("cannot read footprints.json");
        } else if (load_footprints(library, footprints, "footprints.json", &error) != 0 &&
                   error == NULL) {
            error = strdup("(no message)");
        }
        free(footprints);
    }
    if (error == NULL) {
        char *supplies = read_file(PARTS_DIR "/supply.json");

        error = supplies == NULL ? strdup("cannot read supply.json")
                                 : load_file(library, supplies, "supply.json");
    }
    if (error != NULL) {
        printf("FAIL: fixture: %s\n", error);
        free(error);
        return 1;
    }
    real_library = part_library_create();
    if (real_library == NULL || load_parts_dir(real_library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    check_cpu_board();
    part_library_destroy(real_library);
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("charlcd_test: all passed\n");
    return 0;
}
