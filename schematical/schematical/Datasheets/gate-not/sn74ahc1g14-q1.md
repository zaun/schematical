---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g14-q1",
  "title": "SN74AHC1G14-Q1 automotive single Schmitt-trigger inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc1g14-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS956A",
    "date": "2023-10",
    "pages": 26
  },
  "devices": ["gate:1G14"],
  "parts": ["SN74AHC1G14QDBVRQ1", "SN74AHC1G14QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G14-Q1 automotive single Schmitt-trigger inverter gate

## Description

The SN74AHC1G14-Q1 is a single inverter gate. The device performs the
Boolean function Y = !{A}. [[1::1]] [[1::2]]

The device functions as an independent inverter gate, but because of the
Schmitt action, gates may have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T−~) signals. [[9::2]]

AEC-Q100 qualified for automotive applications. Operating range of 2-V
to 5.5-V. Low power consumption, 10-µA maximum I~CC~. ±8-mA output drive
at 5 V. Latch-up performance exceeds 250 mA per JESD 17. [[1::3]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No Connect
A: Input
GND: Ground
Y: Output
VCC: Power Supply
```
[[3::Table 5-1]]

## Functional description

The SN74AHC1G14-Q1 is a single inverter gate. The device performs the
Boolean function Y = !{A}. [[1::1]] [[1::2]]

The device functions as an independent inverter gate, but because of the
Schmitt action, gates may have different input threshold levels for
positive-going (V~T+~) and negative-going (V~T−~) signals. [[9::2]]

Conversion note: the functional block diagram shows one Schmitt-trigger inverter with input A and output Y.

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I). [[9::3]]

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. For additional information regarding Schmitt-trigger inputs,
please see Understanding Schmitt Triggers. [[9::4]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[9::5]]

Unused push-pull CMOS outputs should be left disconnected. [[9::6]]

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 8-1. [[10::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[10::2]]

Conversion note: Figure 8-1 draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

### Device functional modes

Table 8-1 lists the functional modes of the SN74AHC1G14-Q1. [[10::4]]

## Function tables

```table id=function caption="Function table"
| Input [1] | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = high voltage level, L = low voltage level
```
[[10::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage range | | -0.5 | 7 | V |
| V~O~ | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state | | -0.5 | 4.6 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 | -20 | | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ | -20 | 20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | -50 | 50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level 2 | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | ^ |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply Voltage | | 2 | 5.5 | V |
| V~IH~ | high-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | input voltage | | 0 | 5.5 | V |
| V~O~ | output voltage | | 0 | V~CC~ | V |
| I~OH~ | high-level output current | V~CC~ = 2 V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | low-level output current | V~CC~ = 2 V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 | | 100 | ns/V |
| Δt/Δv | input transition rise or fall rate | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature [1] | | -55 | 125 | °C |

[1] Source conflict: this table (p.5) prints a minimum of -55 °C; the features (p.1) give device temperature grade 1, -40°C to +125°C, and the electrical and switching characteristics are specified from -40°C to 125°C.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G14-Q1"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278.0 | 293.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 208.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 180.6 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 120.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 179.5 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | V~CC~ = 3 V | 1.2 | | 2.20 | 1.2 | | 2.20 | V |
| ^ | ^ | V~CC~ = 4.5 V | 1.75 | | 3.15 | 1.75 | | 3.15 | V |
| ^ | ^ | V~CC~ = 5.5 V | 2.15 | | 3.85 | 2.15 | | 3.85 | V |
| V~T-~ | Negative-going input threshold voltage | V~CC~ = 3 V | 0.9 | | 1.9 | 0.9 | | 1.9 | V |
| ^ | ^ | V~CC~ = 4.5 V | 1.35 | | 2.75 | 1.35 | | 2.75 | V |
| ^ | ^ | V~CC~ = 5.5 V | 1.65 | | 3.35 | 1.65 | | 3.35 | V |
| ΔV~T~ | Hysteresis (V~T+~ − V~T−~) | V~CC~ = 3 V | 0.3 | | 1.2 | 0.3 | | 1.2 | V |
| ^ | ^ | V~CC~ = 4.5 V | 0.4 | | 1.4 | 0.4 | | 1.4 | V |
| ^ | ^ | V~CC~ = 5.5 V | 0.5 | | 1.6 | 0.5 | | 1.6 | V |
| V~OH~ | I~OH~ = -50 μA | V~CC~ = 2 V | 1.9 | | | 1.9 | | | V |
| ^ | I~OH~ = -50 μA | V~CC~ = 3 V | 2.9 | | | 2.9 | | | V |
| ^ | I~OH~ = -50 μA | V~CC~ = 4.5 V | 4.4 | | | 4.4 | | | V |
| ^ | I~OH~ = -4 mA | V~CC~ = 3 V | 2.58 | | | 2.48 | | | V |
| ^ | I~OH~ = -8 mA | V~CC~ = 4.5 V | 3.94 | | | 3.8 | | | V |
| V~OL~ | I~OL~ = 50 μA [1] | V~CC~ = 2 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 50 μA [1] | V~CC~ = 3 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 50 μA [1] | V~CC~ = 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | I~OL~ = 4 mA [1] | V~CC~ = 3 V | | | 0.36 | | | 0.44 | V |
| ^ | I~OL~ = 8 mA [1] | V~CC~ = 4.5 V | | | 0.36 | | | 0.44 | V |
| I~I~ | V~I~ = 5.5V or GND | 0 V to 5.5 V | -0.1 | | 0.1 | -1 | | 1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | | 4 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 1.7 | 10 | | | | pF |
| C~O~ | V~O~ = V~CC~ or GND | 5 V | | 3 | | | | | pF |
| C~PD~ | Power dissipation capacitance | 5 V | | 14 | | | | | pF |

[1] Source error: printed as I~OH~ = 50 μA, I~OH~ = 4 mA and I~OH~ = 8 mA in the V~OL~ rows; the rows give V~OL~, so the condition is I~OL~.
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | V~CC~ | T~A~ = 25°C | < | < | -40°C to 85°C | < | < | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| T~PLH~ | A | Y | C~L~ = 15 pF | 3.3 V ± 0.3 V | | 5 | 7.1 | 1 | | 8.5 | 1 | | 9.5 | ns |
| T~PHL~ | A | Y | C~L~ = 15 pF | 3.3 V ± 0.3 V | | 5 | 7.1 | 1 | | 8.5 | 1 | | 9.5 | ns |
| T~PLH~ | A | Y | C~L~ = 50 pF | 3.3 V ± 0.3 V | | 7.5 | 10.6 | 1 | | 12 | 1 | | 13 | ns |
| T~PHL~ | A | Y | C~L~ = 50 pF | 3.3 V ± 0.3 V | | 7.5 | 10.6 | 1 | | 12 | 1 | | 13 | ns |
| T~PLH~ | A | Y | C~L~ = 15 pF | 5 V ± 0.5 V | | 3.6 | 5.5 | 1 | | 6.5 | 1 | | 7 | ns |
| T~PHL~ | A | Y | C~L~ = 15 pF | 5 V ± 0.5 V | | 3.6 | 5.5 | 1 | | 6.5 | 1 | | 7 | ns |
| T~PLH~ | A | Y | C~L~ = 50 pF [1] | 5 V ± 0.5 V | | 5.1 | 7.5 | 1 | | 6.5 | 1 | | 7 | ns |
| T~PHL~ | A | Y | C~L~ = 50 pF [1] | 5 V ± 0.5 V | | 5.1 | 7.5 | 1 | | 6.5 | 1 | | 7 | ns |

[1] Unconfirmed: at 5 V the 50 pF rows print the same maximums over temperature as the 15 pF rows, below their own 25°C maximum.
```
[[6::table #1]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[7::1]]

```curve id=voh-5v caption="Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```
[[7::Figure 6-2]]

```curve id=vol-5v caption="Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.42]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.48]]}
 ]}
```
[[7::Figure 6-3]]

```curve id=voh-3v3 caption="Output voltage vs current in HIGH state; 3.3-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [2.5, 3.3]},
 "series": [
  {"label": "-40°C", "points": [[-25, 2.69], [-12.5, 3.0], [-0.5, 3.29]]},
  {"label": "25°C", "points": [[-25, 2.62], [-12.5, 2.975], [-0.5, 3.29]]},
  {"label": "125°C", "points": [[-25, 2.51], [-12.5, 2.93], [-0.5, 3.29]]}
 ]}
```
[[7::Figure 6-4]]

```curve id=vol-3v3 caption="Output voltage vs current in LOW state; 3.3-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.6]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.475]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.51]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.565]]}
 ]}
```
[[7::Figure 6-5]]

Conversion note: Figure 6-1 plots supply current across input voltage for the 3.3-V and 5.0-V supply, a peak at the input threshold that is not digitized here.

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω. [[8::1]]

For clock inputs, f~max~ is measured when the input duty cycle is 50%. [[8::2]]

The outputs are measured one at a time with one input transition per
measurement. [[8::3]]

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and
t~f~ is the same as t~t~. [[8::4]] [[8::6]] [[8::8]]

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```
[[8::Figure 7-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and input and output transition times"
VM: 50%
```
[[8::Figure 7-3]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

The SN74AHC1G14-Q1can be used to add an additional stage to a counter
with an external flip-flop. Because counters use a negative edge
trigger, the flip-flop's clock input must be inverted to provide this
function. Having Schmitt-trigger inputs is important in this application
to eliminate any noise issues that could impact the counting function
which could lead to incorrect frequency division. [[11::2]]

The SN74AHC1G14-Q1 is a high drive CMOS device that can be used for
implementing inversion logic with a high output drive. [[11::3]]

Conversion note: the typical application block diagram (Figure 9-1) shows a counter whose last output drives the inverter, the inverter driving the clock of a D-type flip-flop whose !{Q} output returns to its D input, and a Clear signal on the CLR inputs of both.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section. [[11::5]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHC1G14-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings. [[11::6]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHC1G14-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[11::7]]

The SN74AHC1G14-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF. [[11::8]]

The SN74AHC1G14-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[12::1]]

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note. [[12::2]]

Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[12::3]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[12::4]]

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[12::5]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHC1G14-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[12::6]]

The SN74AHC1G14-Q1 has no input signal transition rate requirements
because it has Schmitt-trigger inputs. [[12::7]]

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit. [[12::8]]

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[12::9]]

Source conflict: § 9.2.1.2 (p.12) says the device has no input signal transition rate requirements; the Recommended Operating Conditions (p.5) give maximum input transition rise or fall rates.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[12::11]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device. [[12::12]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[12::13]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::14]]

