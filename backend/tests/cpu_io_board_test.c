/*
 * cpu_io_board_test: split from cpu_test.c by plan-68-H; the shared helpers are
 * support/cpu_test_bench.c.
 */

#include "cpu_test_bench.h"

/* ---- The W65C02S I/O board (plan-14-E section 3.4) --------------------------- */

/*
 * The program, at 8000 (EEPROM offset 0):
 *
 *   8000 A2 FF     RESET: LDX #$FF
 *   8002 9A               TXS
 *   8003 9C 00 02         STZ $0200
 *   8006 A9 FF            LDA #$FF
 *   8008 8D 02 60         STA $6002      ; VIA DDRB
 *   800B A9 55            LDA #$55
 *   800D 8D 00 60         STA $6000      ; VIA ORB
 *   8010 A9 FF            LDA #$FF
 *   8012 8D 00 50         STA $5000      ; PIA DDRA (CRA bit 2 = 0 after reset)
 *   8015 A9 04            LDA #$04
 *   8017 8D 01 50         STA $5001      ; PIA CRA: select ORA
 *   801A A9 AA            LDA #$AA
 *   801C 8D 00 50         STA $5000      ; PIA ORA
 *   801F 8D 01 40         STA $4001      ; ACIA programmed reset
 *   8022 A9 1F            LDA #$1F
 *   8024 8D 03 40         STA $4003      ; 1 stop, 8 bits, baud generator, 19200
 *   8027 A9 0B            LDA #$0B
 *   8029 8D 02 40         STA $4002      ; no parity, RTS low, no TX/RX int, DTR
 *   802C A9 48            LDA #'H'
 *   802E 8D 00 40         STA $4000
 *   8031 20 6C 80         JSR DELAY
 *   8034 A9 49            LDA #'I'
 *   8036 8D 00 40         STA $4000
 *   8039 20 6C 80         JSR DELAY
 *   803C A9 00            LDA #$00
 *   803E 8D 0B 60         STA $600B      ; ACR: T1 one-shot
 *   8041 A9 C0            LDA #$C0
 *   8043 8D 0E 60         STA $600E      ; IER: enable T1
 *   8046 A9 E8            LDA #$E8
 *   8048 8D 04 60         STA $6004
 *   804B A9 03            LDA #$03
 *   804D 8D 05 60         STA $6005      ; T1 = 1000
 *   8050 58               CLI
 *   8051 AD 00 02  WAIT:  LDA $0200
 *   8054 F0 FB            BEQ WAIT
 *   8056 8D 01 02         STA $0201
 *   8059 AD 00 40         LDA $4000      ; the ACIA's RDR: the looped-back byte
 *   805C 8D 02 02         STA $0202
 *   805F 80 FE     DONE:  BRA DONE
 *   8061 48        IRQ:   PHA
 *   8062 AD 04 60         LDA $6004      ; reading T1C-L clears IFR6
 *   8065 A9 01            LDA #$01
 *   8067 8D 00 02         STA $0200
 *   806A 68               PLA
 *   806B 40               RTI
 *   806C A2 80     DELAY: LDX #$80
 *   806E CA        D1:    DEX
 *   806F D0 FD            BNE D1         ; 640 cycles > one 19200-baud character (521 us)
 *   8071 60               RTS
 *
 *   FFFC 00 80 (RESET), FFFE 61 80 (IRQ)
 */
