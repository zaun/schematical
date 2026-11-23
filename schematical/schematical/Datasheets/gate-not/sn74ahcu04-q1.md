---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahcu04-q1",
  "title": "SN74AHCU04-Q1 hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahcu04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS596A",
    "date": "2008-04",
    "pages": 13
  },
  "devices": ["gate:74_U04"],
  "parts": ["SN74AHCU04QPWRG4Q1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHCU04-Q1 hex inverter

## Description

The SN74AHCU04-Q1 device contains six independent inverters. This device
performs the Boolean function Y = !{A}. Internal circuitry consists of
single-stage inverters that can be used in analog applications such as
crystal oscillators. [[1::2]]

Qualified for Automotive Applications. Operating Range 2-V to 5.5-V
V~CC~. Unbuffered Outputs. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
GND: Ground
VCC: Positive supply
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

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
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

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.4 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 4.4 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.3 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.6 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.1 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = −40°C to 125°C | < | T~A~ = −40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 µA | 2 V | 1.8 | 2 | | 1.8 | | 1.8 | | V |
| ^ | ^ | ^ | 3 V | 2.7 | 3 | | 2.7 | | 2.7 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4 | 4.5 | | 4 | | 4 | | ^ |
| ^ | ^ | I~OH~ = −4 mA | 3 V | 2.58 | | | 2.3 | | 2.4 | | ^ |
| ^ | ^ | I~OH~ = −8 mA | 4.5 V | 3.94 | | | 3.5 | | 3.65 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.2 | | 0.2 | | 0.2 | V |
| ^ | ^ | ^ | 3 V | | | 0.3 | | 0.3 | | 0.3 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.5 | | 0.5 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | | 10 | pF |
```
[[3::table #1]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.5 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.5 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 4 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1 | V |

[1] Characteristics are for surface-mount packages only.
```
[[4::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 7.3 | pF |
```
[[4::table #2]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | T~A~ = −40°C to 125°C | < | T~A~ = −40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5 | 7.1 | 1 | 10.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 7.1 | 1 | 10.5 | 1 | 8.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.5 | 10.6 | 1 | 14 | 1 | 12 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.5 | 10.6 | 1 | 14 | 1 | 12 | ^ |
```
[[3::table #2]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | T~A~ = −40°C to 125°C | < | T~A~ = −40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.5 | 5.5 | 1 | 8.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.5 | 5.5 | 1 | 8.5 | 1 | 6.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5 | 7 | 1 | 11 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 7 | 1 | 11 | 1 | 9 | ^ |
```
[[3::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[5::1]]

```figure ref=load-cmos-50pf caption="Figure 1. Load circuit for totem-pole outputs"
CL: see table
```
[[5::Figure 1]]

```figure ref=waveforms-tpd caption="Figure 1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[5::Figure 1]]

Conversion note: the source figure (Figure 1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms with their
note B, which are not drawn here.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHCU04QPWRG4Q1 | Reel of 2000 | -40 to 125 | AHCU04Q | Last Time Buy
```
[[6::table #1]]

## Conversion notes

- Source pages 1–5 converted. The ordering line is from the package
  option addendum (p.6; it ends on p.7). Tape and reel and mechanical
  pages (8–13) are left out; the generated Package section replaces
  them.
- Dropped: the logic diagram (one inverter, input A, output Y), the
  ordering table on p.1 (it gives "Tape and reel" and the parts
  SN74AHCU04QDRQ1 and SN74AHCU04QPWRQ1, which the catalog does not carry
  and the package option addendum does not list), the latch-up and ESD
  feature lines.
- The source's description (p.1) names the device "SN74AHCU04"; the data
  sheet's title is SN74AHCU04-Q1, and the name is written with the
  suffix here.
- The feature list is written as sentences in the Description.
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the µA units of the I~OH~/I~OL~ (V~CC~ = 2 V) rows (p.2),
  the I~OH~ = −50 µA and I~OL~ = 50 µA conditions and the units of the
  I~I~ and I~CC~ rows (p.3; the text layer prints them as mA); the
  overbar of Y = !{A} (p.1); 50% V~CC~ (V~M~, Figure 1, p.5).
- Source issues: none found.
