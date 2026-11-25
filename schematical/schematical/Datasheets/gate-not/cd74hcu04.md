---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74hcu04",
  "title": "CD74HCU04 high-speed CMOS logic hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/cd74hcu04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS127D",
    "date": "2004-05",
    "pages": 18
  },
  "devices": ["gate:74_U04"],
  "parts": ["CD74HCU04E", "CD74HCU04M", "CD74HCU04M96", "CD74HCU04PWR"],
  "packages": {"p14": "N, D, PW"},
  "converted": "2026-11-21"
}
---

# CD74HCU04 high-speed CMOS logic hex inverter

## Description

The CD74HCU04 unbuffered hex inverter utilizes silicon-gate CMOS
technology to achieve operation speeds similar to LSTTL gates, with the
low power consumption of standard CMOS integrated circuits. These
devices especially are useful in crystal oscillator and analog
applications. [[1::3]]

Typical Propagation Delay: 6ns at V~CC~ = 5V, C~L~ = 15pF, T~A~ = 25°C,
Fastest Part in QMOS Line. Wide Operating Temperature Range . . . -55°C
to 125°C. Balanced Propagation Delay and Transition Times. Significant
Power Reduction Compared to LSTTL Logic ICs. 2-V to 6-V Operation. High
Noise Immunity: N~IL~ = 20%, N~IH~ = 30% of V~CC~ at V~CC~ = 5V. CMOS
Input Compatibility, I~l~ ≤ 1µA at V~OL~, V~OH~. [[1::2]] [[1::3]] [[1::4]] [[1::5]]

