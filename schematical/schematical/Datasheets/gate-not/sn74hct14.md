---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hct14",
  "title": "SNx4HCT14 hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74hct14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS225G",
    "date": "2016-11",
    "pages": 34
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74HCT14D", "SN74HCT14DBR", "SN74HCT14DGVR", "SN74HCT14DR", "SN74HCT14DRG3", "SN74HCT14DT", "SN74HCT14N", "SN74HCT14PWR", "SN74HCT14PWT"],
  "packages": {"p14": "D, DB, DGV, J, N, PW, W"},
  "converted": "2026-11-21"
}
---

# SNx4HCT14 hex Schmitt-trigger inverters

## Description

The SNx4HCT14 devices contain six independent inverters. The devices
perform the Boolean function Y = !{A} in positive logic.

Operating Voltage Range of 4.5 V to 5.5 V. Outputs Can Drive Up to 10
LSTTL Loads. Inputs Are TTL-Voltage Compatible.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1 input
1Y: Channel 1 output
2A: Channel 2 input
2Y: Channel 2 output
3A: Channel 3 input
3Y: Channel 3 output
4A: Channel 4 input
4Y: Channel 4 output
5A: Channel 5 input
5Y: Channel 5 output
6A: Channel 6 input
6Y: Channel 6 output
GND: Ground
VCC: Power supply
```

## Functional description

The SNx4HCT14 Schmitt-Trigger devices contain six independent
inverters. They perform the Boolean function Y = !{A} in positive
logic.

Schmitt-Trigger inputs are designed to provide a minimum separation
between positive and negative switching thresholds. This allows for
noisy or slow inputs that would cause problems such as oscillation or
excessive current draw with normal CMOS inputs.

Conversion note: the functional block diagram (p.8), like the logic diagram (p.1), shows one inverter with a Schmitt-trigger symbol, input A and output Y.

The wide operating range of these devices allow them to be used in a
variety of systems that use different logic levels. The outputs can
drive up to 10 LSTTL loads each. The balanced drive outputs can source
or sink 8 mA at 5-V V~CC~. This device is also input TTL compatible.

Source conflict: the feature description (p.8) and the detailed design procedure (p.9) give 8 mA at 5-V V~CC~; the features list (p.1) prints "±4-mA Output Drive at 5 V".

## Function tables

Table 1 lists the functional modes of the SNx4HCT14.

```table id=function caption="Function table"
| Input A | Output Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Parameter | Conditions | Min | Max | Unit |
|---|---|--:|--:|---|
| Supply voltage, V~CC~ | | -0.5 | 7 | V |
| Input voltage, V~I~ [2] | | -0.5 | V~CC~ + 0.5 | V |
| Output voltage, V~O~ [2] | | -0.5 | V~CC~ + 0.5 | V |
| Input clamp current, I~IK~ | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| Output clamp current, I~OK~ | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| Continuous output current, I~O~ | V~O~ = 0 to V~CC~ | | ±25 | mA |
| Continuous current through V~CC~ or GND | | | ±50 | mA |
| Operating junction temperature, T~J~ | | | 150 | °C |
| Storage temperature, T~stg~ | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 4.5 | 5.5 | V |
| V~I~ | Input voltage | | 0 | V~CC~ | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| T~A~ | Operating free-air temperature | SN54HCT14 | -55 | 125 | °C |
| ^ | ^ | SN74HCT14 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74HCT14 | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB (SSOP) 14 pins | DGV (TVSOP) 14 pins | N (PDIP) 14 pins | PW (TSSOP) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 90.9 | 105 | 132.2 | 55.3 | 120.2 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 51 | 57 | 51.7 | 42.5 | 48.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 45.2 | 52.4 | 61.4 | 35.1 | 61.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 18.4 | 22.2 | 5.5 | 27.2 | 5.7 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 44.9 | 51.8 | 60.7 | 35 | 61.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | V~CC~ = 4.5 V | T~A~ = 25°C | 1.2 | 1.5 | 1.9 | V |
| ^ | ^ | ^ | SN54HCT14 | 1.2 | | 1.9 | V |
| ^ | ^ | ^ | SN74HCT14 | 1.2 | | 1.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | T~A~ = 25°C | 1.4 | 1.7 | 2.1 | V |
| ^ | ^ | ^ | SN54HCT14 | 1.4 | | 2.1 | V |
| ^ | ^ | ^ | SN74HCT14 | 1.4 | | 2.1 | V |
| V~T–~ | Negative-going threshold | V~CC~ = 4.5 V | T~A~ = 25°C | 0.5 | 0.9 | 1.2 | V |
| ^ | ^ | ^ | SN54HCT14 | 0.5 | | 1.2 | V |
| ^ | ^ | ^ | SN74HCT14 | 0.5 | | 1.2 | V |
| ^ | ^ | V~CC~ = 5.5 V | T~A~ = 25°C | 0.6 | 1 | 1.4 | V |
| ^ | ^ | ^ | SN54HCT14 | 0.6 | | 1.4 | V |
| ^ | ^ | ^ | SN74HCT14 | 0.6 | | 1.4 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | V~CC~ = 4.5 V | T~A~ = 25°C | 0.4 | 0.6 | 1.4 | V |
| ^ | ^ | ^ | SN54HCT14 | 0.4 | | 1.4 | V |
| ^ | ^ | ^ | SN74HCT14 | 0.4 | | 1.4 | V |
| ^ | ^ | V~CC~ = 5.5 V | T~A~ = 25°C | 0.4 | 0.65 | 1.5 | V |
| ^ | ^ | ^ | SN54HCT14 | 0.4 | | 1.5 | V |
| ^ | ^ | ^ | SN74HCT14 | 0.4 | | 1.5 | V |
| V~OH~ | High-level output voltage | I~OH~ = –20 µA and V~CC~ = 4.5 V | T~A~ = 25°C | 4.4 | 4.49 | | V |
| ^ | ^ | ^ | SN54HCT14 | 4.4 | | | V |
| ^ | ^ | ^ | SN74HCT14 | 4.4 | | | V |
| ^ | ^ | I~OH~ = –4 mA and V~CC~ = 4.5 V | T~A~ = 25°C | 3.98 | 4.3 | | V |
| ^ | ^ | ^ | SN54HCT14 | 3.7 | | | V |
| ^ | ^ | ^ | SN74HCT14 | 3.84 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 µA and V~CC~ = 4.5 V | T~A~ = 25°C | | 0.001 | 0.1 | V |
| ^ | ^ | ^ | SN54HCT14 | | | 0.1 | V |
| ^ | ^ | ^ | SN74HCT14 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA and V~CC~ = 4.5 V | T~A~ = 25°C | | 0.17 | 0.26 | V |
| ^ | ^ | ^ | SN54HCT14 | | | 0.4 | V |
| ^ | ^ | ^ | SN74HCT14 | | | 0.33 | V |
| I~I~ | Input current | V~I~ = V~CC~ or GND and V~CC~ = 5.5 V | T~A~ = 25°C | | | ±0.1 | µA |
| ^ | ^ | ^ | SN54HCT14 | | | ±1 | µA |
| ^ | ^ | ^ | SN74HCT14 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0, and V~CC~ = 5.5 V | T~A~ = 25°C | | | 2 | µA |
| ^ | ^ | ^ | SN54HCT14 | | | 40 | µA |
| ^ | ^ | ^ | SN74HCT14 | | | 20 | µA |
| ΔI~CC~ [1] | Change in supply current | One input at 0.5 V or 2.4 V, other inputs at GND or V~CC~, and V~CC~ = 5.5 V | T~A~ = 25°C | | 0.2 | 2.4 | mA |
| ^ | ^ | ^ | SN54HCT14 | | | 3 | mA |
| ^ | ^ | ^ | SN74HCT14 | | | 2.9 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND and V~CC~ = 5 V | T~A~ = 25°C | | 3 | 10 | pF |
| ^ | ^ | ^ | SN54HCT14 | | | 10 | pF |
| ^ | ^ | ^ | SN74HCT14 | | | 10 | pF |

