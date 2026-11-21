---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1gu04",
  "title": "SN74LVC1GU04 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1gu04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES215Y",
    "date": "2017-12",
    "pages": 49
  },
  "devices": ["gate:1GU04", "gate:1GU04_SON"],
  "parts": ["74LVC1GU04DCKTG4", "SN74LVC1GU04DBVR", "SN74LVC1GU04DBVT", "SN74LVC1GU04DCKR", "SN74LVC1GU04DCKT", "SN74LVC1GU04DPWR", "SN74LVC1GU04DRLR", "SN74LVC1GU04YZPR", "SN74LVC1GU04DRY2", "SN74LVC1GU04DRYR", "SN74LVC1GU04DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "dsbga": "YZP", "p6": "DRY, DSF"},
  "converted": "2026-11-21"
}
---

# SN74LVC1GU04 single inverter gate

## Description

This single inverter gate is designed for 1.65-V to 5.5-V V~CC~
operation.

The SN74LVC1GU04 device contains one inverter with an unbuffered output
and performs the Boolean function Y = !{A}.

NanoFree package technology is a major breakthrough in device packaging
concepts, using the die as the package.

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V.
Unbuffered Output. Maximum t~pd~ of 3.7 ns at 3.3 V. Low Power
Consumption, 10-μA Maximum I~CC~. ±24-mA Output Drive at 3.3 V. I~off~
Supports Live Insertion, Partial-Power-Down Mode, and Back-Drive
Protection. Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II.

## Pinout

