---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv21a",
  "title": "SN74LV21A dual 4-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lv21a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES340F",
    "date": "2023-07",
    "pages": 25
  },
  "devices": ["gate:74_21"],
  "parts": ["SN74LV21AD", "SN74LV21ADBR", "SN74LV21ADGVR", "SN74LV21ADR", "SN74LV21ANSR", "SN74LV21APW", "SN74LV21APWR", "SN74LV21APWRG4"],
  "packages": {"p14": "D, DB, DGV, NS, PW"},
  "converted": "2026-11-08"
}
---

# SN74LV21A dual 4-input positive-AND gate

## Description

These dual 4-input positive-AND gates are designed for 2-V to 5.5-V
V~CC~ operation. The SN74LV21A devices perform the Boolean function
Y = A • B • C • D in positive logic.

These devices are fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the devices when they are powered
down.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A: Input A
1B, 2B: Input B
1C, 2C: Input C
1D, 2D: Input D
1Y, 2Y: Output Y
GND: Ground pin
VCC: Power pin
NC: Not internally connected
```

## Function tables

```table id=function caption="Function table (each gate)"
| A [1] | B [1] | C [1] | D [1] | Y [2] |
|:-:|:-:|:-:|:-:|:-:|
| H | H | H | H | H |
| L | X | X | X | L |
| X | L | X | X | L |
| X | X | L | X | L |
| X | X | X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Do not Care, Z = High Impedance
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range applied in high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range applied in power-off state [2] | | -0.5 | 7 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

Latch-up performance exceeds 100 mA per JESD 78, Class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise and fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D | DB | DGV | NS | PW | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 96 | 127 | 76 | 113 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953). All packages 14 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -2 mA | 2.3 V | 2 | | | V |
| ^ | ^ | I~OH~ = -6 mA | 3 V | 2.48 | | | V |
| ^ | ^ | I~OH~ = -12 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | V |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | µA |
| I~off~ | Off-state leakage current | V~I~ or V~O~ = 0 to 5.5 V | 0 V | | | 5 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 1.9 | | pF |
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25 °C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | 0 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.2 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 17.4 | pF |
| ^ | ^ | ^ | 5 V | 20.2 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement). The
SN74LV21A columns are over the recommended operating free-air
temperature range.

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | Load capacitance | T~A~ = 25 °C | < | < | SN74LV21A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, C or D | Y | C~L~ = 15 pF | | 7 | 12 | 1 | 14 | ns |
| t~pd~ | A, B, C or D | Y | C~L~ = 50 pF | | 9.2 | 15.7 | 1 | 19 | ns |
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | Load capacitance | T~A~ = 25 °C | < | < | SN74LV21A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, C, or D | Y | C~L~ = 15 pF | | 5.1 | 7 | 1 | 8.5 | ns |
| t~pd~ | A, B, C, or D | Y | C~L~ = 50 pF | | 6.6 | 10.5 | 1 | 12 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | Load capacitance | T~A~ = 25 °C | < | < | SN74LV21A | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, C, or D | Y | C~L~ = 15 pF | | 3.8 | 5 | 1 | 6 | ns |
| t~pd~ | A, B, C, or D | Y | C~L~ = 50 pF | | 4.9 | 7 | 1 | 8 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance; it is 15 pF or 50 pF as given
in the [switching characteristics](#switching-characteristics). All
input pulses are supplied by generators having the following
characteristics: PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The
outputs are measured one at a time, with one input transition per
measurement. t~PHL~ and t~PLH~ are the same as t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: 50 % V~CC~
```

Figure 6-1 also shows the load circuit for 3-state and open-drain
outputs and the voltage waveforms for pulse duration, setup and hold
times, and enable and disable times. All parameters and waveforms are
not applicable to all devices.

## Package

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

## Ordering information

```ordering
SN74LV21AD | Obsolete | -40 to 85 | LV21A
SN74LV21ADBR | Reel of 2000 | -40 to 85 | LV21A
SN74LV21ADGVR | Reel of 2000 | -40 to 85 | LV21A
SN74LV21ADR | Reel of 2500 | -40 to 85 | LV21A
SN74LV21ANSR | Reel of 2000 | -40 to 85 | 74LV21A
SN74LV21APW | Obsolete | -40 to 85 | LV21A
SN74LV21APWR | Reel of 2000 | -40 to 85 | LV21A
SN74LV21APWRG4 | 2000 | -40 to 85 | LV21A
```

Status in the package option addendum: SN74LV21AD and SN74LV21APW are
Obsolete, SN74LV21ADGVR is NRND, SN74LV21ANSR is Last.

## Conversion notes

- Source pages 1–9 converted; the package option addendum, tape and reel
  and mechanical pages (10–25) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features list (except the latch-up rating), package
  information table, simplified schematic and logic diagram (two 4-input
  AND gates), related links table, support pages, revision history.
- From images: 50 % V~CC~ measurement levels (Figure 6-1, p.7, an image).
- Source issues:
  - Source conflict: the package option addendum (p.10) lists
    SN74LV21APWRG4 twice, with carrier "null" and RoHS "Yes" in one row
    and "No" in the other; the packing column gives only the quantity.
