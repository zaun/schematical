---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "atmega4809",
  "title": "ATmega4808/4809 megaAVR 0-series 8-bit microcontrollers with 48 KB Flash",
  "class": "microcontroller",
  "source": {
    "pdf": "mcu/atmega4809.pdf",
    "vendor": "Microchip",
    "document": "DS40002173A",
    "date": "2020-01",
    "pages": 551
  },
  "devices": [],
  "parts": [],
  "pending": "AVR microcontrollers are not in the catalog yet",
  "converted": "2026-11-07"
}
---

# ATmega4808/4809 megaAVR 0-series 8-bit microcontrollers with 48 KB Flash

## Description

8-bit AVR microcontrollers (AVR CPU with two-cycle hardware multiplier,
up to 20 MHz) with 48 KB in-system self-programmable Flash, 6 KB SRAM,
256 bytes of EEPROM and a 64-byte user row. Flash endurance 10,000
write/erase cycles, EEPROM 100,000; data retention 40 years at 55 °C.
Supply 1.8 V to 5.5 V. Industrial (–40 °C to 85 °C) and extended
(–40 °C to 125 °C) temperature ranges; VAO variants are qualified to
AEC-Q100.

The ATmega4808 comes in 28-pin SSOP and 32-pin TQFP/VQFN packages, the
ATmega4809 in 40-pin PDIP and 48-pin TQFP/UQFN. The 40-pin ATmega4809
uses the 48-pin die with fewer pads bonded: PB5:0 and PC7:6 must have
their input buffer disabled (PORT.PINnCTRL ISC = INPUT_DISABLE) or their
pull-up enabled (PULLUPEN).

On-chip: a two-level interrupt controller; an event system (6 channels on
the 28/32-pin parts, 8 on the 40/48-pin parts); clocks from a 16/20 MHz
internal oscillator, a 32.768 kHz ultra-low-power oscillator, a
32.768 kHz crystal oscillator or an external clock; power-on reset,
brown-out detector and voltage-level monitor; idle, standby and
power-down sleep modes; one 16-bit timer/counter type A (three compare
channels, or split into two 8-bit timers); three (28/32-pin) or four
(40/48-pin) 16-bit timer/counters type B; a 16-bit real-time counter
with periodic interrupt timer; three or four USARTs; one SPI; one TWI
(I²C; standard, fast and fast-mode plus, master and slave at the same
time on different pins); configurable custom logic with four LUTs; one
analog comparator (4 positive, 3 negative inputs); one 10-bit 150 ksps
ADC (8, 12 or 16 channels by package); internal references of 0.55,
1.1, 1.5, 2.5 and 4.3 V; a CRC memory scanner; a window watchdog;
external interrupts on every I/O pin; and the single-pin UPDI for
programming and on-chip debugging. Up to 41 I/O lines.

```table id=family caption="Package variants"
| Feature | ATmega4808 SSOP28 | ATmega4808 TQFP32/VQFN32 | ATmega4809 PDIP40 | ATmega4809 TQFP48/UQFN48 |
|---|---|---|---|---|
| Pins | 28 | 32 | 40 | 48 |
| TCB instances | 3 | 3 | 4 | 4 |
| USART instances | 3 | 3 | 4 | 4 |
| ADC channels | 8 | 12 | 16 | 16 |
| Event system channels | 6 | 6 | 8 | 8 |
| General-purpose I/O | 23 | 27 | 33 | 41 |
| Ports | PA7:0, PC3:0, PD7:0, PF0, PF1, PF6 | PA7:0, PC3:0, PD7:0, PF6:0 | PA7:0, PC5:0, PD7:0, PE3:0, PF6:0 | PA7:0, PB5:0, PC7:0, PD7:0, PE3:0, PF6:0 |
| Fully asynchronous external interrupt pins | 6 | 7 | 8 | 10 |
```

## Pinout

```table id=pinout-table caption="Pin numbers per package"
| Pin name | TQFP48/UQFN48 | PDIP40 | TQFP32/VQFN32 | SSOP28 |
|---|--:|--:|--:|--:|
| PA0 | 44 | 33 | 30 | 22 |
| PA1 | 45 | 34 | 31 | 23 |
| PA2 | 46 | 35 | 32 | 24 |
| PA3 | 47 | 36 | 1 | 25 |
| PA4 | 48 | 37 | 2 | 26 |
| PA5 | 1 | 38 | 3 | 27 |
| PA6 | 2 | 39 | 4 | 28 |
| PA7 | 3 | 40 | 5 | 1 |
| PB0 | 4 | | | |
| PB1 | 5 | | | |
| PB2 | 6 | | | |
| PB3 | 7 | | | |
| PB4 | 8 | | | |
| PB5 | 9 | | | |
| PC0 | 10 | 1 | 6 | 2 |
| PC1 | 11 | 2 | 7 | 3 |
| PC2 | 12 | 3 | 8 | 4 |
| PC3 | 13 | 4 | 9 | 5 |
| VDD | 14 | 5 | | |
| GND | 15 | 6 | | |
| PC4 | 16 | 7 | | |
| PC5 | 17 | 8 | | |
| PC6 | 18 | | | |
| PC7 | 19 | | | |
| PD0 | 20 | 9 | 10 | 6 |
| PD1 | 21 | 10 | 11 | 7 |
| PD2 | 22 | 11 | 12 | 8 |
| PD3 | 23 | 12 | 13 | 9 |
| PD4 | 24 | 13 | 14 | 10 |
| PD5 | 25 | 14 | 15 | 11 |
| PD6 | 26 | 15 | 16 | 12 |
| PD7 | 27 | 16 | 17 | 13 |
| AVDD | 28 | 17 | 18 | 14 |
| GND | 29 | 18 | 19 | 15 |
| PE0 | 30 | 19 | | |
| PE1 | 31 | 20 | | |
| PE2 | 32 | 21 | | |
| PE3 | 33 | 22 | | |
| PF0 | 34 | 23 | 20 | 16 |
| PF1 | 35 | 24 | 21 | 17 |
| PF2 | 36 | 25 | 22 | |
| PF3 | 37 | 26 | 23 | |
| PF4 | 38 | 27 | 24 | |
| PF5 | 39 | 28 | 25 | |
| PF6 | 40 | 29 | 26 | 18 |
| UPDI | 41 | 30 | 27 | 19 |
| VDD | 42 | 31 | 28 | 20 |
| GND | 43 | 32 | 29 | 21 |
```

The VQFN32 and UQFN48 exposed pads are not given a pin function in the
source.

## Pin functions

```pins
PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7: Port A I/O
PB0, PB1, PB2, PB3, PB4, PB5: Port B I/O (48-pin only)
PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7: Port C I/O (PC4, PC5 on 40/48-pin, PC6, PC7 on 48-pin only)
PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7: Port D I/O
PE0, PE1, PE2, PE3: Port E I/O (40/48-pin only)
PF0, PF1, PF2, PF3, PF4, PF5, PF6: Port F I/O (PF2–PF5 not on SSOP28). PF6 is the !{RESET} input when FUSE.SYSCFG0.RSTPINCFG = 1, a GPIO otherwise.
UPDI: Unified program and debug interface: dedicated single-wire, half-duplex programming and debug pin; input with pull-up when idle
VDD: Digital supply
AVDD: Analog supply (port D)
GND: Ground
```

Every I/O pin can be an event-system input and an external interrupt
source; Px2 and Px6 of each port detect interrupts fully
asynchronously. Pin names are Pxn (port x, pin n); signals are named
PORTx_PINn. In the table below `n,TxD` is a signal of USARTn, `0-WO2` is
TCA0 waveform output 2, `1-WO` the output of TCB1, `0-IN0` input 0 of
CCL LUT0. Special: EXTCLK external clock input, CLKOUT system clock
output, TOSC1/TOSC2 32.768 kHz crystal, VREFA external ADC reference,
TWI marks the pins that carry a TWI function.

```table id=mux caption="I/O multiplexing [1]"
| Pin | Special | ADC0 | AC0 | USARTn | SPI0 | TWI0 | TCA0 | TCBn | EVSYS | CCL LUTn |
|---|---|---|---|---|---|---|---|---|---|---|
| PA0 | EXTCLK | | | 0,TxD | | | 0-WO0 | | | 0-IN0 |
| PA1 | | | | 0,RxD | | | 0-WO1 | | | 0-IN1 |
| PA2 | TWI | | | 0,XCK | | SDA(MS) | 0-WO2 | 0-WO | EVOUTA | 0-IN2 |
| PA3 | TWI | | | 0,XDIR | | SCL(MS) | 0-WO3 | 1-WO | | 0-OUT |
| PA4 | | | | 0,TxD [2] | MOSI | | 0-WO4 | | | |
| PA5 | | | | 0,RxD [2] | MISO | | 0-WO5 | | | |
| PA6 | | | | 0,XCK [2] | SCK | | | | | 0-OUT [2] |
| PA7 | CLKOUT | | OUT | 0,XDIR [2] | !{SS} | | | | EVOUTA [2] | |
| PB0 | | | | 3,TxD | | | 0-WO0 [2] | | | |
| PB1 | | | | 3,RxD | | | 0-WO1 [2] | | | |
| PB2 | | | | 3,XCK | | | 0-WO2 [2] | | EVOUTB | |
| PB3 | | | | 3,XDIR | | | 0-WO3 [2] | | | |
| PB4 | | | | 3,TxD [2] | | | 0-WO4 [2] | 2-WO [2] | | |
| PB5 | | | | 3,RxD [2] | | | 0-WO5 [2] | 3-WO | | |
| PC0 | | | | 1,TxD | MOSI [2] | | 0-WO0 [2] | 2-WO | | 1-IN0 |
| PC1 | | | | 1,RxD | MISO [2] | | 0-WO1 [2] | 3-WO [2] | | 1-IN1 |
| PC2 | TWI | | | 1,XCK | SCK [2] | SDA(MS) [2] | 0-WO2 [2] | | EVOUTC | 1-IN2 |
| PC3 | TWI | | | 1,XDIR | !{SS} [2] | SCL(MS) [2] | 0-WO3 [2] | | | 1-OUT |
| PC4 | | | | 1,TxD [2] | | | 0-WO4 [2] | | | |
| PC5 | | | | 1,RxD [2] | | | 0-WO5 [2] | | | |
| PC6 | | | | 1,XCK [2] | | | | | | 1-OUT [2] |
| PC7 | | | | 1,XDIR [2] | | | | | EVOUTC [2] | |
| PD0 | | AIN0 | | | | | 0-WO0 [2] | | | 2-IN0 |
| PD1 | | AIN1 | P3 | | | | 0-WO1 [2] | | | 2-IN1 |
| PD2 | | AIN2 | P0 | | | | 0-WO2 [2] | | EVOUTD | 2-IN2 |
| PD3 | | AIN3 | N0 | | | | 0-WO3 [2] | | | 2-OUT |
| PD4 | | AIN4 | P1 | | | | 0-WO4 [2] | | | |
| PD5 | | AIN5 | N1 | | | | 0-WO5 [2] | | | |
| PD6 | | AIN6 | P2 | | | | | | | 2-OUT [2] |
| PD7 | VREFA | AIN7 | N2 | | | | | | EVOUTD [2] | |
| PE0 | | AIN8 | | | MOSI [2] | | 0-WO0 [2] | | | |
| PE1 | | AIN9 | | | MISO [2] | | 0-WO1 [2] | | | |
| PE2 | | AIN10 | | | SCK [2] | | 0-WO2 [2] | | EVOUTE | |
| PE3 | | AIN11 | | | !{SS} [2] | | 0-WO3 [2] | | | |
| PF0 | TOSC1 | | | 2,TxD | | | 0-WO0 [2] | | | 3-IN0 |
| PF1 | TOSC2 | | | 2,RxD | | | 0-WO1 [2] | | | 3-IN1 |
| PF2 | TWI | AIN12 | | 2,XCK | | SDA(S) [2] | 0-WO2 [2] | | EVOUTF | 3-IN2 |
| PF3 | TWI | AIN13 | | 2,XDIR | | SCL(S) [2] | 0-WO3 [2] | | | 3-OUT |
| PF4 | | AIN14 | | 2,TxD [2] | | | 0-WO4 [2] | 0-WO [2] | | |
| PF5 | | AIN15 | | 2,RxD [2] | | | 0-WO5 [2] | 1-WO [2] | | |
| PF6 | !{RESET} | | | 2,XCK [2] | | | | | | 3-OUT [2] |

[1] SDA(MS)/SCL(MS): TWI master/slave pins; SDA(S)/SCL(S): the separate slave pins of dual mode. AC0 P0–P3 and N0–N2 are the comparator's positive and negative inputs.
[2] Alternate pin position, selected in [PORTMUX](#peripherals).
```

## Functional description

Block diagram: the CPU (with on-chip debug), the CRC scanner and the
UPDI are bus masters on a bus matrix whose slaves are the SRAM, the
Flash and EEPROM (through NVMCTRL) and the I/O bus to the peripherals:
ports and GPIOR, CPUINT, the system-management block (RSTCTRL, CLKCTRL,
SLPCTRL, WDT, RTC), detectors and references (POR, BOD/VLM, bandgap),
clock generation (OSC20M, OSC32K, XOSC32K, EXTCLK, CLKOUT), and the
peripherals tied together by the event routing network (EVSYS, CCL,
TCA, TCB, USART, SPI, TWI, AC, ADC).

Conventions used in the register descriptions:

- **Register access**: R = read, W = write; registers with the suffix
  SET, CLR and TGL form a pair with the register they act on (OUT and
  OUTSET read the same value): writing 1 to a bit of the CLR, SET or TGL
  register clears, sets or toggles that bit in both; 0 bits are
  unaffected.
- **Enable-protected** bits can only be written while the peripheral's
  ENABLE bit is 0 (or in the same write that sets it).
- **CCP**: the register (or bit) is under configuration change
  protection; see [Programming model](#programming-model).
- **Synchronized** registers cross into another clock domain; their
  busy flags must be checked.
- A peripheral's software reset resets all its registers except its
  debug control register.
- In C, registers are named `<peripheral_instance_name>.<register_name>`
  (`CPU.SREG`, `USART2.CTRLA`, `PORTB.DIR`); a predefined value is
  written `<peripheral_name>_<bit_field_name>_<bit_field_value>_gc`
  (`USART_RXMODE_CLK2X_gc`), the value from the Name column of the
  register tables. Peripherals with different register sets in
  different modes add a mode name (`TCA0.SINGLE.CTRL =
  TCA_SINGLE_WGMODE_FRQ_gc`).

## Programming model

The AVR CPU is a Harvard machine with a single-level pipeline: the next
instruction is fetched while one executes, so most instructions run in
one CPU clock (CLK_CPU, undivided), up to 1 MIPS/MHz. An ALU operation
on two registers reads both operands, executes and writes the result in
one cycle. The ALU does 8- and 16-bit arithmetic, logic and bit
operations and updates SREG; the hardware multiplier multiplies two 8-bit
numbers into a 16-bit result in two cycles (signed, unsigned,
signed × unsigned, and the fractional forms of these). Program flow:
after reset the CPU executes from Flash address 0x0000; JMP and CALL
reach the whole program space; most instructions are one 16-bit word,
a few are two.

### Register file

Thirty-two 8-bit general-purpose registers R0–R31, single-cycle, in
their own address space (not reachable with data-memory instructions).
The register file supports one 8-bit operand and 8-bit result, two
8-bit operands and an 8-bit result, two 8-bit operands and a 16-bit
result, and one 16-bit operand and 16-bit result. R26:R27, R28:R29 and
R30:R31 form the 16-bit pointers X, Y and Z (low byte in the lower
register), used for indirect data addressing with fixed displacement,
post-increment and pre-decrement. All three serve LD/ST; only Z serves
LPM, IJMP and ICALL.

```table id=cpu-regfile caption="Register file"
| Register | Use |
|---|---|
| R0–R25 | General purpose (R0:R1 receive multiply results) |
| R26, R27 | XL, XH: X pointer |
| R28, R29 | YL, YH: Y pointer |
| R30, R31 | ZL, ZH: Z pointer |
```

### CPU I/O registers (base 0x0030)

```table id=cpu-ccp caption="CCP (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | CCP[7:0] | R/W | 0x00 | Write a signature to unlock protected I/O registers or protected instructions for the next four instructions: 0x9D = SPM: allow self-programming; 0xD8 = IOREG: unprotect protected I/O registers. While the IOREG unlock is active CCP\[0\] reads 1; while the SPM unlock is active CCP\[1\] reads 1; CCP[7:2] always read 0. |
```

```table id=cpu-sp caption="SP (+0x0D, 16-bit: SPL, SPH), reset: top of internal SRAM"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | SP[15:0] | R/W | | Stack pointer. Only the bits needed for the data memory are implemented; unused bits read 0. Writing SPL disables interrupts for four instructions or until the next I/O memory write, whichever comes first. |
```

```table id=cpu-sreg caption="SREG (+0x0F), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | I | R/W | 0 | Global interrupt enable: 0 disables all interrupts regardless of the peripheral enables. Not cleared by hardware on interrupt entry. Set and cleared by SEI and CLI; changing I through the I/O register costs one wait state. |
| 6 | T | R/W | 0 | Bit copy storage: source/destination of BST and BLD |
| 5 | H | R/W | 0 | Half carry (BCD arithmetic) |
| 4 | S | R/W | 0 | Sign: S = N ⊕ V |
| 3 | V | R/W | 0 | Two's complement overflow |
| 2 | N | R/W | 0 | Negative result |
| 1 | Z | R/W | 0 | Zero result |
| 0 | C | R/W | 0 | Carry |
```

SREG is updated by every ALU instruction as listed in the
[instruction set](#instruction-set). It is **not** saved or restored on
interrupt entry and RETI; the interrupt routine must do it.

### Stack

The stack lives in SRAM and grows down. SP points to the top of the
stack and is set to the highest SRAM address at reset; if software moves
it, it must point above the SRAM start (0x2800) and be set before any
call and before interrupts are enabled.

```table id=cpu-stack caption="Stack pointer changes"
| Instruction | SP | Description |
|---|---|---|
| PUSH | decremented by 1 | Data pushed |
| CALL, ICALL, RCALL, interrupt | decremented by 2 | Return address pushed |
| POP | incremented by 1 | Data popped |
| RET, RETI | incremented by 2 | Return address popped |
```

Return addresses are word addresses (byte address 0x0006 is stacked as
0x0003); the low byte is pushed first, at the higher address.

### 16-bit registers

The data bus is 8 bits wide; 16-bit peripheral registers are reached
through a temporary register. Write the low byte first (it goes to the
temporary register), then the high byte (both bytes land in the same
cycle). Read the low byte first (the high byte is copied to the
temporary register in the same cycle), then the high byte. An interrupt
routine that accesses the same 16-bit register between the two accesses
corrupts the sequence, so disable interrupts around such accesses when
needed. Software can read and write the temporary registers directly.

### Configuration change protection

Protected I/O registers (marked CCP in the register tables) and the
NVMCTRL command register are locked. To write a protected I/O register,
write 0xD8 (IOREG) to CPU.CCP, then within four instructions write the
register (with its enable/change/lock bit in the same write where it has
one). To execute self-programming, write 0x9D (SPM) to CPU.CCP, then
within four instructions write the NVMCTRL command. The unlock ends at
once if the CPU writes an I/O register or data memory, accesses Flash,
NVMCTRL or EEPROM by load or store, or executes SLEEP. While unlocked,
all interrupts (including NMI) are ignored; their flags are set and the
requests stay pending until the four instructions are over, then run by
level and priority.

### Reset and interrupts

Reset sources (power-on, brown-out, external pin, watchdog, software,
UPDI) are handled by [RSTCTRL](#peripherals); after reset the CPU
starts at Flash address 0x0000 with SP
at the top of SRAM and SREG cleared. Interrupts are prioritized by
[CPUINT](#peripherals): a non-maskable interrupt (CRC failure), level 1
(high, one vector selectable) and level 0 (normal) with static or
round-robin priority. The vector table is in the
[memory map](#memory-map). A peripheral sets its INTFLAGS flag; with the
matching INTCTRL enable set and SREG.I = 1 the request stays active
until the flag is cleared.

### On-chip debug

Native OCD through the [UPDI](#peripherals): two hardware breakpoints;
breakpoints on change of flow, on interrupts and in software (BREAK);
run-time readout of SP, PC and SREG; the register file can be read and
written while stopped, and execution can resume.

## Instruction set

The PDF's summary gives mnemonic, operands, description, operation, flags
and clock cycles; it gives neither opcodes nor instruction words, so the
table has no opcode or words column. Most instructions are one 16-bit
word; a few use a 32-bit format (see [programming model](#programming-model)).

```table id=isa-operands caption="Operand and notation symbols"
| Symbol | Meaning |
|---|---|
| Rd, Rr | Destination (and source) register, source register, in the register file |
| R | Result after the instruction is executed |
| K | Constant literal or byte data (8-bit) |
| k | Constant address data for the program counter |
| b | Bit in the register file or I/O register (3-bit) |
| s | Bit in the status register (3-bit) |
| X, Y, Z | Indirect address registers: X = R27:R26, Y = R29:R28, Z = R31:R30 |
| A (P) | I/O space address |
| q | Displacement for indirect addressing with displacement (6-bit) |
| UU, SS, SU | Multiply operands unsigned × unsigned, signed × signed, signed × unsigned |
| DS(X), DS(Y), DS(Z), DS(k) | Data-space location addressed by X, Y, Z or the constant k |
| PS(Z) | Program-space location addressed by Z |
| I/O(A) | Location A in I/O space |
| STACK, SP | Stack for return addresses and pushed registers; stack pointer |
| C, Z, N, V, S, H, T, I | SREG flags: carry, zero, negative, two's complement overflow, sign (S = N ⊕ V), half carry, transfer bit (BLD/BST), global interrupt enable |
| ∧ ∨ ⊕ << >> == | AND, OR, XOR, shift left, shift right, comparison; Rd(n) is bit n of Rd |
```

```table id=isa-addressing caption="Addressing modes"
| Mode | Operand syntax | Effective address / operand |
|---|---|---|
| Register direct | Rd; Rd, Rr | Register file R0–R31 (one or two registers) |
| Immediate | Rd, K | 8-bit constant K in the instruction (ADIW/SBIW: K added to the pair Rd+1:Rd) |
| I/O direct | A; A, b | I/O space address A in the instruction (IN, OUT; SBI, CBI, SBIC, SBIS on the low 32 I/O addresses) |
| Data direct | k | 16-bit data-space address k in the second instruction word (LDS, STS); the whole 64 KB unified data space |
| Data indirect | X, Y, Z | Data-space address in the pointer register |
| Indirect, post-increment | X+, Y+, Z+ | Address in the pointer; the pointer is then incremented by 1 |
| Indirect, pre-decrement | -X, -Y, -Z | The pointer is decremented by 1, then used as the address |
| Indirect with displacement | Y+q, Z+q | Pointer Y or Z plus the 6-bit displacement q |
| Program memory constant | Z; Z+ | Program-space byte addressed by Z (LPM; Z only) |
| Direct program | k | JMP, CALL: PC ← k (whole program space) |
| Relative program | k | RJMP, RCALL, branches: PC ← PC + k + 1 |
| Indirect program | — | IJMP, ICALL: PC ← Z |
```

```table id=isa caption="Instruction set summary [1][2]"
| Mnemonic | Operands | Description | Operation | Flags | Cycles |
|---|---|---|---|---|---|
| Arithmetic and logic | < | < | < | < | < |
| ADD | Rd, Rr | Add without carry | Rd ← Rd + Rr | Z,C,N,V,S,H | 1 |
| ADC | Rd, Rr | Add with carry | Rd ← Rd + Rr + C | Z,C,N,V,S,H | 1 |
| ADIW | Rd, K | Add immediate to word | Rd+1:Rd ← Rd+1:Rd + K | Z,C,N,V,S | 2 |
| SUB | Rd, Rr | Subtract without carry | Rd ← Rd - Rr | Z,C,N,V,S,H | 1 |
| SUBI | Rd, K | Subtract immediate | Rd ← Rd - K | Z,C,N,V,S,H | 1 |
| SBC | Rd, Rr | Subtract with carry | Rd ← Rd - Rr - C | Z,C,N,V,S,H | 1 |
| SBCI | Rd, K | Subtract immediate with carry | Rd ← Rd - K - C | Z,C,N,V,S,H | 1 |
| SBIW | Rd, K | Subtract immediate from word | Rd+1:Rd ← Rd+1:Rd - K | Z,C,N,V,S | 2 |
| AND | Rd, Rr | Logical AND | Rd ← Rd ∧ Rr | Z,N,V,S | 1 |
| ANDI | Rd, K | Logical AND with immediate | Rd ← Rd ∧ K | Z,N,V,S | 1 |
| OR | Rd, Rr | Logical OR | Rd ← Rd ∨ Rr | Z,N,V,S | 1 |
| ORI | Rd, K | Logical OR with immediate | Rd ← Rd ∨ K | Z,N,V,S | 1 |
| EOR | Rd, Rr | Exclusive OR | Rd ← Rd ⊕ Rr | Z,N,V,S | 1 |
| COM | Rd | One's complement | Rd ← 0xFF - Rd | Z,C,N,V,S | 1 |
| NEG | Rd | Two's complement | Rd ← 0x00 - Rd | Z,C,N,V,S,H | 1 |
| SBR | Rd, K | Set bit(s) in register | Rd ← Rd ∨ K | Z,N,V,S | 1 |
| CBR | Rd, K | Clear bit(s) in register | Rd ← Rd ∧ (0xFF - K) | Z,N,V,S | 1 |
| INC | Rd | Increment | Rd ← Rd + 1 | Z,N,V,S | 1 |
| DEC | Rd | Decrement | Rd ← Rd - 1 | Z,N,V,S | 1 |
| TST | Rd | Test for zero or minus | Rd ← Rd ∧ Rd | Z,N,V,S | 1 |
| CLR | Rd | Clear register | Rd ← Rd ⊕ Rd | Z,N,V,S | 1 |
| SER | Rd | Set register | Rd ← 0xFF | None | 1 |
| MUL | Rd, Rr | Multiply unsigned | R1:R0 ← Rd × Rr (UU) | Z,C | 2 |
| MULS | Rd, Rr | Multiply signed | R1:R0 ← Rd × Rr (SS) | Z,C | 2 |
| MULSU | Rd, Rr | Multiply signed with unsigned | R1:R0 ← Rd × Rr (SU) | Z,C | 2 |
| FMUL | Rd, Rr | Fractional multiply unsigned | R1:R0 ← Rd × Rr << 1 (UU) | Z,C | 2 |
| FMULS | Rd, Rr | Fractional multiply signed | R1:R0 ← Rd × Rr << 1 (SS) | Z,C | 2 |
| FMULSU | Rd, Rr | Fractional multiply signed with unsigned | R1:R0 ← Rd × Rr << 1 (SU) | Z,C | 2 |
| Branch | < | < | < | < | < |
| RJMP | k | Relative jump | PC ← PC + k + 1 | None | 2 |
| IJMP | | Indirect jump to (Z) | PC ← Z | None | 2 |
| JMP | k | Jump | PC ← k | None | 3 |
| RCALL | k | Relative call subroutine | PC ← PC + k + 1 | None | 2/3 |
| ICALL | | Indirect call to (Z) | PC ← Z | None | 2/3 |
| CALL | k | Call subroutine | PC ← k | None | 3/4 |
| RET | | Subroutine return | PC ← STACK | None | 4/5 |
| RETI | | Interrupt return | PC ← STACK | I | 4/5 |
| CPSE | Rd, Rr | Compare, skip if equal | if (Rd == Rr) PC ← PC + 2 or 3 | None | 1/2/3 |
| CP | Rd, Rr | Compare | Rd - Rr | Z,C,N,V,S,H | 1 |
| CPC | Rd, Rr | Compare with carry | Rd - Rr - C | Z,C,N,V,S,H | 1 |
| CPI | Rd, K | Compare with immediate | Rd - K | Z,C,N,V,S,H | 1 |
| SBRC | Rr, b | Skip if bit in register cleared | if (Rr(b) == 0) PC ← PC + 2 or 3 | None | 1/2/3 |
| SBRS | Rr, b | Skip if bit in register set | if (Rr(b) == 1) PC ← PC + 2 or 3 | None | 1/2/3 |
| SBIC | A, b | Skip if bit in I/O register cleared | if (I/O(A,b) == 0) PC ← PC + 2 or 3 | None | 1/2/3 |
| SBIS | A, b | Skip if bit in I/O register set | if (I/O(A,b) == 1) PC ← PC + 2 or 3 | None | 1/2/3 |
| BRBS | s, k | Branch if status flag set | if (SREG(s) == 1) PC ← PC + k + 1 | None | 1/2 |
| BRBC | s, k | Branch if status flag cleared | if (SREG(s) == 0) PC ← PC + k + 1 | None | 1/2 |
| BREQ | k | Branch if equal | if (Z == 1) PC ← PC + k + 1 | None | 1/2 |
| BRNE | k | Branch if not equal | if (Z == 0) PC ← PC + k + 1 | None | 1/2 |
| BRCS | k | Branch if carry set | if (C == 1) PC ← PC + k + 1 | None | 1/2 |
| BRCC | k | Branch if carry cleared | if (C == 0) PC ← PC + k + 1 | None | 1/2 |
| BRSH | k | Branch if same or higher | if (C == 0) PC ← PC + k + 1 | None | 1/2 |
| BRLO | k | Branch if lower | if (C == 1) PC ← PC + k + 1 | None | 1/2 |
| BRMI | k | Branch if minus | if (N == 1) PC ← PC + k + 1 | None | 1/2 |
| BRPL | k | Branch if plus | if (N == 0) PC ← PC + k + 1 | None | 1/2 |
| BRGE | k | Branch if greater or equal, signed | if (N ⊕ V == 0) PC ← PC + k + 1 | None | 1/2 |
| BRLT | k | Branch if less than, signed | if (N ⊕ V == 1) PC ← PC + k + 1 | None | 1/2 |
| BRHS | k | Branch if half carry flag set | if (H == 1) PC ← PC + k + 1 | None | 1/2 |
| BRHC | k | Branch if half carry flag cleared | if (H == 0) PC ← PC + k + 1 | None | 1/2 |
| BRTS | k | Branch if T flag set | if (T == 1) PC ← PC + k + 1 | None | 1/2 |
| BRTC | k | Branch if T flag cleared | if (T == 0) PC ← PC + k + 1 | None | 1/2 |
| BRVS | k | Branch if overflow flag set | if (V == 1) PC ← PC + k + 1 | None | 1/2 |
| BRVC | k | Branch if overflow flag cleared | if (V == 0) PC ← PC + k + 1 | None | 1/2 |
| BRIE | k | Branch if interrupt enabled | if (I == 1) PC ← PC + k + 1 | None | 1/2 |
| BRID | k | Branch if interrupt disabled | if (I == 0) PC ← PC + k + 1 | None | 1/2 |
| Data transfer | < | < | < | < | < |
| MOV | Rd, Rr | Copy register | Rd ← Rr | None | 1 |
| MOVW | Rd, Rr | Copy register pair | Rd+1:Rd ← Rr+1:Rr | None | 1 |
| LDI | Rd, K | Load immediate | Rd ← K | None | 1 |
| LDS | Rd, k | Load direct from data space | Rd ← DS(k) | None | 3 [1] |
| LD | Rd, X | Load indirect | Rd ← DS(X) | None | 2 [1] |
| LD | Rd, X+ | Load indirect and post-increment | Rd ← DS(X), X ← X + 1 | None | 2 [1] |
| LD | Rd, -X | Load indirect and pre-decrement | X ← X - 1, Rd ← DS(X) | None | 2 [1] |
| LD | Rd, Y | Load indirect | Rd ← DS(Y) | None | 2 [1] |
| LD | Rd, Y+ | Load indirect and post-increment | Rd ← DS(Y), Y ← Y + 1 | None | 2 [1] |
| LD | Rd, -Y | Load indirect and pre-decrement | Y ← Y - 1, Rd ← DS(Y) | None | 2 [1] |
| LDD | Rd, Y+q | Load indirect with displacement | Rd ← DS(Y + q) | None | 2 [1] |
| LD | Rd, Z | Load indirect | Rd ← DS(Z) | None | 2 [1] |
| LD | Rd, Z+ | Load indirect and post-increment | Rd ← DS(Z), Z ← Z + 1 | None | 2 [1] |
| LD | Rd, -Z | Load indirect and pre-decrement | Z ← Z - 1, Rd ← DS(Z) | None | 2 [1] |
| LDD | Rd, Z+q | Load indirect with displacement | Rd ← DS(Z + q) | None | 2 [1] |
| STS | k, Rr | Store direct to data space | DS(k) ← Rr [3] | None | 2 [1][2] |
| ST | X, Rr | Store indirect | DS(X) ← Rr | None | 1 [1][2] |
| ST | X+, Rr | Store indirect and post-increment | DS(X) ← Rr, X ← X + 1 | None | 1 [1][2] |
| ST | -X, Rr | Store indirect and pre-decrement | X ← X - 1, DS(X) ← Rr | None | 1 [1][2] |
| ST | Y, Rr | Store indirect | DS(Y) ← Rr | None | 1 [1][2] |
| ST | Y+, Rr | Store indirect and post-increment | DS(Y) ← Rr, Y ← Y + 1 | None | 1 [1][2] |
| ST | -Y, Rr | Store indirect and pre-decrement | Y ← Y - 1, DS(Y) ← Rr | None | 1 [1][2] |
| STD | Y+q, Rr | Store indirect with displacement | DS(Y + q) ← Rr | None | 1 [1][2] |
| ST | Z, Rr | Store indirect | DS(Z) ← Rr | None | 1 [1][2] |
| ST | Z+, Rr | Store indirect and post-increment | DS(Z) ← Rr, Z ← Z + 1 | None | 1 [1][2] |
| ST | -Z, Rr | Store indirect and pre-decrement | Z ← Z - 1, DS(Z) ← Rr | None | 1 [1][2] |
| STD | Z+q, Rr | Store indirect with displacement | DS(Z + q) ← Rr | None | 1 [1][2] |
| LPM | | Load program memory | R0 ← PS(Z) | None | 3 |
| LPM | Rd, Z | Load program memory | Rd ← PS(Z) | None | 3 |
| LPM | Rd, Z+ | Load program memory and post-increment | Rd ← PS(Z), Z ← Z + 1 | None | 3 |
| IN | Rd, A | In from I/O location | Rd ← I/O(A) | None | 1 |
| OUT | A, Rr | Out to I/O location | I/O(A) ← Rr | None | 1 |
| PUSH | Rr | Push register on stack | STACK ← Rr | None | 1 |
| POP | Rd | Pop register from stack | Rd ← STACK | None | 2 |
| Bit and bit-test | < | < | < | < | < |
| LSL | Rd | Logical shift left | Rd(n+1) ← Rd(n) for n = 0…6, Rd(0) ← 0, C ← Rd(7) | Z,C,N,V,H | 1 |
| LSR | Rd | Logical shift right | Rd(n) ← Rd(n+1) for n = 0…6, Rd(7) ← 0, C ← Rd(0) | Z,C,N,V | 1 |
| ROL | Rd | Rotate left through carry | Rd(0) ← C, Rd(n+1) ← Rd(n) for n = 0…6, C ← Rd(7) | Z,C,N,V,H | 1 |
| ROR | Rd | Rotate right through carry | Rd(7) ← C, Rd(n) ← Rd(n+1) for n = 0…6, C ← Rd(0) | Z,C,N,V | 1 |
| ASR | Rd | Arithmetic shift right | Rd(n) ← Rd(n+1) for n = 0…6, C ← Rd(0), Rd(7) ← Rd(7) | Z,C,N,V | 1 |
| SWAP | Rd | Swap nibbles | Rd(3…0) ↔ Rd(7…4) | None | 1 |
| SBI | A, b | Set bit in I/O register | I/O(A, b) ← 1 | None | 1 |
| CBI | A, b | Clear bit in I/O register | I/O(A, b) ← 0 | None | 1 |
| BST | Rr, b | Bit store from register to T | T ← Rr(b) | T | 1 |
| BLD | Rd, b | Bit load from T to register | Rd(b) ← T | None | 1 |
| BSET | s | Flag set | SREG(s) ← 1 | SREG(s) | 1 |
| BCLR | s | Flag clear | SREG(s) ← 0 | SREG(s) | 1 |
| SEC | | Set carry | C ← 1 | C | 1 |
| CLC | | Clear carry | C ← 0 | C | 1 |
| SEN | | Set negative flag | N ← 1 | N | 1 |
| CLN | | Clear negative flag | N ← 0 | N | 1 |
| SEZ | | Set zero flag | Z ← 1 | Z | 1 |
| CLZ | | Clear zero flag | Z ← 0 | Z | 1 |
| SEI | | Global interrupt enable | I ← 1 | I | 1 |
| CLI | | Global interrupt disable | I ← 0 | I | 1 |
| SES | | Set signed test flag | S ← 1 | S | 1 |
| CLS | | Clear signed test flag | S ← 0 | S | 1 |
| SEV | | Set two's complement overflow | V ← 1 | V | 1 |
| CLV | | Clear two's complement overflow | V ← 0 | V | 1 |
| SET | | Set T in SREG | T ← 1 | T | 1 |
| CLT | | Clear T in SREG | T ← 0 | T | 1 |
| SEH | | Set half carry flag in SREG | H ← 1 | H | 1 |
| CLH | | Clear half carry flag in SREG | H ← 0 | H | 1 |
| MCU control | < | < | < | < | < |
| BREAK | | Break (see [UPDI](#peripherals) debug) | | None | 1 |
| NOP | | No operation | | None | 1 |
| SLEEP | | Sleep (see SLPCTRL) | | None | 1 |
| WDR | | Watchdog reset (see WDT) | | None | 1 |

[1] Data-memory access cycles assume internal RAM; they are not valid for accesses to the NVM. Add at least one extra cycle when reading Flash and EEPROM.
[2] One extra cycle must be added when accessing the lower 64 bytes of I/O space.
[3] Source error: the instruction set summary gives STS's operation as DS(k) ← Rd; its operands are k, Rr, and ST/STD store Rr.
```

## Memory map

```table id=map caption="Data space (both devices)"
| Address | Size | Contents |
|---|---|---|
| 0x0000–0x003F | 64 I/O registers | I/O registers (IN/OUT; 0x00–0x1F bit-accessible with SBI, CBI, SBIS, SBIC) |
| 0x0040–0x0FFF | 960 Ext I/O registers [1] | Extended I/O registers (LD/LDS/LDD, ST/STS/STD) |
| 0x1000–0x13FF | | NVM I/O registers and data: NVMCTRL 0x1000, SIGROW 0x1100, FUSE 0x1280, USERROW 0x1300 |
| 0x1400–0x14FF | 256 B | EEPROM |
| 0x1500–0x27FF | | Reserved |
| 0x2800–0x3FFF | 6 KB | Internal SRAM |
| 0x4000–0xFFFF | 48 KB | Flash, mapped for LD/ST |

[1] Unconfirmed: Figure 7-1 labels the block 0x0040 – 0x0FFF "960 Ext I/O Registers"; the PDF does not say how that count relates to the address range.
```

Code space: Flash at 0x0000–0xBFFF (byte addresses for LPM; the program
counter addresses all of it). Peripheral base addresses are listed under
[registers and commands](#registers).

```table id=mem-props caption="Memory properties"
| Memory | Size | Page size | Pages | Start address |
|---|---|---|--:|---|
| Flash | 48 KB, 16-bit wide | 128 B | 384 | 0x0000 code space, 0x4000 data space |
| SRAM | 6 KB | | | 0x2800 |
| EEPROM | 256 B | 64 B | 4 | 0x1400 |
| User row | 64 B | | | 0x1300 |
```

**Flash** holds the program; it can be split for write protection into
a boot section, an application code section and an application data
section (FUSE.BOOTEND, FUSE.APPEND), and code in one section can be
barred from writing others (see [NVMCTRL](#peripherals)). It is read
with LPM (from 0x0000) or with LD (from 0x4000). The CRC scanner is a
second bus master that reads it. **SRAM** holds data only; code cannot
execute from it. **EEPROM** supports single-byte read and write through
NVMCTRL. The **user row** is one extra EEPROM page for firmware
settings, read and written by the CPU like EEPROM, by the UPDI when the
part is unlocked, and written (blindly) by the UPDI even when it is
locked; a chip erase leaves it alone.

### Interrupt vectors

Each vector is two words (room for a JMP); addresses are word offsets
from the vector table base, which CPUINT.CTRLA.IVSEL places at the start
of the application section (IVSEL = 0) or of the boot section
(IVSEL = 1). A peripheral sets its INTFLAGS flag on the interrupt
condition; with the matching INTCTRL enable set and interrupts enabled
globally, the request stays active until the flag is cleared.

```table id=vectors caption="Interrupt vectors"
| Vector | Address | Source | Packages |
|--:|---|---|---|
| 0 | 0x00 | RESET | all |
| 1 | 0x02 | NMI: non-maskable interrupt from CRCSCAN | all |
| 2 | 0x04 | VLM: voltage level monitor (BOD) | all |
| 3 | 0x06 | RTC: overflow or compare match | all |
| 4 | 0x08 | PIT: periodic interrupt (RTC) | all |
| 5 | 0x0A | CCL | all |
| 6 | 0x0C | PORTA external interrupt | all |
| 7 | 0x0E | TCA0 overflow | all |
| 8 | 0x10 | TCA0 underflow (split mode) | all |
| 9 | 0x12 | TCA0 compare channel 0 | all |
| 10 | 0x14 | TCA0 compare channel 1 | all |
| 11 | 0x16 | TCA0 compare channel 2 | all |
| 12 | 0x18 | TCB0 capture | all |
| 13 | 0x1A | TCB1 capture | all |
| 14 | 0x1C | TWI0 slave | all |
| 15 | 0x1E | TWI0 master | all |
| 16 | 0x20 | SPI0 | all |
| 17 | 0x22 | USART0 receive complete | all |
| 18 | 0x24 | USART0 data register empty | all |
| 19 | 0x26 | USART0 transmit complete | all |
| 20 | 0x28 | PORTD external interrupt | all |
| 21 | 0x2A | AC0 compare | all |
| 22 | 0x2C | ADC0 result ready | all |
| 23 | 0x2E | ADC0 window compare | all |
| 24 | 0x30 | PORTC external interrupt | all |
| 25 | 0x32 | TCB2 capture | all |
| 26 | 0x34 | USART1 receive complete | all |
| 27 | 0x36 | USART1 data register empty | all |
| 28 | 0x38 | USART1 transmit complete | all |
| 29 | 0x3A | PORTF external interrupt | all |
| 30 | 0x3C | NVM ready | all |
| 31 | 0x3E | USART2 receive complete | all |
| 32 | 0x40 | USART2 data register empty | all |
| 33 | 0x42 | USART2 transmit complete | all |
| 34 | 0x44 | PORTB external interrupt | 48-pin |
| 35 | 0x46 | PORTE external interrupt | 40, 48-pin |
| 36 | 0x48 | TCB3 capture | 40, 48-pin |
| 37 | 0x4A | USART3 receive complete | 40, 48-pin |
| 38 | 0x4C | USART3 data register empty | 40, 48-pin |
| 39 | 0x4E | USART3 transmit complete | 40, 48-pin |
```

### I/O memory and general-purpose registers

All I/O and peripherals are in the I/O space. 0x00–0x3F is reached in
one cycle with IN and OUT; 0x00–0x1F also with SBI, CBI, SBIS and SBIC,
which act on the named bit only, so they can be used on registers with
write-1-to-clear flags. Reserved bits should be written 0; reserved
addresses should never be written. Four general-purpose I/O registers
GPIOR0–GPIOR3 at 0x001C–0x001F (reset 0x00, all bits R/W) hold global
variables and flags with bit access.

### Signature row (SIGROW, base 0x1100)

Factory-programmed, read-only (also readable when the part is locked).
Reset values are the factory contents.

```table id=sigrow caption="Signature row"
| Offset | Name | Bits | Content |
|---|---|---|---|
| 0x00–0x02 | DEVICEID0–2 | 7:0 | Device ID: ATmega4809 = 0x1E 0x96 0x51; ATmega4808 = 0x1E 0x96 0x50 |
| 0x03–0x0C | SERNUM0–9 | 7:0 | Ten-byte unique serial number |
| 0x18 | OSCCAL16M0 | 6:0 OSCCAL16M | 16 MHz oscillator calibration; copied to CLKCTRL.OSC20MCALIBA at reset when FUSE.OSCCFG selects 16 MHz |
| 0x19 | OSCCAL16M1 | 3:0 OSCCAL16MTCAL | 16 MHz oscillator temperature calibration; copied to CLKCTRL.OSC20MCALIBB at reset when 16 MHz is selected |
| 0x1A | OSCCAL20M0 | 6:0 OSCCAL20M | 20 MHz oscillator calibration; copied to OSC20MCALIBA when 20 MHz is selected |
| 0x1B | OSCCAL20M1 | 3:0 OSCCAL20MTCAL | 20 MHz oscillator temperature calibration; copied to OSC20MCALIBB when 20 MHz is selected |
| 0x20 | TEMPSENSE0 | 7:0 | Temperature sensor gain/slope correction (unsigned), see [ADC](#peripherals) |
| 0x21 | TEMPSENSE1 | 7:0 | Temperature sensor offset correction (signed) |
| 0x22 | OSC16ERR3V | 7:0 | Signed frequency error at 16 MHz, 3 V, measured in production |
| 0x23 | OSC16ERR5V | 7:0 | Signed frequency error at 16 MHz, 5 V |
| 0x24 | OSC20ERR3V | 7:0 | Signed frequency error at 20 MHz, 3 V |
| 0x25 | OSC20ERR5V | 7:0 | Signed frequency error at 20 MHz, 5 V |
```

### Fuses (FUSE, base 0x1280)

Fuses hold the device configuration, are valid from power-up, and are
copied into their target registers at the end of the start-up sequence.
The CPU can read them; only the UPDI can program or clear them.
Changes take effect after the next reset. Reserved bits must be written
0.

```table id=fuse-wdtcfg caption="WDTCFG (+0x00), factory value 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | WINDOW[3:0] | R | x | Loaded into WDT.CTRLA.WINDOW at reset |
| 3:0 | PERIOD[3:0] | R | x | Loaded into WDT.CTRLA.PERIOD at reset |
```

```table id=fuse-bodcfg caption="BODCFG (+0x01), factory value 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:5 | LVL[2:0] | R | x | Loaded into BOD.CTRLB.LVL: 0x0 = BODLEVEL0 (1.8 V), 0x2 = BODLEVEL2 (2.6 V), 0x7 = BODLEVEL7 (4.3 V); other reserved. Typical values; see the BOD electrical characteristics. |
| 4 | SAMPFREQ | R | x | Loaded into BOD.CTRLA.SAMPFREQ: 0 = 1 kHz, 1 = 125 Hz sampling |
| 3:2 | ACTIVE[1:0] | R | x | Loaded into BOD.CTRLA.ACTIVE (active and idle): 0x0 = disabled, 0x1 = enabled, 0x2 = sampled, 0x3 = enabled with wake-up halted until the BOD is ready |
| 1:0 | SLEEP[1:0] | R | x | Loaded into BOD.CTRLA.SLEEP: 0x0 = disabled, 0x1 = enabled, 0x2 = sampled, 0x3 reserved |
```

The BOD settings are reloaded from BODCFG only after a power-on reset;
other resets leave the BOD configuration unchanged.

```table id=fuse-osccfg caption="OSCCFG (+0x02), factory value 0x02"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | OSCLOCK | R | x | Loaded into CLKCTRL.OSC20MCALIBB.LOCK: 0 = 20 MHz oscillator calibration registers writable at run time, 1 = locked |
| 1:0 | FREQSEL[1:0] | R | x | OSC20M frequency, and which factory calibration is loaded into CAL20M and TEMPCAL20M: 0x1 = 16 MHz, 0x2 = 20 MHz; 0x0, 0x3 reserved |
```

```table id=fuse-syscfg0 caption="SYSCFG0 (+0x05), factory value 0xC0"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | CRCSRC[1:0] | R | x | CRC scan at reset: 0x0 = FLASH: whole Flash (boot, application code and data); 0x1 = BOOT: boot section; 0x2 = BOOTAPP: application code and boot sections; 0x3 = NOCRC: none. See [CRCSCAN](#peripherals). |
| 3 | RSTPINCFG | R | x | PF6 function: 0 = GPIO, 1 = !{RESET} |
| 0 | EESAVE | R | x | 0 = chip erase erases the EEPROM, 1 = EEPROM kept. A locked device always erases the EEPROM. |
```

```table id=fuse-syscfg1 caption="SYSCFG1 (+0x06), factory value 0x07"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | SUT[2:0] | R | x | Start-up time from power-on to code execution: 0x0 = 0 ms, 0x1 = 1 ms, 0x2 = 2 ms, 0x3 = 4 ms, 0x4 = 8 ms, 0x5 = 16 ms, 0x6 = 32 ms, 0x7 = 64 ms |
```

```table id=fuse-append caption="APPEND (+0x07) and BOOTEND (+0x08), factory value 0x00"
| Fuse | Bit | Name | Access | Description |
|---|---|---|---|---|
| APPEND | 7:0 | APPEND[7:0] | R | End of the application code section, in 256-byte blocks (boot size plus application code size); the remaining Flash is application data. 0x00: application code runs from BOOTEND × 256 to the end of Flash. |
| BOOTEND | 7:0 | BOOTEND[7:0] | R | End of the boot section in 256-byte blocks; 0x00 makes the whole Flash boot section. With APPEND and BOOTEND both 0x00 the whole Flash is boot section. |
```

```table id=fuse-lockbit caption="LOCKBIT (+0x0A), factory value 0xC5"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | LOCKBIT[7:0] | R | x | 0xC5 = valid key, device open; any other value = device locked. When locked, the UPDI cannot reach the system bus and can only read its CS space and the ASI. |
```

### Lock

Writing any invalid key to FUSE.LOCKBIT locks the part: Flash (all
sections), SRAM, EEPROM and fuses can no longer be read or written
through the UPDI; the application keeps full access. The only way to
unlock is a chip erase, which erases all memories to factory default.

```table id=lock-access caption="Memory access by CPU and UPDI"
| Memory | CPU read | CPU write | UPDI read, unlocked | UPDI write, unlocked | UPDI read, locked | UPDI write, locked |
|---|---|---|---|---|---|---|
| SRAM | Yes | Yes | Yes | Yes | No [1] | No |
| Registers | Yes | Yes | Yes | Yes | No [1] | No |
| Flash | Yes | Yes | Yes | Yes | No [1] | No |
| EEPROM | Yes | Yes | Yes | Yes | No [1] | No |
| USERROW | Yes | Yes | Yes | Yes | No [1] | Yes [2] |
| SIGROW | Yes | No | Yes | No | No [1] | No |
| Other fuses | Yes | No | Yes | Yes | No [1] | No |

[1] A read may appear to succeed but returns corrupt data, so verification through the UPDI fails.
[2] Blind write with the fuse-write (user-row) command; the current contents cannot be read.
```

## Peripherals

One subsection per peripheral, in the source's order. Register offsets
are relative to the peripheral's base address (see
[registers and commands](#registers)); bits a table does not list are
reserved and must be written 0. "CCP" in a caption marks a register
under configuration change protection (see
[programming model](#programming-model)).

### SYSCFG: system configuration

```table id=syscfg-revid caption="REVID (+0x01), reset: revision ID"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | REVID[7:0] | R | x | Device revision: 0x00 = A, 0x01 = B, and so on. Lets firmware adapt to the part revision. |
```

### NVMCTRL: nonvolatile memory controller

The interface between the CPU and the Flash, EEPROM, signature row and
user row (one NVM block). The Flash holds the program and may hold data;
the EEPROM can be programmed while the CPU runs from Flash.

#### Flash sections

The Flash is written and erased one page at a time. It is split, in
blocks of 256 bytes, into three sections set by the fuses FUSE.BOOTEND
and FUSE.APPEND: BOOT from the start of Flash to BOOTEND, application
code (APPCODE) from BOOTEND to APPEND, and application data (APPDATA)
from APPEND to the end. APPEND must be 0 or ≥ BOOTEND. Interrupt vectors
sit after the BOOT section by default (movable in CPUINT). Example:
BOOTEND = 0x04, APPEND = 0x08 gives 4 × 256 bytes BOOT, the next
4 × 256 bytes APPCODE, the rest APPDATA.

```table id=nvmctrl-sections caption="Flash sections from BOOTEND and APPEND (byte offsets into Flash)"
| BOOTEND | APPEND | BOOT | APPCODE | APPDATA |
|---|---|---|---|---|
| 0 | 0 | 0 to FLASHEND | — | — |
| > 0 | 0 | 0 to 256 × BOOTEND | 256 × BOOTEND to FLASHEND | — |
| > 0 | = BOOTEND | 0 to 256 × BOOTEND | — | 256 × BOOTEND to FLASHEND |
| > 0 | > BOOTEND | 0 to 256 × BOOTEND | 256 × BOOTEND to 256 × APPEND | 256 × APPEND to FLASHEND |
```

Write protection is directional: code in BOOT may write APPCODE and
APPDATA, code in APPCODE may write APPDATA, code in APPDATA may write
neither Flash nor EEPROM. The CPU can never write the BOOT section.
CTRLB.APCWP locks APPCODE against writes and CTRLB.BOOTLOCK locks BOOT
against reads and instruction fetch, both until the next reset.

The EEPROM is paged too, but erases and writes with byte granularity:
only bytes written to the page buffer are erased or written. The user
row is one extra EEPROM page, written like EEPROM, not erased by chip
erase, and writable through UPDI on a locked device.

#### Access and commands

Flash and EEPROM are read with load instructions at their data-space
addresses (see [memory map](#memory-map)); a read while a write or erase
is in progress stalls the CPU (bus wait) until it completes. The page
buffer is loaded by storing to the memory's addresses; the address LSbs
pick the buffer location, and the new data is ANDed with the buffer's
previous content. Flash, EEPROM and user row share the one page buffer,
so only one can be programmed at a time. The buffer is set to all 1s
after a reset, after any page write or erase, after a page-buffer-clear
command, and on wake-up from any sleep mode.

A Flash page must be erased before it is written (writing an unerased
page corrupts it): either fill the buffer and issue Erase/Write Page, or
store to one location of the page, issue Erase Page, fill the buffer and
issue Write Page (split commands are shorter each, so the erase can run
at a non-critical time). EEPROM works the same, on the updated bytes
only.

To issue a command: wait until STATUS.EEBUSY and STATUS.FBUSY are 0;
write the SPM key (0x9D) to CPU.CCP; write CTRLA.CMD within the next
four instructions.

- **Write page (WP)** writes the page buffer to Flash or EEPROM. During
  a Flash write the CPU is halted; during an EEPROM write it keeps
  running. The buffer is cleared afterwards.
- **Erase page (ER)** needs at least one byte written to the buffer.
  Flash: write one address in the page, then the whole page is erased,
  CPU halted. EEPROM: only the bytes written to the buffer are erased
  (write every byte of the page to erase it all); the CPU keeps running.
  The buffer is cleared afterwards.
- **Erase/write page (ERWP)** erases the page and then writes the buffer
  to it without clearing the buffer in between. CPU halted for Flash,
  running for EEPROM. Buffer cleared afterwards.
- **Page buffer clear (PBC)** sets the buffer to all 1s; the CPU is
  halted for seven cycles.
- **Chip erase (CHER)** erases Flash and EEPROM (EEPROM is kept when
  FUSE.SYSCFG0.EESAVE is set), ignoring BOOTLOCK and APCWP; all 1s after.
- **EEPROM erase (EEER)** sets the whole EEPROM to 1s; CPU halted.
- **Write fuse (WFU)** is UPDI-only: write the fuse address to ADDR, the
  value to DATA, issue the command; the new value takes effect after a
  reset. Fuses are read with ordinary loads.

#### Corruption, interrupts, sleep

Too low a V~DD~ can corrupt Flash or EEPROM, both because a write needs a
minimum voltage and because the CPU can misexecute. Keep the part in
reset during low supply (enable the BOD), use the BOD's voltage level
monitor to avoid starting an EEPROM write near the BOD level, or use an
external low-V~DD~ reset; a reset during a write aborts the write.

Interrupt EEREADY (vector offset 0x00, "NVM"): the EEPROM is ready for a
new write/erase. With no write in progress, NVMCTRL sleeps with the
system; a write in progress keeps the NVM block, NVMCTRL and the system
clock running until it finishes, in every sleep mode including
power-down. EEREADY wakes the part only from idle. The page buffer is
cleared on wake-up. CTRLA is configuration change protected with key SPM.

#### Registers

```table id=nvmctrl-ctrla caption="CTRLA (+0x00), reset 0x00, CCP (SPM)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | CMD[2:0] | R/W | 0x0 | Command, written within four instructions of the SPM key: 0x0 = no command; 0x1 = WP: write page buffer to memory (ADDR selects which); 0x2 = ER: erase page; 0x3 = ERWP: erase and write page; 0x4 = PBC: page buffer clear; 0x5 = CHER: chip erase, Flash and EEPROM (EEPROM kept if EESAVE = 1); 0x6 = EEER: EEPROM erase; 0x7 = WFU: write fuse (UPDI only) |
```

```table id=nvmctrl-ctrlb caption="CTRLB (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | BOOTLOCK | R/W | 0 | Boot section lock: 1 = reads of BOOT return 0 and fetches return 0 as instruction. Writable from the BOOT section only; takes effect when BOOT is first left after the write; cleared only by reset. |
| 0 | APCWP | R/W | 0 | Application code section write protection: 1 = APPCODE can't be written. Can only be written 1; cleared only by reset. |
```

```table id=nvmctrl-status caption="STATUS (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2 | WRERROR | R | 0 | Write error in the last operation (e.g. page buffer written in different sections before a page write, or a write to a protected area) |
| 1 | EEBUSY | R | 0 | EEPROM busy with a command |
| 0 | FBUSY | R | 0 | Flash busy with a command |
```

```table id=nvmctrl-intctrl caption="INTCTRL (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | EEREADY | R/W | 0 | EEPROM ready interrupt enable. A level interrupt: active while INTFLAGS.EEREADY is set, which it is whenever the EEPROM is idle, so enable it only after issuing an NVM command, and disable it in the handler. |
```

```table id=nvmctrl-intflags caption="INTFLAGS (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | EEREADY | R/W | 0 | Set continuously while the EEPROM is not busy; cleared by writing 1 |
```

```table id=nvmctrl-data caption="DATA (+0x06, 16-bit: DATAL, DATAH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | DATA[15:0] | R/W | 0 | Data for UPDI fuse writes |
```

```table id=nvmctrl-addr caption="ADDR (+0x08, 16-bit: ADDRL, ADDRH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | ADDR[15:0] | R/W | 0 | Address of the last memory location updated (fuse address for WFU) |
```

### CLKCTRL: clock controller

Controls, distributes and prescales the clocks. Every peripheral
requests the clocks it needs, and a requested source is started
automatically (routed to the right source when several exist); an
unrequested source turns off.

- **CLK_MAIN**, the main clock, is selected by MCLKCTRLA.CLKSEL and
  divided by the main prescaler. It drives CLK_CPU (CPU, SRAM, NVMCTRL's
  memory access) and CLK_PER (every peripheral not listed below, the I/O
  bus). It always runs in active and idle; in standby only when a
  peripheral requests it or the source's RUNSTDBY is 1; in power-down it
  stops after all NVM operations complete.
- **Asynchronous clocks**, configured in their peripherals: CLK_RTC for
  the RTC/PIT (requested while the RTC/PIT is enabled; change its source
  only while the peripheral is disabled), CLK_WDT for the WDT (while
  enabled), CLK_BOD for the BOD (while enabled in sampled mode).

CLKOUT (output) carries CLK_PER when MCLKCTRLA.CLKOUT = 1.

#### Sources

- **OSC20M**, 16/20 MHz internal oscillator; FUSE.OSCCFG.FREQSEL picks
  16 or 20 MHz at reset. Factory calibration is loaded from fuses at
  reset: OSC20MCALIBA.CAL20M (around the centre frequency) and
  OSC20MCALIBB.TEMPCAL20M (slope of the temperature-drift compensation).
  Fuse OSCLOCK = 1 locks the calibration; it is also locked while
  OSC20M is the main clock and MCLKLOCK.LOCKEN = 1. The calibration
  registers are CCP-protected.
- **OSCULP32K**, 32 kHz ultra-low-power internal oscillator: less
  accurate than a crystal; also supplies the 1 kHz clock for the RTC, WDT
  and BOD. Start-up: the oscillator start-up time plus four cycles.
- **XOSC32K**, 32.768 kHz crystal oscillator on the dedicated pins TOSC1
  and TOSC2, or (XOSC32KCTRLA.SEL = 1) a 32 kHz external clock on TOSC1.
  It must be enabled (XOSC32KCTRLA.ENABLE = 1) before it can run, even
  when requested; enabling overrides the pins' GPIO configuration.
  Crystal start-up time is set by CSUT; with an external clock it is two
  cycles.
- **EXTCLK**, an external clock on the EXTCLK pin, configured
  automatically when a peripheral requests it; start-up two cycles.

The MCLKSTATUS status bits show whether each source is running and
stable. Writing an oscillator's RUNSTDBY = 1 requests it permanently
(running in every mode except power-down) and removes the start-up time
when a peripheral later requests it.

**Stored frequency error.** For a wider operating range, the signature
row holds the factory-measured frequency error after calibration, as
signed bytes: SIGROW.OSC16ERR3V and OSC16ERR5V (error from 16 MHz at
3 V and 5 V), OSC20ERR3V and OSC20ERR5V (from 20 MHz). Each is a
compressed Q1.10 fixed-point value: MSb the sign, the seven LSbs the
low bits of the Q1.10. Applied to a USART baud value (asynchronous, no
auto-baud):

```equation
BAUD~actual~ = BAUD~ideal~ + (BAUD~ideal~ × SigRowError) / 1024
```

The smallest legal BAUD value is 0x40, so BAUD~ideal~ should not be
below 0x4A, so the result stays legal even with the largest negative
correction.

#### Main clock switching and prescaler

Any source can be the main clock and can be changed safely at run time.
A switch to an external source only happens once clock edges show it is
stable; until then the switch is pending, and no other source can be
selected again without a reset. MCLKSTATUS.SOSC = 1 while a switch is
pending; EXTS and XOSC32KS show the external sources' stability. If an
external source fails while it is the main clock, only the WDT (through
a system reset) can recover. The prescaler divides CLK_MAIN by 1 (off),
2, 4, 6, 8, 10, 12, 16, 24, 32, 48 or 64 into CLK_PER.

After any reset CLK_MAIN is OSC20M (at the FUSE.OSCCFG.FREQSEL frequency)
with the prescaler dividing by 6.

All CLKCTRL registers with a writable bit (MCLKCTRLA, MCLKCTRLB,
MCLKLOCK, OSC20MCTRLA, OSC20MCALIBA, OSC20MCALIBB, OSC32KCTRLA,
XOSC32KCTRLA) are configuration change protected with key IOREG.

#### Registers

```table id=clkctrl-mclkctrla caption="MCLKCTRLA (+0x00), reset 0x00, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | CLKOUT | R/W | 0 | 1 = system clock output on the CLKOUT pin. No output in sleep unless a peripheral uses the system clock. |
| 1:0 | CLKSEL[1:0] | R/W | 0x0 | Main clock source: 0x0 = OSC20M: 16/20 MHz internal oscillator; 0x1 = OSCULP32K: 32 kHz internal ultra-low-power oscillator; 0x2 = XOSC32K: 32.768 kHz crystal oscillator; 0x3 = EXTCLK: external clock |
```

```table id=clkctrl-mclkctrlb caption="MCLKCTRLB (+0x01), reset 0x11, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4:1 | PDIV[3:0] | R/W | 0x8 | Prescaler division, used when PEN = 1; may be changed at run time: 0x0 = 2, 0x1 = 4, 0x2 = 8, 0x3 = 16, 0x4 = 32, 0x5 = 64, 0x8 = 6, 0x9 = 10, 0xA = 12, 0xB = 24, 0xC = 48; other reserved. Software must keep CLK_PER within its maximum (see [electrical characteristics](#electrical-characteristics)). |
| 0 | PEN | R/W | 1 | Prescaler enable: 0 = CLK_PER = CLK_MAIN regardless of PDIV, 1 = divide by PDIV |
```

```table id=clkctrl-mclklock caption="MCLKLOCK (+0x02), reset from FUSE.OSCCFG.OSCLOCK, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | LOCKEN | R/W | x | 1 = MCLKCTRLA, MCLKCTRLB and, where applicable, the current main source's calibration are locked against software writes; MCLKLOCK itself is then locked until the next hardware reset. Loaded from FUSE.OSCCFG.OSCLOCK at reset. |
```

```table id=clkctrl-mclkstatus caption="MCLKSTATUS (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | EXTS | R | 0 | 1 = EXTCLK has started |
| 6 | XOSC32KS | R | 0 | 1 = XOSC32K is stable [1] |
| 5 | OSC32KS | R | 0 | 1 = OSCULP32K is stable [1] |
| 4 | OSC20MS | R | 0 | 1 = OSC20M is stable [1] |
| 0 | SOSC | R | 0 | Main clock oscillator changing: 1 = a CLK_MAIN source switch is pending; the clock changes as soon as the new source is stable |

[1] Valid only when the source is requested as main clock or by another module; reads 0 if the source has RUNSTDBY = 1 but is not requested.
```

```table id=clkctrl-osc20mctrla caption="OSC20MCTRLA (+0x10), reset 0x00, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | RUNSTDBY | R/W | 0 | Run standby: 1 = oscillator forced on in all modes even when unused, so standby wake-up does not wait for start-up. Output only goes to requesting peripherals; after a request the clock gate opens in four oscillator cycles, without the analog start-up time. |
```

```table id=clkctrl-osc20mcaliba caption="OSC20MCALIBA (+0x11), reset from FUSE.OSCCFG.FREQSEL, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:0 | CAL20M[6:0] | R/W | x | Fine-tunes the frequency around the current centre frequency. Factory value for the FREQSEL frequency loaded at reset. |
```

```table id=clkctrl-osc20mcalibb caption="OSC20MCALIBB (+0x12), reset from FUSE.OSCCFG, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | LOCK | R | x | Calibration locked by fuse: 1 = OSC20MCALIBA and OSC20MCALIBB can't be changed. Loaded from FUSE.OSCCFG.OSCLOCK. |
| 3:0 | TEMPCAL20M[3:0] | R/W | x | Slope of the temperature compensation. Factory value for the FREQSEL frequency loaded at reset. |
```

```table id=clkctrl-osc32kctrla caption="OSC32KCTRLA (+0x18), reset 0x00, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | RUNSTDBY | R/W | 0 | Run standby, as OSC20MCTRLA.RUNSTDBY, for OSCULP32K |
```

SEL and CSUT can't be changed while ENABLE = 1 or MCLKSTATUS.XOSC32KS = 1:
to change them, write ENABLE = 0, wait for XOSC32KS = 0, then re-enable
with the new settings.

```table id=clkctrl-xosc32kctrla caption="XOSC32KCTRLA (+0x1C), reset 0x00, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5:4 | CSUT[1:0] | R/W | 0x0 | Crystal start-up time: 0x0 = 1K: 1k cycles; 0x1 = 16K: 16k cycles; 0x2 = 32K: 32k cycles; 0x3 = 64K: 64k cycles. Not applied when SEL = 1. Read-only while ENABLE = 1. |
| 2 | SEL | R/W | 0 | Source: 0 = external crystal, 1 = external clock on TOSC1. Read-only while ENABLE = 1. |
| 1 | RUNSTDBY | R/W | 0 | 1 = (with ENABLE = 1) crystal oscillator started and kept on in active, idle and standby even when unused; once started, a request gets the output after two to three crystal cycles. 0 = runs only when requested and ENABLE = 1. Output goes only to requesting peripherals. |
| 0 | ENABLE | R/W | 0 | 1 = XOSC32K may run; the TOSC1 and TOSC2 pin configuration is overridden, SEL and CSUT become read-only |
```

#### Electrical characteristics

V~DD~ = 3 V unless noted. Oscillator frequencies above the speed
specification must be divided so that the CPU clock stays within it.

```table id=osc20m-char caption="16/20 MHz internal oscillator (OSC20M)"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f~OSC20M~ | Factory calibration frequency | FREQSEL = 0x1, T~A~ = 25 °C, 3.0 V | | 16 | | MHz |
| ^ | ^ | FREQSEL = 0x2, T~A~ = 25 °C, 3.0 V | | 20 | | MHz |
| f~CAL~ | Frequency calibration range | FREQSEL = 0x1 | 14.5 | | 17.5 | MHz |
| ^ | ^ | FREQSEL = 0x2 | 18.5 | | 21.5 | MHz |
| E~TOTAL~ | Total error from target frequency, 16 and 20 MHz | T~A~ = 25 °C, 3.0 V | -1.5 | | 1.5 | % |
| ^ | ^ | T~A~ = 0 °C to 70 °C, V~DD~ = 1.8 V to 3.6 V | -2.0 | | 2.0 | % |
| ^ | ^ | Full operation range | -4.0 | | 4.0 | % |
| E~DRIFT~ | Accuracy relative to the factory-stored frequency value, 16 and 20 MHz, factory calibrated at V~DD~ = 3 V [1] | T~A~ = 0 °C to 70 °C, V~DD~ = 1.8 V to 5.5 V | -1.8 | | 1.8 | % |
| Δf~OSC20M~ | Calibration step size | | | 0.75 | | % |
| D~OSC20M~ | Duty cycle | | | 50 | | % |
| t~startup~ | Start-up time | Within 2 % accuracy | | 12 | | µs |

[1] See the OSC20M calibration description.
```

```table id=osculp32k-char caption="32.768 kHz internal ultra-low-power oscillator (OSCULP32K)"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f~OSCULP32K~ | Factory calibration frequency | | | 32.768 | | kHz |
| ^ | Factory calibration accuracy | T~A~ = 25 °C, 3.0 V | -3 | | 3 | % |
| E~TOTAL~ | Total error from target frequency | T~A~ = 0 °C to 70 °C, V~DD~ = 1.8 V to 3.6 V | -10 | | 10 | % |
| ^ | ^ | Full operation range | -20 | | 20 | % |
| D~OSCULP32K~ | Duty cycle | | | 50 | | % |
| t~startup~ | Start-up time | | | 250 | | µs |
```

```table id=xosc32k-char caption="32.768 kHz external crystal oscillator (XOSC32K)"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f~out~ | Frequency | | | 32.768 | | kHz |
| t~startup~ | Start-up time | C~L~ = 7.5 pF | | 300 | | ms |
| C~L~ | Crystal load capacitance [1] | | 7.5 | | 12.5 | pF |
| C~TOSC1/TOSC2~ | Parasitic pin capacitance | | | 5.5 | | pF |
| ESR | Equivalent series resistance, safety factor 3 [1] | C~L~ = 7.5 pF | | | 80 | kΩ |
| ^ | ^ | C~L~ = 12.5 pF | | | 40 | kΩ |

[1] Design guidance only, not production tested.
```

External clock on EXTCLK: t~CLCL~ is the period, t~CHCX~ and t~CLCX~ the
high and low times measured at V~IH1~ and V~IL1~, t~CLCH~ and t~CHCL~ the
rise and fall times.

```table id=extclk-char caption="External clock characteristics [1]"
| Symbol | Parameter | V~DD~ = 1.8 V to 5.5 V | < | V~DD~ = 2.7 V to 5.5 V | < | V~DD~ = 4.5 V to 5.5 V | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| f~CLCL~ | Frequency | 0 | 5.0 | 0 | 10.0 | 0 | 20.0 | MHz |
| t~CLCL~ | Clock period | 200 | | 100 | | 50 | | ns |
| t~CHCX~ | High time | 80 | | 40 | | 20 | | ns |
| t~CLCX~ | Low time | 80 | | 40 | | 20 | | ns |
| t~CLCH~ | Rise time (for maximum frequency) | | 40 | | 20 | | 10 | ns |
| t~CHCL~ | Fall time (for maximum frequency) | | 40 | | 20 | | 10 | ns |
| Δt~CLCL~ | Change in period from one clock cycle to the next | | 20 | | 20 | | 20 | % |

[1] t~CHCX~ through Δt~CLCL~ are design guidance only, not production tested.
```

### SLPCTRL: sleep controller

Handles the transitions between active mode and three sleep modes:
idle, standby and power-down. To sleep: enable the interrupts that are
to wake the part, and global interrupts (with none enabled only a reset
wakes it); write CTRLA.SMODE and CTRLA.SEN = 1; execute SLEEP. An
interrupt wakes the part, runs its handler, and execution continues
after the SLEEP instruction; any reset ends sleep and starts from the
reset vector. Register file, SRAM and registers keep their contents in
sleep.

- **Idle:** the CPU stops; no peripheral is stopped; every interrupt
  can wake the part.
- **Standby:** each peripheral runs or not according to its RUNSTBY
  bit, so consumption lies between idle and power-down. SleepWalking is
  available for the ADC.
- **Power-down:** the BOD, WDT and PIT (part of the RTC) run. Wake-up
  sources: pin change interrupt, PIT, VLM, TWI address match and CCL.

```table id=slpctrl-modes caption="Activity in sleep modes (X = active)"
| Group | Peripheral or source | Clock | Idle | Standby | Power-down |
|---|---|---|:-:|:-:|:-:|
| Active clock domain | CPU | CLK_CPU | | | |
| ^ | Peripherals | CLK_PER | X | | |
| ^ | RTC | CLK_RTC | X | X [1] | |
| ^ | CCL | CLK_PER [2] | X | X [1] | |
| ^ | ADCn | CLK_PER | X | X [1] | |
| ^ | TCBn | CLK_PER | X | X [1] | |
| ^ | PIT (RTC) | CLK_RTC | X | X | X |
| ^ | BOD (VLM) | CLK_BOD | X | X | X |
| ^ | WDT | CLK_WDT | X | X | X |
| Oscillators | Main clock source | | X | X [1] | |
| ^ | PIT and RTC clock source | | X | X [1] | X [3] |
| ^ | BOD oscillator | | X | X | X |
| ^ | WDT oscillator | | X | X | X |
| Wake-up sources | INTn and pin change | | X | X | X |
| ^ | TWI address match | | X | X | X |
| ^ | PIT (RTC) | | X | X | X |
| ^ | CCL | | X | X [1] | X [4] |
| ^ | RTC | | X | X [1] | |
| ^ | USART start-of-frame | | X | X [1] | |
| ^ | TCBn | | X | X [1] | |
| ^ | ADCn window | | X | X [1] | |
| ^ | ACn | | X | X [1] | |
| ^ | All other interrupts | | X | | |

[1] Only when the peripheral's RUNSTBY bit is set.
[2] The CCL can select between several clock sources.
[3] PIT only.
[4] CCL can wake up the device if no internal clock source is required.
```

**Wake-up time** is six CLK_PER cycles plus the main clock source's
start-up time: idle 6 CLK (the main source keeps running); standby
6 CLK plus the oscillator start-up, if the main clock was stopped
(depends on the peripheral configuration); power-down 6 CLK plus the
oscillator start-up (only OSCULP32K and the RTC clock may still run, if
the BOD or WDT uses them). Source start-up times are under
[CLKCTRL](#clkctrl-clock-controller). With FUSE.BODCFG.ACTIVE = 0x3 the
part also waits until the BOD is ready before running code, extending
the wake-up time when the BOD is slower.

**Debug:** SLPCTRL runs normally while debugging; a break while asleep
wakes the part to active mode even with no interrupt pending.
Peripherals that need periodic CPU service may misbehave or lose data
while debugging is halted.

#### Registers

```table id=slpctrl-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:1 | SMODE[1:0] | R/W | 0x0 | Sleep mode entered by SLEEP when SEN = 1: 0x0 = IDLE; 0x1 = STANDBY; 0x2 = PDOWN: power-down; other reserved |
| 0 | SEN | R/W | 0 | Sleep enable: must be 1 before SLEEP for the part to enter the selected mode |
```

### RSTCTRL: reset controller

Resets the device to its initial state and records the source. Always
enabled; some sources must be enabled by fuse or software first. After
any reset, RSTFR shows the source (after a power-on reset only PORF),
and every register loaded from fuses is reloaded. Flags stay until
written 1. !{RESET} (digital input) is the external reset pin.

- **Power-on reset (POR):** on-chip, always enabled; active while V~DD~
  is below the POR threshold, rising or falling.
- **Brown-out reset (BOD):** V~DD~ below the fuse-selected BOD level.
  When the BOD is unused it is forced to its minimum level, for safe
  internal reset and chip erase.
- **Software reset:** writing SWRR.SWRE = 1 resets immediately; the
  part stays in reset until the reset sequence completes.
- **External reset:** enabled by FUSE.SYSCFG0.RSTPINCFG; requests reset
  while !{RESET} is low, and the part stays in reset until it is high.
- **Watchdog reset:** the WDT was not cleared within its time-out.
- **UPDI reset:** requested by an external programmer or debugger
  through the UPDI.

```table id=rstctrl-domains caption="Logic affected by each reset (X = yes) [1]"
| Reset | Fuses reloaded | TCD pin override available | TCD pin override settings reset | BOD configuration reset | UPDI reset | Other volatile logic reset |
|---|:-:|:-:|:-:|:-:|:-:|:-:|
| POR | X | | X | X | X | X |
| BOD | X | X | | | X | X |
| Software | X | X | | | | X |
| External | X | X | | | | X |
| Watchdog | X | X | | | | X |
| UPDI | X | X | | | | X |

[1] Unconfirmed: as printed. The table has TCD columns, but this device has no TCD (timer/counter type D) in its peripheral address map.
```

**Reset time:** the time the sources are active (!{RESET} low, V~DD~
below the POR or BOD threshold), then an internal initialization that
is lengthened by the start-up time fuse (FUSE.SYSCFG1.SUT) and, if
FUSE.SYSCFG0.CRCSRC runs the CRCSCAN at start-up, by the scan. RSTCTRL
runs in all modes. SWRR is configuration change protected (key IOREG).

#### Registers

```table id=rstctrl-rstfr caption="RSTFR (+0x00), reset 0xXX"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5 | UPDIRF | R/W | x | UPDI reset occurred |
| 4 | SWRF | R/W | x | Software reset occurred |
| 3 | WDRF | R/W | x | Watchdog reset occurred |
| 2 | EXTRF | R/W | x | External reset occurred |
| 1 | BORF | R/W | x | Brown-out reset occurred |
| 0 | PORF | R/W | x | Power-on reset occurred. Cleared only by writing 1. After a POR only PORF is set and every other flag cleared; no other flag can be set before a full system boot has run after the POR. |
```

All RSTFR flags clear by writing 1; all but PORF are also cleared by a
power-on reset.

```table id=rstctrl-swrr caption="SWRR (+0x01), reset 0x00, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | SWRE | R/W | 0 | Software reset enable: writing 1 resets the device. Always reads 0. |
```

#### Electrical characteristics

```table id=rst-char caption="External reset characteristics"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~IH_RST~ | Input high voltage for !{RESET} | | 0.7 × V~DD~ | | V~DD~ + 0.2 | V |
| V~IL_RST~ | Input low voltage for !{RESET} | | -0.2 | | 0.3 × V~DD~ | V |
| t~MIN_RST~ | Minimum pulse width on !{RESET} [1] | | | | 2.5 | µs |
| R~p_RST~ | !{RESET} pull-up resistor | V~Reset~ = 0 V | 20 | 35 | 50 | kΩ |

[1] Design guidance only, not production tested. Unconfirmed: the source prints this minimum pulse width in the Max column; kept there.
```

### CPUINT: CPU interrupt controller

Each peripheral interrupt is enabled in the peripheral's INTCTRL register;
interrupts are enabled globally by SREG.I = 1 (not cleared when an
interrupt is acknowledged). Interrupt flags are not cleared automatically
when the handler runs; each INTFLAGS register says how its flags are
cleared. When a request is acknowledged the PC is pushed and set to the
interrupt vector (normally a JMP to the handler); one instruction is
always executed before any pending interrupt is served. STATUS tracks the
level being executed so that RETI returns CPUINT to the state it had
before the interrupt; STATUS is not saved automatically. If no interrupt is
ever enabled, the vector locations may hold ordinary code.

Initialization order: optionally set IVSEL, CVT and LVL0RR in CTRLA and the
level 1 vector in LVL1VEC; configure and enable the interrupt in the
peripheral; then set SREG.I.

**Response time.** At least three CPU cycles to the vector: one to finish
the current instruction, two to push the PC, then three for the vector's
JMP. A multicycle instruction is completed before the interrupt is served.
From sleep, the response is five cycles longer, plus the start-up time of
the sleep mode. RETI takes four to five cycles (PC size), popping the PC
and incrementing SP.

**Priority.** Three levels: non-maskable (NMI, highest: device-fixed
sources, must still be enabled in the peripheral; executed regardless of
SREG.I and never changes it; nothing interrupts an NMI handler; several
NMIs are ordered by vector address, lowest first), level 1 (high: one
vector, chosen by writing its vector number to LVL1VEC; interrupts level 0
handlers, which resume afterwards), and level 0 (normal: all other
vectors). Vectors are numbered IVEC0 (reset), IVEC1 (NMI), … IVECn as in
the [interrupt vector table](#memory-map); reset, NMI and the level 1 vector
are skipped by the level 0 scheduler.

- *Static scheduling* (LVL0RR = 0): LVL0PRI holds the vector number with
  the lowest priority; the next vector number has the highest, wrapping
  around. With LVL0PRI = 0 (reset), IVEC0 is highest and IVECn lowest; with
  LVL0PRI = Y, IVEC Y+1 is highest, IVEC Y lowest, and IVEC0 ranks below
  IVECn. Static scheduling can starve vectors.
- *Round robin* (LVL0RR = 1): hardware writes the number of each
  acknowledged level 0 vector to LVL0PRI, so it becomes the lowest
  priority next time.

**Compact vector table** (CVT = 1): only three vectors: NMI at vector
address 1, level 1 at 2, and all level 0 interrupts share vector address 3.
For applications with few interrupt sources.

IVSEL and CVT are configuration change protected (CPU.CCP key IOREG; the
protected write must follow within four instructions, otherwise the
register is unchanged); LVL0RR is not.

#### Registers

```table id=cpuint-ctrla caption="CTRLA (+0x00), reset 0x00, CCP (IVSEL, CVT)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | IVSEL | R/W | 0 | Interrupt vector select: 0 = vectors at the start of the application section of Flash, 1 = at the start of the boot section. A boot section, if defined, precedes the application section, whose start is set by FUSE.BOOTEND. |
| 5 | CVT | R/W | 0 | Compact vector table: 0 = disabled, 1 = enabled |
| 0 | LVL0RR | R/W | 0 | Round-robin priority enable: 0 = fixed level 0 priority (lowest vector address highest, offset by LVL0PRI), 1 = round robin for level 0 |
```

```table id=cpuint-status caption="STATUS (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | NMIEX | R | 0 | Non-maskable interrupt executing. Cleared by RETI from the handler. |
| 1 | LVL1EX | R | 0 | Level 1 interrupt executing, or its handler interrupted by an NMI. Cleared by RETI. |
| 0 | LVL0EX | R | 0 | Level 0 interrupt executing, or its handler interrupted by a level 1 interrupt or an NMI. Cleared by RETI. |
```

```table id=cpuint-lvl0pri caption="LVL0PRI (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | LVL0PRI[7:0] | R/W | 0x00 | Vector number with the lowest level 0 priority (see scheduling above); updated by hardware in round-robin mode |
```

```table id=cpuint-lvl1vec caption="LVL1VEC (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | LVL1VEC[7:0] | R/W | 0x00 | Vector number of the single level 1 (high priority) interrupt; 0x00 = no level 1 vector |
```

### EVSYS: event system

Direct peripheral-to-peripheral signaling without the CPU, with short,
predictable response times. Eight channels; each is driven by one
generator, selected by CHANNELn (0x00 = channel off, the reset state; not
every generator reaches every channel), and may have several users. Each
user has its own multiplexer, USERn, that picks the channel it listens to.
An event usually corresponds to one of the generator's interrupt
conditions. Pins: EVOUTx, one event output per I/O port (an event user;
pin set by PORTMUX.EVSYSROUTEA).

Setup: configure the generator (e.g. a timer's prescaler and compare
value), configure the user to act on events (e.g. ADC start on event),
then route the generator to a channel with CHANNELn and connect the user
with USERn.

**Synchronization.** Every channel has an asynchronous and a synchronous
subchannel; each user is hardwired to one of them. The asynchronous
subchannel carries the generator's signal unchanged. On the synchronous
subchannel an asynchronous generator's signal is first synchronized
(automatically), delaying it by two system clock cycles (2–3 cycles
depending on when the event occurs); such a signal must last at least one
peripheral clock cycle to pass the synchronizer. Asynchronous events can
be generated, and asynchronous users can respond, in sleep modes with no
clock running and between clock edges.

**Software events.** Writing 1 to STROBE[n] inverts channel n for one
system clock cycle; users treat it like any other event.

**Sleep and debug.** The event system works in all sleep modes, except
software events (they need the system clock). Asynchronous generators and
users work in Standby without their clock; synchronous ones need their
clock, so they work in Idle, or in Standby when their RUNSTDBY bit is set.
Unaffected by debug mode.

```table id=evsys-generators caption="Event generators"
| Generator | Event | Clock domain | Event length | Constraint for a synchronous user |
|---|---|---|---|---|
| UPDI | SYNC character | CLK_PDI | Waveform: SYNC character on the PDI RX input, synchronized to CLK_PDI | User's synchronizing clock must be fast enough to see the event |
| RTC | Overflow | CLK_RTC | Pulse, 1 CLK_RTC | None |
| ^ | Compare match | CLK_RTC | Pulse, 1 CLK_RTC | ^ |
| PIT | RTC prescaled clock | CLK_RTC | Level | |
| CCL LUT | LUT output | Asynchronous | Depends on CCL configuration | CCL clock ≤ CLK_PER, or CCL inputs stable for at least one CLK_PER period |
| AC | Comparator result | Asynchronous | Level, typically ≥ 1 µs | AC input frequency ≤ f~CLK_PER~ |
| ADC | Result ready | CLK_ADC | Pulse, 1 CLK_PER | None |
| PORT | Pin input | Asynchronous | Level, externally controlled | Input stable for longer than one CLK_PER period |
| USART | Baud clock | TXCLK | Level | None |
| SPI | Master clock | SCK | Level | None |
| TCA | Overflow; underflow in split mode; compare match 0, 1, 2 | CLK_PER | Pulse, 1 CLK_PER | None |
| TCB | CAPT interrupt flag set | CLK_PER | Pulse, 1 CLK_PER | None |
```

```table id=evsys-users-modes caption="Event user input formats"
| User | Event mode | Input format | Asynchronous |
|---|---|---|---|
| TCAn | CNT_POSEDGE, CNT_ANYEDGE | Edge | No |
| ^ | CNT_HIGHLVL, UPDOWN | Level | No |
| TCBn | Time-out check; input capture on event; frequency, pulse-width, and frequency-and-pulse-width measurement | Edge | No |
| ^ | Single-shot | Edge | Yes |
| USARTn | IrDA mode | Level | No |
| CCL LUTn input x | LUT input or clock signal | Level | Yes |
| ADCn | Start on event | Edge | Yes |
| EVOUTx | Forward event to pin | Level | Yes |
```

#### Registers

```table id=evsys-strobe caption="STROBEx (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | STROBE[7:0] | W | 0x00 | Writing 1 to bit n inverts channel n for one system clock cycle (one software event) |
```

```table id=evsys-channel caption="CHANNELn (+0x10 + n, n = 0–7), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | GENERATOR[7:0] | R/W | 0x00 | Generator routed onto the channel (table below); 0x00 = channel off |
```

```table id=evsys-genval caption="CHANNELn.GENERATOR values"
| Value | Name | Async/sync | Channels and event |
|---|---|---|---|
| 0x01 | UPDI | Sync | All: SYNC character |
| 0x06 | RTC_OVF | Async | All: RTC overflow |
| 0x07 | RTC_CMP | Async | All: RTC compare match |
| 0x08 | RTC_PIT0 | Async | Even channels: RTC clock ÷ 8192; odd channels: ÷ 512 |
| 0x09 | RTC_PIT1 | Async | Even: ÷ 4096; odd: ÷ 256 |
| 0x0A | RTC_PIT2 | Async | Even: ÷ 2048; odd: ÷ 128 |
| 0x0B | RTC_PIT3 | Async | Even: ÷ 1024; odd: ÷ 64 |
| 0x10 + n | CCL_LUTn | Async | All: LUTn output (n = 0–3) |
| 0x20 | AC0 | Async | All: AC0 output |
| 0x24 | ADC0 | Sync | All: result ready (RESRDY) |
| 0x40 + n | PORT0_PINn | Async | CH0, CH1: PORTA pin n; CH2, CH3: PORTC pin n; CH4, CH5: PORTE pin n; CH6, CH7: none |
| 0x48 + n | PORT1_PINn | Async | CH0, CH1: PORTB pin n; CH2, CH3: PORTD pin n; CH4, CH5: PORTF pin n; CH6, CH7: none |
| 0x60 + n | USARTn | Sync | All: USARTn XCK |
| 0x68 | SPI0 | Sync | All: SPI0 SCK |
| 0x80 | TCA0_OVF_LUNF | Sync | All: TCA0 overflow / low-byte underflow |
| 0x81 | TCA0_HUNF | Sync | All: TCA0 high-byte underflow |
| 0x84 | TCA0_CMP0 | Sync | All: TCA0 compare 0 |
| 0x85 | TCA0_CMP1 | Sync | All: TCA0 compare 1 |
| 0x86 | TCA0_CMP2 | Sync | All: TCA0 compare 2 |
| 0xA0 + 2n | TCBn | Sync | All: TCBn CAPT (n = 0–3) |
```

```table id=evsys-user caption="USERn (+0x20 + n, n = 0–23), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | CHANNEL[7:0] | R/W | 0x00 | 0 = no channel (off); n = connected to channel n-1 |
```

```table id=evsys-users caption="Event users (USERn index)"
| n | Name | Async/sync | User |
|---|---|---|---|
| 0–7 | CCLLUT0A, CCLLUT0B, CCLLUT1A, CCLLUT1B, CCLLUT2A, CCLLUT2B, CCLLUT3A, CCLLUT3B | Async | CCL LUTn event input A or B |
| 8 | ADC0 | Async | ADC0 trigger |
| 9–14 | EVOUTA, EVOUTB, EVOUTC, EVOUTD, EVOUTE, EVOUTF | Async | Event pin output A–F |
| 15–18 | USART0, USART1, USART2, USART3 | Sync | USARTn event input |
| 19 | TCA0 | Sync | TCA0 event input |
| 20–23 | TCB0, TCB1, TCB2, TCB3 | Both | TCBn event input |
```

### PORTMUX: port multiplexer

Enables or disables peripheral pin functions and moves them between
default and alternative pins (see [pin functions](#pin-functions) for the
signals on each pin).

#### Registers

```table id=portmux-evsysroutea caption="EVSYSROUTEA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5 | EVOUTF | R/W | 0 | 0 = DEFAULT: EVOUTF on PF2; 1 reserved |
| 4 | EVOUTE | R/W | 0 | 0 = DEFAULT: EVOUTE on PE2; 1 reserved |
| 3 | EVOUTD | R/W | 0 | 0 = DEFAULT: PD2; 1 = ALT1: PD7 |
| 2 | EVOUTC | R/W | 0 | 0 = DEFAULT: PC2; 1 = ALT1: PC7 |
| 1 | EVOUTB | R/W | 0 | 0 = DEFAULT: EVOUTB on PB2; 1 reserved |
| 0 | EVOUTA | R/W | 0 | 0 = DEFAULT: PA2; 1 = ALT1: PA7 |
```

```table id=portmux-cclroutea caption="CCLROUTEA (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3 | LUT3 | R/W | 0 | LUT3 output: 0 = DEFAULT: PF3; 1 = ALT1: PF6 |
| 2 | LUT2 | R/W | 0 | LUT2 output: 0 = DEFAULT: PD3; 1 = ALT1: PD6 |
| 1 | LUT1 | R/W | 0 | LUT1 output: 0 = DEFAULT: PC3; 1 = ALT1: PC6 |
| 0 | LUT0 | R/W | 0 | LUT0 output: 0 = DEFAULT: PA3; 1 = ALT1: PA6 |
```

```table id=portmux-usartroutea caption="USARTROUTEA (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | USART3[1:0] | R/W | 0 | 0x0 = DEFAULT: PB[3:0]; 0x1 = ALT1: PB[5:4]; 0x3 = NONE: no pins; 0x2 reserved |
| 5:4 | USART2[1:0] | R/W | 0 | 0x0 = DEFAULT: PF[3:0]; 0x1 = ALT1: PF[6:4]; 0x3 = NONE; 0x2 reserved |
| 3:2 | USART1[1:0] | R/W | 0 | 0x0 = DEFAULT: PC[3:0]; 0x1 = ALT1: PC[7:4]; 0x3 = NONE; 0x2 reserved |
| 1:0 | USART0[1:0] | R/W | 0 | 0x0 = DEFAULT: PA[3:0]; 0x1 = ALT1: PA[7:4]; 0x3 = NONE; 0x2 reserved |
```

```table id=portmux-twispiroutea caption="TWISPIROUTEA (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5:4 | TWI0[1:0] | R/W | 0 | 0x0 = DEFAULT: SCL/SDA on PA[3:2], dual-mode slave on PC[3:2]; 0x1 = ALT1: PA[3:2], slave on PF[3:2]; 0x2 = ALT2: PC[3:2], slave on PF[3:2]; 0x3 reserved |
| 1:0 | SPI0[1:0] | R/W | 0 | 0x0 = DEFAULT: PA[7:4]; 0x1 = ALT1: PC[3:0]; 0x2 = ALT2: PE[3:0]; 0x3 = NONE: no pins |
```

```table id=portmux-tcaroutea caption="TCAROUTEA (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | TCA0[2:0] | R/W | 0 | TCA0 outputs on: 0x0 = PORTA: PA[5:0]; 0x1 = PORTB: PB[5:0]; 0x2 = PORTC: PC[5:0]; 0x3 = PORTD: PD[5:0]; 0x4 = PORTE: PE[5:0]; 0x5 = PORTF: PF[5:0]; other reserved |
```

```table id=portmux-tcbroutea caption="TCBROUTEA (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3 | TCB3 | R/W | 0 | 0 = DEFAULT: PB5; 1 = ALT1: PC1 |
| 2 | TCB2 | R/W | 0 | 0 = DEFAULT: PC0; 1 = ALT1: PB4 |
| 1 | TCB1 | R/W | 0 | 0 = DEFAULT: PA3; 1 = ALT1: PF5 |
| 0 | TCB0 | R/W | 0 | 0 = DEFAULT: PA2; 1 = ALT1: PF4 |
```

### PORT and VPORT: I/O pin configuration

Each PORT instance (PORTA–PORTF; which pins exist depends on the package,
see [pin functions](#pin-functions)) controls up to eight pins Pxn. Bit n
of DIR enables pin n's output driver and bit n of OUT sets its level; the
pin can always be read through IN, whether it is an input or an output,
unless its input buffer is disabled. IN is synchronized to the main
clock; the synchronizers are not clocked for pins with ISC =
INPUT_DISABLE. The DIRSET/DIRCLR/DIRTGL and OUTSET/OUTCLR/OUTTGL registers
are hardware read-modify-write: writing 1 to bit n sets, clears or toggles
DIR[n] or OUT[n] without disturbing the other pins; writing 1 to IN[n]
also toggles OUT[n]. Pin n's configuration register is PINnCTRL at
PORT base + 0x10 + n.

After reset every pin is connected to its port, output tri-stated and
input buffer enabled, even with no clock running; pins with a special
function (the debugger pin) may differ. For lowest power, disable the
input buffer (ISC = INPUT_DISABLE) of unused pins and of pins used as
analog inputs or outputs.

PINnCTRL sets per pin: INVEN inverts both input and output (toggling INVEN
makes an edge on the pin that peripherals, interrupts and events see);
PULLUPEN enables the pull-up; ISC sets the interrupt sense. PORTCTRL.SRL
enables slew-rate limiting for the whole port.

**Peripheral override.** An enabled peripheral connected to a pin (default
or PORTMUX alternative location) overrides the PORT's control of it in a
peripheral-dependent way, possibly only partly; pins not overridden stay
general-purpose I/O.

**Interrupts.** One interrupt vector per port (PORTx); INTn in INTFLAGS is
set when pin n matches its ISC setting, and the request stays active until
the flag is cleared (write 1). Cautions when changing interrupt settings:
an edge caused by toggling INVEN in the same cycle as the interrupt
setting may not cause a request; an input disabled while an interrupt is
being synchronized may request that interrupt when re-enabled, even with
a different setting; an interrupt being synchronized while its setting
changes may be lost.

Only pins Px2 and Px6 of each port have full asynchronous sensing (see
[pin functions](#pin-functions)); the others are synchronous or
partly asynchronous:

```table id=port-async caption="Fully and partly asynchronous sense pins"
| Property | Synchronous or partly asynchronous pins | Fully asynchronous pins |
|---|---|---|
| Minimum pulse width to trigger an interrupt | One system clock cycle | Less than one system clock cycle |
| Wake from sleep, main clock running | All sense configurations | All sense configurations |
| Wake from sleep, main clock stopped | BOTHEDGES or LEVEL only (RISING, FALLING do not wake) | All sense configurations |
| Interrupt dead time | No new interrupt for three cycles after the previous one | Less than one system clock cycle |
| Minimum wake-up pulse length | Level must be held until the system clock has restarted | Less than one system clock cycle |
```

**Events.** Every pin is an asynchronous event generator: its event output
is the pin value when the input buffer is enabled, 0 when it is disabled.
PORT has no event inputs.

**Sleep.** All pin configuration is independent of sleep mode, except
interrupts and input synchronization: PORT always runs on the main clock,
and input synchronization stops when that clock stops.

**Virtual ports.** VPORTA–VPORTF map DIR, OUT, IN and INTFLAGS of each
port into the bit-addressable I/O space, so SBI, CBI, SBIS and SBIC work
on them; a VPORT write has the same effect as a write to the PORT
register.

#### Registers (PORTx)

```table id=port-dir caption="DIR (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DIR[7:0] | R/W | 0x00 | 1 = pin n is an output; 0 = input only (configured by PINnCTRL.ISC). Controls only the output enable: the input stays enabled. |
```

```table id=port-dirset caption="DIRSET (+0x01), DIRCLR (+0x02), DIRTGL (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DIRSET[7:0], DIRCLR[7:0], DIRTGL[7:0] | R/W | 0x00 | Writing 1 to bit n sets, clears or toggles DIR[n]; writing 0 has no effect. Reads return DIR. |
```

```table id=port-out caption="OUT (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | OUT[7:0] | R/W | 0x00 | Output value: 1 = pin n driven high, 0 = low; effective only when the pin is an output |
```

```table id=port-outset caption="OUTSET (+0x05), OUTCLR (+0x06), OUTTGL (+0x07), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | OUTSET[7:0], OUTCLR[7:0], OUTTGL[7:0] | R/W | 0x00 | Writing 1 to bit n sets, clears or toggles OUT[n]; writing 0 has no effect. Reads return OUT. |
```

```table id=port-in caption="IN (+0x08), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | IN[7:0] | R/W | 0x00 | Pin values when the digital input buffer is enabled (not sampled, cannot be read, when disabled). Writing 1 to bit n toggles OUT[n]. |
```

```table id=port-intflags caption="INTFLAGS (+0x09), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | INT[7:0] | R/W | 0x00 | Set when pin n's change or state matches its ISC setting. Write 1 to clear. |
```

```table id=port-portctrl caption="PORTCTRL (+0x0A), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | SRL | R/W | 0 | Slew rate limit enable for all pins of the port |
```

```table id=port-pinctrl caption="PINnCTRL (+0x10 + n, n = 0–7), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | INVEN | R/W | 0 | Inverted I/O: 0 = input and output not inverted, 1 = inverted |
| 3 | PULLUPEN | R/W | 0 | Pull-up: 0 = disabled, 1 = enabled |
| 2:0 | ISC[2:0] | R/W | 0x0 | Input/sense configuration: 0x0 = INTDISABLE: interrupt disabled, input buffer enabled; 0x1 = BOTHEDGES; 0x2 = RISING; 0x3 = FALLING; 0x4 = INPUT_DISABLE: interrupt and digital input buffer disabled (IN cannot be read); 0x5 = LEVEL: sense low level; other reserved |
```

#### Registers (VPORTx)

```table id=vport-regs caption="VPORTx: DIR (+0x00), OUT (+0x01), IN (+0x02), INTFLAGS (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DIR[7:0] | R/W | 0x00 | Same as PORTx.DIR |
| 7:0 | OUT[7:0] | R/W | 0x00 | Same as PORTx.OUT |
| 7:0 | IN[7:0] | R/W | 0x00 | Same as PORTx.IN; writing 1 to bit n toggles OUT[n] |
| 7:0 | INT[7:0] | R [1] | 0x00 | Same as PORTx.INTFLAGS: set when pin n matches its ISC setting and is a port interrupt source; write 1 to clear |

[1] Source conflict: the PDF gives INTFLAGS access as R, yet says writing 1 clears a flag.
```

### BOD: brown-out detector

The BOD compares V~DD~ with a programmable brown-out threshold and
holds the device in reset below it. A voltage level monitor (VLM)
compares V~DD~ with a second, higher threshold (5 %, 15 % or 25 % above
the BOD level) and can raise an interrupt as an early warning. Both
compare against the bandgap. The BOD runs in one of three modes:
enabled (continuously active), sampled (switched on briefly at 1 kHz
or 125 Hz to check V~DD~) or disabled.

The BOD is set mainly by fuses (FUSE.BODCFG), loaded at reset: the
level (LVL), the sample frequency and the mode in active and idle
(ACTIVE) cannot be changed by the CPU. The mode in standby and
power-down (SLEEP) is loaded from the fuse and can be changed by
software (CCP, IOREG key). Entering standby or power-down, the BOD
switches to the SLEEP mode; on wake-up it returns to the ACTIVE mode.

The VLM follows the BOD mode: with the BOD off, the VLM is off even if
VLMIE = 1; with the BOD sampled, the VLM is sampled too. Enable it with
VLMIE, choose the crossing direction with VLMCFG and the level with
VLMLVL. When the VLM interrupt is enabled, VLMIF is always set if
VLMCFG = 0x2, and may be set if VLMCFG = 0x0 or 0x1. Interrupt vector:
VLM (supply crossing the VLM threshold as set by VLMCFG). The VLM
interrupt is not executed while the CPU is halted in debug mode.

#### Registers

```table id=bod-ctrla caption="CTRLA (+0x00), reset from FUSE.BODCFG, CCP (SLEEP, SAMPFREQ) [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4 | SAMPFREQ | R | x | Sample frequency in sampled mode: 0 = 1 kHz, 1 = 125 Hz. Loaded from FUSE.BODCFG.SAMPFREQ. |
| 3:2 | ACTIVE[1:0] | R | x | Mode in active and idle: 0x0 = disabled; 0x1 = enabled; 0x2 = sampled; 0x3 = enabled, with wake-up halted until the BOD is ready. Loaded from FUSE.BODCFG.ACTIVE. |
| 1:0 | SLEEP[1:0] | R/W | x | Mode in standby and power-down: 0x0 = disabled; 0x1 = enabled; 0x2 = sampled; 0x3 reserved. Loaded from FUSE.BODCFG.SLEEP. |

[1] Source conflict: SAMPFREQ's access is given as R, yet its description says the bit is under Configuration Change Protection.
```

```table id=bod-ctrlb caption="CTRLB (+0x01), reset from FUSE.BODCFG"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | LVL[2:0] | R | x | BOD threshold (typical; see BOD and POR characteristics): 0x0 = BODLEVEL0: 1.8 V; 0x2 = BODLEVEL2: 2.6 V; 0x7 = BODLEVEL7: 4.3 V. Loaded from FUSE.BODCFG.LVL. |
```

```table id=bod-vlmctrla caption="VLMCTRLA (+0x08), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1:0 | VLMLVL[1:0] | R/W | 0 | VLM threshold relative to the BOD threshold: 0x0 = 5 % above; 0x1 = 15 % above; 0x2 = 25 % above; 0x3 reserved |
```

```table id=bod-intctrl caption="INTCTRL (+0x09), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:1 | VLMCFG[1:0] | R/W | 0 | VLM interrupt trigger: 0x0 = V~DD~ crosses the VLM threshold from above; 0x1 = from below; 0x2 = either direction; 0x3 reserved |
| 0 | VLMIE | R/W | 0 | 1 = VLM interrupt enabled |
```

```table id=bod-intflags caption="INTFLAGS (+0x0A), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | VLMIF | R/W | 0 | Set on a VLM trigger as configured by VLMCFG; only updated while the BOD is enabled. Write 1 to clear. |
```

```table id=bod-status caption="STATUS (+0x0B), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | VLMS | R | 0 | 0 = V~DD~ above the VLM threshold, 1 = below. Valid only while the BOD is enabled. |
```

#### Electrical characteristics

```table id=bod-char caption="Power supply, power-on reset and brown-out detector"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| SR~ON~ | Power-on slope [1][2] | | | | 100 | V/ms |
| V~POR~ | POR threshold, V~DD~ falling [1] | V~DD~ falls or rises at 0.5 V/ms or slower | 0.8 | | 1.6 | V |
| ^ | POR threshold, V~DD~ rising | ^ | 1.4 [1] | | 1.8 | V |
| V~BOD~ | BOD detection level (falling and rising) | BODLEVEL0 | 1.7 | 1.8 | 2.0 | V |
| ^ | ^ | BODLEVEL2 | 2.4 | 2.6 | 2.9 | V |
| ^ | ^ | BODLEVEL7 | 3.9 | 4.3 | 4.5 | V |
| V~HYS~ | Hysteresis | BODLEVEL0 | | 25 | | mV |
| ^ | ^ | BODLEVEL2 | | 40 | | mV |
| ^ | ^ | BODLEVEL7 | | 80 | | mV |
| t~BOD~ | Detection time | Continuous | | 7 | | µs |
| ^ | ^ | Sampled, 1 kHz | | 1 | | ms |
| ^ | ^ | Sampled, 125 Hz | | 8 | | ms |
| t~startup~ | Start-up time | From enable to ready | | 40 | | µs |
| V~INT~ | Voltage level monitor interrupt level, above the selected BOD level | Level 0 | | 4 | | % |
| ^ | ^ | Level 1 | | 13 | | % |
| ^ | ^ | Level 2 | | 25 | | % |

[1] Design guidance only, not production tested.
[2] A faster slope can reset the device if the voltage changes after the initial power-up.
```

### VREF: voltage reference

VREF selects the internal reference levels, generated from the
bandgap, for ADC0 and for AC0 (its DACREF): 0.55 V, 1.1 V, 1.5 V,
2.5 V, 4.3 V, and for AC0 also AV~DD~. Select the level (CTRLA) before
enabling the ADC or AC; the reference buffer and the bandgap are then
enabled automatically when the peripheral requests them. Changing the
reference while the module is enabled can lead to unpredictable
behavior. The force-enable bits in CTRLB keep a reference on whether
requested or not, removing its start-up time at the cost of power.

#### Registers

```table id=vref-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | ADC0REFSEL[2:0] | R/W | 0 | ADC0 internal reference: 0x0 = 0V55: 0.55 V; 0x1 = 1V1: 1.1 V; 0x2 = 2V5: 2.5 V; 0x3 = 4V3: 4.3 V; 0x4 = 1V5: 1.5 V; other reserved |
| 2:0 | AC0REFSEL[2:0] | R/W | 0 | AC0 (DACREF) reference: 0x0 = 0V55: 0.55 V; 0x1 = 1V1: 1.1 V; 0x2 = 2V5: 2.5 V; 0x3 = 4V3: 4.3 V; 0x4 = 1V5: 1.5 V; 0x5, 0x6 reserved; 0x7 = AVDD: AV~DD~ |
```

```table id=vref-ctrlb caption="CTRLB (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | ADC0REFEN | R/W | 0 | 1 = ADC0 reference forced on even when not requested; 0 = enabled and disabled automatically on request |
| 0 | AC0REFEN | R/W | 0 | 1 = AC0 DACREF reference forced on even when not requested; 0 = automatic |
```

#### Electrical characteristics

```table id=vref-char caption="Internal voltage reference [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| t~start~ | Start-up time | | | 25 | | µs |
| V~DD~ | Supply voltage range | 0V55, 1V1, 1V5 | 1.8 | | 5.5 | V |
| ^ | ^ | 2V5 | 3.0 | | 5.5 | V |
| ^ | ^ | 4V3 | 4.8 | | 5.5 | V |

[1] Design guidance only, not production tested.
```

The 0V55 … 4V3 names are the ADC0REFSEL and AC0REFSEL values in
VREF.CTRLA.

```table id=vref-acc caption="Internal reference voltage accuracy, V~DD~ = 1.8 V to 5.5 V [1]"
| Parameter | Reference | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| ADC reference voltage error | 1V1 | T = 0 °C to 105 °C | -2.0 | 2.0 | % |
| ^ | 0V55, 1V5, 2V5, 4V3 | T = 0 °C to 105 °C | -3.0 | 3.0 | % |
| ^ | 0V55, 1V1, 1V5, 2V5, 4V3 | T = -40 °C to 125 °C | -5.0 | 5.0 | % |
| AC reference voltage error | 0V55, 1V1, 1V5, 2V5 | T = 0 °C to 105 °C | -3.0 | 3.0 | % |
| ^ | 0V55, 1V1, 1V5, 2V5, 4V3 | T = -40 °C to 125 °C | -5.0 | 5.0 | % |

[1] Based on characterization, not covered by production test limits.
```

### WDT: watchdog timer

The WDT resets the system unless the WDR instruction restarts it
within the time-out period. It is clocked by CLK_WDT_OSC, the 1 kHz
output of OSCULP32K, asynchronous to the system clock, so it keeps
running in every sleep mode where that clock runs and resets the
device even if the main clock fails. OSCULP32K is not accurate: the
actual time-out varies from device to device, so choose periods that
are valid for all devices.

**Enabling.** A non-zero PERIOD in CTRLA enables the WDT; a non-zero
WINDOW also enables window mode. CTRLA's reset value comes from
FUSE.WDTCFG, so the WDT can be on from boot; if PERIOD is non-zero
after the boot code, LOCK is set automatically.

**Normal mode** (WINDOW = 0x0): one time-out period T~OWDT~ of 8 ms to
8 s (11 settings). Each WDR starts a new period; no WDR before it ends
causes a system reset.

**Window mode** (WINDOW ≠ 0x0): a closed period T~OWDTW~ (WINDOW, 8 ms
to 8 s) during which a WDR causes a system reset, followed by an open
period (PERIOD, 8 ms to 8 s) during which the WDR must come; the total
time-out is the sum of both. After window mode is enabled, or after
leaving debug mode, the first closed period starts at the first WDR.

**Protection.** CTRLA and STATUS.LOCK are under CCP (IOREG key: write
the key to CPU.CCP, then the register within four instructions;
otherwise the write is ignored). LOCK = 1 makes CTRLA read-only, so
the WDT cannot be disabled by software; LOCK can only be written 1, and
cleared only in debug mode.

**Synchronization.** Writes to CTRLA (PERIOD, WINDOW) are synchronized
into the WDT clock domain; STATUS.SYNCBUSY is 1 meanwhile, and CTRLA
must not be written while SYNCBUSY = 1. A WDR takes two to three WDT
clock cycles to synchronize; a WDR issued while one is being
synchronized is ignored.

**Debug.** The WDT runs normally during run-time debugging. Halting
the CPU in debug mode halts the WDT and resets its counter; when the
CPU restarts in window mode, the first closed period is skipped and a
normal-mode time-out runs instead.

#### Registers

```table id=wdt-ctrla caption="CTRLA (+0x00), reset from FUSE.WDTCFG, CCP"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | WINDOW[3:0] | R/W | x | Closed window period ([wdt-periods](#wdt-periods)); 0x0 = OFF: normal mode, non-zero = window mode. Read-only while STATUS.LOCK = 1. |
| 3:0 | PERIOD[3:0] | R/W | x | Time-out period in normal mode, open window period in window mode ([wdt-periods](#wdt-periods)); 0x0 = OFF: WDT disabled, non-zero enables it. Read-only while STATUS.LOCK = 1. |
```

```table id=wdt-periods caption="WINDOW and PERIOD values [1]"
| Value | Name | Period |
|---|---|---|
| 0x0 | OFF | — |
| 0x1 | 8CLK | 0.008 s |
| 0x2 | 16CLK | 0.016 s |
| 0x3 | 32CLK | 0.032 s |
| 0x4 | 64CLK | 0.064 s |
| 0x5 | 128CLK | 0.128 s |
| 0x6 | 256CLK | 0.256 s |
| 0x7 | 512CLK | 0.512 s |
| 0x8 | 1KCLK | 1.024 s |
| 0x9 | 2KCLK | 2.048 s |
| 0xA | 4KCLK | 4.096 s |
| 0xB | 8KCLK | 8.192 s |
| 0xC–0xF | — | Reserved |

[1] Nominal, at 1 kHz CLK_WDT_OSC. Values as in the WINDOW table. Source conflict: the PERIOD table gives 0x8–0xB as 1.0 s, 2.0 s, 4.1 s and 8.2 s.
```

```table id=wdt-status caption="STATUS (+0x01), reset 0x00, CCP (LOCK only)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | LOCK | R/W | 0 | 1 = CTRLA write-protected. Can only be written 1; cleared only in debug mode. Set automatically when PERIOD ≠ 0 after the boot code. |
| 0 | SYNCBUSY | R | 0 | 1 while a CTRLA write is being synchronized into the WDT clock domain; cleared by hardware. Not under CCP. |
```

### TCA: 16-bit timer/counter type A

One instance, TCA0. A 16-bit base counter (CNT) with period (PER) and
three compare channels (CMP0–CMP2), each with a buffer register
(PERBUF, CMPnBUF); or, in split mode, two 8-bit timers with three
compare channels each. The counter counts CLK_PER through a prescaler
(÷1 to ÷1024, CLKSEL) or counts events, or events gate it or set its
direction. Each compare channel drives a waveform output WOn (WO0–WO2;
WO3–WO5 in split mode) whose pins PORTMUX.TCAROUTEA selects.

Block diagram: base counter (CNT, PER, PERBUF with its BV flag) and
control logic fed by CTRLA, CTRLB and EVCTRL produce OVF and UPDATE
from the TOP and BOTTOM comparisons; each compare unit (CMPn, CMPnBUF
with BV) compares to CNT and drives a waveform generator and the CMPn
interrupt and event.

Definitions: **BOTTOM** is CNT = 0x0000; **MAX** is all ones; **TOP**
is the highest value of the count sequence; **UPDATE** is met at BOTTOM
or TOP, depending on the waveform generation mode, and copies valid
buffer values into PER and CMPn unless CTRLE.LUPD is set. "Timer"
means counting periodic clock ticks, "counter" counting sporadic ticks
such as events.

**Initialization.** Write TOP to PER; set CTRLA.ENABLE (the counter
counts clock ticks at the CLKSEL prescaler setting); optionally set
EVCTRL.CNTEI to count events instead; read the count from CNT.

**Normal operation.** The counter counts in the direction of
CTRLE.DIR until TOP or BOTTOM. Counting up, it wraps to 0 at the clock
tick after TOP; counting down, it is reloaded with PER at BOTTOM.
Writing CNT while running takes effect at once and has priority over
count, clear and reload; DIR may also be changed while running.

**Double buffering.** PER and CMPn are double-buffered by PERBUF and
CMPnBUF. Each buffer has a buffer-valid flag (CTRLF.PERBV, CMPnBV), set
when the buffer is written and cleared at UPDATE, when the buffer is
copied. PER and CMPn are also directly writable, which bypasses the
buffer (for initialization). Unbuffered, a period change is immediate:
since CNT and PER are compared continuously, writing a TOP below the
current CNT while counting up (in any mode) makes the counter run to
MAX and wrap before the next compare match. Buffered, the buffer may be
written at any time; PER changes only at UPDATE, which prevents
wraparound and odd waveforms.

**Compare channels.** Channel n compares CNT with CMPn continuously; a
match sets INTFLAGS.CMPn at the next timer clock cycle (and the
optional interrupt). Buffered updates of CMPn happen at TOP or BOTTOM
(the UPDATE condition), giving glitch-free, symmetrical pulses.

#### Waveform generation

For a waveform on the pin: select a mode in CTRLB.WGMODE; count clock
ticks, not events (EVCTRL.CNTEI = 0); set CTRLB.CMPnEN (which overrides
the pin's PORT output value; PORTMUX can pick an alternative pin); set
the pin as output (PORTx.DIR[n] = 1); optionally invert it with
PORTx.PINnCTRL.INVEN. N below is the prescaler division (CLKSEL).

```table id=tca-wgmode caption="Waveform generation modes (CTRLB.WGMODE)"
| Value | Name | Mode | TOP | UPDATE | OVF |
|---|---|---|---|---|---|
| 0x0 | NORMAL | Normal (no waveform) | PER | TOP [1] | TOP [1] |
| 0x1 | FRQ | Frequency | CMP0 | TOP [1] | TOP [1] |
| 0x3 | SINGLESLOPE | Single-slope PWM | PER | BOTTOM | BOTTOM |
| 0x5 | DSTOP | Dual-slope PWM | PER | BOTTOM | TOP |
| 0x6 | DSBOTH | Dual-slope PWM | PER | BOTTOM | TOP and BOTTOM |
| 0x7 | DSBOTTOM | Dual-slope PWM | PER | BOTTOM | BOTTOM |

[1] When counting up. Values 0x2 and 0x4 are reserved.
```

**Frequency (FRQ).** CMP0 sets the period instead of PER; the output
toggles on each compare match between CNT and CMPn. The highest
frequency, f~CLK_PER~/2, is reached with CMP0 = 0x0000 and N = 1.

```equation
f~FRQ~ = f~CLK_PER~ / (2 × N × (CMP0 + 1))
```

**Single-slope PWM.** PER sets the period, CMPn the duty cycle. The
counter counts BOTTOM to TOP and restarts at BOTTOM; WOn is set at
BOTTOM and cleared on the CNT = CMPn match. CMPn = BOTTOM gives a static
low output, CMPn > TOP a static high output. Resolution 2 bits
(PER = 0x0002) to 16 bits (PER = MAX − 1).

```equation
R~PWM_SS~ = log(PER + 2) / log(2)
f~PWM_SS~ = f~CLK_PER~ / (N × (PER + 1))
```

**Dual-slope PWM.** PER sets the period, CMPn the duty cycle. The
counter counts BOTTOM to TOP, then TOP to BOTTOM; WOn is set at BOTTOM,
cleared on compare match while counting up and set on compare match
while counting down. CMPn = BOTTOM gives a static low output, CMPn =
TOP a static high output. The maximum frequency is half that of
single-slope PWM. Resolution 2 bits (PER = 0x0003) to 16 bits
(PER = MAX).

```equation
R~PWM_DS~ = log(PER + 1) / log(2)
f~PWM_DS~ = f~CLK_PER~ / (2 × N × PER)
```

**Port override.** With CMPnEN = 1 and a waveform mode selected, the
waveform replaces the pin's OUT value (pin direction must be output);
PORTx.PINnCTRL.INVEN inverts it.

#### Commands

Software commands, written to CTRLESET.CMD, act at once. UPDATE does
what an UPDATE condition does, but ignores LUPD. RESTART restarts the
current waveform period: the counter, direction and all compare outputs
are set to 0. RESET returns all TCA registers to their initial values;
it is only accepted while the timer is stopped (CTRLA.ENABLE = 0).

#### Split mode

CTRLD.SPLITM = 1 splits TCA into two 8-bit timers (low byte: LCNT,
LPER, LCMP0–2 driving WO0–WO2; high byte: HCNT, HPER, HCMP0–2 driving
WO3–WO5), each with three compare channels for PWM. Split mode counts
down only, LCNT and HCNT are independent, and the only waveform mode is
single-slope PWM (WGMODE = SINGLESLOPE). The low timer keeps the
normal interrupts; the high timer has an underflow interrupt (HUNF) but
no compare interrupt or flag. Event actions are not supported (event
inputs unused), buffer registers and BV flags are unused, and all
registers are accessed as bytes. Switching between normal and split
mode changes what registers and bits mean but not their values, so
disable the TCA (ENABLE = 0) and issue a RESET command (CMD = RESET,
CMDEN = BOTH) before changing mode. Then: set SPLITM, write the TOP
values to LPER and HPER, set ENABLE; read the counts from LCNT and HCNT.

#### Events, interrupts, sleep, debug

```table id=tca-events caption="TCA0 event generators (pulse, CLK_PER domain, one CLK_PER period long) [1]"
| Generator | Normal mode | Split mode |
|---|---|---|
| OVF_LUNF | Overflow | Low byte timer underflow |
| HUNF | — | High byte timer underflow |
| CMP0 | Compare channel 0 match | Low byte timer compare channel 0 match |
| CMP1 | Compare channel 1 match | Low byte timer compare channel 1 match |
| CMP2 | Compare channel 2 match | Low byte timer compare channel 2 match |

[1] An event is generated under the same conditions that set the matching INTFLAGS flag.
```

Event user TCA0 (synchronous; normal mode only), enabled by
EVCTRL.CNTEI, action set by EVACT: count on positive edge (edge
detection), count on any edge (edge), count prescaled clock cycles
while the event is high (level), or count prescaled clock cycles with
the event level setting the direction, up when low and down when high
(level).

Interrupt vectors, normal mode: OVF (counter reached TOP or BOTTOM),
CMP0, CMP1, CMP2 (counter matches CMPn). Split mode: LUNF (low byte
timer reached BOTTOM), HUNF (high byte timer reached BOTTOM), LCMP0–2
(low byte counter matches LCMPn). A request is raised while the flag in
INTFLAGS and its enable in INTCTRL are both set, until the flag is
cleared by writing 1 to it.

TCA keeps running in idle sleep mode (not in standby or power-down).
In break debug mode it halts and ignores events unless DBGCTRL.DBGRUN
= 1.

The 16-bit registers are read and written through the shared TEMP
register (see the CPU's 16-bit access rule): one TEMP for all 16-bit
registers of TCA. Each 16-bit register's low byte (suffix L) is at its
offset, the high byte (suffix H) at offset + 1. CPU and UPDI writes to
CNT have priority over internal updates.

#### Registers, normal mode

```table id=tca-ctrla caption="CTRLA (+0x00), reset 0x00 (both modes)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:1 | CLKSEL[2:0] | R/W | 0 | Clock f~TCA~ = f~CLK_PER~ / N: 0x0 = DIV1, 0x1 = DIV2, 0x2 = DIV4, 0x3 = DIV8, 0x4 = DIV16, 0x5 = DIV64, 0x6 = DIV256, 0x7 = DIV1024 |
| 0 | ENABLE | R/W | 0 | 1 = TCA enabled |
```

```table id=tca-ctrlb caption="CTRLB (+0x01), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | CMP2EN, CMP1EN, CMP0EN | R/W | 0 | Compare n enable: in FRQ and PWM modes, 1 = WOn overrides the pin's PORT output value (pin must be an output) |
| 3 | ALUPD | R/W | 0 | Auto-lock update: 0 = LUPD is left alone; 1 = hardware sets LUPD until the CMPnBV bits of all enabled channels are 1, then clears it; at the next UPDATE the buffers are copied and LUPD is set again. So no CMPnBUF is copied until all enabled buffers are written. |
| 2:0 | WGMODE[2:0] | R/W | 0 | Waveform generation mode: counting sequence, TOP, UPDATE, OVF and waveform, see the [modes table](#tca-wgmode). NORMAL makes no waveform; in other modes the output reaches the pin only with CMPnEN set and the pin an output. |
```

```table id=tca-ctrlc caption="CTRLC (+0x02), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | CMP2OV, CMP1OV, CMP0OV | R/W | 0 | Compare output value: direct access to the waveform generator output n while the TCA is not enabled, to set or clear WOn while stopped |
```

```table id=tca-ctrld caption="CTRLD (+0x03), reset 0x00 (both modes)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | SPLITM | R/W | 0 | 1 = split mode: two 8-bit timers; the register map changes |
```

CTRLECLR (+0x04) and CTRLESET (+0x05) are two views of CTRLE: writing
1 to a bit in CTRLECLR clears it, in CTRLESET sets it (no
read-modify-write needed); both read the current value. CTRLFCLR
(+0x06) and CTRLFSET (+0x07) do the same for CTRLF.

```table id=tca-ctrle caption="CTRLECLR (+0x04), CTRLESET (+0x05), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:2 | CMD[1:0] | R/W | 0 | Command (written to CTRLESET; always reads 0): 0x0 = NONE; 0x1 = UPDATE: force update; 0x2 = RESTART: force restart; 0x3 = RESET: force hard reset (ignored while enabled) |
| 1 | LUPD | R/W | 0 | Lock update: 0 = buffered registers are copied at each UPDATE condition; 1 = no copy at UPDATE (ensures all buffers are valid first) |
| 0 | DIR | R/W | 0 | Counter direction: 0 = up, 1 = down. Normally set by the waveform mode or event action; software may change it. |
```

```table id=tca-ctrlf caption="CTRLFCLR (+0x06), CTRLFSET (+0x07), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:1 | CMP2BV, CMP1BV, CMP0BV | R/W | 0 | Compare n buffer valid: set when CMPnBUF is written; cleared automatically at UPDATE |
| 0 | PERBV | R/W | 0 | Period buffer valid: set when PERBUF is written; cleared automatically at UPDATE |
```

```table id=tca-evctrl caption="EVCTRL (+0x09), reset 0x00, normal mode only"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:1 | EVACT[2:0] | R/W | 0 | Event action: 0x0 = EVACT_POSEDGE: count on positive event edge; 0x1 = EVACT_ANYEDGE: count on any event edge; 0x2 = EVACT_HIGHLVL: count prescaled clock cycles while the event is high; 0x3 = EVACT_UPDOWN: count prescaled clock cycles, event low = up, high = down; other reserved |
| 0 | CNTEI | R/W | 0 | 1 = count on event input, as EVACT selects |
```

```table id=tca-intctrl caption="INTCTRL (+0x0A), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | CMP2, CMP1, CMP0 | R/W | 0 | 1 = compare channel n interrupt enabled |
| 0 | OVF | R/W | 0 | 1 = overflow/underflow interrupt enabled |
```

```table id=tca-intflags caption="INTFLAGS (+0x0B), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | CMP2, CMP1, CMP0 | R/W | 0 | Compare channel n flag: set on a CNT = CMPn match, in every mode. Not cleared automatically; write 1 to clear. |
| 0 | OVF | R/W | 0 | Overflow/underflow flag: set at TOP (overflow) or BOTTOM (underflow), depending on WGMODE. Not cleared automatically; write 1 to clear. |
```

```table id=tca-dbgctrl caption="DBGCTRL (+0x0E), reset 0x00 (both modes)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | Run in debug: 0 = halted in break debug mode, events ignored; 1 = keeps running while the CPU is halted |
```

```table id=tca-temp caption="TEMP (+0x0F), reset 0x00, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | TEMP[7:0] | R/W | 0 | Temporary byte the CPU uses for single-cycle 16-bit access to CNT, PER, CMPn, PERBUF, CMPnBUF; readable and writable by software |
```

```table id=tca-cnt caption="CNT (+0x20, 16-bit: CNTL, CNTH), reset 0x0000, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CNT[15:0] | R/W | 0x0000 | Counter value. CPU and UPDI writes have priority over internal updates. |
```

```table id=tca-per caption="PER (+0x26, 16-bit: PERL, PERH), reset 0xFFFF, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | PER[15:0] | R/W | 0xFFFF | TOP value in every mode except FRQ |
```

```table id=tca-cmp caption="CMPn (+0x28 + 2n, n = 0…2, 16-bit: CMPnL, CMPnH), reset 0x0000, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CMP[15:0] | R/W | 0x0000 | Compare value, compared continuously to CNT (normally to generate waveforms). Loaded from CMPnBUF at UPDATE. |
```

```table id=tca-perbuf caption="PERBUF (+0x36, 16-bit: PERBUFL, PERBUFH), reset 0xFFFF, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | PERBUF[15:0] | R/W | 0xFFFF | Period buffer; a CPU or UPDI write sets CTRLF.PERBV. The register summary also lists PERBUFH (+0x37, reset 0xFF) separately: it is PERBUF[15:8]. |
```

```table id=tca-cmpbuf caption="CMPnBUF (+0x38 + 2n, n = 0…2, 16-bit: CMPnBUFL, CMPnBUFH), reset 0x0000, normal mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CMPBUF[15:0] | R/W | 0x0000 | Compare n buffer; a CPU or UPDI write sets CTRLF.CMPnBV |
```

#### Registers, split mode

CTRLA, CTRLD and DBGCTRL are as in normal mode. EVCTRL, TEMP, CTRLF
and the buffer registers are not used. All registers are bytes.

```table id=tca-s-ctrlb caption="CTRLB (+0x01), reset 0x00, split mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | HCMP2EN, HCMP1EN, HCMP0EN | R/W | 0 | High byte compare n enable: 1 = in PWM mode, WO[n+3] overrides the pin's PORT output value |
| 2:0 | LCMP2EN, LCMP1EN, LCMP0EN | R/W | 0 | Low byte compare n enable: 1 = in PWM mode, WOn overrides the pin's PORT output value |
```

```table id=tca-s-ctrlc caption="CTRLC (+0x02), reset 0x00, split mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | HCMP2OV, HCMP1OV, HCMP0OV | R/W | 0 | High byte compare n output value: sets or clears WO[n+3] while the TCA is not enabled |
| 2:0 | LCMP2OV, LCMP1OV, LCMP0OV | R/W | 0 | Low byte compare n output value: sets or clears WOn while the TCA is not enabled |
```

```table id=tca-s-ctrle caption="CTRLECLR (+0x04), CTRLESET (+0x05), reset 0x00, split mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:2 | CMD[1:0] | R/W | 0 | Command, always reads 0; used with CMDEN: 0x0 = NONE; 0x2 = RESTART: force restart; 0x3 = RESET: force hard reset (ignored while enabled; needs CMDEN = BOTH); 0x1 reserved |
| 1:0 | CMDEN[1:0] | R/W | 0 | Command enable, which timers CMD applies to: 0x0 = NONE; 0x3 = BOTH: low and high byte timer; 0x1, 0x2 reserved |
```

```table id=tca-s-intctrl caption="INTCTRL (+0x0A), reset 0x00, split mode [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | LCMP2, LCMP1, LCMP0 | R/W | 0 | 1 = low byte compare channel n interrupt enabled |
| 1 | HUNF | R/W | 0 | 1 = high byte underflow interrupt enabled |
| 0 | LUNF | R/W | 0 | 1 = low byte underflow interrupt enabled |

[1] Source error: the bit headings call LCMP2 "Low byte Compare Channel 0 Interrupt Enable"; the bit's name and the LCMPn text make it channel 2.
```

```table id=tca-s-intflags caption="INTFLAGS (+0x0B), reset 0x00, split mode [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:4 | LCMP2, LCMP1, LCMP0 | R/W | 0 | Low byte compare channel n flag: set on an LCNT = LCMPn match. Write 1 to clear. |
| 1 | HUNF | R/W | 0 | High byte underflow flag: set when the high byte timer reaches BOTTOM. Write 1 to clear. |
| 0 | LUNF | R/W | 0 | Low byte underflow flag: set when the low byte timer reaches BOTTOM. Write 1 to clear. |

[1] Source error: the bit headings call LCMP2 and LCMP1 "Low byte Compare Channel 0 Interrupt Flag"; the bit names and the LCMPn text make them channels 2 and 1.
```

```table id=tca-s-counters caption="Split-mode counter, period and compare registers, each 8 bits R/W, reset 0x00"
| Offset | Register | Description |
|---|---|---|
| +0x20 | LCNT | Low byte timer count. CPU and UPDI writes have priority over count, clear and reload. |
| +0x21 | HCNT | High byte timer count, same priority rule |
| +0x26 | LPER | Low byte timer TOP [1] |
| +0x27 | HPER | High byte timer TOP [1] |
| +0x28 + 2n | LCMPn (n = 0…2) | Low byte compare value of channel n, compared continuously to LCNT |
| +0x29 + 2n | HCMPn (n = 0…2) | High byte compare value of channel n, compared continuously to HCNT |

[1] Source conflict: the PDF gives a reset of 0x00 for LPER and HPER, though they occupy PER, which resets to 0xFFFF; mode changes keep register values.
```

### TCB: 16-bit timer/counter type B

Four instances, TCB0–TCB3, each with one waveform output WO (its pin is
chosen in PORTMUX). A 16-bit counter CNT and a 16-bit capture/compare
register CCMP, clocked by CLK_PER, CLK_PER/2 or the clock CLK_TCA of
TCA0 (then the TCB counts on exactly the clock TCA0 uses, and with
SYNCUPD = 1 it restarts whenever TCA0 restarts or overflows, which
synchronizes captures with the PWM period). BOTTOM = 0x0000, MAX =
0xFFFF, TOP = the highest value of the count sequence. Eight modes are
selected by CTRLB.CNTMODE. To start (periodic interrupt mode is the
default): write TOP to CCMP, then set CTRLA.ENABLE; the counter counts
CLK_TCB ticks and raises CAPT (interrupt and event) at TOP. Event
pulses must be longer than one system clock cycle to be detected as
edges.

#### Modes

- **Periodic interrupt (INT).** Counts from BOTTOM to TOP (= CCMP) and
  restarts; CAPT at TOP. If TOP is changed to a value below CNT, the
  counter counts to MAX, wraps and restarts from BOTTOM.
- **Time-out check (TIMEOUT).** Starts counting on the first edge of
  the event input and stops on the next; which edge starts is set by
  EVCTRL.EDGE. If CNT reaches TOP before the second edge, CAPT is set.
  In the frozen state after a stop edge, the counter restarts on a new
  start edge; reading CNT or CCMP or writing STATUS.RUN then has no
  effect. TOP below CNT: counts to MAX and restarts from BOTTOM.
- **Input capture on event (CAPT).** Counts BOTTOM to MAX continuously;
  on an event edge (rising or falling, EDGE) CNT is copied to CCMP and
  CAPT is set. Write CNT = 0 when entering this mode from another mode.
- **Frequency measurement (FRQ).** On the selected edge CNT is copied
  to CCMP, CAPT is set and the counter restarts from BOTTOM.
- **Pulse-width measurement (PW).** The counter restarts on a rising
  edge and captures on the next falling edge (EDGE = 1 swaps them),
  then sets CAPT. Edge detection switches automatically; the edges must
  be at least two clock cycles apart.
- **Frequency and pulse-width measurement (FRQPW).** Starts on a rising
  edge, captures CNT into CCMP on the following falling edge, stops on
  the second rising edge and sets CAPT (EDGE = 1 swaps the polarities).
  Further edges are ignored until CCMP is read; reading CCMP's low byte
  clears CAPT and arms the next sequence, which resets CNT to BOTTOM at
  the next start edge, so read CNT (period) before CCMP (pulse width).
- **Single-shot (SINGLE).** Generates a pulse of CCMP clock cycles on
  each event edge. While stopped the output is low; an event resets
  the counter and it counts BOTTOM to TOP with the output high (STATUS.RUN
  shows whether it counts); at CCMP it stops and the output is low for
  at least one prescaler cycle, during which a new event is ignored.
  The output goes high two clock cycles after the event; with
  CTRLB.ASYNC = 1 it goes high at once (the counter still starts two to
  three CLK_TCB cycles later) and the event is edge-triggered, so
  changes shorter than one system clock cycle are caught. The counter
  starts as soon as the module is enabled, even without an event, and
  also when EDGE = 1 while it is enabled; writing TOP to CNT prevents
  both.
- **8-bit PWM (PWM8).** CCMPL and CCMPH act as two independent 8-bit
  compare registers. The counter counts continuously from BOTTOM to
  CCMPL; the output is set at BOTTOM and cleared when CNT reaches
  CCMPH. CCMPH is the number of cycles the output is high; CCMPL + 1 is
  the period of the output pulse. Source conflict: the same paragraph
  first says the period (T) is controlled by CCMPH and CCMPL controls
  the duty cycle. CCMPH = 0
  gives a constant low output, CCMPH > CCMPL a constant high output.
  CAPT is set when CNT reaches CCMPL.

#### Output

```table id=tcb-output caption="Output configuration"
| CCMPEN | CNTMODE | ASYNC | Output |
|---|---|---|---|
| 1 | Single-shot | 0 | High when the counter starts, low when it stops |
| ^ | ^ | 1 | High when the event arrives, low when the counter stops |
| ^ | 8-bit PWM | — | 8-bit PWM waveform |
| ^ | Other modes | — | Level set by CTRLB.CCMPINIT |
| 0 | — | — | No output |
```

Changing modes while the TCB is enabled can produce an unpredictable
output. An interrupt flag may be set while configuring; clear INTFLAGS
after configuration.

**Noise canceler.** With EVCTRL.FILTER = 1 the event input is sampled
on the system clock (not prescaled) and passed to the edge detector
only after four equal consecutive samples; this adds four system clock
cycles of delay before the capture register updates.

#### Events, interrupts, sleep, debug

The TCB generates the CAPT event: a pulse of one CLK_PER period, under
the same conditions that set INTFLAGS.CAPT. It uses two event inputs:
CAPT (edge-detected; with EVCTRL.CAPTEI = 1 it triggers the action of
the current mode and EDGE; synchronous, or both synchronous and
asynchronous in single-shot mode) and COUNT (an event used as the
clock source with a count mode, synchronous). Source conflict: the
text enables COUNT by setting CTRLA.CLKSEL to EVENT, but CLKSEL lists
no EVENT value. An event needs to last at
least one CLK_PER cycle unless asynchronous single-shot is used. One
interrupt vector, CAPT, enabled by INTCTRL.CAPT. The TCB stops in
standby unless CTRLA.RUNSTDBY = 1 (not available with CLKSEL =
CLKTCA), and always stops in power-down.

16-bit registers CNT and CCMP are accessed through the shared TEMP
register (low byte first on write, low byte first on read; see the CPU
section). CPU and UPDI writes to CNT have priority over internal
updates.

#### Registers

```table id=tcb-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | RUNSTDBY | R/W | 0 | 1 = run in standby sleep mode. Not applicable with CLKSEL = 0x2. |
| 4 | SYNCUPD | R/W | 0 | 1 = restart the TCB whenever TCA0 restarts or overflows |
| 2:1 | CLKSEL[1:0] | R/W | 0x0 | Clock: 0x0 = CLKDIV1: CLK_PER; 0x1 = CLKDIV2: CLK_PER/2; 0x2 = CLKTCA: CLK_TCA from TCA0; 0x3 reserved |
| 0 | ENABLE | R/W | 0 | 1 = TCB enabled |
```

```table id=tcb-ctrlb caption="CTRLB (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | ASYNC | R/W | 0 | Single-shot output: 0 = goes high when the counter starts, after synchronization; 1 = goes high when the event arrives |
| 5 | CCMPINIT | R/W | 0 | Initial output level when the pin output is used: 0 = low, 1 = high |
| 4 | CCMPEN | R/W | 0 | Compare/capture output: 0 = output is 0, 1 = output has a valid value |
| 2:0 | CNTMODE[2:0] | R/W | 0x0 | Mode: 0x0 = INT: periodic interrupt; 0x1 = TIMEOUT: time-out check; 0x2 = CAPT: input capture on event; 0x3 = FRQ: frequency measurement; 0x4 = PW: pulse-width measurement; 0x5 = FRQPW: frequency and pulse-width measurement; 0x6 = SINGLE: single-shot; 0x7 = PWM8: 8-bit PWM |
```

```table id=tcb-evctrl caption="EVCTRL (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | FILTER | R/W | 0 | 1 = input capture noise cancellation filter on |
| 4 | EDGE | R/W | 0 | Event edge; effect per mode in the table below |
| 0 | CAPTEI | R/W | 0 | 1 = capture event input enabled |
```

```table id=tcb-edge caption="Effect of EVCTRL.EDGE per mode (— = no effect)"
| Mode | EDGE | Rising edge | Falling edge |
|---|:-:|---|---|
| Periodic interrupt | 0 | — | — |
| ^ | 1 | — | — |
| Time-out check | 0 | Start counter | Stop counter |
| ^ | 1 | Stop counter | Start counter |
| Input capture on event | 0 | Capture, interrupt | — |
| ^ | 1 | — | Capture, interrupt |
| Frequency measurement | 0 | Capture, clear and restart counter, interrupt | — |
| ^ | 1 | — | Capture, clear and restart counter, interrupt |
| Pulse-width measurement | 0 | Clear and restart counter | Capture, interrupt |
| ^ | 1 | Capture, interrupt | Clear and restart counter |
| Frequency and pulse-width measurement | 0 | 1st rising: clear and restart counter; 2nd rising: stop counter, interrupt | Following falling: capture |
| ^ | 1 | Following rising: capture | 1st falling: clear and restart counter; 2nd falling: stop counter, interrupt |
| Single-shot | 0 | Start counter | — |
| ^ | 1 | — | Start counter |
| 8-bit PWM | 0 | — | — |
| ^ | 1 | — | — |
```

```table id=tcb-intctrl caption="INTCTRL (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | CAPT | R/W | 0 | 1 = capture interrupt enabled |
```

```table id=tcb-intflags caption="INTFLAGS (+0x06), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | CAPT | R/W | 0 | Capture interrupt flag, set as in the table below. Cleared by writing 1, or in capture modes by reading CCMP (low byte). |
```

```table id=tcb-capt caption="CAPT set conditions by mode"
| Mode | CAPT is set | TOP | CAPT action |
|---|---|---|---|
| Periodic interrupt, time-out check, single-shot | When the counter reaches TOP | CCMP | CNT == TOP |
| Frequency measurement | On the edge that loads the capture register and restarts the counter; cleared when the capture is read | — | On event: copy CNT to CCMP and restart (CNT = BOTTOM) |
| Input capture on event | When an event loads the capture register; cleared when the capture is read | — | On event: copy CNT to CCMP, continue counting |
| Pulse-width measurement | On the edge that loads the capture register (the previous edge started the count); cleared when the capture is read | — | ^ |
| Frequency and pulse-width measurement | On the second (rising or falling) edge, when the counter stops; cleared when the capture is read | — | ^ |
| 8-bit PWM | When the counter reaches CCMPL | CCMPL | CNT == CCMPL |
```

```table id=tcb-status caption="STATUS (+0x07), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | RUN | R | 0 | 1 = counter running. Read-only, can't be set by UPDI. |
```

```table id=tcb-dbgctrl caption="DBGCTRL (+0x08), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | 0 = halted in break debug mode, events ignored; 1 = keeps running while the CPU is halted |
```

```table id=tcb-temp caption="TEMP (+0x09), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | TEMP[7:0] | R/W | 0x00 | Temporary byte for single-cycle 16-bit access to CNT and CCMP; one shared by all of them; readable and writable by software |
```

```table id=tcb-cnt caption="CNT (+0x0A, 16-bit: CNTL, CNTH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CNT[15:0] | R/W | 0x0000 | Counter value |
```

```table id=tcb-ccmp caption="CCMP (+0x0C, 16-bit: CCMPL, CCMPH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CCMP[15:0] | R/W | 0x0000 | Capture modes: the captured counter value. Periodic interrupt, time-out and single-shot: TOP. 8-bit PWM: CCMPL (period) and CCMPH (compare) are independent 8-bit registers. |
```

### RTC: real-time counter

Two timing functions on one clock, CLK_RTC, each enabled on its own:
the real-time counter (RTC) and the periodic interrupt timer (PIT).
Clock sources (RTC.CLKSEL): OSCULP32K at 32.768 kHz or divided by 32
(1.024 kHz), the 32.768 kHz crystal oscillator XOSC32K on TOSC1/TOSC2
(with any load capacitors needed) or an external clock on TOSC1, or an
external clock on the EXTCLK pin. For XOSC32K or the TOSC1 clock,
XOSC32K must be enabled and CLKCTRL.XOSC32KCTRLA's SEL and RUNSTDBY
set to match. CLK_PER must be at least four times CLK_RTC for reading
the counter, whatever the prescaler setting.

**RTC.** A 15-bit prescaler (÷1 to ÷32768) feeds the 16-bit counter
CNT, which is compared with the period register PER and the compare
register CMP. The compare interrupt/event fires on the first count
after CNT equals CMP; the overflow interrupt/event on the first count
after CNT equals PER, and the overflow also clears CNT. At 32.768 kHz
the finest resolution is 30.5 µs with time-outs up to 2 s; at 1 s
resolution the longest time-out is 65536 s (over 18 h). Set-up:
configure the oscillator in CLKCTRL, write RTC.CLKSEL, write CMP and/or
PER, enable interrupts in INTCTRL (CMP, OVF), then write PRESCALER and
RTCEN = 1 in CTRLA. Clearing RTCEN disables it. Source error: the
text calls this bit ENABLE (RTC.CTRLA); the register description and
summary name it RTCEN.

**PIT.** Uses the same prescaler counter: it requests an interrupt
every n CLK_RTC cycles, n = 4, 8, 16 … 32768 (PITCTRLA.PERIOD), and
outputs eight 50 % duty-cycle event clocks. Set-up: configure CLK_RTC,
set PITINTCTRL.PI, then write PERIOD and PITEN = 1 in PITCTRLA. The
prescaler is off when both RTCEN and PITEN are 0 and runs when either
is 1, so the first PIT interrupt and the first RTC tick come anywhere
between 0 and one full period after enabling. After the first
interrupt the PIT output toggles every half period. Example: with
PERIOD = CYC16 the PIT output follows prescaler counter bit 3, a
period of 16 CLK_RTC cycles.

The RTC is used internally during start-up: always check the busy bits
in STATUS and PITSTATUS, also before the first configuration.

**Crystal error correction.** With CTRLA.CORREN = 1 the prescaler
corrects the crystal clock by the ppm value in CALIB, written by
software from an external calibration and/or temperature compensation.
In each interval of about 10^6^ input clock cycles, one cycle is added
(SIGN = 0, counts slower) or removed (SIGN = 1, counts faster) ERROR
times, spread over the interval; the RTC can so be trimmed to ±1 ppm.
The correction shows in CNT and in the PIT intervals. Disabling it
finishes an ongoing correction cycle first.

**Events.** RTC: OVF (counter reached its top value and wrapped to 0)
and CMP (CNT matched CMP), each a strobe of one CLK_RTC cycle,
synchronous to CLK_RTC. PIT (when enabled): event 0 to event 7 are
clocks of 8192, 4096, 2048, 1024, 512, 256, 128 and 64 CLK_RTC cycles
period.

**Interrupts.** Vector RTC: OVF and CMP (flags in INTFLAGS, enables in
INTCTRL). Vector PIT: a PERIOD has passed (PITINTFLAGS, PITINTCTRL).

**Sleep.** The RTC runs in idle, and in standby if CTRLA.RUNSTDBY = 1.
The PIT runs in every sleep mode.

**Synchronization.** RTC and PIT run on CLK_RTC, asynchronous to
CLK_PER; writes to CTRLA, CNT, PER, CMP and PITCTRLA take effect two
CLK_RTC cycles later. Busy flags STATUS.CTRLABUSY, CNTBUSY, PERBUSY,
CMPBUSY and PITSTATUS.CTRLBUSY must be 0 before writing those
registers. Source error: the text calls the PIT busy flag SYNCBUSY; the
PITSTATUS register description and summary name it CTRLBUSY. 16-bit CNT, PER and CMP are accessed through the shared
TEMP register (see the CPU section).

**Debug.** RTC: with DBGCTRL.DBGRUN = 0 it halts and ignores events
while the CPU is halted. PIT: with PITDBGCTRL.DBGRUN = 0 the PIT output
is low while the CPU is halted; if it was high at the break, the new
rising edge on restart sets the flag, giving one extra PIT interrupt;
if it was low, it resumes without one.

#### Registers

```table id=rtc-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RUNSTDBY | R/W | 0 | 1 = RTC enabled in standby sleep mode |
| 6:3 | PRESCALER[3:0] | R/W | 0x0 | CLK_RTC prescaling 2^n^, n = value: 0x0 = DIV1 (no prescaling), 0x1 = DIV2, 0x2 = DIV4 … 0xE = DIV16384, 0xF = DIV32768. Takes effect two CLK_RTC cycles after the write; check CTRLABUSY = 0 before writing. |
| 2 | CORREN | R/W | 0 | 1 = frequency correction enabled |
| 0 | RTCEN | R/W | 0 | 1 = RTC enabled |
```

```table id=rtc-status caption="STATUS (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3 | CMPBUSY | R | 0 | 1 = CMP being synchronized into the RTC clock domain |
| 2 | PERBUSY | R | 0 | 1 = PER being synchronized |
| 1 | CNTBUSY | R | 0 | 1 = CNT being synchronized |
| 0 | CTRLABUSY | R | 0 | 1 = CTRLA being synchronized |
```

```table id=rtc-intctrl caption="INTCTRL (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | CMP | R/W | 0 | 1 = interrupt when CNT matches CMP |
| 0 | OVF | R/W | 0 | 1 = interrupt when CNT matched PER and wrapped to 0 |
```

```table id=rtc-intflags caption="INTFLAGS (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | CMP | R/W | 0 | Set when CNT matches CMP. Write 1 to clear. |
| 0 | OVF | R/W | 0 | Set when CNT reached PER and wrapped to 0. Write 1 to clear. |
```

```table id=rtc-temp caption="TEMP (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | TEMP[7:0] | R/W | 0x00 | Temporary byte for single-cycle 16-bit access to CNT, PER and CMP; one shared by all of them |
```

```table id=rtc-dbgctrl caption="DBGCTRL (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | 0 = halted in break debug mode, events ignored; 1 = keeps running while the CPU is halted |
```

```table id=rtc-calib caption="CALIB (+0x06), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | SIGN | R/W | 0 | Correction direction: 0 = positive, prescaler counts slower; 1 = negative, prescaler counts faster (needs PRESCALER ≥ DIV2) |
| 6:0 | ERROR[6:0] | R/W | 0x00 | Correction clocks per 10^6^ RTC clock cycles (ppm) |
```

```table id=rtc-clksel caption="CLKSEL (+0x07), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1:0 | CLKSEL[1:0] | R/W | 0x0 | CLK_RTC source: 0x0 = INT32K: 32.768 kHz from OSCULP32K; 0x1 = INT1K: 1.024 kHz from OSCULP32K; 0x2 = TOSC32K: 32.768 kHz from XOSC32K or external clock on TOSC1; 0x3 = EXTCLK: external clock on the EXTCLK pin |
```

```table id=rtc-cnt caption="CNT (+0x08, 16-bit: CNTL, CNTH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CNT[15:0] | R/W | 0x0000 | Counter value. Takes effect two CLK_RTC cycles after the write; check CNTBUSY = 0 before writing. |
```

```table id=rtc-per caption="PER (+0x0A, 16-bit: PERL, PERH), reset 0xFFFF"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | PER[15:0] | R/W | 0xFFFF | Period (top) value. Takes effect two CLK_RTC cycles after the write; check PERBUSY = 0 before writing. |
```

```table id=rtc-cmp caption="CMP (+0x0C, 16-bit: CMPL, CMPH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | CMP[15:0] | R/W | 0x0000 | Compare value. Check CMPBUSY = 0 before writing. |
```

```table id=rtc-pitctrla caption="PITCTRLA (+0x10), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:3 | PERIOD[3:0] | R/W | 0x0 | CLK_RTC cycles between interrupts: 0x0 = OFF: no interrupt; 0x1 = CYC4, 0x2 = CYC8, 0x3 = CYC16 … 0xE = CYC32768 (2^n+1^ cycles for value n); 0xF reserved. Check CTRLBUSY = 0 before writing. |
| 0 | PITEN | R/W | 0 | 1 = PIT enabled |
```

```table id=rtc-pitstatus caption="PITSTATUS (+0x11), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | CTRLBUSY | R | 0 | 1 = PITCTRLA being synchronized into the RTC clock domain |
```

```table id=rtc-pitintctrl caption="PITINTCTRL (+0x12), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | PI | R/W | 0 | 1 = periodic interrupt enabled |
```

```table id=rtc-pitintflags caption="PITINTFLAGS (+0x13), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | PI | R/W | 0 | Set when a periodic interrupt is issued. Write 1 to clear. |
```

```table id=rtc-pitdbgctrl caption="PITDBGCTRL (+0x15), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | 0 = halted in break debug mode, events ignored; 1 = keeps running while the CPU is halted |
```

### USART: universal synchronous and asynchronous receiver and transmitter

Four instances, USART0–USART3. Full- or half-duplex, asynchronous or
synchronous, frames of 5–9 data bits with 1 or 2 stop bits, a fractional
baud-rate generator, odd/even parity, buffer-overflow and frame-error
detection, noise filtering (false start-bit detection, digital low-pass
filter), master SPI mode, multiprocessor mode, start-of-frame detection,
an IrDA-compatible IRCOM pulse codec and LIN slave support. The
transmitter has a single write buffer and a shift register; the receiver
a two-level buffer and a shift register. Block diagram: baud-rate
generator (XCK), transmit shift register (TXD, XDIR) and receive shift
register with RX buffer (RXD).

```table id=usart-signals caption="USART signals"
| Signal | Type | Description |
|---|---|---|
| XCK | Output/input | Clock for synchronous operation |
| XDIR | Output | Transmit enable for RS-485 |
| TxD | Output/input | Transmit line (also the receive line in one-wire mode) |
| RxD | Input | Receive line |
```

#### Initialization

Full duplex: set BAUD; set the frame format and mode (CTRLC); make the
TXD pin an output; enable transmitter and receiver (CTRLB). One-wire
half duplex: set LBME (CTRLA) to connect TXD to the receiver; enable the
pin's pull-up (PORTx.PINnCTRL.PULLUPEN); set ODME (CTRLB; hardware then
makes TXD an output); set BAUD; set CTRLC; enable transmitter and
receiver. In both cases keep global interrupts disabled during an
interrupt-driven initialization, and make sure no transmission is in
progress before changing the baud rate or frame format.

#### Frame format

A frame is one start bit (always low), 5, 6, 7, 8 or 9 data bits (LSb
first), an optional even or odd parity bit after the data bits, and 1 or
2 stop bits (always high). The next frame may follow at once, or the
line returns to idle (high).

```wavedrom id=usart-frame caption="USART frame (bits in brackets optional)"
{"signal": [
  {"name": "TxD", "wave": "1.0============1.", "data": ["0", "1", "2", "3", "4", "[5]", "[6]", "[7]", "[8]", "[P]", "Sp1", "[Sp2]"]}
 ],
 "config": {"hscale": 2}}
```

#### Clock generation

The shift clock comes from the fractional baud-rate generator or, in
synchronous slave mode, from the XCK pin. BAUD holds the division factor
shifted left by six bits: the six LSbs are a fraction that dynamically
adjusts the bit period for a closer approximation. The integer part must
be at least 1, so BAUD is 64 to 65535. In synchronous mode only
BAUD[15:6] counts and BAUD[5:0] must be 0. S is the number of samples
per bit: 16 in asynchronous normal mode, 8 in asynchronous double-speed
mode, 2 in synchronous mode.

```table id=usart-baud caption="Baud-rate equations (results can be written to BAUD directly)"
| Mode | Conditions | Baud rate | BAUD register value |
|---|---|---|---|
| Asynchronous | f~BAUD~ ≤ f~CLK_PER~ / S, BAUD ≥ 64 | f~BAUD~ = 64 × f~CLK_PER~ / (S × BAUD) | BAUD = 64 × f~CLK_PER~ / (S × f~BAUD~) |
| Synchronous master | f~BAUD~ ≤ f~CLK_PER~ / S, BAUD ≥ 64 | f~BAUD~ = f~CLK_PER~ / (S × BAUD[15:6]) | BAUD[15:6] = f~CLK_PER~ / (S × f~BAUD~) |
```

#### Transmission and reception

Writing TXDATA starts a transmission: the buffer moves to the shift
register when that is empty. TXDATA may only be written while DREIF is
set. When the whole frame has been shifted out and the buffer is empty,
TXCIF is set. For frames under 8 bits the unused MSbs written are
ignored; for 9-bit frames write TXDATAH (DATA[8]) before TXDATAL (except
9BITL, low byte first). Disabling the transmitter takes effect only when
the shift register and buffer are empty; the USART then releases TXD and
hardware makes the pin an input.

The receiver needs its pin configured as an input (PORTx.DIRn = 0). On a
valid start bit, the bits are sampled at the baud rate or XCK into the
shift register up to the first stop bit (a second stop bit is ignored);
the frame then moves to the receive buffer and RXCIF is set. Unused MSbs
of short frames read 0. The error flags (FERR, BUFOVF, PERR) travel
with their frame in the buffer: read RXDATAH before RXDATAL, since
reading RXDATAL shifts the buffer (with CHSIZE = 9BITL, RXDATAH shifts it
and RXDATAL must be read first). Disabling the receiver is immediate: the
buffer is flushed and an ongoing reception is lost. To flush the buffer
in normal operation, read RXDATAH/RXDATAL until RXCIF clears.

#### Synchronous operation

The XCK pin direction selects slave (input) or master (output); the
port pin direction must be set to output for master mode or to input
for slave mode (PORTx.DIRn). RxD is sampled on the XCK edge opposite the
one TxD changes on: with the XCK pin's INVEN = 0 (PORTx.PINnCTRL) a
rising edge starts each data bit and RxD is sampled on the falling edge;
with INVEN = 1 the falling edge starts the bit and RxD is sampled on the
rising edge. In synchronous slave mode BAUD has no effect; clock
recovery needs each XCK edge sampled at least twice, so

```equation
f~Slave_XCK~ < f~CLK_PER~ / 4
```

and less if XCK has jitter or a duty cycle other than 50/50.

#### Master SPI mode

CMODE = MSPI. Frames are always 8 data bits, MSb or LSb first (UDORD),
with no start, stop or parity bits; XCK comes from the baud-rate
generator. Each bit is driven for a full clock period and sampled in the
middle. INVEN on the XCK pin and UCPHA select the timing. Transmission
and reception work as in the other modes (same flags and interrupts);
the receiver error flags are unused and read 0. Compared with the SPI
peripheral, write-collision protection, double-speed mode and
multi-master support are missing, and there is no slave select: TXD =
MOSI, RXD = MISO, XCK = SCK.

```table id=usart-mspi caption="INVEN and UCPHA in master SPI mode [1]"
| INVEN | UCPHA | Leading edge | Trailing edge |
|:-:|:-:|---|---|
| 0 | 0 | Rising, sample | Falling, transmit |
| 0 | 1 | Rising, transmit | Falling, sample |
| 1 | 0 | Falling, sample | Rising, transmit |
| 1 | 1 | Falling, transmit | Rising, sample |

[1] The leading edge is the first clock edge of a clock cycle, the trailing edge the last.
```

#### Asynchronous clock and data recovery

Received frames are sampled at 16 × the baud rate (RXMODE = NORMAL) or
8 × (CLK2X, double speed; larger synchronization error). A falling edge
from idle starts start-bit detection (sample 1 is the first that reads
0); samples 8, 9, 10 (normal) or 4, 5, 6 (double speed) then decide: two
or three zeros accept the start bit, otherwise it is rejected. Each data
and parity bit is decided by majority of the same three centre samples.
Only the first stop bit is checked; a 0 sets FERR. The next start bit's
first low sample may come right after the stop bit's majority samples in
normal mode (sample 10), and must wait for the first sample after them
in double-speed mode.

```table id=usart-err-normal caption="Recommended maximum receiver baud-rate error, normal speed [1]"
| D | R~slow~ (%) | R~fast~ (%) | Max. total error (%) | Recommended max. receiver error (%) |
|--:|--:|--:|---|---|
| 5 | 93.20 | 106.67 | -6.80/+6.67 | ±3.0 |
| 6 | 94.12 | 105.79 | -5.88/+5.79 | ±2.5 |
| 7 | 94.81 | 105.11 | -5.19/+5.11 | ±2.0 |
| 8 | 95.36 | 104.58 | -4.54/+4.58 | ±2.0 |
| 9 | 95.81 | 104.14 | -4.19/+4.14 | ±1.5 |
| 10 | 96.17 | 103.78 | -3.83/+3.78 | ±1.5 |

[1] D = character size plus parity size (5 to 10 bits). R~slow~ and R~fast~ are the slowest and fastest incoming data rates accepted, relative to the receiver baud rate. The recommendation assumes receiver and transmitter share the total error equally.
```

```table id=usart-err-2x caption="Recommended maximum receiver baud-rate error, double speed [1]"
| D | R~slow~ (%) | R~fast~ (%) | Max. total error (%) | Recommended max. receiver error (%) |
|--:|--:|--:|---|---|
| 5 | 94.12 | 105.66 | -5.88/+5.66 | ±2.5 |
| 6 | 94.92 | 104.92 | -5.08/+4.92 | ±2.0 |
| 7 | 95.52 | 104.35 | -4.48/+4.35 | ±1.5 |
| 8 | 96.00 | 103.90 | -4.00/+3.90 | ±1.5 |
| 9 | 96.39 | 103.53 | -3.61/+3.53 | ±1.5 |
| 10 | 96.70 | 103.23 | -3.30/+3.23 | ±1.0 |

[1] As in the normal-speed table.
```

```equation
R~slow~ = S × (D + 1) / (S × (D + 1) + S~F~ - 1)
R~fast~ = S × (D + 2) / (S × (D + 1) + S~M~)
```

S = 16 (normal) or 8 (double speed); S~F~, the first majority sample, is
8 or 4; S~M~, the middle one, is 9 or 5.

Double-speed mode (RXMODE = CLK2X) doubles the baud rate for a given
BAUD setting and halves the samples per bit (16 to 8), so it needs a
more accurate baud setting and clock. It has no effect in synchronous
mode (write RXMODE = 0) and cannot be used with IRCOM.

#### Auto-baud

Both auto-baud modes (asynchronous slave only) expect a break field
followed by a sync field. A break is 12 or more consecutive low samples;
at the sync field's start bit a counter on CLK_PER starts and runs for
8 T~bit~; its final value becomes the new BAUD. GENAUTO (generic): the
sync character is valid when eight alternating bits are seen, any sync
that gives a valid BAUD (0x0064–0xFFFF) updates BAUD, and setting WFB
accepts a break of any length (shorter than 12 cycles too), so any new
baud rate can be set without knowing the current one. LINAUTO (LIN
constrained): WFB is not supported, the line must be low 12 CLK_PER
cycles or more for a valid break, every two sync bits must fall within
32 ± 6 baud samples of the internal baud rate, and the sync character
must be 0x55; otherwise ISFIF is set and BAUD is unchanged. The ABW
field sets the tolerance window for the sync bits.

```wavedrom id=usart-autobaud caption="Auto-baud frame: break field, then sync field (0x55)"
{"signal": [
  {"name": "RxD", "wave": "1.0.......1.0101010101", "node": "............A.......B"}
 ],
 "edge": ["A<->B 8 Tbit"],
 "config": {"hscale": 2}}
```

#### Half duplex: one-wire and RS-485

One-wire: LBME = 1 connects TXD internally to the receiver, making TXD a
combined TxD/RxD line; RXD is disconnected and free for other
peripherals. To avoid shorts when several devices drive the line, ODME
makes the transmitter open drain (only pulls low); with the pin's
pull-up the line idles high. The USART receives its own transmission,
which can be compared with the data sent to detect collisions.

RS-485 (RS485 field in CTRLA) supports an external differential line
driver. RS485[0] = 1 drives XDIR high one baud clock before the frame is
shifted out (guard time) and low when the transmission, including stop
bits, is complete. RS485[1] = 1 makes TXD an output one clock cycle
before transmission and an input again when it is complete. RS-485 can
be combined with one-wire mode (LBME).

```wavedrom id=usart-xdir caption="XDIR drive timing (RS485[0] = 1)"
{"signal": [
  {"name": "TxD", "wave": "1.0=========1.", "data": ["0", "1", "2", "3", "4", "5", "6", "7", "Sp1"], "node": "..B"},
  {"name": "XDIR", "wave": "01..........0.", "node": ".A"}
 ],
 "edge": ["A<->B guard time"],
 "config": {"hscale": 2}}
```

#### IRCOM

CMODE = IRCOM selects IrDA 1.4 compatible infrared pulse coding, up to
115.2 kbps. The TXD/RXD pin levels are the inverse of the infrared
pulses. The IRCOM receiver can take its input from an event channel
instead of RXD (EVCTRL.IREI; RXD is then disabled). Transmit pulses
(TXPLCTRL): 3/16 of the bit period, a fixed number of CLK_PER cycles,
or no modulation. Receive (RXPLCTRL): a programmable minimum pulse width
for a pulse to decode as 0; shorter pulses are discarded and the bit
decodes as 1. Double-speed mode is not allowed with IRCOM.

#### Parity

PMODE selects even parity (parity bit = 1 when the number of data 1s is
odd) or odd parity (parity bit = 1 when it is even). The receiver checks
each frame and sets PERR on a mismatch. In LINAUTO mode only the
protected identifier field (ID0–ID5, P0, P1) is checked:

```equation
P0 = ID0 XOR ID1 XOR ID2 XOR ID4
P1 = NOT (ID1 XOR ID3 XOR ID4 XOR ID5)
```

#### Start-of-frame detection

Asynchronous mode only, enabled by SFDEN. In standby sleep, a
high-to-low transition on RXD powers up the oscillator and enables the
USART clock; the rest of the frame is received if the baud rate is slow
enough for the oscillator start-up time (see the oscillator start-up
times in the electrical characteristics). A detected start bit in
standby sets RXSIF. After a false start bit the device returns to
standby unless another wake-up source triggered. RXSIF and RXCIF share
the RXC interrupt line with separate enables. The SLEEP instruction does
not stop the oscillator while communication is ongoing.

```table id=usart-sfd caption="Start-of-frame detection modes"
| SFDEN | RXSIF interrupt | RXCIF interrupt | Behavior |
|:-:|---|---|---|
| 0 | x | x | Standard mode |
| 1 | Disabled | Disabled | Only the oscillator is powered during frame reception; with both interrupts disabled and buffer overflow ignored, all incoming frames are lost |
| 1 | Disabled | Enabled | System and all clocks wake on the receive-complete interrupt |
| 1 | Enabled | x | System and all clocks wake when a start bit is detected |
```

#### Multiprocessor communication

MPCM = 1: the receiver ignores frames that are not address frames (the
transmitter is unaffected). The frame-type bit is the first stop bit for
5- to 8-bit frames (the transmitter must then use two stop bits) or the
ninth bit for 9-bit frames: 1 = address, 0 = data. Procedure: all slaves
run with MPCM = 1; the master sends an address frame, which every slave
reads; the addressed slave clears MPCM and receives the data frames,
while the others ignore them; after the last data frame it sets MPCM
again and waits for the next address frame.

#### Events, interrupts, debug

Event generator: USARTn XCK, a pulse of one XCK period in the CLK_PER
domain (master SPI and synchronous master modes). Event user: USARTn
IREI, the IrDA event input (pulse, synchronous). Interrupt vectors:
RXC (unread data in the receive buffer, RXCIE; start of frame detected,
RXSIE; auto-baud error/ISFIF, ABEIE), DRE (transmit buffer empty, DREIE)
and TXC (frame shifted out and buffer empty, TXCIE). A request stays
active until its flag is cleared. DBGCTRL.DBGRUN keeps the USART running
when the CPU is halted by the debugger.

#### Registers

```table id=usart-rxdatal caption="RXDATAL (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DATA[7:0] | R | 0 | Received data, low eight bits. Reading shifts the two-level receive buffer (except with CHSIZE = 9BITL, where RXDATAH does) |
```

```table id=usart-rxdatah caption="RXDATAH (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXCIF | R | 0 | Receive complete: 1 while the receive buffer holds unread data; 0 when it is empty (also after the receiver is disabled, which flushes it) |
| 6 | BUFOVF | R | 0 | Buffer overflow: the buffer was full (two characters), a character waited in the shift register and a new start bit came. Valid until RXDATAL is read. Unused in master SPI mode |
| 2 | FERR | R | 0 | Frame error: the first stop bit of the next readable frame was 0. Not affected by SBMODE. Valid until RXDATAL is read. Unused in master SPI mode |
| 1 | PERR | R | 0 | Parity error in the next character in the buffer (reads 0 with parity disabled). In LINAUTO, the parity check of the protected identifier field, valid when DATA\[8\] reads 0. Valid until RXDATAL is read. Unused in master SPI mode |
| 0 | DATA\[8\] | R | 0 | Ninth data bit of 9-bit frames. In LINAUTO: 0 = data is the protected identifier field, 1 = data is in the response space of the LIN frame |
```

```table id=usart-txdatal caption="TXDATAL (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DATA[7:0] | R/W | 0 | Transmit buffer. Writable only while DREIF = 1 (other writes are ignored). Unused MSbs of 5- to 7-bit frames are ignored |
```

```table id=usart-txdatah caption="TXDATAH (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DATA\[8\] | W | 0 | Ninth data bit (CHSIZE = 9BIT). Write before TXDATAL, except with 9BITL (write TXDATAL first). Unused in master SPI mode |
```

```table id=usart-status caption="STATUS (+0x04), reset 0x00 [1] [2]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXCIF | R | 0 | Receive complete, as in RXDATAH. An interrupt routine must read RXDATA to clear it, or the interrupt repeats |
| 6 | TXCIF | R/W | 0 | Transmit complete: frame shifted out and transmit buffer empty. Cleared when the TXC vector executes, or by writing 1 |
| 5 | DREIF | R | 1 | Data register empty: 1 = transmit buffer ready for new data (set after reset). Cleared by writing TXDATAL; an interrupt routine must write TXDATA or disable DREIE. Always write 0 here when writing STATUS |
| 4 | RXSIF | R/W | 0 | Receive start: a valid idle-to-start transition on RxD in standby sleep. Reads 0 without start-of-frame detection. Cleared only by writing 1. Unused in master SPI mode |
| 3 | ISFIF | R/W | 0 | Inconsistent sync field: in auto-baud, the sync bit time was too fast or slow for a valid BAUD; in LINAUTO, also a sync character other than 0x55. Writing 1 clears it and returns the USART to idle |
| 1 | BDF | R/W | 0 | Break detected (LINAUTO and GENAUTO; unused in NORMAL and CLK2X): set after a valid break and sync character; fixed threshold of 11 low bits. Cleared when the next data is received or by writing 1 |
| 0 | WFB | R/W | 0 | Wait for break: writing 1 registers the next low and high transition on RxD as a break of any width; with GENAUTO this sets any baud rate in the BAUD range. Reads 0 |

[1] Source conflict: the PDF gives BDF's threshold as 11 bits here and the break as 12 or more low samples in the auto-baud description.
[2] Source conflict: the register header gives reset 0x00; DREIF's bit reset is 1 and its text says it is set after a Reset.
```

```table id=usart-ctrla caption="CTRLA (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXCIE | R/W | 0 | Receive complete interrupt enable (vector RXC, on RXCIF) |
| 6 | TXCIE | R/W | 0 | Transmit complete interrupt enable (vector TXC, on TXCIF) |
| 5 | DREIE | R/W | 0 | Data register empty interrupt enable (vector DRE, on DREIF) |
| 4 | RXSIE | R/W | 0 | Receiver start-of-frame interrupt enable (vector RXC) |
| 3 | LBME | R/W | 0 | Loop-back: connects TXD internally to the receiver and disconnects RXD from it |
| 2 | ABEIE | R/W | 0 | Auto-baud error interrupt enable (vector RXC, on ISFIF) |
| 1:0 | RS485[1:0] | R/W | 0 | RS485\[0\] = 1: XDIR driven high one clock cycle before transmission, low when complete. RS485\[1\] = 1: TXD made an output one clock cycle before transmission, an input when complete |
```

```table id=usart-ctrlb caption="CTRLB (+0x06), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXEN | R/W | 0 | Receiver enable. Disabling flushes the receive buffer (FERR, BUFOVF, PERR invalid) and, in GENAUTO/LINAUTO, resets the auto-baud logic |
| 6 | TXEN | R/W | 0 | Transmitter enable; overrides the TXD pin. Disabling takes effect when no data is pending; the pin is then made an input by hardware |
| 4 | SFDEN | R/W | 0 | Start-of-frame detection enable: an idle-to-start transition on RxD wakes the system from idle or standby sleep |
| 3 | ODME | R/W | 0 | Open-drain mode for TXD (enable the pin's pull-up so the line doesn't float) |
| 2:1 | RXMODE[1:0] | R/W | 0 | Receiver mode: 0x0 = NORMAL; 0x1 = CLK2X: double speed (divisor 16 → 8, asynchronous only); 0x2 = GENAUTO: generic auto-baud; 0x3 = LINAUTO: LIN constrained auto-baud. Auto-baud modes only in asynchronous slave mode. For synchronous operation RXMODE should always be written 0x0; it must be 0x0 with IRCOM |
| 0 | MPCM | R/W | 0 | Multiprocessor communication mode: receiver ignores frames without address information |
```

```table id=usart-ctrlc caption="CTRLC (+0x07), reset 0x03, all modes except master SPI"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | CMODE[1:0] | R/W | 0 | Communication mode: 0x0 = ASYNCHRONOUS; 0x1 = SYNCHRONOUS; 0x2 = IRCOM: infrared; 0x3 = MSPI: master SPI (changes this register's layout, below) |
| 5:4 | PMODE[1:0] | R/W | 0 | Parity: 0x0 = DISABLED; 0x2 = EVEN; 0x3 = ODD; 0x1 reserved |
| 3 | SBMODE | R/W | 0 | Stop bits sent: 0 = 1, 1 = 2 (the receiver ignores this) |
| 2:0 | CHSIZE[2:0] | R/W | 0x3 | Character size (receiver and transmitter): 0x0 = 5BIT; 0x1 = 6BIT; 0x2 = 7BIT; 0x3 = 8BIT; 0x6 = 9BITL: 9 bits, low byte first; 0x7 = 9BITH: 9 bits, high byte first; 0x4, 0x5 reserved |
```

```table id=usart-ctrlc-mspi caption="CTRLC (+0x07), reset 0x00, master SPI mode (CMODE = MSPI)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | CMODE[1:0] | R/W | 0 | Communication mode, as above; a value other than 0x3 restores the other layout |
| 2 | UDORD | R/W | 0 | Data order: 0 = MSb first, 1 = LSb first. Changing it corrupts ongoing communication in both directions |
| 1 | UCPHA | R/W | 0 | Clock phase: data sampled on the leading (0) or trailing (1) XCK edge (see the INVEN/UCPHA table) |
```

```table id=usart-baud-reg caption="BAUD (+0x08, 16-bit: BAUDL, BAUDH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | BAUD[15:0] | R/W | 0 | Baud-rate setting (equations above). A write updates the baud-rate prescaler at once and corrupts ongoing transmission and reception |
```

```table id=usart-ctrld caption="CTRLD (+0x0A), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | ABW[1:0] | R/W | 0 | Auto-baud window for validating the sync character bits: 0x0 = WDW0: 18 % tolerance; 0x1 = WDW1: 15 %; 0x2 = WDW2: 21 %; 0x3 = WDW3: 25 % |
```

```table id=usart-dbgctrl caption="DBGCTRL (+0x0B), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | Debug run: 0 = halted in break debug mode, ignores events; 1 = keeps running while the CPU is halted |
```

```table id=usart-evctrl caption="EVCTRL (+0x0C), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | IREI | R/W | 0 | IrDA event input enable: the IRCOM receiver takes its input from the event system; the RXD pin input is disabled |
```

```table id=usart-txplctrl caption="TXPLCTRL (+0x0D), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | TXPL[7:0] | R/W | 0 | IRCOM transmit pulse length (only in IRCOM mode; set before TXEN): 0x00 = 3/16 of the bit period; 0x01–0xFE = fixed pulse of TXPL system clock periods, starting on the rising edge of the baud clock; 0xFF = no pulse coding, RX and TX pass through unaltered (allows half-duplex, loop-back testing and RX input from an event channel) |
```

```table id=usart-rxplctrl caption="RXPLCTRL (+0x0E), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6:0 | RXPL[6:0] | R/W | 0 | IRCOM receive filter (only in IRCOM mode; set before RXEN): 0x00 = filtering disabled; 0x01–0x7F = a pulse needs RXPL + 1 samples to be accepted |
```

#### Electrical characteristics

The USART in master SPI mode uses the same timing parameters as the SPI
master (see the SPI master timing diagram).

```table id=usart-spi-char caption="USART in SPI master mode, timing characteristics [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| f~SCK~ | SCK clock frequency | | | 10 | MHz |
| t~SCK~ | SCK period | 100 | | | ns |
| t~SCKW~ | SCK high/low width | | 0.5 × t~SCK~ | | ns |
| t~SCKR~ | SCK rise time | | 2.7 | | ns |
| t~SCKF~ | SCK fall time | | 2.7 | | ns |
| t~MIS~ | MISO setup to SCK | | 10 | | ns |
| t~MIH~ | MISO hold after SCK | | 10 | | ns |
| t~MOS~ | MOSI setup to SCK | | 0.5 × t~SCK~ | | ns |
| t~MOH~ | MOSI hold after SCK | | 1.0 | | ns |

[1] Design guidance only, not production tested.
```

### SPI: serial peripheral interface

One instance, SPI0: full-duplex synchronous transfers on three or four
pins, master or slave, MSb or LSb first, seven bit rates, write-collision
protection, wake-up from idle sleep, and a double-speed (CLK_PER/2)
master mode. The master starts a transfer by pulling the slave's !{SS}
low and drives SCK; data moves from master to slave on MOSI and from
slave to master on MISO at the same time. Block diagram: an 8-bit shift
register, a clock generator (master), a transmit buffer register and
first and second receive buffer registers. DATA is not a physical
register: a write goes to the shift register in normal mode and to the
transmit buffer in buffer mode; a read returns the first receive buffer
in normal mode and the second receive buffer in buffer mode.

```table id=spi-signals caption="SPI pins: direction when the SPI is enabled [1]"
| Signal | Description | Master mode | Slave mode |
|---|---|---|---|
| MOSI | Master out, slave in | User defined | Input |
| MISO | Master in, slave out | Input | User defined |
| SCK | Serial clock | User defined | Input |
| SS | Slave select | User defined | Input |

[1] "Input": the SPI sets the direction. "User defined": the port sets it; as an input the pin is a normal input, as an output its value comes from the SPI. Exception: a slave's MISO configured as output is driven only while !{SS} is low and tri-stated while !{SS} is high.
```

#### Initialization

Configure the !{SS} pin in the port; select master or slave (CTRLA.MASTER);
in master mode set PRESC and CLK2X; optionally set MODE (CTRLB), DORD
(CTRLA), buffer mode (BUFEN, BUFWR) and, in master mode, SSD to disable
multi-master support; then set CTRLA.ENABLE.

#### Master mode

Writing DATA starts a transfer: the clock generator runs and the eight
bits are shifted out; then IF (normal mode) is set. With SSD = 1 the SPI
ignores !{SS}, which is a normal I/O pin. With SSD = 0 and !{SS} an
output, the pin is a normal I/O too. With SSD = 0 and !{SS} an input,
!{SS} must stay high: a low level means another master is taking the
bus, and the hardware clears MASTER (the SPI becomes a slave and the pin
directions change accordingly) and sets IF (and SSIF in buffer mode),
interrupting if enabled. If !{SS} can't be guaranteed high between
transfers, check MASTER before writing each byte; software must set
MASTER again to resume master mode.

**Normal mode (master):** single-buffered transmit, double-buffered
receive. DATA must not be written before the transfer is complete; an
early write corrupts the transmitted data and sets WRCOL. A received
byte goes to the first receive buffer when the transfer completes and
must be read (via DATA) before the next transfer completes, or it is
lost. The transmit buffer and second receive buffer are unused. IF is
set after each transfer; IE enables its interrupt.

**Buffer mode (master, BUFEN = 1; BUFWR has no effect):** double-buffered
transmit, triple-buffered receive. DATA may be written while DREIF is
set: the first byte is sent at once, the next waits in the transmit
buffer. Received bytes enter a two-entry FIFO (first and second receive
buffers), read through DATA; read it at least every second transfer.
TXCIF is set when the shift register and transmit buffer are both empty.

#### Slave mode

SCK is sampled by CLK_PER, so its low and high periods must each be
longer than two CLK_PER cycles. !{SS}, MOSI and SCK are always inputs.
With !{SS} low the slave is selected and clocks data out on MISO (if
MISO is an output); with !{SS} high it is deselected, receives nothing,
and an output MISO is tri-stated. Bringing !{SS} high resets the slave
state machine: a transfer in progress stops at once and its sent and
received data are lost. !{SS} thereby frames bytes and keeps the slave's
bit counter in step with the master.

```table id=spi-ss-slave caption="!{SS} in slave mode"
| !{SS} level | Slave | MISO, port output | MISO, port input |
|---|---|---|---|
| High | Deselected | Tri-stated | Input |
| Low | Selected | Output | Input |
```

**Normal mode (slave):** while !{SS} is high the SPI idles; software may
update DATA, but nothing is shifted until !{SS} goes low. Data shifts out
from the first SCK edge after !{SS} falls; IF is set when a byte is
complete. New data may be written before the received byte is read, but
not before the transfer is complete: an early write is ignored and sets
WRCOL. A partly received byte is lost when !{SS} goes high.

**Buffer mode (slave, BUFEN = 1):** adds the transmit buffer, the second
receive buffer and the DREIF, TXCIF, RXCIF, SSIF and BUFOVF flags. Every
write to DATA goes to the transmit buffer, and is accepted only while
DREIF is 1 (otherwise it is lost). With BUFWR = 0, the first byte written
is not copied to the shift register until one transfer has completed, so
the first byte sent is a dummy (whatever the shift register held,
usually the last byte received). With BUFWR = 1, a write while the SPI is
enabled and !{SS} is high goes straight to the shift register (the next
cycle), so the first byte sent is real. DREIF goes low on each write to
the transmit buffer and high after a transfer, when the buffer moves to
the shift register; RXCIF is set one cycle after DREIF rises; TXCIF one
cycle after RXCIF, once the shift register and transmit buffer are both
sent.

```wavedrom id=spi-buffer caption="Slave buffer mode, BUFWR = 0: the first byte sent is a dummy; 0x45, written while DREIF is low, is lost"
{"signal": [
  {"name": "SS#", "wave": "1.0.............1"},
  {"name": "Write DATA", "wave": "010...10.1010...."},
  {"name": "Write value", "wave": "x=x...=x.=x=x....", "data": ["0x43", "0x44", "0x45", "0x46"]},
  {"name": "DREIF", "wave": "1.0..1.0..1.01..."},
  {"name": "Data sent", "wave": "x..=.x.=..=..=..x", "data": ["dummy", "0x43", "0x44", "0x46"]}
 ],
 "config": {"hscale": 2}}
```

#### Data modes

MODE (CTRLB) selects SCK polarity and phase. Data is shifted out and
latched on opposite SCK edges. When the leading edge is rising, SCK
idles low; when falling, SCK idles high.

```table id=spi-modes caption="SPI transfer modes"
| MODE | Leading edge | Trailing edge | SCK idle |
|:-:|---|---|---|
| 0 | Rising, sample | Falling, setup | Low |
| 1 | Rising, setup | Falling, sample | Low |
| 2 | Falling, sample | Rising, setup | High |
| 3 | Falling, setup | Rising, sample | High |
```

```wavedrom id=spi-mode0 caption="SPI mode 0 (mode 2: SCK inverted)"
{"signal": [
  {"name": "SS#", "wave": "10.......1"},
  {"name": "SCK", "wave": "0.P......."},
  {"name": "MOSI", "wave": "x========x", "data": ["1", "2", "3", "4", "5", "6", "7", "8"]},
  {"name": "MISO", "wave": "z========z", "data": ["1", "2", "3", "4", "5", "6", "7", "8"]}
 ]}
```

```wavedrom id=spi-mode1 caption="SPI mode 1 (mode 3: SCK inverted): data changes on the leading edge"
{"signal": [
  {"name": "SS#", "wave": "10........1"},
  {"name": "SCK", "wave": "0.P......l."},
  {"name": "MOSI", "wave": "x.========x", "data": ["1", "2", "3", "4", "5", "6", "7", "8"]},
  {"name": "MISO", "wave": "z.========z", "data": ["1", "2", "3", "4", "5", "6", "7", "8"]}
 ]}
```

#### Events, interrupts

Event output: the SPI's SCK, which toggles only while the SPI is
enabled, in master mode and transmitting (idle level per MODE). No event
inputs. One interrupt vector, SPI, with the sources SSI (slave select
trigger), DRE (data register empty), TXC (transfer complete) and RXC
(receive complete) in buffer mode, or IF in normal mode. A request stays
active until its flag is cleared.

#### Registers

```table id=spi-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | DORD | R/W | 0 | Data order: 0 = MSb first, 1 = LSb first |
| 5 | MASTER | R/W | 0 | 0 = slave, 1 = master. Cleared (and IF set) when !{SS} is an input driven low in master mode with SSD = 0; write 1 again to resume master mode |
| 4 | CLK2X | R/W | 0 | Clock double: 1 = SCK frequency (after the prescaler) doubled in master mode |
| 2:1 | PRESC[1:0] | R/W | 0 | SCK rate in master mode (no effect in slave mode): 0x0 = DIV4: CLK_PER/4; 0x1 = DIV16: CLK_PER/16; 0x2 = DIV64: CLK_PER/64; 0x3 = DIV128: CLK_PER/128 |
| 0 | ENABLE | R/W | 0 | 1 = SPI enabled |
```

```table id=spi-ctrlb caption="CTRLB (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | BUFEN | R/W | 0 | Buffer mode enable: two receive buffers, one transmit buffer, separate transmit-complete and receive-complete flags |
| 6 | BUFWR | R/W | 0 | Buffer mode wait for receive: 0 = one transfer must complete before data is copied to the shift register (first byte sent is a dummy); 1 = a write while the SPI is enabled and !{SS} is high goes directly to the shift register |
| 2 | SSD | R/W | 0 | Slave select disable in master mode: 0 = !{SS} input low switches the master to slave mode; 1 = !{SS} does not affect master mode |
| 1:0 | MODE[1:0] | R/W | 0 | Transfer mode 0–3 (table above) |
```

```table id=spi-intctrl caption="INTCTRL (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXCIE | R/W | 0 | Receive complete interrupt enable (buffer mode; reads 0 otherwise), on RXCIF |
| 6 | TXCIE | R/W | 0 | Transfer complete interrupt enable (buffer mode; reads 0 otherwise), on TXCIF |
| 5 | DREIE | R/W | 0 | Data register empty interrupt enable (buffer mode; reads 0 otherwise), on DREIF |
| 4 | SSIE | R/W | 0 | Slave select trigger interrupt enable (buffer mode; reads 0 otherwise), on SSIF |
| 0 | IE | R/W | 0 | SPI interrupt enable when not in buffer mode, on IF |
```

```table id=spi-intflags caption="INTFLAGS (+0x03), reset 0x00, normal mode (BUFEN = 0)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | IF | R/W | 0 | Interrupt flag: a byte has been completely shifted in and out; also set when !{SS} (input) is driven low in master mode. Cleared when the vector executes, or by reading INTFLAGS with IF set and then accessing DATA |
| 6 | WRCOL | R/W | 0 | Write collision: DATA written before a complete byte was shifted out. Cleared by reading INTFLAGS with WRCOL set and then accessing DATA |
```

```table id=spi-intflags-buf caption="INTFLAGS (+0x03), reset 0x00, buffer mode (BUFEN = 1) [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXCIF | R/W | 0 | Receive complete: unread data in the receive buffer; 0 when empty. An interrupt routine must read DATA, or the interrupt repeats. Also cleared by writing 1 |
| 6 | TXCIF | R/W | 0 | Transfer complete: shift register empty and no new data in the transmit buffer. Cleared by writing 1 |
| 5 | DREIF | R/W | 0 | Data register empty: 1 = transmit buffer empty, 0 = it holds data not yet moved to the shift register. Cleared by writing DATA; an interrupt routine must write DATA or disable DREIE |
| 4 | SSIF | R/W | 0 | Slave select trigger: the SPI was a master and !{SS} was pulled low externally, so it is now a slave. Only with SSD = 0. Cleared by writing 1 |
| 0 | BUFOVF | R/W | 0 | Buffer overflow: receive buffer full (two bytes) and a third byte received in the shift register. Without transmit data, not set before a new transfer starts. Valid until DATA is read. Always write 0 here when writing INTFLAGS |

[1] Source conflict: the PDF says DREIF "is cleared after a Reset to indicate that the transmitter is ready", while its reset value is 0; kept as given.
```

```table id=spi-data caption="DATA (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DATA[7:0] | R/W | 0 | Write: byte to send (starts a transfer in master mode). Read: received byte; in buffer mode reads the second receive buffer and moves the first into the second |
```

#### Electrical characteristics

```table id=spi-char caption="SPI timing characteristics [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f~SCK~ | SCK clock frequency | Master | | | 10 | MHz |
| t~SCK~ | SCK period | Master | 100 | | | ns |
| t~SCKW~ | SCK high/low width | Master | | 0.5 × t~SCK~ | | ns |
| t~SCKR~ | SCK rise time | Master | | 2.7 | | ns |
| t~SCKF~ | SCK fall time | Master | | 2.7 | | ns |
| t~MIS~ | MISO setup to SCK | Master | | 10 | | ns |
| t~MIH~ | MISO hold after SCK | Master | | 10 | | ns |
| t~MOS~ | MOSI setup to SCK | Master | | 0.5 × t~SCK~ | | ns |
| t~MOH~ | MOSI hold after SCK | Master | | 1.0 | | ns |
| f~SSCK~ | Slave SCK clock frequency | Slave | | | 5 | MHz |
| t~SSCK~ | Slave SCK period | Slave | 4 × t~CLKPER~ | | | ns |
| t~SSCKW~ | SCK high/low width | Slave | 2 × t~CLKPER~ | | | ns |
| t~SSCKR~ | SCK rise time | Slave | | | 1600 | ns |
| t~SSCKF~ | SCK fall time | Slave | | | 1600 | ns |
| t~SIS~ | MOSI setup to SCK | Slave | 3.0 | | | ns |
| t~SIH~ | MOSI hold after SCK | Slave | t~CLKPER~ | | | ns |
| t~SSS~ | !{SS} setup to SCK | Slave | 21 | | | ns |
| t~SSH~ | !{SS} hold after SCK | Slave | 20 | | | ns |
| t~SOS~ | MISO setup to SCK | Slave | | 8.0 | | ns |
| t~SOH~ | MISO hold after SCK | Slave | | 13 | | ns |
| t~SOSS~ | MISO setup after !{SS} low | Slave | | 11 | | ns |
| t~SOSH~ | MISO hold after !{SS} low [2] | Slave | | 8.0 | | ns |

[1] Design guidance only, not production tested. t~CLKPER~ is the CLK_PER period.
[2] Source conflict: the table says "MISO hold after SS low"; Figure 32-7 places t~SOSH~ after !{SS} goes high.
```

Both diagrams show CPOL = 0 (CPOL = 1 inverts SCK); data is sampled on
the leading edge.

```wavedrom id=spi-master-timing caption="SPI master timing"
{"signal": [
  {"name": "SS#", "wave": "10.......1"},
  {"name": "SCK", "wave": "0.1.0.1.0.", "node": "..A.B.C.D"},
  {"name": "MISO", "wave": "x=..x..=.x", "data": ["MSb", "LSb"], "node": ".E..F"},
  {"name": "MOSI", "wave": "x=...=.=.x", "data": ["MSb", "", "LSb"], "node": ".G...H"}
 ],
 "edge": ["G<->A tMOS", "E<->A tMIS", "A<->F tMIH", "B<->H tMOH", "A<->B tSCKW", "A<->C tSCK"],
 "config": {"hscale": 2}}
```

```wavedrom id=spi-slave-timing caption="SPI slave timing"
{"signal": [
  {"name": "SS#", "wave": "10........1.", "node": ".A........B"},
  {"name": "SCK", "wave": "0..1.0.1.0..", "node": "...C.D.E.F"},
  {"name": "MOSI", "wave": "x.=..x..=x..", "data": ["MSb", "LSb"], "node": "..G..H"},
  {"name": "MISO", "wave": "z.=...=....z", "data": ["MSb", "LSb"], "node": "..I...L....K"}
 ],
 "edge": ["A<->I tSOSS", "D<->L tSOS", "A<->C tSSS", "G<->C tSIS", "C<->D tSSCKW", "C<->E tSSCK", "C<->H tSIH", "F<->B tSSH", "B<->K tSOSH"],
 "config": {"hscale": 2}}
```

### TWI: two-wire interface

An I²C- and SMBus 2.0-compatible two-wire interface (one instance,
TWI0) with independent master and slave units, each enabled and
configured separately. Speeds: Standard mode (Sm, 100 kHz) and Fast
mode (Fm, 400 kHz) with slew-rate-limited outputs, Fast mode plus
(Fm+, 1 MHz) with ×10 output drive; SMBus at 100 kHz with
configurable Layer 1 time-outs. The master supports multi-master
operation with full arbitration, including arbitration between a
Start/Repeated Start and a data bit; slave arbitration allows the SMBus
Address Resolution Protocol (ARP). The slave recognizes 7-bit and
general-call addresses in hardware (10-bit with software help), with
an address mask or a second address, or can hand every address to
software; address recognition works in every sleep mode, Power-Down
included, so an address match can wake the device. An input filter
suppresses bus noise. START, STOP, bus collisions, bus errors,
arbitration lost and clock hold are detected and flagged separately
for master and slave. Signals: SCL (serial clock) and SDA (serial
data), both digital I/O; the bus needs one pull-up R~P~ per line
(series resistors R~S~ optional).

In **dual mode** (DUALCTRL.ENABLE = 1) the slave uses an alternative
pair of pins (see PORTMUX), so master and slave run fully
independently and at the same time; the master's ENABLE turns on the
primary SCL/SDA pins and DUALCTRL.ENABLE the secondary ones, and
DUALCTRL holds the slave's own SDA hold time and Fm+ setting.

#### Initialization

Before enabling master or slave, set SDASETUP, SDAHOLD and (if used)
FMPEN in CTRLA, and, for alternate slave pins, DUALCTRL. **Master:**
write MBAUD first (TIMEOUT depends on it), then MCTRLA.ENABLE = 1 and,
on SMBus, a TIMEOUT. Without a TIMEOUT (I²C), the bus state must be
forced to idle by writing BUSSTATE = 0x1 in MSTATUS at a safe moment;
I²C does not say when a multi-master bus may be assumed idle, so after
all masters are enabled one supervising master should do a transfer
first: its STOP tells every bus state monitor that the bus is idle.
**Slave:** write SADDR, then SCTRLA.ENABLE = 1; the slave then waits
for a byte addressed to it.

#### Bus protocol

SCL and SDA are open-collector (wired-AND) lines. A transaction starts
with a START (S: SDA falls while SCL is high) and ends with a STOP
(P: SDA rises while SCL is high); a START not directly after a STOP is
a Repeated Start (Sr). SDA must be stable while SCL is high and may
change only while SCL is low (the TWI hardware ensures this). Packets
are 8 data bits, MSB first, plus an acknowledge bit: the receiver
pulls SDA low in the ninth clock for ACK and leaves it high for NACK.
After S, the master sends a 7-bit address and the R/W bit (0 = master
write, 1 = master read); the address, R/W and ACK form the address
packet, one per START (also with 10-bit addressing). A master write
may stop directly after the address packet; the master must end the
write when the slave NACKs a data byte. In a master read the slave
sends and the master ACKs each byte, ending with a NACK before the
STOP. A combined transaction is several reads/writes separated by Sr.
There is no limit on the number of data packets.

Any device may stretch the SCL low period (hold SCL low after seeing
it low): wake-up stretching (an AVR slave stretches only directly
before or after the ACK/NACK bit, since it need not wake for traffic
not addressed to it), periodic bit-level stretching (slows the whole
bus), and random byte-level stretching before or after ACK/NACK. Not
every I²C device supports stretching. A master may start only on an
idle bus; a master that fails to drive SDA high loses arbitration and
must wait for a STOP; slaves take no part. Arbitration between Sr and
a data bit, P and a data bit, or Sr and P is not allowed and needs
software handling. Clock synchronization: SCL is the wired-AND of all
masters' clocks; the longest low period sets SCL low time and the
shortest high period sets SCL high time.

```wavedrom id=twi-transaction caption="TWI transaction, 7-bit address: S, address packet, data packets, P"
{"signal": [
  {"name": "SDA", "wave": "10=.==.==.==1", "data": ["ADDRESS[6:0]", "R/W", "ACK", "DATA[7:0]", "ACK", "DATA[7:0]", "ACK/NACK"]},
  {"name": "SCL", "wave": "1.0.........1"}
 ],
 "config": {"hscale": 3}}
```

#### Bus state logic

While the master is enabled, the bus state logic (START/STOP
detectors, collision detection, inactive-bus time-out, bit counter)
tracks the bus in every sleep mode, Power-Down included; MSTATUS.BUSSTATE
reports it.

```table id=twi-busstate caption="Bus state transitions (MSTATUS.BUSSTATE)"
| State | Value | Entered when | Left when |
|---|---|---|---|
| UNKNOWN | 0b00 | System reset, or master enabled | Software writes BUSSTATE = 0x1 (→ IDLE); a STOP is detected (→ IDLE); inactive-bus time-out (→ IDLE) |
| IDLE | 0b01 | STOP, time-out, forced by software | External START (→ BUSY); writing MADDR issues S (→ OWNER) |
| BUSY | 0b11 | External START; arbitration lost | STOP or inactive-bus time-out (→ IDLE) |
| OWNER | 0b10 | This master issued S | STOP command (→ IDLE); collision / arbitration lost (→ BUSY until STOP) |
```

Once a known state is established, only a system reset or disabling
the master returns it to UNKNOWN. A Repeated Start changes the state
only if arbitration is lost while issuing it, which can only happen if
arbitration has been going on since the first START (two masters sent
exactly the same address and data before one issued Sr). The TWI
follows the I²C and SMBus electrical specifications and timing but is
not 100 % compliant; set the master's inactive-bus time-out to ensure
correct behavior.

#### Master operation

Byte-oriented, with separate read (RIF) and write (WIF) interrupt
flags, usable for polling, and status flags for ACK/NACK received, bus
error, arbitration lost, clock hold and bus state. While an interrupt
flag is set the master holds SCL low; clearing the flag (by the
accesses listed under MSTATUS) releases it. Most conditions are handled
automatically to keep the interrupt count low.

**Clock generation.** MBAUD sets T~LOW~ and T~HIGH~; T~RISE~ and
T~FALL~ come from the bus. T~FALL~ counts as part of T~LOW~ (wired-AND)
and can usually be taken as zero (set by the open-drain current limit
and bus impedance); T~RISE~ lies between T~LOW~ and T~HIGH~ until a high
level is detected. MBAUD must give f~SCL~ ≤ 100 kHz, 400 kHz or 1 MHz
for Sm, Fm or Fm+. The BAUD value times both SCL high and SCL low:

```equation
f~SCL~ = 1 / (T~LOW~ + T~HIGH~ + T~RISE~)
f~SCL~ = f~CLK_PER~ / (10 + 2 × BAUD + f~CLK_PER~ × T~RISE~)
```

**Address packet.** Writing MADDR (7-bit address and R/W in bit 0)
issues S and the address; on a busy bus the master first waits for
idle. Four outcomes, handled by software:

- **M1, arbitration lost or bus error during the address:** WIF and
  ARBLOST set (plus BUSERR for a bus error); SDA output disabled, SCL
  released; the master may do nothing until the bus is idle again.
- **M2, address not acknowledged:** WIF and RXACK set (RXACK is the
  physical ACK bit: no slave pulled it low); clock hold active.
- **M3, address ACKed, R/W = 0:** WIF set, RXACK cleared; clock hold
  active.
- **M4, address ACKed, R/W = 1:** the master receives the first data
  byte, then sets RIF and clears RXACK; clock hold active.

**Transmit (after M3).** Writing MDATA sends a byte and clears WIF; the
master watches for collisions and errors, and WIF is set again when
the byte is done, on ARBLOST or on BUSERR. RXACK is valid only if WIF
is set and neither ARBLOST nor BUSERR is, so check those first. RXACK =
0: the slave ACKed and is ready for more. On NACK the master must
either issue Sr (write MADDR) or a STOP (MCTRLB.MCMD = STOP). How an
I²C slave treats Sr is its own choice; for SMBus it is defined by the
higher protocol layers.

**Receive (after M4).** RIF is set with one byte received; the master
answers each byte with ACK or NACK (ACKACT, applied when MDATA is read
or a command is written). A NACK may not go out if arbitration is lost
during it; a collision sets ARBLOST.

**Quick command** (MCTRLA.QCEN = 1, SMBus): the R/W bit of the address
is the command (turn a function on/off, enter or leave standby); no
data is sent. After the slave's ACK, RIF or WIF is set according to
R/W, and the master accepts a STOP command in MCTRLB.MCMD (or an Sr by
writing MADDR). Frame: S, address, R/W, A, P.

**Smart mode** (MCTRLA.SMEN = 1): the master sends the acknowledge
action as soon as MDATA is read. Only active with ACKACT = ACK; with
ACKACT = NACK no NACK is generated on the MDATA read.

#### Slave operation

Byte-oriented, with a data interrupt flag (DIF) and an address/stop
flag (APIF), usable for polling, and status flags for ACK/NACK
received, clock hold, collision, bus error and direction. While a flag
is set the slave holds SCL low. With SCTRLA.PMEN = 1 the slave answers
every address (software address recognition).

**Address packet.** After a START the slave checks the address byte;
on a match it ACKs, stores the address in SDATA and sets APIF (a
general call also sets APIF); otherwise it does not ACK and waits for
the next START. A START directly followed by a STOP is illegal and sets
BUSERR. SSTATUS.DIR is the received R/W bit. Outcomes:

- **S1, accepted, R/W = 1 (master read):** SCL held low; if the slave
  ACKs, DIF is set to ask for transmit data; data, Sr or P may follow.
  After a NACK the slave waits for a new START and address match.
- **S2, accepted, R/W = 0 (master write):** SCL held low; after ACK the
  slave waits for data; data, Sr or P may follow. After NACK it waits
  for a new START and address match.
- **S3, collision:** the slave could not send a high level or NACK;
  COLL set, data and ACK outputs disabled, clock hold released; a
  START or Sr is accepted.
- **S4, STOP received:** APIF set with AP = 0 (STOP, not an address
  match); requires PIEN = 1.

**Receive.** When a byte arrives DIF is set and the slave must ACK or
NACK; after a NACK it must expect P or Sr. **Transmit.** The slave
writes SDATA to send; DIF is set when the byte is done; on a NACK from
the master it stops sending and expects P or Sr.

**Smart mode** (SCTRLA.SMEN = 1): reading or writing SDATA clears DIF
and the operation continues; without smart mode the slave always
waits for an SCMD command.

#### Interrupts, sleep and debug

Two vectors: **TWI slave** (DIF or APIF in SSTATUS) and **TWI master**
(RIF or WIF in MSTATUS); the conditions of a vector are ORed, so read
the status register to see which is present. The bus state logic and
the slave keep working in every sleep mode, Power-Down included; when
a slave detects a START in sleep, it stretches the clock during the
wake-up period until the system clock runs. The master stops in all
sleep modes. In break debug mode the TWI halts and ignores events
unless DBGCTRL.DBGRUN = 1.

#### Registers

```table id=twi-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4 | SDASETUP | R/W | 0 | SDA setup time on SDA out while the slave part is read: 0 = 4CYC: four clock cycles; 1 = 8CYC: eight clock cycles |
| 3:2 | SDAHOLD[1:0] | R/W | 0x0 | SDA hold time, see the SDA hold time table |
| 1 | FMPEN | R/W | 0 | Fast mode plus (1 MHz) for the TWI, or for the master in dual mode: 0 = disabled, 1 = enabled |
```

```table id=twi-sdahold caption="SDA hold time (CTRLA.SDAHOLD, DUALCTRL.SDAHOLD)"
| Value | Nominal hold time (ns) | Range across all corners (ns) | Description |
|---|---|---|---|
| 0x0 | 0 (off) | 0 | Hold time off |
| 0x1 | 50 | 36 to 131 | Backward-compatible setting |
| 0x2 | 300 | 180 to 630 | Meets the SMBus specification under typical conditions |
| 0x3 | 500 | 300 to 1050 | Meets the SMBus specification across all corners |
```

```table id=twi-dualctrl caption="DUALCTRL (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:2 | SDAHOLD[1:0] | R/W | 0x0 | SDA hold time of the slave in dual mode (same values as CTRLA.SDAHOLD); ignored while dual control is disabled |
| 1 | FMPEN | R/W | 0 | Fm+ (1 MHz) for the slave in dual mode; ignored while dual control is disabled |
| 0 | ENABLE | R/W | 0 | 1 = dual control enabled (slave on the alternative pins) |
```

```table id=twi-dbgctrl caption="DBGCTRL (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | 0 = halted in break debug mode, events ignored; 1 = keeps running while the CPU is halted |
```

```table id=twi-mctrla caption="MCTRLA (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RIEN | R/W | 0 | Read interrupt enable: master read interrupt when RIEN, RIF and CPU.SREG.I are all 1 |
| 6 | WIEN | R/W | 0 | Write interrupt enable: master write interrupt when WIEN, WIF and CPU.SREG.I are all 1 |
| 4 | QCEN | R/W | 0 | Quick command enable: the interrupt flag is set right after the slave ACKs the address; software then issues STOP (MCTRLB.MCMD) or Sr (MADDR) |
| 3:2 | TIMEOUT[1:0] | R/W | 0x0 | Inactive bus time-out; when non-zero, a bus inactive for longer sends the bus state logic to IDLE: 0x0 = DISABLED: time-out off (I²C); 0x1 = 50US: 50 µs (SMBus); 0x2 = 100US: 100 µs; 0x3 = 200US: 200 µs (times assume MBAUD set for 100 kHz) |
| 1 | SMEN | R/W | 0 | Master smart mode: the acknowledge action is sent right after MDATA is read |
| 0 | ENABLE | R/W | 0 | 1 = TWI master enabled |
```

```table id=twi-mctrlb caption="MCTRLB (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3 | FLUSH | R/W | 0 | Writing 1 disables and re-enables the master for one clock cycle, clearing its internal state and both MADDR and MDATA; writing 0 has no effect. Write MADDR before MDATA: if MDATA is written first the master transmits invalid data, which FLUSH avoids. |
| 2 | ACKACT | R/W | 0 | Acknowledge action, performed when MDATA is read or a command is written to MCMD: 0 = send ACK (the default for master read), 1 = send NACK. An ordinary bit, not a strobe. Not used while the master sends. |
| 1:0 | MCMD[1:0] | R/W | 0x0 | Command strobe, reads 0; see the master command table. ACKACT and MCMD can be written together. |
```

```table id=twi-mcmd caption="Master commands (MCTRLB.MCMD)"
| MCMD | Name | DIR [2] | Operation |
|---|---|---|---|
| 0x0 | NOACT | X | No action |
| 0x1 | REPSTART | X | Acknowledge action, then Repeated Start |
| 0x2 | RECVTRANS | 0 | Acknowledge action, then a byte read |
| ^ | ^ | 1 | Acknowledge action (none), then a byte send [1] |
| 0x3 | STOP | X | Acknowledge action, then a STOP condition |

[1] For a master being a sender, it will normally wait for new data written to MDATA. The acknowledge action bits and command bits can be written at the same time.
[2] Source conflict: DIR as printed in Table 25-4 (0 = byte read, 1 = byte send); MADDR's R/W bit and SSTATUS.DIR use 1 for a read.
```

MSTATUS is regarded as read-only in normal use: its flags are cleared
by writing MADDR, writing MDATA, or writing a command to MCTRLB.MCMD.

```table id=twi-mstatus caption="MSTATUS (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RIF | R/W | 0 | Read interrupt flag: set when a master byte read completes without arbitration loss or bus error (read triggered by reading MDATA or by writing MADDR with ADDR\[0\] = 1); needs the slave to have ACKed the address. Write 1 to clear (not needed normally); cleared like CLKHOLD. Drives the master read interrupt (RIEN). |
| 6 | WIF | R/W | 0 | Write interrupt flag: set when a master address or byte write completes, even with a bus error or arbitration loss. Write 1 to clear (not needed normally); cleared like CLKHOLD. Drives the master write interrupt (WIEN). |
| 5 | CLKHOLD | R/W | 0 | 1 = the master is holding SCL low. Cleared by writing 1 to it, writing MADDR, writing MDATA, reading MDATA while ACKACT is ACK or NACK, or writing a valid command to MCTRLB. |
| 4 | RXACK | R | 0 | Most recent acknowledge bit from the slave: 0 = ACK, 1 = NACK |
| 3 | ARBLOST | R/W | 0 | 1 = arbitration lost while sending a high data bit or NACK, or while issuing S/Sr. Write 1 to clear (not needed normally); cleared like CLKHOLD. Software must then abort or resend; both start with writing MADDR, which clears ARBLOST. |
| 2 | BUSERR | R/W | 0 | Bus error: an illegal bus condition, i.e. a protocol-violating S, Sr or P (S directly followed by P, for example). Write 1 to clear; writing MADDR also clears it. Treat like ARBLOST (bus ownership lost). Detected only with the bus state logic enabled and the system clock at least 4 × f~SCL~. |
| 1:0 | BUSSTATE[1:0] | R/W | 0x0 | Bus state: 0x0 = UNKNOWN, 0x1 = IDLE, 0x2 = OWNER: this TWI controls the bus, 0x3 = BUSY. Writing 0x1 forces IDLE; no other state can be forced. Unknown after reset, re-enable, or while the master is disabled. |
```

```table id=twi-mbaud caption="MBAUD (+0x06), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | BAUD[7:0] | R/W | 0x00 | Sets SCL high and low time (see clock generation); write while MCTRLA.ENABLE = 0 |
```

```table id=twi-maddr caption="MADDR (+0x07), reset 0x00 [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | ADDR[7:0] | R/W | 0x00 | Writing starts a START and address sequence; bit 0 is the R/W bit. Bus state UNKNOWN: WIF and BUSERR set, operation ends. Bus BUSY: waits for IDLE. Bus IDLE: issues S, copies ADDR into the shift register (MDATA), sends it and receives the ACK; then holds SCL low (CLKHOLD = 1) unless arbitration was lost, and sets WIF. Bus already OWNER: issues Sr instead (no wait for idle; after a read, the acknowledge action is sent before the Sr). For R/W = 1 the master receives one data byte before setting RIF. Writing clears all master flags (BUSERR, ARBLOST, RIF, WIF). Reading has no bus effect. |

[1] Source error: the text puts CLKHOLD in a "Master Setup register (TWIn.MSETUP)"; this TWI has no MSETUP, and CLKHOLD is in MSTATUS.
```

```table id=twi-mdata caption="MDATA (+0x08), reset 0x00 [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DATA[7:0] | R/W | 0x00 | Direct access to the master's shift register (bus data out and in). Not accessible during a byte transfer (writes blocked); valid only while the master holds SCL (CLKHOLD = 1); software that tracks the protocol state through interrupts or flags need not check CLKHOLD. **Write**: sends the byte (regardless of ACKACT) and receives the ACK, then sets WIF even on bus error or arbitration loss; in multi-master systems check ARBLOST, then abort or rewrite. All bits are clocked even after losing arbitration (only 1s are sent for the rest of the byte). Clears RIF and WIF; ARBLOST and BUSERR unchanged. **Read**: sends the ACKACT acknowledge, releases the clock for one byte, then sets RIF on success, or WIF instead if arbitration was lost sending NACK or a bus error occurred (RIF and WIF are never both set). Reading with ACKACT = ACK or NACK clears RIF and WIF; ARBLOST and BUSERR unchanged. |

[1] Source error: the text places ACKACT "in TWIn.MSTATUS"; the register summary and MCTRLB description put it in MCTRLB.
```

```table id=twi-sctrla caption="SCTRLA (+0x09), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | DIEN | R/W | 0 | Data interrupt enable: slave data interrupt when DIEN, DIF and CPU.SREG.I are all 1 |
| 6 | APIEN | R/W | 0 | Address/stop interrupt enable: slave address or stop interrupt when APIEN, APIF and CPU.SREG.I are all 1. The stop interrupt shares flag and vector with the address interrupt; SSTATUS.AP tells which occurred. |
| 5 | PIEN | R/W | 0 | Stop interrupt enable: 1 = APIF is also set on a STOP condition; needs the system clock at least 4 × f~SCL~ |
| 2 | PMEN | R/W | 0 | Address recognition mode: 1 = respond to every received address; 0 = match against SADDR (and SADDRMASK) |
| 1 | SMEN | R/W | 0 | Slave smart mode: an SCMD command or a read/write of SDATA clears the interrupt and the operation continues; when 0 the slave always waits for an SCMD command |
| 0 | ENABLE | R/W | 0 | 1 = TWI slave enabled |
```

```table id=twi-sctrlb caption="SCTRLB (+0x0A), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2 | ACKACT | R/W | 0 | Acknowledge action, performed when SDATA is read or written or an SCMD command is written: 0 = ACK: send ACK, 1 = NACK: send NACK. An ordinary bit, not a strobe. |
| 1:0 | SCMD[1:0] | R/W | 0x0 | Command strobe, reads 0; see the slave command table. ACKACT and SCMD can be written together. |
```

```table id=twi-scmd caption="Slave commands (SCTRLB.SCMD)"
| SCMD | Name | Used | DIR | Operation |
|---|---|---|---|---|
| 0x0 | NOACT | | X | No action |
| 0x1 | — | | X | Reserved |
| 0x2 | COMPTRANS | To complete a transaction | 0 | Acknowledge action, then wait for any S/Sr |
| ^ | ^ | ^ | 1 | Wait for any S/Sr |
| 0x3 | RESPONSE | In response to an address interrupt (APIF) | 0 | Acknowledge action, then receive the next byte |
| ^ | ^ | ^ | 1 | Acknowledge action, then a slave data interrupt |
| ^ | ^ | In response to a data interrupt (DIF) | 0 | Acknowledge action, then receive the next byte |
| ^ | ^ | ^ | 1 | A byte read operation, then the acknowledge action |
```

SSTATUS is regarded as read-only in normal use: its flags are cleared
by accessing SDATA or writing a command to SCTRLB.SCMD.

```table id=twi-sstatus caption="SSTATUS (+0x0B), reset 0x00 [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | DIF | R/W | 0 | Data interrupt flag: set when a slave byte transmit or receive completes without bus error; also set at the end of a transaction that failed by collision (see COLL). Write 1 to clear; also cleared by writing SDATA, reading SDATA, or writing a valid command to SCTRLB. Drives the slave data interrupt (DIEN). |
| 6 | APIF | R/W | 0 | Address/stop interrupt flag: set on a valid address match or a STOP condition (with PIEN). Write 1 to clear; also cleared like DIF. Drives the slave address/stop interrupt (APIEN), one vector for both. |
| 5 | CLKHOLD | R | 0 | 1 = the slave is holding SCL low; set with an address or data interrupt, cleared when that interrupt is cleared |
| 4 | RXACK | R | 0 | Most recent acknowledge bit from the master: 0 = ACK, 1 = NACK |
| 3 | COLL | R/W | 0 | Collision: the slave could not send a high data bit or NACK. The slave carries on but drives no low values on SDA (data and ACK outputs disabled); DIF is set at the end of the failed transaction, or APIF if the NACK collided after an address match. Write 1 to clear; cleared by any S/Sr. Meant for ARP systems; elsewhere a collision is a protocol violation, treat it as a bus error. |
| 2 | BUSERR | R/W | 0 | Bus error: protocol-violating S, Sr or P (S directly followed by P, for example). Write 1 to clear. Assume the packet is corrupt and restart by waiting for a new S. Detected by the master circuitry: needs MCTRLA.ENABLE = 1 and the system clock at least 4 × f~SCL~. |
| 1 | DIR | R | 0 | R/W bit of the last address packet received: 1 = master read in progress, 0 = master write |
| 0 | AP | R | 0 | What set APIF: 0 = STOP: a STOP condition, 1 = ADR: address detection |

[1] Source error: the APIF text refers to "the AIEN control bit in TWIn.CTRLA"; the bit is APIEN in SCTRLA.
```

```table id=twi-saddr caption="SADDR (+0x0C), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | ADDR[7:0] | R/W | 0x00 | Slave address, used with SADDRMASK; a recognized address sets APIF. ADDR[7:1] = 7-bit slave address; ADDR\[0\] = 1 enables general-call recognition. For 10-bit addresses only the first byte is matched (ADDR[7:1] = 0b11110aa, aa = address bits 9:8); software handles the second byte. |
```

```table id=twi-sdata caption="SDATA (+0x0D), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DATA[7:0] | R/W | 0x00 | Direct access to the slave's shift register (bus data out and in). Not accessible during a byte transfer (writes blocked); valid only while the slave holds SCL (CLKHOLD = 1); software that tracks the protocol state need not check CLKHOLD. With clock hold active, reading or writing triggers the bus operation given by SCTRLB.ACKACT and the access type. |
```

```table id=twi-saddrmask caption="SADDRMASK (+0x0E), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:1 | ADDRMASK[6:0] | R/W | 0x00 | ADDREN = 0: address mask, a 1 makes the matching SADDR bit always match. ADDREN = 1: a second slave address. |
| 0 | ADDREN | R/W | 0 | 1 = respond to the two addresses in SADDR and SADDRMASK; 0 = SADDRMASK masks SADDR |
```

#### Electrical characteristics

```table id=twi-char caption="TWI specifications [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f~SCL~ | SCL clock frequency | Maximum requires a 10 MHz system clock, so V~DD~ = 2.7 V to 5.5 V and T = -40 °C to 105 °C | 0 | | 1000 | kHz |
| V~IH~ | Input high voltage | | 0.7 × V~DD~ | | | V |
| V~IL~ | Input low voltage | | | | 0.3 × V~DD~ | V |
| V~HYS~ | Hysteresis of Schmitt-trigger inputs | | 0.1 × V~DD~ | | 0.4 × V~DD~ | V |
| V~OL~ | Output low voltage | I~load~ = 20 mA, Fast mode Plus | | | 0.2 × V~DD~ | V |
| ^ | ^ | I~load~ = 3 mA, normal mode, V~DD~ > 2 V | | | 0.4 | V |
| ^ | ^ | I~load~ = 3 mA, normal mode, V~DD~ ≤ 2 V | | | 0.2 × V~DD~ | V |
| I~OL~ | Low-level output current | f~SCL~ ≤ 400 kHz, V~OL~ = 0.4 V | 3 | | | mA |
| ^ | ^ | f~SCL~ ≤ 1 MHz, V~OL~ = 0.4 V | 20 | | | mA |
| C~B~ | Capacitive load for each bus line | f~SCL~ ≤ 100 kHz | | | 400 | pF |
| ^ | ^ | f~SCL~ ≤ 400 kHz | | | 400 | pF |
| ^ | ^ | f~SCL~ ≤ 1 MHz | | | 550 | pF |
| t~R~ | Rise time for both SDA and SCL | f~SCL~ ≤ 100 kHz | | | 1000 | ns |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 20 | | 300 | ns |
| ^ | ^ | f~SCL~ ≤ 1 MHz | | | 120 | ns |
| t~OF~ | Output fall time from V~IHmin~ to V~ILmax~, 10 pF < bus capacitance < 400 pF | f~SCL~ ≤ 100 kHz | | | 250 | ns |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 20 × (V~DD~ / 5.5 V) | | 250 | ns |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 20 × (V~DD~ / 5.5 V) | | 120 | ns |
| t~SP~ | Spikes suppressed by the input filter | | 0 | | 50 | ns |
| I~L~ | Input current for each I/O pin | 0.1 × V~DD~ < V~I~ < 0.9 × V~DD~ | | | 1 | µA |
| C~I~ | Capacitance for each I/O pin | | | | 10 | pF |
| R~P~ | Pull-up resistor value [2] | f~SCL~ ≤ 100 kHz | (V~DD~ - V~OL(max)~) / I~OL~ | | 1000 ns / (0.8473 × C~B~) | Ω |
| ^ | ^ | f~SCL~ ≤ 400 kHz | | | 300 ns / (0.8473 × C~B~) | Ω |
| ^ | ^ | f~SCL~ ≤ 1 MHz | | | 120 ns / (0.8473 × C~B~) | Ω |
| t~HD;STA~ | Hold time, (repeated) START condition | f~SCL~ ≤ 100 kHz | 4.0 | | | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 0.6 | | | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0.26 | | | µs |
| t~LOW~ | Low period of SCL | f~SCL~ ≤ 100 kHz | 4.7 | | | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 1.3 | | | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0.5 | | | µs |
| t~HIGH~ | High period of SCL | f~SCL~ ≤ 100 kHz | 4.0 | | | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 0.6 | | | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0.26 | | | µs |
| t~SU;STA~ | Setup time, repeated START condition | f~SCL~ ≤ 100 kHz | 4.7 | | | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 0.6 | | | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0.26 | | | µs |
| t~HD;DAT~ | Data hold time | f~SCL~ ≤ 100 kHz | 0 | | 3.45 | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 0 | | 0.9 | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0 | | 0.45 | µs |
| t~SU;DAT~ | Data setup time | f~SCL~ ≤ 100 kHz | 250 | | | ns |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 100 | | | ns |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 50 | | | ns |
| t~SU;STO~ | Setup time, STOP condition | f~SCL~ ≤ 100 kHz | 4 | | | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 0.6 | | | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0.26 | | | µs |
| t~BUF~ | Bus free time between a STOP and START condition | f~SCL~ ≤ 100 kHz | 4.7 | | | µs |
| ^ | ^ | f~SCL~ ≤ 400 kHz | 1.3 | | | µs |
| ^ | ^ | f~SCL~ ≤ 1 MHz | 0.5 | | | µs |

[1] Design guidance only, not production tested.
[2] R~P~ minimum is given for f~SCL~ ≤ 100 kHz only.
```

```wavedrom id=twi-timing caption="TWI timing"
{"signal": [
  {"name": "SCL", "wave": "01..0.1.0.1.0.1...", "node": ".A..B.C.D.F...E..."},
  {"name": "SDA", "wave": "1.0..=...=...0.1.0", "data": ["data", "data"], "node": "..J......K.....P.Q"}
 ],
 "edge": ["A<->J tSU;STA", "J<->B tHD;STA", "B<->C tLOW", "C<->D tHIGH", "D<->K tHD;DAT", "K<->F tSU;DAT", "E<->P tSU;STO", "P<->Q tBUF"],
 "config": {"hscale": 2}}
```

### CRCSCAN: cyclic redundancy check memory scan

Computes a CRC-16-CCITT checksum over a Flash section (the whole Flash,
the boot plus application code sections, or the boot section only) and
compares it with a pre-calculated 16-bit checksum stored in the last
two bytes of that section; a match sets STATUS.OK, a mismatch clears it
and can raise a non-maskable interrupt (NMI). An n-bit CRC detects any
single error burst up to n bits long, and a fraction 1 - 2^-n^ of
longer bursts.

```equation
CRC-16-CCITT: x^16^ + x^12^ + x^5^ + 1
```

The CRC reads the section byte by byte from byte 0, each byte shifted
in MSB first, updating the checksum per byte; the checksum register
starts at 0xFFFF. The check passes when the section's last bytes hold
the correct checksum:

```table id=crcscan-checksum caption="Placement of the pre-calculated checksum in Flash"
| Section checked | CHECKSUM[15:8] at | CHECKSUM[7:0] at |
|---|---|---|
| BOOT | FUSE.BOOTEND × 256 - 2 | FUSE.BOOTEND × 256 - 1 |
| BOOT and APPLICATION | FUSE.APPEND × 256 - 2 | FUSE.APPEND × 256 - 1 |
| Full Flash | FLASHEND - 1 | FLASHEND |
```

**Software start:** write CTRLB.SRC, then CTRLA.ENABLE = 1; the CRC
starts three cycles later, the CPU running meanwhile. **Scan at reset:**
FUSE.SYSCFG0.CRCSRC can make the CRCSCAN check the code before the
device leaves reset. On success normal execution starts with
CTRLA.ENABLE = 1, CTRLB.SRC showing the checked section(s) and
STATUS.OK = 1. On failure the CPU does not start and hangs executing no
code, with ENABLE = 1, SRC showing the section(s) and OK = 0, which the
debug interface can observe.

The CRC runs in priority mode: it has priority access to the Flash and
stalls the CPU until done, fetching a 16-bit word every third main
clock cycle (or as configured for the scan from startup).

**Interrupt.** One source, the NMI, on CRC failure: STATUS.OK is
cleared, and an NMI is generated when OK is cleared while CTRLA.NMIEN =
1. NMIEN can be set but only cleared by a system reset; the NMI request
stays active until a system reset and cannot be disabled; it fires
even with interrupts globally disabled.

**Sleep.** Halted in every sleep mode; resumes when the CPU wakes. If
sleep is entered during the three cycles after ENABLE is written, the
CRC does not start until the CPU wakes, and any interrupt handler runs
after the CRC has finished.

**Debug.** Any debugger access to the device (a peripheral or memory
read or write) disables the CRCSCAN. A scan that was busy restarts when
the debugger accesses an internal register or disconnects; meanwhile
STATUS.BUSY reads 1 but no section is checked. The debugger has
synchronized copies of the CRC status bits in its own register space,
readable without disabling the CRCSCAN; reading them ensures the
CRCSCAN is enabled. The debugger may write STATUS directly: BUSY = 0
stops the ongoing operation (it will not restart); BUSY = 1 starts a
single check with the CTRLB settings once the debugger allows it; while
BUSY = 1, CTRLB and CTRLA.NMIEN cannot change. OK = 0 can trigger the
NMI if NMIEN = 1 (after an NMI no CRCSCAN writes are allowed); OK = 1
makes OK read 1 while BUSY = 0. Debugger writes to CTRLA and CTRLB act
like CPU writes.

#### Registers

```table id=crcscan-ctrla caption="CTRLA (+0x00), reset 0x00; not writable after an NMI has been triggered"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RESET | R/W | 0 | Strobe: writing 1 clears CTRLA, CTRLB and STATUS one clock cycle later. With NMIEN = 0 writable whether busy or not, effective at once; with NMIEN = 1 writable only while BUSY = 0. |
| 1 | NMIEN | R/W | 0 | 1 = any CRC failure triggers an NMI. Can be written 1 only while BUSY = 0; cleared only by a system reset (not by RESET). |
| 0 | ENABLE | R/W | 0 | Writing 1 enables the CRCSCAN with the current settings and starts a check; stays 1 after the check completes, and writing 1 again starts a new check. Writing 0 disables it after the ongoing check reaches the end of its section (the preferred way to stop a continuous background check); a failure in that check is still detected and can cause the NMI. Reads 1 when execution starts after a scan at reset. Poll STATUS.BUSY to see whether a check is running. |
```

```table id=crcscan-ctrlb caption="CTRLB (+0x01), reset 0x00; not writable while busy or after an NMI has been triggered"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1:0 | SRC[1:0] | R/W | 0x0 | Section checked (sizes set by the BOOTEND and APPEND fuses): 0x0 = FLASH: entire Flash (boot, application code and application data); 0x1 = BOOTAPP: boot and application code sections; 0x2 = BOOT: boot section; 0x3 reserved. After a scan at reset, reads the section(s) that were checked. |
```

```table id=crcscan-status caption="STATUS (+0x02), reset 0x02; read-only"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | OK | R | 1 | 1 = the previous CRC completed successfully. Set by reset; cleared when the CRCSCAN is enabled; reads 0 while busy. While running continuously, assume OK until it fails or is stopped. |
| 0 | BUSY | R | 0 | 1 = a check is running; access to the control registers is limited meanwhile |
```

### CCL: configurable custom logic

Four look-up tables (LUT0–LUT3), each with three inputs IN[2:0], an
8-bit truth table, a synchronizer/filter and an edge detector, plus two
sequencers (SEQ0 on LUT0/LUT1, SEQ1 on LUT2/LUT3). The output of each
LUT can go to its LUTn-OUT pin (OUTEN overrides the PORT setting), to
the Event System (LUTn, a level event, asynchronous, length depending on
the configuration) and to the CCL interrupt. Pins: LUTn-OUT (digital
output), LUTn-IN[2:0] (digital inputs; LUTn-IN[2] can serve as
CLK_LUTn); their locations are in the I/O multiplexing table.

#### Enabling and enable protection

CTRLA.ENABLE enables the CCL; LUTnCTRLA.ENABLE enables each LUT.
SEQCTRLn.SEQSEL and every LUTnCTRLx bit except LUTnCTRLA.ENABLE are
enable-protected: writable only while the corresponding even LUT is
disabled (ENABLE = 0). They may be written in the same write that sets
ENABLE to 1, but not in the one that clears it.

#### Truth table and inputs

The LUT output is TRUTH[n], where n = IN[2]·4 + IN[1]·2 + IN[0] (IN = 000
selects TRUTH[0], 111 selects TRUTH[7]). Unused inputs are masked
(tied low). Each input is selected by INSEL0, INSEL1 (LUTnCTRLB) and
INSEL2 (LUTnCTRLC):

```table id=ccl-insel caption="LUT input sources (INSELx)"
| Value | Name | INSEL0 | INSEL1 | INSEL2 |
|---|---|---|---|---|
| 0x0 | MASK | None (masked, low) | < | < |
| 0x1 | FEEDBACK | Output of the pair's sequencer | < | < |
| 0x2 | LINK | Output of LUT(n+1) (LUT3 takes LUT0) | < | < |
| 0x3 | EVENTA | Event input A of the LUT | < | < |
| 0x4 | EVENTB | Event input B of the LUT | < | < |
| 0x5 | IO | Pin LUTn-IN0 | Pin LUTn-IN1 | Pin LUTn-IN2 |
| 0x6 | AC0 | AC0 output | < | < |
| 0x8 | USART | USART0 TXD | USART1 TXD | USART2 TXD |
| 0x9 | SPI0 | SPI0 MOSI | SPI0 MOSI | SPI0 SCK |
| 0xA | TCA0 | TCA0 WO0 | TCA0 WO1 | TCA0 WO2 |
| 0xC | TCB | TCB0 WO | TCB1 WO | TCB2 WO |
```

Values 0x7, 0xB and 0xD–0xF are reserved. SPI inputs work only in
SPI master mode; USART inputs only in asynchronous mode or synchronous
master mode.

Each LUT has two event users (LUTnA, LUTnB: "LUTn input x or clock
signal"), passed to the LUT without synchronization or edge detection.

#### Filter and edge detector

LUTnCTRLA.FILTSEL: DISABLE gives the plain combinational output (which
may glitch when inputs change); SYNCH synchronizes it to CLK_LUTn,
delaying it by two rising CLK_LUTn edges; FILTER passes only an input
that persists for more than two rising CLK_LUTn edges, delaying the
output by four. EDGEDET = 1 makes the edge detector output a pulse on a
rising edge (for a falling edge, invert the truth table); a filter option
(SYNCH or FILTER) must be enabled with it to avoid unpredictable
behavior. One clock cycle after a LUT is disabled its filter and edge
detector logic are cleared.

#### Sequencer

SEQCTRLn.SEQSEL makes a LUT pair's sequencer a gated D flip-flop, JK
flip-flop, gated D latch or RS latch. Its inputs come from the even and
odd LUTs (after their filter or edge detector, as configured); it is
clocked by the even LUT's CLK_LUTn, and the flip-flop output updates on
the rising clock edge. Disabling the even LUT clears the sequencer
asynchronously; the flip-flop reset R is held for one clock cycle. The
sequencer output drives the pair's even LUTn-OUT (block diagram).

```table id=ccl-seq caption="Sequencer functions (even LUT drives D, J or S; odd LUT drives G, K or R)"
| Function | Reset (R) | G / J / S | D / K / R | OUT |
|---|---|---|---|---|
| D flip-flop (DFF) | 1 | X | X | Clear |
| ^ | 0 | 1 | 1 | Set |
| ^ | 0 | 1 | 0 | Clear |
| ^ | 0 | 0 | X | Hold |
| JK flip-flop (JK) | 1 | X | X | Clear |
| ^ | 0 | 0 | 0 | Hold |
| ^ | 0 | 0 | 1 | Clear |
| ^ | 0 | 1 | 0 | Set |
| ^ | 0 | 1 | 1 | Toggle |
| D latch (DLATCH) [1] | | 0 | X | Hold |
| ^ | | 1 | 0 | Clear |
| ^ | | 1 | 1 | Set |
| RS latch (RS) [1] | | 0 | 0 | Hold |
| ^ | | 0 | 1 | Clear |
| ^ | | 1 | 0 | Set |
| ^ | | 1 | 1 | Forbidden |

[1] The latch tables (27-6, 27-7) have no R column: columns are G, D for DLATCH and S, R for RS.
```

#### Clocks, sleep and interrupts

The filter, edge detector and sequencer are clocked by CLK_PER unless
CLKSRC selects another CLK_LUTn. With CLKSRC = IN2, LUTn-IN[2] clocks
them and IN[2] reads as low in the truth table. Disable the CCL before
changing the clock source.

In Idle the truth tables keep working regardless of RUNSTDBY. In Standby
with RUNSTDBY = 0 the system clock is off, and a LUT whose filter, edge
detector or sequencer is enabled outputs 0; RUNSTDBY = 1 keeps the clock
running. With CLKSRC = IN2 the IN[2] clock keeps them running if the
peripheral supplying it runs in that sleep mode.

The single CCL interrupt vector is shared: INTn in INTFLAGS is set when
LUTn's output changes as INTMODEn in INTCTRL0 selects.

#### Registers

```table id=ccl-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | RUNSTDBY | R/W | 0 | 1 = CLK_PER kept running in Standby (ignored where CLK_PER is not needed) |
| 0 | ENABLE | R/W | 0 | 1 = CCL enabled |
```

```table id=ccl-seqctrl caption="SEQCTRL0 (+0x01), SEQCTRL1 (+0x02), reset 0x00, enable-protected"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:0 | SEQSEL[3:0] | R/W | 0x0 | Sequencer for LUT0/LUT1 (SEQCTRL0) or LUT2/LUT3 (SEQCTRL1): 0x0 = DISABLE; 0x1 = DFF: D flip-flop; 0x2 = JK: JK flip-flop; 0x3 = LATCH: D latch; 0x4 = RS: RS latch; other reserved |
```

```table id=ccl-intctrl0 caption="INTCTRL0 (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6, 5:4, 3:2, 1:0 | INTMODE3 … INTMODE0 | R/W | 0x0 | Interrupt sense for LUTn-OUT: 0x0 = INTDISABLE; 0x1 = RISING; 0x2 = FALLING; 0x3 = BOTH |
```

```table id=ccl-intflags caption="INTFLAGS (+0x07), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:0 | INT3 … INT0 | R/W | 0 | Set when LUTn's output changes as INTMODEn selects; write 1 to clear |
```

```table id=ccl-lutctrla caption="LUTnCTRLA (+0x08 + 4n, n = 0–3), reset 0x00, enable-protected except ENABLE"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | EDGEDET | R/W | 0 | 1 = edge detector enabled |
| 6 | OUTEN | R/W | 0 | 1 = LUT output to the LUTn-OUT pin, overriding the PORT configuration |
| 5:4 | FILTSEL[1:0] | R/W | 0x0 | Output filter: 0x0 = DISABLE; 0x1 = SYNCH: synchronizer; 0x2 = FILTER; 0x3 reserved |
| 3:1 | CLKSRC[2:0] | R/W | 0x0 | CLK_LUTn (the even LUT's also clocks the sequencer): 0x0 = CLKPER: CLK_PER; 0x1 = IN2: LUT input 2; 0x4 = OSC20M: 16/20 MHz oscillator before the prescaler; 0x5 = OSCULP32K: 32.768 kHz internal oscillator; 0x6 = OSCULP1K: 1.024 kHz (OSCULP32K ÷ 32); 0x2, 0x3, 0x7 reserved |
| 0 | ENABLE | R/W | 0 | 1 = LUT enabled |
```

```table id=ccl-lutctrlb caption="LUTnCTRLB (+0x09 + 4n), reset 0x00, enable-protected"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | INSEL1[3:0] | R/W | 0x0 | Input 1 source, see [LUT input sources](#ccl-insel) |
| 3:0 | INSEL0[3:0] | R/W | 0x0 | Input 0 source, see [LUT input sources](#ccl-insel) |
```

```table id=ccl-lutctrlc caption="LUTnCTRLC (+0x0A + 4n), reset 0x00, enable-protected"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:0 | INSEL2[3:0] | R/W | 0x0 | Input 2 source, see [LUT input sources](#ccl-insel) |
```

```table id=ccl-truth caption="TRUTHn (+0x0B + 4n), reset 0x00, enable-protected"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | TRUTH[7:0] | R/W | 0x00 | Truth table: bit k is the output for IN[2:0] = k |
```

### AC: analog comparator

One comparator (AC0). Its output is 1 when the positive input is above
the negative input, else 0. Positive input: pin AINP0–AINP3; negative
input: pin AINN0–AINN2 or DACREF, an internal 8-bit divider of the
reference selected in VREF:

```equation
V~DACREF~ = DACREF / 256 × V~REF~
```

The output can be inverted (MUXCTRL.INVERT, which also inverts it for
every peripheral using it and for the status), sent to the OUT pin
(CTRLA.OUTEN), used as an asynchronous event generator (always, while
the AC is enabled; the AC has no event inputs) and used for the COMP
interrupt. Hysteresis: none, small, medium or large (HYSMODE). Low-power
mode (LPMODE) cuts the comparator current at the cost of a longer
response time. Source error: the AC text names "the Mode bits (MODE)
in ACn.CTRLA", "the MUX Control A register (AC.MUXCTRLA)" and says
"The DAC has the following internal inputs"; the register descriptions
name LPMODE in CTRLA and the MUXCTRL register, and the inputs are the
AC's.

Setup: configure the input pins in PORT, select MUXPOS and MUXNEG,
optionally set OUTEN, then set ENABLE. The output may be invalid during
start-up: at most 2.5 µs for the AC itself; an internal reference
normally takes longer. To avoid the pin being tri-stated when the AC is
disabled, the OUT pin must be configured as output in PORTx.DIR.

Interrupt: STATUS.CMP is set when the output toggles as INTMODE
selects (both, negative or positive edge); INTCTRL.CMP enables the COMP
request, which stays active until CMP is cleared.

Sleep: in Idle the AC runs normally. In Standby it is off unless
RUNSTDBY = 1; then it keeps working (events, interrupt, output on pin)
without CLK_PER, but interrupts and status flags are not updated while
the clock is stopped. In Power-down the AC and its pin output are off.

#### Registers

```table id=ac-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RUNSTDBY | R/W | 0 | 1 = keep operating in Standby |
| 6 | OUTEN | R/W | 0 | 1 = comparator output on the OUT pin |
| 5:4 | INTMODE[1:0] | R/W | 0x0 | Interrupt edge: 0x0 = BOTHEDGE; 0x2 = NEGEDGE; 0x3 = POSEDGE; 0x1 reserved |
| 3 | LPMODE | R/W | 0 | 1 = low-power mode (lower current, longer reaction time) |
| 2:1 | HYSMODE[1:0] | R/W | 0x0 | Hysteresis: 0x0 = NONE; 0x1 = SMALL; 0x2 = MEDIUM; 0x3 = LARGE |
| 0 | ENABLE | R/W | 0 | 1 = AC enabled |
```

```table id=ac-muxctrl caption="MUXCTRL (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | INVERT | R/W | 0 | 1 = output inverted |
| 4:3 | MUXPOS[1:0] | R/W | 0x0 | Positive input: 0x0 = AINP0; 0x1 = AINP1; 0x2 = AINP2; 0x3 = AINP3 |
| 1:0 | MUXNEG[1:0] | R/W | 0x0 | Negative input: 0x0 = AINN0; 0x1 = AINN1; 0x2 = AINN2; 0x3 = DACREF: internal DAC reference |
```

```table id=ac-dacref caption="DACREF (+0x04), reset 0xFF"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DACREF[7:0] | R/W | 0xFF | DAC reference: V~DACREF~ = DACREF / 256 × V~REF~ |
```

```table id=ac-intctrl caption="INTCTRL (+0x06), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | CMP | R/W | 0 | 1 = comparator interrupt enabled |
```

```table id=ac-status caption="STATUS (+0x07), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4 | STATE | R | 0 | Current OUT state, after a three-cycle synchronizer delay |
| 0 | CMP | R/W | 0 | Interrupt flag; write 1 to clear |
```

#### Electrical characteristics

```table id=ac-char caption="Analog comparator characteristics"
| Symbol | Parameter | Conditions | Low-power mode disabled | < | < | Low-power mode enabled | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IN~ | Input voltage | | -0.2 | | V~DD~ | -0.2 | | V~DD~ | V |
| C~IN~ | Input pin capacitance | PD1 to PD6 | | 3.5 | | | 3.5 | | pF |
| ^ | ^ | PD7 | | 14 | | | 14 | | pF |
| V~OFF~ | Input offset voltage | 0.7 V < V~IN~ < V~DD~ - 0.7 V | -20 | ±5 | 20 | -30 | ±10 | 30 | mV |
| ^ | ^ | V~IN~ = -0.2 V (low-power mode enabled: 0 V) to V~DD~ | -40 | ±20 | 40 | -50 | ±30 | 50 | mV |
| I~L~ | Input leakage current | | | 5 | | | 5 | | nA |
| T~START~ | Start-up time | | | 1.3 | | | 1.3 | | µs |
| V~HYS~ | Hysteresis | HYSMODE = 0x0 | 0 | 0 | 10 | 0 | 0 | 10 | mV |
| ^ | ^ | HYSMODE = 0x1 | 0 | 10 | 30 | 0 | 10 | 30 | mV |
| ^ | ^ | HYSMODE = 0x2 | 10 | 30 | 90 | 5 | 25 | 90 | mV |
| ^ | ^ | HYSMODE = 0x3 | 20 | 60 | 150 | 12 | 50 | 190 | mV |
| t~PD~ | Propagation delay | 25 mV overdrive, V~DD~ ≥ 2.7 V | | 50 | | | 150 | | ns |
```

### ADC: analog-to-digital converter

A 10-bit successive-approximation ADC (one instance, ADC0) with a
single-ended input multiplexer referred to GND: the AIN pins, the AC0
DAC reference, the temperature sensor or GND. Results are right
adjusted in RES. The reference is V~DD~ (through a passive switch), the
external VREFA pin, or the internal reference from the VREF peripheral.
A sample-and-hold holds the input during conversion. Up to 64 samples
can be accumulated into one result; a sample delay (fixed or
automatically varied) moves the sampling frequency inside a burst away
from harmonic noise. A window comparator raises a flag or interrupt
when the result is below, above, inside or outside thresholds. The ADC
draws no power while CTRLA.ENABLE = 0; reference and input selections
take effect only once it is enabled.

Pins: AIN[n:0] analog inputs, VREFA external reference input. Both are
configured through PORT; disable the digital input buffer of a pin used
as an ADC input for best results.

#### Setup

Recommended order: RESSEL (CTRLA); optionally FREERUN (CTRLA);
optionally SAMPNUM (CTRLB); REFSEL (CTRLC, default the internal VREF
reference); PRESC (CTRLC); MUXPOS; optionally STARTEI (EVCTRL) with
the Event System configured; then ENABLE (CTRLA). A conversion then
starts from an event or by writing COMMAND.STCONV = 1.

#### Conversion

Writing STCONV = 1 starts a conversion; STCONV reads 1 until it is
done, then hardware clears it (single conversion mode). A channel
change during a conversion takes effect after it. The result is a
single sample or an accumulation, per SAMPNUM. At the end, INTFLAGS.RESRDY
is set (even with its interrupt disabled, so it can be polled); the
interrupt runs if INTCTRL.RESRDY = 1 and interrupts are globally
enabled. With EVCTRL.STARTEI = 1, any event routed to the ADC starts a
conversion: the input is edge sensitive, sets STCONV, and STCONV clears
at the end. In free-running mode the first conversion is started with
STCONV; each conversion then starts as soon as the previous one ends,
each setting RESRDY.

**Clock.** CLK_ADC = CLK_PER / 2 … 256 (PRESC), from any CLK_PER above
100 kHz. The ADC requires an input clock frequency between 50 kHz and
1.5 MHz for maximum resolution; below 10 bits it can be higher than
1.5 MHz to get a higher sample rate. The
prescaler runs from ENABLE = 1, is reset while ENABLE = 0, and is held
reset while no conversion is in progress, so a conversion (from STCONV
or an event) starts at the next rising CLK_ADC edge, a fixed delay in
CLK_PER cycles after the trigger:

```equation
StartDelay = PRESC~factor~ / 2 + 2
```

**Timing.** A normal conversion takes 13 CLK_ADC cycles; sample-and-hold
is two CLK_ADC cycles after the start. When it ends, RES holds the result
and RESRDY is set; RESRDY clears when RES is read or by writing 1 to it.
CTRLD.SAMPDLY (0–15 cycles) and SAMPCTRL.SAMPLEN (0–31 cycles) lengthen
sampling, for high-impedance sources, without slowing the conversion
clock; CTRLD.INITDLY (0–256 cycles) delays the first sample after
enabling the ADC, switching to an internal reference or waking from deep
sleep. In free-running mode, with STCONV = 1, the sample rate is f~S~:

```equation
SampleTime = (2 + SAMPDLY + SAMPLEN) / f~CLK_ADC~
f~S~ = f~CLK_ADC~ / (13 + SAMPDLY + SAMPLEN)
```

```wavedrom id=adc-single caption="ADC single conversion (13 CLK_ADC cycles; sample in cycle 2)"
{"signal": [
  {"name": "CLK_ADC", "wave": "lp............."},
  {"name": "ENABLE", "wave": "1.............."},
  {"name": "STCONV", "wave": "01............0"},
  {"name": "sample", "wave": "0.10..........."},
  {"name": "RESRDY", "wave": "0.............1"},
  {"name": "RES", "wave": "x.............=", "data": ["result"]}
 ],
 "config": {"hscale": 2}}
```

**Changing channel or reference.** MUXPOS and CTRLC.REFSEL go through a
temporary register: the selection follows writes continuously until a
conversion starts, is then locked to give enough sampling time, and
resumes following in the last CLK_ADC cycle before RESRDY is set. A
conversion starts on the rising CLK_ADC edge after STCONV is written 1.
Single conversion mode: select the channel before starting; it may be
changed one CLK_ADC cycle after writing STCONV = 1. Free-running mode:
select it before the first conversion; it may be changed one CLK_ADC
cycle after STCONV = 1, but since the next conversion has already
started, the next result still uses the old channel and later ones the
new. The ADC needs a settling time after an input change (see the ADC
electrical characteristics).

**Reference.** Inputs above the reference convert to 0x3FF. With the
external VREFA, set ADC0REFSEL in VREF.CTRLA to the internal level
closest to but above the applied voltage; for an external reference
above 4.3 V use ADC0REFSEL = 0x3. Source error: the text writes
"ADCnREFSEL[0:2] in the corresponding VREF.CTRLn register"; this device
has one field, ADC0REFSEL[2:0] in VREF.CTRLA. The internal reference comes from the
bandgap through an amplifier in VREF.

**Input circuit.** A source on an AIN pin always sees the pin
capacitance and leakage (I~IH~, I~IL~), selected or not; when selected
it must charge the sample-and-hold capacitor C~in~ (biased at V~DD~/2)
through the series resistance R~in~. The ADC is optimized for sources of
about 10 kΩ output impedance or less, for which sampling time is
negligible; higher impedances need longer sampling.

**Result.** For a 10-bit conversion:

```equation
RES = 1023 × V~IN~ / V~REF~
```

Inputs above the reference give 0x3FF, below 0 V 0x000; accumulated
results never exceed 0xFFC0 (64 × 0x3FF). The data format in ADC and
digital accumulation is 1's complement, where 0x0000 represents the
zero and 0xFFFF represents the largest number (full scale).
Unconfirmed: as printed; "1's complement" does not match the mapping
given in the same sentence. With RESSEL = 1 each conversion is
truncated to its 8 MSbs before accumulation or storage.

**Temperature sensor.** Select the internal reference at 1.1 V in VREF
and REFSEL = 0x0 (INTERNAL); select MUXPOS = TEMPSENSE (this enables the
sensor); set INITDLY ≥ 32 µs × f~CLK_ADC~, SAMPLEN ≥ 32 µs × f~CLK_ADC~
and SAMPCAP = 1; then convert. The output is linear in temperature but
varies between devices; production test stores a gain/slope correction
in SIGROW.TEMPSENSE0 (unsigned) and an offset correction in
SIGROW.TEMPSENSE1 (signed). Temperature in kelvin, with RES the
conversion result (an intermediate product needs more than 16 bits; add
0x80 before the shift to round):

```equation
T(K) = ((RES - TEMPSENSE1) × TEMPSENSE0 + 0x80) >> 8
```

**Window comparator.** Set WINLT and/or WINHT, optionally enable
INTCTRL.WCOMP, then select the mode with a non-zero CTRLE.WINCM. The
comparison is made on the (accumulated) result after the last sample of
an accumulation, so the flag is raised once per conversion.

**Events.** Input: start conversion (STARTEI). Output: a result-ready
event, a one-clock pulse whenever a new result is in RES; it is always
generated while the ADC is enabled (see EVSYS.USERn for the user input). Source error: the text
refers to EVSYS.ASYNCUSERn; this event system's user registers are
USERn.

**Interrupts.** RESRDY (result ready: a result is in RES) and WCOMP
(window comparator, as WINCM selects); flags in INTFLAGS, enables in
INTCTRL. A request stays active until its flag is cleared.

**Sleep.** In standby the ADC is off unless CTRLA.RUNSTBY = 1; then it
keeps running, ongoing conversions complete and interrupts run as
configured. In standby a conversion must be started by an event, or the
ADC must be free running with the first conversion started by software
before sleep; the peripheral clock is requested when needed and turned
off after the conversion. On an event the rising edge sets STCONV and
the conversion starts; at the end RESRDY is set and STCONV cleared. The
reference and supplies need time to settle when activated in standby:
set a non-zero INITDLY. In power-down no conversion is possible: an
ongoing conversion is halted and resumed on wake-up; RESRDY is then set
at its end but RES is invalid.

**Debug.** With DBGCTRL.DBGRUN = 0 the ADC halts in break debug mode
and ignores events.

**16-bit registers.** RES, WINLT and WINHT are read and written as
16-bit values through the shared TEMP register (see the CPU's 16-bit
access rule): low byte at the register's offset, high byte at offset + 1.

**Parameter definitions** (used in the ADC characteristics): offset
error, the deviation of the first transition (0x000 to 0x001) from the
ideal 0.5 LSB; gain error, after offset correction, the deviation of the
last transition (0x3FE to 0x3FF) from the ideal 1.5 LSB below full
scale; INL, after offset and gain correction, the largest deviation of
any transition from ideal; DNL, the largest deviation of a code width
from 1 LSB; quantization error, always ±0.5 LSB; absolute accuracy, the
largest deviation of any unadjusted transition from ideal, ideally
±0.5 LSB. An ideal n-bit ADC converts GND to V~REF~ linearly in 2^n^
steps, codes 0 to 2^n^ - 1.

#### Registers

```table id=adc-ctrla caption="CTRLA (+0x00), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RUNSTBY | R/W | 0 | Run in standby sleep mode |
| 2 | RESSEL | R/W | 0 | Resolution: 0 = 10-bit, results accumulated or stored in RES as is; 1 = 8-bit, each result truncated to its 8 MSbs (2 LSbs discarded) before accumulation or storage |
| 1 | FREERUN | R/W | 0 | 1 = free-running: the first conversion starts by writing STCONV = 1, each next one as soon as the previous completes (signalled by RESRDY) |
| 0 | ENABLE | R/W | 0 | 0 = ADC disabled, 1 = enabled |
```

```table id=adc-ctrlb caption="CTRLB (+0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | SAMPNUM[2:0] | R/W | 0x0 | Consecutive samples accumulated into RES per conversion: 0x0 = NONE: no accumulation; 0x1 = ACC2; 0x2 = ACC4; 0x3 = ACC8; 0x4 = ACC16; 0x5 = ACC32; 0x6 = ACC64; 0x7 reserved |
```

```table id=adc-ctrlc caption="CTRLC (+0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | SAMPCAP | R/W | 0 | Sample capacitance (sets input impedance): 0 = recommended for references below 1 V; 1 = reduced sampling capacitance, recommended for higher references |
| 5:4 | REFSEL[1:0] | R/W | 0x0 | Reference: 0x0 = INTERNAL (from VREF); 0x1 = VDD; 0x2 = VREFA: external pin; other reserved |
| 2:0 | PRESC[2:0] | R/W | 0x0 | CLK_ADC = CLK_PER divided by: 0x0 = DIV2; 0x1 = DIV4; 0x2 = DIV8; 0x3 = DIV16; 0x4 = DIV32; 0x5 = DIV64; 0x6 = DIV128; 0x7 = DIV256 |
```

```table id=adc-ctrld caption="CTRLD (+0x03), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:5 | INITDLY[2:0] | R/W | 0x0 | Delay before the first sample after enabling the ADC, changing to an internal reference, or waking from deep sleep for a measurement, so reference and MUXes are ready: 0x0 = DLY0; 0x1 = DLY16; 0x2 = DLY32; 0x3 = DLY64; 0x4 = DLY128; 0x5 = DLY256 (CLK_ADC cycles); other reserved |
| 4 | ASDV | R/W | 0 | Automatic sampling delay variation: 0 = ASVOFF; 1 = ASVON: SAMPDLY increments by one after each sample, wrapping 0xF to 0x0, to randomize the sampling instant and avoid standing frequency components |
| 3:0 | SAMPDLY[3:0] | R/W | 0x0 | Delay between consecutive samples, in CLK_ADC cycles (0–15), to move the sampling frequency during accumulation away from periodic noise; the sampling capacitor stays open during the delay |
```

```table id=adc-ctrle caption="CTRLE (+0x04), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | WINCM[2:0] | R/W | 0x0 | Window comparator mode (RESULT = the 16-bit accumulated result): 0x0 = NONE: no comparison; 0x1 = BELOW: RESULT < WINLT; 0x2 = ABOVE: RESULT > WINHT; 0x3 = INSIDE: WINLT < RESULT < WINHT; 0x4 = OUTSIDE: RESULT < WINLT or RESULT > WINHT; other reserved |
```

```table id=adc-sampctrl caption="SAMPCTRL (+0x05), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4:0 | SAMPLEN[4:0] | R/W | 0x00 | Extends sampling by this many CLK_ADC cycles beyond the default two, for higher-impedance sources; conversion time grows accordingly |
```

```table id=adc-muxpos caption="MUXPOS (+0x06), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4:0 | MUXPOS[4:0] | R/W | 0x00 | Single-ended input; a change during a conversion takes effect when it completes: 0x00–0x0F = AIN0–AIN15: ADC input pin 0–15; 0x1C = DACREF0: DAC reference in AC0; 0x1E = TEMPSENSE: temperature sensor; 0x1F = GND; 0x10–0x1B, 0x1D reserved |
```

```table id=adc-command caption="COMMAND (+0x08), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | STCONV | R/W | 0 | Write 1 to start a single conversion (in free-running mode, the first). Reads 1 while a conversion is in progress; cleared by hardware when it completes |
```

```table id=adc-evctrl caption="EVCTRL (+0x09), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | STARTEI | R/W | 0 | 1 = an incoming event starts a conversion |
```

```table id=adc-intctrl caption="INTCTRL (+0x0A), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | WCOMP | R/W | 0 | Window comparator interrupt enable |
| 0 | RESRDY | R/W | 0 | Result ready interrupt enable |
```

```table id=adc-intflags caption="INTFLAGS (+0x0B), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | WCOMP | R/W | 0 | Set at the end of a conversion whose result matches WINCM. Cleared by writing 1 or by reading RES; writing 0 has no effect |
| 0 | RESRDY | R/W | 0 | Set when a conversion completes and a new result is ready. Cleared by writing 1 or by reading RES; writing 0 has no effect |
```

```table id=adc-dbgctrl caption="DBGCTRL (+0x0C), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DBGRUN | R/W | 0 | 0 = halted in break debug mode, events ignored; 1 = keeps running while the CPU is halted |
```

```table id=adc-temp caption="TEMP (+0x0D), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | TEMP[7:0] | R/W | 0x00 | Temporary register for single-cycle 16-bit access to RES, WINLT and WINHT (one shared by all) |
```

```table id=adc-res caption="RES (+0x10, 16-bit: RESL, RESH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | RES[15:0] | R | 0x0000 | Conversion or accumulated result, right adjusted (10-bit ADC output ADC[9:0]; max 0x3FF per sample, 0xFFC0 after 64 accumulations) |
```

```table id=adc-winlt caption="WINLT (+0x12, 16-bit: WINLTL, WINLTH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | WINLT[15:0] | R/W | 0x0000 | Window comparator low threshold, compared with the accumulated value (not each sample) |
```

```table id=adc-winht caption="WINHT (+0x14, 16-bit: WINHTL, WINHTH), reset 0x0000"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:0 | WINHT[15:0] | R/W | 0x0000 | Window comparator high threshold, compared with the accumulated value |
```

```table id=adc-calib caption="CALIB (+0x16), reset 0x01"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 0 | DUTYCYC | R/W | 1 | ADC clock duty cycle: 0 = 50 %, required when CLK_ADC > 1.5 MHz (which needs V~DD~ ≥ 2.7 V); 1 = 25 % (high 25 %, low 75 %), required when CLK_ADC ≤ 1.5 MHz |
```

#### Electrical characteristics

Internal-reference conditions: V~DD~ = 1.8 V to 5.5 V, T = -40 °C to
125 °C, DUTYCYC = 25 %, CLK~ADC~ = 13 × f~ADC~, SAMPCAP = 10 pF for the
0.55 V reference and 5 pF for V~REF~ ≥ 1.1 V; valid for all allowed
combinations of V~REF~ and sample rate unless noted. The reference
setting and f~ADC~ must meet the clock, timing and input range tables.

```table id=adc-range caption="Power supply, reference and input range"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~DD~ | Supply voltage | CLK~ADC~ ≤ 1.5 MHz | 1.8 | | 5.5 | V |
| ^ | ^ | CLK~ADC~ > 1.5 MHz | 2.7 | | 5.5 | V |
| V~REF~ | Reference voltage | REFSEL = internal reference | 0.55 | | V~DD~ - 0.5 | V |
| ^ | ^ | REFSEL = external reference | 1.1 | | V~DD~ | V |
| ^ | ^ | REFSEL = V~DD~ | 1.8 | | 5.5 | V |
| C~IN~ | Input capacitance | SAMPCAP = 5 pF | | 5 | | pF |
| ^ | ^ | SAMPCAP = 10 pF | | 10 | | pF |
| V~IN~ | Input voltage range | | 0 | | V~REF~ | V |
| I~BAND~ | Input bandwidth | 1.1 V ≤ V~REF~ | | | 57.5 | kHz |
```

```table id=adc-clock caption="Clock and timing characteristics [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f~ADC~ | Sample rate | 1.1 V ≤ V~REF~ | 15 | | 115 | ksps |
| ^ | ^ | 1.1 V ≤ V~REF~, 8-bit resolution | 15 | | 150 | ksps |
| ^ | ^ | V~REF~ = 0.55 V, 10 bits | 7.5 | | 20 | ksps |
| CLK~ADC~ | Clock frequency | V~REF~ = 0.55 V, 10 bits | 100 | | 260 | kHz |
| ^ | ^ | 1.1 V ≤ V~REF~, 10 bits | 200 | | 1500 | kHz |
| ^ | ^ | 1.1 V ≤ V~REF~, 8-bit resolution | 200 | | 2000 | kHz |
| T~s~ | Sampling time | | 2 | 2 | 33 | CLK~ADC~ cycles |
| T~CONV~ | Conversion time (latency) | Sampling time = 2 CLK~ADC~ | 8.7 | | 50 | µs |
| T~START~ | Start-up time | Internal V~REF~ | | 22 | | µs |

[1] Design guidance only, not production tested.
```

```table id=adc-acc-int caption="Accuracy, internal reference or V~DD~ as reference [1] [3]"
| Symbol | Parameter | Reference | Conditions | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| Res | Resolution | | | | 10 | | bit |
| INL | Integral nonlinearity | Internal, V~REF~ = 0.55 V | f~ADC~ = 7.7 ksps | | 1.0 | 3.0 | LSB |
| ^ | ^ | Internal or V~DD~ | f~ADC~ = 15 ksps | | 1.0 | 3.0 | LSB |
| ^ | ^ | Internal or V~DD~, 1.1 V ≤ V~REF~ | f~ADC~ = 77 ksps | | 1.0 | 3.0 | LSB |
| ^ | ^ | ^ | f~ADC~ = 115 ksps | | 1.2 | 3.0 | LSB |
| DNL | Differential nonlinearity [2] | Internal, V~REF~ = 0.55 V | f~ADC~ = 7.7 ksps | | 0.6 | 1.3 | LSB |
| ^ | ^ | Internal, V~REF~ = 1.1 V | f~ADC~ = 15 ksps | | 0.4 | 1.2 | LSB |
| ^ | ^ | Internal or V~DD~, 1.5 V ≤ V~REF~ | f~ADC~ = 15 ksps | | 0.4 | 1.0 | LSB |
| ^ | ^ | Internal or V~DD~, 1.1 V ≤ V~REF~ | f~ADC~ = 77 ksps | | 0.4 | 1.0 | LSB |
| ^ | ^ | Internal, 1.1 V ≤ V~REF~ | f~ADC~ = 115 ksps | | 0.5 | 1.6 | LSB |
| ^ | ^ | V~DD~, 1.8 V ≤ V~REF~ | f~ADC~ = 115 ksps | | 0.9 | 2.0 | LSB |
| E~ABS~ | Absolute accuracy | Internal, V~REF~ = 1.1 V | T = 0 °C to 105 °C, V~DD~ = 1.8 V to 3.6 V | | < 10 | 30 | LSB |
| ^ | ^ | ^ | V~DD~ = 1.8 V to 3.6 V | | < 15 | 40 | LSB |
| ^ | ^ | V~DD~ | | | 2.5 | 5 | LSB |
| ^ | ^ | Internal | | | < 35 | 65 | LSB |
| E~GAIN~ | Gain error | Internal, V~REF~ = 1.1 V | T = 0 °C to 105 °C, V~DD~ = 1.8 V to 3.6 V | -25 | ±15 | 25 | LSB |
| ^ | ^ | ^ | V~DD~ = 1.8 V to 3.6 V | -35 | ±20 | 35 | LSB |
| ^ | ^ | V~DD~ | | -1 | 2 | 4 | LSB |
| ^ | ^ | Internal | | -60 | ±35 | 60 | LSB |
| E~OFF~ | Offset error | Internal, V~REF~ = 0.55 V | | -5 | -1 | 2 | LSB |
| ^ | ^ | Internal, 1.1 V ≤ V~REF~ | | -4 | -0.5 | 2 | LSB |

[1] Design guidance only, not production tested.
[2] A DNL error of less than or equal to 1 LSB ensures a monotonic transfer function with no missing codes.
[3] Reference setting and f~ADC~ must fulfill the specification in "Clock and Timing Characteristics" and "Power Supply, Reference, and Input Range" tables.
```

External-reference accuracy was characterized with V~REF~ = 1.8 V at
V~DD~ = 1.8 V to 5.5 V, 2.6 V at 2.7 V to 5.5 V, and 4.096 V and 4.3 V at
4.5 V to 5.5 V; V~DD~ = 1.8 V to 5.5 V, T = -40 °C to 125 °C,
DUTYCYC = 25 %, CLK~ADC~ = 13 × f~ADC~, SAMPCAP = 5 pF.

```table id=adc-acc-ext caption="Accuracy, external reference [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| Res | Resolution | | | 10 | | bit |
| INL | Integral nonlinearity | f~ADC~ = 15 ksps | | 0.9 | 3.5 | LSB |
| ^ | ^ | f~ADC~ = 77 ksps | | 0.9 | 3.5 | LSB |
| ^ | ^ | f~ADC~ = 115 ksps | | 1.2 | 3.5 | LSB |
| DNL | Differential nonlinearity [2] | f~ADC~ = 15 ksps | | 0.2 | 1.0 | LSB |
| ^ | ^ | f~ADC~ = 77 ksps | | 0.4 | 1.0 | LSB |
| ^ | ^ | f~ADC~ = 115 ksps | | 0.8 | 2.0 | LSB |
| E~ABS~ | Absolute accuracy | f~ADC~ = 15, 77 or 115 ksps | | 2 | 5 | LSB |
| E~GAIN~ | Gain error | f~ADC~ = 15, 77 or 115 ksps | -1 | 2 | 4 | LSB |
| E~OFF~ | Offset error | | -4 | -0.5 | 2 | LSB |

[1] Design guidance only, not production tested.
[2] DNL ≤ 1 LSB ensures a monotonic transfer function with no missing codes.
```

### UPDI: unified program and debug interface

The UPDI programs and debugs the device over one dedicated pin (UPDI; see [pin functions](#pin-functions)). It can program
every NVM section (Flash, EEPROM, fuses, lock bits, user row) and access
the whole data space (SRAM, I/O, NVM controller) through the bus matrix.
The Asynchronous System Interface (ASI) gives the debugger direct access
to on-chip debug, NVM and system-management functions without bus
access. Its registers (the CS space below) are reachable only through
UPDI instructions, never from the CPU.

Block diagram: the UPDI PHY (one-wire half-duplex UART on the UPDI pad)
feeds the UPDI access layer, which masters the bus matrix; the ASI
connects to the system management, NVM and OCD blocks.

#### Clocks

The PHY layer runs on its own clock from an internal oscillator; the
access (ACC) layer runs on the system clock, with a synchronizer between
them. The UPDI clock is selected by ASI_CTRLA.UPDICLKSEL; after enable
(and after every BREAK) it is 4 MHz.

#### Physical layer

UART-based, fixed frame format, auto-baud. Communication is always
started by the debugger, and every instruction is preceded by a SYNCH
character from which the UPDI takes the baud rate for that instruction
(receive and transmit). There is no baud-rate register; the receiver
samples the start bit and takes a majority vote of the middle samples,
with fractional baud counting.

```table id=updi-frames caption="UPDI frames"
| Frame | Content |
|---|---|
| Data | 1 start bit (0), 8 data bits LSB first, 1 even parity bit, 2 stop bits (1). A wrong parity or stop bit is an error; CTRLA.PARD = 1 ignores parity. |
| IDLE | 12 high bits (line idle) |
| BREAK | 12 low bits: resets the UPDI to its default state; used for error recovery |
| SYNCH | 0x55 as a data frame: sets the baud rate. Expected before every instruction and after a BREAK. |
| ACK | 0x40, sent by the UPDI when an ST or STS has crossed the synchronization boundary and gained bus access; the debugger may then send the next frame |
```

```wavedrom id=updi-frame caption="UPDI data frame (SYNCH 0x55 shown)"
{"signal": [
  {"name": "UPDI", "wave": "1010101010.1.", "node": ".A.........B."}
 ],
 "edge": ["A<->B start bit, D0…D7 LSB first, even parity; then two stop bits"],
 "config": {"hscale": 2}}
```

```table id=updi-baud caption="Recommended baud rate per UPDI clock"
| UPDICLKSEL | UPDI clock | Max. baud rate | Min. baud rate | Recommended BREAK length |
|---|---|---|---|---|
| 0x1 | 16 MHz | 0.9 Mbps | 0.300 kbps | 6.15 ms |
| 0x2 | 8 MHz | 450 kbps | 0.150 kbps | 12.30 ms |
| 0x3 (default) | 4 MHz | 225 kbps | 0.075 kbps | 24.60 ms |
```

Receiver baud-rate error (9 data + parity bits): R~slow~ = 96.39 %,
R~fast~ = 104.76 %, maximum total error +4.76 %/−3.61 %, recommended
maximum receiver error ±1.5 %.

**BREAK.** Resets the UPDI from any state, so no SYNCH precedes it. It is
sampled at the last valid baud rate; since that may be wrong after an
error, a BREAK should last 12 bit times at the lowest baud rate (8192
CLK_PER cycles; table above). Send two consecutive BREAKs: a single BREAK
during an ongoing receive or transmit is not detected but causes a frame
error (RX) or contention error (TX) that aborts the operation, and the
next BREAK is then detected. A BREAK while idle is detected at once.

**Enable.** The UPDI pad is an input with pull-up. A debugger that sees
the pull-up drives the line low for T~Deb0~; the UPDI detects the falling
edge, starts its clock and itself holds the line low (T~UPDI~) until the
clock is stable, then releases it. When the line is high the debugger
must start the first SYNCH within T~DebZ~, or the UPDI disables itself
and the sequence must be repeated. The UPDI also disables itself when
the first SYNCH is too short or too long to be valid (see baud rates
above). After the SYNCH the first instruction may follow.

```table id=updi-enable caption="UPDI enable sequence timing"
| Interval | Meaning | Min | Max |
|---|---|---|---|
| T~RES~ | Handshake/BREAK, UPDI ignores the input | 10 µs | 200 µs |
| T~UPDI~ | UPDI drives the line low until its clock is ready | 10 µs | 200 µs |
| T~Deb0~ | Debugger drives the line low | 0.2 µs | 1 µs |
| T~DebZ~ | Line released until the SYNCH start bit [1] | 200 µs | 14000 µs |

[1] Source conflict: Table 32-30 gives 14000 µs; the text says the UPDI disables itself when a SYNCH is not sent within 13.5 ms.
```

**Disable.** End every session by writing CTRLB.UPDIDIS = 1: this resets
the UPDI including all decoded KEYs and drops its oscillator request.
Otherwise the UPDI and the oscillator request stay enabled and raise the
power consumption.

**Error handling.** Parity, frame, contention, clock-recovery and
access-timeout errors put the UPDI in an error state in which it ignores
everything but a BREAK. Recovery: send a BREAK (this also resets
ASI_CTRLA.UPDICLKSEL to 4 MHz, which changes the baud-rate range), send
a SYNCH at the desired baud rate, read STATUSB.PESIG with LDCS (reading
clears it); the UPDI then accepts the next SYNCH and instruction.

**Direction change.** When switching from receive to transmit the UPDI
inserts a guard time of CTRLA.GTVAL IDLE bits (at the current baud rate)
before its start bit. This is the minimum wait the debugger sees; the
maximum is the timeout (65536 UPDI clock cycles). With a slow system
clock, synchronization plus bus access may make the wait longer. Do not
set GTVAL = 0x7 (no IDLE bits).

**Interbyte delay.** Multi-byte LD/LDS data and the SIB normally come
out with two IDLE (stop) bits between bytes. CTRLA.IBDLY = 1 adds two
more IDLE bits between bytes for slow debuggers; the first byte after a
direction change gets only the guard time.

#### Instruction set

All instructions are one byte, preceded by SYNCH. Bits 7:5 are the
opcode; the low bits hold size, pointer or address fields.

```table id=updi-opcodes caption="UPDI instruction encoding"
| Instruction | Bit 7:5 | Bit 4 | Bit 3:2 | Bit 1:0 | Byte | Operation |
|---|---|---|---|---|---|---|
| LDS | 000 | 0 | Size A | Size B | 0x00–0x0F | Load from data space, direct address |
| LD | 001 | 0 | Ptr | Size A/B | 0x20–0x2F | Load from data space, indirect (pointer), or load the pointer |
| STS | 010 | 0 | Size A | Size B | 0x40–0x4F | Store to data space, direct address |
| ST | 011 | 0 | Ptr | Size A/B | 0x60–0x6F | Store to data space, indirect (pointer), or store the pointer |
| LDCS | 100 | 0 | CS address | < | 0x80–0x8F | Load one byte from CS register 0–15 |
| REPEAT | 101 | 0 | 00 | Size B | 0xA0 | Load the repeat counter |
| STCS | 110 | 0 | CS address | < | 0xC0–0xCF | Store one byte to CS register 0–15 |
| KEY | 111 | 0 | 0, SIB | Size C | 0xE0–0xE5 | Send a KEY or read the SIB |
```

```table id=updi-fields caption="UPDI instruction fields"
| Field | 00 | 01 | 10 | 11 |
|---|---|---|---|---|
| Size A (address size) | byte | word (2 bytes) | reserved | reserved |
| Size B (data size) | byte | word (2 bytes) [1] | reserved | reserved |
| Ptr (pointer access) | *(ptr) | *(ptr++) | ptr (the pointer register itself) | reserved |
| Size C (key size) | 64 bits (8 bytes) | 128 bits (16 bytes), SIB only | reserved | reserved |

[1] REPEAT supports byte size only.
```

SIB (bit 2 of KEY): 0 = the debugger sends a KEY; 1 = the UPDI returns
the System Information Block. CS addresses 0x0–0x3 are the UPDI PHY
registers, 0x4–0xC the ASI registers, 0xD–0xF reserved.

- **LDS**: SYNCH, LDS, then Size A address bytes (LSB first); after the
  guard time the UPDI sends Size B data bytes. Up to 16-bit address and
  data. With REPEAT the address is sent again for every iteration (no
  auto-increment).
- **STS**: SYNCH, STS, Size A address bytes; the UPDI answers ACK; then
  Size B data bytes; the UPDI answers ACK when the data is stored. Up to
  16-bit address and data; usable with REPEAT.
- **LD**: reads Size B bytes from the address in the UPDI pointer
  register (Ptr = 00), with post-increment by the data size (Ptr = 01),
  or reads the pointer register itself (Ptr = 10). Data follows after the
  guard time.
- **ST**: to write the pointer: ST with Ptr = 10 and Size A, then Size A
  address bytes; ACK. To write data: ST with Ptr = 00 (*(ptr)) or 01
  (*(ptr++), the pointer advances by Size B after the write) and Size B,
  then Size B data bytes; ACK after each successful bus write. With
  REPEAT, set the pointer to the block start and use *(ptr++); one Size B
  data frame may be sent after each ACK.
- **LDCS / STCS**: one byte from or to CS register 0–15; the address is
  in the opcode. LDCS data follows after the guard time; STCS has no
  response.
- **REPEAT**: SYNCH, REPEAT, RPT_0 (one byte). The next instruction is
  executed RPT_0 + 1 times; only its first execution needs SYNCH and the
  instruction byte, later ones send (or receive) only the data and keep
  the ACK handshake. Any instruction except REPEAT can be repeated. An
  ongoing repeat is aborted only by a BREAK. With LD/ST use *(ptr++),
  otherwise every iteration hits the same address; with LDS/STS the
  address is sent in each iteration.
- **KEY**: SYNCH, KEY, then 8 key bytes (only 64-bit keys are supported);
  no response. With SIB = 1 the UPDI instead sends 8 or 16 SIB bytes
  after the guard time.

**System Information Block.** ASCII, read with KEY (SIB = 1). 8 bytes
read only the family ID; 16 bytes read all of it.

```table id=updi-sib caption="System Information Block"
| Bytes | Field |
|---|---|
| 6:0 | Family_ID |
| 7 | Reserved |
| 10:8 | NVM_VERSION |
| 13:11 | OCD_VERSION |
| 14 | Reserved |
| 15 | DBG_OSC_FREQ |
```

#### KEY-protected functions

```table id=updi-keys caption="UPDI keys (signature sent LSB first, 64 bits)"
| Key | Signature | Function | Requirement | Cleared by |
|---|---|---|---|---|
| Chip Erase | 0x4E564D4572617365 | Start an NVM chip erase, clear the lock bits | None | UPDI disable or reset |
| NVMPROG | 0x4E564D50726F6720 | Activate NVM programming | Lock bits cleared; ASI_SYS_STATUS.NVMPROG set | End of programming, UPDI reset |
| USERROW-Write | 0x4E564D5573267465 | Program the user row on a locked part | Lock bits set; ASI_SYS_STATUS.UROWPROG set | Writing the key status bit, UPDI reset |
```

Keys may be shifted in in any order; normally Chip Erase comes before
NVMPROG to unlock a part, but an NVMPROG key shifted in first is not
cleared by a later Chip Erase key. Until the lock bits are cleared the
UPDI cannot access the system bus; only CS-space operations work.
"Reset" below means: write 0x59 to ASI_RESET_REQ (system reset), then
0x00 to release it.

**Chip erase.** 1. KEY with the Chip Erase signature. 2. Optional: check
ASI_KEY_STATUS.CHIPERASE. 3. Reset. 4. Poll ASI_SYS_STATUS.LOCKSTATUS
until 0: erase done, lock bits cleared, full access. During chip erase
the BOD is forced on (BOD.CTRLA ACTIVE = 0x1) at the level of
FUSE.BODCFG; if V~DD~ is below that level the device is unserviceable
until V~DD~ rises.

**NVM programming.** Writing to the NVM controller while the CPU runs
gives unpredictable code execution, so: 1. Chip erase (skip if already
unlocked). 2. KEY with the NVMPROG signature. 3. Optional: check
ASI_KEY_STATUS.NVMPROG. 4. Reset. 5. Poll ASI_SYS_STATUS.NVMPROG until 1.
6. Write the NVM through the UPDI. 7. Reset; programming is complete.

**User-row programming on a locked part.** 1. KEY with the
USERROW-Write signature. 2. Optional: check ASI_KEY_STATUS.UROWWRITE.
3. Reset. 4. Poll ASI_SYS_STATUS.UROWPROG until 1. 5. Write the user-row
data (one EEPROM page, 64 bytes) to the first 64 bytes of SRAM, which map
1:1 to the user row; writes outside them are not executed, and the SRAM
cannot be read back in this mode. 6. Write ASI_SYS_CTRLA.UROWWRITE_FINAL
= 1. 7. Poll ASI_SYS_STATUS.UROWPROG until 0. 8. Write
ASI_KEY_STATUS.UROWWRITE = 1. 9. Reset; programming is complete.

#### Events, clock measurement, sleep

The UPDI generates one event, on every positive edge of a SYNCH
character (UPDI clock domain; cannot be disabled). Use: measure the
system clock with a TCB in input capture frequency measurement mode
(TCBn.CTRLB CNTMODE = 0x3, TCBn.EVCTRL CAPTEI = 1, EDGE = 0, the UPDI
event routed to the TCB through EVSYS). Send SYNCH characters at a slow
baud rate (10–50 kbps; 10 kbps gives a 200 µs capture window); ignore the
first capture, use the second. The captured value can be read from
TCBn.CCMP right after the SYNCH, or stored to memory by the CPU.

The PHY layer runs independently of the sleep modes, so the UPDI stays
reachable in every sleep mode, but cannot access the system bus while the
CPU clock is off. ASI_SYS_STATUS.INSLEEP shows the system in idle or
deeper sleep. ASI_SYS_CTRLA.CLKREQ = 1 (the default when the UPDI is
enabled) keeps the system clock running in sleep (sleep is emulated), so
the bus stays accessible in the deepest sleep modes.

#### Registers (CS space)

```table id=updi-statusa caption="STATUSA (CS 0x00), reset 0x10"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | UPDIREV[3:0] | R | 0x1 | UPDI revision |
```

```table id=updi-statusb caption="STATUSB (CS 0x01), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | PESIG[2:0] | R | 0x0 | Error signature, set on an internal error, cleared when read: 0x0 = no error; 0x1 = parity error (wrong parity bit); 0x2 = frame error (wrong stop bits); 0x3 = access layer timeout (no data or response from the ACC layer, e.g. system domain in sleep or reset); 0x4 = clock recovery error (wrong start bit); 0x7 = contention error (driving contention on the UPDI line); 0x5, 0x6 reserved |
```

```table id=updi-ctrla caption="CTRLA (CS 0x02), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | IBDLY | R/W | 0 | 1 = fixed interbyte delay of two IDLE characters between bytes of a multi-byte LD(S); the first byte gets the regular guard time |
| 5 | PARD | R/W | 0 | 1 = parity bit ignored (recommended only for testing) |
| 4 | DTD | R/W | 0 | 1 = timeout detection off (PHY waits for an ACC response within 65536 UPDI clock cycles) |
| 3 | RSD | R/W | 0 | 1 = no response signatures (ACK) from the UPDI, to cut overhead when writing large blocks to NVM. Only when the bus access delay is predictable, or data may be lost. |
| 2:0 | GTVAL[2:0] | R/W | 0x0 | Guard time on RX→TX change, in UPDI clock cycles: 0x0 = 128; 0x1 = 64; 0x2 = 32; 0x3 = 16; 0x4 = 8; 0x5 = 4; 0x6 = 2; 0x7 = off (no IDLE bits) |
```

```table id=updi-ctrlb caption="CTRLB (CS 0x03), reset 0x00 [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 4 | NACKDIS | R | 0 | 1 = no NACK when a system reset hits an ongoing LD(S) or ST(S) |
| 3 | CCDETDIS | R | 0 | 1 = collision and contention detection off |
| 2 | UPDIDIS | R | 0 | Write 1 to disable the UPDI PHY: the clock request drops and the UPDI resets, clearing all PHY settings and KEYs |

[1] Source conflict: access as printed in the PDF (R), although the descriptions have all three bits written.
```

```table id=updi-asi-key-status caption="ASI_KEY_STATUS (CS 0x07), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5 | UROWWRITE | R/W | 0 | 1 = USERROW-Write key active; written in the user-row sequence |
| 4 | NVMPROG | R | 0 | 1 = NVMPROG key active; cleared automatically when programming is done |
| 3 | CHIPERASE | R | 0 | 1 = Chip Erase key active; cleared automatically when the chip erase is done |
```

```table id=updi-asi-reset-req caption="ASI_RESET_REQ (CS 0x08), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | RSTREQ[7:0] | R/W | 0x00 | Write 0x59 to hold the system in reset; any other value releases it. Read: bit 0 = 1 while the UPDI holds a system reset; other bits read 0. The UPDI itself is not reset. |
```

```table id=updi-asi-ctrla caption="ASI_CTRLA (CS 0x09), reset 0x02 [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1:0 | UPDICLKSEL[1:0] | R/W | 0x3 | UPDI clock: 0x1 = 16 MHz; 0x2 = 8 MHz; 0x3 = 4 MHz (default after reset and enable); 0x0 reserved. Other than 4 MHz only recommended with the BOD at its highest level. |

[1] Source conflict: the register header gives reset 0x02; the bit reset values (1, 1) and the text (4 MHz default) give 0x3.
```

```table id=updi-asi-sys-ctrla caption="ASI_SYS_CTRLA (CS 0x0A), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 1 | UROWWRITE_FINAL | R/W | 0 | Write 1 when the user-row data is in SRAM: starts programming it into the user row. Writable only with the USERROW-Write key decoded; written early, the CPU continues without the data. |
| 0 | CLKREQ | R/W | 0 | 1 = ASI requests the system clock regardless of sleep mode, so the ACC layer stays accessible; 0 = request dropped. Set when the UPDI is enabled, cleared when it is disabled. |
```

```table id=updi-asi-sys-status caption="ASI_SYS_STATUS (CS 0x0B), reset 0x01"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5 | RSTSYS | R | 0 | 1 = system domain in reset (including a reset held by ASI_RESET_REQ); cleared on read |
| 4 | INSLEEP | R | 0 | 1 = system domain in idle or deeper sleep |
| 3 | NVMPROG | R | 0 | 1 = NVM programming may start; when done the UPDI must reset the system through ASI_RESET_REQ |
| 2 | UROWPROG | R | 0 | 1 = user-row programming may start; when done the UPDI must write ASI_SYS_CTRLA.UROWWRITE_FINAL |
| 0 | LOCKSTATUS | R | 1 | 1 = device locked; 0 after a chip erase has cleared the lock bits |
```

```table id=updi-asi-crc-status caption="ASI_CRC_STATUS (CS 0x0C), reset 0x00"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 2:0 | CRC_STATUS[2:0] | R | 0x0 | CRCSCAN status, one-hot: 0x0 = not enabled; 0x1 = enabled, busy; 0x2 = done, signature OK; 0x4 = done, signature failed; other reserved |
```

#### Electrical characteristics

Enable sequence: the debugger drives UPDI low for T~Deb0~ to request the
UPDI clock (handshake/BREAK, T~RES~ on a pin also used as !{RESET}); the
UPDI answers by driving its own txd low for T~UPDI~, then releases the
line (clock ready, channel ready); the debugger keeps its txd high-Z for
T~DebZ~ and then sends SYNCH (0x55) for the auto-baud.

```wavedrom id=updi-enable-table caption="UPDI enable sequence"
{"signal": [
  {"name": "UPDI pad", "wave": "1.0...1...3.", "data": ["SYNCH 0x55"], "node": "..A...B"},
  {"name": "UPDI.txd", "wave": "z...0.z.....", "node": "....C.D"},
  {"name": "debugger.txd", "wave": "z.0.z.....3.", "data": ["SYNCH"], "node": "..E.F.....G"}
 ],
 "edge": ["A<->B TRES", "C<->D TUPDI", "E<->F TDeb0", "F<->G TDebZ"],
 "config": {"hscale": 2}}
```

```table id=updi-char caption="UPDI timing [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| T~RES~ | Duration of handshake/BREAK on !{RESET} | 10 | 200 | µs |
| T~UPDI~ | Duration of UPDI.txd = 0 | 10 | 200 | µs |
| T~Deb0~ | Duration of debugger.txd = 0 | 0.2 | 1 | µs |
| T~DebZ~ | Duration of debugger.txd = z | 200 | 14000 | µs |

[1] Design guidance only, not production tested.
```

```table id=updi-rate caption="UPDI maximum bit rate against V~DD~ [1]"
| Parameter | Conditions | Max | Unit |
|---|---|--:|---|
| UPDI bit rate | V~DD~ = 1.8 V to 5.5 V | 225 | kbps |
| ^ | V~DD~ = 2.2 V to 5.5 V | 450 | kbps |
| ^ | V~DD~ = 2.2 V to 5.5 V [2] | 0.9 | Mbps |
| ^ | V~DD~ = 4.5 V to 5.5 V | 1.8 | Mbps |

[1] Design guidance only, not production tested.
[2] Source conflict: Table 32-31 gives two maximum bit rates, 450 kbps and 0.9 Mbps, for the same condition V~DD~ = [2.2, 5.5]V; both kept as printed.
```

## Registers and commands

This section holds only the register summary; every bit is described in
its peripheral's subsection under [Peripherals](#peripherals) (CPU registers
under [Programming model](#programming-model), fuses and signature row under
[Memory map](#memory-map)). It follows the PDF's organization: a base-address
table, then one offset map per peripheral (offsets relative to the base
address; one map per peripheral type, valid for every instance). Offsets not
listed are reserved and must never be written; blank bit cells are reserved
bits. A register sharing one layout across several offsets is one row with a
"+ n" offset. Multi-byte registers have one row per byte.

```table id=regs-base caption="Peripheral module address map (X = present in that package)"
| Base address | Name | Description | 28-pin | 32-pin | 40-pin | 48-pin |
|---|---|---|:-:|:-:|:-:|:-:|
| 0x0000 | VPORTA | Virtual port A | X | X | X | X |
| 0x0004 | VPORTB | Virtual port B | | | | X |
| 0x0008 | VPORTC | Virtual port C | X | X | X | X |
| 0x000C | VPORTD | Virtual port D | X | X | X | X |
| 0x0010 | VPORTE | Virtual port E | | | X | X |
| 0x0014 | VPORTF | Virtual port F | X | X | X | X |
| 0x001C | GPIO | General purpose I/O registers | X | X | X | X |
| 0x0030 | CPU | CPU | X | X | X | X |
| 0x0040 | RSTCTRL | Reset controller | X | X | X | X |
| 0x0050 | SLPCTRL | Sleep controller | X | X | X | X |
| 0x0060 | CLKCTRL | Clock controller | X | X | X | X |
| 0x0080 | BOD | Brown-out detector | X | X | X | X |
| 0x00A0 | VREF | Voltage reference | X | X | X | X |
| 0x0100 | WDT | Watchdog timer | X | X | X | X |
| 0x0110 | CPUINT | Interrupt controller | X | X | X | X |
| 0x0120 | CRCSCAN | Cyclic redundancy check memory scan | X | X | X | X |
| 0x0140 | RTC | Real-time counter | X | X | X | X |
| 0x0180 | EVSYS | Event system | X | X | X | X |
| 0x01C0 | CCL | Configurable custom logic | X | X | X | X |
| 0x0400 | PORTA | Port A configuration | X | X | X | X |
| 0x0420 | PORTB | Port B configuration | | | | X |
| 0x0440 | PORTC | Port C configuration | X | X | X | X |
| 0x0460 | PORTD | Port D configuration | X | X | X | X |
| 0x0480 | PORTE | Port E configuration | | | X | X |
| 0x04A0 | PORTF | Port F configuration | X | X | X | X |
| 0x05E0 | PORTMUX | Port multiplexer | X | X | X | X |
| 0x0600 | ADC0 | Analog-to-digital converter | X | X | X | X |
| 0x0680 | AC0 | Analog comparator 0 | X | X | X | X |
| 0x0800 | USART0 | USART 0 | X | X | X | X |
| 0x0820 | USART1 | USART 1 | X | X | X | X |
| 0x0840 | USART2 | USART 2 | X | X | X | X |
| 0x0860 | USART3 | USART 3 | | | X | X |
| 0x08A0 | TWI0 | Two-wire interface | X | X | X | X |
| 0x08C0 | SPI0 | Serial peripheral interface | X | X | X | X |
| 0x0A00 | TCA0 | Timer/counter type A, instance 0 | X | X | X | X |
| 0x0A80 | TCB0 | Timer/counter type B, instance 0 | X | X | X | X |
| 0x0A90 | TCB1 | Timer/counter type B, instance 1 | X | X | X | X |
| 0x0AA0 | TCB2 | Timer/counter type B, instance 2 | X | X | X | X |
| 0x0AB0 | TCB3 | Timer/counter type B, instance 3 | | | X | X |
| 0x0F00 | SYSCFG | System configuration | X | X | X | X |
| 0x1000 | NVMCTRL | Nonvolatile memory controller | X | X | X | X |
| 0x1100 | SIGROW | Signature row | X | X | X | X |
| 0x1280 | FUSE | Device-specific fuses | X | X | X | X |
| 0x1300 | USERROW | User row | X | X | X | X |
```

```table id=regs-cpu caption="CPU (base 0x0030)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x04 | CCP | CCP[7:0] | < | < | < | < | < | < | < |
| 0x0D | SP (7:0) | SP[7:0] | < | < | < | < | < | < | < |
| 0x0E | SP (15:8) | SP[15:8] | < | < | < | < | < | < | < |
| 0x0F | SREG | I | T | H | S | V | N | Z | C |
```

```table id=regs-sigrow caption="SIGROW, signature row (base 0x1100)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 + n | DEVICEIDn (n = 0–2) | DEVICEID[7:0] | < | < | < | < | < | < | < |
| 0x03 + n | SERNUMn (n = 0–9) | SERNUM[7:0] | < | < | < | < | < | < | < |
| 0x18 | OSCCAL16M0 |  | OSCCAL16M[6:0] | < | < | < | < | < | < |
| 0x19 | OSCCAL16M1 |  |  |  |  | OSCCAL16MTCAL[3:0] | < | < | < |
| 0x1A | OSCCAL20M0 |  | OSCCAL20M[6:0] | < | < | < | < | < | < |
| 0x1B | OSCCAL20M1 |  |  |  |  | OSCCAL20MTCAL[3:0] | < | < | < |
| 0x20 | TEMPSENSE0 | TEMPSENSE[7:0] | < | < | < | < | < | < | < |
| 0x21 | TEMPSENSE1 | TEMPSENSE[7:0] | < | < | < | < | < | < | < |
| 0x22 | OSC16ERR3V | OSC16ERR3V[7:0] | < | < | < | < | < | < | < |
| 0x23 | OSC16ERR5V | OSC16ERR5V[7:0] | < | < | < | < | < | < | < |
| 0x24 | OSC20ERR3V | OSC20ERR3V[7:0] | < | < | < | < | < | < | < |
| 0x25 | OSC20ERR5V | OSC20ERR5V[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-fuse caption="FUSE, fuses (base 0x1280)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | WDTCFG | WINDOW[3:0] | < | < | < | PERIOD[3:0] | < | < | < |
| 0x01 | BODCFG | LVL[2:0] | < | < | SAMPFREQ | ACTIVE[1:0] | < | SLEEP[1:0] | < |
| 0x02 | OSCCFG | OSCLOCK |  |  |  |  |  | FREQSEL[1:0] | < |
| 0x05 | SYSCFG0 | CRCSRC[1:0] | < |  |  | RSTPINCFG |  |  | EESAVE |
| 0x06 | SYSCFG1 |  |  |  |  |  | SUT[2:0] | < | < |
| 0x07 | APPEND | APPEND[7:0] | < | < | < | < | < | < | < |
| 0x08 | BOOTEND | BOOTEND[7:0] | < | < | < | < | < | < | < |
| 0x0A | LOCKBIT | LOCKBIT[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-gpio caption="GPIO, general purpose I/O registers (base 0x001C)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 + n | GPIORn (n = 0–3) | GPIOR[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-syscfg caption="SYSCFG, system configuration (base 0x0F00)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x01 | REVID | REVID[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-nvmctrl caption="NVMCTRL (base 0x1000)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  |  |  |  |  | CMD[2:0] | < | < |
| 0x01 | CTRLB |  |  |  |  |  |  | BOOTLOCK | APCWP |
| 0x02 | STATUS |  |  |  |  |  | WRERROR | EEBUSY | FBUSY |
| 0x03 | INTCTRL |  |  |  |  |  |  |  | EEREADY |
| 0x04 | INTFLAGS |  |  |  |  |  |  |  | EEREADY |
| 0x06 | DATA (7:0) | DATA[7:0] | < | < | < | < | < | < | < |
| 0x07 | DATA (15:8) | DATA[15:8] | < | < | < | < | < | < | < |
| 0x08 | ADDR (7:0) | ADDR[7:0] | < | < | < | < | < | < | < |
| 0x09 | ADDR (15:8) | ADDR[15:8] | < | < | < | < | < | < | < |
```

```table id=regs-clkctrl caption="CLKCTRL (base 0x0060)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | MCLKCTRLA | CLKOUT |  |  |  |  |  | CLKSEL[1:0] | < |
| 0x01 | MCLKCTRLB |  |  |  | PDIV[3:0] | < | < | < | PEN |
| 0x02 | MCLKLOCK |  |  |  |  |  |  |  | LOCKEN |
| 0x03 | MCLKSTATUS | EXTS | XOSC32KS | OSC32KS | OSC20MS |  |  |  | SOSC |
| 0x10 | OSC20MCTRLA |  |  |  |  |  |  | RUNSTDBY |  |
| 0x11 | OSC20MCALIBA |  | CAL20M[6:0] | < | < | < | < | < | < |
| 0x12 | OSC20MCALIBB | LOCK |  |  |  | TEMPCAL20M[3:0] | < | < | < |
| 0x18 | OSC32KCTRLA |  |  |  |  |  |  | RUNSTDBY |  |
| 0x1C | XOSC32KCTRLA |  |  | CSUT[1:0] | < |  | SEL | RUNSTDBY | ENABLE |
```

```table id=regs-slpctrl caption="SLPCTRL (base 0x0050)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  |  |  |  |  | SMODE[1:0] | < | SEN |
```

```table id=regs-rstctrl caption="RSTCTRL (base 0x0040)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | RSTFR |  |  | UPDIRF | SWRF | WDRF | EXTRF | BORF | PORF |
| 0x01 | SWRR |  |  |  |  |  |  |  | SWRE |
```

```table id=regs-cpuint caption="CPUINT (base 0x0110)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  | IVSEL | CVT |  |  |  |  | LVL0RR |
| 0x01 | STATUS | NMIEX |  |  |  |  |  | LVL1EX | LVL0EX |
| 0x02 | LVL0PRI | LVL0PRI[7:0] | < | < | < | < | < | < | < |
| 0x03 | LVL1VEC | LVL1VEC[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-evsys caption="EVSYS (base 0x0180)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | STROBEx | STROBE[7:0] | < | < | < | < | < | < | < |
| 0x10 + n | CHANNELn (n = 0–7) | GENERATOR[7:0] | < | < | < | < | < | < | < |
| 0x20 + n | USERn (n = 0–23) | CHANNEL[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-portmux caption="PORTMUX (base 0x05E0)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | EVSYSROUTEA |  |  | EVOUTF | EVOUTE | EVOUTD | EVOUTC | EVOUTB | EVOUTA |
| 0x01 | CCLROUTEA |  |  |  |  | LUT3 | LUT2 | LUT1 | LUT0 |
| 0x02 | USARTROUTEA | USART3[1:0] | < | USART2[1:0] | < | USART1[1:0] | < | USART0[1:0] | < |
| 0x03 | TWISPIROUTEA |  |  | TWI0[1:0] | < |  |  | SPI0[1:0] | < |
| 0x04 | TCAROUTEA |  |  |  |  |  | TCA0[2:0] | < | < |
| 0x05 | TCBROUTEA |  |  |  |  | TCB3 | TCB2 | TCB1 | TCB0 |
```

```table id=regs-port caption="PORTx (PORTA base 0x0400, PORTB … PORTF every 0x20)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | DIR | DIR[7:0] | < | < | < | < | < | < | < |
| 0x01 | DIRSET | DIRSET[7:0] | < | < | < | < | < | < | < |
| 0x02 | DIRCLR | DIRCLR[7:0] | < | < | < | < | < | < | < |
| 0x03 | DIRTGL | DIRTGL[7:0] | < | < | < | < | < | < | < |
| 0x04 | OUT | OUT[7:0] | < | < | < | < | < | < | < |
| 0x05 | OUTSET | OUTSET[7:0] | < | < | < | < | < | < | < |
| 0x06 | OUTCLR | OUTCLR[7:0] | < | < | < | < | < | < | < |
| 0x07 | OUTTGL | OUTTGL[7:0] | < | < | < | < | < | < | < |
| 0x08 | IN | IN[7:0] | < | < | < | < | < | < | < |
| 0x09 | INTFLAGS | INT[7:0] | < | < | < | < | < | < | < |
| 0x0A | PORTCTRL |  |  |  |  |  |  |  | SRL |
| 0x10 + n | PINnCTRL (n = 0–7) | INVEN |  |  |  | PULLUPEN | ISC[2:0] | < | < |
```

```table id=regs-vport caption="VPORTx (VPORTA base 0x0000, VPORTB … VPORTF every 0x04)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | DIR | DIR[7:0] | < | < | < | < | < | < | < |
| 0x01 | OUT | OUT[7:0] | < | < | < | < | < | < | < |
| 0x02 | IN | IN[7:0] | < | < | < | < | < | < | < |
| 0x03 | INTFLAGS | INT[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-bod caption="BOD (base 0x0080)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  |  |  | SAMPFREQ | ACTIVE[1:0] | < | SLEEP[1:0] | < |
| 0x01 | CTRLB |  |  |  |  |  | LVL[2:0] | < | < |
| 0x08 | VLMCTRLA |  |  |  |  |  |  | VLMLVL[1:0] | < |
| 0x09 | INTCTRL |  |  |  |  |  | VLMCFG[1:0] | < | VLMIE |
| 0x0A | INTFLAGS |  |  |  |  |  |  |  | VLMIF |
| 0x0B | STATUS |  |  |  |  |  |  |  | VLMS |
```

```table id=regs-vref caption="VREF (base 0x00A0)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  | ADC0REFSEL[2:0] | < | < |  | AC0REFSEL[2:0] | < | < |
| 0x01 | CTRLB |  |  |  |  |  |  | ADC0REFEN | AC0REFEN |
```

```table id=regs-wdt caption="WDT (base 0x0100)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA | WINDOW[3:0] | < | < | < | PERIOD[3:0] | < | < | < |
| 0x01 | STATUS | LOCK |  |  |  |  |  |  | SYNCBUSY |
```

```table id=regs-tca caption="TCA0 in normal mode, CTRLD.SPLITM = 0 (base 0x0A00)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  |  |  |  | CLKSEL[2:0] | < | < | ENABLE |
| 0x01 | CTRLB |  | CMP2EN | CMP1EN | CMP0EN | ALUPD | WGMODE[2:0] | < | < |
| 0x02 | CTRLC |  |  |  |  |  | CMP2OV | CMP1OV | CMP0OV |
| 0x03 | CTRLD |  |  |  |  |  |  |  | SPLITM |
| 0x04 | CTRLECLR |  |  |  |  | CMD[1:0] | < | LUPD | DIR |
| 0x05 | CTRLESET |  |  |  |  | CMD[1:0] | < | LUPD | DIR |
| 0x06 | CTRLFCLR |  |  |  |  | CMP2BV | CMP1BV | CMP0BV | PERBV |
| 0x07 | CTRLFSET |  |  |  |  | CMP2BV | CMP1BV | CMP0BV | PERBV |
| 0x09 | EVCTRL |  |  |  |  | EVACT[2:0] | < | < | CNTEI |
| 0x0A | INTCTRL |  | CMP2 | CMP1 | CMP0 |  |  |  | OVF |
| 0x0B | INTFLAGS |  | CMP2 | CMP1 | CMP0 |  |  |  | OVF |
| 0x0E | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x0F | TEMP | TEMP[7:0] | < | < | < | < | < | < | < |
| 0x20 | CNT (7:0) | CNT[7:0] | < | < | < | < | < | < | < |
| 0x21 | CNT (15:8) | CNT[15:8] | < | < | < | < | < | < | < |
| 0x26 | PER (7:0) | PER[7:0] | < | < | < | < | < | < | < |
| 0x27 | PER (15:8) | PER[15:8] | < | < | < | < | < | < | < |
| 0x28 + 2n | CMPn (7:0), n = 0–2 | CMP[7:0] | < | < | < | < | < | < | < |
| 0x29 + 2n | CMPn (15:8) | CMP[15:8] | < | < | < | < | < | < | < |
| 0x36 | PERBUF (7:0) | PERBUF[7:0] | < | < | < | < | < | < | < |
| 0x37 | PERBUF (15:8) | PERBUF[15:8] | < | < | < | < | < | < | < |
| 0x37 | PERBUFH [2] | PERBUF[15:8] | < | < | < | < | < | < | < |
| 0x38 + 2n | CMPnBUF (7:0) [1] | CMPBUF[7:0] | < | < | < | < | < | < | < |
| 0x39 + 2n | CMPnBUF (15:8) | CMPBUF[15:8] | < | < | < | < | < | < | < |

[1] Source error: printed "CMP0nBUF", "CMP1nBUF", "CMP2nBUF" in the register summary; the register descriptions name them CMPnBUF.
[2] The PDF lists PERBUFH at 0x37 a second time, after the 16-bit PERBUF row; it is the high byte of PERBUF.
```

```table id=regs-tca-split caption="TCA0 in split mode, CTRLD.SPLITM = 1 (base 0x0A00)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  |  |  |  | CLKSEL[2:0] | < | < | ENABLE |
| 0x01 | CTRLB |  | HCMP2EN | HCMP1EN | HCMP0EN |  | LCMP2EN | LCMP1EN | LCMP0EN |
| 0x02 | CTRLC |  | HCMP2OV | HCMP1OV | HCMP0OV |  | LCMP2OV | LCMP1OV | LCMP0OV |
| 0x03 | CTRLD |  |  |  |  |  |  |  | SPLITM |
| 0x04 | CTRLECLR |  |  |  |  | CMD[1:0] | < | CMDEN[1:0] | < |
| 0x05 | CTRLESET |  |  |  |  | CMD[1:0] | < | CMDEN[1:0] | < |
| 0x0A | INTCTRL |  | LCMP2 | LCMP1 | LCMP0 |  |  | HUNF | LUNF |
| 0x0B | INTFLAGS |  | LCMP2 | LCMP1 | LCMP0 |  |  | HUNF | LUNF |
| 0x0E | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x20 | LCNT | LCNT[7:0] | < | < | < | < | < | < | < |
| 0x21 | HCNT | HCNT[7:0] | < | < | < | < | < | < | < |
| 0x26 | LPER | LPER[7:0] | < | < | < | < | < | < | < |
| 0x27 | HPER | HPER[7:0] | < | < | < | < | < | < | < |
| 0x28 + 2n | LCMPn (n = 0–2) | LCMP[7:0] | < | < | < | < | < | < | < |
| 0x29 + 2n | HCMPn (n = 0–2) | HCMP[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-tcb caption="TCBn (TCB0 base 0x0A80, TCB1 … TCB3 every 0x10)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  | RUNSTDBY |  | SYNCUPD |  | CLKSEL[1:0] | < | ENABLE |
| 0x01 | CTRLB |  | ASYNC | CCMPINIT | CCMPEN |  | CNTMODE[2:0] | < | < |
| 0x04 | EVCTRL |  | FILTER |  | EDGE |  |  |  | CAPTEI |
| 0x05 | INTCTRL |  |  |  |  |  |  |  | CAPT |
| 0x06 | INTFLAGS |  |  |  |  |  |  |  | CAPT |
| 0x07 | STATUS |  |  |  |  |  |  |  | RUN |
| 0x08 | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x09 | TEMP | TEMP[7:0] | < | < | < | < | < | < | < |
| 0x0A | CNT (7:0) | CNT[7:0] | < | < | < | < | < | < | < |
| 0x0B | CNT (15:8) | CNT[15:8] | < | < | < | < | < | < | < |
| 0x0C | CCMP (7:0) | CCMP[7:0] | < | < | < | < | < | < | < |
| 0x0D | CCMP (15:8) | CCMP[15:8] | < | < | < | < | < | < | < |
```

```table id=regs-rtc caption="RTC, including the PIT (base 0x0140)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA | RUNSTDBY | PRESCALER[3:0] | < | < | < | CORREN |  | RTCEN |
| 0x01 | STATUS |  |  |  |  | CMPBUSY | PERBUSY | CNTBUSY | CTRLABUSY |
| 0x02 | INTCTRL |  |  |  |  |  |  | CMP | OVF |
| 0x03 | INTFLAGS |  |  |  |  |  |  | CMP | OVF |
| 0x04 | TEMP | TEMP[7:0] | < | < | < | < | < | < | < |
| 0x05 | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x06 | CALIB | SIGN | ERROR[6:0] | < | < | < | < | < | < |
| 0x07 | CLKSEL |  |  |  |  |  |  | CLKSEL[1:0] | < |
| 0x08 | CNT (7:0) | CNT[7:0] | < | < | < | < | < | < | < |
| 0x09 | CNT (15:8) | CNT[15:8] | < | < | < | < | < | < | < |
| 0x0A | PER (7:0) | PER[7:0] | < | < | < | < | < | < | < |
| 0x0B | PER (15:8) | PER[15:8] | < | < | < | < | < | < | < |
| 0x0C | CMP (7:0) | CMP[7:0] | < | < | < | < | < | < | < |
| 0x0D | CMP (15:8) | CMP[15:8] | < | < | < | < | < | < | < |
| 0x10 | PITCTRLA |  | PERIOD[3:0] | < | < | < |  |  | PITEN |
| 0x11 | PITSTATUS |  |  |  |  |  |  |  | CTRLBUSY |
| 0x12 | PITINTCTRL |  |  |  |  |  |  |  | PI |
| 0x13 | PITINTFLAGS |  |  |  |  |  |  |  | PI |
| 0x15 | PITDBGCTRL |  |  |  |  |  |  |  | DBGRUN |
```

```table id=regs-usart caption="USARTn (USART0 base 0x0800, USART1 … USART3 every 0x20)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | RXDATAL | DATA[7:0] | < | < | < | < | < | < | < |
| 0x01 | RXDATAH | RXCIF | BUFOVF |  |  |  | FERR | PERR | DATA\[8\] |
| 0x02 | TXDATAL | DATA[7:0] | < | < | < | < | < | < | < |
| 0x03 | TXDATAH |  |  |  |  |  |  |  | DATA\[8\] |
| 0x04 | STATUS | RXCIF | TXCIF | DREIF | RXSIF | ISFIF |  | BDF | WFB |
| 0x05 | CTRLA | RXCIE | TXCIE | DREIE | RXSIE | LBME | ABEIE | RS485[1:0] | < |
| 0x06 | CTRLB | RXEN | TXEN |  | SFDEN | ODME | RXMODE[1:0] | < | MPCM |
| 0x07 | CTRLC (CMODE ≠ MSPI) | CMODE[1:0] | < | PMODE[1:0] | < | SBMODE | CHSIZE[2:0] | < | < |
| 0x07 | CTRLC | CMODE[1:0] | < |  |  |  | UDORD | UCPHA |  |
| 0x08 | BAUD (7:0) | BAUD[7:0] | < | < | < | < | < | < | < |
| 0x09 | BAUD (15:8) | BAUD[15:8] | < | < | < | < | < | < | < |
| 0x0A | CTRLD | ABW[1:0] | < |  |  |  |  |  |  |
| 0x0B | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x0C | EVCTRL |  |  |  |  |  |  |  | IREI |
| 0x0D | TXPLCTRL | TXPL[7:0] | < | < | < | < | < | < | < |
| 0x0E | RXPLCTRL |  | RXPL[6:0] | < | < | < | < | < | < |
```

```table id=regs-spi caption="SPI0 (base 0x08C0)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  | DORD | MASTER | CLK2X |  | PRESC[1:0] | < | ENABLE |
| 0x01 | CTRLB | BUFEN | BUFWR |  |  |  | SSD | MODE[1:0] | < |
| 0x02 | INTCTRL | RXCIE | TXCIE | DREIE | SSIE |  |  |  | IE |
| 0x03 | INTFLAGS (BUFEN = 0) | IF | WRCOL |  |  |  |  |  |  |
| 0x03 | INTFLAGS | RXCIF | TXCIF | DREIF | SSIF |  |  |  | BUFOVF |
| 0x04 | DATA | DATA[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-twi caption="TWI0 (base 0x08A0)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  |  |  | SDASETUP | SDAHOLD[1:0] | < | FMPEN |  |
| 0x01 | DUALCTRL |  |  |  |  | SDAHOLD[1:0] | < | FMPEN | ENABLE |
| 0x02 | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x03 | MCTRLA | RIEN | WIEN |  | QCEN | TIMEOUT[1:0] | < | SMEN | ENABLE |
| 0x04 | MCTRLB |  |  |  |  | FLUSH | ACKACT | MCMD[1:0] | < |
| 0x05 | MSTATUS | RIF | WIF | CLKHOLD | RXACK | ARBLOST | BUSERR | BUSSTATE[1:0] | < |
| 0x06 | MBAUD | BAUD[7:0] | < | < | < | < | < | < | < |
| 0x07 | MADDR | ADDR[7:0] | < | < | < | < | < | < | < |
| 0x08 | MDATA | DATA[7:0] | < | < | < | < | < | < | < |
| 0x09 | SCTRLA | DIEN | APIEN | PIEN |  |  | PMEN | SMEN | ENABLE |
| 0x0A | SCTRLB |  |  |  |  |  | ACKACT | SCMD[1:0] | < |
| 0x0B | SSTATUS | DIF | APIF | CLKHOLD | RXACK | COLL | BUSERR | DIR | AP |
| 0x0C | SADDR | ADDR[7:0] | < | < | < | < | < | < | < |
| 0x0D | SDATA | DATA[7:0] | < | < | < | < | < | < | < |
| 0x0E | SADDRMASK | ADDRMASK[6:0] | < | < | < | < | < | < | ADDREN |
```

```table id=regs-crcscan caption="CRCSCAN (base 0x0120)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA | RESET |  |  |  |  |  | NMIEN | ENABLE |
| 0x01 | CTRLB |  |  |  |  |  |  | SRC[1:0] | < |
| 0x02 | STATUS |  |  |  |  |  |  | OK | BUSY |
```

```table id=regs-ccl caption="CCL (base 0x01C0)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA |  | RUNSTDBY |  |  |  |  |  | ENABLE |
| 0x01 | SEQCTRL0 |  |  |  |  | SEQSEL0[3:0] | < | < | < |
| 0x02 | SEQCTRL1 |  |  |  |  | SEQSEL1[3:0] | < | < | < |
| 0x05 | INTCTRL0 | INTMODE3[1:0] | < | INTMODE2[1:0] | < | INTMODE1[1:0] | < | INTMODE0[1:0] | < |
| 0x07 | INTFLAGS |  |  |  |  | INT3 | INT2 | INT1 | INT0 |
| 0x08 + 4n | LUTnCTRLA (n = 0–3) | EDGEDET | OUTEN | FILTSEL[1:0] | < | CLKSRC[2:0] | < | < | ENABLE |
| 0x09 + 4n | LUTnCTRLB | INSEL1[3:0] | < | < | < | INSEL0[3:0] | < | < | < |
| 0x0A + 4n | LUTnCTRLC |  |  |  |  | INSEL2[3:0] | < | < | < |
| 0x0B + 4n | TRUTHn | TRUTH[7:0] | < | < | < | < | < | < | < |
```

```table id=regs-ac caption="AC0 (base 0x0680)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA | RUNSTDBY | OUTEN | INTMODE[1:0] | < | LPMODE | HYSMODE[1:0] | < | ENABLE |
| 0x02 | MUXCTRL | INVERT |  |  | MUXPOS[1:0] | < |  | MUXNEG[1:0] | < |
| 0x04 | DACREF | DACREF[7:0] | < | < | < | < | < | < | < |
| 0x06 | INTCTRL |  |  |  |  |  |  |  | CMP |
| 0x07 | STATUS |  |  |  | STATE |  |  |  | CMP |
```

```table id=regs-adc caption="ADC0 (base 0x0600)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | CTRLA | RUNSTBY |  |  |  |  | RESSEL | FREERUN | ENABLE |
| 0x01 | CTRLB |  |  |  |  |  | SAMPNUM[2:0] | < | < |
| 0x02 | CTRLC |  | SAMPCAP | REFSEL[1:0] | < |  | PRESC[2:0] | < | < |
| 0x03 | CTRLD | INITDLY[2:0] | < | < | ASDV | SAMPDLY[3:0] | < | < | < |
| 0x04 | CTRLE |  |  |  |  |  | WINCM[2:0] | < | < |
| 0x05 | SAMPCTRL |  |  |  | SAMPLEN[4:0] | < | < | < | < |
| 0x06 | MUXPOS |  |  |  | MUXPOS[4:0] | < | < | < | < |
| 0x08 | COMMAND |  |  |  |  |  |  |  | STCONV |
| 0x09 | EVCTRL |  |  |  |  |  |  |  | STARTEI |
| 0x0A | INTCTRL |  |  |  |  |  |  | WCOMP | RESRDY |
| 0x0B | INTFLAGS |  |  |  |  |  |  | WCOMP | RESRDY |
| 0x0C | DBGCTRL |  |  |  |  |  |  |  | DBGRUN |
| 0x0D | TEMP | TEMP[7:0] | < | < | < | < | < | < | < |
| 0x10 | RES (7:0) | RES[7:0] | < | < | < | < | < | < | < |
| 0x11 | RES (15:8) | RES[15:8] | < | < | < | < | < | < | < |
| 0x12 | WINLT (7:0) | WINLT[7:0] | < | < | < | < | < | < | < |
| 0x13 | WINLT (15:8) | WINLT[15:8] | < | < | < | < | < | < | < |
| 0x14 | WINHT (7:0) | WINHT[7:0] | < | < | < | < | < | < | < |
| 0x15 | WINHT (15:8) | WINHT[15:8] | < | < | < | < | < | < | < |
| 0x16 | CALIB |  |  |  |  |  |  |  | DUTYCYC |
```

```table id=regs-updi caption="UPDI control/status (CS) space, accessed with LDCS/STCS (no data-space address)"
| Offset | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0x00 | STATUSA | UPDIREV[3:0] | < | < | < |  |  |  |  |
| 0x01 | STATUSB |  |  |  |  |  | PESIG[2:0] | < | < |
| 0x02 | CTRLA | IBDLY |  | PARD | DTD | RSD | GTVAL[2:0] | < | < |
| 0x03 | CTRLB |  |  |  | NACKDIS | CCDETDIS | UPDIDIS |  |  |
| 0x07 | ASI_KEY_STATUS |  |  | UROWWRITE | NVMPROG | CHIPERASE |  |  |  |
| 0x08 | ASI_RESET_REQ | RSTREQ[7:0] | < | < | < | < | < | < | < |
| 0x09 | ASI_CTRLA |  |  |  |  |  |  | UPDICLKSEL[1:0] | < |
| 0x0A | ASI_SYS_CTRLA |  |  |  |  |  |  |  | CLKREQ |
| 0x0B | ASI_SYS_STATUS |  |  | RSTSYS | INSLEEP | NVMPROG | UROWPROG |  | LOCKSTATUS |
| 0x0C | ASI_CRC_STATUS |  |  |  |  |  | CRC_STATUS[2:0] | < | < |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~DD~ | Power supply voltage | | -0.5 | 6 | V |
| I~VDD~ | Current into a VDD pin | T~A~ = -40 °C to 85 °C | | 200 | mA |
| ^ | ^ | T~A~ = 85 °C to 125 °C | | 100 | mA |
| I~GND~ | Current out of a GND pin | T~A~ = -40 °C to 85 °C | | 200 | mA |
| ^ | ^ | T~A~ = 85 °C to 125 °C | | 100 | mA |
| V~PIN~ | Pin voltage with respect to GND | | -0.5 | V~DD~ + 0.5 | V |
| I~PIN~ | I/O pin sink/source current | | -40 | 40 | mA |
| I~c1~ | I/O pin injection current, except the !{RESET} pin [2] | V~pin~ < GND - 0.6 V or 5.5 V < V~pin~ ≤ 6.1 V; 4.9 V < V~DD~ ≤ 5.5 V | -1 | 1 | mA |
| I~c2~ | I/O pin injection current, except the !{RESET} pin [2] | V~pin~ < GND - 0.6 V or V~pin~ ≤ 5.5 V; V~DD~ ≤ 4.9 V | -15 | 15 | mA |
| T~storage~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond these may cause permanent damage; functional operation at these or any conditions beyond the operational sections is not implied. Long exposure may affect reliability.
[2] Below GND - 0.6 V a current-limiting resistor is required: R = (GND - 0.6 V - V~pin~) / I~Cn~. Above V~DD~ + 0.6 V likewise: R = (V~pin~ - (V~DD~ + 0.6 V)) / I~Cn~.
```

## Recommended operating conditions

Typical values are at T~A~ = 25 °C and V~DD~ = 3 V unless noted; minimum
and maximum values hold over the operating temperature and voltage range
unless noted. Typical values are design guidance only.

```table id=roc caption="General operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~DD~ | Operating supply voltage [1] | | 1.8 [2] | 5.5 | V |
| ^ | ^ | Automotive (VAO) grade and extended temperature range | 2.7 [3] | 5.5 | V |
| T~A~ | Operating temperature | Extended | -40 | 125 | °C |
| ^ | ^ | Industrial | -40 | 85 | °C |

[1] During chip erase the BOD is forced on; if V~DD~ is below the configured V~BOD~ the erase fails.
[2] Guaranteed down to 1.8 V, or to the BOD trigger level V~BOD~ when the BOD is active.
[3] Guaranteed down to 2.7 V, or to the BOD trigger level V~BOD~ when the BOD is active.
```

```table id=fmax caption="Operating voltage and frequency"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| f~CLK_CPU~ | Nominal system clock frequency | V~DD~ = 1.8 V to 5.5 V, T~A~ = -40 °C to 105 °C [1][4] | 0 | 5 | MHz |
| ^ | ^ | V~DD~ = 2.7 V to 5.5 V, T~A~ = -40 °C to 105 °C [2][4] | 0 | 10 | MHz |
| ^ | ^ | V~DD~ = 4.5 V to 5.5 V, T~A~ = -40 °C to 105 °C [3][4] | 0 | 20 | MHz |
| ^ | ^ | V~DD~ = 2.7 V to 5.5 V, T~A~ = -40 °C to 125 °C [2] | 0 | 8 | MHz |
| ^ | ^ | V~DD~ = 4.5 V to 5.5 V, T~A~ = -40 °C to 125 °C [3] | 0 | 16 | MHz |

[1] Guaranteed down to V~BOD~ with BODLEVEL0.
[2] Guaranteed down to V~BOD~ with BODLEVEL2.
[3] Guaranteed down to V~BOD~ with BODLEVEL7.
[4] Does not apply to automotive (-VAO) parts.
```

The maximum frequency is linear in V~DD~ between the corners: for
-40 °C to 105 °C, 5 MHz at 1.8 V rising to 10 MHz at 2.7 V and to 20 MHz
at 4.5 V, flat to 5.5 V; for -40 °C to 125 °C, 8 MHz at 2.7 V rising to
16 MHz at 4.5 V, flat to 5.5 V (no operation below 2.7 V).

## Thermal information

```equation
T~J~ = T~A~ + P~D~ × R~θJA~
R~θJA~ = R~θJC~ + R~θCA~
```

P~D~ is the total power dissipation (system plus I/O). R~θJC~ is fixed
by the device; R~θCA~ depends on the PCB, heat sinking and airflow, and
good thermal management brings R~θJA~ close to R~θJC~.

```table id=thermal caption="Power dissipation and junction temperature vs. temperature, T~A~ = -40 °C to 125 °C [1]"
| Symbol | Parameter | VQFN32 | TQFP32 | UQFN48 | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 41 | 69 | 36 | °C/W |
| R~θJC~ | Junction-to-case thermal resistance | 16 | 20 | 14.4 | °C/W |
| P~D~ | Power dissipation, typical, active 5 V/20 MHz | 50 | 50 | 50 | mW |
| T~J~ - T~A~ | Typical | 2 | 3.5 | 1.8 | °C |

[1] Unconfirmed: Table 32-4 prints the TQFP48 row as 68, 17, 50, 3.4 under the columns T~A~ Range, R~θJA~, R~θJC~ and P~D~, with the T~J~ - T~A~ cell empty, so its values do not line up with the column headings; left out of this table rather than placed by guess. The table has no SSOP28 or PDIP40 row.
```

## Electrical characteristics

### Power consumption

Measured at V~DD~ = 3 V, T~A~ = 25 °C, OSC20M as the system clock unless
noted, peripherals disabled, I/O ports driven low with inputs disabled.

```table id=idd caption="Power consumption in active and idle mode"
| Parameter | Clock source | PDIV division | f~CLK_CPU~ | V~DD~ | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|---|
| Active supply current | OSC20M, FREQSEL = 0x2 | 1 | 20 MHz | 5 V | 11.4 | | mA |
| ^ | ^ | 2 | 10 MHz | 5 V | 5.7 | | mA |
| ^ | ^ | ^ | ^ | 3 V | 3.2 | | mA |
| ^ | ^ | 4 | 5 MHz | 5 V | 3.8 | 5 | mA |
| ^ | ^ | ^ | ^ | 3 V | 1.6 | | mA |
| ^ | ^ | ^ | ^ | 2 V | 1.2 | | mA |
| ^ | OSC20M, FREQSEL = 0x1 | 1 | 16 MHz | 5 V | 8.6 | | mA |
| ^ | ^ | 2 | 8 MHz | 5 V | 4.3 | | mA |
| ^ | ^ | ^ | ^ | 3 V | 2.6 | | mA |
| ^ | OSCULP32K | 1 | 32.768 kHz | 5 V | 21 | | µA |
| ^ | ^ | ^ | ^ | 3 V | 12 | | µA |
| ^ | ^ | ^ | ^ | 2 V | 8 | | µA |
| Idle supply current | OSC20M, FREQSEL = 0x2 | 1 | 20 MHz | 5 V | 2.8 | | mA |
| ^ | ^ | 2 | 10 MHz | 5 V | 1.4 | | mA |
| ^ | ^ | ^ | ^ | 3 V | 0.8 | | mA |
| ^ | ^ | 4 | 5 MHz | 5 V | 1.3 | 3.5 | mA |
| ^ | ^ | ^ | ^ | 3 V | 0.4 | | mA |
| ^ | ^ | ^ | ^ | 2 V | 0.3 | | mA |
| ^ | OSC20M, FREQSEL = 0x1 | 1 | 16 MHz | 5 V | 2.3 | | mA |
| ^ | ^ | 2 | 8 MHz | 5 V | 1.3 | | mA |
| ^ | ^ | ^ | ^ | 3 V | 0.7 | | mA |
| ^ | OSCULP32K | 1 | 32.768 kHz | 5 V | 5.6 | | µA |
| ^ | ^ | ^ | ^ | 3 V | 2.8 | | µA |
| ^ | ^ | ^ | ^ | 2 V | 1.8 | | µA |
```

```table id=ipd caption="Power consumption in power-down, standby and reset, V~DD~ = 3 V"
| Parameter | Conditions | 25 °C | 85 °C [1] | 125 °C | Unit |
| ^ | ^ | Typ | Max | Max | ^ |
|---|---|--:|--:|--:|---|
| Standby supply current | RTC at 1.024 kHz from external XOSC32K (C~L~ = 7.5 pF) | 0.7 | | | µA |
| ^ | RTC at 1.024 kHz from internal OSCULP32K | 0.7 | 6 | 16 | µA |
| Power-down/standby supply current | All peripherals stopped (power-down and standby are then the same) | 0.1 | 5 | 15 | µA |
| Reset supply current | !{RESET} pulled low | 100 | | | µA |

[1] Design guidance only, not tested.
```

### Peripherals power consumption

Additional current of each peripheral, to add to the base consumption
above. V~DD~ = 3 V, T = 25 °C, OSC20M at 1 MHz as the system clock and
idle sleep mode unless noted. Some peripherals request their clock in
standby; see the peripheral subsections.

```table id=iperiph caption="Peripherals power consumption [1]"
| Parameter | Conditions | Typ | Unit |
|---|---|--:|---|
| BOD | Continuous | 19 | µA |
| ^ | Sampling at 1 kHz | 1.2 | µA |
| TCA | 16-bit count at 1 MHz | 13.0 | µA |
| TCB | 16-bit count at 1 MHz | 7.4 | µA |
| RTC | 16-bit count at OSCULP32K | 1.2 | µA |
| WDT (including OSCULP32K) | | 0.7 | µA |
| OSC20M | | 130 | µA |
| AC | Fast mode [2] | 92 | µA |
| ^ | Low-power mode [2] | 45 | µA |
| ADC [3] | 50 ksps | 330 | µA |
| ^ | 100 ksps | 340 | µA |
| XOSC32K | C~L~ = 7.5 pF | 0.5 | µA |
| OSCULP32K | | 0.4 | µA |
| USART | Enabled at 9600 baud | 13.0 | µA |
| SPI (master) | Enabled at 100 kHz | 2.1 | µA |
| TWI (master) | Enabled at 100 kHz | 24.0 | µA |
| TWI (slave) | Enabled at 100 kHz | 17.0 | µA |
| Flash programming | Erase operation | 1.5 | mA |
| ^ | Write operation | 3.0 | mA |

[1] Current of the module only.
[2] CPU in standby mode.
[3] Average, ADC in free-running mode.
```

The current drawn by one pin driving a capacitive load C~load~ at an
average switching frequency f~sw~:

```equation
I~CP~ ≈ V~DD~ × C~load~ × f~sw~
```

### I/O pin characteristics

```table id=io caption="I/O pin characteristics" conditions="T~A~ = -40 °C to 85 °C, V~DD~ = 1.8 V to 5.5 V unless noted"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~IL~ | Input low voltage | | -0.2 | | 0.3 × V~DD~ | V |
| V~IH~ | Input high voltage | | 0.7 × V~DD~ | | V~DD~ + 0.2 | V |
| I~IH~, I~IL~ | I/O pin input leakage current | V~DD~ = 5.5 V, pin high | | < 0.05 | | µA |
| ^ | ^ | V~DD~ = 5.5 V, pin low | | < 0.05 | | µA |
| V~OL~ | Output low voltage | V~DD~ = 1.8 V, I~OL~ = 1.5 mA | | | 0.36 | V |
| ^ | ^ | V~DD~ = 3.0 V, I~OL~ = 7.5 mA | | | 0.6 | V |
| ^ | ^ | V~DD~ = 5.0 V, I~OL~ = 15 mA | | | 1 | V |
| V~OH~ | Output high voltage | V~DD~ = 1.8 V, I~OH~ = 1.5 mA | 1.44 | | | V |
| ^ | ^ | V~DD~ = 3.0 V, I~OH~ = 7.5 mA | 2.4 | | | V |
| ^ | ^ | V~DD~ = 5.0 V, I~OH~ = 15 mA | 4 | | | V |
| I~total~ | Maximum combined sink/source current per pin group [1] [2] | T~A~ = 125 °C | | | 100 | mA |
| ^ | ^ | T~A~ = 25 °C | | | 200 | mA |
| t~RISE~ | Rise time | V~DD~ = 3.0 V, load = 20 pF | | 2.5 | | ns |
| ^ | ^ | V~DD~ = 5.0 V, load = 20 pF | | 1.5 | | ns |
| ^ | ^ | V~DD~ = 3.0 V, load = 20 pF, slew rate enabled | | 19 | | ns |
| ^ | ^ | V~DD~ = 5.0 V, load = 20 pF, slew rate enabled | | 9 | | ns |
| t~FALL~ | Fall time | V~DD~ = 3.0 V, load = 20 pF | | 2.0 | | ns |
| ^ | ^ | V~DD~ = 5.0 V, load = 20 pF | | 1.3 | | ns |
| ^ | ^ | V~DD~ = 3.0 V, load = 20 pF, slew rate enabled | | 21 | | ns |
| ^ | ^ | V~DD~ = 5.0 V, load = 20 pF, slew rate enabled | | 11 | | ns |
| C~pin~ | I/O pin capacitance | Except TOSC, VREFA and TWI pins | | 3.5 | | pF |
| ^ | ^ | TOSC pins | | 4 | | pF |
| ^ | ^ | TWI pins | | 10 | | pF |
| ^ | ^ | VREFA pin | | 14 | | pF |
| R~p~ | Pull-up resistor | | 20 | 35 | 50 | kΩ |

[1] Pin group A (PA[7:0], PF[6:2]), pin group B (PB[7:0], PC[7:0]), pin group C (PD[7:0], PE[3:0], PF[1:0]). For 28-pin and 32-pin devices, pin group A and B should be seen as a single group. The combined continuous sink/source current for each group should not exceed the limits.
[2] Design guidance only, not production tested.
```

## Switching characteristics

### Start-up and wake-up time

```table id=wakeup caption="Start-up and wake-up time"
| Symbol | Parameter | Clock source | PDIV division | f~CLK_CPU~ | V~DD~ | Typ | Unit |
|---|---|---|---|---|---|--:|---|
| t~startup~ | Start-up time from release of any reset source to the first instruction | Any | Any | Any | Any | 200 | µs |
| t~wakeup~ | Wake-up from idle [1] | OSC20M, FREQSEL = 0x2 | 1 | 20 MHz | 5 V | 1 | µs |
| ^ | ^ | ^ | 2 | 10 MHz | 3 V | 2 | µs |
| ^ | ^ | ^ | 4 | 5 MHz | 2 V | 4 | µs |
| ^ | ^ | OSC20M, FREQSEL = 0x1 | 1 | 16 MHz | 5 V | 1.2 | µs |
| ^ | ^ | ^ | 2 | 8 MHz | 3 V | 2.4 | µs |
| ^ | ^ | OSCULP32K | 1 | 32.768 kHz | | 700 | µs |
| ^ | Wake-up from standby or power-down, clock source stopped [1] | OSC20M, FREQSEL = 0x2 | 1 | 20 MHz | 5 V | 12 | µs |
| ^ | ^ | ^ | 2 | 10 MHz | 3 V | 13 | µs |
| ^ | ^ | ^ | 4 | 5 MHz | 2 V | 15 | µs |
| ^ | ^ | OSC20M, FREQSEL = 0x1 | 1 | 16 MHz | 5 V | 16 | µs |
| ^ | ^ | ^ | 2 | 8 MHz | 3 V | 15 | µs |
| ^ | ^ | OSCULP32K | 1 | 32.768 kHz | | 750 | µs |

[1] From the wake-up request until the peripheral clock is available on the pin. All peripherals start on the first clock cycle; the CPU is halted for four more clock cycles before executing.
```

### Programming time

```table id=prog-time caption="Programming times, typical"
| Parameter | Typ | Unit |
|---|--:|---|
| Page buffer clear | 7 | CLK_CPU cycles |
| Page write | 2 | ms |
| Page erase | 2 | ms |
| Page erase-write | 4 | ms |
| Chip erase | 4 | ms |
| EEPROM erase | 4 | ms |
```

## Typical characteristics

Twelve of the PDF's 73 curves, digitized to about 2 % of full scale; typical, not guaranteed. In the supply-current-vs-frequency curves each V~DD~ line ends at the highest frequency plotted for it.

```curve id=typ-active-f caption="Active supply current vs. frequency (1–20 MHz), T~A~ = 25 °C (Figure 33-1)"
{"x": {"label": "f", "unit": "MHz", "range": [0, 20]}, "y": {"label": "I_DD", "unit": "mA", "range": [0, 12]},
 "series": [
   {"label": "VDD = 1.8 V", "points": [[1, 0.2], [5, 0.7]]},
   {"label": "VDD = 2.2 V", "points": [[1, 0.25], [8, 1.35]]},
   {"label": "VDD = 2.7 V", "points": [[1, 0.28], [10, 2.05]]},
   {"label": "VDD = 3 V", "points": [[1, 0.3], [10, 2.3]]},
   {"label": "VDD = 3.6 V", "points": [[1, 0.37], [15, 4.2]]},
   {"label": "VDD = 4.2 V", "points": [[1, 0.42], [20, 6.6]]},
   {"label": "VDD = 5 V", "points": [[1, 0.48], [20, 8.5]]},
   {"label": "VDD = 5.5 V", "points": [[1, 0.55], [20, 9.6]]}]}
```

```curve id=typ-active-vdd caption="Active supply current vs. V~DD~, T~A~ = 25 °C (Figure 33-5)"
{"x": {"label": "V_DD", "unit": "V", "range": [1.5, 5.5]}, "y": {"label": "I_DD", "unit": "mA", "range": [0, 12]},
 "series": [
   {"label": "f = 1.25 MHz", "points": [[1.8, 0.4], [3.5, 0.75], [5.5, 1.2]]},
   {"label": "f = 2.5 MHz", "points": [[1.8, 0.55], [3.5, 1.05], [5.5, 1.8]]},
   {"label": "f = 5 MHz", "points": [[1.8, 0.9], [3.0, 1.45], [4.0, 2.05], [5.5, 2.95]]},
   {"label": "f = 10 MHz", "points": [[2.7, 2.25], [3.5, 3.0], [4.5, 4.05], [5.5, 5.15]]},
   {"label": "f = 20 MHz", "points": [[4.5, 7.35], [4.8, 7.95], [4.9, 8.4], [5.5, 9.7]]}]}
```

```curve id=typ-idle-f caption="Idle supply current vs. frequency (1–20 MHz), T~A~ = 25 °C (Figure 33-8)"
{"x": {"label": "f", "unit": "MHz", "range": [0, 20]}, "y": {"label": "I_DD", "unit": "mA", "range": [0, 5]},
 "series": [
   {"label": "VDD = 1.8 V", "points": [[1, 0.07], [5, 0.22]]},
   {"label": "VDD = 2.2 V", "points": [[1, 0.08], [8, 0.42]]},
   {"label": "VDD = 2.7 V", "points": [[1, 0.1], [10, 0.68]]},
   {"label": "VDD = 3 V", "points": [[1, 0.11], [10, 0.77]]},
   {"label": "VDD = 3.6 V", "points": [[1, 0.12], [15, 1.42]]},
   {"label": "VDD = 4.2 V", "points": [[1, 0.14], [20, 2.28]]},
   {"label": "VDD = 5 V", "points": [[1, 0.16], [20, 2.85]]},
   {"label": "VDD = 5.5 V", "points": [[1, 0.18], [20, 3.22]]}]}
```

```curve id=typ-pd-vdd caption="Power-down supply current vs. V~DD~, all functions disabled (Figure 33-14)"
{"x": {"label": "V_DD", "unit": "V", "range": [1.5, 5.5]}, "y": {"label": "I_DD", "unit": "µA", "range": [0, 8]},
 "series": [
   {"label": "-40 °C", "points": [[1.8, 0.05], [5.5, 0.08]]},
   {"label": "-20 °C", "points": [[1.8, 0.05], [5.5, 0.09]]},
   {"label": "0 °C", "points": [[1.8, 0.06], [5.5, 0.1]]},
   {"label": "25 °C", "points": [[1.8, 0.08], [5.5, 0.15]]},
   {"label": "70 °C", "points": [[1.8, 0.25], [3.5, 0.28], [4.5, 0.35], [5.0, 0.43], [5.5, 0.58]]},
   {"label": "85 °C", "points": [[1.8, 0.42], [3.0, 0.47], [4.0, 0.55], [4.5, 0.62], [5.0, 0.75], [5.5, 1.0]]},
   {"label": "105 °C", "points": [[1.8, 1.15], [2.5, 1.22], [3.0, 1.28], [3.5, 1.37], [4.0, 1.48], [4.5, 1.65], [5.0, 1.93], [5.5, 2.4]]},
   {"label": "125 °C", "points": [[1.8, 3.15], [2.0, 3.2], [2.5, 3.4], [3.0, 3.62], [3.5, 3.85], [4.0, 4.2], [4.5, 4.6], [5.0, 5.2], [5.25, 5.6], [5.5, 6.15]]}]}
```

```curve id=typ-vol caption="I/O pin output voltage vs. sink current, T~A~ = 25 °C (Figure 33-26)"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 20]}, "y": {"label": "V_OL", "unit": "V", "range": [0, 1]},
 "series": [
   {"label": "VDD = 1.8 V", "points": [[0, 0], [2, 0.1], [4, 0.22], [6, 0.38], [7, 0.5], [7.5, 0.6], [8, 0.79]]},
   {"label": "VDD = 2 V", "points": [[0, 0], [2, 0.09], [4, 0.19], [6, 0.3], [8, 0.43], [9, 0.52], [10, 0.65]]},
   {"label": "VDD = 2.2 V", "points": [[0, 0], [2, 0.08], [4, 0.16], [6, 0.26], [8, 0.36], [10, 0.48], [11, 0.55], [12, 0.63]]},
   {"label": "VDD = 3 V", "points": [[0, 0], [4, 0.12], [8, 0.25], [12, 0.38], [16, 0.53], [20, 0.7]]},
   {"label": "VDD = 4 V", "points": [[0, 0], [4, 0.1], [8, 0.2], [12, 0.31], [16, 0.42], [20, 0.53]]},
   {"label": "VDD = 5 V", "points": [[0, 0], [4, 0.09], [8, 0.18], [12, 0.27], [16, 0.37], [20, 0.47]]}]}
```

```curve id=typ-voh caption="I/O pin output voltage vs. source current, T~A~ = 25 °C (Figure 33-30)"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 20]}, "y": {"label": "V_OH", "unit": "V", "range": [0, 5]},
 "series": [
   {"label": "VDD = 1.8 V", "points": [[0, 1.8], [2, 1.68], [4, 1.55], [6, 1.38], [7, 1.27], [8, 1.07]]},
   {"label": "VDD = 2 V", "points": [[0, 1.98], [2, 1.89], [4, 1.79], [6, 1.66], [8, 1.5], [9, 1.4], [10, 1.26]]},
   {"label": "VDD = 2.2 V", "points": [[0, 2.18], [4, 2.02], [8, 1.78], [10, 1.63], [12, 1.45]]},
   {"label": "VDD = 3 V", "points": [[0, 2.98], [4, 2.87], [8, 2.72], [12, 2.53], [16, 2.35], [20, 2.13]]},
   {"label": "VDD = 4 V", "points": [[0, 3.97], [4, 3.88], [8, 3.76], [12, 3.61], [16, 3.48], [20, 3.34]]},
   {"label": "VDD = 5 V", "points": [[0, 4.97], [4, 4.88], [8, 4.77], [12, 4.66], [16, 4.55], [20, 4.43]]}]}
```

```curve id=typ-vref11 caption="Internal 1.1 V reference error vs. temperature (Figure 33-35)"
{"x": {"label": "T_A", "unit": "°C", "range": [-40, 125]}, "y": {"label": "V_REF error", "unit": "%", "range": [-1, 1]},
 "series": [
   {"label": "VDD = 2 V", "points": [[-40, -0.1], [-20, 0.05], [0, 0.15], [20, 0.2], [40, 0.235], [60, 0.235], [80, 0.2], [100, 0.13], [125, 0.0]]},
   {"label": "VDD = 3 V", "points": [[-40, -0.1], [-20, 0.06], [0, 0.17], [20, 0.24], [40, 0.27], [60, 0.25], [80, 0.2], [100, 0.13], [125, 0.03]]},
   {"label": "VDD = 5 V", "points": [[-40, -0.02], [-20, 0.11], [0, 0.21], [20, 0.26], [40, 0.27], [60, 0.25], [80, 0.2], [100, 0.13], [125, 0.04]]}]}
```

```curve id=typ-bod0 caption="BOD threshold vs. temperature, BODLEVEL0 (Figure 33-41)"
{"x": {"label": "T_A", "unit": "°C", "range": [-40, 125]}, "y": {"label": "V_BOD", "unit": "V", "range": [1.7, 1.9]},
 "series": [
   {"label": "Falling VDD", "points": [[-40, 1.784], [0, 1.789], [25, 1.791], [60, 1.792], [85, 1.791], [125, 1.789]]},
   {"label": "Rising VDD", "points": [[-40, 1.809], [0, 1.814], [25, 1.816], [60, 1.817], [85, 1.816], [125, 1.813]]}]}
```

```curve id=typ-osc20m-t caption="OSC20M frequency vs. temperature, 20 MHz setting (Figure 33-70)"
{"x": {"label": "T_A", "unit": "°C", "range": [-40, 125]}, "y": {"label": "f", "unit": "MHz", "range": [19.5, 20.5]},
 "series": [
   {"label": "VDD = 1.8 V", "points": [[-40, 19.8], [-20, 19.885], [0, 19.955], [20, 20.02], [40, 20.065], [60, 20.09], [80, 20.095], [100, 20.09], [125, 20.06]]},
   {"label": "VDD = 2.2 V", "points": [[-40, 19.8], [-20, 19.885], [0, 19.955], [20, 20.02], [40, 20.065], [60, 20.09], [80, 20.095], [100, 20.09], [125, 20.06]]},
   {"label": "VDD = 2.7 V", "points": [[-40, 19.8], [-20, 19.885], [0, 19.955], [20, 20.02], [40, 20.065], [60, 20.09], [80, 20.095], [100, 20.09], [125, 20.06]]},
   {"label": "VDD = 3 V", "points": [[-40, 19.79], [-20, 19.88], [0, 19.95], [20, 20.01], [40, 20.06], [60, 20.085], [80, 20.09], [100, 20.08], [125, 20.05]]},
   {"label": "VDD = 3.6 V", "points": [[-40, 19.79], [-20, 19.88], [0, 19.95], [20, 20.01], [40, 20.06], [60, 20.085], [80, 20.09], [100, 20.08], [125, 20.05]]},
   {"label": "VDD = 5 V", "points": [[-40, 19.78], [-20, 19.87], [0, 19.94], [20, 20.0], [40, 20.045], [60, 20.07], [80, 20.075], [100, 20.06], [125, 20.025]]},
   {"label": "VDD = 5.5 V", "points": [[-40, 19.78], [-20, 19.87], [0, 19.94], [20, 20.0], [40, 20.045], [60, 20.07], [80, 20.075], [100, 20.06], [125, 20.025]]}]}
```

```curve id=typ-osc20m-vdd caption="OSC20M frequency vs. V~DD~, 20 MHz setting (Figure 33-71)"
{"x": {"label": "V_DD", "unit": "V", "range": [1.5, 5.5]}, "y": {"label": "f", "unit": "MHz", "range": [19.5, 20.5]},
 "series": [
   {"label": "-40 °C", "points": [[1.7, 19.79], [4.0, 19.79], [5.5, 19.78]]},
   {"label": "-20 °C", "points": [[1.7, 19.89], [4.0, 19.885], [5.5, 19.88]]},
   {"label": "0 °C", "points": [[1.7, 19.96], [4.0, 19.95], [5.5, 19.94]]},
   {"label": "25 °C", "points": [[1.7, 20.04], [4.0, 20.03], [5.5, 20.01]]},
   {"label": "70 °C", "points": [[1.7, 20.1], [4.0, 20.09], [5.5, 20.07]]},
   {"label": "85 °C", "points": [[1.7, 20.1], [4.0, 20.08], [5.5, 20.07]]},
   {"label": "105 °C", "points": [[1.7, 20.09], [4.0, 20.08], [5.5, 20.05]]},
   {"label": "125 °C", "points": [[1.7, 20.06], [4.0, 20.04], [5.5, 20.02]]}]}
```

```curve id=typ-osculp-t caption="OSCULP32K frequency vs. temperature (Figure 33-72)"
{"x": {"label": "T_A", "unit": "°C", "range": [-40, 125]}, "y": {"label": "f", "unit": "kHz", "range": [30, 40]},
 "series": [
   {"label": "VDD = 1.8 V", "points": [[-40, 30.5], [-20, 31.3], [0, 31.9], [20, 32.4], [40, 32.7], [60, 32.75], [80, 32.6], [100, 32.25], [125, 31.65]]},
   {"label": "VDD = 2.2 V", "points": [[-40, 30.55], [-20, 31.35], [0, 31.95], [20, 32.45], [40, 32.75], [60, 32.8], [80, 32.65], [100, 32.3], [125, 31.7]]},
   {"label": "VDD = 2.7 V", "points": [[-40, 30.65], [-20, 31.45], [0, 32.05], [20, 32.55], [40, 32.85], [60, 32.9], [80, 32.75], [100, 32.4], [125, 31.8]]},
   {"label": "VDD = 3 V", "points": [[-40, 30.75], [-20, 31.55], [0, 32.15], [20, 32.65], [40, 32.95], [60, 33.0], [80, 32.85], [100, 32.5], [125, 31.9]]},
   {"label": "VDD = 3.6 V", "points": [[-40, 30.85], [-20, 31.65], [0, 32.25], [20, 32.75], [40, 33.05], [60, 33.1], [80, 32.95], [100, 32.6], [125, 32.0]]},
   {"label": "VDD = 5 V", "points": [[-40, 32.55], [-20, 33.3], [0, 33.9], [20, 34.3], [40, 34.5], [60, 34.5], [80, 34.3], [100, 33.9], [125, 33.15]]},
   {"label": "VDD = 5.5 V", "points": [[-40, 33.45], [-20, 34.4], [0, 35.15], [20, 35.6], [40, 35.85], [60, 35.85], [80, 35.6], [100, 35.15], [125, 34.4]]}]}
```

```curve id=typ-osculp-vdd caption="OSCULP32K frequency vs. V~DD~ (Figure 33-73)"
{"x": {"label": "V_DD", "unit": "V", "range": [1.5, 5.5]}, "y": {"label": "f", "unit": "kHz", "range": [30, 40]},
 "series": [
   {"label": "-40 °C", "points": [[1.7, 30.55], [2.2, 30.45], [3.0, 30.6], [3.5, 30.75], [4.0, 31.1], [4.5, 31.7], [5.0, 32.5], [5.5, 33.4]]},
   {"label": "-20 °C", "points": [[1.7, 31.35], [2.2, 31.28], [3.0, 31.4], [3.5, 31.55], [4.0, 31.85], [4.5, 32.4], [5.0, 33.4], [5.5, 34.4]]},
   {"label": "0 °C", "points": [[1.7, 31.95], [2.2, 31.9], [3.0, 32.0], [3.5, 32.15], [4.0, 32.4], [4.5, 32.95], [5.0, 33.9], [5.5, 35.1]]},
   {"label": "25 °C", "points": [[1.7, 32.25], [2.2, 32.25], [3.0, 32.35], [3.5, 32.4], [4.0, 32.6], [4.5, 33.0], [5.0, 34.0], [5.5, 35.3]]},
   {"label": "70 °C", "points": [[1.7, 32.75], [2.2, 32.75], [3.0, 32.9], [3.5, 32.95], [4.0, 33.1], [4.5, 33.6], [5.0, 34.5], [5.5, 35.7]]},
   {"label": "85 °C", "points": [[1.7, 32.6], [2.2, 32.6], [3.0, 32.75], [3.5, 32.85], [4.0, 33.0], [4.5, 33.5], [5.0, 34.3], [5.5, 35.5]]},
   {"label": "105 °C", "points": [[1.7, 32.5], [2.2, 32.5], [3.0, 32.65], [3.5, 32.75], [4.0, 32.95], [4.5, 33.45], [5.0, 34.3], [5.5, 35.6]]},
   {"label": "125 °C", "points": [[1.7, 31.55], [2.2, 31.6], [3.0, 31.7], [3.5, 31.72], [4.0, 31.9], [4.5, 32.35], [5.0, 33.2], [5.5, 34.3]]}]}
```

## Package

```table id=pkg caption="Packages (Table 35-1 and the drawing pages)"
| Package | Drawing number (Table 35-1) | Drawing on the drawing page | Style | Package code | Max weight (mg) |
|---|---|---|---|---|--:|
| SSOP28 | C04-00073 | C04-073 Rev C | SS | N2X | 229 |
| TQFP32 | C04-00074 | C04-074 Rev C | PT | T5X | 100 |
| VQFN32 | C04-21395 | C04-21395-RXB Rev B | RXB | RXB | 61 |
| PDIP40 | C04-00016 | C04-016B | P | S2X | 6500 |
| TQFP48 | C04-00300 | C04-300-PT Rev D | PT | Y8X | 140 |
| UQFN48 | C04-00153 | C04-153A | MV | R7X | TBD |
```

```table id=pkg-dim caption="Package dimensions (millimeters; PDIP40 in inches)"
| Package | Dimension | Symbol | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| SSOP28: 28-lead plastic shrink small outline (SS), 5.30 mm body | < | < | < | < | < | < |
| SSOP28 | Number of pins | N | | 28 | | |
| ^ | Pitch [1] | e | | 0.65 | | mm |
| ^ | Overall height | A | | | 2.00 | mm |
| ^ | Molded package thickness | A2 | 1.65 | 1.75 | 1.85 | mm |
| ^ | Standoff | A1 | 0.05 | | | mm |
| ^ | Overall width | E | 7.40 | 7.80 | 8.20 | mm |
| ^ | Molded package width [2] | E1 | 5.00 | 5.30 | 5.60 | mm |
| ^ | Overall length [2] | D | 9.90 | 10.20 | 10.50 | mm |
| ^ | Foot length | L | 0.55 | 0.75 | 0.95 | mm |
| ^ | Footprint [3] | L1 | | 1.25 | | mm |
| ^ | Lead thickness | c | 0.09 | | 0.25 | mm |
| ^ | Foot angle | | 0 | 4 | 8 | ° |
| ^ | Lead width | b | 0.22 | | 0.38 | mm |
| TQFP32: 32-lead plastic thin quad flatpack (PT), 7x7x1.0 mm body, 2.00 mm footprint | < | < | < | < | < | < |
| TQFP32 | Number of leads | N | | 32 | | |
| ^ | Lead pitch [1] | e | | 0.80 | | mm |
| ^ | Overall height | A | | | 1.20 | mm |
| ^ | Standoff | A1 | 0.05 | | 0.15 | mm |
| ^ | Molded package thickness | A2 | 0.95 | 1.00 | 1.05 | mm |
| ^ | Foot length | L | 0.45 | 0.60 | 0.75 | mm |
| ^ | Footprint [3] | L1 | | 1.00 | | mm |
| ^ | Foot angle | | 0 | | 7 | ° |
| ^ | Overall width [1] | E | | 9.00 | | mm |
| ^ | Overall length [1] | D | | 9.00 | | mm |
| ^ | Molded package width [1] | E1 | | 7.00 | | mm |
| ^ | Molded package length [1] | D1 | | 7.00 | | mm |
| ^ | Lead width | b | 0.30 | 0.37 | 0.45 | mm |
| ^ | Mold draft angle top | | 11 | | 13 | ° |
| VQFN32: 32-lead very thin plastic quad flat, no lead (RXB), 5x5x0.9 mm body, 3.1x3.1 mm exposed pad [4] | < | < | < | < | < | < |
| VQFN32 | Number of terminals | N | | 32 | | |
| ^ | Pitch [1] | e | | 0.50 | | mm |
| ^ | Overall height | A | 0.80 | 0.85 | 0.90 | mm |
| ^ | Standoff | A1 | 0.00 | 0.02 | 0.05 | mm |
| ^ | Terminal thickness [3] | A3 | | 0.203 | | mm |
| ^ | Overall length [1] | D | | 5.00 | | mm |
| ^ | Exposed pad length | D2 | 3.00 | 3.10 | 3.20 | mm |
| ^ | Overall width [1] | E | | 5.00 | | mm |
| ^ | Exposed pad width | E2 | 3.00 | 3.10 | 3.20 | mm |
| ^ | Terminal width | b | 0.18 | 0.25 | 0.30 | mm |
| ^ | Terminal length | L | 0.30 | 0.40 | 0.50 | mm |
| ^ | Terminal-to-exposed-pad | K | 0.20 | | | mm |
| PDIP40: 40-lead plastic dual in-line (P), 600 mil body | < | < | < | < | < | < |
| PDIP40 | Number of pins | N | | 40 | | |
| ^ | Pitch [1] | e | | .100 | | in |
| ^ | Top to seating plane | A | | | .250 | in |
| ^ | Molded package thickness | A2 | .125 | | .195 | in |
| ^ | Base to seating plane | A1 | .015 | | | in |
| ^ | Shoulder to shoulder width | E | .590 | | .625 | in |
| ^ | Molded package width [5] | E1 | .485 | | .580 | in |
| ^ | Overall length [5] | D | 1.980 | | 2.095 | in |
| ^ | Tip to seating plane | L | .115 | | .200 | in |
| ^ | Lead thickness | c | .008 | | .015 | in |
| ^ | Upper lead width | b1 | .030 | | .070 | in |
| ^ | Lower lead width | b | .014 | | .023 | in |
| ^ | Overall row spacing | eB | | | .700 | in |
| TQFP48: 48-lead plastic thin quad flatpack (PT), 7x7x1.0 mm body | < | < | < | < | < | < |
| TQFP48 | Number of terminals | N | | 48 | | |
| ^ | Pitch [1] | e | | 0.50 | | mm |
| ^ | Overall height | A | | | 1.20 | mm |
| ^ | Standoff | A1 | 0.05 | | 0.15 | mm |
| ^ | Molded package thickness | A2 | 0.95 | 1.00 | 1.05 | mm |
| ^ | Overall length [1] | D | | 9.00 | | mm |
| ^ | Molded package length [1] | D1 | | 7.00 | | mm |
| ^ | Overall width [1] | E | | 9.00 | | mm |
| ^ | Molded package width [1] | E1 | | 7.00 | | mm |
| ^ | Terminal width | b | 0.17 | 0.22 | 0.27 | mm |
| ^ | Terminal thickness | c | 0.09 | | 0.16 | mm |
| ^ | Terminal length | L | 0.45 | 0.60 | 0.75 | mm |
| ^ | Footprint [3] | L1 | | 1.00 | | mm |
| ^ | Lead bend radius | R1 | 0.08 | | | mm |
| ^ | Lead bend radius | R2 | 0.08 | | 0.20 | mm |
| ^ | Foot angle | ϴ | 0 | 3.5 | 7 | ° |
| ^ | Lead angle | ϴ1 | 0 | | | ° |
| ^ | Mold draft angle | ϴ2 | 11 | 12 | 13 | ° |
| UQFN48: 48-lead plastic ultra thin quad flat, no lead (MV), 6x6x0.5 mm body [4] | < | < | < | < | < | < |
| UQFN48 | Number of pins | N | | 48 | | |
| ^ | Pitch [1] | e | | 0.40 | | mm |
| ^ | Overall height | A | 0.45 | 0.50 | 0.55 | mm |
| ^ | Standoff | A1 | 0.00 | 0.02 | 0.05 | mm |
| ^ | Contact thickness [3] | A3 | | 0.127 | | mm |
| ^ | Overall width [1] | E | | 6.00 | | mm |
| ^ | Exposed pad width | E2 | 4.45 | 4.60 | 4.75 | mm |
| ^ | Overall length [1] | D | | 6.00 | | mm |
| ^ | Exposed pad length | D2 | 4.45 | 4.60 | 4.75 | mm |
| ^ | Contact width | b | 0.15 | 0.20 | 0.25 | mm |
| ^ | Contact length | L | 0.30 | 0.40 | 0.50 | mm |
| ^ | Contact-to-exposed pad | K | 0.20 | | | mm |

[1] BSC: basic dimension, theoretically exact value shown without tolerances (dimensioning and tolerancing per ASME Y14.5M).
[2] Dimensions D and E1 do not include mold flash or protrusions. Mold flash or protrusions shall not exceed 0.20mm per side.
[3] REF: reference dimension, usually without tolerance, for information purposes only.
[4] Package is saw singulated.
[5] Dimensions D and E1 do not include mold flash or protrusions. Mold flash or protrusions shall not exceed .010" per side.
```

Pin 1 visual index feature may vary, but must be located within the
hatched area of the drawing. Lead finish: J-STD-609 material code e3.
Microchip's recommended land patterns (C04-2073, C04-2074, C04-23395,
C04-2300) are not copied; a catalog footprint will be drawn instead
once the parts are added.

## Ordering information

```table id=ordering-table caption="Product numbers (48 KB Flash, 6 KB SRAM) [1]"
| Part | Package | Temperature range (°C) | Max. CPU speed | Supply | Carrier |
|---|---|---|---|---|---|
| ATmega4808-XF | SSOP28 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tube |
| ATmega4808-XFR | SSOP28 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4808-XFR-VAO | SSOP28 | -40 to 125 | 16 MHz | 2.7–5.5 V | Tape and reel |
| ATmega4808-XU | SSOP28 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tube |
| ATmega4808-XUR | SSOP28 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4808-AF | TQFP32 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4808-AFR | TQFP32 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4808-AFR-VAO | TQFP32 | -40 to 125 | 16 MHz | 2.7–5.5 V | Tape and reel |
| ATmega4808-AU | TQFP32 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4808-AUR | TQFP32 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4808-MF | VQFN32 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4808-MFR | VQFN32 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4808-MU | VQFN32 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4808-MUR | VQFN32 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4809-PF | PDIP40 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tube |
| ATmega4809-AF | TQFP48 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4809-AFR | TQFP48 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4809-AFR-VAO | TQFP48 | -40 to 125 | 16 MHz | 2.7–5.5 V | Tape and reel |
| ATmega4809-AU | TQFP48 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4809-AUR | TQFP48 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4809-MF | UQFN48 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4809-MFR | UQFN48 | -40 to 125 | 20 MHz | 1.8–5.5 V | Tape and reel |
| ATmega4809-MU | UQFN48 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tray |
| ATmega4809-MUR | UQFN48 | -40 to 85 | 20 MHz | 1.8–5.5 V | Tape and reel |

[1] Pb-free, RoHS compliant, halide free. VAO parts are qualified to AEC-Q100, may use a different package and can have additional electrical specifications.
```

```table id=partkey caption="Part number key: AT mega 48 0 9 - M F R - VAO"
| Field | Values |
|---|---|
| Flash size | 48 = 48 KB |
| Series | 0 |
| Pin count | 9 = 48 pins (PDIP: 40), 8 = 32 pins (SSOP: 28) |
| Package | A = TQFP, M = QFN (UQFN/VQFN), P = PDIP, X = SSOP |
| Temperature | U = -40 °C to 85 °C (industrial), F = -40 °C to 125 °C (extended) |
| Carrier | R = tape and reel; none = tube or tray |
| Variant | VAO = automotive; none = industrial |
```

## Conversion notes

- Pages converted: 12–520 (errata pointer, block diagram, pinout and
  I/O multiplexing, conventions, CPU, memories, every peripheral
  chapter, instruction set summary, electrical and typical
  characteristics, ordering) and the dimension tables of the package
  drawings (522–543). Dropped by the standard rules: cover features
  list and family overview figures (pp 1–5, facts kept in the
  Description), table of contents (6–11), vendor pinout drawings
  (14–17, replaced by the pin table), package outline drawings and
  land patterns (522–543, dimensions kept), revision history
  (544–547), product-identification repeat, code-protection and legal
  pages (548–551).
- Layout: each peripheral's description, register bit tables and its
  own electrical/timing tables (from chapter 32) sit in its
  [Peripherals](#peripherals) subsection. Chip-level characteristics
  (ratings, operating conditions, power consumption, I/O pins, wake-up,
  programming time) stay in their own sections. Fuses, signature row,
  user row and lock bits sit in [Memory map](#memory-map), as in the
  source's Memories chapter. Registers and commands holds the base
  address table and the per-peripheral offset maps (the source is
  organized as base address + offset).
- Register tables omit reserved bits (stated once at the top of
  Peripherals). Registers that differ only in what a write does
  (DIR/DIRSET/DIRCLR/DIRTGL, CTRLECLR/CTRLESET, …) and identical
  instances (USART0–3, TCB0–3, PORTA–F, LUT0–3) are described once.
  Single-bit names written with an index (DATA\[8\]) are escaped so
  the checker doesn't read them as note markers.
- Dropped figures: all block diagrams (one sentence each), timing
  figures whose content the text states (TCA, TCB and RTC waveform
  examples, CPU pipeline figures 6-2/6-3, CPUINT scheduling 13-2…13-5,
  WDT 19-2/19-3, TWI bus figures and flowcharts 25-13/25-16, PIT
  enable 22-2, ADC error definitions 29-2…29-5 and 5-1…5-4, UPDI
  per-instruction frames), the RS-485 and IRCOM connection figures, the
  CRC shift-register figure, the maximum-frequency-vs-V~DD~ figures
  32-1/32-2 (stated as text: linear between the corner points), and
  example C code (USART baud compensation, ADC temperature, register
  assignment examples). Redrawn as WaveDrom: USART frame, auto-baud,
  XDIR timing, SPI modes and buffer mode, TWI transaction, SPI and TWI
  electrical timing, UPDI SYNCH frame and enable timing, one ADC
  single-conversion diagram.
- Typical characteristics: 12 of 73 curves kept by the processor rule
  and digitized to about 2 % of full scale (straight-line curves as
  their end points); 61 dropped: active supply current at 0.1–1 MHz,
  vs. temperature (16, 20 MHz), vs. V~DD~ at 1–16 MHz and at
  32.768 kHz (33-2, 33-3, 33-4, 33-6, 33-7); idle current at low
  frequency, vs. temperature, at 32.768 kHz (33-9…33-12); power-down vs.
  temperature (33-13); standby with RTC or sampled BOD (33-15…33-17);
  power-on current at BODLEVEL7 (33-18); I/O hysteresis and input
  thresholds (33-19…33-22); output voltage vs. sink and source current
  at 1.8, 3.0 and 5.0 V over temperature (33-23…33-25, 33-27…33-29);
  pull-up current (33-31…33-33); 0.55, 2.5 and 4.3 V references vs.
  temperature (33-34, 33-36, 33-37); BOD current continuous and sampled
  (33-38…33-40); BOD threshold at BODLEVEL2 and BODLEVEL7 (33-42,
  33-43); ADC absolute accuracy, DNL, gain, INL and offset vs. V~DD~
  and vs. V~REF~ with internal and external reference (33-44…33-63);
  AC hysteresis and offset vs. V~CM~ (33-64…33-67); OSC20M calibration
  step size and frequency vs. calibration value (33-68, 33-69).
- Derived: 0x14FF and 0x27FF (end of EEPROM and of the reserved block,
  one below the next start address 0x1500 / 0x2800 in Figure 7-1, p.37);
  0xBFFF (end of the 48 KB Flash in code space from 0x0000, Figure 7-1
  and Table 7-1, p.37–38).
- From images: the UQFN48 dimensions 6.00, 0.40, 0.45–0.55, 0.00–0.05,
  0.127, 4.45–4.60–4.75, 0.15–0.25, 0.30–0.50, 0.20 and its notes
  (drawing C04-153A sheet 2, p.541, a scanned page); the typical
  characteristics curve points (digitized from the chapter 33 figures);
  the OSC20M baud compensation equation (BAUD~actual~, BAUD~ideal~,
  SigRowError, p.87) and the free-running sample-rate equation (p.401),
  whose text layer is unreadable glyphs.
- Source issues:
  - Unconfirmed, as printed: Figure 7-1's "960 Ext I/O Registers" count
    for 0x0040–0x0FFF (memory map note).
  - Source error, corrected: STS operation printed DS(k) ← Rd (Rr).
  - Unconfirmed, as printed: Table 12-1 has TCD columns; this device
    has no TCD.
  - Unconfirmed, as printed: t~MIN_RST~ in the Max column.
  - Source conflict, kept with notes: VPORT.INTFLAGS access R vs
    write-1-to-clear; BOD.CTRLA SAMPFREQ access R vs CCP; WDT PERIOD
    table (1.0/2.0/4.1/8.2 s) vs WINDOW table (1.024/2.048/4.096/8.192 s);
    TCA split-mode LPER/HPER reset 0x00 vs PER 0xFFFF.
  - Source error, corrected: TCA split-mode INTCTRL/INTFLAGS headings
    call LCMP2 (and LCMP1 in INTFLAGS) "compare channel 0"; register
    summary prints CMP0nBUF/CMP1nBUF/CMP2nBUF for CMPnBUF.
  - Source conflict, kept with notes: TCB 8-bit PWM period said to be
    controlled by CCMPH and to be CCMPL + 1; TCB COUNT enabled by
    CLKSEL = EVENT, which CLKSEL lacks.
  - Source error, corrected: RTC text calls RTCEN "ENABLE" and the PIT
    busy flag "SYNCBUSY" (CTRLBUSY).
  - Source conflict, kept with notes: USART BDF threshold 11 bits vs
    break of 12 or more samples; SPI buffer-mode DREIF "cleared after a
    Reset to indicate that the transmitter is ready" vs reset value 0;
    TWI MCMD DIR (0 = read, 1 = send) vs MADDR R/W and SSTATUS.DIR
    (1 = read).
  - Source error, corrected: TWI text names TWIn.MSETUP (CLKHOLD is in
    MSTATUS), ACKACT in MSTATUS (it is in MCTRLB), AIEN in CTRLA (APIEN
    in SCTRLA); AC text names MODE and AC.MUXCTRLA (LPMODE, MUXCTRL) and
    calls the AC "the DAC"; ADC text names ADCnREFSEL[0:2] in VREF.CTRLn
    (ADC0REFSEL[2:0] in VREF.CTRLA) and EVSYS.ASYNCUSERn (USERn).
  - Unconfirmed, as printed: ADC data format called "1's complement"
    with 0x0000 = zero, 0xFFFF = full scale.
  - Source conflict, kept with notes: UPDI T~DebZ~ max 14000 µs (Table
    32-30) vs 13.5 ms in the text; UPDI CTRLB access R though written;
    ASI_CTRLA reset 0x02 in the header vs bit resets 1, 1 (0x3).
  - Unconfirmed, left out with a note: Table 32-4's TQFP48 row (values
    do not line up with the column headings).
  - Source conflict, kept with a note: Table 32-31 gives 450 kbps and
    0.9 Mbps for the same V~DD~ = [2.2, 5.5]V.
  - Source conflict, kept with a note: Table 32-19 describes t~SOSH~ as
    MISO hold after !{SS} low; Figure 32-7 shows it after !{SS} high.
