---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g32-ep",
  "title": "SN74LVC1G32-EP single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc1g32-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES458D",
    "date": "2007-06",
    "pages": 17
  },
  "devices": ["gate:1G32"],
  "parts": ["SN74LVC1G32IDCKREP", "SN74LVC1G32MDBVREP", "SN74LVC1G32MDCKREP", "V62/04734-01XE", "V62/04734-02XE", "V62/04734-02YE"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G32-EP single 2-input positive-OR gate

## Description

This single 2-input positive-OR gate is designed for 1.65-V to 5.5-V
V~CC~ operation. [[1::4]]

The SN74LVC1G32-EP performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic. [[1::5]]

This device is fully specified for partial power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::6]]

Extended Temperature Performance of –55°C to 125°C. Supports 5-V V~CC~
Operation. Inputs Accept Voltages to 5.5 V. Latch-Up Performance Exceeds
100 mA Per JESD 78, Class II. [[1::1]] [[1::2]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input
B: Input
GND: Ground
Y: Output
VCC: Supply voltage
```
[[1::table #1]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[1::table #2]]

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
| θ~JA~ | Package thermal impedance [4] | DBV package | | 324.1 | °C/W |
| ^ | ^ | DCK package | | 252 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

ESD Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

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
| ∆t/∆v | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | SN74LVC1G32IDCKREP | -40 | 85 | °C |
| ^ | ^ | SN74LVC1G32MDCKREP | -55 | 125 | °C |
| ^ | ^ | SN74LVC1G32MDBVREP | ^ | ^ | ^ |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 μA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | μA |
| ∆I~CC~ | | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[4::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 20 | 20 | 21 | 22 | pF |
```
[[4::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.9 | 7.2 | 0.8 | 4.4 | 0.9 | 3.6 | 0.8 | 3.4 | ns |
```
[[4::table #3]]

```table id=sw-30-50pf caption="Switching characteristics, C~L~ = 30 pF or 50 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.8 | 8 | 1.2 | 6.5 | 1.1 | 5.7 | 1 | 4.5 | ns |
```
[[4::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. [[5::2]]

All parameters and waveforms are not applicable to all devices. [[5::2]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figures also show pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[5::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[5::Figure 1]]

```table id=test-conditions-1 caption="Test conditions (Figure 1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~∆~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```
[[5::table #2]]

```table id=test-conditions-2 caption="Test conditions (Figure 2)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~∆~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[5::Figure 1]]

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74LVC1G32IDCKREP | Reel of 3000 | -40 to 85 | CGO
SN74LVC1G32MDBVREP | Reel of 3000 | -55 to 125 | SBGM
SN74LVC1G32MDCKREP | Reel of 3000 | -55 to 125 | BYB
V62/04734-01XE | Reel of 3000 | -40 to 85 | CGO
V62/04734-02XE | Reel of 3000 | -55 to 125 | BYB
V62/04734-02YE | Reel of 3000 | -55 to 125 | SBGM
```
[[7::table #1]]

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –40°C to 85°C | SOT (SC-70) – DCK | Reel of 3000 | SN74LVC1G32IDCKREP | CG0 |
| –55°C to 125°C | SOT (SC-70) – DCK | Reel of 3000 | SN74LVC1G32MDCKREP | BYB |
| ^ | SOP (SOT-23) – DBV | Reel of 3000 | SN74LVC1G32MDBVREP | SBGM |

[1] The actual top-side making has one additional character that designates the wafer fab/assembly site.
```
[[1::table #3]]

Source conflict: the ordering information table (p.1) gives the top-side marking of SN74LVC1G32IDCKREP as CG0, the package option addendum (p.7) as CGO.

Source error: the note of the ordering information table (p.1) prints "making" for "marking".

## Conversion notes

- Source pages 1–6 converted. The ordering lines are from the package
  option addendum (pp.7–8). The tape and reel and mechanical pages
  (9–17) are left out; the generated Package section replaces them.
- Dropped: features list (the temperature, 5-V, input-voltage, latch-up
  and ESD lines are kept in text), the qualification-pedigree footnote,
  the vendor pinout, the other two notes of the page-1 ordering
  information table, the logic
  diagram (p.2, one OR gate, A pin 1, B pin 2, Y pin 4).
- The pin descriptions are added; the source has no pin function table.
  The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The source's description (p.1) names the device "SN74LVC1G32"; the
  data sheet's title is SN74LVC1G32-EP, and the name is written with the
  suffix here.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 1 and 2 (pp.5–6)
  are the same circuit and waveforms with different load values, each
  with its own table.
- From images: overbars of Y = !{!{A} • !{B}} (p.1; the text layer
  prints "Y + A • B").
- Source issues:
  - Source conflict: the ordering information table (p.1) gives the
    top-side marking of SN74LVC1G32IDCKREP as CG0; the package option
    addendum (p.7) gives CGO.
  - Source error: the marking note of the ordering information table
    (p.1) prints "making" for "marking"; kept as printed.
