---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g00",
  "title": "SN74AHCT1G00 single 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74ahct1g00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS316P",
    "date": "2024-02",
    "pages": 23
  },
  "devices": ["gate:1G00"],
  "parts": ["74AHCT1G00DBVRG4", "74AHCT1G00DBVTG4", "74AHCT1G00DCKRE4", "74AHCT1G00DCKRG4", "74AHCT1G00DCKTG4", "SN74AHCT1G00DBVR", "SN74AHCT1G00DBVT", "SN74AHCT1G00DCK3", "SN74AHCT1G00DCKR", "SN74AHCT1G00DCKT"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G00 single 2-input positive-NAND gate

## Description

The SN74AHCT1G00 device performs the Boolean function Y = !{A × B} or
Y = !{A} + !{B} in positive logic. [[1::1]]

Operating Range of 4.5 V to 5.5 V. Maximum t~pd~ of 7.1 ns at 5 V. Low
Power Consumption, 10-μA Maximum I~CC~. ±8-mA Output Drive at 5 V.
Inputs Are TTL-Voltage Compatible. Latch-up Performance Exceeds 250 mA
Per JESD 17. [[1::2]]

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

The SN74AHCT1G00 device performs the Boolean function Y = !{A × B} or
Y = !{A} + !{B} in positive logic. [[1::1]]

The device has TTL inputs that allow up translation from 3.3 V to 5 V.
The inputs are high impedance when V~CC~ = 0 V. [[8::2]]

Conversion note: the logic diagram (Figure 7-1) shows one 2-input NAND gate with inputs A (pin 1) and B (pin 2) and output Y (pin 4).

The device is ideal for operating in a 5-V logic system. The low
propagation delay allows fast switching and higher speeds of operation.
In addition, the low power consumption makes this device a good choice
for portable and battery power-sensitive applications. [[8::3]]

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

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | -20 | 20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| | Continuous current through V~CC~ or GND | | -50 | 50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level Input voltage | | 0.8 | V |
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

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT1G00 | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC-70) 5 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | I~OH~ = –8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| ΔI~CC~ [1] | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 10.5 | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuits and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 5 | 6.2 | 1 | 7.1 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | 5 | 6.2 | 1 | 7.1 | 1 | 8 | ^ |
| t~PLH~ | A or B | Y | C~L~ = 15 pF [1] | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ^ |

[1] Unconfirmed: the source prints C~L~ = 15 pF for both groups of rows.
```
[[5::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 5]},
 "series": [
  {"label": "TPD", "points": [[-55, 3], [-40, 3.2], [0, 3.56], [25, 3.8], [50, 3.98], [85, 4.2], [125, 4.6]]}
 ]}
```
[[6::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::Figure 6-1]]

Conversion note: in Figure 6-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

The SN74AHCT1G00 is a low drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The TTL inputs can except voltages down to
3.3 V and translate up to 5 V. [[9::2]]

Conversion note: the typical application schematic (Figure 8-1) shows a 3.3-V or 5-V µC or system logic driving the two gate inputs; the gate runs from a 5-V regulated V~CC~ bypassed with 0.1 µF, and its output drives a 5-V accessory.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads so consider routing and load conditions to prevent ringing. [[9::3]]

Source conflict: the Application Information (p.9) speaks of a low drive device with slow edge rates, while these Design Requirements (p.9) speak of a high drive that creates fast edges (both kept).

### Detailed design procedure

