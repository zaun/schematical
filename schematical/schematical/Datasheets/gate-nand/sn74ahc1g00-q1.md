---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g00-q1",
  "title": "SN74AHC1G00-Q1 automotive single 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74ahc1g00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SLOS424D",
    "date": "2024-10",
    "pages": 26
  },
  "devices": ["gate:1G00"],
  "parts": ["SN74AHC1G00DBVRQ1", "SN74AHC1G00QDCKRQ1", "SN74AHC1G00WDTXRQ1"],
  "packages": {"p5": "DBV, DCK, DTX"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G00-Q1 automotive single 2-input positive-NAND gate

## Description

The SN74AHC1G00-Q1 performs the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic. [[1::1]]

Qualified for automotive applications. Operating range of 2V to 5.5V.
Max t~pd~ of 6.5ns at 5V. Low power consumption, 10µA Max I~CC~. ±8mA
output drive at 5V. Schmitt-trigger action at all inputs makes the
circuit tolerant for slower input rise and fall time. [[1::2]]

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

Conversion note: the functional block diagram (Figure 7-1) is the logic diagram of one 2-input NAND gate, inputs A (pin 1) and B (pin 2), output Y (pin 4).

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[8::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
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

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 | ±1000 | ^ |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| ∆t/∆v | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G00-Q1"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SOT-SC70) 5 pins | DTX (X2SON) 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278.0 | 293.4 | 184.7 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40 C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 μA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | ^ |
| ^ | ^ | I~OH~ = −4 mA | 3 V | 2.58 | | | 2.48 | | ^ |
| ^ | ^ | I~OH~ = −8 mA | 4.5 V | 3.94 | | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 μA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | pF |
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 9.5 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25 C | < | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5.5 | 7.9 | 1 | 11.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 11.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8 | 11.4 | 1 | 15 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8 | 11.4 | 1 | 15 | ^ |
```
[[5::table #3]]

```table id=sw-5v caption="Switching characteristics, 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 3.7 | 5.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.7 | 5.5 | 1 | 8.5 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.2 | 7.5 | 1 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.2 | 7.5 | 1 | 10.5 | ^ |
```
[[6::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::Figure 6-1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

In this application, two 2-input NAND gates are used to create an
active-low SR latch as shown in Figure 8-1. The two additional gates can
be used for a second SR latch, or the inputs can be grounded and both
channels left unused. [[9::2]]

The SN74AHC1G00-Q1 is used to drive the tamper indicator LED and provide
one bit of data to the system controller. When the tamper switch outputs
LOW, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a LOW signal to the R
input which returns the Q output back to LOW. [[9::3]]

Conversion note: the typical application block diagram (Figure 8-1) shows two cross-coupled NAND gates; the tamper switch drives the !{S} input, the system controller drives the !{R} input, which has a pull-up resistor R~1~, and the Q output goes to the system controller and through R~2~ to the tamper indicator LED.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout. [[9::6]]
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance. This can be
   accomplished by providing short, appropriately sized traces from the
   SN74HC00-Q1 to the receiving device. [[9::6]]
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O~(max)) Ω. This will ensure that the maximum output
   current from the
   [Absolute Maximum Ratings](#absolute-maximum-ratings) is not
   violated. Most CMOS inputs have a resistive load measured in
   megaohms; much larger than the minimum calculated above. [[9::6]]
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   Cpd Calculation [[9::6]]

Source error: step 2 of the source names the SN74HC00-Q1 (p.9); this datasheet is for the SN74AHC1G00-Q1.

Conversion note: the application timing diagram (Figure 8-2) shows the !{R} and !{S} inputs and the Q output of the latch; Q is undefined at first, goes high when !{S} goes low and goes low when !{R} goes low.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-μF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass capacitors to reject different
frequencies of noise. The 0.1-μF and 1-μF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results, as shown in the following
layout example. [[10::1]]

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
convenient. [[10::2]]

Conversion note: the example layout (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 μF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=X2SON-5-DTX
```

## Ordering information

```ordering
SN74AHC1G00DBVRQ1 | Reel of 3000 | -40 to 125 | 36CH
SN74AHC1G00QDCKRQ1 | Reel of 3000 | -40 to 125 | AAU
SN74AHC1G00WDTXRQ1 | Reel of 3000 | -40 to 125 | 9
```
[[13::table #1]]

## Conversion notes

- Source pages 1–10 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.13–14). The support and revision history pages (11–12),
  tape and reel and mechanical pages (15–26) are left out; the generated
  Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the logic
  diagram (one NAND gate); the typical application block diagram
  (Figure 8-1), the application timing diagram (Figure 8-2) and the
  example layout (Figure 8-3) are described in text.
- The source's Design Requirements heading (8.2.1) has no text under it.
- Section 5.1, named in step 3 of the detailed design procedure, is the
  source's Absolute Maximum Ratings.
- The absolute maximum ratings of the source print the conditions in
  parentheses; they are put in the Conditions cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- Figure 6-1 (p.7) is a picture; only its notes and S1 table are in the
  text layer.
- From images: 50% V~CC~ (V~M~, Figure 6-1, p.7); overbars of
  Y = !{A • B} and Y = !{A} + !{B} (Description, p.1); the overbars of
  !{R} and !{S}, R~1~, R~2~ and the latch arrangement (Figure 8-1, p.9);
  the !{R}, !{S} and Q waveforms (Figure 8-2, p.10).
- Source issues:
  - Source error: step 2 of the detailed design procedure names the
    SN74HC00-Q1 (Application information note).
  - Source error: the temperature headers are printed "–40 C TO 125°C"
    (electrical characteristics, p.5) and "T~A~ = 25 C" (switching
    characteristics, 3.3 V, p.5), without the degree sign; kept as
    printed.
