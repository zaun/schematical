---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv14b-ep",
  "title": "SN74LV14B-EP enhanced product, 2-V to 5.5-V, hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lv14b-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS947",
    "date": "2023-08",
    "pages": 21
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74LV14BMPWREP"],
  "packages": {"p14": "PW"},
  "converted": "2026-11-21"
}
---

# SN74LV14B-EP enhanced product, 2-V to 5.5-V, hex Schmitt-trigger inverters

## Description

The SN74LV14B-EP device contains six independent Inverter with
Schmitt-trigger inputs designed for 2 V to 5.5 V V~CC~ operation. Each
gate performs the Boolean function Y = !{A} in positive logic. [[1::2]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the devices when they are powered down. [[8::2]]

Maximum t~pd~ of 14 ns at 5 V. Supports mixed-mode voltage operation on
all ports. Latch-up performance exceeds 250 mA per JESD 17. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input
1Y: Channel 1, output
2A: Channel 2, Input
2Y: Channel 2, output
3A: Channel 3, Input
3Y: Channel 3, output
GND: Ground
4Y: Channel 4, output
4A: Channel 4, input
5Y: Channel 5, output
5A: Channel 5, input
6Y: Channel 6, output
6A: Channel 6, input
VCC: Positive supply
```
[[3::Table 5-1]]

## Functional description

The SN74LV14B-EP device contains six independent Inverter with
Schmitt-trigger inputs designed for 2 V to 5.5 V V~CC~ operation. Each
gate performs the Boolean function Y = !{A} in positive logic. [[1::2]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the devices when they are powered down. [[8::2]]

Conversion note: the functional block diagram shows one channel from
input xA through a Schmitt-trigger buffer, an inverter and a buffer to
output xY.

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I). [[8::3]]

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. [[8::4]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[8::5]]

Unused push-pull CMOS outputs should be left disconnected. [[8::6]]

### Partial power down (I~off~)

This device includes circuitry to disable all outputs when the supply
pin is held at 0 V. When disabled, the outputs will neither source nor
sink current, regardless of the input voltages applied. The amount of
leakage current at each output is defined by the I~off~ specification in
the Electrical Characteristics table. [[8::7]]

### Clamp diode structure

Figure 8-1 shows the inputs and outputs to this device have negative
clamping diodes only. [[8::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[8::9]]

Conversion note: Figure 8-1 (electrical placement of clamping diodes for
each input and output, p.9) shows one diode from GND to the input,
carrying -I~IK~, and one diode from GND to the output, carrying -I~OK~,
and no diode to V~CC~.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input [1] | Output [2] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Do not Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 5.5 V | V~CC~ × 0.7 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 5.5 V | | V~CC~ × 0.3 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND for proper device operation. See Implications of Slow or Floating CMOS Inputs.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LV14B-EP | Unit |
| ^ | ^ | PW (TSSOP), 14 pins | ^ |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 151 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 80 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 94.2 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 28 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 93.6 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | °C/W |

[1] For more information about traditional and new thermal metrics, see Semiconductor and IC Package Thermal Metrics.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)."
| Symbol | Parameter | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | 2.5 V | | | 1.78 | V |
| ^ | ^ | 3.3 V | | | 2.31 | ^ |
| ^ | ^ | 5 V | | | 3.5 | ^ |
| V~T–~ | Negative-going threshold | 2.5 V | 0.75 | | | V |
| ^ | ^ | 3.3 V | 0.97 | | | ^ |
| ^ | ^ | 5 V | 1.5 | | | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | 2.5 V | 0.25 | | 1 | V |
| ^ | ^ | 3.3 V | 0.33 | | 1.37 | ^ |
| ^ | ^ | 5 V | 0.5 | | 2 | ^ |
| V~OH~ | I~OH~ = –50 µA [1] | 2 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | I~OH~ = –2 mA | 2.3 V | 2 | | | ^ |
| ^ | I~OH~ = –6 mA | 3 V | 2.48 | | | ^ |
| ^ | I~OH~ = –12 mA | 4.5 V | 3.8 | | | ^ |
| V~OL~ | I~OL~ = 50 µA [1] | 2 V to 5.5 V | | | 0.1 | V |
| ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | ^ |
| ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | ^ |
| ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | µA |
| I~off~ | V~I~ or V~O~ = 0 to 5.5 V | 0 V | | | 5 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 2.3 | | pF |

[1] Source error: printed as I~OH~ = –50 mA and I~OL~ = 50 mA (p.5); the recommended operating conditions (p.4) give I~OH~ = –50 µA and I~OL~ = 50 µA at V~CC~ = 2 V and –12 mA and 12 mA at most, and the absolute maximum continuous output current is ±25 mA, so the unit is µA.
```
[[5::table #2]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74LV14A [2] | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
[2] Source error: the source heads these columns SN74LV14A in the data sheet of the SN74LV14B-EP (p.6). Kept as printed.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 8.8 | pF |
| ^ | ^ | ^ | 5 V | 9.6 | ^ |
```
[[6::table #2]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over operating free-air temperature range (unless otherwise noted), (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load cap | 25°C | < | < | –55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 9.6 | 16.3 | 1 | | 20.4 | ns |
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted), (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load cap | 25°C | < | < | –55°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | C~L~ = 50 pF | | 6.7 | 10.6 | 1 | | 14 | ns |
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-vcc caption="Figure 6-1. TPD vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 12]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 10.2], [3.3, 7.3], [5, 5.1]]}
 ]}
```
[[6::Figure 6-1]]

```curve id=tpd-temp caption="Figure 6-2. TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.7], [-40, 4.9], [25, 5.1], [85, 5.5], [125, 5.9]]}
 ]}
```
[[6::Figure 6-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, and t~f~ ≤ 3 ns. The outputs are measured one
at a time, with one input transition per measurement. t~PHL~ and t~PLH~
are the same as t~pd~. All parameters and waveforms are not applicable
to all devices. [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[7::Figure 7-1]]

Conversion note: Figure 7-1 also shows the load circuit for 3-state and
open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the voltage
waveforms for pulse duration, setup and hold times, and enable and
disable times, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

The SN74LV14B-EP can be used to add an additional stage to a counter
with an external flip-flop. Because counters use a negative edge
trigger, the flip-flop's clock input must be inverted to provide this
function. Having Schmitt-trigger inputs is important in this application
to eliminate any noise issues that could impact the counting function
which could lead to incorrect frequency division. This function only
requires one of the six available inverters in the SN74LV14B-EP device,
so the remaining channels can be used for other applications needing an
inverted signal or improved signal integrity. Unused inputs must be
terminated at V~CC~ or GND. Unused outputs can be left floating. [[10::2]]

Conversion note: the typical application block diagram (Figure 9-1,
p.10) shows a Counter with Clear and Input, whose last output goes
through one Schmitt-trigger inverter to the clock input of a D-Type
Flip-Flop that shares the Clear line, has its inverted output connected
to D, and whose Q output is the added stage.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics section. [[11::1]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74LV14B-EP plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings. [[11::2]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LV14B-EP plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[11::3]]

The SN74LV14B-EP can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF. [[11::4]]

The SN74LV14B-EP can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[11::5]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[11::8]]

Source conflict: this caution (p.11) refers to a maximum junction
temperature listed in the Absolute Maximum Ratings, whose table (p.4)
has no junction temperature row.

### Input considerations

Input signals must cross V~t-(min)~ to be considered a logic LOW, and
V~t+(max)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[11::9]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LV14B-EP (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[11::10]]

The SN74LV14B-EP has no input signal transition rate requirements
because it has Schmitt-trigger inputs. [[11::11]]

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit. [[11::12]]

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[11::13]]

Source conflict: the Typical Characteristics (p.6) hold only the two
propagation delay figures and no plot of this additional current.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[12::1]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device. [[12::2]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[12::3]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::4]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[12::6]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, optimize performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74LV14B-EP to one or more of the receiving devices. [[12::6]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, which will no violate the maximum output
   current from the Absolute Maximum Ratings. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously. [[12::6]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[12::6]]

Source error: "which will no violate" is the source's wording for "will
not violate" (Detailed Design Procedure, p.12); kept as printed.

Conversion note: the application timing diagram (Figure 9-2, p.12) shows
the counter's last output, its inverse and the added stage, which
toggles on each falling edge of the counter's last output.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Absolute Maximum Ratings section.
Each V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, TI recommends a 0.1-μF
capacitor; if there are multiple V~CC~ terminals, then TI recommends a
0.01-μF or 0.022-μF capacitor for each power terminal. Multiple bypass
capacitors can be paralleled to reject different frequencies of noise.
Frequencies of 0.1 μF and 1 μF are commonly used in parallel. The bypass
capacitor must be installed as close as possible to the power terminal
for best results. [[12::7]]

Source conflict: this section (p.12) places the supply voltage rating in
the Absolute Maximum Ratings (–0.5 V to 7 V, p.4), while Power
Considerations (p.11) requires the supply voltage to be within the
Recommended Operating Conditions (2 V to 5.5 V, p.4).

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such
unused input pins must not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states. All unused inputs of digital logic devices must be connected to
a logic high or logic low voltage, as defined by the input voltage
specifications, to prevent them from floating. The logic level that must
be applied to any particular unused input depends on the function of the
device. Generally, the inputs are tied to GND or V~CC~, whichever makes
more sense for the logic function or is more convenient. [[13::1]]

Conversion note: the layout example for the SN74LV14B-EP in the PW
package (Figure 9-3, p.13) shows a bypass capacitor placed close to the
device between V~CC~ and GND, a GND flood fill recommended for improved
signal isolation, noise reduction, and thermal dissipation, an unused
input tied to GND, an unused input tied to V~CC~, an unused output left
floating, and the advice to avoid 90° corners for signal lines.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LV14BMPWREP | Reel of 3000 | -55 to 125 | LV14BEP
```
[[15::table #1]]

## Conversion notes

- Source pages 1–13 converted; of p.2 (table of contents and revision
  history) nothing is kept. The ordering line is from the package option
  addendum (pp.15–16). The device and documentation support page (14),
  tape and reel and mechanical pages (17–21) are dropped; the generated
  Package section replaces them.
- Dropped: features and applications lists (three feature lines are in
  Description), the package information table, the logic diagram of
  page 1 (the same as the functional block diagram), the vendor pin
  drawing, the sentences that only point to other documents or sections
  ("For additional information regarding Schmitt-trigger inputs…",
  "Total power consumption can be calculated…", "Thermal increase can be
  calculated…", "Refer to the Feature Description section…"). The
  functional block diagram, the clamping diode figure, the application
  block diagram, the application timing diagram and the layout example
  are each described in one sentence.
- Not claimed here: V62/23623-01XE of the package option addendum, which
  the catalog does not carry.
- The electrical characteristics table of the source has one PARAMETER
  heading over the symbol and over a second column that holds the name
  of V~T+~, V~T–~ and ΔV~T~ and the test condition of the other rows; it
  is the Parameter column here.
- The dash in the thermal table (no value) is an empty cell.
- The two CAUTION boxes (pp.8 and 11) are kept as paragraphs.
- Of Figure 7-1 (p.7) the load circuit for totem-pole outputs and the
  propagation delay waveforms are kept, with notes A, C, D, G and H.
- Curves digitized from Figures 6-1 and 6-2 (p.6) to about 2 % of full
  scale.
- From images: overbars of Y = !{A} (p.1 and p.8); the curve points of
  Figures 6-1 and 6-2 (p.6): 10.2, 7.3, 5.1, -40, 25, 85, 4.7, 4.9, 5.5
  and 5.9.
- Source issues:
  - Source error: V~OH~ and V~OL~ rows printed with I~OH~ = –50 mA and
    I~OL~ = 50 mA; corrected to µA (electrical characteristics note 1).
  - Source error: the noise characteristics (p.6) head the value columns
    SN74LV14A; kept as printed.
  - Source error: "which will no violate" (Detailed Design Procedure,
    p.12), kept as printed.
  - Source conflict: the caution of Power Considerations (p.11) refers
    to a maximum junction temperature in the Absolute Maximum Ratings,
    which list none (p.4).
  - Source conflict: Input Considerations (p.11) refers to a plot of
    additional supply current in the Typical Characteristics, which
    have none (p.6).
  - Source conflict: Power Supply Recommendations (p.12) takes the
    supply voltage range from the Absolute Maximum Ratings, Power
    Considerations (p.11) from the Recommended Operating Conditions.
