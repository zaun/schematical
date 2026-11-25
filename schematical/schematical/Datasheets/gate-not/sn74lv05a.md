---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv05a",
  "title": "SN74LV05A hex inverters with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv05a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS391K",
    "date": "2023-03",
    "pages": 26
  },
  "devices": ["gate:74_05"],
  "parts": ["SN74LV05AD", "SN74LV05ADGVR", "SN74LV05ADR", "SN74LV05ANSR", "SN74LV05APW", "SN74LV05APWR", "SN74LV05APWT"],
  "packages": {"p14": "D, DGV, NS, PW"},
  "converted": "2026-11-21"
}
---

# SN74LV05A hex inverters with open-drain outputs

## Description

The SN74LV05A device contains six independent inverters designed for
2 V to 5.5 V V~CC~ operation. [[9::1]]

This device performs the Boolean function Y = !{A}. [[1::5]]

Support mixed-mode voltage operation on all ports. I~off~ supports
partial-power-down mode operation. Latch-up performance exceeds 250 mA
per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1Y: 1Y Output
2A: 2A Input
2Y: 2Y Output
3A: 3A Input
3Y: 3Y Output
GND: Ground Pin
4Y: 4Y Output
4A: 4A Input
5Y: 5Y Output
5A: 5A Input
6Y: 6Y Output
6A: 6A Input
VCC: Power Pin
```
[[3::Table 5-1]]

## Functional description

The SN74LV05A device contains six independent inverters designed for
2-V to 5.5-V V~CC~ operation. [[9::1]]

This device performs the Boolean function Y = !{A}. [[1::5]]

The open-drain outputs require pull-up resistors to perform correctly
and can be connected to other open-drain outputs to implement
active-low, wired-OR or active-high wired-AND functions. [[9::3]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[9::4]]

Conversion note: the functional block diagram (Figure 8-1) shows one
inverter with an open-drain output, input A and output Y.

- Wide operating voltage range [[9::5]]
  - Operates from 2 V to 5.5 V [[9::5]]
- Allows down-voltage translation [[9::5]]
  - Inputs accept voltages to 5.5 V [[9::5]]
- I~off~ feature [[9::5]]
  - Allows voltages on the inputs and outputs when V~CC~ is 0 V [[9::5]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input [1] | Output [2] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[9::Table 8-1]]

Source conflict: the function table's note gives the output state H as
"Driving High", while the overview says the open-drain outputs require
pull-up resistors to perform correctly. Both as printed.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
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
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Section 6.3 is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5-V maximum.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 2500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
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
| V~O~ | Output voltage | | 0 | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV05A | < | < | < | Unit |
| ^ | ^ | D, 14 pins | DGV, 14 pins | NS, 14 pins | PW, 14 pins | ^ |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 94.9 | 130.4 | 91.4 | 122.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 56.3 | 53.4 | 49.0 | 51.3 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 49.2 | 63.5 | 50.2 | 64.4 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 20.7 | 7.3 | 15.3 | 6.8 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 48.9 | 62.8 | 49.8 | 63.8 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | 0.4 | | 0.4 | V |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | 0.44 | | 0.44 | V |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | 0.55 | | 0.6 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | ±1 | | ±1 | µA |
| I~CC~ | Static supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 | | | 20 | | 20 | | 20 | µA |
| I~off~ | Input/Output PowerOff Leakage Current | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | 5 | | 5 | µA |
| C~i~ | Input capacitance V [1] | V~I~ = V~CC~ or GND | 3.3 V | | 2.5 | | | | | | pF |

[1] Source error: the source prints a stray "V" after "Input capacitance"; kept as printed.
```
[[2::table #1]] [[5::table #3]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | SN74LV05A | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.55 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.04 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.12 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.97 | V |
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 2.5 | pF |
| ^ | ^ | ^ | 5 V | 3 | pF |
```
[[7::table #1]]

## Switching characteristics

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.6 [1] | 10.4 [1] | 1 | 13 | 1 | 13.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.8 [1] | 12.2 [1] | 1 | 15 | 1 | 16.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 6.1 | 15.2 | 1 | 18 | 1 | 18.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.1 | 16.6 | 1 | 19.5 | 1 | 21 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #2]]

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 2.9 [1] | 7.1 [1] | 1 | 8.5 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4 [1] | 7.1 [1] | 1 | 8.5 | 1 | 9.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 4.7 | 10.6 | 1 | 12 | 1 | 12.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.8 | 10.6 | 1 | 12 | 1 | 13 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 2.2 [1] | 5.5 [1] | 1 | 6.5 | 1 | 7 | ns |
| t~PHL~ | ^ | ^ | ^ | | 2.9 [1] | 5.5 [1] | 1 | 6.5 | 1 | 7.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 3.4 | 7.5 | 1 | 8.5 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.2 | 7.5 | 1 | 8.5 | 1 | 9.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="Figure 6-1. TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.3], [0, 2.7], [25, 2.9], [50, 3.0], [85, 3.2], [125, 3.6]]}
 ]}
```
[[7::Figure 6-1]]

```curve id=tpd-vcc caption="Figure 6-2. TPD vs V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 5.8], [3.3, 4], [5, 2.9]]}
 ]}
```
[[7::Figure 6-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. [[8::Figure 7-1]]

Conversion note: C~L~ is 15 pF or 50 pF as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-ttl caption="Load circuit for open-drain outputs"
RL: 1 kΩ
CL: 15 pF or 50 pF
```
[[8::Figure 7-1]]

Conversion note: the voltage waveforms of Figure 7-1 (propagation delay
times) show an input pulse between 0 V and V~CC~ and the inverted
output; t~PHL~ is measured from 50% V~CC~ of the rising input to
50% V~CC~ of the falling output, and t~PLH~ from 50% V~CC~ of the
falling input to V~OL~ + 0.3 V of the output, which rises from V~OL~ to
≈V~CC~.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

SN74LV05A is a low-drive, open-drain CMOS device that can be used for a
multitude of buffer type functions. The inputs are 5.5-V tolerant and
the outputs are open-drain and 5.5-V tolerant, allowing it to translate
up to 5.5 V or down to any other voltage between GND and 5.5 V. [[10::2]]

Conversion note: the typical application schematic (Figure 9-1) shows a
buffer function, in which two LV05A inverters, each driven by a µC or
logic, have their outputs tied together and to a pull-up resistor to
VPU as a wired OR to a µC or logic, and a basic LED driver, in which one
LV05A inverter drives an LED in series with a resistor to VPU.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions [[10::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     Recommended Operating Conditions table. [[10::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[11::1]]
2. Recommend Output Conditions [[11::1]]
   - Load currents should not exceed 35 mA per output and 50 mA total
     for the part. [[11::1]]

Source conflict: the design procedure allows load currents of 35 mA per
output, while the absolute maximum ratings give a continuous output
current of ±25 mA. Both as printed.

Conversion note: the application curve (Figure 9-2, Output at Power Up
with 4k Pull-Up 3.3 V) plots two traces, "VOUT input high" and "VOUT
input low", over a supply ramp from 0 to 3.3 V; one trace follows the
ramp to 3.3 V and the other follows it to about 0.6 V and then falls
to 0.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[12::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[12::2]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states. [[12::3]]

Specified in Figure 9-3 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. [[12::4]]

Conversion note: the layout diagram (Figure 9-3) shows one gate with its
unused input tied to V~CC~ and one with its unused input tied to
ground.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74LV05AD | | -40 to 125 | LV05A | Obsolete
SN74LV05ADGVR | Reel of 2000 | -40 to 125 | LV05A
SN74LV05ADR | Reel of 2500 | -40 to 125 | LV05A
SN74LV05ANSR | Reel of 2000 | -40 to 125 | 74LV05A
SN74LV05APW | | -40 to 125 | LV05A | Obsolete
SN74LV05APWR | Reel of 2000 | -40 to 125 | L05A, LV05A
SN74LV05APWT | | -40 to 125 | LV05A | Obsolete
```
[[14::table #1]]

## Conversion notes

- Source pages 1–13 converted (of page 13 nothing is kept); the package
  option addendum (pp.14–15), tape and reel and mechanical pages
  (16–26) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists (three feature lines are in
  Description), package information table, table of contents, revision
  history, the device and documentation support section, the
  simplified schematic (p.1), the vendor pinout drawing, the
  functional block diagram, the typical application schematic
  (Figure 9-1), the application curve (Figure 9-2) and the layout
  diagram (Figure 9-3), the last four each described in one sentence.
- The package option addendum also lists SN74LV05APWRG4, which the
  catalog does not carry.
- "Section 6.3" of the source is written as that section's name,
  Recommended Operating Conditions, except in the note of the absolute
  maximum ratings, which is kept whole.
- The thermal table of the source prints "14 PINS" once under the four
  package columns; it is written in each column header here.
- The I~CC~ row of the electrical characteristics prints its V~CC~ as
  "5.5", without a unit; kept as printed.
- The open-drain load circuit of Figure 7-1 (p.8) is drawn with the
  shared figure; its voltage waveforms are described in one sentence.
  The figure prints no C~L~ value.
- Curves digitized from Figures 6-1 and 6-2 (p.7) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} (p.1 and p.9); the curve points of
  Figures 6-1 and 6-2 (p.7); 1 kΩ (R~L~), 50% V~CC~, V~OL~ + 0.3 V,
  ≈V~CC~ and 0 V (Figure 7-1, p.8); the labels LV05A and VPU of
  Figure 9-1 (p.10); the trace labels and the 0.6 V of Figure 9-2
  (p.11).
- From images: the notes of Figure 7-1 (p.8): "A. C~L~ includes probe
  and jig capacitance. B. All input pulses are supplied by generators
  having the following characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω,
  t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. C. The outputs are measured one at a time,
  with one input transition per measurement."
- Source issues:
  - Source error: the C~i~ row of the electrical characteristics (p.6)
    prints "Input capacitance V"; kept as printed.
  - Source conflict: the function table's note (p.9) gives the output
    state H as "Driving High", while the overview (p.9) says the
    open-drain outputs require pull-up resistors. Both kept.
  - Source conflict: the detailed design procedure (p.11) allows 35 mA
    per output, while the absolute maximum ratings (p.4) give a
    continuous output current of ±25 mA. Both kept.
  - Source conflict: Application Information (p.10) calls the part a
    "low-drive, open-drain CMOS device", while Design Requirements
    (p.10) says it "has balanced output drive" and "The high drive will
    also create fast edges". Both kept.
  - Source error: in Figure 9-2 (p.11) the trace that falls to 0 is
    labelled "VOUT input low" and the trace that follows the supply
    "VOUT input high", against the function table (input H gives output
    L), and the vertical axis is labelled V~CC~ and the horizontal axis
    VOUT; kept out of the description above, which names no trace.
