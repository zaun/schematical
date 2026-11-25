---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv6t06-ep",
  "title": "SN74LV6T06-EP enhanced product, hex inverter buffers/drivers with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv6t06-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS984",
    "date": "2024-01",
    "pages": 24
  },
  "devices": ["gate:74_6T06"],
  "parts": ["SN74LV6T06PWREP"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74LV6T06-EP enhanced product, hex inverter buffers/drivers with open-drain outputs

## Description

The SN74LV6T06-EP device contains six independent inverters with
open-drain outputs and extended voltage operation to allow for level
translation. Each inverter performs the Boolean function Y = !{A} in
positive logic. The output level is referenced to the supply voltage
(V~CC~) and supports 1.2V, 1.8V, 2.5V, 3.3V, and 5V CMOS levels. [[1::2]]

The input is designed with a lower threshold circuit to support up
translation for lower voltage CMOS inputs (for example, 1.2V input to
1.8V output or 1.8V input to 3.3V output). In addition, the 5V tolerant
input pins enable down translation (for example, 3.3V to 2.5V output). [[1::5]]

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
[[3::Table 4-1]]

## Functional description

The SN74LV6T06-EP device contains six independent inverters with
open-drain outputs. Each inverter performs the Boolean function
Y = !{A} in positive logic. [[1::2]]

Conversion note: the functional block diagram shows one channel, from
xA through an inverter to the open-drain output xY.

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are
high impedance and are typically modeled as a resistor in parallel with
the input capacitance given in the Electrical Characteristics. The
worst case resistance is calculated with the maximum input voltage,
given in the Absolute Maximum Ratings, and the maximum input leakage
current, given in the Electrical Characteristics, using Ohm's law
(R = V ÷ I). [[9::2]]

Standard CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet
this specification will result in excessive power consumption and could
cause oscillations. More details can be found in Implications of Slow
or Floating CMOS Inputs. [[9::3]]

Do not leave standard CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a
system will not be actively driving an input at all times, then a
pull-up or pull-down resistor can be added to provide a valid input
voltage during these times. The resistor value will depend on multiple
factors; a 10-kΩ resistor, however, is recommended and will typically
meet all requirements. [[9::4]]

### Open-drain CMOS outputs

This device includes open-drain CMOS outputs. Open-drain outputs can
only drive the output low. When in the high logical state, open-drain
outputs will be in a high-impedance state. The drive capability of this
device may create fast edges into light loads, so routing and load
conditions should be considered to prevent ringing. Additionally, the
outputs of this device are capable of driving larger currents than the
device can sustain without being damaged. It is important for the
output power of the device to be limited to avoid damage due to
overcurrent. The electrical and thermal limits defined in the Absolute
Maximum Ratings must be followed at all times. [[9::5]]

When placed into the high-impedance state, the output will neither
source nor sink current, with the exception of minor leakage current as
defined in the Electrical Characteristics table. In the high-impedance
state, the output voltage is not controlled by the device and is
dependent on external factors. If no other drivers are connected to the
node, then this is known as a floating node and the voltage is unknown.
A pull-up resistor can be connected to the output to provide a known
voltage at the output while it is in the high-impedance state. The
value of the resistor will depend on multiple factors, including
parasitic capacitance and power consumption limitations. Typically, a
10kΩ resistor can be used to meet these requirements. [[9::4]] [[9::6]]

Unused open-drain CMOS outputs should be left disconnected. [[9::7]]

### LVxT enhanced input voltage

The SN74LV6T06-EP belongs to TI's LVxT family of logic devices with
integrated voltage level translation. This family of devices was
designed with reduced input voltage thresholds to support
up-translation, and inputs tolerant of signals with up to 5.5V levels
to support down-translation. For proper functionality, input signals
must remain at or above the specified V~IH(MIN)~ level for a HIGH input
state, and at or below the specified V~IL(MAX)~ for a LOW input state.
Figure 7-1 shows the typical V~IH~ and V~IL~ levels for the LVxT family
of devices, as well as the voltage levels for standard CMOS devices for
comparison. Conversion note: Figure 7-1 is not drawn here. [[9::8]]

The inputs are high impedance and are typically modeled as a resistor
in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I). [[9::2]]

