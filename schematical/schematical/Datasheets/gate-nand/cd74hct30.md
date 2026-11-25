---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hct30",
  "title": "CDx4HCT30 single 8-input NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/cd74hct30.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS419",
    "date": "2020-06",
    "pages": 27
  },
  "devices": ["gate:74_30"],
  "parts": ["5962-8974601CA", "CD54HCT30F3A", "CD74HCT30E", "CD74HCT30M", "CD74HCT30M96", "CD74HCT30MT"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CDx4HCT30 single 8-input NAND gate

## Description

This device contains one independent 8-input NAND gate. Each gate
performs the Boolean function Y = !{A ● B ● C ● D ● E ● F ● G ● H} in positive logic.
LSTTL input logic compatible: V~IL(max)~ = 0.8 V, V~IH(min)~ = 2 V. CMOS
input logic compatible: I~I~ ≤ 1 µA at V~OL~, V~OH~. Buffered inputs.
4.5 V to 5.5 V operation. Wide operating temperature range: -55°C to
+125°C. Supports fanout up to 10 LSTTL loads. Significant power
reduction compared to LSTTL logic ICs. [[1::1]] [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
A: Input A
B: Input B
C: Input C
D: Input D
E: Input E
F: Input F
GND: Ground
Y: Output Y
NC: Not internally connected
G: Input G
H: Input H
VCC: Positive Supply
```
[[1::table #1]]

## Functional description

This device contains one independent 8-input NAND gate. Each gate
performs the Boolean function Y = !{A ● B ● C ● D ● E ● F ● G ● H} in positive logic. [[1::1]]

Conversion note: the functional block diagram shows one 8-input NAND
gate, inputs xA to xH, output xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges into
light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable of
driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the Absolute Maximum Ratings must be followed at all
times. [[8::2]]

The CD74HCT30 can drive a load with a total capacitance less than or
equal to the maximum load listed in the Switching Characteristics
connected to a high-impedance CMOS input while still meeting all of the
datasheet specifications. Larger capacitive loads can be applied,
however it is not recommended to exceed the provided load value. If
larger capacitive loads are required, it is recommended to add a series
resistor between the output and the capacitor to limit output current to
the values given in the Absolute Maximum Ratings. [[8::3]]

### TTL-compatible CMOS inputs

TTL-Compatible CMOS inputs are high impedance and are typically modeled
as a resistor from the input to ground in parallel with the input
capacitance given in the Electrical Characteristics. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings, and the maximum input leakage current, given
in the Electrical Characteristics, using ohm's law (R = V ÷ I). [[8::3]] [[8::4]]

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in the Recommended Operating Conditions to avoid excessive
current consumption and oscillations. Source conflict: the Recommended
Operating Conditions table gives the input transition time t~t~, not
Δt/Δv. If a slow or noisy input signal is required, a
device with a Schmitt-trigger input should be used to condition the
input signal prior to the TTL-compatible CMOS input. [[8::5]]

TTL-Compatible CMOS inputs have a lower threshold voltage than standard
CMOS inputs to allow for compatibility with older bipolar logic devices.
See the Recommended Operating Conditions for the valid input voltages
for the CD74HCT30. [[9::1]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes as depicted in Figure 8-1. Voltages beyond the values
specified in the Absolute Maximum Ratings table can cause damage to the
device. The recommended input and output voltage ratings may be exceeded
if the input and output clamp-current ratings are observed. [[9::2]] [[9::3]]

```figure ref=clamp-diodes caption="Figure 8-1. Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 8-1]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | < | < | < | < | < | Output |
| A | B | C | D | E | F | G | H | Y |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| H | H | H | H | H | H | H | H | L |
| L | X | X | X | X | X | X | X | H |
| X | L | X | X | X | X | X | X | H |
| X | X | L | X | X | X | X | X | H |
| X | X | X | L | X | X | X | X | H |
| X | X | X | X | L | X | X | X | H |
| X | X | X | X | X | L | X | X | H |
| X | X | X | X | X | X | L | X | H |
| X | X | X | X | X | X | X | L | H |
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ > -0.5 V or V~O~ < V~CC~ + 0.5 V [4] | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | Plastic package | | 150 | °C |
| ^ | ^ | Hermetic package or die | | 175 | °C |
| | Lead temperature (soldering 10s) | SOIC - Lead tips only | | 300 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Rating may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Condition. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Guaranteed by design.
[4] Unconfirmed: the source gives this condition as printed, "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V", which every output voltage meets.
```
[[4::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 V to 5.5 V | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 V to 5.5 V | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| t~t~ | Input transition time | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 5.5 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | | -55 | | 125 | °C |
```
[[4::table #2]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | CD74HCT30 | < | Unit |
| ^ | ^ | N (PDIP) 14 pins | D (SOIC) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 64.7 | 101.8 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 52.4 | 58.3 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 44.4 | 57.7 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 32.0 | 19.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 44.2 | 57.3 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[4::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | Test conditions | < | V~CC~ | 25°C | < | < | –40°C to 85°C | < | –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20 µA | 4.5 V | 4.4 | | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | | | 3.84 | | 3.7 | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | | 0.26 | | 0.33 | | 0.4 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ and GND | I~O~ = 0 | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND | I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 40 | µA |
| ΔI~CC~ [1] | Additional Quiescent Device Current Per Input Pin. | V~I~ = V~CC~ – 2.1 | | 4.5 V to 5.5 V | | 100 | 360 | | 450 | | 490 | µA |
| C~i~ | Input capacitance | | < | 5 V | | | 10 | | 10 | | 10 | pF |

[1] For dual-supply systems theoretical worst case (V~I~ = 2.4 V, V~CC~ = 5.5 V) specification is 1.8 mA.
```
[[5::table #1]]

```table id=op caption="Operating characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 5 V | | 26 | | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | From | To | Test conditions | V~CC~ | 25°C | < | –40°C to 85°C | –55°C to 125°C | Unit |
| ^ | ^ | ^ | ^ | ^ | ^ | Typ | Max | Max | Max | ^ |
|---|---|---|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B [1] | Y | C~L~ = 50 pF | 4.5 V | | 28 | 35 | 42 | ns |
| ^ | ^ | A or B [1] | Y | C~L~ = 15 pF | 5 V | 11 | | | | ns |
| t~t~ | Transition-time | | Y | C~L~ = 50 pF | 4.5 V | | 15 | 19 | 22 | ns |

[1] Unconfirmed: the source prints "A or B" as the input; the device has inputs A to H.
```
[[5::table #3]]

## Typical characteristics

T~A~ = 25°C [[5::1]]

```curve id=voh caption="Figure 6-1. Typical output voltage in the high state (V~OH~)"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 7]},
 "series": [
  {"label": "2-V", "points": [[0, 1.98], [1, 1.82], [1.5, 1.68]]},
  {"label": "4.5-V", "points": [[0, 4.49], [4, 4.33]]},
  {"label": "6-V", "points": [[0, 5.98], [5.5, 5.82]]}
 ]}
```
[[6::Figure 6-1]]

```curve id=vol caption="Figure 6-2. Typical output voltage in the low state (V~OL~)"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.3]},
 "series": [
  {"label": "2-V", "points": [[0, 0], [1.5, 0.097]]},
  {"label": "4.5-V", "points": [[0, 0], [4, 0.115]]},
  {"label": "6-V", "points": [[0, 0], [5.5, 0.126]]}
 ]}
