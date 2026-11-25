---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g32-ep",
  "title": "SN74LVC2G32-EP dual 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc2g32-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES543A",
    "date": "2006-08",
    "pages": 13
  },
  "devices": ["gate:2G32"],
  "parts": ["SN74LVC2G32MDCUREP", "V62/06630-01XE"],
  "packages": {"p8": "DCU"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G32-EP dual 2-input positive-OR gate

## Description

This dual 2-input positive-OR gate is designed for 1.65-V to 5.5-V
V~CC~ operation. [[1::4]]

The SN74LVC2G32 performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::6]]

Controlled Baseline. One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of –55°C to 125°C. Enhanced Diminishing
Manufacturing Sources (DMS) Support. Enhanced Product-Change
Notification. Qualification Pedigree. Supports 5-V V~CC~ Operation.
Inputs Accept Voltages to 5.5 V. Max t~pd~ of 3.8 ns at 3.3 V. Source
conflict: the switching characteristics (p.4) give a maximum t~pd~ of
5.8 ns at V~CC~ = 3.3 V ± 0.3 V. Low Power Consumption, 10-µA Max
I~CC~. ±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground
Bounce) <0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output
V~OH~ Undershoot) >2 V at V~CC~ = 3.3 V, T~A~ = 25°C. I~off~ Supports
Partial Power-Down-Mode Operation. Latch-Up Performance Exceeds 100 mA
Per JESD 78, Class II. [[1::1]] [[1::2]]

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. This
includes, but is not limited to, Highly Accelerated Stress Test (HAST)
or biased 85/85, temperature cycle, autoclave or unbiased HAST,
electromigration, bond intermetallic life, and mold compound life. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits. [[1::3]]

## Pinout

```pinout pinout=p8
```

## Pin functions

```pins
1A, 1B: Gate 1 inputs
2A, 2B: Gate 2 inputs
1Y, 2Y: Gate outputs
GND: Ground
VCC: Supply voltage
```
[[1::table #1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[1::table #2]]

Conversion note: the logic diagram (positive logic) shows two 2-input
OR gates, 1A (pin 1) and 1B (pin 2) to 1Y (pin 7), and 2A (pin 5) and
2B (pin 6) to 2Y (pin 3).

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
| θ~JA~ | Package thermal impedance [4] | | | 220 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
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
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.6 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.6 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[4::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 17 | 17 | 17 | 19 | pF |
```
[[4::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.4 | 11 | 1 | 7.5 | 1 | 5.8 | 1 | 4.7 | ns |
```
[[4::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[5::2]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
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

```table id=test-conditions caption="Test conditions (Figure 1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[5::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[5::Figure 1]]

## Package

```package footprint=VSSOP-8
```

## Ordering information

```ordering
SN74LVC2G32MDCUREP | Reel of 3000 | -55 to 125 | BUE
V62/06630-01XE | Reel of 3000 | -40 to 85 | BUE
```
[[6::table #1]]

```table id=ordering-table caption="Ordering information"
| T~A~ | Package [1] | < | Orderable part number | Top-side marking [2] |
|---|---|---|---|---|
| –55°C to 125°C | SSOP – DCU | Reel of 3000 | SN74LVC2G32MDCUREP | BUE |

[1] Package drawings, standard packing quantities, thermal data, symbolization, and PCB design guidelines are available at www.ti.com/sc/package.
[2] DCU: The actual top-side marking has three additional characters that designate the year, month, and assembly/test site.
```
[[1::table #3]]

## Conversion notes

- Source pages 1–5 converted. The ordering lines are from the package
  option addendum (pp.6–7). The tape and reel (8–9) and mechanical
  pages (10–12) and the notice page (13) are left out; the generated
  Package section replaces them.
- Dropped: the vendor pinout drawing, the logic diagram (described in
  one sentence). The ordering information table of p.1 follows the
  generated ordering block.
  The features are sentences in Description and, for ESD protection,
  under Absolute maximum ratings; the qualification-pedigree footnote
  is the last paragraph of Description.
- The source has no pin functions table; the pin descriptions are
  written from the pin names and the function table.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- From images: Y = A + B and the overbars of Y = !{!{A} • !{B}} (p.1;
  the text layer prints "Y + A ) B or Y + A • B").
- Source issues:
  - Source conflict: the features (p.1) give "Max t~pd~ of 3.8 ns at
    3.3 V"; the switching characteristics (p.4) give a maximum t~pd~ of
    5.8 ns at V~CC~ = 3.3 V ± 0.3 V. Both kept.
  - Unconfirmed: the package option addendum (p.6) gives V62/06630-01XE
    an operating temperature of -40 to 85 °C, while it gives
    SN74LVC2G32MDCUREP, with the same marking, -55 to 125 °C.
