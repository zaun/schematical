---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvcu04a",
  "title": "SN74LVCU04A hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvcu04a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS282M",
    "date": "2005-02",
    "pages": 21
  },
  "devices": ["gate:74_U04"],
  "parts": ["SN74LVCU04AD", "SN74LVCU04ADBR", "SN74LVCU04ADGVR", "SN74LVCU04ADR", "SN74LVCU04ADT", "SN74LVCU04APW", "SN74LVCU04APWR"],
  "packages": {"p14": "D, DB, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74LVCU04A hex inverter

## Description

This hex inverter is designed for 1.65-V to 3.6-V V~CC~ operation. [[1::1]]

The SN74LVCU04A contains six independent inverters with unbuffered
outputs and performs the Boolean function Y = !{A}. [[1::2]]

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of these devices as translators in a mixed 3.3-V/5-V
system environment. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y, unbuffered
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

Conversion note: the logic diagram, each inverter (positive logic, p.2), shows one inverter from A to Y.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | D package | | 86 | °C/W |
| ^ | ^ | DB package | | 96 | ^ |
| ^ | ^ | DGV package | | 127 | ^ |
| ^ | ^ | NS package | | 76 | ^ |
| ^ | ^ | PW package | | 113 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V | 1.32 | | V |
| ^ | ^ | V~CC~ = 2.3 V | 1.84 | | ^ |
| ^ | ^ | V~CC~ = 2.7 V | 2.16 | | ^ |
| ^ | ^ | V~CC~ = 3 V | 2.4 | | ^ |
| ^ | ^ | V~CC~ = 3.6 V | 2.88 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V | | 0.4 | V |
| ^ | ^ | V~CC~ = 2.3 V | | 0.5 | ^ |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -24 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 24 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100 µA, V~IL~ = 0 V | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = –4 mA, V~IL~ = 0 V | 1.65 V | 1.2 | | | ^ |
| ^ | I~OH~ = –8 mA, V~IL~ = 0 V | 2.3 V | 1.7 | | | ^ |
| ^ | I~OH~ = –12 mA, V~IL~ = 0 V | 2.7 V | 2.2 | | | ^ |
| ^ | ^ | 3 V | 2.4 | | | ^ |
| ^ | I~OH~ = –24 mA, V~IL~ = 0 V | 3 V | 2.2 | | | ^ |
| V~OL~ | I~OL~ = 100 µA, V~IH~ = V~CC~ | 1.65 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 4 mA, V~IH~ = V~CC~ | 1.65 V | | | 0.45 | ^ |
| ^ | I~OL~ = 8 mA, V~IH~ = V~CC~ | 2.3 V | | | 0.7 | ^ |
| ^ | I~OL~ = 12 mA, V~IH~ = V~CC~ | 2.7 V | | | 0.4 | ^ |
| ^ | I~OL~ = 24 mA, V~IH~ = V~CC~ | 3 V | | | 0.55 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[3::table #2]]

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per inverter | f = 10 MHz | 3 | 4 | 5 | pF |
```
[[4::table #1]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1 | 7.3 | 1 | 6.7 | 1 | 4.7 | 1 | 3.8 | ns |
| t~sk(o)~ | | | | | | | | | | 1 | ns |
```
[[4::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[5::2]]

Conversion note: in the load circuit of Figure 1 one R~L~ goes from the output to ground and a second R~L~ from the output to switch S1, which goes to V~LOAD~, open or GND; the figure below draws the load with S1 open.

```table id=pmi-s1 caption="Figure 1. Switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[5::table #1]]

```figure ref=load-rl-cl caption="Figure 1, load circuit (S1 open)"
RL: see table
CL: see table
```
[[5::Figure 1]]

```table id=pmi-load caption="Figure 1. Load values"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|--:|--:|--:|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```
[[5::table #2]]

```figure ref=waveforms-tpd caption="Figure 1, voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[5::Figure 1]]

Conversion note: Figure 1 also shows pulse-duration, setup-and-hold and enable and disable waveforms with their notes B, E and F, which do not apply to this part.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74LVCU04AD | Tube of 50 | -40 to 85 | LVCU04A
SN74LVCU04ADBR | Reel of 2000 | -40 to 85 | LCU04A
SN74LVCU04ADGVR | Reel of 2000 | -40 to 85 | LCU04A
SN74LVCU04ADR | Reel of 2500 | -40 to 85 | LVCU04A
SN74LVCU04ADT | Reel of 250 | -40 to 85 | LVCU04A
SN74LVCU04APW | Tube of 90 | -40 to 85 | LCU04A
SN74LVCU04APWR | Reel of 2000 | -40 to 85 | LCU04A
```
[[1::table #3]]

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6–7), tape
  and reel, tube and mechanical pages (8–21) dropped, replaced by the
  generated Package and Ordering sections. The ordering lines are from
  the package option addendum.
- Dropped: the features list except the lines quoted, the vendor pin
  drawing, the source's ordering information table on p.1, the logic
  diagram (described in text), and the pulse-duration, setup-and-hold
  and enable and disable waveforms of Figure 1 with its notes B, E and
  F.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The electrical characteristics table of the source gives symbols
  only, so the file's table has no Parameter column.
- The ordering table on p.1 also lists SN74LVCU04ANSR (SOP – NS) and
  SN74LVCU04APWT, which the package option addendum doesn't list, and
  the addendum (p.6) also lists SN74LVCU04ADRG4, SN74LVCU04APWRE4 and
  SN74LVCU04APWRG4; the catalog doesn't carry them.
- From images: overbar of Y = !{A} (p.1).
- Source issues:
  - Unconfirmed: the V~OL~ maximum at I~OL~ = 8 mA, V~CC~ = 2.3 V is
    0.7 V, above the 0.4 V at 12 mA and the 0.55 V at 24 mA (p.3).
  - Unconfirmed: the features list (p.1) prints "Inputs Accept Voltages
    to 5.5 V" twice.