Source conflict: § 9.2.1.3 (p.12) speaks of two channels within the same device; the function table (p.10) and the pin table (p.3) show a single gate. Kept as printed.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[12::16]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHC1G14-Q1 to one or more of the receiving devices. [[13::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[13::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[13::1]]

Conversion note: the application timing diagram (Figure 9-2) shows the counter's last output, its inverse and the flip-flop output, which toggles at half the frequency on each falling edge of the counter output; the legend reads Input – 32 kHz and 1 kHz for the flip-flop output.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example. [[13::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used). Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient. [[13::3]]

Conversion note: the example layout (Figure 9-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74AHC1G14QDBVRQ1 | Reel of 3000 | -40 to 125 | 38TH
SN74AHC1G14QDCKRQ1 | Reel of 3000 | -40 to 125 | 1QH
```
[[16::table #1]]

## Conversion notes

- Source pages 1–14 converted; p.2 holds only the table of contents and
  revision history, which are dropped. The ordering lines are from the
  package option addendum (p.16). The support page (15), tape and reel
  and mechanical pages (17–26) are left out; the generated Package
  section replaces them.
- Dropped: applications list (features kept as sentences in the
  Description), package information table, the two "Refer to the Feature
  Description section" sentences of § 9.2.1; the simplified logic
  diagram (p.1), the functional block diagram (p.9), the clamp-diode
  figure (Figure 8-1; the shared clamp-diodes figure also draws an input
  clamp to V~CC~, which this part does not have), the application block
  diagram (Figure 9-1), the application timing diagram (Figure 9-2) and
  the example layout (Figure 9-3) are described in text.
- The pin table's note "I = input, O = output, I/O = input or output,
  G = ground, P = power." is on its TYPE column, which the generated pin
  table draws from the catalog. The source gives pin 1 (NC) the type I.
- The Electrical characteristics table of the source prints the symbols
  under PARAMETER and the names of V~T+~, V~T-~, ΔV~T~ and C~PD~ under
  TEST CONDITIONS; they are kept in that column. The table continues on
  p.6 (C~PD~).
- The input transition rate unit is printed "nS/V" in the source (p.5);
  written ns/V. The switching symbols are printed T~PLH~ and T~PHL~
  (p.6) and kept. The V~CC~ cells of the electrical characteristics are
  printed "Vcc = 3 V"; written V~CC~.
- The notes of Figures 7-1 to 7-3 (C~L~, t~pd~, t~t~) are given as text
  beside the figures. The load circuit (Figure 7-1) prints no value for
  C~L~, which is 15 pF or 50 pF in the switching characteristics.
- Curves digitized from Figures 6-2 to 6-5 (nearly straight lines in the
  source) to about 2 % of full scale. Figure 6-1 is not digitized.
- From images: the overbar of Y = !{A} (p.1 and p.9); the overbar of
  !{Q} (Figure 9-1, p.11); the curve points -0.5, -12.5, 4.99, 4.455,
  4.385, 0.01, 0.42, 0.44, 0.48, 2.69, 3.0, 2.62, 2.975, 2.51, 2.93,
  3.29, 0.475, 0.51, 0.565 (Figures 6-2 to 6-5, p.7); 0.1 µF
  (Figure 9-3, p.14; the text layer prints "0.1 F"); ±2000 and ±1000
  (ESD ratings, p.4; the text layer breaks the values over two lines).
- Source issues:
  - Source error: V~OL~ rows printed with I~OH~ (Electrical
    characteristics note).
  - Source conflict: operating free-air temperature minimum of -55 °C
    against -40°C elsewhere (Recommended operating conditions note).
  - Source conflict: no input transition rate requirements against the
    Δt/Δv rows of the Recommended Operating Conditions (Input
    considerations note).
  - Source conflict: two channels named for a single gate (Output
    considerations note).
  - Unconfirmed: 5 V, 50 pF maximums over temperature (Switching
    characteristics note).
  - Source error: "SN74AHC1G14-Q1can" is printed without a space (p.11);
    kept as printed.
