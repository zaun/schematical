---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74als02",
  "title": "SN54ALS02A, SN54AS02, SN74ALS02A, SN74AS02 quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74als02.pdf",
    "vendor": "Texas Instruments",
    "document": "SDAS111B",
    "date": "1994-12",
    "pages": 22
  },
  "devices": ["gate:74_02"],
  "parts": ["5962-8684401DA", "SN54ALS02AJ", "SN74ALS02AD", "SN74ALS02ADR", "SN74ALS02AN", "SN74ALS02ANSR", "SNJ54ALS02AJ", "SNJ54ALS02AW", "SN54AS02J", "SN74AS02DR", "SN74AS02N", "SN74AS02NSR", "SNJ54AS02J", "5962-86844012A", "SNJ54ALS02AFK", "SNJ54AS02FK"],
  "packages": {"p14": "D, J, N, NS, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SN54ALS02A, SN54AS02, SN74ALS02A, SN74AS02 quadruple 2-input positive-NOR gates

## Description

These devices contain four independent 2-input positive-NOR gates. They
perform the Boolean functions Y = !{A + B} or Y = !{A} • !{B} in positive
logic.

The SN54ALS02A and SN54AS02 are characterized for operation over the
full military temperature range of –55°C to 125°C. The SN74ALS02A and
SN74AS02 are characterized for operation from 0°C to 70°C.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Positive supply
NC: No internal connection
```

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
```

## Absolute maximum ratings

```table id=abs-max-als caption="Absolute maximum ratings, SN54ALS02A and SN74ALS02A [1]" parts="*ALS02*, 5962-86844*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54ALS02A | -55 | 125 | °C |
| ^ | ^ | SN74ALS02A | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```

```table id=abs-max-as caption="Absolute maximum ratings, SN54AS02 and SN74AS02 [1]" parts="*AS02*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54AS02 | -55 | 125 | °C |
| ^ | ^ | SN74AS02 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```

## Recommended operating conditions

```table id=roc-als caption="Recommended operating conditions, SN54ALS02A and SN74ALS02A" parts="*ALS02*, 5962-86844*"
| Symbol | Parameter | SN54ALS02A | < | < | SN74ALS02A | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 [1] | | | 0.8 | V |
| ^ | ^ | | | 0.7 [2] | | | | V |
| I~OH~ | High-level output current | | | -0.4 | | | -0.4 | mA |
| I~OL~ | Low-level output current | | | 4 | | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |

[1] Applies over temperature range –55°C to 70°C
[2] Applies over temperature range 70°C to 125°C
```

```table id=roc-as caption="Recommended operating conditions, SN54AS02 and SN74AS02" parts="*AS02*"
| Symbol | Parameter | SN54AS02 | < | < | SN74AS02 | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 | | | 0.8 | V |
| I~OH~ | High-level output current | | | -2 | | | -2 | mA |
| I~OL~ | Low-level output current | | | 20 | | | 20 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```

## Electrical characteristics

```table id=ec-als caption="Electrical characteristics, SN54ALS02A and SN74ALS02A" parts="*ALS02*, 5962-86844*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54ALS02A | < | < | SN74ALS02A | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.5 | | | -1.5 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -0.4 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 4 mA | | 0.25 | 0.4 | | 0.25 | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | | | | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.1 | | | -0.1 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -20 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 0.86 | 2.2 | | 0.86 | 2.2 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 2.16 | 4 | | 2.16 | 4 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```

```table id=ec-as caption="Electrical characteristics, SN54AS02 and SN74AS02" parts="*AS02*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54AS02 | < | < | SN74AS02 | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -2 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 20 mA | | 0.35 | 0.5 | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.5 | | | -0.5 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -30 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 3.7 | 5.9 | | 3.7 | 5.9 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 12.5 | 20.1 | | 12.5 | 20.1 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-als caption="Switching characteristics, SN54ALS02A and SN74ALS02A (see Figure 1)" parts="*ALS02*, 5962-86844*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54ALS02A | < | SN74ALS02A | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1 | 16 | 1 | 12 | ns |
| t~PHL~ | ^ | ^ | 1 | 11 | 1 | 10 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```

```table id=sw-as caption="Switching characteristics, SN54AS02 and SN74AS02 (see Figure 1)" parts="*AS02*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54AS02 | < | SN74AS02 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1 | 6 | 1 | 4.5 | ns |
| t~PHL~ | ^ | ^ | 1 | 5 | 1 | 4.5 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```

## Parameter measurement information

Series 54ALS/74ALS and 54AS/74AS devices. C~L~ includes probe and jig
capacitance. All input pulses have the following characteristics:
PRR ≤ 1 MHz, t~r~ = t~f~ = 2 ns, duty cycle = 50%. The outputs are
measured one at a time with one transition per measurement.

Conversion note: in the figure the input pulses swing from 0.3 V to 3.5 V and all waveforms are measured at 1.3 V.

```figure ref=load-rl-cl caption="Load circuit for bi-state totem-pole outputs"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 1.3 V
```

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-8684401DA | Tube of 25 | -55 to 125 | 5962-8684401DA SNJ54ALS02AW
SN54ALS02AJ | Tube of 25 | -55 to 125 | SN54ALS02AJ
SN74ALS02AD | | 0 to 70 | ALS02A | Obsolete
SN74ALS02ADR | Reel of 2500 | 0 to 70 | ALS02A
SN74ALS02AN | Tube of 25 | 0 to 70 | SN74ALS02AN
SN74ALS02ANSR | Reel of 2000 | 0 to 70 | ALS02A
SNJ54ALS02AJ | Tube of 25 | -55 to 125 | SNJ54ALS02AJ
SNJ54ALS02AW | Tube of 25 | -55 to 125 | 5962-8684401DA SNJ54ALS02AW
SN54AS02J | Tube of 25 | -55 to 125 | SN54AS02J
SN74AS02DR | Reel of 2500 | 0 to 70 | AS02
SN74AS02N | Tube of 25 | 0 to 70 | SN74AS02N
SN74AS02NSR | Reel of 2000 | 0 to 70 | 74AS02
SNJ54AS02J | Tube of 25 | -55 to 125 | SNJ54AS02J
5962-86844012A | Tube of 55 | -55 to 125 | 5962-86844012A SNJ54ALS02AFK
SNJ54ALS02AFK | Tube of 55 | -55 to 125 | 5962-86844012A SNJ54ALS02AFK
SNJ54AS02FK | Tube of 55 | -55 to 125 | SNJ54AS02FK
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6–8), tape
  and reel, tube and mechanical pages (9–22) dropped, replaced by the
  generated Package and Ordering sections. The addendum also lists
  JM38510 / M38510 military parts the catalog doesn't carry.
- Dropped: feature list (package options), the logic symbol and logic
  diagram (four 2-input NOR gates), and the parts of Figure 1 that
  don't apply to these totem-pole parts: the open-collector and 3-state
  load circuits, setup/hold, pulse-duration and enable/disable
  waveforms, and Notes B and C (3-state outputs).
- The electrical characteristics tables of the source name their
  parameters by symbol only; no parameter names are added.
- From images: overbars of Y = !{A + B} and Y = !{A} • !{B} (p.1).
- Source issues: none found.
