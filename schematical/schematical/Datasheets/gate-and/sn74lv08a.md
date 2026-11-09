---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lv08a",
  "title": "SN74LV08A quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lv08a.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS387N",
    "date": "2025-09",
    "pages": 30
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74LV08AD", "SN74LV08ADBR", "SN74LV08ADGVR", "SN74LV08ADR", "SN74LV08ANSR", "SN74LV08APW", "SN74LV08APWR", "SN74LV08APWRG3", "SN74LV08APWRG4", "SN74LV08APWT", "SN74LV08ARGYR", "SN74LV08ARGYRG4"],
  "packages": {"p14": "D, DB, DGV, NS, PW, RGY"},
  "converted": "2026-11-08"
}
---

# SN74LV08A quadruple 2-input positive-AND gates

## Description

This quadruple 2-input positive-AND gate is designed for 2 V to 5.5 V
V~CC~ operation. The SN74LV08A device performs the Boolean function
Y = A • B or Y = !{!{A} + !{B}} in positive logic.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the devices when they are powered down. Inputs
accept voltages to 5.5 V, which allows down voltage translation.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Input A
1B, 2B, 3B, 4B: Input B
1Y, 2Y, 3Y, 4Y: Output Y
GND: Ground pin
VCC: Power pin
```

## Function tables

```table id=function caption="Function table (each gate)"
| A [1] | B [1] | Y [2] |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = High voltage level, L = Low voltage level, X = Don’t care.
[2] H = Driving high, L = Driving low, Z = High-impedance state.
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] This value is limited to 5.5 V maximum.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22C101, all pins [2] | ±1000 | V |
| ^ | ^ | Machine Model | ±200 | V |

[1] JEDEC document JEP155 states that 500 V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250 V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | V~CC~ × 0.7 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | V~CC~ × 0.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | V~CC~ × 0.3 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V [2] | | V~CC~ × 0.3 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | -2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | -6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | -12 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 2 | mA |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 6 | mA |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 12 | mA |
| Δt/Δv | Input transition rise and fall rate | V~CC~ = 2.3 V to 2.7 V | | 200 | ns/V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
[2] Unconfirmed: the source prints this cell as "VCC × 0.3 5", with a stray 5 after the value.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D | DB | DGV | N | NS | PW | RGY | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 90.6 | 107.1 | 129.0 | 57.4 | 90.7 | 122.6 | 57.5 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 50.9 | 59.6 | 52.1 | 44.9 | 48.3 | 51.4 | 70.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 44.8 | 54.4 | 62.0 | 37.2 | 49.4 | 64.4 | 33.6 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 14.7 | 20.5 | 6.5 | 30.1 | 14.6 | 6.7 | 3.4 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 44.5 | 53.8 | 61.3 | 37.1 | 49.1 | 63.8 | 33.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | | 13.9 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953). All packages 14 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40 °C to 85 °C | < | < | –40 °C to 125 °C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 2 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -2 mA | 2.3 V | 2 | | | 2 | | | V |
| ^ | ^ | I~OH~ = -6 mA | 3 V | 2.48 | | | 2.48 | | | V |
| ^ | ^ | I~OH~ = -12 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 2 mA | 2.3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 6 mA | 3 V | | | 0.44 | | | 0.44 | V |
| ^ | ^ | I~OL~ = 12 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±1 | | | ±1 | µA |
| I~CC~ | Static supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 20 | | | 20 | µA |
| I~off~ | Input/output power-off leakage current | V~I~ or V~O~ = 0 to 5.5 V | 0 | | | 5 | | | 5 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 3.3 V | | 3.3 | | | 3.3 | | pF |
| ^ | ^ | ^ | 5 V | | 3.3 | | | 3.3 | | pF |
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 3.3 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.2 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.1 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 3.1 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.31 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.99 | V |
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 10 MHz | 3.3 V | 8 | pF |
| ^ | ^ | ^ | 5 V | 10 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-2v5 caption="Switching characteristics, V~CC~ = 2.5 V ± 0.2 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | From | To | Load capacitance | T~A~ = 25 °C | < | < | –40 °C to 85 °C | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | C~L~ = 15 pF | | 7.9 [1] | 13.8 [1] | 1 | 16 | 1 | 17 | ns |
| ^ | ^ | ^ | ^ | C~L~ = 50 pF | | 10.5 | 17.3 | 1 | 20 | 1 | 21 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | From | To | Load capacitance | T~A~ = 25 °C | < | < | –40 °C to 85 °C | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | C~L~ = 15 pF | | 5.6 [1] | 8.8 [1] | 1 | 10.5 | 1 | 11.5 | ns |
| ^ | ^ | ^ | ^ | C~L~ = 50 pF | | 7.5 | 12.3 | 1 | 14 | 1 | 15 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | From | To | Load capacitance | T~A~ = 25 °C | < | < | –40 °C to 85 °C | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | Propagation delay | A or B | Y | C~L~ = 15 pF | | 4.1 [1] | 5.9 [1] | 1 | 7 | 1 | 8 | ns |
| ^ | ^ | ^ | ^ | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

