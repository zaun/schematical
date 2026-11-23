---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn54ahc14",
  "title": "SNx4AHC14 hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn54ahc14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS238P",
    "date": "2024-02",
    "pages": 43
  },
  "devices": ["gate:74_14"],
  "parts": ["5962-9680201QCA"],
  "packages": {"p14": "D, DB, DGV, J, N, NS, PW, W, RGY, BQA"},
  "converted": "2026-11-21"
}
---

# SNx4AHC14 hex Schmitt-trigger inverters

## Description

The SNx4AHC14 devices contain six independent inverters. These devices
perform the boolean function Y = !{A}. [[1::1]]

Each circuit functions as an independent inverter, but, because of the
Schmitt-Trigger action, the inverters have different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals. [[1::3]]

Operating range: 2V to 5.5V. ±8mA output drive at 5V. Schmitt-Trigger
inputs enable input noise resistance. Low power consumption: 20µA
maximum I~CC~. Latch-up performance exceeds 250mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1 Input
1Y: Channel 1 Output
2A: Channel 2 Input
2Y: Channel 2 Output
3A: Channel 3 Input
3Y: Channel 3 Output
4A: Channel 4 Input
4Y: Channel 4 Output
5A: Channel 5 Input
5Y: Channel 5 Output
6A: Channel 6 Input
6Y: Channel 6 Output
GND: Ground
VCC: Power supply
```
[[3::Table 4-1]]

## Functional description

The SNx4AHC14 Schmitt-Trigger devices contain six independent
inverters. They perform the Boolean function Y = !{A} in positive logic. [[10::1]]

Schmitt-Trigger inputs are designed to provide a minimum separation
between positive and negative switching thresholds. This allows for
noisy or slow inputs that would cause problems such as oscillation or
excessive current draw with normal CMOS inputs. [[10::2]]

Conversion note: the functional block diagram shows one inverter with a Schmitt-trigger symbol, input A and output Y.

The wide operating range of the device allows it to be used in a
variety of systems that use different logic levels. The outpus can
drive up to 10 LSTTL loads each. The balanced drive outputs can source
or sink 8 mA at 5-V V~CC~. [[10::3]]

Source error: "outpus" is the source's spelling of "outputs" (Feature Description, p.10); kept as printed.

## Function tables

Table 7-1 lists the functional modes of the SNx4AHC14. [[10::4]]

```table id=function caption="Function table"
| Input A | Output Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[3::Table 4-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Virtual operating junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| T~A~ | Operating free-air temperature | SN54AHC14 | -55 | 125 | °C |
| ^ | ^ | SN74AHC14 | -40 | 125 | ^ |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHC14 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | DB (SSOP) 14 pins | DGV (TVSOP) 14 pins | N (PDIP) 14 pins | NS (SO) 14 pins | PW (TSSOP) 14 pins | RGY (VQFN) 14 pins | BQA (WQFN) 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 137.8 | 141.9 | 61.9 | 94.7 | 147.7 | 87.1 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.8 | 90 | 61.1 | 49.5 | 52.5 | 77.4 | 92.6 | 90.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 81 | 98.3 | 71.3 | 41.7 | 53.4 | 90.9 | 62.5 | 56.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 37 | 42.7 | 9.7 | 34.7 | 21.3 | 27.2 | 22.8 | 9.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 80.6 | 97 | 70.6 | 41.7 | 53.1 | 90.2 | 61.7 | 56.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | N/A | 45.1 | 33.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | < | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | V~CC~ = 3 V | | 1.2 | | 2.2 | V |
| ^ | ^ | | V~CC~ = 4.5 V | | 1.75 | | 3.15 | ^ |
| ^ | ^ | | V~CC~ = 5.5 V | | 2.15 | | 3.85 | ^ |
| V~T–~ | Negative-going input threshold voltage | | V~CC~ = 3 V | | 0.9 | | 1.9 | V |
| ^ | ^ | | V~CC~ = 4.5 V | | 1.35 | | 2.75 | ^ |
| ^ | ^ | | V~CC~ = 5.5 V | | 1.65 | | 3.35 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | V~CC~ = 3 V | | 0.3 | | 1.2 | V |
| ^ | ^ | | V~CC~ = 4.5 V | | 0.4 | | 1.4 | ^ |
| ^ | ^ | | V~CC~ = 5.5 V | | 0.5 | | 1.6 | ^ |
| V~OH~ | | I~OH~ = –50 µA | V~CC~ = 2 V | | 1.9 | 2 | | V |
| ^ | ^ | ^ | V~CC~ = 3 V | | 2.9 | 3 | | ^ |
| ^ | ^ | ^ | V~CC~ = 4.5 V | | 4.4 | 4.5 | | ^ |
| ^ | ^ | I~OH~ = –4 mA, V~CC~ = 3 V | T~A~ = 25°C | | 2.58 | | | ^ |
| ^ | ^ | ^ | SNx4AHC14 | | 2.48 | | | ^ |
| ^ | ^ | I~OH~ = –8 mA, V~CC~ = 4.5 V [3] | T~A~ = 25°C | | 3.94 | | | ^ |
| ^ | ^ | ^ | SNx4AHC14 | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 50 µA [4] | V~CC~ = 2 V | | | | 0.1 | V |
| ^ | ^ | ^ | V~CC~ = 3 V | | | | 0.1 | ^ |
| ^ | ^ | ^ | V~CC~ = 4.5 V | | | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA, V~CC~ = 3 V [4] | T~A~ = 25°C | | | | 0.36 | ^ |
| ^ | ^ | ^ | SN54AHC14 | | | | 0.5 | ^ |
| ^ | ^ | ^ | SN74AHC14 | T~A~ = –40°C to 85°C | | | 0.44 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 8 mA, V~CC~ = 4.5 V | T~A~ = 25°C | | | | 0.36 | ^ |
| ^ | ^ | ^ | SN54AHC14 | | | | 0.5 | ^ |
| ^ | ^ | ^ | SN74AHC14 | T~A~ = –40°C to 85°C | | | 0.44 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to 125°C | | | 0.5 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND, V~CC~ = 0 V to 5.5 V | T~A~ = 25°C | | | | ±0.1 | µA |
| ^ | ^ | ^ | SNx4AHC14 | | | | ±1 [1] | ^ |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0, V~CC~ = 5.5 V | T~A~ = 25°C | | | | 1 | µA |
| ^ | ^ | ^ | SNx4AHC14 | | | | 20 | ^ |
| C~I~ | | V~I~ = V~CC~ or GND, V~CC~ = 5 V | T~A~ = 25°C | | | 2 | 10 | pF |
| ^ | ^ | ^ | SN74AHC14 | | | | 10 | ^ |
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz, V~CC~ = 5 V | | | | 9 | | pF |
| NOISE [2] | < | < | < | < | < | < | < | < |
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C | | | | 0.8 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C | | | | -0.4 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C | | | | 4.6 | | V |
| V~IH(D)~ | High-level dynamic input voltage | V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C | | | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C | | | | | 1.5 | V |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] Characteristics are for surface-mount packages only.
[3] Source error: printed as I~OL~ = –8 mA; the row gives V~OH~, and the recommended operating conditions give I~OH~ = –8 mA at V~CC~ = 5 V ± 0.5 V, so the condition is I~OH~.
[4] Source error: printed as I~OH~ = 50 µA and I~OH~ = 4 mA; the rows give V~OL~, and the recommended operating conditions give I~OL~ = 50 µA and 4 mA, so the condition is I~OL~.
```
[[5::table #2]] [[6::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="V~CC~ = 3.3 V ± 0.3 V and over operating free-air temperature range (unless otherwise noted; see Parameter Measurement Information)"
| Symbol | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| t~PLH~ | From A (input) to Y (output), C~L~ = 15 pF | T~A~ = 25°C | 8.3 [1] | | 12.8 [1] | ns |
| ^ | ^ | SN54AHC14 | 1 [1] | | 15 [1] | ^ |
| ^ | ^ | SN74AHC14 | 1 | | 16 | ^ |
| t~PHL~ | From A (input) to Y (output), C~L~ = 15 pF | T~A~ = 25°C | 8.3 [1] | | 12.8 [1] | ns |
| ^ | ^ | SN54AHC14 | 1 [1] | | 15 [1] | ^ |
| ^ | ^ | SN74AHC14 | 1 | | 16 | ^ |
| t~PLH~ | From A (input) to Y (output), C~L~ = 50 pF | T~A~ = 25°C | 10.8 | | 16.3 | ns |
| ^ | ^ | SN54AHC14 | 1 | | 18.5 | ^ |
| ^ | ^ | SN74AHC14 | 1 | | 19.5 | ^ |
| t~PHL~ | From A (input) to Y (output), C~L~ = 50 pF | T~A~ = 25°C | 10.8 | | 16.3 | ns |
| ^ | ^ | SN54AHC14 | 1 | | 18.5 | ^ |
| ^ | ^ | SN74AHC14 | 1 | | 19.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[7::table #1]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="V~CC~ = 5 V ± 0.5 V and over operating free-air temperature range (unless otherwise noted; see Parameter Measurement Information)"
| Symbol | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| t~PLH~ | From A (input) to Y (output), C~L~ = 15 pF | T~A~ = 25°C | 5.5 [1] | | 8.6 [1] | ns |
| ^ | ^ | SN54AHC14 | 1 [1] | | 10 [1] | ^ |
| ^ | ^ | SN74AHC14 | 1 | | 10 | ^ |
| t~PHL~ | From A (input) to Y (output), C~L~ = 15 pF | T~A~ = 25°C | 5.5 [1] | | 8.6 [1] | ns |
| ^ | ^ | SN54AHC14 | 1 [1] | | 10 [1] | ^ |
| ^ | ^ | SN74AHC14 | 1 | | 10 | ^ |
| t~PLH~ | From A (input) to Y (output), C~L~ = 50 pF | T~A~ = 25°C | 7 | | 10.6 | ns |
| ^ | ^ | SNx4AHC14 | 1 | | 12 | ^ |
| t~PHL~ | From A (input) to Y (output), C~L~ = 50 pF | T~A~ = 25°C | 7 | | 10.6 | ns |
| ^ | ^ | SNx4AHC14 | 1 | | 12 | ^ |

[1] Source conflict: the 5-V table (p.7) marks these values with (1) and prints no note under the table; the 3.3-V table (p.7) prints note (1): On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[7::table #2]]

Unconfirmed: both switching tables print the T~A~ = 25°C values 8.3, 10.8, 5.5 and 7 in the MIN column and nothing under TYP, while the minimum over the temperature range is 1 ns.

## Typical characteristics

C~L~ = 50 pF (unless otherwise noted) [[7::3]]

```curve id=tpd-vcc caption="Propagation delay vs supply voltage"
{"x": {"label": "Supply Voltage VCC", "unit": "V", "range": [3, 5]},
 "y": {"label": "Propagation Delay", "unit": "ns", "range": [8.5, 13.5]},
 "series": [
  {"label": "Propagation delay", "points": [[3, 13.1], [4, 10.9], [4.4, 10], [5, 8.7]]}
 ]}