Conversion note: DBV, DCK, DRL and DPW share one pin numbering, and DRY and DSF another.

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input
GND: Ground
NC: Not connected
DNU: Do not use
VCC: Positive Supply
Y: Output
```

NC – No internal connection

## Functional description

The SN74LVC1GU04 device contains one inverter with an unbuffered output
with a maximum sink current of 32 mA.

Conversion note: the logic diagram (Figure 3) shows one inverter, A (pin 2) to Y (pin 4).

### Balanced high-drive CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The high-drive capability of this device creates fast edges into light
loads, so routing and load conditions must be considered to prevent
ringing. Additionally, the outputs of this device are capable of driving
larger currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to overcurrent. The electrical and
thermal limits defined in the Absolute Maximum Ratings must be followed
at all times.

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst-case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using ohm's law (R = V ÷ I).

Signals that are applied to the inputs need to have fast edge rates, as
shown by Δt/Δv in the Recommended Operating Conditions, to avoid
excessive current consumption and oscillations. If a slow or noisy input
signal is required, a device with a Schmitt-trigger input should be used
to condition the input signal prior to the standard CMOS input.

Source conflict: § 8.3.2 (p.9) refers to Δt/Δv in the Recommended Operating Conditions; that table (p.5) has no Δt/Δv row.

### Negative clamping diodes

The inputs and outputs to this device have negative clamping diodes as
shown in Figure 4.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

Conversion note: Figure 4 draws one clamp diode from the input to GND (-I~IK~) and one from the output to GND (-I~OK~), and none to V~CC~.

### Partial power down (I~off~)

The inputs and outputs for this device enter a high-impedance state when
the supply voltage is 0 V. The maximum leakage into or out of any input
or output pin on the device is specified by I~off~ in the Electrical
Characteristics.

Source conflict: § 8.3.4 (p.10) says I~off~ is specified in the Electrical Characteristics; that table (p.6) has no I~off~ row.

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Recommended Operating Conditions.

### Unbuffered logic

A standard CMOS logic function typically consists of at least three
stages: the input inverter, the logic function, and the output inverter.
Some devices have multiple stages at the input or output for various
reasons. An unbuffered CMOS logic function eliminates the extra input
and output stages; the device only contains the required logic function
which is directly driven from the inputs and directly drives the
outputs.

The unbuffered inverter is commonly used in oscillator circuits because
it is less sensitive to parameter changes in the oscillator circuit due
to having lower total gain than a buffered equivalent. To learn more
about how to use an unbuffered inverter in an oscillator circuit, see
Use of the CMOS Unbuffered Inverter in Oscillator Circuits.

### Device functional modes

Table 1 lists the functional modes of the SN74LVC1GU04.

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in Recommended Operating Conditions.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 | ±1000 | ^ |
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.65 | 5.5 | V |
| V~IH~ | High-level input voltage | I~O~ = -100 μA | 0.75 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | I~O~ = 100 μA | | 0.25 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. For more information, see the Implications of Slow or Floating CMOS Inputs application report.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC1GU04 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23), 5 pins | DCK (SC70), 5 pins | DRL (SOT-5X3), 5 pins | DRY (SON), 5 pins [2] | DSF (SON), 5 pins [2] | DPW (X2SON), 5 pins | YZV (DSBGA), 4 pins | YZP (DSBGA), 5 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 231.5 | 276.1 | 296.2 | 369.6 | 410.3 | 511 | 168.2 | 144.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 139.4 | 178.9 | 137.3 | 257.6 | 208.4 | 241.9 | 2.1 | 1.3 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 71.1 | 70.9 | 145.3 | 230.8 | 262.6 | 374.2 | 55.9 | 39.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 45.2 | 47 | 14.7 | 77.2 | 36 | 45 | 1.1 | 0.5 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 70.7 | 69.3 | 145.9 | 231 | 262.3 | 373.3 | 56.3 | 39.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | 168.0 | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
[2] Source conflict: this table (p.6) heads the DRY and DSF columns "5 PINS"; the device information table (p.1) and the pin drawings (p.4) give both SON packages 6 pins. Kept as printed.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range, T~A~ = –40°C to +125°C (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | V~IL~ = 0 V, I~OH~ = -100 µA, V~CC~ = 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | V~IL~ = 0 V, I~OH~ = -4 mA, V~CC~ = 1.65 V | 1.2 | | | ^ |
| ^ | ^ | V~IL~ = 0 V, I~OH~ = -8 mA, V~CC~ = 2.3 V | 1.9 | | | ^ |
| ^ | ^ | V~IL~ = 0 V, I~OH~ = -16 mA, V~CC~ = 3 V | 2.4 | | | ^ |
| ^ | ^ | V~IL~ = 0 V, I~OH~ = -24 mA, V~CC~ = 3 V | 2.3 | | | ^ |
| ^ | ^ | V~IL~ = 0 V, I~OH~ = -32 mA, V~CC~ = 4.5 V | 3.8 | | | ^ |
| V~OL~ | Low-level output voltage | V~IH~ = V~CC~, I~OL~ = 100 µA, V~CC~ = 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | V~IH~ = V~CC~, I~OL~ = 4 mA, V~CC~ = 1.65 V | | | 0.45 | ^ |
| ^ | ^ | V~IH~ = V~CC~, I~OL~ = 8 mA, V~CC~ = 2.3 V | | | 0.3 | ^ |
| ^ | ^ | V~IH~ = V~CC~, I~OL~ = 16 mA, V~CC~ = 3 V | | | 0.4 | ^ |
| ^ | ^ | V~IH~ = V~CC~, I~OL~ = 24 mA, V~CC~ = 3 V | | | 0.55 | ^ |
| ^ | ^ | V~IH~ = V~CC~, I~OL~ = 32 mA, V~CC~ = 4.5 V | | | 0.55 | ^ |
| I~I~ | Input leakage current | A Input: V~I~ = 5.5 V or GND, V~CC~ = 0 V to 5.5 V | | | ±5 | μA |
| I~CC~ | Supply current | V~I~ = 5.5 V or GND, I~O~ = 0, V~CC~ = 1.65 V to 5.5 V | | | 10 | μA |
| C~I~ | Input capacitance | V~I~ = V~CC~ or GND, V~CC~ = 3.3 V, T~A~ = –40°C to 85°C | | 7 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | < | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | V~CC~ = 1.8 V | 9 | pF |
| ^ | ^ | ^ | V~CC~ = 2.5 V | 11 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V | 13 | ^ |
| ^ | ^ | ^ | V~CC~ = 5 V | 27 | ^ |
```

## Switching characteristics

