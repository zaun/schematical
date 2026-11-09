---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc11a",
  "title": "SN74LVC11A triple 3-input AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc11a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS993A",
    "date": "2024-05",
    "pages": 30
  },
  "devices": ["gate:74_11"],
  "parts": ["SN74LVC11ABQAR", "SN74LVC11ADR", "SN74LVC11APWR"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-08"
}
---

# SN74LVC11A triple 3-input AND gates

## Description

This device contains three independent 3-input AND gates. Each gate
performs the Boolean function Y = A × B × C in positive logic. It
operates from 1.1V to 3.6V, and its input pins are 5.5V tolerant.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Channel input A
1B, 2B, 3B: Channel input B
1C, 2C, 3C: Channel input C
1Y, 2Y, 3Y: Channel output Y
GND: Ground
VCC: Positive supply
```

BQA package only: the thermal pad can be connected to GND or left
floating. Do not connect to any other signal or supply.

## Functional description

Logic diagram: inputs xA, xB and xC of each gate drive output xY.

Standard CMOS inputs are high impedance and are typically modeled as a
resistor from the input to ground in parallel with the input capacitance
given in the [electrical characteristics](#electrical-characteristics).
The worst case resistance is calculated with the maximum input voltage,
given in the [absolute maximum ratings](#absolute-maximum-ratings), and
the maximum input leakage current, using ohm's law (R = V ÷ I). Signals
applied to the inputs need to have fast edge rates, as defined by the
input transition time in the
[recommended operating conditions](#recommended-operating-conditions), to
avoid excessive current consumption and oscillations. If a slow or noisy
input signal is required, a device with a Schmitt-trigger input should
be used to condition the input signal prior to the standard CMOS input.

Balanced CMOS push-pull outputs: a balanced output allows the device to
sink and source similar currents. The drive capability of this device
may create fast edges into light loads, so routing and load conditions
should be considered to prevent ringing. Additionally, the outputs of
this device are capable of driving larger currents than the device can
sustain without being damaged. It is important for the output power of
the device to be limited to avoid damage due to over-current. The
electrical and thermal limits defined in the absolute maximum ratings
must be followed at all times.

The SN74LVC11A can drive a load with a total capacitance less than or
equal to the maximum load listed in the
[switching characteristics](#switching-characteristics) connected to a
high-impedance CMOS input while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied, however it is
not recommended to exceed the provided load value. If larger capacitive
loads are required, it is recommended to add a series resistor between
the output and the capacitor to limit output current to the values given
in the absolute maximum ratings.

Clamp diode structure: the inputs and outputs to this device have
negative clamping diodes only. Voltages beyond the values specified in
the absolute maximum ratings table can cause damage to the device. The
input and output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

## Function tables

```table id=function caption="Function table [1]"
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | H |
| L | X | X | L |
| X | L | X | L |
| X | X | L | L |

[1] H = high voltage level, L = low voltage level, X = do not care
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 V | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 V | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| I~O~ | Continuous output current through V~CC~ or GND | | | ±100 | mA |
| T~J~ | Junction temperature [3] | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Unconfirmed: the source gives a minimum of -65 °C for junction temperature, the same as storage temperature.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.1 | 3.6 | V |
| V~I~ | Input voltage | | | 5.5 | V |
| V~O~ | Output voltage | (High or low state) | | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.8 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.8 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 10 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
| V~IH~ | High-level input voltage | V~CC~ = 1.1 V | 0.75 | | V |
| ^ | ^ | V~CC~ = 1.2 V | 0.78 | | V |
| ^ | ^ | V~CC~ = 1.5 V | 0.975 | | V |
| ^ | ^ | V~CC~ = 1.65 V | 1.075 | | V |
| ^ | ^ | V~CC~ = 1.95 V | 1.2675 | | V |
| ^ | ^ | V~CC~ = 2.3 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.1 V | | 0.40 | V |
| ^ | ^ | V~CC~ = 1.2 V | | 0.42 | V |
| ^ | ^ | V~CC~ = 1.5 V | | 0.525 | V |
| ^ | ^ | V~CC~ = 1.65 V | | 0.5775 | V |
| ^ | ^ | V~CC~ = 1.95 V | | 0.6825 | V |
| ^ | ^ | V~CC~ = 2.3 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3.6 V | | 0.8 | V |
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | PW (TSSOP) | D (SOIC) | BQA (WQFN) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 150.8 | 127.8 | 102.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.3 | 81.9 | 96.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 93.8 | 84.4 | 70.9 | °C/W |
| Ψ~JT~ | Junction-to-top characterization parameter | 24.7 | 39.6 | 16.6 | °C/W |
| Ψ~JB~ | Junction-to-board characterization parameter | 93.2 | 83.9 | 70.9 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | 50.1 | °C/W |

