---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hct14-q1",
  "title": "SN74HCT14-Q1 hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74hct14-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS588A",
    "date": "2008-04",
    "pages": 12
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74HCT14QPWRG4Q1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HCT14-Q1 hex Schmitt-trigger inverter

## Description

The device contains six independent inverters. The device performs the
Boolean function Y = !{A} in positive logic.

Qualified for Automotive Applications. Operating Voltage Range of 4.5 V
to 5.5 V. Outputs Can Drive Up To 10 LSTTL Loads. Inputs Are
TTL-Voltage Compatible.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output
GND: Ground
VCC: Supply voltage
```

## Functional description

Conversion note: the logic diagram (positive logic) (p.1) shows one inverter with a Schmitt-trigger symbol, input A, output Y.

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
| V~I~ | Input voltage range (see Note 1) [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range (see Note 1) [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance (see Note 2) [3] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions (see Note 3) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | | 4.5 V | 1.2 | 1.5 | 1.9 | 1.2 | 1.9 | V |
| ^ | ^ | | 5.5 V | 1.4 | 1.7 | 2.1 | 1.4 | 2.1 | ^ |
| V~T−~ | Negative-going threshold | | 4.5 V | 0.5 | 0.9 | 1.2 | 0.5 | 1.2 | V |
| ^ | ^ | | 5.5 V | 0.6 | 1 | 1.4 | 0.6 | 1.4 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ − V~T−~) | | 4.5 V | 0.4 | 0.6 | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.4 | 0.65 | 1.5 | 0.4 | 1.5 | ^ |
| V~OH~ | | I~OH~ = -20 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | 4.3 | | 3.7 | | ^ |
| V~OL~ | | I~OL~ = 20 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 4.5 V | | 0.17 | 0.26 | | 0.4 | ^ |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | µA |
| ΔI~CC~ [1] | | One input at 0.5 V or 2.4 V, Other inputs at GND or V~CC~ | 5.5 V | | 0.2 | 2.4 | | 3 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 3 | 10 | | 10 | pF |

[1] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

Conversion note: the source prints no temperature heading over the second Min and Max columns of this table and of the switching characteristics table.

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load | 10 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 50 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 4.5 V | | 20 | 32 | | 48 | ns |
| ^ | ^ | ^ | 5.5 V | | 18 | 30 | | 45 | ^ |
| t~t~ | | Y | 4.5 V | | 7 | 15 | | 22 | ns |
| ^ | ^ | ^ | 5.5 V | | 6 | 14 | | 20 | ^ |
```

## Typical characteristics

Conversion note: the two graphs of supply current against input voltage (p.4) are drawn at V~CC~ = 4.5 V and V~CC~ = 5.5 V, each with one curve for V~I~ = 0 to V~CC~ and one for V~I~ = V~CC~ to 0; they are not digitized here.

Conversion note: the two graphs of output voltage against input voltage (p.4) are drawn at V~CC~ = 4.5 V and V~CC~ = 5.5 V as a hysteresis loop, with one path for V~I~ = Up and one for V~I~ = Down; they are not digitized here.

## Parameter measurement information

```figure ref=load-cmos-50pf caption="Figure 1: load circuit"
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Figure 1: voltage waveforms, propagation delay and output rise and fall times"
VM: 1.3 V
```

C~L~ includes probe and test-fixture capacitance. Phase relationships
between waveforms were chosen arbitrarily. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ = 6 ns, t~f~ = 6 ns. The outputs are measured one at
a time, with one input transition per measurement. t~PLH~ and t~PHL~
are the same as t~pd~.

Conversion note: in Figure 1 the input swings from 0 V to 3 V, and the voltage waveform of the input rise and fall times marks t~r~ and t~f~ between the 10% and 90% points.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCT14QPWRG4Q1 | Reel of 2000 | -40 to 125 | HCT14Q
```

## Conversion notes

- Source pages 1–4 converted; the package option addendum (p.5–6),
  tape and reel and mechanical pages and the notice (7–12) dropped,
  replaced by the generated Package and Ordering sections. The ordering
  line is from the package option addendum (p.5).
- Dropped: the feature list (four features are kept as sentences in the
  Description), the ordering information table of p.1 (it does not list
  the claimed part), the vendor's pin drawing (p.1). The logic diagram
  and the four typical-characteristics graphs (p.4) are described in
  text.
- The source has no pin functions table; the pin descriptions are the
  converter's.
- The source covers the D (SOIC) package too, and the package option
  addendum also lists SN74HCT14QPWRQ1; the catalog carries neither a D
  part nor that part under this datasheet.
- The electrical characteristics table of the source prints a parameter
  name only for V~T+~, V~T−~ and ΔV~T~; the other Parameter cells are
  empty. The absolute maximum ratings print their conditions in
  parentheses.
- The description (p.1) names the device "SN74HCT14"; the name is left
  out of that sentence.
- From images: overbar of Y = !{A} (p.1).
