---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74alvc08",
  "title": "SN74ALVC08 quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74alvc08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES101I",
    "date": "2004-10",
    "pages": 19
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74ALVC08D", "SN74ALVC08DGVR", "SN74ALVC08DR", "SN74ALVC08DRE4", "SN74ALVC08DRG4", "SN74ALVC08NSR", "SN74ALVC08PWR", "SN74ALVC08PWRE4", "SN74ALVC08PWRG4", "SN74ALVC08RGYR"],
  "packages": {"p14": "D, DGV, NS, PW, RGY"},
  "converted": "2026-11-08"
}
---

# SN74ALVC08 quadruple 2-input positive-AND gate

## Description

This quadruple 2-input positive-AND gate is designed for 1.65-V to 3.6-V
V~CC~ operation. The device performs the Boolean function Y = A · B or
Y = !{!{A} + !{B}} in positive logic. Outputs drive ±24 mA at 3.3 V.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Positive supply
```

## Function tables

Logic diagram: four 2-input AND gates.

```table id=function caption="Function table (each gate)"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance | D package [4] | | 86 | °C/W |
| ^ | ^ | DGV package [4] | | 127 | °C/W |
| ^ | ^ | NS package [4] | | 76 | °C/W |
| ^ | ^ | PW package [4] | | 113 | °C/W |
| ^ | ^ | RGY package [5] | | 47 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 4.6 V maximum.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
[5] The package thermal impedance is calculated in accordance with JESD 51-5.
```

ESD protection exceeds JESD 22: 2000-V human-body model (A114-A),
200-V machine model (A115-A), 1000-V charged-device model (C101).
Latch-up performance exceeds 250 mA per JESD 17.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 1.65 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -12 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 12 | mA |
| ^ | ^ | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | < | 1.65 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -4 mA | < | 1.65 V | 1.2 | | | V |
| ^ | I~OH~ = -6 mA | < | 2.3 V | 2 | | | V |
| ^ | I~OH~ = -12 mA | < | 2.3 V | 1.7 | | | V |
| ^ | ^ | < | 2.7 V | 2.2 | | | V |
| ^ | ^ | < | 3 V | 2.4 | | | V |
| ^ | I~OH~ = -24 mA | < | 3 V | 2 | | | V |
| V~OL~ | I~OL~ = 100 µA | < | 1.65 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 4 mA | < | 1.65 V | | | 0.45 | V |
| ^ | I~OL~ = 6 mA | < | 2.3 V | | | 0.4 | V |
| ^ | I~OL~ = 12 mA | < | 2.3 V | | | 0.7 | V |
| ^ | ^ | < | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA | < | 3 V | | | 0.55 | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND | I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V | Other inputs at V~CC~ or GND | 3 V to 3.6 V | | | 750 | µA |
| C~i~ | V~I~ = V~CC~ or GND | < | 3.3 V | | 4.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | C~L~ = 0, f = 10 MHz | 24 | 25 | 26 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics (see Figure 1)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.2 | 5.3 | 1 | 3.2 | | 3 | 1.2 | 2.9 | ns |
```

## Parameter measurement information

Figure 1, load circuit and voltage waveforms. S1: open for t~pd~; R~L~
and C~L~ load the output to ground. C~L~
includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6V | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: see table
CL: see table
```

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: see table
```

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74ALVC08RGYR | Reel of 3000 | -40 to 85 | VA08
SN74ALVC08D | Tube of 50 | -40 to 85 | ALVC08
SN74ALVC08DR | Reel of 2500 | -40 to 85 | ALVC08
SN74ALVC08DRE4 | Reel of 2500 | -40 to 85 | ALVC08
SN74ALVC08DRG4 | Reel of 2500 | -40 to 85 | ALVC08
SN74ALVC08NSR | Reel of 2000 | -40 to 85 | ALVC08
SN74ALVC08PWR | Reel of 2000 | -40 to 85 | VA08
SN74ALVC08PWRE4 | Reel of 2000 | -40 to 85 | VA08
SN74ALVC08PWRG4 | Reel of 2000 | -40 to 85 | VA08
SN74ALVC08DGVR | Reel of 2000 | -40 to 85 | VA08
```

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel and mechanical pages dropped, replaced by the generated Package
  and Ordering sections. Packing quantities come from the package option
  addendum.
- Dropped: features list (ESD and latch-up kept as a sentence), the
  pulse-duration, setup/hold and enable/disable waveforms of the generic
  measurement figure (they do not apply to this part).
- The second form of the Boolean function loses its overbars in the PDF
  text layer ("Y = A + B"); written as Y = !{!{A} + !{B}} from the page
  image.
- Source issues: none found.
