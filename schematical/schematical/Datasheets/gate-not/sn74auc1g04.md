---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc1g04",
  "title": "SN74AUC1G04 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74auc1g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES370R",
    "date": "2017-06",
    "pages": 30
  },
  "devices": ["gate:1G04", "gate:1G04_SON"],
  "parts": ["SN74AUC1G04DBVR", "SN74AUC1G04DCKR", "SN74AUC1G04DRLR", "SN74AUC1G04YZPR", "SN74AUC1G04DRYR"],
  "packages": {"p5": "DBV, DCK, DRL", "dsbga": "YZP", "p6": "DRY"},
  "converted": "2026-11-21"
}
---

# SN74AUC1G04 single inverter gate

## Description

Latch-up performance exceeds 100 mA per JESD 78, Class II. Optimized for
1.8-V operation and is 3.6-V I/O tolerant to support mixed-mode signal
operation. I~off~ supports partial power down mode and back drive
protection. Sub-1-V operable. Max t~pd~ of 2.2 ns at 1.8 V. Low power
consumption, 10-µA maximum I~CC~. ±8-mA output drive at 1.8 V. [[1::2]]

This single inverter gate is operational at 0.8-V to 2.7-V V~CC~, but is
designed specifically for 1.65-V to 1.95-V V~CC~ operation. [[1::1]]

The SN74AUC1G04 performs the Boolean function Y = !{A}. [[1::4]]

NanoFree™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the ouput, preventing damaging
current backflow through the device when it is powered down. [[1::6]]

Source error: "ouput" is the source's spelling of "output" (Description, p.1); kept as printed.

For more information about AUC Little Logic devices, see Applications of
Texas Instruments AUC Sub-1-V Little Logic Devices, SCEA027. [[1::7]]

## Pinout

Conversion note: the source draws the YZP package in bottom view, and the drawings here are generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: A logic input
DNU: Do not use
GND: Ground
NC: No internal connection
VCC: Positive supply
Y: Y inverted output
```
[[3::table #1]]

## Functional description

Conversion note: the functional block diagram (Figure 2, logic diagram, positive logic) shows one inverter, A (pin 2) to Y (pin 4).

### Device functional modes

Table 1 lists the functional modes of the SN74AUC1G04. [[8::1]]

## Function tables

```table id=function caption="Table 1. Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[8::Table 1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 3.6 | V |
| V~I~ | Input voltage [2] | | -0.5 | 3.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 3.6 | V |
| ^ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
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
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | V |
| ^ | ^ | Machine Model (A115-A) | 200 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="See [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | -3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | -8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -9 | mA |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74AUC1G04 | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT-5X3) 5 pins | DRY (SON) 6 pins | YZP (DSBGA) 5 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 206 | 252 | 142 | 234 | 132 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | < | 0.8 V | | 0.55 | | V |
| ^ | ^ | I~OH~ = -3 mA | < | 1.1 V | 0.8 | | | V |
| ^ | ^ | I~OH~ = -5 mA | < | 1.4 V | 1 | | | V |
| ^ | ^ | I~OH~ = -8 mA | < | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -9 mA | < | 2.3 V | 1.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A input | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ = V~O~ or 2.7 V [2] | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 3 | | pF |

[1] All typical values are at T~A~ = 25°C.
[2] Unconfirmed: the source prints this I~off~ test condition as "V~I~ = V~O~ or 2.7 V" (p.5); kept as printed.
```
[[5::table #3]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 14 | 14 | 14 | 14 | 19 | pF |
```
[[6::table #1]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics: C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 4.4 | 0.8 | 3 | 0.5 | 2 | 0.5 | 1 | 2.1 | 0.5 | 1.6 | ns |
```
[[5::table #4]]

```table id=sw-30pf caption="Switching characteristics: C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.6 | 1.2 | 2.2 | 0.5 | 1.9 | ns |
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

```table id=pmi-load caption="Figure 1. Load values"
| V~CC~ | C~L~ | R~L~ | V~D~ |
|---|--:|--:|--:|
| 0.8 V | 15 pF | 2 kΩ | 0.1 V |
| 1.2 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.5 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.8 V ± 0.15 V | 15 pF | 2 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 15 pF | 2 kΩ | 0.15 V |
| 1.8 V ± 0.15 V | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 30 pF | 500 Ω | 0.15 V |
```
[[5::table #6]]

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

```package footprint=SON-6-DRY
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74AUC1G04DBVR | Reel of 3000 | -40 to 85 | U04R
SN74AUC1G04DCKR | Reel of 3000 | -40 to 85 | UC5, UCF, UCR
SN74AUC1G04DRLR | Reel of 4000 | -40 to 85 | UC7, UCR
SN74AUC1G04YZPR | Reel of 3000 | -40 to 85 | UCN
SN74AUC1G04DRYR | Reel of 5000 | -40 to 85 | UC
```
[[10::table #1]]

## Conversion notes

- Source pages 1–8 converted; p.2 holds only the table of contents and
  the revision history, which are dropped. The ordering lines are from
  the package option addendum (pp.10–11). The support page (9), tape and
  reel and mechanical pages (12–30) are left out; the generated Package
  section replaces them.
- Dropped: the features list except the lines quoted in the Description,
  the applications list, the device information table (p.1), the logic
  diagram on p.1 (the same as Figure 2), the enable/disable-time,
  pulse-duration and setup/hold parts of Figure 1 with their notes B, E
  and F (this gate has no output enable), and the S1 positions for
  t~PLZ~/t~PZL~ (2 × V~CC~) and t~PHZ~/t~PZH~ (GND).
- The package option addendum also lists SN74AUC1G04DBVRG4,
  SN74AUC1G04DCKRE4, SN74AUC1G04DCKRG4 and SN74AUC1G04DRYRG4, which the
  catalog doesn't carry.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- Figure 1 (p.7) prints V~D~ over the last column of its load values
  table; kept as printed.
- From images: overbar of Y = !{A} (p.1).
- From images: Ω in 2 kΩ, 1 kΩ, 500 Ω (load values) and Z~O~ = 50 Ω
  (Figure 1, p.7; the text layer prints "W").
- Source issues:
  - Source error: "ouput" for "output" in the Description (p.1); kept as
    printed.
  - Unconfirmed: the I~off~ test condition "V~I~ = V~O~ or 2.7 V" in the
    electrical characteristics (p.5); kept as printed.