Input signals must transition between valid logic states quickly, as
defined by the input transition rate in the Recommended Operating
Conditions table. Failing to meet this specification will result in
excessive power consumption and could cause oscillations. More details
can be found in the Implications of Slow or Floating CMOS Inputs
application report. [[10::2]]

Do not leave inputs floating at any time during operation. Unused
inputs must be terminated at a valid high or low voltage level. If a
system will not be actively driving an input at all times, then a
pull-up or pull-down resistor can be added to provide a valid input
voltage during these times. The resistor value will depend on multiple
factors; however, a 10-kΩ resistor is recommended and will typically
meet all requirements. [[10::3]]

#### Down translation

Signals can be translated down using the SN74LV6T06-EP. The voltage
applied at the V~CC~ will determine the output voltage and the input
thresholds as described in the Recommended Operating Conditions and
Electrical Characteristics tables. [[10::5]]

When connected to a high-impedance input, the output voltage will be
approximately V~CC~ in the HIGH state, and 0V in the LOW state. As
shown in Figure 7-1, ensure that the input signals in the HIGH state
are between V~IH(MIN)~ and 5.5V, and input signals in the LOW state are
lower than V~IL(MAX)~. [[10::6]]

As shown in Figure 7-2 for example, the standard CMOS inputs for
devices operating at 5.0V, 3.3V or 2.5V can be down-translated to match
1.8V CMOS signals when operating from 1.8V V~CC~. [[10::7]]

Down Translation Combinations are as follows: [[10::8]]

- 1.8V V~CC~ – Inputs from 2.5V, 3.3V, and 5.0V [[10::7]]
- 2.5V V~CC~ – Inputs from 3.3V and 5.0V [[11::4]]
- 3.3V V~CC~ – Inputs from 5.0V [[11::4]]

#### Up translation

Input signals can be up translated using the SN74LV6T06-EP. The voltage
applied at V~CC~ will determine the output voltage and the input
thresholds as described in the Recommended Operating Conditions and
Electrical Characteristics tables. When connected to a high-impedance
input, the output voltage will be approximately V~CC~ in the HIGH
state, and 0V in the LOW state. [[10::5]] [[10::6]]

The inputs have reduced thresholds that allow for input HIGH state
levels which are much lower than standard values. For example, standard
CMOS inputs for a device operating at a 5V supply will have a
V~IH(MIN)~ of 3.5V. For the SN74LV6T06-EP, V~IH(MIN)~ with a 5V supply
is only 2V, which would allow for up-translation from a typical 2.5V to
5V signals. [[11::2]]

As shown in Figure 7-2, ensure that the input signals in the HIGH state
are above V~IH(MIN)~ and input signals in the LOW state are lower than
V~IL(MAX)~. [[11::3]]

Up Translation Combinations are as follows: [[10::8]]

- 1.8V V~CC~ – Inputs from 1.2V [[11::4]]
- 2.5V V~CC~ – Inputs from 1.8V [[11::4]]
- 3.3V V~CC~ – Inputs from 1.8V and 2.5V [[1::5]]
- 5.0V V~CC~ – Inputs from 2.5V and 3.3V [[11::4]]

Conversion note: Figure 7-2 shows a 5.0 V or 3.3 V system driving
LVxTxx logic at V~CC~ = 5.0 V into a 5.0 V system, and a 5.0 V to
1.2 V system driving LVxTxx logic at V~CC~ = 1.8 V into a 1.8 V system.

### Clamp diode structure

As Figure 7-3 shows, the outputs to this device have both positive and
negative clamping diodes, and the inputs to this device have negative
clamping diodes only. Conversion note: Figure 7-3 draws the output
diodes to V~CC~ and to GND, and the input diode to GND. [[11::6]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[11::7]]

## Function tables

Table 7-1 lists the functional modes of the SN74LV6T06-EP. [[12::1]]

