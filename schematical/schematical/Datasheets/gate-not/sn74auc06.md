---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc06",
  "title": "SN74AUC06 hex inverter buffer/driver with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74auc06.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES471",
    "date": "2005-08",
    "pages": 9
  },
  "devices": ["gate:74_06"],
  "parts": ["SN74AUC06RGYR"],
  "packages": {"p14": "RGY"},
  "converted": "2026-11-21"
}
---

# SN74AUC06 hex inverter buffer/driver with open-drain outputs

## Description

Optimized for 1.8-V operation and is 3.6-V I/O tolerant to support
mixed-mode signal operation.

This hex inverter buffer/driver is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation.

The outputs of the SN74AUC06 are open drain and can be connected to
other open-drain outputs to implement active-low wired-OR or active-high
wired-AND functions.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

## Pinout

```pinout pinout=p14
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A, 3A, 4A, 5A, 6A: Buffer/driver input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Buffer/driver output Y (open drain)
GND: Ground
VCC: Supply voltage
```

## Functional description

Conversion note: the logic diagram (each buffer/driver, positive logic) shows one inverter with input A and output Y, drawn with an open-drain symbol.

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
| V~CC~ | Supply voltage range | | -0.5 | 3.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 3.6 | V |
| V~O~ | Output voltage range [2] | | -0.5 | 3.6 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | | | 47 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-5.
```

Latch-up performance exceeds 100 mA per JESD 78, Class II. ESD
protection exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | 3.6 | V |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A inputs | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 2.5 | | pF |

[1] All typical values are at T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 2 | 2 | 2 | 3 | 3 | pF |
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 4.2 | 0.8 | 3.6 | 0.5 | 2.6 | 0.5 | 1.4 | 3.1 | 0.4 | 2.2 | ns |
```

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 | 1.7 | 2.8 | 0.8 | 1.3 | ns |
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
|---|---|---|---|---|--:|--:|--:|
| 0.8 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 2 kΩ | 0.1 V |
| 1.2 V ± 0.1 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 2 kΩ | 0.1 V |
| 1.5 V ± 0.1 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 2 kΩ | 0.1 V |
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 2 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 2 kΩ | 0.15 V |
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
```

Conversion note: the voltage waveforms of Figure 1 draw the input and the output control from 0 V to V~I~ with the propagation delay times and the enable and disable times measured at V~M~, and output waveform 1 (S1 at V~LOAD~) falling from V~LOAD~/2 to V~OL~, with t~PLZ~ ending at V~OL~ + V~Δ~.

## Package

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74AUC06RGYR | Reel of 3000 | -40 to 85 | MS06
```

## Conversion notes

- Source pages 1–4 converted; the package option addendum (p.5), tape
  and reel and mechanical pages (6–9) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: the features list except the lines quoted, and the source's
  ordering information table on p.1 (–40°C to 85°C, QFN – RGY, Tape and
  reel, SN74AUC06RGYR, marking MS06).
- The load circuit and voltage waveforms of Figure 1 (p.4) match no
  shared figure and are described in sentences; the figure's notes and
  tables are kept.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
