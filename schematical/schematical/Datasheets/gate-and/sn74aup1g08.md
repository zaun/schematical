---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1g08",
  "title": "SN74AUP1G08 low-power single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74aup1g08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES502Q",
    "date": "2024-03",
    "pages": 54
  },
  "devices": ["gate:1G08", "gate:1G08_DPW", "gate:1G08_SON"],
  "parts": ["SN74AUP1G08DBVR", "SN74AUP1G08DBVT", "SN74AUP1G08DCKR", "SN74AUP1G08DCKRE4", "SN74AUP1G08DCKT", "SN74AUP1G08DRLR", "SN74AUP1G08YZPR", "SN74AUP1G08DPWR", "SN74AUP1G08DRY2", "SN74AUP1G08DRY2G4", "SN74AUP1G08DRYR", "SN74AUP1G08DSF2", "SN74AUP1G08DSF2G4", "SN74AUP1G08DSFR", "SN74AUP1G08YFPR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP, YFP"},
  "converted": "2026-11-08"
}
---

# SN74AUP1G08 low-power single 2-input positive-AND gate

## Description

This single 2-input positive-AND gate is designed for 0.8-V to 3.6-V
V~CC~ operation and performs the Boolean function Y = A · B or
Y = !{!{A} + !{B}} in positive logic.

The AUP family has quiescent power consumption less than 1 µA. The
device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. The I~off~
feature also allows for live insertion. Inputs are 3.6-V tolerant and
have hysteresis, which allows slow input transitions and better
switching noise immunity at the input.

## Pinout

DBV, DCK and DRL have A on pin 1 and B on pin 2; the DPW (X2SON)
package has B on pin 1 and A on pin 2. YFP ball B2 is DNU (do not use).

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input A
B: Input B
Y: Output Y
GND: Ground
VCC: Power pin
NC: No internal connection
DNU: Do not use
```

## Function tables

```table id=function caption="Function table"
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | L |
| H | L | L |
| H | H | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 4.6 | V |
| V~I~ | Input voltage [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | -1.9 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | -4 | mA |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | 4 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (SOT-23), 5 pins | DCK (SC70), 5 pins | DRL (SOT), 5 pins | DSF (SON), 6 pins | DRY (SON), 6 pins | DPW (X2SON), 5 pins | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 298.6 | 314.4 | 349.7 | 407.1 | 554.9 | 291.8 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 240.2 | 128.7 | 120.5 | 232 | 385.4 | 224.2 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 134.6 | 100.6 | 171.4 | 306.9 | 388.2 | 245.8 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 114.5 | 7.1 | 10.8 | 40.3 | 159 | 245.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 133.9 | 99.8 | 169.4 | 306 | 384.1 | 195.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to +85 °C | < | Unit |
| ^ | ^ | ^ | < | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -20 µA | < | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = -1.1 mA | < | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | V |
| ^ | ^ | I~OH~ = -1.7 mA | < | 1.4 V | 1.11 | | | 1.03 | | V |
| ^ | ^ | I~OH~ = -1.9 mA | < | 1.65 V | 1.32 | | | 1.3 | | V |
| ^ | ^ | I~OH~ = -2.3 mA | < | 2.3 V | 2.05 | | | 1.97 | | V |
| ^ | ^ | I~OH~ = -3.1 mA | < | 2.3 V | 1.9 | | | 1.85 | | V |
| ^ | ^ | I~OH~ = -2.7 mA | < | 3 V | 2.72 | | | 2.67 | | V |
| ^ | ^ | I~OH~ = -4 mA | < | 3 V | 2.6 | | | 2.55 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 µA | < | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | < | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | V |
| ^ | ^ | I~OL~ = 1.7 mA | < | 1.4 V | | | 0.31 | | 0.37 | V |
| ^ | ^ | I~OL~ = 1.9 mA | < | 1.65 V | | | 0.31 | | 0.35 | V |
| ^ | ^ | I~OL~ = 2.3 mA | < | 2.3 V | | | 0.31 | | 0.33 | V |
| ^ | ^ | I~OL~ = 3.1 mA | < | 2.3 V | | | 0.44 | | 0.45 | V |
| ^ | ^ | I~OL~ = 2.7 mA | < | 3 V | | | 0.31 | | 0.33 | V |
| ^ | ^ | I~OL~ = 4 mA | < | 3 V | | | 0.44 | | 0.45 | V |
| I~I~ | A or B input | V~I~ = GND to 3.6 V | < | 0 V to 3.6 V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | < | 0 V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | < | 0 V to 0.2 V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V) | I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ - 0.6 V [1] | I~O~ = 0 | 3.3 V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 0 V | | 1.5 | | | | pF |
| ^ | | ^ | < | 3.6 V | | 1.5 | | | | pF |
| C~o~ | | V~O~ = GND | < | 0 V | | 3 | | | | pF |

