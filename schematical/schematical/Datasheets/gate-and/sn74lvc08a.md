---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc08a",
  "title": "SNx4LVC08A quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc08a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS283W",
    "date": "2024-07",
    "pages": 45
  },
  "devices": ["gate:74_08"],
  "parts": ["5962-9753401QCA", "5962-9753401QDA", "SN74LVC08ABQAR", "SN74LVC08AD", "SN74LVC08ADBR", "SN74LVC08ADBRE4", "SN74LVC08ADE4", "SN74LVC08ADG4", "SN74LVC08ADR", "SN74LVC08ADRE4", "SN74LVC08ADRG3", "SN74LVC08ADRG4", "SN74LVC08ADT", "SN74LVC08ANSR", "SN74LVC08ANSRE4", "SN74LVC08APW", "SN74LVC08APWG4", "SN74LVC08APWR", "SN74LVC08APWRE4", "SN74LVC08APWRG3", "SN74LVC08APWRG4", "SN74LVC08APWT", "SN74LVC08APWTG4", "SN74LVC08ARGYR", "SN74LVC08ARGYRG4", "SNJ54LVC08AJ", "SNJ54LVC08AW", "5962-9753401Q2A", "SNJ54LVC08AFK"],
  "packages": {"p14": "D, DB, NS, J, W, PW, BQA, RGY", "lccc20": "FK"},
  "converted": "2026-11-08"
}
---

# SNx4LVC08A quadruple 2-input positive-AND gates

## Description

The SN54LVC08A quadruple 2-input positive-AND gate is designed for 2.7 V
to 3.6 V V~CC~ operation, and the SN74LVC08A quadruple 2-input
positive-AND gate is designed for 1.65 V to 3.6 V V~CC~ operation. The
devices perform the Boolean function Y = A · B or Y = !{!{A} + !{B}} in
positive logic.

Inputs can be driven from either 3.3 V or 5 V devices. This feature
allows the use of these devices as translators in a mixed 3.3 V/5 V
system environment. SNx4LVC08A is specified from –40 °C to +85 °C and
–40 °C to +125 °C; SN54LVC08A from –55 °C to +125 °C.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel input A
1B, 2B, 3B, 4B: Channel input B
1Y, 2Y, 3Y, 4Y: Channel output
GND: Ground
VCC: Positive supply
NC: No internal connection
```

BQA package only: the thermal pad can be connected to GND or left
floating. Do not connect to any other signal or supply.

## Functional description

Each gate performs Y = A · B. Logic diagram: four 2-input AND gates.

Balanced high-drive CMOS push-pull outputs: a balanced output allows the
device to sink and source similar currents. The high drive capability
creates fast edges into light loads, so routing and load conditions
should be considered to prevent ringing. The outputs are capable of
driving larger currents than the device can sustain without being
damaged. It is important for the power output of the device to be
limited to avoid thermal runaway and damage due to over-current. The electrical and thermal
limits defined in the [absolute maximum ratings](#absolute-maximum-ratings)
must be followed at all times.

Standard CMOS inputs are high impedance and are typically modelled as a
resistor in parallel with the input capacitance given in the
[electrical characteristics](#electrical-characteristics). The worst case
resistance is calculated with the maximum input voltage and the maximum
input leakage current, using ohm's law (R = V ÷ I). Signals applied to
the inputs need to have fast edge rates, as defined by Δt/Δv in the
[recommended operating conditions](#recommended-operating-conditions), to
avoid excessive currents and oscillations. If a slow or noisy input
signal is required, a device with a Schmitt-trigger input should be
utilized to condition the input signal.

Clamp diodes: the inputs have negative clamping diodes. The outputs have
both positive and negative clamping diodes. Voltages beyond the absolute
maximum ratings can cause damage to the device. The input
negative-voltage and output voltage ratings may be exceeded if the input
and output clamp-current ratings are observed.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

Over-voltage tolerant inputs: input signals can be driven above the
supply voltage so long as they remain below the maximum input voltage
value specified in the absolute maximum ratings.

## Function tables

```table id=function caption="Truth table (each gate)"
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
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| P~tot~ | Power dissipation [4] [5] | T~A~ = –40 °C to +125 °C | | 500 | mW |
| T~J~ | Junction temperature [6] | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
[4] For the D package: above 70 °C, the value of P~tot~ derates linearly with 8 mW/K.
[5] For the DB, NS, and PW packages: above 60 °C, the value of P~tot~ derates linearly with 5.5 mW/K.
[6] Unconfirmed: the source gives a minimum of –65 °C for junction temperature, the same as storage temperature.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |
| ^ | ^ | Machine Model (MM) A115-A | 200 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc-54 caption="Recommended operating conditions, SN54LVC08A [1]" parts=SNJ54*,5962-* conditions="Over operating free-air temperature range (unless otherwise noted), –55 °C to +125 °C"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 8 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

