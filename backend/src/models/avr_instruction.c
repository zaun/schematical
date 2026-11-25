/*
 * AVR instruction execution. A C port of avr8js 0.21.1's
 * src/cpu/instruction.ts (Copyright (c) Uri Shaked and contributors, MIT;
 * see LICENSE-avr8js). plan-62-A.
 *
 * The decode branches are in avr8js's order (the first match wins there
 * too), numbered 1..AVR_BRANCHES for the coverage counter. Instruction
 * timing is the ATmega328P's, as in avr8js.
 *
 * Seven behaviors differ from avr8js, each marked "avr8js:" below, because
 * avr8js disagrees with Microchip's AVR Instruction Set Manual there. With
 * cpu->exact set the code does what avr8js does instead, which is what the
 * oracle tests run.
 */

#include "avr.h"

#define SREG(cpu) ((cpu)->data[AVR_SREG])
#define BRANCH(cpu, id) do { if ((cpu)->coverage != NULL) { (cpu)->coverage[id]++; } } while (0)

/* A raw read of data memory, as avr8js's cpu.data[i]: no hook; past the end
   (or before the start) it is `undefined` there, which every use turns into 0. */
static uint8_t raw_get(const AvrCpu *cpu, int32_t index) {
    return index >= 0 && (uint32_t)index < cpu->data_size ? cpu->data[index] : 0;
}

/* A raw store, as cpu.data[i] = v: dropped outside the array. */
static void raw_set(AvrCpu *cpu, int32_t index, uint8_t value) {
    if (index >= 0 && (uint32_t)index < cpu->data_size) {
        cpu->data[index] = value;
    }
}

static uint8_t prog_byte(const AvrCpu *cpu, uint32_t index) {
    return index < cpu->flash_bytes ? cpu->flash[index] : 0;
}

static int is_two_word_instruction(uint16_t opcode) {
    return (opcode & 0xfe0f) == 0x9000 ||   /* LDS */
           (opcode & 0xfe0f) == 0x9200 ||   /* STS */
           (opcode & 0xfe0e) == 0x940e ||   /* CALL */
           (opcode & 0xfe0e) == 0x940c;     /* JMP */
}

/* S = N xor V, from the flags already in sreg. */
static int sign_flag(int sreg) {
    return (((sreg >> 2) & 1) ^ ((sreg >> 3) & 1)) ? 0x10 : 0;
}

/*
 * The half-carry flag H from a half-carry expression.
 *
 * avr8js: takes bit 0 of the expression (`1 & (...)`). The instruction set
 * manual defines H as the carry out of bit 3 (H = Rd3·Rr3 + Rr3·/R3 +
 * /R3·Rd3, and the like), which is bit 3 of the same expression: ADD
 * 0x08 + 0x08 sets H on a real chip, and avr8js leaves it clear. We follow
 * the manual. `exact` keeps avr8js's bit 0 for the oracle tests.
 */
static int half_carry(const AvrCpu *cpu, int expression) {
    return (expression & (cpu->exact ? 1 : 8)) ? 0x20 : 0;
}

/* Pushes a return address (2 bytes, or 3 with a 22-bit pc) and moves SP. */
static void push_return(AvrCpu *cpu, int64_t ret) {
    int32_t sp = avr_get16(cpu, AVR_SP);

    raw_set(cpu, sp, (uint8_t)ret);
    raw_set(cpu, sp - 1, (uint8_t)(ret >> 8));
    if (cpu->pc22) {
        raw_set(cpu, sp - 2, (uint8_t)(ret >> 16));
    }
    avr_set16(cpu, AVR_SP, (uint32_t)(sp - (cpu->pc22 ? 3 : 2)));
}

/* Skips the next instruction (CPSE, SBRC, SBRS, SBIC, SBIS). */
static void skip_next(AvrCpu *cpu, int64_t *pc) {
    int skip = is_two_word_instruction(avr_flash_word(cpu, *pc + 1)) ? 2 : 1;

    *pc += skip;
    cpu->cycles += (uint64_t)skip;
}

