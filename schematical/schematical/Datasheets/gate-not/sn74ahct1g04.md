---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g04",
  "title": "SN74AHCT1G04 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct1g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS319T",
    "date": "2025-07",
    "pages": 24
  },
  "devices": ["gate:1G04"],
  "parts": ["SN74AHCT1G04DBVR", "SN74AHCT1G04DBVT", "SN74AHCT1G04DCKR"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G04 single inverter gate

## Description

The SN74AHCT1G04 contains one gate. The device performs the Boolean
function Y = !{A}. [[1::1]]

Operating range 4.5V to 5.5V. Max t~pd~ of 7.5ns at 5V. Low power
consumption, 10μA max I~CC~. ±8mA output drive at 5V. Inputs are
TTL-voltage compatible. Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

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
[[3::Table 4-1]]

## Functional description

The SN74AHCT1G04 device contains one inverter. This device has TTL input
levels that allow up translation from 3.3V to 5V. [[8::1]]

Conversion note: the logic diagram (Figure 7-1) shows one inverter, input A, output Y.

### Feature description

- V~CC~ is optimized at 5V [[8::2]]
- Allows up voltage translation from 3.3V to 5V [[8::2]]
  - Inputs accept V~IH~ levels of 2V [[8::2]]
- Slow edge rates minimize output ringing [[8::2]]
- Inputs are TTL-Voltage compatible [[1::2]]

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
[[8::Table 7-1]]

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

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under [Section 5.3](#recommended-operating-conditions) is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±1500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
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
| Δt/Δv | Input Transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004)
```
[[4::table #3]]

Conversion note: the source prints the marker of this note on the MIN heading as well as on the conditions line.

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT1G04"
| Symbol | Thermal metric [1] | DBV 5 pins | DCK 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | ^ |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = –50µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –8mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | Low level output voltage | I~OL~ = 50µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | Input leakage current | V~I~ = 5.5V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| ΔI~CC~ [1] | Supply-Current Change | One input at 3.4V, Other Inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | | | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0V or V~CC~.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1MHz | 14 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5V ± 0.5V (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B [1] | Y | C~L~ = 15pF | 4.7 | | 1 | 7.5 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | 4.7 | | 1 | 7.5 | 1 | 8 | ^ |
| t~PLH~ | A or B [1] | Y | C~L~ = 50pF | 5.5 | | 1 | 8.5 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 | | 1 | 8.5 | 1 | 9 | ^ |

[1] Source error: the source prints the input as "A or B" (p.5), kept as printed; the pin table (p.3) and the function table (p.8) give the device one input, A.
```
[[5::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.4], [-40, 4.6], [0, 4.78], [25, 4.9], [50, 5.1], [85, 5.4], [125, 5.6]]}
 ]}
