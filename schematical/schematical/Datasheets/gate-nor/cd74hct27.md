---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hct27",
  "title": "CDx4HCT27 triple 3-input NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/cd74hct27.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS406",
    "date": "2019-08",
    "pages": 27
  },
  "devices": ["gate:74_27"],
  "parts": ["5962-8970301CA", "CD54HCT27F3A", "CD74HCT27E", "CD74HCT27M", "CD74HCT27M96"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CDx4HCT27 triple 3-input NOR gates

## Description

This device contains three independent 3-input NOR gates. Each gate
performs the Boolean function Y = !{A + B + C} in positive logic. LSTTL
input logic compatible: V~IL(max)~ = 0.8 V, V~IH(min)~ = 2 V. CMOS input
logic compatible: I~I~ ≤ 1 µA at V~OL~, V~OH~. Buffered inputs. 4.5 V to
5.5 V operation. Wide operating temperature range: -55°C to +125°C. Supports fanout up to 10 LSTTL
loads. Significant power reduction compared to LSTTL logic ICs.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
2A: Channel 2, Input A
2B: Channel 2, Input B
2C: Channel 2, Input C
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
3C: Channel 3, Input C
1Y: Channel 1, Output Y
1C: Channel 1, Input C
VCC: Positive Supply
```

## Functional description

This device contains three independent 3-input NOR gates. Each gate
performs the Boolean function Y = !{A + B + C} in positive logic.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges into
light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable of
driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the Absolute Maximum Ratings must be followed at all
times.

The CD74HCT27 can drive a load with a total capacitance less than or
equal to the maximum load listed in the Switching Characteristics
connected to a high-impedance CMOS input while still meeting all of the
datasheet specifications. Larger capacitive loads can be applied,
however it is not recommended to exceed the provided load value. If
larger capacitive loads are required, it is recommended to add a series
resistor between the output and the capacitor to limit output current to
the values given in the Absolute Maximum Ratings.

### TTL-compatible CMOS inputs

TTL-Compatible CMOS inputs are high impedance and are typically modeled
as a resistor from the input to ground in parallel with the input
capacitance given in the Electrical Characteristics. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings, and the maximum input leakage current, given
in the Electrical Characteristics, using ohm's law (R = V ÷ I).

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in the Recommended Operating Conditions to avoid excessive
current consumption and oscillations. Source conflict: the Recommended
Operating Conditions table gives the input transition rise and fall
time t~t~, not Δt/Δv. If a slow or noisy input signal is required, a
device with a Schmitt-trigger input should be used to condition the
input signal prior to the TTL-compatible CMOS input.

TTL-Compatible CMOS inputs have a lower threshold voltage than standard
CMOS inputs to allow for compatibility with older bipolar logic devices.
See the Recommended Operating Conditions for the valid input voltages
for the CD74HCT27.

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes. Voltages beyond the values specified in the Absolute
Maximum Ratings table can cause damage to the device. The recommended
input and output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | X | X | L |
| X | H | X | L |
| X | X | H | L |
| L | L | L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ > -0.5 V or V~O~ < V~CC~ + 0.5 V [4] | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| | Lead temperature (soldering 10s) | SOIC - lead tips only | | 300 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Rating may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Condition. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Guaranteed by design.
[4] Unconfirmed: the source gives this condition as printed, "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V", which every output voltage meets.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 V to 5.5 V | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 V to 5.5 V | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| t~t~ | Input transition rise and fall time | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 5.5 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | | -55 | | 125 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | CD74HCT27 | < | Unit |
| ^ | ^ | N (PDIP) 14 pins | D (SOIC) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 63.5 | 100.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 51.2 | 56.7 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 43.2 | 56.1 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 30.9 | 18.5 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 43.0 | 55.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | Test conditions | < | V~CC~ | 25 °C | < | < | –40 °C to 85 °C | < | –55 °C to 125 °C | < | Unit |
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

```table id=op caption="Operating characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 5 V | | 28 | | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted). Operating free-air temperature (T~A~)"
| Symbol | Parameter | From | To | Test conditions | V~CC~ | 25 °C | < | –40 °C to 85 °C | –55 °C to 125 °C | Unit |
| ^ | ^ | ^ | ^ | ^ | ^ | Typ | Max | Max | Max | ^ |
|---|---|---|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A, B, or C | Y | C~L~ = 50 pF | 4.5 V | | 23 | 29 | 35 | ns |
| ^ | ^ | ^ | Y | C~L~ = 15 pF | 5 V | 9 | | | | ns |
| t~t~ | Transition-time | | Y | C~L~ = 50 pF | 4.5 V | | 15 | 19 | 22 | ns |
```

## Typical characteristics

```curve id=voh caption="Typical output voltage in the high state (V~OH~), T~A~ = 25 °C"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 7]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 1.98], [1, 1.82], [1.5, 1.68]]},
  {"label": "VCC = 4.5 V", "points": [[0, 4.49], [4, 4.33]]},
  {"label": "VCC = 6 V", "points": [[0, 5.98], [5.5, 5.82]]}
 ]}
