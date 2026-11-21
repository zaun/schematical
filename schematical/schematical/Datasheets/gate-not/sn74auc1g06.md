---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc1g06",
  "title": "SN74AUC1G06 single inverter buffer/driver with open-drain output",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74auc1g06.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES372M",
    "date": "2007-02",
    "pages": 19
  },
  "devices": ["gate:1G06"],
  "parts": ["SN74AUC1G06DBVR", "SN74AUC1G06DCKR", "SN74AUC1G06YZPR"],
  "packages": {"p5": "DBV, DCK", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74AUC1G06 single inverter buffer/driver with open-drain output

## Description

Optimized for 1.8-V operation and is 3.6-V I/O tolerant to support
mixed-mode signal operation.

This single inverter buffer/driver is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation.

The output of the SN74AUC1G06 is open drain and can be connected to
other open-drain outputs to implement active-low wired-OR or active-high
wired-AND functions.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

For more information about AUC Little Logic devices, please refer to the
TI application report, Applications of Texas Instruments AUC Sub-1-V
Little Logic Devices, literature number SCEA027.

## Pinout

Conversion note: DBV and DCK share one pin order. Conversion note:
the source draws the YZP package in bottom view, and the drawing here is
generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions of A, Y, GND and VCC are the converter's.

```pins
A: Inverter input
Y: Open-drain output
GND: Ground
VCC: Supply voltage
NC: No internal connection
DNU: Do not use
```

## Functional description

Conversion note: the logic diagram (positive logic) shows one inverter with an open-drain output, A (pin 2) to Y (pin 4).

## Function tables

```table id=function caption="Function table"
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
| V~CC~ | Supply voltage range | | -0.5 | 3.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 3.6 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | DBV package | | 206 | °C/W |
| ^ | ^ | DCK package | | 252 | ^ |
| ^ | ^ | YZP package | | 132 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

Latch-up performance exceeds 100 mA per JESD 78, Class II. ESD
protection exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | 3.6 | V |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 1.6 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 1.65 V | | 10 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | ^ |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | ^ |
| I~I~ | A input | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 3 | | pF |

[1] All typical values are at T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 2 | 2 | 2 | 2 | 7 | pF |
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5 | 0.3 | 3.1 | 0.2 | 2.4 | 0.2 | 1.4 | 2.5 | 0.2 | 1.7 | ns |
```

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.5 | 1.6 | 2.5 | 0.2 | 1.8 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~.

Conversion note: the source's test table sets S1 open for
t~PLH~/t~PHL~, which leaves R~L~ and C~L~ from the output to GND, and its
waveforms draw the input from 0 V to V~CC~ with the delays measured at
V~CC~/2.

Unconfirmed: the test table gives S1 open for t~PLH~/t~PHL~, which leaves the open-drain output of this device with no pull-up; the table is kept as printed.

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

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74AUC1G06DBVR | Reel of 3000 | -40 to 85 | U06R
SN74AUC1G06DCKR | Reel of 3000 | -40 to 85 | UTR
SN74AUC1G06YZPR | Reel of 3000 | -40 to 85 | UTN
```

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –40°C to 85°C | NanoFree – WCSP (DSBGA) 0.23-mm Large Bump – YZP (Pb-free) | Reel of 3000 | SN74AUC1G06YZPR | _ _ _UT_ |
| ^ | SOT (SOT-23) – DBV | Reel of 3000 | SN74AUC1G06DBVR | U06_ |
| ^ | SOT (SC-70) – DCK | Reel of 3000 | SN74AUC1G06DCKR | UT_ |

[1] DBV/DCK: The actual top-side marking has one additional character that designates the assembly/test site. YZP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the assembly/test site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6–7), tape
  and reel and mechanical pages (8–19) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: the features list except the lines quoted, the NanoStar and
  NanoFree package text, the enable/disable-time, pulse-duration and
  setup/hold parts of Figure 1 with their notes B, E and F (this device
  has no output enable), and the S1 positions for t~PLZ~/t~PZL~
  (2 × V~CC~) and t~PHZ~/t~PZH~ (GND).
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The addendum also lists SN74AUC1G06DBVRE4 and SN74AUC1G06DCKR1G4,
  which the catalog doesn't carry.
- From images: V~Δ~ (load table of Figure 1, p.5; the text layer reads
  "VD"); "No internal connection" and "Do not use" (legend of
  the pin drawing, p.1).
- Source issues:
  - Unconfirmed: S1 open for t~PLH~/t~PHL~ on an open-drain output
    (parameter measurement information).