```
[[6::Figure 6-2]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. The outputs are measured one at a
time, with one input transition per measurement. C~L~= 50 pF and
includes probe and jig capacitance. t~t~ is the greater of t~r~ and
t~f~. The maximum between t~PLH~ and t~PHL~ is used for t~pd~. [[7::1]] [[7::2]] [[7::4]]

```figure ref=load-cmos-50pf caption="Figure 7-1. Load circuit"
CL: 50 pF
```
[[7::Figure 7-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Figures 7-2 and 7-3. Voltage waveforms, transition times and propagation delays"
VM: 50%
```
[[7::Figure 7-2]]

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

In this application, two 8-input NAND gates are used to create an
active-low SR latch as shown in Figure 9-1. Unused inputs can be tied
together or to V~CC~ [[10::2]]

This device is used to drive the tamper indicator LED and provide one
bit of data to the system controller. When the tamper switch outputs
LOW, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a LOW signal to the R
input which returns the Q output back to LOW. [[10::3]]

Conversion note: the typical application schematic (Figure 9-1) shows
two cross-coupled NAND gates, one with input R driven by the system
controller through R1, the other with inputs SA, SB-F and SG driven by
Tamper Switch 1 to Tamper Switch 7, and output Q driving the system
controller and, through R2, the tamper indicator LED.

Conversion note: the typical application timing diagram (Figure 9-2)
shows Q going high when S goes low and going low again when R goes low.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics. [[10::4]]

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the CD74HCT30 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics. The
logic device can only source or sink as much current as it is provided
at the supply and ground pins, respectively. Be sure not to exceed the
maximum total current through GND or V~CC~ listed in the Absolute
Maximum Ratings. [[10::5]]

