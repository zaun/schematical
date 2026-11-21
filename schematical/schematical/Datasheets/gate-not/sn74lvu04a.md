---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvu04a",
  "title": "SN54LVU04A, SN74LVU04A hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvu04a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES130L",
    "date": "2004-12",
    "pages": 20
  },
  "devices": ["gate:74_U04"],
  "parts": ["SN74LVU04AD", "SN74LVU04ADBR", "SN74LVU04ADR", "SN74LVU04ANSR", "SN74LVU04APW", "SN74LVU04APWR", "SN74LVU04APWT"],
  "packages": {"p14": "D, DB, DGV, NS, PW, J, W"},
  "converted": "2026-11-21"
}
---

# SN54LVU04A, SN74LVU04A hex inverters

## Description

These hex inverters are designed for 2-V to 5.5-V V~CC~ operation.

The ’LVU04A devices contain six independent inverters with unbuffered
outputs. These devices perform the Boolean function Y = !{A}.

Unbuffered Outputs. Max t~pd~ of 6.5 ns at 5 V. Typical V~OLP~ (Output
Ground Bounce) <0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~
(Output V~OH~ Undershoot) >2.3 V at V~CC~ = 3.3 V, T~A~ = 25°C. Support
Mixed-Mode Voltage Operation on All Ports. Latch-Up Performance Exceeds
250 mA Per JESD 17. ESD Protection Exceeds JESD 22. 2000-V Human-Body
Model (A114-A). 200-V Machine Model (A115-A). 1000-V Charged-Device
Model (C101).

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y (unbuffered)
GND: Ground
VCC: Supply voltage
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

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [4] | D package | | 86 | °C/W |
| ^ | ^ | DB package | | 96 | °C/W |
| ^ | ^ | DGV package | | 127 | °C/W |
| ^ | ^ | NS package | | 76 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| ^ | ^ | RGY package [5] | | 47 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
[5] The package thermal impedance is calculated in accordance with JESD 51-5.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | SN54LVU04A | < | SN74LVU04A | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.7 | | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.8 | | V~CC~ × 0.8 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.8 | | V~CC~ × 0.8 | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.8 | | V~CC~ × 0.8 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.3 | | 0.3 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.2 | | V~CC~ × 0.2 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.2 | | V~CC~ × 0.2 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.2 | | V~CC~ × 0.2 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | | -6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | | -12 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | | 6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | | 12 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | SN54LVU04A | < | < | SN74LVU04A | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = −2 mA | 2.3 V | 2 | | | 2 | | | ^ |
| ^ | ^ | I~OH~ = −6 mA | 3 V | 2.48 | | | 2.48 | | | ^ |
| ^ | ^ | I~OH~ = −12 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±1 | | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | | 4 | | pF |
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74LVU04A | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.5 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 5.6 | pF |
| ^ | ^ | ^ | 5 V | 6.7 | ^ |
```

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 2.5 V ± 0.2 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LVU04A | < | SN74LVU04A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 3.2 [1] | 10.9 [1] | 1 [1] | 14 [1] | 1 | 14 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 6.6 | 13.4 | 1 | 16 | 1 | 16 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LVU04A | < | SN74LVU04A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 2.5 [1] | 8.9 [1] | 1 [1] | 10.5 [1] | 1 | 10.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 4.7 | 11.4 | 1 | 13 | 1 | 13 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54LVU04A | < | SN74LVU04A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15 pF | | 2.2 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 3.9 | 7 | 1 | 8 | 1 | 8 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices.

```figure ref=load-cmos-50pf caption="Figure 1. Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Figure 1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: the source figure (Figure 1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms with their
notes B, E and F, which are not drawn here.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LVU04AD | | -40 to 85 | LVU04A | Obsolete
SN74LVU04ADBR | Reel of 2000 | -40 to 85 | LU04A
SN74LVU04ADR | Reel of 2500 | -40 to 85 | LVU04A
SN74LVU04ANSR | Reel of 2000 | -40 to 85 | LVU04A
SN74LVU04APW | | -40 to 85 | LU04A | Obsolete
SN74LVU04APWR | Reel of 2000 | -40 to 85 | LU04A
SN74LVU04APWT | | -40 to 85 | LU04A | Obsolete
```

## Conversion notes

- Source pages 1–5 converted. The ordering lines are from the package
  option addendum (p.6; it ends on p.7). Tape and reel and mechanical
  pages (8–20) are left out; the generated Package section replaces
  them.
- Dropped: the logic diagram (one inverter, input A, output Y), the
  ordering table on p.1 (the Ordering block follows the package option
  addendum).
- The catalog does not carry the SN54LVU04A parts (SNJ54LVU04AJ,
  SNJ54LVU04AW, SNJ54LVU04AFK), SN74LVU04ARGYR and SN74LVU04ADGVR of the
  ordering table on p.1, or SN74LVU04APWRG4 of the addendum, and the
  claimed parts use no RGY or FK pinout; the RGY and FK pin drawings
  (p.1) are not converted. The SN54LVU04A columns of the tables are
  kept; pages 3 and 4 print "PRODUCT PREVIEW" across those columns.
- The feature list is written as sentences in the Description.
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells. The
  source prints "(see Note 3)" on each package line from D to PW and
  "(see Note 4)" on the RGY line; its notes 1 to 4 are notes 2 to 5
  here.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the overbar of Y = !{A} (p.1).
- Source issues: none found.