```table id=roc-74 caption="Recommended operating conditions, SN74LVC08A [1]" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | T~A~ = 25 °C | < | –40 °C to +85 °C | < | –40 °C to +125 °C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 3.6 | 1.65 | 3.6 | 1.65 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | 1.5 | | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | 0.65 × V~CC~ | | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | 1.7 | | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | | 0.35 × V~CC~ | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | | 0.7 | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | | 0.8 | | 0.8 | V |
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
| Δt/Δv | Input transition rise or fall rate | | | 8 | | 8 | | 8 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74LVC08A [1]"
| Symbol | Parameter | BQA (WQFN) | D (SOIC) | DB (SSOP) | NS (SO) | PW (TSSOP) | RGY (VQFN) [2] | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 102.3 | 127.8 | 140.4 | 123.8 | 150.8 | 92.1 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 96.8 | 81.9 | 65.3 | 52.7 | 56.0 | 56.6 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 70.9 | 84.4 | 60.2 | 53.9 | 69.5 | 27.5 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 16.6 | 39.6 | 25.3 | 17.9 | 8.9 | 4.5 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 70.9 | 83.9 | 59.6 | 53.6 | 68.9 | 27.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | 50.1 | | | | | 19.1 | °C/W |

[1] All packages 14 pins. For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report. R~θJC(bot)~ is N/A for the D, DB, NS and PW packages.
[2] Source error: the thermal table labels the RGY package "LCCC"; the device information table and pin configuration give RGY as VQFN.
```

## Electrical characteristics

```table id=ec-54 caption="Electrical characteristics, SN54LVC08A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | –55 °C to +125 °C | < | < | Unit |
| ^ | ^ | < | ^ | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | < | 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -12 mA | < | 2.7 V | 2.2 | | | V |
| ^ | ^ | < | 3 V | 2.4 | | | V |
| ^ | I~OH~ = -24 mA | < | 3 V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100 µA | < | 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 12 mA | < | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA | < | 3 V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | < | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND | I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V | Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | < | 3.3 V | | 5 | | pF |

[1] T~A~ = 25 °C
```

```table id=ec-74 caption="Electrical characteristics, SN74LVC08A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | –40 °C to +85 °C | < | –40 °C to +125 °C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V~CC~ - 0.2 | | V~CC~ - 0.3 | | V |
| ^ | I~OH~ = -4 mA | 1.65 V | 1.29 | | | 1.2 | | 1.05 | | V |
| ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.7 | | 1.55 | | V |
| ^ | I~OH~ = -12 mA | 2.7 V | 2.2 | | | 2.2 | | 2.05 | | V |
| ^ | ^ | 3 V | 2.4 | | | 2.4 | | 2.25 | | V |
| ^ | I~OH~ = -24 mA | 3 V | 2.3 | | | 2.2 | | 2 | | V |
| V~OL~ | I~OL~ = 100 µA | 1.65 V to 3.6 V | | | 0.1 | | 0.2 | | 0.3 | V |
| ^ | I~OL~ = 4 mA | 1.65 V | | | 0.24 | | 0.45 | | 0.6 | V |
| ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | 0.7 | | 0.75 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | | 0.4 | | 0.6 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | | 0.55 | | 0.8 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±1 | | ±5 | | ±20 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 1 | | 10 | | 40 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | | 500 | | 5000 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | | pF |
```

