---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g06",
  "title": "SN74LVC2G06 dual inverter buffer and driver with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2g06.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES307J",
    "date": "2015-07",
    "pages": 33
  },
  "devices": ["gate:2G06"],
  "parts": ["SN74LVC2G06DBVR", "SN74LVC2G06DCKR", "SN74LVC2G06DRYR", "SN74LVC2G06DSFR", "SN74LVC2G06YZPR"],
  "packages": {"p6": "DBV, DCK, DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G06 dual inverter buffer and driver with open-drain outputs

## Description

This dual inverter buffer and driver is designed for 1.65-V to 5.5-V
V~CC~ operation. [[1::2]]

The output of the SN74LVC2G06 device is an open-drain which can be
connected to other open-drain outputs to implement active-low, wired-OR,
or active-high wired-AND functions. The maximum sink current is 32 mA. [[1::4]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[9::1]]

NanoFree package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::6]]

Supports 5-V V~CC~ Operation. Max t~pd~ of 3.4 ns at 3.3 V. Low Power
Consumption, 10-μA Max I~CC~. ±24-mA Output Drive at 3.3 V. Typical
V~OLP~ (Output Ground Bounce) <0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C.
Typical V~OHV~ (Output V~OH~ Undershoot) >2 V at V~CC~ = 3.3 V,
T~A~ = 25°C. Inputs and Open-Drain Outputs Accept Voltages up to 5.5 V.
I~off~ Supports Live Insertion, Partial-Power-Down Mode and Back-Drive
Protection. Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II.
Supports Down-Translation (5 V to 3.3 V and 3.3 V to 1.8 V). [[1::3]]

## Pinout

