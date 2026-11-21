---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv04a-ep",
  "title": "SN74LV04A-EP hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv04a-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS563A",
    "date": "2004-05",
    "pages": 13
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74LV04ATPWREP"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74LV04A-EP hex inverter

## Description

This hex inverter is designed for 2-V to 5.5-V V~CC~ operation.

The SN74LV04A-EP contains six independent inverters. This device
performs the Boolean function Y = !{A}.

The device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

Extended Temperature Performance of –40°C to 105°C. Supports Mixed-Mode
Voltage Operation on All Ports.

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
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [4] | | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 105 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | I~OH~ = -2 mA | 2.3 V | 2 | | | V |
| ^ | I~OH~ = -6 mA | 3 V | 2.48 | | | V |
| ^ | I~OH~ = -12 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | V |
| ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | V |
| ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 2.3 | | pF |
| ^ | ^ | 5 V | | 2.3 | | pF |
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.3 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 9.6 | pF |
| ^ | ^ | ^ | 5 V | 11.4 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 2.5 V ± 0.2 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 10 | 15.5 | 1 | 18 | ns |
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 7.3 | 10.6 | 1 | 12 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 5.1 | 7.5 | 1 | 8.5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: Figure 1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse
duration, setup and hold times, and enable and disable times.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LV04ATPWREP | Reel of 2000 | -40 to 105 | LV04AEP
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum (pp.6–7), tape
  and reel and mechanical pages (8–13) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: features list (two feature lines are in Description),
  qualification pedigree note, the vendor pinout, the page-1 ordering
  information table, the logic diagram (p.1, one inverter, input A,
  output Y).
- The Description of the source names the device "SN74LV04A"; the data
  sheet's title is SN74LV04A-EP, and the name is written with the suffix
  here.
- The package option addendum also lists V62/04691-01XE, which the
  catalog does not carry.
- The source has no pin functions table; the pin descriptions are
  written from the pin names and the function table.
- The absolute maximum ratings of the source are a list; each line is a
  row here, with its name, symbol and condition in separate cells.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column.
- The switching tables' second Min/Max group has no heading in the
  source.
- The load figure prints no C~L~ value; 50 pF is the load capacitance of
  the switching tables (p.3).
- From images: overbars of Y = !{A} (p.1).
- Source issues: none found.
