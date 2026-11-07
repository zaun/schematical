#ifndef CPU_STATE_H
#define CPU_STATE_H

/*
 * A processor's state as the simulation sees it (plan-09-B section 4.3),
 * read with circuit_cpu_state (circuit.h).
 */

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CPU_RUNNING,
    CPU_LOST,          /* unknown state: power-up, a used unknown input, a timing violation; only a reset leaves it */
    CPU_RESET,         /* the reset input is held active */
    CPU_WAITING,       /* W65C02S WAI */
    CPU_STOPPED,       /* W65C02S STP */
    CPU_HALTED,        /* Z80 HALT */
    CPU_JAMMED,        /* NMOS 6502 JAM opcode */
    CPU_BUS_RELEASED   /* Z80 BUSACK */
} CpuStatus;

/* One register: its name ("A", "PC", "AF'"), value and width in bits. */
typedef struct { const char *name; uint32_t value; unsigned bits; } CpuRegister;

enum { CPU_MAX_REGISTERS = 24 };

typedef struct CpuState {
    CpuStatus status;
    uint64_t cycles;          /* clock cycles since the last reset ended */
    size_t register_count;    /* 0 while LOST or RESET */
    CpuRegister registers[CPU_MAX_REGISTERS];
    int has_instruction;      /* 1 once an instruction has completed since reset */
    uint32_t instruction_address;  /* 24-bit (bank:address) on the 65816, 16-bit elsewhere */
    uint8_t instruction_bytes[4];
    size_t instruction_length;
    char instruction_text[32]; /* the vendored disassembler's text, e.g. "LDA #$41" */
} CpuState;

#endif