[1] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load | 10 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range and C~L~ = 50 pF (unless otherwise noted; see Figure 5)"
| Symbol | Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation (delay) time | A | Y | V~CC~ = 4.5 V | T~A~ = 25°C | | 20 | 32 | ns |
| ^ | ^ | ^ | ^ | ^ | SN54HCT14 | | | 48 | ns |
| ^ | ^ | ^ | ^ | ^ | SN74HCT14 | | | 40 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 5.5 V | T~A~ = 25°C | | 18 | 30 | ns |
| ^ | ^ | ^ | ^ | ^ | SN54HCT14 | | | 45 | ns |
| ^ | ^ | ^ | ^ | ^ | SN74HCT14 | | | 38 | ns |
| t~t~ | | | Y | V~CC~ = 4.5 V | T~A~ = 25°C | | 7 | 15 | ns |
| ^ | ^ | ^ | ^ | ^ | SN54HCT14 | | | 22 | ns |
| ^ | ^ | ^ | ^ | ^ | SN74HCT14 | | | 19 | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 5.5 V | T~A~ = 25°C | | 6 | 14 | ns |
| ^ | ^ | ^ | ^ | ^ | SN54HCT14 | | | 20 | ns |
| ^ | ^ | ^ | ^ | ^ | SN74HCT14 | | | 17 | ns |
```

## Typical characteristics

Conversion note: Figures 1 and 2 (p.6) plot the supply current against the input voltage at V~CC~ = 4.5 V and V~CC~ = 5.5 V, with one curve for V~I~ = 0 to V~CC~ and one for V~I~ = V~CC~ to 0; they are not digitized here.

Conversion note: Figures 3 and 4 (p.7) plot the output voltage against the input voltage at V~CC~ = 4.5 V and V~CC~ = 5.5 V as a hysteresis loop, with one path for V~I~ = Up and one for V~I~ = Down; they are not digitized here.

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance.

Phase relationships between waveforms were chosen arbitrarily. All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 6 ns, t~f~ = 6 ns.

The outputs are measured one at a time with one input transition per
measurement.

t~PLH~ and t~PHL~ are the same as t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output rise and fall times"
VM: 1.3 V
```

