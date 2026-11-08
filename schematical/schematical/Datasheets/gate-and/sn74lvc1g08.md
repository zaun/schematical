---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g08",
  "title": "SN74LVC1G08 single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc1g08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES217AA",
    "date": "2026-08",
    "pages": 50
  },
  "devices": ["gate:1G08", "gate:1G08_DPW", "gate:1G08_SON"],
  "parts": [
    "SN74LVC1G08DBVR", "SN74LVC1G08DBVRE4", "SN74LVC1G08DBVRG4", "SN74LVC1G08DBVT",
    "SN74LVC1G08DBVTE4", "SN74LVC1G08DCKR", "SN74LVC1G08DCKRE4", "SN74LVC1G08DCKRG4",
    "SN74LVC1G08DCKT", "SN74LVC1G08DCKTE4", "SN74LVC1G08DCKTG4", "SN74LVC1G08DRLR",
    "SN74LVC1G08DRLRG4", "SN74LVC1G08YZPR", "SN74LVC1G08DPWR", "SN74LVC1G08DRY2",
    "SN74LVC1G08DRYR", "SN74LVC1G08DRYRG4", "SN74LVC1G08DSF2", "SN74LVC1G08DSFR",
    "SN74LVC1G08DSFRG4"
  ],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-08"
}
---

# SN74LVC1G08 single 2-input positive-AND gate

## Description

The SN74LVC1G08 device is a single 2-input positive-AND gate. The device
performs the Boolean function Y = A · B in positive logic.
It operates from 1.65 V to 5.5 V V~CC~; the inputs accept voltages to 5.5 V
at any V~CC~, so the device can translate down to V~CC~. Outputs are
balanced push-pull outputs. I~off~ circuitry disables the output when V~CC~
is 0 V, for partial-power-down applications.

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

The DPW package swaps A and B: A is pin 2 and B is pin 1.

```pins
A: Input
B: Input
GND: Ground
Y: Output
VCC: Power pin
NC: Not connected
```

## Function tables

```table id=function caption="Function table"
| A [1] | B [1] | Y [2] |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~J(max)~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 0 | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 0 | 1000 | V |

[1] JEDEC document JEP155 states that 500 V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250 V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Package | Pins | R~θJA~ | R~θJC(top)~ | R~θJB~ | Ψ~JT~ | Ψ~JB~ | R~θJC(bot)~ | Unit |
|---|--:|--:|--:|--:|--:|--:|--:|---|
| DBV (SOT-23) | 5 | 357.1 | 263.7 | 264.4 | 195.6 | 262.2 | | °C/W |
| DCK (SOT-SC70) | 5 | 371 | 297.5 | 258.6 | 195.6 | 256.2 | | °C/W |
| DRL (SOT-5X3) | 5 | 364.1 | 280.6 | 261.5 | 195.6 | 259.2 | | °C/W |
| DRY (SON) | 6 | 438.8 | 276.8 | 271.7 | 83.8 | 271.4 | | °C/W |
| YZP (DSBGA) | 5 | 130 | 54 | 51 | 1 | 50 | | °C/W |
| DPW (X2SON) | 5 | 340 | 215 | 294 | 41 | 294 | 250 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
```

The source gives no thermal data for the DSF package.

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40 °C to 85 °C | < | < | –40 °C to 125 °C recommended | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.15 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | Partial-power-down leakage | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | Supply current | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | Supply-current change | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | | 4 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25 °C.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | –40 °C to 85 °C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.5 | 7.2 | 0.7 | 4.4 | 0.8 | 3.6 | 0.8 | 3.4 | ns |
```

```table id=sw-1v8-2v5 caption="Switching characteristics, 1.8 V and 2.5 V" conditions="Over recommended operating free-air temperature range, (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40 °C to 85 °C | < | –40 °C to 125 °C recommended | < | –40 °C to 85 °C | < | –40 °C to 125 °C recommended | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 2.5 V ± 0.2 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.4 | 8 | 2.4 | 10 | 1.1 | 5.5 | 1.1 | 7 | ns |
```

```table id=sw-3v3-5v caption="Switching characteristics, 3.3 V and 5 V" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 6-2)"
| Symbol | From (input) | To (output) | –40 °C to 85 °C | < | –40 °C to 125 °C recommended | < | –40 °C to 85 °C | < | –40 °C to 125 °C recommended | < | Unit |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1 | 4.5 | 1 | 6 | 1 | 4 | 1 | 5 | ns |
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 21 | 24 | 26 | 31 | pF |
```

## Typical characteristics

```curve id=tpd-temp caption="t~pd~ across temperature at 3.3 V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 2.7], [-37, 2.9], [0, 3.0], [25, 3.1], [85, 3.7], [125, 5.0]]}
 ]}
