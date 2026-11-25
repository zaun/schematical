---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g10",
  "title": "SN74LVC1G10 single 3-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lvc1g10.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES486E",
    "date": "2013-12",
    "pages": 29
  },
  "devices": ["gate:1G10"],
  "parts": ["SN74LVC1G10DBVR", "SN74LVC1G10DBVRG4", "SN74LVC1G10DCKR", "SN74LVC1G10DCKRG4", "SN74LVC1G10DRYR", "SN74LVC1G10DSFR", "SN74LVC1G10YZPR"],
  "packages": {"p6": "DBV, DCK, DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G10 single 3-input positive-NAND gate

## Description

The SN74LVC1G10 performs the Boolean function Y = !{A • B • C} or
Y = !{A} + !{B} + !{C} in positive logic. [[1::1]]

NanoFree™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::2]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::3]]

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Provides
Down Translation to V~CC~. Max t~pd~ of 3.8 ns at 3.3 V. Low Power
Consumption, 10-μA Max I~CC~. ±24-mA Output Drive at 3.3 V. I~off~
Supports Live Insertion, Partial-Power-Down Mode, and Back Drive
Protection. Latch-Up Performance Exceeds 100 mA per JESD 78, Class II. [[1::1]]

## Pinout

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input A
B: Input B
C: Input C
GND: Ground
VCC: Supply voltage
Y: Output Y
```
[[1::table #1]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```
[[2::table #1]]

Conversion note: the logic diagram (positive logic) shows one 3-input
NAND gate with inputs A, B and C and output Y.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | DBV package | | 165 | °C/W |
| ^ | ^ | DCK package | | 259 | ^ |
| ^ | ^ | YZP package | | 123 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #2]]

ESD Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged Device Model (C101). [[1::1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 10 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –100 μA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = –8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = –16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = –24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = –32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | All inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 3.5 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[4::table #1]]

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 17 | 18 | 19 | 22 | pF |
```
[[5::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | SN74LVC1G10, –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | 2 | 14.8 | 1.3 | 5.5 | 0.8 | 3.8 | 0.6 | 2.7 | ns |
```
[[5::table #2]]

```table id=sw-30-50pf-85 caption="Switching characteristics, C~L~ = 30 pF or 50 pF, –40°C to 85°C" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | SN74LVC1G10, –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | 2.5 | 18 | 1.6 | 6.5 | 1.4 | 5 | 1 | 3.6 | ns |
```
[[5::table #3]]

```table id=sw-30-50pf-125 caption="Switching characteristics, C~L~ = 30 pF or 50 pF, –40°C to 125°C" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | SN74LVC1G10, –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | 2.5 | 20.8 | 1.6 | 8.2 | 1.4 | 6.4 | 1 | 4.7 | ns |
```
[[5::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[6::1]]

Conversion note: in both load circuits one R~L~ goes from the output to
ground and a second R~L~ from the output to switch S1, which goes to
V~LOAD~, open or GND; the figures below draw the load with S1 open.
Conversion note: both source figures also show pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

### Figure 1 (C~L~ = 15 pF)

```table id=s1-fig1 caption="Figure 1, load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[6::table #1]]

```figure ref=load-rl-cl caption="Figure 1, load circuit (S1 open)"
RL: see table
CL: see table
```
[[6::Figure 1]]

```table id=test-conditions-fig1 caption="Figure 1, test conditions"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```
[[6::table #2]]

```figure ref=waveforms-tpd caption="Figure 1, voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[6::Figure 1]]

### Figure 2 (C~L~ = 30 pF or 50 pF)

```table id=s1-fig2 caption="Figure 2, load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[6::table #3]]

```figure ref=load-rl-cl caption="Figure 2, load circuit (S1 open)"
RL: see table
CL: see table
```
[[6::Figure 1]]

```table id=test-conditions-fig2 caption="Figure 2, test conditions"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[7::table #1]]

```figure ref=waveforms-tpd caption="Figure 2, voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[6::Figure 1]]

## Package

```package footprint=SOT23-6
```

```package footprint=SC70-6
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-6-YZP
```

## Ordering information

```ordering
SN74LVC1G10DBVR | Reel of 3000 | -40 to 125 | C105, C10R
SN74LVC1G10DBVRG4 | Reel of 3000 | -40 to 125 | C105
SN74LVC1G10DCKR | Reel of 3000 | -40 to 125 | C25, C2F, C2J, C2R
SN74LVC1G10DCKRG4 | Reel of 3000 | -40 to 125 | C25
SN74LVC1G10DRYR | Reel of 5000 | -40 to 125 | C2
SN74LVC1G10DSFR | Reel of 5000 | -40 to 125 | C2
SN74LVC1G10YZPR | Reel of 3000 | -40 to 85 | C27, C2N
```
[[9::table #1]]

## Conversion notes

- Source pages 1–7 converted. The ordering lines are from the package
  option addendum (pp.9–10). The revision history page (8), tape and
  reel and mechanical pages (11–29) are left out; the generated Package
  section replaces them.
- Dropped: the vendor pinout drawings, the ESD caution, the logic
  diagram (described in one sentence), the NanoFree package feature
  line. The ESD protection feature line follows the absolute maximum
  ratings; the other features are sentences in Description.
- The source has no pin functions table; the pin descriptions are
  written from the pin names and the function table.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~. The source prints
  the C~i~ typical value 3.5 once, between the two temperature groups.
- The load circuits are drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- The package thermal impedance (absolute maximum ratings) is given for
  the DBV, DCK and YZP packages only; the source gives none for the DRY
  and DSF packages.
- From images: the overbars of Y = !{A • B • C} and
  Y = !{A} + !{B} + !{C} (p.1).
- From images: ≤2 ns, ≤2.5 ns, 1 MΩ, 1 kΩ, 500 Ω, 50 Ω and V~Δ~ in the
  Figure 1 and Figure 2 tables and notes (pp.6–7; the text layer has
  other glyphs).
- Source issues:
  - Source conflict: the recommended operating conditions (p.3) give
    T~A~ –40 to 125 °C for all packages; the package option addendum
    (p.9) lists SN74LVC1G10YZPR at -40 to 85.
  - Unconfirmed: the input transition rise or fall rate at
    V~CC~ = 5 V ± 0.5 V is 10 ns/V, the same as at 3.3 V ± 0.3 V (p.3).