Conversion note: in the source's voltage waveforms (Figure 5, p.7) the input swings between 0 V and 3 V in the propagation delay waveforms and between 0 V and V~CC~ in the input rise and fall times waveform, and input and outputs are measured at 1.3 V, 10% and 90%.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining
suitability of components for their purposes. Customers should validate
and test their design implementation to confirm system functionality.

The SN74HCT14 device is a Schmitt-Trigger input CMOS device that can be
used for a multitude of inverting buffer type functions. The
application shown here takes advantage of the Schmitt-Trigger inputs to
produce a delay for a logic input.

Conversion note: Figure 6 (p.9), the simplified application schematic, shows INPUT driving inverter 1A to 1Y, resistor R~1~ from 1Y to the input 2A with capacitor C~1~ from 2A to ground, and 2Y as OUTPUT.

### Design requirements

This device uses CMOS technology. Take care to avoid bus contention,
because it can drive currents that would exceed maximum limits.
Parallel output drive can create fast edges into light loads, so
consider routing and load conditions to prevent ringing.

### Detailed design procedure

This circuit is designed around an RC network that produces a slow
input to the second inverter. The RC time constant, τ, is calculated
from: τ = RC.

The delay time for this circuit is from
t~delay(min)~ = –ln |1 – V~T+(min)~ / V~CC~| τ to
t~delay(max)~ = –ln |1 – V~T+(max)~ / V~CC~| τ. It must be noted that
the delay is consistent for each device, but because the switching
threshold is only ensured between the minimum and maximum value, the
output pulse length varies between devices. These values must be
calculated by using the minimum and maximum ensured V~T+~ values in the
Electrical Characteristics.