static void board_io(void) {
    static const uint8_t PROGRAM[] = {
        0xA2, 0xFF, 0x9A, 0x9C, 0x00, 0x02, 0xA9, 0xFF, 0x8D, 0x02, 0x60, 0xA9, 0x55, 0x8D,
        0x00, 0x60, 0xA9, 0xFF, 0x8D, 0x00, 0x50, 0xA9, 0x04, 0x8D, 0x01, 0x50, 0xA9, 0xAA,
        0x8D, 0x00, 0x50, 0x8D, 0x01, 0x40, 0xA9, 0x1F, 0x8D, 0x03, 0x40, 0xA9, 0x0B, 0x8D,
        0x02, 0x40, 0xA9, 0x48, 0x8D, 0x00, 0x40, 0x20, 0x6C, 0x80, 0xA9, 0x49, 0x8D, 0x00,
        0x40, 0x20, 0x6C, 0x80, 0xA9, 0x00, 0x8D, 0x0B, 0x60, 0xA9, 0xC0, 0x8D, 0x0E, 0x60,
        0xA9, 0xE8, 0x8D, 0x04, 0x60, 0xA9, 0x03, 0x8D, 0x05, 0x60, 0x58, 0xAD, 0x00, 0x02,
        0xF0, 0xFB, 0x8D, 0x01, 0x02, 0xAD, 0x00, 0x40, 0x8D, 0x02, 0x02, 0x80, 0xFE, 0x48,
        0xAD, 0x04, 0x60, 0xA9, 0x01, 0x8D, 0x00, 0x02, 0x68, 0x40, 0xA2, 0x80, 0xCA, 0xD0,
        0xFD, 0x60
    };
    static uint8_t image[32768];
    int before = failures;
    IoBoard *board;
    CpuState state;
    uint16_t sent[8];
    size_t n;
    uint8_t v;

    memset(image, 0xFF, sizeof image);
    memcpy(image, PROGRAM, sizeof PROGRAM);
    image[0x7FFC] = 0x00;
    image[0x7FFD] = 0x80;
    image[0x7FFE] = 0x61;
    image[0x7FFF] = 0x80;
    expect(PROGRAM[0x61] == 0x48 && PROGRAM[0x6C] == 0xA2, "io_board: IRQ at 8061, DELAY at 806C");
    board = io_board_create(library, image, sizeof image);
    if (board == NULL || !io_board_run(board, NS(4000000))) {
        expect(0, "io_board: the board did not run");
        io_board_destroy(board);
        printf("io_board FAIL\n");
        return;
    }
    io_board_state(board, &state);
    expect(state.status == CPU_RUNNING, "io_board: CPU RUNNING");
    expect(io_board_sram(board, 0x0200, &v) && v == 0x01 && io_board_sram(board, 0x0201, &v) &&
               v == 0x01,
           "io_board: SRAM 0200 = 01, 0201 = 01 (the T1 interrupt ran)");
    expect(io_board_port(board, "VIA", 'B', &v) && v == 0x55, "io_board: VIA PB = 55");
    expect(io_board_port(board, "PIA", 'A', &v) && v == 0xAA, "io_board: PIA PA = AA");
    n = io_board_sent(board, sent, 8);
    expect(n == 2 && sent[0] == 'H' && sent[1] == 'I', "io_board: the ACIA sent HI");
    expect(io_board_sram(board, 0x0202, &v) && v == 'I',
           "io_board: SRAM 0202 = 'I' (the ACIA's RDR, looped back)");
    io_board_destroy(board);
    printf("io_board %s\n", failures == before ? "ok" : "FAIL");
}

