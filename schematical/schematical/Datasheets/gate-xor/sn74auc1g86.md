---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc1g86",
  "title": "SN74AUC1G86 single 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74auc1g86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES389J",
    "date": "2007-11",
    "pages": 19
  },
  "devices": ["gate:1G86"],
  "parts": ["SN74AUC1G86DBVR", "SN74AUC1G86DCKR", "SN74AUC1G86DCKRG4", "SN74AUC1G86YZPR"],
  "packages": {"p5": "DBV, DCK", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74AUC1G86 single 2-input exclusive-OR gate

## Description

This single 2-input exclusive-OR gate is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation. Optimized for 1.8-V operation and is 3.6-V I/O tolerant to
support mixed-mode signal operation.

The SN74AUC1G86 performs the Boolean function Y = A ⊕ B or
Y = !{A}B + A!{B} in positive logic.

A common application is as a true/complement element. If the input is
low, the other input is reproduced in true form at the output. If the
input is high, the signal on the other input is reproduced inverted at
the output.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

## Pinout

Conversion note: DBV and DCK share one pin order. Conversion note: the
source draws the YZP package in bottom view, and the drawing here is
generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Gate input A
B: Gate input B
Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an SN74AUC1G86 gate in
positive logic; negation may be shown at any two ports. Conversion note:
the five symbols are not drawn here.

The output is active (low) if all inputs stand at the same logic level
(i.e., A = B). Conversion note: this is the logic-identity element
(symbol =).

The output is active (low) if an even number of inputs (i.e., 0 or 2)
are active. Conversion note: this is the even-parity element (symbol
2k).

The output is active (high) if an odd number of inputs (i.e., only 1 of
the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1).

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 3.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 3.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 3.6 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | DBV package | | 206 | °C/W |
| ^ | ^ | DCK package | | 252 | °C/W |
| ^ | ^ | YZP package | | 154 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

ESD protection exceeds JESD 22: 2000-V Human-Body Model (A114-A),
200-V Machine Model (A115-A), 1000-V Charged-Device Model (C101).
Latch-up performance exceeds 100 mA per JESD 78, Class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | -3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | -8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -9 | mA |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | < | 0.8 V | | 0.55 | | V |
| ^ | ^ | I~OH~ = -3 mA | < | 1.1 V | 0.8 | | | V |
| ^ | ^ | I~OH~ = -5 mA | < | 1.4 V | 1 | | | V |
| ^ | ^ | I~OH~ = -8 mA | < | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -9 mA | < | 2.3 V | 1.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A or B input | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 2.5 | | pF |

[1] All typical values are at T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 16 | 16 | 16.5 | 17 | 18.5 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5.5 | 0.8 | 3.8 | 0.5 | 2.6 | 0.4 | 1 | 1.7 | 0.3 | 1.3 | ns |
| ^ | B | ^ | 5 | 0.8 | 3.8 | 0.5 | 2.6 | 0.4 | 1 | 1.7 | 0.3 | 1.2 | ns |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 | 1.5 | 2.6 | 0.7 | 2 | ns |
| ^ | B | ^ | 0.8 | 1.5 | 2.6 | 0.7 | 2 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. Conversion note: the source's test table sets S1 open for
t~PLH~/t~PHL~, which leaves R~L~ and C~L~ from the output to GND, and its
waveforms draw the input from 0 V to V~CC~ with the delays measured at
V~CC~/2.

```table id=pmi-load caption="Load values"
| V~CC~ | C~L~ | R~L~ | V~Δ~ |
|---|--:|--:|--:|
| 0.8 V | 15 pF | 2 kΩ | 0.1 V |
| 1.2 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.5 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.8 V ± 0.15 V | 15 pF | 2 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 15 pF | 2 kΩ | 0.15 V |
| 1.8 V ± 0.15 V | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 30 pF | 500 Ω | 0.15 V |
```

```figure ref=load-rl-cl caption="Load circuit for t~PLH~/t~PHL~ (S1 open)"
RL: see table
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74AUC1G86DBVR | Reel of 3000 | -40 to 85 | U86R
SN74AUC1G86DCKR | Reel of 3000 | -40 to 85 | UHR
SN74AUC1G86DCKRG4 | Reel of 3000 | -40 to 85 | UHR
SN74AUC1G86YZPR | Reel of 3000 | -40 to 85 | UHN
```

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –40°C to 85°C | NanoFree WCSP (DSBGA) – YZP (Pb-free) | Reel of 3000 | SN74AUC1G86YZPR | _ _ _UH_ |
| ^ | SOT (SOT-23) – DBV | Reel of 3000 | SN74AUC1G86DBVR | U86_ |
| ^ | SOT (SC-70) – DCK | Reel of 3000 | SN74AUC1G86DCKR | UH_ |

[1] DBV/DCK: The actual top-side marking has one additional character that designates the assembly/test site. YZP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the assembly/test site.
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum, tape and
  reel and mechanical pages (6–19) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: the features list, the NanoFree package text, the five
  exclusive-OR symbol drawings (p.2), and the enable/disable-time,
  pulse-duration and setup/hold parts of Figure 1 (this gate has no
  output enable), with the S1 positions for t~PLZ~/t~PZL~ (2 × V~CC~)
  and t~PHZ~/t~PZH~ (GND).
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The source has no pin table; the pin descriptions are the
  converter's.
- The ordering block follows the package option addendum (p.6).
- From images: overbars of Y = !{A}B + A!{B} (p.1); V~Δ~ in the load
  table of Figure 1 (p.5; the text layer reads "VD").
