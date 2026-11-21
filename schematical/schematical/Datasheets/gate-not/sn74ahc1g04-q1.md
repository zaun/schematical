---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g04-q1",
  "title": "SN74AHC1G04-Q1 automotive single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc1g04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS540D",
    "date": "2024-01",
    "pages": 24
  },
  "devices": ["gate:1G04"],
  "parts": ["1A1G04QDBVRG4Q1", "CAHC1G04QDCKRG4Q1", "SN74AHC1G04QDBVRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G04-Q1 automotive single inverter gate

## Description

The SN74AHC1G04-Q1 contains one inverter gate. The device performs the
Boolean function Y = !{A}.

Qualified for automotive applications. Operating range 2 V to 5.5 V.
± 8-mA output drive at 5 V. Latch-up performance exceeds 250 mA Per
JESD 17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No Connection
A: Input A
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```

## Functional description

The SN74AHC1G04 device contains one inverter gate. The device performs
the Boolean function Y = !{A}. This single gate inverter has
Schmitt-Trigger action on its input, allowing for slower rise and fall
times and some noise rejection. This is not a true Schmitt-Trigger, so
there is a limit on rise and fall times.

Conversion note: the logic diagram (Figure 7-1) shows one inverter, input A, output Y.

## Function tables

```table id=function caption="Function table"
| Input [1] | Output [2] |
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
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through each V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under [Section 5.3](#recommended-operating-conditions) is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-Body Model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±1500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G04-Q1"
| Symbol | Thermal metric [1] | DBV 5 pins | DCK 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA [1] | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | pF |

[1] Source error: printed as I~OH~ = 50 µA; the rows give V~OL~, and the recommended operating conditions give 50 µA as I~OL~, so the condition is I~OL~.
```

Conversion note: the source prints no heading over the second Min and Max columns of this table and of the switching characteristics tables.

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.5 | 10.6 | 1 | 14.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.5 | 10.6 | 1 | 14.5 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.3 | 7.5 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.3 | 7.5 | 1 | 10 | ns |
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
VM: 50% VCC
```

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

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

The SN74AHC1G04-Q1 is used to directly control the !{RESET} pin of a
motor controller. The controller requires four input signals to all be
HIGH before being enabled, and should be disabled in the event that any
one signal goes LOW. The 4-input AND gate function combines the four
individual reset signals into a single active-low reset signal.

Unconfirmed: this application text and Figure 8-1 describe 2-input AND gates and a fourth gate, while the SN74AHC1G04-Q1 holds one inverter; kept as printed.

Conversion note: in the typical application block diagram (Figure 8-1) one AND gate combines !{OC} (over current detection) with PG (power supply), a second combines ON/!{OFF} (on/off switch) with !{OT} (over temp detection), and a third combines the two results into the !{RESET} input of a motor controller.

### Design requirements

#### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the SN74AHC1G04-Q1
plus the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings.

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SN74AHC1G04-Q1 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings.

The SN74AHC1G04-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF.

The SN74AHC1G04-Q1 can drive a load with total resistance described by
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

Source conflict: this caution (p.9) says T~J(max)~ is listed in the Absolute Maximum Ratings; that table (p.4) lists no junction temperature.

#### Input considerations

Input signals must cross V~IL(max)~ V~t-(min)~ to be considered a logic
LOW, and V~IH(min)~ V~t+(max)~ to be considered a logic HIGH. Do not
exceed the maximum input voltage range found in the Absolute Maximum
Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHC1G04-Q1 (as specified in the Electrical Characteristics), and
the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

The SN74AHC1G04-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability.

The SN74AHC1G04-Q1 has no input signal transition rate requirements
because it has Schmitt-trigger inputs.

Source conflict: the paragraph above (p.9) requires fast input transitions for CMOS inputs, and this paragraph says there are no input signal transition rate requirements; the Overview (p.7) says the input is not a true Schmitt-Trigger and there is a limit on rise and fall times. All are kept as printed.

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value will
provide the peak-to-peak limit.

Source conflict: the Electrical Characteristics table (p.5) has no ΔV~T~ row; the source gives no hysteresis value.

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics.

Source conflict: the source has no Typical Characteristics section and no Feature Description section, which this section and the next refer to.

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

Open-drain outputs can be connected together directly to produce a
wired-AND configuration or for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

Refer to the Feature Description section for additional information
regarding the outputs for this device.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74AHC1G04-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows the !{OC}, PG, ON/!{OFF}, !{OT} and !{RESET} signals against time.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Load Circuit And Voltage Waveforms table.

Source error: Load Circuit And Voltage Waveforms (p.6) is a figure and gives no supply voltage rating; the supply voltage MIN and MAX are in the Recommended Operating Conditions (p.4).

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended. If
there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

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

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a 0.1 μF bypass capacitor placed close to the device, and no 90° corners for signal lines.

Source conflict: Figure 8-3 (p.11), titled Example Layout for the SN74AHC1G04-Q1, draws a 14-pin package; the SN74AHC1G04-Q1 has 5 pins (p.3).

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
1A1G04QDBVRG4Q1 | Reel of 3000 | -40 to 125 | A04S
CAHC1G04QDCKRG4Q1 | Reel of 3000 | -40 to 125 | ACS, ACU
SN74AHC1G04QDBVRQ1 | Reel of 3000 | -40 to 125 | 38DH, A04S
```

## Conversion notes

- Source pages 1–11 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.14–15). The support and revision history page (12), the
  mechanical information page (13), tape and reel and mechanical pages
  (16–24) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the
  simplified schematic and the logic diagram (one inverter); the typical
  application block diagram (Figure 8-1), the application timing diagram
  (Figure 8-2) and the example layout (Figure 8-3) are described in
  text.
- Section 5.3, named in absolute maximum ratings note 1, is the source's
  Recommended Operating Conditions.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The CAUTION box of the source (p.9) is kept as a paragraph at the end
  of Power considerations.
- The addendum also lists SN74AHC1G04QDCKRQ1, which the catalog doesn't
  carry.
- Figure 6-1 (p.6) is a picture with no text layer; the load circuit and
  the propagation delay waveforms were compared with the page image.
- From images: the overbar of Y = !{A} (p.1 and Overview, p.7); the
  overbar of !{RESET} (§ 8.1, p.8); the overbars of !{OC}, ON/!{OFF},
  !{OT} and !{RESET} and the gate arrangement (Figure 8-1, p.8, and
  Figure 8-2, p.10); 50% V~CC~ (V~M~, Figure 6-1, p.6).
- Source issues:
  - Source error: the first V~OL~ rows are printed with I~OH~ = 50 µA;
    corrected to I~OL~ (electrical characteristics note 1).
  - Source error: the power supply recommendations name the Load Circuit
    And Voltage Waveforms table for the supply voltage rating (Power
    supply recommendations note).
  - Source conflict: T~J(max)~ said to be listed in the Absolute Maximum
    Ratings, which list no junction temperature (Power considerations
    note).
  - Source conflict: fast input transitions required vs no input
    transition rate requirements vs a limit on rise and fall times
    (Input considerations note).
  - Source conflict: ΔV~T(min)~ referenced but not in the Electrical
    Characteristics table (Input considerations note).
  - Source conflict: Typical Characteristics and Feature Description
    sections referenced but not in the source (Input considerations
    note).
  - Source conflict: Figure 8-3 draws a 14-pin package for this 5-pin
    part (Layout guidelines note).
  - Unconfirmed: the application text (§ 8.1) and Figure 8-1 describe
    2-input AND gates and "the fourth gate", while the SN74AHC1G04-Q1
    holds one inverter; kept as printed.
  - Unconfirmed: the Overview (p.7) names the device SN74AHC1G04,
    without the -Q1 suffix; kept as printed.
  - Unconfirmed: "V~IL(max)~ V~t-(min)~" and "V~IH(min)~ V~t+(max)~"
    (§ 8.2.1.2) are printed as two symbols side by side; kept as
    printed.
