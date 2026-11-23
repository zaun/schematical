---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1t04",
  "title": "SN74AUP1T04 low power, 1.8/2.5/3.3-V input, 3.3-V CMOS output, single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup1t04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES800",
    "date": "2010-04",
    "pages": 15
  },
  "devices": ["gate:1T04"],
  "parts": ["SN74AUP1T04DCKR"],
  "packages": {"p5": "DCK"},
  "converted": "2026-11-21"
}
---

# SN74AUP1T04 low power, 1.8/2.5/3.3-V input, 3.3-V CMOS output, single inverter gate

## Description

The SN74AUP1T04 performs the Boolean function Y = !{A} with designation
for logic-level translation applications with output referenced to
supply V~CC~. [[1::3]]

AUP technology is the industry's lowest-power logic technology designed
for use in extending battery-life in operating. All input levels that
accept 1.8-V LVCMOS signals, while operating from either a single 3.3-V
or 2.5-V V~CC~ supply. This product also maintains excellent signal
integrity. [[1::4]]

## Pinout

```pinout pinout=p5
```

## Pin functions

Conversion note: the source has no pin description table, and the descriptions below are the converter's, from the DCK package drawing and the function table.

```pins
NC: No connection
A: Input (lower level input)
GND: Ground
Y: Output (V~CC~ CMOS)
VCC: Supply
```
[[1::table #1]]

## Functional description

The wide V~CC~ range of 2.3 V to 3.6 V allows the possibility of
switching output level to connect to external controllers or processors. [[1::5]]

Schmitt-trigger inputs (ΔV~T~ = 210 mV between positive and negative
input transitions) offer improved noise immunity during switching
transitions, which is especially useful on analog mixed-mode designs.
Schmitt-trigger inputs reject input noise, ensure integrity of output
signals, and allow for slow input signal transition. [[1::6]]

I~off~ is a feature that allows for powered-down conditions
(V~CC~ = 0 V) and is important in portable and mobile applications. When
V~CC~ = 0 V, signals in the range from 0 V to 3.6 V can be applied to
the inputs and outputs of the device. No damage occurs to the device
under these conditions. [[1::7]]

The SN74AUP1T04 is designed with optimized current-drive capability of
4 mA to reduce line reflections, overshoot, and undershoot caused by
high-drive outputs. [[1::8]]

Conversion note: the logic diagram (inverter gate) shows one inverter with input A on pin 2 and output Y on pin 4.

Conversion note: Figure 3 (typical design examples, p.3) shows four translations through the device: a 1.8-V system and a 2.5-V system each to a 3.3-V system with the device on 3.3 V (V~IH~ = 1.19 V, V~IL~ = 0.5 V), and a 1.8-V system and a 3.3-V system each to a 2.5-V system with the device on 2.5 V (V~IH~ = 1.10 V, V~IL~ = 0.35 V).

Conversion note: Figure 4 (switching thresholds for 1.8-V to 3.3-V translation, p.3) shows the input switching waveform of a 1.8-V system crossing V~T+~ max = V~IH~ min = 1.19 V and V~T−~ min = V~IL~ max = 0.5 V, and the output switching waveform into a 3.3-V system with V~OH~ min and V~OL~ max, the device on 3.3 V.

## Function tables

```table id=function caption="Function table"
| Input (Lower Level Input) | Output (V~CC~ CMOS) |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[2::table #1]]

```table id=levels-2v5 caption="Supply V~CC~ = 2.3 V to 2.7 V (2.5 V)"
| Inputs: V~T+~ max = V~IH~ min, V~T-~ min = V~IL~ max | < | Output CMOS |
| A | B [1] | Y |
|:-:|:-:|:-:|
| V~IH~ = 1.1 V | < | V~OH~ = 1.85 V |
| V~IL~ = 0.35 V | < | V~OL~ = 0.45 V |

[1] Unconfirmed: the source prints a column B; the device has one input, A (DCK package drawing, p.1, and function table, p.2).
```
[[2::table #2]]

```table id=levels-3v3 caption="Supply V~CC~ = 3 V to 3.6 V (3.3 V)"
| Inputs: V~T+~ max = V~IH~ min, V~T-~ min = V~IL~ max | < | Output CMOS |
| A | B [1] | Y |
|:-:|:-:|:-:|
| V~IH~ = 1.19 V | < | V~OH~ = 2.55 V |
| V~IL~ = 0.5 V | < | V~OL~ = 0.45 V |

[1] Unconfirmed: the source prints a column B; the device has one input, A (DCK package drawing, p.1, and function table, p.2).
```
[[2::table #3]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [3] | DCK package | | 259 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[4::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2.3 | 3.6 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.3 V | | -3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | -4 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2.3 V | | 3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | 4 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 2.3 V to 2.7 V | 0.6 | | 1.1 | 0.6 | 1.1 | V |
| ^ | ^ | | 3 V to 3.6 V | 0.75 | | 1.16 | 0.75 | 1.19 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 2.3 V to 2.7 V | 0.35 | | 0.6 | 0.35 | 0.6 | V |
| ^ | ^ | | 3 V to 3.6 V | 0.5 | | 0.85 | 0.5 | 0.85 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 2.3 V to 2.7 V | 0.23 | | 0.6 | 0.1 | 0.6 | V |
| ^ | ^ | | 3 V to 3.6 V | 0.25 | | 0.56 | 0.15 | 0.56 | ^ |
| V~OH~ | | I~OH~ = –20 µA | 2.3 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = –2.3 mA | 2.3 V | 2.05 | | | 1.97 | | ^ |
| ^ | ^ | I~OH~ = –3.1 mA | ^ | 1.9 | | | 1.85 | | ^ |
| ^ | ^ | I~OH~ = –2.7 mA | 3 V | 2.72 | | | 2.67 | | ^ |
| ^ | ^ | I~OH~ = –4 mA | ^ | 2.6 | | | 2.55 | | ^ |
| V~OL~ | | I~OL~ = 20 µA | 2.3 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1 mA | ^ | | | 0.44 | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4 mA | ^ | | | 0.44 | | 0.45 | ^ |
| I~I~ | All inputs | V~I~ = 3.6 V or GND | 0 V to 3.6 V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.1 | | 0.5 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.5 | µA |
| I~CC~ | | V~I~ = 3.6 V or GND, I~O~ = 0 | 2.3 V to 3.6 V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | One input at 0.3 V or 1.1 V, Other inputs at 0 or V~CC~, I~O~ = 0 | 2.3 V to 2.7 V | | | | | 4 | µA |
| ^ | ^ | One input at 0.45 V or 1.2 V, Other inputs at 0 or V~CC~, I~O~ = 0 | 3 V to 3.6 V | | | | | 12 | ^ |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 1.5 | | | | pF |
| C~o~ | | V~O~ = V~CC~ or GND | 3.3 V | | 3 | | | | pF |
```
[[5::table #1]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 4 | 5 | pF |
```
[[7::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-1 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V, V~I~ = 1.8 V ± 0.15 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 2.5 V ± 0.2 V, V~I~ = 1.8 V ± 0.15 V (unless otherwise noted) (see Figure 5)"
| Parameter | From (input) | To (output) | C~L~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 pF | 1.8 | 2.3 | 2.9 | 0.5 | 6.8 | ns |
| ^ | ^ | ^ | 10 pF | 2.3 | 2.8 | 3.4 | 1 | 7.9 | ^ |
| ^ | ^ | ^ | 15 pF | 2.6 | 3.1 | 3.8 | 1 | 8.7 | ^ |
| ^ | ^ | ^ | 30 pF | 3.8 | 4.4 | 5.1 | 1.5 | 10.8 | ^ |
```
[[5::table #2]]

```table id=sw-2 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V, V~I~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 2.5 V ± 0.2 V, V~I~ = 2.5 V ± 0.2 V (unless otherwise noted) (see Figure 5)"
| Parameter | From (input) | To (output) | C~L~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 pF | 1.8 | 2.3 | 3.1 | 0.5 | 6 | ns |
| ^ | ^ | ^ | 10 pF | 2.2 | 2.8 | 3.5 | 1 | 7.1 | ^ |
| ^ | ^ | ^ | 15 pF | 2.6 | 3.2 | 5.2 [1] | 1 | 7.9 | ^ |
| ^ | ^ | ^ | 30 pF | 3.7 | 4.4 | 5.2 | 1.5 | 10 | ^ |

[1] Unconfirmed: the source prints a 15 pF maximum of 5.2 ns at T~A~ = 25°C, the same as the 30 pF maximum of this table.
```
[[6::table #1]]

```table id=sw-3 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V, V~I~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 2.5 V ± 0.2 V, V~I~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 5)"
| Parameter | From (input) | To (output) | C~L~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 pF | 2 | 2.7 | 3.5 | 0.5 | 5.5 | ns |
| ^ | ^ | ^ | 10 pF | 2.4 | 3.1 | 3.9 | 1 | 6.5 | ^ |
| ^ | ^ | ^ | 15 pF | 2.8 | 3.5 | 4.3 | 1 | 7.4 | ^ |
| ^ | ^ | ^ | 30 pF | 4 | 4.7 | 5.5 | 1.5 | 9.5 | ^ |
```
[[6::table #2]]

```table id=sw-4 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V, V~I~ = 1.8 V ± 0.15 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V, V~I~ = 1.8 V ± 0.15 V (unless otherwise noted) (see Figure 5)"
| Parameter | From (input) | To (output) | C~L~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 pF | 1.6 | 2 | 2.5 | 0.5 | 8 | ns |
| ^ | ^ | ^ | 10 pF | 2 | 2.4 | 2.9 | 1 | 8.5 | ^ |
| ^ | ^ | ^ | 15 pF | 2.3 | 2.8 | 3.3 | 1 | 9.1 | ^ |
| ^ | ^ | ^ | 30 pF | 3.4 | 3.9 | 4.4 | 1.5 | 9.8 | ^ |
```
[[6::table #3]]

```table id=sw-5 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V, V~I~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V, V~I~ = 2.5 V ± 0.2 V (unless otherwise noted) (see Figure 5)"
| Parameter | From (input) | To (output) | C~L~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 pF | 1.6 | 1.9 | 2.4 | 0.5 | 5.3 | ns |
| ^ | ^ | ^ | 10 pF | 2 | 2.3 | 2.7 | 1 | 6.1 | ^ |
| ^ | ^ | ^ | 15 pF | 2.3 | 2.7 | 3.1 | 1 | 6.8 | ^ |
| ^ | ^ | ^ | 30 pF | 3.4 | 3.8 | 4.2 | 1.5 | 8.5 | ^ |
```
[[6::table #4]]

```table id=sw-6 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V, V~I~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V, V~I~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 5)"
| Parameter | From (input) | To (output) | C~L~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 pF | 1.6 | 2.1 | 2.7 | 0.5 | 4.7 | ns |
| ^ | ^ | ^ | 10 pF | 2 | 2.4 | 3 | 1 | 5.7 | ^ |
| ^ | ^ | ^ | 15 pF | 2.3 | 2.7 | 3.3 | 1 | 6.2 | ^ |
| ^ | ^ | ^ | 30 pF | 3.4 | 3.8 | 4.4 | 1.5 | 7.8 | ^ |
```
[[7::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a
time, with one transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~. [[8::1]]

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~MI~ | V~I~/2 | V~I~/2 |
| V~MO~ | V~CC~/2 | V~CC~/2 |
```
[[8::table #1]]

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```
[[8::Figure 5]]

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: VMI on the input, VMO on the outputs
```
[[8::figure #1]]

## Package

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74AUP1T04DCKR | Reel of 3000 | -40 to 85 | 6CF
```
[[2::table #4]]

Conversion note: the marking above is from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information [1]"
| T~A~ | Package [2] | < | Orderable part number | Top-side marking [3] |
|---|---|---|---|---|
| –40°C to 85°C | SOT (SC-70) – DCK | Reel of 3000 | SN74AUP1T04DCKR | 6C_ |
| ^ | ^ | Reel of 250 | SN74AUP1T04DCKT | ^ |

[1] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI web site at www.ti.com.
[2] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
[3] The actual top-side marking has one additional character that designates the wafer fab/assembly site.
```
[[2::table #5]]

## Conversion notes

- Source pages 1–8 converted; the tape and reel and mechanical pages
  (10–15) dropped, replaced by the generated Package and Ordering
  sections. The ordering line is from the package option addendum
  (p.9).
- The source is in TI's older layout, with no Detailed Description or
  Application chapter; its description (p.1) is split between
  Description and Functional description, in the source's order.
- The ordering information table of p.2 is kept as a table; it also
  lists SN74AUP1T04DCKT, which the catalog doesn't carry.
- Dropped: the features list, Figure 1 (family
  power comparison) and Figure 2 (25 MHz switching waveform of the
  AUP1G08). The logic diagram, Figure 3 and Figure 4 are described in
  text.
- Figure 5 (p.8): the load circuit and the propagation delay waveforms
  were compared with the page image; the input waveform is measured at
  V~MI~ and the output waveforms at V~MO~.
- From images: the overbar of Y = !{A} (p.1); θ~JA~ (absolute maximum
  ratings, p.4; the text layer reads qJA); µA in I~OH~ = –20 µA,
  I~OL~ = 20 µA and the unit of I~I~, I~off~, ΔI~off~, I~CC~ and
  ΔI~CC~ (electrical characteristics, p.5; the text layer reads mA).
- Source issues:
  - Unconfirmed: the two supply-level tables (p.2) print input columns
    A and B; the device has one input, A (function tables note 1).
  - Unconfirmed: t~pd~ maximum at 15 pF, T~A~ = 25°C in the
    V~CC~ = 2.5 V ± 0.2 V, V~I~ = 2.5 V ± 0.2 V table (p.6) is printed
    as 5.2 ns, the same as at 30 pF (switching characteristics note 1).
  - Unconfirmed: the second paragraph of the description (p.1) reads
    "in extending battery-life in operating. All input levels that
    accept 1.8-V LVCMOS signals, while operating from"; kept as
    printed.
  - Unconfirmed: the description (p.1) gives ΔV~T~ = 210 mV, and the
    electrical characteristics (p.5) give ΔV~T~ minimums of 0.23 V and
    0.25 V at 25°C.
  - Unconfirmed: Figure 5 (p.8) labels the output measurement level
    both V~MO~ and V~Mo~.
