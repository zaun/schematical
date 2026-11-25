---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g06-q1",
  "title": "SN74LVC1G06-Q1 automotive single inverter buffer or driver with open-drain output",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1g06-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SLVSJ13",
    "date": "2025-06",
    "pages": 29
  },
  "devices": ["gate:1G06"],
  "parts": ["SN74LVC1G06DBVRQ1", "SN74LVC1G06DCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G06-Q1 automotive single inverter buffer or driver with open-drain output

## Description

Operating range from 1.65V to 5.5V. 5.5V tolerant input pins. Latch-up
performance exceeds 100mA per JESD 78. [[1::2]]

The SN74LVC1G06-Q1 device is an Inverter Buffer with an Open-Drain
output. [[1::3]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No Connect. Leave floating or connect to ground.
A: Input A
GND: Ground
Y: Output Y
VCC: Positive Supply
```
[[3::Table 4-1]]

I = input, O = output, I/O = input or output, G = ground, P = power. [[3::2]]

## Functional description

The SN74LVC1G06-Q1 device contains one open-drain inverter with a
maximum sink current of 32mA. This device is fully specified for
partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs when the device is powered down. This inhibits
current backflow into the device which prevents damage to the device. [[8::1]]

Conversion note: the logic diagram (Figure 7-1, positive logic) shows one inverter with an open-drain output, A (pin 2) to Y (pin 4).

### Open-drain CMOS outputs

This device includes open-drain CMOS outputs. Open-drain outputs can
only drive the output low. When in the high logical state, open-drain
outputs will be in a high-impedance state. The drive capability of this
device may create fast edges into light loads, so routing and load
conditions should be considered to prevent ringing. Additionally, the
outputs of this device are capable of driving larger currents than the
device can sustain without being damaged. It is important for the output
power of the device to be limited to avoid damage due to overcurrent.
The electrical and thermal limits defined in the Absolute Maximum
Ratings must be followed at all times. [[9::1]]

When placed into the high-impedance state, the output will neither
source nor sink current, with the exception of minor leakage current as
defined in the Electrical Characteristics table. In the high-impedance
state, the output voltage is not controlled by the device and is
dependent on external factors. If no other drivers are connected to the
node, then this is known as a floating node and the voltage is unknown.
A pull-up resistor can be connected to the output to provide a known
voltage at the output while it is in the high-impedance state. The value
of the resistor will depend on multiple factors, including parasitic
capacitance and power consumption limitations. Typically, a 10kΩ
resistor can be used to meet these requirements. [[9::2]]

Unused open-drain CMOS outputs should be left disconnected. [[9::3]]

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I). [[9::4]]

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in Implications of Slow or
Floating CMOS Inputs. [[9::5]]

Do not leave standard CMOS inputs floating at any time during operation.
Unused inputs must be terminated at V~CC~ or GND. If a system will not
be actively driving an input at all times, then a pull-up or pull-down
resistor can be added to provide a valid input voltage during these
times. The resistor value will depend on multiple factors; a 10kΩ
resistor, however, is recommended and will typically meet all
requirements. [[9::6]]

### Clamp diode structure

Figure 7-2 shows the inputs and outputs to this device have negative
clamping diodes only. [[10::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[10::2]]

```figure ref=clamp-diodes caption="Figure 7-2. Electrical placement of clamping diodes for each input and output"
```
[[10::Figure 7-2]]

### Device functional modes

Function Table lists the functional modes of the SN74LVC1G06-Q1. [[10::4]]

## Function tables

```table id=function caption="Table 7-1. Function table"
| Input A | Output Y |
|:-:|:-:|
| L | Hi-Z |
| H | L |
```
[[10::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0V | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0V | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | ^ |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Functional | 1.2 | | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | (High or low state) | 0 | V~CC~ | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | ^ |
| ^ | ^ | V~CC~ = 3.0V to 3.6V | 2 | | ^ |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 0.7 × V~CC~ | | ^ |
| V~IL~ | Low-Level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3.0V to 3.6V | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.3 × V~CC~ | ^ |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3.0V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3.0V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.1V to 5.5V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Package | Pins | Thermal metric [1] | < | < | < | < | < | Unit |
| ^ | ^ | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | ^ |
|---|--:|--:|--:|--:|--:|--:|--:|---|
| DBV (SOT-23, 5) | 5 | 357.1 | 263.7 | 264.4 | 195.6 | 262.2 | | °C/W |
| DCK (SOT-SC70, 5) | 5 | 371.0 | 297.5 | 258.6 | 195.6 | 256.2 | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|---|
| V~OL~ | I~OL~ = 100 μA | Over Recommended Operating Conditions | | | 0.2 | V |
| V~OL~ | I~OL~ = 4 mA | 1.65V | | | 0.45 | V |
| V~OL~ | I~OL~ = 8 mA | 2.3V | | | 0.3 | V |
| V~OL~ | I~OL~ = 12 mA | 2.7V | | | 0.4 | V |
| V~OL~ | I~OL~ = 16 mA | 3V | | | 0.4 | V |
| V~OL~ | I~OL~ = 24 mA | 3V | | | 0.55 | V |
| V~OL~ | I~OL~ = 32 mA | 4.5V | | | 0.55 | V |
| I~I~ | V~I~ = V~CC~ or GND | V~CC~ = 0V to 5.5V | | ±1 | ±5 | µA |
| I~off~ | V~I~ or V~O~ = V~CC~ | V~CC~ = 0V | | ±1 | ±10 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | V~CC~ = 1.65V to 5.5V | | 1 | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 3.0V to 5.5V | | | 500 | µA |
| C~I~ | V~I~ = V~CC~ or GND | 3.3V | | 3.5 | | pF |
| C~O~ | V~O~ = V~CC~ or GND | 3.3V | | 6.3 | | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information"
| Parameter | From (input) | To (output) | Load capacitance | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 15pF | 1.8V ± 0.15V | 2.1 | 5 | 6 | ns |
| t~pd~ | A | Y | C~L~ = 15pF | 2.5V ± 0.2V | 1.1 | 3 | 3.5 | ns |
| t~pd~ | A | Y | C~L~ = 15pF | 3.3V ± 0.3V | 1.2 | 3 | 3.5 | ns |
| t~pd~ | A | Y | C~L~ = 15pF | 5.0V ± 0.5V | 1 | 2 | 2.5 | ns |
| t~pd~ | A | Y | C~L~ = 50pF | 1.8V ± 0.15V | 2.2 | 6 | 7 | ns |
| t~pd~ | A | Y | C~L~ = 50pF | 2.5V ± 0.2V | 1.1 | 4 | 4.5 | ns |
| t~pd~ | A | Y | C~L~ = 50pF | 3.3V ± 0.3V | 1.2 | 4 | 4.5 | ns |
| t~pd~ | A | Y | C~L~ = 50pF | 5.0V ± 0.5V | 1 | 3 | 3.5 | ns |
| C~pd~ | | | f = 10MHz | 1.8V | | 2 | | pF |
| C~pd~ | | | f = 10MHz | 2.5V | | 2 | | pF |
| C~pd~ | | | f = 10MHz | 3.3V | | 2 | | pF |
| C~pd~ | | | f = 10MHz | 5V | | 3 | | pF |
```
[[5::table #4]] [[6::table #1]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[5::5]]

```curve id=icc-vin-low caption="Figure 5-1. Supply current across input voltage 1.8V and 2.5V supply"
{"x": {"label": "V_IN - Input Voltage", "unit": "V", "range": [0, 2.5]},
 "y": {"label": "I_CC - Supply Current", "unit": "µA", "range": [0, 60]},
 "series": [
  {"label": "1.8 V", "points": [[0.45, 0], [0.6, 3], [0.75, 13], [0.9, 8], [1, 5.5], [1.15, 2], [1.3, 0]]},
  {"label": "2.5 V", "points": [[0.45, 0], [0.6, 3], [0.75, 12], [0.9, 36], [0.94, 60], [0.96, 41], [1.25, 24], [1.5, 12], [1.75, 4], [2, 0]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=icc-vin-high caption="Figure 5-2. Supply current across input voltage 3.3V and 5.0V supply"
{"x": {"label": "V_IN - Input Voltage", "unit": "V", "range": [0, 5]},
 "y": {"label": "I_CC - Supply Current", "unit": "µA", "range": [0, 800]},
 "series": [
  {"label": "3.3 V", "points": [[0.6, 0], [0.9, 30], [1.1, 90], [1.13, 185], [1.15, 90], [1.5, 60], [2, 30], [2.3, 10], [2.6, 0]]},
  {"label": "5.0 V", "points": [[0.6, 0], [1, 50], [1.3, 170], [1.4, 330], [1.46, 640], [1.5, 230], [2, 175], [2.5, 125], [3, 80], [3.5, 40], [4, 12], [4.3, 0]]}
 ]}
```
[[6::Figure 5-2]]

```curve id=icc-vcc caption="Figure 5-3. Supply current across supply voltage"
{"x": {"label": "V_CC", "unit": "V", "range": [0.5, 5.5]},
 "y": {"label": "I_CC", "unit": "nA", "range": [0, 80]},
 "series": [
  {"label": "25°C", "points": [[0.5, 1], [1.5, 2], [2.5, 3], [3.5, 4.5], [4.5, 8], [5, 10], [5.5, 14]]},
  {"label": "125°C", "points": [[0.5, 28], [1, 31], [1.5, 35], [2, 36], [2.5, 39], [2.9, 31], [3.5, 43], [4, 39], [4.5, 48], [4.6, 40], [5, 50], [5.5, 66]]},
  {"label": "-40°C", "points": [[0.6, 0], [1, 6], [1.5, 11], [2, 16], [2.5, 20], [3, 24.5], [3.5, 28], [4, 31], [4.5, 35], [5, 38], [5.5, 44]]}
 ]}
```
[[6::Figure 5-3]]

```curve id=vol-iol caption="Figure 5-4. Output voltage vs current in LOW state"
{"x": {"label": "I_OL", "unit": "A", "range": [0, 0.035]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.175]},
 "series": [
  {"label": "1.8 V", "points": [[0.0005, 0.007], [0.004, 0.05]]},
  {"label": "2.5 V", "points": [[0.0005, 0.006], [0.008, 0.069]]},
  {"label": "3.3 V", "points": [[0.0005, 0.004], [0.024, 0.165]]},
  {"label": "5 V", "points": [[0.0005, 0.003], [0.032, 0.17]]}
 ]}
```
[[6::Figure 5-4]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily for the
examples listed in the following table. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1MHz,
Z~O~ = 50Ω, t~t~ ≤ 2.5ns. [[7::1]]

The outputs are measured individually with one input transition per
measurement. [[7::2]]

```table id=s1 caption="Table 6-1. Open-drain outputs"
| Test | S1 |
|---|---|
| t~PLZ~, t~PZL~ | CLOSED |
```
[[7::Table 6-1]]

```table id=load caption="Table 6-2. 3-state or open-drain outputs"
| V~CC~ | V~t~ | R~L~ | C~L~ | ΔV | V~LOAD~ |
|---|---|---|---|---|---|
| 1.8V ± 0.15V | V~CC~/2 | 1kΩ | 15pF/30pF | 0.15V | 2×V~CC~ |
| 2.5V ± 0.2V | V~CC~/2 | 500Ω | 15pF/30pF | 0.15V | 2×V~CC~ |
| 3.3V ± 0.3V | 1.5V | 500Ω | 15pF/50pF | 0.3V | 6V |
| 5.0V ± 0.5V | 1.5V | 500Ω | 15pF/50pF | 0.3V | 6V |
```
[[7::Table 6-1]]

Conversion note: Figure 6-1, load circuit for open-drain outputs, shows
the output under test and its test point with C~L~ and one R~L~ to
ground, and a second R~L~ through switch S1 to V~LOAD~.

Conversion note: Figure 6-2, voltage waveforms propagation delays,
shows t~PLZ~ from V~t~ of an input transition to V~OL~ + ΔV of the
rising output and t~PZL~ from V~t~ of an input transition to 50% of the
falling output, for two outputs of opposite phase, and it is not drawn
here.

C~L~ includes probe and test-fixture capacitance. t~PLZ~ is the same as t~dis~. t~PZL~ is the same as t~en~. [[7::3]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

The SN74LVC1G06-Q1 is a high-drive CMOS device that can be used in an
LED application. It can sink 32mA of current at 4.5V making it
appropriate for high-drive applications. It is good for high-speed
applications up to 100MHz. The inputs are 5.5V tolerant allowing it to
translate up or down to V~CC~. Figure 8-1 shows a simple LED driver
application for a single channel of the device. [[11::2]]

Conversion note: in the typical application diagram (Figure 8-1) the
input is driven from an MCU, and the output is connected to the cathode
of an LED whose anode goes through a resistor to VPU.

### Design requirements

#### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics of the device as described in the Electrical
Characteristics section. [[12::1]]

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[12::2]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LVC1G06-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Ensure the
maximum total current through GND listed in the Absolute Maximum Ratings
is not exceeded. [[12::3]]

The SN74LVC1G06-Q1 can drive a load with a total capacitance less than
or equal to 50pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50pF. [[12::4]]

The SN74LVC1G06-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin. [[12::5]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation. [[12::6]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[12::7]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[12::8]]

#### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum input
voltage range found in the Absolute Maximum Ratings. [[13::1]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LVC1G06-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. [[13::2]]

The SN74LVC1G06-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability. [[13::3]]

Refer to the Feature Description section for additional information
regarding the inputs for this device. [[13::4]]

#### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[13::5]]

Open-drain outputs can be connected together directly to produce a
wired-AND configuration or for additional output drive strength. [[13::6]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[13::7]]

Refer to the Feature Description section for additional information
regarding the outputs for this device. [[13::8]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[13::9]]
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74LVC1G06-Q1 to one or more of the receiving devices. [[13::9]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[13::9]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[13::9]]

Conversion note: the application curve (Figure 8-2) shows the output
inverting a square-wave input, with the LED status ON while the input
is high and OFF while it is low.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the power
terminal as possible for best results. [[15::1]]

### Layout guidelines

- Bypass capacitor placement [[16::1]]
  - Place near the positive supply terminal of the device [[15::2]]
  - Provide an electrically short ground return path [[15::2]]
  - Use wide traces to minimize impedance [[15::2]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[15::2]]
- Signal trace geometry [[15::2]]
  - 8mil to 12mil trace width [[15::2]]
  - Lengths less than 12cm to minimize transmission line effects [[15::2]]
  - Avoid 90° corners for signal traces [[15::2]]
  - Use an unbroken ground plane below signal traces [[15::2]]
  - Flood fill areas around signal traces with ground [[15::2]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[15::2]]
  - For traces longer than 12cm [[15::2]]
    - Use impedance controlled traces [[15::2]]
    - Source-terminate using a series damping resistor near the output [[15::2]]
    - Avoid branches; buffer each signal that must branch separately [[15::2]]

Conversion note: the layout examples (Figures 8-3 to 8-7) show trace
corners for improved signal integrity, bypass capacitor placement next
to the V~CC~ pin for several package styles, and a damping resistor at
the transmitting port of a long controlled-impedance trace.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74LVC1G06DBVRQ1 | Reel of 3000 | -40 to 125 | 3L1U, 3RWF
SN74LVC1G06DCKRQ1 | Reel of 3000 | -40 to 125 | 1TW
```
[[19::table #1]]

## Conversion notes

- Source pages 1 and 3–16 converted. The ordering lines are from the
  package option addendum (p.19). The table of contents (p.2), the
  support and revision history page (17), the mechanical information
  page (18), the rest of the addendum (p.20), tape and reel (pp.21–22)
  and mechanical pages (23–29) are left out; the generated Package
  section replaces them.
- Dropped: the features list except the lines quoted, the applications
  list, the package information table and the vendor pinout drawings.
  The logic diagram, the load circuit (Figure 6-1), the waveforms
  (Figure 6-2), the typical application diagram (Figure 8-1), the
  application curve (Figure 8-2) and the layout examples (Figures 8-3 to
  8-7) are described in text.
- The pin table's note on its TYPE column is kept as a line under the
  pin descriptions; the generated pin table draws the type from the
  catalog.
- The CAUTION boxes of Clamp Diode Structure and of Power Considerations
  are kept as paragraphs.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- The source's switching characteristics table holds the C~pd~ rows; it
  is kept as one table, as printed.
- Curves digitized from Figures 5-1 to 5-4 (p.6) to about 2 % of full
  scale; the peaks of Figures 5-1 and 5-2 are approximate.
- From images: the curve points of Figures 5-1 to 5-4 (p.6); the
  arrangement of Figure 6-1 and Figure 6-2 (p.7), Figure 8-1 (p.11),
  Figure 8-2 (p.14) and Figures 8-3 to 8-7 (pp.15–16); ΔV in "V~OL~ +
  ΔV" of Figure 6-2 (p.7; the text layer prints "VOL + V").
- Source issues:
  - Source error: the switching characteristics conditions (pp.5–6)
    print "See #i#Parameter Measurement Information"; the "#i#" is kept
    out.
  - Unconfirmed: the Absolute Maximum Ratings (p.4) print a minimum
    junction temperature of -65 °C.
  - Unconfirmed: the Recommended Operating Conditions (p.4) list a
    high-level output current I~OH~ for this open-drain output.
  - Unconfirmed: the input transition rate (p.5) is given for
    V~CC~ = 1.1V to 5.5V, below the functional supply voltage of 1.2 V
    (p.4).
  - Unconfirmed: Ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.5).
  - Unconfirmed: Power Considerations (p.12) speaks of outputting in
    the HIGH state for an open-drain output.
