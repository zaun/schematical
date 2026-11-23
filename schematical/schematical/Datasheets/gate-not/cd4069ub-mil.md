---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd4069ub-mil",
  "title": "CD4069UB CMOS hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/cd4069ub-mil.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS054E",
    "date": "2019-01",
    "pages": 34
  },
  "devices": ["gate:4069UB"],
  "parts": ["CD4069UBF3A", "CD4069UBM96", "M38510/17401BCA"],
  "packages": {"p14": "D, J, N, NS, PW"},
  "converted": "2026-11-21"
}
---

# CD4069UB CMOS hex inverter

## Description

The CD4069UB device consist of six CMOS inverter circuits. These
devices are intended for all general-purpose inverter applications
where the medium-power TTL-drive and logic-level-conversion
capabilities of circuits such as the CD4009 and CD4049 hex inverter and
buffers are not required. [[1::1]]

Standardized symmetrical output characteristics. Medium speed
operation: t~PHL~, t~PLH~ = 30 ns at 10 V (Typical). 100% Tested for
quiescent current at 20 V. Maximum input current of 1 µA at 18 V over
full package-temperature range, 100 nA at 18 V and 25°C. Meets all
requirements of JEDEC tentative standard No. 13B, Standard
Specifications for Description of B Series CMOS Devices. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
A: A input
B: B input
C: C input
D: D input
E: E input
F: F input
G: G output
H: H output
I: I output
J: J output
K: K output
L: L output
VDD: Positive supply
VSS: Negative supply
```
[[1::table #1]]

Conversion note: the source names the output pins by their equations,
G = !{A}, H = !{B}, I = !{C}, J = !{D}, K = !{E} and L = !{F}.

## Functional description

The CD4069UB device has six inverter circuits. The recommended
operating range is from 3 V to 18 V. The CD4069UB-series types are
supplied in 14-pin hermetic dual-in-line ceramic packages (F3A suffix),
14-pin dual-in-line plastic packages (E suffix), 14-pin small-outline
packages (M, MT, M96, and NSR suffixes), and 14-pin thin shrink
small-outline packages (PW and PWR suffixes). [[13::1]]

Conversion note: the functional block diagram shows six inverters, each
from an input (A to F) to its output (G = !{A} to L = !{F}), with the
pin numbers of the pinout.

CD4069UB has standardized symmetrical output characteristics and a wide
operating voltage range from 3 V to 18 V with quiescent current tested
at 20 V. This has a medium operation speed of t~PHL~, t~PLH~ = 30 ns
(typical) at 10 V. The operating temperature is from –55°C to 125°C.
CB4069B meets all requirements of JEDEC tentative standard No. 13B,
Standard Specifications for Description of B Series CMOS Devices. [[13::2]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A, B, C, D, E, F | G, H, I, J, K, L |
|:-:|:-:|
| H | L |
| L | H |
```
[[13::Table 1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~DD~ | DC supply-voltage (voltages referenced to V~SS~ terminal) | | -0.5 | 20 | V |
| V~I~ | Input voltage, all inputs | | -0.5 | V~DD~ + 0.5 | V |
| I~IK~ | DC input current, any one input | | -10 | 10 | mA |
| P~D~ | Power dissipation per package | –55°C to 100°C | | 500 | mW |
| ^ | ^ | 100°C to 125°C | 12 [3] | 200 | mW |
| ^ | Device dissipation per output transistor | Full range (all package types) | | 100 | mW |
| | Lead temperature [2] | | | 265 | °C |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] During soldering at distance 1/16 inch ± 1/32 inch (1.59 mm ± 0.79 mm) from case for 10 s maximum
[3] Unconfirmed: the source prints 12 under MIN on the 100°C to 125°C row and does not say what it is.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±500 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±200 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~DD~ | Supply voltage | 3 | 18 | V |
| T~A~ | Operating temperature | -55 | 125 | °C |
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | CD4069UB | < | < | < | < | Unit |
| ^ | ^ | D (SOIC), 14 pins | J (CDIP), 14 pins | N (PDIP), 14 pins | NS (SO), 14 pins | PW (TSSOP), 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 94.9 | | 57.9 | 91.2 | 122.1 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 56.4 | 28.5 | 45.5 | 48.8 | 50.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 49.2 | | 37.7 | 50 | 63.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 21.1 | | 30.6 | 15 | 6.3 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 48.9 | | 37.6 | 49.6 | 63.3 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics – static" conditions="T~A~ = 25°C; input t~r~, t~f~ = 20 ns; C~L~ = 50 pF; R~L~ = 200 kΩ (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| I~DD~ | Quiescent device current | V~IN~ = 0V or 5 V, V~DD~ = 5 V | T~A~ = –55°C |  |  | 0.25 | µA |
| ^ | ^ | ^ | T~A~ = –40°C |  |  | 0.25 | µA |
| ^ | ^ | ^ | T~A~ = 25°C |  | 0.01 | 0.25 | µA |
| ^ | ^ | ^ | T~A~ = 85°C |  |  | 7.5 | µA |
| ^ | ^ | ^ | T~A~ = 125°C |  |  | 7.5 | µA |
| ^ | ^ | V~IN~ = 0 or 10 V, V~DD~ = 10 V | T~A~ = –55°C |  |  | 0.5 | µA |
| ^ | ^ | ^ | T~A~ = –40°C |  |  | 0.5 | µA |
| ^ | ^ | ^ | T~A~ = 25°C |  | 0.01 | 0.5 | µA |
| ^ | ^ | ^ | T~A~ = 85°C |  |  | 15 | µA |
| ^ | ^ | ^ | T~A~ = 125°C |  |  | 15 | µA |
| ^ | ^ | V~IN~ = 0 or 15 V, V~DD~ = 15 V | T~A~ = –55°C |  |  | 1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C |  |  | 1 | µA |
| ^ | ^ | ^ | T~A~ = 25°C |  | 0.01 | 1 | µA |
| ^ | ^ | ^ | T~A~ = 85°C |  |  | 30 | µA |
| ^ | ^ | ^ | T~A~ = 125°C |  |  | 30 | µA |
| ^ | ^ | V~IN~ = 0 or 20 V, V~DD~ = 20 V | T~A~ = –55°C |  |  | 5 | µA |
| ^ | ^ | ^ | T~A~ = –40°C |  |  | 5 | µA |
| ^ | ^ | ^ | T~A~ = 25°C |  | 0.02 | 5 | µA |
| ^ | ^ | ^ | T~A~ = 85°C |  |  | 150 | µA |
| ^ | ^ | ^ | T~A~ = 125°C |  |  | 150 | µA |
| I~OL~ | Output low (sink) current | V~O~ = 0.4 V, V~IN~ = 5 V, V~DD~ = 5 V | T~A~ = –55°C | 0.64 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | 0.61 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | 0.51 | 1 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | 0.42 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | 0.36 |  |  | mA |
| ^ | ^ | V~O~ = 0.5 V, V~IN~ = 10 V, V~DD~ = 10 V | T~A~ = –55°C | 1.6 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | 1.5 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | 1.3 | 2.6 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | 1.1 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | 0.9 |  |  | mA |
| ^ | ^ | V~O~ = 1.5 V, V~IN~ = 15 V, V~DD~ = 15 V | T~A~ = –55°C | 4.2 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | 4 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | 3.4 | 6.8 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | 2.8 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | 2.4 |  |  | mA |
| I~OH~ | Output high (source) current | V~O~ = 4.6 V, V~IN~ = 0 V, V~DD~ = 5 V | T~A~ = –55°C | -0.64 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | -0.61 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | -0.51 | -1 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | -0.42 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | -0.36 [1] |  |  | mA |
| ^ | ^ | V~O~ = 2.5 V, V~IN~ = 0 V, V~DD~ = 5 V | T~A~ = –55°C | -2 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | -1.8 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | -1.6 | -3.2 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | -1.3 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | -1.15 |  |  | mA |
| ^ | ^ | V~O~ = 9.5 V, V~IN~ = 0 V, V~DD~ = 10 V | T~A~ = –55°C | -1.6 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | -1.5 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | -1.3 | -2.6 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | -1.1 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | -0.9 |  |  | mA |
| ^ | ^ | V~O~ = 13.5 V, V~IN~ = 0 V, V~DD~ = 15 V | T~A~ = –55°C | -4.2 |  |  | mA |
| ^ | ^ | ^ | T~A~ = –40°C | -4 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 25°C | -3.4 | -6.8 |  | mA |
| ^ | ^ | ^ | T~A~ = 85°C | -2.8 |  |  | mA |
| ^ | ^ | ^ | T~A~ = 125°C | -2.4 |  |  | mA |
| V~OL~ | Low-level output voltage | V~IN~ = 5 V, V~DD~ = 5 V | T~A~ = 25°C |  | 0 | 0.05 | V |
| ^ | ^ | ^ | All other temperatures |  |  | 0.05 | V |
| ^ | ^ | V~IN~ = 10 V, V~DD~ = 10 V | T~A~ = 25°C |  | 0 | 0.05 | V |
| ^ | ^ | ^ | All other temperatures |  |  | 0.05 | V |
| ^ | ^ | V~IN~ = 15 V, V~DD~ = 15 V | T~A~ = 25°C |  | 0 | 0.05 | V |
| ^ | ^ | ^ | All other temperatures |  |  | 0.05 | V |
| V~OH~ | High-level output voltage | V~IN~ = 0 V, V~DD~ = 5 V | T~A~ = 25°C | 4.95 | 5 |  | V |
| ^ | ^ | ^ | All other temperatures | 4.95 |  |  | V |
| ^ | ^ | V~IN~ = 0 V, V~DD~ = 10 V | T~A~ = 25°C | 9.95 | 10 |  | V |
| ^ | ^ | ^ | All other temperatures | 9.95 |  |  | V |
| ^ | ^ | V~IN~ = 0 V, V~DD~ = 15 V | T~A~ = 25°C | 14.95 | 15 |  | V |
| ^ | ^ | ^ | All other temperatures | 14.95 |  |  | V |
| V~IL~ | Input low voltage | V~O~ = 4.5 V, V~DD~ = 5 V, all temperatures | < | | | 1 | V |
| ^ | ^ | V~O~ = 9 V, V~DD~ = 10 V, all temperatures | < | | | 2 | V |
| ^ | ^ | V~O~ = 13.5 V, V~DD~ = 15 V, all temperatures | < | | | 2.5 | V |
| V~IH~ | Input high voltage | V~O~ = 0.5 V, V~DD~ = 5 V, all temperatures | < | 4 | | | V |
| ^ | ^ | V~O~ = 1 V, V~DD~ = 10 V, all temperatures | < | 8 | | | V |
| ^ | ^ | V~O~ = 1.5 V, V~DD~ = 15 V, all temperatures | < | 12.5 | | | V |
| I~IN~ | Input current | V~IN~ = 0 V to 18 V, V~DD~ = 18 V | T~A~ = –55°C |  |  | ±01 [2] | µA |
| ^ | ^ | ^ | T~A~ = –40°C |  |  | ±01 [2] | µA |
| ^ | ^ | ^ | T~A~ = 25°C |  | [3] | ±1 [2] | µA |
| ^ | ^ | ^ | T~A~ = 85°C |  |  | ±1 | µA |
| ^ | ^ | ^ | T~A~ = 125°C |  |  | ±1 | µA |

[1] Source error: the source prints –0..36 here (p.6); the I~OL~ row at V~DD~ = 5 V and 125°C gives 0.36.
[2] Unconfirmed: the source prints ±01 at –55°C and –40°C and ±1 at 25°C; the features give a maximum input current of 1 µA over full package-temperature range and 100 nA at 25°C.
[3] Conversion note: the source prints a typical value of ±10^–5^ µA here, which the value cell cannot hold.
```
[[5::table #1]] [[6::table #1]]

## Switching characteristics

```table id=sw caption="Electrical characteristics – dynamic" conditions="T~A~ = 25°C; input t~r~, t~f~ = 20 ns; C~L~ = 50 pF; R~L~ = 200 kΩ (unless otherwise noted)"
| Symbol | Parameter | Test conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| t~PLH~, t~PHL~ | Propagation delay time | V~DD~ (V) = 5 | | 55 | 110 | ns |
| ^ | ^ | V~DD~ (V) = 10 | | 30 | 60 | ns |
| ^ | ^ | V~DD~ (V) = 15 | | 25 | 50 | ns |
| t~THL~, t~TLH~ | Transition time | V~DD~ (V) = 5 | | 100 | 200 | ns |
| ^ | ^ | V~DD~ (V) = 10 | | 50 | 100 | ns |
| ^ | ^ | V~DD~ (V) = 15 | | 40 | 80 | ns |
| C~IN~ | Input capacitance | Any input | | 10 | 15 | pF |
```
[[5::table #2]]

## Typical characteristics

```curve id=vtc caption="Minimum and maximum voltage transfer characteristics"
{"x": {"label": "Input voltage", "unit": "V", "range": [0, 25]},
 "y": {"label": "Output voltage", "unit": "V", "range": [0, 20]},
 "series": [
   {"label": "5-V Min VDD", "points": [[0, 5], [1, 4.9], [1.4, 4.5], [1.6, 2], [1.8, 0.8], [2.2, 0.2], [2.8, 0]]},
   {"label": "5-V Max VDD", "points": [[0, 5], [2, 4.9], [2.8, 4.5], [3.3, 3.8], [3.5, 2], [3.9, 1], [5, 0.4], [6.5, 0]]},
   {"label": "10-V Min VDD", "points": [[0, 10], [2, 9.9], [2.6, 9.3], [3, 8], [3.2, 3], [3.6, 1.8], [4.5, 0.8], [6, 0.2], [7, 0]]},
   {"label": "10-V Max VDD", "points": [[0, 10], [4, 9.9], [5.5, 9.4], [6.5, 8.2], [7, 6.5], [7.2, 2.5], [7.8, 1.5], [9, 0.5], [10, 0]]},
   {"label": "15-V Min VDD", "points": [[0, 14.8], [1, 14], [2, 13], [3, 12], [4, 10.5], [4.6, 8.5], [4.8, 4], [5.5, 2.5], [7, 1.2], [9, 0.2], [10, 0]]},
   {"label": "15-V Max VDD", "points": [[0, 15], [5, 15], [7, 14.5], [9, 13.5], [10, 12.5], [10.5, 10.5], [10.7, 5], [11.3, 3.5], [13, 1.5], [15, 0.3], [16, 0]]}
 ]}
```
[[8::Figure 1]]

Conversion note: Figure 2, typical voltage transfer characteristics as a function of temperature, shows the transfer curves at 5 V, 10 V and 15 V, each at -55°C and at 125°C, the two temperatures nearly on one line, and is not converted.

Conversion note: Figure 3, typical current and voltage transfer characteristics, shows the output voltage and the supply current against input voltage at V~DD~ = 5 V, 10 V and 15 V, and is not converted.

```curve id=iol-typ caption="Typical output low (sink) current characteristics"
{"x": {"label": "Drain-to-source voltage", "unit": "V", "range": [0, 25]},
 "y": {"label": "Output low (sink) current", "unit": "mA", "range": [0, 40]},
 "series": [
   {"label": "Gate-to-source voltage = 5 V", "points": [[0, 0], [1, 1.5], [2, 2.8], [3, 3.5], [4, 3.9], [5, 4.1]]},
   {"label": "Gate-to-source voltage = 10 V", "points": [[0, 0], [1, 4], [2, 7.5], [3, 10], [4, 11.8], [5, 12.8], [7, 13.8], [10, 14.2]]},
   {"label": "Gate-to-source voltage = 15 V", "points": [[0, 0], [2, 10], [4, 16.5], [6, 21], [8, 24], [10, 25.5], [12, 26.6], [15, 27.2]]}
 ]}
```
[[8::Figure 4]]

```curve id=iol-min caption="Minimum output low (sink) current characteristics"
{"x": {"label": "Drain-to-source voltage", "unit": "V", "range": [0, 25]},
 "y": {"label": "Output low (sink) current", "unit": "mA", "range": [0, 20]},
 "series": [
   {"label": "Gate-to-source voltage = 5 V", "points": [[0, 0], [1, 0.7], [2, 1.2], [3, 1.5], [4, 1.65], [5, 1.7]]},
   {"label": "Gate-to-source voltage = 10 V", "points": [[0, 0], [1, 2], [2, 3.5], [3, 5], [4, 6], [5, 6.7], [7, 7.2], [10, 7.3]]},
   {"label": "Gate-to-source voltage = 15 V", "points": [[0, 0], [2, 5], [4, 8.8], [6, 10.8], [8, 11.6], [10, 12], [15, 12.2]]}
 ]}
```
[[8::Figure 5]]

```curve id=ioh-typ caption="Typical output high (source) current characteristics"
{"x": {"label": "Drain-to-source voltage", "unit": "V", "range": [-25, 0]},
 "y": {"label": "Output high (source) current", "unit": "mA", "range": [-40, 0]},
 "series": [
   {"label": "Gate-to-source voltage = -5 V", "points": [[-5, -4.1], [-3, -3.6], [-2, -3], [-1, -1.8], [0, 0]]},
   {"label": "Gate-to-source voltage = -10 V", "points": [[-10, -19.7], [-8, -18.6], [-6, -16.5], [-4, -13], [-2, -7.5], [-1, -4], [0, 0]]},
   {"label": "Gate-to-source voltage = -15 V", "points": [[-15, -29.5], [-12, -28.5], [-10, -27], [-8, -24.5], [-6, -21], [-4, -15.5], [-2, -8.5], [0, 0]]}
 ]}
```
[[8::Figure 6]]

```curve id=ioh-min caption="Minimum output high (source) current characteristics"
{"x": {"label": "Drain-to-source voltage", "unit": "V", "range": [-25, 0]},
 "y": {"label": "Output high (source) current", "unit": "mA", "range": [-20, 0]},
 "series": [
   {"label": "Gate-to-source voltage = -5 V", "points": [[-5, -2.3], [-3, -1.9], [-2, -1.5], [-1, -0.9], [0, 0]]},
   {"label": "Gate-to-source voltage = -10 V", "points": [[-10, -9.4], [-8, -8.8], [-6, -7.8], [-4, -6], [-2, -3.3], [0, 0]]},
   {"label": "Gate-to-source voltage = -15 V", "points": [[-15, -15], [-12, -14.3], [-10, -13.2], [-8, -11.8], [-6, -10], [-4, -7.2], [-2, -3.8], [0, 0]]}
 ]}
```
[[9::Figure 7]]

```curve id=tpd-cl caption="Typical propagation delay time vs load capacitance"
{"x": {"label": "Load capacitance", "unit": "pF", "range": [0, 150]},
 "y": {"label": "Propagation delay time", "unit": "ns", "range": [0, 120]},
 "series": [
   {"label": "VDD = 5 V", "points": [[15, 31], [100, 79]]},
   {"label": "VDD = 10 V", "points": [[15, 18], [100, 43]]},
   {"label": "VDD = 15 V", "points": [[15, 16], [100, 33]]}
 ]}
```
[[9::Figure 8]]

```curve id=tpd-vdd caption="Typical propagation delay time vs supply voltage"
{"x": {"label": "Supply voltage", "unit": "V", "range": [0, 25]},
 "y": {"label": "Propagation delay time", "unit": "ns", "range": [0, 140]},
 "series": [
   {"label": "CL = 15 pF", "points": [[3, 42], [5, 33], [7.5, 25], [10, 21], [12.5, 19], [15, 18], [18, 18]]},
   {"label": "CL = 50 pF", "points": [[3, 79], [4, 63], [5, 53], [7.5, 37], [10, 29], [12.5, 25], [15, 23.5], [18, 24]]}
 ]}
```
[[9::Figure 9]]

## Parameter measurement information

Conversion note: Figure 10, schematic diagram of one of six identical inverters, shows the input A, pin 1 (3, 5, 9, 11, 13), with protection diodes to V~DD~ and V~SS~ and a series resistor, driving a p and an n transistor whose common drain is the output G = !{A}, pin 2 (4, 6, 8, 10, 12).

Conversion note: the quiescent device current test circuit (Figure 11) shows the inputs switched together to V~DD~ or V~SS~ and the I~DD~ meter in the V~SS~ lead.

Conversion note: the noise immunity test circuit (Figure 12) shows an input switched to V~IH~ or V~IL~ and a DVM on the outputs.

Conversion note: the input leakage current test circuit (Figure 13) shows a current meter between the inputs and V~DD~ or V~SS~.

Conversion note: the dynamic electrical characteristics test circuit (Figure 14) drives one inverter from a pulse generator, t~r~ = t~f~ = 20 ns, and loads its output as below.

```figure ref=load-rl-cl caption="Dynamic electrical characteristics test circuit"
RL: 200 kΩ
CL: 50 pF
```
[[11::Figure 14]]

```figure ref=waveforms-cmos-tpd-tt caption="Dynamic electrical characteristics waveform"
VM: 50% V~DD~
```
[[11::Figure 14]]

Conversion note: the waveform shows only the inverting output, with t~PHL~, t~PLH~, t~THL~ and t~TLH~ and the 10%, 50% and 90% levels of V~DD~.

Conversion note: Figure 15, typical crystal oscillator circuit, shows 1/6 CD4069 with R~f~ from output to input, R~S~ from the output to an XTAL, and capacitors C~S~ and C~T~ from the two XTAL terminals to ground.

Conversion note: Figure 16, high-input impedance amplifier, shows 1/6 CD4069 from IN to OUT with R~f~ ≈ 10 MΩ from output to input.

Conversion note: Figure 17, typical RC oscillator circuit, shows 1/3 CD4069, two inverters in series, with C~T~ from the second output, R~T~ from the first output and R~S~ to the first input, joined at one node.

Conversion note: Figure 18, input pulse shaping circuit, shows 1/3 CD4069, two inverters in series from IN through R~S~ to OUT, with R~f~ from OUT back to the first input, and gives the switching points below.

```equation
Upper switching point: V~P~ = (R~S~ + R~f~) / R~f~ × V~DD~ / 2
Lower switching point: V~N~ = (R~f~ – R~S~) / R~f~ × V~DD~ / 2
R~f~ > R~S~
```

Conversion note: the dynamic power dissipation test circuit (Figure 19) shows a CD4069UB with a current meter, a 0.1 µF and a 500 µF capacitor in its supply lead, C~L~ on the output at pin 2 and an input driven at 10 kHz, 100 kHz, 1 MHz.

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes. Customers should validate
and test their design implementation to confirm system functionality. [[14::1]]

The CD4069UB device has a low input current of 1 µA at 18 V over full
package-temperature range and 100 nA at 18 V, 25°C. This device has a
wide operating voltage range from 3 V to 18 V and used in high voltage
applications. [[14::2]]

### Typical application

Conversion note: the CD4069UB application (Figure 20) shows one
inverter driven by a logic signal, with an LED and a resistor R in
series from V~CC~ to its output.

The CD4069UB device is the industry's highest logic inverter operating
at 18 V under recommended conditions. The lower drive capabilities
makes it suitable for driving light loads like LED and greatly reduces
chances of overshoots and undershoots. [[14::3]]

The recommended input conditions for Figure 20 includes rise time and
fall time specifications (see Δt/ΔV in Recommended Operating
Conditions) and specified high and low levels (see V~IH~ and V~IL~ in
Recommended Operating Conditions). Inputs are not overvoltage tolerant
and must be below V~CC~ level because of the presence of input clamp
diodes to V~CC~. [[14::4]]

The recommended output condition for the CD4069UB application includes
specific load currents. Load currents must be limited so as to not
exceed the total power (continuous current through V~CC~ or GND) for
the device. These limits are located in the Absolute Maximum Ratings.
Outputs must not be pulled above V~CC~. [[14::5]]

```curve id=tt-cl caption="Typical transition time vs load capacitance"
{"x": {"label": "Load capacitance", "unit": "pF", "range": [0, 140]},
 "y": {"label": "Transition time", "unit": "ns", "range": [0, 300]},
 "series": [
   {"label": "Supply voltage = 5 V", "points": [[15, 61], [100, 154]]},
   {"label": "Supply voltage = 10 V", "points": [[15, 29], [100, 80]]},
   {"label": "Supply voltage = 15 V", "points": [[15, 27], [100, 58]]}
 ]}
```
[[15::Figure 21]]

```curve id=pd-f caption="Typical dynamic power dissipation vs frequency"
{"x": {"label": "Input frequency", "unit": "kHz", "range": [10, 100000], "log": true},
 "y": {"label": "Power dissipation per inverter", "unit": "µW", "range": [10, 100000], "log": true},
 "series": [
   {"label": "VDD = 5 V (CL = 50 pF)", "points": [[10, 15], [29500, 36600]]},
   {"label": "VDD = 10 V (CL = 15 pF)", "points": [[10, 19], [34600, 72700]]},
   {"label": "VDD = 10 V (CL = 50 pF)", "points": [[10, 74], [10500, 63000]]},
   {"label": "VDD = 15 V (CL = 50 pF)", "points": [[10, 169], [5340, 87000]]}
 ]}
```
[[15::Figure 22]]

```curve id=tpd-norm caption="Variation of normalized propagation delay time (t~PHL~ and t~PLH~) with supply voltage"
{"x": {"label": "Supply voltage", "unit": "V", "range": [2, 16]},
 "y": {"label": "Normalized propagation delay time", "unit": "", "range": [0, 5]},
 "series": [
   {"label": "Normalized propagation delay time", "points": [[3, 4.5], [3.5, 3.8], [4, 3.1], [5, 2.4], [6, 1.9], [8, 1.3], [10, 1], [12, 0.82], [14, 0.72], [15, 0.7]]}
 ]}
```
[[15::Figure 23]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in Recommended Operating Conditions. [[16::1]]

Each V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, TI recommends a 0.1-μF
capacitor. If there are multiple V~CC~ pins, then TI recommends a
0.01-μF or 0.022-μF capacitor for each power pin. It is acceptable to
parallel multiple bypass capacitors to reject different frequencies of
noise. 0.1-μF and 1-μF capacitors are commonly used in parallel. The
bypass capacitor must be installed as close to the power pin as
possible for best results. [[16::2]]

### Layout

When using multiple bit logic devices, inputs must never float. [[16::3]]

In many cases, digital logic device functions or parts of these
functions are unused (for example, when only two inputs of a
triple-input and gate are used, or only 3 of the 4 buffer gates are
used). Such input pins must not be left unconnected because the
undefined voltages at the outside connections result in undefined
operational states. This rule must be observed under all circumstances
specified in the next paragraph. [[16::4]]

All unused inputs of digital logic devices must be connected to a high
or low bias to prevent them from floating. See the application note,
Implications of Slow or Floating CMOS Inputs (SCBA004), for more
information on the effects of floating inputs. The logic level must
apply to any particular unused input depending on the function of the
device. Generally, they are tied to GND or V~CC~ (whichever is
convenient). [[16::5]]

Conversion note: the layout example shows a gate with an unused input
tied to V~CC~ and a gate with an unused input tied to ground.

## Package

```package footprint=CDIP-14
```

```package footprint=SOIC-14
```

## Ordering information

```ordering
CD4069UBF3A | Tube of 25 | -55 to 125 | CD4069UBF3A | NRND
CD4069UBM96 | Reel of 2500 | -55 to 125 | CD4069UBM
M38510/17401BCA | Tube of 25 | -55 to 125 | JM38510/17401BCA | NRND
```
[[18::table #1]]

## Conversion notes

- Source pages 1 and 3–16 converted. Dropped: the table of contents and
  revision history (p.2), device and documentation support (p.17), and
  the package option addendum, tape and reel, tube and mechanical pages
  and the notice (18–34), replaced by the generated Package and
  Ordering sections. Ordering packing, temperature and marking columns
  are from the package option addendum (p.18).
- Dropped: the Device Information table and the functional diagram
  (p.1), the functional block diagram (p.13), Figures 2 and 3 (p.8),
  the test and application circuits (pp. 9–12, 14) and the layout
  example (p.16), each described in a Conversion note.
- Curves digitized from Figures 1 and 4 to 9 (pp. 8–9) and 21 to 23
  (p.15) to about 5 % of full scale. Figures 8, 21 and 22 are drawn as
  straight lines; the V~DD~ = 10 V (C~L~ = 15 pF) line of Figure 22 is
  dashed in the source.
- The static electrical characteristics table prints one unit for each
  parameter; it is written on every row. Dashes in the J (CDIP) column
  of the thermal table are empty cells.
- The I~IN~ typical value at 25°C, printed ±10^–5^ µA (p.7), is given in
  a note under the table.
- The package option addendum also lists CD4069UBE, CD4069UBEE4,
  CD4069UBM, CD4069UBMT, CD4069UBNSR, CD4069UBPW, CD4069UBPWR and
  JM38510/17401BCA, which the catalog carries under the datasheet
  cd4069ub (the same PDF), and CD4069UBF and CD4069UBPWRG4.A, which
  the catalog doesn't carry.
- From images: overbars of G = !{A}, H = !{B}, I = !{C}, J = !{D},
  K = !{E}, L = !{F} (pin drawing and Pin Functions, p.3; functional
  diagram, p.1; functional block diagram, p.13; Figure 10, p.9).
- From images: the switching-point equations of Figure 18 with their
  names V~P~ and V~N~ (p.12; the text layer has other glyphs), and
  R~f~ ≈ 10 MΩ (Figure 16, p.11).
- Source issues:
  - Source error: I~OH~ at V~O~ = 4.6 V and 125°C is printed
    "–0..36" (p.6); written -0.36.
  - Unconfirmed: the I~IN~ maximum is printed ±01 at –55°C and –40°C
    and ±1 at 25°C (p.7).
  - Unconfirmed: 12 under MIN on the power dissipation row for 100°C
    to 125°C (p.4).
  - Source error: "CB4069B" in the feature description (p.13) for
    CD4069UB, and "The CD4069UB device consist" in the Description
    (p.1), kept as printed.
  - Source conflict: the detailed design procedure (p.14) refers to
    Δt/ΔV, V~IH~ and V~IL~ in Recommended Operating Conditions; that
    table (p.4) has only V~DD~ and T~A~. The application text names
    V~CC~ and GND where the rest of the source has V~DD~ and V~SS~.
  - Unconfirmed: the static electrical characteristics table is headed
    T~A~ = 25°C although its rows give other temperatures.
