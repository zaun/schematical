---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv6t14-ep",
  "title": "SN74LV6T14-EP hex Schmitt-trigger inverters with integrated translation",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv6t14-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS982",
    "date": "2024-01",
    "pages": 26
  },
  "devices": ["gate:74_6T14"],
  "parts": ["SN74LV6T14PWREP"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74LV6T14-EP hex Schmitt-trigger inverters with integrated translation

## Description

The SN74LV6T14-EP device contains six independent Inverter with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A} in positive logic. The output level is referenced to the supply
voltage (V~CC~) and supports 1.8-V, 2.5V, 3.3V, and 5V CMOS levels.

The input is designed with a lower threshold circuit to support up
translation for lower voltage CMOS inputs (for example, 1.2V input to
1.8V output or 1.8V input to 3.3V output). In addition, the 5V tolerant
input pins enable down translation (for example, 3.3V to 2.5V output).

Wide operating range of 1.8V to 5.5V.

5.5V tolerant input pins. Up to 150Mbps with 5V or 3.3V V~CC~. Latch-up
performance exceeds 250mA per JESD 17.

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

## Functional description

The SN74LV6T14-EP device contains six independent Inverter with
Schmitt-trigger inputs. Each gate performs the Boolean function
Y = !{A} in positive logic. The output level is referenced to the supply
voltage (V~CC~) and supports 1.8-V, 2.5V, 3.3V, and 5V CMOS levels.

Conversion note: the functional block diagram shows one of six channels,
from input xA through a Schmitt-trigger buffer and an inverter to output
xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due to
overcurrent. The electrical and thermal limits defined in the Absolute
Maximum Ratings must be followed at all times.

Unused push-pull CMOS outputs should be left disconnected.

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes
only as shown in Figure 7-1. Conversion note: Figure 7-1 draws the
output diodes to V~CC~ (+I~OK~) and to GND (-I~OK~), and the input diode
to GND (-I~IK~).

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. For additional information regarding Schmitt-trigger inputs,
please see Understanding Schmitt Triggers.

### LVxT enhanced input voltage

The SN74LV6T14-EP belongs to TI's LVxT family of logic devices with
integrated voltage level translation. This family of devices was
designed with reduced input voltage thresholds to support
up-translation, and inputs tolerant of signals with up to 5.5 V levels
to support down-translation. The output voltage will always be
referenced to the supply voltage (V~CC~), as described in the
Electrical Characteristics table. For proper functionality, input
signals must remain at or above the specified V~IH(MIN)~ level for a
HIGH input state, and at or below the specified V~IL(MAX)~ for a LOW
input state. Source error: the source prints "at or below" for the HIGH
input state; its Up Translation section ("above V~IH(MIN)~") shows it
is at or above. Corrected. Figure 7-2 shows the typical V~IH~ and
V~IL~ levels for the LVxT family of devices, as well as the voltage
levels for standard CMOS devices for comparison. Conversion note:
Figure 7-2 is not drawn here.

The inputs are high impedance and are typically modeled as a resistor
in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I).

The inputs require that input signals transition between valid logic
states quickly, as defined by the input transition time or rate in the
Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could
cause oscillations. More details can be found in the Implications of
Slow or Floating CMOS Inputs application report.

Do not leave inputs floating at any time during operation. Unused
inputs must be terminated at V~CC~ or GND. If a system will not be
actively driving an input at all times, then a pull-up or pull-down
resistor can be added to provide a valid input voltage during these
times. The resistor value will depend on multiple factors; however, a
10-kΩ resistor is recommended and will typically meet all requirements.

#### Down translation

Signals can be translated down using the SN74LV6T14-EP. The voltage
applied at the V~CC~ will determine the output voltage and the input
thresholds as described in the Recommended Operating Conditions and
Electrical Characteristics tables.

When connected to a high-impedance input, the output voltage will be
approximately V~CC~ in the HIGH state, and 0V in the LOW state. As
shown in Figure 7-2, ensure that the input signals in the HIGH state
are between V~IH(MIN)~ and 5.5V, and input signals in the LOW state are
lower than V~IL(MAX)~.

As shown in Figure 7-3 for example, the standard CMOS inputs for
devices operating at 5.0V, 3.3V or 2.5V can be down-translated to match
1.8V CMOS signals when operating from 1.8-V V~CC~.

Down Translation Combinations are as follows:

- 1.8-V V~CC~ – Inputs from 2.5V, 3.3V, and 5.0V
- 2.5V V~CC~ – Inputs from 3.3V and 5.0V
- 3.3V V~CC~ – Inputs from 5.0V

#### Up translation

Input signals can be up translated using the SN74LV6T14-EP. The voltage
applied at V~CC~ will determine the output voltage and the input
thresholds as described in the Recommended Operating Conditions and
Electrical Characteristics tables. When connected to a high-impedance
input, the output voltage will be approximately V~CC~ in the HIGH
state, and 0V in the LOW state.

