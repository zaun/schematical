---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvu04a-q1",
  "title": "SN74LVU04A-Q1 hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvu04a-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES649A",
    "date": "2008-04",
    "pages": 12
  },
  "devices": ["gate:74_U04"],
  "parts": ["SN74LVU04AQPWRG4Q1"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74LVU04A-Q1 hex inverter

## Description

This hex inverter is designed for 2-V to 5.5-V V~CC~ operation. [[1::2]]

The SN74LVU04A-Q1 contains six independent inverters with unbuffered
outputs. This device performs the Boolean function Y = !{A}. [[1::3]]

Qualified for Automotive Applications. Unbuffered Outputs. Typical
V~OLP~ (Output Ground Bounce) <0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C.
Typical V~OHV~ (Output V~OH~ Undershoot) >2.3 V at V~CC~ = 3.3 V,
T~A~ = 25°C. Supports Mixed-Mode Voltage Operation on All Ports. [[1::1]]

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
[[1::table #1]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [4] | | | 113 | °C/W |
| | ESD rating [5] | Human-Body Model (H1C) | | 1.5 | kV |
| ^ | ^ | Charged-Device Model (C5) | | 1 | ^ |
| ^ | ^ | Machine Model (M3) | | 200 | V |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
[5] ESD protection level per AEC Q100 classification
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.8 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.8 | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.8 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.3 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.2 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.2 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.8 [2] | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
[2] Unconfirmed: the other V~IL~ rows above 2 V are a factor of V~CC~; this row is a fixed 0.8 V. Kept as printed.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | –40°C to 125°C | < | < | –40°C to 85°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | V~IL~ = 0 V | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –2 mA | ^ | 2.3 V | 2 | | | 2 | | | ^ |
| ^ | ^ | I~OH~ = –6 mA | ^ | 3 V | 2.48 | | | 2.48 | | | ^ |
| ^ | ^ | I~OH~ = –12 mA | ^ | 4.5 V | 3.7 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 50 µA | V~IH~ = V~CC~ | 2 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | ^ | 2.3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 6 mA | ^ | 3 V | | | 0.44 | | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 12 mA | ^ | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | < | 0 V to 5.5 V | | | ±1 | | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | < | 5.5 V | | | 20 | | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 3.3 V | | 4 | | | 4 | | pF |
```
[[3::table #1]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.5 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```
[[3::table #2]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 5.6 | pF |
| ^ | ^ | ^ | 5 V | 6.7 | ^ |
```
[[3::table #3]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 125°C | < | –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 4.7 | 11.4 | 1 | 16 | 1 | 13 | ns |
```
[[3::table #4]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 125°C | < | –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 3.9 | 7 | 1 | 11 | 1 | 8 | ns |
```
[[3::table #5]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices. [[4::1]]

```figure ref=load-cmos-50pf caption="Figure 1. Load circuit for totem-pole outputs"
CL: see table
```
[[4::Figure 1]]

```figure ref=waveforms-tpd caption="Figure 1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[4::Figure 1]]

Conversion note: the source figure (Figure 1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms with their
notes B, E and F, which are not drawn here.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LVU04AQPWRG4Q1 | Reel of 2000 | -40 to 125 | LU04AQ
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted. The ordering line is from the package
  option addendum (p.5; it ends on p.6). Tape and reel and mechanical
  pages (7–12) are left out; the generated Package section replaces
  them.
- Dropped: the logic diagram (one inverter, input A, output Y), the
  ordering table on p.1 (it gives the part SN74LVU04AQPWRQ1, which the
  catalog does not carry and the package option addendum does not
  list) with its two notes.
- The feature list is written as sentences in the Description.
- In the absolute maximum ratings, the source prints the ESD
  classification beside each value, "1.5 (H1C)", "1 (C5)" and
  "200 (M3)"; the classification is in the Conditions cell here.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the overbar of Y = !{A} (p.1).
- Source issues:
  - Unconfirmed: V~IL~ maximum of 0.8 V at V~CC~ = 4.5 V to 5.5 V, where
    the other rows are V~CC~ × 0.2 (recommended operating conditions
    note 2).
