---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc05",
  "title": "SN54AHC05, SN74AHC05 hex inverters with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc05.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS357H",
    "date": "2003-07",
    "pages": 21
  },
  "devices": ["gate:74_05"],
  "parts": ["SN74AHC05D", "SN74AHC05DBR", "SN74AHC05DR", "SN74AHC05N", "SN74AHC05PW", "SN74AHC05PWR"],
  "packages": {"p14": "D, DB, DGV, N, PW, J, W"},
  "converted": "2026-11-21"
}
---

# SN54AHC05, SN74AHC05 hex inverters with open-drain outputs

## Description

The ’AHC05 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

The open-drain outputs require pullup resistors to perform correctly.
They can be connected to other open-drain outputs to implement
active-low wired-OR or active-high wired-AND functions.

Operating Range 2-V to 5.5-V V~CC~. Latch-Up Performance Exceeds 250 mA
Per JESD 17. ESD Protection Exceeds JESD 22. 2000-V Human-Body Model
(A114-A). 200-V Machine Model (A115-A). 1000-V Charged-Device Model
(C101).

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y (open drain)
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
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [3] | D package | | 86 | °C/W |
| ^ | ^ | DB package | | 96 | °C/W |
| ^ | ^ | DGV package | | 127 | °C/W |
| ^ | ^ | N package | | 80 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | SN54AHC05 | < | SN74AHC05 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | 2.1 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | 3.85 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | | 0.9 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | | 8 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHC05 | < | SN74AHC05 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | | 0.44 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2.5 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 3 | pF |
```

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHC05 | < | SN74AHC05 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLZ~ | A | Y | C~L~ = 15 pF | | 2.9 [1] | 7.1 [1] | 1 [1] | 8.5 [1] | 1 | 8.5 | ns |
| t~PZL~ | ^ | ^ | ^ | | 4 [1] | 7.1 [1] | 1 [1] | 8.5 [1] | 1 | 8.5 | ^ |
| t~PLZ~ | A | Y | C~L~ = 50 pF | | 4.7 | 10.6 | 1 | 12 | 1 | 12 | ns |
| t~PZL~ | ^ | ^ | ^ | | 5.8 | 10.6 | 1 | 12 | 1 | 12 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHC05 | < | SN74AHC05 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLZ~ | A | Y | C~L~ = 15 pF | | 2.2 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | ns |
| t~PZL~ | ^ | ^ | ^ | | 2.9 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | ^ |
| t~PLZ~ | A | Y | C~L~ = 50 pF | | 3.4 | 7.5 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PZL~ | ^ | ^ | ^ | | 4.2 | 7.5 | 1 | 8.5 | 1 | 8.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. Waveform 1 is for an output
with internal conditions such that the output is low except when
disabled by the output control. Waveform 2 is for an output with
internal conditions such that the output is high except when disabled by
the output control. All input pulses are supplied by generators having
the following characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns,
t~f~ ≤ 3 ns. The outputs are measured one at a time with one input
transition per measurement. All parameters and waveforms are not
applicable to all devices.

```figure ref=load-ttl caption="Figure 1. Load circuit for 3-state and open-drain outputs, S1 at V~CC~"
RL: 1 kΩ
CL: see table
```

```table id=s1 caption="Figure 1. Switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~CC~ |
| t~PHZ~/t~PZH~ | GND |
| Open Drain | V~CC~ |
```

Conversion note: in the source's load circuit for 3-state and open-drain
outputs (Figure 1), R~L~ goes to a switch, S1, with the positions
V~CC~, Open and GND; the figure above draws it with S1 at V~CC~.

Conversion note: the source figure (Figure 1) also shows the load
circuit for totem-pole outputs and the pulse-duration, setup-and-hold,
propagation-delay and enable/disable voltage waveforms, which are not
drawn here; the enable and disable waveforms measure t~PZL~ and t~PLZ~
at 50% V~CC~ and V~OL~ + 0.3 V on output waveform 1 (S1 at V~CC~).

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHC05D | | -40 to 85 | AHC05 | Obsolete
SN74AHC05DBR | Reel of 2000 | -40 to 85 | HA05
SN74AHC05DR | Reel of 2500 | -40 to 85 | AHC05 | NRND
SN74AHC05N | Tube of 25 | -40 to 85 | SN74AHC05N
SN74AHC05PW | | -40 to 85 | HA05 | Obsolete
SN74AHC05PWR | Reel of 2000 | -40 to 85 | HA05
```

## Conversion notes

- Source pages 1–5 converted. The ordering lines are from the package
  option addendum (p.6; it ends on p.7). Tape and reel, tube and
  mechanical pages (8–21) are left out; the generated Package section
  replaces them.
- Dropped: the logic diagram, each inverter (one inverter, input A,
  output Y), the ordering table on p.1 (it gives "Tube" and "Tape and
  reel"; the Ordering block follows the package option addendum).
- The catalog does not carry the SN54AHC05 parts (SNJ54AHC05J,
  SNJ54AHC05W, SNJ54AHC05FK) or SN74AHC05DGVR of the ordering table on
  p.1, and has no FK (20-pin) pinout for this device; the FK pin drawing
  (p.1) is not converted. The SN54AHC05 columns of the tables are kept.
- Pages 3 and 4 carry a "PRODUCT PREVIEW" mark across the SN54AHC05
  columns and the footer "PRODUCT PREVIEW information concerns products
  in the formative or design phase of development."
- The feature list is written as sentences in the Description.
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the µA unit of the I~OL~ (V~CC~ = 2 V) row, the
  I~OL~ = 50 µA condition and the units of the I~I~ and I~CC~ rows (p.3;
  the text layer prints them as mA); the overbar of Y = !{A} (p.1);
  1 kΩ (R~L~), 50% V~CC~ and V~OL~ + 0.3 V (Figure 1, p.5).
- Source issues: none found.
