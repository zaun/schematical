---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct00",
  "title": "SNx4AHCT00 quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74ahct00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS229L",
    "date": "2023-05",
    "pages": 42
  },
  "devices": ["gate:74_00"],
  "parts": ["5962-9682301QCA", "5962-9682301QDA", "SN74AHCT00BQAR", "SN74AHCT00D", "SN74AHCT00DBR", "SN74AHCT00DGVR", "SN74AHCT00DR", "SN74AHCT00DRE4", "SN74AHCT00N", "SN74AHCT00NE4", "SN74AHCT00NSR", "SN74AHCT00PW", "SN74AHCT00PWR", "SN74AHCT00RGYR", "SNJ54AHCT00J", "SNJ54AHCT00W", "5962-9682301Q2A", "SNJ54AHCT00FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT00 quadruple 2-input positive-NAND gates

## Description

The ’AHCT00 devices perform the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic. [[1::1]]

Operating range of 4.5 V to 5.5 V. Low power consumption, 10-µA maximum
I~CC~. ±8-mA output drive at 5 V. Inputs are TTL-voltage compatible.
Latch-up performance exceeds 250 mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2B: Channel 2, Input B
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
3Y: Channel 3, Output Y
4A: Channel 4, Input A
4B: Channel 4, Input B
4Y: Channel 4, Output Y
GND: Ground
NC: No Connection
VCC: Positive Supply
```
[[4::Table 5-2]]

Source error: both pin tables of the source (pp.3–4) print the types and descriptions of channels 3 and 4 one row off (3A and 4A as "O, Output Y", 3B and 4B as "I, Input A", 3Y and 4Y as "I, Input B"); the pin drawings on the same pages and the logic diagram have A and B as the inputs and Y as the output, as given here.

## Functional description

This device contains four independent 2-input NAND Gates. Each gate
performs the Boolean function Y = !{A ● B} in positive logic. [[9::1]]

Conversion note: the functional block diagram shows one gate, with inputs xA and xB each buffered into a NAND gate whose output is buffered to xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. [[9::2]]

Unused push-pull CMOS outputs should be left disconnected. [[9::3]]

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold. [[9::4]]

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
Electrical Characteristics. The worst case resistance is calculated with
the maximum input voltage, given in the Absolute Maximum Ratings, and
the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I). [[9::5]]

TTL-compatible CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in the Implications of Slow or
Floating CMOS Inputs application report. [[9::6]]

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a system
will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10-kΩ resistor is recommended and will typically meet all
requirements. [[9::7]]

### Clamp diode structure

As Figure 8-1 shows, the outputs to this device have both positive and
negative clamping diodes, and the inputs to this device have negative
clamping diodes only. [[9::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[9::9]]

Conversion note: Figure 8-1 (electrical placement of clamping diodes for each input and output) draws the output clamps to V~CC~ (+I~OK~) and GND (-I~OK~) and the input clamp to GND (-I~IK~).

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[10::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < -0.5 V | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous output current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | SN54AHCT00 | < | SN74AHCT00 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[5::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT00 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB (SSOP) 14 pins | DGV (TVSOP) 14 pins | N (PDIP 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | BQA (WQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 96 | 127 | 80 | 76 | 113 | 47 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
```
[[6::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT00 | < | SN74AHCT00 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[6::table #2]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C (see [1])"
| Symbol | Parameter | SN74AHCT00 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.5 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #3]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 10.5 | pF |
```
[[7::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHCT00 | < | SN74AHCT00 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | 1 | 9 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[8::1]]

Conversion note: in Figure 7-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[8::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[8::Figure 7-1]]

Conversion note: the source figure (Figure 7-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

In this application, two 2-input NAND gates are used to create an
active-low SR latch as shown in Figure 9-1. The two additional gates can
be used for a second SR latch, or the inputs can be grounded and both
channels left unused. [[11::2]]

The AHCT00 is used to drive the tamper indicator LED and provide one bit
of data to the system controller. When the tamper switch outputs LOW,
the output Q becomes HIGH. This output remains HIGH until the system
controller addresses the event and sends a LOW signal to the !{R} input
which returns the Q output back to LOW. [[11::3]]

The inputs of this active-low SR latch can often be driven by open-drain
outputs which can produce slow input transition rates when they
transition from LOW to Hi-Z. This makes the AHCT00 ideal for the
application because it has Schmitt-trigger inputs that do not have input
transition rate requirements. [[11::4]]

Source conflict: this paragraph (p.11) says the AHCT00 has Schmitt-trigger inputs without input transition rate requirements, while the Recommended Operating Conditions (p.5) limit the input transition rate and the Feature Description (p.9) and Input Considerations (p.12) say the inputs require fast transitions.

Conversion note: the typical application block diagram (Figure 9-1) shows two cross-coupled NAND gates; the tamper switch drives the !{S} input, the system controller drives the !{R} input, which has a pull-up resistor R~1~, and the Q output goes to the system controller and through R~2~ to the tamper indicator LED.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics as described in the Electrical
Characteristics section. [[11::6]]

The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs of the AHCT00 plus the
maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Be sure to not exceed the maximum total current
through V~CC~ listed in the Absolute Maximum Ratings. [[11::7]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the AHCT00 plus the maximum supply current,
I~CC~, listed in the Electrical Characteristics, and any transient
current required for switching. The logic device can only sink as much
current that can be sunk into its ground connection. Be sure to not
exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[11::8]]

The AHCT00 can drive a load with a total capacitance less than or equal
to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF. [[12::1]]

The AHCT00 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage at
the V~CC~ pin. [[12::2]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation. [[12::3]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices. [[12::4]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[12::5]]

Source conflict: this caution (p.12) says T~J(max)~ is listed in the Absolute Maximum Ratings, while that table (p.5) has no junction temperature row.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings. [[12::6]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the AHCT00 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10-kΩ resistor
value is often used due to these factors. [[12::7]]

The AHCT00 has CMOS inputs and thus requires fast input transitions to
operate correctly, as defined in the Recommended Operating Conditions
table. Slow input transitions can cause oscillations, additional power
consumption, and reduction in device reliability. [[12::8]]

Refer to the Feature Description section for additional information
regarding the inputs for this device. [[12::9]]

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[12::10]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This can
cause excessive current and damage to the device. [[12::11]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[12::12]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[12::13]]

Refer to the Feature Description section for additional information
regarding the outputs for this device. [[12::14]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[12::15]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   AHCT00 to one or more of the receiving devices. [[13::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. This will ensure that the maximum output
   current from the Absolute Maximum Ratings is not violated. Most CMOS
   inputs have a resistive load measured in MΩ; much larger than the
   minimum calculated previously. [[13::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[13::1]]

Conversion note: the application timing diagram (Figure 9-2) shows the !{R} and !{S} inputs and the Q output of the latch; Q is undefined at first, goes HIGH when !{S} goes LOW and goes LOW when !{R} goes LOW.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in Layout Example. [[13::2]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices inputs must
never be left floating. In many cases, functions or parts of functions
of digital logic devices are unused; for example, when only two inputs
of a triple-input AND gate are used or only 3 of the 4 buffer gates are
used. Such unused input pins must not be left unconnected because the
undefined voltages at the outside connections result in undefined
operational states. All unused inputs of digital logic devices must be
connected to a logic high or logic low voltage, as defined by the input
voltage specifications, to prevent them from floating. The logic level
that must be applied to any particular unused input depends on the
function of the device. Generally, the inputs are tied to GND or V~CC~,
whichever makes more sense for the logic function or is more convenient. [[13::3]]

Conversion note: the example layout (Figure 9-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 μF bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=PDIP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9682301Q2A | Tube of 55 | -55 to 125 | 5962-9682301Q2A SNJ54AHCT00FK
5962-9682301QCA | Tube of 25 | -55 to 125 | 5962-9682301QCA SNJ54AHCT00J
5962-9682301QDA | Tube of 25 | -55 to 125 | 5962-9682301QDA SNJ54AHCT00W
SN74AHCT00BQAR | Reel of 3000 | -40 to 125 | AHCT00
SN74AHCT00D | | -40 to 85 | AHCT00 | Obsolete
SN74AHCT00DBR | Reel of 2000 | -40 to 85 | HB00
SN74AHCT00DGVR | Reel of 2000 | -40 to 85 | HB00
SN74AHCT00DR | Reel of 2500 | -40 to 85 | AHCT00
SN74AHCT00DRE4 | Reel of 2500 | -40 to 85 | AHCT00
SN74AHCT00N | Tube of 25 | -40 to 85 | SN74AHCT00N
SN74AHCT00NE4 | Tube of 25 | -40 to 85 | SN74AHCT00N
SN74AHCT00NSR | Reel of 2000 | -40 to 85 | AHCT00
SN74AHCT00PW | | -40 to 85 | HB00 | Obsolete
SN74AHCT00PWR | Reel of 2000 | -40 to 85 | HB00
SN74AHCT00RGYR | Reel of 3000 | -40 to 85 | HB00
SNJ54AHCT00FK | Tube of 55 | -55 to 125 | 5962-9682301Q2A SNJ54AHCT00FK
SNJ54AHCT00J | Tube of 25 | -55 to 125 | 5962-9682301QCA SNJ54AHCT00J
SNJ54AHCT00W | Tube of 25 | -55 to 125 | 5962-9682301QDA SNJ54AHCT00W
```
[[15::table #1]]

## Conversion notes

- Source pages 1–13 converted; p.2 holds only the table of contents and
  the revision history, which are dropped. The ordering lines are from
  the package option addendum (pp.15–17). The support and mechanical
  information page (14) and the tape and reel and mechanical pages
  (18–42) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the logic
  diagram (p.1); the functional block diagram, the clamp diode figure
  (Figure 8-1), the typical application block diagram (Figure 9-1), the
  application timing diagram (Figure 9-2) and the example layout
  (Figure 9-3) are described in text.
- Note B of Figure 7-1 belongs to the enable and disable waveforms,
  which are not drawn, and is left out.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- Figure 7-1 (p.8) is a picture; only its notes and S1 table are in the
  text layer.
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1) and
  of Y = !{A ● B} (Overview, p.9); the overbar of !{R} in the
  application text and the overbars of !{R} and !{S}, R~1~, R~2~ and the
  latch arrangement in Figure 9-1 (p.11); 0 V, 3 V, 1.5 V and 50% V~CC~
  (Figure 7-1, p.8); the drawings of the functional block diagram (p.9),
  Figure 8-1 (p.10), Figure 9-2 and Figure 9-3 (p.13).
- Source issues:
  - Source error: the Pin Functions tables (pp.3–4) print the types and
    descriptions of 3A, 3B, 3Y, 4A, 4B and 4Y one row off; corrected
    from the pin drawings (Pin functions note).
  - Source error: Figure 5-3 (p.4) is captioned "FK Package, 14-Pin
    LCCC"; the drawing, Table 5-2 and the package information table
    (p.1) have 20 pins.
  - Source error: the Thermal Information header (p.6) prints "N (PDIP"
    without the closing parenthesis; kept as printed.
  - Source conflict: the Application Information (p.11) speaks of
    Schmitt-trigger inputs without input transition rate requirements;
    the Recommended Operating Conditions (p.5), the Feature Description
    (p.9) and Input Considerations (p.12) require fast input
    transitions.
  - Source conflict: the caution on p.12 refers to T~J(max)~ listed in
    the Absolute Maximum Ratings, which have no such row (p.5).
  - Source conflict: the Recommended Operating Conditions (p.5) give the
    SN74AHCT00 an operating temperature of -40 °C to 85 °C; the package
    option addendum (p.15) gives -40 to 125 for SN74AHCT00BQAR.
  - Source conflict: the package information table (p.1) calls the RGY
    package QFN, the Thermal Information (p.6) and the package option
    addendum (p.15) VQFN.
  - Source conflict: the Features (p.1) give a 10-µA maximum I~CC~, the
    electrical characteristics table (p.6) 2 µA and 20 µA.
  - Unconfirmed: the Thermal Information (p.6) has no column for the J,
    W and FK packages.
