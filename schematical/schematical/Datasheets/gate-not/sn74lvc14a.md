---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc14a",
  "title": "SNx4LVC14A hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc14a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS285AC",
    "date": "2022-04",
    "pages": 41
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74LVC14AD", "SN74LVC14ADBR", "SN74LVC14ADGVR", "SN74LVC14ADRG3", "SN74LVC14ADRG4", "SN74LVC14ADT", "SN74LVC14ANSR", "SN74LVC14APWR", "SN74LVC14APWRE4", "SN74LVC14APWRG3", "SN74LVC14ARGYR"],
  "packages": {"p14": "D, DB, DGV, NS, J, W, PW, RGY"},
  "converted": "2026-11-21"
}
---

# SNx4LVC14A hex Schmitt-trigger inverters

## Description

The SN54LVC14A hex Schmitt-trigger inverter is designed for 2.7-V to
3.6-V V~CC~ operation, and the SN74LVC14A hex Schmitt-trigger inverter
is designed for 1.65-V to 3.6-V V~CC~ operation. [[1::1]]

The devices contain six independent inverters and perform the Boolean
function Y = !{A}. [[1::3]]

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of these devices as translators in a mixed 3.3-V or 5-V
system environment. [[1::4]]

Latch-up performance exceeds 100 mA per JESD 78, Class II. Operate from
1.65 V to 3.6 V V~CC~. Specified from –40°C to +85°C, –40°C to 125°C,
and –55°C to 125°C. Inputs accept voltages to 5.5 V. Max t~pd~ of 6.4 ns
at 3.3 V. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Data input
GND: Ground
VCC: Positive supply
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Data output
```
[[4::Table 5-1]]

## Functional description

Conversion note: the logic diagram, each inverter (Figure 8-1), shows
one inverter with a Schmitt-trigger symbol, input A and output Y.

### Balanced high-drive CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The high drive capability of this device creates fast edges into light
loads so routing and load conditions should be considered to prevent
ringing. Additionally, the outputs of this device are capable of driving
larger currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to over-current. The electrical and
thermal limits defined the in the Absolute Maximum Ratings section must
be followed at all times. [[11::5]]

Source error: "defined the in the" is the source's wording (Section
8.3.1, p.11); kept as printed.

### CMOS Schmitt-trigger inputs

This device includes inputs with the Schmitt-trigger architecture. These
inputs are high impedance and are typically modeled as a resistor in
parallel with the input capacitance given in the Electrical
Characteristics table from the input to ground. The worst case
resistance is calculated with the maximum input voltage, given in the
Absolute Maximum Ratings table, and the maximum input leakage current,
given in the Electrical Characteristics table, using Ohm's law
(R = V ÷ I). [[11::6]]

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs with slow
transitioning signals will increase dynamic current consumption of the
device. [[11::7]]

### Clamp diodes

The inputs to this device have negative clamping diodes. The outputs to
this device have both positive and negative clamping diodes as shown in
Figure 8-2. [[11::8]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[11::9]]

Conversion note: Figure 8-2, electrical placement of clamping diodes for
each input and output, shows one diode from GND to the input (-I~IK~),
one from GND to the output (-I~OK~) and one from the output to V~CC~
(+I~OK~).

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Absolute Maximum Ratings table. [[12::2]]

## Function tables

Table 8-1 lists the functional modes for the SN54LVC14A and SN74LVC14A
devices. [[12::3]]

```table id=function caption="Function table (each inverter)"
| Input A | Output Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[12::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| P~tot~ | Power dissipation | T~A~ = –40°C to +125°C [4] [5] | | 500 | mW |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
[4] For the D package: above 70°C, the value of P~tot~ derates linearly with 8 mW/K.
[5] For the DB, DGV, NS, and PW packages: above 60°C, the value of P~tot~ derates linearly with 5.5 mW/K.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | +2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | +1000 | V |
| ^ | ^ | Machine Model | 200 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc-54 caption="Recommended operating conditions: SN54LVC14A" conditions="See [1]"
| Symbol | Parameter | Conditions | SN54LVC14A | < | Unit |
| ^ | ^ | ^ | –55 to +125°C | < | ^ |
| ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[5::table #3]]

```table id=roc-74 caption="Recommended operating conditions: SN74LVC14A" conditions="See [1]"
| Symbol | Parameter | Conditions | SN74LVC14A | < | < | < | < | < | Unit |
| ^ | ^ | ^ | T~A~ = 25°C | < | –40 to +85°C | < | –40 to +125°C | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | 1.65 | 3.6 | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | 1.5 | | 1.5 | | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | | -4 | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | | -8 | | -8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | | -12 | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | | 8 | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | | 12 | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | | 24 | | 24 | mA |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[6::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74LVC14A | < | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB (SSOP) 14 pins | DGV (TVSOP) 14 pins | NS (SO) 14 pins | PW (TSSOP) 14 pins | RGY (LCCC) 20 pins [2] | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 117.6 | 131.8 | 153.5 | 115.7 | 145.9 | 93.8 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.2 | 83.9 | 75.2 | 72.2 | 73.4 | 106.7 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 71.9 | 79.2 | 86.6 | 74.4 | 87.7 | 69.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 39.3 | 41.7 | 19.9 | 33.7 | 18.9 | 22.0 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 71.6 | 78.6 | 85.9 | 74.1 | 87.1 | 70.0 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | 49.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
[2] Source conflict: the thermal table (p.6) heads the RGY column "(LCCC)" and "20 PINS"; the device information table (p.1), Figure 5-2 (p.4) and the package option addendum (p.18) give RGY as a 14-pin VQFN.
```
[[6::table #2]]

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics, SN54LVC14A" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | SN54LVC14A | < | < | Unit |
| ^ | ^ | ^ | ^ | –55 to +125°C | < | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | | 2.7 V | 0.8 | | 2 | V |
| ^ | ^ | | 3 V | 0.9 | | 2 | V |
| ^ | ^ | | 3.6 V | 1.1 | | 2 | V |
| V~T–~ | Negative-going threshold | | 2.7 V | 0.4 | | 1.4 | V |
| ^ | ^ | | 3 V | 0.6 | | 1.5 | V |
| ^ | ^ | | 3.6 V | 0.8 | | 1.7 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T-~) | | 2.7 V | 0.3 | | 1.1 | V |
| ^ | ^ | | 3 V | 0.3 | | 1.2 | V |
| ^ | ^ | | 3.6 V | 0.3 | | 1.2 | V |
| V~OH~ | | I~OH~ = –100 μA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | | I~OH~ = –12 mA [2] | 2.7 V | 2.2 | | | V |
| ^ | | ^ | 3 V [2] | 2.4 | | | V |
| ^ | | I~OH~ = –24 mA [2] | 3 V | 2.2 | | | V |
| V~OL~ [2] | | I~OL~ = 100 μA | 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | | I~OL~ = 12 mA [2] | 2.7 V | | | 0.4 | V |
| ^ | | I~OL~ = 24 mA | 3 V | | | 0.55 | V |
| I~I~ [2] | | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | μA |
| I~CC~ [2] | | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | μA |
| ΔI~CC~ [2] | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | μA |
| C~i~ [2] | | V~I~ = V~CC~ or GND | 3.3 V | | 5 [1] | | pF |

[1] T~A~ = 25°C
[2] Source error: the continued table (p.7) prints V~OL~, I~I~, I~CC~, ΔI~CC~ and C~i~ in the wrong cells (V~OL~ and I~CC~ as test conditions of the V~OH~ rows, I~I~ as a V~CC~, ΔI~CC~ as the symbol of the V~OL~ rows, C~i~ as a test condition) and no symbol on the last four rows; the SN74LVC14A table (p.8) has the same rows with I~OH~ = –12 mA at 2.7 V and 3 V, I~OH~ = –24 mA at 3 V, I~OL~ = 12 mA at 2.7 V and the symbols V~OL~, I~I~, I~CC~, ΔI~CC~ and C~i~, so the marked cells are written as there.
```
[[7::table #1]]

```table id=ec-74 caption="Electrical characteristics, SN74LVC14A" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | SN74LVC14A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | T~A~ = 25°C | < | < | –40 to +85°C | < | –40 to +125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | | 1.65 V | 0.4 | | 1.3 | 0.4 | 1.3 | 0.4 | 1.3 | V |
| ^ | ^ | | 1.95 V | 0.6 | | 1.5 | 0.6 | 1.5 | 0.6 | 1.5 | V |
| ^ | ^ | | 2.3 V | 0.8 | | 1.7 | 0.8 | 1.7 | 0.8 | 1.7 | V |
| ^ | ^ | | 2.5 V | 0.8 | | 1.7 | 0.8 | 1.7 | 0.8 | 1.7 | V |
| ^ | ^ | | 2.7 V | 0.8 | | 2 | 0.8 | 2 | 0.8 | 2 | V |
| ^ | ^ | | 3 V | 0.9 | | 2 | 0.9 | 2 | 0.9 | 2 | V |
| ^ | ^ | | 3.6 V | 1.1 | | 2 | 1.1 | 2 | 1.1 | 2 | V |
| V~T–~ | Negative-going threshold | | 1.65 V | 0.15 | | 0.85 | 0.15 | 0.85 | 0.15 | 0.85 | V |
| ^ | ^ | | 1.95 V | 0.25 | | 0.95 | 0.25 | 0.95 | 0.25 | 0.95 | V |
| ^ | ^ | | 2.3 V | 0.4 | | 1.2 | 0.4 | 1.2 | 0.4 | 1.2 | V |
| ^ | ^ | | 2.5 V | 0.4 | | 1.2 | 0.4 | 1.2 | 0.4 | 1.2 | V |
| ^ | ^ | | 2.7 V | 0.4 | | 1.4 | 0.4 | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 3 V | 0.6 | | 1.5 | 0.6 | 1.5 | 0.6 | 1.5 | V |
| ^ | ^ | | 3.6 V | 0.8 | | 1.7 | 0.8 | 1.7 | 0.8 | 1.7 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T-~) | | 1.65 V | 0.1 | | 1.15 | 0.1 | 1.15 | 0.1 | 1.15 | V |
| ^ | ^ | | 1.95 V | 0.15 | | 1.25 | 0.15 | 1.25 | 0.15 | 1.25 | V |
| ^ | ^ | | 2.3 V | 0.25 | | 1.3 | 0.25 | 1.3 | 0.25 | 1.3 | V |
| ^ | ^ | | 2.5 V | 0.25 | | 1.3 | 0.25 | 1.3 | 0.25 | 1.3 | V |
| ^ | ^ | | 2.7 V | 0.3 | | 1.1 | 0.3 | 1.1 | 0.3 | 1.1 | V |
| ^ | ^ | | 3 V | 0.3 | | 1.2 | 0.3 | 1.2 | 0.3 | 1.2 | V |
| ^ | ^ | | 3.6 V | 0.3 | | 1.2 | 0.3 | 1.2 | 0.3 | 1.2 | V |
| V~OH~ | | I~OH~ = –100 μA | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | | I~OH~ = –4 mA | 1.65 V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | | I~OH~ = –8 mA | 2.3 V | 1.9 | | | 1.7 | | 1.65 | | V |
| ^ | | I~OH~ = –12 mA | 2.7 V | 2.2 | | | 2.2 | | 2.05 | | V |
| ^ | | ^ | 3 V | 2.4 | | | 2.4 | | 2.25 | | V |
| ^ | | I~OH~ = –24 mA | 3 V | 2.3 | | | 2.2 | | 2 | | V |
| V~OL~ | | I~OL~ = 100 μA | 1.65 V to 3.6 V | | | 0.1 | | 0.2 | | 0.3 | V |
| ^ | | I~OL~ = 4 mA | 1.65 V | | | 0.24 | | 0.45 | | 0.6 | V |
| ^ | | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | 0.7 | | 0.75 | V |
| ^ | | I~OL~ = 12 mA | 2.7 V | | | 0.4 | | 0.4 | | 0.6 | V |
| ^ | | I~OL~ = 24 mA | 3 V | | | 0.55 | | 0.55 | | 0.8 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 3.6 V | | | ±1 | | ±5 | | ±20 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 1 | | 10 | | 40 | μA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | | 500 | | 5000 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | | pF |
```
[[7::table #2]] [[8::table #1]]

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 11 | 12 | 15 | pF |
```
[[9::table #1]]

## Switching characteristics

```table id=sw-54 caption="Switching characteristics, SN54LVC14A" conditions="Over operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | V~CC~ | SN54LVC14A | < | Unit |
| ^ | ^ | ^ | ^ | –55 to +125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A | Y | 2.7 V | | 7.5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 6.4 | ns |
```
[[8::table #2]]

```table id=sw-74 caption="Switching characteristics, SN74LVC14A" conditions="Over operating free-air temperature range (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | V~CC~ | SN74LVC14A | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | ^ | T~A~ = 25°C | < | < | –40 to +85°C | < | –40 to +125°C | < | ^ |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 V ± 0.15 V | 1 | 5 | 10.5 | 1 | 11 | 1 | 13 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.4 | 7.3 | 1 | 7.8 | 1 | 10 | ns |
| ^ | ^ | ^ | 2.7 V | 1 | 3.6 | 7.3 | 1 | 7.5 | 1 | 9.5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.2 | 6.2 | 1 | 6.4 | 1 | 8 | ns |
| t~sk(o)~ | | | 3.3 V ± 0.3 V | | | 1 | | 1 | | 1.5 | ns |
```
[[9::table #2]]

## Typical characteristics

```curve id=cpd-vcc caption="Power dissipation capacitance vs. power supply voltage"
{"x": {"label": "Power Supply Voltage (VCC)", "unit": "V", "range": [1.5, 3.5]},
 "y": {"label": "Power Dissipation Capacitance (Cpd)", "unit": "pF", "range": [10, 16]},
 "series": [
  {"label": "Cpd", "points": [[1.8, 11], [2.5, 12], [3.3, 15]]}
 ]}
```
[[9::Figure 6-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[10::2]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: Figure 7-1 also shows pulse-duration, setup-and-hold
and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[10::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[10::Figure 7-1]]

```table id=test-conditions caption="Test conditions (Figure 7-1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```
[[10::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[10::Figure 7-1]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[13::1]]

Physically interactive interface elements like push buttons or rotary
knobs offer simple and easy ways to interact with an electronic system.
Many of these physical interface elements often have issues with
bouncing, or where the physical conductive contact can connect and
disconnect multiple times during a button push or release. This bouncing
can cause one or more faulty transient signals to be passed during this
transitional period. To mitigate these faulty signals, use a
Schmitt-trigger, or a device with hysteresis, to remove these faulty
signals. Hysteresis allows a device to remember its history, and in this
case, the LVC14A uses this memory to debounce the physical element's
signal, or filter the faulty transient signals and pass only the valid
signal each time the element is used. In this example, we show a push
button signal passed through an LVC14A that is debounced and inverted to
the MCU for push detection. [[13::2]]

### Typical application

The signal effects of the debounce circuit can be seen when comparing
Figure 9-2 and Figure 9-3. In Figure 9-2, the input is a very poor
quality signal due to the error in the physical push button. If the MCU
attempts to sample this input to detect a push, there is high
probability that multiple push events will be falsely detected. Once the
debounce circuit has been implemented, the input is cleaned up, and the
MCU can perform push detection without any error, as seen in Figure 9-3. [[13::3]]

Conversion note: the debouncer application diagram (Figure 9-1) shows a
push button on input 1A of an LVC14A supplied from V~CC~ = 3.3 V, and
output 1Y going to an MCU (MSP43x).

### Design requirements

The SN74LVC14A device uses CMOS technology and has balanced output
drive. Take care to avoid bus contention because it can drive currents
that would exceed maximum limits. [[13::4]]

The SN74LVC14A allows for performing logical Boolean functions with
hysteresis using digital signals. All input signals should remain as
close as possible to either 0 V or V~CC~ for optimal operation. [[13::5]]

### Detailed design procedure

1. Recommended input conditions: [[14::1]]
   - For specified high and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions: SN74LVC14A table. [[14::1]]
   - Inputs and outputs are overvoltage tolerant and can therefore go as
     high as 3.6 V at any valid V~CC~. [[14::1]]
2. Recommended output conditions: [[14::1]]
   - Load currents should not exceed ±50 mA. [[14::1]]
3. Frequency selection criterion: [[14::1]]
   - Added trace resistance and capacitance can reduce maximum frequency
     capability; follow the layout practices listed in the Layout
     section [[14::1]]

### Application curves

Conversion note: Figure 9-2 (input response without debounce circuit)
plots V~BUT~ against t rising, dropping and recovering twice before it
settles high, and Figure 9-3 (input response with debounce circuit)
plots it rising once, smoothly.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating listed in the Absolute Maximum Ratings table. [[14::4]]

Each V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1-µF bypass capacitor
is recommended. If multiple pins are labeled V~CC~, then a 0.01-µF or
0.022-µF capacitor is recommended for each V~CC~ because the V~CC~ pins
are tied together internally. To reject different frequencies of noise,
use multiple bypass capacitors in parallel. Capacitors with values of
0.1 µF and 1 µF are commonly used in parallel. The bypass capacitor
should be installed as close to the power terminal as possible for best
results. [[14::5]]

### Layout guidelines

When using multiple-bit logic devices, inputs must never float. [[15::1]]

In many cases, functions (or parts of functions) of digital logic
devices are unused, for example, when only two inputs of a triple-input
AND gate are used or when only 3 of the 4 buffer gates are used. Such
input pins must not be left unconnected, because the undefined voltages
at the outside connections result in undefined operational states.
Figure 11-1 specifies the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that must be applied to any particular unused input depends on the
function of the device. Generally they are tied to GND or V~CC~,
whichever makes more sense or is more convenient. It is generally
acceptable to float outputs, unless the part is a transceiver. [[15::2]]

Conversion note: the layout diagrams (Figure 11-1) show two gates, one
with its unused input tied to V~CC~ and one with its unused input tied
to ground.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74LVC14AD | Tube of 50 | -40 to 125 | LVC14A
SN74LVC14ADBR | Reel of 2000 | -40 to 125 | LC14A
SN74LVC14ADGVR | Reel of 2000 | -40 to 125 | LC14A
SN74LVC14ADRG3 | Reel of 2500 | -40 to 125 | LVC14A
SN74LVC14ADRG4 | Reel of 2500 | -40 to 125 | LVC14A
SN74LVC14ADT | Reel of 250 | -40 to 125 | LVC14A
SN74LVC14ANSR | Reel of 2000 | -40 to 125 | LVC14A
SN74LVC14APWR | Reel of 2000 | -40 to 125 | LC14A
SN74LVC14APWRE4 | Reel of 2000 | -40 to 125 | LC14A
SN74LVC14APWRG3 | Reel of 2000 | -40 to 125 | LC14A
SN74LVC14ARGYR | Reel of 3000 | -40 to 125 | LC14A
```
[[21::table #1]]

## Conversion notes

- Source pages 1 and 4–15 converted. The ordering lines are from the
  package option addendum (pp.17–18). The table of contents and revision
  history (pp.2–3), the support page (16), the rest of the addendum
  (pp.19–20), tape and reel and mechanical pages (21–41) are left out;
  the generated Package section replaces them.
- Dropped: features list (the latch-up, supply, temperature,
  input-voltage, t~pd~ and MIL-PRF-38535 lines are kept in Description;
  its ESD lines are in the ESD ratings table), applications list, device
  information table, the vendor pinout drawings, the logic diagram
  (pp.1 and 11, described in one sentence), the overview (p.11), which
  repeats the description, the reference to Understanding Schmitt
  Triggers (p.11), the television-remote example and the dual-supply and
  transceiver sentences of the application chapters (pp.13–15); the
  application diagram, application curves and layout diagrams are
  described in one sentence each.
- Not claimed here: the other orderables of the package option addendum
  (SN54 and 5962 parts, SN74LVC14ADR, SN74LVC14APW, SN74LVC14APWT and
  further E4/G4 variants). The FK (LCCC, 20) pin column of Table 5-1 and
  its NC pins are left out: the catalog has no 20-pin pinout for this
  device.
- The electrical characteristics tables of the source give no parameter
  name for V~OH~, V~OL~, I~I~, I~CC~, ΔI~CC~ and C~i~, so those
  Parameter cells are empty.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 7-1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- The clamp diode figure (Figure 8-2, p.12) has no diode from the input
  to V~CC~, so the shared clamp-diodes figure is not used.
- Figure 6-1 (p.9) is digitized to about 2 % of full scale; its axes
  print no units, V and pF are from the operating characteristics table.
- From images: overbars of Y = !{A} (p.1); Ω (Z~O~ = 50 Ω, 1 kΩ and
  500 Ω in Figure 7-1, p.10; the text layer has "W"); V~Δ~ (Figure 7-1,
  p.10; the text layer has "VD"); the cell positions of the SN54LVC14A
  electrical characteristics (p.7).
- Source issues:
  - Source conflict: the description says SN54LVC14A is designed for
    2.7-V to 3.6-V V~CC~ operation (p.1), while the SN54LVC14A
    recommended operating conditions (p.5) give an operating V~CC~
    minimum of 2.
  - Source conflict: the thermal table (p.6) heads the RGY column
    "(LCCC)" and "20 PINS"; elsewhere RGY is a 14-pin VQFN (thermal
    table note).
  - Source error: the SN54LVC14A electrical characteristics (p.7) print
    several symbols in the wrong cells; written as in the SN74LVC14A
    table (electrical characteristics note).
  - Source error: "defined the in the" (Section 8.3.1, p.11), kept as
    printed.
  - Unconfirmed: the detailed design procedure (p.14) refers to V~IH~
    and V~IL~ in the Recommended Operating Conditions: SN74LVC14A table,
    which has no such rows, and says inputs and outputs can go as high
    as 3.6 V, while the recommended input voltage maximum is 5.5 V.
  - Unconfirmed: the power supply recommendations (p.14) refer to the
    supply voltage rating listed in the Absolute Maximum Ratings table.
  - Unconfirmed: the SN74LVC14A electrical characteristics (p.8) give
    V~OH~ at I~OH~ = –8 mA as 1.65 V minimum at –40 to +125°C and V~OL~
    at I~OL~ = 8 mA as 0.75 V maximum, and a hysteresis maximum at 2.7 V
    (1.1) below that at 2.5 V (1.3).
