---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc3g06",
  "title": "SN74LVC3G06 triple inverter buffer/driver with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc3g06.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES364J",
    "date": "2013-11",
    "pages": 20
  },
  "devices": ["gate:3G06"],
  "parts": ["SN74LVC3G06DCTR", "SN74LVC3G06DCUR", "SN74LVC3G06DCUT", "SN74LVC3G06YZPR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC3G06 triple inverter buffer/driver with open-drain outputs

## Description

This triple inverter buffer/driver is designed for 1.65-V to 5.5-V
V~CC~ operation.

The output of the SN74LVC3G06 is open drain and can be connected to
other open-drain outputs to implement active-low wired-OR or active-high
wired-AND functions. The maximum sink current is 32 mA.

NanoFree™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package.

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down.

Supports 5-V V~CC~ Operation. Input and Open-Drain Output Accepts
Voltages up to 5.5 V.

## Pinout

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A, 3A: Inverter input A
1Y, 2Y, 3Y: Inverter output Y, open drain
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

Conversion note: the logic diagram (positive logic, p.2) shows three inverters with open-drain outputs, 1A (pin 1) to 1Y (pin 7), 2A (pin 3) to 2Y (pin 5) and 3A (pin 6) to 3Y (pin 2).

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | DCT package | | 220 | °C/W |
| ^ | ^ | DCU package | | 227 | ^ |
| ^ | ^ | YZP package | | 102 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.75 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.75 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 3.5 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 2 | 2 | 3 | 4 | pF |
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | SN74LVC3G06, –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 | 7.2 | 1 | 3.9 | 1 | 3.4 | 1 | 2.9 | ns |
```

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | SN74LVC3G06, –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 | 7.8 | 1 | 4.5 | 1 | 4.0 | 1 | 3.5 | ns |
| t~pd~ | A or B | Y | 2.6 | 9.8 | 1 | 5.8 | 1 | 5.3 | 1 | 4.8 | ns |
```

Unconfirmed: the –40°C to 125°C table prints a second t~pd~ row from "A or B" to Y with other values; the device has no B input.

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
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

Conversion note: the voltage waveforms of Figure 1 draw the input and the output control from 0 V to V~I~ with the propagation delay times and the enable and disable times measured at V~M~, and output waveform 1 (S1 at V~LOAD~) falling from V~LOAD~/2 to V~OL~, with t~PLZ~ ending at V~OL~ + V~Δ~.

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

```package footprint=DSBGA-8-YZP
```

## Ordering information

```ordering
SN74LVC3G06DCTR | Reel of 3000 | -40 to 125 | 2WX5, C06, R, Z
SN74LVC3G06DCUR | Reel of 3000 | -40 to 125 | C06J, C06Q, C06R
SN74LVC3G06DCUT | Reel of 250 | -40 to 125 | C06J, C06Q, C06R
SN74LVC3G06YZPR | Reel of 3000 | -40 to 85 | CTN
```

## Conversion notes

- Source pages 1–5 converted; the revision history (p.6), the package
  option addendum (p.7–8), tape and reel and mechanical pages (9–20)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are from the package option addendum.
- Dropped: the features list except the lines quoted, the ESD caution,
  the vendor pin drawings and the logic diagram (described in text).
- The load circuit and voltage waveforms of Figure 1 (p.5) match no
  shared figure and are described in sentences; the figure's notes and
  tables are kept.
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The electrical characteristics table of the source gives symbols
  only, so its Parameter cells are empty except for I~I~. The source
  prints the C~i~ typical value 3.5 once, between the two temperature
  groups.
- The package option addendum (p.7) also lists SN74LVC3G06DCURG4
  (marking C06R), which the catalog doesn't carry.
- The text layer heads the last column of the Figure 1 load table V∆
  and prints "V~OL~ + V∆" (p.5); the rendered page shows a bare V in
  both places.
- Source issues:
  - Unconfirmed: the –40°C to 125°C switching table (p.4) prints a
    second t~pd~ row from "A or B" to Y; the device has no B input.
  - Unconfirmed: the function table (p.2) prints H as the output for a
    low input, and the output is open drain (p.1).
  - Source conflict: the recommended operating conditions (p.3) give
    T~A~ –40 to 85 °C; the electrical and switching characteristics
    (p.3–4) have –40°C to 125°C columns, and the package option
    addendum (p.7) lists the DCT and DCU parts at -40 to 125.
