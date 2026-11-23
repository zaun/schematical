---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g32-q1",
  "title": "SN74AHCT1G32-Q1 automotive single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahct1g32-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS606F",
    "date": "2024-10",
    "pages": 27
  },
  "devices": ["gate:1G32"],
  "parts": ["CAHCT1G32QDBVRQ1", "CAHCT1G32QDCKRQ1", "CAHCT1G32WDTXRQ1"],
  "packages": {"p5": "DBV, DCK, DTX"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G32-Q1 automotive single 2-input positive-OR gate

## Description

The SN74AHCT1G32-Q1 device is a single 2-input positive-OR gate. The
device performs the Boolean function Y = A + B in positive logic. [[1::1]]

Qualified for automotive applications. Operating range of 4.5V to 5.5V.
Max t~pd~ of 9.5ns at 5V. Low power consumption, 20µA max I~CC~. Output
drive of ±8mA at 5V. Inputs are TTL-voltage compatible. [[1::2]]

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
[[3::Table 4-1]]

## Functional description

The SN74AHCT1G32-Q1 device is a single 2-input positive-OR gate. The
device performs the Boolean function Y = A + B in positive logic. [[1::1]]

Source error: the Overview (p.7) prints "Y = A + B or Y = !{A × B}" (one bar over A × B); the function table (Table 7-1) gives Y = L only when A and B are both L, so !{A × B} (a NAND) is kept out.

The device has TTL inputs that allow up translation from 3.3 V to 5 V.
The inputs are high impedance when V~CC~ = 0 V. [[7::2]]

Conversion note: the logic diagram (Figure 7-1) shows one OR gate with inputs A (pin 1) and B (pin 2) and output Y (pin 4).

- Slow rise and fall time on outputs allow for low noise outputs. [[7::3]]
- TTL inputs Allows up translation from 3.3 V to 5 V [[7::3]]

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[7::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage | | -0.5 | 7 | V |
| V~O~ | Output voltage | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
```
[[4::table #1]]

Note: The input and output voltage ratings may be exceeded if the input
and output current ratings are observed. [[4::3]]

The package thermal impedance is calculated in accordance with JESD
51-7. [[4::3]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged-Device Model (C5), per AEC Q100-011, all pins | ±1000 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT1G32-Q1"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SOT-70) 5 pins | DTX (X2SON) 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 184.7 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | 3.7 | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.52 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 20 | µA |
| ΔI~CC~ [1] | Supply-current change | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.65 | mA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 11.5 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 | 6.9 | 1 | 8 | | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 6.9 | 1 | 8 | | 9.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | | 10.5 | ns |
```
[[5::table #3]]

## Parameter measurement information

Conversion note: Figure 6-1 (p.6) is a picture; its notes are quoted in the Conversion notes, and it gives no value for C~L~, which is 15 pF or 50 pF in the [switching characteristics](#sw).

Conversion note: in the figure the inputs swing 0 V to 3 V and are measured at 1.5 V, and the outputs are measured at 50% V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[6::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[6::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[8::1]]

Conversion note: the typical application block diagram (Figure 8-1) is drawn with AND gate symbols: !{OC} from Over Current Detection and PG from the Power Supply go to one gate, ON/!{OFF} from the On/Off Switch and !{OT} from Over Temp Detection to a second, and a third gate combines the two outputs to drive !{RESET} of the Motor Controller.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the
electrical characteristics of the device as described in the Electrical
Characteristics section. [[8::3]]

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the
SN74AHCT1G32-Q1 plus the maximum static supply current, I~CC~, listed
in the Electrical Characteristics, and any transient current required
for switching. The logic device can only source as much current that is
provided by the positive supply source. Be sure to not exceed the
maximum total current through V~CC~ listed in the Absolute Maximum
Ratings. [[8::4]]

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SN74AHCT1G32-Q1 plus the
maximum supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only sink as much current that can be sunk into its
ground connection. Be sure to not exceed the maximum total current
through GND listed in the Absolute Maximum Ratings. [[8::5]]

The SN74AHCT1G32-Q1 can drive a load with a total capacitance less than
or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF. [[8::6]]

The SN74AHCT1G32-Q1 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin. [[8::7]]

Total power consumption can be calculated using the information
provided in the CMOS Power Consumption and Cpd Calculation application
note. [[8::8]]

Thermal increase can be calculated using the information provided in
the Thermal Characteristics of Standard Linear and Logic (SLL) Packages
and Devices application note. [[8::9]]

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device. [[9::1]]

Source conflict: this caution (p.9) says T~J(max)~ is listed in the Absolute Maximum Ratings; that table (p.4) lists no junction temperature.

### Input considerations

Input signals must cross V~IL(max)~ V~t-(min)~ to be considered a logic
LOW, and V~IH(min)~ V~t+(max)~ to be considered a logic HIGH. Do not
exceed the maximum input voltage range found in the Absolute Maximum
Ratings. [[9::2]]

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SN74AHCT1G32-Q1 (as specified in the Electrical Characteristics),
and the desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors. [[9::3]]

The SN74AHCT1G32-Q1 has CMOS inputs and thus requires fast input
transitions to operate correctly, as defined in the Recommended
Operating Conditions table. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability. [[9::4]]

The SN74AHCT1G32-Q1 has no input signal transition rate requirements
because it has Schmitt-trigger inputs. [[9::5]]

Another benefit to having Schmitt-trigger inputs is the ability to
reject noise. Noise with a large enough amplitude can still cause
issues. To know how much noise is too much, please refer to the
ΔV~T(min)~ in the Electrical Characteristics. This hysteresis value
will provide the peak-to-peak limit. [[9::6]]

Unlike what happens with standard CMOS inputs, Schmitt-trigger inputs
can be held at any valid value without causing huge increases in power
consumption. The typical additional current caused by holding an input
at a value other than V~CC~ or ground is plotted in the Typical
Characteristics. [[9::7]]

Source conflict: § 8.1.1.2 (p.9) says both that the device "requires fast input transitions" and that it "has no input signal transition rate requirements because it has Schmitt-trigger inputs"; the recommended operating conditions (p.4) limit Δt/Δv to 20 ns/V, and the source has no ΔV~T~ row, no V~t-~ or V~t+~ levels and no Typical Characteristics. All kept as printed.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics. [[9::9]]

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This
can cause excessive current and damage to the device. [[9::10]]

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. [[9::11]]

Open-drain outputs can be connected together directly to produce a
wired-AND configuration or for additional output drive strength. [[9::12]]

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground. [[9::13]]

Source conflict: § 8.1.1.3 (p.9) speaks of two channels within the same device and of open-drain outputs; the device is a single gate (p.1) and the function table (p.7) shows no high-impedance output. Kept as printed.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section. [[10::1]]
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. This can
   be accomplished by providing short, appropriately sized traces from
   the SN74AHCT1G32-Q1 to one or more of the receiving devices. [[10::1]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   Absolute Maximum Ratings is not violated. Most CMOS inputs have a
   resistive load measured in MΩ; much larger than the minimum
   calculated previously. [[10::1]]
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the application report, CMOS Power Consumption
   and Cpd Calculation. [[10::1]]

Conversion note: the application timing diagram (Figure 8-2) shows the !{OC}, PG, ON/!{OFF}, !{OT} and !{RESET} waveforms.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Section 5.3 table. [[10::2]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 μF and 1 μF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results. [[10::3]]

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used). Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. [[10::4]]

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=X2SON-5-DTX
```

## Ordering information

```ordering
CAHCT1G32QDBVRQ1 | Reel of 3000 | -40 to 125 | 36GH, B32U
CAHCT1G32QDCKRQ1 | Reel of 3000 | -40 to 125 | BGU
CAHCT1G32WDTXRQ1 | Reel of 3000 | -40 to 125 | 8
```
[[14::table #1]]

## Conversion notes

- Source pages 1–11 converted. The ordering lines are from the package
  option addendum (p.14). The support and revision history page (12),
  page 13, tape and reel and mechanical pages (16–27) are left out; the
  generated Package section replaces them.
- Dropped: applications list (features kept as sentences in the
  Description), package information table, the two "Refer to the
  Feature Description section" sentences of § 8.1.1; the logic diagram
  (p.1 and Figure 7-1), the application block diagram (Figure 8-1), the
  application timing diagram (Figure 8-2) and the example layout
  (Figure 8-3) are described in text.
- The pin table's note "I = Input, O = Output, I/O = Input or Output,
  G = Ground, P = Power." is on its TYPE column, which the generated pin
  table draws from the catalog.
- The absolute maximum ratings print two lines under the table without a
  marker ("Note: The input and output voltage ratings…" and "The package
  thermal impedance…"); they follow the table as text.
- The source has no typical characteristics, although § 8.1.1.2 refers
  to them.
- Figure 6-1 (p.6) is a picture with no text layer. Its notes read:
  "A. C~L~ includes probe and jig capacitance. C. All input pulses are
  supplied by generators having the following characteristics: PRR ≤
  1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. D. The outputs are
  measured one at a time, with one input transition per measurement.
  E. All parameters and waveforms are not applicable to all devices."
- From images: the notes of Figure 6-1 quoted above, the 0 V, 3 V and
  1.5 V input levels and 50% V~CC~ (Figure 6-1, p.6); the overbar of
  !{A × B} (Overview, p.7); the overbars of !{OC}, ON/!{OFF}, !{OT} and
  !{RESET} (Figures 8-1 and 8-2, pp.8 and 10); 0.1 µF (Figure 8-3,
  p.11; the text layer prints "0.1 F").
- Source issues:
  - Source error: Overview formula "Y = A + B or Y = !{A × B}"
    (Functional description note).
  - Unconfirmed: the ESD ratings name the charged-device model "(C5)".
  - Source conflict: T~J(max)~ said to be listed in the Absolute
    Maximum Ratings, which list none (Application information note).
  - Source conflict: fast input transitions required vs Schmitt-trigger
    inputs with no transition rate requirement, with references to
    ΔV~T(min)~ and Typical Characteristics the source does not have
    (Input considerations note).
  - Source conflict: two channels and open-drain outputs named for a
    single push-pull gate (Output considerations note).
  - Unconfirmed: the typical application block diagram (Figure 8-1) is
    drawn with AND gate symbols in this OR gate's datasheet.