Conversion note: the source draws the YZP package in bottom view, and the drawing here is generated from the catalog.

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
GND: Ground
1A: Input 1
2A: Input 2
1Y: Open-drain output 1
2Y: Open-drain output 2
VCC: Power pin
```
[[3::table #1]]

Source error: the Pin Functions table (p.3) gives the I/O type of 1Y, open-drain output 1, as I.

## Functional description

The SN74LVC2G06 dual open-drain inverter device contains one open-drain
inverter with a maximum sink current of 32 mA. This device is fully
specified for partial-power-down applications using I~off~. The I~off~
circuitry disables the outputs, preventing damaging current backflow
through the device when it is powered down. [[9::1]]

Source conflict: the overview (p.9) says the dual device contains one open-drain inverter, and the logic diagram (p.1) shows two.

Conversion note: the functional block diagram (p.9) shows one inverter with an open-drain output, A on pin 1 to Y on pin 3. Source conflict: the logic diagram (p.1) and the pin table (p.3) put 1A on pin 1, 1Y on pin 6, 2A on pin 3 and 2Y on pin 4.

### Feature description

The wide operating voltage range of 1.65 V to 5.5 V allows the
SN74LVC2G06 to be used in systems with many different voltage rails. In
addition, the voltage tolerance on the output allows the device to be
used for inverting up-translation or down-translation. The I~OFF~
feature safely allows voltage on the inputs and outputs when there's no
V~CC~ is present. [[9::2]]

### Device functional modes

Table 1 lists the functional modes of the SN74LVC2G06. [[9::3]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| L | Hi-Z |
| H | L |
```
[[9::Table 1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high or low state [2] [3] | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage Temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | +2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | +1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | | 0.3 × V~CC~ | ^ |
| V~I~ | Input voltage | | 0 | | 5.5 | V |
| V~O~ | Output voltage | | 0 | | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | | 16 | ^ |
| ^ | ^ | ^ | | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | | 10 | ^ |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -40 | | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC2G06 | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23), 6 pins | DCK (SC70), 6 pins | DRY (SON), 6 pins | YPZ (DSBGA), 6 pins [2] | DSF (SON), 6 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 165 | 259 | 234 | 123 | 300 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
[2] Source error: the table (p.5) heads this column YPZ; the DSBGA package of this data sheet is YZP (pp.1, 3). Kept as printed.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | μA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | μA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 3.5 | | | 3.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[6::table #1]]

```table id=cpd caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 2 | 2 | 3 | 4 | pF |
```
[[6::table #2]]

## Switching characteristics

Conversion note: Figure 3 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics for –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 | 7.2 | 1 | 3.9 | 1 | 3.4 | 1 | 2.9 | ns |
```
[[6::table #3]]

```table id=sw-125 caption="Switching characteristics for –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 | 8.2 | 1 | 4.4 | 1 | 3.9 | 1 | 3.4 | ns |
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="TPD across temperature at 3.3-V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 2.5]},
 "series": [
  {"label": "TPD", "points": [[-55, 1.47], [-40, 1.53], [0, 1.64], [25, 1.71], [85, 1.86], [125, 1.98]]}
 ]}
```
[[7::Figure 1]]

```curve id=tpd-vcc caption="TPD across V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[1.8, 5.5], [2.5, 3.1], [3.3, 1.7], [5.0, 2.1]]}
 ]}
```
[[7::Figure 2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators have the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. Because this device has open-drain outputs, t~PLZ~ and
t~PZL~ are the same as t~PD~. t~PZL~ is measured at V~M~. t~PLZ~ is
measured at V~OL~ + V~Δ~. All parameters and waveforms are not
applicable to all devices. [[8::1]]

Conversion note: the load circuit (open drain) has C~L~ and one R~L~ from the output to GND and a second R~L~ from the output to switch S1, which goes to V~LOAD~, open or GND.
Conversion note: the source figure also shows pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PZL~ (see Notes E and F) | V~LOAD~ |
| t~PLZ~ (see Notes E and G) | V~LOAD~ |
| t~PHZ~/t~PZH~ | V~LOAD~ |
```
[[8::table #1]]

```table id=test-conditions caption="Test conditions (Figure 3)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[8::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[8::figure #1]]

## Application information

Information in the following applications sections is not part of the TI
component specification and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[10::1]]

The SN74LVC2G06 is a dual high-drive CMOS device that implements a
high-output drive buffer, such as an LED application. This device can
sink 32 mA of current at 4.5 V making it ideal for high-drive
applications and high-speed applications up to 100 MHz. The inputs are
5.5-V tolerant and let it to translate up or down to V~CC~. The
following Typical Application shows a simple LED driver application for
a single channel of the device. [[10::2]]

Conversion note: the typical application diagram (Figure 4) shows one inverter driven from an MCU, with an LED and a resistor from its output to V~PU~.

### Design requirements

This device uses CMOS technology and has balanced output drive. Avoid
bus contention because it can drive currents that exceed maximum limits.
The high drive also creates fast edges into light loads. Consider
routing and load conditions to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions [[10::4]]
   - Rise time and fall time specs. See (Δt/ΔV) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[10::4]]
   - Specified high and low levels. See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~. [[10::4]]
2. Recommend Output Conditions [[10::4]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed (Continuous current through V~CC~ or GND) total current
     for the part. These limits are located in Absolute Maximum Ratings
     table. [[10::4]]
   - Do not pull outputs above 5.5 V. [[10::4]]

### Application curve

```curve id=icc-freq caption="I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "µA", "range": [0, 1600]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 60], [40, 115], [60, 175], [75, 220]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 95], [40, 190], [60, 285], [75, 355]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [20, 160], [35, 280], [45, 365], [60, 480], [70, 560], [75, 590]]},
  {"label": "Icc 5V", "points": [[1, 20], [20, 410], [30, 615], [35, 700], [45, 920], [50, 990], [55, 1100], [70, 1400], [75, 1500]]}
 ]}
```
[[11::Figure 5]]

### Power supply recommendations

The power supply can be any voltage between the min and max supply
voltage rating located in the Recommended Operating Conditions table. [[11::1]]

Each V~CC~ pin must have a good bypass capacitor in order to prevent
power disturbance. For devices with a single supply, a 0.1-μF capacitor
is recommended and if there are multiple V~CC~ pins then a 0.01-μF or
0.022-μF capacitor is recommended for each power pin. It is ok to
parallel multiple bypass caps to reject different frequencies of noise.
0.1-μF and 1-μF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results. [[11::2]]

### Layout guidelines

When using multiple bit logic devices, inputs must never float. In many
cases, functions or parts of functions of digital logic devices are
unused. Examples include when only two inputs of a triple input and gate
are used or when only three of the four buffer gates are used. Avoid
leaving input pins unconnected because the undefined voltages at the
outside connections result in undefined operational states. Observe the
following rules under all circumstances. Connect all unused inputs of
digital logic devices to a high or low bias to prevent them from
floating. Based on the function of the device, apply the logic level to
any unused input. Based on convenience, tie unused inputs to the GND or
the V~CC~. [[11::3]]

Conversion note: the layout recommendation (Figure 6) shows a gate with its unused input tied to V~CC~ and a gate with its unused input tied to ground.

## Package

```package footprint=SOT23-6
```

```package footprint=SC70-6
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-6-YZP
```

## Ordering information

```ordering
SN74LVC2G06DBVR | Reel of 3000 | -40 to 125 | C065, C06R
SN74LVC2G06DCKR | Reel of 3000 | -40 to 125 | CT5, CTJ, CTR
SN74LVC2G06DRYR | Reel of 5000 | -40 to 125 | CT
SN74LVC2G06DSFR | Reel of 5000 | -40 to 125 | CT
SN74LVC2G06YZPR | Reel of 3000 | -40 to 85 | CT7, CTN
```
[[13::table #1]]

## Conversion notes

- Source pages 1 and 3–12 converted. The ordering lines are from the
  package option addendum (pp.13–14). The table of contents and revision
  history page (2), tape and reel (15–16), mechanical pages (17–32) and
  the notice page (33) are left out; the generated Package section
  replaces them.
- Dropped: applications list, device information table, the vendor
  pinout drawings, the NanoFree package and ESD protection feature
  lines, the community resources, trademarks, ESD caution and glossary
  sections (p.12); the logic diagram, the functional block diagram, the
  typical application diagram (Figure 4), the layout recommendation
  (Figure 6) and the open-drain load circuit of Figure 3 are described
  in text. The other features are sentences in Description.
- Note B of Figure 3 (p.8), on the enable and disable waveforms, is
  dropped with those waveforms.
- The source's electrical characteristics table gives symbols only, so
  its Parameter cells are empty except for I~I~.
- Curves digitized from Figures 1, 2 and 5 to about 2 % of full scale.
- From images: the arrangement of the drawings described in text
  (pp.1, 8, 9, 10, 11).
- Source issues:
  - Source error: the Pin Functions table (p.3) gives the I/O type of
    1Y as I; it is an open-drain output.
  - Source conflict: the overview (p.9) says the dual device contains
    one open-drain inverter, and its block diagram shows one inverter
    from pin 1 to pin 3; the logic diagram (p.1) and the pin table (p.3)
    show two inverters, pin 1 to pin 6 and pin 3 to pin 4.
  - Source error: the Thermal Information table (p.5) heads the DSBGA
    column YPZ; the package is YZP. Kept as printed.
