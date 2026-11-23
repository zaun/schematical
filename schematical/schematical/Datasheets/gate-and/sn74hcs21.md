---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hcs21",
  "title": "SN74HCS21 dual 4-input AND gates with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74hcs21.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS793B",
    "date": "2026-10",
    "pages": 25
  },
  "devices": ["gate:74_21"],
  "parts": ["SN74HCS21DR", "SN74HCS21PWR"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-08"
}
---

# SN74HCS21 dual 4-input AND gates with Schmitt-trigger inputs

## Description

This device contains two independent 4-input AND gates with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = A · B · C · D in positive logic. [[8::1]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A: Channel 1–2, input A
1B, 2B: Channel 1–2, input B
1C, 2C: Channel 1–2, input C
1D, 2D: Channel 1–2, input D
1Y, 2Y: Channel 1–2, output Y
GND: Ground
VCC: Positive supply
NC: Not internally connected
```
[[3::Table 4-1]]

## Functional description

Conversion note: the functional block diagram shows one of two
channels, a 4-input AND gate with inputs xA, xB, xC and xD and output
xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges
into light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed
at all times. [[8::2]]

### CMOS Schmitt-trigger inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the
[electrical characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage, given in
the absolute maximum ratings, and the maximum input leakage current,
given in the electrical characteristics, using ohm's law (R = V ÷ I). [[8::3]]

The Schmitt-trigger input architecture provides hysteresis as defined
by ΔV~T~ in the electrical characteristics, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended
to properly terminate unused inputs. Driving the inputs slowly will
also increase dynamic current consumption of the device. [[8::4]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes. Voltages beyond the values specified in the
[Section 5.1](#absolute-maximum-ratings) table can cause damage to the
device. The input negative-voltage and output voltage ratings may be
exceeded if the input and output clamp-current ratings are observed. [[8::5]] [[8::6]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | C | D | Y |
|:-:|:-:|:-:|:-:|:-:|
| H | H | H | H | H |
| L | X | X | X | L |
| X | L | X | X | L |
| X | X | L | X | L |
| X | X | X | L | L |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 or V~I~ > V~CC~ + 0.5 | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 or V~O~ > V~CC~ + 0.5 | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±35 | mA |
| | Continuous current through V~CC~ or GND | | | ±70 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings can be exceeded if the input and output current ratings are observed. Do not exceed the absolute maximum voltage supply rating.
[3] Verified by design.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±4000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1500 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 2 | | 6 | V |
| V~I~ | Input voltage | 0 | | V~CC~ | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate [1] | | | | ns/V |
| T~A~ | Ambient temperature | -40 | | 125 | °C |

[1] Max: Unlimited.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (SN74HCS21) [1]"
| Symbol | Parameter | PW (TSSOP), 14 pins | D (SOIC), 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151.7 | 133.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 79.4 | 89.0 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.7 | 89.5 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 25.2 | 45.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 94.1 | 89.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[4::table #4]]

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
| ΔV~T~ | V~T+~ - V~T-~ | | < | 2 V | 0.54 | 0.64 | 0.71 | V |
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
```
[[5::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | From | To | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B [1] | Y | 2 V | | 22 | 52 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 9 | 22 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 8 | 16 | ns |
| t~t~ | Transition-time | | Y | 2 V | | 9 | 17 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 5 | 8 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 4 | 7 | ns |

[1] Unconfirmed: the source names inputs A or B only; each gate also has inputs C and D.
```
[[5::table #2]]

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
[[6::Figure 5-1]]

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
[[6::Figure 5-2]]

Conversion note: the source also plots typical supply current against
input voltage (V~CC~ = 2 V to 6 V).

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 2.5 ns. The outputs
are measured one at a time, with one input transition per measurement.
C~L~ = 50 pF and includes probe and jig capacitance. The maximum
between t~PLH~ and t~PHL~ is used for t~pd~. [[7::1]] [[7::2]] [[7::4]]

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Propagation delays and transition times"
VM: 50 %
```
[[7::Figure 6-2]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

In this application, one channel of the SN74HCS21 is used. The
additional channel can be used individually or the inputs can be
grounded and the channel left unused. The SN74HCS21 is used to drive
the !{RESET} pin of the system controller. When any of the inputs to the
gates become LOW, the controller will be disabled. The controller will
only operate when all inputs are HIGH, indicating normal operation. [[10::2]] [[10::3]]

Conversion note: in the source's block diagram the inputs are over
current detection (!{OC}), power good (PG), an on/off switch
(ON/!{OFF}) and a case tamper switch (CASE).

- All signals in the system operate at 5V [[10::5]]
- All inputs must be HIGH for normal operation [[10::5]]
- Any input switching to LOW will force the output LOW [[10::5]]

### Power considerations

Ensure the desired supply voltage is within the range specified in the
[recommended operating conditions](#recommended-operating-conditions).
The supply voltage sets the device's electrical characteristics as
described in the [electrical characteristics](#electrical-characteristics). [[10::6]]

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the SN74HCS21 plus the maximum
supply current, I~CC~, listed in the electrical characteristics. The
logic device can only source or sink as much current as it is provided
at the supply and ground pins, respectively. Be sure not to exceed the
maximum total current through GND or V~CC~ listed in the absolute
maximum ratings. [[10::7]]

The SN74HCS21 can drive a load with a total capacitance less than or
equal to 50pF connected to a high-impedance CMOS input while still
meeting all of the datasheet specifications. Larger capacitive loads
can be applied, however it is not recommended to exceed 70pF. [[10::8]]

The maximum junction temperature, T~J(max)~ listed in the
[Section 5.1](#absolute-maximum-ratings), is an additional limitation
to prevent damage to the device. Do not violate any values listed in
the Section 5.1. These limits are provided to prevent damage to the
device. [[11::1]]

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the absolute maximum ratings. [[11::2]]

Unused inputs must be terminated to either V~CC~ or ground. These can
be directly terminated if the input is completely unused, or they can
be connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The resistor size is limited by drive current of the
controller, leakage current into the SN74HCS21, as specified in the
electrical characteristics, and the desired input transition rate. A
10kΩ resistor value is often used due to these factors. [[11::3]]

The SN74HCS21 has no input signal transition rate requirements because
it has Schmitt-trigger inputs. [[11::4]]

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the electrical characteristics. This hysteresis value
will provide the peak-to-peak limit. [[11::5]]

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the typical
characteristics. [[11::6]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the electrical characteristics.
Similarly, the ground voltage is used to produce the output LOW
voltage. Sinking current into the output will increase the output
voltage as specified by the V~OL~ specification in the electrical
characteristics. [[11::8]]

Source error: the source continues "The plots in and provide a typical
relationship between output voltage and current for this device"; the
figure references are missing, and the sentence is kept out.

Unused outputs can be left floating. [[11::9]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. [[11::11]]
2. Ensure the capacitive load at the output is ≤ 70pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74HCS21 to the receiving device. [[11::11]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / 25mA) Ω. This will ensure that the maximum output current
   from the [Section 5.1](#absolute-maximum-ratings) is not violated.
   Most CMOS inputs have a resistive load measured in megaohms; much
   larger than the minimum calculated above. [[11::11]]
4. Thermal issues are rarely a concern for logic gates. [[11::11]]

Source conflict: the procedure sizes the resistive load with 25mA; the
absolute maximum ratings give a continuous output current I~O~ of
±35 mA.

### Application curves

```wavedrom id=app-timing caption="Application timing diagram"
{"signal": [
  {"name": "OC#", "wave": "10101010"},
  {"name": "PG", "wave": "1.0.1.0."},
  {"name": "ON/OFF#", "wave": "1..0.1.."},
  {"name": "OT#", "wave": "1...0..."},
  {"name": "RESET#", "wave": "10......"}
]}
```
[[12::Figure 8-2]]

Source conflict: the timing diagram's fourth input is !{OT}; the block
diagram's fourth input is CASE.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor must be installed as close to the power
terminal as possible for best results. [[12::1]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used. Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[12::2]]

Conversion note: the source's layout example uses a GND flood fill,
places the bypass capacitor close to the device, ties unused inputs to
V~CC~, and avoids 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HCS21DR | Reel of 2500 | -40 to 125 | HCS21
SN74HCS21PWR | Reel of 2000 | -40 to 125 | HCS21
```
[[1::table #1]]

## Conversion notes

- Source pages 1–14 converted; the package option addendum, tape and
  reel and mechanical pages (15–25) dropped, replaced by the generated
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
  - Unconfirmed: the switching characteristics give t~pd~ from "A or
    B" only, though each gate has inputs A to D.
  - Source conflict: the application timing diagram's fourth input is
    !{OT}; the application block diagram's is CASE (case tamper
    switch).
  - Source conflict: the features list gives "±7.8mA output drive at
    5V"; the electrical characteristics specify I~OH~ = -7.8 mA and
    I~OL~ = 7.8 mA at 6 V.
  - Source conflict: the detailed design procedure sizes the resistive
    load as larger than (V~CC~ / 25mA) Ω to keep within the maximum
    output current of the absolute maximum ratings, which give I~O~
    ±35 mA.
  - Conversion note: "Section 5.1" in the source's text is its
    Absolute Maximum Ratings section; the words are kept and linked to
    that section.
  - Source error: the output considerations say "The plots in and
    provide a typical relationship between output voltage and current";
    the figure references are missing in the source, and the sentence
    is left out.
