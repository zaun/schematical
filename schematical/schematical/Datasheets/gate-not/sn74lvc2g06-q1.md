---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g06-q1",
  "title": "SN74LVC2G06-Q1 dual inverter buffer/driver with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc2g06-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES617A",
    "date": "2008-04",
    "pages": 13
  },
  "devices": ["gate:2G06"],
  "parts": ["SN74LVC2G06QDCKRQ1"],
  "packages": {"p6": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G06-Q1 dual inverter buffer/driver with open-drain outputs

## Description

This dual inverter buffer/driver is designed for 1.65-V to 5.5-V V~CC~
operation.

The output of the SN74LVC2G06-Q1 device is open drain and can be connected
to other open-drain outputs to implement active-low wired-OR or
active-high wired-AND functions. The maximum sink current is 32 mA.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

Qualified for Automotive Applications. Supports 5-V V~CC~ Operation.
Max t~pd~ of 3.4 ns at 3.3 V. Low Power Consumption, 10-µA
Max I~CC~. ±24-mA Output Drive at 3.3 V. Typical V~OLP~ (Output Ground
Bounce) <0.8 V at V~CC~ = 3.3 V, T~A~ = 25°C. Typical V~OHV~ (Output
V~OH~ Undershoot) >2 V at V~CC~ = 3.3 V, T~A~ = 25°C. Inputs and
Open-Drain Outputs Accept Voltages up to 5.5 V. I~off~ Supports
Partial-Power-Down Mode Operation.

## Pinout

Conversion note: DBV and DCK share one pin order.

```pinout pinout=p6
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A: Inverter input A
1Y, 2Y: Open-drain inverter output Y
GND: Ground
VCC: Supply voltage
```

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

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | DBV package | | 165 | °C/W |
| ^ | ^ | DCK package | | 259 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
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
| V~O~ | Output voltage | | 0 | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OL~ | | I~OL~ = 100 µA | < | 1.65 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | < | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | < | 2.3 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | T~A~ = –40°C to 85°C | 3 V | | | 0.4 | ^ |
| ^ | ^ | ^ | T~A~ = 125°C | ^ | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 24 mA | T~A~ = –40°C to 85°C | 3 V | | | 0.55 | ^ |
| ^ | ^ | ^ | T~A~ = 125°C | ^ | | | 0.65 | ^ |
| ^ | ^ | I~OL~ = 32 mA | T~A~ = –40°C to 85°C | 4.5 V | | | 0.55 | ^ |
| ^ | ^ | ^ | T~A~ = 125°C | ^ | | | 0.65 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | < | 0 to 5.5 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, | I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, | Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 3.3 V | | 3.5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=cpd caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 2 | 2 | 3 | 4 | pF |
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 1.8 | 7.2 | 1 | 3.9 | 1 | 3.4 | 1 | 2.9 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. Since this device has open-drain outputs, t~PLZ~ and
t~PZL~ are the same as t~pd~. t~PZL~ is measured at V~M~. t~PLZ~ is
measured at V~OL~ + V~Δ~. All parameters and waveforms are not
applicable to all devices.

Conversion note: the load circuit (open drain) has C~L~ and one R~L~ from the output to GND and a second R~L~ from the output to switch S1, which goes to V~LOAD~, open or GND.
Conversion note: the source figure also shows pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PZL~ (see Notes E and F) | V~LOAD~ |
| t~PLZ~ (see Notes E and G) | V~LOAD~ |
| t~PHZ~/t~PZH~ | V~LOAD~ |
```

```table id=test-conditions caption="Test conditions (Figure 1)"
| V~CC~ | Input | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```

## Package

```package footprint=SC70-6
```

## Ordering information

```ordering
SN74LVC2G06QDCKRQ1 | Reel of 3000 | -40 to 125 | CTO
```

Conversion note: the marking above is from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information [1]"
| T~A~ | Package [2] | < | Orderable part number | Top-side marking [3] |
|---|---|---|---|---|
| –40°C to 125°C | SOT (SOT-23) – DBV | Tape and reel | SN74LVC2G06QDBVRQ1 | C06_ |
| ^ | SOT (SC-70) – DCK | Tape and reel | SN74LVC2G06QDCKRQ1 | CT_ |

[1] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI web site at www.ti.com.
[2] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
[3] DBV/DCK: The actual top-side marking has one additional character that designates the wafer fab/assembly site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```

## Conversion notes

- Source pages 1–5 converted. The ordering line is from the package
  option addendum (pp.6–7). The tape and reel (8–9), mechanical pages
  (10–12) and the notice page (13) are left out; the generated Package
  section replaces them.
- Dropped: the vendor pinout drawing; the logic diagram and
  the open-drain load circuit of Figure 1 are described in text. The
  features are sentences in Description and under Absolute maximum
  ratings (the source has no ESD ratings table).
- Note B of Figure 1 (p.5), on the enable and disable waveforms, is
  dropped with those waveforms.
- The source's electrical characteristics table gives symbols only, so
  its Parameter cells are empty except for I~I~.
- The ordering information table (p.1) also lists SN74LVC2G06QDBVRQ1,
  which the package option addendum and the catalog do not carry.
- From images: the arrangement of the logic diagram (p.2) and of the
  load circuit (p.5).
- Source issues: none found.