- Recommended input conditions: [[9::4]]
  - Specified high and low levels. See (V~IH~ and V~IL~) in
    [Recommended Operating Conditions](#recommended-operating-conditions). [[9::4]]
  - Inputs are overvoltage tolerant allowing them to go as high as 5.5 V
    at any valid V~CC~. [[9::4]]
- Recommended output conditions: [[9::4]]
  - Load currents should not exceed 25 mA per output and 50 mA total for
    the part. [[9::4]]
  - Outputs should not be pulled above V~CC~. [[9::4]]

Conversion note: the application curve (Figure 8-2, switching characteristics comparison) is an oscilloscope-style plot of the falling outputs of the AHCT1G00, AC00 and HC00 against time, 0 to 20 ns, at V~CC~ = 5 V with a load of 50 Ω/50 pF.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
74AHCT1G00DBVRG4 | Reel of 3000 | -40 to 125 | B00G
74AHCT1G00DBVTG4 | | -40 to 125 | B00G | Obsolete
74AHCT1G00DCKRE4 | Reel of 3000 | -40 to 125 | BA3
74AHCT1G00DCKRG4 | Reel of 3000 | -40 to 125 | BA3
74AHCT1G00DCKTG4 | | -40 to 125 | BA3 | Obsolete
SN74AHCT1G00DBVR | Reel of 3000 | -40 to 125 | 37UH, 3C5F, B003, B00G, B00J, B00S
SN74AHCT1G00DBVT | | -40 to 125 | B003, B00G, B00J, B00S | Obsolete
SN74AHCT1G00DCK3 | Reel of 3000 | -40 to 85 | BAY | Last Time Buy
SN74AHCT1G00DCKR | Reel of 3000 | -40 to 125 | 1QO, BA3, BAG, BAJ, BAS
SN74AHCT1G00DCKT | | -40 to 125 | BA3, BAG, BAJ, BAS | Obsolete
```
[[13::table #1]]

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.13–14). The support and revision history page
  (11), the mechanical information page (12) and the tape and reel and
  mechanical pages (15–23) are left out; the generated Package section
  replaces them.
- Dropped: applications list (the features are kept as sentences in the
  Description), table of contents (p.2), package information table, the
  vendor pin drawing (p.3); the logic diagrams (p.1 and Figure 7-1), the
  typical application schematic (Figure 8-1) and the switching
  characteristics comparison (Figure 8-2) are described in text.
- From images: page 7 (Figure 6-1 and its notes A to E) is a picture
  with no text layer: the Parameter measurement information sentences,
  "C~L~ includes probe and jig capacitance.", "All input pulses are
  supplied by generators having the following characteristics:
  PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns.", "The outputs
  are measured one at a time with one input transition per
  measurement." and "All parameters and waveforms are not applicable to
  all devices." are read from it. Note B belongs to the enable and
  disable waveforms, which are not drawn, and is left out.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column. The switching
  characteristics table has no MIN column at T~A~ = 25°C.
- Section 5.3 of the source is Recommended Operating Conditions.
- The TPD curve is digitized from Figure 5-1 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for it and no unit on the
  temperature axis.
- From images: the overbars of Y = !{A × B} and Y = !{A} + !{B} (p.1 and
  p.8); the curve points -55, -40, 0, 25, 50, 85, 125 and 3, 3.2, 3.56,
  3.8, 3.98, 4.2, 4.6 and the axis ranges -100 to 150 and 0 to 5
  (Figure 5-1, p.6); notes A, C, D and E, 0 V, 3 V, 1.5 V, 50% V~CC~,
  R~L~ = 1 kΩ, S1 and the drawing of Figure 6-1 (p.7); the pin numbers
  of Figure 7-1 (p.8); 3.3-V, 5-V, 0.1 µF, µC (Figure 8-1, p.9, which
  prints "0.1uf" and "uC"); AC00, HC00, AHCT1G00, 0 to 20 ns, V~CC~ =
  5 V and 50 Ω/50 pF (Figure 8-2, p.10).
- Source issues:
  - Unconfirmed: the switching characteristics (p.5) print C~L~ = 15 pF
    for both groups of rows (table note).
  - Source error: the Application Information (p.9) prints "The TTL
    inputs can except voltages"; kept as printed.
  - Source conflict: the Application Information (p.9) speaks of low
    drive and slow edge rates, the Design Requirements (p.9) of high
    drive and fast edges.
  - Unconfirmed: the Features (p.1) give a maximum t~pd~ of 7.1 ns at
    5 V, which the switching characteristics (p.5) give for –40°C to
    85°C; for –40°C to 125°C the table gives 8 ns and 10 ns.
