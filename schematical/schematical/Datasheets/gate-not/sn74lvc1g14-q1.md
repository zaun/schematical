---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g14-q1",
  "title": "SN74LVC1G14-Q1 single Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1g14-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES865E",
    "date": "2025-10",
    "pages": 35
  },
  "devices": ["gate:1G14", "gate:1G14_SON"],
  "parts": ["SN74LVC1G14DBVRQ1", "SN74LVC1G14QDRYRQ1"],
  "packages": {"p5": "DBV, DCK", "p6": "DRY"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G14-Q1 single Schmitt-trigger inverter

## Description

This single Schmitt-trigger inverter is designed for 1.65V to 5.5V
V~CC~ operation. [[1::1]]

The SN74LVC1G14-Q1 device contains one inverter and performs the Boolean
function Y = !{A}. The device functions as an independent inverter, but
because of Schmitt action, it may have different input threshold levels
for positive-going (V~T+~) and negative-going (V~T–~) signals. [[9::1]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::4]]

Qualified for automotive applications. Supports 5V V~CC~ operation.
Inputs accept voltages to 5.5V. Maximum t~pd~ of 4.6ns at 3.3V. Low
power consumption, 10µA maximum I~CC~. ±24mA output drive at 3.3V.
I~off~ supports partial-power-down mode operation. Latch-up performance
exceeds 100mA per JESD 78, class II. [[1::2]]

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

## Pin functions

