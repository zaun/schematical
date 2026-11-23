---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g86",
  "title": "SN74AHC1G86 single 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ahc1g86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS323P",
    "date": "2024-02",
    "pages": 26
  },
  "devices": ["gate:1G86"],
  "parts": ["SN74AHC1G86DBVR", "SN74AHC1G86DBVRE4", "SN74AHC1G86DBVRG4", "SN74AHC1G86DBVT", "SN74AHC1G86DCKR", "SN74AHC1G86DCKRE4", "SN74AHC1G86DCKRG4", "SN74AHC1G86DCKT", "SN74AHC1G86DCKTG4", "SN74AHC1G86DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G86 single 2-input exclusive-OR gate

## Description

The SN74AHC1G86 is a single 2-input exclusive-OR gate. The device
performs the Boolean function Y = !{A}B + A!{B} in positive logic. [[1::1]]

Source error: the Description (p.1) and Overview (p.8) print the first
form as Y = A × B (an AND), while the function table (Table 7-1) gives
Y = H only when A and B differ; the first form is kept out.

Operating range of 2V to 5.5V. Max t~pd~ of 8ns at 5V. Low power
consumption, 10µA max I~CC~. ±8mA output drive at 5V. Schmitt-trigger
action at all inputs makes the circuit tolerant for slower input rise
and fall time. Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

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

A common application is as a true or complementary element. If one of
the inputs is low, the other input is reproduced in true form at the
output. If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[8::2]]

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an SN74AHC1G86 gate in
positive logic; negation may be shown at any twoports. Conversion note:
the five symbols of Figure 7-1 are not drawn here. [[1::4]] [[1::5]]

The output is active (low) if all inputs stand at the same logic level
(that is, A = B). Conversion note: this is the logic-identity element
(symbol =). [[1::6]]

The output is active (low) if an even number of inputs (that is, 0 or
2) are active. Conversion note: this is the even-parity element (symbol
2k). [[1::7]]

The output is active (high) if an odd number of inputs (that is, only 1
of the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1). [[1::8]]

### Feature description

- Wide operating voltage range [[8::8]]
  - Operates from 2 V to 5.5 V [[8::8]]
- Allows down-voltage translation [[8::8]]
  - Inputs accept voltages to 5.5 V [[8::8]]
- The low drive and slow edge rates will minimize overshoot and
  undershoot on the outputs [[9::2]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[8::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous channel current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under [Section 5.3](#recommended-operating-conditions) is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
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

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G86: thermal metric [1]"
| Symbol | Parameter | DBV 5 pins | DCK 5 pins | DRL 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 328.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | 105.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | 150.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | 6.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | 148.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | | 10 | pF |
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```
[[6::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 7 | 11 | 1 | 13 | 1 | 14 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7 | 11 | 1 | 13 | 1 | 14 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 9.5 | 14.5 | 1 | 16.5 | 1 | 17.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 9.5 | 14.5 | 1 | 16.5 | 1 | 17.5 | ns |
```
[[5::table #3]]

```table id=sw-5v caption="Switching characteristics, 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 4.8 | 6.8 | 1 | 8 | 1 | 8.6 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.8 | 6.8 | 1 | 8 | 1 | 8.6 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 6.3 | 8.8 | 1 | 10 | 1 | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.3 | 8.8 | 1 | 10 | 1 | 11 | ns |
```
[[6::table #2]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 3.7], [-40, 3.9], [25, 4.5], [85, 4.9], [125, 5.2]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "V_CC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD in ns", "points": [[2, 6.9], [3, 5], [4.5, 4.8], [5.5, 4.2]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. Conversion note: C~L~
is 15 pF or 50 pF, as given in the
[switching characteristics](#switching-characteristics). [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
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
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[9::1]]

SN74AHC1G86 is a low-drive CMOS device that can be used for a multitude
of bus interface type applications where output ringing is a concern.
The low drive and slow edge rates will minimize overshoot and undershoot
on the outputs. The inputs can accept voltages to 5.5 V at any valid
V~CC~ making it Ideal for down translation. [[9::2]]

Conversion note: the typical application schematic (Figure 8-1) shows the gate, supplied from a 3.3-V or 5-V regulated rail with a 0.1 µF capacitor to ground, between 5-V system logic on a 5-V accessory and a µC or system logic.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[9::3]]

Source conflict: Application Information (§ 8.1) says "low drive and slow edge rates", Design Requirements (§ 8.2.1) says "the high drive will also create fast edges"; both are kept as printed.

### Detailed design procedure

1. Recommended Input Conditions [[9::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[9::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[9::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[9::4]]
2. Recommend Output Conditions [[9::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part. [[9::4]]
   - Outputs should not be pulled above V~CC~. [[9::4]]

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[10::1]]

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

Conversion note: the layout diagram (Figure 8-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1G86DBVR | Reel of 3000 | -40 to 125 | 39JH, 3C4F, A863, A86G, A86J, A86S
SN74AHC1G86DBVRE4 | Reel of 3000 | -40 to 125 | A86G
SN74AHC1G86DBVRG4 | Reel of 3000 | -40 to 125 | A86G
SN74AHC1G86DBVT | | -40 to 125 | A863, A86G, A86J, A86S | Obsolete
SN74AHC1G86DCKR | Reel of 3000 | -40 to 125 | 1RC, AH3, AHG, AHJ, AHS
SN74AHC1G86DCKRE4 | Reel of 3000 | -40 to 125 | AH3
SN74AHC1G86DCKRG4 | Reel of 3000 | -40 to 125 | AH3
SN74AHC1G86DCKT | | -40 to 125 | AH3, AHG, AHJ, AHS | Obsolete
SN74AHC1G86DCKTG4 | Reel of 250 | -40 to 125 | AH3
SN74AHC1G86DRLR | Reel of 4000 | -40 to 125 | AHS
```
[[13::table #1]]

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.13–14). The support and revision history page
  (11), the mechanical information page (12), tape and reel and
  mechanical pages (15–26) are left out; the generated Package section
  replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the
  simplified schematic and functional block diagram (five equivalent
  exclusive-OR symbols and three element symbols, p.1 and Figure 7-1);
  the typical application schematic (Figure 8-1) and the layout diagram
  (Figure 8-3) are described in text.
- Dropped: Figure 8-2 (switching characteristics comparison, an
  oscilloscope-style plot of V~I~ and the AC08, HC08 and AHC1G86 outputs
  at V~CC~ = 5 V, load 50 Ω/50 pF); it is not a characteristic of this
  part alone.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for Figure 5-1 and no load
  for Figure 5-2.
- From images: the overbars of Y = !{A}B + A!{B} (p.1 and p.8); the
  curve points -55, 3.7, 3.9, 85, 4.9, 5.2 (Figure 5-1, p.6) and 6.9,
  4.2 (Figure 5-2, p.6); AC08, HC08, 50 Ω (Figure 8-2, p.10).
- Source issues:
  - Source error: Description and Overview print Y = A × B as the first
    form (Description note).
  - Source error: "any twoports" (p.1 and Figure 7-1) is printed
    without a space; kept as printed.
  - Source error: "Recommend Output Conditions" (§ 8.2.2); kept as
    printed.
  - Source conflict: "low drive and slow edge rates" (§ 8.1) vs "high
    drive ... fast edges" (§ 8.2.1) (Application information note).
