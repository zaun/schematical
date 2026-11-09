---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g08-q1",
  "title": "SN74AHC1G08-Q1 automotive single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahc1g08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS592F",
    "date": "2024-10",
    "pages": 26
  },
  "devices": ["gate:1G08"],
  "parts": ["SN74AHC1G08QDBVRQ1", "SN74AHC1G08QDCKRQ1", "SN74AHC1G08WDTXRQ1"],
  "packages": {"p5": "DBV, DCK, DTX"},
  "converted": "2026-11-08"
}
---

# SN74AHC1G08-Q1 automotive single 2-input positive-AND gate

## Description

The SN74AHC1G08 is a single 2-input positive-AND gate. The device
performs the Boolean function Y = A • B in positive logic.

Qualified for automotive applications. Operating range of 2 V to 5.5 V;
±8 mA output drive at 5 V; Schmitt-trigger action at all inputs makes the
circuit tolerant for slower input rise and fall time.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
Y: Output Y
GND: Ground Pin
VCC: Power Pin
```

## Functional description

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table [1], which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device.

[1] Source conflict: the Electrical Characteristics table (p.5) has no ΔV~T~ row; the source gives no hysteresis value.

## Function tables

```table id=function caption="Function table"
| A [1] | B [1] | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = high voltage level, L = low voltage level, X = do not care, Z = high impedance
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge, human body model (HBM), per AEC Q100-002 [3] | | | ±2000 | V |
| ^ | Electrostatic discharge, charged device model (CDM), per AEC Q100-011 | | | ±1000 | V |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | I Suffix | -40 | 85 | °C |
| ^ | ^ | Q Suffix | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004. Source error: the table title carries marker 3; this is its only note.
```

The source prints the limits in two identical Min/Max column pairs, one
per suffix; they differ only in T~A~.

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (SOT-23) | DCK (SOT-SC70) | DTX (X2SON) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 184.7 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953). All packages 5 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | –40 °C to 85 °C | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 mA [1] | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | 2.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.7 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 mA [1] | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | | 0.52 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | | 0.52 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V or GND, I~O~ = 0 [2] | 5.5 V | | | 1 | | 10 | | 20 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | | 10 | pF |

[1] Source conflict: printed as mA here; the recommended operating conditions give I~OH~ = –50 µA and I~OL~ = 50 µA at V~CC~ = 2 V, and the absolute maximum continuous output current is ±25 mA.
[2] Unconfirmed: the source prints "V~I~ = V or GND" with no subscript on V.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25 °C | < | < | –40 °C to 85 °C | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 6.2 | 8.8 | 1 | 10.5 | | 12.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.2 | 8.8 | 1 | 10.5 | | 12.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8.7 | 12.3 | 1 | 14 | | 16.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.7 | 12.3 | 1 | 14 | | 16.5 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25 °C | < | < | –40 °C to 85 °C | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 4.3 | 5.9 | | 7 | | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.3 | 5.9 | | 7 | | 9 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.8 | 7.9 | | 9 | | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.8 | 7.9 | | 9 | | 11 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. C~L~ is 15 pF or 50 pF,
as given in the [switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50 % VCC
```

The source figure also shows the 3-state and open-drain load circuit,
pulse-duration, setup-and-hold and enable/disable waveforms; all
parameters and waveforms are not applicable to all devices.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

Typical application (Figure 8-1): one gate ANDs !{OC} (over current
detection) with PG (power supply), a second ANDs ON/!{OFF} (on/off
switch) with !{OT} (over temp detection), and a third ANDs the two
results into the !{RESET} input of a motor controller.

Power supply: any voltage between the minimum and maximum supply voltage
rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. The
0.1-µF and 1-µF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power terminal as possible
for best results.

Layout: when using multiple-input and multiple-channel logic devices,
inputs must never be left floating. All unused inputs of digital logic
devices must be connected to a logic high or logic low voltage, as
defined by the input voltage specifications, to prevent them from
floating. Generally, the inputs are tied to GND or V~CC~, whichever makes
more sense for the logic function or is more convenient. The layout
example recommends a GND flood fill, a 0.1 µF bypass capacitor placed
close to the device, and no 90° corners on signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=X2SON-5-DTX
```

## Ordering information

```ordering
SN74AHC1G08QDBVRQ1 | Reel of 3000 | -40 to 125 | 38FH, A08U
SN74AHC1G08QDCKRQ1 | Reel of 3000 | -40 to 125 | AEU
SN74AHC1G08WDTXRQ1 | Reel of 3000 | -40 to 125 | 2
```

Where several markings are listed, only one appears on a part.

## Conversion notes

- Source pages 1–10 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists (kept as one sentence),
  package information table, logic diagram (one AND gate), the layout
  example drawing (summarized in one sentence).
- The ESD ratings table is merged into Absolute maximum ratings.
- The typical application block diagram (Figure 8-1) is described in
  text.
- From images: OC, PG, ON/OFF, OT, RESET signal names and the gate
  arrangement (Figure 8-1, p.9).
- Source issues:
  - Source conflict: ΔV~T~ hysteresis referenced but not in the
    Electrical Characteristics table (Functional description note).
  - Source conflict: V~OH~/V~OL~ tested at I~OH~ = –50 mA / I~OL~ = 50 mA
    vs ±50 µA in the recommended operating conditions (EC table note).
  - Unconfirmed: I~CC~ test condition printed "V~I~ = V or GND" (EC table
    note).
  - Source error: the recommended operating conditions title carries
    marker 3 for its only note (ROC table note).
