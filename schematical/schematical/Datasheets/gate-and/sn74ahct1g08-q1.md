---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g08-q1",
  "title": "SN74AHCT1G08-Q1 automotive 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahct1g08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS941A",
    "date": "2024-01",
    "pages": 25
  },
  "devices": ["gate:1G08"],
  "parts": ["CAHCT1G08QDBVRQ1", "CAHCT1G08QDCKRQ1"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-08"
}
---

# SN74AHCT1G08-Q1 automotive 2-input positive-AND gate

## Description

The SN74AHCT1G08-Q1 is a 2-input AND Gate. Each gate performs the Boolean
function Y = A × B in positive logic.

AEC-Q100 qualified for automotive applications: device temperature
grade 1, -40 °C to +125 °C. Operating range of 4.5 V to 5.5 V; inputs are
TTL-voltage compatible; latch-up performance exceeds 250 mA per JESD 17.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
Y: Output Y
GND: Ground
VCC: Positive Supply
```

## Functional description

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the output power of the device to be limited to avoid
damage due to overcurrent. The electrical and thermal limits defined in
the Absolute Maximum Ratings must be followed at all times. Unused
push-pull CMOS outputs should be left disconnected.

### TTL-compatible CMOS inputs

These inputs are specifically designed to interface with TTL logic
devices by having a reduced input voltage threshold. TTL-compatible CMOS
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using Ohm's law (R = V ÷ I).

TTL-compatible CMOS inputs require that input signals transition between
valid logic states quickly, as defined by the input transition time or
rate in the Recommended Operating Conditions table. Failing to meet this
specification will result in excessive power consumption and could cause
oscillations.

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a system
will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10-kΩ resistor is recommended and will typically meet all
requirements.

### Clamp diode structure

The outputs to this device have both positive and negative clamping
diodes, and the inputs to this device have negative clamping diodes only
(Figure 7-1: output clamps to V~CC~ (+I~OK~) and GND (–I~OK~), input
clamp to GND (–I~IK~)). Voltages beyond the values specified in the
Absolute Maximum Ratings table can cause damage to the device. The input
and output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

## Function tables

```table id=function caption="Function table"
| A [1] | B [1] | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = high voltage level, L = low voltage level, X = do not care, Z = high impedance
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage range | | -0.5 | 7 | V |
| V~O~ | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current [2] | V~I~ < -0.5 V | | -20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < -0.5 V or V~O~ > V~CC~ + 0.5 V | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge, human body model (HBM) [3] | AEC Q100-002, HBM ESD Classification Level 2 | | ±2000 | V |
| ^ | Electrostatic discharge, charged device model (CDM) | AEC Q100-011, CDM ESD Classification Level C4B | | ±1000 | V |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| V~IH~ | High-level input voltage | 2.00 | | V |
| V~IL~ | Low-Level input voltage | | 0.8 | V |
| I~O~ | Output current | | ±8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report: Implications of Slow or Floating CMOS Inputs.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (SOT-23) | DCK (SC70) | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278.0 | 293.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 208.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 180.6 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 120.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 179.5 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report. Both packages 5 pins. R~θJC(bot)~ (junction-to-case, bottom) is N/A for both.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | -40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA [1] | 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA [1] | 4.5 V | | | 0.36 | | | 0.44 | V |
| I~I~ | Input current | V~I~ = 0 V to V~CC~ | 0 V to 5.5 V | | | ±0.1 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | | 10 | µA |
| ΔI~CC~ | Supply current change | One input at 0.3 V or 3.4 V, other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | | 1.5 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 2 | 10 | pF |
| C~PD~ | Power dissipation capacitance [2] [3] | F = 1 MHz | 5 V | | 11 | | | | | pF |

[1] Source error: printed as I~OH~ = 50 µA and I~OH~ = 8 mA in the V~OL~ rows; the rows give V~OL~, so the condition is I~OL~.
[2] C~PD~ is used to determine the dynamic power consumption, per channel.
[3] P~D~ = V~CC~^2^ × F~I~ × (C~PD~ + C~L~) where F~I~ = input frequency, C~L~ = output load capacitance, V~CC~ = supply voltage
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | -40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PD~ | A or B | Y | C~L~ = 15 pF | | 5 | 6.2 | | | 8 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | | 5.5 | 7.9 | | | 10 | ns |
```

## Typical characteristics

```curve id=voh caption="Output voltage vs current in HIGH state; 5-V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-25, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.3, 5]},
 "series": [
  {"label": "-40°C", "points": [[-25, 4.5], [-0.5, 4.99]]},
  {"label": "25°C", "points": [[-25, 4.455], [-0.5, 4.99]]},
  {"label": "125°C", "points": [[-25, 4.385], [-0.5, 4.99]]}
 ]}
