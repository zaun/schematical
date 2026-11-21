---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g14",
  "title": "SN74AHCT1G14 single Schmitt-trigger inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct1g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS322R",
    "date": "2024-02",
    "pages": 25
  },
  "devices": ["gate:1G14"],
  "parts": ["SN74AHCT1G14DBVR", "SN74AHCT1G14DCKR", "SN74AHCT1G14DCKT"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G14 single Schmitt-trigger inverter gate

## Description

The SN74AHCT1G14 contains a single inverter gate. The device performs
the Boolean function Y = !{A}.

Operating range 4.5V to 5.5V. Max t~pd~ of 8ns at 5V. Low power
consumption, 1μA max I~CC~. ±8mA output drive at 5V. Inputs are
TTL-voltage compatible. Latch-up performance exceeds 250 mA per JESD 17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No connect
A: Data Input
GND: Ground
Y: Data Output
VCC: Power
```

## Functional description

The SN74AHCT1G14 contains a single inverter gate. The device performs
the Boolean function Y = !{A}.

The device functions as an independent inverter gate, but because of the
Schmitt action, gates may have different input threshold levels for
positive- (V~T+~) and negative-going (V~T–~) signals.

Conversion note: the functional block diagram shows one Schmitt-trigger inverter, input A (pin 2), output Y (pin 4).

## Function tables

```table id=function caption="Function table"
| Inputs [1] | Output [2] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT1G14"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | T~A~ = 25°C | 4.5 V | 0.9 | | 2 | V |
| ^ | ^ | | T~A~ = –40°C to 85°C | ^ | 0.9 | | | V |
| ^ | ^ | | T~A~ = –40°C to 125°C | ^ | 0.9 | | 2 | V |
| ^ | ^ | | T~A~ = 25°C | 5.5 V | 1.1 | | 2 | V |
| ^ | ^ | | T~A~ = –40°C to 85°C | ^ | 1.1 | | | V |
| ^ | ^ | | T~A~ = –40°C to 125°C | ^ | 1.1 | | 2 | V |
| V~T–~ | Negative-going input threshold voltage | | T~A~ = 25°C | 4.5 V | 0.5 | | 1.6 | V |
| ^ | ^ | | T~A~ = –40°C to 85°C | ^ | 0.5 | | | V |
| ^ | ^ | | T~A~ = –40°C to 125°C | ^ | 0.5 | | 1.6 | V |
| ^ | ^ | | T~A~ = 25°C | 5.5 V | 0.6 | | 1.5 | V |
| ^ | ^ | | T~A~ = –40°C to 85°C | ^ | 0.6 | | | V |
| ^ | ^ | | T~A~ = –40°C to 125°C | ^ | 0.6 | | 1.5 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T-~) | | T~A~ = 25°C | 4.5 V | 0.4 | | 1.4 | V |
| ^ | ^ | | T~A~ = –40°C to 85°C | ^ | 0.4 | | | V |
| ^ | ^ | | T~A~ = –40°C to 125°C | ^ | 0.4 | | 1.4 | V |
| ^ | ^ | | T~A~ = 25°C | 5.5 V | 0.5 | | 1.6 | V |
| ^ | ^ | | T~A~ = –40°C to 85°C | ^ | 0.4 | | | V |
| ^ | ^ | | T~A~ = –40°C to 125°C | ^ | 0.5 | | 1.6 | V |
| V~OH~ | High level output voltage | I~OH~ = –50 mA [2] | T~A~ = 25°C | 4.5 V | 4.4 | 4.5 | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | 4.4 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | 4.4 | | | V |
| ^ | ^ | I~OH~ = –8 mA | T~A~ = 25°C | ^ | 3.94 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | 3.88 | | | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | 3.7 | | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 mA [2] | T~A~ = 25°C | 4.5 V | | | 0.1 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | | | 0.1 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | T~A~ = 25°C | ^ | | | 0.36 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | | | 0.44 | V |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | T~A~ = 25°C | 0 V to 5.5 V | | | ±0.1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | | | ±1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | T~A~ = 25°C | 5.5 V | | | 1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | | | 10 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | | | 10 | µA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | T~A~ = 25°C | 5 V | | 2 | 10 | pF |
| ^ | ^ | ^ | T~A~ = –40°C to 85°C | ^ | | | 10 | pF |
| ^ | ^ | ^ | T~A~ = –40°C to 125°C | ^ | | | 10 | pF |

