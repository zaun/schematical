---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct14-ep",
  "title": "SN74AHCT14-EP hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct14-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS492",
    "date": "2003-06",
    "pages": 9
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74AHCT14MDREP"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHCT14-EP hex Schmitt-trigger inverter

## Description

The SN74AHCT14-EP contains six independent inverters. This device
performs the Boolean function Y = !{A}. [[1::3]]

Each circuit functions as an independent inverter, but because of the
Schmitt action, the inverters have different input threshold levels for
positive-going (V~T+~) and for negative-going (V~T–~) signals. [[1::4]]

Controlled Baseline. One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of –55°C to 125°C. Enhanced Diminishing
Manufacturing Sources (DMS) Support. Enhanced Product-Change
Notification. Qualification Pedigree. EPIC (Enhanced-Performance
Implanted CMOS) Process. Inputs Are TTL-Voltage Compatible. Latch-Up
Performance Exceeds 250 mA Per JESD 17. ESD Protection Exceeds 2000 V
Per MIL-STD-883, Method 3015; Exceeds 200 V Using Machine Model
(C = 200 pF, R = 0). [[1::1]]

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
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 4.5 V | 0.9 | | 1.9 | 0.9 | 1.9 | V |
| ^ | ^ | | 5.5 V | 1 | | 2.1 | 1 | 2.1 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 4.5 V | 0.5 | | 1.5 | 0.5 | 1.5 | V |
| ^ | ^ | | 5.5 V | 0.6 | | 1.7 | 0.6 | 1.7 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 4.5 V | 0.4 | | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.4 | | 1.5 | 0.4 | 1.5 | ^ |
| V~OH~ | | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| ΔI~CC~ [1] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels rather than 0 V or V~CC~.
```
[[3::table #1]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.9 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.7 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.1 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[3::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```
[[3::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4 | 7 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4 | 7 | 1 | 8 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.5 | 8 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 8 | 1 | 9 | ^ |
```
[[3::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. [[4::1]]

Conversion note: in Figure 1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Figure 1. Load circuit for totem-pole outputs"
CL: see table
```
[[4::Figure 1]]

```figure ref=waveforms-tpd caption="Figure 1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[4::Figure 1]]

Conversion note: the source figure (Figure 1) also shows the load
circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1
table, and the pulse-duration, setup-and-hold and enable/disable
waveforms with their note B, which are not drawn here.

## Package

```package footprint=SOIC-14
```

## Ordering information

```ordering
SN74AHCT14MDREP | Reel of 2500 | -55 to 125 | AHCT14MEP | NRND
```
[[1::table #3]]

## Conversion notes

- Source pages 1–4 converted. The ordering line is from the package
  option addendum (p.5; it ends on p.6). Tape and reel and notice pages
  (7–9) are left out.
- Dropped: the logic symbol and the logic diagram, each inverter (one
  Schmitt-trigger inverter, input A, output Y), the vendor pin drawing
  (p.1), the ordering table on p.1 (it gives "Tape and reel"; the
  Ordering block follows the package option addendum).
- The catalog does not carry SN74AHCT14MPWREP, V62/03656-01XE and
  V62/03656-01YE, which the package option addendum lists.
- The source's description (p.1) names the device "SN74AHCT14"; the data
  sheet's title is SN74AHCT14-EP, and the name is written with the
  suffix here.
- The feature list is written as sentences in the Description; the
  footnote of "Qualification Pedigree" follows them.
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells.
- The electrical characteristics table of the source prints the name of
  V~T+~, V~T–~ and ΔV~T~ under the symbol in its PARAMETER column and
  only the symbol of the other rows, whose Parameter cells are empty.
  The electrical and switching tables print no heading over their last
  MIN and MAX columns.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the I~OH~ = –50 µA and I~OL~ = 50 µA conditions and the
  µA units of the I~I~ and I~CC~ rows (p.3; the text layer prints them
  as mA); the overbar of Y = !{A} (p.1); 0 V, 3 V, 1.5 V and 50% V~CC~
  (Figure 1, p.4).
- Source issues: none found.
