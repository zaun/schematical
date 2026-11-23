---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc04-ep",
  "title": "SN74AHC04-EP hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc04-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS483A",
    "date": "2003-09",
    "pages": 10
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74AHC04MPWREP"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHC04-EP hex inverter

## Description

The SN74AHC04-EP contains six independent inverters. This device
performs the Boolean function Y = !{A}. [[1::3]]

Controlled Baseline. One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of –55°C to 125°C. Enhanced Diminishing
Manufacturing Sources (DMS) Support. Enhanced Product-Change
Notification. Qualification Pedigree. EPIC (Enhanced-Performance
Implanted CMOS) Process. Operating Range 2-V to 5.5-V V~CC~. ESD
Protection Exceeds 1500 V Per MIL-STD-833, Method 3015; Exceeds 150 V
Using Machine Model (C = 200 pF, R = 0). Unconfirmed: "MIL-STD-833" is
as the source prints it. [[1::1]]

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. This
includes, but is not limited to, Highly Accelerated Stress Test (HAST)
or biased 85/85, temperature cycle, autoclave or unbiased HAST,
electromigration, bond intermetallic life, and mold compound life. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
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
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | ^ |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | ^ |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | pF |
```
[[4::table #1]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.8 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[4::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```
[[5::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5 | 8.9 | 1 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 8.9 | 1 | 10.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.5 | 11.4 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.5 | 11.4 | 1 | 13 | ^ |
```
[[4::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.8 | 5.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.8 | 5.5 | 1 | 6.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.3 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.3 | 7.5 | 1 | 8.5 | ^ |
```
[[4::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. [[5::1]]

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
SN74AHC04MPWREP | Reel of 2000 | -55 to 125 | AHC04EP
```
[[1::table #3]]

## Conversion notes

- Source pages 1–5 converted. The ordering line is from the package
  option addendum (p.6; it ends on p.7). Tape and reel and notice pages
  (8–10) are left out.
- Dropped: the logic symbol and the logic diagram, each inverter (one
  inverter, input A, output Y), the ordering table on p.1 (it gives
  "Tape and reel"; the Ordering block follows the package option
  addendum).
- The catalog does not carry SN74AHC04MDREP, V62/03646-01XE and
  V62/03646-01YE, which the package option addendum lists.
- The source's description (p.1) names the device "SN74AHC04"; the data
  sheet's title is SN74AHC04-EP, and the name is written with the suffix
  here.
- The feature list is written as sentences in the Description; the
  footnote of "Qualification Pedigree" follows them.
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. The electrical and switching tables
  print no heading over their last MIN and MAX columns.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the µA units of the I~OH~/I~OL~ (V~CC~ = 2 V) rows (p.3),
  the I~OH~ = –50 µA and I~OL~ = 50 µA conditions and the units of the
  I~I~ and I~CC~ rows (p.4; the text layer prints them as mA); the
  overbar of Y = !{A} (p.1); 50% V~CC~ (V~M~, Figure 1, p.5).
- Source issues: Unconfirmed: the feature list prints "MIL-STD-833"
  (p.1); it is kept as printed.
