---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc04",
  "title": "SNx4AHC04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS231P",
    "date": "2023-06",
    "pages": 38
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74AHC04BQAR", "SN74AHC04D", "SN74AHC04DBR", "SN74AHC04DGVR", "SN74AHC04DR", "SN74AHC04N", "SN74AHC04NSR", "SN74AHC04PW", "SN74AHC04PWR", "SN74AHC04RGYR"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W"},
  "converted": "2026-11-21"
}
---

# SNx4AHC04 hex inverters

## Description

The 'AHC04 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

Operating range of 2 V to 5.5 V. Latch-up performance exceeds 250 mA
per JESD 17.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1Y: 1Y Output
2A: 2A Input
2Y: 2Y Output
3A: 3A Input
3Y: 3Y Output
4A: 4A Input
4Y: 4Y Output
5A: 5A Input
5Y: 5Y Output
6A: 6A Input
6Y: 6Y Output
GND: Ground Pin
VCC: Power Pin
```

Source error: the pin functions table (Table 4-1, p.3) prints I in the
I/O column of the 5Y row; its description is 5Y Output and the other
five Y rows print O.

## Functional description

Conversion note: the logic diagram, each gate (positive logic)
(Figure 2-1 and the functional block diagram), shows one inverter with
input A and output Y.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0) | | -20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~OK~ [3] | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Unconfirmed: the source prints the symbol I~OK~ for the continuous output current as well as for the output clamp current; kept as printed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | SN54AHC04 | < | SN74AHC04 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~= 2 V | 1.5 | | 1.5 | | V |
| ^ | ^ | V~CC~= 3V | 2.1 | | 2.1 | | ^ |
| ^ | ^ | V~CC~= 5.5 V | 3.85 | | 3.85 | | ^ |
| V~IL~ | Low-level Input voltage | V~CC~= 2 V | | 0.5 | | 0.5 | V |
| ^ | ^ | V~CC~= 3 V | | 0.9 | | 0.9 | ^ |
| ^ | ^ | V~CC~= 5.5 V | | 1.65 | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~= 2 V [1] | | -50 | | -50 | mA |
| ^ | ^ | V~CC~= 3.3 V ± 0.3 V | | -4 | | -4 | ^ |
| ^ | ^ | V~CC~= 5 V ± 0.5 V | | -8 | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~= 2 V [1] | | 50 | | 50 | mA |
| ^ | ^ | V~CC~= 3.3 V ± 0.3 V | | 4 | | 4 | ^ |
| ^ | ^ | V~CC~= 5 V ± 0.5 V | | 8 | | 8 | ^ |
| Δt/Δv | Input Transition rise or fall rate | V~CC~= 3.3 V ± 0.3 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~= 5 V ± 0.5 V | | 20 | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 125 | °C |

[1] Source error: the source prints the unit mA for the V~CC~ = 2 V rows, kept as printed; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = –50 µA and I~OL~ = 50 µA.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SNx4AHC04 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | D 14 pins | DB 14 pins | DGV 14 pins | N 14 pins | NS 14 pins | PW 14 pins | RGY 14 pins | BQA 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 96 | 127 | 80 | 76 | 147.7 | 47 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –55°C to 125°C, SN54AHC04 | < | T~A~ = –40°C to 85°C, SN74AHC04 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC04 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | 2.9 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | 4.4 | | ^ |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | 2.48 | | ^ |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA [2] | 3 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 8 mA [2] | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | | 10 | | | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] Source error: printed as I~OH~ = 4 mA and I~OH~ = 8 mA; the rows give V~OL~, and the recommended operating conditions give I~OL~ = 4 mA and 8 mA, so the condition is I~OL~.
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74AHC04 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.8 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC04 | < | T~A~ = –40°C to 85°C, SN74AHC04 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC04 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | 5 [1] | 8.9 [1] | 1 [1] | 10.5 [1] | 1 | 10.5 | 1 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5 [1] | 8.9 [1] | 1 [1] | 10.5 [1] | 1 | 10.5 | 1 | 10.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | 7.5 | 11.4 | 1 | 13 | 1 | 13 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | 7.5 | 11.4 | 1 | 13 | 1 | 13 | 1 | 13 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Test conditions | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC04 | < | T~A~ = –40°C to 85°C, SN74AHC04 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC04 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | 3.8 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 3.8 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 6.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | 5.3 | 7.5 | 1 | 8.5 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.3 | 7.5 | 1 | 8.5 | 1 | 8.5 | 1 | 8.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance.

All input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns.

The outputs are measured one at a time with one input transition per
measurement.

All parameters and waveforms are not applicable to all devices.

```figure ref=load-cmos-50pf caption="Figure 6-1. Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Figure 6-1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms with their
note B, which are not drawn here.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHC04BQAR | Reel of 3000 | -40 to 125 | AHC04
SN74AHC04D | | -40 to 125 | AHC04 | Obsolete
SN74AHC04DBR | Reel of 2000 | -40 to 125 | HA04
SN74AHC04DGVR | Reel of 2000 | -40 to 125 | HA04 | Last Time Buy
SN74AHC04DR | Reel of 2500 | -40 to 125 | AHC04
SN74AHC04N | Tube of 25 | -40 to 125 | SN74AHC04N
SN74AHC04NSR | Reel of 2000 | -40 to 125 | AHC04 | NRND
SN74AHC04PW | | -40 to 125 | HA04 | Obsolete
SN74AHC04PWR | Reel of 2000 | -40 to 125 | HA04
SN74AHC04RGYR | Reel of 3000 | -40 to 125 | HA04
```

## Conversion notes

- Source pages 1 and 3–8 converted; the contents and revision history
  page (2), the support page (9), the package option addendum, tape and
  reel and mechanical pages (10–38) dropped, replaced by the generated
  Package and Ordering sections. The ordering lines are the parts' rows
  of the package option addendum (p.10).
- Dropped: device information table, the SN54AHC04 FK pinout drawing
  and the FK and NC rows of the pin functions table (no FK part is in
  the catalog); the logic diagram (Figure 2-1, p.1, and the functional
  block diagram, p.8) is described in text.
- The catalog does not carry the SN54AHC04 parts (5962-9680501Q2A,
  5962-9680501QCA, 5962-9680501QDA, SNJ54AHC04FK, SNJ54AHC04J,
  SNJ54AHC04W) of the package option addendum; the SN54AHC04 columns of
  the tables are kept.
- The source has no application chapter.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. The recommended operating
  conditions print no conditions line.
- In the 15 pF rows of the 5-V switching table the text layer runs the
  note marker into the values (3.81, 5.51, 11, 6.51).
- Figure 6-1 (p.7) is a picture; only its notes are in the text layer.
  The load figure prints no C~L~ value; the switching tables give
  C~L~ = 15 pF and 50 pF.
- From images: overbar of Y = !{A} (Description, p.1); 50% V~CC~
  (V~M~, Figure 6-1, p.7); 3.8, 5.5, 1 and 6.5 with their note marker
  (5-V switching table, p.6).
- Source issues:
  - Source error: the pin functions table prints I for the 5Y output
    (p.3); see Pin functions.
  - Unconfirmed: I~OK~ printed as the symbol of the continuous output
    current (absolute maximum ratings note 3).
  - Source error: I~OH~/I~OL~ at V~CC~ = 2 V printed in mA, kept as
    printed (recommended operating conditions note 1).
  - Source error: V~OL~ rows printed with I~OH~ = 4 mA / 8 mA; corrected
    to I~OL~ (electrical characteristics note 2).
