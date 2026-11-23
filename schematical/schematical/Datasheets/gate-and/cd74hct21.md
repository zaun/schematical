---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hct21",
  "title": "CD74HCT21 dual 4-input AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/cd74hct21.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS418",
    "date": "2020-03",
    "pages": 24
  },
  "devices": ["gate:74_21"],
  "parts": ["CD74HCT21E", "CD74HCT21M", "CD74HCT21M96", "CD74HCT21MT"],
  "packages": {"p14": "D, N"},
  "converted": "2026-11-08"
}
---

# CD74HCT21 dual 4-input AND gates

## Description

This device contains two independent 4-input AND gates. Each gate
performs the Boolean function Y = A · B · C · D in positive logic. [[1::1]]

LSTTL input logic compatible. V~IL(max)~ = 0.8 V, V~IH(min)~ = 2 V.
Buffered inputs. 4.5 V to 5.5 V operation. Wide operating temperature
range: -55°C to +125°C. Supports fanout up to 10 LSTTL loads. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A: Channel input A
1B, 2B: Channel input B
1C, 2C: Channel input C
1D, 2D: Channel input D
1Y, 2Y: Channel output Y
GND: Ground
VCC: Positive supply
NC: Not internally connected
```
[[1::table #1]]

## Functional description

Outputs: a balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges into
light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable of
driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits defined in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed
at all times. The CD74HCT21 can drive a load with a total capacitance less
than or equal to the maximum load listed in the
[switching characteristics](#switching-characteristics) connected to a
high-impedance CMOS input while still meeting all of the datasheet
specifications. Larger capacitive loads can be applied, however it is
not recommended to exceed the provided load value. If larger capacitive
loads are required, it is recommended to add a series resistor between
the output and the capacitor to limit output current. [[8::2]] [[8::3]]

Inputs: TTL-compatible CMOS inputs are high impedance and are typically
modeled as a resistor from the input to ground in parallel with the
input capacitance. Signals applied to the inputs need to have fast edge
rates, as defined in the
[recommended operating conditions](#recommended-operating-conditions),
to avoid excessive current consumption and oscillations. If a slow or
noisy input signal is required, a device with a Schmitt-trigger input
should be used to condition the input signal prior to the
TTL-compatible CMOS input. TTL-compatible CMOS inputs have a lower
threshold voltage than standard CMOS inputs to allow for compatibility
with older bipolar logic devices. [[8::3]] [[8::5]] [[8::6]]

The inputs and outputs have both positive and negative clamping diodes.
Voltages beyond the values specified in the absolute maximum ratings
table can cause damage to the device. The recommended input and output voltage ratings may be exceeded
if the input and output clamp-current ratings are observed. [[9::1]] [[9::2]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 8-1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | < | < | Output |
| A | B | C | D | Y |
|:-:|:-:|:-:|:-:|:-:|
| H | H | H | H | H |
| L | X | X | X | L |
| X | L | X | X | L |
| X | X | L | X | L |
| X | X | X | L | L |
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ > -0.5 V or V~O~ < V~CC~ + 0.5 V [4] | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | | | 150 | °C |
| | Lead temperature (soldering 10 s) | SOIC - Lead Tips Only | | 300 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Rating may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Condition. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Guaranteed by design.
[4] Unconfirmed: the source gives this condition as printed, "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V", which every output voltage meets.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 V to 5.5 V | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 V to 5.5 V | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| t~t~ | Input transition rise and fall time | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 5.5 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | | -55 | | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (CD74HCT21) [1]"
| Symbol | Parameter | N (PDIP) 14 pins | D (SOIC) 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 64.9 | 102.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 52.7 | 58.7 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 44.7 | 58.2 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 32.3 | 19.9 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 44.4 | 57.8 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[4::table #4]] [[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | 25 °C | < | < | –40 °C to 85 °C | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ | I~OH~ = -20 µA | 4.5 V | 4.4 | | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | | | 3.84 | | 3.7 | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IL~ | I~OL~ = 20 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | I~OL~ = 4 mA | 4.5 V | | | 0.26 | | 0.33 | | 0.4 | V |
| I~I~ | Input leakage current | V~I~ = V~CC~ and GND | I~O~ = 0 | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND | I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 40 | µA |
| ΔI~CC~ [1] | Additional quiescent device current per input pin | V~I~ = V~CC~ – 2.1 | < | 4.5 V to 5.5 V | | 100 | 360 | | 450 | | 490 | µA |
| C~i~ | Input capacitance | | < | 5 V | | | 10 | | 10 | | 10 | pF |

[1] For dual-supply systems theoretical worst case (V~I~ = 2.4 V, V~CC~ = 5.5 V) specification is 1.8 mA.
```
[[5::table #2]]

```table id=op caption="Operating characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 5 V | 42 | pF |
```
[[5::table #3]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | From | To | Test conditions | V~CC~ | 25 °C | < | –40 °C to 85 °C | –55 °C to 125 °C | Unit |
| ^ | ^ | ^ | ^ | ^ | ^ | Typ | Max | Max | Max | ^ |
|---|---|---|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A, B, C or D | Y | C~L~ = 50 pF | 4.5 V | | 27 | 34 | 41 | ns |
| ^ | ^ | ^ | ^ | C~L~ = 15 pF | 5 V | 11 | | | | ns |
| t~t~ | Transition-time | | Y | C~L~ = 50 pF | 4.5 V | | 15 | 19 | 22 | ns |
```
[[5::table #4]]

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 6 ns. The outputs are measured one at a
time, with one input transition per measurement. C~L~ = 50 pF and
includes probe and jig capacitance. t~t~ is the greater of t~r~ and
t~f~. The maximum between t~PLH~ and t~PHL~ is used for t~pd~. [[7::1]] [[7::2]] [[7::4]]

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```
[[7::Figure 7-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and transition times"
VM: 50 %
```
[[7::Figure 7-2]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

Typical application: this device is used to directly control the RESET
pin of a motor controller. The controller requires four input signals
to all be HIGH before being enabled, and should be disabled in the event
that any one signal goes LOW. The 4-input AND gate function combines the
four individual reset signals into a single active-low reset signal. [[10::2]]

Power: the supply must be capable of sourcing current equal to the total
current to be sourced by all outputs plus the maximum supply current,
I~CC~. Be sure not to exceed the maximum total current through GND or
V~CC~ listed in the absolute maximum ratings. Each V~CC~ terminal should
have a bypass capacitor to prevent power disturbance. A 0.1-µF capacitor
is recommended for this device. The 0.1-µF and 1-µF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the power
terminal as possible. [[10::4]] [[12::1]]

Inputs: unused inputs must be terminated to either V~CC~ or ground.
These can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
is to be used sometimes, but not always. A 10-kΩ resistor value is
often used. [[11::1]]

Outputs: unused outputs can be left floating. Do not connect outputs
directly to V~CC~ or ground. [[11::4]]

Detailed design procedure: add a decoupling capacitor from V~CC~ to GND.
The capacitor needs to be placed physically close to the device and
electrically close to both the V~CC~ and GND pins. Ensure the capacitive load at the output is ≤ 70 pF.
This is not a hard limit, however it will ensure optimal performance.
Ensure the resistive load at the output is larger than (V~CC~ /
I~O(max)~) Ω. This will ensure that the maximum output current from the
absolute maximum ratings is not violated. [[11::6]]

Layout: inputs must not ever be left floating. Such unused input pins
must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. All unused inputs of
digital logic devices must be connected to a logic high or logic low
voltage, as defined by the input voltage specifications, to prevent them
from floating. The logic level that must be applied to any particular
unused input depends on the function of the device. Generally, the inputs are tied to GND or V~CC~, whichever
makes more sense for the logic function or is more convenient. [[13::1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

## Ordering information

```ordering
CD74HCT21E | Tube of 25 | -55 to 125 | CD74HCT21E
CD74HCT21M | | -55 to 125 | HCT21M | Obsolete
CD74HCT21M96 | Reel of 2500 | -55 to 125 | HCT21M
CD74HCT21MT | | -55 to 125 | HCT21M | Obsolete
```
[[15::table #1]]

## Conversion notes

- Source pages 1–14 converted; the package option addendum, tape and
  reel and mechanical pages (15–24) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: features and applications lists, device information table,
  revision history, support pages, functional block diagram, layout
  example and application figures (described in text).
- Typical characteristics (V~OH~ and V~OL~ against output current) not
  converted.
- The switching tables' Min columns and the empty Min/Typ columns at
  –40 °C to 85 °C and –55 °C to 125 °C hold no values and are left out.
- Ordering vendor columns are from the package option addendum (p. 15).
- Source issues:
  - Unconfirmed: I~O~ condition "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V" (absolute maximum ratings note 4).
