---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs11",
  "title": "SN74HCS11 triple 3-input AND gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74hcs11.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS790C",
    "date": "2026-10",
    "pages": 26
  },
  "devices": ["gate:74_11"],
  "parts": ["SN74HCS11DR", "SN74HCS11PWR"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-08"
}
---

# SN74HCS11 triple 3-input AND gates with Schmitt-trigger inputs

## Description

This device contains three independent 3-input AND gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = A · B · C in positive logic.

The Schmitt-trigger inputs have no input transition rate requirements,
so slow or noisy input signals are allowed. Outputs are balanced CMOS
push-pull outputs.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Channel 1–3, input A
1B, 2B, 3B: Channel 1–3, input B
1C, 2C, 3C: Channel 1–3, input C
1Y, 2Y, 3Y: Channel 1–3, output Y
GND: Ground
VCC: Positive supply
```

## Functional description

Logic diagram (one of three channels): a 3-input AND gate, inputs xA,
xB and xC, output xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to
sink and source similar currents. The drive capability of this device
can create fast edges into light loads so routing and load conditions
must be considered to prevent ringing. The outputs are capable of
driving larger currents than the device can sustain without being
damaged; limit the output power of the device to avoid damage due to
over-current. The electrical and thermal limits defined in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed
at all times.

### CMOS Schmitt-trigger inputs

Standard CMOS inputs are high impedance and
are typically modeled as a resistor in parallel with the input
capacitance given in the
[electrical characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage (absolute
maximum ratings) and the maximum input leakage current (electrical
characteristics), using Ohm's law (R = V ÷ I). The Schmitt-trigger
input architecture provides hysteresis as defined by ΔV~T~, which makes
this device extremely tolerant to slow or noisy inputs. While the inputs
can be driven much slower than standard CMOS inputs, TI recommends
properly terminating unused inputs. Driving the inputs slowly increases
dynamic current consumption of the device.

### Clamp diode structure

The inputs and outputs have both positive and negative
clamping diodes. The input negative-voltage and output voltage ratings
can be exceeded if the input and output clamp-current ratings are
observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | H |
| L | X | X | L |
| X | L | X | L |
| X | X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 or V~I~ > V~CC~ + 0.5 | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 or V~O~ > V~CC~ + 0.5 | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±4000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1500 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 2 | 5 | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate [1] | | | | ns/V |
| T~A~ | Ambient temperature | -40 | | 125 | °C |

[1] Max: Unlimited.
```

## Thermal information

```table id=thermal caption="Thermal information (SN74HCS11) [1]"
| Symbol | Parameter | PW (TSSOP), 14 pins | D (SOIC), 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151.7 | 133.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 79.4 | 89.0 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 89.5 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 25.2 | 45.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.1 | 89.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance [2] | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
[2] N/A for both packages.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive switching threshold | | < | 2 V | 1.13 | 1.26 | 1.37 | V |
| ^ | ^ | ^ | ^ | 4.5 V | 2.31 | 2.47 | 2.66 | V |
| ^ | ^ | ^ | ^ | 6 V | 2.99 | 3.18 | 3.44 | V |
| V~T-~ | Negative switching threshold | | < | 2 V | 0.55 | 0.62 | 0.71 | V |
| ^ | ^ | ^ | ^ | 4.5 V | 1.21 | 1.36 | 1.5 | V |
| ^ | ^ | ^ | ^ | 6 V | 1.56 | 1.79 | 1.99 | V |
| ΔV~T~ | V~T+~ - V~T-~ [1] | | < | 2 V | 0.54 | 0.64 | 0.71 | V |
| ^ | ^ | ^ | ^ | 4.5 V | 0.94 | 1.11 | 1.27 | V |
| ^ | ^ | ^ | ^ | 6 V | 1.20 | 1.38 | 1.57 | V |
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20 µA | 2 V to 6 V | V~CC~ - 0.1 | V~CC~ - 0.002 | | V |
| ^ | ^ | ^ | I~OH~ = -6 mA | 4.5 V | 4.0 | 4.3 | | V |
| ^ | ^ | ^ | I~OH~ = -7.8 mA | 6 V | 5.4 | 5.75 | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 2 V to 6 V | | 0.002 | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | 0.18 | 0.30 | V |
| ^ | ^ | ^ | I~OL~ = 7.8 mA | 6 V | | 0.22 | 0.33 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ or 0 | < | 6 V | | ±100 | ±1000 | nA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0, I~O~ = 0 | < | 6 V | | 0.1 | 2 | µA |
| C~i~ | Input capacitance | | < | 2 V to 6 V | | | 5 | pF |
| C~pd~ | Power dissipation capacitance per gate | No load | < | 2 V to 6 V | | 10 | | pF |

[1] Source conflict: the revision history says the ΔV~T~ limits at 2 V V~CC~ were changed to 0.55 V (min) and 0.67 V (max); this table prints 0.54 and 0.71.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | From | To | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B or C | Y | 2 V | | 13 | 40 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 6 | 17 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 5 | 16 | ns |
| t~t~ | Transition-time | | Y | 2 V | | 9 | 17 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 5 | 8 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 4 | 7 | ns |
```

