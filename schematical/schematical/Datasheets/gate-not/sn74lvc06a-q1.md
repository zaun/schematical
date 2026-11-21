---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc06a-q1",
  "title": "SN74LVC06A-Q1 automotive hex inverter buffer/driver with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc06a-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS761B",
    "date": "2008-04",
    "pages": 15
  },
  "devices": ["gate:74_06"],
  "parts": ["SN74LVC06AQDRG4Q1", "SN74LVC06AQDRQ1", "SN74LVC06AQPWRG4Q1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74LVC06A-Q1 automotive hex inverter buffer/driver with open-drain outputs

## Description

This hex inverter buffer/driver is designed for 1.65-V to 3.6-V V~CC~
operation.

The outputs of the device are open drain and can be connected to other
open-drain outputs to implement active-low wired-OR or active-high
wired-AND functions. The maximum sink current is 24 mA.

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of this device as a translator in a mixed 3.3-V/5-V
system environment.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

Qualified for Automotive Applications. Operate From 1.65 V to 3.6 V.
Inputs and Open-Drain Outputs Accept Voltages up to 5.5 V. Latch-Up
Performance Exceeds 250 mA Per JESD 17.

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

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OL~ | I~OL~ = 100 µA | 1.65 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | V |
| ^ | I~OL~ = 8 mA | 2.3 V | | | 0.7 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.5 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.65 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 1 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | ^ |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per buffer/driver | f = 10 MHz | 2.5 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A | Y | | 3.9 | 1 | 3.7 | ns |
```

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
devices.

Conversion note: the load circuit of Figure 1 (open drain) connects the output under test to C~L~ and one R~L~ to GND and through a second R~L~ to switch S1, which has the positions V~LOAD~, Open and GND.

```table id=pmi-s1 caption="Figure 1. Switch S1"
| Test | S1 |
|---|---|
| t~PZL~ (see Notes E and F) | V~LOAD~ |
| t~PLZ~ (see Notes E and G) | V~LOAD~ |
| t~PHZ~/t~PZH~ | V~LOAD~ |
```

```table id=pmi-load caption="Figure 1. Load values"
| V~CC~ | Input | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 2.7 V | 2.7 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```

Conversion note: the voltage waveforms of Figure 1 draw the input and the output control from 0 V to V~I~ with the propagation delay times and the enable and disable times measured at V~M~, and output waveform 1 (S1 at V~LOAD~) falling from V~LOAD~/2 to V~OL~, with t~PLZ~ ending at V~OL~ + V~Δ~.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LVC06AQDRG4Q1 | Reel of 2500 | -40 to 125 | LVC06AQ
SN74LVC06AQDRQ1 | Reel of 2500 | -40 to 125 | LVC06AQ
SN74LVC06AQPWRG4Q1 | Reel of 2000 | -40 to 125 | LVC06AQ
```

## Conversion notes

- Source pages 1–4 converted. The ordering lines are from the package
  option addendum (p.5). The rest of the addendum (p.6), tape and reel
  (pp.7–8), mechanical pages (9–14) and the notice page (15) are left
  out; the generated Package section replaces them.
- Dropped: features list (except the automotive, supply, input-voltage
  and latch-up lines), the ordering information table of p.1, the vendor
  pinout drawing and the logic diagram, each inverter (one inverter with
  input A and output Y, p.1).
- The source has no pin functions table; the pin descriptions are the
  converter's, written from the pin names, the function table and the
  description.
- The catalog does not carry SN74LVC06AQPWRQ1 of the ordering
  information table on p.1; the package option addendum (p.5) does not
  list it.
- The load circuit and voltage waveforms of Figure 1 (p.4) match no
  shared figure and are described in sentences; the figure's notes and
  tables are kept.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column. It has no I~off~ row.
- The thermal impedance rows are part of the source's absolute maximum
  ratings table and stay there.
- The description (p.1) names the device "SN74LVC06A" in "The outputs of
  the SN74LVC06A device"; the name is left out of that sentence here.
- Source issues: none found.
