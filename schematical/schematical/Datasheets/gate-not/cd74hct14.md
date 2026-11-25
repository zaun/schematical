---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hct14",
  "title": "CDx4HCT14 hex inverters with Schmitt-trigger inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/cd74hct14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS402",
    "date": "2021-06",
    "pages": 30
  },
  "devices": ["gate:74_14"],
  "parts": ["CD74HCT14E", "CD74HCT14M", "CD74HCT14M96", "CD74HCT14MT", "CD74HCT14PWR"],
  "packages": {"p14": "D, N, PW"},
  "converted": "2026-11-21"
}
---

# CDx4HCT14 hex inverters with Schmitt-trigger inputs

## Description

This device contains six independent inverters with Schmitt-trigger
inputs. Each gate performs the Boolean function Y = !{A} in positive
logic.
LSTTL input logic compatible: V~IL(max)~ = 0.8 V, V~IH(min)~ = 2 V.
CMOS input logic compatible: I~I~ ≤ 1 µA at V~OL~, V~OH~. Buffered
inputs. 4.5 V to 5.5 V operation. Wide operating temperature range:
-55°C to +125°C. Supports fanout up to 10 LSTTL loads. Significant
power reduction compared to LSTTL logic ICs. [[1::2]] [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3Y: Channel 3, Output Y
GND: Ground
4Y: Channel 4, Output Y
4A: Channel 4, Input A
5Y: Channel 5, Output Y
5A: Channel 5, Input A
6Y: Channel 6, Output Y
6A: Channel 6, Input A
VCC: Positive Supply
```
[[1::table #1]]

## Functional description

This device contains six independent inverters with Schmitt-trigger
inputs. Each gate performs the Boolean function Y = !{A} in positive
logic. [[1::2]]

Conversion note: the functional block diagram (p.8) shows one inverter with a Schmitt-trigger input xA and output xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The drive capability of this device may create fast edges into light
loads so routing and load conditions should be considered to prevent
ringing. Additionally, the outputs of this device are capable of driving
larger currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to over-current. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[8::2]]

The CD74HCT14 can drive a load with a total capacitance less than or
equal to the maximum load listed in the Switching Characteristics
connected to a high-impedance CMOS input while still meeting all of the
datasheet specifications. Larger capacitive loads can be applied,
however it is not recommended to exceed the provided load value. If
larger capacitive loads are required, it is recommended to add a series
resistor between the output and the capacitor to limit output current to
the values given in the Absolute Maximum Ratings. [[8::3]]

### TTL-compatible Schmitt-trigger CMOS inputs

TTL-Compatible Schmitt-trigger CMOS inputs are high impedance and are
typically modeled as a resistor from the input to ground in parallel
with the input capacitance given in the Electrical Characteristics. The
worst case resistance is calculated with the maximum input voltage,
given in the Absolute Maximum Ratings, and the maximum input leakage
current, given in the Electrical Characteristics, using ohm's law
(R = V ÷ I). [[8::4]]

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs slowly will also
increase dynamic current consumption of the device. For additional
information regarding Schmitt-trigger inputs, please see Understanding
Schmitt Triggers. [[8::5]]

TTL-Compatible CMOS inputs have a lower threshold voltage than standard
CMOS inputs to allow for compatibility with older bipolar logic devices.
See the Recommended Operating Conditions for the valid input voltages
for the CD74HCT14. [[8::6]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes as depicted in Figure 8-1. [[9::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The recommended input and output
voltage ratings may be exceeded if the input and output clamp-current
ratings are observed. [[9::2]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 8-1]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| L | H |
| H | L |
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
| | Lead temperature (soldering 10s) | SOIC - lead tips only | | 300 | °C |
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

```table id=thermal caption="Thermal information (CD74HCT14)"
| Symbol | Thermal metric [1] | N (PDIP) 14 pins | D (SOIC) 14 pins | PW (TSSOP) 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 62.9 | 95.5 | 119.6 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.7 | 49.9 | 42.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 42.7 | 51.7 | 64.5 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 30.3 | 12.8 | 4.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 42.4 | 51.3 | 63.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[4::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | Test conditions | < | V~CC~ | 25°C | < | < | –40°C to 85°C | < | < | –55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive switching threshold | | < | 4.5 V | 1.2 | | 1.9 | 1.2 | | 1.9 | 1.2 | | 1.9 | V |
| ^ | ^ | | < | 5.5 V | 1.4 | | 2.1 | 1.4 | | 2.1 | 1.4 | | 2.1 | ^ |
| V~T-~ | Negative switching threshold | | < | 4.5 V | 0.5 | | 1.2 | 0.5 | | 1.2 | 0.5 | | 1.2 | V |
| ^ | ^ | | < | 5.5 V | 0.6 | | 1.4 | 0.6 | | 1.4 | 0.6 | | 1.4 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ - V~T-~) | | < | 4.5 V | 0.4 | | 1.4 | 0.4 | | 1.4 | 0.4 | | 1.4 | V |
| ^ | ^ | | < | 5.5 V | 0.4 | | 1.5 | 0.4 | | 1.5 | 0.4 | | 1.5 | ^ |
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20 µA | 4.5 V | 4.4 | | | 4.4 | | | 4.4 | | | V |
| ^ | ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | | | 3.84 | | | 3.7 | | | ^ |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 4.5 V | | | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | | 0.26 | | | 0.33 | | | 0.4 | ^ |
| I~I~ | Input leakage current | V~I~ = V~CC~ and GND | I~O~ = 0 | 5.5 V | | | ±0.1 | | | ±1 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND | I~O~ = 0 | 5.5 V | | | 2 | | | 20 | | | 40 | µA |
| ΔI~CC~ [1] | Additional Quiescent Device Current Per Input Pin. | V~I~ = V~CC~ – 2.1 | | 4.5 V to 5.5 V | | | 360 | | | 450 | | | 490 | µA |
| C~i~ | Input capacitance | | < | 5 V | | | 10 | | | 10 | | | 10 | pF |

[1] For dual-supply systems theoretical worst case (V~I~ = 2.4 V, V~CC~ = 5.5 V) specification is 1.8 mA.
```
[[5::table #1]]

```table id=op caption="Operating characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 5 V | | 20 | | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | From | To | Test conditions | V~CC~ | 25°C | < | < | –40°C to 85°C | < | < | –55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A | Y | C~L~ = 50 pF | 4.5 V | | | 38 | | | 48 | | | 57 | ns |
| ^ | ^ | ^ | ^ | C~L~ = 15 pF | 5 V | | 16 | | | | | | | | ^ |
| t~t~ | Transition-time | | Y | C~L~ = 50 pF | 4.5 V | | | 15 | | | 19 | | | 22 | ns |
```
[[5::table #3]]

## Typical characteristics

```curve id=voh caption="Typical output voltage in the high state (V~OH~), T~A~ = 25°C"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 7]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 1.98], [1, 1.85], [1.5, 1.69]]},
  {"label": "VCC = 4.5 V", "points": [[0, 4.47], [4, 4.3]]},
  {"label": "VCC = 6 V", "points": [[0, 5.98], [5.5, 5.8]]}
 ]}
