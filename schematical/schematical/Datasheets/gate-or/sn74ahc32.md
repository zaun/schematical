---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc32",
  "title": "SNx4AHC32 quadruple 2-input positive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahc32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS247L",
    "date": "2024-02",
    "pages": 44
  },
  "devices": ["gate:74_32"],
  "parts": ["5962-9682501QCA", "5962-9682501QDA", "SN74AHC32BQAR", "SN74AHC32D", "SN74AHC32DBR", "SN74AHC32DGVR", "SN74AHC32DR", "SN74AHC32DRG4", "SN74AHC32N", "SN74AHC32NSR", "SN74AHC32PW", "SN74AHC32PWR", "SN74AHC32RGYR", "SNJ54AHC32J", "SNJ54AHC32W", "5962-9682501Q2A", "SNJ54AHC32FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHC32 quadruple 2-input positive-OR gates

## Description

The SNx4AHC32 devices are quadruple 2-input positive-OR gates. These
devices perform the Boolean function Y = A + B in positive logic. [[1::1]]

Source error: the Description (p.1) prints a first form, Y = !{A × B}
(a NAND), while the function table (Table 7-1) gives Y = L only when A
and B are both L; the first form is kept out.

Operating range 2V to 5.5V V~CC~. Low power consumption, 10µA maximum
I~CC~. ±8mA output drive at 5V. Latch-up performance exceeds 250mA per
JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
1Y: 1Y Output
2A: 2A Input
2B: 2B Input
2Y: 2Y Output
3A: 3A Input
3B: 3B Input
3Y: 3Y Output
4A: 4A Input
4B: 4B Input
4Y: 4Y Output
GND: Ground Pin
NC: No Connection
VCC: Power Pin
```
[[3::Table 4-1]]

Conversion note: the pin table also lists a Thermal Pad, RGY and BQA Package Only.

## Functional description

The SNx4AHC32 contains four independent 2-input OR Gates. Each gate
performs the Boolean function Y = A + B in positive logic. [[9::1]]

Conversion note: the functional block diagram shows one 2-input OR gate with inputs xA and xB and output xY.

### Standard CMOS inputs

This device includes standard CMOS inputs. Standard CMOS inputs are high
impedance and are typically modeled as a resistor in parallel with the
input capacitance given in the Electrical Characteristics. The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using Ohm's law (R = V ÷ I). [[10::1]]

Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition time or rate in
the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations. More details can be found in Implications of Slow or
Floating CMOS Inputs. [[10::2]]

Do not leave standard CMOS inputs floating at any time during operation.
Unused inputs must be terminated at V~CC~ or GND. If a system will not
be actively driving an input at all times, then a pull-up or pull-down
resistor can be added to provide a valid input voltage during these
times. The resistor value will depend on multiple factors; a 10kΩ
resistor, however, is recommended and will typically meet all
requirements. [[10::3]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due to
overcurrent. The electrical and thermal limits defined in the Absolute
Maximum Ratings must be followed at all times. [[10::4]]

Unused push-pull CMOS outputs should be left disconnected. [[10::5]]

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
as shown in Figure 7-1. [[10::6]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[10::7]]

Conversion note: Figure 7-1 (electrical placement of clamping diodes for each input and output) shows a diode from GND to the input, and diodes from GND to the output and from the output to V~CC~.

### Device functional modes

Table 7-1 lists the functional modes of the SNx4AHC32. [[10::9]]

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | H | H |
| H | L | H |
| L | L | L |

[1] H = high voltage level, L = low voltage level, X = do not care, Z = high impedance
```
[[10::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | SN54AHC32 | < | SN74AHC32 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | 1.5 | | V |
| ^ | ^ | V~CC~ = 3V | 2.1 | | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | 3.85 | | V |
| V~IL~ | Low-level Input voltage | V~CC~ = 2 V | | 0.5 | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V [1] | | -50 | | -50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V [1] | | 50 | | 50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | | 8 | mA |
| Δt/Δv | Input Transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 125 | °C |

[1] Unconfirmed: the source prints the unit mA for the V~CC~ = 2 V rows; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = –50 µA and I~OL~ = 50 µA.
```
[[5::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SNx4AHC32: thermal metric [1]"
| Symbol | Parameter | D [2] 14 | DB [2] 14 | DGV [2] 14 | N [2] 14 | NS [2] 14 | PW [2] 14 | RGY [3] 14 | BQA 14 | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ [4] | Junction-to-ambient thermal resistance | 124.6 | 96 | 127 | 80 | 76 | 147.7 | 47 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
[2] The package thermal impedance is calculated in accordance with JESD 51-7.
[3] The package thermal impedance is calculated in accordance with JESD 51-5
[4] Conversion note: the text layer prints this symbol as "R θJA", in two pieces.
```
[[6::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –55°C to 125°C, SN54AHC32 | < | T~A~ = –40°C to 85°C, SN74AHC32 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC32 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA [2] | 3 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA [2] | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | | 10 | | | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] Source error: printed as I~OH~ = 4 mA and I~OH~ = 8 mA; the rows give V~OL~, and the recommended operating conditions give I~OL~ = 4 mA and 8 mA, so the condition is I~OL~.
```
[[6::table #2]]

```table id=noise caption="Noise characteristics, SN74AHC32" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.3 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.3 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.7 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[7::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 14 | pF |
```
[[7::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC32 | < | T~A~ = –40°C to 85°C, SN74AHC32 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC32 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 5.5 [1] | 7.9 [1] | 1 [1] | 9.5 [1] | 1 | 9.5 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 [1] | 7.9 [1] | 1 [1] | 9.5 [1] | 1 | 9.5 | 1 | 9.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 8 | 11.4 | 1 | 13 | 1 | 13 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | 8 | 11.4 | 1 | 13 | 1 | 13 | 1 | 13 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC32 | < | T~A~ = –40°C to 85°C, SN74AHC32 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC32 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 3.8 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 3.8 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 6.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 5.3 | 7.5 | 1 | 8.5 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.3 | 7.5 | 1 | 8.5 | 1 | 8.5 | 1 | 8.5 | ns |

[1] Source conflict: the 5-V table (p.7) marks these values with (1) and prints no note under the table; the 3.3-V table (p.6) prints note (1): On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[7::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[8::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[8::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[8::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[12::1]]

In this application, three 2-input OR gates are combined to produce a
4-input OR gate function as shown in Figure 8-1. The fourth gate can be
used for another application in the system, or the inputs can be
grounded and the channel left unused. [[12::2]]

The SNx4AHC32 is used to directly control the Enable pin of a fan
driver. The fan driver requires only one input signal to be HIGH before
being enabled, and should be disabled in the event that all signals go
LOW. The 4-input OR gate function combines the four individual overheat
signals into a single active-high enable signal. [[12::3]]

Temperature sensors can often be spread throughout a system rather than
being in a centralized location. This would mean longer length traces or
wires to pass signals through leading to slower edge transitions. This
makes the SNx4AHC32 useful for combining the incoming signals. [[12::4]]

Conversion note: the typical application block diagram (Figure 8-1) shows two OR gates, one fed by the Device 1 and Device 2 Overheat signals and one by the Device 3 and Device 4 Overheat signals, and a third OR gate that combines their outputs into the EN input of the fan driver.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the electrical
characteristics of the device as described in the Electrical
Characteristics section. [[12::6]]

The positive voltage supply must be capable of sourcing current equal to
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. [[12::7]]

The ground must be capable of sinking current equal to the total current
to be sunk by all outputs of the SNx4AHC32 plus the maximum supply
current, I~CC~, listed in the Electrical Characteristics, and any
transient current required for switching. The logic device can only sink
as much current that can be sunk into its ground connection. Be sure to
not exceed the maximum total current through GND listed in the Absolute
Maximum Ratings. [[13::1]]

The SNx4AHC32 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50 pF. [[13::2]]

The SNx4AHC32 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the HIGH
state, the output voltage in the equation is defined as the difference
between the measured output voltage and the supply voltage at the V~CC~
pin. [[13::3]]

Total power consumption can be calculated using the information provided
in CMOS Power Consumption and Cpd Calculation application note. [[13::4]]

Thermal increase can be calculated using the information provided in
Thermal Characteristics of Standard Linear and Logic (SLL) Packages and
Devices application note. [[13::5]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.
Source conflict: the Absolute Maximum Ratings table (p.5) lists no
junction temperature. [[13::6]]

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Source error: the source prints this sentence
without the two levels the signals must cross (p.13); kept as printed.
Do not exceed the maximum input voltage range found in the Absolute
Maximum Ratings. [[13::7]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for a
default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SNx4AHC32 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10-kΩ resistor
value is often used due to these factors. [[13::8]]

Refer to the Feature Description section for additional information
regarding the inputs for this device. [[13::9]]

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified by
the V~OL~ specification in the Electrical Characteristics. [[13::10]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[13::11]]

Refer to the Feature Description section for additional information
regarding the outputs for this device. [[13::12]]

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[13::13]]
2. Ensure the capacitive load at the output is ≤ 50pF. This is not a
   hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SNx4AHC32 to one or more of the receiving devices. [[13::13]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this will prevent the maximum output
   current from the Absolute Maximum Ratings from being violated. Most
   CMOS inputs have a resistive load measured in MΩ; much larger than
   the minimum calculated previously. [[13::13]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[14::1]]

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
layout example. [[14::2]]

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
convenient. [[14::3]]

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill, a bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and no 90° corners for signal lines.

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
5962-9682501Q2A | Tube of 55 | -55 to 125 | 5962-9682501Q2A SNJ54AHC32FK
5962-9682501QCA | Tube of 25 | -55 to 125 | 5962-9682501QCA SNJ54AHC32J
5962-9682501QDA | Tube of 25 | -55 to 125 | 5962-9682501QDA SNJ54AHC32W
SN74AHC32BQAR | Reel of 3000 | -40 to 125 | AHC32
SN74AHC32D | | -40 to 125 | AHC32 | Obsolete
SN74AHC32DBR | Reel of 2000 | -40 to 125 | HA32
SN74AHC32DGVR | Reel of 2000 | -40 to 125 | HA32
SN74AHC32DR | Reel of 2500 | -40 to 125 | AHC32
SN74AHC32DRG4 | Reel of 2500 | -40 to 125 | AHC32
SN74AHC32N | Tube of 25 | -40 to 125 | SN74AHC32N | NRND
SN74AHC32NSR | Reel of 2000 | -40 to 125 | AHC32 | Last Time Buy
SN74AHC32PW | | -40 to 125 | HA32 | Obsolete
SN74AHC32PWR | Reel of 2000 | -40 to 125 | HA32
SN74AHC32RGYR | Reel of 3000 | -40 to 125 | HA32
SNJ54AHC32FK | Tube of 55 | -55 to 125 | 5962-9682501Q2A SNJ54AHC32FK
SNJ54AHC32J | Tube of 25 | -55 to 125 | 5962-9682501QCA SNJ54AHC32J
SNJ54AHC32W | Tube of 25 | -55 to 125 | 5962-9682501QDA SNJ54AHC32W
```
[[17::table #1]]

## Conversion notes

- Source pages 1–15 converted. The ordering lines are from the package
  option addendum (pp.17–19). The support text on p.15, the support and
  revision history page (16), tape and reel and mechanical pages
  (20–44) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), device information table, the logic
  diagram and functional block diagram (one OR gate); the clamp diode
  figure (Figure 7-1), the typical application block diagram (Figure
  8-1), the application timing diagram (Figure 8-2) and the example
  layout (Figure 8-3) are described in text.
- The absolute maximum ratings of the source print the parameter name,
  symbol and condition of each line in one cell ("Input clamp current,
  I~IK~ (V~I~ < 0)"); they are put in separate cells.
- The thermal table's symbol is printed "R θJA" in two pieces in the
  text layer (thermal note 4).
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The source's function table is split over pp.10–11; it is one table
  here.
- Figure 6-1 (p.8) is a picture; only its notes are in the text layer.
- From images: 50% V~CC~ (V~M~, Figure 6-1, p.8); the overbar of
  Y = !{A × B} (Description, p.1: one bar over the whole of A × B); the
  gate arrangement of Figure 8-1 (p.12), the diode placement of Figure
  7-1 (p.10) and the EN waveform of Figure 8-2 (p.14).
- Source issues:
  - Source error: the Description prints Y = !{A × B} as the first form
    (Description note).
  - Source error: the pin table (Table 4-1, p.3) prints pin 23 for 1B in
    the J, W column; the pin drawing (Figure 4-1) shows 1B on pin 2.
  - Source error: V~OL~ rows printed with I~OH~ = 4 mA / 8 mA; corrected
    to I~OL~ (electrical characteristics note 2).
  - Source error: § 8.2.1.2 "Input signals must cross to be considered a
    logic LOW, and to be considered a logic HIGH" names no levels; kept
    as printed.
  - Source conflict: the caution of § 8.2.1.1 says T~J(max)~ is listed
    in the Absolute Maximum Ratings; the table lists no junction
    temperature.
  - Source conflict: the 5-V switching table marks values with (1) and
    prints no note; the 3.3-V table prints it (switching table note).
  - Unconfirmed: I~OH~/I~OL~ at V~CC~ = 2 V printed as ±50 mA (ROC table
    note).
