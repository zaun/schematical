---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc1g32",
  "title": "SN74AUC1G32 single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74auc1g32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES377P",
    "date": "2017-06",
    "pages": 26
  },
  "devices": ["gate:1G32"],
  "parts": ["SN74AUC1G32DBVR", "SN74AUC1G32DBVRG4", "SN74AUC1G32DCKR", "SN74AUC1G32DCKRG4", "SN74AUC1G32DRLR", "SN74AUC1G32YZPR"],
  "packages": {"p5": "DBV, DCK, DRL", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74AUC1G32 single 2-input positive-OR gate

## Description

This single 2-input positive-OR gate is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation. Optimized for 1.8-V operation and is 3.6-V I/O tolerant to
support mixed-mode signal operation. [[1::1]] [[1::2]]

The SN74AUC1G32 device performs the Boolean function
Y = A + B or Y = !{!{A} · !{B}} in positive logic. [[1::3]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::5]]

## Pinout

Conversion note: the source draws the YZP package in bottom view, and the drawing here is
generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input A
B: Input B
Y: Output Y
GND: Ground
VCC: Positive Supply
```
[[3::table #1]]

## Functional description

Conversion note: the logic diagram (positive logic) shows one 2-input OR gate with inputs A and B and output Y.

Table 1 lists the functional modes of SN74AUC1G32. [[8::1]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[8::Table 1]]

Source error: the source prints "(Each Inverter)" under the title of this table of an OR gate, kept out.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 3.6 | V |
| V~I~ | Input voltage [2] | | -0.5 | 3.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 3.6 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | -3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | -5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -9 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | ^ |
| Δt/Δv | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AUC1G32 | < | < | < | Unit |
| ^ | ^ | DBV, 5 pins | DCK, 5 pins | DRL, 5 pins | YZP, 5 pins | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 206 | 252 | 142 | 132 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | < | 0.8 V | | 0.55 | | ^ |
| ^ | ^ | I~OH~ = -3 mA | < | 1.1 V | 0.8 | | | ^ |
| ^ | ^ | I~OH~ = -5 mA | < | 1.4 V | 1 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA | < | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -9 mA | < | 2.3 V | 1.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | ^ |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | ^ |
| I~I~ | A or B input | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 4 | | pF |

[1] All typical values are at T~A~ = 25°C.
```
[[5::table #3]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 14 | 14 | 15 | 15 | 20 | pF |
```
[[6::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics: C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 4.8 | 1 | 3.5 | 0.6 | 2.3 | 0.5 | 0.9 | 1.5 | 0.3 | 1.4 | ns |
```
[[5::table #4]]

```table id=sw-30pf caption="Switching characteristics: C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 | 1.4 | 2.4 | 0.6 | 2.1 | ns |
```
[[5::table #5]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices. [[7::1]]

Conversion note: the source's test table sets S1 open for
t~PLH~/t~PHL~, which leaves R~L~ and C~L~ from the output to GND, and its
waveforms draw the input from 0 V to V~CC~ with the delays measured at
V~CC~/2.

```table id=pmi-load caption="Load values"
| V~CC~ | C~L~ | R~L~ | V~Δ~ |
|---|--:|--:|--:|
| 0.8 V | 15 pF | 2 kΩ | 0.1 V |
| 1.2 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.5 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.8 V ± 0.15 V | 15 pF | 2 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 15 pF | 2 kΩ | 0.15 V |
| 1.8 V ± 0.15 V | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 30 pF | 500 Ω | 0.15 V |
```
[[7::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~PLH~/t~PHL~ (S1 open)"
RL: see table
CL: see table
```
[[7::Figure 1]]

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```
[[7::figure #1]]

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74AUC1G32DBVR | Reel of 3000 | -40 to 85 | U32F, U32R
SN74AUC1G32DBVRG4 | Reel of 3000 | -40 to 85 | U32F
SN74AUC1G32DCKR | Reel of 3000 | -40 to 85 | UG5, UGF, UGR
SN74AUC1G32DCKRG4 | Reel of 3000 | -40 to 85 | UG5, UGF, UGR
SN74AUC1G32DRLR | Reel of 4000 | -40 to 85 | UG7, UGR
SN74AUC1G32YZPR | Reel of 3000 | -40 to 85 | UGN
```
[[10::table #1]]

## Conversion notes

- Source pages 1–8 converted; the documentation support, package option
  addendum (p.10), tape and reel and mechanical pages (9–26) dropped,
  replaced by the generated Package and Ordering sections. The table of
  contents and revision history on page 2 are dropped too.
- Dropped: the features and applications lists, the NanoFree package
  text, the Device Information table, the enable/disable-time,
  pulse-duration and setup/hold parts of Figure 1 with their notes B, E
  and F (this gate has no output enable), and the S1 positions for
  t~PLZ~/t~PZL~ (2 × V~CC~) and t~PHZ~/t~PZH~ (GND).
- From images: overbars of Y = !{!{A} · !{B}} (p.1; the text layer reads
  "Y + A ) B or Y + A · B").
- From images: V~Δ~ (load table of Figure 1, p.7; the text layer reads
  "VD").
- Source issues: Source error: the function table on p.8 is subtitled
  "(Each Inverter)", but the device is a single OR gate; the subtitle is
  kept out.
