---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc08",
  "title": "SN74AUC08 quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74auc08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES512A",
    "date": "2005-03",
    "pages": 9
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74AUC08RGYR", "SN74AUC08RGYRG4"],
  "packages": {"p14": "RGY"},
  "converted": "2026-11-08"
}
---

# SN74AUC08 quadruple 2-input positive-AND gate

## Description

Optimized for 1.8-V operation
and is 3.6-V I/O tolerant to support mixed-mode signal operation. [[1::1]]

This quadruple 2-input positive-AND gate is operational at 0.8-V to
2.7-V V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation. The SN74AUC08 device performs the Boolean function Y = A • B
or Y = !{!{A} + !{B}} in positive logic. [[1::2]] [[1::3]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::4]]

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
[[1::table #1]]

## Function tables

Conversion note: the source's logic diagram (each gate, positive logic) draws one 2-input AND gate with inputs A and B and output Y.

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```
[[1::table #2]]

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
| θ~JA~ | Package thermal impedance [3] | | | 47 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-5.
```
[[2::table #1]]

ESD protection exceeds JESD 22: 2000-V human-body model (A114-A),
200-V machine model (A115-A), 1000-V charged-device model (C101).
Latch-up performance exceeds 100 mA per JESD 78, Class II. [[1::1]]

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
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 1.65 V [2] | | 20 | ns/V |
| ^ | ^ | V~CC~ = 1.65 V to 1.95 V [3] | | 15 | ns/V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V [3] | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
[2] The data was taken at C~L~ = 15 pF, R~L~ = 2 kΩ (see Figure 1).
[3] The data was taken at C~L~ = 30 pF, R~L~ = 500 Ω (see Figure 1).
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = –100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | I~OH~ = –0.7 mA | < | 0.8 V | | 0.55 | | V |
| ^ | I~OH~ = –3 mA | < | 1.1 V | 0.8 | | | V |
| ^ | I~OH~ = –5 mA | < | 1.4 V | 1 | | | V |
| ^ | I~OH~ = –8 mA | < | 1.65 V | 1.2 | | | V |
| ^ | I~OH~ = –9 mA | < | 2.3 V | 1.8 | | | V |
| V~OL~ | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A or B inputs | V~I~ = V~CC~ or GND | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | V~I~ = V~CC~ or GND | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | V~I~ = V~CC~ or GND | < | 2.5 V | | 2 | | pF |

[1] All typical values are at T~A~ = 25°C.
```
[[3::table #1]]

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 14 | 14 | 14 | 14 | 17 | pF |
```
[[3::table #2]]

## Switching characteristics

```table id=sw-15 caption="Switching characteristics, C~L~ = 15 pF (see Figure 1)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 5.4 | 0.9 | 3.4 | 0.6 | 2.3 | 0.4 | 1 | 1.9 | 0.3 | 1.3 | ns |
```
[[3::table #3]]

```table id=sw-30 caption="Switching characteristics, C~L~ = 30 pF (see Figure 1)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.7 | 1.5 | 2.3 | 0.5 | 1.8 | ns |
```
[[3::table #4]]

## Parameter measurement information

C~L~ includes probe and jig
capacitance. All input pulses are supplied by generators having the
following characteristics: PRR ≤ 10 MHz, Z~O~ = 50 Ω, slew rate ≥
1 V/ns. The outputs are measured one at a time, with one transition per
measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters and
waveforms are not applicable to all devices. Figure 1, load circuit and
voltage waveforms. Conversion note: the
source's test table sets S1 open for t~PLH~/t~PHL~, which leaves R~L~
and C~L~ from the output to GND, and its waveforms draw the input from
0 V to V~CC~ with the delays measured at V~CC~/2. [[4::1]] [[4::2]]

```table id=pmi caption="Load circuit values"
| V~CC~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|
| 0.8 V | 15 pF | 2 kΩ | 0.1 V |
| 1.2 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.5 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.8 V ± 0.15 V | 15 pF | 2 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 15 pF | 2 kΩ | 0.15 V |
| 1.8 V ± 0.15 V | 30 pF | 1 kΩ [1] | 0.15 V |
| 2.5 V ± 0.2 V | 30 pF | 500 Ω | 0.15 V |

[1] Source conflict: Figure 1 (p.4) gives R~L~ = 1 kΩ for V~CC~ = 1.8 V ± 0.15 V at C~L~ = 30 pF; note 3 of the recommended operating conditions (p.2) says the V~CC~ = 1.65 V to 1.95 V data was taken at C~L~ = 30 pF, R~L~ = 500 Ω.
```
[[4::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: see table
CL: see table
```
[[4::Figure 1]]

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: V~CC~/2
```
[[4::figure #1]]

## Package

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74AUC08RGYR | Reel of 3000 | -40 to 85 | MS08
SN74AUC08RGYRG4 | Reel of 3000 | -40 to 85 | MS08
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel and mechanical pages dropped, replaced by the generated Package
  and Ordering sections. Packing of SN74AUC08RGYRG4 comes from the
  package option addendum.
- Dropped: features list (ESD and latch-up kept as a sentence), the
  pulse-duration, setup/hold and enable/disable waveforms of the generic
  measurement figure (they do not apply to this part).
- From images: Y = A • B or Y = !{!{A} + !{B}}, the equals signs and
  overbars (p.1; the text layer reads "Y  A • B or Y  A  B").
- Source issues: Source conflict: R~L~ at C~L~ = 30 pF and V~CC~ = 1.8 V ± 0.15 V is 1 kΩ in Figure 1 (p.4) and 500 Ω in note 3 of the recommended operating conditions (p.2); both kept.