## Typical characteristics

```curve id=rol caption="Output driver resistance in low state, T~A~ = 25 °C"
{"x": {"label": "Output sink current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [26, 46]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 41.5], [1.25, 41.6], [2.5, 42.0], [3.75, 42.4], [6.25, 43.5], [8.75, 45.0], [9.75, 45.8]]},
  {"label": "VCC = 3.3 V", "points": [[0, 31.35], [5, 31.55], [10, 31.85], [15, 32.25], [20, 32.8], [25, 33.45]]},
  {"label": "VCC = 4.5 V", "points": [[0, 28.5], [5, 28.5], [10, 28.6], [15, 28.75], [20, 28.95], [25, 29.15]]},
  {"label": "VCC = 6 V", "points": [[0, 27.0], [5, 27.0], [10, 27.05], [15, 27.1], [20, 27.2], [25, 27.3]]}
 ]}
```

```curve id=roh caption="Output driver resistance in high state, T~A~ = 25 °C"
{"x": {"label": "Output source current", "unit": "mA", "range": [0, 25]},
 "y": {"label": "Output resistance", "unit": "Ω", "range": [30, 70]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0.5, 54.1], [1.25, 54.4], [3.75, 56.6], [6.25, 59.3], [8.75, 63.2], [10, 66.0]]},
  {"label": "VCC = 3.3 V", "points": [[0, 39.0], [5, 39.5], [10, 40.3], [15, 41.2], [20, 42.4], [25, 43.8]]},
  {"label": "VCC = 4.5 V", "points": [[0, 34.2], [5, 34.35], [10, 34.7], [15, 35.0], [20, 35.35], [25, 35.8]]},
  {"label": "VCC = 6 V", "points": [[0, 31.2], [5, 31.3], [10, 31.45], [15, 31.6], [20, 31.8], [25, 31.95]]}
 ]}
```

The source also plots typical supply current against input voltage
(V~CC~ = 2 V to 6 V).

## Parameter measurement information

