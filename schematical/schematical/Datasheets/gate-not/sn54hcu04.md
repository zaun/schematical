---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn54hcu04",
  "title": "SN54HCU04, SN74HCU04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn54hcu04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS079E",
    "date": "2004-03",
    "pages": 23
  },
  "devices": ["gate:74_U04"],
  "parts": ["86010012A", "SN54HCU04J", "SN74HCU04NE4", "SN74HCU04PWRG4", "SNJ54HCU04FK", "SNJ54HCU04J"],
  "packages": {"p14": "D, DB, N, NS, PW, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SN54HCU04, SN74HCU04 hex inverters

## Description

The ’HCU04 devices contain six independent inverters. They perform the
Boolean function Y = !{A} in positive logic. [[1::3]]

Wide Operating Voltage Range of 2 V to 6 V. Outputs Can Drive Up To 10
LSTTL Loads. Unbuffered Outputs. [[1::1]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output
GND: Ground
VCC: Supply voltage
NC: No internal connection
```
[[1::table #1]]

## Functional description

Conversion note: the logic diagram (positive logic) (p.2) shows one inverter, input A, output Y.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[2::table #1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| I~IK~ | Input clamp current (see Note 1) [2] | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ | Output clamp current (see Note 1) [2] | (V~O~ < 0 or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance (see Note 2) [3] | D package | | 86 | °C/W |
| ^ | ^ | DB package | | 96 | °C/W |
| ^ | ^ | N package | | 80 | °C/W |
| ^ | ^ | NS package | | 76 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions (see Note 3) [1]"
| Symbol | Parameter | Conditions | SN54HCU04 | < | < | SN74HCU04 | < | < | Unit |
| ^ | ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5 | 6 | 2 | 5 | 6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.7 | | | 1.7 | | | V |
| ^ | ^ | V~CC~ = 4.5 V | 3.6 | | | 3.6 | | | ^ |
| ^ | ^ | V~CC~ = 6 V | 4.8 | | | 4.8 | | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | | 0.5 | | | 0.5 | V |
| ^ | ^ | V~CC~ = 4.5 V | | | 1.35 | | | 1.35 | ^ |
| ^ | ^ | V~CC~ = 6 V | | | 1.8 | | | 1.8 | ^ |
| V~I~ | Input voltage | | 0 | | V~CC~ | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | 0 | | V~CC~ | V |
| T~A~ | Operating free-air temperature | | -55 | | 125 | -40 | | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | T~A~ = 25°C | < | < | SN54HCU04 | < | SN74HCU04 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | V~I~ = V~CC~ or GND | I~OH~ = -20 µA | 2 V | 1.8 | | | 1.8 | | 1.8 | | V |
| ^ | ^ | ^ | 4.5 V | 4 | | | 4 | | 4 | | ^ |
| ^ | ^ | ^ | 6 V | 5.5 | | | 5.5 | | 5.5 | | ^ |
| ^ | ^ | I~OH~ = -4 mA | 4.5 V | 3.98 | | | 3.7 | | 3.84 | | ^ |
| ^ | ^ | I~OH~ = -5.2 mA | 6 V | 5.48 | | | 5.2 | | 5.34 | | ^ |
| V~OL~ | V~I~ = V~CC~ or GND | I~OL~ = 20 µA | 2 V | | | 0.2 | | 0.2 | | 0.2 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.5 | | 0.5 | | 0.5 | ^ |
| ^ | ^ | ^ | 6 V | | | 0.5 | | 0.5 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 4.5 V | | | 0.26 | | 0.4 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 5.2 mA | 6 V | | | 0.26 | | 0.4 | | 0.33 | ^ |
| I~I~ | V~I~ = V~CC~ or 0 | < | 6 V | | | ±100 | | ±1000 | | ±1000 | nA |
| I~CC~ | V~I~ = V~CC~ or 0, I~O~ = 0 | < | 6 V | | | 2 | | 40 | | 20 | µA |
| C~i~ | | < | 2 V to 6 V | | 3 | 10 | | 10 | | 10 | pF |
```
[[3::table #1]]

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per inverter | No load | 20 | pF |
```
[[3::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 50 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | SN54HCU04 | < | SN74HCU04 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 2 V | | 40 | 80 | | 120 | | 100 | ns |
| ^ | ^ | ^ | 4.5 V | | 8 | 16 | | 24 | | 20 | ^ |
| ^ | ^ | ^ | 6 V | | 7 | 14 | | 20 | | 17 | ^ |
| t~f~ | | Y | 2 V | | 38 | 75 | | 110 | | 95 | ns |
| ^ | ^ | ^ | 4.5 V | | 8 | 15 | | 22 | | 19 | ^ |
| ^ | ^ | ^ | 6 V | | 6 | 13 | | 19 | | 16 | ^ |
```
[[3::table #3]]

## Parameter measurement information

```figure ref=load-cmos-50pf caption="Figure 1: load circuit"
CL: 50 pF
```
[[4::Figure 1]]

```figure ref=waveforms-cmos-tpd-tt caption="Figure 1: voltage waveforms, propagation delay and output transition times"
VM: 50%
```
[[4::Figure 1]]

t~r~ is not applicable to ’HCU devices. [[4::1]]

C~L~ includes probe and test-fixture capacitance. Phase relationships
between waveforms were chosen arbitrarily. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ = 6 ns, t~f~ = 6 ns. The outputs are measured one at
a time, with one input transition per measurement. t~PLH~ and t~PHL~
are the same as t~pd~. [[4::1]]

Conversion note: in Figure 1 the input swings from 0 V to V~CC~, the voltage waveform of the input rise and fall times marks t~r~ and t~f~ between the 10% and 90% points, and the output rise time t~r~ carries the † of the sentence above.

## Package

```package footprint=CDIP-14
```

```package footprint=LCCC-20
```

```package footprint=PDIP-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
86010012A | Tube of 55 | -55 to 125 | 86010012A SNJ54HCU04FK
SN54HCU04J | Tube of 25 | -55 to 125 | SN54HCU04J
SN74HCU04NE4 | Tube of 25 | -40 to 85 | SN74HCU04N
SN74HCU04PWRG4 | Reel of 2000 | -40 to 85 | HCU04
SNJ54HCU04FK | Tube of 55 | -55 to 125 | 86010012A SNJ54HCU04FK
SNJ54HCU04J | Tube of 25 | -55 to 125 | 8601001CA SNJ54HCU04J
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum (p.5–7),
  tape and reel, tube and mechanical pages and the notice (8–23)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are from the package option addendum (p.5).
- Dropped: the feature list (three features are kept as sentences in
  the Description), the ordering information table of p.1, the vendor's
  pin drawings (p.1). The logic diagram is described in text.
- The source has no pin functions table; the pin descriptions are the
  converter's.
- The package option addendum also lists 8601001CA, SN74HCU04D,
  SN74HCU04DR, SN74HCU04DT, SN74HCU04N, SN74HCU04NSR, SN74HCU04PW,
  SN74HCU04PWR and SN74HCU04PWT; the catalog carries them under the
  datasheet sn74hcu04, which is the same PDF. The ordering information
  table of p.1 also lists SN74HCU04DBR and SNJ54HCU04W, which the
  package option addendum does not list.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column. The absolute maximum ratings
  print their conditions in parentheses.
- From images: overbar of Y = !{A} (p.1).
- Source issues:
  - Unconfirmed: the switching characteristics (p.3) name the output
    transition time t~f~, and Figure 1 (p.4) says t~r~ is not
    applicable to ’HCU devices; kept as printed.
