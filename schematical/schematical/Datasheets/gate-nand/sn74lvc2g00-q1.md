---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g00-q1",
  "title": "SN74LVC2G00-Q1 automotive, dual 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lvc2g00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SLVSNX3",
    "date": "2026-09",
    "pages": 26
  },
  "devices": ["gate:2G00"],
  "parts": ["SN74LVC2G00QDCTRQ1"],
  "packages": {"p8": "DCT"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G00-Q1 automotive, dual 2-input positive-NAND gates

## Description

The SN74LVC2G00-Q1 device is a dual 2-input positive NAND gate. Each
gate performs the Boolean function Y = !{A • B} or Y = !{A} + !{B} in
positive logic. [[1::2]]

AEC-Q100 qualified for automotive applications: Device temperature
grade 1: -40°C to +125°C. Operating range from 1.65V to 3.6V.
Over-voltage tolerant inputs support up to 5.5V independent of V~CC~.
Maximum propagation delay of 3.8ns at 5V V~CC~. Supports
partial-power-down with back drive protection (I~off~). High output
drive strength: ±24mA at 3.3V. [[1::1]]

Source conflict: the features give an operating range from 1.65V to
3.6V, the Recommended Operating Conditions an operating supply voltage
of 1.65 V to 5.5 V.

## Pinout

```pinout pinout=p8
```

## Pin functions

```pins
1A: Channel 1 input A
1B: Channel 1 input B
1Y: Channel 1 output
2A: Channel 2 input A
2B: Channel 2 input B
2Y: Channel 2 output
GND: Ground
VCC: Positive supply
```
[[3::Table 4-1]]

I= input, O = Output, P = Power, G = Ground [[3::Table 4-1]]

## Functional description

This device is a dual 2-input NAND Gate. Each gate performs the Boolean
function Y = !{A ● B} in positive logic. [[1::2]]

Conversion note: the logic diagram (positive logic) shows one of two
channels, a 2-input NAND gate with buffered inputs xA and xB and a
buffered output xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device can create fast edges into light loads,
so routing and load conditions must be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. Limit the
output power of the device to avoid damage due to overcurrent. The
electrical and thermal limits defined in the Absolute Maximum Ratings
must be followed at all times. [[11::2]]

Unused push-pull CMOS outputs must be left disconnected. [[11::3]]

### Partial power down (I~off~)

This device includes circuitry to disable all outputs when the supply
pin is held at 0V. When disabled, the outputs neither source nor sink
current, regardless of the input voltages. The amount of leakage current
at each output is defined by the I~off~ specification in the Electrical
Characteristics table. [[11::4]]

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I). [[11::5]]

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the Recommended Operating Conditions table. Failing to meet this
specification results in excessive power consumption and can cause
oscillations. See more details in Implications of Slow or Floating CMOS
Inputs. [[11::6]]

Do not leave standard CMOS inputs floating at any time during operation.
Terminate unused inputs at V~CC~ or GND. If a system does not always
drive an input, consider adding a pull-up or pull-down resistor to
provide a valid input voltage. The resistor value depends on multiple
factors; a 10kΩ resistor, however, is recommended and typically meets
all requirements. [[11::7]]

### Clamp diode structure