```
[[6::Figure 6-1]]

```curve id=vol caption="Typical output voltage in the low state (V~OL~), T~A~ = 25°C"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.3]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 0], [1.5, 0.098]]},
  {"label": "VCC = 4.5 V", "points": [[0, 0], [4, 0.115]]},
  {"label": "VCC = 6 V", "points": [[0, 0], [5.5, 0.127]]}
 ]}
```
[[6::Figure 6-2]]

Unconfirmed: Figures 6-1 and 6-2 plot 2-V and 6-V curves, outside the 4.5 V to 5.5 V supply voltage of the recommended operating conditions.

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. The outputs are measured one at a
time, with one input transition per measurement. [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```
[[7::Figure 7-1]]

C~L~= 50 pF and includes probe and jig capacitance. [[7::2]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, transition times and propagation delays"
VM: 50%
```
[[7::Figure 7-2]]

t~t~ is the greater of t~r~ and t~f~. The maximum between t~PLH~ and
t~PHL~ is used for t~pd~. [[7::2]] [[7::4]]

Conversion note: Figures 7-2 and 7-3 (p.7) draw the input between 0 V and V~CC~, with the transition times measured between the 10% and 90% points.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

This device can be used to add an additional stage to a counter with an
external flip-flop. Because counters use a negative edge trigger, the
flip-flop's clock input must be inverted to provide this function. This
function only requires one of the six available inverters in the device,
so the remaining channels can be used for other applications needing an
inverted signal or improved signal integrity. Unused inputs must be
terminated at V~CC~ or GND. Unused outputs can be left floating. [[10::2]]

Conversion note: the typical application schematic (Figure 9-1, p.10) shows a Counter with outputs 2^0^ to 2^3^ and a D-Type Flip-Flop with output 2^4^, the counter's 2^3^ output going through one Schmitt-trigger inverter to the flip-flop's clock input, the flip-flop's !{Q} output fed back to its D input, and the Clear line on both CLR inputs.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics. [[10::3]]

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the CD74HCT14 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics. The
logic device can only source or sink as much current as it is provided
at the supply and ground pins, respectively. Be sure not to exceed the
maximum total current through GND or V~CC~ listed in the Absolute
Maximum Ratings. [[10::4]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation. [[10::5]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[10::6]]

The maximum junction temperature, T~J~(max) listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[10::7]]

### Input considerations

Input signals must cross V~t-~(min) to be considered a logic LOW, and
V~t+~(max) to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[11::1]]

Unused inputs must be terminated to either V~CC~ or ground. These can be
directly terminated if the input is completely unused, or they can be
connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a default
state of HIGH, and a pull-down resistor is used for a default state of
LOW. The resistor size is limited by drive current of the controller,
leakage current into the CD74HCT14, as specified in the Electrical
Characteristics, and the desired input transition rate. A 10-kΩ resistor
value is often used due to these factors. [[11::2]]

The CD74HCT14 has no input signal transition rate requirements because
it has Schmitt-trigger inputs. [[11::3]]

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T~(min) in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit. [[11::4]]

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[11::5]]

Source conflict: the Typical Characteristics (p.5–6) hold only the two output voltage plots, and no plot of that additional current.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
Similarly, the ground voltage is used to produce the output LOW voltage.
Sinking current into the output will increase the output voltage as
specified by the V~OL~ specification in the Electrical Characteristics. [[11::7]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[11::8]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[11::10]]
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   CD74HCT14 to the receiving device. [[11::10]]
3. Ensure the resistive load at the output is larger than (V~CC~ /
   I~O~(max)) Ω. This will ensure that the maximum output current from
   the Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in megaohms; much larger than the minimum
   calculated above. [[11::10]]
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   Cpd Calculation [[11::10]]

Conversion note: the typical application timing diagram (Figure 9-2, p.11) shows the 2^3^, !{2^3^} and 2^4^ waveforms, with the Input at 32 kHz and 2^4^ at 1 kHz.

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

Conversion note: the example layout for the CD74HCT14 (Figure 11-1, p.13) shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, one unused input tied to GND and one tied to V~CC~, an unused output left floating, and a GND flood fill.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
CD74HCT14E | Tube of 25 | -55 to 125 | CD74HCT14E
CD74HCT14M | | -55 to 125 | HCT14M | Obsolete
CD74HCT14M96 | Reel of 2500 | -55 to 125 | HCT14M
CD74HCT14MT | | -55 to 125 | HCT14M | Obsolete
CD74HCT14PWR | Reel of 2000 | -55 to 125 | HK14
```
[[15::table #1]]

## Conversion notes

- Source pages 1–13 converted; the support page (14), the package
  option addendum (p.15–16), tape and reel, tube and mechanical pages
  (17–30) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are from the package option addendum.
- Dropped: features and applications lists (the feature lines are in
  the Description), the device information table, table of contents,
  revision history, the functional block diagram, the typical
  application schematic, the application timing diagram and the layout
  example (the block diagram on p.8 and Figures 9-1, 9-2 and 11-1,
  described in text), and the "Refer to the Feature Description section"
  sentences of 9.2.1.2 and 9.2.1.3.
- Curves digitized from Figures 6-1 and 6-2 (p.6) to about 2 % of full
  scale (straight lines in the source, the 2-V V~OH~ line slightly
  bent).
- Figures 7-2 and 7-3 (transition times, propagation delays) are shown
  by one shared waveform figure. The notes A of Figures 7-1 to 7-3 are
  the sentences next to the figures.
- The catalog does not carry the CD54HCT14 (CDIP) parts and
  CD74HCT14PWR1G4 that the package option addendum lists.
- From images: overbars of Y = !{A} (p.1 and p.8), of !{Q} (Figure 9-1,
  p.10) and of !{2^3^} (Figure 9-2, p.11).
- From images: the curve points of Figures 6-1 and 6-2 (p.6): 1.98,
  1.85, 1.69, 4.47, 4.3, 5.98, 5.8, 0.098, 0.115, 0.127.
- Source issues:
  - Unconfirmed: I~O~ condition "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V"
    (absolute maximum ratings note 4).
  - Unconfirmed: the typical characteristics (p.6) plot 2-V and 6-V
    curves for a 4.5 V to 5.5 V device.
  - Source conflict: 9.2.1.2 (p.11) says the additional current of an
    input held between the rails is plotted in the Typical
    Characteristics; they hold no such plot.