Conversion note: Figure 2 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics: T~A~ = –40°C to +85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (See Figure 2)"
| Symbol | Parameter | Test conditions | < | Min | Max | Unit |
|---|---|---|---|--:|--:|---|
| t~pd~ | Propagation delay | A-to-Y | V~CC~ = 1.8 V ± 0.15 V | 1.3 | 5 | ns |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1 | 4 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1.1 | 3.7 | ^ |
| ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 3 | ^ |
```

```table id=sw-125 caption="Switching characteristics: T~A~ = –40°C to +125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (See Figure 2)"
| Symbol | Parameter | Test conditions | < | Min | Max | Unit |
|---|---|---|---|--:|--:|---|
| t~pd~ | Propagation delay | A-to-Y | V~CC~ = 1.8 V ± 0.15 V | 1.3 | 5.5 | ns |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | 1 | 4.5 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | 1.1 | 4.2 | ^ |
| ^ | ^ | ^ | V~CC~ = 5 V ± 0.5 V | 1 | 3.5 | ^ |
```

## Typical characteristics

```curve id=tpd-vcc caption="t~pd~ vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [1, 6]},
 "y": {"label": "tpd", "unit": "ns", "range": [0, 5]},
 "series": [
  {"label": "tpd(MAX)", "points": [[2, 5], [2.5, 4.5], [3.3, 4.1], [5, 3.5]]},
  {"label": "tpd(MIN)", "points": [[1.8, 1.3], [2.5, 1], [3.3, 1.15], [5, 1]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 2)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

The unbuffered inverter is commonly used in oscillator circuits because
it is less sensitive to parameter changes in the oscillator circuit due
to having lower total gain than a buffered equivalent. An example
application circuit is shown in Figure 5. To learn more about how to use
an unbuffered inverter in an oscillator circuit, refer to the Use of the
CMOS Unbuffered Inverter in Oscillator Circuits application report.

Conversion note: the typical application diagram (Figure 5) shows a crystal oscillator built around the SN74LVC1GU04, with a feedback resistor R~F~, a series resistor R~S~, a crystal with load capacitance C~L~ and capacitors C~1~ and C~2~ to ground, whose output drives an SN74LVC1G14 into a capacitor C to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive also creates fast edges into light
loads, so routing and load conditions should be considered to prevent
ringing.

### Detailed design procedure

To learn more about how to use an unbuffered inverter in an oscillator
circuit, refer to the Use of the CMOS Unbuffered Inverter in Oscillator
Circuits application report.

1. Recommended Input Conditions
   - Specified high and low levels. See (V~IH~ and V~IL~) in
     Recommended Operating Conditions.
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in Recommended Operating Conditions at any valid V~CC~.
2. Absolute Maximum Output Conditions
   - Load currents must not exceed (I~O~ max) per output and must not
     exceed (Continuous current through V~CC~ or GND) total current for
     the part. These limits are located in Absolute Maximum Ratings.
   - Outputs must not be pulled above the voltage rated in the Absolute
     Maximum Ratings.

### Application curve

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "µA", "range": [0, 1600]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [75, 210]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [75, 350]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [75, 600]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 400], [40, 810], [60, 1200], [75, 1480]]}
 ]}