[1] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to +85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 18 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.3 | 12.8 | 2.1 | 15.6 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.2 | 8.7 | 0.9 | 10.3 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.2 | 6.6 | 0.5 | 8.2 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 4.4 | 0.5 | 5.5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 3.5 | 0.5 | 4.3 | ns |
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to +85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 21 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.5 | 14.7 | 1 | 17.2 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.2 | 10 | 0.5 | 11.3 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 7.7 | 0.5 | 9 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 5.2 | 0.5 | 6.1 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 4.2 | 0.5 | 4.7 | ns |
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to +85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 24 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.9 | 16.3 | 3.1 | 19.9 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 7.2 | 11.1 | 1.8 | 13.2 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.8 | 8.7 | 1.1 | 10.6 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4.3 | 5.9 | 0.5 | 7.3 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.4 | 4.8 | 0.5 | 5.9 | ns |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to +85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 32.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 13.1 | 20.9 | 4.4 | 25.5 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9.5 | 14.2 | 2.9 | 16.9 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.7 | 11 | 2 | 13.5 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.7 | 7.6 | 1.3 | 9.4 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.7 | 6.2 | 1 | 7.5 | ns |
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 4 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4 | pF |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4 | pF |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4 | pF |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 4.1 | pF |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.3 | pF |
```

## Typical characteristics

```curve id=tpd-vcc caption="t~pd~ vs V~CC~, 15 pF load"
{"x": {"label": "VCC", "unit": "V", "range": [0, 4]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 30]},
 "series": [
  {"label": "TPD", "points": [[0.8, 24], [1.2, 10], [1.5, 7.2], [1.8, 5.8], [2.5, 4.3], [3.3, 3.4]]}
 ]}
