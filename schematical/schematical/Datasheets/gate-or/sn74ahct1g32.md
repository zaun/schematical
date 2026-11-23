---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g32",
  "title": "SN74AHCT1G32 single 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahct1g32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS320P",
    "date": "2024-03",
    "pages": 25
  },
  "devices": ["gate:1G32"],
  "parts": ["74AHCT1G32DBVRG4", "74AHCT1G32DCKRG4", "SN74AHCT1G32DBVR", "SN74AHCT1G32DCK3", "SN74AHCT1G32DCKR", "SN74AHCT1G32DCKT", "SN74AHCT1G32DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G32 single 2-input positive-OR gate

## Description

The SN74AHCT1G32 device is a single 2-input positive-OR gate. The
device performs the Boolean function Y = A + B or Y = !{!{A} • !{B}} in
positive logic. [[1::1]]

Operating range of 4.5V to 5.5V. Max t~pd~ of 8ns at 5V. Low power
consumption, 10µA max I~CC~. ±8mA output drive at 5V. Inputs are
TTL-voltage compatible. Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

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

The SN74AHCT1G32 device is a single 2-input positive-OR gate. The
device performs the Boolean function Y = A + B or Y = !{!{A} • !{B}} in
positive logic. [[1::1]]

The device has TTL inputs that allow up translation from 3.3 V to 5 V.
The inputs are high impedance when V~CC~ = 0 V. [[8::2]]

Conversion note: the logic diagram (Figure 7-1) shows one OR gate with inputs A and B and output Y.

- Slow rise and fall time on outputs allow for low noise outputs. [[8::3]]
- TTL inputs [[1::2]]
  - Allows up translation from 3.3 V to 5 V [[8::3]]

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
[[8::Table 7-1]]

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
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| T~J~ | Junction Temperature | | | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Section 5.3 is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

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

```table id=thermal caption="Thermal information, SN74AHCT1G32, 5 pins"
| Symbol | Thermal metric [1] | DBV | DCK | DRL | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 287.6 | 328.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 97.7 | 105.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 65 [2] | 150.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 2.0 | 6.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 64.2 | 148.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
[2] Unconfirmed: the source prints "65." with a decimal point and no digit after it.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| ΔI~CC~ [1] | Supply-current change | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 11.5 | pF |
```
[[5::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 | 6.9 | 1 | 8 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 6.9 | 1 | 8 | 1 | 9 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ns |
```
[[5::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 3], [-40, 3.2], [25, 3.8], [85, 4.2], [125, 4.6]]}
 ]}
```
[[6::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::1]]

Conversion note: C~L~ is 15 pF or 50 pF, as given in the [switching characteristics](#sw); in the figure the inputs swing 0 V to 3 V and are measured at 1.5 V, and the outputs are measured at 50% V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

SN74AHCT1G32 is a low-drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The TTL inputs can accept voltages down to
3.3 V and translate up to 5 V. [[9::2]]

Conversion note: in the typical application schematic (Figure 8-1) a µC or System Logic block supplied from 3.3 V or 5 V drives the gate inputs, the gate runs from a 5-V regulated V~CC~ bypassed with 0.1 µF, and its output drives a 5-V accessory.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[9::3]]

### Detailed design procedure

1. Recommended Input Conditions [[9::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the Section 5.3 table. [[9::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the Section 5.3 table. [[9::4]]
2. Recommend Output Conditions [[9::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total for the part. [[9::4]]
   - Outputs should not be pulled above V~CC~. [[9::4]]

Conversion note: the application curve (Figure 8-2, Switching Characteristics Comparison, a picture) shows an input edge V~I~ and the falling output edges of AC32, HC32 and AHCT1G32 gates at V~CC~ = 5 V, Load = 50 Ω/50 pF, and is not drawn here.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Section 5.3 table. [[10::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 μF and 1 μF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results. [[10::2]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such
input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states. [[10::3]]

Specified in Figure 8-3 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I/Os so they also cannot float when disabled. [[10::4]]

Conversion note: Figure 8-3, the layout diagram, shows one gate with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
74AHCT1G32DBVRG4 | Reel of 3000 | -40 to 125 | 3CCF, B32G
74AHCT1G32DCKRG4 | Reel of 3000 | -40 to 125 | BG3
SN74AHCT1G32DBVR | Reel of 3000 | -40 to 125 | 36FH, 3CBF, B323, B32G, B32J, B32L, B32S
SN74AHCT1G32DCK3 | Reel of 3000 | -40 to 85 | BGY | Last Time Buy
SN74AHCT1G32DCKR | Reel of 3000 | -40 to 125 | 1QU, BG3, BGG, BGJ, BGL, BGS
SN74AHCT1G32DCKT | | -40 to 125 | BG3, BGG, BGJ, BGS | Obsolete
SN74AHCT1G32DRLR | Reel of 4000 | -40 to 125 | BGS
```
[[12::table #1]]

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.12–13). The support and revision history page
  (11), tape and reel and mechanical pages (14–25) are left out; the
  generated Package section replaces them.
- Dropped: applications list (features kept as sentences in the
  Description), package information table; the simplified schematic and
  logic diagram (p.1 and Figure 7-1), the typical application schematic
  (Figure 8-1), the switching characteristics comparison (Figure 8-2)
  and the layout diagram (Figure 8-3) are described in text.
- The pin table's note "Signal Types: I = Input, O = Output, I/O =
  Input or Output" is on its TYPE column, which the generated pin table
  draws from the catalog.
- The curve is digitized from Figure 5-1 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for it.
- From images: the overbars of Y = !{!{A} • !{B}} (pp.1 and 8); the
  curve points -55, -40, 25, 85, 125 and 3, 3.2, 3.8, 4.2, 4.6
  (Figure 5-1, p.6); AC32, HC32, AHCT1G32, V~I~ and "Load = 50 Ω/50 pF"
  (Figure 8-2, p.10, a picture).
- Source issues:
  - Unconfirmed: R~θJB~ of the DCK package printed "65." (thermal
    information note 2).
  - Source conflict: the Application Information (p.9) says low drive
    and slow edge rates, the Design Requirements (p.9) say balanced
    output drive whose high drive will create fast edges into light
    loads; both are kept.
