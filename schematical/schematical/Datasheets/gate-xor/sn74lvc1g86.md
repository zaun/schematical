---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g86",
  "title": "SN74LVC1G86 single 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lvc1g86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES222R",
    "date": "2026-08",
    "pages": 33
  },
  "devices": ["gate:1G86"],
  "parts": ["SN74LVC1G86DBVR", "SN74LVC1G86DBVRE4", "SN74LVC1G86DBVRG4", "SN74LVC1G86DBVT", "SN74LVC1G86DCKR", "SN74LVC1G86DCKRE4", "SN74LVC1G86DCKRG4", "SN74LVC1G86DCKT", "SN74LVC1G86DCKTG4", "SN74LVC1G86DRLR", "SN74LVC1G86YZPR"],
  "packages": {"p5": "DBV, DCK, DRL", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G86 single 2-input exclusive-OR gate

## Description

The SN74LVC1G86 device performs the Boolean function
Y = !{A}B + A!{B} in positive logic. This single 2-input exclusive-OR
gate is designed for operation from 1.65V to 5.5V V~CC~. [[1::1]]

If the input is low, the other input is reproduced in true form at the
output. If the input is high, the signal on the other input is
reproduced inverted at the output. This device has low power
consumption with maximum t~pd~ of 4ns at 3.3V and 15pF capacitive load.
The maximum output drive is ±32mA at 4.5V and ±24mA at 3.3V. [[1::4]]

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current back flow through the device when it is powered down. [[8::3]]

Supports 5V V~CC~ operation. Inputs are over voltage tolerant up to
5.5V. Supports down translation to V~CC~. Latch-up performance exceeds
100mA per JESD 78, Class II. [[1::2]]

## Pinout

```pinout pinout=p5
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground
VCC: Positive Supply
Y: Output Y
```
[[3::table #1]]

## Functional description

A common application is as a true and complement element. [[8::2]]

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an SN74LVC1G86 gate in
positive logic; negation can be shown at any two ports. Conversion
note: the five symbols of the functional block diagram are not drawn
here. [[8::4]]

### Balanced high-drive CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The high drive capability of this device creates fast edges
into light loads so routing and load conditions must be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. Limit the power output of the device to avoid thermal runaway
and damage due to over-current. The electrical and thermal limits
defined the in the [Absolute Maximum Ratings](#absolute-maximum-ratings)
must be followed at all times. [[8::5]]

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the
[Electrical Characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage, given in
the [Recommended Operating Conditions](#recommended-operating-conditions),
and the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I). [[8::6]]

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in Recommended Operating Conditions to avoid excessive
currents and oscillations. If tolerance to a slow or noisy input signal
is required, a device with a Schmitt-trigger input must be used to
condition the input signal prior to the standard CMOS input. [[8::7]]

### Clamp diodes

The inputs and outputs to this device have negative clamping diodes.
Avoid any voltage below or above the input or output voltage specified
in the Absolute Maximum Ratings. In this event, the current must be
limited to the maximum input or output clamp current value indicated in
the Absolute Maximum Ratings to avoid damage to the device. [[9::1]] [[9::2]]

Conversion note: Figure 7-1 shows one clamping diode from GND to the
input (-I~IK~) and one from GND to the output (-I~OK~), and none to
V~CC~.

### Partial power down (I~off~)

The inputs and outputs for this device enter a high impedance state
when the supply voltage is 0V. The maximum leakage into or out of any
input or output pin on the device is specified by I~off~ in the
Electrical Characteristics. [[9::4]]

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Recommended Operating Conditions. [[9::5]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input negative-voltage and output voltage ratings can be exceeded if the input and output clamp-current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3V to 3.6V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3V to 3.6V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | mA |
| ^ | ^ | V~CC~ = 3V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | mA |
| ^ | ^ | V~CC~ = 3V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8V ± 0.15V, 2.5V ± 0.2V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3V ± 0.3V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5V ± 0.5V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | YZP package | -40 | 85 | °C |
| ^ | ^ | DCK, DBV, and DRL packages | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to maintain proper device operation. Refer to the TI application note, Implications of Slow or Floating CMOS Inputs.
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information: thermal metric [1]"
| Package | Pins | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | Unit |
|---|--:|--:|--:|--:|--:|--:|--:|---|
| DBV (SOT-23) | 5 | 357.1 | 263.7 | 264.4 | 195.6 | 262.2 | | °C/W |
| DCK (SOT-SC70) | 5 | 371 | 297.5 | 258.6 | 195.6 | 256.2 | | °C/W |
| YZP (DSBGA) | 5 | 130 | | | | | | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #2]]

Conversion note: the source gives no thermal data for the DRL package.

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | < | 1.65V to 5.5V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4mA | < | 1.65V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8mA | < | 2.3V | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16mA | < | 3V | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24mA | < | ^ | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32mA | < | 4.5V | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | < | 1.65V to 5.5V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4mA | < | 1.65V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8mA | < | 2.3V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16mA | < | 3V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24mA | < | ^ | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32mA | < | 4.5V | | | 0.55 | V |
| I~I~ | A or B input | V~I~ = 5.5V or GND | < | 0 to 5.5V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | –40°C to 85°C | 1.65V to 5.5V | | | 10 | µA |
| ^ | ^ | ^ | –40°C to 125°C | ^ | | | 15 | µA |
| ΔI~CC~ | | One input at V~CC~ - 0.6V, Other inputs at V~CC~ or GND | < | 3V to 5.5V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 3.3V | | 6 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```
[[5::table #3]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8V | V~CC~ = 2.5V | V~CC~ = 3.3V | V~CC~ = 5V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 22 | 22 | 22 | 24 | pF |
```
[[6::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Test conditions | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | T~A~ = –40°C to +85°C | 2.1 | 9.1 | 1 | 4.9 | 0.6 | 4 | 0.8 | 3.3 | ns |
```
[[6::table #2]]

```table id=sw-30-50pf caption="Switching characteristics, C~L~ = 30pF or 50pF [1]" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Test conditions | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | –40°C to +85°C temperature range, see Figure 6-1 | 3.5 | 9.9 | 1.8 | 5.5 | 1.3 | 5 | 1 | 4 | ns |
| ^ | ^ | ^ | –40°C to +125°C temperature range, see Figure 6-1 | 3.5 | 12 | 1.8 | 7 | 1.3 | 6 | 1 | 5 | ns |

[1] Source conflict: this table is headed C~L~ = 30pF or 50pF and points to Figure 6-1, whose table gives C~L~ = 15 pF and R~L~ = 1 MΩ at every V~CC~ (p.7); the source has no table of the 30pF or 50pF load.
```
[[6::table #3]]

## Typical characteristics

```curve id=voh-ioh caption="V~oh~ vs I~oh~ at 4.5V (V~CC~ = 4.5 V, V~IH~ = 3.1 V, V~IL~ = 1.35 V)"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 200]},
 "y": {"label": "V_OH", "unit": "V", "range": [-1, 5]},
 "series": [
  {"label": "85 °C", "points": [[0, 4.49], [20, 4.31], [40, 4.05], [60, 3.85], [80, 3.52], [100, 3.19], [120, 2.8], [140, 2.31], [150, 1.9], [155, 1.52], [160, 1.05], [164, 0.48], [166, -0.14], [168, -0.45], [180, -0.64], [200, -0.74]]},
  {"label": "25 °C", "points": [[0, 4.49], [20, 4.31], [40, 4.11], [60, 3.9], [80, 3.63], [100, 3.38], [120, 3.09], [140, 2.72], [160, 2.05], [170, 1.4], [175, 0.78], [177, 0.25], [178, -0.45], [180, -0.56], [200, -0.74]]},
  {"label": "-40 °C", "points": [[0, 4.49], [20, 4.34], [40, 4.15], [60, 3.98], [80, 3.76], [100, 3.54], [120, 3.28], [140, 3.02], [160, 2.59], [180, 1.86], [186, 1.32], [189, 0.94], [190, 0.32], [192, -0.37], [195, -0.64], [200, -0.74]]}
 ]}
```
[[6::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. [[7::1]]

All parameters and waveforms are not applicable to all devices. [[7::1]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
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

```table id=test-conditions caption="Test conditions (Figure 6-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```
[[6::table #4]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[7::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

The SN74LVC1G86 device can accept input voltages up to 5.5V at any valid
V~CC~ which makes the device an excellent choice for down translation.
Therefore, the SN74LVC1G86 is designed for various bus interface
applications. [[10::2]]

Conversion note: the typical application schematic (Figure 8-1) shows the gate, supplied from a 3.3-V or 5-V regulated rail with a 0.1 µF capacitor to ground, between 5-V system logic on a 5-V accessory and a µC or system logic.

### Design requirements

This device uses CMOS technology and has balanced output drive. Avoid
bus contention because it can drive currents that exceed maximum
limits. The high drive also creates fast edges into light loads, so
routing and load conditions must be considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions [[10::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[10::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as 5.5V
     at any valid V~CC~. [[10::4]]
2. Recommended Output Conditions [[10::4]]
   - Load currents must not exceed 32mA per output and 50mA total for
     the part. [[10::4]]
   - Outputs must not be pulled above V~CC~. [[10::4]]

### Application curve

```curve id=icc-vin caption="I~CC~ vs. V~IN~ (V~IH~ = 4.5V, V~IL~ = 0V, V~CC~ = 5.5V, Temp = 25 °C)"
{"x": {"label": "V_IN", "unit": "V", "range": [0, 6]},
 "y": {"label": "I_CC", "unit": "mA", "range": [0, 100]},
 "series": [
  {"label": "Low->High", "points": [[0, 0], [0.8, 0], [1, 0.5], [1.5, 4], [1.9, 9], [1.98, 15], [2, 30], [2.02, 62], [2.2, 69], [2.5, 73.5], [2.85, 75.7], [2.9, 75], [2.92, 30], [2.95, 18], [3, 14], [3.25, 9.7], [3.5, 7.4], [4, 2.7], [4.5, 0.1], [6, 0]]},
  {"label": "High->Low", "points": [[0, 0], [1, 0], [1.5, 4], [2, 9.4], [2.2, 11.5], [2.22, 15], [2.23, 40], [2.25, 68], [2.4, 72.5], [2.8, 73.5], [2.95, 76.4], [3, 70.8], [3.05, 69.8], [3.2, 71.8], [3.27, 71], [3.3, 55], [3.38, 48.5], [3.4, 30], [3.42, 15], [3.5, 8], [4, 3.4], [4.5, 1], [5, 0], [6, 0]]}
 ]}
```
[[6::Figure 5-1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table. [[11::1]]

Each V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1µF is recommended. If
there are multiple V~CC~ pins, 0.01µF or 0.022µF is recommended for
each power pin. Parallel multiple bypass caps to reject different
frequencies of noise. A 0.1µF and 1µF are commonly used in parallel.
The bypass capacitor must be installed as close to the power pin as
possible for best results. [[11::2]]

### Layout guidelines

Even low data rate digital signals can have high frequency signal
components due to fast edge rates. When a PCB trace turns a corner at a
90° angle, a reflection can occur. A reflection occurs primarily
because of the change of width of the trace. At the apex of the turn,
the trace width increases to 1.414 times the width. This increase
upsets the transmission-line characteristics, especially the
distributed capacitance and self–inductance of the trace which results
in the reflection. Not all PCB traces can be straight and therefore
some traces must turn corners. Figure 8-3 shows progressively better
techniques of rounding corners. Only the last example (BEST) maintains
constant trace width and minimizes reflections. Conversion note:
Figure 8-3 (trace example: WORST, BETTER, BEST corners) is not drawn
here. [[11::3]]

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74LVC1G86DBVR | Reel of 3000 | -40 to 125 | C865, C86F, C86J, C86K, C86R
SN74LVC1G86DBVRE4 | Reel of 3000 | -40 to 125 | C86F
SN74LVC1G86DBVRG4 | Reel of 3000 | -40 to 125 | C86F
SN74LVC1G86DBVT | Reel of 250 | -40 to 125 | C865, C86F, C86J, C86K, C86R
SN74LVC1G86DCKR | Reel of 3000 | -40 to 125 | CH5, CHF, CHJ, CHK, CHR
SN74LVC1G86DCKRE4 | Reel of 3000 | -40 to 125 | CH5
SN74LVC1G86DCKRG4 | Reel of 3000 | -40 to 125 | CH5
SN74LVC1G86DCKT | Reel of 250 | -40 to 125 | CH5, CHF, CHJ, CHK, CHR
SN74LVC1G86DCKTG4 | Reel of 250 | -40 to 125 | CH5
SN74LVC1G86DRLR | Reel of 4000 | -40 to 125 | CH7, CHR
SN74LVC1G86YZPR | Reel of 3000 | -40 to 85 | CH7, CHN
```
[[14::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering lines are from the package
  option addendum (pp.14–16). The support and revision history page
  (13), tape and reel and mechanical pages (17–33) are left out; the
  generated Package section replaces them.
- Dropped: features and applications lists, package information table,
  the functional block diagram (five equivalent exclusive-OR symbols),
  the trace example (Figure 8-3); the typical application schematic
  (Figure 8-1) and the clamping-diode figure (Figure 7-1) are described
  in text.
- From images: the overbars of Y = !{A}B + A!{B} (pp.1 and 8).
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- Curves digitized from Figure 5-1 and Figure 8-2 to about 2 % of full
  scale; the near-vertical parts of both figures are less exact, and
  Figure 8-2's lines run slightly backwards there, which a curve of
  ascending x can't show.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- Source issues:
  - Source conflict: § 5.7 is headed C~L~ = 30pF or 50pF and points to
    Figure 6-1, whose table gives C~L~ = 15 pF and R~L~ = 1 MΩ at every
    V~CC~ (p.7); the source has no table of the 30pF or 50pF load
    (switching table note 1).
  - Source error: § 7.3.1 prints "defined the in the"; kept as printed.
