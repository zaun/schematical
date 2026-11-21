---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g32-q1",
  "title": "SN74AHC1G32-Q1 automotive single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahc1g32-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS544E",
    "date": "2024-10",
    "pages": 25
  },
  "devices": ["gate:1G32"],
  "parts": ["SN74AHC1G32QDBVRQ1", "SN74AHC1G32QDCKRQ1", "SN74AHC1G32TDBVRQ1", "SN74AHC1G32TDCKRQ1", "SN74AHC1G32WDTXRQ1"],
  "packages": {"p5": "DBV, DCK, DTX"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G32-Q1 automotive single 2-input positive-OR gate

## Description

The SN74AHC1G32-Q1 is a single 2-input positive-OR gate. The device
performs the Boolean function Y = A + B in positive logic.

Qualified for automotive applications. Operating range of 2V to 5.5V.
Maximum t~pd~ of 6.5ns at 5V. Low power consumption, 10μA maximum
I~CC~. ±8mA output drive at 5V. Latch-up performance exceeds 250mA per
JESD 17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```

## Functional description

This device contains a 2-input OR Gate. The gate performs the Boolean
function Y = A + B in positive logic. The output level is referenced to
the supply voltage (V~CC~) and supports 2.0-V, 3.0-V, and 5-V CMOS
levels.

Conversion note: the logic diagram (Figure 7-1) shows one 2-input OR gate.

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I).

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in Implications of Slow or
Floating CMOS Inputs.

Do not leave standard CMOS inputs floating at any time during operation.
Unused inputs must be terminated at V~CC~ or GND. If a system will not
be actively driving an input at all times, then a pull-up or pull-down
resistor can be added to provide a valid input voltage during these
times. The resistor value will depend on multiple factors; a 10-kΩ
resistor, however, is recommended and will typically meet all
requirements.

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

Unused push-pull CMOS outputs must be left disconnected.

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 7-2.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

Conversion note: Figure 7-2 (electrical placement of clamping diodes for each input and output) shows a diode from GND to the input, and diodes from GND to the output and from the output to V~CC~.

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |

[1] H = high voltage level, L = low voltage level, X = do not care
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature (unless otherwise noted)"
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

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±1500 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature (unless otherwise noted)"
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
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature (SN74AHC1G32-Q1T) | | -40 | 105 | °C |
| T~A~ | Operating free-air temperature (SN74AHC1G32-Q1Q) | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND for proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G32-Q1: thermal metric [1]"
| Symbol | Parameter | DBV 5 pins | DCK 5 pins | DTX 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 184.7 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 μA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = −4 mA | 3 V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = −8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 μA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | pF |
```

Conversion note: the source prints no heading over the second Min and Max columns of this table and of the switching characteristics tables.

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 14 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8 | 11.4 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8 | 11.4 | 1 | 13 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | | | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.3 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.3 | 7.5 | 1 | 8.5 | ns |
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

In this application, three 2-input OR gates are combined to produce a
4-input OR gate function as shown in Typical Application Block Diagram.
The fourth gate can be used for another application in the system, or
the inputs can be grounded and the channel left unused.

The SN74AHC1G32-Q1 device is used to directly control the enable pin of
a fan driver. The fan driver requires only one input signal to be HIGH
before being enabled, and should be disabled in the event that all
signals go LOW. The 4-input OR gate function combines the four
individual overheat signals into a single active-high enable signal.

Temperature sensors can often be spread throughout a system rather than
being in a centralized location. This would mean longer length traces or
wires to pass signals through leading to slower edge transitions. This
makes the SN74AHC1G32-Q1 useful for combining the incoming signals.

Conversion note: the typical application block diagram (Figure 8-1) shows two OR gates, one fed by the Device 1 and Device 2 Overheat signals and one by the Device 3 and Device 4 Overheat signals, and a third OR gate that combines their outputs into the EN input of the fan driver.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SN74AHC1G32-Q1 to one or more of the receiving devices.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation.

Conversion note: the application timing diagram (Figure 8-2) shows EN high whenever any of the Device 1 to Device 4 signals is high.

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

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a bypass capacitor placed close to the device, and no 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=X2SON-5-DTX
```

## Ordering information

```ordering
SN74AHC1G32QDBVRQ1 | Reel of 3000 | -40 to 125 | 39DH
SN74AHC1G32QDCKRQ1 | Reel of 3000 | -40 to 125 | 1OF
SN74AHC1G32TDBVRQ1 | Reel of 3000 | -40 to 105 | 39GH, A32U
SN74AHC1G32TDCKRQ1 | Reel of 3000 | -40 to 105 | AGU
SN74AHC1G32WDTXRQ1 | Reel of 3000 | -40 to 125 | 3
```

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.12–13). The support, revision history and
  mechanical information page (11), tape and reel and mechanical pages
  (14–25) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description; the AEC-Q100 temperature grade and ESD
  classification lines are left out), package information table, the
  logic diagram (one OR gate); the clamp diode figure (Figure 7-2), the
  typical application block diagram (Figure 8-1), the application timing
  diagram (Figure 8-2) and the example layout (Figure 8-3) are described
  in text.
- The Design Requirements heading (§ 8.2.1) of the source has no text.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- From images: 50% V~CC~ (V~M~, Figure 6-1, p.6); the gate arrangement of Figure 8-1 (p.9), the diode
  placement of Figure 7-2 (p.8) and the EN waveform of Figure 8-2
  (p.10).
- Source issues:
  - Unconfirmed: the application text (§ 8.1) speaks of "the fourth
    gate" and a channel left unused, while the SN74AHC1G32-Q1 holds one
    gate; kept as printed.