/* The W65C816S board (plan-14-I section 3.2): native mode writes to bank 0
   and bank 1 through the bank latch and reads both back.

     8000 18           CLC
     8001 FB           XCE                  ; native mode
     8002 E2 30        SEP #$30             ; 8-bit A, X, Y
     8004 A9 11        LDA #$11
     8006 8F 00 01 00  STA $000100
     800A A9 22        LDA #$22
     800C 8F 00 01 01  STA $010100
     8010 AF 00 01 00  LDA $000100
     8014 8F 00 02 00  STA $000200
     8018 AF 00 01 01  LDA $010100
     801C 8F 01 02 00  STA $000201
     8020 C2 20        REP #$20             ; 16-bit A
     8022 A9 44 33     LDA #$3344
     8025 8F 02 02 00  STA $000202
     8029 80 FE        BRA $8029
*/
static void board_w65c816s(void) {
    static const uint8_t IMAGE[] = {
        0x18, 0xFB, 0xE2, 0x30, 0xA9, 0x11, 0x8F, 0x00, 0x01, 0x00, 0xA9, 0x22, 0x8F, 0x00,
        0x01, 0x01, 0xAF, 0x00, 0x01, 0x00, 0x8F, 0x00, 0x02, 0x00, 0xAF, 0x00, 0x01, 0x01,
        0x8F, 0x01, 0x02, 0x00, 0xC2, 0x20, 0xA9, 0x44, 0x33, 0x8F, 0x02, 0x02, 0x00, 0x80,
        0xFE
    };
    CpuBoard *board = cpu_board_create(library, "W65C816S6TPG-14", IMAGE, sizeof IMAGE, 0x8000);
    CpuState state;
    int failed = failures;
    uint32_t pc;

    expect(IMAGE[0x29] == 0x80 && IMAGE[0x22] == 0xA9, "W65C816S board: BRA at 8029, LDA # at 8022");
    if (board == NULL || !cpu_board_run(board, NS(2000000))) {
        expect(0, "W65C816S board did not run");
        cpu_board_destroy(board);
        printf("board_w65c816s FAIL\n");
        return;
    }
    cpu_board_state(board, &state);
    pc = reg_value(&state, "PC");
    expect(state.status == CPU_RUNNING, "W65C816S board: RUNNING");
    expect(sram_is((const CpuBoard *)board, 0x00100, 0x11) && sram_is(board, 0x08100, 0x22),
           "W65C816S board: SRAM 00100 = 11, 08100 = 22 (bank 1 -> A15)");
    expect(sram_is(board, 0x00200, 0x11) && sram_is(board, 0x00201, 0x22) &&
               sram_is(board, 0x00202, 0x44) && sram_is(board, 0x00203, 0x33),
           "W65C816S board: SRAM 00200..00203 = 11 22 44 33");
    expect(reg_value(&state, "E") == 0 && reg_value(&state, "PBR") == 0 &&
               pc >= 0x8029 && pc <= 0x802B && reg_value(&state, "C") == 0x3344,
           "W65C816S board: E 0, PBR 00, PC in the loop, C 3344");
    if (failures != failed) {
        printf("  E %u PBR %02x PC %04x C %04x\n", (unsigned)reg_value(&state, "E"),
               (unsigned)reg_value(&state, "PBR"), (unsigned)pc, (unsigned)reg_value(&state, "C"));
    }
    cpu_board_destroy(board);
    printf("board_w65c816s %s\n", failures == failed ? "ok" : "FAIL");
}

/* The Z80 board: count to 10 at 8200, CALL a store at 8201, HALT. */
static void board_z80(void) {
    static const uint8_t IMAGE[] = {
        0x31, 0x00, 0x00, 0x3E, 0x00, 0x3C, 0x32, 0x00, 0x82, 0xFE, 0x10, 0x20, 0xF8, 0xCD,
        0x20, 0x00, 0x76, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0x32, 0x01, 0x82, 0xC9
    };
    CpuBoard *board = cpu_board_create(library, "Z84C0010PEG", IMAGE, sizeof IMAGE, -1);
    CpuState state;

    if (board == NULL || !cpu_board_run(board, NS(2000000))) {
        expect(0, "Z84C0010PEG board did not run");
        cpu_board_destroy(board);
        return;
    }
    cpu_board_state(board, &state);
    expect(sram_is(board, 0x0200, 0x10) && sram_is(board, 0x0201, 0x10),
           "Z84C0010PEG board: SRAM 0200 = 10 and 0201 = 10 (the CALLed store)");
    expect(sram_is(board, 0x7FFE, 0x10) && sram_is(board, 0x7FFF, 0x00),
           "Z84C0010PEG board: SRAM 7FFE-7FFF = 10 00 (the return address)");
    expect(state.status == CPU_HALTED && (reg_value(&state, "AF") >> 8) == 0x10 &&
               strcmp(state.instruction_text, "HALT") == 0,
           "Z84C0010PEG board: HALTED, A = 10, last instruction HALT");
    cpu_board_destroy(board);
}

int main(void) {
    size_t i;

    library = part_library_create();
    if (load_parts_dir(library, PARTS_DIR) != 0 || load_fixture() != 0) {
        return 2;
    }
    for (i = 0; i < sizeof CORE_NAMES / sizeof CORE_NAMES[0]; i++) {
    }
    board_z80();
    board_w65c816s();
    board_io();

    part_library_destroy(library);
    if (failures > 0) {
        printf("cpu_test: %d failed\n", failures);
        return 1;
    }
    printf("cpu_test: all passed\n");
    return 0;
}