```

```curve id=vol caption="Typical output voltage in the low state (V~OL~), T~A~ = 25 °C"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.3]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 0], [1.5, 0.097]]},
  {"label": "VCC = 4.5 V", "points": [[0, 0], [4, 0.115]]},
  {"label": "VCC = 6 V", "points": [[0, 0], [5.5, 0.126]]}
 ]}
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. The outputs are measured one at a
time, with one input transition per measurement. C~L~ = 50 pF and
includes probe and jig capacitance. t~t~ is the greater of t~r~ and
t~f~. The maximum between t~PLH~ and t~PHL~ is used for t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and transition times"
VM: 50 %
```

## Application information

Information in the following applications sections is not part of the
TI component specification.

### Typical application

In this application, two 3-input NOR gates are used to create an SR
latch. The additional gate can be used for another application, or the
inputs can be grounded and the channel left unused.

This device is used to drive the tamper indicator LED and provide one
bit of data to the system controller. When the tamper switch outputs
HIGH, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a HIGH signal to the R
input which returns the Q output back to LOW.

Conversion note: the typical application schematic (Figure 9-1) shows
two cross-coupled NOR gates between Tamper Switch 1, Tamper Switch 2
and the system controller, with inputs RA, RB, SA and SB, output Q,
resistors R1 and R2 and the tamper indicator LED.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics.

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the CD74HCT27 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics. The
logic device can only source or sink as much current as it is provided
at the supply and ground pins, respectively. Be sure not to exceed the
maximum total current through GND or V~CC~ listed in the Absolute
Maximum Ratings.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

### Input considerations

Unused inputs must be terminated to either V~CC~ or ground. These can be
directly terminated if the input is completely unused, or they can be
connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a default
state of HIGH, and a pull-down resistor is used for a default state of
LOW. The resistor size is limited by drive current of the controller,
leakage current into the CD74HCT27, as specified in the Electrical
Characteristics, and the desired input transition rate. A 10-kΩ resistor
value is often used due to these factors.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
Similarly, the ground voltage is used to produce the output LOW voltage.
Sinking current into the output will increase the output voltage as
specified by the V~OL~ specification in the Electrical Characteristics.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   CD74HCT27 to the receiving device.
3. Ensure the resistive load at the output is larger than (V~CC~ /
   I~O(max)~) Ω. This will ensure that the maximum output current from
   the Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in megaohms; much larger than the minimum
   calculated above.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-µF and 1-µF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results.

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
sense for the logic function or is more convenient.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-8970301CA | Tube of 25 | -55 to 125 | 5962-8970301CA CD54HCT27F3A
CD54HCT27F3A | Tube of 25 | -55 to 125 | 5962-8970301CA CD54HCT27F3A
CD74HCT27E | Tube of 25 | -55 to 125 | CD74HCT27E
CD74HCT27M | | -55 to 125 | HCT27M | Obsolete
CD74HCT27M96 | Reel of 2500 | -55 to 125 | HCT27M
```

## Conversion notes

- Source pages 1–13 converted; the support page (14) and the package
  option addendum, tape and reel and mechanical pages (15–27) dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, device information table,
  revision history, functional block diagram (one 3-input NOR gate, xA,
  xB and xC to xY), layout example and the typical application timing
  diagram (Figure 9-2), the power-consumption and thermal references of
  9.2.1.1 and step 4 of the design procedure.
- Curves digitized from the two typical-characteristics figures to about
  2 % of full scale (straight lines in the source, the 2-V V~OH~ line
  slightly bent).
- The switching table's Min columns and the empty Typ columns at
  –40 °C to 85 °C and –55 °C to 125 °C of the electrical and switching
  tables hold no values and are left out.
- Figures 7-2 and 7-3 (transition times, propagation delays) are drawn
  as one shared waveform figure.
- Ordering vendor columns are from the package option addendum (p. 15).
- From images: overbars of Y = !{A + B + C} (p.1, p.8).
- Source issues:
  - Unconfirmed: I~O~ condition "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V" (absolute maximum ratings note 4).
  - Source conflict: the TTL-compatible CMOS inputs text points to Δt/Δv in the Recommended Operating Conditions (p.8); that table gives the input transition rise and fall time t~t~ (p.4).
  - Unconfirmed: the typical-characteristics curves are labelled 2-V, 4.5-V and 6-V (p.6); the recommended supply voltage is 4.5 V to 5.5 V.
  - Source conflict: the layout guidelines give "a triple-input AND gate" as the example (p.13); this device is a triple 3-input NOR gate.