[1] All packages 14 pins. For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100 µA | 1.1 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = –4 mA | 1.65 V | 1.2 | | | V |
| ^ | I~OH~ = –8 mA | 2.3 V | 1.75 | | | V |
| ^ | I~OH~ = –12 mA | 2.7 V | 2.2 | | | V |
| ^ | ^ | 3V | 2.4 | | | V |
| ^ | I~OH~ = –24 mA | 3V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100 µA [1] | 1.1 V to 3.6 V | | | 0.15 | V |
| ^ | I~OL~ = 4 mA [1] | 1.65 V | | | 0.45 | V |
| ^ | I~OL~ = 8 mA [1] | 2.3 V | | | 0.7 | V |
| ^ | I~OL~ = 12 mA [1] | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA [1] | 3V | | | 0.55 | V |
| I~I~ | V~I~ = V~CC~ or GND | 3.6 V | | | ±5 | µA |
| I~off~ | V~I~ or V~O~ = V~CC~ | 0V | | | ±10 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 40 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~I~ | V~I~ = V~CC~ or GND | 3.3 V | | 4.9 | | pF |
| C~O~ | V~O~ = V~CC~ or GND | 3.3 V | | 6.3 | | pF |
| C~PD~ | f = 10 MHz | 1.8 V | | 31 | | pF |
| ^ | ^ | 2.5 V | | 31 | | pF |
| ^ | ^ | 3.3 V | | 32 | | pF |

