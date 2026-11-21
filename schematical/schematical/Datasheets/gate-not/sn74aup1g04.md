---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1g04",
  "title": "SN74AUP1G04 low-power single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup1g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES571K",
    "date": "2014-06",
    "pages": 48
  },
  "devices": ["gate:1G04", "gate:1G04_SON"],
  "parts": ["SN74AUP1G04DBVR", "SN74AUP1G04DBVT", "SN74AUP1G04DCKR", "SN74AUP1G04DCKT", "SN74AUP1G04DPWR", "SN74AUP1G04DRLR", "SN74AUP1G04YFPR", "SN74AUP1G04DRY2", "SN74AUP1G04DRYR", "SN74AUP1G04DSF2", "SN74AUP1G04DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga4": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP1G04 low-power single inverter gate

## Description

The SN74AUP1G04 device is a single inverter gate performs the Boolean
function Y = !{A}.

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga4
```

## Pin functions

```pins
NC: No Connection
A: Input A
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```

## Functional description

The SN74AUP1G04 device is a single inverter gate performs the Boolean
function Y = !{A}.

The AUP family of devices has quiescent power consumption less than 1 µA
and comes in the ultra small DPW package. The DPW package technology is
a major breakthrough in IC packaging. Its tiny 0.64 mm square footprint
saves significant board space over other package options while still
retaining the traditional manufacturing friendly lead pitch of 0.5 mm.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. The I~off~
feature also allows for live insertion.

Conversion note: the functional block diagram shows one inverter with input A and output Y.

- Wide operating V~CC~ range of 0.8 V to 3.6 V
- 3.6-V I/O tolerant to support down translation
- Input hysteresis allows slow input transition and better switching noise immunity at the input
- I~off~ feature allows voltages on the inputs and outputs when V~CC~ is 0 V
- Low noise due to slower edge rates

## Function tables

```table id=function caption="Function table"
| Input | Output [1] |
| A | Y [1] |
|:-:|:-:|
| H | L |
| L | H |

[1] Source error: the source heads this column INPUT B; the device has one input, A, and the output Y (pin functions, p.3), and Y = !{A} (p.11).
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=handling caption="Handling ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 0 | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 0 | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | ^ |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | -1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -4 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 4 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | DBV 5 pins | DCK 5 pins | DPW 5 pins | DRL 5 pins | DRY 6 pins | DSF 6 pins | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 298.6 | 314.4 | 291.8 | 349.7 | 554.9 | 407.1 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 240.2 | 128.7 | 224.2 | 120.5 | 385.4 | 232.0 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 134.6 | 100.6 | 245.8 | 171.4 | 388.2 | 306.9 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 114.5 | 7.1 | 31.4 | 10.8 | 159.0 | 40.3 | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 133.9 | 99.8 | 245.6 | 169.4 | 384.1 | 306.0 | ^ |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | 195.4 | N/A | N/A | N/A | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –20 μA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = –1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | ^ |
| ^ | ^ | I~OH~ = –1.7 mA | 1.4 V | 1.11 | | | 1.03 | | ^ |
| ^ | ^ | I~OH~ = –1.9 mA | 1.65 V | 1.32 | | | 1.3 | | ^ |
| ^ | ^ | I~OH~ = –2.3 mA | 2.3 V | 2.05 | | | 1.97 | | ^ |
| ^ | ^ | I~OH~ = –3.1 mA | ^ | 1.9 | | | 1.85 | | ^ |
| ^ | ^ | I~OH~ = –2.7 mA | 3 V | 2.72 | | | 2.67 | | ^ |
| ^ | ^ | I~OH~ = –4 mA | ^ | 2.6 | | | 2.55 | | ^ |
| V~OL~ | | I~OL~ = 20 μA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | ^ |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | ^ |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | ^ |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1 mA | ^ | | | 0.44 | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4 mA | ^ | | | 0.44 | | 0.45 | ^ |
| I~I~ | A input | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | μA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | μA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | μA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | μA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6 V, I~O~ = 0 | 3.3 V | | | 40 | | 50 | μA |
| C~I~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6 V | | 1.5 | | | | ^ |
| C~o~ | | V~O~ = V~CC~ or GND | 3.6 V | | 2.5 | | | | pF |
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 3.9 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.9 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.9 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 3.9 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 3.9 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.1 | ^ |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 V | | 15.6 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.3 | 5.9 | 10.8 | 2.1 | 13.5 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.5 | 4.2 | 7 | 1.6 | 8.8 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.2 | 3.4 | 5.9 | 1.4 | 7 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.7 | 2.5 | 4 | 1.3 | 4.9 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.4 | 2.1 | 3.2 | 1.2 | 3.9 | ^ |
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 V | | 17.7 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.9 | 6.9 | 12.2 | 3.1 | 15 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3 | 5 | 8.1 | 2.5 | 9.9 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.6 | 4 | 6.9 | 2.1 | 7.9 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 2.1 | 3 | 4.6 | 1.7 | 5.6 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.8 | 2.5 | 3.8 | 1.5 | 4.5 | ^ |
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 V | | 19.5 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.7 | 7.8 | 13 | 3.8 | 15.9 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.7 | 5.6 | 8.6 | 3.1 | 10.6 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 3.2 | 4.6 | 7.4 | 2.6 | 8.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 2.5 | 3.5 | 5.1 | 2.1 | 6.1 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 2.2 | 2.9 | 4.2 | 1.9 | 5 | ^ |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3 and Figure 4)"
| Parameter | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 V | | 25.4 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 6.8 | 10.4 | 16 | 6.1 | 19 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 5.3 | 7.6 | 10.8 | 4.8 | 12.9 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4.6 | 6.3 | 9.2 | 4.1 | 10.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 3.6 | 4.8 | 6.5 | 3.3 | 7.6 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 3.2 | 4 | 5.4 | 2.9 | 6.2 | ^ |
```

## Typical characteristics

```curve id=tpd-vcc caption="TPD vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [0, 4]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 18]},
 "series": [
  {"label": "TPD in ns", "points": [[0.8, 16], [1.2, 6], [1.5, 4.2], [1.8, 3.4], [2.5, 2.5], [3.3, 2.1]]}
 ]}
