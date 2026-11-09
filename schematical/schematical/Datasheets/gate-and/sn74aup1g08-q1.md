---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1g08-q1",
  "title": "SN74AUP1G08-Q1 low-power single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74aup1g08-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES847",
    "date": "2012-12",
    "pages": 16
  },
  "devices": ["gate:1G08"],
  "parts": ["SN74AUP1G08QDCKRQ1"],
  "packages": {"p5": "DCK"},
  "converted": "2026-11-08"
}
---

# SN74AUP1G08-Q1 low-power single 2-input positive-AND gate

## Description

This single 2-input positive-AND gate performs the Boolean function
Y = A • B or Y = !{!{A} + !{B}} in positive logic. It is AEC-Q100
qualified (device temperature grade 1: –40 °C to 125 °C ambient
operating temperature range).

The AUP family is TI's solution to the low-power needs of the industry
in battery-powered portable applications. This family ensures a very low
static- and dynamic-power consumption across the entire V~CC~ range of
0.8 V to 3.6 V. Inputs and outputs are 3.6-V tolerant, and the inputs
have Schmitt-trigger action, which allows slow input transition and
better switching noise immunity at the input.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
Y: Output Y
GND: Ground
VCC: Supply voltage
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
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| ESD ratings | Human body model (HBM) AEC-Q100 classification level H2 | | | 2 | kV |
| ^ | Charged device model (CDM) AEC-Q100 classification level C3B | | | 750 | V |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
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
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DCK (5 pins) | Unit |
|---|---|--:|---|
| θ~JA~ | Junction-to-ambient thermal resistance | 304.7 | °C/W |
| θ~JCtop~ | Junction-to-case (top) thermal resistance | 115.3 | °C/W |
| θ~JB~ | Junction-to-board thermal resistance | 80.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 3.5 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 79.4 | °C/W |
| θ~JCbot~ | Junction-to-case (bottom) thermal resistance [2] | | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
[2] The source gives N/A for this metric.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | T~A~ = 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -20 μA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = -1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | 0.7 × V~CC~ | | V |
| ^ | ^ | I~OH~ = -1.7 mA | 1.4 V | 1.11 | | | 1.03 | | 1.03 | | V |
| ^ | ^ | I~OH~ = -1.9 mA | 1.65 V | 1.32 | | | 1.3 | | 1.3 | | V |
| ^ | ^ | I~OH~ = -2.3 mA | 2.3 V | 2.05 | | | 1.97 | | 1.97 | | V |
| ^ | ^ | I~OH~ = -3.1 mA | 2.3 V | 1.9 | | | 1.85 | | 1.85 | | V |
| ^ | ^ | I~OH~ = -2.7 mA | 3 V | 2.72 | | | 2.67 | | 2.67 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.6 | | | 2.55 | | 2.55 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 μA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | | 0.3 × V~CC~ | V |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | | 0.37 | V |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | | 0.35 | V |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | | 0.33 | V |
| ^ | ^ | I~OL~ = 3.1 mA | 2.3 V | | | 0.44 | | 0.45 | | 0.45 | V |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | | 0.33 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.44 | | 0.45 | | 0.45 | V |
| I~I~ | A or B input | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | | 0.5 | μA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | | 0.8 | μA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | | 0.8 | μA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | | 1.2 | μA |
| ΔI~CC~ | | V~I~ = V~CC~ - 0.6 V [1], I~O~ = 0 | 3.3 V | | | 40 | | 50 | | 23 [2] | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | | | pF |
| ^ | | ^ | 3.6 V | | 1.5 | | | | | | pF |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | | | pF |

[1] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND.
[2] Unconfirmed: the source gives 23 µA at 125 °C, below the 50 µA limit for –40 °C to 85 °C.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | Unit |
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
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | Unit |
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
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | Unit |
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
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | T~A~ = 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 32.8 | | | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 13.1 | 20.9 | 4.4 | 25.5 | 4.4 | 27.8 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9.5 | 14.2 | 2.9 | 16.9 | 2.9 | 18 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.7 | 11 | 2 | 13.5 | 2 | 19.7 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.7 | 7.6 | 1.3 | 9.4 | 1.3 | 11 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.7 | 6.2 | 1 | 7.5 | 1 | 8.7 | ns |
```

The source gives 125 °C switching limits only for C~L~ = 30 pF.

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

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. V~I~ = V~CC~ and V~M~ = V~CC~/2 at every V~CC~ (0.8 V to
3.3 V ± 0.3 V). The source heads the test-condition table
"T~A~ = -25°C to 85°C" (see [Conversion notes](#conversion-notes)).

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```

## Package

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74AUP1G08QDCKRQ1 | Reel of 3000 | -40 to 125 | SIJ, SIT
```

The source's ordering information table gives top-side marking SIT; the package option
addendum lists (SIJ, SIT). The catalog version, SN74AUP1G08, has its own
datasheet.

## Conversion notes

- Source pages 1–8 converted; the package option addendum, tape and reel
  and mechanical pages (9–16) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: Figure 1 (family power comparison) and Figure 2 (25 MHz
  switching waveform), the logic diagram, the NanoStar package text
  (no NanoStar part is offered), Figure 4 (enable and disable times
  load circuit and waveforms) and the pulse-duration and setup/hold
  waveforms of Figure 3, which the source notes are not applicable to
  all devices.
- The source has no pin-function table; the pin descriptions come from
  the DCK pinout drawing and the function table.
- From images: Y = !{!{A} + !{B}} overbars (p.2; the text layer reads
  "Y + A ) B"); Ω in "1 MΩ" and "50 Ω" (p.7, text layer "MW", "W").
- Source issues:
  - Unconfirmed: ΔI~CC~ maximum at T~A~ = 125 °C is 23 µA, lower than
    the 50 µA maximum at –40 °C to 85 °C (p.5).
  - Unconfirmed: the parameter-measurement test-condition tables in
    Figures 3 and 4 are headed "T~A~ = -25°C to 85°C", while the device
    is specified from –40 °C to 125 °C (p.7–8).