```pins
A: Input
GND: Ground
NC: No internal connection.
VCC: Supply or power pin
Y: Output
```
[[3::table #1]]

## Functional description

The SN74LVC1G14-Q1 device contains one Schmitt Trigger Inverter and
performs the Boolean function Y = !{A}. The device functions as an
independent inverter, but because of Schmitt Trigger action, it will
have different input threshold levels for a positive-going (V~t+~) and
negative-going (V~t-~) signals. [[9::1]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuit disables the output, preventing damaging
current back-flow through the device when it is powered down. [[9::2]]

Conversion note: the functional block diagram shows one Schmitt-trigger inverter, A (pin 2) to Y (pin 4).

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due to
overcurrent. The electrical and thermal limits defined in the Absolute
Maximum Ratings must be followed at all times. [[10::1]]

Unused push-pull CMOS outputs must be left disconnected. [[10::2]]

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I). [[10::3]]

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. For additional information regarding Schmitt-trigger inputs,
please see Understanding Schmitt Triggers. [[10::4]]

### Clamp diode structure

Figure 7-1 shows the inputs and outputs to this device have negative
clamping diodes only. [[11::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[11::2]]

Conversion note: Figure 7-1 draws one clamp diode from the input to GND (-I~IK~) and one from the output to GND (-I~OK~), and none to V~CC~.

### Device functional modes

Table 7-1 shows the functional modes of the SN74LVC1G14-Q1 device. [[12::1]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[12::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.53 [4] | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] Unconfirmed: the source prints –0.53 for the minimum input voltage, where the other voltage rows print –0.5.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per AEC Q100-011 | ±750 | ^ |

[1] AEC Q100-002 indicates that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC1G14-Q1 | < | < | Unit |
| ^ | ^ | DBV (SOT-23), 5 pins | DCK (SC70), 5 pins | DRY (SON), 6 pins | ^ |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371.0 | 264 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | 167 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | 142 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | 26 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | 142 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 1.65 V | 0.79 | | 1.16 | V |
| ^ | ^ | | 2.3 V | 1.11 | | 1.56 | ^ |
| ^ | ^ | | 3 V | 1.5 | | 1.87 | ^ |
| ^ | ^ | | 4.5 V | 2.16 | | 2.74 | ^ |
| ^ | ^ | | 5.5 V | 2.61 | | 3.33 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 1.65 V | 0.39 | | 0.64 | V |
| ^ | ^ | | 2.3 V | 0.58 | | 0.89 | ^ |
| ^ | ^ | | 3 V | 0.84 | | 1.16 | ^ |
| ^ | ^ | | 4.5 V | 1.41 | | 1.79 | ^ |
| ^ | ^ | | 5.5 V | 1.87 | | 2.29 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 1.65 V | 0.37 | | 0.62 | V |
| ^ | ^ | | 2.3 V | 0.48 | | 0.77 | ^ |
| ^ | ^ | | 3 V | 0.56 | | 0.87 | ^ |
| ^ | ^ | | 4.5 V | 0.71 | | 1.04 | ^ |
| ^ | ^ | | 5.5 V | 0.71 | | 1.11 | ^ |
| V~OH~ | | I~OH~ = -100 µA [2] | 1.65 V to 4.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA [2] | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA [2] | 2.3 V | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = -16 mA [2] | 3 V | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = -24 mA [2] | ^ | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = -32 mA [2] | 4.5 V | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 4.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.70 | ^ |
| I~I~ | A input | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
[2] Source error: printed as I~OL~ = –100 µA, –4 mA, –8 mA, –16 mA, –24 mA and –32 mA in the V~OH~ rows; the rows give V~OH~, and the recommended operating conditions give these currents as I~OH~.
```
[[5::table #2]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 20 | 21 | 22 | 25 | pF |
```
[[6::table #1]]

## Switching characteristics

Conversion note: Figures 6-1 and 6-2 are in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 2.8 | 9.9 | 1.6 | 5.5 | 1.5 | 4.6 | 0.9 | 4.4 | ns |
```
[[6::table #2]]

```table id=sw-50pf caption="Switching characteristics, C~L~ = 30 pF or 50 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-2)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.8 | 13 | 2 | 8 | 1.8 | 6.5 | 1.2 | 6 | ns |
```
[[6::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3 V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [3.1, 3.8]},
 "series": [
  {"label": "TPD", "points": [[-55, 3.17], [-40, 3.25], [0, 3.57], [25, 3.7], [40, 3.7], [85, 3.6], [100, 3.63], [125, 3.75]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 6.8], [2.5, 4.2], [3.3, 3.6], [5, 2.8]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[7::2]]

Conversion note: Figures 6-1 and 6-2 draw the same load circuit, switch table, waveforms and notes, and differ only in the C~L~ and R~L~ columns of their test-condition tables.
Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figures also show pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[7::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[7::Figure 6-1]]

```table id=test-conditions-15pf caption="Test conditions (Figure 6-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```
[[6::table #4]]

```table id=test-conditions-50pf caption="Test conditions (Figure 6-2)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #5]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[7::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[13::1]]

The device is a high-drive CMOS device that can be used for a multitude
of buffer type functions where the input is slow or noisy. The device
can produce 24mA of drive current at 3.3V, making it Ideal for driving
multiple outputs and good for high-speed applications up to 100MHz. The
inputs are 5.5V tolerant allowing it to translate down to V~CC~. [[13::2]]

Conversion note: the typical application schematic (Figure 8-1) shows a crystal oscillator built around an inverter, with a feedback resistor R~F~, a series resistor R~S~, a crystal with load capacitance C~L~ and capacitors C~1~ and C~2~ to ground, whose output drives the SN74LVC1G14-Q1 into a capacitor C to ground.

### Power considerations

Ensure that the desired supply voltage is within the range specified in
the Electrical Characteristics. The supply voltage sets the device
electrical characteristics of the device, as described in the Electrical
Characteristics section. [[14::1]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74LVC1G14-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Ensure the maximum total current through V~CC~
listed in the Absolute Maximum Ratings is not exceeded. [[14::2]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LVC1G14-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Ensure the
maximum total current through GND listed in the Absolute Maximum Ratings
is not exceeded. [[14::3]]

The SN74LVC1G14-Q1 can drive a load with a total capacitance less than
or equal to 50pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50pF. [[14::4]]

The SN74LVC1G14-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[14::5]]

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note. [[14::6]]

Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[14::7]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[14::8]]

Source conflict: § 8.2.1.1 (p.14) says the maximum junction temperature is listed in the Absolute Maximum Ratings; that table (p.4) has no junction temperature row.

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[14::9]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LVC1G14-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. [[14::10]]

The SN74LVC1G14-Q1 has no input signal transition rate requirements
because it has Schmitt-Trigger inputs. [[14::11]]

Another benefit to having Schmitt-Trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit. [[14::12]]

Unlike what happens with standard CMOS inputs, Schmitt-Trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[14::13]]

Source conflict: § 8.2.1.2 (p.14) says this current is plotted in the Typical Characteristics; that section (p.6) plots only the propagation delay.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[15::2]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device. [[15::3]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[15::4]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[15::5]]

Source conflict: § 8.2.1.3 (p.15) speaks of two channels within the same device; the function table (p.12) and the pin table (p.3) show a single inverter. Kept as printed.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[16::1]]
2. Verify that the capacitive load at the output is ≤ 50pF. This is not
   a hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74LVC1G14-Q1 to one or more of the receiving devices. [[16::1]]
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this prevents the maximum output current
   from the Absolute Maximum Ratings from being violated. Most CMOS
   inputs have a resistive load measured in MΩ; much larger than the
   minimum calculated previously. [[16::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the CMOS Power Consumption and Cpd Calculation
   application note. [[16::1]]

### Application curve

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "mA", "range": [0, 10]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [40, 1.4], [75, 2.6]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [40, 2.1], [75, 3.9]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [40, 2.9], [75, 5.3]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 2.5], [40, 4.9], [60, 7.1], [70, 8.8], [75, 9.1]]}
 ]}
```
[[16::Figure 8-2]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. [[17::1]]

A 0.1μF capacitor is recommended for this device. It is acceptable to
parallel multiple bypass capacitors to reject different frequencies of
noise. The 0.1μF and 1μF capacitors are commonly used in parallel. The
bypass capacitor should be installed as close to the power terminal as
possible for best results. [[17::2]]

### Layout guidelines

- Bypass capacitor placement [[18::3]]
  - Place near the positive supply terminal of the device [[17::3]]
  - Provide an electrically short ground return path [[17::3]]
  - Use wide traces to minimize impedance [[17::3]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[17::3]]
- Signal trace geometry [[17::3]]
  - 8mil to 12mil trace width [[17::3]]
  - Lengths less than 12cm to minimize transmission line effects [[17::3]]
  - Avoid 90° corners for signal traces [[17::3]]
  - Use an unbroken ground plane below signal traces [[17::3]]
  - Flood fill areas around signal traces with ground [[17::3]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[17::3]]
  - For traces longer than 12cm [[17::3]]
    - Use impedance controlled traces [[17::3]]
    - Source-terminate using a series damping resistor near the output [[17::3]]
    - Avoid branches; buffer each signal that must branch separately [[17::3]]

Conversion note: the layout examples show trace corners rated worst, better and best (Figure 8-3), a bypass capacitor placed beside the V~CC~ pin of a TSSOP, a WQFN and a SOT or SC70 package (Figures 8-4 to 8-6), and a series damping resistor at the transmitting port of a long controlled-impedance trace (Figure 8-7).

## Package

```package footprint=SOT23-5
```

```package footprint=SON-6-DRY
```

## Ordering information

```ordering
SN74LVC1G14DBVRQ1 | Reel of 3000 | -40 to 125 | 3L4U, 3RXF
SN74LVC1G14QDRYRQ1 | Reel of 5000 | -40 to 125 | FE
```
[[21::table #1]]

## Conversion notes

- Source pages 1 and 3–18 converted. The ordering lines are from the
  package option addendum (pp.21–22). The table of contents (p.2), the
  support and revision history pages (19–20), tape and reel (23–24),
  mechanical pages (25–34) and the notice page (35) are left out; the
  generated Package section replaces them.
- Dropped: applications list, the AEC-Q100 result lines of the features
  list (the other features are sentences in Description), package
  information table, the vendor pinout drawings, the two "Refer to the
  Feature Description" sentences of § 8.2.1; the simplified schematic
  (p.1), the functional block diagram (p.9), the clamp-diode figure
  (Figure 7-1; the shared clamp-diodes figure also draws clamps to
  V~CC~, which this part does not have), the typical application
  schematic (Figure 8-1) and the layout examples (Figures 8-3 to 8-7)
  are described in text.
- The source's electrical characteristics table continues on p.6 (C~i~
  and the note on typical values); it is one table here. It prints the
  names of V~T+~, V~T–~ and ΔV~T~ under their symbols and gives symbols
  only for the other rows, so those Parameter cells are empty except for
  I~I~.
- Figures 6-1 and 6-2 (pp.7–8) are drawn once: the load circuit with the
  shared R~L~ ∥ C~L~ figure (the second R~L~ and S1 are described in
  text), the propagation-delay waveforms with the shared figure, and the
  switch table and notes once. Their notes B, E and F are left out (this
  inverter has no output enable).
- Curves digitized from Figures 5-1, 5-2 and 8-2 to about 2 % of full
  scale.
- The source also covers the DCK package (SN74LVC1G14QDCKRQ1), which the
  catalog doesn't carry.
- From images: overbar of Y = !{A} (p.1 and p.9); 1 MΩ, 1 kΩ, 500 Ω,
  50 Ω and V~Δ~ (pp.7–8; the text layer prints "1 MW", "1 kW", "500 W",
  "50 W" and "VD"); R~θJC(top)~ (p.5; the text layer breaks the symbol
  over two lines); the curve points 3.17, 3.25, 3.57, 3.63, 3.75
  (Figure 5-1, p.6), 6.8, 4.2, 3.6 (Figure 5-2, p.6), 1.4, 2.1, 3.9,
  2.9, 5.3, 4.9, 7.1, 8.8, 9.1 (Figure 8-2, p.16).
- Source issues:
  - Unconfirmed: minimum input voltage printed –0.53 (Absolute maximum
    ratings note).
  - Source error: V~OH~ rows printed with I~OL~ (Electrical
    characteristics note).
  - Source conflict: maximum junction temperature said to be in the
    Absolute Maximum Ratings, which have no such row (Power
    considerations note).
  - Source conflict: input current plot said to be in the Typical
    Characteristics, which have none (Input considerations note).
  - Source conflict: two channels named for a single inverter (Output
    considerations note).
