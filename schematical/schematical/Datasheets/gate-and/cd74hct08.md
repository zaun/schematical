---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hct08",
  "title": "CDx4HCT08 quadruple 2-input AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/cd74hct08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS403A",
    "date": "2024-08",
    "pages": 23
  },
  "devices": ["gate:74_08"],
  "parts": ["5962-8688301CA", "CD54HCT08F", "CD54HCT08F3A", "CD74HCT08E", "CD74HCT08M", "CD74HCT08M96"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-08"
}
---

# CDx4HCT08 quadruple 2-input AND gates

## Description

This device contains four independent 2-input AND gates. Each gate
performs the Boolean function Y = A · B in positive logic. [[1::1]]

LSTTL input logic compatible. V~IL(max)~ = 0.8 V, V~IH(min)~ = 2 V.
Buffered inputs. 4.5 V to 5.5 V operation. Wide operating temperature
range: -55°C to +125°C. Supports fanout up to 10 LSTTL loads. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel input A
1B, 2B, 3B, 4B: Channel input B
1Y, 2Y, 3Y, 4Y: Channel output Y
GND: Ground
VCC: Positive supply
```
[[1::table #1]]

## Functional description

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The drive capability of this device may create fast edges into light
loads so routing and load conditions should be considered to prevent
ringing. Additionally, the outputs of this device are capable of driving
larger currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to over-current. The electrical and thermal limits defined in
the [Absolute Maximum Ratings](#absolute-maximum-ratings) must be
followed at all times. [[8::2]]

The CD74HCT08 can drive a load with a total capacitance less than or
equal to the maximum load listed in the
[Switching Characteristics](#switching-characteristics) connected to a
high-impedance CMOS input while still meeting all of the datasheet
specifications. Larger capacitive loads can be applied, however it is
not recommended to exceed the provided load value. If larger capacitive
loads are required, it is recommended to add a series resistor between
the output and the capacitor to limit output current to the values given
in the Absolute Maximum Ratings. [[8::3]]

### TTL-compatible CMOS inputs

TTL-Compatible CMOS inputs are high impedance and are typically modeled
as a resistor from the input to ground in parallel with the input
capacitance given in the
[Electrical Characteristics](#electrical-characteristics). The worst
case resistance is calculated with the maximum input voltage, given in
the Absolute Maximum Ratings, and the maximum input leakage current,
given in the Electrical Characteristics, using ohm's law (R = V ÷ I). [[8::3]] [[8::4]]

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in the
[Recommended Operating Conditions](#recommended-operating-conditions) to
avoid excessive current consumption and oscillations. If a slow or noisy
input signal is required, a device with a Schmitt-trigger input should
be used to condition the input signal prior to the TTL-compatible CMOS
input. [[8::5]]

TTL-Compatible CMOS inputs have a lower threshold voltage than standard
CMOS inputs to allow for compatibility with older bipolar logic devices.
See the Recommended Operating Conditions for the valid input voltages
for the CD74HCT08. [[8::6]]

### Clamp diode structure

The inputs and outputs to this device have both positive and negative
clamping diodes. Voltages beyond the values specified in the Absolute
Maximum Ratings table can cause damage to the device. The recommended
input and output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[9::1]] [[9::2]]

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```
[[9::Figure 7-1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V or V~I~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ > -0.5 V or V~O~ < V~CC~ + 0.5 V [4] | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature [3] | Plastic package | | 150 | °C |
| ^ | ^ | Hermetic package or die | | 175 | °C |
| | Lead temperature (soldering 10 s) | SOIC - lead tips only | | 300 | °C |
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
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per AEC Q100-002 [1] HBM ESD Classification Level 2 | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per AEC Q100-011 CDM ESD Classification Level C6 | ±1000 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 4.5 to 5.5 V | 2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 4.5 to 5.5 V | | | 0.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise and fall rate [1] | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 5.5 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | | -55 | | 125 | °C |

[1] Unconfirmed: the source gives this rate in ns, without a per-volt unit.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (CD74HCT08) [1]"
| Symbol | Parameter | N (PDIP) 14 pins | D (SOIC) 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 103.8 | 138.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 91.6 | 93.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 83.5 | 94.7 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 71.1 | 49.1 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 83.4 | 94.3 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[4::table #4]] [[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | 25 °C | < | < | –40 °C to 85 °C | < | –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OH~ = -20 µA | 4.5 V | 4.4 | | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | | | 3.84 | | 3.7 | | V |
| V~OL~ | Low-level output voltage | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
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
| C~pd~ | Power dissipation capacitance per gate | No load | 5 V | 51 | pF |
```
[[5::table #3]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25 °C (unless otherwise noted)"
| Symbol | Parameter | From | To | Test conditions | V~CC~ | 25 °C | < | –40 °C to 85 °C | –55 °C to 125 °C | Unit |
| ^ | ^ | ^ | ^ | ^ | ^ | Typ | Max | Max | Max | ^ |
|---|---|---|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | C~L~ = 50 pF | 4.5 V | | 25 | 31 | 38 | ns |
| ^ | ^ | ^ | ^ | C~L~ = 15 pF | 5 V | 10 | | | | ns |
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
[[7::Figure 6-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and transition times"
VM: 50 %
```
[[7::Figure 6-2]]

## Application information

Information in the following applications sections is not part of the TI
component specification. [[10::1]]

Typical application: three 2-input AND gates are combined to produce a
4-input AND gate function. The fourth gate can be used for another
application in the system, or the inputs can be grounded and the channel
left unused. This device is used to directly control the RESET pin of a
motor controller. The controller requires four input signals to all be
HIGH before being enabled, and should be disabled in the event that any
one signal goes LOW. The 4-input AND gate function combines the four
individual reset signals into a single active-low reset signal.
Conversion note: the schematic (Figure 8-1) names the four signals OC
(over current detection), PG (power supply), ON/OFF (on/off switch) and
OT (over temp detection). [[10::2]] [[10::3]]

Power: the supply must be capable of sourcing current equal to the total
current to be sourced by all outputs plus the maximum supply current,
I~CC~. Be sure not to exceed the maximum total current through GND or
V~CC~ listed in the absolute maximum ratings. Each V~CC~ terminal should
have a bypass capacitor to prevent power disturbance. A 0.1-µF capacitor
is recommended for this device. The 0.1-µF and 1-µF capacitors are commonly used in
parallel. The bypass capacitor should be installed as close to the power
terminal as possible. [[10::5]] [[11::8]]

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
makes more sense for the logic function or is more convenient. [[12::1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-8688301CA | Tube of 25 | -55 to 125 | 5962-8688301CA CD54HCT08F3A
CD54HCT08F | Tube of 25 | -55 to 125 | CD54HCT08F
CD54HCT08F3A | Tube of 25 | -55 to 125 | 5962-8688301CA CD54HCT08F3A
CD74HCT08E | Tube of 25 | -55 to 125 | CD74HCT08E
CD74HCT08M | | -55 to 125 | HCT08M | Obsolete
CD74HCT08M96 | Reel of 2500 | -55 to 125 | HCT08M
```
[[14::table #1]]

## Conversion notes

- Source pages 1–13 converted; the package option addendum, tape and
  reel and mechanical pages (14–23) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: features and applications lists, device information table,
  revision history, support pages, functional block diagram, layout
  example and application figures (described in text).
- Typical characteristics (V~OH~ and V~OL~ against output current) not
  converted.
- The switching tables' Min columns and the empty Min/Typ columns at
  –40 °C to 85 °C and –55 °C to 125 °C hold no values and are left out.
- Ordering vendor columns are from the package option addendum (p. 14).
- Source issues:
  - Unconfirmed: input transition rise and fall rate given in ns (ROC note 1).
  - Unconfirmed: I~O~ condition "V~O~ > –0.5 V or V~O~ < V~CC~ + 0.5 V" (absolute maximum ratings note 4).