Source conflict: the features (p.1) give the typical propagation delay at V~CC~ = 5V, C~L~ = 15pF as 6ns; the switching specifications (p.4) give 5 ns.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output
GND: Ground
VCC: Supply voltage
```
[[1::table #1]]

## Functional description

Conversion note: the functional diagram (p.2) shows six inverters, each from nA to nY, on the pins of the pinout.

Conversion note: the schematic diagram (p.2) shows one inverter as a single pair of complementary transistors between V~CC~ and ground, with the input on pin 1 (3, 5, 9, 11, 13) and the output on pin 2 (4, 6, 8, 10, 12).

## Function tables

Conversion note: the source prints no function table; its logic symbol (p.2) is an inverter from nA to nY.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | DC Supply Voltage | Voltages Referenced to Ground | -0.5 | +7 | V |
| I~IK~ | DC Input Diode Current | For V~I~ < -0.5V or V~I~ > V~CC~ + 0.5V | | ±20 | mA |
| I~OK~ | DC Output Diode Current | For V~O~ < -0.5V or V~O~ > V~CC~ + 0.5V | | ±20 | mA |
| I~O~ | DC Drain Current, per Output | For V~O~ > -0.5V or V~O~ < V~CC~ + 0.5V [2] | | ±25 | mA |
| I~CC~ | DC V~CC~ or Ground Current | | | ±50 | mA |

[1] CAUTION: Stresses above those listed in “Absolute Maximum Ratings” may cause permanent damage to the device. This is a stress only rating, and operation of the device at these or any other conditions above those indicated in the operational sections of this specification is not implied.
[2] Unconfirmed: the source gives this condition as printed, "For V~O~ > -0.5V or V~O~ < V~CC~ + 0.5V", which every output voltage meets.
```
[[3::table #1]]

## Recommended operating conditions

```table id=roc caption="Operating conditions"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| T~A~ | Temperature Range | -55 | 125 | °C |
| V~CC~ | Supply Voltage Range | 2 | 6 | V |
| V~I~, V~O~ | DC Input or Output Voltage | 0 | V~CC~ | V |
```
[[3::table #2]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| θ~JA~ | Thermal Resistance (Typical, Note 1) [1] | E (PDIP) Package | | 80 | | °C/W |
| ^ | ^ | M (SOIC) Package | | 86 | | °C/W |
| ^ | ^ | PW (TSSOP) Package | | 113 | | °C/W |
| | Maximum Junction Temperature (Hermetic Package or Die) | | | | 175 | °C |
| | Maximum Junction Temperature (Plastic Package) | | | | 150 | °C |
| | Maximum Storage Temperature Range | | -65 | | 150 | °C |
| | Maximum Lead Temperature (Soldering 10s) | (SOIC - Lead Tips Only) | | | 300 | °C |

[1] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[3::table #3]]

## Electrical characteristics

```table id=dc caption="DC electrical specifications"
| Symbol | Parameter | Test conditions | < | V~CC~ (V) | 25°C | < | -40°C to +85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | V~I~ (V) | I~O~ (mA) | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IH~ | High Level Input Voltage | | | 2 | 1.7 | | 1.7 | | 1.7 | | V |
| ^ | ^ | | | 4.5 | 3.6 | | 3.6 | | 3.6 | | V |
| ^ | ^ | | | 6 | 4.8 | | 4.8 | | 4.8 | | V |
| V~IL~ | Low Level Input Voltage | | | 2 | | 0.3 | | 0.3 | | 0.3 | V |
| ^ | ^ | | | 4.5 | | 0.8 | | 0.8 | | 0.8 | V |
| ^ | ^ | | | 6 | | 1.1 | | 1.1 | | 1.1 | V |
| V~OH~ | High Level Output Voltage CMOS Loads | V~IH~ or V~IL~ | -0.02 | 2 | 1.8 | | 1.8 | | 1.8 | | V |
| ^ | ^ | ^ | -0.02 | 4.5 | 4 | | 4 | | 4 | | V |
| ^ | ^ | ^ | -0.02 | 6 | 5.5 | | 5.5 | | 5.5 | | V |
| ^ | High Level Output Voltage TTL Loads | V~CC~ or GND | -4 | 4.5 | 3.98 | | 3.84 | | 3.7 | | V |
| ^ | ^ | ^ | -5.2 | 6 | 5.48 | | 5.34 | | 5.2 | | V |
| V~OL~ | Low Level Output Voltage CMOS Loads | V~IH~ or V~IL~ | 0.02 | 2 | | 0.2 | | 0.2 | | 0.2 | V |
| ^ | ^ | ^ | 0.02 | 4.5 | | 0.5 | | 0.5 | | 0.5 | V |
| ^ | ^ | ^ | 0.02 | 6 | | 0.5 | | 0.5 | | 0.5 | V |
| ^ | Low Level Output Voltage TTL Loads | V~CC~ or GND | 4 | 4.5 | | 0.26 | | 0.33 | | 0.4 | V |
| ^ | ^ | ^ | 5.2 | 6 | | 0.26 | | 0.33 | | 0.4 | V |
| I~I~ | Input Leakage Current | V~CC~ or GND | | 6 | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Quiescent Device Current | V~CC~ or GND | 0 | 6 | | 2 | | 20 | | 40 | µA |
```
[[3::table #4]]

```table id=cap caption="Capacitances (from the switching specifications)"
| Symbol | Parameter | Test conditions | V~CC~ (V) | 25°C | < | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| C~I~ | Input Capacitance | See Figure 3 | | | | | | | | | pF |
| C~PD~ | Power Dissipation Capacitance (Notes 2, 3) [1] [2] | | 5 | | 14 | | | | | | pF |

[1] C~PD~ is used to determine the dynamic power consumption, per inverter.
[2] P~D~ = V~CC~^2^ f~i~ (C~PD~ + C~L~) where f~i~ = input frequency, C~L~ = output load capacitance, V~CC~ = supply voltage.
```
[[4::table #1]]

Conversion note: the source prints these two rows as the last rows of its switching specifications table (p.4), with "See Figure 3" across the value columns of the input capacitance row.

## Switching characteristics

```table id=sw caption="Switching specifications" conditions="Input t~r~, t~f~ = 6ns"
| Symbol | Parameter | Test conditions | V~CC~ (V) | 25°C | < | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~, t~PHL~ | Propagation Delay, Input to Output Y (Figure 1) | C~L~ = 50pF | 2 | | | 70 | | 90 | | 105 | ns |
| ^ | ^ | C~L~ = 50pF | 4.5 | | | 14 | | 18 | | 21 | ns |
| ^ | ^ | C~L~ = 15pF | 5 | | 5 | | | | | | ns |
| ^ | ^ | C~L~ = 50pF | 6 | | | 12 | | 15 | | 18 | ns |
| t~TLH~, t~THL~ | Transition Times (Figure 1) | C~L~ = 50pF | 2 | | | 75 | | 95 | 18 [1] | 110 | ns |
| ^ | ^ | ^ | 4.5 | | | 15 | | 19 | | 22 | ns |
| ^ | ^ | ^ | 6 | | | 13 | | 16 | | 19 | ns |

[1] Unconfirmed: the source prints 18 in the Min column of -55°C to 125°C on the 2-V transition-times row; no other row of the table has a minimum.
```
[[4::table #2]]

## Typical characteristics

```curve id=icc caption="Typical inverter supply current as function of input voltage, T~A~ = 25°C"
{"x": {"label": "V_I", "unit": "V", "range": [0, 7]},
 "y": {"label": "I_CC", "unit": "mA", "range": [0, 27.5]},
 "series": [
  {"label": "VCC = 6V", "points": [[0.3, 0], [1, 2], [1.5, 5.6], [1.8, 12.4], [2.2, 18.8], [2.65, 25.5], [3.05, 18.8], [3.25, 15], [3.7, 6.9], [4, 3.2], [4.25, 1.2], [4.6, 0]]},
  {"label": "VCC = 4.5V", "points": [[0.4, 0], [1, 0.6], [1.45, 3.4], [1.8, 8.5], [2, 12.4], [2.2, 9.8], [2.55, 5.6], [2.9, 2], [3.25, 0.8], [3.75, 0.2]]},
  {"label": "VCC = 2V", "points": [[0.4, 0], [1, 0.6], [1.4, 0.1]]}
 ]}
```
[[4::figure #1]]

```curve id=ci caption="Input capacitance as a function of input voltage, T~A~ = 25°C, input pin 5 conditions"
{"x": {"label": "V_IN", "unit": "V", "range": [0, 6]},
 "y": {"label": "C_I", "unit": "pF", "range": [0, 70]},
 "series": [
  {"label": "VDD = 2V, VI 0-2V", "points": [[0, 7], [0.8, 7.5], [0.95, 12], [1, 66], [1.05, 10], [1.2, 7], [2, 5.5]]},
  {"label": "VDD = 3V, VI 0-3V", "points": [[0, 7], [1.2, 8.5], [1.4, 14], [1.5, 51], [1.6, 13], [1.8, 9], [3, 6]]},
  {"label": "VDD = 4V, VI 0-4V", "points": [[0, 7], [1.6, 10.5], [1.85, 18], [2, 40.5], [2.1, 15], [2.3, 10.5], [4, 5.5]]},
  {"label": "VDD = 5V, VI 0-5V", "points": [[0, 7], [2, 10], [2.3, 16], [2.5, 36], [2.65, 13], [3, 9.5], [5, 6]]},
  {"label": "VDD = 6V, VI 0-6V", "points": [[0, 7], [2.5, 11.5], [2.85, 17], [3, 33], [3.15, 14], [3.4, 10], [6, 6]]}
 ]}
```
[[5::figure #1]]

## Parameter measurement information

```figure ref=waveforms-cmos-tpd-tt caption="HC and HCU transition times and propagation delay times, combination logic"
VM: 50%
```
[[4::figure #1]]

Conversion note: Figure 1 (p.4) draws the input between GND and V~CC~ with t~r~ = 6ns and t~f~ = 6ns, the inverting output only, and the 10%, 50% and 90% points on both.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
CD74HCU04E | Tube of 25 | -55 to 125 | CD74HCU04E
CD74HCU04M | | -55 to 125 | HCU04M | Obsolete
CD74HCU04M96 | Reel of 2500 | -55 to 125 | HCU04M
CD74HCU04PWR | Reel of 2000 | -55 to 125 | HJU04
```
[[1::table #2]]

When ordering, use the entire part number. The suffixes 96 and R denote
tape and reel. The suffix T denotes a small-quantity reel of 250. [[1::6]]

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6–7),
  tape and reel, tube and mechanical pages and the notice (8–18)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are from the package option addendum.
- Dropped: the features list (the feature lines are in the
  Description), the page-1 ordering information table (its note is under
  Ordering information), the pinout drawing, the functional diagram, logic symbol and schematic
  diagram (p.2, described in text), and the ESD caution.
- The source prints its junction, storage and lead temperature limits
  under Thermal Information (p.3); they are kept there.
- The input capacitance and power dissipation capacitance rows of the
  switching specifications table (p.4) are in a table of their own
  under Electrical characteristics, with the table's notes 2 and 3.
- The source names the packages E (PDIP), M (SOIC) and PW (TSSOP);
  the package option addendum names them N, D and PW.
- The source has no pin table; the pin descriptions are the
  converter's.
- Curves digitized from Figures 2 and 3 (p.4–5) to about 4 % of full
  scale (thick lines in the source); the capacitance peaks of Figure 3
  are narrower than a few points can show.
- The page-1 ordering information table also names CD74HCU04MT, and the
  package option addendum CD74HCU04M96E4 and CD74HCU04M96G4, which the
  catalog doesn't carry.
- From images: the curve points of Figure 2 (p.4): 0.3, 5.6, 1.8, 12.4,
  2.2, 18.8, 2.65, 25.5, 3.05, 3.25, 3.7, 6.9, 3.2, 4.25, 1.2, 4.6,
  0.4, 0.6, 1.45, 3.4, 8.5, 9.8, 2.55, 2.9, 0.8, 3.75, 0.2, 1.4, 0.1,
  and the axis ends 7 and 27.5.
- From images: the curve points of Figure 3 (p.5): 7.5, 0.95, 12, 66,
  1.05, 8.5, 14, 51, 1.6, 13, 9, 10.5, 1.85, 18, 40.5, 2.1, 2.3, 16,
  36, 9.5, 11.5, 2.85, 17, 33, 3.15, 5.5.
- Source issues:
  - Source conflict: typical propagation delay at V~CC~ = 5V,
    C~L~ = 15pF: 6ns in the features (p.1), 5 ns in the switching
    specifications (p.4).
  - Unconfirmed: I~O~ condition "For V~O~ > -0.5V or V~O~ < V~CC~ +
    0.5V" (absolute maximum ratings note 2).
  - Unconfirmed: the 18 in the Min column of -55°C to 125°C on the 2-V
    transition-times row (switching specifications note 1).