```table id=operating caption="Operating characteristics, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 1.8 V | 7 | pF |
| ^ | ^ | ^ | 2.5 V | 9.8 | pF |
| ^ | ^ | ^ | 3.3 V | 10 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement) for the
load circuit and voltage waveforms.

```table id=sw-54 caption="Switching characteristics, SN54LVC08A" parts=SNJ54*,5962-* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ | –55 °C to +125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | ^ |
|---|---|---|---|--:|--:|---|
| t~pd~ | A or B | Y | 2.7 V | | 4.8 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 4.1 | ns |
```

```table id=sw-74 caption="Switching characteristics, SN74LVC08A" parts=SN74* conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25 °C | < | < | –40 °C to +85 °C | < | –40 °C to +125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.8 V ± 0.15 V | 1 | 5 | 9.3 | 1 | 9.8 | 1 | 11.3 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 2.9 | 6.4 | 1 | 6.9 | 1 | 9 | ns |
| ^ | ^ | ^ | 2.7 V | 1 | 3 | 4.6 | 1 | 4.8 | 1 | 6 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.6 | 3.9 | 1 | 4.1 | 1 | 5.5 | ns |
| t~sk(o)~ | | | 3.3 V ± 0.3 V | | | | | 1 | | 1.5 | ns |
```

## Parameter measurement information

For t~PLH~/t~PHL~ the switch S1 is open, which leaves R~L~ and C~L~ from
the output to ground. C~L~ includes probe and jig capacitance. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 10 MHz, Z~O~ = 50 Ω. The outputs are measured one at a time, with
one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices.

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~D~ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: see table
CL: see table
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Application information

Information in the applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

The SN74LVC08A is used to drive CMOS devices and for implementing AND
logic. The LVC family can support current drive of about 24 mA at 3-V
V~CC~. The inputs are 5.5-V tolerant, allowing it to translate down to
V~CC~.

Typical application: the four AND gates can be used for individual AND
function or to implement complex Boolean logic. Figure 8-1 of the source
shows an example of implementing a 3-input AND function, Z = A × B × C,
driving an LED. The device uses CMOS technology and has balanced output drive. Take care
to avoid bus contention because it can drive currents that would exceed
maximum limits. The high drive will also create fast edges into light
loads, so routing and load conditions should be considered to prevent
ringing. TI recommends to use a series resistance to limit the LED
current. With V~CC~ = 3 V, an LED current of 10 mA and an LED forward
voltage of 2.5 V:

```equation
R = (V~CC~ – V~LED~) / I
R = (3 – 2.5) / 0.01 = 50 Ω
```

Power supply: the power supply can be any voltage between the minimum
and maximum supply voltage rating in the
[recommended operating conditions](#recommended-operating-conditions).
The V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. TI recommends to use a 0.1-µF capacitor. 0.1-µF and 1-µF
capacitors are commonly used in parallel. The bypass capacitor should be
installed as close to the power pin as possible.

Layout: inputs should not float. All unused inputs of digital logic
devices must be connected to a high or low bias to prevent them from
floating; generally they will be tied to GND or V~CC~. Unused outputs
are left floating. Avoid 90° corners for signal lines: at the apex of the
turn the trace width increases to 1.414 times the width, which upsets the
transmission-line characteristics and results in a reflection. Rounded
corners that maintain constant trace width minimize reflections.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9753401Q2A | Tube of 55 | -55 to 125 | 5962-9753401Q2A SNJ54LVC08AFK
5962-9753401QCA | Tube of 25 | -55 to 125 | 5962-9753401QCA SNJ54LVC08AJ
5962-9753401QDA | Tube of 25 | -55 to 125 | 5962-9753401QDA SNJ54LVC08AW
SN74LVC08ABQAR | Reel of 3000 | -40 to 125 | LVC08A
SN74LVC08AD | Tube of 50 | -40 to 125 | LVC08A
SN74LVC08ADBR | Reel of 2000 | -40 to 125 | LC08A
SN74LVC08ADBRE4 | Reel of 2000 | -40 to 125 | LC08A
SN74LVC08ADE4 | Tube of 50 | -40 to 125 | LVC08A
SN74LVC08ADG4 | Tube of 50 | -40 to 125 | LVC08A
SN74LVC08ADR | Reel of 2500 | -40 to 125 | LVC08A
SN74LVC08ADRE4 | Reel of 2500 | -40 to 125 | LVC08A
SN74LVC08ADRG3 | Reel of 2500 | -40 to 125 | LVC08A
SN74LVC08ADRG4 | Reel of 2500 | -40 to 125 | LVC08A
SN74LVC08ADT | Reel of 250 | -40 to 125 | LVC08A
SN74LVC08ANSR | Reel of 2000 | -40 to 125 | LVC08A
SN74LVC08ANSRE4 | Reel of 2000 | -40 to 125 | LVC08A
SN74LVC08APW | Tube of 90 | -40 to 125 | LC08A
SN74LVC08APWG4 | Tube of 90 | -40 to 125 | LC08A
SN74LVC08APWR | Reel of 2000 | -40 to 125 | LC08A
SN74LVC08APWRE4 | Reel of 2000 | -40 to 125 | LC08A
SN74LVC08APWRG3 | Reel of 2000 | -40 to 125 | LC08A
SN74LVC08APWRG4 | Reel of 2000 | -40 to 125 | LC08A
SN74LVC08APWT | Reel of 250 | -40 to 125 | LC08A
SN74LVC08APWTG4 | Reel of 250 | -40 to 125 | LC08A
SN74LVC08ARGYR | Reel of 3000 | -40 to 125 | LC08A
SN74LVC08ARGYRG4 | Reel of 3000 | -40 to 125 | LC08A
SNJ54LVC08AFK | Tube of 55 | -55 to 125 | 5962-9753401Q2A SNJ54LVC08AFK
SNJ54LVC08AJ | Tube of 25 | -55 to 125 | 5962-9753401QCA SNJ54LVC08AJ
SNJ54LVC08AW | Tube of 25 | -55 to 125 | 5962-9753401QDA SNJ54LVC08AW
```