The resistor value must be chosen such that the maximum current to and
from the SN74HCT14 is 8 mA at 5-V V~CC~.

Conversion note: Figure 7 (p.10), the application curve, shows the ideal capacitor voltage V~C~ rising toward V~CC~ over time and the output voltage V~OUT~ switching between t~delay(min)~ and t~delay(max)~, where V~C~ crosses V~T+(min)~ and V~T+(max)~.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
The V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. TI recommends using a 0.1-µF capacitor on the V~CC~
terminal, and must be placed as close as possible to the pin for best
results.

### Layout guidelines

When using multiple bit logic devices, inputs must never float. In many
cases, functions or parts of functions of digital logic devices are
unused, for example, when only two inputs of a triple-input AND gate
are used or only three of the four buffer gates are used. Such inputs
must not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. All unused
inputs of digital logic devices must be connected to a high or low bias
to prevent them from floating. The logic level that must be applied to
any particular unused input depends on the function of the device.
Generally they are tied to GND or V~CC~, whichever makes more sense or
is more convenient. Floating outputs are generally acceptable, unless
the part is a transceiver.

Conversion note: Figure 8 (p.10), the layout diagram, shows a gate with its unused input tied to V~CC~ and a gate with its unused input tied to ground.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74HCT14D | | -40 to 85 | HCT14 | Obsolete
SN74HCT14DBR | Reel of 2000 | -40 to 85 | HT14 | NRND
SN74HCT14DGVR | Reel of 2000 | -40 to 85 | HT14
SN74HCT14DR | Reel of 2500 | -40 to 85 | HCT14
SN74HCT14DRG3 | Reel of 2500 | -40 to 85 | HCT14 | NRND
SN74HCT14DT | | -40 to 85 | HCT14 | Obsolete
SN74HCT14N | Tube of 25 | -40 to 85 | SN74HCT14N
SN74HCT14PWR | Reel of 2000 | -40 to 85 | HT14
SN74HCT14PWT | | -40 to 85 | HT14 | Obsolete
```

## Conversion notes

- Source pages 1–10 converted; the device and documentation support
  page (11), the package option addendum (p.12–14), tape and reel, tube
  and mechanical pages (15–34) dropped, replaced by the generated
  Package and Ordering sections. The ordering lines are from the
  package option addendum.
- Dropped: features and applications lists, the device information
  table, table of contents and revision history (p.2), the vendor's pin
  drawings and the pin numbers of the pin functions table (p.3), the
  logic diagram and functional block diagram, the application
  schematic, application curve and layout diagram (Figures 6, 7 and 8,
  described in text).
- The source also covers the SN54HCT14 in the J (CDIP), W (CFP) and FK
  (20-pin LCCC) packages, and further orderables (SN74HCT14DRG4,
  SN74HCT14NE4, SN74HCT14PWRE4, SN74HCT14PWRG4, SNJ54HCT14FK,
  SNJ54HCT14J, SNJ54HCT14W and the 5962- numbers), which the catalog
  doesn't carry; the SN54HCT14 rows of the tables are kept.
- The source's Power Supply Recommendations and Layout chapters (10 and
  11) are filed under Application information.
- The absolute maximum ratings table prints each parameter with its
  symbol in one cell; it is kept that way, without a Symbol column.
- The notes A to D of Figure 5 (p.7) are given as text above the shared
  load circuit and waveform figures. The typical characteristics
  (Figures 1 to 4) are not digitized.
- From images: overbars of Y = !{A} (p.1, p.8).
- Source issues:
  - Source conflict: output drive of 8 mA at 5-V V~CC~ in the feature
    description (p.8) and the detailed design procedure (p.9) against
    "±4-mA Output Drive at 5 V" in the features list (p.1).
  - Source error: the feature description (p.8) prints "The wide
    operating range of these devices allow them"; kept as printed.
