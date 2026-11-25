/*
 * An AVR disassembler (plan-62-D), for the processor window's "last
 * instruction". It is ours, not a port: avr8js has none. It decodes in the
 * order of avr_instruction.c's branches, so the two agree on what a word
 * is, and its text assembles back to the same words in avr8js's assembler
 * (avr_periph_test, `dasm: N words round-trip`).
 *
 * Text: upper-case mnemonic; registers R0-R31; constants, I/O and data
 * addresses and absolute targets as 0x..; a relative target as .+N or .-N,
 * in bytes from the next instruction; pointer operands X, X+, -X, Y+q, Z+q.
 * The flag and condition forms (SEC, BREQ, ...) are used for BSET, BCLR,
 * BRBS and BRBC. A word the core runs as a no-op without being NOP, SLEEP,
 * SPM, WDR or BREAK prints as .DW 0x....
 */

#include <stdio.h>

#include "avr.h"

static const char *const SET_FLAG[8] = {"SEC", "SEZ", "SEN", "SEV", "SES", "SEH", "SET", "SEI"};
static const char *const CLEAR_FLAG[8] = {"CLC", "CLZ", "CLN", "CLV", "CLS", "CLH", "CLT", "CLI"};
static const char *const BRANCH_SET[8] = {"BRCS", "BREQ", "BRMI", "BRVS", "BRLT", "BRHS", "BRTS",
                                           "BRIE"};
static const char *const BRANCH_CLEAR[8] = {"BRCC", "BRNE", "BRPL", "BRVC", "BRGE", "BRHC", "BRTC",
                                             "BRID"};

/* "MNEMONIC .+N" or ".-N": N bytes from the next instruction. */
static void relative(char *out, size_t cap, const char *mnemonic, int words) {
    if (words < 0) {
        snprintf(out, cap, "%s .-%d", mnemonic, -words * 2);
    } else {
        snprintf(out, cap, "%s .+%d", mnemonic, words * 2);
    }
}