```

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table.

The V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended, and it is ok to parallel
multiple bypass caps to reject different frequencies of noise. 0.1-µF
and 1-µF capacitors are commonly used in parallel. The bypass capacitor
must be installed as close to the power pin as possible for best
results.

### Layout guidelines

Even low data rate digital signals can contain high-frequency signal
components due to fast edge rates. When a printed-circuit board (PCB)
trace turns a corner at a 90° angle, a reflection can occur. A
reflection occurs primarily because of the change of width of the trace.
At the apex of the turn, the trace width increases to 1.414 times the
width. This increase upsets the transmission-line characteristics,
especially the distributed capacitance and self–inductance of the trace
which results in the reflection. Not all PCB traces can be straight and
therefore some traces must turn corners. Figure 7 shows progressively
better techniques of rounding corners. Only the last example (BEST)
maintains constant trace width and minimizes reflections.

An example layout is given in Figure 8 for the DPW (X2SON-5) package.
This example layout includes a 0402 (metric) capacitor and uses the
measurements found in the example board layout appended to this end of
this datasheet. A via of diameter 0.1 mm (3.973 mil) is placed directly
in the center of the device. This via can be used to trace out the
center pin connection through another board layer, or it can be left out
of the layout

Conversion note: Figure 7 shows trace corners rated worst, better and best, and Figure 8 the DPW land pattern with a 0402 bypass capacitor beside it.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=X2SON-5-DPW
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
74LVC1GU04DCKTG4 | Reel of 250 | -40 to 125 | CD5, CDS
SN74LVC1GU04DBVR | Reel of 3000 | -40 to 125 | CU45, CU4F, CU4J, CU4R, CU4T, CU4H, CU4P, CU4S
SN74LVC1GU04DBVT | Reel of 250 | -40 to 125 | CU45, CU4F, CU4J, CU4R, CU4H, CU4P, CU4S
SN74LVC1GU04DCKR | Reel of 3000 | -40 to 125 | CD5, CDF, CDJ, CDK, CDR, CDT, CDH, CDP, CDS
SN74LVC1GU04DCKT | Reel of 250 | -40 to 125 | CD5, CDF, CDJ, CDK, CDR, CDT, CDH, CDP, CDS
SN74LVC1GU04DPWR | Reel of 3000 | -40 to 85 | C, CM
SN74LVC1GU04DRLR | Reel of 4000 | -40 to 125 | CDR
SN74LVC1GU04YZPR | Reel of 3000 | -40 to 85 | CDN
SN74LVC1GU04DRY2 | Reel of 5000 | -40 to 125 | CD
SN74LVC1GU04DRYR | Reel of 5000 | -40 to 125 | CD
SN74LVC1GU04DSFR | Reel of 5000 | -40 to 125 | CD
```

## Conversion notes

- Source pages 1 and 3–13 converted. The ordering lines are from the
  package option addendum (pp.15–17). The table of contents and revision
  history page (2), the support page (14), tape and reel (18–21),
  mechanical pages (22–48) and the notice page (49) are left out; the
  generated Package section replaces them.
- Dropped: applications list, device information table, the vendor
  pinout drawings, the NanoFree package and ESD protection feature
  lines; the logic diagram (p.1 and Figure 3), the clamp-diode figure
  (Figure 4; the shared clamp-diodes figure also draws clamps to V~CC~,
  which this part does not have), the typical application diagram
  (Figure 5) and the layout examples (Figures 7 and 8) are described in
  text. The other features are sentences in Description.
- The note "NC – No internal connection" is under both pin tables of
  the source (pp.3–4) and is given once. The DNU description is the
  legend of the YZP drawing (p.4), whose table lists balls A1 and B2 as
  NC.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. The pulse-duration,
  setup-and-hold and enable/disable waveforms of Figure 2 and their
  notes B, E and F are left out (this inverter has no output enable).
- Curves digitized from Figures 1 and 6 to about 2 % of full scale.
- The source also covers the YZV package (SN74LVC1GU04YZVR), and the
  addendum lists further E4, G4 and .B orderables; the catalog doesn't
  carry them.
- From images: overbar of Y = !{A} (p.1); ≤2 ns, ≤2.5 ns, ≤ 10 MHz,
  1 kΩ, 500 Ω, 50 Ω and V~Δ~ (p.8; the text layer prints "£2 ns",
  "1 kW", "500 W", "50 W" and "VD"); the curve points 4.1, 1.15
  (Figure 1, p.7, where the t~pd(MAX)~ line starts at the top of the
  plot), 210, 350, 600, 400, 810, 1480 (Figure 6, p.12).
- Source issues:
  - Source conflict: DRY and DSF headed 5 pins in the thermal table,
    6 pins elsewhere (Thermal information note).
  - Source conflict: Δt/Δv said to be in the Recommended Operating
    Conditions, which have no such row (Standard CMOS inputs note).
  - Source conflict: I~off~ said to be in the Electrical
    Characteristics, which have no such row (Partial power down note).
  - Source conflict: the YZP drawing (p.4) is headed "6-Pin DSBGA";
    the device information table (p.1) and the thermal table (p.6) give
    the YZP package 5 pins.
