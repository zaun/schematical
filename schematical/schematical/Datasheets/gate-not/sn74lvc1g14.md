---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g14",
  "title": "SN74LVC1G14 single Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES218AA",
    "date": "2025-10",
    "pages": 52
  },
  "devices": ["gate:1G14", "gate:1G14_SON"],
  "parts": ["SN74LVC1G14DBVT", "SN74LVC1G14DCKR", "SN74LVC1G14DCKRG4", "SN74LVC1G14DCKT", "SN74LVC1G14DCKTE4", "SN74LVC1G14DPWR", "SN74LVC1G14DRLR", "SN74LVC1G14YZPR", "SN74LVC1G14DRYR", "SN74LVC1G14DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G14 single Schmitt-trigger inverter

## Description

Latch-up performance exceeds 100mA per JESD 78, Class II. Supports 5V
V~CC~ operation. Inputs accept voltages to 5.5V. [[1::2]]

This single Schmitt-trigger inverter is designed for 1.65V to 5.5V
V~CC~ operation. [[1::1]]

The SN74LVC1G14 device contains one inverter and performs the Boolean
function Y = !{A}. The device functions as an independent inverter with
Schmitt-trigger inputs, so the device has different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals to
provide hysteresis (ΔV~T~) which makes the device tolerant to slow or
noisy input signals. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when the device is
powered down. This inhibits current backflow into the device which
prevents damage to the device. [[1::4]]

## Pinout

Conversion note: the source draws the YZP package in bottom view, and
the drawing here is generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Signal Input
GND: Ground
NC: No internal connection
DNU: Do not use
VCC: Positive Supply
Y: Signal Output
```
[[3::table #1]]

Pins labeled N.C. can be connected to any signal or voltage source,
including ground. They should always be soldered to the board. [[4::1]]

Pins labeled DNU should not be connected to any signal or voltage
source, including ground. They should always be soldered to the board. [[4::1]]

## Functional description

The SN74LVC1G14 single Schmitt-trigger inverter is designed for 1.65 V
to 5.5 V operation and performs the Boolean function Y = !{A}. This
device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when the device is
powered down. This inhibits current backflow into the device which
prevents damage to the device. [[1::1]] [[1::4]]

Conversion note: the logic diagram (Figure 6-1, positive logic) shows one Schmitt-trigger inverter, A (pin 2) to Y (pin 4), for the DBV, DCK, DRL, DRY, DPW, and YZP packages.

### Balanced high-drive CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The high drive capability of this device creates fast edges into light
loads so routing and load conditions should be considered to prevent
ringing. Additionally, the outputs of this device are capable of driving
larger currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to over-current. The electrical and
thermal limits defined the in the Absolute Maximum Ratings must be
followed at all times. [[10::4]]

### CMOS Schmitt-trigger inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using ohm's law (R = V ÷ I). [[10::6]]

The Schmitt-trigger input architecture provides hysteresis as define in
the Electrical Characteristics, which makes this device extremely
tolerant to slow or noisy inputs. While the inputs can be driven much
slower than standard CMOS inputs, it is still recommended to properly
terminate unused inputs. Driving the inputs slowly will also increase
dynamic current consumption of the device. [[10::7]]

### Clamp diodes

The inputs and outputs to this device have negative clamping diodes. [[11::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output negative-voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[11::2]]

```figure ref=clamp-diodes caption="Figure 6-3. Electrical placement of clamping diodes for each input and output"
```
[[11::Figure 6-3]]

### Partial power down (I~off~)

The inputs and outputs for this device enter a high impedance state when
the supply voltage is 0 V. The maximum leakage into or out of any input
or output pin on the device is specified by I~off~ in the Electrical
Characteristics. [[11::4]]

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Absolute Maximum Ratings. [[11::5]]

### Device functional modes

Table 6-1 lists the functional modes of the SN74LVC1G14 device. [[11::6]]

## Function tables

```table id=function caption="Table 6-1. Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[11::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~j~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | ^ |
| ^ | ^ | Machine Model (A115-A) | 200 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
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
| T~A~ | Operating free-air temperature | YZP, YZV, and DPW packages | -40 | 85 | °C |
| ^ | ^ | All other packages | -40 | 125 | ^ |

[1] All unused inputs of the device must be held at V~CC~ or GND to assure proper device operation. See Implications of Slow or Floating CMOS Inputs.
```
[[6::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LVC1G14 | < | < | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT-5X3) 5 pins | DRY (SON) 5 pins [2] | DPW (X2SON) 5 pins | YZV (DSBGA) 4 pins | YZP (DSBGA) 5 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371.0 | 296.2 | 369.6 | 522.9 | 168.2 | 146.2 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | 137.3 | 257.6 | 250.5 | 2.1 | 1.4 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | 145.3 | 230.8 | 384.0 | 55.9 | 39.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | 14.7 | 77.2 | 46.5 | 1.1 | 0.7 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | 145.9 | 231.0 | 382.8 | 56.3 | 39.3 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | 174.1 | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
[2] Source error: the table prints "5 PINS" under DRY; the package information (p.1) and the pin drawing (p.3) give the DRY package 6 pins. Kept as printed.
```
[[6::table #2]]

Conversion note: the source gives no thermal data for the DSF package.

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C [2] | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 1.65 V | 0.79 | | 1.16 | 0.79 | | 1.16 | V |
| ^ | ^ | | 2.3 V | 1.11 | | 1.56 | 1.11 | | 1.56 | ^ |
| ^ | ^ | | 3 V | 1.5 | | 1.87 | 1.5 | | 1.87 | ^ |
| ^ | ^ | | 4.5 V | 2.16 | | 2.74 | 2.16 | | 2.74 | ^ |
| ^ | ^ | | 5.5 V | 2.61 | | 3.33 | 2.61 | | 3.33 | ^ |
| V~T–~ | Negative-going input threshold voltage | DBV, DCK, DRL, DRY, DSF, YZV and YZP packages | 1.65 V | 0.39 | | 0.62 | 0.39 | | 0.64 | V |
| ^ | ^ | ^ | 2.3 V | 0.58 | | 0.87 | 0.58 | | 0.89 | ^ |
| ^ | ^ | ^ | 3 V | 0.84 | | 1.14 | 0.84 | | 1.16 | ^ |
| ^ | ^ | ^ | 4.5 V | 1.41 | | 1.79 | 1.41 | | 1.79 | ^ |
| ^ | ^ | ^ | 5.5 V | 1.87 | | 2.29 | 1.87 | | 2.29 | ^ |
| V~T–~ | Negative-going input threshold voltage | DPW package | 1.65 V | 0.44 | | 0.67 | | | | V |
| ^ | ^ | ^ | 2.3 V | 0.63 | | 0.92 | | | | ^ |
| ^ | ^ | ^ | 3 V | 0.89 | | 1.19 | | | | ^ |
| ^ | ^ | ^ | 4.5 V | 1.46 | | 1.84 | | | | ^ |
| ^ | ^ | ^ | 5.5 V | 1.92 | | 2.34 | | | | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 1.65 V | 0.37 | | 0.62 | 0.37 | | 0.62 | V |
| ^ | ^ | | 2.3 V | 0.48 | | 0.77 | 0.48 | | 0.77 | ^ |
| ^ | ^ | | 3 V | 0.56 | | 0.87 | 0.56 | | 0.87 | ^ |
| ^ | ^ | | 4.5 V | 0.71 | | 1.04 | 0.71 | | 1.04 | ^ |
| ^ | ^ | | 5.5 V | 0.71 | | 1.11 | 0.71 | | 1.11 | ^ |
| V~OH~ | | I~OL~ = –100 µA [3] | 1.65 V to 4.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OL~ = –4 mA | 1.65 V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | I~OL~ = –8 mA | 2.3 V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | I~OL~ = –16 mA | 3 V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | I~OL~ = –24 mA | ^ | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | I~OL~ = –32 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.7 | ^ |
| I~I~ | A input | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4.5 | | | 4.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
[2] These specifications do not apply to DPW, YZV and YZP packages. DPW, YZV and YZP have a recommended operating free-air temperature range of –40°C to 85°C.
[3] Source error: the V~OH~ rows print their test currents as I~OL~; the negative values are the high-level output currents I~OH~ of the Recommended Operating Conditions (p.6). Kept as printed.
```
[[7::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 1.8 V | 20 | pF |
| ^ | ^ | ^ | 2.5 V | 21 | ^ |
| ^ | ^ | ^ | 3.3 V | 22 | ^ |
| ^ | ^ | ^ | 5 V | 25 | ^ |
```
[[8::table #1]]

## Switching characteristics

```table id=sw-85 caption="Switching characteristics: –40°C to 85°C" conditions="Over recommended operating free-air temperature range, (–40°C to 85°C unless otherwise noted)"
| Parameter | From (input) | To (output) | V~CC~ | C~L~ = 15 pF | < | C~L~ = 30 pF or 50 pF | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 V ± 0.15 V | 2.8 | 9.9 | 3.8 | 11 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.6 | 5.5 | 2 | 6.5 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.6 | 1.8 | 5.5 | ^ |
| ^ | ^ | ^ | 5 V ± 0.5 V | 0.9 | 4.4 | 1.2 | 5 | ^ |
```
[[8::table #2]]

```table id=sw-125 caption="Switching characteristics: –40°C to 125°C" conditions="Over operating free-air temperature range, (–40°C to 125°C unless otherwise noted)"
| Parameter | From (input) | To (output) | V~CC~ | C~L~ = 30 pF or 50 pF | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A | Y | 1.8 V ± 0.15 V | 3.8 | 13 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 2 | 8 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.8 | 6.5 | ^ |
| ^ | ^ | ^ | 5 V ± 0.5 V | 1.2 | 6 | ^ |
```
[[8::table #3]]

## Typical characteristics

T~A~ = 25°C [[7::4]]

```curve id=voh-ioh caption="Figure 5-1. Typical V~OH~ vs I~OH~ - 25°C"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 35]},
 "y": {"label": "V_OH", "unit": "V", "range": [1.5, 5.5]},
 "series": [
  {"label": "VCC = 1.8 V", "points": [[0, 1.8], [4, 1.7]]},
  {"label": "VCC = 2.5 V", "points": [[0, 2.5], [8, 2.38]]},
  {"label": "VCC = 3.3 V", "points": [[0, 3.3], [24, 3.0]]},
  {"label": "VCC = 5 V", "points": [[0, 5.0], [32, 4.7]]}
 ]}
```
[[8::Figure 5-1]]

```curve id=vol-iol caption="Figure 5-2. Typical V~OL~ vs I~OL~ - 25°C"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 35]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.35]},
 "series": [
  {"label": "VCC = 1.8 V", "points": [[0, 0], [4, 0.07]]},
  {"label": "VCC = 2.5 V", "points": [[0, 0], [8, 0.105]]},
  {"label": "VCC = 3.3 V", "points": [[0, 0], [24, 0.265]]},
  {"label": "VCC = 5 V", "points": [[0, 0], [32, 0.32]]}
 ]}
```
[[8::Figure 5-1]]

```curve id=vt-vcc caption="Figure 7-2. Interpolated threshold voltages vs. V~CC~"
{"x": {"label": "Supply Voltage", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "Input Voltage", "unit": "V", "range": [0, 3.5]},
 "series": [
  {"label": "VT+(min)", "points": [[1.65, 0.79], [2.3, 1.11], [3, 1.5], [4.5, 2.16], [5.5, 2.61]]},
  {"label": "VT+(max)", "points": [[1.65, 1.16], [2.3, 1.56], [3, 1.87], [4.5, 2.74], [5.5, 3.33]]},
  {"label": "VT-(min)", "points": [[1.65, 0.39], [2.3, 0.58], [3, 0.84], [4.5, 0.72], [5.5, 0.72]]},
  {"label": "VT-(max)", "points": [[1.65, 0.62], [2.3, 0.87], [3, 1.14], [4.5, 1.04], [5.5, 1.11]]}
 ]}
```
[[13::Figure 7-2]]

## Parameter measurement information

Input pulse is supplied by generator having the following
characteristics: PRR ≤ 10MHz. Z~O~ = 50Ω. [[9::1]]

The outputs are measured one at a time, with one transition per
measurement. [[9::1]]

C~L~ includes probe and jig capacitance. [[9::2]]

```figure ref=load-rl-cl caption="Figure 6-1. Load circuit"
RL: see table
CL: see table
```
[[9::Figure 6-1]]

```table id=test-conditions caption="Table 6-1. Parameter measurement conditions"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~D~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| ^ | ^ | ^ | ^ | ^ | 30 pF | 1 kΩ | ^ |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| ^ | ^ | ^ | ^ | ^ | 30 pF | 500 Ω | ^ |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| ^ | ^ | ^ | ^ | ^ | 50 pF | 500 Ω | ^ |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
| ^ | ^ | ^ | ^ | ^ | 50 pF | 500 Ω | ^ |
```
[[9::Table 6-1]]

```figure ref=waveforms-tpd caption="Figure 6-2. Voltage waveforms, propagation delay times, inverting and non-inverting outputs"
VM: see table
```
[[9::Figure 6-2]]

The maximum value of t~pd~ is the worst case of t~PLH~ or t~PHL~ [[9::3]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[12::1]]

Mechanical input elements, such as push buttons or rotary knobs, offer
simple ways to interact with electronic systems. Typically, these
elements have recoil or bouncing, where the mechanical element makes and
breaks contact multiple times during human interaction. This bouncing
can cause one or more repeated signals to be passed, triggering multiple
actions when only a single input was intended. One potential solution to
mitigating these multiple inputs is by utilizing a Schmitt-trigger to
create a debounce circuit. Figure 7-1 shows an example of this solution. [[12::2]]

The input due to the push button switches multiple times, causing the
output of a non Schmitt-trigger device to trigger multiple times, while
the Schmitt-trigger input device with RC delay limits the output pulse
to a single pulse desired by the user. The separated positive and
negative input voltage threshold values, see Figure 7-2, prevent
multiple triggers from occurring. [[12::3]]

Conversion note: in the push button debounce circuit schematic
(Figure 7-1), R~1~ pulls the push button up to V~CC~, the button
connects that node to ground, and R~2~ with C~1~ to ground filters the
node into input A.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive also creates fast edges into light
loads so routing and load conditions should be considered to prevent
ringing. [[12::5]]

### Detailed design procedure

1. Recommended Input Conditions: [[12::6]]
   - For specified high and low levels, see (V~T+~ and V~T-~) in the
     Recommended Operating Conditions table. [[12::6]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~. [[12::6]]
2. Recommended Output Conditions: [[12::6]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed (Continuous current through V~CC~ or GND) total current
     for the part. These limits are located in the Absolute Maximum
     Ratings table. [[12::6]]

### Application curve

Figure 7-2 is created from the values given in the Electrical
Characteristics. Linear interpolation shows the values between each
given point. [[13::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table. [[13::2]]

The V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended, and it is ok to parallel
multiple bypass caps to reject different frequencies of noise. 0.1-µF
and 1-µF capacitors are commonly used in parallel. The bypass capacitor
must be installed as close to the power pin as possible for best
results. [[13::3]]

### Layout guidelines

Even low data rate digital signals can contain high-frequency signal
components due to fast edge rates. When a printed-circuit board (PCB)
trace turns a corner at a 90° angle, a reflection can occur. A
reflection occurs primarily because of the change of width of the trace.
At the apex of the turn, the trace width increases to 1.414 times the
width. This increase upsets the transmission-line characteristics,
especially the distributed capacitance and self–inductance of the trace
which results in the reflection. Not all PCB traces can be straight and
therefore some traces must turn corners. [[13::4]]

An example layout is given in Figure 7-3 for the DPW (X2SON-5) package.
This example layout includes a 0402 (metric) capacitor and uses the
measurements found in the example board layout appended to this end of
this datasheet. A via of diameter 0.1 mm (3.973 mil) is placed directly
in the center of the device. This via can be used to trace out the
center pin connection through another board layer, or it can be left out
of the layout [[13::5]]

Conversion note: the layout example (Figure 7-3) is not drawn here.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=X2SON-5-DPW
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74LVC1G14DBVT | Reel of 250 | -40 to 125 | C145, C14F, C14J, C14K, C14R, C14H, C14S
SN74LVC1G14DCKR | Reel of 3000 | -40 to 125 | CF5, CFF, CFJ, CFK, CFR, CFT, CFH, CFS
SN74LVC1G14DCKRG4 | Reel of 3000 | -40 to 125 | CF5, CFS
SN74LVC1G14DCKT | Reel of 250 | -40 to 125 | CF5, CFF, CFJ, CFK, CFR, CFT, CFH, CFS
SN74LVC1G14DCKTE4 | Reel of 250 | -40 to 125 | CF5, CFS
SN74LVC1G14DPWR | Reel of 3000 | -40 to 85 | 9H
SN74LVC1G14DRLR | Reel of 4000 | -40 to 125 | CF7, CFR
SN74LVC1G14YZPR | Reel of 3000 | -40 to 85 | CF7, CFN
SN74LVC1G14DRYR | Reel of 5000 | -40 to 125 | CF
SN74LVC1G14DSFR | Reel of 5000 | -40 to 125 | CF
```
[[17::table #1]] [[18::table #1]]

## Conversion notes

- Source pages 1 and 3–14 converted. The ordering lines are from the
  package option addendum (pp.17–18). The table of contents (p.2), the
  support and revision history pages (15–16), the rest of the addendum
  (pp.19–20), tape and reel (pp.21–24) and mechanical pages (25–52) are
  left out; the generated Package section replaces them.
- Dropped: the features list except the lines quoted, the NanoFree
  package sentence of the description, the applications list, the
  package information table and the vendor pinout drawings with their
  legends. The logic diagrams, the push button debounce circuit
  (Figure 7-1) and the layout example (Figure 7-3) are described in
  text.
- The pin table's two notes are kept as paragraphs under the pin
  descriptions. The source names the unconnected pins N.C.; the catalog
  name is NC.
- The CAUTION box of Clamp Diodes is kept as a paragraph.
- The source's electrical characteristics table gives symbols only for
  V~OH~, V~OL~, I~off~, I~CC~, ΔI~CC~ and C~i~, so their Parameter cells
  are empty.
- The catalog doesn't carry the YZV (4-pin DSBGA) parts; the YZV pinout
  (A at A1, V~CC~ at A2, GND at B1, Y at B2) and its logic diagram
  (Figure 6-2, A pin 1 to Y pin 3) are not drawn. The addendum also
  lists SN74LVC1G14DBVR, SN74LVC1G14DBVRE4, SN74LVC1G14DBVRG4,
  SN74LVC1G14DBVTE4, SN74LVC1G14DBVTG4, SN74LVC1G14DCKRE4,
  SN74LVC1G14DCKTG4, SN74LVC1G14DRLRG4, SN74LVC1G14DSFRG4 and
  SN74LVC1G14YZVR, which the catalog doesn't carry.
- Curves digitized from Figures 5-1, 5-2 (p.8) and 7-2 (p.13) to about
  2 % of full scale.
- From images: overbar of Y = !{A} (p.1, p.10); the curve points of
  Figures 5-1, 5-2 and 7-2; the arrangement of Figure 7-1 (p.12).
- Source issues:
  - Source error: the V~OH~ rows of the electrical characteristics (p.7)
    print their test currents as I~OL~; they are the I~OH~ values of the
    Recommended Operating Conditions (p.6). Kept as printed.
  - Source error: the Thermal Information table (p.6) prints "5 PINS"
    under DRY; the package information (p.1) and the pin drawing (p.3)
    give 6 pins. Kept as printed.
  - Source conflict: Figure 7-2 (p.13) is said to be created from the
    electrical characteristics, but it draws V~T–~ near 0.7 V (min) and
    1.1 V (max) at 4.5 V and 5.5 V, where the table (p.7) gives 1.41 V
    to 2.29 V.
  - Source conflict: "Figure 6-1" and "Figure 6-2" are both the load
    circuit and waveforms (p.9) and the logic diagrams (p.10), and
    "Table 6-1" is both the parameter measurement conditions (p.9) and
    the function table (p.11); the Parameter Measurement Information
    heading (p.9) has no section number.
  - Source conflict: the detailed design procedure (p.12) points to
    V~T+~ and V~T-~ in the Recommended Operating Conditions table; they
    are in the Electrical Characteristics (p.7).
  - Source conflict: V~OH~ and V~OL~ at 100 µA are given for
    V~CC~ = 1.65 V to 4.5 V (p.7), while the supply range is 1.65 V to
    5.5 V (p.6).
  - Unconfirmed: ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.6).
  - Unconfirmed: the load figure (Figure 6-1, p.9) has no V~LOAD~, which
    Table 6-1 lists.