```
[[7::Figure 5-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. [[9::1]]

All input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. [[9::1]]

The outputs are measured one at a time with one input transition per
measurement. [[9::1]]

All parameters and waveforms are not applicable to all devices. [[9::1]]

```figure ref=load-cmos-50pf caption="Figure 6-1. Load circuit for totem-pole outputs"
CL: see table
```
[[8::Figure 6-1]]

```figure ref=waveforms-tpd caption="Figure 6-5. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```
[[8::Figure 6-5]]

Conversion note: the source also shows the load circuit for 3-state and open-drain outputs (Figure 6-2, R~L~ = 1 kΩ) with its S1 table (Table 6-1), and the pulse-duration (Figure 6-3), setup-and-hold (Figure 6-4) and enable/disable (Figure 6-6) waveforms with their note B, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

The SN74AHC14 device is a Schmitt-Trigger input CMOS device that can be
used for a multitude of inverting buffer type functions. The application
shown here takes advantage of the Schmitt-Trigger inputs to produce a
delay for a logic input. [[11::2]]

Conversion note: the simplified application schematic (Figure 8-1) shows INPUT on 1A, a resistor R~1~ from 1Y to 2A, a capacitor C~1~ from 2A to ground, and OUTPUT on 2Y.

### Design requirements

This device uses CMOS technology. Take care to avoid bus contention
because it can drive currents that would exceed maximum limits. Parallel
output drive can create fast edges into light loads so consider routing
and load conditions to prevent ringing. [[11::3]]

