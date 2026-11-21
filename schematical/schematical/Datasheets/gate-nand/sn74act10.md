---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act10",
  "title": "SNx4ACT10 triple 3-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74act10.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS526E",
    "date": "2025-07",
    "pages": 44
  },
  "devices": ["gate:74_10"],
  "parts": ["5962-9218201MCA", "5962-9218201MDA", "SN74ACT10BQAR", "SN74ACT10D", "SN74ACT10DBR", "SN74ACT10DR", "SN74ACT10DRE4", "SN74ACT10N", "SN74ACT10PWR", "SNJ54ACT10J", "SNJ54ACT10W", "5962-9218201M2A", "SNJ54ACT10FK"],
  "packages": {"p14": "D, DB, N, PW, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4ACT10 triple 3-input positive-NAND gates

## Description

The SNx4ACT10 devices contain three independent 3-input NAND gates. The
devices perform the Boolean functions Y = !{A • B • C} or
Y = !{A} + !{B} + !{C} in positive logic.

4.5V to 5.5V V~CC~ operation. Inputs accept voltages to 5.5V. Max
t~pd~ of 9.5ns at 5V. Inputs are TTL-voltage compatible.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
2A: Channel 2, Input A
2B: Channel 2, Input B
2C: Channel 2, Input C
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
3C: Channel 3, Input C
1Y: Channel 1, Output Y
1C: Channel 1, Input C
VCC: Positive Supply
NC: Not internally connected
```

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad, BQA package only.

## Functional description

This device contains three independent 3-input NAND gates. Each gate
performs the Boolean function Y = !{A•B•C} in positive logic.

Conversion note: the functional block diagram shows one 3-input NAND
gate, with buffered inputs xA, xB and xC and buffered output xY.

### Balanced CMOS push-pull outputs

This device includes balanced CMOS push-pull outputs. The term balanced
indicates that the device can sink and source similar currents. The
drive capability of this device may create fast edges into light loads,
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important to limit the output power of the device to avoid damage due
to overcurrent. The electrical and thermal limits defined in the
Absolute Maximum Ratings must be followed at all times.

Unused push-pull CMOS outputs must be left disconnected.

### TTL-compatible CMOS inputs

This device includes TTL-compatible CMOS inputs. These inputs are
specifically designed to interface with TTL logic devices by having a
reduced input voltage threshold.

TTL-compatible CMOS inputs are high impedance and are typically modeled
as a resistor in parallel with the input capacitance given in the
Electrical Characteristics. The worst case resistance is calculated
with the maximum input voltage, given in the Absolute Maximum Ratings,
and the maximum input leakage current, given in the Electrical
Characteristics, using Ohm's law (R = V ÷ I).

TTL-compatible CMOS inputs require that input signals transition
between valid logic states quickly, as defined by the input transition
time or rate in the Recommended Operating Conditions table. Failing to
meet this specification will result in excessive power consumption and
could cause oscillations. More details can be found in the Implications
of Slow or Floating CMOS Inputs application report.

Do not leave TTL-compatible CMOS inputs floating at any time during
operation. Unused inputs must be terminated at V~CC~ or GND. If a
system will not be actively driving an input at all times, a pull-up or
pull-down resistor can be added to provide a valid input voltage during
these times. The resistor value will depend on multiple factors;
however, a 10kΩ resistor is recommended and typically will meet all
requirements.

### Clamp diode structure

As shown in Figure 7-1, the inputs and outputs to this device have both
positive and negative clamping diodes.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input and output voltage
ratings may be exceeded if the input and output clamp-current ratings
are observed.

```figure ref=clamp-diodes caption="Figure 7-1. Electrical placement of clamping diodes for each input and output"
```

## Function tables

```table id=function caption="Function table"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | SN54ACT10 | < | SN74ACT10 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 8 | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the Implications of Slow or Floating CMOS Inputs application note.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SNx4ACT10 | < | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB 14 pins | N 14 pins | NS 14 pins | PW (TSSOP) 14 pins | BQA (WQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 96 | 80 | 76 | 145.7 | 91.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54ACT10 | < | SN74ACT10 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | 5.4 | | V |
| ^ | ^ | I~OH~ = −24 mA | 4.5 V | 3.86 | | | 3.7 | | 3.76 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | 4.76 | | V |
| ^ | ^ | I~OH~ = −50 mA [1] | 5.5 V | | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = −75 mA [1] | 5.5 V | | | | | | 3.85 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | | 1.65 | V |
| I~I~ | | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 4 | | 80 | | 40 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2.6 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 10 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 25 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5V ± 0.5V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | T~A~ = 25°C | < | < | SN54ACT10 | < | SN74ACT10 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | Any | Y | 1 | 6.5 | 9 | 1 | 10 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | 1 | 6.5 | 9 | 1 | 905 [1] | 1 | 905 [1] | ns |

[1] Unconfirmed: the source prints 905 ns as the maximum t~PHL~ of the SN54ACT10 and of the SN74ACT10; the features (p.1) give a max t~pd~ of 9.5ns at 5V.
```

## Typical characteristics

T~A~ = 25°C (unless otherwise noted)

```curve id=voh-ioh caption="Figure 5-1. Output voltage vs current in HIGH state; 5V supply"
{"x": {"label": "I_OH", "unit": "mA", "range": [-50, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [4.24, 5.04]},
 "series": [
   {"label": "25°C", "points": [[-50, 4.43], [-25, 4.71], [0, 4.99]]},
   {"label": "125°C", "points": [[-50, 4.33], [-25, 4.66], [0, 4.99]]},
   {"label": "-40°C", "points": [[-50, 4.52], [-25, 4.76], [0, 4.99]]}
 ]}
```

```curve id=vol-iol caption="Figure 5-2. Output voltage vs current in LOW state; 5V supply"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 50]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 0.4]},
 "series": [
   {"label": "25°C", "points": [[0, 0], [25, 0.145], [50, 0.29]]},
   {"label": "125°C", "points": [[0, 0], [25, 0.178], [50, 0.356]]},
   {"label": "-40°C", "points": [[0, 0], [25, 0.12], [50, 0.24]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance.

All input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ v 2.5 ns, t~f~ v 2.5 ns.

The outputs are measured one at a time with one input transition per
measurement.

Conversion note: the load circuit also has a 500 Ω resistor from the
output to switch S1 (2 × V~CC~ or open), and S1 is open for
t~PLH~/t~PHL~, which leaves the load below. Conversion note: in the
voltage waveforms the input swings 0 V to 3 V and is measured at 1.5 V,
and the outputs are measured at 50% V~CC~.

```figure ref=load-rl-cl caption="Figure 6-1. Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Figure 6-1. Voltage waveforms"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

In this application, two 3-input NAND gates are used to create an
active-low SR latch as shown in Figure 9-1. The additional gate can be
used elsewhere in the system, or the inputs can be grounded and left
unused.

The SNx4ACT10 is used to drive the tamper indicator LED and provide one
bit of data to the system controller. When the tamper switch outputs
LOW, the output Q becomes HIGH. This output remains HIGH until the
system controller addresses the event and sends a LOW signal to the R
input which returns the Q output back to LOW.

Conversion note: in the typical application block diagram (Figure 9-1)
two cross-coupled 3-input NAND gates form the latch; the system
controller drives !{R~A~} and !{R~B~} together, which have resistor R1
to the supply, tamper switches 1 and 2 drive !{S~A~} and !{S~B~}, and Q
goes to the system controller and through R2 to the tamper indicator
LED.

```wavedrom id=app-timing caption="Figure 9-2. Application timing diagram"
{"signal": [
  {"name": "R#", "wave": "101..01..."},
  {"name": "S#", "wave": "1..01..0.."},
  {"name": "Q", "wave": "x0.1.0.1.."}
]}
```

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the device's
electrical characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal
to the total current to be sourced by all outputs of the SNx4ACT10 plus
the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching. The
logic device can only source as much current that is provided by the
positive supply source. Ensure the maximum total current through V~CC~
listed in the Absolute Maximum Ratings is not exceeded.

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SNx4ACT10 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection.
Ensure the maximum total current through GND listed in the Absolute
Maximum Ratings is not exceeded.

The SNx4ACT10 can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF.

The SNx4ACT10 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OH~ and V~OL~. When outputting
in the HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin.

Total power consumption can be calculated using the information
provided in the CMOS Power Consumption and Cpd Calculation application
note.

Thermal increase can be calculated using the information provided in
the Thermal Characteristics of Standard Linear and Logic (SLL) Packages
and Devices application note.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.
Source conflict: the Absolute Maximum Ratings table (p.5) lists no
junction temperature.

### Input considerations

Input signals must cross V~IL(max)~ to be considered a logic LOW, and
V~IH(min)~ to be considered a logic HIGH. Do not exceed the maximum
input voltage range found in the Absolute Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SNx4ACT10 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10kΩ resistor
value is often used due to these factors.

The SNx4ACT10 has CMOS inputs and thus requires fast input transitions
to operate correctly, as defined in the Recommended Operating
Conditions table. Slow input transitions can cause oscillations,
additional power consumption, and reduction in device reliability.

### Output considerations

The positive supply voltage is used to produce the output HIGH voltage.
Drawing current from the output will decrease the output voltage as
specified by the V~OH~ specification in the Electrical Characteristics.
The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics.

Push-pull outputs that could be in opposite states, even for a very
short time period, should never be connected directly together. This
can cause excessive current and damage to the device.

Two channels within the same device with the same input signals can be
connected in parallel for additional output drive strength.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins. An example layout is shown in the Layout
   section.
2. Verify that the capacitive load at the output is ≤ 50pF. This is not
   a hard limit; by design, however, it will optimize performance. This
   can be accomplished by providing short, appropriately sized traces
   from the SNx4ACT10 to one or more of the receiving devices.
3. Verify that the resistive load at the output is larger than
   (V~CC~ / I~O(max)~)Ω. Doing this prevents the maximum output current
   from the Absolute Maximum Ratings from being violated. Most CMOS
   inputs have a resistive load measured in MΩ; much larger than the
   minimum calculated previously.
4. Thermal issues are rarely a concern for logic gates; the power
   consumption and thermal increase, however, can be calculated using
   the steps provided in the CMOS Power Consumption and Cpd Calculation
   application note.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1μF capacitor is recommended for this device. It
is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1μF and 1μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results.

### Layout guidelines

- Bypass capacitor placement
  - Place near the positive supply terminal of the device
  - Provide an electrically short ground return path
  - Use wide traces to minimize impedance
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible
- Signal trace geometry
  - 8mil to 12mil trace width
  - Lengths less than 12cm to minimize transmission line effects
  - Avoid 90° corners for signal traces
  - Use an unbroken ground plane below signal traces
  - Flood fill areas around signal traces with ground
  - Parallel traces must be separated by at least 3x dielectric
    thickness
  - For traces longer than 12cm
    - Use impedance controlled traces
    - Source-terminate using a series damping resistor near the output
    - Avoid branches; buffer each signal that must branch separately

Conversion note: the layout example drawings (Figures 9-3 to 9-7) show
trace corners for improved signal integrity, bypass capacitor placement
for TSSOP, WQFN and SOT packages, and damping resistor placement, and
are not drawn here.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=PDIP-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9218201M2A | Tube of 55 | -55 to 125 | 5962-9218201M2A SNJ54ACT10FK
5962-9218201MCA | Tube of 25 | -55 to 125 | 5962-9218201MCA SNJ54ACT10J
5962-9218201MDA | Tube of 25 | -55 to 125 | 5962-9218201MDA SNJ54ACT10W
SN74ACT10BQAR | Reel of 3000 | -40 to 85 | AD10
SN74ACT10D | | -40 to 85 | ACT10 | Obsolete
SN74ACT10DBR | Reel of 2000 | -40 to 85 | AD10
SN74ACT10DR | Reel of 2500 | -40 to 85 | ACT10
SN74ACT10DRE4 | Reel of 2500 | -40 to 85 | ACT10
SN74ACT10N | Tube of 25 | -40 to 85 | SN74ACT10N
SN74ACT10PWR | Reel of 2000 | -40 to 85 | AD10
SNJ54ACT10FK | Tube of 55 | -55 to 125 | 5962-9218201M2A SNJ54ACT10FK
SNJ54ACT10J | Tube of 25 | -55 to 125 | 5962-9218201MCA SNJ54ACT10J
SNJ54ACT10W | Tube of 25 | -55 to 125 | 5962-9218201MDA SNJ54ACT10W
```

## Conversion notes

- Source pages 1–16 converted; the support and revision history pages
  (17–18), the package option addendum, tape and reel and mechanical
  pages (19–44) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are the parts' rows of the package option
  addendum (pp.19–20).
- Dropped: applications list, device information table, the logic
  diagram of page 1, the layout example drawings (Figures 9-3 to 9-7),
  the references to the Feature Description section in the input and
  output considerations.
- The electrical characteristics table of the source gives only symbols
  in its Parameter column; no parameter names are added.
- The thermal table lists an NS package that the device information
  table and the package option addendum do not have.
- The pin functions table heads its 14-pin column "PDIP, CDIP"; the
  drawings give the same numbers for the J, W, D, PW, N, DB and BQA
  packages.
- Typical characteristics: Figures 5-1 and 5-2 (p.7) digitized to about
  2 % of full scale.
- The functional block diagram and the typical application block
  diagram (Figure 9-1) are described in text; the application timing
  diagram is redrawn from Figure 9-2 (p.14), where Q is drawn hatched
  (undefined) in the first interval.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: overbars of Y = !{A • B • C} and Y = !{A} + !{B} + !{C}
  (p.1) and of Y = !{A•B•C} (p.9); 500 Ω (both load resistors),
  2 × V~CC~ (S1), 50 pF (C~L~), 3 V, 0 V and 1.5 V (input), 50% V~CC~
  (outputs) (Figure 6-1, p.8); the curve points of Figures 5-1 and 5-2
  (p.7); the overbars of !{R~A~}, !{R~B~}, !{S~A~}, !{S~B~}, R1 to the
  supply (Figure 9-1, p.12) and of !{R} and !{S} (R#, S#, Figure 9-2,
  p.14).
- Source issues:
  - Unconfirmed: maximum t~PHL~ printed as 905 ns for the SN54ACT10 and
    the SN74ACT10 (switching characteristics note 1).
  - Source error: note B of Figure 6-1 (p.8) prints "t~r~ v 2.5 ns,
    t~f~ v 2.5 ns", with "v" where a relation sign belongs; kept as
    printed.
  - Source conflict: the caution names T~J(max)~ in the Absolute Maximum
    Ratings; that table has no T~J~ row (power considerations).