```

```curve id=tpd-vcc caption="t~pd~ across V~CC~ at 25 °C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD", "points": [[1.8, 6.9], [2.5, 4.0], [3.3, 3.0], [5.0, 2.5]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

The load circuit also has an R~L~ from the output to switch S1 (V~LOAD~,
open or GND). S1 is open for t~PLH~/t~PHL~, which leaves the load below.
The source figure also shows pulse-duration, setup/hold and
enable/disable waveforms; all parameters and waveforms are not
applicable to all devices.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions caption="Test conditions (Figure 6-1 and Figure 6-2)"
| V~CC~ | Figure | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | 6-1 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | 6-1 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 6-1 | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | 6-1 | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
| 1.8 V ± 0.15 V | 6-2 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 6-2 | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 6-2 | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | 6-2 | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: see table
```

## Application information

A common application for AND gates is power sequencing: a processor or
other delicate device with specific voltage timing requirements. Using
the SN74LVC1G08 to verify that the processor has turned on can protect it
from any harmful signals. In the source's example (Figure 8-1), A is tied
to V~CC~ = 5 V, an MCU (MSP43x) drives B, Y drives the EN input of a
temperature sensor, and the sensor's V~O~ returns to the MCU.

Outputs: balanced CMOS push-pull; the device can sink and source similar
currents. The drive capability can create fast edges into light loads,
so routing and load conditions must be considered to prevent ringing.
The outputs are capable of driving larger currents than the device can
sustain without being damaged; limit the output power. The limits in the
[absolute maximum ratings](#absolute-maximum-ratings) must be followed at
all times. Take care to avoid bus contention. Push-pull outputs that can
be in opposite states, even for a very short time period, must never be
connected directly together. Two channels within the same device with
the same input signals can be connected in parallel for additional
output drive strength. Unused push-pull CMOS outputs must be left
disconnected; unused outputs can be left floating. Do not connect
outputs directly to V~CC~ or ground.

Partial power down (I~off~): circuitry disables all outputs when the
supply pin is held at 0 V; the outputs then neither source nor sink
current, regardless of the input voltages.

Inputs: standard CMOS, high impedance, modeled as a resistor in parallel
with the input capacitance; the worst-case resistance is the maximum
input voltage divided by the maximum input leakage current (R = V ÷ I).
Inputs are over-voltage tolerant: they support high-state input voltages
larger than the supply voltage, as the input protection circuit does not
include a standard positive clamp diode. Do not exceed the maximum input
voltage range in the absolute maximum ratings. Input signals must cross
V~IL~(max) to be considered a logic LOW and V~IH~(min) to be considered
a logic HIGH, and must transition quickly, as defined by Δt/Δv in the
[recommended operating conditions](#recommended-operating-conditions).
Failing to meet this results in excessive power consumption and can
cause oscillations. All input signals must remain as close as possible
to either 0 V or V~CC~ for peak operation. Do not leave standard CMOS
inputs floating at any time during operation. Terminate unused inputs at
V~CC~ or GND; an input used sometimes, but not always, can have a pullup
(default HIGH) or pulldown (default LOW) resistor. A 10 kΩ resistor is
recommended and typically meets all requirements.

```figure ref=clamp-diodes caption="Electrical placement of clamping diodes for each input and output"
```

The inputs and outputs have negative clamping diodes only. The input and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

Loads: the device can drive a total capacitance less than or equal to
50 pF while still meeting all of the data sheet specifications. Larger
capacitive loads can be applied; however, TI recommends adding a series
resistor between the output and capacitor. The total load resistance is
R~L~ ≥ V~O~ / I~O~, with V~O~ and I~O~ from V~OH~ and V~OL~ in the
[electrical characteristics](#electrical-characteristics); in the HIGH
state, V~O~ is the difference between the measured output voltage and
the supply voltage at the V~CC~ pin. Keep load currents not exceeding
±50 mA.

Design procedure: inputs and outputs are overvoltage tolerant and can
therefore go as high as 5.5 V at any valid V~CC~ (see Source conflict in
[Conversion notes](#conversion-notes)).

Power: the supply can be any voltage between the minimum and maximum
supply voltage rating in the recommended operating conditions. The supply
must be capable of sourcing, and the ground of sinking, the total current
of all outputs plus the maximum static supply current I~CC~ and any
transient switching current; do not exceed the maximum total current
through V~CC~ or GND. Do not violate the junction temperature T~J(max)~.
A 0.1 µF bypass capacitor is recommended on each V~CC~ terminal; 0.1 µF
and 1 µF are commonly used in parallel to reject different frequencies of
noise.

Layout: place the bypass capacitor near the positive supply terminal,
with an electrically short ground return path and wide traces; keep the
device, capacitors and traces on the same side of the board whenever
possible. Signal traces: 8 mil to 12 mil width, lengths less than 12 cm
to minimize transmission line effects, no 90° corners, an unbroken ground
plane below, ground flood fill around them. Parallel traces must be
separated by at least 3x dielectric thickness. For traces longer than
12 cm, use impedance controlled traces, source-terminate using a series
damping resistor near the output, and avoid branches; buffer each signal
that must branch separately.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=X2SON-5-DPW
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–15 converted; support pages, revision history (16–17),
  the package option addendum, tape and reel and mechanical pages
  (18–50) dropped, replaced by the generated Package and Ordering
  sections.
- Dropped: features and applications lists, package information table,
  logic diagram (one AND gate), layout example figures (Figures 8-3 to
  8-7, generic; they show 14- and 20-pin packages), the application
  timing diagram (Figure 8-2, V~CC~, GPO, V~O~ and EN steps); the power
  sequencing circuit (Figure 8-1) is described in text, with its labels
  (MCU (MSP43x), Temp Sensor, EN, V~O~) read from the page image.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 6-1 and 6-2 are the
  same circuit with different values, joined in one table.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
- The text layer prints "£2 ns", "1 MW" and "500 W"; the page image
  shows ≤ 2 ns, 1 MΩ and 500 Ω.
- Parameter names in the electrical characteristics table are added
  (the source gives symbols only).
- The source's Pin Functions table gives YZP pin numbers 1–5 in the DBV
  column; the catalog names the balls (A1, B1, C1, C2, A2).
- Source issues:
  - Source error: the second form of the Boolean function, "Y = A + B"
    with overbars, is printed with one bar over A + B (§ 7.1, p.10) and
    as Ā + B̅ in the text layer (p.1); neither is an AND. The function
    table (p.11) gives Y = A · B. Kept out.
  - Source conflict: § 8.2.2 says inputs and outputs "can therefore go as
    high as 5.5V at any valid VCC"; the absolute maximum ratings limit
    V~O~ in the high or low state to V~CC~ + 0.5 V (p.4).
  - Source error: § 8.2.1.2 says the fast input transitions are "defined
    in the Electrical Characteristics table"; Δt/Δv is in the Recommended
    Operating Conditions (p.5), as § 7.3.3 and § 8.2.2 say. Corrected.
  - Source error: § 8.2.1.1 says the supply voltage range is "specified
    in the Electrical Characteristics"; it is in the Recommended Operating
    Conditions (p.5), as § 8.3 says. Kept out.
  - Unconfirmed: Ψ~JT~ is 195.6 °C/W for all three of DBV, DCK and DRL
    (p.5).