void avr_instruction(AvrCpu *cpu) {
    /* avr8js's pc is a JavaScript number: an instruction may leave it at -1
       (a jump to 0) before the final + 1. */
    int64_t pc = cpu->pc;
    const uint16_t opcode = avr_flash_word(cpu, pc);
    uint8_t *data = cpu->data;
    const int d5 = (opcode & 0x1f0) >> 4;                     /* Rd, 0-31 */
    const int r5 = (opcode & 0xf) | ((opcode & 0x200) >> 5);  /* Rr, 0-31 */
    const int d4 = ((opcode & 0xf0) >> 4) + 16;               /* Rd, 16-31 */
    const int k8 = (opcode & 0xf) | ((opcode & 0xf00) >> 4);  /* K, 8 bits */
    const int q6 = (opcode & 7) | ((opcode & 0xc00) >> 7) | ((opcode & 0x2000) >> 8);

    if ((opcode & 0xfc00) == 0x1c00) {
        /* ADC, 0001 11rd dddd rrrr */
        const int d = data[d5];
        const int r = data[r5];
        const int sum = d + r + (SREG(cpu) & 1);
        const int R = sum & 255;
        int sreg = SREG(cpu) & 0xc0;

        BRANCH(cpu, 1);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= ((R ^ r) & (d ^ R) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= (sum & 256) ? 1 : 0;
        sreg |= half_carry(cpu, (d & r) | (r & ~R) | (~R & d));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfc00) == 0xc00) {
        /* ADD, 0000 11rd dddd rrrr */
        const int d = data[d5];
        const int r = data[r5];
        const int R = (d + r) & 255;
        int sreg = SREG(cpu) & 0xc0;

        BRANCH(cpu, 2);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= ((R ^ r) & (R ^ d) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= ((d + r) & 256) ? 1 : 0;
        sreg |= half_carry(cpu, (d & r) | (r & ~R) | (~R & d));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xff00) == 0x9600) {
        /* ADIW, 1001 0110 KKdd KKKK */
        const uint32_t addr = 2 * ((opcode & 0x30) >> 4) + 24;
        const int value = avr_get16(cpu, addr);
        const int R = (value + ((opcode & 0xf) | ((opcode & 0xc0) >> 2))) & 0xffff;
        int sreg = SREG(cpu) & 0xe0;

        BRANCH(cpu, 3);
        avr_set16(cpu, addr, (uint32_t)R);
        sreg |= R ? 0 : 2;
        sreg |= (0x8000 & R) ? 4 : 0;
        sreg |= (~value & R & 0x8000) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= (~R & value & 0x8000) ? 1 : 0;
        SREG(cpu) = (uint8_t)sreg;
        cpu->cycles++;
    } else if ((opcode & 0xfc00) == 0x2000) {
        /* AND, 0010 00rd dddd rrrr */
        const int R = data[d5] & data[r5];
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 4);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xf000) == 0x7000) {
        /* ANDI, 0111 KKKK dddd KKKK */
        const int R = data[d4] & k8;
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 5);
        data[d4] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfe0f) == 0x9405) {
        /* ASR, 1001 010d dddd 0101 */
        const int value = data[d5];
        const int R = (value >> 1) | (128 & value);
        int sreg = SREG(cpu) & 0xe0;

        BRANCH(cpu, 6);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= value & 1;
        sreg |= (((sreg >> 2) & 1) ^ (sreg & 1)) ? 8 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xff8f) == 0x9488) {
        /* BCLR, 1001 0100 1sss 1000 */
        BRANCH(cpu, 7);
        SREG(cpu) &= (uint8_t)~(1 << ((opcode & 0x70) >> 4));
    } else if ((opcode & 0xfe08) == 0xf800) {
        /* BLD, 1111 100d dddd 0bbb */
        const int b = opcode & 7;

        BRANCH(cpu, 8);
        data[d5] = (uint8_t)((~(1 << b) & data[d5]) | (((SREG(cpu) >> 6) & 1) << b));
    } else if ((opcode & 0xfc00) == 0xf400) {
        /* BRBC, 1111 01kk kkkk ksss */
        BRANCH(cpu, 9);
        if (!(SREG(cpu) & (1 << (opcode & 7)))) {
            pc = pc + (((opcode & 0x1f8) >> 3) - ((opcode & 0x200) ? 0x40 : 0));
            cpu->cycles++;
        }
    } else if ((opcode & 0xfc00) == 0xf000) {
        /* BRBS, 1111 00kk kkkk ksss */
        BRANCH(cpu, 10);
        if (SREG(cpu) & (1 << (opcode & 7))) {
            pc = pc + (((opcode & 0x1f8) >> 3) - ((opcode & 0x200) ? 0x40 : 0));
            cpu->cycles++;
        }
    } else if ((opcode & 0xff8f) == 0x9408) {
        /* BSET, 1001 0100 0sss 1000 */
        BRANCH(cpu, 11);
        SREG(cpu) |= (uint8_t)(1 << ((opcode & 0x70) >> 4));
    } else if ((opcode & 0xfe08) == 0xfa00) {
        /* BST, 1111 101d dddd 0bbb */
        const int d = data[d5];
        const int b = opcode & 7;

        BRANCH(cpu, 12);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xbf) | (((d >> b) & 1) ? 0x40 : 0));
    } else if ((opcode & 0xfe0e) == 0x940e) {
        /* CALL, 1001 010k kkkk 111k kkkk kkkk kkkk kkkk */
        const int64_t k = avr_flash_word(cpu, pc + 1) | ((opcode & 1) << 16) |
                          ((opcode & 0x1f0) << 13);

        BRANCH(cpu, 13);
        push_return(cpu, pc + 2);
        pc = k - 1;
        cpu->cycles += cpu->pc22 ? 4 : 3;
    } else if ((opcode & 0xff00) == 0x9800) {
        /* CBI, 1001 1000 AAAA Abbb */
        const int A = opcode & 0xf8;
        const int b = opcode & 7;
        const int R = avr_read_data(cpu, (A >> 3) + 32);
        const int mask = 1 << b;

        BRANCH(cpu, 14);
        avr_write_data(cpu, (A >> 3) + 32, (uint8_t)(R & ~mask), (uint8_t)mask);
        /* avr8js: CBI takes 1 cycle (it has no cycles++ here, unlike its
           SBI). The instruction set manual gives CBI 2 cycles on these
           devices, the same as SBI. We follow the manual. `exact` keeps
           avr8js's 1 for the oracle tests. */
        if (!cpu->exact) {
            cpu->cycles++;
        }
    } else if ((opcode & 0xfe0f) == 0x9400) {
        /* COM, 1001 010d dddd 0000 */
        const int R = 255 - data[d5];
        int sreg = (SREG(cpu) & 0xe1) | 1;

        BRANCH(cpu, 15);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfc00) == 0x1400) {
        /* CP, 0001 01rd dddd rrrr */
        const int val1 = data[d5];
        const int val2 = data[r5];
        const int R = val1 - val2;
        int sreg = SREG(cpu) & 0xc0;

        BRANCH(cpu, 16);
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= ((val1 ^ val2) & (val1 ^ R) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= val2 > val1 ? 1 : 0;
        sreg |= half_carry(cpu, (~val1 & val2) | (val2 & R) | (R & ~val1));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfc00) == 0x400) {
        /* CPC, 0000 01rd dddd rrrr */
        const int arg1 = data[d5];
        const int arg2 = data[r5];
        int sreg = SREG(cpu);
        const int r = arg1 - arg2 - (sreg & 1);
        /* avr8js: tests `!r` on the difference before it is cut to 8 bits,
           so 0 - 255 - 1 (= -256, the byte 0x00) clears Z. The manual keeps
           the previous Z whenever the 8-bit result is zero. We follow the
           manual. `exact` keeps avr8js's test for the oracle tests. */
        const int zero = cpu->exact ? !r : (r & 0xff) == 0;

        BRANCH(cpu, 17);
        sreg = (sreg & 0xc0) | ((zero && ((sreg >> 1) & 1)) ? 2 : 0) |
               (arg2 + (sreg & 1) > arg1 ? 1 : 0);
        sreg |= (128 & r) ? 4 : 0;
        sreg |= ((arg1 ^ arg2) & (arg1 ^ r) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= half_carry(cpu, (~arg1 & arg2) | (arg2 & r) | (r & ~arg1));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xf000) == 0x3000) {
        /* CPI, 0011 KKKK dddd KKKK */
        const int arg1 = data[d4];
        const int arg2 = k8;
        const int r = arg1 - arg2;
        int sreg = SREG(cpu) & 0xc0;

        BRANCH(cpu, 18);
        sreg |= r ? 0 : 2;
        sreg |= (128 & r) ? 4 : 0;
        sreg |= ((arg1 ^ arg2) & (arg1 ^ r) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= arg2 > arg1 ? 1 : 0;
        sreg |= half_carry(cpu, (~arg1 & arg2) | (arg2 & r) | (r & ~arg1));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfc00) == 0x1000) {
        /* CPSE, 0001 00rd dddd rrrr */
        BRANCH(cpu, 19);
        if (data[d5] == data[r5]) {
            skip_next(cpu, &pc);
        }
    } else if ((opcode & 0xfe0f) == 0x940a) {
        /* DEC, 1001 010d dddd 1010 */
        const int value = data[d5];
        const int R = value - 1;
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 20);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= 128 == value ? 8 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if (opcode == 0x9519) {
        /* EICALL, 1001 0101 0001 1001 */
        const int64_t ret = pc + 1;
        const int32_t sp = avr_get16(cpu, AVR_SP);
        const int eind = data[0x5c];

        BRANCH(cpu, 21);
        raw_set(cpu, sp, (uint8_t)ret);
        raw_set(cpu, sp - 1, (uint8_t)(ret >> 8));
        raw_set(cpu, sp - 2, (uint8_t)(ret >> 16));
        avr_set16(cpu, AVR_SP, (uint32_t)(sp - 3));
        pc = (int64_t)((eind << 16) | avr_get16(cpu, 30)) - 1;
        cpu->cycles += 3;
    } else if (opcode == 0x9419) {
        /* EIJMP, 1001 0100 0001 1001 */
        const int eind = data[0x5c];

        BRANCH(cpu, 22);
        pc = (int64_t)((eind << 16) | avr_get16(cpu, 30)) - 1;
        cpu->cycles++;
    } else if (opcode == 0x95d8) {
        /* ELPM, 1001 0101 1101 1000 */
        const uint32_t rampz = data[0x5b];

        BRANCH(cpu, 23);
        data[0] = prog_byte(cpu, (rampz << 16) | avr_get16(cpu, 30));
        cpu->cycles += 2;
    } else if ((opcode & 0xfe0f) == 0x9006) {
        /* ELPM(REG), 1001 000d dddd 0110 */
        const uint32_t rampz = data[0x5b];

        BRANCH(cpu, 24);
        data[d5] = prog_byte(cpu, (rampz << 16) | avr_get16(cpu, 30));
        cpu->cycles += 2;
    } else if ((opcode & 0xfe0f) == 0x9007) {
        /* ELPM(INC), 1001 000d dddd 0111 */
        const uint32_t rampz = data[0x5b];
        const uint32_t i = avr_get16(cpu, 30);

        BRANCH(cpu, 25);
        data[d5] = prog_byte(cpu, (rampz << 16) | i);
        avr_set16(cpu, 30, i + 1);
        if (i == 0xffff) {
            /* avr8js: % (progBytes.length >> 16), a division by zero (NaN,
               stored as 0) on a flash of 64 KB or less. */
            const uint32_t pages = cpu->flash_bytes >> 16;

            data[0x5b] = pages != 0 ? (uint8_t)((rampz + 1) % pages) : 0;
        }
        cpu->cycles += 2;
    } else if ((opcode & 0xfc00) == 0x2400) {
        /* EOR, 0010 01rd dddd rrrr */
        const int R = data[d5] ^ data[r5];
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 26);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xff88) == 0x308) {
        /* FMUL, 0000 0011 0ddd 1rrr */
        const int v1 = data[((opcode & 0x70) >> 4) + 16];
        const int v2 = data[(opcode & 7) + 16];
        const int R = (v1 * v2) << 1;

        BRANCH(cpu, 27);
        avr_set16(cpu, 0, (uint32_t)R);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xfc) | ((0xffff & R) ? 0 : 2) |
                              (((v1 * v2) & 0x8000) ? 1 : 0));
        cpu->cycles++;
    } else if ((opcode & 0xff88) == 0x380) {
        /* FMULS, 0000 0011 1ddd 0rrr */
        const int v1 = (int8_t)data[((opcode & 0x70) >> 4) + 16];
        const int v2 = (int8_t)data[(opcode & 7) + 16];
        const int R = (v1 * v2) * 2;

        BRANCH(cpu, 28);
        avr_set16(cpu, 0, (uint32_t)R);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xfc) | ((0xffff & R) ? 0 : 2) |
                              (((v1 * v2) & 0x8000) ? 1 : 0));
        cpu->cycles++;
    } else if ((opcode & 0xff88) == 0x388) {
        /* FMULSU, 0000 0011 1ddd 1rrr */
        const int v1 = (int8_t)data[((opcode & 0x70) >> 4) + 16];
        const int v2 = data[(opcode & 7) + 16];
        const int R = (v1 * v2) * 2;
        /* avr8js: sets Z when the product is NOT zero (`0xffff & R ? 2 : 0`,
           the reverse of its FMUL and FMULS). The manual sets Z when the
           16-bit result is zero. We follow the manual. `exact` keeps
           avr8js's reversed flag for the oracle tests. */
        const int zero = cpu->exact ? (0xffff & R) != 0 : (0xffff & R) == 0;

        BRANCH(cpu, 29);
        avr_set16(cpu, 0, (uint32_t)R);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xfc) | (zero ? 2 : 0) |
                              (((v1 * v2) & 0x8000) ? 1 : 0));
        cpu->cycles++;
    } else if (opcode == 0x9509) {
        /* ICALL, 1001 0101 0000 1001 */
        BRANCH(cpu, 30);
        push_return(cpu, pc + 1);
        pc = (int64_t)avr_get16(cpu, 30) - 1;
        cpu->cycles += cpu->pc22 ? 3 : 2;
    } else if (opcode == 0x9409) {
        /* IJMP, 1001 0100 0000 1001 */
        BRANCH(cpu, 31);
        pc = (int64_t)avr_get16(cpu, 30) - 1;
        cpu->cycles++;
    } else if ((opcode & 0xf800) == 0xb000) {
        /* IN, 1011 0AAd dddd AAAA */
        BRANCH(cpu, 32);
        data[d5] = avr_read_data(cpu, ((opcode & 0xf) | ((opcode & 0x600) >> 5)) + 32);
    } else if ((opcode & 0xfe0f) == 0x9403) {
        /* INC, 1001 010d dddd 0011 */
        const int d = data[d5];
        const int r = (d + 1) & 255;
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 33);
        data[d5] = (uint8_t)r;
        sreg |= r ? 0 : 2;
        sreg |= (128 & r) ? 4 : 0;
        sreg |= 127 == d ? 8 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfe0e) == 0x940c) {
        /* JMP, 1001 010k kkkk 110k kkkk kkkk kkkk kkkk */
        BRANCH(cpu, 34);
        pc = (int64_t)(avr_flash_word(cpu, pc + 1) | ((opcode & 1) << 16) |
                       ((opcode & 0x1f0) << 13)) - 1;
        cpu->cycles += 2;
    } else if ((opcode & 0xfe0f) == 0x9206) {
        /* LAC, 1001 001r rrrr 0110 */
        const int clear = data[d5];
        const int value = avr_read_data(cpu, avr_get16(cpu, 30));

        BRANCH(cpu, 35);
        avr_write_data(cpu, avr_get16(cpu, 30), (uint8_t)(value & (255 - clear)), 0xff);
        data[d5] = (uint8_t)value;
    } else if ((opcode & 0xfe0f) == 0x9205) {
        /* LAS, 1001 001r rrrr 0101 */
        const int set = data[d5];
        const int value = avr_read_data(cpu, avr_get16(cpu, 30));

        BRANCH(cpu, 36);
        avr_write_data(cpu, avr_get16(cpu, 30), (uint8_t)(value | set), 0xff);
        data[d5] = (uint8_t)value;
    } else if ((opcode & 0xfe0f) == 0x9207) {
        /* LAT, 1001 001r rrrr 0111 */
        const int r = data[d5];
        const int R = avr_read_data(cpu, avr_get16(cpu, 30));

        BRANCH(cpu, 37);
        avr_write_data(cpu, avr_get16(cpu, 30), (uint8_t)(r ^ R), 0xff);
        data[d5] = (uint8_t)R;
    } else if ((opcode & 0xf000) == 0xe000) {
        /* LDI, 1110 KKKK dddd KKKK */
        BRANCH(cpu, 38);
        data[d4] = (uint8_t)k8;
    } else if ((opcode & 0xfe0f) == 0x9000) {
        /* LDS, 1001 000d dddd 0000 kkkk kkkk kkkk kkkk */
        BRANCH(cpu, 39);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, avr_flash_word(cpu, pc + 1));
        pc++;
    } else if ((opcode & 0xfe0f) == 0x900c) {
        /* LDX, 1001 000d dddd 1100 */
        BRANCH(cpu, 40);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, avr_get16(cpu, 26));
    } else if ((opcode & 0xfe0f) == 0x900d) {
        /* LDX(INC), 1001 000d dddd 1101 */
        const int32_t x = avr_get16(cpu, 26);

        BRANCH(cpu, 41);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, x);
        avr_set16(cpu, 26, (uint32_t)(x + 1));
    } else if ((opcode & 0xfe0f) == 0x900e) {
        /* LDX(DEC), 1001 000d dddd 1110 */
        const int32_t x = (int32_t)avr_get16(cpu, 26) - 1;

        BRANCH(cpu, 42);
        avr_set16(cpu, 26, (uint32_t)x);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, x);
    } else if ((opcode & 0xfe0f) == 0x8008) {
        /* LDY, 1000 000d dddd 1000 */
        BRANCH(cpu, 43);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, avr_get16(cpu, 28));
    } else if ((opcode & 0xfe0f) == 0x9009) {
        /* LDY(INC), 1001 000d dddd 1001 */
        const int32_t y = avr_get16(cpu, 28);

        BRANCH(cpu, 44);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, y);
        avr_set16(cpu, 28, (uint32_t)(y + 1));
    } else if ((opcode & 0xfe0f) == 0x900a) {
        /* LDY(DEC), 1001 000d dddd 1010 */
        const int32_t y = (int32_t)avr_get16(cpu, 28) - 1;

        BRANCH(cpu, 45);
        avr_set16(cpu, 28, (uint32_t)y);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, y);
    } else if ((opcode & 0xd208) == 0x8008 && q6) {
        /* LDDY, 10q0 qq0d dddd 1qqq */
        BRANCH(cpu, 46);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, (int32_t)avr_get16(cpu, 28) + q6);
    } else if ((opcode & 0xfe0f) == 0x8000) {
        /* LDZ, 1000 000d dddd 0000 */
        BRANCH(cpu, 47);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, avr_get16(cpu, 30));
    } else if ((opcode & 0xfe0f) == 0x9001) {
        /* LDZ(INC), 1001 000d dddd 0001 */
        const int32_t z = avr_get16(cpu, 30);

        BRANCH(cpu, 48);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, z);
        avr_set16(cpu, 30, (uint32_t)(z + 1));
    } else if ((opcode & 0xfe0f) == 0x9002) {
        /* LDZ(DEC), 1001 000d dddd 0010 */
        const int32_t z = (int32_t)avr_get16(cpu, 30) - 1;

        BRANCH(cpu, 49);
        avr_set16(cpu, 30, (uint32_t)z);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, z);
    } else if ((opcode & 0xd208) == 0x8000 && q6) {
        /* LDDZ, 10q0 qq0d dddd 0qqq */
        BRANCH(cpu, 50);
        cpu->cycles++;
        data[d5] = avr_read_data(cpu, (int32_t)avr_get16(cpu, 30) + q6);
    } else if (opcode == 0x95c8) {
        /* LPM, 1001 0101 1100 1000 */
        BRANCH(cpu, 51);
        data[0] = prog_byte(cpu, avr_get16(cpu, 30));
        cpu->cycles += 2;
    } else if ((opcode & 0xfe0f) == 0x9004) {
        /* LPM(REG), 1001 000d dddd 0100 */
        BRANCH(cpu, 52);
        data[d5] = prog_byte(cpu, avr_get16(cpu, 30));
        cpu->cycles += 2;
    } else if ((opcode & 0xfe0f) == 0x9005) {
        /* LPM(INC), 1001 000d dddd 0101 */
        const uint32_t i = avr_get16(cpu, 30);

        BRANCH(cpu, 53);
        data[d5] = prog_byte(cpu, i);
        avr_set16(cpu, 30, i + 1);
        cpu->cycles += 2;
    } else if ((opcode & 0xfe0f) == 0x9406) {
        /* LSR, 1001 010d dddd 0110 */
        const int value = data[d5];
        const int R = value >> 1;
        int sreg = SREG(cpu) & 0xe0;

        BRANCH(cpu, 54);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= value & 1;
        sreg |= (((sreg >> 2) & 1) ^ (sreg & 1)) ? 8 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfc00) == 0x2c00) {
        /* MOV, 0010 11rd dddd rrrr */
        BRANCH(cpu, 55);
        data[d5] = data[r5];
    } else if ((opcode & 0xff00) == 0x100) {
        /* MOVW, 0000 0001 dddd rrrr */
        const int r2 = 2 * (opcode & 0xf);
        const int d2 = 2 * ((opcode & 0xf0) >> 4);

        BRANCH(cpu, 56);
        data[d2] = data[r2];
        data[d2 + 1] = data[r2 + 1];
    } else if ((opcode & 0xfc00) == 0x9c00) {
        /* MUL, 1001 11rd dddd rrrr */
        const int R = data[d5] * data[r5];

        BRANCH(cpu, 57);
        avr_set16(cpu, 0, (uint32_t)R);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xfc) | ((0xffff & R) ? 0 : 2) |
                              ((0x8000 & R) ? 1 : 0));
        cpu->cycles++;
    } else if ((opcode & 0xff00) == 0x200) {
        /* MULS, 0000 0010 dddd rrrr */
        const int R = (int8_t)data[d4] * (int8_t)data[(opcode & 0xf) + 16];

        BRANCH(cpu, 58);
        avr_set16(cpu, 0, (uint32_t)R);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xfc) | ((0xffff & R) ? 0 : 2) |
                              ((0x8000 & R) ? 1 : 0));
        cpu->cycles++;
    } else if ((opcode & 0xff88) == 0x300) {
        /* MULSU, 0000 0011 0ddd 0rrr */
        const int R = (int8_t)data[((opcode & 0x70) >> 4) + 16] * data[(opcode & 7) + 16];

        BRANCH(cpu, 59);
        avr_set16(cpu, 0, (uint32_t)R);
        SREG(cpu) = (uint8_t)((SREG(cpu) & 0xfc) | ((0xffff & R) ? 0 : 2) |
                              ((0x8000 & R) ? 1 : 0));
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x9401) {
        /* NEG, 1001 010d dddd 0001 */
        const int value = data[d5];
        const int R = 0 - value;
        int sreg = SREG(cpu) & 0xc0;

        BRANCH(cpu, 60);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        /* avr8js: tests `128 === R` with R = 0 - value, which is -128 for
           0x80 and never 128, so V is never set. The manual sets V when the
           result is 0x80. We follow the manual. `exact` keeps avr8js's
           never-set V for the oracle tests. */
        sreg |= (!cpu->exact && (R & 0xff) == 0x80) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= R ? 1 : 0;
        sreg |= half_carry(cpu, R | value);
        SREG(cpu) = (uint8_t)sreg;
    } else if (opcode == 0) {
        /* NOP, 0000 0000 0000 0000 */
        BRANCH(cpu, 61);
    } else if ((opcode & 0xfc00) == 0x2800) {
        /* OR, 0010 10rd dddd rrrr */
        const int R = data[d5] | data[r5];
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 62);
        data[d5] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xf000) == 0x6000) {
        /* SBR (ORI), 0110 KKKK dddd KKKK */
        const int R = data[d4] | k8;
        int sreg = SREG(cpu) & 0xe1;

        BRANCH(cpu, 63);
        data[d4] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xf800) == 0xb800) {
        /* OUT, 1011 1AAr rrrr AAAA */
        BRANCH(cpu, 64);
        avr_write_data(cpu, ((opcode & 0xf) | ((opcode & 0x600) >> 5)) + 32, data[d5], 0xff);
    } else if ((opcode & 0xfe0f) == 0x900f) {
        /* POP, 1001 000d dddd 1111 */
        const int32_t value = (int32_t)avr_get16(cpu, AVR_SP) + 1;

        BRANCH(cpu, 65);
        avr_set16(cpu, AVR_SP, (uint32_t)value);
        data[d5] = raw_get(cpu, value);
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x920f) {
        /* PUSH, 1001 001d dddd 1111 */
        const int32_t value = avr_get16(cpu, AVR_SP);

        BRANCH(cpu, 66);
        raw_set(cpu, value, data[d5]);
        avr_set16(cpu, AVR_SP, (uint32_t)(value - 1));
        cpu->cycles++;
    } else if ((opcode & 0xf000) == 0xd000) {
        /* RCALL, 1101 kkkk kkkk kkkk */
        const int k = (opcode & 0x7ff) - ((opcode & 0x800) ? 0x800 : 0);

        BRANCH(cpu, 67);
        push_return(cpu, pc + 1);
        pc += k;
        cpu->cycles += cpu->pc22 ? 3 : 2;
    } else if (opcode == 0x9508 || opcode == 0x9518) {
        /* RET, 1001 0101 0000 1000; RETI, 1001 0101 0001 1000 */
        const int32_t i = (int32_t)avr_get16(cpu, AVR_SP) + (cpu->pc22 ? 3 : 2);

        BRANCH(cpu, opcode == 0x9508 ? 68 : 69);
        avr_set16(cpu, AVR_SP, (uint32_t)i);
        pc = (int64_t)(raw_get(cpu, i - 1) << 8) + raw_get(cpu, i) - 1;
        if (cpu->pc22) {
            pc = (int64_t)((int32_t)pc | (raw_get(cpu, i - 2) << 16));
        }
        cpu->cycles += cpu->pc22 ? 4 : 3;
        if (opcode == 0x9518) {
            SREG(cpu) |= 0x80;   /* enable interrupts */
        }
    } else if ((opcode & 0xf000) == 0xc000) {
        /* RJMP, 1100 kkkk kkkk kkkk */
        BRANCH(cpu, 70);
        pc = pc + ((opcode & 0x7ff) - ((opcode & 0x800) ? 0x800 : 0));
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x9407) {
        /* ROR, 1001 010d dddd 0111 */
        const int d = data[d5];
        const int r = (d >> 1) | ((SREG(cpu) & 1) << 7);
        int sreg = SREG(cpu) & 0xe0;

        BRANCH(cpu, 71);
        data[d5] = (uint8_t)r;
        sreg |= r ? 0 : 2;
        sreg |= (128 & r) ? 4 : 0;
        sreg |= (1 & d) ? 1 : 0;
        sreg |= (((sreg >> 2) & 1) ^ (sreg & 1)) ? 8 : 0;
        sreg |= sign_flag(sreg);
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfc00) == 0x800 || (opcode & 0xf000) == 0x4000) {
        /* SBC, 0000 10rd dddd rrrr; SBCI, 0100 KKKK dddd KKKK */
        const int immediate = (opcode & 0xf000) == 0x4000;
        const int target = immediate ? d4 : d5;
        const int val1 = data[target];
        const int val2 = immediate ? k8 : data[r5];
        int sreg = SREG(cpu);
        const int R = val1 - val2 - (sreg & 1);
        /* avr8js: `!R` on the uncut difference, as in CPC above; the manual
           keeps the previous Z whenever the 8-bit result is zero. `exact`
           keeps avr8js's test. */
        const int zero = cpu->exact ? !R : (R & 0xff) == 0;

        BRANCH(cpu, immediate ? 73 : 72);
        data[target] = (uint8_t)R;
        sreg = (sreg & 0xc0) | ((zero && ((sreg >> 1) & 1)) ? 2 : 0) |
               (val2 + (sreg & 1) > val1 ? 1 : 0);
        sreg |= (128 & R) ? 4 : 0;
        sreg |= ((val1 ^ val2) & (val1 ^ R) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= half_carry(cpu, (~val1 & val2) | (val2 & R) | (R & ~val1));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xff00) == 0x9a00) {
        /* SBI, 1001 1010 AAAA Abbb */
        const int target = ((opcode & 0xf8) >> 3) + 32;
        const int mask = 1 << (opcode & 7);

        BRANCH(cpu, 74);
        avr_write_data(cpu, target, (uint8_t)(avr_read_data(cpu, target) | mask), (uint8_t)mask);
        cpu->cycles++;
    } else if ((opcode & 0xff00) == 0x9900) {
        /* SBIC, 1001 1001 AAAA Abbb */
        const int value = avr_read_data(cpu, ((opcode & 0xf8) >> 3) + 32);

        BRANCH(cpu, 75);
        if (!(value & (1 << (opcode & 7)))) {
            skip_next(cpu, &pc);
        }
    } else if ((opcode & 0xff00) == 0x9b00) {
        /* SBIS, 1001 1011 AAAA Abbb */
        const int value = avr_read_data(cpu, ((opcode & 0xf8) >> 3) + 32);

        BRANCH(cpu, 76);
        if (value & (1 << (opcode & 7))) {
            skip_next(cpu, &pc);
        }
    } else if ((opcode & 0xff00) == 0x9700) {
        /* SBIW, 1001 0111 KKdd KKKK */
        const uint32_t i = 2 * ((opcode & 0x30) >> 4) + 24;
        const int a = avr_get16(cpu, i);
        const int l = (opcode & 0xf) | ((opcode & 0xc0) >> 2);
        const int R = a - l;
        /* avr8js: clears H here (mask 0xc0) and then sets it from a
           half-carry expression. The manual lists SBIW's flags as S, V, N,
           Z, C: H is not affected. We follow the manual and keep H.
           `exact` keeps avr8js's rewritten H for the oracle tests. */
        int sreg = SREG(cpu) & (cpu->exact ? 0xc0 : 0xe0);

        BRANCH(cpu, 77);
        avr_set16(cpu, i, (uint32_t)R);
        sreg |= R ? 0 : 2;
        sreg |= (0x8000 & R) ? 4 : 0;
        sreg |= (a & ~R & 0x8000) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= l > a ? 1 : 0;
        if (cpu->exact) {
            sreg |= (1 & ((~a & l) | (l & R) | (R & ~a))) ? 0x20 : 0;
        }
        SREG(cpu) = (uint8_t)sreg;
        cpu->cycles++;
    } else if ((opcode & 0xfe08) == 0xfc00) {
        /* SBRC, 1111 110r rrrr 0bbb */
        BRANCH(cpu, 78);
        if (!(data[d5] & (1 << (opcode & 7)))) {
            skip_next(cpu, &pc);
        }
    } else if ((opcode & 0xfe08) == 0xfe00) {
        /* SBRS, 1111 111r rrrr 0bbb */
        BRANCH(cpu, 79);
        if (data[d5] & (1 << (opcode & 7))) {
            skip_next(cpu, &pc);
        }
    } else if (opcode == 0x9588) {
        /* SLEEP, 1001 0101 1000 1000: not implemented in avr8js */
        BRANCH(cpu, 80);
    } else if (opcode == 0x95e8) {
        /* SPM, 1001 0101 1110 1000: not implemented in avr8js */
        BRANCH(cpu, 81);
    } else if (opcode == 0x95f8) {
        /* SPM(INC), 1001 0101 1111 1000: not implemented in avr8js */
        BRANCH(cpu, 82);
    } else if ((opcode & 0xfe0f) == 0x9200) {
        /* STS, 1001 001d dddd 0000 kkkk kkkk kkkk kkkk */
        BRANCH(cpu, 83);
        avr_write_data(cpu, avr_flash_word(cpu, pc + 1), data[d5], 0xff);
        pc++;
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x920c) {
        /* STX, 1001 001r rrrr 1100 */
        BRANCH(cpu, 84);
        avr_write_data(cpu, avr_get16(cpu, 26), data[d5], 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x920d) {
        /* STX(INC), 1001 001r rrrr 1101 */
        const int32_t x = avr_get16(cpu, 26);

        BRANCH(cpu, 85);
        avr_write_data(cpu, x, data[d5], 0xff);
        avr_set16(cpu, 26, (uint32_t)(x + 1));
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x920e) {
        /* STX(DEC), 1001 001r rrrr 1110 */
        const uint8_t i = data[d5];
        const int32_t x = (int32_t)avr_get16(cpu, 26) - 1;

        BRANCH(cpu, 86);
        avr_set16(cpu, 26, (uint32_t)x);
        avr_write_data(cpu, x, i, 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x8208) {
        /* STY, 1000 001r rrrr 1000 */
        BRANCH(cpu, 87);
        avr_write_data(cpu, avr_get16(cpu, 28), data[d5], 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x9209) {
        /* STY(INC), 1001 001r rrrr 1001 */
        const uint8_t i = data[d5];
        const int32_t y = avr_get16(cpu, 28);

        BRANCH(cpu, 88);
        avr_write_data(cpu, y, i, 0xff);
        avr_set16(cpu, 28, (uint32_t)(y + 1));
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x920a) {
        /* STY(DEC), 1001 001r rrrr 1010 */
        const uint8_t i = data[d5];
        const int32_t y = (int32_t)avr_get16(cpu, 28) - 1;

        BRANCH(cpu, 89);
        avr_set16(cpu, 28, (uint32_t)y);
        avr_write_data(cpu, y, i, 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xd208) == 0x8208 && q6) {
        /* STDY, 10q0 qq1r rrrr 1qqq */
        BRANCH(cpu, 90);
        avr_write_data(cpu, (int32_t)avr_get16(cpu, 28) + q6, data[d5], 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x8200) {
        /* STZ, 1000 001r rrrr 0000 */
        BRANCH(cpu, 91);
        avr_write_data(cpu, avr_get16(cpu, 30), data[d5], 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x9201) {
        /* STZ(INC), 1001 001r rrrr 0001 */
        const int32_t z = avr_get16(cpu, 30);

        BRANCH(cpu, 92);
        avr_write_data(cpu, z, data[d5], 0xff);
        avr_set16(cpu, 30, (uint32_t)(z + 1));
        cpu->cycles++;
    } else if ((opcode & 0xfe0f) == 0x9202) {
        /* STZ(DEC), 1001 001r rrrr 0010 */
        const uint8_t i = data[d5];
        const int32_t z = (int32_t)avr_get16(cpu, 30) - 1;

        BRANCH(cpu, 93);
        avr_set16(cpu, 30, (uint32_t)z);
        avr_write_data(cpu, z, i, 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xd208) == 0x8200 && q6) {
        /* STDZ, 10q0 qq1r rrrr 0qqq */
        BRANCH(cpu, 94);
        avr_write_data(cpu, (int32_t)avr_get16(cpu, 30) + q6, data[d5], 0xff);
        cpu->cycles++;
    } else if ((opcode & 0xfc00) == 0x1800 || (opcode & 0xf000) == 0x5000) {
        /* SUB, 0001 10rd dddd rrrr; SUBI, 0101 KKKK dddd KKKK */
        const int immediate = (opcode & 0xf000) == 0x5000;
        const int target = immediate ? d4 : d5;
        const int val1 = data[target];
        const int val2 = immediate ? k8 : data[r5];
        const int R = val1 - val2;
        int sreg = SREG(cpu) & 0xc0;

        BRANCH(cpu, immediate ? 96 : 95);
        data[target] = (uint8_t)R;
        sreg |= R ? 0 : 2;
        sreg |= (128 & R) ? 4 : 0;
        sreg |= ((val1 ^ val2) & (val1 ^ R) & 128) ? 8 : 0;
        sreg |= sign_flag(sreg);
        sreg |= val2 > val1 ? 1 : 0;
        sreg |= half_carry(cpu, (~val1 & val2) | (val2 & R) | (R & ~val1));
        SREG(cpu) = (uint8_t)sreg;
    } else if ((opcode & 0xfe0f) == 0x9402) {
        /* SWAP, 1001 010d dddd 0010 */
        const int i = data[d5];

        BRANCH(cpu, 97);
        data[d5] = (uint8_t)(((15 & i) << 4) | ((240 & i) >> 4));
    } else if (opcode == 0x95a8) {
        /* WDR, 1001 0101 1010 1000 */
        BRANCH(cpu, 98);
        if (cpu->watchdog_reset.fn != NULL) {
            cpu->watchdog_reset.fn(cpu->watchdog_reset.ctx);
        }
    } else if ((opcode & 0xfe0f) == 0x9204) {
        /* XCH, 1001 001r rrrr 0100 */
        const uint8_t val1 = data[d5];
        const uint8_t val2 = raw_get(cpu, avr_get16(cpu, 30));

        BRANCH(cpu, 99);
        raw_set(cpu, avr_get16(cpu, 30), val1);
        data[d5] = val2;
    }

    /* avr8js: (pc + 1) % progMem.length, on a number that may be -1. */
    pc += 1;
    if (cpu->flash_words != 0) {
        pc %= (int64_t)cpu->flash_words;
        if (pc < 0) {
            /* A relative jump below address 0. avr8js keeps a negative pc and
               never recovers; a real chip's program counter wraps. */
            pc += (int64_t)cpu->flash_words;
        }
    }
    cpu->pc = (uint32_t)pc;
    cpu->cycles++;
}