```curve id=tpd-vcc caption="TPD vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 9]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 7.9], [3.3, 5.6], [5, 4.1]]}
 ]}
```

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 3.7], [-40, 3.9], [25, 4.1], [85, 4.5], [125, 4.9]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance; it is 15 pF or 50 pF as given
in the [switching characteristics](#switching-characteristics). All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: 50 % V~CC~
```

Figure 6-1 also shows the load circuit for 3-state and open-drain
outputs (R~L~ = 1 kΩ) and the voltage waveforms for pulse duration, setup
and hold times, and enable and disable times. All parameters and
waveforms are not applicable to all devices.

## Application information

The SN74LV08A is a low-drive CMOS device that can be used for a multitude
of bus interface type applications where output ringing is a concern.
The low drive and slow edge rates will minimize overshoot and undershoot
on the outputs. The inputs can accept voltages up to 5.5 V at any valid
V~CC~, making it ideal for down translation.

Typical application: a 5 V µC or system logic drives the 1A…4A inputs
(the B inputs are not shown); the 1Y…4Y outputs drive µC/system logic or
LEDs, with V~CC~ at 5 V.

Design requirements: this device uses CMOS technology and has balanced
output drive. Care should be taken to avoid bus contention because it can
drive currents that would exceed maximum limits. The high drive will also
create fast edges into light loads, so routing and load conditions should
be considered to prevent ringing.

- Inputs: for specified high and low levels, see V~IH~ and V~IL~ in the
  [recommended operating conditions](#recommended-operating-conditions).
  Inputs are overvoltage tolerant, allowing them to go as high as 5.5 V at
  any valid V~CC~.
- Outputs: load currents should not exceed 25 mA per output and 50 mA
  total for the part. Outputs should not be pulled above V~CC~.

Power supply: any voltage between the minimum and maximum supply voltage
in the recommended operating conditions. Each V~CC~ terminal should have
a good bypass capacitor to prevent power disturbance; for devices with a
single supply, a 0.1 µF capacitor is recommended. It is ok to parallel
multiple bypass capacitors to reject different frequencies of noise;
0.1 µF and 1.0 µF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power terminal as possible.

Layout: inputs should not float. Unused input pins should not be left
unconnected, because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a high or low bias to prevent them
from floating; generally they will be tied to GND or V~CC~, whichever
makes more sense or is more convenient. It is acceptable to float outputs
unless the part is a transceiver.

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

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74LV08AD | Obsolete | -40 to 125 | LV08A
SN74LV08ADBR | Reel of 2000 | -40 to 125 | LV08A
SN74LV08ADGVR | Reel of 2000 | -40 to 125 | LV08A
SN74LV08ADR | Reel of 2500 | -40 to 125 | LV08A
SN74LV08ANSR | Reel of 2000 | -40 to 125 | 74LV08A
SN74LV08APW | Obsolete | -40 to 125 | LV08A
SN74LV08APWR | Reel of 2000 | -40 to 125 | LV08A
SN74LV08APWRG3 | Reel of 2000 | -40 to 125 | LV08A
SN74LV08APWRG4 | Reel of 2000 | -40 to 125 | LV08A
SN74LV08APWT | Obsolete | -40 to 125 | LV08A
SN74LV08ARGYR | Reel of 3000 | -40 to 125 | LV08A
SN74LV08ARGYRG4 | Reel of 3000 | -40 to 125 | LV08A
```

Status in the package option addendum: SN74LV08AD, SN74LV08APW and
SN74LV08APWT are Obsolete. The thermal table also gives the N package,
which has no orderable part.

## Conversion notes

- Source pages 1–13 converted; the package option addendum, tape and reel
  and mechanical pages (14–30) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: features and applications lists, device information table,
  simplified schematic and logic diagram (four 2-input AND gates),
  Feature Description bullets (folded into Description), the empty
  application curves figure (Figure 8-2), layout example figure, support
  pages, revision history.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
- The typical application (Figure 8-1) is described in text.
- C~i~ typical values sit across the Min and Typ columns in the source;
  they are placed under Typ.
- Source issues:
  - Source error: the Overview (p.9) names the part "SN74LA08A"; the
    title and every other page say SN74LV08A. Corrected.
  - Unconfirmed: V~IL~ at V~CC~ = 4.5 V to 5.5 V is printed "VCC × 0.3 5"
    (p.5); kept as V~CC~ × 0.3 with a table note.
  - Source conflict: Application Information (p.10) calls the part a
    "low-drive CMOS device" with "slow edge rates", while Design
    Requirements (p.10) says "The high drive will also create fast
    edges". Both kept.