unsigned avr_dasm(uint16_t opcode, uint16_t next, uint32_t address, char *out, size_t cap) {
    const int d5 = (opcode & 0x1f0) >> 4;
    const int r5 = (opcode & 0xf) | ((opcode & 0x200) >> 5);
    const int d4 = ((opcode & 0xf0) >> 4) + 16;
    const int k8 = (opcode & 0xf) | ((opcode & 0xf00) >> 4);
    const int q6 = (opcode & 7) | ((opcode & 0xc00) >> 7) | ((opcode & 0x2000) >> 8);
    const int bit = opcode & 7;
    const int io5 = (opcode & 0xf8) >> 3;                          /* SBI, CBI, SBIC, SBIS */
    const int io6 = (opcode & 0xf) | ((opcode & 0x600) >> 5);      /* IN, OUT */
    const unsigned long far = ((unsigned long)next | ((unsigned long)(opcode & 1) << 16) |
                               ((unsigned long)(opcode & 0x1f0) << 13)) * 2;   /* JMP, CALL: bytes */
    const int k12 = (opcode & 0x7ff) - ((opcode & 0x800) ? 0x800 : 0);
    const int k7 = ((opcode & 0x1f8) >> 3) - ((opcode & 0x200) ? 0x40 : 0);
    const char *two = NULL;     /* "MNEMONIC" of an Rd, Rr instruction */
    const char *imm = NULL;     /* ... of an Rd, K instruction */
    const char *one = NULL;     /* ... of an Rd instruction */

    (void)address;
    if ((opcode & 0xfc00) == 0x1c00) {
        two = "ADC";
    } else if ((opcode & 0xfc00) == 0xc00) {
        two = "ADD";
    } else if ((opcode & 0xff00) == 0x9600) {
        snprintf(out, cap, "ADIW R%d, 0x%02X", 2 * ((opcode & 0x30) >> 4) + 24,
                 (opcode & 0xf) | ((opcode & 0xc0) >> 2));
        return 1;
    } else if ((opcode & 0xfc00) == 0x2000) {
        two = "AND";
    } else if ((opcode & 0xf000) == 0x7000) {
        imm = "ANDI";
    } else if ((opcode & 0xfe0f) == 0x9405) {
        one = "ASR";
    } else if ((opcode & 0xff8f) == 0x9488) {
        snprintf(out, cap, "%s", CLEAR_FLAG[(opcode & 0x70) >> 4]);
        return 1;
    } else if ((opcode & 0xfe08) == 0xf800) {
        snprintf(out, cap, "BLD R%d, %d", d5, bit);
        return 1;
    } else if ((opcode & 0xfc00) == 0xf400) {
        relative(out, cap, BRANCH_CLEAR[bit], k7);
        return 1;
    } else if ((opcode & 0xfc00) == 0xf000) {
        relative(out, cap, BRANCH_SET[bit], k7);
        return 1;
    } else if ((opcode & 0xff8f) == 0x9408) {
        snprintf(out, cap, "%s", SET_FLAG[(opcode & 0x70) >> 4]);
        return 1;
    } else if ((opcode & 0xfe08) == 0xfa00) {
        snprintf(out, cap, "BST R%d, %d", d5, bit);
        return 1;
    } else if ((opcode & 0xfe0e) == 0x940e) {
        snprintf(out, cap, "CALL 0x%04lX", far);
        return 2;
    } else if ((opcode & 0xff00) == 0x9800) {
        snprintf(out, cap, "CBI 0x%02X, %d", io5, bit);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9400) {
        one = "COM";
    } else if ((opcode & 0xfc00) == 0x1400) {
        two = "CP";
    } else if ((opcode & 0xfc00) == 0x400) {
        two = "CPC";
    } else if ((opcode & 0xf000) == 0x3000) {
        imm = "CPI";
    } else if ((opcode & 0xfc00) == 0x1000) {
        two = "CPSE";
    } else if ((opcode & 0xfe0f) == 0x940a) {
        one = "DEC";
    } else if (opcode == 0x9519) {
        snprintf(out, cap, "EICALL");
        return 1;
    } else if (opcode == 0x9419) {
        snprintf(out, cap, "EIJMP");
        return 1;
    } else if (opcode == 0x95d8) {
        snprintf(out, cap, "ELPM");
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9006) {
        snprintf(out, cap, "ELPM R%d, Z", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9007) {
        snprintf(out, cap, "ELPM R%d, Z+", d5);
        return 1;
    } else if ((opcode & 0xfc00) == 0x2400) {
        two = "EOR";
    } else if ((opcode & 0xff88) == 0x308) {
        snprintf(out, cap, "FMUL R%d, R%d", ((opcode & 0x70) >> 4) + 16, (opcode & 7) + 16);
        return 1;
    } else if ((opcode & 0xff88) == 0x380) {
        snprintf(out, cap, "FMULS R%d, R%d", ((opcode & 0x70) >> 4) + 16, (opcode & 7) + 16);
        return 1;
    } else if ((opcode & 0xff88) == 0x388) {
        snprintf(out, cap, "FMULSU R%d, R%d", ((opcode & 0x70) >> 4) + 16, (opcode & 7) + 16);
        return 1;
    } else if (opcode == 0x9509) {
        snprintf(out, cap, "ICALL");
        return 1;
    } else if (opcode == 0x9409) {
        snprintf(out, cap, "IJMP");
        return 1;
    } else if ((opcode & 0xf800) == 0xb000) {
        snprintf(out, cap, "IN R%d, 0x%02X", d5, io6);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9403) {
        one = "INC";
    } else if ((opcode & 0xfe0e) == 0x940c) {
        snprintf(out, cap, "JMP 0x%04lX", far);
        return 2;
    } else if ((opcode & 0xfe0f) == 0x9206) {
        snprintf(out, cap, "LAC Z, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9205) {
        snprintf(out, cap, "LAS Z, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9207) {
        snprintf(out, cap, "LAT Z, R%d", d5);
        return 1;
    } else if ((opcode & 0xf000) == 0xe000) {
        imm = "LDI";
    } else if ((opcode & 0xfe0f) == 0x9000) {
        snprintf(out, cap, "LDS R%d, 0x%04X", d5, next);
        return 2;
    } else if ((opcode & 0xfe0f) == 0x900c) {
        snprintf(out, cap, "LD R%d, X", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x900d) {
        snprintf(out, cap, "LD R%d, X+", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x900e) {
        snprintf(out, cap, "LD R%d, -X", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x8008) {
        snprintf(out, cap, "LD R%d, Y", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9009) {
        snprintf(out, cap, "LD R%d, Y+", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x900a) {
        snprintf(out, cap, "LD R%d, -Y", d5);
        return 1;
    } else if ((opcode & 0xd208) == 0x8008 && q6) {
        snprintf(out, cap, "LDD R%d, Y+%d", d5, q6);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x8000) {
        snprintf(out, cap, "LD R%d, Z", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9001) {
        snprintf(out, cap, "LD R%d, Z+", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9002) {
        snprintf(out, cap, "LD R%d, -Z", d5);
        return 1;
    } else if ((opcode & 0xd208) == 0x8000 && q6) {
        snprintf(out, cap, "LDD R%d, Z+%d", d5, q6);
        return 1;
    } else if (opcode == 0x95c8) {
        snprintf(out, cap, "LPM");
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9004) {
        snprintf(out, cap, "LPM R%d, Z", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9005) {
        snprintf(out, cap, "LPM R%d, Z+", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9406) {
        one = "LSR";
    } else if ((opcode & 0xfc00) == 0x2c00) {
        two = "MOV";
    } else if ((opcode & 0xff00) == 0x100) {
        snprintf(out, cap, "MOVW R%d, R%d", 2 * ((opcode & 0xf0) >> 4), 2 * (opcode & 0xf));
        return 1;
    } else if ((opcode & 0xfc00) == 0x9c00) {
        two = "MUL";
    } else if ((opcode & 0xff00) == 0x200) {
        snprintf(out, cap, "MULS R%d, R%d", d4, (opcode & 0xf) + 16);
        return 1;
    } else if ((opcode & 0xff88) == 0x300) {
        snprintf(out, cap, "MULSU R%d, R%d", ((opcode & 0x70) >> 4) + 16, (opcode & 7) + 16);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9401) {
        one = "NEG";
    } else if (opcode == 0) {
        snprintf(out, cap, "NOP");
        return 1;
    } else if ((opcode & 0xfc00) == 0x2800) {
        two = "OR";
    } else if ((opcode & 0xf000) == 0x6000) {
        imm = "ORI";
    } else if ((opcode & 0xf800) == 0xb800) {
        snprintf(out, cap, "OUT 0x%02X, R%d", io6, d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x900f) {
        one = "POP";
    } else if ((opcode & 0xfe0f) == 0x920f) {
        one = "PUSH";
    } else if ((opcode & 0xf000) == 0xd000) {
        relative(out, cap, "RCALL", k12);
        return 1;
    } else if (opcode == 0x9508) {
        snprintf(out, cap, "RET");
        return 1;
    } else if (opcode == 0x9518) {
        snprintf(out, cap, "RETI");
        return 1;
    } else if ((opcode & 0xf000) == 0xc000) {
        relative(out, cap, "RJMP", k12);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9407) {
        one = "ROR";
    } else if ((opcode & 0xfc00) == 0x800) {
        two = "SBC";
    } else if ((opcode & 0xf000) == 0x4000) {
        imm = "SBCI";
    } else if ((opcode & 0xff00) == 0x9a00) {
        snprintf(out, cap, "SBI 0x%02X, %d", io5, bit);
        return 1;
    } else if ((opcode & 0xff00) == 0x9900) {
        snprintf(out, cap, "SBIC 0x%02X, %d", io5, bit);
        return 1;
    } else if ((opcode & 0xff00) == 0x9b00) {
        snprintf(out, cap, "SBIS 0x%02X, %d", io5, bit);
        return 1;
    } else if ((opcode & 0xff00) == 0x9700) {
        snprintf(out, cap, "SBIW R%d, 0x%02X", 2 * ((opcode & 0x30) >> 4) + 24,
                 (opcode & 0xf) | ((opcode & 0xc0) >> 2));
        return 1;
    } else if ((opcode & 0xfe08) == 0xfc00) {
        snprintf(out, cap, "SBRC R%d, %d", d5, bit);
        return 1;
    } else if ((opcode & 0xfe08) == 0xfe00) {
        snprintf(out, cap, "SBRS R%d, %d", d5, bit);
        return 1;
    } else if (opcode == 0x9588) {
        snprintf(out, cap, "SLEEP");
        return 1;
    } else if (opcode == 0x95e8) {
        snprintf(out, cap, "SPM");
        return 1;
    } else if (opcode == 0x95f8) {
        snprintf(out, cap, "SPM Z+");
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9200) {
        snprintf(out, cap, "STS 0x%04X, R%d", next, d5);
        return 2;
    } else if ((opcode & 0xfe0f) == 0x920c) {
        snprintf(out, cap, "ST X, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x920d) {
        snprintf(out, cap, "ST X+, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x920e) {
        snprintf(out, cap, "ST -X, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x8208) {
        snprintf(out, cap, "ST Y, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9209) {
        snprintf(out, cap, "ST Y+, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x920a) {
        snprintf(out, cap, "ST -Y, R%d", d5);
        return 1;
    } else if ((opcode & 0xd208) == 0x8208 && q6) {
        snprintf(out, cap, "STD Y+%d, R%d", q6, d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x8200) {
        snprintf(out, cap, "ST Z, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9201) {
        snprintf(out, cap, "ST Z+, R%d", d5);
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9202) {
        snprintf(out, cap, "ST -Z, R%d", d5);
        return 1;
    } else if ((opcode & 0xd208) == 0x8200 && q6) {
        snprintf(out, cap, "STD Z+%d, R%d", q6, d5);
        return 1;
    } else if ((opcode & 0xfc00) == 0x1800) {
        two = "SUB";
    } else if ((opcode & 0xf000) == 0x5000) {
        imm = "SUBI";
    } else if ((opcode & 0xfe0f) == 0x9402) {
        one = "SWAP";
    } else if (opcode == 0x95a8) {
        snprintf(out, cap, "WDR");
        return 1;
    } else if ((opcode & 0xfe0f) == 0x9204) {
        snprintf(out, cap, "XCH Z, R%d", d5);
        return 1;
    } else if (opcode == 0x9598) {
        /* The core has no branch for BREAK (avr8js runs it as a no-op); it
           is a real instruction all the same. */
        snprintf(out, cap, "BREAK");
        return 1;
    }

    if (two != NULL) {
        snprintf(out, cap, "%s R%d, R%d", two, d5, r5);
    } else if (imm != NULL) {
        snprintf(out, cap, "%s R%d, 0x%02X", imm, d4, k8);
    } else if (one != NULL) {
        snprintf(out, cap, "%s R%d", one, d5);
    } else {
        snprintf(out, cap, ".DW 0x%04X", opcode);
    }
    return 1;
}
