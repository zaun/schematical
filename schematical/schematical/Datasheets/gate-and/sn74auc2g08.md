---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc2g08",
  "title": "SN74AUC2G08 dual 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74auc2g08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES477C",
    "date": "2007-01",
    "pages": 19
  },
  "devices": ["gate:2G08"],
  "parts": ["SN74AUC2G08DCTR", "SN74AUC2G08DCUR", "SN74AUC2G08DCURG4", "SN74AUC2G08YZPR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-08"
}
---

# SN74AUC2G08 dual 2-input positive-AND gate

## Description

This dual 2-input positive-AND gate is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation. The SN74AUC2G08 performs the Boolean function A • B or
Y = !{!{A} + !{B}} in positive logic. It is 3.6-V I/O tolerant to support
mixed-mode signal operation.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

For more information about AUC Little Logic devices, refer to the TI
application report, Applications of Texas Instruments AUC Sub-1-V Little
Logic Devices, literature number SCEA027.

## Pinout

DCT and DCU share one pin order. The source draws the YZP package in
bottom view; the drawing here is generated from the catalog.

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A, 2A: Gate input A
1B, 2B: Gate input B
1Y, 2Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table (each gate) [1]"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] Source error: the source heads the input columns CLK and D; the logic diagram and pinout name the inputs 1A, 1B, 2A and 2B.
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
| θ~JA~ | Package thermal impedance [3] | DCT package | | 220 | °C/W |
| ^ | ^ | DCU package | | 227 | °C/W |
| ^ | ^ | YZP package | | 102 | °C/W |
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
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | < | 0.8 V | | 0.55 | | V |
| ^ | ^ | I~OH~ = -3 mA | < | 1.1 V | 0.8 | | | V |
| ^ | ^ | I~OH~ = -5 mA | < | 1.4 V | 1 | | | V |
| ^ | ^ | I~OH~ = -8 mA | < | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -9 mA | < | 2.3 V | 1.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A or B inputs | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 2 | | pF |

[1] All typical values are at T~A~ = 25°C.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted)"
| Symbol | From | To | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 9.2 | 0.7 | 3 | 0.8 | 1.9 | 0.6 | 1 | 1.5 | 0.5 | 1 | ns |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted)"
| Symbol | From | To | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.2 | 1.5 | 2.1 | 1 | 1.6 | ns |
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 12 | 13 | 13 | 13 | 15 | pF |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. For t~PLH~/t~PHL~, switch S1 is open, so the output sees C~L~ and
R~L~ to GND. Inputs switch between 0 V and V~CC~; timing is measured at
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

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

```package footprint=DSBGA-8-YZP
```

## Ordering information

```ordering
SN74AUC2G08DCTR | Reel of 3000 | -40 to 85 | U08 (R, Z)
SN74AUC2G08DCUR | Reel of 3000 | -40 to 85 | U08Q, U08R
SN74AUC2G08DCURG4 | Reel of 3000 | -40 to 85 | U08R
SN74AUC2G08YZPR | Reel of 3000 | -40 to 85 | UEN
```

Markings are from the package option addendum. The source's ordering
table gives the top-side markings U08_ _ _ (DCT), U08_ (DCU) and
_ _ _UE_ (YZP), where the underscores stand for date, sequence and
assembly/test-site characters.

## Conversion notes

- Source pages 1–5 converted; the package option addendum, tape and
  reel and mechanical pages (6–19) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: the NanoFree package text, the ordering-table package
  descriptions, and the enable/disable-time, pulse-duration and
  setup/hold parts of Figure 1 (the source notes that not all
  parameters and waveforms apply to all devices; this gate has no
  output enable), and the S1 positions for t~PLZ~/t~PZL~ (2 × V~CC~)
  and t~PHZ~/t~PZH~ (GND).
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- From images: Y = !{!{A} + !{B}} overbars (p.1; the text layer reads
  "Y = A + B").
- Source issues:
  - Source error: the function table heads the inputs CLK and D (p.2);
    the logic diagram and pinout on p.1–2 name them A and B. Corrected.
