---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "attiny85",
  "title": "ATtiny25/45/85 8-bit AVR microcontroller with 2/4/8 KB in-system programmable flash",
  "class": "microcontroller",
  "source": {
    "pdf": "mcu/attiny85.pdf",
    "vendor": "Atmel",
    "document": "2586Q-AVR-08/2013",
    "date": "2013-08",
    "pages": 234
  },
  "devices": [],
  "parts": [],
  "pending": "AVR microcontrollers are not in the catalog yet",
  "converted": "2026-11-07"
}
---

# ATtiny25/45/85 8-bit AVR microcontroller with 2/4/8 KB in-system programmable flash

## Description

An 8-bit AVR RISC microcontroller: 32 general purpose registers wired
straight to the ALU, most of its 120 instructions executing in one
clock cycle (close to 1 MIPS per MHz), Harvard architecture with a
one-stage pipeline. ATtiny25/45/85 differ only in memory size: 2/4/8 KB
of in-system programmable flash (10,000 write/erase cycles), 128/256/512
bytes of EEPROM (100,000 cycles) and 128/256/512 bytes of SRAM.

On chip: six I/O lines (Port B, one shared with !{RESET}), an 8-bit
Timer/Counter0 with two PWM channels, an 8-bit high-speed
Timer/Counter1 clocked up to 64 MHz from an on-chip PLL with two PWM
outputs, complementary outputs and a dead time generator, a Universal
Serial Interface (USI) for SPI- and TWI-style serial links, a 10-bit ADC
with four single-ended channels, two differential pairs with 1× or 20×
gain and a temperature sensor, an analog comparator, external and
pin-change interrupts, a watchdog timer with its own oscillator, a
calibrated internal RC oscillator, brown-out detection, debugWIRE
on-chip debugging and SPI in-system programming. Idle, ADC noise
reduction and power-down sleep modes; 0.1 µA in power-down at 1.8 V,
300 µA active at 1 MHz and 1.8 V.

