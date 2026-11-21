---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahcu04-ep",
  "title": "SN74AHCU04-EP hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahcu04-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS706",
    "date": "2007-03",
    "pages": 12
  },
  "devices": ["gate:74_U04"],
  "parts": ["SN74AHCU04MPWREP"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHCU04-EP hex inverters

## Description

The ’HC04 devices contain six independent inverters. They perform the
Boolean function Y = !{A} in positive logic.

Source error: the description (p.1) names the ’HC04 devices; the data sheet's title and its ordering table give the SN74AHCU04-EP. Kept as printed.

Controlled Baseline. One Assembly Site. One Test Site. One Fabrication
Site. Extended Temperature Performance of –55°C to 125°C. Enhanced
Diminishing Manufacturing Sources (DMS) Support. Enhanced Product-Change
Notification. Qualification Pedigree. Operating Range 2-V to 5.5-V
V~CC~. Unbuffered Outputs. Latch-Up Performance Exceeds 100 mA Per
JESD 78, Class II. ESD Protection Exceeds JESD 22. 2000-V Human-Body
Model (A114-A). 200-V Machine Model (A115-A). 1000-V Charged-Device
Model (C101).

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. This
includes, but is not limited to, Highly Accelerated Stress Test (HAST)
or biased 85/85, temperature cycle, autoclave or unbiased HAST,
electromigration, bond intermetallic life, and mold compound life. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y (unbuffered)
GND: Ground
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ = 0 to V~CC~ [4] | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | V~CC~ or GND | | ±50 | mA |
| θ~JA~ | Package thermal impedance [3] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -60 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output clamp-current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
[4] Unconfirmed: the condition of the output clamp current is printed "V~O~ < 0 or V~O~ = 0 to V~CC~", which repeats the condition of the continuous output current row below it.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.4 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 4.4 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.3 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.6 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.1 | ^ |
| V~I~ | Input voltage | | 0 | V~CC~ | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-Level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 | | -8 | ^ |
| I~OL~ | Low-Level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 | | 8 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | T~A~ = –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.8 | | 1.8 | | V |
| ^ | ^ | ^ | 3 V | 2.7 | | 2.7 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4 | | 4 | | ^ |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | 2.3 | | ^ |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | 3.5 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | 0.1 | | 0.2 | V |
| ^ | ^ | ^ | 3 V | | 0.1 | | 0.3 | ^ |
| ^ | ^ | ^ | 4.5 V | | 0.1 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | 0.26 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | 0.26 | | 0.5 | ^ |
| I~I~ | | V~I~ = V~CC~ or GND | 0 V to 5.5 V | | ±0.1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, O = 0 [1] | 5.5 V | | 2 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 10 | | 10 | pF |

[1] Unconfirmed: the second condition of the I~CC~ row is printed as a subscript O alone, "O = 0", without its symbol.
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.5 | | V |
| V~OL(V)~ | Quiet output, minimun dynamic V~OL(V)~ | | -0.5 | | V |
| V~OH(V)~ | Quiet output, minimun dynamic V~OH~ | | 4.3 | | V |
| V~IH(D)~ | HIgh-level dynamic input voltage V~IH(D)~ | 4 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage V~IH(D)~ | | | 1 | V |

[1] Characterists are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 7.3 | pF |
```

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 ± 0.3 V" conditions="Over operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ [1] | T~A~ = 25°C | < | T~A~ = –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 50 pF | 1 | 10.6 | 1 | 14 | ns |
| t~PHL~ | ^ | ^ | ^ | 1 | 10.6 | 1 | 14 | ns |

[1] Source error: the source heads this column V~CC~; its cell gives the load capacitance, C~L~ = 50 pF, and V~CC~ is in the line over the table. Kept as printed.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 ± 0.5 V" conditions="Over operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ [1] | T~A~ = 25°C | < | T~A~ = –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 50 pF | 1 | 7 | 1 | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | 1 | 7 | 1 | 11 | ns |

[1] Source error: the source heads this column V~CC~; its cell gives the load capacitance, C~L~ = 50 pF, and V~CC~ is in the line over the table. Kept as printed.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one transition per measurement. All parameters and waveforms
are not applicable to all devices.

Unconfirmed: note C of Figure 1 (p.4) prints "PRR ≤ MHz", with no number.

```figure ref=load-cmos-50pf caption="Figure 1. Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Figure 1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: the load circuit of the source figure (Figure 1) prints
no value for C~L~, which is 50 pF in the switching characteristics; the
figure also shows the load circuit for 3-state and open-drain outputs
with its S1 table, and the pulse-duration, setup-and-hold and
enable/disable waveforms with their note B, which are not drawn here.

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHCU04MPWREP | Reel of 2000 | -55 to 125 | AHCU04M | NRND
```

## Conversion notes

- Source pages 1–4 converted. The ordering line is from the package
  option addendum (p.5; it ends on p.6). Tape and reel and mechanical
  pages (7–12) are left out; the generated Package section replaces
  them.
- Dropped: the logic diagram (one inverter, input A, output Y), the
  ordering table on p.1 with its two notes (it gives "Tape and Reel";
  the Ordering block follows the package option addendum).
- The catalog does not carry V62/07619-01XE, which the package option
  addendum lists.
- The feature list is written as sentences in the Description; the
  footnote of "Qualification Pedigree" follows them.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. Its I~I~ row is printed with
  V~I~ = V~CC~ or GND and its C~i~ condition ends with a comma.
- The source has no pin table; the pin descriptions are the converter's.
- From images: the overbar of Y = !{A} (p.1); the ≤ signs and Ω of note
  C (Figure 1, p.4; the text layer has other glyphs).
- Source issues:
  - Source error: the description names the ’HC04 devices (Description
    note).
  - Unconfirmed: output clamp current condition "V~O~ < 0 or V~O~ = 0 to
    V~CC~" (absolute maximum ratings note 4).
  - Unconfirmed: I~CC~ condition "O = 0" printed without its symbol
    (electrical characteristics note 1).
  - Source error: the switching tables head the load capacitance column
    V~CC~; kept as printed (switching characteristics note 1).
  - Unconfirmed: "PRR ≤ MHz" printed with no number (Parameter
    measurement information note).
  - Source error: misprints kept as printed: "minimun" (twice), "HIgh"
    and "Characterists" in the noise characteristics (p.3), where the
    V~OL(V)~ row repeats its symbol in its name and the V~IH(D)~ and
    V~IL(D)~ rows both end with V~IH(D)~; "ODERABLE PART NUMBER" (p.1)
    and "PARAMETER MEASURMENT INFORMATION" (p.4) in headings that are
    not carried over.
  - Unconfirmed: the storage temperature range is printed with a
    minimum of –60°C (p.2); kept as printed.
