---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74als08",
  "title": "SN54ALS08, SN54AS08, SN74ALS08, SN74AS08 quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74als08.pdf",
    "vendor": "Texas Instruments",
    "document": "SDAS191A",
    "date": "1994-12",
    "pages": 22
  },
  "devices": ["gate:74_08"],
  "parts": ["5962-8684201CA", "5962-8684201DA", "SN54ALS08J", "SN74ALS08D", "SN74ALS08DR", "SN74ALS08DRG4", "SN74ALS08N", "SN74ALS08NE4", "SN74ALS08NSR", "SNJ54ALS08J", "SNJ54ALS08W", "SN54AS08J", "SN74AS08D", "SN74AS08DR", "SN74AS08N", "SN74AS08NSR", "SNJ54AS08J", "SNJ54AS08W", "5962-86842012A", "SNJ54ALS08FK", "SNJ54AS08FK"],
  "packages": {"p14": "D, J, N, NS, W", "lccc20": "FK"},
  "converted": "2026-11-08"
}
---

# SN54ALS08, SN54AS08, SN74ALS08, SN74AS08 quadruple 2-input positive-AND gates

## Description

These devices contain four independent 2-input positive-AND gates. They
perform the Boolean functions Y = A • B or Y = !{!{A} + !{B}} in positive
logic. Source error: the source prints the bar over the whole sum
shifted to the left and a line up, under "They perform", so that the
formula reads Y = !{A} + !{B}; the bar is written over the sum, which
is the function Y = A • B.

The SN54ALS08 and SN54AS08 are characterized for operation over the
full military temperature range of –55 °C to 125 °C. The SN74ALS08 and
SN74AS08 are characterized for operation from 0 °C to 70 °C.

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
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max-als caption="Absolute maximum ratings, SN54ALS08 and SN74ALS08 [1]" parts="*ALS08*, 5962-86842*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54ALS08 | -55 | 125 | °C |
| ^ | ^ | SN74ALS08 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```

```table id=abs-max-as caption="Absolute maximum ratings, SN54AS08 and SN74AS08 [1]" parts=*AS08* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54AS08 | -55 | 125 | °C |
| ^ | ^ | SN74AS08 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```

## Recommended operating conditions

```table id=roc-als caption="Recommended operating conditions, SN54ALS08 and SN74ALS08" parts="*ALS08*, 5962-86842*"
| Symbol | Parameter | SN54ALS08 | < | < | SN74ALS08 | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 [1] | | | 0.8 | V |
| ^ | ^ | | | 0.7 [2] | | | | V |
| I~OH~ | High-level output current | | | -0.4 | | | -0.4 | mA |
| I~OL~ | Low-level output current | | | 4 | | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |

[1] Applies over temperature range –55 °C to 70 °C.
[2] Applies over temperature range 70 °C to 125 °C.
```

```table id=roc-as caption="Recommended operating conditions, SN54AS08 and SN74AS08" parts=*AS08*
| Symbol | Parameter | SN54AS08 | < | < | SN74AS08 | < | < | Unit |
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

```table id=ec-als caption="Electrical characteristics, SN54ALS08 and SN74ALS08" parts="*ALS08*, 5962-86842*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54ALS08 | < | < | SN74ALS08 | < | < | Unit |
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
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 1.3 | 2.4 | | 1.3 | 2.4 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 0 | | 2.2 | 4 | | 2.2 | 4 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25 °C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```

```table id=ec-as caption="Electrical characteristics, SN54AS08 and SN74AS08" parts=*AS08* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54AS08 | < | < | SN74AS08 | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -2 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 20 mA | | 0.35 | 0.5 | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.5 | | | -0.5 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -30 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 5.8 | 9.3 | | 5.8 | 9.3 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 0 | | 14.9 | 24 | | 14.9 | 24 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25 °C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-als caption="Switching characteristics, SN54ALS08 and SN74ALS08 (see Figure 1)" parts="*ALS08*, 5962-86842*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54ALS08 | < | SN74ALS08 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 2 | 14 | 4 | 14 | ns |
| t~PHL~ | A or B | Y | 2 | 12.5 | 3 | 10 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```

