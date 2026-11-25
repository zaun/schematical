---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act86",
  "title": "CD54ACT86, CD74ACT86 quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/cd74act86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS322",
    "date": "2003-01",
    "pages": 17
  },
  "devices": ["gate:74_86"],
  "parts": ["CD54ACT86F3A", "CD74ACT86E", "CD74ACT86M", "CD74ACT86M96"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CD54ACT86, CD74ACT86 quadruple 2-input exclusive-OR gates

## Description

The 'ACT86 devices are quadruple 2-input exclusive-OR gates. These
devices perform the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in
positive logic. [[1::2]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the
output. If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[1::3]]

Inputs Are TTL-Voltage Compatible. Speed of Bipolar F, AS, and S, With
Significantly Reduced Power Consumption. Balanced Propagation Delays. ±24-mA Output
Drive Current. Fanout to 15 F Devices. SCR-Latchup-Resistant CMOS
Process and Circuit Design. Exceeds 2-kV ESD Protection Per
MIL-STD-883, Method 3015. [[1::1]]

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

### Exclusive-OR logic

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an CD74AC86 gate in positive
logic; negation may be shown at any two ports. Source error: this
sentence of the CD54ACT86, CD74ACT86 data sheet names the CD74AC86;
kept as printed. Conversion note: the five symbols are not drawn here. [[2::1]] [[2::2]]

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
| Inputs | < | Output Y |
| A | B | ^ |
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
| V~CC~ | Supply voltage range | | -0.5 | 6 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ (see Note 1) [2] | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ (see Note 1) [2] | | ±50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance (see Note 2) [3] | E package | | 80 | °C/W |
| ^ | ^ | M package | | 86 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions (see Note 3) [1]"
| Symbol | Parameter | T~A~ = 25°C | < | –55°C to 125°C | < | –40°C to 85°C | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 10 | | 10 | | 10 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | T~A~ = 25°C | < | –55°C to 125°C | < | –40°C to 85°C | < | Unit |
| ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | V~I~ = V~IH~ or V~IL~ | I~OH~ = -50 µA | 4.5 V | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | 3.7 | | 3.8 | | V |
| ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | | 3.85 | | V |
| V~OL~ | V~I~ = V~IH~ or V~IL~ | I~OL~ = 50 µA | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | 1.65 | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, | I~O~ = 0 | 5.5 V | | 4 | | 80 | | 40 | µA |
| ΔI~CC~ [2] | V~I~ = V~CC~ – 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 3 | | 2.8 | mA |
| C~i~ | | < | | | 10 | | 10 | | 10 | pF |

[1] Test one output at a time, not exceeding 1-second duration. Measurement is made by forcing indicated current and measuring voltage to minimize power dissipation. Test verifies a minimum 50-Ω transmission-line drive capability at 85°C and 75-Ω transmission-line drive capability at 125°C.
[2] Additional quiescent supply current per input pin, TTL inputs high, 1 unit load
```
[[3::table #1]]

```table id=input-load caption="ACT input load table [1]" values=Value
| Input | Unit load | Unit |
|---|--:|---|
| All | 0.48 | |

[1] Unit Load is ΔI~CC~ limit specified in electrical characteristics table (e.g., 2.4 mA at 25°C).
```
[[3::table #2]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 57 | pF |
```
[[3::table #3]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | –55°C to 125°C | < | –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 3.7 | 14.6 | 3.8 | 13.3 | ns |
| t~PHL~ | A or B | Y | 3.7 | 14.6 | 3.8 | 13.3 | ns |
```
[[3::table #4]]

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. Phase relationships
between waveforms are arbitrary. The outputs are measured one at a time
with one input transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~. All parameters and waveforms are not applicable to all
devices. [[4::1]]

Conversion note: the load circuit of Figure 1 also has R1 = 500 Ω from
the output to switch S1 (2 × V~CC~, Open or GND), and S1 is Open for
t~PLH~/t~PHL~, which leaves R2 and C~L~ below. Conversion note: in the
voltage waveforms the input swings 0 V to 3 V and is measured at 1.5 V,
and the outputs are measured at 50% V~CC~ (transition times at 10% and
90%).

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```
[[4::Figure 1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output transition times"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```
[[4::Figure 1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
CD54ACT86F3A | Tube of 25 | -55 to 125 | CD54ACT86F3A
CD74ACT86E | Tube of 25 | -55 to 125 | CD74ACT86E
CD74ACT86M | | -55 to 125 | ACT86M | Obsolete
CD74ACT86M96 | Reel of 2500 | -55 to 125 | ACT86M | NRND
```
[[1::table #3]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum, tape and
  reel, tube and mechanical pages (5–16) and the notice page (17)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: the page-1 ordering table and the vendor's pinout drawing;
  the five exclusive-OR symbols and the three element symbols (p.2)
  are described in text.
- Figure 1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this gate; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, their
  S1 settings and their notes (B, D, G, H) are dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- Ordering vendor columns are from the package option addendum (p.5).
- From images: overbars of Y = !{A}B + A!{B} (p.1).
- From images: ΔI~CC~ (electrical characteristics and input load table, p.3; the text
  layer prints DICC and ICC).
- Source issues:
  - Source error: "valid for an CD74AC86 gate" in the exclusive-OR logic text of this CD54ACT86, CD74ACT86 data sheet (p.2); kept as printed.
