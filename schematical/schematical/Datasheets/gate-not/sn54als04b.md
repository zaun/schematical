---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn54als04b",
  "title": "SN54ALS04B, SN54AS04, SN74ALS04B, SN74AS04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn54als04b.pdf",
    "vendor": "Texas Instruments",
    "document": "SDAS063B",
    "date": "1994-12",
    "pages": 25
  },
  "devices": ["gate:74_04"],
  "parts": ["5962-86843012A", "5962-8684301DA", "M38510/37006B2A", "M38510/37006BCA", "SN54ALS04BJ", "SN54AS04J", "SN74ALS04BDRG4", "SN74ALS04BNSRE4", "SNJ54ALS04BFK", "SNJ54ALS04BJ", "SNJ54ALS04BW", "SNJ54AS04FK", "SNJ54AS04J"],
  "packages": {"p14": "D, J, N, NS, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SN54ALS04B, SN54AS04, SN74ALS04B, SN74AS04 hex inverters

## Description

These devices contain six independent hex inverters. They perform the
Boolean function Y = !{A}.

The SN54ALS04B and SN54AS04 are characterized for operation over the
full military temperature range of −55°C to 125°C. The SN74ALS04B and
SN74AS04 are characterized for operation from 0°C to 70°C.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
GND: Ground
VCC: Positive supply
NC: No internal connection
```

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max-als caption="Absolute maximum ratings, SN54ALS04B and SN74ALS04B [1]" parts="*ALS04B*, 5962-8684301*, M38510/37006*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54ALS04B | -55 | 125 | °C |
| ^ | ^ | SN74ALS04B | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```

```table id=abs-max-as caption="Absolute maximum ratings, SN54AS04 and SN74AS04 [1]" parts="SN54AS04*, SNJ54AS04*" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54AS04 | -55 | 125 | °C |
| ^ | ^ | SN74AS04 | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```

## Recommended operating conditions

```table id=roc-als caption="Recommended operating conditions, SN54ALS04B and SN74ALS04B" parts="*ALS04B*, 5962-8684301*, M38510/37006*"
| Symbol | Parameter | SN54ALS04B | < | < | SN74ALS04B | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 [1] | | | 0.8 | V |
| ^ | ^ | | | 0.7 [2] | | | | V |
| I~OH~ | High-level output current | | | -0.4 | | | -0.4 | mA |
| I~OL~ | Low-level output current | | | 4 | | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |

[1] Applies over −55°C to 70°C temperature range
[2] Applies over 70°C to 125°C temperature range
```

```table id=roc-as caption="Recommended operating conditions, SN54AS04 and SN74AS04" parts="SN54AS04*, SNJ54AS04*"
| Symbol | Parameter | SN54AS04 | < | < | SN74AS04 | < | < | Unit |
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

```table id=ec-als caption="Electrical characteristics, SN54ALS04B and SN74ALS04B" parts="*ALS04B*, 5962-8684301*, M38510/37006*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54ALS04B | < | < | SN74ALS04B | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -0.4 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 4 mA | | 0.25 | 0.4 | | 0.25 | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | | | | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.1 | | | -0.1 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -20 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 0.65 | 1.1 | | 0.65 | 1.1 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 2.9 | 4.4 | | 2.9 | 4.2 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```

```table id=ec-as caption="Electrical characteristics, SN54AS04 and SN74AS04" parts="SN54AS04*, SNJ54AS04*" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54AS04 | < | < | SN74AS04 | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OH~ | V~CC~ = 4.5 V to 5.5 V | I~OH~ = -2 mA | V~CC~ - 2 | | | V~CC~ - 2 | | | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 20 mA | | 0.35 | 0.5 | | 0.35 | 0.5 | V |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.5 | | | -0.5 | mA |
| I~O~ [2] | V~CC~ = 5.5 V | V~O~ = 2.25 V | -30 | | -112 | -30 | | -112 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 3 | 4.8 | | 3 | 4.8 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 14 | 26.3 | | 14 | 26.3 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[2] The output conditions have been chosen to produce a current that closely approximates one half of the true short-circuit output current, I~OS~.
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-als caption="Switching characteristics, SN54ALS04B and SN74ALS04B (see Figure 1)" parts="*ALS04B*, 5962-8684301*, M38510/37006*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54ALS04B | < | SN74ALS04B | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 3 | 17 | 3 | 11 | ns |
| t~PHL~ | ^ | ^ | 2 | 13 | 2 | 8 | ns |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```

```table id=sw-as caption="Switching characteristics, SN54AS04 and SN74AS04 (see Figure 1)" parts="SN54AS04*, SNJ54AS04*" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 500 Ω, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54AS04 | < | SN74AS04 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 1 | 6 | 1 | 5 | ns |
| t~PHL~ | ^ | ^ | 1 | 4.5 | 1 | 4 | ns |

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
5962-86843012A | Tube of 55 | -55 to 125 | 5962-86843012A SNJ54ALS04BFK
5962-8684301DA | Tube of 25 | -55 to 125 | 5962-8684301DA SNJ54ALS04BW
M38510/37006B2A | Tube of 55 | -55 to 125 | JM38510/37006B2A
M38510/37006BCA | Tube of 25 | -55 to 125 | JM38510/37006BCA
SN54ALS04BJ | Tube of 25 | -55 to 125 | SN54ALS04BJ
SN54AS04J | Tube of 25 | -55 to 125 | SN54AS04J
SN74ALS04BDRG4 | Reel of 2500 | 0 to 70 | ALS04B
SN74ALS04BNSRE4 | Reel of 2000 | 0 to 70 | ALS04B
SNJ54ALS04BFK | Tube of 55 | -55 to 125 | 5962-86843012A SNJ54ALS04BFK
SNJ54ALS04BJ | Tube of 25 | -55 to 125 | SNJ54ALS04BJ
SNJ54ALS04BW | Tube of 25 | -55 to 125 | 5962-8684301DA SNJ54ALS04BW
SNJ54AS04FK | Tube of 55 | -55 to 125 | SNJ54AS04FK
SNJ54AS04J | Tube of 25 | -55 to 125 | SNJ54AS04J
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6–8), tape
  and reel, tube and mechanical pages (9–25) dropped, replaced by the
  generated Package and Ordering sections. The addendum also lists
  the JM38510/37006 parts and further SN74ALS04B and SN74AS04
  orderables that this file does not claim.
- The M38510/37006 parts are filed with the SN54ALS04B tables. The
  source does not say which device they are; the addendum only lists
  them.
- The W (CFP) package of 5962-8684301DA and SNJ54ALS04BW is listed in
  the package option addendum only; the source's pin drawings name the
  J, D, N and FK packages.
- Dropped: feature list (package options), the logic symbol and logic
  diagram (six inverters), and the parts of Figure 1 that don't apply
  to these totem-pole parts: the open-collector and 3-state load
  circuits, setup/hold, pulse-duration and enable/disable waveforms,
  and Notes B and C (3-state outputs).
- The electrical characteristics tables of the source name their
  parameters by symbol only; no parameter names are added.
- The page headers with the device names are not in the PDF text layer;
  the title is read from the page image (p.1).
- From images: overbar of Y = !{A} (p.1).
- Source issues: none found.
