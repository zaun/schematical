---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc86a-ep",
  "title": "SN74LVC86A-EP quadruple 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lvc86a-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS752B",
    "date": "2007-07",
    "pages": 15
  },
  "devices": ["gate:74_86"],
  "parts": ["SN74LVC86AMDREP", "SN74LVC86AQDREP", "SN74LVC86AQPWREP", "V62/04670-01XE", "V62/04670-01YE", "V62/04670-02XE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74LVC86A-EP quadruple 2-input exclusive-OR gate

## Description

Controlled Baseline – One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of –40°C to 125°C and –55°C to 125°C.
Inputs Accept Voltages to 5.5 V. [[1::1]]

The SN74LVC86A quadruple 2-input exclusive-OR gate is designed for 2.7-V
to 3.6-V V~CC~ operation. The device performs the Boolean function
Y = A ⊕ B or Y = !{A}B + A!{B} in positive logic. [[1::3]] [[1::4]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the output.
If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[1::5]]

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of this device as a translator in a mixed 3.3-V/5-V
system environment. [[1::6]]

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

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These five equivalent
exclusive-OR symbols are valid for an SN74LVC86A gate in positive logic;
negation may be shown at any two ports. Conversion note: the five
symbols are not drawn here. [[2::1]] [[2::2]]

The output is active (low) if all inputs stand at the same logic level
(i.e., A = B). Conversion note: this is the logic-identity element
(symbol =). [[2::3]]

The output is active (low) if an even number of inputs (i.e., 0 or 2)
are active. Conversion note: this is the even-parity element (symbol
2k). [[2::4]]

The output is active (high) if an odd number of inputs (i.e., only 1 of
the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1). [[2::5]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[1::table #2]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 V | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 V | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
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
| Δt/Δv | Input transition rise or fall rate | | | 9 | ns/V |
| T~A~ | Operating free-air temperature | Q suffix | -40 | 125 | °C |
| ^ | ^ | M suffix | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -12 mA | 2.7 V | 2.2 | | | V |
| ^ | ^ | 3 V | 2.4 | | | V |
| ^ | I~OH~ = -24 mA | 3 V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100 µA | 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[3::table #2]]

```table id=operating caption="Operating characteristics, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 7.5 | 8.5 | pF |
```
[[3::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics (see Figure 1)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A | Y | | 5.6 | 1 | 4.6 | ns |
```
[[3::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[4::2]]

Conversion note: for t~PLH~/t~PHL~ the switch S1 of the load circuit
(Figure 1) is open, which leaves R~L~ and C~L~ from the output to ground.

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```
[[4::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: 500 Ω
CL: 50 pF
```
[[4::Figure 1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V
```
[[4::Figure 1]]

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LVC86AMDREP | Reel of 2500 | -55 to 125 | LVC86AM
SN74LVC86AQDREP | Reel of 2500 | -40 to 125 | LVC86AE
SN74LVC86AQPWREP | Reel of 2000 | -40 to 125 | LVC86AE
V62/04670-01XE | Reel of 2500 | -40 to 125 | LVC86AE
V62/04670-01YE | Reel of 2000 | -40 to 125 | LVC86AE
V62/04670-02XE | Reel of 2500 | -55 to 125 | LVC86AM
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel and mechanical pages (5–15) dropped, replaced by the generated
  Package and Ordering sections. The ordering block follows the package
  option addendum (p.5).
- Dropped: features list (except the controlled baseline, the
  temperature ranges and the input voltage), the ordering information
  table of p.1, the exclusive-OR symbol drawings (p.2), the
  pulse-duration, setup/hold and enable/disable waveforms of the generic
  measurement figure and their notes B, E and F (they do not apply to
  this part).
- From images: overbars of Y = !{A}B + A!{B} (p.1; the text layer has
  "Y = AB + AB").
- The source has no pin functions table; the pin descriptions are
  written from the pin names and the function table.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column.