### Detailed design procedure

This circuit is designed around an RC network that produces a slow
input to the second inverter. The RC time constant, τ, is calculated
from: τ = RC. [[11::4]]

The delay time for this circuit is from
t~delay(min)~ = –ln |1 – V~T+(min)~ / V~CC~| τ to
t~delay(max)~ = –ln |1 – V~T+(max)~ / V~CC~| τ. It must be noted that
the delay is consistent for each device, but because the switching
threshold is only ensured between the minimum and maximum value, the
output pulse length varies between devices. These values must be
calculated by using the minimum and maximum V~T+~ values in the
Electrical Characteristics. [[11::5]]

The resistor value must be chosen such that the maximum current to and
from the SN74AHC14 is 8 mA at 5-V V~CC~. [[11::6]]

Conversion note: the application curve (Figure 8-2, ideal capacitor voltage and output voltage with positive switching threshold) plots the capacitor voltage V~C~ rising toward V~CC~ and the output V~OUT~ switching between t~delay(min)~ and t~delay(max)~, where V~C~ crosses the band from V~T+(min)~ to V~T+(max)~.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
The V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. TI recommends using a 0.1-µF capacitor on the V~CC~
terminal, and must be placed as close as possible to the pin for best
results. [[12::2]]

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
the part is a transceiver. [[12::3]]