```table id=function caption="Function table"
| Input A [1] | Output Y [2] |
|:-:|:-:|
| H | L |
| L | Z |

[1] H = high voltage level, L = low voltage level, X = do not care
[2] H = driving high, L = driving low, Z = high impedance
```
[[3::Table 4-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < -0.5V | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < -0.5V or V~O~ > V~CC~ +0.5V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous output current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage [1] | | 1.6 | 5.5 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 2V | 1.1 | | V |
| ^ | ^ | V~CC~ = 2.25 V to 2.75 V | 1.28 | | V |
| ^ | ^ | V~CC~ = 3V to 3.6V | 1.45 | | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 2 | | V |
| V~IL~ | Low-Level input voltage | V~CC~ = 1.65 V to 2V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.25 V to 2.75 V | | 0.65 | V |
| ^ | ^ | V~CC~ = 3V to 3.6V | | 0.75 | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.85 | V |
| I~O~ | Output current | V~CC~ = 1.6V to 2V | | ±3 | mA |
| ^ | ^ | V~CC~ = 2.25 V to 2.75 V | | ±7 | mA |
| ^ | ^ | V~CC~ = 3.3V to 5.0V | | ±15 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.6V to 5.0V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] Source conflict: this table gives 1.6 V minimum; the features list gives a wide operating range of 1.8V to 5.5V.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LV6T06-EP | Unit |
| ^ | ^ | PW (TSSOP), 14 pins | ^ |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 147.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 77.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 90.9 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 27.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 90.2 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | -55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | I~OL~ = 50µA | 1.65 V to 5.5V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 2mA | 1.65 V to 2V | | 0.1 [1] | 0.2 | | | 0.25 | V |
| ^ | I~OL~ = 3mA | 2.25 V to 2.75 V | | 0.1 [1] | 0.15 | | | 0.2 | V |
| ^ | I~OL~ = 5.5mA | 3V to 3.6V | | 0.2 [1] | 0.2 | | | 0.25 | V |
| ^ | I~OL~ = 8mA | 4.5V to 5.5V | | 0.3 [1] | 0.3 | | | 0.35 | V |
| I~I~ | V~I~ = 0V or V~CC~ | 0V to 5.5V | | | ±0.1 | | | ±1 | µA |
| I~CC~ | V~I~ = 0V or V~CC~, I~O~ = 0; open on loading | 1.65 V to 5.5V | | | 2 | | | 20 | µA |
| ΔI~CC~ | One input at 0.3V or 3.4V, other inputs at 0 or V~CC~, I~O~ = 0 | 5.5V | | | 1.35 | | | 1.5 | mA |
| ^ | One input at 0.3V or 1.1V, other inputs at 0 or V~CC~, I~O~ = 0 | 1.8V | | | 10 | | | 20 | µA |
| C~I~ | V~I~ = V~CC~ or GND | 5V | | 4 | 10 | | | 10 | pF |
| C~O~ | V~O~ = V~CC~ or GND | 5V | | 3 | | | | | pF |
| C~PD~ | No load, F = 1MHz | 5V | | 14 | | | | | pF |

[1] Typical value at nearest nominal voltage (1.8V, 2.5V, 3.3V, and 5V)
```
[[5::table #2]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5V, C~L~ = 50pF, T~A~ = 25°C"
| Symbol | Description | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 1 | 1.2 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | -0.8 | -0.3 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | 5 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.1 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.5 | V |
```
[[6::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical ratings measured at T~A~ = 25°C (unless otherwise noted)."
| Symbol | From (input) | To (output) | Load capacitance | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~PHL~ | A | Y | C~L~ = 15pF | 1.8V | | 15.6 | 40.1 | ns |
| t~PLH~ | A | Y | C~L~ = 15pF | 1.8V | | 11.8 | 40.1 | ns |
| t~PHL~ | A | Y | C~L~ = 50pF | 1.8V | | 21.0 | 46.7 | ns |
| t~PLH~ | A | Y | C~L~ = 50pF | 1.8V | | 16.1 | 46.7 | ns |
| t~PHL~ | A | Y | C~L~ = 15pF | 2.5V | | 10.6 | 24.0 | ns |
| t~PLH~ | A | Y | C~L~ = 15pF | 2.5V | | 7.1 | 24.0 | ns |
| t~PHL~ | A | Y | C~L~ = 50pF | 2.5V | | 13.5 | 25.4 | ns |
| t~PLH~ | A | Y | C~L~ = 50pF | 2.5V | | 10.1 | 25.4 | ns |
| t~PHL~ | A | Y | C~L~ = 15pF | 3.3V | | 7.9 | 15.2 | ns |
| t~PLH~ | A | Y | C~L~ = 15pF | 3.3V | | 5.4 | 13.8 | ns |
| t~PHL~ | A | Y | C~L~ = 50pF | 3.3V | | 10.2 | 18.3 | ns |
| t~PLH~ | A | Y | C~L~ = 50pF | 3.3V | | 7.8 | 16.0 | ns |
| t~PHL~ | A | Y | C~L~ = 15pF | 5V | | 5.3 | 10.2 | ns |
| t~PLH~ | A | Y | C~L~ = 15pF | 5V | | 4.2 | 9.9 | ns |
| t~PHL~ | A | Y | C~L~ = 50pF | 5V | | 7.1 | 12.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50pF | 5V | | 5.8 | 11.5 | ns |
```
[[5::table #3]] [[6::table #2]]

Source conflict: the switching characteristics name the delays t~PHL~
and t~PLH~; the parameter measurement information names them t~PLZ~ and
t~PZL~.

## Typical characteristics

Conversion note: the source plots, at T~A~ = 25°C unless otherwise
noted, supply current across input voltage (Figures 5-1 and 5-2),
supply current across supply voltage (Figure 5-3), and output voltage
vs current in LOW state (Figures 5-4 to 5-8); the curves are not drawn
here.

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ < 2.5 ns. [[8::1]]

The outputs are measured individually with one input transition per
measurement. [[8::2]]

```table id=pmi-tests caption="Test conditions"
| Test | S1 | R~L~ | C~L~ | ΔV | V~CC~ |
|---|---|---|---|---|---|
| t~PLZ~, t~PZL~ | CLOSED | 1kΩ | 15pF, 50pF | 0.15V | ≤ 2.5V |
| t~PLZ~, t~PZL~ | CLOSED | 1kΩ | 15pF, 50pF | 0.3V | > 2.5V |
```
[[8::table #1]]

C~L~ includes probe and test-fixture capacitance. t~PLZ~ is the same as
t~dis~. t~PZL~ is the same as t~en~. [[8::3]]

```figure ref=load-ttl caption="Load circuit for open-drain outputs"
RL: 1kΩ
CL: 15pF, 50pF
```
[[8::Figure 6-1]]

Conversion note: in the source's load circuit (Figure 6-1), R~L~ goes
to V~CC~ through a switch, S~1~, and the output has a test point.

Conversion note: Figure 6-2, voltage waveforms propagation delays,
measures t~PLZ~ from 50% of the input to 10% V~CC~ of the rising output
and t~PZL~ from 50% of the input to 50% of the falling output, and on
the second output waveform t~PZL~ to 50% of the falling output and
t~PLZ~ to 10% of the rising output; the input swings from 0V to V~CC~.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[13::1]]

The SN74LV6T06-EP can be used to drive LEDs with control from very low
voltage sources. The supply voltage of the SN74LV6T06-EP can be set as
low as 1.8V (±0.15 V), which allows for controlling an LED powered by
up to 5V with an input voltage as low as 1.2V. As an example, with a
1.8V SN74LV6T06-EP supply voltage, the output of the SN74LV6T06-EP is
expected to have an equivalent low-state resistance of 30 Ω
(R~OL~ = V~OL~ / I~OL~ = 0.15 V / 5mA, from the Typical
Characteristics). Therefore the calculation of the current limiting
resistor is: [[13::2]]

```equation
R~1~ = (V~PU~ − V~F~) / I~D~ − 30
```

with V~PU~ shown in Equation 1 as the diode pull-up voltage (separate
from the device supply voltage, V~CC~), V~F~ as the diode forward
voltage, and I~D~ as the desired diode current. [[13::3]]

Conversion note: the typical application block diagram (Figure 8-1)
shows Input driving one open-drain inverter, whose output sinks the
cathode of an LED fed from V~PU~ through R~1~.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the
electrical characteristics of the device as described in the Electrical
Characteristics section. [[13::5]]

The positive voltage supply must be capable of sourcing current equal
to the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[13::6]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74LV6T06-EP plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection. Be
sure to not exceed the maximum total current through GND listed in the
Absolute Maximum Ratings. [[13::7]]

The SN74LV6T06-EP can drive a load with a total capacitance less than
or equal to 50pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50pF. [[13::8]]

The SN74LV6T06-EP can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the
HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[13::9]] [[14::1]]

Total power consumption can be calculated using the information
provided in the CMOS Power Consumption and Cpd Calculation application
note. [[14::2]]

Thermal increase can be calculated using the information provided in
the Thermal Characteristics of Standard Linear and Logic (SLL) Packages
and Devices application note. [[14::3]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.
Source conflict: this caution refers to a maximum junction temperature
listed in the Absolute Maximum Ratings; that table has no T~J~ row. [[14::4]]

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Source error: the source leaves out the two
levels the input signals must cross. Do not exceed the maximum input
voltage range found in the Absolute Maximum Ratings. [[14::5]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LV6T06-EP (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[14::6]]

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics. [[14::8]]

Open-drain outputs can be connected together directly to produce a
wired-AND configuration or for additional output drive strength. [[14::9]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[14::10]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[14::12]]
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74LV6T06-EP to one or more of the receiving devices. [[14::12]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[14::12]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[14::12]]

Conversion note: the application timing diagram (Figure 8-2) shows the
output inverting the input, with the LED status ON while the input is
high and OFF while it is low.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1μF capacitor is recommended for this device. It
is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1μF and 1μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results. [[15::1]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used). Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[15::2]]

Conversion note: the layout example (Figure 8-3) recommends a GND flood
fill, places the bypass capacitor close to the device, ties an unused
input to V~CC~, leaves an unused output floating, and avoids 90°
corners for signal lines.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LV6T06PWREP | Reel of 3000 | -55 to 125 | LV606EP
```
[[17::table #1]]

## Conversion notes

- Source pages 1–15 converted; the device and documentation support
  page and revision history (16), the package option addendum
  (pp.17–18), tape and reel and mechanical pages (19–24) dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, the package information
  table, the simplified logic diagram (p.1), the table of contents
  (p.2), the typical characteristics curves (Figures 5-1 to 5-8,
  pp.6–7), Figure 7-1 (LVxT input voltage levels chart, p.10) and the
  vendor pinout drawing. The functional block diagram, the translation
  example (Figure 7-2), the clamp diode figure (Figure 7-3), the
  propagation delay waveforms (Figure 6-2), the typical application
  block diagram (Figure 8-1), the application timing diagram
  (Figure 8-2) and the layout example (Figure 8-3) are described in one
  sentence each.
- The switching characteristics table, which the source splits over
  pages 5 and 6, is one table here.
- The package option addendum also lists V62/24620-01XE, which the
  catalog does not carry.
- From images: overbar of Y = !{A} (pp.1, 9); the equation
  R~1~ = (V~PU~ − V~F~) / I~D~ − 30 (p.13); the clamp diode connections
  (Figure 7-3, p.11); the measurement points of Figure 6-2 (p.8); the
  typical application block diagram (Figure 8-1, p.13); the application
  timing diagram (Figure 8-2, p.15).
- Source issues:
  - Source error: "Input signals must cross to be considered a logic
    LOW, and to be considered a logic HIGH" (p.14) leaves out the two
    levels; kept as printed.
  - Source conflict: V~CC~ minimum 1.6 V in the recommended operating
    conditions (p.4) against "Wide operating range of 1.8V to 5.5V"
    (p.1).
  - Source conflict: switching characteristics for t~PHL~ and t~PLH~
    (pp.5–6) against t~PLZ~ and t~PZL~ in the parameter measurement
    information (p.8).
  - Source conflict: the caution on the maximum junction temperature
    "listed in the Absolute Maximum Ratings" (p.14) against that
    table, which has no T~J~ row (p.4).
