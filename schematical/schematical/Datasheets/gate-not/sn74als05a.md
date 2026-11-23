---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74als05a",
  "title": "SN54ALS05A, SN74ALS05A hex inverters with open-collector outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74als05a.pdf",
    "vendor": "Texas Instruments",
    "document": "SDAS190A",
    "date": "1994-12",
    "pages": 19
  },
  "devices": ["gate:74_05"],
  "parts": ["SN74ALS05AD", "SN74ALS05ADR", "SN74ALS05AN", "SN74ALS05ANSR"],
  "packages": {"p14": "D, N, NS"},
  "converted": "2026-11-21"
}
---

# SN54ALS05A, SN74ALS05A hex inverters with open-collector outputs

## Description

These devices contain six independent hex inverters with open-collector outputs.
They perform the Boolean function Y = !{A}. The open-collector outputs
require pullup resistors to perform correctly. These outputs can be
connected to other open-collector outputs to implement active-low
wired-OR or active-high wired-AND functions. Open-collector devices are
often used to generate higher V~OH~ levels. [[1::2]]

The SN54ALS05A is characterized for operation over the full military
temperature range of –55°C to 125°C. The SN74ALS05A is characterized for
operation from 0°C to 70°C. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
GND: Ground
VCC: Positive supply
```
[[1::table #1]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | | 7 | V |
| V~I~ | Input voltage | | | 7 | V |
| | Off-state output voltage | | | 7 | V |
| T~A~ | Operating free-air temperature range | SN54ALS05A | -55 | 125 | °C |
| ^ | ^ | SN74ALS05A | 0 | 70 | °C |
| | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | SN54ALS05A | < | < | SN74ALS05A | < | < | Unit |
| ^ | ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5 | 5.5 | 4.5 | 5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.7 | | | 0.8 | V |
| V~OH~ | High-level output voltage | | | 5.5 | | | 5.5 | V |
| I~OL~ | Low-level output current | | | 4 | | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | | 125 | 0 | | 70 | °C |
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | SN54ALS05A | < | < | SN74ALS05A | < | < | Unit |
| ^ | ^ | < | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~IK~ | V~CC~ = 4.5 V | I~I~ = -18 mA | | | -1.2 | | | -1.2 | V |
| V~OL~ | V~CC~ = 4.5 V | I~OL~ = 4 mA | | 0.25 | 0.4 | | 0.25 | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | | | | | 0.35 | 0.5 | ^ |
| I~I~ | V~CC~ = 5.5 V | V~I~ = 7 V | | | 0.1 | | | 0.1 | mA |
| I~IH~ | V~CC~ = 5.5 V | V~I~ = 2.7 V | | | 20 | | | 20 | µA |
| I~IL~ | V~CC~ = 5.5 V | V~I~ = 0.4 V | | | -0.1 | | | -0.1 | mA |
| I~OH~ | V~CC~ = 4.5 V | V~OH~ = 5.5 V | | | 0.1 | | | 0.1 | mA |
| I~CCH~ | V~CC~ = 5.5 V | V~I~ = 0 | | 0.65 | 1.1 | | 0.65 | 1.1 | mA |
| I~CCL~ | V~CC~ = 5.5 V | V~I~ = 4.5 V | | 2.9 | 4.2 | | 2.9 | 4.2 | mA |

[1] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
```
[[2::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics (see Figure 1)" conditions="V~CC~ = 4.5 V to 5.5 V, C~L~ = 50 pF, R~L~ = 2 kΩ, T~A~ = MIN to MAX [1]"
| Symbol | From (input) | To (output) | SN54ALS05A | < | SN74ALS05A | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 23 | 84 | 23 | 54 | ns |
| t~PHL~ | ^ | ^ | 4 | 24 | 4 | 14 | ^ |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
```
[[2::table #4]]

## Parameter measurement information

Series 54ALS/74ALS and 54AS/74AS devices. C~L~ includes probe and jig
capacitance. All input pulses have the following characteristics:
PRR ≤ 1 MHz, t~r~ = t~f~ = 2 ns, duty cycle = 50%. The outputs are
measured one at a time with one transition per measurement. [[3::1]]

Conversion note: in Figure 1 the input pulses swing from 0.3 V to 3.5 V and all waveforms are measured at 1.3 V.

```figure ref=load-ttl caption="Figure 1. Load circuit for open-collector outputs"
RL: 2 kΩ
CL: 50 pF
```
[[3::Figure 1]]

```figure ref=waveforms-tpd caption="Figure 1. Voltage waveforms, propagation delay times"
VM: 1.3 V
```
[[3::Figure 1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

## Ordering information

```ordering
SN74ALS05AD | | 0 to 70 | ALS05A | Obsolete
SN74ALS05ADR | Reel of 2500 | 0 to 70 | ALS05A
SN74ALS05AN | Tube of 25 | 0 to 70 | SN74ALS05AN
SN74ALS05ANSR | Reel of 2000 | 0 to 70 | ALS05A
```
[[4::table #1]]

## Conversion notes

- Source pages 1–3 converted. The ordering lines are from the package
  option addendum (pp.4–5). Tape and reel, tube and mechanical pages
  (6–19) are left out; the generated Package section replaces them.
- Dropped: feature list (package options), the logic symbol and the
  logic diagram (six inverters, 1A to 1Y through 6A to 6Y), and the
  parts of Figure 1 that don't apply to these open-collector parts: the
  bi-state totem-pole and 3-state load circuits, setup/hold,
  pulse-duration and enable/disable waveforms, and Notes B and C
  (3-state outputs).
- The package option addendum also lists SN54ALS05AJ, SNJ54ALS05AJ,
  SNJ54ALS05AW, SNJ54ALS05AFK, 5962-88540012A, 5962-8854001CA and
  5962-8854001DA, which the catalog doesn't carry.
- The absolute maximum ratings of the source are a list; each line is a
  row here. The electrical characteristics table of the source names its
  parameters by symbol only; no parameter names are added.
- The source has no pin table; the pin descriptions are the converter's.
  Page 1 draws the D, J, N and FK packages; the NS package is named only
  in the package option addendum. The FK (LCCC, 20) pinout is not drawn
  here: the catalog doesn't carry an FK part.
- From images: overbar of Y = !{A} (p.1).
- Source issues: none found.