The inputs have reduced thresholds that allow for input HIGH state
levels which are much lower than standard values. For example, standard
CMOS inputs for a device operating at a 5V supply will have a
V~IH(MIN)~ of 3.5V. For the SN74LV6T14-EP, V~IH(MIN)~ with a 5V supply
is only 2V, which would allow for up-translation from a typical 2.5V to
5V signals.

As shown in Figure 7-3, ensure that the input signals in the HIGH state
are above V~IH(MIN)~ and input signals in the LOW state are lower than
V~IL(MAX)~.

Up Translation Combinations are as follows:

- 1.8-V V~CC~ – Inputs from 1.2V
- 2.5V V~CC~ – Inputs from 1.8V
- 3.3V V~CC~ – Inputs from 1.8V and 2.5V
- 5.0V V~CC~ – Inputs from 2.5V and 3.3V

Conversion note: Figure 7-3, the LVxT up and down translation example,
is not drawn here.

## Function tables

Table 7-1 lists the functional modes of the SN74LV6T14-EP.

```table id=function caption="Function table"
| Inputs [1] | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = high voltage level, L = low voltage level, X = do not care, Z = high impedance
```

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

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.6 | 5.5 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~O~ | Output current | V~CC~ = 1.6V to 2V | | ±3 | mA |
| ^ | ^ | V~CC~ = 2.25 V to 2.75 V | | ±7 | mA |
| ^ | ^ | V~CC~ = 3.3V to 5.0V | | ±15 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.6V to 5.0V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LV6T14-EP | Unit |
| Symbol | Parameter | PW (TSSOP) 14 pins | ^ |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 147.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 77.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 90.9 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 27.2 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 90.2 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | -55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | 1.65 V to 2V | 0.6 | | 1.2 | 0.5 | | 1.27 | V |
| ^ | ^ | 2.25 V to 2.75 V | 0.73 | | 1.39 | 0.64 | | 1.44 | V |
| ^ | ^ | 3V to 3.6V | 0.88 | | 1.59 | 0.80 | | 1.63 | V |
| ^ | ^ | 4.5V to 5.5V | 1.15 | | 2.03 | 1.1 | | 2.07 | V |
| V~T-~ | Negative-going input threshold voltage | 1.65 V to 2V | 0.225 | | 0.685 | 0.185 | | 0.755 | V |
| ^ | ^ | 2.25 V to 2.75 V | 0.295 | | 0.775 | 0.265 | | 0.805 | V |
| ^ | ^ | 3V to 3.6V | 0.385 | | 0.875 | 0.345 | | 0.895 | V |
| ^ | ^ | 4.5V to 5.5V | 0.535 | | 1.075 | 0.495 | | 1.085 | V |
| ΔV~T~ | Hysteresis (V~T+~ − V~T−~) | 1.65 V to 2V | 0.35 | | 0.68 | 0.28 | | 0.8 | V |
| ^ | ^ | 2.25 V to 2.75 V | 0.4 | | 0.77 | 0.33 | | 0.87 | V |
| ^ | ^ | 3V to 3.6V | 0.44 | | 0.88 | 0.38 | | 0.91 | V |
| ^ | ^ | 4.5V to 5.5V | 0.53 | | 1.2 | 0.51 | | 1.4 | V |
| V~OH~ | I~OH~ = -50 µA | 1.65 V to 5.5V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | I~OH~ = -2mA | 1.65 V to 2V | 1.28 | 1.7 [1] | | 1.21 | | | V |
| ^ | I~OH~ = -3mA | 2.25 V to 2.75 V | 2 | 2.4 [1] | | 1.93 | | | V |
| ^ | I~OH~ = -5.5mA | 3V to 3.6V | 2.6 | 3.08 [1] | | 2.49 | | | V |
| ^ | I~OH~ = -8mA | 4.5V to 5.5V | 4.1 | 4.65 [1] | | 3.95 | | | V |
| V~OL~ | I~OL~ = 50 µA | 1.65 V to 5.5V | | | 0.1 | | | 0.1 | V |
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

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5V, C~L~ = 50pF, T~A~ = 25°C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 1 | 1.2 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | -0.8 | -0.3 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | 5 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.1 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.5 | V |
```

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

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1MHz, Z~O~ = 50Ω.

For clock inputs, f~max~ is measured when the input duty cycle is 50%.

The outputs are measured one at a time with one input transition per
measurement.

C~L~ includes probe and test-fixture capacitance. The greater between
t~PLH~ and t~PHL~ is the same as t~pd~. The greater between t~r~ and
t~f~ is the same as t~t~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: see table
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and input and output transition times"
VM: 50%
```

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality.

The SN74LV6T14-EP can be used to add an additional stage to a counter
with an external flip-flop. Because counters use a negative edge
trigger, the flip-flop's clock input must be inverted to provide this
function. Having Schmitt-trigger inputs is important in this application
to eliminate any noise issues that could impact the counting function
which could lead to incorrect frequency division. This function only
requires one of the six available inverters in the SN74LV6T14-EP device,
so the remaining channels can be used for other applications needing an
inverted signal or improved signal integrity. Unused inputs must be
terminated at V~CC~ or GND. Unused outputs can be left floating.

