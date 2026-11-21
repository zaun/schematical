---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc02-ep",
  "title": "SN74AHC02-EP quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahc02-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS480",
    "date": "2003-05",
    "pages": 9
  },
  "devices": ["gate:74_02"],
  "parts": ["SN74AHC02MPWREP", "V62/03645-01XE"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74AHC02-EP quadruple 2-input positive-NOR gates

## Description

The ’AHC02 device contains four independent 2-input NOR gates that
perform the Boolean function Y = !{A} • !{B} or Y = !{A + B} in positive
logic.

Controlled Baseline. Extended Temperature Performance of –55°C to 125°C.
Operating Range 2-V to 5.5-V V~CC~. Latch-Up Performance Exceeds 250 mA
Per JESD 17. ESD Protection Exceeds JESD 22.

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits.

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
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
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
| θ~JA~ | Package thermal impedance [3] | | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
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
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN74AHC02-EP | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | pF |
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.9 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 15 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74AHC02-EP | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5.6 | 7.9 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.6 | 7.9 | 1 | 9.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8.1 | 11.4 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.1 | 11.4 | 1 | 13 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN74AHC02-EP | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 3.6 | 5.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.6 | 5.5 | 1 | 6.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.1 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.1 | 7.5 | 1 | 8.5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```

Conversion note: the source figure (Figure 1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHC02MPWREP | Reel of 2000 | -55 to 125 | AHC02EP
V62/03645-01XE | Reel of 2000 | -55 to 125 | AHC02EP
```

## Conversion notes

- Source pages 1–4 converted. The ordering lines are from the package
  option addendum (pp.5–6). Tape and reel and mechanical pages (7–9) are
  left out; the generated Package section replaces them.
- Dropped: the feature list (kept in part as sentences in the
  Description), the logic diagram (four NOR gates with the PW pin
  numbers), the ordering table on p.1 (it gives "Tape and reel"; the
  Ordering block follows the package option addendum).
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the µA units of the I~OH~/I~OL~ (V~CC~ = 2 V) rows (p.2),
  the I~OH~ = –50 µA and I~OL~ = 50 µA conditions and the units of the
  I~I~ and I~CC~ rows (p.3; the text layer prints them as mA); the dot
  and the overbars of Y = !{A} • !{B} and Y = !{A + B} (p.1: a bar over
  A, a bar over B, and one bar over the whole sum; the text layer prints
  the dot as "S").
- Source issues: none found.
