---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g32",
  "title": "SN74LVC1G32 single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc1g32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES219W",
    "date": "2026-08",
    "pages": 51
  },
  "devices": ["gate:1G32", "gate:1G32_DPW", "gate:1G32_SON"],
  "parts": ["SN74LVC1G32DBVR", "SN74LVC1G32DBVRE4", "SN74LVC1G32DBVRG4", "SN74LVC1G32DBVT", "SN74LVC1G32DBVTG4", "SN74LVC1G32DCK3", "SN74LVC1G32DCKR", "SN74LVC1G32DCKRE4", "SN74LVC1G32DCKRG4", "SN74LVC1G32DCKT", "SN74LVC1G32DCKTG4", "SN74LVC1G32DRLR", "SN74LVC1G32DRLRG4", "SN74LVC1G32YZPR", "SN74LVC1G32DPWR", "SN74LVC1G32DRY2", "SN74LVC1G32DRY2G4", "SN74LVC1G32DRYR", "SN74LVC1G32DSF2", "SN74LVC1G32DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G32 single 2-input positive-OR gate

## Description

The SN74LVC1G32 device is a single 2-input positive OR gate. The
SN74LVC1G32 device performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

Supports 5V supply voltage (V~CC~) operation. Inputs accept voltages to
5.5V. Supports down translation to V~CC~.

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: the DPW drawing above is the catalog pinout (B at pin 1,
A at pin 2); this data sheet draws the DPW package with A at pin 1 and B
at pin 2 (Figure 4-2).

```pins
A: Input A
B: Input B
GND: Ground Pin
Y: Output Y
VCC: Power Pin
NC: No internal connection
```

## Functional description

The SN74LVC1G32 device contains one two-input positive OR gate device
and performs the Boolean function Y = A + B or Y = !{!{A} • !{B}}. This
device is fully specified for partial-powerdown applications using
I~OFF~. The I~OFF~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device can create fast edges into light loads,
so routing and load conditions must be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. Limit the
output power of the device to avoid damage due to overcurrent. The
electrical and thermal limits defined in the
[Absolute Maximum Ratings](#absolute-maximum-ratings) must be followed at
all times.

Unused push-pull CMOS outputs must be left disconnected.

### Partial power down (I~off~)

This device includes circuitry to disable all outputs when the supply
pin is held at 0V. When disabled, the outputs neither source nor sink
current, regardless of the input voltages. The amount of leakage current
at each output is defined by the I~off~ specification in the Electrical
Characteristics table.

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I).

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the [Recommended Operating Conditions](#recommended-operating-conditions)
table. Failing to meet this specification results in excessive power
consumption and can cause oscillations.

Do not leave standard CMOS inputs floating at any time during operation.
Terminate unused inputs at V~CC~ or GND. If a system does not always
drive an input, consider adding a pull-up or pull-down resistor to
provide a valid input voltage. The resistor value depends on multiple
factors; a 10kΩ resistor, however, is recommended and typically meets
all requirements.

### Clamp diode structure

Figure 7-1 shows the inputs and outputs to this device have negative
clamping diodes only.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

### Over-voltage tolerant inputs

The SN74LVC1G32 includes over-voltage tolerant inputs. The input pins
support high-state input voltages larger than the supply voltage by
utilizing an input protection circuit that does not include a standard
positive clamp diode. See Recommended Operating Conditions for the
recommended input voltage.

Do not exceed the maximum input voltage range provided in Absolute
Maximum Ratings. Exceeding the maximum input voltage range rating can
cause immediate and permanent damage to the device.

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y [2] |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings can be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Definition | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | DSBGA package | -40 | 85 | °C |
| ^ | ^ | All other packages | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Package | Pins | Thermal metric [1] | < | < | < | < | < | Unit |
| ^ | ^ | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | ^ |
|---|--:|--:|--:|--:|--:|--:|--:|---|
| DBV (SOT-23) | 5 | 357.1 | 263.7 | 264.4 | 195.6 | 262.2 | | °C/W |
| DCK (SOT-SC70) | 5 | 371 | 297.5 | 258.6 | 195.6 | 256.2 | | °C/W |
| DRL (SOT-5X3) | 5 | 364.1 | 280.6 | 261.5 | 195.6 | 259.2 | | °C/W |
| DRY (SON) | 6 | 439 | 277 | 271 | 84 | 271 | | °C/W |
| YZP (DSBGA) | 5 | 130 | 54 | 51 | 1 | 50 | | °C/W |
| DPW (X2SON) | 5 | 340 | 215 | 294 | 41 | 194 | 250 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```

Conversion note: the source gives no thermal data for the DSF package.

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C recommended | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.5 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.65 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.65 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±25 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 20 | 20 | 21 | 22 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.9 | 7.2 | 0.8 | 4.4 | 0.9 | 3.6 | 0.8 | 3.4 | ns |
```

```table id=sw-1v8-2v5 caption="Switching characteristics, 1.8 V and 2.5V" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) [1] (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | –40°C to 125°C recommended | < | –40°C to 85°C | < | –40°C to 125°C recommended | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 2.5 V ± 0.2 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.8 | 8 | 2.8 | 9 | 1.2 | 5.5 | 1.2 | 6 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3-5v caption="Switching characteristics, 3.3 V and 5 V" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) [1] (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | –40°C to 85°C | < | –40°C to 125°C recommended | < | –40°C to 85°C | < | –40°C to 125°C recommended | < | Unit |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.1 | 4.5 | 1 | 4 | 1 | 4 | 1 | 4.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested
```

Unconfirmed: at V~CC~ = 3.3 V the –40°C to 125°C maximum (4 ns) is below the –40°C to 85°C maximum (4.5 ns).

## Typical characteristics

```curve id=tpd-temp caption="Powerdown timing vs temperature, V~CC~ = 3.3 V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "Powerdown Timing", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "Powerdown Timing", "points": [[-55, 2.7], [-40, 2.9], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```

```curve id=tpd-vcc caption="Powerdown timing vs supply voltage, T~A~ = 25°C"
{"x": {"label": "Supply Voltage", "unit": "V", "range": [0, 6]},
 "y": {"label": "Powerdown Timing", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "Powerdown Timing", "points": [[1.8, 6.9], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

All parameters and waveforms are not applicable to all devices.

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

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

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

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

In this application, three 2-input OR gates are combined to produce a
4-input OR gate function as shown in Figure 8-1.

The SN74LVC1G32 device is used to directly control the enable pin of a
fan driver. The fan driver requires only one input signal to be HIGH
before being enabled, and should be disabled in the event that all
signals go LOW. The 4-input OR gate function combines the four
individual overheat signals into a single active-high enable signal.

Conversion note: in the typical application block diagram (Figure 8-1),
Device 1 Overheat and Device 2 Overheat drive one OR gate, Device 3
Overheat and Device 4 Overheat drive a second, and a third OR gate
combines the two outputs and drives the EN input of the Fan Driver.

### Design requirements

#### Power considerations

Verify that the desired supply voltage is within the range specified in
the Electrical Characteristics. Source error: the supply voltage range
is in the Recommended Operating Conditions (p.6), as Power Supply
Recommendations (p.15) says. The supply voltage sets the device
electrical characteristics, as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74LVC1G32 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Verify that the maximum total current through
V~CC~ listed in the Absolute Maximum Ratings is not exceeded.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74LVC1G32 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into the ground connection. Verify that
the maximum total current through GND listed in the Absolute Maximum
Ratings is not exceeded.

The SN74LVC1G32 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, TI recommends adding a
series resistor between the output and capacitor to prevent excessive
current.

The SN74LVC1G32 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting in
the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

#### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum input
voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
the inputs can be connected with a pullup or pulldown resistor if the
input is used sometimes, but not always. A pullup resistor is used for a
default state of HIGH, and a pulldown resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74LVC1G32 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10kΩ
resistor value is often used due to these factors.

The SN74LVC1G32 has CMOS inputs and thus requires fast input transitions
to operate correctly, as defined in the Electrical Characteristics
table. Source error: the input transition rate is in the Recommended
Operating Conditions (p.6), as Standard CMOS Inputs (p.11) says. Slow
input transitions can cause oscillations, additional power consumption,
and reduction in device reliability.

#### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output decreases the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output increases the output voltage as specified by the
V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that can be in opposite states, even for a very short
time period, must never be connected directly together. This can cause
excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Verify that the capacitive load at the output is ≤ 50pF. Low load
   capacitance can be accomplished by providing short, appropriately
   sized traces from the SN74LVC1G32 to the receiving device.
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Never violate the maximum output current from
   the Absolute Maximum Ratings. Most CMOS inputs have a resistive load
   measured in MΩ; much larger than the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; however, the
   power consumption and thermal increase can be calculated using the
   steps provided in the CMOS Power Consumption and Cpd Calculation
   application note.

Conversion note: the application timing diagram (Figure 8-2) shows EN
high while any of Device 1 to Device 4 is high.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating listed in the Recommended Operating Conditions.

Verify that each V~CC~ terminal has a good bypass capacitor to prevent
power disturbance. For the SN74LVC1G32, a 0.1μF bypass capacitor is
recommended. To reject different frequencies of noise, use multiple
bypass capacitors in parallel. Capacitors with values of 0.1μF and 1μF
are commonly used in parallel.

### Layout guidelines

- Bypass capacitor placement
  - Place near the positive supply terminal of the device
  - Provide an electrically short ground return path
  - Use wide traces to minimize impedance
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible
- Signal trace geometry
  - 8mil to 12mil trace width
  - Lengths less than 12cm to minimize transmission line effects
  - Avoid 90° corners for signal traces
  - Use an unbroken ground plane below signal traces
  - Flood fill areas around signal traces with ground
  - Parallel traces must be separated by at least 3x dielectric
    thickness
  - For traces longer than 12cm
    - Use impedance controlled traces
    - Source-terminate using a series damping resistor near the output
    - Avoid branches; buffer each signal that must branch separately

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
SN74LVC1G32DBVR | Reel of 3000 | -40 to 125 | C325, C32F, C32J, C32K, C32R, C32H, C32P, C32S
SN74LVC1G32DBVRE4 | Reel of 3000 | -40 to 125 | C32F, C32J
SN74LVC1G32DBVRG4 | Reel of 3000 | -40 to 125 | C32F, C32J
SN74LVC1G32DBVT | Reel of 250 | -40 to 125 | C325, C32F, C32J, C32K, C32R, C32H, C32P, C32S
SN74LVC1G32DBVTG4 | Reel of 250 | -40 to 125 | C32F
SN74LVC1G32DCK3 | | -40 to 125 | CGF, CGZ | Obsolete
SN74LVC1G32DCKR | Reel of 3000 | -40 to 125 | CG5, CGF, CGJ, CGK, CGR, CGH, CGP, CGS
SN74LVC1G32DCKRE4 | Reel of 3000 | -40 to 125 | CG5 CGS
SN74LVC1G32DCKRG4 | Reel of 3000 | -40 to 125 | CG5 CGS
SN74LVC1G32DCKT | Reel of 250 | -40 to 125 | CG5, CGF, CGJ, CGK, CGR, CGH, CGP, CGS
SN74LVC1G32DCKTG4 | Reel of 250 | -40 to 125 | CG5 CGS
SN74LVC1G32DPWR | Reel of 3000 | -40 to 125 | N4
SN74LVC1G32DRLR | Reel of 4000 | -40 to 125 | CG7, CGR
SN74LVC1G32DRLRG4 | Reel of 4000 | -40 to 125 | CG7, CGR
SN74LVC1G32DRY2 | Reel of 5000 | -40 to 125 | CG
SN74LVC1G32DRY2G4 | Reel of 5000 | -40 to 125 | CG
SN74LVC1G32DRYR | Reel of 5000 | -40 to 125 | CG
SN74LVC1G32DSF2 | Reel of 5000 | -40 to 125 | CG
SN74LVC1G32DSFR | Reel of 5000 | -40 to 125 | CG
SN74LVC1G32YZPR | Reel of 3000 | -40 to 85 | CG, CG7
```

## Conversion notes

- Source pages 1–15 converted. The ordering lines are from the package
  option addendum (pp.20–23). The layout example figures (p.16), support
  pages, revision history (17–19), tape and reel and mechanical pages
  (24–51) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (three feature lines are in
  Description), package information table, logic diagram and functional
  block diagram (one OR gate), layout example figures (Figures 8-3 to
  8-7, generic: trace corners, bypass capacitor placement and a damping
  resistor), references to other documents; the typical application
  block diagram (Figure 8-1) and the application timing diagram (Figure
  8-2) are described in text.
- From images: the overbars of Y = !{!{A} • !{B}} (pp.1, 11); Device 1 to
  Device 4 Overheat, Fan Driver, EN (labels of Figures 8-1 and 8-2,
  pp.13, 15).
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 6-1 and 6-2 are the
  same circuit with different values, joined in one table.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
- The text layer prints "£2 ns", "1 MW", "500 W" and "VD"; the page
  image shows ≤ 2 ns, 1 MΩ, 500 Ω and V~Δ~.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The source's Pin Functions table (Table 4-1) gives pin numbers 1–5 for
  all 5-pin packages; the catalog names the YZP balls (A1, B1, C1, C2,
  A2).
- Source issues:
  - Source error: Power Considerations (p.13) says the supply voltage
    range is "specified in the Electrical Characteristics"; it is in the
    Recommended Operating Conditions (p.6), as Power Supply
    Recommendations (p.15) says.
  - Source error: Input Considerations (p.14) says the fast input
    transitions are "defined in the Electrical Characteristics table";
    Δt/Δv is in the Recommended Operating Conditions (p.6), as Standard
    CMOS Inputs (p.11) says.
  - Source error: the y axis of Figure 5-2 (p.8) is labelled 0, 1, 2, 3,
    4, 5, 5, 7, 8; the second 5 stands where 6 belongs.
  - Source conflict: the design requirements (p.14) refer to "the
    maximum junction temperature, TJ(max) listed in the Absolute Maximum
    Ratings"; the table (p.5) names the row T~J~ and gives it a minimum
    of –65 as well as the maximum of 150.
  - Unconfirmed: in the 3.3 V switching table (p.8) the –40°C to 125°C
    maximum t~pd~ (4 ns) is below the –40°C to 85°C maximum (4.5 ns).
  - Unconfirmed: Ψ~JT~ is 195.6 °C/W for all three of DBV, DCK and DRL
    (p.6).