ATtiny25V/45V/85V run from 1.8 V to 5.5 V (4 MHz at 1.8 V, 10 MHz at
2.7 V); ATtiny25/45/85 from 2.7 V to 5.5 V (10 MHz at 2.7 V, 20 MHz at
4.5 V). Industrial temperature range; see [ordering](#ordering) for the
105 °C and 125 °C ATtiny25 grades. Packages: 8-pin PDIP and SOIC, 8-pin
TSSOP (ATtiny45 only), 20-pad QFN/MLF.

## Pinout

```table id=pinout-table caption="Pin numbers by package (top view)"
| Pin name | Alternate functions | PDIP, SOIC, TSSOP (8P3, 8S2, S8S1, 8X) | QFN/MLF (20M1) |
|---|---|--:|--:|
| PB5 | PCINT5, !{RESET}, ADC0, dW | 1 | 1 |
| PB3 | PCINT3, XTAL1, CLKI, !{OC1B}, ADC3 | 2 | 2 |
| PB4 | PCINT4, XTAL2, CLKO, OC1B, ADC2 | 3 | 5 |
| GND | | 4 | 8 |
| PB0 | MOSI, DI, SDA, AIN0, OC0A, !{OC1A}, AREF, PCINT0 | 5 | 11 |
| PB1 | MISO, DO, AIN1, OC0B, OC1A, PCINT1 | 6 | 12 |
| PB2 | SCK, USCK, SCL, ADC1, T0, INT0, PCINT2 | 7 | 14 |
| VCC | | 8 | 15 |
| DNC | | | 3, 4, 6, 7, 9, 10, 13, 16, 17, 18, 19, 20 |
| EP | | | pad |
```

## Pin functions

```pins
VCC: Supply voltage
GND: Ground
PB0, PB1, PB2, PB3, PB4: Port B, 6-bit bidirectional I/O with a pull-up per bit; symmetrical drive with high sink and source capability. Inputs pulled low externally source current while their pull-up is on. Tri-stated while a reset is active, even without a clock. Alternate functions: see I/O ports under Peripherals.
PB5: Port B bit 5, shared with !{RESET}: a low level longer than the minimum pulse width (see electrical characteristics) resets the part, even with no clock running, unless the reset pin is disabled (RSTDISBL fuse); shorter pulses are not guaranteed to reset. Usable as a weak I/O pin when the reset function is disabled. Also debugWIRE (dW) and ADC0.
DNC: Do not connect (QFN/MLF)
EP: Exposed bottom pad (QFN/MLF); solder to ground
```

In ATtiny15 compatibility mode the ATtiny25 exchanges PB3 and PB4
(pins 2 and 3) for backward compatibility with the ATtiny15.

## Functional description

Block diagram: the AVR core (program counter, stack pointer,
instruction register and decoder, 32 general purpose registers with X,
Y and Z, ALU, status register) shares an 8-bit data bus with the flash,
SRAM, EEPROM, interrupt unit, watchdog timer, MCU control and status
registers, Timer/Counter0, Timer/Counter1, USI, ADC and analog
comparator, Port B data and direction registers and drivers, the
calibrated internal oscillator, timing and control, and the programming
logic.

Idle mode stops the CPU while SRAM, timers, ADC, analog comparator and
interrupts keep running. Power-down keeps the register contents but
stops all chip functions until the next interrupt or hardware reset. ADC
noise reduction mode stops the CPU and all I/O modules except the ADC.
The flash can be reprogrammed in system over SPI, by a conventional
non-volatile memory programmer, or by boot code running on the core.

Data retention: projected failure rate well under 1 ppm over 20 years
at 85 °C, or 100 years at 25 °C.

### Errata

Revision letters are the die revision of each device. ATtiny25 rev D–F,
ATtiny45 rev F–G and ATtiny85 rev B–C have no known errata. ATtiny25
rev A was not sampled.

```table id=errata-table caption="Errata by device revision"
| Erratum | Affected | Workaround |
|---|---|---|
| EEPROM read may return invalid data at low clock frequency and/or low supply voltage [1] | ATtiny25 rev B–C; ATtiny45 rev A–E; ATtiny85 rev A | Don't use the EEPROM below 1 MHz together with below 2 V: below 1 MHz keep V~CC~ above 2 V; below 2 V keep the clock above 1 MHz |
| PLL does not lock below 6.0 MHz | ATtiny45 rev A–C | Run at 6.0 MHz or higher when using the PLL |
| EEPROM read from application code does not work in lock bit mode 3 (LB2, LB1 programmed) | ATtiny45 rev A–C | Don't use lock mode 3 when the application reads the EEPROM |
| Timer/Counter1 PWM output OC1B–!{OC1B} works only when COM1B[1:0] equals COM1A[1:0] | ATtiny45 rev B–C (fixed in rev D) | Use the same setting in COM1A[1:0] and COM1B[1:0] |
| Power-down current too high when an external clock is selected but its pin is still an output, when the EEPROM was read before power-down, or when V~CC~ ≥ 4.5 V | ATtiny45 rev A | Don't make the clock pin an output; don't read the EEPROM if power-down current matters; keep V~CC~ below 4.5 V |
| debugWIRE loses communication when an interrupt arrives while single stepping | ATtiny45 rev A | Disable interrupts while single stepping; debug interrupts with breakpoints inside the handler |

[1] Temperature dependent, not characterized; the guideline holds at room temperature.
```

## Programming model

The CPU is clocked by clk~CPU~ straight from the selected clock source,
with no internal division. Program and data have separate memories and
buses; while one instruction executes the next is fetched, so an ALU
operation on two registers (operand fetch, execute, result write-back)
completes in one clock cycle.

### Status register

```table id=sreg caption="SREG – status register (I/O 0x3F)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | I | R/W | 0 | Global interrupt enable. 0 disables every interrupt regardless of its own enable bit. Cleared by hardware when an interrupt is taken, set by RETI; set and cleared by SEI and CLI. |
| 6 | T | R/W | 0 | Bit copy storage: source of BLD and destination of BST |
| 5 | H | R/W | 0 | Half carry, for BCD arithmetic |
| 4 | S | R/W | 0 | Sign: S = N ⊕ V |
| 3 | V | R/W | 0 | Two's complement overflow |
| 2 | N | R/W | 0 | Negative result |
| 1 | Z | R/W | 0 | Zero result |
| 0 | C | R/W | 0 | Carry |
```

SREG is updated by every ALU operation, as listed in the
[instruction set](#instruction-set), and is not saved or restored on
interrupt entry or exit; the handler must do that.

### General purpose registers

R0–R31, 8 bits each, also mapped at data addresses 0x00–0x1F. The
register file supports one 8-bit operand and 8-bit result, two 8-bit
operands and an 8-bit result, two 8-bit operands and a 16-bit result,
and a 16-bit operand and 16-bit result. R26–R31 pair into the 16-bit
pointers used for indirect data addressing, with fixed displacement,
post-increment and pre-decrement; Z also addresses program memory (LPM,
SPM, IJMP, ICALL).

```table id=xyz caption="Pointer registers"
| Pointer | High byte | Low byte |
|---|---|---|
| X | R27 (0x1B), XH | R26 (0x1A), XL |
| Y | R29 (0x1D), YH | R28 (0x1C), YL |
| Z | R31 (0x1F), ZH | R30 (0x1E), ZL |
```

### Stack pointer

The stack lives in SRAM and grows down; SP points to the top of the
stack. Software must set SP above 0x60 before any call or interrupt
(reset sets it to RAMEND). PUSH decrements SP by one, a call or
interrupt by two (return address); POP increments it by one, RET and
RETI by two.

```table id=sp caption="SPH (I/O 0x3E) and SPL (I/O 0x3D) – stack pointer"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 15:8 | SP[15:8] (SPH) | R/W | RAMEND | Stack pointer high byte; the register summary implements SP9, SP8 only |
| 7:0 | SP[7:0] (SPL) | R/W | RAMEND | Stack pointer low byte |
```

### Reset and interrupt handling

Reset and each interrupt source have their own vector at the bottom of
program memory ([memory map](#memory-map)); the lower the vector address,
the higher the priority (RESET, then INT0). An interrupt needs its own
enable bit and SREG.I set.

When an interrupt is taken, I is cleared, disabling all interrupts;
setting I in the handler allows nested interrupts, and RETI sets I
again. Two kinds of interrupt exist:

- **Flag-triggered.** The event sets an interrupt flag; vectoring to the
  handler clears it in hardware, and writing 1 to the flag clears it in
  software. A flag set while the interrupt or I is disabled is
  remembered and served, in priority order, once enabled.
- **Level-triggered.** Fires as long as the condition is present, and
  may have no flag; a condition that goes away before the interrupt is
  enabled is lost.

After RETI the CPU always executes one more main-program instruction
before serving a pending interrupt. CLI takes effect at once: no
interrupt runs after it, even one arriving with it. The instruction
after SEI always runs before any pending interrupt (so `SEI` then
`SLEEP` sleeps before the interrupt is served).

Response time is at least four clock cycles, during which the program
counter is pushed; the vector, normally a jump, adds three. A
multi-cycle instruction in progress completes first. From sleep, the
response is four cycles longer, on top of the wake-up start-up time.
RETI takes four cycles: the program counter (two bytes) is popped, SP
increases by two and I is set.

## Instruction set

Operands: Rd, Rr = registers R0–R31; Rdl = low register of a word pair
(Rdh:Rdl); K = constant; k = address; q = displacement; b = bit in a
register or I/O register (0–7); s = SREG bit (0–7); P = I/O address;
X, Y, Z = pointer registers. ∧ = AND, ∨ = OR, ⊕ = exclusive OR.

```table id=isa caption="Instruction set summary"
| Mnemonic | Operands | Description | Operation | Flags | Words [1] | Clocks |
|---|---|---|---|---|--:|--:|
| Arithmetic and logic | < | < | < | < | < | < |
| ADD | Rd, Rr | Add two registers | Rd ← Rd + Rr | Z, C, N, V, H | 1 | 1 |
| ADC | Rd, Rr | Add with carry two registers | Rd ← Rd + Rr + C | Z, C, N, V, H | 1 | 1 |
| ADIW | Rdl, K | Add immediate to word | Rdh:Rdl ← Rdh:Rdl + K | Z, C, N, V, S | 1 | 2 |
| SUB | Rd, Rr | Subtract two registers | Rd ← Rd − Rr | Z, C, N, V, H | 1 | 1 |
| SUBI | Rd, K | Subtract constant from register | Rd ← Rd − K | Z, C, N, V, H | 1 | 1 |
| SBC | Rd, Rr | Subtract with carry two registers | Rd ← Rd − Rr − C | Z, C, N, V, H | 1 | 1 |
| SBCI | Rd, K | Subtract with carry constant from register | Rd ← Rd − K − C | Z, C, N, V, H | 1 | 1 |
| SBIW | Rdl, K | Subtract immediate from word | Rdh:Rdl ← Rdh:Rdl − K | Z, C, N, V, S | 1 | 2 |
| AND | Rd, Rr | Logical AND registers | Rd ← Rd ∧ Rr | Z, N, V | 1 | 1 |
| ANDI | Rd, K | Logical AND register and constant | Rd ← Rd ∧ K | Z, N, V | 1 | 1 |
| OR | Rd, Rr | Logical OR registers | Rd ← Rd ∨ Rr | Z, N, V | 1 | 1 |
| ORI | Rd, K | Logical OR register and constant | Rd ← Rd ∨ K | Z, N, V | 1 | 1 |
| EOR | Rd, Rr | Exclusive OR registers | Rd ← Rd ⊕ Rr | Z, N, V | 1 | 1 |
| COM | Rd | One's complement | Rd ← 0xFF − Rd | Z, C, N, V | 1 | 1 |
| NEG | Rd | Two's complement | Rd ← 0x00 − Rd | Z, C, N, V, H | 1 | 1 |
| SBR | Rd, K | Set bit(s) in register | Rd ← Rd ∨ K | Z, N, V | 1 | 1 |
| CBR | Rd, K | Clear bit(s) in register | Rd ← Rd ∧ (0xFF − K) | Z, N, V | 1 | 1 |
| INC | Rd | Increment | Rd ← Rd + 1 | Z, N, V | 1 | 1 |
| DEC | Rd | Decrement | Rd ← Rd − 1 | Z, N, V | 1 | 1 |
| TST | Rd | Test for zero or minus | Rd ← Rd ∧ Rd | Z, N, V | 1 | 1 |
| CLR | Rd | Clear register | Rd ← Rd ⊕ Rd | Z, N, V | 1 | 1 |
| SER | Rd | Set register | Rd ← 0xFF | None | 1 | 1 |
| Branch | < | < | < | < | < | < |
| RJMP | k | Relative jump | PC ← PC + k + 1 | None | 1 | 2 |
| IJMP | | Indirect jump to (Z) | PC ← Z | None | 1 | 2 |
| RCALL | k | Relative subroutine call | PC ← PC + k + 1 | None | 1 | 3 |
| ICALL | | Indirect call to (Z) | PC ← Z | None | 1 | 3 |
| RET | | Subroutine return | PC ← STACK | None | 1 | 4 |
| RETI | | Interrupt return | PC ← STACK | I | 1 | 4 |
| CPSE | Rd, Rr | Compare, skip if equal | if (Rd = Rr) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| CP | Rd, Rr | Compare | Rd − Rr | Z, N, V, C, H | 1 | 1 |
| CPC | Rd, Rr | Compare with carry | Rd − Rr − C | Z, N, V, C, H | 1 | 1 |
| CPI | Rd, K | Compare register with immediate | Rd − K | Z, N, V, C, H | 1 | 1 |
| SBRC | Rr, b | Skip if bit in register cleared | if (Rr(b) = 0) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| SBRS | Rr, b | Skip if bit in register set | if (Rr(b) = 1) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| SBIC | P, b | Skip if bit in I/O register cleared | if (P(b) = 0) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| SBIS | P, b | Skip if bit in I/O register set | if (P(b) = 1) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| BRBS | s, k | Branch if status flag set | if (SREG(s) = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRBC | s, k | Branch if status flag cleared | if (SREG(s) = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BREQ | k | Branch if equal | if (Z = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRNE | k | Branch if not equal | if (Z = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRCS | k | Branch if carry set | if (C = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRCC | k | Branch if carry cleared | if (C = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRSH | k | Branch if same or higher | if (C = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRLO | k | Branch if lower | if (C = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRMI | k | Branch if minus | if (N = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRPL | k | Branch if plus | if (N = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRGE | k | Branch if greater or equal, signed | if (N ⊕ V = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRLT | k | Branch if less than zero, signed | if (N ⊕ V = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRHS | k | Branch if half carry set | if (H = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRHC | k | Branch if half carry cleared | if (H = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRTS | k | Branch if T set | if (T = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRTC | k | Branch if T cleared | if (T = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRVS | k | Branch if overflow set | if (V = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRVC | k | Branch if overflow cleared | if (V = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRIE | k | Branch if interrupt enabled | if (I = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRID | k | Branch if interrupt disabled | if (I = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| Bit and bit-test | < | < | < | < | < | < |
| SBI | P, b | Set bit in I/O register | I/O(P, b) ← 1 | None | 1 | 2 |
| CBI | P, b | Clear bit in I/O register | I/O(P, b) ← 0 | None | 1 | 2 |
| LSL | Rd | Logical shift left | Rd(n+1) ← Rd(n), Rd(0) ← 0 | Z, C, N, V | 1 | 1 |
| LSR | Rd | Logical shift right | Rd(n) ← Rd(n+1), Rd(7) ← 0 | Z, C, N, V | 1 | 1 |
| ROL | Rd | Rotate left through carry | Rd(0) ← C, Rd(n+1) ← Rd(n), C ← Rd(7) | Z, C, N, V | 1 | 1 |
| ROR | Rd | Rotate right through carry | Rd(7) ← C, Rd(n) ← Rd(n+1), C ← Rd(0) | Z, C, N, V | 1 | 1 |
| ASR | Rd | Arithmetic shift right | Rd(n) ← Rd(n+1), n = 0…6 | Z, C, N, V | 1 | 1 |
| SWAP | Rd | Swap nibbles | Rd(3:0) ← Rd(7:4), Rd(7:4) ← Rd(3:0) | None | 1 | 1 |
| BSET | s | Flag set | SREG(s) ← 1 | SREG(s) | 1 | 1 |
| BCLR | s | Flag clear | SREG(s) ← 0 | SREG(s) | 1 | 1 |
| BST | Rr, b | Bit store from register to T | T ← Rr(b) | T | 1 | 1 |
| BLD | Rd, b | Bit load from T to register | Rd(b) ← T | None | 1 | 1 |
| SEC, CLC | | Set, clear carry | C ← 1, C ← 0 | C | 1 | 1 |
| SEN, CLN | | Set, clear negative flag | N ← 1, N ← 0 | N | 1 | 1 |
| SEZ, CLZ | | Set, clear zero flag | Z ← 1, Z ← 0 | Z | 1 | 1 |
| SEI, CLI | | Global interrupt enable, disable | I ← 1, I ← 0 | I | 1 | 1 |
| SES, CLS | | Set, clear signed test flag | S ← 1, S ← 0 | S | 1 | 1 |
| SEV, CLV | | Set, clear two's complement overflow | V ← 1, V ← 0 | V | 1 | 1 |
| SET, CLT | | Set, clear T | T ← 1, T ← 0 | T | 1 | 1 |
| SEH, CLH | | Set, clear half carry flag | H ← 1, H ← 0 | H | 1 | 1 |
| Data transfer | < | < | < | < | < | < |
| MOV | Rd, Rr | Move between registers | Rd ← Rr | None | 1 | 1 |
| MOVW | Rd, Rr | Copy register word | Rd+1:Rd ← Rr+1:Rr | None | 1 | 1 |
| LDI | Rd, K | Load immediate | Rd ← K | None | 1 | 1 |
| LD | Rd, X / Y / Z | Load indirect | Rd ← (X), (Y), (Z) | None | 1 | 2 |
| LD | Rd, X+ / Y+ / Z+ | Load indirect, post-increment | Rd ← (X), X ← X + 1 (same for Y, Z) | None | 1 | 2 |
| LD | Rd, −X / −Y / −Z | Load indirect, pre-decrement | X ← X − 1, Rd ← (X) (same for Y, Z) | None | 1 | 2 |
| LDD | Rd, Y+q / Z+q | Load indirect with displacement | Rd ← (Y + q), (Z + q) | None | 1 | 2 |
| LDS | Rd, k | Load direct from data space | Rd ← (k) | None | 2 | 2 |
| ST | X / Y / Z, Rr | Store indirect | (X), (Y), (Z) ← Rr | None | 1 | 2 |
| ST | X+ / Y+ / Z+, Rr | Store indirect, post-increment | (X) ← Rr, X ← X + 1 (same for Y, Z) | None | 1 | 2 |
| ST | −X / −Y / −Z, Rr | Store indirect, pre-decrement | X ← X − 1, (X) ← Rr (same for Y, Z) | None | 1 | 2 |
| STD | Y+q / Z+q, Rr | Store indirect with displacement | (Y + q), (Z + q) ← Rr | None | 1 | 2 |
| STS | k, Rr | Store direct to data space | (k) ← Rr | None | 2 | 2 |
| LPM | | Load program memory | R0 ← (Z) | None | 1 | 3 |
| LPM | Rd, Z | Load program memory | Rd ← (Z) | None | 1 | 3 |
| LPM | Rd, Z+ | Load program memory, post-increment | Rd ← (Z), Z ← Z + 1 | None | 1 | 3 |
| SPM | | Store program memory | (Z) ← R1:R0 | None | 1 | [2] |
| IN | Rd, P | In port | Rd ← P | None | 1 | 1 |
| OUT | P, Rr | Out port | P ← Rr | None | 1 | 1 |
| PUSH | Rr | Push register on stack | STACK ← Rr | None | 1 | 2 |
| POP | Rd | Pop register from stack | Rd ← STACK | None | 1 | 2 |
| MCU control | < | < | < | < | < | < |
| NOP | | No operation | | None | 1 | 1 |
| SLEEP | | Sleep | see power management and sleep modes | None | 1 | 1 |
| WDR | | Watchdog reset | see reset and watchdog | None | 1 | 1 |
| BREAK | | Break | on-chip debug only | None | 1 | [3] |

[1] Not in the source table: most AVR instructions are one 16-bit word; LDS and STS are the 32-bit ones.
[2] Not given; SPM halts the CPU during the flash operation (see self-programming).
[3] N/A.
```

## Memory map

```table id=mem-sizes caption="Memory sizes"
| Memory | ATtiny25 | ATtiny45 | ATtiny85 |
|---|---|---|---|
| Flash | 2 KB, 1024 × 16 | 4 KB, 2048 × 16 | 8 KB, 4096 × 16 |
| Program counter | 10 bits | 11 bits | 12 bits |
| Program memory addresses (words) | 0x0000–0x03FF | 0x0000–0x07FF | 0x0000–0x0FFF |
| SRAM | 128 bytes | 256 bytes | 512 bytes |
| Data space end (RAMEND) | 0x00DF | 0x015F | 0x025F |
| EEPROM | 128 bytes | 256 bytes | 512 bytes |
```

**Program memory.** Flash, 16 bits wide since every instruction is 16 or
32 bits; at least 10,000 write/erase cycles. Constant tables can sit
anywhere in it (read with LPM). Programming: see memory programming
under Peripherals.

**Data memory.** One linear space: 0x0000–0x001F the 32 registers,
0x0020–0x005F the 64 I/O registers, 0x0060–RAMEND the SRAM. Five
addressing modes reach all of it: direct (whole data space), indirect
with displacement (63 locations above Y or Z), indirect, indirect with
pre-decrement and indirect with post-increment (X, Y, Z).

**EEPROM.** A separate data space of single bytes, at least 100,000
write/erase cycles; see EEPROM under Peripherals.

An SRAM access takes two clk~CPU~ cycles: the address is computed in
the first and is valid in the second, when the WR or RD strobe occurs.

```wavedrom id=sram-access caption="On-chip data SRAM access cycles"
{"signal": [
  {"name": "clkCPU", "wave": "p.."},
  {"name": "Address", "wave": "==.", "data": ["compute address", "address valid"]},
  {"name": "Data (write)", "wave": "z=z", "data": ["data"]},
  {"name": "WR", "wave": "010"},
  {"name": "Data (read)", "wave": "z=z", "data": ["data"], "phase": -0.2},
  {"name": "RD", "wave": "010"}
 ],
 "config": {"hscale": 2}}
```

**I/O memory.** All I/O registers and peripherals sit in the I/O space
([register summary](#registers)). IN and OUT use I/O addresses
0x00–0x3F; LD and ST reach them as data space, at the I/O address plus
0x20. Registers 0x00–0x1F are bit-accessible with SBI and CBI and
testable with SBIS and SBIC. Some status flags are cleared by writing 1;
SBI and CBI touch only the named bit, so they are safe on such
registers. Reserved bits read as zero and should be written zero;
reserved addresses must never be written. GPIOR0–GPIOR2 (0x11–0x13)
are general purpose I/O registers for any use.

```table id=vectors caption="Reset and interrupt vectors, highest priority first"
| Vector | Program address | Source | Interrupt definition |
|--:|---|---|---|
| 1 | 0x0000 | RESET | External pin, power-on reset, brown-out reset, watchdog reset |
| 2 | 0x0001 | INT0 | External interrupt request 0 |
| 3 | 0x0002 | PCINT0 | Pin change interrupt request 0 |
| 4 | 0x0003 | TIMER1_COMPA | Timer/Counter1 compare match A |
| 5 | 0x0004 | TIMER1_OVF | Timer/Counter1 overflow |
| 6 | 0x0005 | TIMER0_OVF | Timer/Counter0 overflow |
| 7 | 0x0006 | EE_RDY | EEPROM ready |
| 8 | 0x0007 | ANA_COMP | Analog comparator |
| 9 | 0x0008 | ADC | ADC conversion complete |
| 10 | 0x0009 | TIMER1_COMPB | Timer/Counter1 compare match B |
| 11 | 0x000A | TIMER0_COMPA | Timer/Counter0 compare match A |
| 12 | 0x000B | TIMER0_COMPB | Timer/Counter0 compare match B |
| 13 | 0x000C | WDT | Watchdog time-out |
| 14 | 0x000D | USI_START | USI start condition |
| 15 | 0x000E | USI_OVF | USI counter overflow |
```

Each vector is one word, normally an RJMP to the handler; the main
program may start at 0x000F. Vectors of interrupts the program never
enables may hold ordinary code.

## Peripherals

### EEPROM

128/256/512 bytes (ATtiny25/45/85) of data EEPROM in a separate data
space, read and written one byte at a time, endurance at least 100,000
write/erase cycles. EEAR, EEDR and EECR are in the I/O space. A read
halts the CPU for four clock cycles before the next instruction, a
write for two. The calibrated internal oscillator times EEPROM
accesses (see OSCCAL under Clock system: do not calibrate above
8.8 MHz while writing EEPROM or flash). Programming times are in table
ee-modes; EEPE stays set until the operation completes, and no other
EEPROM operation is possible meanwhile.

**Programming modes** (EEPM[1:0], then EEMPE and EEPE, see the timed
sequence below):

- **Atomic** (EEPM = 00): write the address to EEAR and the data to
  EEDR; EEPE erases and writes in one operation.
- **Split**: erase (EEPM = 01, address in EEAR) and write (EEPM = 10,
  address in EEAR, data in EEDR) are separate operations, so the slow
  erase can be done ahead (after power-up) and only the short write is
  needed later (when the supply may be falling). A location written
  without a prior erase holds undefined data.

**Write sequence** (timed; interrupts must not occur during it):

1. Wait until EEPE is 0.
2. Write EEPM[1:0] to EECR (with EEMPE = EEPE = 0).
3. Write EEAR (EEARH, EEARL) and, for a write, EEDR.
4. Write EEMPE = 1.
5. Within four clock cycles, write EEPE = 1.

**Read:** wait until EEPE is 0, write EEAR, write EERE = 1; the data is
in EEDR at once (the CPU halts four cycles). While a write is in
progress, EEPROM can't be read and EEAR can't be changed.

**Preventing corruption.** Too low a V~CC~ corrupts EEPROM data, both
because the write needs a minimum voltage and because the CPU may
execute wrongly. Keep the part in reset while V~CC~ is too low, with
the brown-out detector or an external reset circuit. A write in
progress when a reset occurs completes if the supply is sufficient.

```table id=ee-modes caption="EEPROM programming modes"
| EEPM1 | EEPM0 | Programming time | Operation |
|:-:|:-:|---|---|
| 0 | 0 | 3.4 ms | Erase and write in one operation (atomic) |
| 0 | 1 | 1.8 ms | Erase only |
| 1 | 0 | 1.8 ms | Write only |
| 1 | 1 | — | Reserved |
```

#### EEARH – EEPROM address register high (0x1F)

```table id=ee-earh caption="EEARH bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:1 | — | R | 0 | Reserved, read 0 |
| 0 | EEAR8 | R/W | X (ATtiny85), 0 | EEPROM address bit 8, ATtiny85 only; reserved (reads 0) in ATtiny25/45 |
```

#### EEARL – EEPROM address register low (0x1E)

```table id=ee-earl caption="EEARL bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | EEAR7 | R/W | X | EEPROM address bit 7; reserved (reads 0) in ATtiny25 |
| 6:0 | EEAR[6:0] | R/W | X | EEPROM address, linear 0 … 127/255/511. EEAR is undefined after reset: write it before the first access. |
```

#### EEDR – EEPROM data register (0x1D)

```table id=ee-edr caption="EEDR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | EEDR[7:0] | R/W | 0 | Write: data to write at EEAR. Read: data read from EEAR. |
```

#### EECR – EEPROM control register (0x1C)

```table id=ee-ecr caption="EECR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved, read 0; write bit 7 as 0 |
| 5:4 | EEPM[1:0] | R/W | X | Programming mode for the next EEPE write, table ee-modes. Writes are ignored while EEPE is set. Reset to 00 unless the EEPROM is busy programming. |
| 3 | EERIE | R/W | 0 | EEPROM ready interrupt enable (with SREG I). The interrupt fires continuously while the non-volatile memory is ready for programming. |
| 2 | EEMPE | R/W | 0 | Master program enable: EEPE = 1 written within four clock cycles programs the EEPROM; otherwise EEPE has no effect. Cleared by hardware four cycles after it is written. |
| 1 | EEPE | R/W | X | Program enable: starts the operation EEPM selects (EEMPE must be 1 first). Cleared by hardware when the access time has elapsed. The CPU halts two cycles after EEPE is set. |
| 0 | EERE | R/W | 0 | Read enable: write 1 to read the byte at EEAR into EEDR. Poll EEPE first. |
```

### Clock system

Clock domains: clk~CPU~ (core, register file, SREG, stack), clk~I/O~
(most I/O modules such as the timers, and the external interrupt
module, though some external interrupts are detected asynchronously
and work with clk~I/O~ halted), clk~FLASH~ (flash interface, normally
running with clk~CPU~), clk~ADC~ (its own domain, so CPU and I/O clocks
can be halted to cut noise during conversions), and clk~PCK~, the fast
peripheral clock for Timer/Counter1. Sleep modes halt unused domains.
Block diagram: the clock multiplexer selects external clock, PLL,
calibrated RC, 128 kHz watchdog oscillator, low-frequency or crystal
oscillator; the system clock prescaler divides it into clk~CPU~,
clk~I/O~, clk~ADC~ and clk~FLASH~; the watchdog oscillator also times
reset.

**PLL (clk~PCK~).** The PLL multiplies the internal 8.0 MHz RC
oscillator by 8 to give a 64 MHz fast peripheral clock; with PLLCSR
LSM = 1 it uses the RC oscillator divided by two, giving 32 MHz (LSM
can't be set while the PLL clock is the system clock). clk~PCK~ or a
clock prescaled from it can clock Timer/Counter1 or, divided by 4
(16 MHz), the system. The PLL tracks OSCCAL; if the RC oscillator is
tuned above 8 MHz the PLL output saturates at 85 MHz (worst case) and
is no longer locked, so don't calibrate above 8 MHz when using the
PLL. The PLL is on when PLLCSR PLLE = 1 or CKSEL = 0001 or 0011;
PLLCSR PLOCK = 1 when locked (PLLCSR is described under
Timer/Counter1). The RC oscillator and PLL are off in power-down and
standby.

**ATtiny15 compatibility mode** (CKSEL = 0011): the RC oscillator is
calibrated to 6.4 MHz and the PLL multiplies by 4, giving a 25.6 MHz
clk~PCK~ as in ATtiny15; the system clock is always 6.4 MHz / 4 =
1.6 MHz, the system clock prescaler is disabled (CLKPR and CKDIV8 have
no effect), and low-speed mode (LSM) is not implemented.

```table id=clk-sel caption="Clock source selection [1]"
| Clock source | CKSEL[3:0] |
|---|---|
| External clock | 0000 |
| High-frequency PLL clock, 16 MHz | 0001 |
| Calibrated internal oscillator, 8.0 MHz [2] | 0010 |
| Calibrated internal oscillator, 6.4 MHz, ATtiny15 compatibility mode (1.6 MHz system clock) | 0011 |
| Internal 128 kHz oscillator | 0100 |
| Low-frequency crystal oscillator | 0110 |
| Crystal oscillator / ceramic resonator | 1000–1111 |
| Reserved | 0101, 0111 |

[1] For all fuses, 1 = unprogrammed, 0 = programmed.
[2] Shipped setting.
```

When the CPU wakes from power-down, the selected source times the
start-up so the oscillator is stable before execution. From reset an
additional delay, timed by the watchdog oscillator, lets the supply
settle: 512 cycles (4 ms typical) or 8K (8,192) cycles (64 ms typical).

**Default.** Shipped with CKSEL = 0010, SUT = 10 and CKDIV8
programmed: internal RC oscillator at 8 MHz, longest start-up, divided
by 8, so a 1.0 MHz system clock.

#### External clock

Drive CLKI from the external source (CKSEL = 0000). The frequency must
not change by more than 2 % from one cycle to the next; keep the part
in reset during larger changes, or change the frequency with the
system clock prescaler instead.

```table id=clk-sut-ext caption="Start-up times, external clock, calibrated 8 MHz RC oscillator and 128 kHz oscillator"
| SUT[1:0] | Start-up time from power-down | Additional delay from reset (V~CC~ = 5.0 V) | Recommended usage |
|---|---|---|---|
| 00 | 6 CK | 14 CK [1] | BOD enabled |
| 01 | 6 CK | 14 CK + 4 ms | Fast rising power |
| 10 | 6 CK | 14 CK + 64 ms [2] | Slowly rising power |
| 11 | Reserved | < | < |

[1] Calibrated RC and 128 kHz oscillators: with the RSTDISBL fuse programmed, this becomes 14 CK + 4 ms so that programming mode can be entered.
[2] Shipped setting (calibrated RC oscillator).
```

#### High-frequency PLL clock

CKSEL = 0001: the 64 MHz PLL clock divided by four, 16 MHz nominal.

```table id=clk-sut-pll caption="Start-up times, high-frequency PLL clock"
| SUT[1:0] | Start-up time from power-down | Additional delay from power-on reset (V~CC~ = 5.0 V) | Recommended usage |
|---|---|---|---|
| 00 | 14 CK + 1K (1024) CK + 4 ms | 4 ms | BOD enabled |
| 01 | 14 CK + 16K (16384) CK + 4 ms | 4 ms | Fast rising power |
| 10 | 14 CK + 1K (1024) CK + 64 ms | 4 ms | Slowly rising power |
| 11 | 14 CK + 16K (16384) CK + 64 ms | 4 ms | Slowly rising power |
```

#### Calibrated internal oscillator

About 8.0 MHz (CKSEL = 0010), or 6.4 MHz in ATtiny15 compatibility
mode (CKSEL = 0011); no external components. Voltage and temperature
dependent; at reset hardware loads the factory calibration byte into
OSCCAL (factory and user calibration accuracy: see
[switching characteristics](#switching-characteristics)). The watchdog
oscillator still clocks the watchdog and reset time-out. Start-up
times for 8 MHz are in table clk-sut-ext.

```table id=clk-sut-t15 caption="Start-up times, calibrated RC oscillator in ATtiny15 compatibility mode"
| SUT[1:0] | Start-up time from power-down | Additional delay from reset (V~CC~ = 5.0 V) |
|---|---|---|
| 00 | 6 CK | 14 CK + 64 ms |
| 01 | 6 CK | 14 CK + 64 ms |
| 10 | 6 CK | 14 CK + 4 ms |
| 11 | 1 CK | 14 CK [1] |

[1] With the RSTDISBL fuse programmed, 14 CK + 4 ms so that programming mode can be entered.
```

#### Internal 128 kHz oscillator

Low-power 128 kHz oscillator (nominal at 3 V, 25 °C), CKSEL = 0100.
Start-up times: table clk-sut-ext.

#### Low-frequency crystal oscillator

A 32.768 kHz watch crystal between XTAL1 and XTAL2, CKSEL = 0110.
Internal load capacitance: 16 pF on XTAL1 and 6 pF on XTAL2; see the
crystal maker's data for the load it needs.

```table id=clk-sut-lf caption="Start-up times, low-frequency crystal oscillator"
| SUT[1:0] | Start-up time from power-down | Additional delay from reset (V~CC~ = 5.0 V) | Recommended usage |
|---|---|---|---|
| 00 | 1K (1024) CK [1] | 4 ms | Fast rising power or BOD enabled |
| 01 | 1K (1024) CK [1] | 64 ms | Slowly rising power |
| 10 | 32K (32768) CK | 64 ms | Stable frequency at start-up |
| 11 | Reserved | < | < |

[1] Only if frequency stability at start-up is not important.
```

#### Crystal oscillator / ceramic resonator

XTAL1 and XTAL2 are the input and output of an inverting amplifier for
a quartz crystal or ceramic resonator, with C1 and C2 from each pin to
GND. C1 and C2 are always equal; their best value depends on the
crystal, stray capacitance and noise. For resonators use the maker's
values. CKSEL[3:1] picks the frequency range, CKSEL0 with SUT[1:0] the
start-up time. Crystal oscillators can't be output on CLKO.

```table id=clk-xtal caption="Crystal oscillator operating modes"
| CKSEL[3:1] | Frequency range | C1, C2 for crystals |
|---|---|---|
| 100 [1] | 0.4–0.9 MHz | — |
| 101 | 0.9–3.0 MHz | 12–22 pF |
| 110 | 3.0–8.0 MHz | 12–22 pF |
| 111 | 8.0 MHz and above | 12–22 pF |

[1] Ceramic resonators only, not crystals.
```

```table id=clk-sut-xtal caption="Start-up times, crystal oscillator / ceramic resonator"
| CKSEL0 | SUT[1:0] | Start-up time from power-down | Additional delay from reset | Recommended usage |
|---|---|---|---|---|
| 0 | 00 | 258 CK [1] | 14 CK + 4 ms | Ceramic resonator, fast rising power |
| 0 | 01 | 258 CK [1] | 14 CK + 64 ms | Ceramic resonator, slowly rising power |
| 0 | 10 | 1K (1024) CK [2] | 14 CK | Ceramic resonator, BOD enabled |
| 0 | 11 | 1K (1024) CK [2] | 14 CK + 4 ms | Ceramic resonator, fast rising power |
| 1 | 00 | 1K (1024) CK [2] | 14 CK + 64 ms | Ceramic resonator, slowly rising power |
| 1 | 01 | 16K (16384) CK | 14 CK | Crystal oscillator, BOD enabled |
| 1 | 10 | 16K (16384) CK | 14 CK + 4 ms | Crystal oscillator, fast rising power |
| 1 | 11 | 16K (16384) CK | 14 CK + 64 ms | Crystal oscillator, slowly rising power |

[1] Only when not operating close to the maximum frequency and frequency stability at start-up is not important. Not for crystals.
[2] For ceramic resonators, with frequency stability at start-up. Usable with crystals when not close to the maximum frequency and stability at start-up is not important.
```

#### System clock prescaler

CLKPR divides the selected source for clk~CPU~, clk~I/O~, clk~ADC~ and
clk~FLASH~ (all clock sources), to save power. Switching is
glitch-free and no intermediate frequency is higher than either the
old or the new setting. The new setting takes effect between T1 + T2
and T1 + 2 × T2 after CLKPS is written (T1 = old clock period, T2 = new
period), with two active clock edges in that interval; the exact time
can't be predicted, since the prescaler's ripple counter runs at the
undivided clock.

#### Clock output

With the CKOUT fuse programmed, the system clock (after the prescaler)
is output on CLKO (PB4) when that pin isn't XTAL2, overriding the pin's
normal function; not during reset. Usable with the internal RC, WDT
oscillator, PLL and external clock; not with crystal oscillators.

#### OSCCAL – Oscillator calibration register (0x31)

```table id=clk-osccal caption="OSCCAL bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | CAL7 | R/W | factory | Range: 0 = low frequency range, 1 = high range. The ranges overlap: OSCCAL = 0x7F gives a higher frequency than 0x80. |
| 6:0 | CAL[6:0] | R/W | factory | Tuning within the range: 0x00 lowest, 0x7F highest frequency |
```

Reset loads the factory calibration value (factory-calibrated
frequency). Software may write OSCCAL to change the frequency, within
the range in the switching characteristics; outside it calibration is
not guaranteed. The oscillator times EEPROM and flash writes: do not
calibrate above 8.8 MHz when writing them. Change OSCCAL in small
steps, at most 0x20 per change; more than 2 % change between clock
cycles may cause unpredictable behavior, so keep the part in reset
during larger changes.

```table id=clk-oscrange caption="Internal RC oscillator frequency range (typical, relative to nominal)"
| OSCCAL value | Lowest frequency | Highest frequency |
|---|---|---|
| 0x00 | 50 % | 100 % |
| 0x3F | 75 % | 150 % |
| 0x7F | 100 % | 200 % |
```

#### CLKPR – Clock prescale register (0x26)

```table id=clk-clkpr caption="CLKPR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | CLKPCE | R/W | 0 | Prescaler change enable. Updated only when the other CLKPR bits are written 0 at the same time; cleared by hardware four cycles after it is written or when CLKPS is written. Rewriting it within the four cycles neither extends the time-out nor clears it. |
| 6:4 | — | R | 0 | Reserved, read 0 |
| 3:0 | CLKPS[3:0] | R/W | 0000 or 0011 [1] | Division factor, table clk-div; may be changed at run time |

[1] 0000 with CKDIV8 unprogrammed, 0011 (divide by 8) with CKDIV8 programmed (shipped). Any value may be written regardless of CKDIV8; software must pick a division that keeps the clock within the maximum frequency for the operating conditions.
```

Timed sequence to change CLKPS (interrupts disabled):

1. Write CLKPR = 0x80 (CLKPCE = 1, all other bits 0).
2. Within four cycles, write the new CLKPS value with CLKPCE = 0.

```table id=clk-div caption="Clock prescaler select [1]"
| CLKPS[3:0] | Division factor |
|---|---|
| 0000 | 1 |
| 0001 | 2 |
| 0010 | 4 |
| 0011 | 8 |
| 0100 | 16 |
| 0101 | 32 |
| 0110 | 64 |
| 0111 | 128 |
| 1000 | 256 |
| 1001–1111 | Reserved |

[1] Disabled in ATtiny15 compatibility mode: the system clock is always 1.6 MHz.
```

### Power management and sleep modes

Writing MCUCR SE = 1 and executing SLEEP enters the sleep mode MCUCR
SM[1:0] selects. An enabled interrupt wakes the MCU: it is halted four
cycles beyond the start-up time, runs the interrupt routine, then
continues after the SLEEP. Register file and SRAM are unaltered. A
reset during sleep wakes the MCU into the reset vector. A level
interrupt used for wake-up must be held long enough to wake the MCU and
enter the interrupt routine (see Interrupts).

- **Idle** (SM = 00): stops clk~CPU~ and clk~FLASH~; analog comparator,
  ADC, USI, timers, watchdog and interrupts keep running, so external and
  internal interrupts (timer overflow) wake the part. Setting ACSR ACD
  turns the comparator off when its interrupt isn't needed. An enabled
  ADC starts a conversion on entry.
- **ADC noise reduction** (SM = 01): also stops clk~I/O~; ADC, external
  interrupts and watchdog keep running. An enabled ADC starts a
  conversion on entry. Wake-up: ADC conversion complete, external reset,
  watchdog reset, brown-out reset, SPM/EEPROM ready, INT0 level
  interrupt, pin change interrupt (and USI start condition, watchdog
  interrupt; table pm-modes).
- **Power-down** (SM = 10): stops the oscillator and every generated
  clock; only asynchronous modules run. Wake-up: external reset,
  watchdog reset or interrupt, brown-out reset, USI start condition,
  INT0 level interrupt, pin change interrupt.

```table id=pm-modes caption="Active clock domains and wake-up sources in the sleep modes"
| Sleep mode | clk~CPU~ | clk~FLASH~ | clk~IO~ | clk~ADC~ | clk~PCK~ | Main clock source enabled | INT0 and pin change | SPM/EEPROM ready | USI start condition | ADC | Other I/O | Watchdog interrupt |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Idle | | | X | X | X | X | X | X | X | X | X | X |
| ADC noise reduction | | | | X | | X | X [1] | X | X | X | | X |
| Power-down | | | | | | | X [1] | | X | | | X |

[1] INT0: level interrupt only.
```

**Software BOD disable.** With the BOD enabled by the BODLEVEL fuses,
it normally keeps monitoring V~CC~ in sleep. Writing MCUCR BODS = 1
(timed sequence, below) turns it off during power-down, so sleep
current drops to the fuse-disabled level. The BOD is off right after
sleep is entered and is re-enabled automatically on wake-up. With the
BOD disabled, the wake-up time equals the wake-up from reset: set
SUT[1:0] and CKSEL[3:0] so the bandgap reference has started and the
BOD works before code runs. Implemented only in ATtiny25 revision E,
ATtiny45 revision D, ATtiny85 revision C and newer (revision marked on
the bottom of 8P3 and 8S2 packages, the top of 20M1); in older parts
BODS and BODSE are unused and read 0.

**Power reduction register.** PRR stops the clock to single
peripherals in active and idle mode (other sleep modes stop it
anyway). The peripheral's state is frozen and its I/O registers can't
be read or written; resources it holds stay occupied, so disable the
peripheral first. Clearing the PRR bit restores the state from before
shutdown.

**Minimizing power.** Use the deepest sleep mode possible and disable
unneeded functions:

- ADC: stays on in every sleep mode if enabled; disable it first. The
  first conversion after re-enabling is an extended conversion.
- Analog comparator: disable it before idle if unused and before ADC
  noise reduction; other modes disable it automatically, except that a
  comparator using the internal reference keeps the reference on in any
  mode, so disable it in all sleep modes then.
- Brown-out detector: when enabled by fuses it runs in all sleep modes
  and is a significant part of deep-sleep current; turn it off if not
  needed (or use software BOD disable).
- Internal voltage reference: on whenever the BOD, the comparator
  (ACBG) or the ADC needs it; off otherwise. After turning it on, allow
  its start-up time before using the output (see Reset and watchdog).
- Watchdog: when enabled it runs in all sleep modes; turn it off if not
  needed.
- Port pins: drive no resistive loads. When clk~I/O~ and clk~ADC~ are
  both stopped, input buffers are disabled except those needed for
  wake-up (see I/O ports). A floating input, or one near V~CC~/2, with
  its buffer enabled draws excessive current; for analog inputs disable
  the digital buffer with DIDR0 at all times (see ADC).

#### MCUCR – MCU control register (0x35)

```table id=pm-mcucr caption="MCUCR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | BODS | R | 0 | BOD sleep: 1 = BOD off during the next power-down sleep. Set by the timed sequence below; active three clock cycles after it is set, cleared automatically after three more; SLEEP must execute while it is active. |
| 6 | PUD | R/W | 0 | Pull-up disable: 1 disables all I/O pull-ups even when DDxn = 0 and PORTxn = 1 (see I/O ports) |
| 5 | SE | R/W | 0 | Sleep enable: SLEEP enters sleep only when SE = 1. Set it just before SLEEP and clear it right after wake-up. |
| 4:3 | SM[1:0] | R/W | 00 | Sleep mode: 00 = idle; 01 = ADC noise reduction; 10 = power-down; 11 = reserved |
| 2 | BODSE | R | 0 | BOD sleep enable: enables writing BODS (timed sequence) |
| 1:0 | ISC0[1:0] | R/W | 00 | INT0 sense control, see Interrupts |
```

Timed sequence to set BODS:

1. Write BODS = 1 and BODSE = 1 in one write.
2. Within four clock cycles, write BODS = 1 and BODSE = 0.
3. Execute SLEEP within the three cycles BODS is active.

#### PRR – Power reduction register (0x20)

```table id=pm-prr caption="PRR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | — | R | 0 | Reserved, read 0 |
| 3 | PRTIM1 | R/W | 0 | 1 = shut down Timer/Counter1; it resumes as before when cleared |
| 2 | PRTIM0 | R/W | 0 | 1 = shut down Timer/Counter0; it resumes as before when cleared |
| 1 | PRUSI | R/W | 0 | 1 = stop the USI clock; re-initialize the USI after clearing |
| 0 | PRADC | R/W | 0 | 1 = shut down the ADC (disable the ADC first). The analog comparator uses the ADC clock in part and can't be used while PRADC = 1. |
```

### Reset and watchdog

During reset all I/O registers take their initial values and execution
starts at the reset vector, which must hold an RJMP to the reset
handler (unused interrupt vectors may hold ordinary code). The I/O
ports go to their initial state as soon as a reset source is active,
without any clock. When all reset sources are inactive, a delay counter
stretches the internal reset by t~TOUT~ (set by the SUT and CKSEL
fuses; see Clock system) so the supply settles. Reset logic: the
power-on, brown-out, external (spike-filtered !{RESET} with pull-up)
and watchdog reset circuits set the MCUSR flags and drive the internal
reset through the delay counters, clocked by the watchdog oscillator.
Electrical values (V~POT~, V~RST~, t~RST~, V~BOT~, t~BOD~, bandgap
start-up) are in [electrical characteristics](#electrical-characteristics).

Four reset sources:

- **Power-on reset**: an on-chip detector holds the part in reset while
  V~CC~ is below V~POT~. Rising through V~POT~ starts the delay counter;
  falling below it asserts reset again at once. It can also detect a
  supply failure.
- **External reset**: a low level on !{RESET} (when enabled) longer than
  the minimum pulse width resets the part even without a clock; shorter
  pulses are not guaranteed to. When !{RESET} rises through V~RST~, the
  part starts after t~TOUT~. If !{RESET} is held low past power-up, the
  delay counter starts at its rising edge.
- **Watchdog reset**: a time-out gives a one-CK reset pulse; t~TOUT~
  counts from its falling edge.
- **Brown-out reset**: with the BOD enabled by the BODLEVEL fuses,
  V~CC~ below V~BOT-~ = V~BOT~ - V~HYST~/2 resets the part at once; when
  V~CC~ rises above V~BOT+~ = V~BOT~ + V~HYST~/2 the part starts after
  t~TOUT~. A drop shorter than t~BOD~ is not detected.

```wavedrom id=rst-por caption="Start-up: RESET tied to VCC, and RESET released later"
{"signal": [
  {"name": "VCC above VPOT", "wave": "01........"},
  {"name": "RESET above VRST", "wave": "0...1.....", "node": "....A"},
  {"name": "TIME-OUT", "wave": "0...1..0..", "node": "....B..C"},
  {"name": "INTERNAL RESET", "wave": "1......0..", "node": ".......D"}
 ],
 "edge": ["B<->C tTOUT"]}
```

With !{RESET} tied to V~CC~ the time-out starts when V~CC~ reaches
V~POT~; with !{RESET} held low externally it starts when !{RESET}
reaches V~RST~. The external-reset, brown-out and watchdog cases are
the same: the internal reset is released t~TOUT~ after the last source
goes inactive.

**Internal voltage reference.** A bandgap reference serves the BOD and
can feed the analog comparator or the ADC. It is on only while the BOD
is enabled by the BODLEVEL fuses, ACSR ACBG = 1, or the ADC is enabled.
Without the BOD, allow its start-up time after setting ACBG or enabling
the ADC before using the comparator or ADC output. Avoid all three
conditions to have it off in power-down.

**Watchdog timer.** Clocked by a separate 128 kHz on-chip oscillator,
with ten time-out periods (table wd-prescale). WDR resets the timer;
so do disabling it and a chip reset. On time-out it resets the part,
or interrupts instead (useful to wake from power-down), per table
wd-modes. The WDTON fuse picks a safety level:

```table id=wd-safety caption="Watchdog configuration by the WDTON fuse"
| WDTON | Safety level | WDT initial state | To disable the WDT | To change the time-out |
|---|---|---|---|---|
| Unprogrammed | 1 | Disabled | Timed sequence | No limitations |
| Programmed | 2 | Enabled | Always enabled | Timed sequence |
```

Timed sequences (interrupts disabled):

- **Level 1, disable:** (1) in one write, set WDCE = 1 and WDE = 1 (WDE
  must be written 1 even if already 1); (2) within four clock cycles,
  in one write, set WDE and WDP as wanted with WDCE = 0. WDRF overrides
  WDE, so clear MCUSR WDRF first. Enabling (WDE = 1) needs no sequence.
- **Level 2, change time-out:** (1) in one write, set WDCE = 1 and
  WDE = 1; (2) within four clock cycles, write WDP as wanted with
  WDCE = 0 (WDE's value is irrelevant). The watchdog can't be disabled.

If the application doesn't use the watchdog, run the disable sequence
at initialization (WDR, clear WDRF, then the level-1 disable sequence,
keeping the old WDP while setting WDCE and WDE): an accidental enable,
by a runaway pointer or a brown-out, would otherwise cause a loop of
watchdog resets.

#### MCUSR – MCU status register (0x34)

Read and clear MCUSR as early as possible in the program; the flags of
the next reset then identify its source.

```table id=wd-mcusr caption="MCUSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | — | R | 0 | Reserved, read 0 |
| 3 | WDRF | R/W | [1] | Watchdog reset flag; cleared by power-on reset or by writing 0 |
| 2 | BORF | R/W | [1] | Brown-out reset flag; cleared by power-on reset or by writing 0 |
| 1 | EXTRF | R/W | [1] | External reset flag; cleared by power-on reset or by writing 0 |
| 0 | PORF | R/W | [1] | Power-on reset flag; cleared only by writing 0 |

[1] Set by the reset source that occurred.
```

#### WDTCR – Watchdog timer control register (0x21)

```table id=wd-wdtcr caption="WDTCR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | WDIF | R/W | 0 | Time-out interrupt flag: set on a time-out in interrupt mode; cleared by executing the interrupt vector or by writing 1. The interrupt runs when WDIE and SREG I are set. |
| 6 | WDIE | R/W | 0 | Time-out interrupt enable, table wd-modes. With WDE = 1, hardware clears WDIE on a time-out, so the next time-out resets; set WDIE again after each interrupt to avoid the reset. |
| 5 | WDP3 | R/W | 0 | Prescaler bit 3, table wd-prescale |
| 4 | WDCE | R/W | 0 | Change enable: must be 1 when WDE is written 0 or WDP is changed; cleared by hardware four clock cycles after it is written |
| 3 | WDE | R/W | X | Watchdog enable; can be cleared only with WDCE = 1 (timed sequence). Always 1 in safety level 2. In safety level 1, WDE is forced to 1 while MCUSR WDRF = 1. |
| 2:0 | WDP[2:0] | R/W | 000 | Prescaler bits 2:0, table wd-prescale |
```

```table id=wd-modes caption="Watchdog timer configuration"
| WDE | WDIE | Watchdog state | Action on time-out |
|:-:|:-:|---|---|
| 0 | 0 | Stopped | None |
| 0 | 1 | Running | Interrupt |
| 1 | 0 | Running | Reset |
| 1 | 1 | Running | Interrupt |
```

```table id=wd-prescale caption="Watchdog timer prescale select"
| WDP[3:0] | WDT oscillator cycles | Typical time-out at V~CC~ = 5.0 V |
|---|---|---|
| 0000 | 2K (2048) | 16 ms |
| 0001 | 4K (4096) | 32 ms |
| 0010 | 8K (8192) | 64 ms |
| 0011 | 16K (16384) | 0.125 s |
| 0100 | 32K (32768) | 0.25 s |
| 0101 | 64K (65536) | 0.5 s |
| 0110 | 128K (131072) | 1.0 s |
| 0111 | 256K (262144) | 2.0 s |
| 1000 | 512K (524288) | 4.0 s |
| 1001 | 1024K (1048576) | 8.0 s |
| 1010–1111 | Reserved [1] | |

[1] If selected, one of the valid settings below 1010 is used.
```

### Interrupts

The reset and interrupt vectors are listed in the [memory map](#memory-map).

**External interrupts** come from INT0 and from the PCINT[5:0] pins.
Enabled, they trigger even when the pin is an output, which gives a
way to generate a software interrupt. A pin change interrupt (PCI)
triggers when any PCINT pin enabled in PCMSK toggles; pin changes are
detected asynchronously, so they wake the part from any sleep mode.
INT0 triggers on a falling or rising edge, any change, or a low level
(MCUCR ISC0[1:0], table int-isc); a level interrupt keeps triggering
while the pin is held low. Edge detection on INT0 needs clk~I/O~.

**Low-level interrupt.** Detected asynchronously, so it wakes the part
from any sleep mode (clk~I/O~ runs only in idle). For wake-up from
power-down the level must be held until the wake-up completes (the
start-up time set by SUT and CKSEL); if it goes away before the end of
the start-up time the MCU still wakes but no interrupt is generated,
and execution continues after the SLEEP instruction.

**Pin change timing.** A change on PCINT(0) passes the pin latch and a
synchronizer clocked by clk; PCIF is set three clock cycles after the
first rising clock edge that samples the change:

```wavedrom id=int-pcint caption="Timing of a pin change interrupt"
{"signal": [
  {"name": "clk", "wave": "hlhlhlhlh"},
  {"name": "PCINT(0)", "wave": "01......."},
  {"name": "pin_lat", "wave": "01......."},
  {"name": "pin_sync", "wave": "0.1......"},
  {"name": "pcint_in_(0)", "wave": "010......"},
  {"name": "pcint_syn", "wave": "0.1.0...."},
  {"name": "pcint_setflag", "wave": "0...1.0.."},
  {"name": "PCIF", "wave": "0.....1.."}
 ]}
```

MCUCR (bits ISC0[1:0]) is described under Power management and sleep
modes.

```table id=int-isc caption="INT0 sense control (MCUCR ISC0[1:0])"
| ISC01 | ISC00 | Interrupt request on |
|:-:|:-:|---|
| 0 | 0 | Low level of INT0 |
| 0 | 1 | Any logical change on INT0 |
| 1 | 0 | Falling edge of INT0 |
| 1 | 1 | Rising edge of INT0 |
```

INT0 is sampled before edges are detected. With edge or change
sensing, pulses longer than one clock period generate an interrupt;
shorter ones are not guaranteed to. With low-level sensing, the low
level must be held until the current instruction completes.

#### GIMSK – General interrupt mask register (0x3B)

```table id=int-gimsk caption="GIMSK bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | — | R | 0 | Reserved, read 0 |
| 6 | INT0 | R/W | 0 | External interrupt request 0 enable (with SREG I); sense set by MCUCR ISC0. Pin activity requests the interrupt even when INT0 is an output. Vector INT0. |
| 5 | PCIE | R/W | 0 | Pin change interrupt enable (with SREG I): any change on a PCINT[5:0] pin enabled in PCMSK interrupts. Vector PCINT0. |
| 4:0 | — | R | 0 | Reserved, read 0 |
```

#### GIFR – General interrupt flag register (0x3A)

```table id=int-gifr caption="GIFR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | — | R | 0 | Reserved, read 0 |
| 6 | INTF0 | R/W | 0 | External interrupt flag 0: set by an edge or change on INT0 that requests an interrupt; the MCU jumps to the vector if SREG I and GIMSK INT0 are set. Cleared when the routine runs or by writing 1. Always 0 when INT0 is a level interrupt. |
| 5 | PCIF | R/W | 0 | Pin change interrupt flag: set by a change on any enabled PCINT[5:0] pin; the MCU jumps to the vector if SREG I and GIMSK PCIE are set. Cleared when the routine runs or by writing 1. |
| 4:0 | — | R | 0 | Reserved, read 0 |
```

#### PCMSK – Pin change mask register (0x15)

```table id=int-pcmsk caption="PCMSK bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved, read 0 |
| 5:0 | PCINT[5:0] | R/W | 0 | 1 = pin change interrupt enabled on the corresponding pin (with GIMSK PCIE) |
```

### I/O ports

Port B (PB5:PB0) is a bidirectional port with true read-modify-write:
SBI and CBI change one pin's direction, output value or pull-up without
disturbing the others. Outputs have symmetrical drive with high sink
and source capability, strong enough to drive LED displays directly.
Every pin has a selectable pull-up of supply-invariant resistance and
protection diodes to V~CC~ and GND (see
[electrical characteristics](#electrical-characteristics)). Each port
has three registers: PORTx (data, R/W), DDRx (direction, R/W) and PINx
(input pins, read-only; writing 1 to a PINx bit toggles the PORTx
bit, whatever DDRx holds; SBI can toggle one pin). MCUCR PUD disables
all pull-ups. Enabling an alternate function on some pins leaves the
others usable as general I/O.

**Configuring a pin.** DDxn = 1 makes Pxn an output, driven to PORTxn.
DDxn = 0 makes it an input; PORTxn = 1 then turns on the pull-up. Pins
are tri-stated as soon as reset is active, even with no clock running.

```table id=io-config caption="Port pin configurations"
| DDxn | PORTxn | PUD (MCUCR) | I/O | Pull-up | Comment |
|:-:|:-:|:-:|---|---|---|
| 0 | 0 | X | Input | No | Tri-state (Hi-Z) |
| 0 | 1 | 0 | Input | Yes | Pxn sources current if pulled low externally |
| 0 | 1 | 1 | Input | No | Tri-state (Hi-Z) |
| 1 | 0 | X | Output | No | Output low (sink) |
| 1 | 1 | X | Output | No | Output high (source) |
```

**Switching between input and output.** Going from tri-state
({DDxn, PORTxn} = 00) to output high (11) passes through either pull-up
enabled (01) or output low (10). The pull-up state is normally
acceptable in a high-impedance environment; otherwise set PUD to
disable all pull-ups. Going from input with pull-up to output low has
the same problem: use tri-state (00) or output high (11) as the
intermediate step.

**Reading the pin.** PINxn reads the pin whatever DDxn holds, through a
synchronizer (a latch transparent while the system clock is high,
followed by the PINxn flip-flop) that avoids metastability but adds
delay: an external transition reaches PINxn between ½ and 1½ system
clock periods later (t~pd,min~, t~pd,max~). To read back a value just
written to PORTx, insert a NOP between the OUT and the IN: the delay
through the synchronizer is then one system clock period.

**Digital input and sleep.** In power-down the sleep controller clamps
each pin's Schmitt-trigger input to ground (SLEEP signal) so floating
or mid-supply inputs draw no current. Pins enabled as external
interrupts, and pins whose alternate functions require it, are not
clamped. If a high level is present on an asynchronous external
interrupt pin set for rising edge, falling edge or any change while
that interrupt is disabled, its flag is set on resuming from this
sleep mode, because the clamping makes the requested change.

**Unconnected pins.** Give unused pins a defined level to save current
in reset, active and idle mode. The simplest way is the internal
pull-up (off during reset); for low reset current use an external pull-up
or pull-down. Don't tie unused pins directly to V~CC~ or GND: a pin
accidentally made an output would then draw excessive current.

**Alternate functions.** Each alternate function overrides the pin's
control through the signals of table io-override, generated inside the
module. The digital input DI feeds the alternate function after the
Schmitt trigger but before the synchronizer (modules synchronize it
themselves unless it is used as a clock); AIO connects the analog
function straight to the pad.

```table id=io-override caption="Overriding signals for alternate functions"
| Signal | Full name | Description |
|---|---|---|
| PUOE | Pull-up override enable | 1: PUOV controls the pull-up. 0: the pull-up is on when {DDxn, PORTxn, PUD} = 010. |
| PUOV | Pull-up override value | With PUOE = 1: pull-up on/off when PUOV = 1/0, whatever DDxn, PORTxn and PUD hold |
| DDOE | Data direction override enable | 1: DDOV enables the output driver. 0: DDxn enables it. |
| DDOV | Data direction override value | With DDOE = 1: output driver on/off when DDOV = 1/0, whatever DDxn holds |
| PVOE | Port value override enable | 1 with the driver on: PVOV is the port value. 0: PORTxn is. |
| PVOV | Port value override value | With PVOE = 1: port value = PVOV, whatever PORTxn holds |
| PTOE | Port toggle override enable | 1: PORTxn is inverted |
| DIEOE | Digital input enable override enable | 1: DIEOV enables the digital input. 0: the MCU state (normal, sleep) does. |
| DIEOV | Digital input enable override value | With DIEOE = 1: digital input on/off when DIEOV = 1/0, whatever the MCU state |
| DI | Digital input | Input to the alternate function |
| AIO | Analog input/output | Analog connection to the pad, bidirectional |
```

```table id=io-altfn caption="Port B alternate functions"
| Pin | Alternate functions |
|---|---|
| PB5 | !{RESET} reset input; dW debugWIRE I/O; ADC0 ADC input channel 0; PCINT5 pin change interrupt source 5 |
| PB4 | XTAL2 crystal oscillator output; CLKO system clock output; ADC2 ADC input channel 2; OC1B Timer/Counter1 compare match B output; PCINT4 pin change interrupt source 4 |
| PB3 | XTAL1 crystal oscillator input; CLKI external clock input; ADC3 ADC input channel 3; !{OC1B} complementary Timer/Counter1 compare match B output; PCINT3 pin change interrupt source 3 |
| PB2 | SCK serial clock input; ADC1 ADC input channel 1; T0 Timer/Counter0 clock source; USCK USI clock (three-wire mode); SCL USI clock (two-wire mode); INT0 external interrupt 0 input; PCINT2 pin change interrupt source 2 |
| PB1 | MISO SPI master data input / slave data output; AIN1 analog comparator negative input; OC0B Timer/Counter0 compare match B output; OC1A Timer/Counter1 compare match A output; DO USI data output (three-wire mode); PCINT1 pin change interrupt source 1 |
| PB0 | MOSI SPI master data output / slave data input; AIN0 analog comparator positive input; OC0A Timer/Counter0 compare match A output; !{OC1A} complementary Timer/Counter1 compare match A output; DI USI data input (three-wire mode); SDA USI data (two-wire mode); AREF external analog reference; PCINT0 pin change interrupt source 0 |
```

Details per pin:

- **PB5.** !{RESET}: active-low external reset, enabled while the
  RSTDISBL fuse is unprogrammed; the pull-up is on and the output driver
  and digital input are off. dW: with the DWEN fuse programmed and the
  lock bits unprogrammed, debugWIRE is active and the pin becomes a
  wire-AND (open-drain) bidirectional I/O with pull-up, the link to the
  emulator.
- **PB4.** XTAL2: clock pin for all clock sources except the internal
  calibrated RC oscillator and the external clock; then not usable as
  I/O. CLKO: the divided system clock is output when the CKOUT fuse is
  programmed, whatever PORTB4 and DDB4 hold. OC1B: output for
  Timer/Counter1 compare match B and its PWM output when DDB4 = 1.
- **PB3.** XTAL1: clock pin for all clock sources except the internal
  calibrated RC oscillator; then not usable as I/O. CLKI: external
  clock input. !{OC1B}: inverted Timer/Counter1 compare match B and PWM
  output when DDB3 = 1.
- **PB2.** SCK: SPI clock (master output, slave input); as a slave the
  pin is an input whatever DDB2 holds, as a master DDB2 sets its
  direction; while forced to input, PORTB2 still controls the pull-up.
  T0: Timer/Counter0 counter source. USCK/SCL: USI three-wire / two-wire
  clock.
- **PB1.** MISO: SPI master input / slave output; as a master the pin
  is an input whatever DDB1 holds, as a slave DDB1 sets its direction;
  PORTB1 still controls the pull-up when forced to input. AIN1: make
  the pin an input with the pull-up off so the digital port doesn't
  disturb the comparator. OC0B, OC1A: Timer/Counter0 compare match B
  and Timer/Counter1 compare match A outputs (and PWM outputs) when
  DDB1 = 1. DO: USI three-wire data output; overrides PORTB1 and is
  driven when DDB1 = 1; PORTB1 still enables the pull-up when the pin
  is an input.
- **PB0.** MOSI: SPI master output / slave input; as a slave the pin is
  an input whatever DDB0 holds, as a master DDB0 sets its direction;
  PORTB0 still controls the pull-up when forced to input. AIN0: make it
  an input with the pull-up off. OC0A: Timer/Counter0 compare match A
  and PWM output when DDB0 = 1. !{OC1A}: inverted Timer/Counter1
  compare match A and PWM output when DDB0 = 1. SDA: USI two-wire data.
  AREF: external ADC reference; pull-up and output driver are off when
  the pin is used as external reference or for the internal reference's
  external capacitor. DI: USI three-wire data input; the USI doesn't
  override the port, so make the pin an input.

```table id=io-ovr-hi caption="Overriding signals for alternate functions in PB5:PB3 [1]"
| Signal | PB5 / !{RESET} / ADC0 / PCINT5 | PB4 / ADC2 / XTAL2 / OC1B / PCINT4 | PB3 / ADC3 / XTAL1 / !{OC1B} / PCINT3 |
|---|---|---|---|
| PUOE | !{RSTDISBL} • DWEN | 0 | 0 |
| PUOV | 1 | 0 | 0 |
| DDOE | RSTDISBL • DWEN | 0 | 0 |
| DDOV | debugWIRE transmit | 0 | 0 |
| PVOE | 0 | OC1B enable | !{OC1B} enable |
| PVOV | 0 | OC1B | OC1B |
| PTOE | 0 | 0 | 0 |
| DIEOE | !{RSTDISBL} + (PCINT5 • PCIE + ADC0D) | PCINT4 • PCIE + ADC2D | PCINT3 • PCIE + ADC3D |
| DIEOV | ADC0D | ADC2D | ADC3D |
| DI | PCINT5 input | PCINT4 input | PCINT3 input |
| AIO | RESET input, ADC0 input | ADC2 input | ADC3 input |

[1] RSTDISBL and DWEN are 1 when the fuse is 0 (programmed).
```

```table id=io-ovr-lo caption="Overriding signals for alternate functions in PB2:PB0"
| Signal | PB2 / SCK / ADC1 / T0 / USCK / SCL / INT0 / PCINT2 | PB1 / MISO / DO / AIN1 / OC1A / OC0B / PCINT1 | PB0 / MOSI / DI / SDA / AIN0 / AREF / !{OC1A} / OC0A / PCINT0 |
|---|---|---|---|
| PUOE | USI_TWO_WIRE | 0 | USI_TWO_WIRE |
| PUOV | 0 | 0 | 0 |
| DDOE | USI_TWO_WIRE | 0 | USI_TWO_WIRE |
| DDOV | (USI_SCL_HOLD + !{PORTB2}) • DDB2 | 0 | (!{SDA} + !{PORTB0}) • DDB0 |
| PVOE | USI_TWO_WIRE • DDB2 | OC0B enable + OC1A enable + USI_THREE_WIRE | OC0A enable + !{OC1A} enable + (USI_TWO_WIRE • DDB0) |
| PVOV | 0 | OC0B + OC1A + DO | OC0A + !{OC1A} |
| PTOE | USITC | 0 | 0 |
| DIEOE | PCINT2 • PCIE + ADC1D + USISIE | PCINT1 • PCIE + AIN1D | PCINT0 • PCIE + AIN0D + USISIE |
| DIEOV | ADC1D | AIN1D | AIN0D |
| DI | T0 / USCK / SCL / INT0 / PCINT2 input | PCINT1 input | DI / SDA / PCINT0 input |
| AIO | ADC1 input | Analog comparator negative input | Analog comparator positive input |
```

MCUCR (bit PUD) is described under Power management and sleep modes.

#### PORTB – Port B data register (0x18)

```table id=io-portb caption="PORTB bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved, read 0 |
| 5:0 | PORTB[5:0] | R/W | 0 | Output: drive level. Input: 1 = pull-up on. |
```

#### DDRB – Port B data direction register (0x17)

```table id=io-ddrb caption="DDRB bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved, read 0 |
| 5:0 | DDB[5:0] | R/W | 0 | 1 = output, 0 = input |
```

#### PINB – Port B input pins (0x16)

```table id=io-pinb caption="PINB bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved, read 0 |
| 5:0 | PINB[5:0] | R/W | N/A | Read: synchronized pin levels. Write 1: toggles the PORTB bit. |
```

### Timer/Counter0

An 8-bit timer/counter (TCNT0) with two output compare units (OCR0A,
OCR0B; outputs OC0A, OC0B), double-buffered compare registers in the PWM
modes, clear timer on compare match (auto reload), glitch-free phase
correct PWM, variable PWM period, a frequency generator and three
interrupt sources (TOV0, OCF0A, OCF0B; flags in TIFR, enables in TIMSK).
Block diagram: TCNT0 with control logic and clock select feeding two
8-bit comparators against OCR0A/OCR0B, each driving a waveform generator.

```table id=t0-defs caption="Counter definitions"
| Term | Meaning |
|---|---|
| BOTTOM | The counter reaches BOTTOM when it becomes 0x00 |
| MAX | The counter reaches MAX when it becomes 0xFF (255) |
| TOP | The highest value in the count sequence: the fixed value 0xFF (MAX) or the value in OCR0A, depending on the mode |
```

#### Clock sources and prescaler

CS0[2:0] in TCCR0B select the timer clock clk~T0~: none (timer stopped;
TCNT0 still readable and writable), clk~I/O~ directly, clk~I/O~/8, /64,
/256 or /1024 from the prescaler, or the T0 pin. The prescaler runs freely
and is shared with no other clock select, so with a prescaled clock
(6 > CS0[2:0] > 1) the first count after enabling comes 1 to N + 1 system
clocks later (N = 8, 64, 256 or 1024). PSR0 in GTCCR resets the
prescaler, to synchronise the timer to program execution.

**External clock (T0).** T0 is sampled every system clock by a
synchronizer (registers clocked on the rising clk~I/O~ edge, latch
transparent while clk~I/O~ is high) and an edge detector that makes one
clk~T0~ pulse per rising (CS0 = 7) or falling (CS0 = 6) edge. Delay from a
T0 edge to the counter update: 2.5 to 3.5 system clocks. Enable or disable
the clock input only while T0 has been stable for at least one system
clock, or a false count may occur. Each half period of the external clock
must exceed one system clock (f~ExtClk~ < f~clk_I/O~/2 at 50/50 % duty);
because of oscillator tolerances, keep f~ExtClk~ below f~clk_I/O~/2.5. An
external clock can't be prescaled. T0 clocks the counter even when the pin
is an output, so software can count.

#### Counter and compare units

Each timer clock the counter is cleared, incremented or decremented
according to WGM0[2:0]; a CPU write to TCNT0 overrides (has priority over)
clear and count. TOV0 is set according to the mode.

The comparator matches TCNT0 against OCR0A and OCR0B continuously; a
match sets OCF0x at the next timer clock, which requests an interrupt if
enabled (cleared when the interrupt executes, or by writing 1). OCR0x is
double buffered in the PWM modes (updates at TOP or BOTTOM, so PWM pulses
are never odd-length or asymmetric) and written directly in Normal and
CTC modes; with buffering on, the CPU accesses the buffer.

- **Force output compare.** In non-PWM modes writing 1 to FOC0x updates
  OC0x as a real match would (per COM0x), without setting OCF0x or
  clearing the timer.
- **Match blocking.** Any CPU write to TCNT0 blocks a compare match in the
  next timer clock, even when the timer is stopped, so OCR0x can be
  initialised to TCNT0's value without an interrupt. Writing TCNT0 equal
  to OCR0x therefore loses the match; don't write BOTTOM while counting
  down.
- Set up OC0x before setting the pin's DDR bit to output; the easiest way
  is FOC0x in Normal mode. The OC0x registers keep their value across
  waveform mode changes. COM0x[1:0] is not double buffered and takes
  effect at once (at the first compare match after the write; for non-PWM
  modes FOC0x makes it immediate).

**Output unit.** If either COM0x bit is set, OC0x from the waveform
generator overrides the port output, but the pin direction still comes
from DDRB: the DDR bit must be 1 for OC0x to be seen on the pin. The
override doesn't depend on the waveform mode. COM0x = 0 means no action on
the OC0x register at the next match. A system reset clears the internal
OC0x register.

#### Modes of operation

- **Normal (WGM0 = 0).** Counts up, overruns from 0xFF (TOP) to 0x00;
  TOV0 is set in the same timer clock that TCNT0 becomes zero (a ninth bit
  that is only set, not cleared). A new counter value can be written at
  any time. Using output compare to make waveforms in Normal mode is not
  recommended (CPU time).
- **CTC (WGM0 = 2).** The counter clears when TCNT0 matches OCR0A, so
  OCR0A sets TOP and the resolution. OCF0A can interrupt at TOP to update
  TOP; there is no double buffering, so if the new OCR0A is below the
  current TCNT0 the match is missed and the counter wraps through 0xFF
  first. With COM0A = 1 (toggle), OC0A toggles each match, at most
  f~clk_I/O~/2 (OCR0A = 0). TOV0 is set when the counter goes from MAX to
  0x00.
- **Fast PWM (WGM0 = 3, TOP = 0xFF; WGM0 = 7, TOP = OCR0A).** Single
  slope, BOTTOM to TOP then cleared at the next timer clock; twice the
  frequency of phase correct PWM. Non-inverting (COM0x = 2): OC0x cleared
  on match, set at BOTTOM; inverting (COM0x = 3): set on match, cleared at
  BOTTOM. TOV0 is set at TOP. COM0A = 1 with WGM02 = 1 toggles OC0A on
  each match (50 % duty, at most f~clk_I/O~/2 with OCR0A = 0, double
  buffered unlike CTC); not available for OC0B. OCR0A = BOTTOM gives a
  narrow spike every MAX + 1 timer clocks; OCR0A = MAX a constant high or
  low output.
- **Phase correct PWM (WGM0 = 1, TOP = 0xFF; WGM0 = 5, TOP = OCR0A).**
  Dual slope, BOTTOM to TOP to BOTTOM; TCNT0 equals TOP for one timer
  clock. Non-inverting: OC0x cleared on match while up-counting, set on
  match while down-counting; inverting is the opposite. TOV0 is set at
  BOTTOM. COM0A = 1 with WGM02 = 1 toggles OC0A on match; not for OC0B.
  OCR0A = BOTTOM gives a constant low output, MAX constant high
  (non-inverted; inverted is the opposite). OC0x may change at BOTTOM
  without a match, to keep the output symmetric around BOTTOM: when OCR0A
  changes away from MAX, and when the timer starts above OCR0A and so
  misses the up-counting match.

```equation
f~OC0x~ = f~clk_I/O~ / (2 × N × (1 + OCR0x))   (CTC, toggle)
f~OC0xPWM~ = f~clk_I/O~ / (N × 256)   (fast PWM)
f~OC0xPCPWM~ = f~clk_I/O~ / (N × 510)   (phase correct PWM)
```

N is the prescale factor (1, 8, 64, 256 or 1024).

#### Timing

clk~T0~ acts as a clock enable. TOV0 is set as TCNT0 goes from MAX to
BOTTOM (all modes except phase correct PWM); OCF0B in all modes and OCF0A
except where OCR0A is TOP are set as TCNT0 leaves the OCR0x value; in CTC
and in fast PWM with OCR0A as TOP, OCF0A is set and TCNT0 cleared
together, as TCNT0 goes from TOP to BOTTOM.

```wavedrom id=t0-tov-noprescale caption="Timer/Counter0, no prescaling"
{"signal": [
  {"name": "clkIO", "wave": "p..."},
  {"name": "clkT0 (clkIO/1)", "wave": "1..."},
  {"name": "TCNT0", "wave": "====", "data": ["MAX-1", "MAX", "BOTTOM", "BOTTOM+1"]},
  {"name": "TOV0", "wave": "0.1."}
 ]}
```

```wavedrom id=t0-tov-prescale caption="Timer/Counter0, prescaler f~clk_I/O~/8"
{"signal": [
  {"name": "clkIO", "wave": "p........................."},
  {"name": "clkT0 (clkIO/8)", "wave": "0.....10......10......10.."},
  {"name": "TCNT0", "wave": "=......=.......=.......=..", "data": ["MAX-1", "MAX", "BOTTOM", "BOTTOM+1"]},
  {"name": "TOV0", "wave": "0..............1.........."}
 ]}
```

```wavedrom id=t0-ocf caption="Setting of OCF0x, prescaler f~clk_I/O~/8"
{"signal": [
  {"name": "clkIO", "wave": "p........................."},
  {"name": "clkT0 (clkIO/8)", "wave": "0.....10......10......10.."},
  {"name": "TCNT0", "wave": "=......=.......=.......=..", "data": ["OCR0x-1", "OCR0x", "OCR0x+1", "OCR0x+2"]},
  {"name": "OCR0x", "wave": "=.........................", "data": ["OCR0x value"]},
  {"name": "OCF0x", "wave": "0..............1.........."}
 ]}
```

```wavedrom id=t0-ctc caption="CTC: setting of OCF0A and clearing of TCNT0, prescaler f~clk_I/O~/8"
{"signal": [
  {"name": "clkIO", "wave": "p........................."},
  {"name": "clkT0 (clkIO/8)", "wave": "0.....10......10......10.."},
  {"name": "TCNT0", "wave": "=......=.......=.......=..", "data": ["TOP-1", "TOP", "BOTTOM", "BOTTOM+1"]},
  {"name": "OCR0A", "wave": "=.........................", "data": ["TOP"]},
  {"name": "OCF0A", "wave": "0..............1.........."}
 ]}
```

#### GTCCR – General timer/counter control register (0x2C)

```table id=t0-gtccr caption="GTCCR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | TSM | R/W | 0 | Timer/counter synchronization mode: 1 keeps the value written to PSR0 (prescaler reset held), so Timer/Counter0 is halted while it is configured. Writing 0 clears PSR0 in hardware and the timer starts. |
| 6 | PWM1B | R/W | 0 | Timer/Counter1 PWM mode on OCR1B; the counter resets to 0x00 in the CPU clock after a match with OCR1C |
| 5:4 | COM1B[1:0] | R/W | 0 | Timer/Counter1 comparator B output mode; see Timer/Counter1 |
| 3 | FOC1B | W | 0 | Force output compare 1B: writing 1 changes OC1B (PB4) per COM1B as on a compare match, without an interrupt. COM1B written in the same cycle is used. Reads 0. Not in use when PWM1B = 1. |
| 2 | FOC1A | W | 0 | Force output compare 1A: as FOC1B, for OC1A (PB1), COM1A, PWM1A |
| 1 | PSR1 | R/W | 0 | Prescaler reset Timer/Counter1: 1 resets the prescaler (TCNT1 unaffected); cleared by hardware when done. Writing 0 has no effect; reads 0. |
| 0 | PSR0 | R/W | 0 | Prescaler reset Timer/Counter0: 1 resets the prescaler; cleared by hardware at once unless TSM = 1 |
```

#### TCCR0A – Timer/Counter0 control register A (0x2A)

```table id=t0-tccr0a caption="TCCR0A bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | COM0A[1:0] | R/W | 0 | Compare match output A mode (OC0A); see tables t0-com-nonpwm, t0-com-fast, t0-com-pc. Any bit set overrides the port function of the pin; its DDR bit must be 1. |
| 5:4 | COM0B[1:0] | R/W | 0 | Compare match output B mode (OC0B), as COM0A |
| 3:2 | — | R | 0 | Reserved |
| 1:0 | WGM0[1:0] | R/W | 0 | Waveform generation mode, with WGM02 in TCCR0B; see table t0-wgm |
```

```table id=t0-com-nonpwm caption="COM0x[1:0], Normal and CTC modes"
| COM0x[1:0] | Description |
|---|---|
| 00 | Normal port operation, OC0x disconnected |
| 01 | Toggle OC0x on compare match |
| 10 | Clear OC0x on compare match |
| 11 | Set OC0x on compare match |
```

```table id=t0-com-fast caption="COM0x[1:0], fast PWM mode [1]"
| COM0x[1:0] | Description |
|---|---|
| 00 | Normal port operation, OC0x disconnected |
| 01 | Reserved (OC0A: toggle on compare match when WGM02 = 1) |
| 10 | Clear OC0x on compare match, set at BOTTOM (non-inverting) |
| 11 | Set OC0x on compare match, clear at BOTTOM (inverting) |

[1] When OCR0x equals TOP and COM0x1 is set, the compare match is ignored, but the set or clear is done at BOTTOM.
```

```table id=t0-com-pc caption="COM0x[1:0], phase correct PWM mode [1]"
| COM0x[1:0] | Description |
|---|---|
| 00 | Normal port operation, OC0x disconnected |
| 01 | Reserved (OC0A: toggle on compare match when WGM02 = 1) |
| 10 | Clear OC0x on compare match when up-counting, set when down-counting |
| 11 | Set OC0x on compare match when up-counting, clear when down-counting |

[1] When OCR0x equals TOP and COM0x1 is set, the compare match is ignored, but the set or clear is done at TOP.
```

```table id=t0-wgm caption="Waveform generation modes"
| Mode | WGM02 | WGM01 | WGM00 | Operation | TOP | OCR0x update | TOV0 set on |
|--:|:-:|:-:|:-:|---|---|---|---|
| 0 | 0 | 0 | 0 | Normal | 0xFF | Immediate | MAX |
| 1 | 0 | 0 | 1 | PWM, phase correct | 0xFF | TOP | BOTTOM |
| 2 | 0 | 1 | 0 | CTC | OCR0A | Immediate | MAX |
| 3 | 0 | 1 | 1 | Fast PWM | 0xFF | BOTTOM | MAX |
| 4 | 1 | 0 | 0 | Reserved | — | — | — |
| 5 | 1 | 0 | 1 | PWM, phase correct | OCR0A | TOP | BOTTOM |
| 6 | 1 | 1 | 0 | Reserved | — | — | — |
| 7 | 1 | 1 | 1 | Fast PWM | OCR0A | BOTTOM | TOP |
```

#### TCCR0B – Timer/Counter0 control register B (0x33)

```table id=t0-tccr0b caption="TCCR0B bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | FOC0A | W | 0 | Force output compare A: strobe, active only in non-PWM modes (write 0 in PWM modes). Forces a compare match on the waveform generator; OC0A changes per COM0A. No interrupt, and no timer clear in CTC. Reads 0. |
| 6 | FOC0B | W | 0 | Force output compare B, as FOC0A for OC0B |
| 5:4 | — | R | 0 | Reserved |
| 3 | WGM02 | R/W | 0 | Waveform generation mode bit 2; see table t0-wgm |
| 2:0 | CS0[2:0] | R/W | 0 | Clock select: 000 = no clock (stopped); 001 = clk~I/O~; 010 = clk~I/O~/8; 011 = clk~I/O~/64; 100 = clk~I/O~/256; 101 = clk~I/O~/1024; 110 = T0 pin, falling edge; 111 = T0 pin, rising edge |
```

#### TCNT0 (0x32), OCR0A (0x29), OCR0B (0x28)

All R/W, reset 0x00. TCNT0 reads and writes the 8-bit counter; a write
blocks the compare match at the next timer clock, and changing TCNT0
while running risks missing a match. OCR0A and OCR0B are compared with
TCNT0 continuously; a match can interrupt or drive OC0A/OC0B.

#### TIMSK – Timer/counter interrupt mask register (0x39)

An interrupt is enabled when its bit and the I bit in SREG are 1; it runs
when the matching TIFR flag is set.

```table id=t0-timsk caption="TIMSK bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | — | R | 0 | Reserved |
| 6 | OCIE1A | R/W | 0 | Timer/Counter1 compare match A interrupt enable |
| 5 | OCIE1B | R/W | 0 | Timer/Counter1 compare match B interrupt enable |
| 4 | OCIE0A | R/W | 0 | Timer/Counter0 compare match A interrupt enable |
| 3 | OCIE0B | R/W | 0 | Timer/Counter0 compare match B interrupt enable |
| 2 | TOIE1 | R/W | 0 | Timer/Counter1 overflow interrupt enable |
| 1 | TOIE0 | R/W | 0 | Timer/Counter0 overflow interrupt enable |
| 0 | — | R | 0 | Reserved |
```

#### TIFR – Timer/counter interrupt flag register (0x38)

Each flag is cleared by hardware when its interrupt vector executes, or by
writing 1 to it (Timer/Counter1 flags: after a synchronization clock
cycle).

```table id=t0-tifr caption="TIFR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | — | R | 0 | Reserved |
| 6 | OCF1A | R/W | 0 | Set on a compare match between TCNT1 and OCR1A |
| 5 | OCF1B | R/W | 0 | Set on a compare match between TCNT1 and OCR1B |
| 4 | OCF0A | R/W | 0 | Set on a compare match between TCNT0 and OCR0A |
| 3 | OCF0B | R/W | 0 | Set on a compare match between TCNT0 and OCR0B |
| 2 | TOV1 | R/W | 0 | Timer/Counter1 overflow. Normal mode (PWM1A = PWM1B = 0): set on overflow. PWM mode (PWM1A or PWM1B = 1): set on a compare match between TCNT1 and OCR1C. |
| 1 | TOV0 | R/W | 0 | Timer/Counter0 overflow; when it is set depends on WGM0 (table t0-wgm) |
| 0 | — | R | 0 | Reserved |
```

### Timer/Counter1

An 8-bit up-counter with its own 14-bit prescaler, three compare
registers (OCR1A, OCR1B, OCR1C) and two PWM channels A and B, each with a
true and an inverted output (OC1A on PB1, !{OC1A} on PB0, OC1B on PB4,
!{OC1B} on PB3) through a dead time generator. Flags are in TIFR,
enables in TIMSK, controls in TCCR1 and GTCCR (all described under
Timer/Counter0).

**Prescaler.** Two clocking modes. Synchronous (PCKE = 0): the
timebase is the system clock CK. Asynchronous (PCKE = 1 in PLLCSR): the
timebase is the fast peripheral clock PCK, 64 MHz from the PLL (32 MHz in
low speed mode, LSM = 1). CS1[3:0] select T1CK, T1CK/2 … T1CK/16384, or
stop. PSR1 in GTCCR resets the prescaler. In asynchronous mode the CPU
may run from any clock source, but the system clock must be lower than a
third of PCK: the synchronizer needs at least two PCK edges while the
system clock is high, or data or control values may be lost.

**Asynchronous start-up.** Enable the PLL (PLLE), wait 100 µs for it to
stabilize, poll PLOCK until it is 1, then set PCKE.

**Synchronization.** Register writes pass through input synchronization
registers before they reach the counter; TCNT1 and the flags pass through
output synchronization. TCCR1, GTCCR, OCR1A, OCR1B and OCR1C read back at
once after a write; TCNT1, OCF1A, OCF1B and TOV1 read back delayed.

```table id=t1-sync caption="Timer/Counter1 synchronization delays"
| Mode | Input synchronization | Timer/Counter1 | Output synchronization (stage 1) | Output synchronization (stage 2) |
|---|---|---|---|---|
| Synchronous | 1/2 CK | 1 CK | 1 CK | 1/2 CK |
| Asynchronous | 1–2 PCK | 1 PCK | about 1 CK | none |
```

**Compare units.** In Normal mode all three compare registers work:
OCR1A acts on OC1A (PB1) and can raise the OC1A interrupt, OCR1B on OC1B
(PB4) with the OC1B interrupt, and OCR1C is the clear-on-match value
(with CTC1 = 1). TOV1 is set when TCNT1 goes from 0xFF to 0x00. The
inverted outputs !{OC1A} and !{OC1B} are not connected in Normal mode. A
compare match happens only when the counter counts to the compare value;
a software write that makes TCNT1 equal to OCR1x makes no match. A match
sets its flag after a synchronization delay.

**PWM mode** (PWM1A in TCCR1, PWM1B in GTCCR). TCNT1 and OCR1C form a
free-running, glitch-free dual 8-bit PWM: the counter counts from 0x00 to
OCR1C and restarts at 0x00 (cleared in the CPU clock after the OCR1C
match), and OCR1A/OCR1B set the duty cycles. The match with OCR1C sets
TOV1 after a synchronization delay (overflow interrupt from 0xFF to 0x00
or from OCR1C to 0x00). The complementary outputs have zero non-overlap
time by default; the dead time generator can insert it. OCR1A/OCR1B
writes go to a temporary location and latch when TCNT1 reaches OCR1C,
which prevents odd-length pulses (glitches) from an unsynchronized write;
until then a read returns the most recently written value.

```table id=t1-com-pwm caption="COM1x[1:0] in PWM mode (x = A or B)"
| COM1x[1:0] | OC1x | !{OC1x} |
|---|---|---|
| 00 | Not connected | Not connected |
| 01 | Cleared on compare match, set when TCNT1 = 0x00 | Set on compare match, cleared when TCNT1 = 0x00 |
| 10 | Cleared on compare match, set when TCNT1 = 0x00 | Not connected |
| 11 | Set on compare match, cleared when TCNT1 = 0x00 | Not connected |
```

```table id=t1-pwm-ext caption="PWM outputs when OCR1x = 0x00 or OCR1C"
| COM1x[1:0] | OCR1x | OC1x | !{OC1x} |
|---|---|---|---|
| 01 | 0x00 | L | H |
| 01 | OCR1C | H | L |
| 10 | 0x00 | L | Not connected |
| 10 | OCR1C | H | Not connected |
| 11 | 0x00 | H | Not connected |
| 11 | OCR1C | L | Not connected |
```

```equation
f~PWM~ = f~TCK1~ / (OCR1C + 1)
R = log~2~(OCR1C + 1)   (resolution, bits)
```

```table id=t1-pwm-freq caption="Clock selection and OCR1C for PWM frequencies, asynchronous mode (PCK = 64 MHz)"
| PWM frequency (kHz) | Clock | CS1[3:0] | OCR1C | Resolution (bits) |
|--:|---|---|--:|--:|
| 20 | PCK/16 | 0101 | 199 | 7.6 |
| 30 | PCK/16 | 0101 | 132 | 7.1 |
| 40 | PCK/8 | 0100 | 199 | 7.6 |
| 50 | PCK/8 | 0100 | 159 | 7.3 |
| 60 | PCK/8 | 0100 | 132 | 7.1 |
| 70 | PCK/4 | 0011 | 228 | 7.8 |
| 80 | PCK/4 | 0011 | 199 | 7.6 |
| 90 | PCK/4 | 0011 | 177 | 7.5 |
| 100 | PCK/4 | 0011 | 159 | 7.3 |
| 110 | PCK/4 | 0011 | 144 | 7.2 |
| 120 | PCK/4 | 0011 | 132 | 7.1 |
| 130 | PCK/2 | 0010 | 245 | 7.9 |
| 140 | PCK/2 | 0010 | 228 | 7.8 |
| 150 | PCK/2 | 0010 | 212 | 7.7 |
| 160 | PCK/2 | 0010 | 199 | 7.6 |
| 170 | PCK/2 | 0010 | 187 | 7.6 |
| 180 | PCK/2 | 0010 | 177 | 7.5 |
| 190 | PCK/2 | 0010 | 167 | 7.4 |
| 200 | PCK/2 | 0010 | 159 | 7.3 |
| 250 | PCK | 0001 | 255 | 8.0 |
| 300 | PCK | 0001 | 212 | 7.7 |
| 350 | PCK | 0001 | 182 | 7.5 |
| 400 | PCK | 0001 | 159 | 7.3 |
| 450 | PCK | 0001 | 141 | 7.1 |
| 500 | PCK | 0001 | 127 | 7.0 |
```

Higher PWM frequencies are possible at lower resolution.

#### TCCR1 – Timer/Counter1 control register (0x30)

```table id=t1-tccr1 caption="TCCR1 bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | CTC1 | R/W | 0 | Clear timer on compare match: 1 resets TCNT1 to 0x00 in the CPU clock after a match with OCR1C; 0 lets it count on |
| 6 | PWM1A | R/W | 0 | PWM mode on OCR1A; the counter resets to 0x00 in the CPU clock after a match with OCR1C |
| 5:4 | COM1A[1:0] | R/W | 0 | Comparator A output mode. Normal mode (OC1A on PB1; !{OC1A} not connected): 00 = disconnected; 01 = toggle OC1A; 10 = clear OC1A; 11 = set OC1A. PWM mode: table t1-com-pwm. The pin's DDR bit must be 1. |
| 3:0 | CS1[3:0] | R/W | 0 | Clock select; see table t1-cs |
```

COM1B[1:0] (GTCCR) work the same way for OC1B on PB4.

```table id=t1-cs caption="Timer/Counter1 prescale select"
| CS1[3:0] | Asynchronous mode | Synchronous mode |
|---|---|---|
| 0000 | Stopped | Stopped |
| 0001 | PCK | CK |
| 0010 | PCK/2 | CK/2 |
| 0011 | PCK/4 | CK/4 |
| 0100 | PCK/8 | CK/8 |
| 0101 | PCK/16 | CK/16 |
| 0110 | PCK/32 | CK/32 |
| 0111 | PCK/64 | CK/64 |
| 1000 | PCK/128 | CK/128 |
| 1001 | PCK/256 | CK/256 |
| 1010 | PCK/512 | CK/512 |
| 1011 | PCK/1024 | CK/1024 |
| 1100 | PCK/2048 | CK/2048 |
| 1101 | PCK/4096 | CK/4096 |
| 1110 | PCK/8192 | CK/8192 |
| 1111 | PCK/16384 | CK/16384 |
```

#### TCNT1 (0x2F), OCR1A (0x2E), OCR1B (0x2B), OCR1C (0x2D)

All R/W; reset 0x00, except OCR1C 0xFF. TCNT1 is the counter; a value
written reaches it after 1.5 CPU clocks (synchronous mode) or at most one
CPU clock (asynchronous mode). OCR1A and OCR1B are compared with TCNT1
continuously; actions on a match are set by TCCR1 and GTCCR. OCR1C is
compared too, clears TCNT1 on a match when CTC1 = 1, and has the same
function in Normal and PWM mode.

#### PLLCSR – PLL control and status register (0x27)

```table id=t1-pllcsr caption="PLLCSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | LSM | R/W | 0 | Low speed mode: 1 scales PCK down from 64 MHz to 32 MHz. Must be set when V~CC~ is below 2.7 V (Timer/Counter1 is not fast enough at low voltage). Stop Timer/Counter1 while changing LSM. Can't be set while the PLL clock is the system clock. |
| 6:3 | — | R | 0 | Reserved |
| 2 | PCKE | R/W | 0 | PCK enable: 1 = asynchronous mode, Timer/Counter1 clocked by PCK; 0 = synchronous mode, clocked by CK. Can be set only when PLLE = 1, and safely only when PLOCK = 1. |
| 1 | PLLE | R/W | 0/1 | PLL enable: 1 starts the PLL (and the internal RC oscillator as its reference if needed). Always 1 when the PLL is the system clock source. |
| 0 | PLOCK | R | 0 | PLL lock detector: 1 = PLL locked to the reference. Ignore it during the initial lock-in (overshoot and undershoot); steady state is reached within 100 µs. Check it before enabling PCK. |
```

### Timer/Counter1 in ATtiny15 mode

Selected by CKSEL = 0011 (any other code gives the normal mode above). It
makes Timer/Counter1, its prescaler and the clock frequencies compatible
with the ATtiny15: CK = 1.6 MHz, PCK = 25.6 MHz. Timer/Counter1 is
always synchronous here, because the same RC oscillator is the PLL source
and the core clock; PCKE and PLLE are always 1. Differences from the
normal mode:

- Prescaler: a 10-bit prescaler on CK and a 3-bit prescaler on PCK, with
  the selections in table t1-t15-cs.
- Only OCR1A and OCR1C are used: one PWM output, OC1A on PB1, no OC1B and
  no inverted outputs. OCR1C replaces the ATtiny15's OCR1B, with the same
  function. In Normal mode output compare works with OCR1A only. TIMSK
  and TIFR use only OCIE1A/TOIE1 and OCF1A/TOV1; GTCCR only FOC1A and PSR1.
- TCCR1, GTCCR, OCR1A and OCR1C read back at once; TCNT1, OCF1A and TOV1
  delayed. Synchronization delays: input 1–2 PCK, timer 1 PCK, output
  about 1 CK, then none. A value written to TCNT1 reaches the counter after one
  CPU clock (at most two in asynchronous mode).
- PWM mode (PWM1A = 1): TCNT1 counts from 0x00 to OCR1C and the OCR1A
  match drives OC1A, up to 25.6 MHz timer clock. OCR1A writes latch at
  OCR1C as in normal mode. COM1A[1:0] in PWM mode: 00 or 01 = OC1A not
  connected; 10 = OC1A cleared on compare match, set when TCNT1 = 0x00;
  11 = OC1A set on compare match, cleared when TCNT1 = 0x00. With OCR1A
  = 0x00 or OCR1C the output is held as in table t1-pwm-ext (OC1A
  column). Normal-mode COM1A: 00 = disconnected, 01 = toggle, 10 =
  clear, 11 = set OC1A.
- CTC1 = 1 resets TCNT1 in the CPU clock after a compare match (see
  Conversion notes on which register).

```equation
f = f~TCK1~ / (OCR1C + 1)
D = ((OCR1A + 1) × T~TCK1~ - T~PCK~) / ((OCR1C + 1) × T~TCK1~)
```

T~PCK~ is the fast peripheral clock period, 1/25.6 MHz = 39.1 ns.

```table id=t1-t15-cs caption="Timer/Counter1 prescale select, ATtiny15 mode"
| CS1[3:0] | T/C1 clock | CS1[3:0] | T/C1 clock |
|---|---|---|---|
| 0000 | Stopped | 1000 | CK/8 |
| 0001 | PCK | 1001 | CK/16 |
| 0010 | PCK/2 | 1010 | CK/32 |
| 0011 | PCK/4 | 1011 | CK/64 |
| 0100 | PCK/8 | 1100 | CK/128 |
| 0101 | CK | 1101 | CK/256 |
| 0110 | CK/2 | 1110 | CK/512 |
| 0111 | CK/4 | 1111 | CK/1024 |
```

### Dead time generator

Two dead time generators, one per Timer/Counter1 PWM channel, insert
non-overlap times into the complementary output pairs (OC1A/!{OC1A},
OC1B/!{OC1B}) so they can drive external power switches safely. Each has
a prescaler dividing the Timer/Counter1 clock (PCK or CK) by 1, 2, 4 or 8
(DTPS1), an edge detector and a 4-bit down counter. On each edge of the
PWM generator output the counter is loaded with DT1xH (rising edge of
PWM1x: delays the rise of OC1x) or DT1xL (falling edge: delays the rise
of !{OC1x}), and the rising output transition is held off until the
counter reaches zero. So the two outputs' dead times are set separately,
in prescaled clock cycles, 0 to 15.

```wavedrom id=dt-pair caption="Complementary output pair with dead time (x = A or B)"
{"signal": [
  {"name": "PWM1x", "wave": "0.1...0...1.", "node": "..A...C"},
  {"name": "OC1x", "wave": "0..1..0....1", "node": "...B"},
  {"name": "OC1x#", "wave": "1.0....1..0.", "node": ".......D"}
 ],
 "edge": ["A<->B tnon-overlap (rising edge)", "C<->D tnon-overlap (falling edge)"],
 "config": {"hscale": 2}}
```

#### DTPS1 – Timer/Counter1 dead time prescaler register (0x23)

```table id=dt-dtps1 caption="DTPS1 bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:2 | — | R | 0 | Reserved |
| 1:0 | DTPS1[1:0] | R/W | 0 | Dead time prescaler, divides the T/C1 clock: 00 = 1 (no division); 01 = 2; 10 = 4; 11 = 8 |
```

#### DT1A (0x25), DT1B (0x24) – Timer/Counter1 dead time A and B

```table id=dt-dt1x caption="DT1A and DT1B bits (x = A or B)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:4 | DT1xH[3:0] | R/W | 0 | Dead time for the OC1x output (delay of its rising edge), in prescaled T/C1 clocks, 0 to 15 |
| 3:0 | DT1xL[3:0] | R/W | 0 | Dead time for the !{OC1x} output (delay of its rising edge), in prescaled T/C1 clocks, 0 to 15 |
```

### USI

The Universal Serial Interface gives the hardware for synchronous
serial transfers; software does the rest. It has an 8-bit data register
USIDR, a buffer register USIBR, a 4-bit counter and a two-wire clock
control unit. Block diagram: USIDR shifts left from DI/SDA; its bit 7
drives DO (three-wire) or SDA (two-wire) through an output latch; the
counter and USIDR are clocked by the same source (USCK/SCL pin,
Timer/Counter0 compare match, or software); the two-wire clock control
unit can hold SCL low.

The output latch delays a change of the data output to the clock edge
opposite the one that samples DI, so input is sampled and output changed
on opposite edges. The latch is transparent during the first half of a
serial clock cycle with an external clock (USICS1 = 1), and always
transparent with an internal clock (USICS1 = 0); a new MSB written while
it is open appears at once. DI/SDA is always the serial input, whatever
the wire mode. The data direction bit of the output pin must be 1 for
USIDR to drive it.

The 4-bit counter is read and written over the data bus and sets USIOIF
when it overflows (15 → 0). With an external clock it counts **both**
clock edges, so it counts edges, not bits: a byte is 16 edges.

Maximum SCL and USCK frequency, and so the maximum data rate in both
two- and three-wire mode, is f~CK~/2. In two-wire slave mode the clock
control unit holds SCL low until the slave is ready, which can lower the
real rate.

#### Three-wire mode

Compatible with SPI modes 0 and 1, without slave-select (SS) logic
(do it in software). Pins: DO, DI, USCK. Two USI units connected
DO→DI exchange their USIDR contents in eight USCK cycles. The master
makes the clock in software, by toggling USCK through PORTB or by
writing 1 to USITC. External clock mode 0 (USICS0 = 0) samples DI on
rising edges and shifts DO on falling edges (SPI mode 0); mode 1
(USICS0 = 1) uses the opposite edges (SPI mode 1).

A transfer: (A, B) slave and master write the byte to send into USIDR,
enable their outputs in DDRB and reset the counter; both at least half
a USCK cycle before the first sampling edge (C), for data setup. (C, D)
the master toggles USCK twice: DI is sampled on the first edge, DO
changes on the second; the counter counts both. This repeats eight
times. (E) after 16 edges the counter overflows, ending the transfer;
the overflow interrupt wakes the CPU from Idle. Without USIBR, the byte
must be handled before the next transfer starts.

```wavedrom id=usi-3wire caption="Three-wire mode timing (USICS0 = 0 and USICS0 = 1)"
{"signal": [
  {"name": "USCK (mode 0)", "wave": "0.1010101010101010.", "node": "..CD..............E"},
  {"name": "USCK (mode 1)", "wave": "1.0101010101010101."},
  {"name": "DO", "wave": "x=.=.=.=.=.=.=.=.x.", "data": ["MSB", "6", "5", "4", "3", "2", "1", "LSB"], "node": ".A"},
  {"name": "DI", "wave": "x=.=.=.=.=.=.=.=.x.", "data": ["MSB", "6", "5", "4", "3", "2", "1", "LSB"], "node": ".B"}
 ],
 "edge": ["A~>C setup ≥ ½ USCK cycle", "B~>C"],
 "config": {"hscale": 2}}
```

As an SPI master, a byte is sent and received by writing it to USIDR,
clearing USIOIF and the counter (write USISR = 1 << USIOIF), and writing
USICR = USIWM0 | USICS1 | USICLK | USITC sixteen times (until USIOIF
sets); the received byte is then in USIDR. Writing USICR alternately with
USIWM0 | USITC and USIWM0 | USITC | USICLK (software strobe) sixteen
times gives the maximum rate, f~SCK~ = f~CK~/2. As a slave, set
USICR = USIWM0 | USICS1 once, then per byte write USIDR, clear USIOIF and
wait for USIOIF.

#### Two-wire mode

Compatible with the TWI (I²C) protocol, but outputs have no slew-rate
limiting and inputs no noise filter. Pins: SDA (DI) and SCL (USCK), both
open-collector. Only the physical layer is in hardware: the master makes
the clock in software (toggling SCL through PORTB); shifting is
automatic in both master and slave; only the slave uses the clock
control unit. Only shifting on negative edges is practical in this mode.
A slave can insert wait states at the start or end of a transfer by
holding SCL low, so a master must check that SCL really went high after
it released it. Data direction is up to the protocol.

A transfer: (A) the master makes a START by pulling SDA low while SCL is
high (write 0 to USIDR bit 7, or to the PORTB bit; DDRB bit = 1). The
slave's start detector sets USISIF, which can interrupt. (B) The start
detector then holds SCL low after the master's falling SCL edge, giving
the slave time to wake and prepare; the slave releases SCL by clearing
USISIF, after resetting the counter. (C) The master sets the first bit and
releases SCL; the slave samples on rising SCL edges. (D) After 8 bits
(address and R/W) the slave counter overflows and SCL is held low; a
slave that is not addressed releases SCL and waits for the next START.
An addressed slave drives SDA low for the acknowledge, then holds SCL
low again: set the counter to 14 before releasing SCL at (D). The R/W
bit decides who drives SDA next (1 = master read: the slave drives). (E)
The slave may hold SCL after the acknowledge. Bytes follow in the same
direction until the master sends a STOP (F) or a new START. A slave that
can take no more data does not acknowledge the last byte; a master
ending a read must signal the end in the acknowledge bit after the last
byte.

```wavedrom id=usi-2wire caption="Two-wire mode, typical transfer (S = START, P = STOP)"
{"signal": [
  {"name": "SDA", "wave": "10.=..=.=...=.01.", "data": ["address 1–7, R/W 8", "ACK 9", "data 1–8", "ACK 9"]},
  {"name": "SCL", "wave": "1.0ppp0p0ppp0p1.."}
 ],
 "config": {"hscale": 2}}
```

The start condition detector is enabled only in two-wire mode. It
delays SDA by 50 ns to 300 ns to sample SCL validly, and works
asynchronously, so a START wakes the CPU from every sleep mode,
Power-down included; the oscillator start-up time (CKSEL fuses) then
adds to the SCL hold time, which the protocol may limit.

#### Other uses

USIDR in three-wire mode makes a compact half-duplex UART. The 4-bit
counter can be used alone with its overflow interrupt (an external clock
counts both edges), combined with an 8-bit timer/counter as a 12-bit
counter, as an extra edge-triggered external interrupt (counter preset
to 0xF, USICS1 selects it; USIOIF/USIOIE serve as flag and enable), or
as a software interrupt triggered by a clock strobe.

#### USIDR – USI data register (0x0F)

Read/write, reset 0x00. A left shift happens on the clock selected by
USICS[1:0] (external edge, Timer/Counter0 compare match, or USICLK). If a
serial clock comes in the same cycle as a write, the written value is
kept and no shift happens. Read it quickly, or use USIBR. With no wire
mode (USIWM[1:0] = 00) DI/SDA and USCK/SCL can still clock and feed
USIDR.

#### USIBR – USI buffer register (0x10)

Read-only, reset 0x00. Loaded with USIDR when a transfer completes, so
the CPU has more time to read the data; reading it affects the flags as
reading USIDR does.

#### USISR – USI status register (0x0E)

```table id=usi-usisr caption="USISR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | USISIF | R/W | 0 | Start condition interrupt flag. Two-wire mode: set by a START. Three-wire or output-disable mode: set by any edge on USCK. Interrupts if USISIE = 1 and I = 1. Cleared only by writing 1; clearing it releases the start-detector hold of SCL. Wakes the CPU from all sleep modes. |
| 6 | USIOIF | R/W | 0 | Counter overflow interrupt flag: set when the counter goes 15 → 0. Interrupts if USIOIE = 1 and I = 1. Cleared only by writing 1; clearing it releases the overflow hold of SCL (two-wire mode). Wakes the CPU from Idle. |
| 5 | USIPF | R/W | 0 | Stop condition flag (two-wire mode): set by a STOP; cleared by writing 1. Not an interrupt flag; for bus-master arbitration. |
| 4 | USIDC | R | 0 | Data output collision: 1 when USIDR bit 7 differs from the pin level. Valid in two-wire mode only; for arbitration. |
| 3:0 | USICNT[3:0] | R/W | 0 | Counter value, read and written directly. Counts each clock from the source chosen by USICS[1:0] and USICLK (external edges, Timer/Counter0 compare match, USICLK or USITC strobe). With no wire mode the USCK/SCL input still clocks it. |
```

#### USICR – USI control register (0x0D)

```table id=usi-usicr caption="USICR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | USISIE | R/W | 0 | Start condition interrupt enable. A pending USISIF interrupts at once when USISIE and I are set. |
| 6 | USIOIE | R/W | 0 | Counter overflow interrupt enable. A pending USIOIF interrupts at once when USIOIE and I are set. |
| 5:4 | USIWM[1:0] | R/W | 0 | Wire mode; see table usi-wm. Affects only the outputs; data and clock inputs always work, so the counter and USIDR can be clocked and DI sampled with outputs disabled. |
| 3:2 | USICS[1:0] | R/W | 0 | Clock source for USIDR and the counter; see table usi-cs. With software strobe or Timer/Counter0 compare match the output latch is transparent, so the output changes at once. |
| 1 | USICLK | W | 0 | Clock strobe. With USICS[1:0] = 00, writing 1 shifts USIDR one step and increments the counter; the output changes in the same instruction cycle, and the bit shifted in was sampled the cycle before. With an external clock (USICS1 = 1) it selects the counter clock instead: 1 = the USITC strobe. Reads 0. |
| 0 | USITC | W | 0 | Toggle clock port pin: writing 1 toggles the USCK/SCL PORTB bit (0 → 1 or 1 → 0), independently of DDRB (the DDRB bit must be 1 to show it on the pin). With USICS1 = 1 and USICLK = 1, writing USITC also clocks the counter directly, for early end-of-transfer detection in a master. Reads 0. |
```

```table id=usi-wm caption="Wire mode, USIWM[1:0]"
| USIWM1 | USIWM0 | Operation |
|:-:|:-:|---|
| 0 | 0 | Outputs, clock hold and start detector disabled; port pins work normally. |
| 0 | 1 | Three-wire mode, pins DO, DI, USCK. DO overrides its PORTB bit, but DDRB still sets the direction; as an input its pull-up follows PORTB. DI and USCK don't affect normal port operation. A master makes clock pulses by toggling PORTB with the pin an output (USITC can do it). |
| 1 | 0 | Two-wire mode, pins SDA (DI) and SCL (USCK) [1]: bidirectional, open-collector, drivers enabled by their DDRB bits. An enabled SDA driver pulls SDA low when USIDR bit 7 or the PORTB bit is 0, otherwise releases it. An enabled SCL driver pulls SCL low when the PORTB bit is 0 or the start detector holds it, otherwise releases it. A START holds SCL low (output enabled) until USISIF is cleared. Inputs are not affected. Pull-ups on SDA and SCL are disabled. |
| 1 | 1 | Two-wire mode as 10, and SCL is also held low after a counter overflow until USIOIF is cleared. |

[1] DI and USCK are called SDA and SCL in two-wire mode.
```

```table id=usi-cs caption="Clock source, USICS[1:0] and USICLK"
| USICS1 | USICS0 | USICLK | USIDR clock | 4-bit counter clock |
|:-:|:-:|:-:|---|---|
| 0 | 0 | 0 | No clock | No clock |
| 0 | 0 | 1 | Software strobe (USICLK) | Software strobe (USICLK) |
| 0 | 1 | X | Timer/Counter0 compare match | Timer/Counter0 compare match |
| 1 | 0 | 0 | External, positive edge | External, both edges |
| 1 | 1 | 0 | External, negative edge | External, both edges |
| 1 | 0 | 1 | External, positive edge | Software strobe (USITC) |
| 1 | 1 | 1 | External, negative edge | Software strobe (USITC) |
```

### Analog comparator

Compares AIN0 (positive) with AIN1 (negative): ACO = 1 when AIN0 is
higher. It has its own interrupt, on output rise, fall or toggle.
ACBG replaces AIN0 with the internal 1.1 V bandgap reference. With
ACME = 1 and the ADC off (ADEN = 0), the ADC multiplexer picks the
negative input instead of AIN1 (table ac-mux); the ADC must be off to
use this.

```table id=ac-mux caption="Comparator negative input"
| ACME | ADEN | MUX[1:0] | Negative input |
|:-:|:-:|:-:|---|
| 0 | X | XX | AIN1 |
| 1 | 1 | XX | AIN1 |
| 1 | 0 | 00 | ADC0 |
| 1 | 0 | 01 | ADC1 |
| 1 | 0 | 10 | ADC2 |
| 1 | 0 | 11 | ADC3 |
```

ACME is bit 6 of ADCSRB, described under ADC below.

#### ACSR – Analog comparator control and status register (0x08)

```table id=ac-acsr caption="ACSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | ACD | R/W | 0 | Analog comparator disable: 1 switches the comparator's power off (saves power in Active and Idle), at any time. Clear ACIE before changing ACD, or an interrupt can occur. |
| 6 | ACBG | R/W | 0 | Bandgap select: 1 = fixed bandgap reference replaces AIN0 on the positive input; 0 = AIN0. The reference needs time to settle; an unsettled first conversion may be wrong (see Internal voltage reference, under Power management). |
| 5 | ACO | R | N/A | Comparator output, synchronized (1–2 clock cycles of delay). |
| 4 | ACI | R/W | 0 | Interrupt flag: set when an output event matches ACIS[1:0]. Interrupts if ACIE = 1 and I = 1. Cleared by hardware when the vector runs, or by writing 1. |
| 3 | ACIE | R/W | 0 | Interrupt enable (with I = 1). |
| 2 | — | R | 0 | Reserved, reads 0 |
| 1:0 | ACIS[1:0] | R/W | 0 | Interrupt mode: 00 = output toggle; 01 = reserved; 10 = falling output edge; 11 = rising output edge. Clear ACIE before changing them, or an interrupt can occur. |
```

DIDR0 bits AIN1D and AIN0D disable the digital inputs of the comparator
pins; see DIDR0 under ADC.

### ADC

A 10-bit successive-approximation ADC with a sample-and-hold, fed by a
multiplexer: four single-ended inputs ADC0–ADC3 (PB5, PB2, PB4, PB3)
against GND, the temperature sensor (ADC4), V~BG~ and GND, and
differential pairs (ADC2/ADC3 and ADC0/ADC1, positive ADC2 or ADC0)
through a gain stage of 1× or 20× (26 dB). The reference V~REF~ is V~CC~,
the AREF pin (PB0), or the internal 1.1 V or 2.56 V reference (the
2.56 V one optionally bypassed by a capacitor on AREF for less noise).
Block diagram: the input multiplexers and gain amplifier feed the
sample-and-hold comparator and a 10-bit DAC, under the conversion logic,
prescaler and trigger select, with ADMUX, ADCSRA, ADCSRB and ADCH/ADCL
on the data bus.

The result is 0 for GND and 0x3FF for V~REF~ minus 1 LSB. ADEN in
ADCSRA enables the ADC; reference and channel selections take effect
only once ADEN is set. With ADEN = 0 the ADC draws no power; switch it
off before power-saving sleep modes.

Selecting ADC0 or ADC2 as both inputs of the gain stage (ADC0–ADC0,
ADC2–ADC2) measures the remaining offset of the gain stage and
converter; subtracting it from later conversions with the same gain
brings the offset error below 1 LSB.

#### Result registers

The result is right adjusted in ADCH:ADCL, or left adjusted with ADLAR =
1. If left adjusted and 8 bits are enough, read only ADCH. Otherwise read
ADCL first, then ADCH: reading ADCL blocks the ADC from updating both
registers until ADCH is read, so a conversion that completes in between
is lost (its interrupt still fires).

#### Starting a conversion

Writing 1 to ADSC starts a single conversion; ADSC reads 1 until the
conversion ends and hardware clears it. A channel change during a
conversion waits for the conversion to finish.

With ADATE = 1, a rising edge of the trigger chosen by ADTS[2:0]
(ADCSRB) resets the ADC prescaler and starts a conversion, giving
conversions at fixed intervals. A trigger still high when a conversion
ends doesn't start another; a rising edge during a conversion is
ignored. The trigger's interrupt flag is set even if its interrupt is
disabled, so conversions can be triggered without interrupts, but the
flag must be cleared before the next event can trigger. Using ADIF as the
trigger gives Free Running mode: the first conversion is started by
writing ADSC, then conversions follow back to back whether or not ADIF is
cleared. With auto triggering on, ADSC still starts single conversions
and still reads 1 during any conversion.

#### Prescaling and conversion timing

The converter needs a 50 kHz to 200 kHz clock for full 10-bit resolution;
up to 1 MHz is usable for less resolution (higher is not recommended).
The 7-bit prescaler (ADPS[2:0]: CK/2 to CK/128) makes a suitable clock
from any CPU clock above 100 kHz. It runs while ADEN = 1 and is held
reset while ADEN = 0.

A conversion starts on the rising ADC clock edge after ADSC is written,
and takes 13 ADC clock cycles; the first after ADEN is set takes 25, to
initialize the analog circuitry. When it completes, the result is
written, ADIF is set and, in single conversion mode, ADSC is cleared at
the same time; setting ADSC again starts the next conversion on the next
rising ADC clock edge. With auto triggering, the prescaler is reset by
the trigger, so the delay from trigger to start is fixed; three extra
CPU clock cycles are used for synchronization. In Free Running mode the
next conversion starts as soon as one completes, with ADSC staying 1.

```table id=adc-conv caption="Conversion time"
| Condition | Sample-and-hold (ADC clock cycles from start) | Total conversion time (ADC clock cycles) |
|---|--:|--:|
| First conversion | 13.5 | 25 |
| Normal conversion | 1.5 | 13 |
| Auto-triggered conversion | 2 | 13.5 |
```

```wavedrom id=adc-single caption="Single conversion"
{"signal": [
  {"name": "ADC clock", "wave": "lpppppppppppppppp", "node": ".A.B..........C"},
  {"name": "ADSC", "wave": "01............0.."},
  {"name": "ADIF", "wave": "0.............1.."},
  {"name": "ADCH", "wave": "x.............=..", "data": ["sign and MSB of result"]},
  {"name": "ADCL", "wave": "x.............=..", "data": ["LSB of result"]}
 ],
 "edge": ["A~>B sample & hold", "A<->C 13 cycles, conversion complete"],
 "config": {"hscale": 2}}
```

MUX and REFS are updated (see below) in the cycle before a conversion
starts and again in the last cycle of a conversion. The first
conversion looks the same with 25 cycles and the sample-and-hold at
13.5; an auto-triggered one starts at the trigger's rising edge with the
prescaler reset, and in Free Running mode cycle 1 of the next
conversion follows cycle 13 directly.

#### Changing channel or reference

MUX[3:0] and REFS[2:0] pass through a temporary register: the selection
follows writes until a conversion starts, is then locked for sampling,
and follows again from the last ADC clock cycle of the conversion (when
ADIF sets). Since a conversion starts on the ADC clock edge after ADSC is
written, don't write ADMUX until one ADC clock cycle after writing ADSC.

With ADATE and ADEN both 1, a trigger can come at any time, so a write
to ADMUX may or may not affect the next conversion. ADMUX is safe to
update (affecting the next conversion) when ADATE or ADEN is 0, during a
conversion at least one ADC clock cycle after the trigger, or after a
conversion before the interrupt flag used as trigger is cleared.

Single conversion mode: select the channel before starting; it may
change one ADC clock cycle after ADSC is written, or (simplest) after the
conversion. Free Running mode: select the channel before the first
conversion; a change one cycle after ADSC, or after the first
conversion, affects the conversion after next (the next has already
started on the old channel).

Inputs above V~REF~ read close to 0x3FF. Discard the first result after
changing the reference. After switching to an internal reference, allow
1 ms settling (ADC enabled) before conversions are reliable.

#### Noise canceler

Conversions can run in ADC Noise Reduction or Idle sleep mode, away from
CPU and I/O noise: with the ADC enabled and idle, single conversion mode
and the conversion-complete interrupt enabled, enter the sleep mode; the
conversion starts once the CPU halts. The ADC interrupt then wakes the
CPU; if another interrupt wakes it first, that one runs and the ADC
interrupt follows when the conversion completes, the CPU staying active
until the next SLEEP. Other sleep modes don't switch the ADC off: clear
ADEN first. Noise Reduction mode is recommended when accuracy matters,
especially above 1 MHz system clock and for the temperature sensor.
Also: keep analog paths short and over the analog ground plane, away
from fast digital tracks; don't switch port outputs during a conversion;
bypass V~CC~ close to the pins.

#### Analog input circuitry

Each ADCn pin has its pin capacitance and leakage (I~IH~, I~IL~)
whether selected or not. When selected, the source drives the
sample-and-hold capacitor C~S/H~ = 14 pF through a series resistance of
1 kΩ to 100 kΩ (to V~CC~/2). Sources of about 10 kΩ or less give a
negligible sampling time; higher impedances need longer, so use them only
for slowly varying signals. Filter out components above f~ADC~/2
(Nyquist) before the input.

#### Accuracy definitions

An n-bit single-ended ADC converts GND…V~REF~ linearly in 2^n^ steps
(codes 0 to 2^n^ − 1). Offset: deviation of the first transition
(0x000 → 0x001) from the ideal 0.5 LSB; ideal 0. Gain error: after offset
correction, deviation of the last transition (0x3FE → 0x3FF) from the
ideal 1.5 LSB below full scale; ideal 0. INL: after offset and gain
correction, the largest deviation of any transition from ideal; ideal 0.
DNL: largest deviation of a code width from 1 LSB; ideal 0. Quantization
error: always ±0.5 LSB. Absolute accuracy: the largest deviation of any
unadjusted transition from ideal, all the above combined; ideal ±0.5 LSB.

#### Conversion result

```equation
Single ended: ADC = V~IN~ × 1024 / V~REF~
Unipolar differential: ADC = (V~POS~ − V~NEG~) × 1024 / V~REF~ × GAIN
Bipolar differential: ADC = (V~POS~ − V~NEG~) × 512 / V~REF~ × GAIN
```

Single ended: 0x000 = GND, 0x3FF = V~REF~ − 1 LSB. Unipolar differential
(default): V~POS~ must exceed V~NEG~, otherwise the result saturates
to 0; result 0x000 (0) to 0x3FF (+1023). Bipolar differential (BIN = 1):
either input may be higher; the result is two's complement, 0x200
(−512) through 0x000 to 0x1FF (+511), 9 bits plus sign. GAIN is 1 or 20.
A signal that is not bipolar loses a bit of range in bipolar mode; for
full range, check the polarity first (ADC9 in ADCH: 1 = negative), then
convert in unipolar mode with IPR set as needed to reverse the pair.

#### Temperature measurement

MUX[3:0] = 1111 selects ADC4, the on-chip temperature sensor; the 1.1 V
internal reference must be selected. Use single conversion mode. The
output is linear in temperature, about 1 LSB/°C; accuracy depends on
calibration: typically ±10 °C after a single calibration at room
temperature, better with two points.

```table id=adc-temp caption="Temperature sensor output, typical"
| Temperature | -40 °C | 25 °C | 85 °C |
|---|--:|--:|--:|
| ADC (LSB) | 230 | 300 | 370 |
```

The output varies from chip to chip; calibrate in software:

```equation
T = k × [(ADCH << 8) | ADCL] + T~OS~
```

k is the slope (very close to 1.0; may be omitted for one-point
calibration; measure at two temperatures for more accuracy) and T~OS~
the sensor offset.

#### ADMUX – ADC multiplexer selection register (0x07)

```table id=adc-admux caption="ADMUX bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | REFS[1:0] | R/W | 0 | Voltage reference, with REFS2; see table adc-refs |
| 5 | ADLAR | R/W | 0 | 1 = left adjust the result. Takes effect at once, even during a conversion. |
| 4 | REFS2 | R/W | 0 | Voltage reference, with REFS[1:0] |
| 3:0 | MUX[3:0] | R/W | 0 | Input channel and gain; see table adc-mux. A change during a conversion takes effect when it completes (ADIF set). |
```

A change of REFS[2:0] during a conversion takes effect when it completes,
and the next conversion then takes 25 ADC clock cycles. With differential
channels and gain, V~CC~ or an external AREF above V~CC~ − 1 V as
reference is not recommended (it degrades accuracy).

```table id=adc-refs caption="Voltage reference, REFS[2:0]"
| REFS2 | REFS1 | REFS0 | V~REF~ |
|:-:|:-:|:-:|---|
| X | 0 | 0 | V~CC~; AREF (PB0) disconnected |
| X | 0 | 1 | External reference on AREF (PB0); internal reference off |
| 0 | 1 | 0 | Internal 1.1 V |
| 0 | 1 | 1 | Reserved |
| 1 | 1 | 0 | Internal 2.56 V, no bypass capacitor; AREF (PB0) disconnected [1] |
| 1 | 1 | 1 | Internal 2.56 V with a bypass capacitor on AREF (PB0) [1] |

[1] Needs V~CC~ ≥ 3 V.
```

```table id=adc-mux caption="Input channel and gain, MUX[3:0]"
| MUX[3:0] | Single-ended input | Positive differential input | Negative differential input | Gain |
|---|---|---|---|---|
| 0000 | ADC0 (PB5) | — | — | — |
| 0001 | ADC1 (PB2) | — | — | — |
| 0010 | ADC2 (PB4) | — | — | — |
| 0011 | ADC3 (PB3) | — | — | — |
| 0100 | — | ADC2 (PB4) | ADC2 (PB4) | 1x |
| 0101 [1] | — | ADC2 (PB4) | ADC2 (PB4) | 20x |
| 0110 | — | ADC2 (PB4) | ADC3 (PB3) | 1x |
| 0111 | — | ADC2 (PB4) | ADC3 (PB3) | 20x |
| 1000 | — | ADC0 (PB5) | ADC0 (PB5) | 1x |
| 1001 | — | ADC0 (PB5) | ADC0 (PB5) | 20x |
| 1010 | — | ADC0 (PB5) | ADC1 (PB2) | 1x |
| 1011 | — | ADC0 (PB5) | ADC1 (PB2) | 20x |
| 1100 [2] | V~BG~ | — | — | — |
| 1101 | GND | — | — | — |
| 1110 | — | — | — | — |
| 1111 [3] | ADC4 (temperature sensor) | — | — | — |

[1] For offset calibration only.
[2] After switching to the internal reference, the ADC needs 1 ms (enabled) to settle; earlier conversions may be unreliable.
[3] Temperature sensor.
```

#### ADCSRA – ADC control and status register A (0x06)

```table id=adc-adcsra caption="ADCSRA bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | ADEN | R/W | 0 | ADC enable. Writing 0 turns the ADC off and ends a conversion in progress. |
| 6 | ADSC | R/W | 0 | Start conversion: write 1 for each single conversion, or for the first in Free Running mode. The first conversion after enabling (or with ADSC written together with ADEN) takes 25 ADC clock cycles instead of 13. Reads 1 during a conversion; writing 0 has no effect. |
| 5 | ADATE | R/W | 0 | Auto trigger enable: a rising edge of the source chosen by ADTS[2:0] starts a conversion |
| 4 | ADIF | R/W | 0 | Interrupt flag: set when a conversion completes and the data registers are updated. Interrupts if ADIE = 1 and I = 1. Cleared by hardware when the vector runs, or by writing 1. A read-modify-write of ADCSRA (also SBI/CBI) can clear a pending flag. |
| 3 | ADIE | R/W | 0 | Conversion complete interrupt enable (with I = 1) |
| 2:0 | ADPS[2:0] | R/W | 0 | Prescaler, system clock ÷ 000 = 2; 001 = 2; 010 = 4; 011 = 8; 100 = 16; 101 = 32; 110 = 64; 111 = 128 |
```

#### ADCH and ADCL – ADC data register (0x05, 0x04)

Read-only, reset 0x00.

```table id=adc-data caption="Result layout"
| ADLAR | Register | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|
| 0 | ADCH | — | — | — | — | — | — | ADC9 | ADC8 |
| ^ | ADCL | ADC7 | ADC6 | ADC5 | ADC4 | ADC3 | ADC2 | ADC1 | ADC0 |
| 1 | ADCH | ADC9 | ADC8 | ADC7 | ADC6 | ADC5 | ADC4 | ADC3 | ADC2 |
| ^ | ADCL | ADC1 | ADC0 | — | — | — | — | — | — |
```

#### ADCSRB – ADC control and status register B (0x03)

```table id=adc-adcsrb caption="ADCSRB bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | BIN | R/W | 0 | Bipolar input mode. 0 = unipolar (default): one-sided conversions, the positive input must be higher or the result saturates, 10-bit resolution. 1 = bipolar: two-sided, two's complement result, 9 bits plus sign. |
| 6 | ACME | R/W | 0 | Analog comparator multiplexer enable: with ADEN = 0, the ADC multiplexer selects the comparator's negative input (see Analog comparator); 0 = AIN1 |
| 5 | IPR | R/W | 0 | Input polarity reversal: swaps the differential pair, so a known polarity can be converted at full 10-bit unipolar resolution (find the polarity first with a bipolar conversion) |
| 4:3 | — | R | 0 | Reserved, read 0 |
| 2:0 | ADTS[2:0] | R/W | 0 | Auto trigger source, used when ADATE = 1: 000 = free running; 001 = analog comparator; 010 = external interrupt request 0; 011 = Timer/Counter0 compare match A; 100 = Timer/Counter0 overflow; 101 = Timer/Counter0 compare match B; 110 = pin change interrupt request. A conversion is triggered by the rising edge of the selected interrupt flag. Switching from a cleared source to a set one makes a rising edge, which starts a conversion if ADEN = 1. Switching to free running (000) is no trigger event, even with ADIF set. |
```

#### DIDR0 – Digital input disable register 0 (0x14)

```table id=adc-didr0 caption="DIDR0 bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved |
| 5 | ADC0D | R/W | 0 | 1 = digital input buffer of the pin off; its PINB bit reads 0. Set it on pins with an analog signal whose digital value isn't needed, to save power in the input buffer. |
| 4 | ADC2D | R/W | 0 | As ADC0D |
| 3 | ADC3D | R/W | 0 | As ADC0D |
| 2 | ADC1D | R/W | 0 | As ADC0D |
| 1 | AIN1D | R/W | 0 | As ADC0D, for the comparator input AIN1 |
| 0 | AIN0D | R/W | 0 | As ADC0D, for the comparator input AIN0 |
```

#### ADC characteristics

```table id=adc-se caption="ADC characteristics, single-ended channels [1]" conditions="T~A~ = -40 °C to 85 °C"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| | Resolution | | | | 10 | bits |
| | Absolute accuracy (INL, DNL, quantization, gain and offset errors) | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 2 | | LSB |
| ^ | ^ | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 1 MHz | | 3 | | LSB |
| ^ | ^ | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz, Noise Reduction mode | | 1.5 | | LSB |
| ^ | ^ | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 1 MHz, Noise Reduction mode | | 2.5 | | LSB |
| | Integral non-linearity (after offset and gain calibration) | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 1 | | LSB |
| | Differential non-linearity | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 0.5 | | LSB |
| | Gain error | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 2.5 | | LSB |
| | Offset error | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 1.5 | | LSB |
| | Conversion time | Free running conversion | 14 | | 280 | µs |
| | Clock frequency | | 50 | | 1000 | kHz |
| V~IN~ | Input voltage | | 0 | | V~REF~ | V |
| | Input bandwidth | | | 38.4 | | kHz |
| AREF | External reference voltage | | 2.0 | | V~CC~ | V |
| V~INT~ | Internal voltage reference | | 1.0 | 1.1 | 1.2 | V |
| ^ | Internal 2.56 V reference [2] | V~CC~ > 3.0 V | 2.3 | 2.56 | 2.8 | V |
| R~REF~ | Reference input resistance | | | 32 | | kΩ |
| R~AIN~ | Analog input resistance | | | 100 | | MΩ |
| | ADC output | | 0 | | 1023 | LSB |

[1] Minimum input voltage given as GND in the source.
[2] Values are guidelines only.
```

```table id=adc-diff caption="ADC characteristics, differential channels" conditions="T~A~ = -40 °C to 85 °C"
| Symbol | Parameter | Conditions | Unipolar mode | < | < | Bipolar mode | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| | Resolution | Gain = 1x | | | 10 | | | 10 | bits |
| ^ | ^ | Gain = 20x | | | 10 | | | 10 | bits |
| | Absolute accuracy (INL, DNL, quantization, gain and offset errors) | Gain = 1x [1] | | 10.0 | | | 8.0 | | LSB |
| ^ | ^ | Gain = 20x [1] | | 20.0 | | | 8.0 | | LSB |
| | Integral non-linearity (after offset and gain calibration) | Gain = 1x [1] | | 4.0 | | | 4.0 | | LSB |
| ^ | ^ | Gain = 20x [1] | | 10.0 | | | 5.0 | | LSB |
| | Gain error | Gain = 1x | | 10.0 | | | 4.0 | | LSB |
| ^ | ^ | Gain = 20x | | 15.0 | | | 5.0 | | LSB |
| | Offset error | Gain = 1x [1] | | 3.0 | | | 3.0 | | LSB |
| ^ | ^ | Gain = 20x [1] | | 4.0 | | | 4.0 | | LSB |
| | Conversion time | Free running conversion | 70 | | 280 | 70 | | 280 | µs |
| | Clock frequency | | 50 | | 200 | 50 | | 200 | kHz |
| V~IN~ | Input voltage [2] | | 0 | | V~CC~ | 0 | | V~CC~ | V |
| V~DIFF~ | Input differential voltage | | | | V~REF~/Gain | | | V~REF~/Gain | V |
| | Input bandwidth | | | 4 | | | 4 | | kHz |
| AREF | External reference voltage | | 2.0 | | V~CC~ - 1.0 | 2.0 | | V~CC~ - 1.0 | V |
| V~INT~ | Internal voltage reference | | 1.0 | 1.1 | 1.2 | 1.0 | 1.1 | 1.2 | V |
| ^ | Internal 2.56 V reference [3] | V~CC~ > 3.0 V | 2.3 | 2.56 | 2.8 | 2.3 | 2.56 | 2.8 | V |
| R~REF~ | Reference input resistance | | | 32 | | | 32 | | kΩ |
| R~AIN~ | Analog input resistance | | | 100 | | | 100 | | MΩ |
| | ADC conversion output | | 0 | | 1023 | -512 | | 511 | LSB |

[1] V~REF~ = 4 V, V~CC~ = 5 V, ADC clock = 50 kHz to 200 kHz.
[2] Minimum given as GND in the source.
[3] Values are guidelines only.
```

### debugWIRE

A one-wire, bidirectional on-chip debug interface: it controls program
flow, executes AVR instructions in the CPU and programs the non-volatile
memories. It emulates all on-chip functions, digital and analog, except
the !{RESET} pin, and runs in real time.

With the DWEN fuse programmed and the lock bits unprogrammed, the
!{RESET} pin becomes the dW line: a wired-AND (open-drain)
bidirectional I/O with pull-up enabled, connected to the debugger
together with V~CC~ (1.8 V to 5.5 V) and GND. The system clock stays
the one selected by the CKSEL fuses. Design rules:

- A pull-up on dW/!{RESET} is optional; if fitted, 10 kΩ to 20 kΩ.
- !{RESET} tied directly to V~CC~ does not work.
- Capacitors on !{RESET} must be disconnected while debugging.
- All external reset sources must be disconnected; an external reset
  is not supported while debugWIRE is enabled.

**Break points.** Program memory break points use the BREAK
instruction: the debugger stores the replaced instruction and executes it
when execution continues; BREAK may also be written into the program by
hand. Every break point change reprograms the flash, which reduces flash
data retention, so parts used for debugging should not be shipped.

**Limitations.** Running at full speed, all I/O functions are emulated
exactly; with the CPU stopped, some I/O registers need care when accessed
from the debugger. The interface is asynchronous and synchronizes to the
system clock: changing the clock in software (CLKPS bits) may break
communication, and clocks below 100 kHz may cause problems. A programmed
DWEN fuse keeps parts of the clock system running in every sleep mode,
raising sleep current; unprogram DWEN when debugWIRE is not used. With
DWEN programmed the flash can be read through debugWIRE even when the lock
bits are set.

#### DWDR – debugWIRE data register (0x22)

```table id=dw-dwdr caption="DWDR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:0 | DWDR[7:0] | R/W | 0 | Communication channel from the running program to the debugger. Only accessible by debugWIRE; not usable as a general-purpose register. |
```

### Self-programming

The MCU can program its own flash through any data interface, using the
SPM instruction. SPM is disabled until the SELFPRGEN fuse is programmed
(0).

The flash is written page by page (page size in table mp-flash-pages
below). A page must be erased before it is written from the temporary
page buffer. The buffer is filled one word at a time with SPM, either
before the page erase (fill, erase, write: this allows read-modify-write
of the page, since the old data can be read while loading) or after it
(erase, fill, write). If only part of a page changes, the rest must be
saved (for example in the buffer) before the erase and written back. The
buffer may be filled in any order; page erase and page write must
address the same page.

Each operation is a timed sequence: set up Z (and R1:R0), write the
SPMCSR value, and execute SPM within four clock cycles.

```table id=spm-ops caption="SPM operations"
| Operation | SPMCSR value | Z-pointer | R1:R0 | Notes |
|---|---|---|---|---|
| Page erase | 0b00000011 | Page address in PCPAGE; other bits ignored | Ignored | CPU halted during the erase |
| Load buffer word | 0b00000001 | Word in PCWORD; Z0 ignored | Instruction word | Each buffer address may be written once only until the buffer is erased |
| Page write | 0b00000101 | Page address in PCPAGE; other bits must be 0 | Ignored | CPU halted during the write |
```

The temporary buffer is erased automatically after a page write, by
writing CTPB, and by a system reset. An EEPROM write during page loading
loses all loaded data. An EEPROM write also blocks all software flash
programming and the reading of fuse and lock bits from software; check
that EEPE in EECR is 0 before writing SPMCSR.

**Flash addressing.** Z addresses the SPM commands. The program counter
splits into PCWORD (word within a page, the low bits) and PCPAGE (page,
the high bits); see table mp-flash-pages. Page erase and page write are
addressed independently, so software must give both the same page. LPM
addresses the flash by byte, so it also uses Z0.

**Reading lock bits, fuses and the signature from software.** Load Z, set
RFLB and SELFPRGEN in SPMCSR (RSIG and SPMEN for the signature imprint
table), and issue LPM within three clock cycles; the destination register
receives the data. RFLB and SELFPRGEN clear when the read completes, or
if no LPM follows within three CPU cycles or no SPM within four; then LPM
works normally. Programmed bits read as 0, unprogrammed as 1.

```table id=spm-read caption="Software read of fuse, lock and signature data"
| Z | SPMCSR bits | Data returned |
|---|---|---|
| 0x0000 | RFLB, SELFPRGEN | Fuse low byte, FLB7…FLB0 |
| 0x0001 | RFLB, SELFPRGEN | Lock bits: bit 1 = LB2, bit 0 = LB1, bits 7:2 undefined |
| 0x0002 | RFLB, SELFPRGEN | Fuse extended byte, FEB7…FEB0 |
| 0x0003 | RFLB, SELFPRGEN | Fuse high byte, FHB7…FHB0 |
| table index | RSIG, SPMEN | Device signature imprint table entry (table mp-dsit) |
```

**Preventing flash corruption.** At too low a V~CC~ a flash write can
fail, and the CPU can execute instructions wrongly. Either keep the part
in reset during low supply (enable the brown-out detector if its level
fits, else an external reset circuit; a write in progress completes if
the supply is sufficient), or keep the core in power-down sleep during
low V~CC~, which protects SPMCSR from unintended writes.

```table id=spm-time caption="SPM programming time (timed by the calibrated RC oscillator)"
| Parameter | Min | Max | Unit |
|---|--:|--:|---|
| Flash write: page erase, page write, or lock-bit write by SPM [1] | 3.7 | 4.5 | ms |

[1] Per individual operation.
```

#### SPMCSR – Store program memory control and status register (0x37)

```table id=spm-spmcsr caption="SPMCSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | — | R | 0 | Reserved, read as 0 |
| 5 | RSIG | R/W | 0 | Read device signature imprint table: an LPM within three cycles after RSIG and SPMEN are set returns the table entry selected by Z |
| 4 | CTPB | R/W | 0 | Clear temporary page buffer: writing it while the buffer is being filled clears the buffer; loaded data is lost |
| 3 | RFLB | R/W | 0 | Read fuse and lock bits: an LPM within three cycles after RFLB and SPMEN are set reads the lock bits or the fuse bits (selected by Z) |
| 2 | PGWRT | R/W | 0 | Page write: written 1 together with SPMEN, the next SPM within four cycles writes the buffer to the page addressed by Z's high part; R1:R0 ignored. Clears when the write completes, or if no SPM follows within four cycles. CPU halted during the write. |
| 1 | PGERS | R/W | 0 | Page erase: written 1 together with SPMEN, the next SPM within four cycles erases the page addressed by Z's high part; R1:R0 ignored. Clears when the erase completes, or if no SPM follows within four cycles. CPU halted during the erase. |
| 0 | SPMEN | R/W | 0 | Store program memory enable: enables SPM for the next four clock cycles. With RSIG, CTPB, RFLB, PGWRT or PGERS the next LPM/SPM has that special meaning; alone, the next SPM stores R1:R0 in the buffer word addressed by Z (Z0 ignored). Clears when the SPM completes, or after four cycles without SPM; stays 1 during page erase and page write until they complete. |
```

### Memory programming

The flash, EEPROM, fuses and lock bits are programmed by serial
downloading (SPI-like, !{RESET} low, low voltage) or by high-voltage
serial programming (12 V on !{RESET}); debugWIRE and self-programming are
described above. "Programmed" means 0, "unprogrammed" 1.

#### Lock bits

Lock bits are erased to 1 only by chip erase. Program the fuses before
the lock bits. Firmware can read them (see Self-programming). For lock
bit security, also unprogram DWEN: debugWIRE can read the flash even with
the lock bits set.

```table id=mp-lock caption="Lock bit byte"
| Bit | Name | Description | Default |
|---|---|---|---|
| 7:2 | — | Unused | 1 |
| 1 | LB2 | Lock bit | 1 |
| 0 | LB1 | Lock bit | 1 |
```

```table id=mp-lock-modes caption="Lock bit protection modes"
| LB mode | LB2 | LB1 | Protection |
|--:|:-:|:-:|---|
| 1 | 1 | 1 | No memory lock features enabled |
| 2 | 1 | 0 | Further programming of flash and EEPROM disabled in high-voltage and serial programming mode. Fuse bits locked in both modes. debugWIRE disabled. |
| 3 | 0 | 0 | Further programming and verification of flash and EEPROM disabled in high-voltage and serial programming mode. Fuse bits locked in both modes. debugWIRE disabled. |
```

#### Fuse bytes

Three fuse bytes. Chip erase does not change them; a programmed LB1
locks them. Fuses are latched on power-up and when the part enters
programming mode; changes take effect only after leaving programming
mode, except EESAVE, which acts as soon as it is programmed. Firmware can
read them (see Self-programming).

```table id=mp-fuse-ext caption="Fuse extended byte"
| Bit | Name | Description | Default |
|---|---|---|---|
| 7:1 | — | Unused | 1 |
| 0 | SELFPRGEN | Self-programming (SPM instruction) enabled | 1 |
```

```table id=mp-fuse-high caption="Fuse high byte"
| Bit | Name | Description | Default |
|---|---|---|---|
| 7 | RSTDISBL [1][2] | External reset disabled | 1 |
| 6 | DWEN [1][2][3] | debugWIRE enabled | 1 |
| 5 | SPIEN [4] | Serial program and data downloading enabled | 0 (SPI programming enabled) |
| 4 | WDTON | Watchdog timer always on (see Reset and watchdog) | 1 |
| 3 | EESAVE | EEPROM preserved through chip erase | 1 (not preserved) |
| 2:0 | BODLEVEL[2:0] | Brown-out detector trigger level (BODLEVEL coding in [electrical characteristics](#electrical-characteristics)) | 111 |

[1] Controls the use of the !{RESET} pin (see I/O ports, alternate functions of Port B).
[2] Once programmed, the part can be programmed only in high-voltage serial mode.
[3] Must be unprogrammed when lock bit security is required.
[4] Not accessible in serial (SPI) programming mode.
```

```table id=mp-fuse-low caption="Fuse low byte"
| Bit | Name | Description | Default |
|---|---|---|---|
| 7 | CKDIV8 | Clock divided by 8 (see Clock system, system clock prescaler) | 0 |
| 6 | CKOUT | System clock output on a pin (see Clock system, clock output buffer) | 1 |
| 5:4 | SUT[1:0] | Start-up time setting [1] | 10 |
| 3:0 | CKSEL[3:0] | Clock source setting [2] | 0010 |

[1] The default gives the longest start-up time for the default clock source (see Clock system).
[2] The default selects the internal 8 MHz RC oscillator (see Clock system).
```

#### Device signature imprint table

A separate address space holding the signature and oscillator
calibration data, readable in both programming modes (even when locked)
and from firmware.

```table id=mp-dsit caption="Device signature imprint table"
| Address | Contents |
|---|---|
| 0x00 | Signature byte 0 |
| 0x01 | Calibration data for the internal oscillator at 8.0 MHz |
| 0x02 | Signature byte 1 |
| 0x03 | Calibration data for the internal oscillator at 6.4 MHz |
| 0x04 | Signature byte 2 |
| 0x05…0x2A | Reserved for internal use |
```

```table id=mp-sig caption="Device signature bytes"
| Part | Byte 0 | Byte 1 | Byte 2 |
|---|---|---|---|
| ATtiny25 | 0x1E | 0x91 | 0x08 |
| ATtiny45 | 0x1E | 0x92 | 0x06 |
| ATtiny85 | 0x1E | 0x93 | 0x0B |
```

During reset the 8 MHz calibration byte is copied into OSCCAL; in
ATtiny15 compatibility mode the 6.4 MHz byte is used instead.

#### Page size

```table id=mp-flash-pages caption="Flash pages"
| Part | Flash size | Page size | PCWORD | Pages | PCPAGE | PCMSB |
|---|---|---|---|--:|---|--:|
| ATtiny25 | 1K words (2 KB) | 16 words | PC[3:0] | 64 | PC[9:4] | 9 |
| ATtiny45 | 2K words (4 KB) | 32 words | PC[4:0] | 64 | PC[10:5] | 10 |
| ATtiny85 | 4K words (8 KB) | 32 words | PC[4:0] | 128 | PC[11:5] | 11 |
```

```table id=mp-eeprom-pages caption="EEPROM pages"
| Part | EEPROM size | Page size | Byte in page | Pages | Page address | EEAMSB |
|---|---|---|---|--:|---|--:|
| ATtiny25 | 128 bytes | 4 bytes | EEA[1:0] | 32 | EEA[6:2] | 6 |
| ATtiny45 | 256 bytes | 4 bytes | EEA[1:0] | 64 | EEA[7:2] | 7 |
| ATtiny85 | 512 bytes | 4 bytes | EEA[1:0] | 128 | EEA[8:2] | 8 |
```

#### Serial downloading

Flash and EEPROM are programmed over SCK, MOSI and MISO while !{RESET}
is held low. A valid clock for the CKSEL setting must be present (none
needs connecting when the internal oscillator is selected). Input data is
clocked on the rising edge of SCK, output data on the falling edge, MSB
first, in four-byte instructions. SCK low and high periods must each be
longer than 2 CPU clock cycles for f~ck~ < 12 MHz, 3 cycles for f~ck~ ≥ 12
MHz. EEPROM writes in this mode include an automatic erase, so no chip
erase is needed first; chip erase sets every flash and EEPROM location to
0xFF.

```table id=mp-spi-pins caption="Serial programming pins"
| Signal | Pin | I/O | Function |
|---|---|---|---|
| MOSI | PB0 | I | Serial data in |
| MISO | PB1 | O | Serial data out |
| SCK | PB2 | I | Serial clock |
```

Programming sequence:

1. Power up with !{RESET} and SCK at 0. If the programmer cannot hold SCK
   low during power-up, give !{RESET} a positive pulse of at least
   t~RST~ plus two CPU clock cycles after SCK is 0.
2. Wait at least 20 ms, then send Programming Enable on MOSI.
3. When in sync, the second byte (0x53) echoes back while the third byte
   is sent. Send all four bytes regardless; if 0x53 did not echo, pulse
   !{RESET} high and send Programming Enable again.
4. Flash is programmed a page at a time: load each byte with Load Program
   Memory Page (word address within the page plus data, low byte before
   high byte for each address), then write the page with Write Program
   Memory Page and the page address. Without RDY/!{BSY} polling, wait at
   least t~WD_FLASH~ before the next page; accessing the interface before
   the write completes can corrupt it.
5. EEPROM, by byte: Write EEPROM Memory with address and data (the
   location is erased first automatically); without polling wait
   t~WD_EEPROM~ before the next byte. By page: load bytes with Load
   EEPROM Memory Page (2 address LSBs), then Write EEPROM Memory Page;
   only loaded locations change; without polling wait t~WD_EEPROM~ before
   the next page. In a chip-erased part, 0xFF bytes need not be written.
6. Verify any location with the read instructions (data on MISO).
7. Set !{RESET} high to start normal operation.
8. Power off if needed: !{RESET} to 1, then V~CC~ off.

```table id=mp-spi-wait caption="Minimum wait before writing the next flash or EEPROM location"
| Symbol | Min | Unit |
|---|--:|---|
| t~WD_FLASH~ | 4.5 | ms |
| t~WD_EEPROM~ | 4.0 | ms |
| t~WD_ERASE~ | 9.0 | ms |
| t~WD_FUSE~ | 4.5 | ms |
```

```table id=mp-spi-instr caption="Serial programming instruction set [1]"
| Instruction | Byte 1 | Byte 2 | Byte 3 | Byte 4 |
|---|---|---|---|---|
| Programming Enable | 0xAC | 0x53 | 0x00 | 0x00 |
| Chip Erase (program memory and EEPROM) | 0xAC | 0x80 | 0x00 | 0x00 |
| Poll RDY/!{BSY} [2] | 0xF0 | 0x00 | 0x00 | data byte out |
| Load instructions | < | < | < | < |
| Load Extended Address byte [3] | 0x4D | 0x00 | extended address | 0x00 |
| Load Program Memory Page, high byte | 0x48 | address MSB | address LSB | high data byte in |
| Load Program Memory Page, low byte | 0x40 | address MSB | address LSB | low data byte in |
| Load EEPROM Memory Page (page access) | 0xC1 | 0x00 | 0000 00aa | data byte in |
| Read instructions | < | < | < | < |
| Read Program Memory, high byte | 0x28 | address MSB | address LSB | high data byte out |
| Read Program Memory, low byte | 0x20 | address MSB | address LSB | low data byte out |
| Read EEPROM Memory | 0xA0 | 0x00 | 00aa aaaa | data byte out |
| Read Lock bits | 0x58 | 0x00 | 0x00 | data byte out |
| Read Signature Byte | 0x30 | 0x00 | 0000 00aa | data byte out |
| Read Fuse bits | 0x50 | 0x00 | 0x00 | data byte out |
| Read Fuse High bits | 0x58 | 0x08 | 0x00 | data byte out |
| Read Extended Fuse bits | 0x50 | 0x08 | 0x00 | data byte out |
| Read Calibration Byte | 0x38 | 0x00 | 0x00 | data byte out |
| Write instructions [4] | < | < | < | < |
| Write Program Memory Page | 0x4C | address MSB | address LSB | 0x00 |
| Write EEPROM Memory | 0xC0 | 0x00 | 00aa aaaa | data byte in |
| Write EEPROM Memory Page (page access) | 0xC2 | 0x00 | 00aa aa00 | 0x00 |
| Write Lock bits | 0xAC | 0xE0 | 0x00 | data byte in |
| Write Fuse bits | 0xAC | 0xA0 | 0x00 | data byte in |
| Write Fuse High bits | 0xAC | 0xA8 | 0x00 | data byte in |
| Write Extended Fuse bits | 0xAC | 0xA4 | 0x00 | data byte in |

[1] a = address bit. Fuse and lock bits: 0 = programmed, 1 = unprogrammed; write unused fuse and lock bits as 1. Program memory is addressed by word, anywhere within the page range.
[2] While the LSB of the data byte out is 1, a programming operation is pending; wait until it is 0 before the next instruction.
[3] Not all instructions apply to all parts.
[4] For the page writes, load the page buffer first (low data byte before high data byte for each address), then write the page; byte 2 and byte 3 give the page number, the buffer offset comes from the load instructions.
```

```table id=mp-spi-char caption="Serial programming characteristics" conditions="T~A~ = -40 °C to 85 °C, V~CC~ = 1.8 V to 5.5 V, unless noted"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| 1/t~CLCL~ | Oscillator frequency | V~CC~ = 1.8 V to 5.5 V | 0 | | 4 | MHz |
| t~CLCL~ | Oscillator period | V~CC~ = 1.8 V to 5.5 V | 250 | | | ns |
| 1/t~CLCL~ | Oscillator frequency | V~CC~ = 2.7 V to 5.5 V | 0 | | 10 | MHz |
| t~CLCL~ | Oscillator period | V~CC~ = 2.7 V to 5.5 V | 100 | | | ns |
| 1/t~CLCL~ | Oscillator frequency | V~CC~ = 4.5 V to 5.5 V | 0 | | 20 | MHz |
| t~CLCL~ | Oscillator period | V~CC~ = 4.5 V to 5.5 V | 50 | | | ns |
| t~SHSL~ | SCK pulse width high | | 2 × t~CLCL~ [1] | | | ns |
| t~SLSH~ | SCK pulse width low | | 2 × t~CLCL~ [1] | | | ns |
| t~OVSH~ | MOSI setup to SCK high | | t~CLCL~ | | | ns |
| t~SHOX~ | MOSI hold after SCK high | | 2 × t~CLCL~ | | | ns |
| t~SLIV~ | SCK low to MISO valid | | | | 100 | ns |

[1] 2 × t~CLCL~ for f~ck~ < 12 MHz, 3 × t~CLCL~ for f~ck~ ≥ 12 MHz.
```

```wavedrom id=mp-spi-timing caption="Serial programming timing"
{"signal": [
  {"name": "MOSI", "wave": "x.=...=....", "data": ["bit n", "bit n-1"], "node": "..D...E"},
  {"name": "SCK", "wave": "0..1.0..1.0", "node": "...A.B..C"},
  {"name": "MISO", "wave": "=.....=....", "data": ["bit n", "bit n-1"], "node": "......F"}
 ],
 "edge": ["D~>A tOVSH", "A~>E tSHOX", "A<->B tSHSL", "B<->C tSLSH", "B~>F tSLIV"],
 "config": {"hscale": 2}}
```

#### High-voltage serial programming

Programs and verifies flash, EEPROM, lock bits and fuses with 11.5 V to
12.5 V on PB5 (!{RESET}) and 4.5 V to 5.5 V on V~CC~. Each instruction
is one or more frames; in each frame SDI and SII each shift 11 bits
(a 0, eight bits MSB first, two 0s) on rising SCI edges while SDO
shifts out, as shown in the instruction table.

```table id=mp-hv-pins caption="High-voltage serial programming pins"
| Signal | Pin | I/O | Function |
|---|---|---|---|
| SDI | PB0 | I | Serial data input |
| SII | PB1 | I | Serial instruction input |
| SDO | PB2 | O | Serial data output |
| SCI | PB3 | I | Serial clock input, minimum period 220 ns |
```

Entering the mode (Prog_enable[2:0] are SDO, SII, SDI):

1. Set the Prog_enable pins to 000, !{RESET} and V~CC~ to 0 V.
2. Apply 4.5 V to 5.5 V between V~CC~ and GND; V~CC~ must reach at least
   1.8 V within 20 µs.
3. Wait 20 µs to 60 µs, then apply 11.5 V to 12.5 V to !{RESET}.
4. Hold the Prog_enable pins at least 10 µs after the high voltage is
   applied, so the Prog_enable signature is latched.
5. Release Prog_enable[2] (SDO) to avoid drive contention.
6. Wait at least 300 µs before sending instructions on SDI/SII.

If V~CC~ cannot rise that fast: in step 2 just apply V~CC~; in step 3
apply 11.5 V to 12.5 V to !{RESET} as soon as V~CC~ reaches 0.9 V to
1.1 V; steps 4 and 5 are the same; in step 6 wait until V~CC~ reaches
4.5 V to 5.5 V before sending instructions. Leave the mode by powering
down or taking !{RESET} to 0 V. Power-off: SCI to 0, !{RESET} to 1, then
V~CC~ off.

```table id=mp-hv-reset caption="High-voltage reset characteristics"
| Symbol | Parameter | Conditions | Min | Unit |
|---|---|---|--:|---|
| V~HVRST~ | !{RESET} pin high-voltage threshold | V~CC~ = 4.5 V | 11.5 | V |
| ^ | ^ | V~CC~ = 5.5 V | 11.5 | V |
| t~HVRST~ | High-voltage period for latching Prog_enable | V~CC~ = 4.5 V | 100 | ns |
| ^ | ^ | V~CC~ = 5.5 V | 100 | ns |
```

The loaded command and address are retained during programming: load a
command once for many locations, skip writing 0xFF after a chip erase
(flash, and EEPROM unless EESAVE is programmed), and load the address
high byte only for a new 256-word flash or 256-byte EEPROM window (also
for signature reads).

Procedures (instruction frames in table mp-hv-instr):

- **Chip erase** erases flash, EEPROM (kept if EESAVE is programmed) and
  the lock bits; the lock bits are reset only after the flash is fully
  erased; fuses are unchanged. Required before reprogramming flash or
  EEPROM. Load Chip Erase, wait after instr. 3 until SDO goes high, then
  load No Operation.
- **Flash**: load Write Flash; load the page buffer; load the high
  address and program the page, waiting after instr. 3 until SDO goes
  high; repeat buffer and page for every page; end with No Operation.
- **EEPROM**: load Write EEPROM; load the page buffer; program the page,
  waiting after instr. 2 until SDO goes high; repeat; end with No
  Operation. Only loaded bytes are written. There is no auto-erase in
  this mode.
- **Reading**: load Read Flash or Read EEPROM, then read; data appears on
  SDO.
- Fuses, lock bits, signature and calibration bytes: see the table.

```table id=mp-hv-instr caption="High-voltage serial programming instruction set [1]"
| Instruction | Pin | Instr. 1/5 | Instr. 2/6 | Instr. 3 | Instr. 4 | Remarks |
|---|---|---|---|---|---|---|
| Chip Erase | SDI | 0_1000_0000_00 | 0_0000_0000_00 | 0_0000_0000_00 | | Wait after instr. 3 until SDO goes high for the chip erase cycle to finish |
| ^ | SII | 0_0100_1100_00 | 0_0110_0100_00 | 0_0110_1100_00 | | ^ |
| Load "Write Flash" command | SDI | 0_0001_0000_00 | | | | Enter flash programming code |
| ^ | SII | 0_0100_1100_00 | | | | ^ |
| Load Flash Page Buffer | SDI | 0_bbbb_bbbb_00 | 0_eeee_eeee_00 | 0_dddd_dddd_00 | 0_0000_0000_00 | Repeat instr. 1–5 until the page buffer is filled or all data within the page is loaded [2] |
| ^ | SII | 0_0000_1100_00 | 0_0010_1100_00 | 0_0011_1100_00 | 0_0111_1101_00 | ^ |
| ^ | SDI (instr. 5) | 0_0000_0000_00 | | | | ^ |
| ^ | SII (instr. 5) | 0_0111_1100_00 | | | | ^ |
| Load Flash High Address and Program Page | SDI | 0_0000_000a_00 | 0_0000_0000_00 | 0_0000_0000_00 | | Wait after instr. 3 until SDO goes high. Repeat instr. 2–3 for each loaded page until the flash or all data is programmed; repeat instr. 1 for a new 256-word page [2] |
| ^ | SII | 0_0001_1100_00 | 0_0110_0100_00 | 0_0110_1100_00 | | ^ |
| Load "Read Flash" command | SDI | 0_0000_0010_00 | | | | Enter flash read mode |
| ^ | SII | 0_0100_1100_00 | | | | ^ |
| Read Flash Low and High Bytes | SDI | 0_bbbb_bbbb_00 | 0_0000_000a_00 | 0_0000_0000_00 | 0_0000_0000_00 | Repeat instr. 1 and 3–6 for each new address; repeat instr. 2 for a new 256-word page |
| ^ | SII | 0_0000_1100_00 | 0_0001_1100_00 | 0_0110_1000_00 | 0_0110_1100_00 | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | q_qqqq_qqqx_xx | ^ |
| ^ | SDI (instr. 5/6) | 0_0000_0000_00 | 0_0000_0000_00 | | | ^ |
| ^ | SII (instr. 5/6) | 0_0111_1000_00 | 0_0111_1100_00 | | | ^ |
| ^ | SDO (instr. 5/6) | x_xxxx_xxxx_xx | p_pppp_pppx_xx | | | ^ |
| Load "Write EEPROM" command | SDI | 0_0001_0001_00 | | | | Enter EEPROM programming mode |
| ^ | SII | 0_0100_1100_00 | | | | ^ |
| Load EEPROM Page Buffer | SDI | 0_00bb_bbbb_00 | 0_aaaa_aaaa_00 | 0_eeee_eeee_00 | 0_0000_0000_00 | Repeat instr. 1–5 until the page buffer is filled or all data within the page is loaded [3] |
| ^ | SII | 0_0000_1100_00 | 0_0001_1100_00 | 0_0010_1100_00 | 0_0110_1101_00 | ^ |
| ^ | SDI (instr. 5) | 0_0000_0000_00 | | | | ^ |
| ^ | SII (instr. 5) | 0_0110_1100_00 | | | | ^ |
| Program EEPROM Page | SDI | 0_0000_0000_00 | 0_0000_0000_00 | | | Wait after instr. 2 until SDO goes high. Repeat instr. 1–2 for each loaded page until the EEPROM or all data is programmed |
| ^ | SII | 0_0110_0100_00 | 0_0110_1100_00 | | | ^ |
| Write EEPROM Byte | SDI | 0_bbbb_bbbb_00 | 0_aaaa_aaaa_00 | 0_eeee_eeee_00 | 0_0000_0000_00 | Repeat instr. 1–6 for each new address. Wait after instr. 6 until SDO goes high [4] |
| ^ | SII | 0_0000_1100_00 | 0_0001_1100_00 | 0_0010_1100_00 | 0_0110_1101_00 | ^ |
| ^ | SDI (instr. 5/6) | 0_0000_0000_00 | 0_0000_0000_00 | | | ^ |
| ^ | SII (instr. 5/6) | 0_0110_0100_00 | 0_0110_1100_00 | | | ^ |
| Load "Read EEPROM" command | SDI | 0_0000_0011_00 | | | | Enter EEPROM read mode |
| ^ | SII | 0_0100_1100_00 | | | | ^ |
| Read EEPROM Byte | SDI | 0_bbbb_bbbb_00 | 0_aaaa_aaaa_00 | 0_0000_0000_00 | 0_0000_0000_00 | Repeat instr. 1 and 3–4 for each new address; repeat instr. 2 for a new 256-byte page |
| ^ | SII | 0_0000_1100_00 | 0_0001_1100_00 | 0_0110_1000_00 | 0_0110_1100_00 | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | q_qqqq_qqq0_00 | ^ |
| Write Fuse Low Bits | SDI | 0_0100_0000_00 | 0_A987_6543_00 | 0_0000_0000_00 | 0_0000_0000_00 | Wait after instr. 4 until SDO goes high. Write A–3 = 0 to program the fuse bit |
| ^ | SII | 0_0100_1100_00 | 0_0010_1100_00 | 0_0110_0100_00 | 0_0110_1100_00 | ^ |
| Write Fuse High Bits | SDI | 0_0100_0000_00 | 0_IHGF_EDCB_00 | 0_0000_0000_00 | 0_0000_0000_00 | Wait after instr. 4 until SDO goes high. Write I–B = 0 to program the fuse bit |
| ^ | SII | 0_0100_1100_00 | 0_0010_1100_00 | 0_0111_0100_00 | 0_0111_1100_00 | ^ |
| Write Fuse Extended Bits | SDI | 0_0100_0000_00 | 0_0000_000J_00 | 0_0000_0000_00 | 0_0000_0000_00 | Wait after instr. 4 until SDO goes high. Write J = 0 to program the fuse bit |
| ^ | SII | 0_0100_1100_00 | 0_0010_1100_00 | 0_0110_0110_00 | 0_0110_1110_00 | ^ |
| Write Lock Bits | SDI | 0_0010_0000_00 | 0_0000_0021_00 | 0_0000_0000_00 | 0_0000_0000_00 | Wait after instr. 4 until SDO goes high. Write 2–1 = 0 to program the lock bit |
| ^ | SII | 0_0100_1100_00 | 0_0010_1100_00 | 0_0110_0100_00 | 0_0110_1100_00 | ^ |
| Read Fuse Low Bits | SDI | 0_0000_0100_00 | 0_0000_0000_00 | 0_0000_0000_00 | | Reading A–3 = 0 means the fuse bit is programmed |
| ^ | SII | 0_0100_1100_00 | 0_0110_1000_00 | 0_0110_1100_00 | | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | A_9876_543x_xx | | ^ |
| Read Fuse High Bits | SDI | 0_0000_0100_00 | 0_0000_0000_00 | 0_0000_0000_00 | | Reading I–B = 0 means the fuse bit is programmed |
| ^ | SII | 0_0100_1100_00 | 0_0111_1010_00 | 0_0111_1110_00 | | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | I_HGFE_DCBx_xx | | ^ |
| Read Fuse Extended Bits | SDI | 0_0000_0100_00 | 0_0000_0000_00 | 0_0000_0000_00 | | Reading J = 0 means the fuse bit is programmed |
| ^ | SII | 0_0100_1100_00 | 0_0110_1010_00 | 0_0110_1110_00 | | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | x_xxxx_xxJx_xx | | ^ |
| Read Lock Bits | SDI | 0_0000_0100_00 | 0_0000_0000_00 | 0_0000_0000_00 | | Reading 2, 1 = 0 means the lock bit is programmed |
| ^ | SII | 0_0100_1100_00 | 0_0111_1000_00 | 0_0111_1100_00 | | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | x_xxxx_x21x_xx | | ^ |
| Read Signature Bytes | SDI | 0_0000_1000_00 | 0_0000_00bb_00 | 0_0000_0000_00 | 0_0000_0000_00 | Repeat instr. 2–4 for each signature byte address |
| ^ | SII | 0_0100_1100_00 | 0_0000_1100_00 | 0_0110_1000_00 | 0_0110_1100_00 | ^ |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | q_qqqq_qqqx_xx | ^ |
| Read Calibration Byte | SDI | 0_0000_1000_00 | 0_0000_0000_00 | 0_0000_0000_00 | 0_0000_0000_00 | |
| ^ | SII | 0_0100_1100_00 | 0_0000_1100_00 | 0_0111_1000_00 | 0_0111_1100_00 | |
| ^ | SDO | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | x_xxxx_xxxx_xx | p_pppp_pppx_xx | |
| Load "No Operation" command | SDI | 0_0000_0000_00 | | | | |
| ^ | SII | 0_0100_1100_00 | | | | |

[1] SDO is x_xxxx_xxxx_xx (don't care) in every frame not listed. a = address high bits, b = address low bits, d = data in high bits, e = data in low bits, p = data out high bits, q = data out low bits, x = don't care, 1 = LB1, 2 = LB2, 3 = CKSEL0, 4 = CKSEL1, 5 = CKSEL2, 6 = CKSEL3, 7 = SUT0, 8 = SUT1, 9 = CKOUT, A = CKDIV8, B = BODLEVEL0, C = BODLEVEL1, D = BODLEVEL2, E = EESAVE, F = WDTON, G = SPIEN, H = DWEN, I = RSTDISBL, J = SELFPRGEN.
[2] For page sizes below 256 words, part of the address bbbb_bbbb is part of the page address.
[3] For page sizes below 256 bytes, part of the address bbbb_bbbb is part of the page address.
[4] The EEPROM is written by page, but only bytes loaded into the page are written; page access is more efficient for several bytes in one page. No EEPROM auto-erase in high-voltage serial programming, only in SPI programming.
```

```table id=mp-hv-char caption="High-voltage serial programming characteristics" conditions="T~A~ = 25 °C ± 10 %, V~CC~ = 5.0 V ± 10 %, unless noted"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| t~SHSL~ | SCI (PB3) pulse width high | 125 | | | ns |
| t~SLSH~ | SCI (PB3) pulse width low | 125 | | | ns |
| t~IVSH~ | SDI (PB0), SII (PB1) valid to SCI (PB3) high | 50 | | | ns |
| t~SHIX~ | SDI (PB0), SII (PB1) hold after SCI (PB3) high | 50 | | | ns |
| t~SHOV~ | SCI (PB3) high to SDO (PB2) valid | | 16 | | ns |
| t~WLWH_PFB~ | Wait after instr. 3 for write fuse bits | | 2.5 | | ms |
```

```wavedrom id=mp-hv-timing caption="High-voltage serial programming timing"
{"signal": [
  {"name": "SDI, SII", "wave": "x.=...=....", "data": ["bit n", "bit n-1"], "node": "..D...E"},
  {"name": "SCI", "wave": "0..1.0..1.0", "node": "...A.B..C"},
  {"name": "SDO", "wave": "=...=....=.", "data": ["bit n", "bit n-1", "bit n-2"], "node": "....F"}
 ],
 "edge": ["D~>A tIVSH", "A~>E tSHIX", "A<->B tSHSL", "B<->C tSLSH", "A~>F tSHOV"],
 "config": {"hscale": 2}}
```

## Registers and commands

I/O address (for IN, OUT, SBI, CBI) and data-space address (for LD, ST,
LDS, STS; I/O address + 0x20). Each register's bits are described under
its peripheral; SREG, SPH and SPL under [programming model](#programming-model).
Addresses not listed (0x00–0x02, 0x09–0x0C, 0x19–0x1B, 0x36, 0x3C) are
reserved and must never be written; reserved bits (–) read as zero and
are written zero. 0x00–0x1F are bit-accessible (SBI, CBI, SBIS, SBIC).

```table id=regsum caption="Register summary"
| I/O | Data | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|---|---|
| 0x3F | 0x5F | SREG | I | T | H | S | V | N | Z | C |
| 0x3E | 0x5E | SPH | – | – | – | – | – | – | SP9 | SP8 |
| 0x3D | 0x5D | SPL | SP7 | SP6 | SP5 | SP4 | SP3 | SP2 | SP1 | SP0 |
| 0x3B | 0x5B | GIMSK | – | INT0 | PCIE | – | – | – | – | – |
| 0x3A | 0x5A | GIFR | – | INTF0 | PCIF | – | – | – | – | – |
| 0x39 | 0x59 | TIMSK | – | OCIE1A | OCIE1B | OCIE0A | OCIE0B | TOIE1 | TOIE0 | – |
| 0x38 | 0x58 | TIFR | – | OCF1A | OCF1B | OCF0A | OCF0B | TOV1 | TOV0 | – |
| 0x37 | 0x57 | SPMCSR | – | – | RSIG | CTPB | RFLB | PGWRT | PGERS | SPMEN |
| 0x35 | 0x55 | MCUCR | BODS | PUD | SE | SM1 | SM0 | BODSE | ISC01 | ISC00 |
| 0x34 | 0x54 | MCUSR | – | – | – | – | WDRF | BORF | EXTRF | PORF |
| 0x33 | 0x53 | TCCR0B | FOC0A | FOC0B | – | – | WGM02 | CS02 | CS01 | CS00 |
| 0x32 | 0x52 | TCNT0 | Timer/Counter0 | < | < | < | < | < | < | < |
| 0x31 | 0x51 | OSCCAL | Oscillator calibration register | < | < | < | < | < | < | < |
| 0x30 | 0x50 | TCCR1 | CTC1 | PWM1A | COM1A1 | COM1A0 | CS13 | CS12 | CS11 | CS10 |
| 0x2F | 0x4F | TCNT1 | Timer/Counter1 | < | < | < | < | < | < | < |
| 0x2E | 0x4E | OCR1A | Timer/Counter1 output compare register A | < | < | < | < | < | < | < |
| 0x2D | 0x4D | OCR1C | Timer/Counter1 output compare register C | < | < | < | < | < | < | < |
| 0x2C | 0x4C | GTCCR | TSM | PWM1B | COM1B1 | COM1B0 | FOC1B | FOC1A | PSR1 | PSR0 |
| 0x2B | 0x4B | OCR1B | Timer/Counter1 output compare register B | < | < | < | < | < | < | < |
| 0x2A | 0x4A | TCCR0A | COM0A1 | COM0A0 | COM0B1 | COM0B0 | – | – | WGM01 | WGM00 |
| 0x29 | 0x49 | OCR0A | Timer/Counter0 output compare register A | < | < | < | < | < | < | < |
| 0x28 | 0x48 | OCR0B | Timer/Counter0 output compare register B | < | < | < | < | < | < | < |
| 0x27 | 0x47 | PLLCSR | LSM | – | – | – | – | PCKE | PLLE | PLOCK |
| 0x26 | 0x46 | CLKPR | CLKPCE | – | – | – | CLKPS3 | CLKPS2 | CLKPS1 | CLKPS0 |
| 0x25 | 0x45 | DT1A | DT1AH3 | DT1AH2 | DT1AH1 | DT1AH0 | DT1AL3 | DT1AL2 | DT1AL1 | DT1AL0 |
| 0x24 | 0x44 | DT1B | DT1BH3 | DT1BH2 | DT1BH1 | DT1BH0 | DT1BL3 | DT1BL2 | DT1BL1 | DT1BL0 |
| 0x23 | 0x43 | DTPS1 | – | – | – | – | – | – | DTPS11 | DTPS10 |
| 0x22 | 0x42 | DWDR | DWDR[7:0] | < | < | < | < | < | < | < |
| 0x21 | 0x41 | WDTCR | WDIF | WDIE | WDP3 | WDCE | WDE | WDP2 | WDP1 | WDP0 |
| 0x20 | 0x40 | PRR | – | – | – | – | PRTIM1 | PRTIM0 | PRUSI | PRADC |
| 0x1F | 0x3F | EEARH | – | – | – | – | – | – | – | EEAR8 |
| 0x1E | 0x3E | EEARL | EEAR7 | EEAR6 | EEAR5 | EEAR4 | EEAR3 | EEAR2 | EEAR1 | EEAR0 |
| 0x1D | 0x3D | EEDR | EEPROM data register | < | < | < | < | < | < | < |
| 0x1C | 0x3C | EECR | – | – | EEPM1 | EEPM0 | EERIE | EEMPE | EEPE | EERE |
| 0x18 | 0x38 | PORTB | – | – | PORTB5 | PORTB4 | PORTB3 | PORTB2 | PORTB1 | PORTB0 |
| 0x17 | 0x37 | DDRB | – | – | DDB5 | DDB4 | DDB3 | DDB2 | DDB1 | DDB0 |
| 0x16 | 0x36 | PINB | – | – | PINB5 | PINB4 | PINB3 | PINB2 | PINB1 | PINB0 |
| 0x15 | 0x35 | PCMSK | – | – | PCINT5 | PCINT4 | PCINT3 | PCINT2 | PCINT1 | PCINT0 |
| 0x14 | 0x34 | DIDR0 | – | – | ADC0D | ADC2D | ADC3D | ADC1D | AIN1D | AIN0D |
| 0x13 | 0x33 | GPIOR2 | General purpose I/O register 2 | < | < | < | < | < | < | < |
| 0x12 | 0x32 | GPIOR1 | General purpose I/O register 1 | < | < | < | < | < | < | < |
| 0x11 | 0x31 | GPIOR0 | General purpose I/O register 0 | < | < | < | < | < | < | < |
| 0x10 | 0x30 | USIBR | USI buffer register | < | < | < | < | < | < | < |
| 0x0F | 0x2F | USIDR | USI data register | < | < | < | < | < | < | < |
| 0x0E | 0x2E | USISR | USISIF | USIOIF | USIPF | USIDC | USICNT3 | USICNT2 | USICNT1 | USICNT0 |
| 0x0D | 0x2D | USICR | USISIE | USIOIE | USIWM1 | USIWM0 | USICS1 | USICS0 | USICLK | USITC |
| 0x08 | 0x28 | ACSR | ACD | ACBG | ACO | ACI | ACIE | – | ACIS1 | ACIS0 |
| 0x07 | 0x27 | ADMUX | REFS1 | REFS0 | ADLAR | REFS2 | MUX3 | MUX2 | MUX1 | MUX0 |
| 0x06 | 0x26 | ADCSRA | ADEN | ADSC | ADATE | ADIF | ADIE | ADPS2 | ADPS1 | ADPS0 |
| 0x05 | 0x25 | ADCH | ADC data register high byte | < | < | < | < | < | < | < |
| 0x04 | 0x24 | ADCL | ADC data register low byte | < | < | < | < | < | < | < |
| 0x03 | 0x23 | ADCSRB | BIN | ACME | IPR | – | – | ADTS2 | ADTS1 | ADTS0 |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]"
| Parameter | Conditions | Min | Max | Unit |
|---|---|--:|--:|---|
| Operating temperature | | -55 | 125 | °C |
| Storage temperature | | -65 | 150 | °C |
| Voltage on any pin except !{RESET} | With respect to ground | -0.5 | V~CC~ + 0.5 | V |
| Voltage on !{RESET} | With respect to ground | -0.5 | 13.0 | V |
| Maximum operating voltage | | | 6.0 | V |
| DC current per I/O pin | | | 40.0 | mA |
| DC current, V~CC~ and GND pins | | | 200.0 | mA |

[1] Stresses above these may cause permanent damage; operation at these or any other conditions beyond the operating sections is not implied. Long exposure to absolute maximum conditions may affect reliability.
```

## Recommended operating conditions

```table id=roc caption="Operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | ATtiny25V/45V/85V | 1.8 | 5.5 | V |
| ^ | ^ | ATtiny25/45/85 | 2.7 | 5.5 | V |
| T~A~ | Operating temperature | | -40 | 85 | °C |
| f~max~ | Maximum clock frequency (safe operating area) [2] | ATtiny25V/45V/85V, V~CC~ = 1.8 V | | 4 | MHz |
| ^ | ^ | ATtiny25V/45V/85V, V~CC~ = 2.7 V to 5.5 V | | 10 | MHz |
| ^ | ^ | ATtiny25/45/85, V~CC~ = 2.7 V | | 10 | MHz |
| ^ | ^ | ATtiny25/45/85, V~CC~ = 4.5 V to 5.5 V | | 20 | MHz |

[1] The source gives no operating-conditions table; the temperature range is the one the DC characteristics are specified over, the supply and frequency limits are from the maximum-frequency figures (21-1, 21-2).
[2] The limit rises linearly with V~CC~ between 1.8 V and 2.7 V (4 to 10 MHz) and between 2.7 V and 4.5 V (10 to 20 MHz).
```

## Electrical characteristics

```table id=dc caption="DC characteristics" conditions="T~A~ = -40 °C to 85 °C; typical values at 25 °C [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~IL~ | Input low voltage, except XTAL1 and !{RESET} pins | V~CC~ = 1.8 V to 2.4 V | -0.5 | | 0.2 × V~CC~ [3] | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | -0.5 | | 0.3 × V~CC~ [3] | V |
| V~IH~ | Input high voltage, except XTAL1 and !{RESET} pins | V~CC~ = 1.8 V to 2.4 V | 0.7 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | 0.6 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| V~IL1~ | Input low voltage, XTAL1 pin, external clock selected | V~CC~ = 1.8 V to 5.5 V | -0.5 | | 0.1 × V~CC~ [3] | V |
| V~IH1~ | Input high voltage, XTAL1 pin, external clock selected | V~CC~ = 1.8 V to 2.4 V | 0.8 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | 0.7 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| V~IL2~ | Input low voltage, !{RESET} pin | V~CC~ = 1.8 V to 5.5 V | -0.5 | | 0.2 × V~CC~ [3] | V |
| V~IH2~ | Input high voltage, !{RESET} pin | V~CC~ = 1.8 V to 5.5 V | 0.9 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| V~IL3~ | Input low voltage, !{RESET} pin as I/O | V~CC~ = 1.8 V to 2.4 V | -0.5 | | 0.2 × V~CC~ [3] | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | -0.5 | | 0.3 × V~CC~ [3] | V |
| V~IH3~ | Input high voltage, !{RESET} pin as I/O | V~CC~ = 1.8 V to 2.4 V | 0.7 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | 0.6 × V~CC~ [2] | | V~CC~ + 0.5 | V |
| V~OL~ | Output low voltage, port B except !{RESET} [4][6] | I~OL~ = 10 mA, V~CC~ = 5 V | | | 0.6 | V |
| ^ | ^ | I~OL~ = 5 mA, V~CC~ = 3 V | | | 0.5 | V |
| V~OH~ | Output high voltage, port B except !{RESET} [5][6] | I~OH~ = -10 mA, V~CC~ = 5 V | 4.3 | | | V |
| ^ | ^ | I~OH~ = -5 mA, V~CC~ = 3 V | 2.5 | | | V |
| I~IL~ | Input leakage current, I/O pin (absolute value) [9] | V~CC~ = 5.5 V, pin low | | | 1 | µA |
| I~IH~ | Input leakage current, I/O pin (absolute value) [9] | V~CC~ = 5.5 V, pin high | | | 1 | µA |
| R~RST~ | Reset pull-up resistor | V~CC~ = 5.5 V, input low | 30 | | 60 | kΩ |
| R~pu~ | I/O pin pull-up resistor | V~CC~ = 5.5 V, input low | 20 | | 50 | kΩ |
| I~CC~ | Power supply current [7] | Active, 1 MHz, V~CC~ = 2 V | | 0.3 | 0.55 | mA |
| ^ | ^ | Active, 4 MHz, V~CC~ = 3 V | | 1.5 | 2.5 | mA |
| ^ | ^ | Active, 8 MHz, V~CC~ = 5 V | | 5 | 8 | mA |
| ^ | ^ | Idle, 1 MHz, V~CC~ = 2 V | | 0.1 | 0.2 | mA |
| ^ | ^ | Idle, 4 MHz, V~CC~ = 3 V | | 0.35 | 0.6 | mA |
| ^ | ^ | Idle, 8 MHz, V~CC~ = 5 V | | 1.2 | 2 | mA |
| ^ | Power-down supply current [8] | WDT enabled, V~CC~ = 3 V | | | 10 | µA |
| ^ | ^ | WDT disabled, V~CC~ = 3 V | | | 2 | µA |

[1] Typical values at 25 °C.
[2] The lowest value at which the pin is guaranteed to be read as high.
[3] The highest value at which the pin is guaranteed to be read as low.
[4] A pin can sink more than the test current in steady state, but the sum of all I~OL~ must not exceed 60 mA; above the test current V~OL~ may exceed the limit, and sinking more than the test current is not guaranteed.
[5] A pin can source more than the test current in steady state, but the sum of all I~OH~ must not exceed 60 mA; above the test current V~OH~ may exceed the limit, and sourcing more than the test current is not guaranteed.
[6] !{RESET} must tolerate high voltage in programming modes and so has a weak driver compared with the other I/O pins (source figures 22-23 to 22-26).
[7] External clock, with the power-saving methods of the power management section; PRR = 0xFF and no I/O drive.
[8] Brown-out detection disabled.
[9] Typically below 0.05 µA.
```

```table id=reset-chars caption="Reset, brown-out and internal voltage characteristics [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~RST~ | !{RESET} pin threshold voltage | V~CC~ = 3 V | 0.2 × V~CC~ | | 0.9 × V~CC~ | V |
| t~RST~ | Minimum pulse width on !{RESET} pin | V~CC~ = 3 V | | | 2.5 | µs |
| V~HYST~ | Brown-out detector hysteresis | | | 50 | | mV |
| t~BOD~ | Minimum pulse width on brown-out reset | | | 2 | | µs |
| V~BG~ | Bandgap reference voltage | V~CC~ = 5.5 V, T~A~ = 25 °C | 1.0 | 1.1 | 1.2 | V |
| t~BG~ | Bandgap reference start-up time | V~CC~ = 2.7 V, T~A~ = 25 °C | | 40 | 70 | µs |
| I~BG~ | Bandgap reference current consumption | V~CC~ = 2.7 V, T~A~ = 25 °C | | 15 | | µA |

[1] Values are guidelines only.
```

Two power-on reset circuits exist. The standard one is in ATtiny25
revision D and older, ATtiny45 revision F and older and ATtiny85
revision B; the enhanced one in ATtiny25 revision E and newer, ATtiny45
revision G and newer and ATtiny85 revision C and newer. The revision is
marked on the package (8P3 and 8S2: bottom; 20M1: top).

```table id=por caption="Power-on reset characteristics [1]" conditions="T~A~ = -40 °C to 85 °C"
| Symbol | Parameter | Standard POR | < | < | Enhanced POR | < | < | Unit |
| ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~POR~ | Release threshold of power-on reset (rising V~CC~) | 0.7 | 1.0 | 1.4 | 1.1 | 1.4 | 1.6 | V |
| V~POA~ | Activation threshold of power-on reset [2] | 0.05 | 0.9 | 1.3 | 0.6 | 1.3 | 1.6 | V |
| SR~ON~ | Power-on slope rate | 0.01 | | 4.5 | 0.01 | | | V/ms |

[1] Values are guidelines only.
[2] Power-on reset does not work unless the supply voltage has been below V~POA~ (falling).
```

```table id=bod caption="Brown-out detection: BODLEVEL fuse coding [1]" conditions="T~A~ = -40 °C to 85 °C"
| BODLEVEL[2:0] | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| 111 | BOD disabled | | | | |
| 110 | V~BOT~ | 1.7 | 1.8 | 2.0 | V |
| 101 | V~BOT~ | 2.5 | 2.7 | 2.9 | V |
| 100 | V~BOT~ | 4.1 | 4.3 | 4.5 | V |
| 0XX | Reserved | | | | |

[1] V~BOT~ may be below the nominal minimum operating voltage for some devices; these are tested down to V~CC~ = V~BOT~ in production, so a brown-out reset is guaranteed before V~CC~ falls to where correct operation is no longer guaranteed.
```

## Switching characteristics

### Calibrated internal RC oscillator

The oscillator can be calibrated by the user more accurately than the
factory calibration; its frequency depends on temperature and voltage
(see [typical characteristics](#typical-characteristics)).

```table id=rc-accuracy caption="Calibration accuracy of the internal RC oscillator [1]"
| Calibration | Target frequency | V~CC~ | Temperature | Value | Unit |
|---|---|---|---|--:|---|
| Factory | 8.0 MHz [2] | 3 V | 25 °C | ±10 | % |
| User | Any fixed frequency within 6 MHz to 8 MHz | Any fixed voltage within 1.8 V to 5.5 V [3] or 2.7 V to 5.5 V [4] | Any fixed temperature within -40 °C to 85 °C | ±1 | % |

[1] Accuracy of the oscillator frequency at the calibration point (fixed temperature and voltage).
[2] ATtiny25/V only: 6.4 MHz in ATtiny15 compatibility mode.
[3] ATtiny25V/45V/85V.
[4] ATtiny25/45/85.
```

### External clock drive

```table id=ext-clock caption="External clock drive (XTAL1), levels V~IL1~ and V~IH1~"
| Symbol | Parameter | V~CC~ = 1.8 V to 5.5 V | < | V~CC~ = 2.7 V to 5.5 V | < | V~CC~ = 4.5 V to 5.5 V | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| 1/t~CLCL~ | Clock frequency | 0 | 4 | 0 | 10 | 0 | 20 | MHz |
| t~CLCL~ | Clock period | 250 | | 100 | | 50 | | ns |
| t~CHCX~ | High time | 100 | | 40 | | 20 | | ns |
| t~CLCX~ | Low time | 100 | | 40 | | 20 | | ns |
| t~CLCH~ | Rise time | | 2.0 | | 1.6 | | 0.5 | µs |
| t~CHCL~ | Fall time | | 2.0 | | 1.6 | | 0.5 | µs |
| Δt~CLCL~ | Change in period from one clock cycle to the next | | 2 | | 2 | | 2 | % |
```

## Timing diagrams

```wavedrom id=ext-clock-wave caption="External clock drive waveform"
{"signal": [
  {"name": "XTAL1", "wave": "0.1..0..1..0", "node": "..A..B..C"}
 ],
 "edge": ["A<->B tCHCX", "B<->C tCLCX", "A<->C tCLCL"],
 "config": {"hscale": 2}}
```

## Typical characteristics

Largely from simulation and characterization of similar devices; not
tested in production. Current measurements: all I/O pins inputs with
pull-ups enabled, rail-to-rail sine wave clock. Power-down current does
not depend on the clock source; the difference between power-down with
and without the watchdog is the watchdog's current. Parts are
characterized above their test limits but only guaranteed at the
frequency of their ordering code. A pin driving a capacitive load draws
about C~L~ × V~CC~ × f (f = average switching frequency of the pin).

```table id=prr-current caption="Additional supply current of I/O modules, typical (enabled in PRR)"
| PRR bit | V~CC~ = 2 V, f = 1 MHz | V~CC~ = 3 V, f = 4 MHz | V~CC~ = 5 V, f = 8 MHz | Active mode, external clock | Idle mode, external clock |
|---|--:|--:|--:|--:|--:|
| PRTIM1 | 45 µA | 300 µA | 1100 µA | +20 % | +80 % |
| PRTIM0 | 5 µA | 30 µA | 110 µA | +2 % | +10 % |
| PRUSI | 5 µA | 25 µA | 100 µA | +2 % | +10 % |
| PRADC | 15 µA | 85 µA | 340 µA | +5 % | +25 % |
```

The percentages scale the active or idle current read from the curves
to other V~CC~ and frequencies. Example: idle at 2 V, 1 MHz is about
0.18 mA; with USI, Timer/Counter0 and ADC enabled:

```equation
I~CC~ = 0.18 mA × (1 + 0.1 + 0.25 + 0.1) ≈ 0.261 mA
```

```curve id=icc-active caption="Active supply current vs frequency, 1 MHz to 20 MHz"
{"x": {"label": "f", "unit": "MHz", "range": [0, 20]},
 "y": {"label": "ICC", "unit": "mA", "range": [0, 14]},
 "series": [
   {"label": "VCC = 5.5 V", "points": [[1, 1.2], [4, 3.7], [12, 8.5], [20, 13.2]]},
   {"label": "VCC = 5.0 V", "points": [[1, 1.0], [4, 3.3], [12, 7.4], [20, 11.5]]},
   {"label": "VCC = 4.5 V", "points": [[1, 0.9], [4, 2.9], [12, 6.5], [20, 10.0]]},
   {"label": "VCC = 4.0 V", "points": [[1, 0.8], [4, 2.5], [10, 5.0], [16.7, 7.0]]},
   {"label": "VCC = 3.3 V", "points": [[1, 0.65], [4, 2.0], [8, 3.2], [12.2, 4.4]]},
   {"label": "VCC = 2.7 V", "points": [[1, 0.5], [4, 1.5], [7, 2.1], [10, 2.65]]},
   {"label": "VCC = 1.8 V", "points": [[1, 0.3], [2, 0.5], [3, 0.7], [4, 0.85]]}
 ]}
```

```curve id=icc-idle caption="Idle supply current vs frequency, 1 MHz to 20 MHz"
{"x": {"label": "f", "unit": "MHz", "range": [0, 20]},
 "y": {"label": "ICC", "unit": "mA", "range": [0, 4]},
 "series": [
   {"label": "VCC = 5.5 V", "points": [[1, 0.25], [10, 1.75], [20, 3.45]]},
   {"label": "VCC = 5.0 V", "points": [[1, 0.22], [10, 1.52], [20, 3.0]]},
   {"label": "VCC = 4.5 V", "points": [[1, 0.19], [10, 1.32], [20, 2.6]]},
   {"label": "VCC = 4.0 V", "points": [[1, 0.16], [10, 1.12], [16.5, 1.8]]},
   {"label": "VCC = 3.3 V", "points": [[1, 0.13], [6, 0.6], [12.2, 1.23]]},
   {"label": "VCC = 2.7 V", "points": [[1, 0.1], [5, 0.4], [10, 0.76]]},
   {"label": "VCC = 1.8 V", "points": [[1, 0.07], [2.5, 0.13], [4, 0.2]]}
 ]}
```

```curve id=icc-pd caption="Power-down supply current vs V~CC~, watchdog timer disabled"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "ICC", "unit": "µA", "range": [0, 1.4]},
 "series": [
   {"label": "85 °C", "points": [[1.8, 0.31], [2.5, 0.38], [3, 0.45], [3.5, 0.53], [4, 0.64], [4.5, 0.78], [5, 0.97], [5.5, 1.23]]},
   {"label": "-40 °C", "points": [[1.8, 0.04], [2.5, 0.12], [3, 0.19], [3.5, 0.27], [4, 0.37], [4.5, 0.48], [5, 0.6], [5.5, 0.76]]},
   {"label": "25 °C", "points": [[1.8, 0.03], [2.5, 0.11], [3, 0.17], [3.5, 0.23], [4, 0.3], [4.5, 0.39], [5, 0.49], [5.5, 0.61]]}
 ]}
```

```curve id=icc-pd-wdt caption="Power-down supply current vs V~CC~, watchdog timer enabled"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "ICC", "unit": "µA", "range": [0, 14]},
 "series": [
   {"label": "-40 °C", "points": [[1.8, 4.2], [2.5, 4.6], [3, 5.0], [3.5, 5.4], [4, 5.9], [4.5, 6.5], [5, 7.3], [5.5, 8.4]]},
   {"label": "25 °C", "points": [[1.8, 3.4], [2.5, 3.7], [3, 4.0], [3.5, 4.4], [4, 4.9], [4.5, 5.6], [5, 6.4], [5.5, 7.5]]},
   {"label": "85 °C", "points": [[1.8, 2.9], [2.5, 3.2], [3, 3.6], [3.5, 4.1], [4, 4.7], [4.5, 5.4], [5, 6.3], [5.5, 7.4]]}
 ]}
```

```curve id=vol-3v caption="I/O pin output voltage vs sink current, V~CC~ = 3 V"
{"x": {"label": "IOL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "VOL", "unit": "V", "range": [0, 1.2]},
 "series": [
   {"label": "85 °C", "points": [[0, 0], [5, 0.2], [10, 0.4], [15, 0.6], [20, 0.82]]},
   {"label": "25 °C", "points": [[0, 0], [5, 0.15], [10, 0.31], [15, 0.47], [20, 0.63]]},
   {"label": "-40 °C", "points": [[0, 0], [5, 0.12], [10, 0.25], [15, 0.38], [20, 0.51]]}
 ]}
```

```curve id=vol-5v caption="I/O pin output voltage vs sink current, V~CC~ = 5 V"
{"x": {"label": "IOL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "VOL", "unit": "V", "range": [0, 0.6]},
 "series": [
   {"label": "85 °C", "points": [[0, 0], [5, 0.13], [10, 0.26], [15, 0.39], [20, 0.53]]},
   {"label": "25 °C", "points": [[0, 0], [5, 0.11], [10, 0.22], [15, 0.33], [20, 0.44]]},
   {"label": "-40 °C", "points": [[0, 0], [5, 0.095], [10, 0.19], [15, 0.285], [20, 0.38]]}
 ]}
```

```curve id=voh-3v caption="I/O pin output voltage vs source current, V~CC~ = 3 V"
{"x": {"label": "IOH", "unit": "mA", "range": [0, 25]},
 "y": {"label": "VOH", "unit": "V", "range": [0, 3.5]},
 "series": [
   {"label": "-40 °C", "points": [[0, 3.0], [5, 2.86], [10, 2.72], [15, 2.56], [20, 2.37]]},
   {"label": "25 °C", "points": [[0, 3.0], [5, 2.85], [10, 2.68], [15, 2.48], [20, 2.25]]},
   {"label": "85 °C", "points": [[0, 3.0], [5, 2.83], [10, 2.64], [15, 2.4], [20, 2.07]]}
 ]}
```

```curve id=voh-5v caption="I/O pin output voltage vs source current, V~CC~ = 5 V"
{"x": {"label": "IOH", "unit": "mA", "range": [0, 25]},
 "y": {"label": "VOH", "unit": "V", "range": [4.4, 5.1]},
 "series": [
   {"label": "-40 °C", "points": [[0, 5.0], [5, 4.89], [10, 4.78], [15, 4.67], [20, 4.56]]},
   {"label": "25 °C", "points": [[0, 5.0], [5, 4.88], [10, 4.76], [15, 4.63], [20, 4.51]]},
   {"label": "85 °C", "points": [[0, 5.0], [5, 4.86], [10, 4.72], [15, 4.58], [20, 4.43]]}
 ]}
```

```curve id=rc8-vcc caption="Calibrated 8 MHz RC oscillator frequency vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "fRC", "unit": "MHz", "range": [7.5, 8.2]},
 "series": [
   {"label": "85 °C", "points": [[1.8, 8.06], [2, 8.1], [2.5, 8.11], [3, 8.115], [4, 8.11], [5, 8.105], [5.5, 8.105]]},
   {"label": "25 °C", "points": [[1.8, 7.91], [2, 7.955], [2.5, 7.96], [3, 7.965], [4, 7.95], [5, 7.935], [5.5, 7.92]]},
   {"label": "-40 °C", "points": [[1.8, 7.75], [1.9, 7.81], [2, 7.82], [2.5, 7.81], [3, 7.795], [4, 7.765], [5, 7.73], [5.5, 7.705]]}
 ]}
```

```curve id=rc8-temp caption="Calibrated 8 MHz RC oscillator frequency vs temperature"
{"x": {"label": "T", "unit": "°C", "range": [-60, 100]},
 "y": {"label": "fRC", "unit": "MHz", "range": [7.7, 8.15]},
 "series": [
   {"label": "VCC = 3.0 V", "points": [[-40, 7.795], [0, 7.898], [40, 8.0], [85, 8.115]]},
   {"label": "VCC = 5.0 V", "points": [[-40, 7.73], [0, 7.85], [40, 7.97], [85, 8.105]]}
 ]}
```

```curve id=bod-temp caption="BOD threshold vs temperature, BODLEVEL = 2.7 V"
{"x": {"label": "T", "unit": "°C", "range": [-50, 100]},
 "y": {"label": "VBOT", "unit": "V", "range": [2.68, 2.8]},
 "series": [
   {"label": "Rising VCC", "points": [[-40, 2.765], [-20, 2.772], [0, 2.779], [20, 2.784], [40, 2.786], [60, 2.785], [85, 2.783]]},
   {"label": "Falling VCC", "points": [[-40, 2.69], [-20, 2.696], [0, 2.702], [20, 2.707], [40, 2.709], [60, 2.709], [85, 2.708]]}
 ]}
```

```curve id=bandgap-temp caption="Bandgap voltage vs temperature"
{"x": {"label": "T", "unit": "°C", "range": [-40, 100]},
 "y": {"label": "VBG", "unit": "V", "range": [1.0, 1.2]},
 "series": [
   {"label": "VCC = 1.8 V", "points": [[-40, 1.111], [-20, 1.11], [0, 1.113], [20, 1.115], [40, 1.117], [60, 1.118], [85, 1.119]]},
   {"label": "VCC = 3 V", "points": [[-40, 1.093], [-20, 1.1], [0, 1.104], [20, 1.108], [40, 1.112], [60, 1.114], [85, 1.115]]},
   {"label": "VCC = 5 V", "points": [[-40, 1.056], [-30, 1.067], [-20, 1.074], [0, 1.083], [20, 1.09], [40, 1.095], [60, 1.099], [85, 1.102]]}
 ]}
```

## Package

```table id=pkg caption="Packages"
| Package | Drawing | Body | Pitch | Height, max | Notes |
|---|---|---|---|---|---|
| 8-lead PDIP, 0.300" wide | Atmel 8P3 rev B (JEDEC MS-001 BA) | 0.365" × 0.250" nom | 0.100" | 0.210" | ATtiny25/45/85 |
| 8-lead SOIC, 0.208" wide (EIAJ) | Atmel 8S2 rev F (EIAJ EDR-7320) | 5.13–5.35 × 5.18–5.40 mm | 1.27 mm | 2.16 mm | ATtiny25/45/85 |
| 8-lead SOIC, 0.150" wide (JEDEC) | Atmel S8S1 rev A (JEDEC MS-012) | 4.80–4.98 × 3.81–3.99 mm | 1.27 mm | 1.75 mm | ATtiny25 only |
| 8-lead TSSOP, 4.4 mm wide | Atmel 8X rev A (JEDEC MO-153AC) | 3.05 × 4.40 mm nom | 0.65 mm | 1.20 mm | ATtiny45 only |
| 20-pad QFN/MLF, 4 × 4 mm | Atmel 20M1 rev B (JEDEC MO-220 WGGD-5, saw singulation) | 4.00 × 4.00 mm | 0.50 mm | 0.80 mm | 2.6 mm exposed pad; all three |
```

```table id=pkg-dims caption="Package dimensions"
| Package | Dimension | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| 8P3 | Seated height (A) [1] | | | 0.210 | in |
| ^ | Body thickness (A2) | 0.115 | 0.130 | 0.195 | in |
| ^ | Lead width (b) [2] | 0.014 | 0.018 | 0.022 | in |
| ^ | Shoulder width (b2) [3] | 0.045 | 0.060 | 0.070 | in |
| ^ | Shoulder width, 4 places (b3) [3] | 0.030 | 0.039 | 0.045 | in |
| ^ | Lead thickness (c) | 0.008 | 0.010 | 0.014 | in |
| ^ | Body length (D) [4] | 0.355 | 0.365 | 0.400 | in |
| ^ | Body end to lead (D1) [4] | 0.005 | | | in |
| ^ | Row spacing, shoulders (E) [5] | 0.300 | 0.310 | 0.325 | in |
| ^ | Body width (E1) [4] | 0.240 | 0.250 | 0.280 | in |
| ^ | Pitch (e) | | 0.100 | | in |
| ^ | Row spacing, leads (eA) [5] | | 0.300 | | in |
| ^ | Lead length (L) [1] | 0.115 | 0.130 | 0.150 | in |
| 8S2 | Height (A) | 1.70 | | 2.16 | mm |
| ^ | Standoff (A1) | 0.05 | | 0.25 | mm |
| ^ | Lead width (b) [6] | 0.35 | | 0.48 | mm |
| ^ | Lead thickness (C) [6] | 0.15 | | 0.35 | mm |
| ^ | Body length (D) | 5.13 | | 5.35 | mm |
| ^ | Body width (E1) [7] | 5.18 | | 5.40 | mm |
| ^ | Overall width (E) | 7.70 | | 8.26 | mm |
| ^ | Foot length (L) | 0.51 | | 0.85 | mm |
| ^ | Pitch (e) | | 1.27 | | mm |
| ^ | Lead angle (θ) | 0 | | 8 | ° |
| S8S1 | Height (A) | 1.35 | | 1.75 | mm |
| ^ | Standoff (A1) | 0.1 | | 0.25 | mm |
| ^ | Lead width (b) | 0.31 | | 0.51 | mm |
| ^ | Lead thickness (C) | 0.17 | | 0.25 | mm |
| ^ | Body length (D) | 4.80 | | 4.98 | mm |
| ^ | Body width (E1) | 3.81 | | 3.99 | mm |
| ^ | Overall width (E) | 5.79 | | 6.20 | mm |
| ^ | Foot length (L) | 0.4 | | 1.27 | mm |
| ^ | Pitch (e) | | 1.27 | | mm |
| ^ | Lead angle | 0 | | 8 | ° |
| 8X | Height (A) | 1.05 | 1.10 | 1.20 | mm |
| ^ | Standoff (A1) | 0.05 | 0.10 | 0.15 | mm |
| ^ | Lead width (b) | 0.25 | | 0.30 | mm |
| ^ | Lead thickness (C) | | 0.127 | | mm |
| ^ | Body length (D) | 2.90 | 3.05 | 3.10 | mm |
| ^ | Body width (E1) | 4.30 | 4.40 | 4.50 | mm |
| ^ | Overall width (E) | 6.20 | 6.40 | 6.60 | mm |
| ^ | Foot length (L) | 0.50 | 0.60 | 0.70 | mm |
| ^ | Pitch (e) | | 0.65 | | mm |
| ^ | Lead angle (Ø) | 0 | | 8 | ° |
| 20M1 | Height (A) | 0.70 | 0.75 | 0.80 | mm |
| ^ | Standoff (A1) | | 0.01 | 0.05 | mm |
| ^ | Lead frame thickness (A2) | | 0.20 | | mm |
| ^ | Pad width (b) | 0.18 | 0.23 | 0.30 | mm |
| ^ | Body (D, E) | | 4.00 | | mm |
| ^ | Exposed pad (D2, E2) | 2.45 | 2.60 | 2.75 | mm |
| ^ | Pitch (e) | | 0.50 | | mm |
| ^ | Pad length (L) | 0.35 | 0.40 | 0.55 | mm |

[1] Measured with the package seated in JEDEC seating plane gauge GS-3.
[2] Pointed or rounded lead tips are preferred, to ease insertion.
[3] Maximum excludes dambar protrusions of up to 0.010" (0.25 mm).
[4] Excludes mold flash or protrusions of up to 0.010".
[5] Measured with the leads constrained perpendicular to the datum.
[6] Plated terminal; plating 0.007–0.021 mm thick.
[7] Excludes die mismatch and resin burrs.
```

## Ordering information

```table id=ordering-table caption="Orderable parts [1]"
| Part | Package | Temperature range | Speed, supply |
|---|---|---|---|
| ATtiny25V-10PU | 8P3 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny25V-10SU, ATtiny25V-10SH | 8S2 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny25V-10SSU, ATtiny25V-10SSH | S8S1 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny25V-10MU | 20M1 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny25V-10SN | 8S2 | -40 °C to 105 °C [2] | 10 MHz, 1.8–5.5 V |
| ATtiny25V-10SSN | S8S1 | -40 °C to 105 °C [2] | 10 MHz, 1.8–5.5 V |
| ATtiny25V-10MF | 20M1 | -40 °C to 125 °C [2] | 10 MHz, 1.8–5.5 V |
| ATtiny25-20PU | 8P3 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny25-20SU, ATtiny25-20SH | 8S2 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny25-20SSU, ATtiny25-20SSH | S8S1 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny25-20MU | 20M1 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny25-20SN | 8S2 | -40 °C to 105 °C [2] | 20 MHz, 2.7–5.5 V |
| ATtiny25-20SSN | S8S1 | -40 °C to 105 °C [2] | 20 MHz, 2.7–5.5 V |
| ATtiny25-20MF | 20M1 | -40 °C to 125 °C [2] | 20 MHz, 2.7–5.5 V |
| ATtiny45V-10PU | 8P3 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny45V-10SU, ATtiny45V-10SH | 8S2 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny45V-10XU | 8X | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny45V-10MU | 20M1 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny45-20PU | 8P3 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny45-20SU, ATtiny45-20SH | 8S2 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny45-20XU | 8X | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny45-20MU | 20M1 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny85V-10PU | 8P3 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny85V-10SU, ATtiny85V-10SH | 8S2 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny85V-10MU | 20M1 | -40 °C to 85 °C | 10 MHz, 1.8–5.5 V |
| ATtiny85-20PU | 8P3 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny85-20SU, ATtiny85-20SH | 8S2 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |
| ATtiny85-20MU | 20M1 | -40 °C to 85 °C | 20 MHz, 2.7–5.5 V |

[1] Every code except the PDIP (-PU) ones also exists with an R suffix for tape and reel (ATtiny85-20SUR). All packages are Pb-free, halide-free and RoHS compliant; the 85 °C parts can also be supplied as wafers. Speed is the maximum; see the speed vs V~CC~ limits in [recommended operating conditions](#recommended-operating-conditions).
[2] Characteristics in the source's appendices A (105 °C) and B (125 °C), which this PDF does not contain.
```

```table id=part-key caption="Part number key: ATtiny<n>[V]-<speed><package><temp>[R]"
| Field | Code | Meaning |
|---|---|---|
| Device | 25, 45, 85 | 2, 4, 8 KB flash |
| Voltage | V | 1.8–5.5 V, 10 MHz grade |
| ^ | (blank) | 2.7–5.5 V, 20 MHz grade |
| Speed | 10, 20 | Maximum MHz |
| Package | P | 8P3 PDIP |
| ^ | S | 8S2 SOIC |
| ^ | SS | S8S1 SOIC |
| ^ | X | 8X TSSOP |
| ^ | M | 20M1 QFN/MLF |
| Temperature and finish | U | -40 °C to 85 °C, matte tin |
| ^ | H | -40 °C to 85 °C, NiPdAu lead finish |
| ^ | N | -40 °C to 105 °C, matte tin |
| ^ | F | -40 °C to 125 °C |
| Packing | R | Tape and reel |
```

## Conversion notes

- Pages 1–16 (pinout, overview, CPU core, memories), 20 (I/O memory), 48–49 (vector table), 200–215 (register summary, instruction set summary, ordering, package dimensions, errata) converted for the Description through Memory map, Registers and commands, Package and Ordering sections. Dropped: feature list (p.1), "About" page (resources, code-example conventions, QTouch library; p.6), the CPU pipelining figures 4-4 and 4-5 (one sentence each), block diagrams 2-1 and 4-1 (one sentence), the interrupt-vector and CLI/SEI code examples (behavior kept as text), package outline drawings (dimensions kept as tables), revision history (pp.216–221), TOC, blank and back pages.
- Errata: the format has no errata section; current errata became the last subsection of Functional description, one table row per erratum with the affected device revisions merged.
- Instruction set: the source summary has no opcode patterns, operand ranges or word counts; the Words column is added from the AVR instruction set (LDS and STS are 32-bit) and flagged in its note. Rows for the X/Y/Z variants of LD/ST and for paired SEx/CLx flag instructions were merged, one row per operation; no operation, flag or cycle count was dropped. Arrows, ⊕ and ∧ lost in text extraction were restored from the page image.
- Ordering: the R (tape and reel) variants are folded into a note rather than listed; the part-number key is assembled from the source's code-indicator notes. The 105 °C and 125 °C ATtiny25 grades point to appendices A and B, which this PDF does not include.
- Source errors: page 5 gives SRAM as 128/256/256 bytes (pages 1 and 15 say 128/256/512; the ATtiny85 data space ends at 0x025F). Page 15 says the data memory has 224/352/607 locations; 0x025F + 1 = 608. The register summary implements SP9:SP8 in SPH while section 4.6.1 draws SP15:SP8. The errata refer to "table 14-4" for COM1x bits.
- Peripherals EEPROM … I/O ports: pages 16–19, 21–47, 49–64 converted (EEPROM, clock system, power management, reset and watchdog, interrupts, I/O ports); the vector table (48–49) is under Memory map, page 20 (I/O memory) under Memory map.
- Dropped there: EEPROM, watchdog-off and port set-up code examples (the EEPROM write, CLKPR, BODS and watchdog timed sequences kept as numbered steps); block diagrams of the clock distribution, PCK clocking, reset logic, watchdog, general digital I/O and alternate-function pin logic (each described in a sentence); the external clock and crystal connection drawings (described in text); the pin synchronization waveforms (Figures 10-3, 10-4, given as the ½–1½ clock delay in text).
- The five reset waveforms (Figures 8-2 to 8-6) became one WaveDrom diagram plus text; the analog V~CC~ ramps are not drawn. The pin change interrupt timing (Figure 9-1) redrawn in WaveDrom at half-clock resolution.
- Merged: the three start-up tables with identical rows (external clock, calibrated RC at 8 MHz, 128 kHz) into one; MCUCR, repeated in chapters 7, 9 and 10, described once under Power management.
- Source errors: Table 6-1's external clock row says the CKSEL fuses must be programmed to "00" (CKSEL[3:0] = 0000); Table 8-3 gives 32K as 32764 cycles (32768); Section 6.4 says CLKO is not output during reset, Section 10.3.1 says it is (kept the clock chapter's statement); the PB1 OC1A and PB0 !{OC1A} descriptions say "Compare Match B" (they are compare match A); the EEARL heading's "Rear/Write" typo; Table 10-4's PB3 column header carries a stray overline across "PB3/A", and Table 10-5's over "MOSI" (dropped). Section 10.2.3 calls output high {DDxn, PORTxn} = 0b10 in one sentence (it is 0b11).
- Pages 65–107 converted (Timer/Counter0, Timer/Counter1, Timer/Counter1 in ATtiny15 mode, dead time generator). Dropped: feature lists, block diagrams (Figures 11-1, 11-3 to 11-6, 12-2, 12-3, 13-1 to 13-3, 14-1, 14-2; each a sentence or a table), the T0 sampling schematic (11-2, described in text), the counter-value sawtooth plots of CTC, fast PWM and phase correct PWM (11-7 to 11-9, described in text), the zero-overlap output pair (12-4) and the OCR latching glitch plots (12-5, 13-4).
- Figures 11-10 to 11-13 redrawn as WaveDrom; Figure 14-3 redrawn with arbitrary dead times. The synchronization delays of Figure 12-2 became a table.
- Registers that the PDF describes two or three times (GTCCR, TIMSK, TIFR, PLLCSR in chapters 11, 12 and 13) are described once, under Timer/Counter0 (GTCCR, TIMSK, TIFR) and Timer/Counter1 (PLLCSR); chapter 13 is reduced to what differs from normal mode. TCNT0/OCR0x, TCNT1/OCR1x and DT1A/DT1B, whose bit tables would be one field each or identical, are described in prose or one shared table.
- Chapter 13 repeats Table 12-3 (PWM frequencies for 64 MHz PCK) as Table 13-3, unchanged; it does not fit ATtiny15 mode (25.6 MHz PCK, different CS1 codes) and is not repeated.
- Source inconsistencies: GTCCR access is R/W, R, R, R, R, R, R, R/W in chapter 11 but R/W, R/W, R/W, R/W, W, W, R/W, R/W in chapters 12 and 13 (the latter kept). In ATtiny15 mode, TCCR1's CTC1 is said to clear TCNT1 after a match with OCR1A, while OCR1C's description says a match with OCR1C clears it (OCR1C replaces ATtiny15's OCR1B, the clear value there); the file states the behavior without naming the register. Table 13-2 lists output levels for COM1A = 01, which Table 13-1 says leaves OC1A unconnected. DT1B's description says "rising edge of OC1A" (means OC1B). Chapter 14 refers to "Figure 46" and "table 46" (means Figures 14-2 and Table 14-1). TIMSK bit descriptions name the vectors $003 (OCIE1A), $009 (OCIE1B) and $004 (TOIE1), see the vector table. Fast PWM text says "AC0A pin" (means OC0A).
- Overbars restored from page renders (Tables 12-1, 12-2; !{OC1A} on PB0, !{OC1B} on PB3; DT1xL acts on !{OC1x}).
- Pages 108–138 converted (USI, analog comparator, ADC), plus the ADC characteristics of pages 167–169 (placed under Peripherals › ADC).
- USI: the SPI master, fast-master and slave assembly examples (15.3.2, 15.3.3) dropped as code examples; their register sequences are kept in one sentence each. Block diagrams (15-1, 16-1, 17-1), the simplified three-/two-wire connection figures (15-2, 15-4), the start-detector logic diagram (15-6), the auto-trigger logic (17-2), the ADC prescaler (17-3), the analog input circuit (17-8) and the accuracy-definition plots (17-9 to 17-12) are described in text.
- Figures 15-3 and 15-5 redrawn as WaveDrom; the A–E/A–F step marks of the source are kept in the prose (15-5 has no nodes). ADC timing figures 17-4 to 17-7 became the conversion-time table plus one WaveDrom of a single conversion; the first, auto-triggered and free-running variants are described in a sentence (they differ only in cycle counts given in the table).
- The two differential ADC tables (21-9 unipolar, 21-10 bipolar) merged into one table with column groups. "GND" as minimum input voltage written 0 (noted in the tables).
- ADMUX: the source's REFS note says "ADCSR" for ADCSRA. Section 17.3 says the offset trick reduces offset error "below 1 LSW" (read: LSB). Table 21-10 gives bipolar resolution as 10 bits while 17.11.3 and BIN say 9 bits plus sign; both kept as written. The source lists differential pairs as "PB3, PB4 or PB2, PB5" in 17.2 but MUX allows only the pairs in table adc-mux.
- Pages 139–160 converted (debugWIRE, self-programming, memory programming) with the serial and high-voltage serial programming characteristics of pages 170–171, placed in the Memory programming subsection.
- Dropped: debugWIRE feature list; debugWIRE, serial programming and high-voltage setup figures (18-1, 20-1, 20-3: connections stated in text); the flash page-addressing figures (19-1, 20-4: given by the page-size table); the serial instruction example figure (20-2) and the serial and high-voltage waveform figures (21-4, 20-5: frame format stated in text); the signature-read assembly example; the application-note URL note.
- The self-programming SPM sequences (four-cycle timed SPMCSR writes) became one table instead of code; the software fuse/lock/signature read procedures became one table.
- High-voltage instruction table: SDO frames that are all don't-care (x_xxxx_xxxx_xx) are omitted and stated in a note; second frames (instr. 5/6) are separate rows marked "(instr. 5/6)".
- Source errors kept out: Table 20-7 heads its last column "Signature Byte 0" (it is byte 2); PGERS says the CPU halts "during the entire Page Write" (Page Erase meant); Table 20-3 heads the extended fuse byte "Fuse High Byte"; Table 20-9 calls the EEPROM byte-in-page field "PCWORD"; serial programming steps 4–5 give fixed address widths ("5 LSB", "6 MSB") that do not fit every part (stated generically); step 5B refers to Table 20-9 for t~WD_EEPROM~ (Table 20-11); 20.7.4 and 20.7.5 cite Table 20-12 and Table 21-11 for page sizes (Tables 20-8 and 20-9).
- Pages 161–166 converted (chapter 21 except ADC and programming characteristics, which sit with their peripherals) and pages 172–199 (chapter 22, typical characteristics).
- The source has no recommended-operating-conditions table; one was built from the DC characteristics' temperature range and the maximum-frequency figures 21-1 and 21-2 (safe operating area drawn as a table with a linear-interpolation note). The figures are not labelled per device; 21-1 (1.8 V, 4–10 MHz) was taken as ATtiny25V/45V/85V and 21-2 (2.7 V, 10–20 MHz) as ATtiny25/45/85.
- External clock drive waveform (figure 21-3) redrawn in WaveDrom without the rise/fall slopes (t~CLCH~, t~CHCL~ stay in the table).
- Standard and enhanced power-on reset tables merged into one table with column groups; the device-revision lists became a paragraph.
- Typical characteristics: 12 of 52 curves kept, chosen by the format's rule and digitized from 90 dpi renders (about 2 % of full scale): figures 22-2, 22-7, 22-11, 22-12, 22-19, 22-20, 22-21, 22-22, 22-40, 22-41, 22-34, 22-37. Dropped (40): 22-1, 22-3 to 22-6, 22-8 to 22-10 (active and idle current at low frequency and vs V~CC~ for the 8 MHz, 1 MHz and 128 kHz RC oscillator), 22-13 to 22-18 (I/O and reset pull-up current), 22-23 to 22-26 (reset pin as I/O output voltage), 22-27 to 22-32 (input thresholds and hysteresis, I/O and reset), 22-33 and 22-35 (BOD threshold at 4.3 V and 1.8 V), 22-36 (bandgap vs V~CC~), 22-38, 22-39 (watchdog oscillator), 22-42 (8 MHz RC vs OSCCAL), 22-43 to 22-45 (1.6 MHz RC), 22-46 to 22-49 (BOD, ADC, comparator and programming current), 22-50 to 22-52 (reset supply current, minimum reset pulse width).
- Tables 22-1 and 22-2 (I/O module current, absolute and percentage) merged into one table.
- Source errors: the standard power-on reset applies to "ATtiny85, revision B, and newer", which contradicts the enhanced one (revision C and newer); read as revision B only. The enhanced POR note names V~POT~ where V~POA~ is meant. Table 21-3's last row repeats the symbol t~CLCL~ for the period change; written Δt~CLCL~. Input leakage typical "< 0.05 µA" became a note.
