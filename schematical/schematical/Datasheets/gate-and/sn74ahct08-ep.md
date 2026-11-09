---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct08-ep",
  "title": "SN74AHCT08-EP quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahct08-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS490",
    "date": "2003-06",
    "pages": 9
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74AHCT08MDREP", "SN74AHCT08MPWREP", "V62/03654-01YE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-08"
}
---

# SN74AHCT08-EP quadruple 2-input positive-AND gate

## Description

The SN74AHCT08 is a quadruple 2-input positive-AND gate. This device
performs the Boolean function Y = A • B or Y = !{!{A} + !{B}} in positive
logic. Inputs are TTL-voltage compatible.

Enhanced product: controlled baseline (one assembly/test site, one
fabrication site), extended temperature performance of –55 °C to 125 °C,
EPIC (Enhanced-Performance Implanted
CMOS) process. Component qualification in accordance with JEDEC and
industry standards to ensure reliable operation over an extended
temperature range; such qualification testing should not be viewed as
justifying use of this component beyond specified performance and
environmental limits.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [3] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

ESD protection exceeds 2000 V per MIL-STD-883, Method 3015; exceeds
200 V using machine model (C = 200 pF, R = 0). Latch-up performance
exceeds 250 mA per JESD 17.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| ΔI~CC~ | Supply current change [1] | One input at 3.4 V, other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels rather than 0 V or V~CC~.
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 | 6.9 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 6.9 | 1 | 8 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. C~L~ is 15 pF or 50 pF,
as given in the [switching characteristics](#sw). Inputs swing 0 V to
3 V and are measured at 1.5 V; outputs are measured at 50 % V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50 % VCC (output)
```

The source figure also shows the 3-state and open-drain load circuit,
pulse-duration, setup-and-hold and enable/disable waveforms, which do
not apply to this part.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHCT08MDREP | Reel of 2500 | -55 to 125 | AHCT08MEP
SN74AHCT08MPWREP | Tape and reel | -55 to 125 | AHT08EP
V62/03654-01YE | Reel of 2500 | -55 to 125 | AHCT08MEP
```

The package option addendum lists SN74AHCT08MPWREP as obsolete and the
other two as NRND.

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and reel
  and mechanical pages dropped, replaced by the generated Package and
  Ordering sections.
- Dropped: the feature list (its ESD and latch-up ratings are kept as a
  sentence under Absolute maximum ratings), logic symbol and logic
  diagram, the vendor ordering table (its data is in the Ordering block).
- From images: the µA units of the V~OH~/V~OL~ (50 µA), I~I~ and I~CC~
  rows, p.3 (the text layer prints them as mA); the overlines of
  Y = !{!{A} + !{B}}, p.1.
- Source issues: none found.