```
[[6::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1MHz, Z~O~ = 50Ω,
t~r~ ≤ 3ns, t~f~ ≤ 3ns. The outputs are measured one at a time with one
input transition per measurement. All parameters and waveforms are not
applicable to all devices. [[7::1]]

Conversion note: in Figure 6-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1
table, and the pulse-duration, setup-and-hold and enable/disable
waveforms with their note B, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

SN74AHCT1G04 is a low-drive CMOS device that can be used for a multitude
of inverting type applications where output ringing is a concern. The
low drive and slow edge rates will minimize overshoot and undershoot on
the outputs. The input switching levels have been lowered to accommodate
TTL inputs of 0.8V V~IL~ and 2V V~IH~. This feature makes it Ideal for
translating up from 3.3V to 5V. Figure 8-2 shows this type of
translation. [[9::2]]

Conversion note: the typical application schematic (Figure 8-1) shows the inverter between a block labelled 3.3-V accessory and a second block, supplied from a 5-V regulated rail with a 0.1 µF capacitor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[9::3]]

Source conflict: Application Information (§ 8.1) and Feature Description (§ 7.3) say low drive and slow edge rates, Design Requirements (§ 8.2.1) says "the high drive will also create fast edges"; both are kept as printed.

### Detailed design procedure

1. Recommended Input Conditions [[9::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[9::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[9::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as 5.5V
     at any valid V~CC~. [[9::4]]
2. Recommend Output Conditions [[9::4]]
   - Load currents should not exceed 25mA per output and 50mA total for
     the part. [[9::4]]
   - Outputs should not be pulled above V~CC~. [[9::4]]

Conversion note: the application curve (Figure 8-2, 3.3V to 5V Translation) is an oscilloscope picture of a falling input edge and the rising output edge, lettered "Translating from 3.3V to 5V"; it is not drawn here.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the
[Recommended Operating Conditions](#recommended-operating-conditions)
table. [[10::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1μF is recommended. If
there are multiple V~CC~ pins, 0.01μF or 0.022μF is recommended for each
power pin. It is acceptable to parallel multiple bypass caps to reject
different frequencies of noise. A 0.1μF and 1μF are commonly used in
parallel. The bypass capacitor should be installed as close to the power
pin as possible for best results. [[10::2]]

### Layout example

Conversion note: Figure 8-3 (Example Trace Corners for Improved Signal Integrity) shows three corners of a trace of width W: a 90° corner marked WORST, a corner cut at 45° marked BETTER, with lengths 2W and ≥ W, and a rounded corner of radius ≥ 5W marked BEST.

Conversion note: Figures 8-4 (TSSOP and Similar Packages), 8-5 (WQFN and Similar Packages) and 8-6 (SOT, SC70 and Similar Packages) show example bypass capacitor placement, a 0.1 μF capacitor next to the V~CC~ pin with a short return to GND.

Conversion note: Figure 8-7 (Example Damping Resistor Placement for Improved Signal Integrity) shows a 22 Ω resistor at the transmitting port of a long controlled-impedance trace to the receiving port.

### Layout guidelines

- Bypass capacitor placement [[11::2]]
  - Place near the positive supply terminal of the device [[11::5]]
  - Provide an electrically short ground return path [[11::5]]
  - Use wide traces to minimize impedance [[11::5]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[11::5]]
- Signal trace geometry [[11::5]]
  - 8mil to 12mil trace width [[11::5]]
  - Lengths less than 12cm to minimize transmission line effects [[11::5]]
  - Avoid 90° corners for signal traces [[11::5]]
  - Use an unbroken ground plane below signal traces [[11::5]]
  - Flood fill areas around signal traces with ground [[11::5]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[11::5]]
  - For traces longer than 12cm [[11::5]]
    - Use impedance controlled traces [[11::5]]
    - Source-terminate using a series damping resistor near the output [[11::5]]
    - Avoid branches; buffer each signal that must branch separately [[11::5]]

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74AHCT1G04DBVR | Reel of 3000 | -40 to 125 | 38VH, 3C7F, B043, B04G, B04J, B04L, B04S
SN74AHCT1G04DBVT | | -40 to 125 | B043, B04G, B04J, B04S | Obsolete
SN74AHCT1G04DCKR | Reel of 3000 | -40 to 125 | 1QS, BC3, BCG, BCJ, BCL, BCS
```
[[14::table #1]]

## Conversion notes

- Source pages 1–11 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.14–15). The support and revision history pages (12–13),
  with the mechanical information section on p.13, and the tape and reel
  and mechanical pages (16–24) are left out; the generated Package
  section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the vendor
  pin drawing (p.3), the simplified schematic (p.1, one inverter); the
  logic diagram (Figure 7-1), the typical application schematic
  (Figure 8-1), the application curve (Figure 8-2) and the layout
  examples (Figures 8-3 to 8-7) are described in text.
- Section 5.3, named in absolute maximum ratings note 1, the detailed
  design procedure and the power supply recommendations, is the source's
  Recommended Operating Conditions.
- The pin table's note "Signal Types: I = Input, O = Output, I/O = Input
  or Output" is on its TYPE column, which the generated pin table draws
  from the catalog.
- The addendum also lists 74AHCT1G04DBVRE4, 74AHCT1G04DBVRG4,
  74AHCT1G04DBVTG4, 74AHCT1G04DCKRE4, 74AHCT1G04DCKRG4, 74AHCT1G04DCKTG4
  and SN74AHCT1G04DCKT, which the catalog doesn't carry.
- The switching characteristics table of the source has no MIN column at
  T~A~ = 25°C and prints no value under its MAX there.
- The curve is digitized from Figure 5-1 (p.6), a picture, to about 2 %
  of full scale; the source states no load or V~CC~ for it.
- Figure 6-1 (p.7) is a picture with no text layer; the load circuit and
  the propagation delay waveforms were compared with the page image.
- From images: the overbar of Y = !{A} (p.1); the curve points -55, 4.4,
  -40, 4.6, 0, 4.78, 25, 4.9, 50, 5.1, 85, 5.4, 125, 5.6 and the axis
  ranges -100 to 150 and 0 to 6 (Figure 5-1, p.6); 0 V, 3 V, 1.5 V,
  50% V~CC~ and R~L~ = 1 kΩ (Figure 6-1, p.7); 3.3-V accessory, 5-V
  regulated and 0.1 µF (Figure 8-1, p.9); "Translating from 3.3V to 5V"
  (Figure 8-2, p.10); 90°, 45°, 2W, ≥ W and ≥ 5W (Figure 8-3, p.10);
  0.1 μF (Figures 8-4 to 8-6, p.11; the text layer prints "0.1 F") and
  22 Ω (Figure 8-7, p.11).
- Source issues:
  - Source error: the switching characteristics print the input as
    "A or B" for this one-input device; kept as printed (switching
    characteristics note 1).
  - Source error: "Recommend Output Conditions" (§ 8.2.2) and "makes it
    Ideal" (§ 8.1); kept as printed.
  - Source conflict: low drive and slow edge rates (§ 7.3, § 8.1) vs
    "high drive ... fast edges" (§ 8.2.1) (Design requirements note).
  - Unconfirmed: the Features (p.1) give a max t~pd~ of 7.5ns at 5V,
    which the switching characteristics (p.5) give for –40°C to 85°C at
    C~L~ = 15pF; the other maximums of the table are 8, 8.5 and 9 ns.