Figure 7-1 shows the inputs and outputs to this device have negative
clamping diodes only. [[11::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings can be exceeded if the input and output clamp-current ratings
are observed. [[12::1]]

Conversion note: Figure 7-1 (electrical placement of clamping diodes for
each input and output) shows a diode from GND to the input, carrying
-I~IK~, and a diode from GND to the output, carrying -I~OK~.

### Over-voltage tolerant inputs

The SN74LVC2G00-Q1 includes over-voltage tolerant inputs. The input pins
support high-state input voltages larger than the supply voltage by
utilizing an input protection circuit that does not include a standard
positive clamp diode. See Recommended Operating Conditions for the
recommended input voltage. [[12::3]]

Do not exceed the maximum input voltage range provided in Absolute
Maximum Ratings. Exceeding the maximum input voltage range rating can
cause immediate and permanent damage to the device. [[12::4]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[12::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0V | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0V | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature [3] | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Unconfirmed: the source prints a minimum junction temperature, the same as the storage temperature's.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 HBM ESD Classification Level 2 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C4B | ±1000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Specifications | Description | Condition | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Functional | 1.2 | | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | (High or low state) | 0 | V~CC~ | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3.0V to 3.6V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-Level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3.0V to 3.6V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.3 × V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | mA |
| ^ | ^ | V~CC~ = 3.0V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | mA |
| ^ | ^ | V~CC~ = 3.0V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.1V to 5.5V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```
[[4::table #3]] [[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | DCT, 5 pins [2] | Unit |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 220 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application note.
[2] Source error: the table's header prints 5 pins; the DCT package of the SN74LVC2G00-Q1 has 8 pins (Package Information, p.1). Kept as printed.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100μA | Over Recommended Operating Conditions | V~CC~ - 0.1 | | | V |
| V~OH~ | I~OH~ = –4mA | 1.65V | 1.2 | | | V |
| V~OH~ | I~OH~ = –8mA | 2.3V | 1.9 | | | V |
| V~OH~ | I~OH~ = –12mA | 2.7V | 2.2 | | | V |
| V~OH~ | I~OH~ = –16mA | 3V | 2.4 | | | V |
| V~OH~ | I~OH~ = –24mA | 3V | 2.3 | | | V |
| V~OH~ | I~OH~ = –32mA | 4.5V | 3.8 | | | V |
| V~OL~ | I~OL~ = 100μA | Over Recommended Operating Conditions | | | 0.2 | V |
| V~OL~ | I~OL~ = 4mA | 1.65V | | | 0.45 | V |
| V~OL~ | I~OL~ = 8mA | 2.3V | | | 0.3 | V |
| V~OL~ | I~OL~ = 12mA | 2.7V | | | 0.4 | V |
| V~OL~ | I~OL~ = 16mA | 3V | | | 0.4 | V |
| V~OL~ | I~OL~ = 24mA | 3V | | | 0.55 | V |
| V~OL~ | I~OL~ = 32mA | 4.5V | | | 0.55 | V |
| I~I~ | V~I~ = V~CC~ or GND | V~CC~ = 0V to 5.5V | | ±1 | ±5 | µA |
| I~off~ | V~I~ or V~O~ = V~CC~ | V~CC~ = 0V | | ±1 | ±10 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | V~CC~ = 1.65V to 5.5V | | 1 | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6V, other inputs at V~CC~ or GND | 3.0V to 5.5V | | | 500 | µA |
| C~I~ | V~I~ = V~CC~ or GND | 3.3V | | 3.5 | | pF |
| C~O~ | V~O~ = V~CC~ or GND | 3.3V | | 6.3 | | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted). See Parameter Measurement Information"
| Symbol | From (input) | To (output) | –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 3.7 | 9.4 | 1.6 | 5.5 | 1.1 | 4.9 | 1 | 3.8 | ns |
```
[[6::table #1]]

## Typical characteristics

T~A~ = 25°C (unless otherwise noted) [[9::1]]

```curve id=icc-vin-low caption="Supply current across input voltage 1.8V and 2.5V supply"
{"x": {"label": "V_IN - Input Voltage", "unit": "V", "range": [0, 2.5]},
 "y": {"label": "I_CC - Supply Current", "unit": "µA", "range": [0, 60]},
 "series": [
  {"label": "1.8 V", "points": [[0.45, 0], [0.6, 3], [0.75, 13], [0.9, 8], [1, 5], [1.15, 1.5], [1.3, 0]]},
  {"label": "2.5 V", "points": [[0.45, 0], [0.6, 3], [0.75, 13], [0.9, 36], [0.94, 58], [0.96, 40], [1.25, 24], [1.5, 12], [1.75, 4], [2, 0]]}
 ]}
```
[[7::Figure 5-1]]

```curve id=icc-vin-high caption="Supply current across input voltage 3.3V and 5.0V supply"
{"x": {"label": "V_IN - Input Voltage", "unit": "V", "range": [0, 5]},
 "y": {"label": "I_CC - Supply Current", "unit": "µA", "range": [0, 800]},
 "series": [
  {"label": "3.3 V", "points": [[0.6, 0], [0.9, 30], [1.1, 90], [1.13, 185], [1.15, 90], [1.5, 62], [2, 30], [2.3, 10], [2.6, 0]]},
  {"label": "5.0 V", "points": [[0.6, 0], [0.8, 15], [1, 50], [1.15, 90], [1.3, 170], [1.4, 330], [1.46, 640], [1.5, 230], [2, 180], [2.5, 125], [3, 80], [3.5, 40], [4, 12], [4.3, 0]]}
 ]}
```
[[7::Figure 5-2]]

```curve id=icc-vcc caption="Supply current across supply voltage"
{"x": {"label": "V_CC", "unit": "V", "range": [0.5, 5.5]},
 "y": {"label": "I_CC", "unit": "nA", "range": [0, 80]},
 "series": [
  {"label": "25°C", "points": [[0.5, 1], [1.5, 2], [2.5, 3], [3.5, 5], [4.5, 8], [5, 10], [5.5, 14]]},
  {"label": "125°C", "points": [[0.5, 29], [1, 32], [1.5, 35], [2, 36], [2.5, 38], [3, 34], [3.5, 41], [4, 39], [4.5, 46], [5, 50], [5.5, 67]]},
  {"label": "-40°C", "points": [[0.5, 1], [1, 6], [1.5, 11], [2, 15], [2.5, 20], [3, 24], [3.5, 28], [4, 31], [4.5, 35], [5, 38], [5.5, 44]]}
 ]}
```
[[7::Figure 5-3]]

```curve id=voh caption="Output voltage vs current in HIGH state"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [1, 5]},
 "series": [
  {"label": "1.8 V", "points": [[-8, 1.47], [-0.5, 1.78]]},
  {"label": "2.5 V", "points": [[-15, 2.02], [-0.5, 2.49]]},
  {"label": "3.3 V", "points": [[-25, 2.62], [-0.5, 3.29]]},
  {"label": "5.0 V", "points": [[-25, 4.45], [-0.5, 4.99]]}
 ]}
```
[[7::Figure 5-4]]

```curve id=vol caption="Output voltage vs current in LOW state"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.55]},
 "series": [
  {"label": "1.8 V", "points": [[0.5, 0.01], [8, 0.245]]},
  {"label": "2.5 V", "points": [[0.5, 0.01], [15, 0.355]]},
  {"label": "3.3 V", "points": [[0.5, 0.01], [25, 0.51]]},
  {"label": "5.0 V", "points": [[0.5, 0.01], [25, 0.445]]}
 ]}
```
[[7::Figure 5-5]]

```curve id=voh-5v caption="Output voltage vs current in HIGH state; 5V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```
[[7::Figure 5-6]]

```curve id=vol-5v caption="Output voltage vs current in LOW state; 5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.418]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.49]]}
 ]}
```
[[8::Figure 5-7]]

```curve id=voh-3v3 caption="Output voltage vs current in HIGH state; 3.3V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [2.5, 3.3]},
 "series": [
  {"label": "-40°C", "points": [[-25, 2.685], [-12.5, 3.0], [-0.5, 3.29]]},
  {"label": "25°C", "points": [[-25, 2.615], [-12.5, 2.975], [-0.5, 3.29]]},
  {"label": "125°C", "points": [[-25, 2.51], [-12.5, 2.93], [-0.5, 3.29]]}
 ]}
```
[[8::Figure 5-8]]

```curve id=vol-3v3 caption="Output voltage vs current in LOW state; 3.3V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.6]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.475]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.508]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.568]]}
 ]}
```
[[8::Figure 5-9]]

```curve id=voh-2v5 caption="Output voltage vs current in HIGH state; 2.5V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-16, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [1.9, 2.5]},
 "series": [
  {"label": "-40°C", "points": [[-15, 2.07], [-8, 2.275], [-0.5, 2.485]]},
  {"label": "25°C", "points": [[-15, 2.02], [-8, 2.255], [-0.5, 2.485]]},
  {"label": "125°C", "points": [[-15, 1.945], [-8, 2.22], [-0.5, 2.485]]}
 ]}
```
[[8::Figure 5-10]]

```curve id=vol-2v5 caption="Output voltage vs current in LOW state; 2.5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 16]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.4]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [15, 0.325]]},
  {"label": "25°C", "points": [[0.5, 0.01], [15, 0.35]]},
  {"label": "125°C", "points": [[0.5, 0.01], [15, 0.395]]}
 ]}
```
[[8::Figure 5-11]]

```curve id=voh-1v8 caption="Output voltage vs current in HIGH state; 1.8V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-8, -0.5]},
 "y": {"label": "V_OH", "unit": "V", "range": [1.4, 1.8]},
 "series": [
  {"label": "-40°C", "points": [[-8, 1.5], [-4, 1.65], [-0.5, 1.78]]},
  {"label": "25°C", "points": [[-8, 1.465], [-4, 1.635], [-0.5, 1.78]]},
  {"label": "125°C", "points": [[-8, 1.405], [-4, 1.61], [-0.5, 1.78]]}
 ]}
```
[[8::Figure 5-12]]

```curve id=vol-1v8 caption="Output voltage vs current in LOW state; 1.8V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0.5, 8]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.28]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.014], [8, 0.225]]},
  {"label": "25°C", "points": [[0.5, 0.015], [8, 0.242]]},
  {"label": "125°C", "points": [[0.5, 0.016], [8, 0.276]]}
 ]}
```
[[9::Figure 5-13]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[10::1]]

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
[[10::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[10::Figure 6-1]]

```table id=test-conditions caption="Test conditions (Figure 6-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[7::table #1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[10::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[13::1]]

In this application, two 2-input NAND gates are used to create an
active-low SR latch as shown in the Typical application block diagram.
The two additional gates can be used for a second SR latch, or the
inputs can be grounded and both channels left unused. Unconfirmed: the
SN74LVC2G00-Q1 has two gates, and the latch uses two. [[13::2]]

The SN74LVC2G00-Q1 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch outputs
LOW, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a LOW signal to the R
input which returns the Q output back to LOW. [[13::3]]

Conversion note: the typical application block diagram (Figure 8-1)
shows two cross-coupled NAND gates, the !{S} input driven by the tamper
switch, the !{R} input driven by the system controller and pulled up
through R~1~, and the output Q going to the system controller and
through R~2~ to the tamper indicator LED to ground.

### Power considerations

Verify that the desired supply voltage is within the range specified in
the Electrical Characteristics. The supply voltage sets the device
electrical characteristics, as described in the Electrical
Characteristics section. [[13::5]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74LVC2G00-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Verify that the maximum total current through
V~CC~ listed in the Absolute Maximum Ratings is not exceeded. [[13::6]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LVC2G00-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into the ground connection. Verify that
the maximum total current through GND listed in the Absolute Maximum
Ratings is not exceeded. [[13::7]]

The SN74LVC2G00-Q1 can drive a load with a total capacitance less than
or equal to 50pF while still meeting all of the datasheet
specifications. Larger capacitive loads can be applied; however, TI
recommends adding a series resistor between the output and capacitor to
prevent excessive current. [[14::1]]

The SN74LVC2G00-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[14::2]]

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note. [[14::3]]

Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[14::4]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[14::5]]

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[14::6]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A pullup resistor is used for a
default state of HIGH, and a pulldown resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LVC2G00-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. [[14::7]]

The SN74LVC2G00-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Electrical
Characteristics table. Slow input transitions can cause oscillations,
additional power consumption, and reduction in device reliability. [[14::8]]

Refer to the Feature Description for additional information regarding
the inputs for this device. [[14::9]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by the
V~OL~ specification in the Electrical Characteristics. [[14::10]]

Push-pull outputs that can be in opposite states, even for a very short
time period, must never be connected directly together. This can cause
excessive current and damage to the device. [[14::11]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[14::12]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[14::13]]

Refer to the Feature Description section for additional information
regarding the outputs for this device. [[14::14]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[14::15]]
2. Verify that the capacitive load at the output is ≤ 50pF. Low load
   capacitance can be accomplished by providing short, appropriately
   sized traces from the SN74LVC2G00-Q1 to the receiving device. [[14::15]]
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Never violate the maximum output current from
   the Absolute Maximum Ratings. Most CMOS inputs have a resistive load
   measured in MΩ; much larger than the minimum calculated previously. [[15::1]]
4. Thermal issues are rarely a concern for logic gates; however, the
   power consumption and thermal increase can be calculated using the
   steps provided in the CMOS Power Consumption and Cpd Calculation
   application note. [[15::1]]

Conversion note: the application timing diagram (Figure 8-2) shows Q
going high when !{S} goes low and Q going low when !{R} goes low.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating listed in the Recommended Operating Conditions. [[15::2]]

Verify that each V~CC~ terminal has a good bypass capacitor to prevent
power disturbance. For the SN74LVC2G00-Q1, a 0.1μF bypass capacitor is
recommended. To reject different frequencies of noise, use multiple
bypass capacitors in parallel. Capacitors with values of 0.1μF and 1μF
are commonly used in parallel. [[15::3]]

### Layout guidelines

- Bypass capacitor placement [[16::2]]
  - Place near the positive supply terminal of the device [[15::4]]
  - Provide an electrically short ground return path [[15::4]]
  - Use wide traces to minimize impedance [[15::4]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[15::4]]
- Signal trace geometry [[15::4]]
  - 8mil to 12mil trace width [[15::4]]
  - Lengths less than 12cm to minimize transmission line effects [[15::4]]
  - Avoid 90° corners for signal traces [[15::4]]
  - Use an unbroken ground plane below signal traces [[15::4]]
  - Flood fill areas around signal traces with ground [[15::4]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[15::4]]
  - For traces longer than 12cm [[15::4]]
    - Use impedance controlled traces [[15::4]]
    - Source-terminate using a series damping resistor near the output [[15::4]]
    - Avoid branches; buffer each signal that must branch separately [[15::4]]

Conversion note: the layout examples (Figures 8-3 to 8-7) show trace
corners for improved signal integrity, bypass capacitor placement for
TSSOP, WQFN, SOT and SC70 and similar packages, and damping resistor
placement at the transmitting port of a long controlled-impedance trace.

## Package

```package footprint=SSOP-8
```

## Ordering information

```ordering
SN74LVC2G00QDCTRQ1 | Reel of 3000 | | 46A5
```
[[19::table #1]]

## Conversion notes

- Source pages 1 and 3–16 converted. The ordering line is from the
  package option addendum (p.19). The table of contents (p.2), the
  support and revision history page (17), the mechanical information
  page (18), the rest of the addendum (p.20), tape and reel (pp.21–22),
  mechanical pages (23–25) and the notice page (26) are left out; the
  generated Package section replaces them.
- Dropped: applications list, package information table, the vendor
  pinout drawing, the ESD classification and the ±8mA and ±4mA drive
  feature lines; the logic diagram, the clamp diode figure (Figure 7-1),
  the typical application block diagram (Figure 8-1), the application
  timing diagram (Figure 8-2) and the layout examples (Figures 8-3 to
  8-7) are described in text. The other features are sentences in
  Description.
- The pin table's note on its TYPE column is kept as a line under the
  pin descriptions; the generated pin table draws the type from the
  catalog.
- The two CAUTION boxes of the Feature Description and the one of the
  Power Considerations are kept as paragraphs.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 6-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- Curves digitized from Figures 5-1 to 5-13 (pp.7–9) to about 2 % of
  full scale.
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1) and
  of Y = !{A ● B} (p.11); the names !{S} and !{R} of Figures 8-1 and 8-2
  (pp.13, 15); ≤2 ns, ≤2.5 ns, ≤ 10 MHz, 1 kΩ, 500 Ω, 50 Ω, V~Δ~ (p.10;
  the text layer prints "£2 ns", "1 kW", "500 W", "VD"); the curve
  points of Figures 5-1 to 5-13 (pp.7–9); the arrangement of Figure 7-1
  (p.12), Figure 8-1 (p.13), Figure 8-2 (p.15) and Figures 8-3 to 8-7
  (p.16).
- Source issues:
  - Source conflict: the features (p.1) give an operating range from
    1.65V to 3.6V; the Recommended Operating Conditions (p.4) give an
    operating supply voltage of 1.65 V to 5.5 V, and the switching
    characteristics (p.6) go to 5V ± 0.5V.
  - Source error: the Thermal Information table (p.5) prints "5 PINS"
    under DCT; the package information (p.1) and the pin table (p.3)
    give 8 pins. Kept as printed.
  - Unconfirmed: the Absolute Maximum Ratings (p.4) print a minimum
    junction temperature of -65 °C.
  - Unconfirmed: the application information (p.13) speaks of "the two
    additional gates" of a device with two gates.
  - The package option addendum (p.19) prints "-" as the operating
    temperature of SN74LVC2G00QDCTRQ1; the field is left empty.