Conversion note: the typical application block diagram (Figure 8-1)
shows a counter with outputs 2^0^ to 2^3^ whose 2^3^ output goes through
one inverter to the clock input of a D-type flip-flop; the flip-flop's
!{Q} output goes back to its D input, its Q output is 2^4^, and Clear
goes to the CLR inputs of both.

### Design requirements

#### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74LV6T14-EP plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LV6T14-EP plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74LV6T14-EP can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF.

The SN74LV6T14-EP can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin.

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note.

Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

#### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LV6T14-EP (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

The SN74LV6T14-EP has no input signal transition rate requirements
because it has Schmitt-trigger inputs.

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit.

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

Refer to the Feature Description section for additional information
regarding the inputs for this device.

#### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

Refer to the Feature Description section for additional information
regarding the outputs for this device.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to be placed physically close to the device and electrically close to both the V~CC~ and GND pins. An example layout is shown in the Layout section.
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a hard limit; it will, however, optimize performance. This can be accomplished by providing short, appropriately sized traces from the SN74LV6T14-EP to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the Absolute Maximum Ratings is not violated. Most CMOS inputs have a resistive load measured in MΩ; much larger than the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power consumption and thermal increase, however, can be calculated using the steps provided in the application report, CMOS Power Consumption and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows the
counter output 2^3^, its inverse and the flip-flop output 2^4^, with the
legend "Input – 32 kHz, 2^4^ – 1 kHz".

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1μF and 1μF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the power
terminal as possible for best results, as shown in the following layout
example.

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
convenient.

Conversion note: the example layout for the SN74LV6T14-EP (Figure 8-3)
is given here by its labels.

- Recommend GND flood fill for improved signal isolation, noise reduction, and thermal dissipation
- Bypass capacitor placed close to the device
- 0.1 µF
- Unused input tied to GND
- Unused input tied to V~CC~
- Unused output left floating
- Avoid 90° corners for signal lines

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LV6T14PWREP | Reel of 3000 | -55 to 125 | LV614EP
```

## Conversion notes

- Source pages 1 and 3–17 converted. The ordering line is from the
  package option addendum (p.19). The table of contents (p.2), the
  support and revision history page (18), the rest of the addendum
  (p.20), tape and reel and mechanical pages (21–26) are left out; the
  generated Package section replaces them.
- Dropped: features list (the operating range, input tolerance, data
  rate and latch-up lines are kept in Description), applications list,
  package information table, the vendor pinout drawing, the pin table's
  note on the TYPE letters, the simplified logic diagram and functional
  block diagram (pp.1 and 11, described in one sentence), the clamp
  diode figure (Figure 7-1, p.11), the application block diagram and
  timing diagram (Figures 8-1 and 8-2, pp.14 and 16), each described in
  one sentence; the layout example (Figure 8-3, p.17) is given as a list
  of its labels.
- Not converted: the typical characteristics (Figures 5-1 to 5-13,
  supply current across input voltage and supply voltage, and output
  voltage vs current in HIGH and LOW state, pp.7–9), the LVxT input
  voltage levels (Figure 7-2, p.12) and the translation example
  (Figure 7-3, p.13).
- The addendum (p.19) also lists V62/24622-01XE, which the catalog does
  not carry.
- The electrical characteristics table of the source prints the
  threshold and hysteresis names in its test conditions column, where
  they stay; it has no Parameter column. The source's Noise
  Characteristics table is under Electrical characteristics. The thermal
  information table continues on p.5 and is one table here.
- The load circuit (Figure 6-1) and the waveforms (Figures 6-2 and 6-3,
  p.10) are drawn with the shared figures; their notes are kept as
  sentences. The load capacitance is in the switching characteristics.
- From images: overbars of Y = !{A} (pp.1 and 11) and of !{Q}
  (Figure 8-1, p.14); the superscripts of 2^0^ to 2^4^ (Figures 8-1 and
  8-2, pp.14 and 16; the text layer has 20 to 24); µF (0.1 µF in
  Figure 8-3, p.17; the text layer has "0.1 F").
- Source issues:
  - Source error: the LVxT enhanced input voltage section (p.12) prints
    "at or below the specified V~IH(MIN)~ level for a HIGH input state";
    the Up Translation section (p.13, "above V~IH(MIN)~") shows it is at
    or above. Corrected.
  - Source conflict: the features (p.1) give a wide operating range of
    1.8V to 5.5V; the recommended operating conditions (p.4) give a
    supply voltage of 1.6 V minimum, and the electrical characteristics
    (p.5) start at 1.65 V.
  - Unconfirmed: the LVxT section (p.12) refers to V~IH(MIN)~ and
    V~IL(MAX)~ and to an input transition rate requirement; the tables
    give thresholds V~T+~ and V~T-~ and no V~IH~ or V~IL~ rows, and the
    input considerations (p.15) say the device has no input signal
    transition rate requirements.
  - Unconfirmed: the power considerations (p.15) refer to the maximum
    junction temperature listed in the Absolute Maximum Ratings, which
    has no junction temperature row.
  - Source error: the source prints "six independent Inverter" (pp.1 and
    11); kept as printed.