[1] Source error: the source writes the V~OL~ test conditions as I~OH~ = 100 µA, 4 mA, 8 mA, 12 mA and 24 mA; the recommended operating conditions give these values as low-level output current I~OL~.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range; typical values measured at T~A~ = 25°C (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | V~CC~ | -40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | ^ |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | A, B or C | Y | C~L~ = 15 pF | 1.2 V ± 0.1 V | | 12 | 23 | ns |
| ^ | ^ | ^ | C~L~ = 15 pF | 1.5 V ± 0.12 V | | 9 | 12 | ns |
| ^ | ^ | ^ | C~L~ = 30 pF | 1.8 V ± 0.15 V | | | 10.2 | ns |
| ^ | ^ | ^ | C~L~ = 30 pF | 2.5 V ± 0.2 V | | | 6.9 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | 2.7 V | | | 4.8 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | 3.3 V ± 0.3 V | | | 4.1 | ns |
| t~sk(o)~ | | | | 3.3 V ± 0.3 V | | | 1.5 | ns |
```

```table id=noise caption="Noise characteristics, V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | -0.8 | -0.3 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 2.2 | 3.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.0 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |
```

## Parameter measurement information

Phase relationships between waveforms were chosen arbitrarily. All input
pulses are supplied by generators having the following characteristics:
PRR ≤ 1MHz, Z~O~ = 50Ω, t~t~ ≤ 2.5ns. The outputs are measured
individually with one input transition per measurement. C~L~ includes
probe and test-fixture capacitance. The greater between t~PLH~ and
t~PHL~ is the same as t~pd~; the greater between t~r~ and t~f~ is the
same as t~t~. Inputs are measured at V~t~, outputs at 50 %.

```table id=pmi caption="Test conditions"
| V~CC~ | V~t~ | R~L~ | C~L~ | ΔV |
|---|---|---|---|---|
| 1.2V ± 0.1V | V~CC~/2 | 2kΩ | 15pF | 0.1V |
| 1.5V ± 0.12V | V~CC~/2 | 2kΩ | 15pF | 0.1V |
| 1.8V ± 0.15V | V~CC~/2 | 1kΩ | 30pF | 0.15V |
| 2.5V ± 0.2V | V~CC~/2 | 500Ω | 30pF | 0.15V |
| 2.7V | 1.5V | 500Ω | 50pF | 0.3V |
| 3.3V ± 0.3V | 1.5V | 500Ω | 50pF | 0.3V |
```

```figure ref=load-rl-cl caption="Load circuit for push-pull outputs"
RL: see table
CL: see table
```

```figure ref=waveforms-cmos-tpd-tt caption="Propagation delays and transition times"
VM: V~t~ (input), 50 % (output)
```

## Application information

Information in the applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

In this application, this device is used to directly control the RESET
pin of a motor controller. The controller requires three input signals
(power good PG, ON/OFF and over-current detection OC) to all be HIGH
before being enabled, and should be disabled in the event that any one
signal goes LOW. The 3-input AND gate function combines the three
individual reset signals into a single active-low reset signal.

Power considerations: ensure the desired supply voltage is within the
range specified in the
[recommended operating conditions](#recommended-operating-conditions).
The positive voltage supply must be capable of sourcing current equal to
the total current to be sourced by all outputs plus the maximum static
supply current, I~CC~, and any transient current required for switching.
Be sure to not exceed the maximum total current through V~CC~ listed in
the absolute maximum ratings. The ground must be capable of sinking
current equal to the total current to be sunk by all outputs plus the
maximum supply current, I~CC~, and any transient current required for
switching. Be sure to not exceed the maximum total current through GND.
The SN74LVC11A can drive a load with a total capacitance less than or
equal to 50pF while still meeting all of the data sheet specifications.
Larger capacitive loads can be applied; however, it is not recommended
to exceed 50pF. The SN74LVC11A can drive a load with total resistance
described by R~L~ ≥ V~O~ / I~O~, with the output voltage and current
defined in the electrical characteristics with V~OH~ and V~OL~. When
outputting in the HIGH state, the output voltage in the equation is
defined as the difference between the measured output voltage and the
supply voltage at the V~CC~ pin. The maximum junction temperature,
T~J(max)~, is an additional limitation to prevent damage to the device.
Do not violate any values listed in the absolute maximum ratings.

Input considerations: input signals must cross V~IL(max)~ to be
considered a logic LOW, and V~IH(min)~ to be considered a logic HIGH. Do
not exceed the maximum input voltage range found in the absolute maximum
ratings. Unused inputs must be terminated to either V~CC~ or ground. The
unused inputs can be directly terminated if the input is completely
unused, or they can be connected with a pull-up or pull-down resistor if
the input will be used sometimes, but not always. A 10kΩ resistor value
is often used. The SN74LVC11A has CMOS inputs and thus requires fast
input transitions to operate correctly. Slow input transitions can cause
oscillations, additional power consumption, and reduction in device
reliability.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output (negative clamps only)"
```

Output considerations: drawing current from the output will decrease the
output voltage as specified by the V~OH~ specification; sinking current
into the output will increase the output voltage as specified by the
V~OL~ specification. Push-pull outputs that could be in opposite states,
even for a very short time period, should never be connected directly
together. This can cause excessive current and damage to the device. Two
channels within the same device with the same input signals can be
connected in parallel for additional output drive strength. Unused
outputs can be left floating. Do not connect outputs directly to V~CC~
or ground.

Detailed design procedure:

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Ensure the capacitive load at the output is ≤ 70pF. This is not a
   hard limit; by design, however, it will optimize performance.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω, so that the maximum output current from the
   absolute maximum ratings is not violated.
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   Cpd Calculation.

Power supply: the power supply can be any voltage between the minimum
and maximum supply voltage rating located in the recommended operating
conditions. Each V~CC~ terminal should have a bypass capacitor to prevent
power disturbance. A 0.1μF capacitor is recommended for this device. The
0.1μF and 1μF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power terminal as possible
for best results.

Layout: when using multiple-input and multiple-channel logic devices
inputs must not ever be left floating. All unused inputs of digital logic
devices must be connected to a logic high or logic low voltage, as
defined by the input voltage specifications, to prevent them from
floating. Generally, the inputs are tied to GND or V~CC~, whichever makes
more sense for the logic function or is more convenient. Example layout:
unused inputs tied to GND or V~CC~; unused output left floating; bypass
capacitor placed close to the device; avoid 90° corners for signal
lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74LVC11ABQAR | Reel of 3000 | -40 to 125 | LVC11A
SN74LVC11ADR | Reel of 3000 | -40 to 125 | (LVC11A, LVC11AQ)
SN74LVC11APWR | Reel of 3000 | -40 to 125 | LVC11A
```

## Conversion notes

- Source pages 1–15 converted; the package option addendum, tape and
  reel and mechanical pages dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features and applications lists, package information table,
  revision history, support pages, the related-documentation list; the
  application schematic (Figure 8-1), its timing diagram (Figure 8-2) and
  the example layout (Figure 8-3) are described in text.
- Not converted: typical characteristics (Figures 5-1 to 5-13, supply
  current vs input and supply voltage, output voltage vs output current).
- The PDF text layer prints Ψ~JB~ as "YJB" (symbol font); the page
  image shows Ψ~JB~.
- Source issues:
  - Source error: the V~OL~ rows of the electrical characteristics give
    their test currents as I~OH~ (p.5); the recommended operating
    conditions (p.4) give 4, 8, 12 and 24 mA as I~OL~. Corrected to
    I~OL~ (see table note).
  - Source conflict: the design requirements say it is not recommended
    to exceed 50pF of load (p.12); the detailed design procedure says to
    ensure the capacitive load is ≤ 70pF (p.13).
  - Source conflict: the feature list gives the charged-device model as
    C101 (p.1); the ESD ratings give JS-002 (p.4).
  - Unconfirmed: junction temperature minimum -65 °C (p.4).