```

```curve id=tpd-temp caption="t~pd~ vs temperature, V~CC~ = 1.8 V, 15 pF load"
{"x": {"label": "Temperature", "unit": "°C", "range": [-50, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD", "points": [[-40, 5.2], [0, 5.6], [25, 5.7], [85, 6.0], [125, 6.6]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. V~I~ = V~CC~ and V~M~ = V~CC~/2 at every V~CC~ (0.8 V to
3.3 V ± 0.3 V).

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```

## Application information

The AUP family is TI's solution to the industry's low-power needs in
battery-powered portable applications. It has a small amount of
hysteresis built in, allowing for slower or noisy input signals. The
lowered drive produces slower edges and prevents overshoot and
undershoot on the outputs. AUP single-gate logic makes translators for
lower-voltage microprocessors powered from 0.8 V to 1.2 V: it can drop
the voltage of peripheral drivers and accessories that are still
powered by 3.3 V.

Typical application (source Figure 8-1): a 3.3-V bus driver drives the
gate's input; the gate runs from a 1 V regulated V~CC~ with a 0.1 µF
bypass capacitor and drives a 1-V microprocessor.

Design requirements: the device uses CMOS technology and has balanced
output drive. Take care to avoid bus contention because it can drive
currents that would exceed maximum limits.

- Inputs: see Δt/Δv, V~IH~ and V~IL~ in the
  [recommended operating conditions](#recommended-operating-conditions).
  Inputs are overvoltage tolerant, allowing them to go as high as 3.6 V
  at any valid V~CC~.
- Outputs: load currents should not exceed 20 mA on the output and
  50 mA total for the part. Outputs should not be pulled above V~CC~.

Power supply: any voltage between the minimum and maximum supply
voltage in the recommended operating conditions. Each V~CC~ terminal
should have a good bypass capacitor to prevent power disturbance; for
devices with a single supply, 0.1 μF is recommended. A 0.1 μF and 1 μF
are commonly used in parallel. The bypass capacitor should be installed
as close to the power terminal as possible.

Layout: inputs should not ever float. Unused input pins should not be
left unconnected, because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs
of digital logic devices must be connected to a high or low bias to
prevent them from floating; generally they will be tied to GND or V~CC~.
It is generally OK to float outputs unless the part is a transceiver.

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

```package footprint=DSBGA-6-YFP
```

## Ordering information

```ordering
SN74AUP1G08DBVR | Reel of 3000 | -40 to 85 | H08F, H08R
SN74AUP1G08DBVT | Reel of 250 | -40 to 85 | H08F, H08R
SN74AUP1G08DCKR | Reel of 3000 | -40 to 85 | HE5, HEF, HEK, …
SN74AUP1G08DCKRE4 | Reel of 3000 | -40 to 85 | HE5, HEF, HEK, …
SN74AUP1G08DCKT | Reel of 250 | -40 to 85 | HE5, HER
SN74AUP1G08DPWR | Reel of 3000 | -40 to 85 | E, E4
SN74AUP1G08DRLR | Reel of 4000 | -40 to 85 | HE7, HER
SN74AUP1G08DRY2 | Reel of 5000 | -40 to 85 | HE
SN74AUP1G08DRY2G4 | Reel of 5000 | -40 to 85 | HE
SN74AUP1G08DRYR | Reel of 5000 | -40 to 85 | HE
SN74AUP1G08DSF2 | Reel of 5000 | -40 to 85 | HE, HER
SN74AUP1G08DSF2G4 | Reel of 5000 | -40 to 85 | HE
SN74AUP1G08DSFR | Reel of 5000 | -40 to 85 | HE, HER
SN74AUP1G08YFPR | Reel of 3000 | -40 to 85 | HEN
SN74AUP1G08YZPR | Reel of 3000 | -40 to 85 | HEN
```

The source also lists SN74AUP1G08DBVRG4 and SN74AUP1G08DPWRG4 (and
`.B` variants), which the catalog doesn't carry. An automotive version,
SN74AUP1G08-Q1, has its own datasheet.

## Conversion notes

- Source pages 1–15 converted; the revision history, package option
  addendum, tape and reel and mechanical pages (15–54) dropped, replaced
  by the generated Package and Ordering sections.
- Dropped: Figure 6-2 (enable and disable times load circuit and
  waveforms) and the pulse-duration and setup/hold waveforms of
  Figure 6-1, which the source notes are not applicable to all devices;
  this gate has no output enable, no clock and no pulse-width limit.
  Also dropped: Figure 8-2 (family power comparison), Figure 8-3
  (25 MHz switching waveform), the layout example and the DPW package
  marketing text.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
- The DCK part markings are cut off in the source's package option
  addendum ("HE5, HEF, HEK, HE"); the list ends with "…" here.
- From images: Y = !{!{A} + !{B}} overbars (p.1, p.12); curve points 24,
  10, 7.2, 5.8, 4.3, 3.4, 5.2, 5.6, 5.7, 6.0, 6.6, 125 (Figures 5-1 and
  5-2, p.9).
- Source issues: none found.
