---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g08-q1",
  "title": "SN74LVC2G08-Q1 dual 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc2g08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES557E",
    "date": "2020-10",
    "pages": 24
  },
  "devices": ["gate:2G08"],
  "parts": ["SN74LVC2G08IDCTRQ1", "SN74LVC2G08QDCURQ1"],
  "packages": {"p8": "DCT, DCU"},
  "converted": "2026-11-08"
}
---

# SN74LVC2G08-Q1 dual 2-input positive-AND gate

## Description

This dual 2-input positive-AND gate is designed for 1.65-V to 5.5-V
V~CC~ operation. The SN74LVC2G08-Q1 performs the Boolean function
Y = A • B or Y = !{!{A} + !{B}} in positive logic. It is AEC-Q100
qualified: device temperature grade 1 (–40 °C to +125 °C) in the DCU
package and grade 3 (–40 °C to +85 °C) in the DCT package. It can be
used as a down translator to translate input from a maximum of 5.5 V
down to the V~CC~ level.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

## Pinout

```pinout pinout=p8
```

## Pin functions

```pins
1A, 1B: Channel 1 logic input
2A, 2B: Channel 2 logic input
1Y, 2Y: Logic level output
GND: Ground
VCC: Power Supply
```

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [1] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [1] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [1] [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | | | -50 | mA |
| I~OK~ | Output clamp current | | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[2] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 | ±1000 | V |

[1] AEC Q100-002 indicates HBM stressing is done in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

Latch-up performance exceeds 100 mA per JESD 78, Class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | SN74LVC2G08DCU-Q1 | -40 | 125 | °C |
| ^ | ^ | SN74LVC2G08DCT-Q1 | -40 | 85 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DCT (SM8), 8 pins | DCU (VSSOP), 8 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 220 | 201.5 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 117.2 | 91.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 100 | 122.6 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 42.4 | 31.8 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 98.9 | 122.1 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | Partial-power-down leakage | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | Supply current | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | Supply-current change | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND, T~A~ = –40 °C to 85 °C | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND, T~A~ = –40 °C to 85 °C | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25 °C.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics"
| Symbol | From (input) | To (output) | T~A~ | V~CC~ | Min | Max | Unit |
|---|---|---|---|---|--:|--:|---|
| t~pd~ | A or B | Y | –40 °C to 85 °C | V~CC~ = 1.8 V ± 0.15 V | 2.6 | 9 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1 | 5.1 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1 | 4.7 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 3.8 | ns |
| ^ | ^ | ^ | –40 °C to 125 °C | V~CC~ = 1.8 V ± 0.15 V | 2.6 | 9.8 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1 | 5.8 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1 | 5.3 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 4.8 | ns |
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 17 | 17 | 17 | 20 | pF |
```

## Typical characteristics

```curve id=tpd-temp caption="t~PD~ across temperature at 3.3-V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [0, 3.0], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```

```curve id=tpd-vcc caption="t~PD~ across V~CC~ at 25 °C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 7.0], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [-20, 80]},
 "y": {"label": "ICC", "unit": "mA", "range": [-2, 10]},
 "series": [
  {"label": "ICC 1.8 V", "points": [[0, 0], [10, 0.3], [40, 1.1], [60, 1.6], [75, 2.0]]},
  {"label": "ICC 2.5 V", "points": [[0, 0], [40, 1.9], [60, 2.5], [70, 2.9], [75, 3.5]]},
  {"label": "ICC 3.3 V", "points": [[0, 0], [30, 1.9], [49, 3.0], [55, 3.7], [60, 4.0], [75, 5.2]]},
  {"label": "ICC 5 V", "points": [[0, 0], [25, 2.9], [30, 3.9], [49, 6.0], [55, 6.9], [67, 8.1], [75, 9.3]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement.

The load circuit also has an R~L~ from the output to switch S1 (V~LOAD~,
open or GND). S1 is open for t~PLH~/t~PHL~, which leaves the load below.
The source figure also shows pulse-duration, setup/hold and
enable/disable waveforms.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 8-1)"
| V~CC~ | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: see table
```

## Application information

The SN74LVC2G08-Q1 is a high-drive CMOS device that can be used for
implementing AND logic with a high output drive, such as an LED
application. It can produce 24 mA of drive current at 3.3 V, making it
ideal for driving multiple outputs and good for high-speed applications
up to 100 MHz. The inputs are 5.5-V tolerant allowing it to translate
down to V~CC~. The source's examples (Figure 10-1) show a gate between
microcontroller or logic signals, and a basic LED driver with the LED
and a resistor from Y to ground.

Outputs: balanced CMOS push-pull; the device can sink and source similar
currents. The drive capability may create fast edges into light loads so
routing and load conditions should be considered to prevent ringing. The
outputs are capable of driving larger currents than the device can
sustain without being damaged. It is important for the output power of
the device to be limited to avoid damage due to overcurrent. The electrical and thermal
limits defined in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed
at all times. Take care to avoid bus contention because it can drive
currents that would exceed maximum limits. Unused push-pull CMOS outputs
should be left disconnected.

Inputs: standard CMOS, high impedance, typically modeled as a resistor
in parallel with the input capacitance; the worst case resistance is the
maximum input voltage divided by the maximum input leakage current
(R = V ÷ I). Standard CMOS inputs require that input signals transition
between valid logic states quickly, as defined by the input transition rate in the
[recommended operating conditions](#recommended-operating-conditions);
failing to meet this will result in excessive power consumption and
could cause oscillations. Do not leave standard CMOS inputs floating at
any time during operation. Unused inputs must be terminated at V~CC~ or
GND; an input that is not actively driven at all times can have a
pull-up or pull-down resistor; a 10-kΩ resistor is recommended and will
typically meet all requirements.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

The inputs and outputs have negative clamping diodes only. The input and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

Partial power down (I~off~): circuitry disables all outputs when the
supply pin is held at 0 V; the outputs will then neither source nor
sink current, regardless of the input voltages applied.

Design procedure:

1. Inputs: rise and fall times per Δt/Δv, and high and low levels per
   V~IH~ and V~IL~, in the recommended operating conditions. Inputs are
   overvoltage tolerant allowing them to go as high as V~I~ maximum at
   any valid V~CC~.
2. Outputs: load currents must not exceed I~O~ maximum per output and
   must not exceed total current (continuous current through V~CC~ or
   GND) for the part; these limits are in the absolute maximum ratings.
   Outputs must not be pulled above V~CC~ in normal operating
   conditions.

Power: the power supply can be any voltage between the minimum and
maximum supply voltage rating in the recommended operating conditions.
Each V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1-µF capacitor is
recommended. 0.1-µF and 1-µF capacitors are commonly used in parallel.
The bypass capacitor must be installed as close to the power pin as
possible for best results.

Layout: inputs must not ever float. Unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a high or low bias to prevent them
from floating; generally they will be tied to GND or V~CC~, whichever
makes more sense or is more convenient.

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1 and 3–12 converted; revision history (p.2), support
  pages, the package option addendum, tape and reel and mechanical pages
  (13–24) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists, device information table,
  logic diagram (two AND gates), the layout example (Figure 13-1: an
  unused input tied to V~CC~ or GND), the multi-V~CC~-pin bypass advice
  (this device has one V~CC~ pin). The typical applications
  (Figure 10-1) are described in text.
- Curves digitized from Figures 7-1, 7-2 and 11-1 to about 2 % of full
  scale.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figure 8-1 prints only notes
  A to D; the measurement waveforms for enable/disable, setup/hold and
  pulse duration are not used by any parameter of this device.
- The text layer prints "£2 ns", "1 kW", "500 W" and the Boolean
  function as "Y + A • B or Y + A ) B"; the page images show ≤ 2 ns,
  1 kΩ, 500 Ω and Y = A • B or Y = !{!{A} + !{B}} (pp.1, 8, 9).
- Parameter names in the electrical characteristics table are added
  (the source gives symbols only).
- Source issues:
  - Source error: § 10.2.1.1 says the I~O~ and continuous-current limits
    "are located in the Recommended Operating Conditions table"; they are
    in the Absolute Maximum Ratings (p.4), as § 9.3.1 says. Corrected.