Total power consumption can be calculated using the information
provided in CMOS Power Consumption and Cpd Calculation. [[10::6]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[10::7]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::1]]

### Input considerations

Unused inputs must be terminated to either V~CC~ or ground. These can be
directly terminated if the input is completely unused, or they can be
connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a default
state of HIGH, and a pull-down resistor is used for a default state of
LOW. The resistor size is limited by drive current of the controller,
leakage current into the CD74HCT30, as specified in the Electrical
Characteristics, and the desired input transition rate. A 10-kΩ resistor
value is often used due to these factors. [[11::2]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
Similarly, the ground voltage is used to produce the output LOW voltage.
Sinking current into the output will increase the output voltage as
specified by the V~OL~ specification in the Electrical Characteristics. [[11::4]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::5]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. [[11::7]]
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   CD74HCT30 to the receiving device. [[11::7]]
3. Ensure the resistive load at the output is larger than (V~CC~ /
   I~O(max)~) Ω. This will ensure that the maximum output current from
   the Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in megaohms; much larger than the minimum
   calculated above. [[11::7]]
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   Cpd Calculation [[11::7]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in Figure 11-1. [[12::1]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
not ever be left floating. In many cases, functions or parts of
functions of digital logic devices are unused; for example, when only
two inputs of a triple-input AND gate are used. Such unused input pins
must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. All unused
inputs of digital logic devices must be connected to a logic high or
logic low voltage, as defined by the input voltage specifications, to
prevent them from floating. The logic level that must be applied to any
particular unused input depends on the function of the device.
Generally, the inputs are tied to GND or V~CC~, whichever makes more
sense for the logic function or is more convenient. [[13::1]]

Conversion note: Figure 11-1, the example layout for the CD74HCT30
(p.13), recommends a GND flood fill, a 0.1 µF bypass capacitor placed
close to the device, unused inputs tied to V~CC~, NC pins left floating,
and no 90° corners for signal lines.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-8974601CA | Tube of 25 | -55 to 125 | 5962-8974601CA CD54HCT30F3A
CD54HCT30F3A | Tube of 25 | -55 to 125 | 5962-8974601CA CD54HCT30F3A
CD74HCT30E | Tube of 25 | -55 to 125 | CD74HCT30E
CD74HCT30M | | -55 to 125 | HCT30M | Obsolete
CD74HCT30M96 | Reel of 2500 | -55 to 125 | HCT30M
CD74HCT30MT | | -55 to 125 | HCT30M | Obsolete
```
[[15::table #1]]

## Conversion notes

- Source pages 1–13 converted; the support page (14) and the package
  option addendum, tape and reel and mechanical pages (15–27) dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, device information table,
  revision history, the functional pinout and functional block diagram
  drawings, the typical application schematic and timing diagram
  (Figures 9-1 and 9-2) and the layout example (Figure 11-1), each
  described in one sentence, and the references to the Feature
  Description section in the input and output considerations.
- Curves digitized from the two typical-characteristics figures (p.6) to
  about 2 % of full scale.
- The switching table's Min columns and the Typ columns at –40°C to 85°C
  and –55°C to 125°C of the electrical and switching tables hold no
  values and are left out.
- Figures 7-2 and 7-3 (transition times, propagation delays) are drawn
  as one shared waveform figure.
- Ordering vendor columns are from the package option addendum (p.15).
- From images: overbars of Y = !{A ● B ● C ● D ● E ● F ● G ● H} (p.1, p.8); the curve
  points of Figures 6-1 and 6-2 (p.6).
- Source issues:
  - Unconfirmed: I~O~ condition "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V" (absolute maximum ratings note 4).
  - Unconfirmed: the switching characteristics give the propagation delay from "A or B" (p.5); the device has inputs A to H.
  - Source conflict: the Description says "one independent 8-input NAND gate" (p.1); the application information says "two 8-input NAND gates are used" (p.10).
  - Source conflict: the TTL-compatible CMOS inputs text points to Δt/Δv in the Recommended Operating Conditions (p.8); that table gives the input transition time t~t~ (p.4).
  - Unconfirmed: the typical-characteristics curves are labelled 2-V, 4.5-V and 6-V (p.6); the recommended supply voltage is 4.5 V to 5.5 V.
  - Source conflict: the layout guidelines give "a triple-input AND gate" as the example (p.13); this device is a single 8-input NAND gate.
