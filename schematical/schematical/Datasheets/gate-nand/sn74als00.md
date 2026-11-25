---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74als00",
  "title": "SN54ALS00A, SN54AS00, SN74ALS00A, SN74AS00 quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74als00.pdf",
    "vendor": "Texas Instruments",
    "document": "SDAS187A",
    "date": "1994-12",
    "pages": 25
  },
  "devices": ["gate:74_00"],
  "parts": ["5962-8683301DA", "SN54ALS00AJ", "SN74ALS00AD", "SN74ALS00ADBR", "SN74ALS00ADR", "SN74ALS00AN", "SN74ALS00ANSR", "SNJ54ALS00AJ", "SNJ54ALS00AW", "SN54AS00J", "SN74AS00D", "SN74AS00DR", "SN74AS00N", "SN74AS00NSR", "SNJ54AS00J", "5962-86833012A", "SNJ54ALS00AFK"],
  "packages": {"p14": "D, DB, J, N, NS, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SN54ALS00A, SN54AS00, SN74ALS00A, SN74AS00 quadruple 2-input positive-NAND gates

## Description

These devices contain four independent 2-input positive-NAND gates. They
perform the Boolean functions Y = !{A • B} or Y = !{A} + !{B} in positive
logic. [[1::1]]

The SN54ALS00A and SN54AS00 are characterized for operation over the
full military temperature range of –55°C to 125°C. The SN74ALS00A and
SN74AS00 are characterized for operation from 0°C to 70°C. [[1::2]]

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
[[1::table #1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max-als caption="Absolute maximum ratings, SN54ALS00A and SN74ALS00A [1]" parts="*ALS00*, 5962-86833*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54ALS00A | -55 | 125 | °C |
| ^ | ^ | SN74ALS00A | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```
[[2::table #1]]

```table id=abs-max-as caption="Absolute maximum ratings, SN54AS00 and SN74AS00 [1]" parts="*AS00*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54AS00 | -55 | 125 | °C |
| ^ | ^ | SN74AS00 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```
[[4::table #1]]

## Recommended operating conditions

```table id=roc-als caption="Recommended operating conditions, SN54ALS00A and SN74ALS00A" parts="*ALS00*, 5962-86833*"
| Symbol | Parameter | SN54ALS00A | < | < | SN74ALS00A | < | < | Unit |
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
[[2::table #2]]

```table id=roc-as caption="Recommended operating conditions, SN54AS00 and SN74AS00" parts="*AS00*"
| Symbol | Parameter | SN54AS00 | < | < | SN74AS00 | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 | | | 0.8 | V |
| I~OH~ | High-level output current | | | -2 | | | -2 | mA |
| I~OL~ | Low-level output current | | | 20 | | | 20 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[4::table #2]]

## Electrical characteristics

```table id=ec-als caption="Electrical characteristics, SN54ALS00A and SN74ALS00A" parts="*ALS00*, 5962-86833*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54ALS00A | < | < | SN74ALS00A | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.5 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -0.4 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 4 mA | | 0.25 | 0.4 | | 0.25 | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | | | | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.1 | | | -0.1 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -20 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 0.5 | 0.85 | | 0.5 | 0.85 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 1.5 | 3 | | 1.5 | 3 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```
[[3::table #1]]

```table id=ec-as caption="Electrical characteristics, SN54AS00 and SN74AS00" parts="*AS00*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54AS00 | < | < | SN74AS00 | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -2 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 20 mA | | 0.35 | 0.5 | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.5 | | | -0.5 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -30 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 2 | 3.2 | | 2 | 3.2 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 10.8 | 17.4 | | 10.8 | 17.4 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```
[[4::table #3]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-als caption="Switching characteristics, SN54ALS00A and SN74ALS00A (see Figure 1)" parts="*ALS00*, 5962-86833*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54ALS00A | < | SN74ALS00A | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 3 | 15 | 3 | 11 | ns |
| t~PHL~ | ^ | ^ | 2 | 9 | 2 | 8 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```
[[3::table #2]]

```table id=sw-as caption="Switching characteristics, SN54AS00 and SN74AS00 (see Figure 1)" parts="*AS00*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54AS00 | < | SN74AS00 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1 | 5 | 1 | 4.5 | ns |
| t~PHL~ | ^ | ^ | 1 | 5 | 1 | 4 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```
[[3::table #3]]

## Parameter measurement information

Series 54ALS/74ALS and 54AS/74AS devices. C~L~ includes probe and jig
capacitance. All input pulses have the following characteristics:
PRR ≤ 1 MHz, t~r~ = t~f~ = 2 ns, duty cycle = 50%. The outputs are
measured one at a time with one transition per measurement. [[5::1]]

Conversion note: in the figure the input pulses swing from 0.3 V to 3.5 V and all waveforms are measured at 1.3 V.

```figure ref=load-rl-cl caption="Load circuit for bi-state totem-pole outputs"
RL: 500 Ω
CL: 50 pF
```
[[5::Figure 1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 1.3 V
```
[[5::Figure 1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-8683301DA | Tube of 25 | -55 to 125 | 5962-8683301DA SNJ54ALS00AW
SN54ALS00AJ | Tube of 25 | -55 to 125 | SN54ALS00AJ
SN74ALS00AD | | 0 to 70 | ALS00A | Obsolete
SN74ALS00ADBR | Reel of 2000 | 0 to 70 | G00A | Last Time Buy
SN74ALS00ADR | Reel of 2500 | 0 to 70 | ALS00A
SN74ALS00AN | Tube of 25 | 0 to 70 | SN74ALS00AN
SN74ALS00ANSR | Reel of 2000 | 0 to 70 | ALS00A
SNJ54ALS00AJ | Tube of 25 | -55 to 125 | SNJ54ALS00AJ
SNJ54ALS00AW | Tube of 25 | -55 to 125 | 5962-8683301DA SNJ54ALS00AW
SN54AS00J | Tube of 25 | -55 to 125 | SN54AS00J
SN74AS00D | | 0 to 70 | AS00 | Obsolete
SN74AS00DR | Reel of 2500 | 0 to 70 | AS00
SN74AS00N | Tube of 25 | 0 to 70 | SN74AS00N
SN74AS00NSR | Reel of 2000 | 0 to 70 | 74AS00
SNJ54AS00J | Tube of 25 | -55 to 125 | SNJ54AS00J
5962-86833012A | Tube of 55 | -55 to 125 | 5962-86833012A SNJ54ALS00AFK
SNJ54ALS00AFK | Tube of 55 | -55 to 125 | 5962-86833012A SNJ54ALS00AFK
```
[[6::table #1]] [[7::table #1]]

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6–8), tape
  and reel, tube and mechanical pages (9–25) dropped, replaced by the
  generated Package and Ordering sections. The addendum also lists
  JM38510 / M38510 military parts the catalog doesn't carry.
- Dropped: feature list (package options), the logic symbol and logic
  diagram (four 2-input NAND gates), and the parts of Figure 1 that
  don't apply to these totem-pole parts: the open-collector and 3-state
  load circuits, setup/hold, pulse-duration and enable/disable
  waveforms, and Notes B and C (3-state outputs).
- The electrical characteristics tables of the source name their
  parameters by symbol only; no parameter names are added.
- The DB, NS and W packages are named only in the package option
  addendum; page 1 draws the D, J, N and FK packages.
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1).
- Source issues: none found.