```table id=sw-as caption="Switching characteristics, SN54AS08 and SN74AS08 (see Figure 1)" parts=*AS08* conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54AS08 | < | SN74AS08 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 1 | 6.5 | 1 | 5.5 | ns |
| t~PHL~ | A or B | Y | 1 | 6.5 | 1 | 5.5 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```

## Parameter measurement information

Series 54ALS/74ALS and 54AS/74AS devices. C~L~ includes probe and jig
capacitance. All input pulses have the following characteristics:
PRR ≤ 1 MHz, t~r~ = t~f~ = 2 ns, duty cycle = 50 %. The outputs are
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
5962-8684201CA | Tube of 25 | -55 to 125 | 5962-8684201CA SNJ54ALS08J
5962-8684201DA | Tube of 25 | -55 to 125 | 5962-8684201DA SNJ54ALS08W
SN54ALS08J | Tube of 25 | -55 to 125 | SN54ALS08J
SN74ALS08D | | 0 to 70 | ALS08 | Obsolete
SN74ALS08DR | Reel of 2500 | 0 to 70 | ALS08
SN74ALS08DRG4 | Reel of 2500 | 0 to 70 | ALS08
SN74ALS08N | Tube of 25 | 0 to 70 | SN74ALS08N
SN74ALS08NE4 | Tube of 25 | 0 to 70 | SN74ALS08N
SN74ALS08NSR | Reel of 2000 | 0 to 70 | ALS08
SNJ54ALS08J | Tube of 25 | -55 to 125 | 5962-8684201CA SNJ54ALS08J
SNJ54ALS08W | Tube of 25 | -55 to 125 | 5962-8684201DA SNJ54ALS08W
SN54AS08J | Tube of 25 | -55 to 125 | SN54AS08J
SN74AS08D | | 0 to 70 | AS08 | Obsolete
SN74AS08DR | Reel of 2500 | 0 to 70 | AS08
SN74AS08N | Tube of 25 | 0 to 70 | SN74AS08N
SN74AS08NSR | Reel of 2000 | 0 to 70 | 74AS08
SNJ54AS08J | Tube of 25 | -55 to 125 | SNJ54AS08J
SNJ54AS08W | Tube of 25 | -55 to 125 | SNJ54AS08W
5962-86842012A | Tube of 55 | -55 to 125 | 5962-86842012A SNJ54ALS08FK
SNJ54ALS08FK | Tube of 55 | -55 to 125 | 5962-86842012A SNJ54ALS08FK
SNJ54AS08FK | Tube of 55 | -55 to 125 | SNJ54AS08FK
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum, tape and
  reel and mechanical pages (6–22) dropped, replaced by the generated
  Package and Ordering sections. Ordering packing, temperature and
  marking columns are from the package option addendum (pages 6–7),
  which also lists JM38510 / M38510 military parts the catalog doesn't
  carry.
- Dropped: feature list, the logic symbol and logic diagram (four
  2-input AND gates), and the parts of Figure 1 that don't apply to
  these totem-pole parts: the open-collector and 3-state load circuits,
  setup/hold, pulse-duration and enable/disable waveforms, and Notes B
  and C (3-state outputs).
- The electrical characteristics tables of the source name their
  parameters by symbol only; no parameter names are added.
- From images: overbars of Y = !{!{A} + !{B}} (p.1).
- Source issues:
  - Source error: the bar over the whole sum of Y = !{!{A} + !{B}} is
    printed out of place, under "They perform" on the line above (p.1),
    which leaves Y = !{A} + !{B}, an OR function. Written with the bar
    over the sum, as Y = A • B and the function table require.
