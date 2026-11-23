---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2gu04",
  "title": "SN74LVC2GU04 dual inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2gu04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES197N",
    "date": "2013-12",
    "pages": 22
  },
  "devices": ["gate:2GU04"],
  "parts": ["SN74LVC2GU04DBVR", "SN74LVC2GU04DBVT", "SN74LVC2GU04DCKT", "SN74LVC2GU04YZPR"],
  "packages": {"p6": "DBV, DCK", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2GU04 dual inverter gate

## Description

This dual inverter is designed for 1.65-V to 5.5-V V~CC~ operation. [[1::1]]

The SN74LVC2GU04 device contains two inverters with unbuffered outputs
and performs the Boolean function Y = !{A}. [[1::3]]

NanoFree™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::4]]

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Max
t~pd~ of 3.7 ns at 3.3 V. Low Power Consumption, 10-µA Max I~CC~.
±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground Bounce)
<0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output V~OH~
Undershoot) >2 V at V~CC~ = 3.3 V, T~A~ = 25°C. I~off~ Supports Live
Insertion, Partial-Power-Down Mode, and Back-Drive Protection. Can Be
Used as a Down Translator to Translate Inputs From a Max of 5.5 V Down
to the V~CC~ Level. Unbuffered Outputs. [[1::2]]

## Pinout

Conversion note: DBV and DCK share one pin order.

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A: Inverter input A
1Y, 2Y: Inverter output Y
GND: Ground
VCC: Supply voltage
```
[[1::table #1]]

## Functional description

Conversion note: the logic diagram (positive logic) shows two inverters, 1A (pin 1) to 1Y (pin 6) and 2A (pin 3) to 2Y (pin 4).

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[2::table #1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
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

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.65 | 5.5 | V |
| V~IH~ | High-level input voltage | I~O~ = –100 µA | 0.75 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | I~O~ = 100 µA | | 0.25 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| T~A~ | Operating free-air temperature [2] | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
[2] Source conflict: this table (p.3) gives –40 to 125 °C with no package named; the package option addendum (p.7) lists SN74LVC2GU04YZPR at -40 to 85 and the DBV and DCK orderables at -40 to 125.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | V~IL~ = 0 V | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | V~IH~ = V~CC~ | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | < | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | < | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| C~I~ | | V~I~ = V~CC~ or GND | < | 3.3 V | | 7 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[3::table #2]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 7 | 7 | 8 | 23 | pF |
```
[[4::table #1]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | SN74LVC2GU04, –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.2 | 5.5 | 1 | 4 | 1.1 | 3.7 | 1 | 3 | ns |
```
[[3::table #3]]

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | SN74LVC2GU04, –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.2 | 6.3 | 1 | 4.5 | 1.1 | 4.2 | 1 | 3.5 | ns |
```
[[4::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[5::1]]

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

```package footprint=SOT23-6
```

```package footprint=SC70-6
```

```package footprint=DSBGA-6-YZP
```

## Ordering information

```ordering
SN74LVC2GU04DBVR | Reel of 3000 | -40 to 125 | CU45, CU4R
SN74LVC2GU04DBVT | Reel of 250 | -40 to 125 | CU45, CU4R
SN74LVC2GU04DCKT | Reel of 250 | -40 to 125 | CD5, CDF, CDJ, CDK, CDR
SN74LVC2GU04YZPR | Reel of 3000 | -40 to 85 | CDN
```
[[7::table #1]]

## Conversion notes

- Source pages 1–5 converted. The ordering lines are from the package
  option addendum (pp.7–8). The revision history page (6), tape and
  reel (9–12), mechanical pages (13–21) and the notice page (22) are
  left out; the generated Package section replaces them.
- Dropped: the vendor pinout drawings and the NanoFree package feature
  line; the enable/disable-time, pulse-duration and setup/hold parts of
  Figure 1 with their notes B, E and F. The logic diagram is described
  in text. The other features are sentences in Description, and the
  latch-up and ESD feature lines are under Absolute maximum ratings
  (the source has no ESD ratings table).
- The addendum also lists SN74LVC2GU04DCKR, 74LVC2GU04DBVRG4,
  74LVC2GU04DCKRG4 and 74LVC2GU04DCKTG4, which the catalog doesn't
  carry.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- From images: overbar of Y = !{A} (p.1; the text layer prints
  "Y = A"), and ≤2 ns, ≤2.5 ns, ≤ 10 MHz, 1 kΩ, 500 Ω, 50 Ω, V~Δ~ (p.5;
  the text layer prints "£2 ns", "1 kW", "500 W", "VD").
- Source issues:
  - Unconfirmed: the features list (p.1) says I~off~ supports live
    insertion, partial-power-down mode and back-drive protection; the
    electrical characteristics (p.3) have no I~off~ row.
  - Source conflict: the recommended operating conditions (p.3) give
    T~A~ –40 to 125 °C for every package; the package option addendum
    (p.7) lists SN74LVC2GU04YZPR at -40 to 85 (see the recommended
    operating conditions).