## Conversion notes

- Source pages 1–17 converted; the package option addendum, tape and
  reel and mechanical pages dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features and applications lists, device information table,
  revision history, support pages, the layout example drawing and the
  trace-corner drawing (described in text), the enable/disable and
  setup/hold waveforms of the generic measurement page (they do not
  apply to this part).
- Not converted: typical characteristics (Figures 5-1 and 5-2, t~pd~ vs
  C~L~) and application curves (Figures 8-2 and 8-3, I~OL~ vs V~OL~ and
  I~OH~ vs V~OH~).
- The application circuit (Figure 8-1) is described in text.
- The Boolean function in the description is garbled in the PDF text
  layer ("Y + A • B or Y + A ) B"); written from the overview (Y = A × B).
- Source issues:
  - Source conflict: the description says SN54LVC08A is designed for
    2.7 V to 3.6 V V~CC~ operation (p.1), while the feature list
    (p.1, "2.0V to 3.6V") and the SN54 recommended operating conditions
    (p.5, operating V~CC~ min 2) give 2 V.
  - Source error: the thermal table header gives RGY as "LCCC" (p.6);
    the device information table (p.1) and Figure 4-2 (p.3) give RGY as
    VQFN. Corrected in the table header.
  - Source error: the pin functions table names the thermal pad row
    "Thermal Information" (p.4); its description is about the thermal
    pad. Written as the thermal pad.
  - Unconfirmed: junction temperature minimum –65 °C (p.5).
