---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g04-ep",
  "title": "SN74LVC2G04-EP dual inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2g04-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SGLS365",
    "date": "2006-08",
    "pages": 14
  },
  "devices": ["gate:2G04"],
  "parts": ["SN74LVC2G04MDCKREP"],
  "packages": {"p6": "DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G04-EP dual inverter gate

## Description

This dual inverter is designed for 1.65-V to 5.5-V V~CC~ operation. The
SN74LVC2G04 performs the Boolean function Y = !{A}. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::7]]

Inputs Accept Voltages to 5.5 V. [[1::1]]

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. This
includes, but is not limited to, Highly Accelerated Stress Test (HAST)
or biased 85/85, temperature cycle, autoclave or unbiased HAST,
electromigration, bond intermetallic life, and mold compound life. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits. [[1::3]]

## Pinout

```pinout pinout=p6
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A: Inverter input A
1Y, 2Y: Inverter output Y
GND: Ground
VCC: Supply voltage
```
[[1::table #1]]

## Functional description

Conversion note: the logic diagram (positive logic) shows two inverters, 1A (pin 1) to 1Y (pin 6) and 2A (pin 3) to 2Y (pin 4).

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[2::table #1]]

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
| θ~JA~ | Package thermal impedance [4] | DBV package | | 165 | °C/W |
| ^ | ^ | DCK package | | 259 | ^ |
| ^ | ^ | DRL package | | 142 | ^ |
| ^ | ^ | YEA/YZA package | | 143 | ^ |
| ^ | ^ | YEP/YZP package | | 123 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[3::table #1]]

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 3.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[5::table #1]]

Typical V~OLP~ (Output Ground Bounce) <0.8 V at V~CC~ = 3.3 V,
T~A~ = 25°C. Typical V~OHV~ (Output V~OH~ Undershoot) >2 V at
V~CC~ = 3.3 V, T~A~ = 25°C. [[1::1]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 14 | 14 | 14 | 16 | pF |
```
[[5::table #2]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.1 | 8 | 1.5 | 6.0 | 1.2 | 5.0 | 1 | 4.0 | ns |
```
[[5::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[6::2]]

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[6::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[6::Figure 1]]

```table id=test-conditions caption="Test conditions (Figure 1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[6::Figure 1]]

## Package

```package footprint=SC70-6
```

## Ordering information

```ordering
SN74LVC2G04MDCKREP | Reel of 3000 | -55 to 125 | BUG
```
[[2::table #2]]

Conversion note: the line above is from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –55°C to 125°C | NanoStar – WCSP (DSBGA) 0,17-mm Small Bump – YEA | Reel of 3000 | SN74LVC2G04MYEAREP [2] | |
| ^ | NanoFree – WCSP (DSBGA) 0,17-mm Small Bump – YZA (Pb-free) | ^ | SN74LVC2G04MYZAREP [2] | |
| ^ | NanoStar – WCSP (DSBGA) 0,23-mm Large Bump – YEP | ^ | SN74LVC2G04MYEPREP [2] | |
| ^ | NanoFree – WCSP (DSBGA) 0,23-mm Large Bump – YZP (Pb-free) | ^ | SN74LVC2G04MYZPREP [2] | |
| ^ | SOT (SOT-23) – DBV | Reel of 3000 | SN74LVC2G04MDBVREP [2] | |
| ^ | SOT (SC-70) – DCK | Reel of 3000 | SN74LVC2G04MDCKREP | BUG |
| ^ | SOT (SOT-563) – DRL | Reel of 4000 | SN74LVC2G04MDRLREP [2] | |

[1] DBV/DCK/DRL: The actual top-side marking has one additional character that designates the assembly/test site. YEA/YZA,YEP/YZP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the assembly/test site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
[2] Product Preview
```
[[2::table #3]]

## Conversion notes

- Source pages 1–6 converted. The ordering line is from the package
  option addendum (pp.7–8). The tape and reel and mechanical pages
  (9–14) are left out; the generated Package section replaces them.
- Dropped: the features list except the lines quoted (the ESD, latch-up,
  ground-bounce and undershoot lines and the qualification-pedigree
  footnote are kept in text), the NanoStar and NanoFree package text,
  the vendor pinout drawings, and the pulse-duration, setup-and-hold and
  enable/disable waveforms of Figure 1 with their notes B, E and F (this
  inverter has no output enable). The logic diagram is described in
  text.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- The source's ordering table (p.2) also lists the YEA, YZA, YEP, YZP,
  DBV and DRL packages as Product Preview; the package option addendum
  lists only the DCK part, and the catalog carries only that part. The
  addendum also lists V62/06632-01XE, which the catalog doesn't carry.
- From images: overbar of Y = !{A} (p.1).
- Source issues: none found.
