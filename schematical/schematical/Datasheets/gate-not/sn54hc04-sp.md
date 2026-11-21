---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn54hc04-sp",
  "title": "SNx4HC04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn54hc04-sp.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS078H",
    "date": "2021-04",
    "pages": 38
  },
  "devices": ["gate:74_04"],
  "parts": ["5962-8409801VCA", "5962-8409801VDA", "84098012A", "8409801CA", "8409801DA", "JM38510/65701BCA", "M38510/65701B2A", "M38510/65701BCA", "SN54HC04J", "SN74HC04DBRE4", "SN74HC04DBRG4", "SN74HC04DR", "SN74HC04DRG4", "SN74HC04NE4", "SN74HC04NSRG4", "SNJ54HC04FK", "SNJ54HC04J", "SNJ54HC04W"],
  "packages": {"p14": "D, DB, N, NS, PW, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4HC04 hex inverters

## Description

This device contains six independent inverters. Each gate performs the
Boolean function Y = !{A} in positive logic.

Buffered inputs. Wide operating voltage range: 2 V to 6 V. Wide
operating temperature range: –40°C to +85°C. Supports fanout up to 10
LSTTL loads.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
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
NC: Not internally connected
```

## Functional description

This device contains six independent inverters. Each gate performs the
Boolean function Y = !{A} in positive logic.

Conversion note: the functional block diagram shows one inverter from xA to xY.

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges
into light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the Absolute Maximum Ratings must be followed at all
times.

The SN74HC04 can drive a load with a total capacitance less than or
equal to the maximum load listed in the Switching Characteristics - 74
connected to a high-impedance CMOS input while still meeting all of the
datasheet specifications. Larger capacitive loads can be applied,
however it is not recommended to exceed the provided load value. If
larger capacitive loads are required, it is recommended to add a series
resistor between the output and the capacitor to limit output current
to the values given in the Absolute Maximum Ratings.

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor from the input to ground in parallel with the input
capacitance given in the Electrical Characteristics - 74. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics - 74, using ohm's law
(R = V ÷ I).

Signals applied to the inputs need to have fast edge rates, as defined
by the input transition time in the Recommended Operating Conditions to
avoid excessive current consumption and oscillations. If a slow or
noisy input signal is required, a device with a Schmitt-trigger input
should be used to condition the input signal prior to the standard CMOS
input.

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes as depicted in Figure 8-1.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The recommended input and output
voltage ratings may be exceeded if the input and output clamp-current
ratings are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| L | H |
| H | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| T~stg~ | Storage temperature | | -60 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Rating may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Condition. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Guaranteed by design.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5 | 6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | | V |
| ^ | ^ | V~CC~ = 4.5 V | 3.15 | | | V |
| ^ | ^ | V~CC~ = 6 V | 4.2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | | 0.5 | V |
| ^ | ^ | V~CC~ = 4.5 V | | | 1.35 | V |
| ^ | ^ | V~CC~ = 6 V | | | 1.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate [1] | V~CC~ = 2 V | | | 1000 | ns |
| ^ | ^ | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 6 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | SN54HC04 | -55 | | 125 | °C |
| ^ | ^ | SN74HC04 | -40 | | 85 | °C |

[1] Unconfirmed: the source gives this rate in ns, without a per-volt unit.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74HC04 | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB (SSOP) 14 pins | N (PDIP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 133.6 | 114.8 | 60.7 | 122.6 | 151.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 89 | 64.5 | 47.8 | 81.8 | 79.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 89.5 | 65.1 | 40.6 | 83.8 | 94.7 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 45.5 | 23.7 | 26.9 | 45.4 | 25.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 89.1 | 64.4 | 40.3 | 83.4 | 94.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec-74 caption="Electrical characteristics - 74" parts=SN74* conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted)."
| Symbol | Parameter | Test conditions | < | V~CC~ | Operating free-air temperature (T~A~) | < | < | < | < | < | Unit |
| ^ | ^ | ^ | < | ^ | 25°C | < | < | -40°C to 85°C | < | < | ^ |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = –20 µA | 2 V | 1.9 | 1.998 | | 1.9 | | | V |
| ^ | ^ | ^ | ^ | 4.5 V | 4.4 | 4.499 | | 4.4 | | | V |
| ^ | ^ | ^ | ^ | 6 V | 5.9 | 5.999 | | 5.9 | | | V |
| ^ | ^ | ^ | I~OH~ = –4 mA | 4.5 V | 3.98 | 4.3 | | 3.84 | | | V |
| ^ | ^ | ^ | I~OH~ = –5.2 mA | 6 V | 5.48 | 5.8 | | 5.34 | | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 2 V | | 0.002 | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | ^ | 4.5 V | | 0.001 | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | ^ | 6 V | | 0.001 | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | 0.17 | 0.26 | | | 0.33 | V |
| ^ | ^ | ^ | I~OL~ = 5.2 mA | 6 V | | 0.15 | 0.26 | | | 0.33 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ or 0 | < | 6 V | | | ±0.1 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0 | V~I~ = V~CC~ or 0 [1] | 6 V | | | 2 | | | 20 | µA |
| C~i~ | Input capacitance | | < | 2 V to 6 V | | 3 | 10 | | | 10 | pF |

[1] Unconfirmed: the source prints V~I~ = V~CC~ or 0 in both condition cells of this row; the Electrical Characteristics - 54 table prints I~O~ = 0 in the second.
```

```table id=ec-54 caption="Electrical characteristics - 54" parts="SN54*,SNJ54*,5962-*,8409801*,JM38510/*,M38510/*" conditions="Over operating free-air temperature range; typical values measured at TA = 25°C (unless otherwise noted)."
| Symbol | Parameter | Test conditions | < | V~CC~ | Operating free-air temperature (T~A~) | < | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | < | ^ | 25°C | < | < | –40°C to 85°C | < | < | –55°C to 125°C | < | < | ^ |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = –20 µA | 2 V | 1.9 | 1.998 | | 1.9 | | | 1.9 | | | V |
| ^ | ^ | ^ | ^ | 4.5 V | 4.4 | 4.499 | | 4.4 | | | 4.4 | | | V |
| ^ | ^ | ^ | ^ | 6 V | 5.9 | 5.999 | | 5.9 | | | 5.9 | | | V |
| ^ | ^ | ^ | I~OH~ = –4 mA | 4.5 V | 3.98 | 4.3 | | 3.84 | | | 3.7 | | | V |
| ^ | ^ | ^ | I~OH~ = –5.2 mA | 6 V | 5.48 | 5.8 | | 5.34 | | | 5.2 | | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 2 V | | 0.002 | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | ^ | 4.5 V | | 0.001 | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | ^ | 6 V | | 0.001 | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | 0.17 | 0.26 | | | 0.33 | | | 0.4 | V |
| ^ | ^ | ^ | I~OL~ = 5.2 mA | 6 V | | 0.15 | 0.26 | | | 0.33 | | | 0.4 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ or 0 | < | 6 V | | | ±0.1 | | | ±1 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or 0 | I~O~ = 0 | 6 V | | | 2 | | | 20 | | | 40 | µA |
| C~i~ | Input capacitance | | < | 2 V to 6 V | | 3 | 10 | | | 10 | | | 10 | pF |
```

```table id=op caption="Operating characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted)."
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 2 V to 6 V | | 20 | | pF |
```

## Switching characteristics

```table id=sw-74 caption="Switching characteristics - 74" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | From | To | V~CC~ | Operating free-air temperature (T~A~) | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | 25°C | < | < | –40°C to 85°C | < | < | ^ |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A | Y | 2 V | | 45 | 95 | | | 120 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 9 | 19 | | | 24 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 8 | 16 | | | 20 | ns |
| t~t~ | Transition-time | | Y | 2 V | | 38 | 75 | | | 95 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 8 | 15 | | | 19 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 6 | 13 | | | 16 | ns |
```

```table id=sw-54 caption="Switching characteristics - 54" parts="SN54*,SNJ54*,5962-*,8409801*,JM38510/*,M38510/*" conditions="Over operating free-air temperature range; typical values measured at TA = 25°C (unless otherwise noted)."
| Symbol | Parameter | From | To | V~CC~ | Operating free-air temperature (T~A~) | < | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | 25°C | < | < | –40°C to 85°C | < | < | –55°C to 125°C | < | < | ^ |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A | Y | 2 V | | 45 | 95 | | | 120 | | | 125 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 9 | 19 | | | 24 | | | 29 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 8 | 16 | | | 20 | | | 25 | ns |
| t~t~ | Transition-time | | Y | 2 V | | 38 | 75 | | | 95 | | | 110 | ns |
| ^ | ^ | ^ | ^ | 4.5 V | | 8 | 15 | | | 19 | | | 22 | ns |
| ^ | ^ | ^ | ^ | 6 V | | 6 | 13 | | | 16 | | | 19 | ns |
```

## Typical characteristics

```curve id=voh caption="Typical output voltage in the high state (V~OH~), T~A~ = 25°C"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 7]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 2.0], [1.5, 1.7]]},
  {"label": "VCC = 4.5 V", "points": [[0, 4.5], [4, 4.33]]},
  {"label": "VCC = 6 V", "points": [[0, 5.98], [5.5, 5.83]]}
 ]}
```

```curve id=vol caption="Typical output voltage in the low state (V~OL~), T~A~ = 25°C"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.3]},
 "series": [
  {"label": "VCC = 2 V", "points": [[0, 0], [1.5, 0.097]]},
  {"label": "VCC = 4.5 V", "points": [[0, 0], [4, 0.115]]},
  {"label": "VCC = 6 V", "points": [[0, 0], [5.5, 0.127]]}
 ]}
```

Conversion note: Figure 6-3 (p.7), typical propagation delay over temperature at V~CC~ = 5 V, and Figure 6-4 (p.7), typical propagation delay over supply, plot TPLH and TPHL on a TPD axis that has no unit, and are not converted.

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. The outputs are
measured one at a time, with one input transition per measurement.
C~L~= 50 pF and includes probe and jig capacitance. t~t~ is the greater
of t~r~ and t~f~. The maximum between t~PLH~ and t~PHL~ is used for
t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, transition times and propagation delays"
VM: 50%
```

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

This device can be used to add an additional stage to a counter with an
external flip-flop. Because counters use a negative edge trigger, the
flip-flop's clock input must be inverted to provide this function. This
function only requires one of the six available inverters in the
device, so the remaining channels can be used for other applications
needing an inverted signal or improved signal integrity. Unused inputs
must be terminated at V~CC~ or GND. Unused outputs can be left
floating.

Conversion note: Figure 9-1 (p.11), the typical application schematic, shows a Counter with Clear and Input signals whose last output drives an inverter, and the inverter drives the clock input of a D-Type Flip-Flop that shares the Clear signal and has its inverted Q output connected back to D.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics - 74.

The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the SN74HC04 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics - 74.
The logic device can only source or sink as much current as it is
provided at the supply and ground pins, respectively. Be sure not to
exceed the maximum total current through GND or V~CC~ listed in the
Absolute Maximum Ratings.

Total power consumption can be calculated using the information
provided in CMOS Power Consumption and Cpd Calculation.

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

### Input considerations

Unused inputs must be terminated to either V~CC~ or ground. These can
be directly terminated if the input is completely unused, or they can
be connected with a pull-up or pull-down resistor if the input is to be
used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The resistor size is limited by drive current of the
controller, leakage current into the SN74HC04, as specified in the
Electrical Characteristics - 74, and the desired input transition rate.
A 10-kΩ resistor value is often used due to these factors.

The SN74HC04 has standard CMOS inputs, so input signal edge rates
cannot be slow. Slow input edge rates can cause oscillations and
damaging shoot-through current. The recommended rates are defined in
the Recommended Operating Conditions.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics
- 74. Similarly, the ground voltage is used to produce the output LOW
voltage. Sinking current into the output will increase the output
voltage as specified by the V~OL~ specification in the Electrical
Characteristics - 74.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in
   Section 11.
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74HC04 to the receiving device.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. This will ensure that the maximum output
   current from the Absolute Maximum Ratings is not violated. Most CMOS
   inputs have a resistive load measured in megaohms; much larger than
   the minimum calculated above.
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   Cpd Calculation

Conversion note: Figure 9-2 (p.12), the typical application timing diagram, shows the counter's last output, its inverse and the flip-flop output at half that frequency, with the legend Input – 32 kHz and the flip-flop output – 1 kHz.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in Figure 11-1.

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs
must not ever be left floating. In many cases, functions or parts of
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

Conversion note: Figure 11-1 (p.13), the example layout for the SN74HC04, shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, an unused input tied to GND, an unused input tied to V~CC~, an unused output left floating, and a GND flood fill.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-8409801VCA | Tube of 25 | -55 to 125 | 5962-8409801VCA SNV54HC04J
5962-8409801VDA | Tube of 25 | -55 to 125 | 5962-8409801VDA SNV54HC04W
84098012A | Tube of 55 | -55 to 125 | 84098012A SNJ54HC04FK
8409801CA | Tube of 25 | -55 to 125 | 8409801CA SNJ54HC04J
8409801DA | Tube of 25 | -55 to 125 | 8409801DA SNJ54HC04W
JM38510/65701BCA | Tube of 25 | -55 to 125 | JM38510/65701BCA
M38510/65701B2A | Tube of 55 | -55 to 125 | JM38510/65701B2A | NRND
M38510/65701BCA | Tube of 25 | -55 to 125 | JM38510/65701BCA
SN54HC04J | Tube of 25 | -55 to 125 | SN54HC04J
SN74HC04DBRE4 | Reel of 2000 | -40 to 85 | HC04 | Last Time Buy
SN74HC04DBRG4 | Reel of 2000 | -40 to 85 | HC04 | Last Time Buy
SN74HC04DR | Reel of 2500 | -40 to 85 | HC04
SN74HC04DRG4 | Reel of 2500 | -40 to 85 | HC04
SN74HC04NE4 | Tube of 25 | -40 to 85 | SN74HC04N
SN74HC04NSRG4 | Reel of 2000 | -40 to 85 | HC04
SNJ54HC04FK | Tube of 55 | -55 to 125 | 84098012A SNJ54HC04FK
SNJ54HC04J | Tube of 25 | -55 to 125 | 8409801CA SNJ54HC04J
SNJ54HC04W | Tube of 25 | -55 to 125 | 8409801DA SNJ54HC04W
```

## Conversion notes

- Source pages 1–13 converted; the support page (14), the package
  option addendum (p.15–18), tape and reel, tube and mechanical pages
  (19–38) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are from the package option addendum.
- Dropped: features and applications lists, device information table,
  table of contents, revision history, the functional pinout, the
  vendor's pin drawings, the functional block diagram, the application
  schematic, timing diagram and layout example (Figures 9-1, 9-2 and
  11-1, described in text), and the typical propagation delay curves
  (Figures 6-3 and 6-4, p.7).
- The package option addendum also lists JM38510/65701B2A and further
  SN74HC04 orderables that this file does not claim.
- Section 11, kept as printed in the design procedure, is Layout.
- The ESD ratings table prints a dash for the charged-device model
  value; the cell is empty here.
- Figures 7-2 and 7-3 are shown by one shared waveform figure.
- Curves digitized from Figures 6-1 and 6-2 (p.7) to about 2 % of full
  scale (straight lines in the source).
- From images: 2.0, 1.7, 4.5, 4.33, 5.98, 5.83, 0.097, 0.115, 0.127
  (curve points read from Figures 6-1 and 6-2, p.7).
- From images: R~θJC(top)~, R~θJC(bot)~ (thermal information table,
  p.5; the text layer breaks the symbol over two lines).
- From images: overbar of Y = !{A} (p.1, p.9).
- Source issues:
  - Unconfirmed: the I~CC~ row of the Electrical Characteristics - 74
    table prints V~I~ = V~CC~ or 0 twice (ec-74 note 1).
  - Unconfirmed: input transition rise and fall rate given in ns (ROC
    note 1).
