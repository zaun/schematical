---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74alvc10",
  "title": "SN74ALVC10 triple 3-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74alvc10.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES106H",
    "date": "2004-10",
    "pages": 18
  },
  "devices": ["gate:74_10"],
  "parts": ["SN74ALVC10D", "SN74ALVC10DGVR", "SN74ALVC10DR", "SN74ALVC10NSR", "SN74ALVC10PWR"],
  "packages": {"p14": "D, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74ALVC10 triple 3-input positive-NAND gate

## Description

This triple 3-input positive-NAND gate is designed for 1.65-V to 3.6-V
V~CC~ operation. The SN74ALVC10 performs the Boolean function
Y = !{A • B • C} or Y = !{A} + !{B} + !{C} in positive logic. [[1::2]]

±24-mA Output Drive at 3.3 V. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Gate input A
1B, 2B, 3B: Gate input B
1C, 2C, 3C: Gate input C
1Y, 2Y, 3Y: Gate output Y
GND: Ground
VCC: Positive supply
```
[[1::table #1]]

## Function tables

Conversion note: the logic diagram shows each gate as a 3-input NAND gate (positive logic).

```table id=function caption="Function table (each gate)"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | D package | | 86 | °C/W |
| ^ | ^ | DGV package | | 127 | °C/W |
| ^ | ^ | NS package | | 76 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 4.6 V maximum.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

Latch-Up Performance Exceeds 250 mA Per JESD 17. ESD Protection Exceeds
JESD 22: 2000-V Human-Body Model (A114-A), 200-V Machine Model (A115-A),
1000-V Charged-Device Model (C101). [[1::1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.65 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -12 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 12 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | < | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -4 mA | < | 1.65 V | 1.2 | | | V |
| ^ | I~OH~ = -6 mA | < | 2.3 V | 2 | | | V |
| ^ | I~OH~ = -12 mA | < | 2.3 V | 1.7 | | | V |
| ^ | ^ | < | 2.7 V | 2.2 | | | V |
| ^ | ^ | < | 3 V | 2.4 | | | V |
| ^ | I~OH~ = -24 mA | < | 3 V | 2 | | | V |
| V~OL~ | I~OL~ = 100 µA | < | 1.65 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 4 mA | < | 1.65 V | | | 0.45 | V |
| ^ | I~OL~ = 6 mA | < | 2.3 V | | | 0.4 | V |
| ^ | I~OL~ = 12 mA | < | 2.3 V | | | 0.7 | V |
| ^ | ^ | < | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA | < | 3 V | | | 0.55 | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND | I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V | Other inputs at V~CC~ or GND | 3 V to 3.6 V | | | 750 | µA |
| C~i~ | V~I~ = V~CC~ or GND | < | 3.3 V | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[3::table #1]]

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | C~L~ = 0, f = 10 MHz | 23 | 24 | 26 | pF |
```
[[3::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics (see Figure 1)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 ± 0.2 V | < | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | 1.1 | 4.8 | 1 | 3 | | 3.3 | 1 | 3 | ns |
```
[[3::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[4::1]]

Conversion note: in the load circuit of Figure 1, S1 is open for t~pd~, so the R~L~ to ground and C~L~ load the output.

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | Input | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```
[[4::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: see table
CL: see table
```
[[4::Figure 1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: see table
```
[[4::Figure 1]]

Conversion note: the source figure also shows the second R~L~ and switch S1 to V~LOAD~ or GND with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms with their notes B, E and F, which do not apply to this part.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74ALVC10D | Tube of 50 | -40 to 85 | ALVC10
SN74ALVC10DGVR | Reel of 2000 | -40 to 85 | VA10
SN74ALVC10DR | Reel of 2500 | -40 to 85 | ALVC10
SN74ALVC10NSR | Reel of 2000 | -40 to 85 | ALVC10
SN74ALVC10PWR | Reel of 2000 | -40 to 85 | VA10
```
[[1::table #3]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum (p.5–6), tape
  and reel, tube and mechanical pages (7–18) dropped, replaced by the
  generated Package and Ordering sections. Packing quantities come from
  the package option addendum.
- Dropped: features list (output drive kept in Description, latch-up
  and ESD kept as sentences under Absolute maximum ratings), the vendor
  ordering table on p.1, the logic diagram, and the parts of Figure 1
  that do not apply to this part (named in the Parameter measurement
  information note).
- The electrical characteristics table of the source names its
  parameters by symbol only; no parameter names are added.
- From images: overbars of Y = !{A • B • C} and Y = !{A} + !{B} + !{C} (p.1).
- Source issues: none found.
