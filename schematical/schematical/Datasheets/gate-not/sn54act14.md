---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn54act14",
  "title": "SNx4ACT14 hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn54act14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS557K",
    "date": "2024-11",
    "pages": 40
  },
  "devices": ["gate:74_14"],
  "parts": ["5962-9218301M2A", "5962-9218301MCA", "5962-9218301MDA", "SN74ACT14D", "SN74ACT14NSRG4", "SN74ACT14PWR1G4", "SNJ54ACT14FK", "SNJ54ACT14J", "SNJ54ACT14W"],
  "packages": {"p14": "D, DB, N, NS, PW, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4ACT14 hex Schmitt-trigger inverter

## Description

These Schmitt-trigger devices contain six independent inverters. [[1::1]]

V~CC~ operation of 4.5V to 5.5V. Inputs accept voltages to 5.5V. Max
t~pd~ of 11ns at 5V. Inputs are TTL-voltage compatible. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: Channel 1, Input A
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3Y: Channel 3, Output Y
GND: Ground
4Y: Channel 4, Output Y
4A: Channel 4, Input A
5Y: Channel 5, Output Y
5A: Channel 5, Input A
6Y: Channel 6, Output Y
6A: Channel 6, Input A
VCC: Positive Supply
NC: No internal connection
```
[[3::Table 4-1]]

Connect the GND pin to the exposed thermal pad for correct operation.
Connect the thermal pad to any internal PCB ground plane using multiple
vias for good thermal performance. Conversion note: the source has this
in the thermal pad row of the pin function table. [[4::1]]

## Functional description

These 'ACT14 devices perform the Boolean function Y = !{A}. Because of
the Schmitt action, they have different input threshold levels for
positive-going (V~T+~) and for negative-going (V~T−~) signals. [[8::1]]

These circuits are temperature compensated and can be triggered from
the slowest of input ramps and still give clean, jitter-free output
signals. They also have a greater noise margin than conventional
inverters. [[8::2]]

Conversion note: the logic diagram (positive logic) and the functional
block diagram show one inverter with the hysteresis symbol, input A and
output Y.

- V~CC~ is optimized at 5 V [[8::3]]
- Allows up voltage translation from 3.3 V to 5 V [[8::3]]
  - Inputs accept V~IH~ levels of 2 V [[8::3]]
- Slow edge rates minimize output ringing [[8::3]]
- Inputs are TTL-Voltage compatible [[1::2]]

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due
to overcurrent. The electrical and thermal limits defined in the
Absolute Maximum Ratings must be followed at all times. [[8::4]]

Unused push-pull CMOS outputs must be left disconnected. [[8::5]]

### Clamp diode structure

As shown in Figure 7-1, the inputs and outputs to this device have both
positive and negative clamping diodes. [[8::6]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed. [[8::7]]

```figure ref=clamp-diodes caption="Figure 7-1. Electrical placement of clamping diodes for each input and output"
```
[[8::Figure 7-1]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [1] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [1] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±1500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | SN54ACT14 | < | SN74ACT14 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | mA |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |
```
[[5::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74ACT14 | < | < | < | < | < | Unit |
| ^ | ^ | BQA (WQFN) 14 pins | D (SOIC) 14 pins | DB (SSOP) 14 pins | N (PDIP) 14 pins | NS (SOP) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 85.4 | 89.9 | 96 | 80 | 76 | 148 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54ACT14 | < | SN74ACT14 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | | 4.5 V | 1.2 | 1.5 | 1.9 | 1.2 | 1.9 | 1.2 | 1.9 | V |
| ^ | ^ | | 5.5 V | 1.4 | 1.7 | 2.1 | 1.4 | 2.1 | 1.4 | 2.1 | V |
| V~T−~ | Negative-going threshold | | 4.5 V | 0.5 | 0.9 | 1.2 | 0.5 | 1.2 | 0.5 | 1.2 | V |
| ^ | ^ | | 5.5 V | 0.6 | 1 | 1.4 | 0.6 | 1.4 | 0.6 | 1.4 | V |
| ∆V~T~ | Hysteresis (V~T+~ − V~T−~) | | 4.5 V | 0.4 | 0.6 | 1.4 | 0.4 | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.4 | 0.6 | 1.5 | 0.4 | 1.5 | 0.4 | 1.5 | V |
| V~OH~ | | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | 5.4 | | V |
| ^ | ^ | I~OH~ = –24 mA | 4.5 V | 3.86 | | | 3.7 | | 3.76 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | 4.76 | | V |
| ^ | ^ | I~OH~ = –50 mA [1] | 5.5 V | | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = –75 mA [1] | 5.5 V | | | | | | 3.85 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | | 1.65 | V |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | | 20 | µA |
| ∆I~CC~ [2] | | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5V | | 4.5 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 2 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[6::table #1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 20 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | T~A~ = 25°C | < | SN54ACT14 | < | SN74ACT14 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 1.5 | 11.5 | 1 | 14 | 1 | 12.5 | ns |
| t~PHL~ | ^ | ^ | 1.5 | 10 | 1 | 13 | 1 | 11 | ns |
```
[[6::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time, with one input transition per measurement. [[7::Figure 6-1]]

Conversion note: the load circuit of Figure 6-1 also has a 500 Ω
resistor from the output to switch S1 (2 × V~CC~ or open), and S1 is
open for t~PLH~/t~PHL~, which leaves the load below. Conversion note: in
the voltage waveforms the input swings 0 V to 3 V and is measured at
1.5 V, and the outputs are measured at 50% V~CC~.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```
[[7::Figure 6-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

The SNx4ACT14 device is a low-drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The inputs are tolerant to 5.5 V at any
valid V~CC~. This feature makes it Ideal for translating down to the
V~CC~ level. Switching Characteristics Comparison shows the reduction
in ringing compared to higher drive parts such as AC. [[10::2]]

Conversion note: the typical application schematic (Figure 8-1) shows a
Counter with outputs 2^0^ to 2^3^ and a D-Type Flip-Flop with output
2^4^, the counter's 2^3^ output going through one inverter to the
flip-flop's clock input, the flip-flop's !{Q} output fed back to its D
input, and the Clear line on both CLR inputs.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions [[10::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     Recommended Operating Conditions table. [[10::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[10::4]]
2. Recommend Output Conditions [[10::4]]
   - Load currents should not exceed 35 mA per output and 70 mA total
     for the part. [[10::4]]
   - Outputs should not be pulled above V~CC~. [[10::4]]

Conversion note: the typical application timing diagram (Figure 8-2)
shows the square waves 2^3^, !{2^3^} and 2^4^, with 2^4^ at half the
frequency of 2^3^, and the legend "Input – 32 kHz, 2^4^ – 1 kHz".

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[11::2]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 μF and 1 μF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results. [[11::3]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such
input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states. [[11::4]]

Specified in Example Diagram are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I/Os so they also cannot float when disabled. [[11::5]]

Conversion note: Figure 8-3, the example layout for the SN74ACT14
(p.11), recommends a GND flood fill for improved signal isolation,
noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor
placed close to the device, unused inputs tied to V~CC~, an unused
output left floating, and no 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9218301M2A | Tube of 55 | -55 to 125 | 5962-9218301M2A SNJ54ACT14FK | Last Time Buy
5962-9218301MCA | Tube of 25 | -55 to 125 | 5962-9218301MCA SNJ54ACT14J | NRND
5962-9218301MDA | Tube of 25 | -55 to 125 | 5962-9218301MDA SNJ54ACT14W | Last Time Buy
SN74ACT14D | | -40 to 85 | ACT14 | Obsolete
SN74ACT14NSRG4 | Reel of 2000 | -40 to 85 | ACT14 | NRND
SN74ACT14PWR1G4 | Reel of 2000 | -40 to 85 | AD14
SNJ54ACT14FK | Tube of 55 | -55 to 125 | 5962-9218301M2A SNJ54ACT14FK | Last Time Buy
SNJ54ACT14J | Tube of 25 | -55 to 125 | 5962-9218301MCA SNJ54ACT14J | NRND
SNJ54ACT14W | Tube of 25 | -55 to 125 | 5962-9218301MDA SNJ54ACT14W | Last Time Buy
```
[[14::table #1]]

## Conversion notes

- Source pages 1–11 converted; the support and revision history pages
  (12–13), the package option addendum, tape and reel and mechanical
  pages (14–40) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (pp.14–15).
- Dropped: applications list, device information table, the vendor's
  pinout drawings (Figures 4-1 to 4-3), the I/O legend of
  the pin function table (the I/O column is generated from the
  catalog), the layout example drawing (Figure 8-3, summarized in one
  sentence).
- The package option addendum also lists SN74ACT14BQAR, SN74ACT14DBR,
  SN74ACT14DR, SN74ACT14N, SN74ACT14NSR, SN74ACT14PW and SN74ACT14PWR,
  which this file doesn't claim.
- The pin function table of the source gives the 14-pin numbers only;
  the FK pin numbers are those of Figure 4-3, and the NC description is
  the figure's "NC − No internal connection".
- The electrical characteristics table of the source gives only symbols
  in its Parameter column for V~OH~ to C~i~; no parameter names are
  added. The recommended operating conditions print no conditions line
  and no conditions column.
- The pin function table continues on the next page (pp.3–4).
- The logic diagram, the functional block diagram, the typical
  application schematic (Figure 8-1) and the typical application timing
  diagram (Figure 8-2) are described in text.
- "Section 5.3" of the source is written as that section's name,
  Recommended Operating Conditions.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: the pin numbers and names of the FK package (Figure 4-3, p.3); overbar of Y = !{A} (p.8); !{Q} (Figure 8-1, p.10); the
  overbar of !{2^3^} and the exponents of 2^0^ to 2^4^ (Figures 8-1 and
  8-2, pp.10–11; the text layer prints 20 to 24 and "±" for "–"); Figure
  6-1 (p.7) is a picture: 500 Ω (both load resistors), 2 × V~CC~ and
  Open (S1), C~L~ = 50 pF, 3 V, 0 V and 1.5 V (input), 50% V~CC~
  (outputs), and its notes "A. C~L~ includes probe and jig capacitance.
  B. All input pulses are supplied by generators having the following
  characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns,
  t~f~ ≤ 2.5 ns. C. The outputs are measured one at a time, with one
  input transition per measurement."
- Source issues:
  - Source error: Figure 4-3 (p.3) labels pin 2 of the FK package "A1";
    the other drawings and the pin function table name the input 1A.
  - Source conflict: the detailed design procedure (p.10) refers to
    Δt/ΔV, V~IH~ and V~IL~ in the recommended operating conditions
    table, which has none of these rows (p.5).
  - Source conflict: the application information (p.10) calls the
    device low-drive with slow edge rates; the design requirements on
    the same page say the high drive will create fast edges.
  - Source conflict: the package option addendum (p.14) gives -40 to
    125 for SN74ACT14BQAR; the recommended operating conditions (p.5)
    give -40 to 85 °C for the SN74ACT14.
