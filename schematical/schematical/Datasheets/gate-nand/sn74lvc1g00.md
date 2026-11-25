---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g00",
  "title": "SN74LVC1G00 single 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74lvc1g00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES212AC",
    "date": "2026-08",
    "pages": 50
  },
  "devices": ["gate:1G00", "gate:1G00_DPW", "gate:1G00_SON"],
  "parts": ["SN74LVC1G00DBVR", "SN74LVC1G00DBVRE4", "SN74LVC1G00DBVRG4", "SN74LVC1G00DBVT", "SN74LVC1G00DBVTE4", "SN74LVC1G00DBVTG4", "SN74LVC1G00DCKR", "SN74LVC1G00DCKRE4", "SN74LVC1G00DCKRG4", "SN74LVC1G00DCKT", "SN74LVC1G00DCKTG4", "SN74LVC1G00DRLR", "SN74LVC1G00YZPR", "SN74LVC1G00DPWR", "SN74LVC1G00DRY2", "SN74LVC1G00DRYR", "SN74LVC1G00DSF2", "SN74LVC1G00DSFR", "SN74LVC1G00DSFRG4"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G00 single 2-input positive-NAND gate

## Description

The SN74LVC1G00 device is a single 2-input positive NAND gate. This
device performs the Boolean function Y = !{A • B} or Y = !{A} + !{B} in
positive logic. [[1::2]]

Supports 5V V~CC~ operation. Inputs accept voltages to 5.5V. Provides
down translation to V~CC~. I~off~ Supports live insertion,
partial-power-down mode, and back drive protection. [[1::1]]

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: in the DPW package A is pin 2 and B is pin 1 (Pin Functions table).

```pins
A: Input
B: Input
GND: Ground
NC: Not connected
VCC: Power pin
Y: Output
```
[[3::table #1]]

## Functional description

This device is a 2-input NAND Gate which performs the Boolean function
Y = !{A ● B} in positive logic. [[10::1]]

Conversion note: the functional block diagram shows one 2-input NAND
gate with buffered inputs A and B and a buffered output Y.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device can create fast edges into light loads,
so routing and load conditions must be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. Limit the
output power of the device to avoid damage due to overcurrent. The
electrical and thermal limits defined in the Absolute Maximum Ratings
must be followed at all times. [[10::2]]

Unused push-pull CMOS outputs must be left disconnected. [[10::3]]

### Partial power down (I~off~)

This device includes circuitry to disable all outputs when the supply
pin is held at 0V. When disabled, the outputs neither source nor sink
current, regardless of the input voltages. The amount of leakage current
at each output is defined by the I~off~ specification in the Electrical
Characteristics table. [[10::4]]

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I). [[10::5]]

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the Recommended Operating Conditions table. Failing to meet this
specification results in excessive power consumption and can cause
oscillations. [[10::6]]

Do not leave standard CMOS inputs floating at any time during operation.
Terminate unused inputs at V~CC~ or GND. If a system does not always
drive an input, consider adding a pull-up or pull-down resistor to
provide a valid input voltage. The resistor value depends on multiple
factors; a 10kΩ resistor, however, is recommended and typically meets
all requirements. [[10::7]]

### Clamp diode structure

Figure 7-1 shows the inputs and outputs to this device have negative
clamping diodes only. [[10::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[11::1]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[11::Figure 7-1]]

### Over-voltage tolerant inputs

The SN74LVC1G00 includes over-voltage tolerant inputs. The input pins
support high-state input voltages larger than the supply voltage by
utilizing an input protection circuit that does not include a standard
positive clamp diode. See Recommended Operating Conditions for the
recommended input voltage. [[11::3]]

Do not exceed the maximum input voltage range provided in Absolute
Maximum Ratings. Exceeding the maximum input voltage range rating can
cause immediate and permanent damage to the device. [[11::4]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[11::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 0 | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | 0 | 1000 | ^ |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ [1] | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | ^ |
| ^ | ^ | V~CC~ = 3V to 3.6V | 2 | | ^ |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 0.7 × V~CC~ | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3V to 3.6V | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.3 × V~CC~ | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8V ± 0.15V, 2.5V ± 0.2V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3V ± 0.3V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5V ± 0.5V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application note, Implications of Slow or Floating CMOS Inputs.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Package | Pins | Thermal metric [1] | < | < | < | < | < | Unit |
| ^ | ^ | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | ^ |
|---|--:|--:|--:|--:|--:|--:|--:|---|
| DBV (SOT-23) | 5 | 357.1 | 263.7 | 264.4 | 195.6 | 262.2 | | °C/W |
| DCK (SOT-SC70) | 5 | 371 | 297.5 | 258.6 | 195.6 | 256.2 | | °C/W |
| DRY (SON) | 6 | 439 | 277 | 271 | 84 | 271 | | °C/W |
| YZP (DSBGA) | 5 | 130 | 54 | 51 | 1 | 50 | | °C/W |
| DPW (X2SON) | 5 | 340 | 215 | 294 | 41 | 294 | 250 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```
[[5::table #2]]

Conversion note: the source gives no thermal data for the DRL and DSF packages.

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | Recommended –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –100μA | 1.65V to 5.5V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –4mA | 1.65V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = –8mA | 2.3V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = –16mA | 3V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = –24mA | ^ | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = –32mA | 4.5V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100μA | 1.65V to 5.5V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4mA | 1.65V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8mA | 2.3V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16mA | 3V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24mA | ^ | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32mA | 4.5V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | A or B inputs | V~I~ = 5.5V or GND | 0 to 5.5V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5V or GND, I~O~ = 0 | 1.65V to 5.5V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6V, Other inputs at V~CC~ or GND | 3V to 5.5V | | | 500 | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3V | | 4 | | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
```
[[6::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8V | V~CC~ = 2.5V | V~CC~ = 3.3V | V~CC~ = 5V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 22 | 22 | 23 | 25 | pF |
```
[[7::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.2 | 7.2 | 0.9 | 4.4 | 0.8 | 3.8 | 0.8 | 3.4 | ns |
```
[[6::table #2]]

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF or 50pF (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 3.1 | 9 | 1.3 | 5.5 | 1 | 4.7 | 1 | 4 | ns |
```
[[6::table #3]]

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF or 50pF (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Recommended –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8V ± 0.15V | < | V~CC~ = 2.5V ± 0.2V | < | V~CC~ = 3.3V ± 0.3V | < | V~CC~ = 5V ± 0.5V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 3.1 | 9.7 | 1.3 | 5.8 | 1 | 5 | 1 | 4.3 | ns |
```
[[7::table #2]]

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-40, 2.9], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```
[[7::Figure 5-1]]

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 7.0], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```
[[7::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. [[9::1]]

All parameters and waveforms are not applicable to all devices. [[8::1]]

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
[[8::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[8::Figure 6-1]]

```table id=test-conditions caption="Test conditions (Figure 6-1 and Figure 6-2)"
| V~CC~ | Figure | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | 6-1 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | 6-1 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 6-1 | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | 6-1 | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
| 1.8 V ± 0.15 V | 6-2 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 6-2 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 6-2 | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | 6-2 | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #4]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[8::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[12::1]]

In this application, two 2-input NAND gates are used to create an
active-low SR latch as shown in Figure 8-1. The two additional gates can
be used for a second SR latch, or the inputs can be grounded and both
channels left unused. [[12::2]]

The SN74LVC1G00 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch outputs
LOW, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a LOW signal to the R
input which returns the Q output back to LOW. [[12::3]]

Conversion note: in the typical application block diagram (Figure 8-1),
two cross-coupled NAND gates form the latch; the Tamper Switch drives
input !{S}, the System Controller drives input !{R}, which has resistor
R~1~ to the supply, and output Q goes to the System Controller and
through resistor R~2~ to the Tamper Indicator LED.

### Design requirements

#### Power considerations

Verify that the desired supply voltage is within the range specified in
the Electrical Characteristics. Source error: the supply voltage range
is in the Recommended Operating Conditions (p.5), as Power Supply
Recommendations (p.14) says. The supply voltage sets the device
electrical characteristics, as described in the Electrical
Characteristics section. [[12::5]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74LVC1G00 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Verify that the maximum total current through
V~CC~ listed in the Absolute Maximum Ratings is not exceeded. [[12::6]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LVC1G00 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into the ground connection. Verify that
the maximum total current through GND listed in the Absolute Maximum
Ratings is not exceeded. [[12::7]]

The SN74LVC1G00 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, TI recommends adding a
series resistor between the output and capacitor to prevent excessive
current. [[13::1]]

The SN74LVC1G00 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting in
the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[13::2]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[13::5]]

#### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum input
voltage range found in the Absolute Maximum Ratings. [[13::6]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A pullup resistor is used for a
default state of HIGH, and a pulldown resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LVC1G00 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors. [[13::7]]

The SN74LVC1G00 has CMOS inputs and thus requires fast input transitions
to operate correctly, as defined in the Electrical Characteristics
table. Source error: the input transition rate is in the Recommended
Operating Conditions (p.5), as Standard CMOS Inputs (p.10) says. Slow
input transitions can cause oscillations, additional power consumption,
and reduction in device reliability. [[13::8]]

#### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by the
V~OL~ specification in the Electrical Characteristics. [[13::10]]

Push-pull outputs that can be in opposite states, even for a very short
time period, must never be connected directly together. This can cause
excessive current and damage to the device. [[13::11]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[13::12]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[13::13]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. [[13::15]]
2. Verify that the capacitive load at the output is ≤ 50pF. Low load
   capacitance can be accomplished by providing short, appropriately
   sized traces from the SN74LVC1G00 to the receiving device. [[13::15]]
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Never violate the maximum output current from
   the Absolute Maximum Ratings. Most CMOS inputs have a resistive load
   measured in MΩ; much larger than the minimum calculated previously. [[14::1]]
4. Thermal issues are rarely a concern for logic gates; however, the
   power consumption and thermal increase can be calculated using the
   steps provided in the CMOS Power Consumption and Cpd Calculation
   application note. [[14::1]]

Conversion note: the application timing diagram (Figure 8-2) shows Q
going low when !{R} goes low and going high when !{S} goes low; Q is
drawn undefined before the first !{R} pulse.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating listed in the Recommended Operating Conditions. [[14::2]]

Verify that each V~CC~ terminal has a good bypass capacitor to prevent
power disturbance. For the SN74LVC1G00, a 0.1μF bypass capacitor is
recommended. To reject different frequencies of noise, use multiple
bypass capacitors in parallel. Capacitors with values of 0.1μF and 1μF
are commonly used in parallel. [[14::3]]

### Layout guidelines

- Bypass capacitor placement [[14::4]]
  - Place near the positive supply terminal of the device [[14::4]]
  - Provide an electrically short ground return path [[14::4]]
  - Use wide traces to minimize impedance [[14::4]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[14::4]]
- Signal trace geometry [[14::4]]
  - 8mil to 12mil trace width [[14::4]]
  - Lengths less than 12cm to minimize transmission line effects [[14::4]]
  - Avoid 90° corners for signal traces [[14::4]]
  - Use an unbroken ground plane below signal traces [[14::4]]
  - Flood fill areas around signal traces with ground [[14::4]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[14::4]]
  - For traces longer than 12cm [[14::4]]
    - Use impedance controlled traces [[14::4]]
    - Source-terminate using a series damping resistor near the output [[14::4]]
    - Avoid branches; buffer each signal that must branch separately [[14::4]]

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=X2SON-5-DPW
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74LVC1G00DBVR | Reel of 3000 | -40 to 125 | C005, C00F, C00J, C00K, C00R, C00H, C00P, C00S
SN74LVC1G00DBVRE4 | Reel of 3000 | -40 to 125 | C00F
SN74LVC1G00DBVRG4 | Reel of 3000 | -40 to 125 | C00F
SN74LVC1G00DBVT | Reel of 250 | -40 to 125 | C005, C00F, C00J, C00K, C00R, C00H, C00P, C00S
SN74LVC1G00DBVTE4 | Reel of 250 | -40 to 125 | C00F
SN74LVC1G00DBVTG4 | Reel of 250 | -40 to 125 | C00F
SN74LVC1G00DCKR | Reel of 3000 | -40 to 125 | CA5, CAF, CAJ, CAK, CAR, CAH, CAP, CAS
SN74LVC1G00DCKRE4 | Reel of 3000 | -40 to 125 | CA5 CAS
SN74LVC1G00DCKRG4 | Reel of 3000 | -40 to 125 | CA5 CAS
SN74LVC1G00DCKT | Reel of 250 | -40 to 125 | CA5, CAF, CAJ, CAK, CAR, CAH, CAP, CAS
SN74LVC1G00DCKTG4 | Reel of 250 | -40 to 125 | CA5 CAS
SN74LVC1G00DPWR | Reel of 3000 | -40 to 125 | H4
SN74LVC1G00DRLR | Reel of 4000 | -40 to 125 | CA7, CAR
SN74LVC1G00DRY2 | Reel of 5000 | -40 to 125 | CA
SN74LVC1G00DRYR | Reel of 5000 | -40 to 125 | CA
SN74LVC1G00DSF2 | Reel of 5000 | -40 to 125 | CA
SN74LVC1G00DSFR | Reel of 5000 | -40 to 125 | CA
SN74LVC1G00DSFRG4 | Reel of 5000 | -40 to 125 | CA
SN74LVC1G00YZPR | Reel of 3000 | -40 to 85 | CA7, CAN
```
[[19::table #1]] [[24::table #1]]

## Conversion notes

- Source pages 1–14 converted. The ordering lines are from the package
  option addendum (pp.19–22). The layout example figures (p.15), support
  pages, revision history (16–18), tape and reel and mechanical pages
  (23–50) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (four feature lines are in
  Description), package information table, logic diagram and functional
  block diagram (one NAND gate), layout example figures (Figures 8-3 to
  8-7, generic: trace corners, bypass capacitor placement and a damping
  resistor), references to other documents; the typical application
  block diagram (Figure 8-1) and the application timing diagram (Figure
  8-2) are described in text.
- From images: the overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1)
  and of Y = !{A ● B} (p.10); !{R}, !{S}, R~1~, R~2~ and the connections
  of Figure 8-1 (p.12); !{R}, !{S} and the shape of Figure 8-2 (p.14).
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 6-1 and 6-2 are the
  same circuit with different values, joined in one table.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
- The text layer prints "£2 ns", "1 MW", "500 W" and "VD"; the page
  image shows ≤ 2 ns, 1 MΩ, 500 Ω and V~Δ~.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The source's pin table gives pin numbers 1–5 for the DBV, DCK, DRL and
  YZP packages together; the catalog names the YZP balls.
- Source issues:
  - Source error: Power Considerations (p.12) says the supply voltage
    range is "specified in the Electrical Characteristics"; it is in the
    Recommended Operating Conditions (p.5), as Power Supply
    Recommendations (p.14) says.
  - Source error: Input Considerations (p.13) says the fast input
    transitions are "defined in the Electrical Characteristics table";
    Δt/Δv is in the Recommended Operating Conditions (p.5), as Standard
    CMOS Inputs (p.10) says.
  - Source conflict: the design requirements (p.13) refer to "the
    maximum junction temperature, TJ(max) listed in the Absolute Maximum
    Ratings"; the table (p.4) has no junction temperature row.
  - Source conflict: the application information (p.12) speaks of "two
    2-input NAND gates" and "the two additional gates"; the SN74LVC1G00
    is a single gate (p.1).
  - Source conflict: the recommended operating conditions (p.5) give
    T~A~ –40 to 125 °C for all packages; the package option addendum
    (p.21) lists SN74LVC1G00YZPR at -40 to 85.
  - Source conflict: the pin drawings (p.3) show a DRL package; the
    package information table (p.1) and the thermal information (p.5)
    do not list it.
  - Unconfirmed: Ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.5).