```

```curve id=vol caption="Output voltage vs current in LOW state; 5-V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 25]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.5]},
 "series": [
  {"label": "-40°C", "points": [[0.5, 0.01], [25, 0.418]]},
  {"label": "25°C", "points": [[0.5, 0.01], [25, 0.44]]},
  {"label": "125°C", "points": [[0.5, 0.01], [25, 0.482]]}
 ]}
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~t~ < 3 ns. The outputs are measured one at a
time with one input transition per measurement. C~L~ includes probe and
test-fixture capacitance. The greater between t~PLH~ and t~PHL~ is the
same as t~pd~; the greater between t~r~ and t~f~ is the same as t~t~.

```figure ref=load-cmos-50pf caption="Load circuit for push-pull outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delays and transition times"
VM: 50 %
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

In this application, three 2-input AND gates are combined to produce a
4-input AND gate function (Figure 8-1). Multiple SN74AHCT1G08-Q1 are used
to directly control the !{RESET} pin of a motor controller. The
controller requires four input signals to all be HIGH before being
enabled, and should be disabled in the event that any one signal goes
LOW. The 4-input AND gate function combines the four individual reset
signals (!{OC} from over current detection, PG from the power supply,
ON/!{OFF} from the on/off switch, !{OT} from over temp detection) into a
single active-low reset signal.

```wavedrom id=app-timing caption="Application timing diagram"
{"signal": [
  {"name": "OC#", "wave": "10101010"},
  {"name": "PG", "wave": "1.0.1.0."},
  {"name": "ON/OFF#", "wave": "1..0.1.."},
  {"name": "OT#", "wave": "1...0..."},
  {"name": "RESET#", "wave": "10......"}
]}
```

Power considerations: ensure the desired supply voltage is within the
range specified in the Recommended Operating Conditions. The positive
voltage supply must be capable of sourcing current equal to the total
current to be sourced by all outputs plus the maximum static supply
current, I~CC~, and any transient current required for switching; the
ground must be capable of sinking the matching current. Be sure to not
exceed the maximum total current through V~CC~ or GND listed in the
Absolute Maximum Ratings. The maximum junction temperature, T~J(max)~
listed in the Absolute Maximum Ratings [1], is an additional limitation
to prevent damage to the device.

[1] Source conflict: the Absolute Maximum Ratings table (p.4) lists no junction temperature.

Load: the SN74AHCT1G08-Q1 can drive a load with a total capacitance less
than or equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF. It can drive a load with total
resistance described by R~L~ ≥ V~O~ / I~O~, with the output voltage and
current defined in the Electrical Characteristics table with V~OH~ and
V~OL~. When outputting in the HIGH state, the output voltage in the
equation is defined as the difference between the measured output
voltage and the supply voltage at the V~CC~ pin. Ensure the resistive
load at the output is larger than (V~CC~ / I~O(max)~) Ω, so that the
maximum output current from the Absolute Maximum Ratings is not violated.

Inputs: input signals must cross V~IL~(max) to be considered a logic
LOW, and V~IH~(min) to be considered a logic HIGH. Do not exceed the
maximum input voltage range found in the Absolute Maximum Ratings.
Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A 10-kΩ resistor value is often used. Slow input transitions
can cause oscillations, additional power consumption, and reduction in
device reliability.

Outputs: push-pull outputs that could be in opposite states, even for a
very short time period, should never be connected directly together.
This can cause excessive current and damage to the device. Two channels
within the same device with the same input signals can be connected in
parallel for additional output drive strength. Unused outputs can be
left floating. Do not connect outputs directly to V~CC~ or ground.

Power supply: any voltage between the minimum and maximum supply voltage
rating located in the Recommended Operating Conditions. Each V~CC~
terminal should have a good bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. The
0.1-µF and 1-µF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power terminal as possible
for best results.

Layout: when using multiple-input and multiple-channel logic devices,
inputs must never be left floating. All unused inputs of digital logic
devices must be connected to a logic high or logic low voltage, as
defined by the input voltage specifications, to prevent them from
floating. The layout example recommends a GND flood fill, a 0.1 µF
bypass capacitor placed close to the device, and no 90° corners on
signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
CAHCT1G08QDBVRQ1 | Reel of 3000 | -40 to 125 | 38XH
CAHCT1G08QDCKRQ1 | Reel of 3000 | -40 to 125 | 1PI
```

## Conversion notes

- Source pages 1–13 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists (kept as one sentence),
  package information table, simplified logic diagram and functional
  block diagram (one AND gate), the clamp-diode figure (Figure 7-1,
  described in text: the shared clamp-diodes figure also draws an input
  clamp to V~CC~, which this part does not have), the layout example
  drawing (summarized in one sentence), application-note references.
- The ESD ratings table is merged into Absolute maximum ratings.
- Curves digitized from Figures 5-1 and 5-2 (straight lines in the
  source) to about 2 % of full scale.
- The application timing diagram is redrawn from Figure 8-2; the block
  diagram (Figure 8-1) is described in text.
- From images: 4.455, 4.385, 4.99, 0.418, 0.482 and the other curve end
  points (Figures 5-1 and 5-2, p.6); the Figure 8-2 levels (p.12).
- Source issues:
  - Source error: V~OL~ rows printed with I~OH~ (EC table note).
  - Source conflict: T~J(max)~ referenced but not in the Absolute Maximum
    Ratings table (Application information note).
