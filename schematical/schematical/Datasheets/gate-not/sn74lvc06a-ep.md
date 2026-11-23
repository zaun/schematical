---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc06a-ep",
  "title": "SN74LVC06A-EP hex inverter buffer/driver with open drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc06a-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS832A",
    "date": "2007-05",
    "pages": 12
  },
  "devices": ["gate:74_06"],
  "parts": ["SN74LVC06AMDREP"],
  "packages": {"p14": "D"},
  "converted": "2026-11-21"
}
---

# SN74LVC06A-EP hex inverter buffer/driver with open drain outputs

## Description

The SN74LVC06A-EP is a hex inverter buffer/driver that is designed for
1.65-V to 3.6-V V~CC~ operation. [[1::3]]

The outputs of the SN74LVC06A-EP device are open drain and can be
connected to other open-drain outputs to implement active low wired OR
or active high wired AND functions. The maximum sink current is 24 mA. [[1::4]]

Inputs can be driven from either 3.3 V or 5 V devices. This feature
allows the use of these devices as translators in a mixed 3.3 V/5 V
system environment. [[1::5]]

This device is fully specified for partial power down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::6]]

Extended Temperature Performance of –55°C to 125°C. Operates From 1.65 V
to 3.6 V. Inputs and Open Drain Outputs Accept Voltages up to 5.5 V.
Latch-Up Performance Exceeds 250 mA Per JESD 17. [[1::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y, open drain
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

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 V | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 V | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | | | 86 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| P~tot~ | Power dissipation [4] | T~A~ = –55°C to 125°C | | 500 | mW |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
[4] Above 70°C the value of P~tot~ derates linearly with 8 mW/K.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | T~A~ = 25°C | < | –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | 5.5 | 0 | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | | 24 | mA |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[1::table #3]] [[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| V~OL~ | I~OL~ = 100 µA | 1.65 V to 3.6 V | | | 0.1 | | 0.3 | V |
| ^ | I~OL~ = 4 mA | 1.65 V | | | 0.24 | | 0.6 | V |
| ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | 0.75 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | | 0.6 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | | 0.8 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±1 | | ±20 | µA |
| I~off~ | V~I~ or V~O~ = 5.5 V | 0 V | | | ±1 | | ±20 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 1 | | 40 | µA |
| ΔI~CC~ | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | | 5000 | µA |
| C~I~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | pF |
```
[[3::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per buffer/driver | f = 10 MHz | 1.8 V | 2.1 | pF |
| ^ | ^ | ^ | 2.5 V | 2.3 | pF |
| ^ | ^ | ^ | 3.3 V | 2.5 | pF |
```
[[3::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 V ± 0.15 V | 1.4 | 3 | 5.1 | 1.4 | 7.6 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 1.9 | 2.8 | 1 | 4 | ns |
| ^ | ^ | ^ | 2.7 V | 1 | 2.4 | 3.7 | 1 | 5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.2 | 3.5 | 1 | 5 | ns |
```
[[3::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. Waveform 1 is for an output
with internal conditions such that the output is low, except when
disabled by the output control. Waveform 2 is for an output with
internal conditions such that the output is high, except when disabled
by the output control. All input pulses are supplied by generators
having the following characteristics: PRR ≤ 10 MHz, Z~O~ = 50 Ω. The
outputs are measured one at a time, with one transition per measurement.
Since this device has open-drain outputs, t~PLZ~ and t~PZL~ are the same
as t~pd~. t~PZL~ is measured at V~M~. t~PLZ~ is measured at
V~OL~ + V~Δ~. All parameters and waveforms are not applicable to all
devices. [[4::1]]

Conversion note: the load circuit of Figure 1 connects the output under test to C~L~ and one R~L~ to GND and through a second R~L~ to switch S1, which has the positions V~LOAD~, Open and GND.

```table id=pmi-s1 caption="Figure 1. Switch S1"
| Test | S1 |
|---|---|
| t~PZL~ (see Notes E and F) | V~LOAD~ |
| t~PLZ~ (see Notes E and G) | V~LOAD~ |
| t~PHZ~/t~PZH~ | V~LOAD~ |
```
[[4::table #1]]

```table id=pmi-load caption="Figure 1. Load values"
| V~CC~ | Input | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```
[[4::table #2]]

Conversion note: the voltage waveforms of Figure 1 draw the input and the output control from 0 V to V~I~ with the propagation delay times and the enable and disable times measured at V~M~, and output waveform 1 (S1 at V~LOAD~) falling from V~LOAD~/2 to V~OL~, with t~PLZ~ ending at V~OL~ + V~Δ~.

## Package

```package footprint=SOIC-14
```

## Ordering information

```ordering
SN74LVC06AMDREP | Reel of 2500 | -55 to 125 | LC06AM
```
[[2::table #3]]

## Conversion notes

- Source pages 1–4 converted. The ordering line is from the package
  option addendum (p.5). The rest of the addendum (p.6), tape and reel
  (pp.7–8), mechanical pages (9–11) and the notice page (12) are left
  out; the generated Package section replaces them.
- Dropped: features list (except the temperature, supply, input-voltage
  and latch-up lines) with its qualification-pedigree footnote, the
  ordering information table of p.2, the vendor pinout drawing and the
  logic diagram, each inverter (one inverter with input A and output Y,
  p.2).
- The source has no pin functions table; the pin descriptions are the
  converter's, written from the pin names, the function table and the
  description.
- The catalog does not carry V62/06661-01XE of the package option
  addendum (p.5).
- The recommended operating conditions table runs over pp.2–3
  ("continued"); it is one table here.
- The load circuit and voltage waveforms of Figure 1 (p.4) match no
  shared figure and are described in sentences; the figure's notes and
  tables are kept.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column.
- The thermal impedance row is part of the source's absolute maximum
  ratings table and stays there.
- The description (p.1) names the device "SN74LVC06A"; it is written
  SN74LVC06A-EP here.
- Source issues:
  - Source conflict: the ordering information table (p.2) gives the
    top-side marking of SN74LVC06AMDREP as LVC06AM; the package option
    addendum (p.5) gives LC06AM.