Conversion note: the layout diagram (Figure 8-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-9680201QCA | Tube of 25 | -55 to 125 | 5962-9680201QCA SNJ54AHC14J
```
[[14::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering line is from the package
  option addendum (pp.14–17). The support and revision history page
  (13), tape and reel and mechanical pages (18–43) are left out; the
  generated Package section replaces them.
- Dropped: applications list, device information table, the logic
  diagram of page 1 (the same as the functional block diagram, described
  in text), the vendor's pin drawings; the application schematic, the
  application curve and the layout diagram are described in text.
- The catalog carries only the 5962-9680201QCA of this PDF's parts under
  this datasheet; the other orderables of the package option addendum
  (SN74AHC14 parts, SNJ54AHC14J, SNJ54AHC14W, SNJ54AHC14FK and their 5962
  numbers) are not claimed here. The FK (LCCC, 20) pin column of Table
  4-1 and its NC pins are left out with them.
- The electrical characteristics table of the source gives no parameter
  name for V~OH~, V~OL~, I~I~, I~CC~ and C~I~, so those Parameter cells
  are empty. The switching tables print the path and load as the test
  condition.
- Figure 5-1 (p.7) is digitized to about 2 % of full scale.
- From images: overbars of Y = !{A} (pp.1 and 10).
- Source issues:
  - Source error: "outpus" for "outputs" (Feature Description, p.10),
    kept as printed.
  - Source error: V~OH~ row printed with I~OL~ = –8 mA; corrected to
    I~OH~ (electrical characteristics note 3).
  - Source error: V~OL~ rows printed with I~OH~ = 50 µA and I~OH~ = 4 mA;
    corrected to I~OL~ (electrical characteristics note 4).
  - Source conflict: the 5-V switching table marks values with (1) and
    prints no note; the 3.3-V table prints it (switching table note).
  - Unconfirmed: the T~A~ = 25°C switching values are printed under MIN
    with no typical value (see Switching characteristics).