[1] Recommended T~A~ = –40°C to 125°C
[2] Unconfirmed: the source prints I~OH~ = –50 mA and I~OL~ = 50 mA for the rows with the higher V~OH~ and the lower V~OL~; its recommended operating conditions give –8 mA and 8 mA as the maximum output currents.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | Test conditions | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | T~A~ = 25°C | | 4 | 7 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1 | | 8 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1 | | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | T~A~ = 25°C | | 4 | 7 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1 | | 8 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1 | | 9 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | T~A~ = 25°C | | 5.5 | 8 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1 | | 9 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1 | | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | T~A~ = 25°C | | 5.5 | 8 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 85°C | 1 | | 9 | ns |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | 1 | | 10 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```

Conversion note: in the source figure (Figure 6-1) the input swings from 0 V to 3 V; the figure also shows the load circuit for 3-state and open-drain outputs with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms with their note B, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

In this application, three 2-input AND gates are combined to produce a
4-input AND gate function as shown in Typical Application Block Diagram.
The fourth gate can be used for another application in the system, or
the inputs can be grounded and the channel left unused.

The SN74AHCT1G14 is used to directly control the !{RESET} pin of a motor
controller. The controller requires four input signals to all be HIGH
before being enabled, and should be disabled in the event that any one
signal goes LOW. The 4-input AND gate function combines the four
individual reset signals into a single active-low reset signal.

Source conflict: the Application Information (p.9) and Figure 8-1 describe three 2-input AND gates of a four-gate device; the Description (p.1), the pin table (p.3) and the function table (p.8) give a single inverter gate. Kept as printed.

Conversion note: the typical application block diagram (Figure 8-1) shows three 2-input AND gates that combine the signals !{OC}, PG, ON/!{OFF} and !{OT}, from an over current detection block, a power supply, an on/off switch and an over temp detection block, into the !{RESET} input of a motor controller.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHCT1G14 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74AHCT1G14 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF.

The SN74AHCT1G14 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin.

Total power consumption can be calculated using the information provided
in the CMOS Power Consumption and Cpd Calculation application note.

Thermal increase can be calculated using the information provided in the
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.

Source conflict: the caution (p.10) names a maximum junction temperature listed in the Absolute Maximum Ratings; that table (p.4) has no junction temperature row.

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Do not exceed the maximum input voltage range
found in the Absolute Maximum Ratings.

Unconfirmed: the first sentence is printed so, without the names of the levels to be crossed (p.10).

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT1G14 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74AHCT1G14 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows the signals !{OC}, PG, ON/!{OFF} and !{OT} and the !{RESET} output of the AND function of Figure 8-1.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example.

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

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

Source conflict: the example layout (Figure 8-3, p.12) labels pin 1 A and pin 2 B; the pin table (p.3) gives pin 1 as NC and pin 2 as A, and the device has no B input.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74AHCT1G14DBVR | Reel of 3000 | -40 to 125 | 36DH, 3CAF, B143, B14G, B14J, B14L, B14S
SN74AHCT1G14DCKR | Reel of 3000 | -40 to 125 | 1QN, BF3, BFG, BFJ, BFL, BFS
SN74AHCT1G14DCKT | | -40 to 125 | BF3, BFG, BFJ, BFS | Obsolete
```

## Conversion notes

- Source pages 1–12 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (p.15; it ends on p.16). The support and revision history
  page (13), the mechanical information page (14), tape and reel and
  mechanical pages (17–25) are left out; the generated Package section
  replaces them.
- Dropped: applications list (the features are kept as sentences in the
  Description), package information table, the two "Refer to the Feature
  Description section" sentences of the input and output considerations
  (the source has no Feature Description section); the logic diagram
  (p.1), the functional block diagram (p.8), the typical application
  block diagram (Figure 8-1), the application timing diagram
  (Figure 8-2) and the example layout (Figure 8-3) are described in
  text.
- The electrical characteristics table of the source prints the
  temperature ranges in an unnamed column under TEST CONDITIONS; they
  are the second Test conditions column here. The table continues on
  p.6, where its note is printed. The unit, printed once for each
  parameter, is repeated on every row.
- The addendum also lists 74AHCT1G14DBVRE4, 74AHCT1G14DBVRG4,
  74AHCT1G14DBVTE4, 74AHCT1G14DBVTG4.A, 74AHCT1G14DCKTE4,
  74AHCT1G14DCKTG4, SN74AHCT1G14DCK3 and SN74AHCT1G14DCKRG4.A, which the
  catalog doesn't carry.
- Figure 6-1 (p.7) is a picture: the load circuit and the propagation
  delay waveforms were compared with the page image. The load circuit
  prints no value for C~L~, which is 15 pF or 50 pF in the switching
  characteristics.
- From images: the overbar of Y = !{A} (p.1 and Overview, p.8); the
  overbars of !{RESET}, !{OC}, !{OFF} and !{OT} (p.9 and Figure 8-2,
  p.11); 1.5 V, 0 V, 3 V and 50% V~CC~ (Figure 6-1, p.7); 0.1 µF
  (Figure 8-3, p.12; the text layer prints "0.1 F").
- Source issues:
  - Unconfirmed: I~OH~ = –50 mA and I~OL~ = 50 mA in the V~OH~ and
    V~OL~ rows (electrical characteristics note 2).
  - Source conflict: the application text and Figure 8-1 describe AND
    gates of a four-gate device (Application information note).
  - Source conflict: a maximum junction temperature is named but not
    listed in the Absolute Maximum Ratings (Power considerations note).
  - Unconfirmed: "Input signals must cross to be considered a logic
    LOW" is printed without the level names (Input considerations
    note).
  - Source conflict: the example layout labels pins 1 and 2 as A and B
    (Layout guidelines note).