Phase relationships between waveforms are chosen arbitrarily. All input
pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 2.5 ns. The outputs
are measured one at a time, with one input transition per measurement.
C~L~ = 50 pF and includes probe and jig capacitance. The maximum
between t~PLH~ and t~PHL~ is used for t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Propagation delays and transition times"
VM: 50 %
```

## Application information

The three 3-input AND gates work independently, but can be combined to
get up to a 7-input AND gate. Use the device with all three inputs
active, or disable one input by directly connecting it to V~CC~ to turn
the device into a 2-input AND gate.

In the source's typical application the SN74HCS11 controls the RESET
pin of a motor controller. This system requires three input signals
(power good PG, over current detection !{OC} and an on/off switch
ON/!{OFF}) to all be HIGH before the controller is enabled, and must be
disabled in the event that any one signal goes LOW. Many power good
signals use open-drain outputs which can produce slow input transition
rates during transition from LOW to Hi-Z mode. This device has
Schmitt-trigger inputs that do not have input transition rate
requirements.

```wavedrom id=app-timing caption="Application timing diagram"
{"signal": [
  {"name": "PG", "wave": "01...0..1."},
  {"name": "OC#", "wave": "0.1...0.1."},
  {"name": "ON/OFF#", "wave": "0..1......"},
  {"name": "Reset#", "wave": "0..1.0..1."}
]}
```

Power: verify that the desired supply voltage is within the range
specified in the [electrical characteristics](#electrical-characteristics).
The positive supply must be capable of sourcing, and the ground of
sinking, the total current of all outputs plus the maximum supply
current I~CC~ and any transient current required for switching. Verify
that the maximum total current through V~CC~ and GND listed in the
absolute maximum ratings is not exceeded. The maximum junction
temperature T~J(max)~ is an additional limitation to prevent damage to
the device. Each V~CC~ terminal must have a good bypass capacitor to
prevent power disturbance; a 0.1 µF capacitor is recommended. The
0.1 µF and 1 µF capacitors are commonly used in parallel. The bypass
capacitor must be installed as close to the power terminal as
possible.

Loads: the device can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the datasheet
specifications. Larger capacitive loads can be applied; however, TI
recommends adding a series resistor between the output and capacitor to
prevent excessive current. The device can drive a load with total
resistance described by R~L~ ≥ V~O~ / I~O~, with the output voltage and
current defined by V~OH~ and V~OL~; in the HIGH state, the output
voltage is the difference between the measured output voltage and the
supply voltage at the V~CC~ pin. Verify that the resistive load is
larger than (V~CC~ / I~O(max)~) Ω. Never violate the maximum output
current from the absolute maximum ratings.

Inputs: input signals must cross V~T-~(min) to be considered a logic
LOW, and V~T+~(max) to be considered a logic HIGH. Do not exceed the
maximum input voltage range found in the absolute maximum ratings.
Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
connected with a pullup or pulldown resistor if the input is used
sometimes, but not always. A 10 kΩ resistor value is often used. Noise
with a large enough amplitude can still cause issues; ΔV~T~(min)
provides the peak-to-peak limit. Schmitt-trigger inputs can be held at
any valid value without causing huge increases in power consumption.

Outputs: push-pull outputs that can be in opposite states, even for a
very short time period, must never be connected directly together. Two
channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. Unused
outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

Layout: inputs must not ever be left floating. All unused inputs must
be connected to a logic high or logic low voltage, as defined by the
input voltage specifications. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. The source's layout example uses a GND flood fill, places
the bypass capacitor close to the device, ties unused inputs to V~CC~,
leaves the unused output floating, and avoids 90° corners for signal
lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS11DR | Reel of 2500 | -40 to 125 | HCS11
SN74HCS11PWR | Reel of 2000 | -40 to 125 | HCS11
```

## Conversion notes

- Source pages 1–15 converted; the package option addendum, tape and
  reel and mechanical pages (16–26) dropped, replaced by the generated
  Package and Ordering sections (packing, temperature range and marking
  taken from the addendum).
- Dropped: features and applications lists, the "Benefits of
  Schmitt-trigger inputs" figure, revision history, support pages. The
  functional block diagram, the typical application block diagram
  (Figure 8-1) and the layout example (Figure 8-3) are described in
  text; the application timing diagram (Figure 8-2) is redrawn as
  WaveDrom.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
  The supply current curves (Figures 5-3 and 5-4) are not digitized.
- From images: the curve points in Figures 5-1 and 5-2 (p.6).
- Source issues:
  - Source conflict: ΔV~T~ limits at 2 V, 0.54/0.71 V in the electrical
    characteristics vs 0.55/0.67 V in the revision history (see the
    electrical characteristics).
  - Source conflict: the features list gives "±7.8mA output drive at
    5V"; the electrical characteristics specify I~OH~ = -7.8 mA and
    I~OL~ = 7.8 mA at 6 V.
  - Source conflict: the package information table on p.1 names part
    number SN74HCS11QDR for the SOIC package; the package option
    addendum lists SN74HCS11DR.