```

```curve id=tpd-temp caption="TPD vs Temperature at 1.8 V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-50, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4.5]},
 "series": [
  {"label": "TPD in ns", "points": [[-40, 3], [25, 3.4], [85, 3.8], [125, 4.2]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, t~r~/t~f~ = 3 ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices.

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | V~CC~ = 1.5 V ± 0.1 V | V~CC~ = 1.8 V ± 0.15 V | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|---|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~M~ | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 |
| V~I~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ |
```

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```

## Application information

The AUP family is TI's premier solution to the industry’s low-power
needs in battery-powered portable applications. This family ensures a
very low static and dynamic power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in an increased battery life. This
product also maintains excellent signal integrity. It has a small amount
of hysteresis built in allowing for slower or noisy input signals. The
lowered drive produces slower edges and prevents overshoot and
undershoot on the outputs.

Conversion note: the typical application schematic (Figure 5) shows a 3.3 V bus driver driving the inverter's input, the inverter on a 1 V regulated V~CC~ with a 0.1 µF capacitor, and the inverter's output driving a 1 V micro processor.

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits.

1. Recommended Input conditions
   - Rise time and fall time specs. See (Δt/ΔV) in Recommended Operating Conditions
   - Specified high and low levels. See (V~IH~ and V~IL~) in Recommended Operating Conditions
   - Inputs are overvoltage tolerant allowing them to go as high as 3.6 V at any valid V~CC~
2. Recommend output conditions
   - Load currents should not exceed 20 mA on the output and 50 mA total for the part
   - Outputs should not be pulled above V~CC~

The AUP family of single gate logic makes excellent translators for the
new lower voltage microprocessors that typically are powered from 0.8 V
to 1.2 V. They can drop the voltage of peripheral drivers and
accessories that are still powered by 3.3 V to the new uC power levels.

### Power supply recommendations

The power supply can be any voltage between the Min and Max supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended; if
there are multiple V~CC~ pins, then 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and a 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Figure 8
specifies the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~, whichever makes
more sense or is more convenient. It is generally acceptable to float
outputs, unless the part is a transceiver. If the transceiver has an
output enable pin, it will disable the output section of the part when
asserted. This will not disable the input section of the I/Os, so they
cannot float when disabled.

Conversion note: the layout diagram (Figure 8, p.14) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

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

```package footprint=DSBGA-4-YFP
```

## Ordering information

```ordering
SN74AUP1G04DBVR | Reel of 3000 | -40 to 85 | H04F, H04R
SN74AUP1G04DBVT | Reel of 250 | -40 to 85 | H04F, H04R
SN74AUP1G04DCKR | Reel of 3000 | -40 to 85 | HC5, HCF, HCK, HCR
SN74AUP1G04DCKT | Reel of 250 | -40 to 85 | HC5, HCF, HCR
SN74AUP1G04DPWR | Reel of 3000 | -40 to 85 | C, C4
SN74AUP1G04DRLR | Reel of 4000 | -40 to 85 | HC7, HCR
SN74AUP1G04DRY2 | Reel of 5000 | -40 to 85 | HC
SN74AUP1G04DRYR | Reel of 5000 | -40 to 85 | HC
SN74AUP1G04DSF2 | Reel of 5000 | -40 to 85 | HC
SN74AUP1G04DSFR | Reel of 5000 | -40 to 85 | HC
SN74AUP1G04YFPR | Reel of 3000 | -40 to 85 | HCN
```

## Conversion notes

- Source pages 1–14 converted; the documentation support page (15),
  tape and reel and mechanical pages (18–48) dropped, replaced by the
  generated Package and Ordering sections. The ordering lines are from
  the package option addendum (pp.16–17). The table of contents and
  revision history on page 2 are dropped too.
- Dropped: the features and applications lists, the Device Information
  table, Figure 4 (enable and disable times load circuit and waveforms,
  p.10) and the pulse-duration and setup/hold waveforms of Figure 3,
  which the source notes are not applicable to all devices (this
  inverter has no output enable and no clock), Figure 6 (family power
  comparison) and Figure 7 (25 MHz switching waveform of the AUP1G08).
  The functional block diagram, the typical application schematic and
  the layout diagram are described in text.
- The addendum also lists SN74AUP1G04DBVRG4.B, SN74AUP1G04DCKR1G4,
  SN74AUP1G04DCKRE4, SN74AUP1G04DCKTG4, SN74AUP1G04DPWRG4 and
  SN74AUP1G04DSFRG4, which the catalog doesn't carry.
- Curves digitized from Figures 1 and 2 (p.8) to about 2 % of full
  scale.
- The thermal table's "n/a" cells are written N/A.
- From images: overbars of Y = !{A} (p.1, p.11); curve points 16, 6,
  4.2, 3.4, 2.5, 2.1, 3, 3.8, 125 (Figures 1 and 2, p.8).
- Source issues:
  - Source error: the function table (Table 1, p.11) heads its second
    column INPUT B; the device has one input, A, and the output Y.
    Corrected to Output Y (function table note 1).
  - Unconfirmed: the description (p.1) and the overview (p.11) read "is
    a single inverter gate performs the Boolean function", without a
    joining word. Kept as printed.
  - Unconfirmed: the pin drawings (p.3) carry the legend "DNU – Do not
    use", and no drawing or pin table row of this datasheet has a DNU
    pin.
  - Unconfirmed: the Device Information table (p.1) lists the X2SON
    package with 4 pins, and the pin functions (p.3) give DPW five
    pins.
