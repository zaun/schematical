---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc2gu04",
  "title": "SN74AUC2GU04 dual inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74auc2gu04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES438C",
    "date": "2007-01",
    "pages": 15
  },
  "devices": ["gate:2GU04"],
  "parts": ["SN74AUC2GU04DBVR", "SN74AUC2GU04DCKR"],
  "packages": {"p6": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AUC2GU04 dual inverter gate

## Description

Optimized for 1.8-V operation and is 3.6-V I/O tolerant to support
mixed-mode signal operation. [[1::1]]

This dual inverter is operational at 0.8-V to 2.7-V V~CC~, but is
designed specifically for 1.65-V to 1.95-V V~CC~ operation. [[1::4]]

The SN74AUC2GU04 contains two inverters with unbuffered outputs and
performs the Boolean function Y = !{A}. [[1::5]]

For more information about AUC Little Logic devices, please refer to the
TI application report, Applications of Texas Instruments AUC Sub-1-V
Little Logic Devices, literature number SCEA027. [[1::7]]

## Pinout

Conversion note: DBV and DCK share one pin order.

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
| V~CC~ | Supply voltage range | | -0.5 | 3.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 3.6 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | DBV package | | 165 | °C/W |
| ^ | ^ | DCK package | | 259 | ^ |
| ^ | ^ | YZP package | | 123 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #2]]

Latch-up performance exceeds 100 mA per JESD 78, Class II. ESD
protection exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~IH~ | High-level input voltage | I~O~ = -100 µA | 0.65 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | I~O~ = -100 µA | | 0.35 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | -3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | -5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -9 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | ^ |
| Δt/Δv | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | V~IL~ = GND | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | ^ | 0.8 V | | 0.55 | | ^ |
| ^ | ^ | I~OH~ = -3 mA | ^ | 1.1 V | 0.8 | | | ^ |
| ^ | ^ | I~OH~ = -5 mA | ^ | 1.4 V | 1 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA | ^ | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -9 mA | ^ | 2.3 V | 1.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | V~IH~ = V~CC~ | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | ^ | 0.8 V | | 0.25 | | ^ |
| ^ | ^ | I~OL~ = 3 mA | ^ | 1.1 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 5 mA | ^ | 1.4 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 8 mA | ^ | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 9 mA | ^ | 2.3 V | | | 0.6 | ^ |
| I~I~ | A inputs | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~I~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 2.5 | | pF |

[1] All typical values are at T~A~ = 25°C.
```
[[3::table #2]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 4.5 | 4.5 | 4.5 | 4.5 | 5.5 | pF |
```
[[4::table #1]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 6.2 | 0.7 | 3.1 | 0.7 | 2.2 | 0.6 | 1.1 | 1.9 | 0.5 | 1.4 | ns |
```
[[4::table #2]]

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.7 | 1.6 | 2.7 | 0.5 | 2 | ns |
```
[[4::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. [[5::1]]

Conversion note: the source's test table sets S1 open for
t~PLH~/t~PHL~, which leaves R~L~ and C~L~ from the output to GND, and its
waveforms draw the input from 0 V to V~CC~ with the delays measured at
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
[[5::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~PLH~/t~PHL~ (S1 open)"
RL: see table
CL: see table
```
[[5::Figure 1]]

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```
[[5::figure #1]]

## Package

```package footprint=SOT23-6
```

```package footprint=SC70-6
```

## Ordering information

```ordering
SN74AUC2GU04DBVR | Reel of 3000 | -40 to 85 | UU4R
SN74AUC2GU04DCKR | Reel of 3000 | -40 to 85 | UDR
```
[[1::table #2]]

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –40°C to 85°C | NanoFree – WCSP (DSBGA) 0.23-mm Large Bump – YZP (Pb-free) | Reel of 3000 | SN74AUC2GU04YZPR | _ _ _UD_ |
| ^ | SOT (SOT-23) – DBV | Reel of 3000 | SN74AUC2GU04DBVR | UU4_ |
| ^ | SOT (SC-70) – DCK | Reel of 3000 | SN74AUC2GU04DCKR | UD_ |

[1] DBV/DCK: The actual top-side marking has one additional character that designates the assembly/test site. YZP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the assembly/test site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```
[[1::table #3]]

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6), tape
  and reel and mechanical pages (7–15) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: the features list except the lines quoted, the NanoFree
  package text, the YZP pin drawing, the enable/disable-time,
  pulse-duration and setup/hold parts of Figure 1 with their notes B, E
  and F (this inverter has no output enable), and the S1 positions for
  t~PLZ~/t~PZL~ (2 × V~CC~) and t~PHZ~/t~PZH~ (GND).
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The source also covers the YZP package (SN74AUC2GU04YZPR), which the
  package option addendum doesn't list and the catalog doesn't carry.
- The addendum also lists 74AUC2GU04DBVRG4 (marking UU4R), which the
  catalog doesn't carry.
- From images: overbar of Y = !{A} (p.1).
- From images: Ω, Δ, ≤ and ≥ in Figure 1 (p.5; the text layer has other
  glyphs).
- Source issues:
  - Unconfirmed: the recommended operating conditions (p.3) give
    I~O~ = –100 µA as the condition of both V~IH~ and V~IL~.
