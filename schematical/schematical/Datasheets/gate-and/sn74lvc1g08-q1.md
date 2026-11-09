---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g08-q1",
  "title": "SN74LVC1G08-Q1 single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc1g08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES556H",
    "date": "2026-08",
    "pages": 30
  },
  "devices": ["gate:1G08", "gate:1G08_SON"],
  "parts": [
    "74LVC1G08QDBVRQ1G4", "74LVC1G08QDCKRQ1G4", "SN74LVC1G08IDCKRQ1", "SN74LVC1G08QDBVRQ1",
    "SN74LVC1G08QDCKRQ1", "SN74LVC1G08QDRYRQ1"
  ],
  "packages": {"p5": "DBV, DCK", "p6": "DRY"},
  "converted": "2026-11-08"
}
---

# SN74LVC1G08-Q1 single 2-input positive-AND gate

## Description

This single 2-input positive-AND gate is designed for 1.65 V to 5.5 V
V~CC~ operation. The SN74LVC1G08-Q1 device performs the Boolean function
Y = A · B in positive logic. It is AEC-Q100 qualified for automotive
applications.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when powered down. The inputs accept
voltages to 5.5 V, which allows down voltage translation to V~CC~. The
CMOS device has high output drive while maintaining low static power
dissipation over a broad V~CC~ operating range.

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground
NC: No internal connection
VCC: Positive Supply
Y: Output Y
```

## Function tables

```table id=function caption="Function table"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per AEC Q100-011 | ±1000 | V |

[1] AEC Q100-002 indicates that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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
| T~A~ | Operating free-air temperature | Q-suffix devices | -40 | 125 | °C |
| ^ | ^ | I-suffix devices | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Implications of Slow or Floating CMOS Inputs application note.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (5 pins) | DCK (5 pins) | DRY (6 pins) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371 | 264.4 | °C/W |
| R~θJCtop~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | 166.6 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | 142.2 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | 26.1 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | 141.6 | °C/W |
| R~θJCbot~ | Junction-to-case (bottom) thermal resistance | | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application note.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40 °C to 85 °C | < | < | –40 °C to 125 °C recommended | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.15 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | Partial-power-down leakage | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | Supply current | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | Supply-current change | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25 °C.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-125 caption="Switching characteristics, T~A~ = –40 °C to 125 °C [1]" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | –40 °C to 125 °C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1 | 10 | 1 | 7.5 | 1 | 6.5 | 1 | 6 | ns |

[1] Unconfirmed: this table refers to Figure 6-1 (C~L~ = 15 pF), yet its maxima exceed those of the –40 °C to 85 °C table, which uses the heavier Figure 6-2 loads (30 pF and 50 pF).
```

```table id=sw-85 caption="Switching characteristics, T~A~ = –40 °C to 85 °C" conditions="Over recommended operating free-air temperature range, (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40 °C to 85 °C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.4 | 8 | 1.1 | 5.5 | 1 | 4.5 | 1 | 4 | ns |
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 21 | 24 | 26 | 31 | pF |
```

## Typical characteristics

```curve id=tpd-temp caption="Propagation delay (t~pd~) across temperature at 3.3 V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [0, 3.0], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```

```curve id=tpd-vcc caption="Propagation delay (t~pd~) across V~CC~ at 25 °C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 6.9], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
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
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

The load circuit also has an R~L~ from the output to switch S1 (V~LOAD~,
open or GND). S1 is open for t~PLH~/t~PHL~, which leaves the load below.
The source figures also show pulse-duration, setup/hold and
enable/disable waveforms; all parameters and waveforms are not
applicable to all devices.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 6-1 and Figure 6-2)"
| V~CC~ | Figure | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | 6-1 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | 6-1 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 6-1 | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | 6-1 | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
| 1.8 V ± 0.15 V | 6-2 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 6-2 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 6-2 | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | 6-2 | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: see table
```

## Application information

The SN74LVC1G08-Q1 is a high-drive CMOS device used for implementing AND
logic with a high output drive, such as LED applications. The device
produces 24 mA of drive current at 3.3 V, for driving multiple outputs
and high speed applications up to 100 MHz. The inputs are 5.5 V tolerant
allowing the device to translate down to V~CC~. The source's examples
(Figure 8-1) show the gate between microcontroller or logic signals, and
as a basic LED driver with the LED and a series resistor from Y to
ground.

This device uses CMOS technology and has balanced output drive. Avoid
bus contention because it can drive currents that would exceed maximum
limits. The high drive will also create fast edges into light loads so
routing and load conditions must be considered to prevent ringing.

Design procedure:

1. Inputs: rise and fall times per Δt/Δv, and high and low levels per
   V~IH~ and V~IL~, in the
   [recommended operating conditions](#recommended-operating-conditions).
   Inputs are overvoltage tolerant allowing them to go as high as
   V~I~ max at any valid V~CC~.
2. Outputs: load currents must not exceed I~O~ max per output and must
   not exceed total current (continuous current through V~CC~ or GND)
   for the part, as given in the
   [absolute maximum ratings](#absolute-maximum-ratings). Outputs must
   not be pulled above V~CC~.

Power: the supply can be any voltage between the min and max supply
voltage rating in the recommended operating conditions. Verify that each
V~CC~ pin has a good bypass capacitor. For devices with a single supply,
a 0.1 µF capacitor is recommended. 0.1 µF and 1 µF capacitors are
commonly used in parallel. Install the bypass capacitor as close to the
power pin as possible for best results.

Layout: verify that the inputs do not ever float. Do not leave unused
input pins unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs
of digital logic devices must be connected to a high or low bias to
prevent them from floating; generally the level is tied to GND or V~CC~,
whichever makes more sense or is more convenient.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SON-6-DRY
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–12 converted; support pages, revision history (13–14),
  tape and reel, the package option addendum and mechanical pages
  (14–30) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists, package information table,
  feature description bullets (repeated in Description), logic diagram
  (one AND gate), the layout example (Figure 8-3, an unused input tied to
  V~CC~ or GND); the application circuits (Figure 8-1) are described in
  text.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 6-1 and 6-2 are the
  same circuit with different values, joined in one table.
- Curves digitized from Figures 5-1, 5-2 and 8-2 to about 2 % of full
  scale.
- The text layer prints "£2 ns", "1 MW", "1 kW" and "500 W"; the page
  image shows ≤ 2 ns, 1 MΩ, 1 kΩ and 500 Ω.
- Parameter names in the electrical characteristics table are added
  (the source gives symbols only).
- The multi-V~CC~-pin bypass advice (0.01 µF or 0.022 µF per pin) is
  dropped: this device has one V~CC~ pin.
- Source issues:
  - Source error: the second form of the Boolean function is printed
    with one overbar over A + B (§ 7.1, p.10) and as Ā + B̅ in the text
    layer (p.1); neither is an AND. The function table (p.10) gives
    Y = A · B. Kept out.
  - Unconfirmed: the –40 °C to 125 °C switching table refers to
    Figure 6-1 (C~L~ = 15 pF) but its maxima exceed the –40 °C to 85 °C
    values measured with Figure 6-2's 30 pF and 50 pF loads (p.6).
  - Unconfirmed: ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.5).
