---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g02-ep",
  "title": "SN74LVC1G02-EP single 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74lvc1g02-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SGLS370",
    "date": "2006-08",
    "pages": 15
  },
  "devices": ["gate:1G02"],
  "parts": ["SN74LVC1G02MDCKREP", "V62/06631-01XE"],
  "packages": {"p5": "DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G02-EP single 2-input positive-NOR gate

## Description

This single 2-input positive-NOR gate is designed for 1.65-V to 5.5-V
V~CC~ operation.

The SN74LVC1G02-EP performs the Boolean function Y = !{A + B} or
Y = !{A} × !{B} in positive logic.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

Extended Temperature Performance of –55°C to 125°C. Supports 5-V V~CC~
Operation. Inputs Accept Voltages to 5.5 V. Latch-Up Performance Exceeds
100 mA Per JESD 78, Class II.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input
B: Input
GND: Ground
Y: Output
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
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
| θ~JA~ | Package thermal impedance [4] | DBV package | | 206 | °C/W |
| ^ | ^ | DCK package | | 252 | ^ |
| ^ | ^ | DRL package | | 142 | ^ |
| ^ | ^ | YEA/YZA package | | 154 | ^ |
| ^ | ^ | YEP/YZP package | | 132 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

ESD Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101).

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
| ∆t/∆v | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –4 mA | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = –8 mA | 2.3 V | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = –16 mA | 3 V | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = –24 mA | ^ | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = –32 mA | 4.5 V | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.6 | ^ |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ∆I~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4 | | pF |

[1] All typical values are at V~CC~ 3.3 V, T~A~ = 25°C.
```

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 23 | 23 | 23 | 25 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF or 50 pF (unless otherwise noted) (see Figure 2)"
| Symbol | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2.8 | 10.7 | 1.2 | 7.3 | 1 | 6 | 1 | 5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators have the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

All parameters and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figures also show pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```

```table id=test-conditions-1 caption="Test conditions (Figure 1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~∆~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 15 pF | 1 MΩ | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 15 pF | 1 MΩ | 0.3 V |
```

```table id=test-conditions-2 caption="Test conditions (Figure 2)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~∆~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Package

```package footprint=SC70-5
```

## Ordering information

```ordering
SN74LVC1G02MDCKREP | Reel of 3000 | -55 to 125 | BUF
V62/06631-01XE | Reel of 3000 | -55 to 125 | BUF
```

Conversion note: the lines above are from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package [1] | < | Orderable part number | Top-side marking [2] |
|---|---|---|---|---|
| –55°C to 125°C | NanoStar™ – WCSP (DSBGA) 0,17-mm Small Bump – YEA | Reel of 3000 | SN74LVC1G02NYEAREP [3] | _ _ _CB_ |
| ^ | NanoFree™ – WCSP (DSBGA) 0,17-mm Small Bump – YZA (Pb-free) | ^ | SN74LVC1G02NYZAREP [3] | ^ |
| ^ | NanoStar™ – WCSP (DSBGA) 0,23-mm Large Bump – YEP | ^ | SN74LVC1G02MYEPREP [3] | ^ |
| ^ | NanoFree™ – WCSP (DSBGA) 0,23-mm Large Bump – YZP (Pb-free) | ^ | SN74LVC1G02MYZPREP [3] | ^ |
| ^ | SOT (SOT-23) – DBV | Reel of 3000 | SN74LVC1G02MDBVREP [3] | C02_ |
| ^ | SOT (SC-70) – DCK | Reel of 3000 | SN74LVC1G02MDCKREP | BUF |
| ^ | SOT (SOT-553) – DRL | Reel of 4000 | SN74LVC1G02MDRLREP [3] | CB_ |

[1] Package drawings, standard packing quantities, thermal data, symbolization, and PCB design guidelines are available at www.ti.com/sc/package.
[2] DBV/DCK/DRL: The actual top-side marking has one additional character that designates the assembly/test site. YEA/YZA,YEP/YZP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the assembly/test site. Pin 1 identifier indicates solder-bump composition(1 = SnPb, • = Pb-free).
[3] Product Preview
```

## Conversion notes

- Source pages 1–7 converted. The ordering lines are from the package
  option addendum (pp.8–9). The tape and reel and mechanical pages
  (10–15) are left out; the generated Package section replaces them.
- Dropped: features list (the temperature, 5-V, input-voltage, latch-up
  and ESD lines are kept in text), the qualification-pedigree footnote,
  the NanoStar and NanoFree sentence of the description, the vendor
  pinout, the logic diagram (p.2, one NOR gate, A pin 1, B pin 2, Y
  pin 4).
- The catalog carries only the DCK parts; the DBV, DRL, YEA, YZA, YEP
  and YZP parts of the source's ordering information table are marked
  Product Preview there and are not in the package option addendum.
- The pin descriptions are added; the source has no pin function table.
  The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The source's description (p.1) names the device "SN74LVC1G02"; the
  data sheet's title is SN74LVC1G02-EP, and the name is written with the
  suffix here.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Figures 1 and 2 (pp.6–7)
  are the same circuit and waveforms with different load values, each
  with its own table.
- The switching characteristics conditions (p.5) refer only to
  Figure 2; no table of the source refers to the 15 pF load of Figure 1.
- From images: overbars of Y = !{A + B} and Y = !{A} × !{B} (p.1).
- Source issues:
  - Source error: note C of Figures 1 and 2 (pp.6–7) prints "generators
    have the following characteristics" for "generators having"; kept as
    printed.
  - Source error: the note of the electrical characteristics table (p.5)
    prints "V~CC~ 3.3 V" without the equals sign; kept as printed.
