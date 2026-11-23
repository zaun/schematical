---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct08",
  "title": "SNx4AHCT08 quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahct08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS237P",
    "date": "2023-10",
    "pages": 41
  },
  "devices": ["gate:74_08"],
  "parts": ["5962-9682101QCA", "5962-9682101QDA", "5962-9682101VDA", "SN74AHCT08BQAR", "SN74AHCT08D", "SN74AHCT08DBR", "SN74AHCT08DGVR", "SN74AHCT08DR", "SN74AHCT08N", "SN74AHCT08NSR", "SN74AHCT08PW", "SN74AHCT08PWR", "SN74AHCT08PWRG4", "SN74AHCT08RGYR", "SNJ54AHCT08J", "SNJ54AHCT08W", "5962-9682101Q2A", "SNJ54AHCT08FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-08"
}
---

# SNx4AHCT08 quadruple 2-input positive-AND gates

## Description

The SNx4AHCT08 devices are quadruple 2-input positive-AND gates. These
devices perform the Boolean function Y = A × B or Y = !{!{A} + !{B}} in
positive logic. [[1::1]]

Inputs are TTL-voltage compatible. TTL inputs allow up translation
from 3.3 V to 5 V. On products compliant to MIL-PRF-38535, all
parameters are tested unless otherwise noted. On all other products,
production processing does not necessarily include testing of all
parameters. Conversion note: SN74AHCT08 is specified from –40 °C to
125 °C and SN54AHCT08 from –55 °C to 125 °C (recommended operating
conditions). [[1::2]] [[9::1]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Input A
1B, 2B, 3B, 4B: Input B
1Y, 2Y, 3Y, 4Y: Output
GND: Ground
VCC: Power
NC: No internal connection
```
[[3::Table 5-1]]

## Functional description

The SNx4AHCT08 devices are quadruple 2-input positive-AND gates with low
drive that will produce slow rise and fall times. This slow transition
reduces ringing on the output signal. The device has TTL inputs that
allow up translation from 3.3 V to 5 V. The inputs are high impedance
when V~CC~ = 0 V. [[9::1]]

Source conflict: the Overview (p.9) says low drive with slow rise and
fall times, while the Design Requirements (p.10) say balanced output
drive whose high drive will create fast edges into light loads; both
are kept.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| T~J~ | Junction temperature | | | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 500-V HBM is possible with the necessary precautions.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 250-V CDM is possible with the necessary precautions.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | SN54AHCT08 | < | SN74AHCT08 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Application Report, Implications of Slow or Floating CMOS Inputs, (SCBA004).
```
[[5::table #3]]

## Thermal information

```table id=thermal caption="Thermal information (SNx4AHCT08) [1]"
| Symbol | Parameter | D (SOIC) | DB (SSOP) | DGV (TVSOP) | N (PDIP) | BQA (WQFN) | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 109.5 | 133.3 | 59.7 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.8 | 62.1 | 55.6 | 47.3 | 90.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 81 | 56.9 | 66.3 | 39.5 | 56.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 37 | 22.6 | 7.8 | 32.4 | 9.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 80.6 | 56.3 | 56.6 | 39.4 | 56.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, (SPRA953). Conversion note: all packages are 14 pins.
```
[[6::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | SN54AHCT08 | < | SN74AHCT08 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[6::table #2]]

```table id=noise caption="Noise characteristics, SN74AHCT08 [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```
[[7::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```
[[7::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | SN54AHCT08 | < | SN74AHCT08 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | 1 | 9 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4.5]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.94], [-40, 2.99], [0, 3.33], [25, 3.58], [50, 3.77], [75, 3.95], [100, 4.14], [125, 4.26]]}
 ]}
```
[[7::Figure 6-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. Conversion note: C~L~ is
15 pF or 50 pF, as given in the [switching characteristics](#sw), and
in the source figure inputs swing 0 V to 3 V and are measured at 1.5 V,
outputs at 50 % V~CC~. [[8::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[8::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50 % VCC (output)
```
[[8::Figure 7-1]]

Conversion note: the source figure also shows the 3-state and open-drain
load circuit, pulse-duration, setup-and-hold and enable/disable
waveforms. All parameters and waveforms are not applicable to all
devices.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. [[10::1]]

The SNx4AHCT08 devices are low-drive CMOS devices that can be used for a
multitude of bus-interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The TTL inputs can except voltages down
to 3.3 V and translate up to 5 V. Conversion note: "except" is the
source's word. Conversion note: in the typical
application (Figure 9-1) a 3.3-V bus driver drives the gate inputs; the gate runs from a 5 V
regulated V~CC~ bypassed with 0.1 µF, and its output drives a 5-V
accessory. [[10::2]]

Design requirements: this device uses CMOS technology and has balanced
output drive. Care should be taken to avoid bus contention because it
can drive currents that would exceed maximum limits. The high drive will
also create fast edges into light loads, so routing and load conditions
should be considered to prevent ringing. [[10::3]]

1. Recommended input conditions [[10::4]]
   - Rise time and fall time specs: See (Δt/ΔV) in the [Recommended Operating Conditions](#recommended-operating-conditions) table. [[10::4]]
   - Specified High and low levels: See (V~IH~ and V~IL~) in the Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at any valid V~CC~ [[10::4]]
2. Recommend output conditions [[10::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total for the part [[10::4]]
   - Outputs should not be pulled above V~CC~ [[10::4]]

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table.
Each V~CC~ pin
should have a good bypass capacitor to prevent power disturbance. For
devices with a single supply, 0.1 µF is recommended. If there are
multiple V~CC~ pins, 0.01 µF or 0.022 µF is recommended for each power
pin. A 0.1 µF and 1 µF are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results. [[11::1]] [[11::2]]

When using multiple bit logic devices inputs should not ever float. [[11::3]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Specified in
Figure 9-3 are the rules that must be observed under all circumstances.
All unused inputs of digital logic devices must be connected to a high
or low bias to prevent them from floating. The logic level that should
be applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~; whichever makes
more sense or is more convenient. It is generally acceptable to float
outputs unless the part is a transceiver. Conversion note: Figure 9-3,
the example layout (p.12), recommends a GND flood fill, a 0.1 µF bypass
capacitor placed close to the device, unused inputs tied to V~CC~, an
unused output left floating, and no 90° corners for signal lines. [[11::4]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
SN74AHCT08BQAR | Reel of 3000 | -40 to 125 | AHCT08
SN74AHCT08D | | -40 to 125 | AHCT08 | Obsolete
SN74AHCT08DBR | Reel of 2000 | -40 to 125 | HB08
SN74AHCT08DGVR | Reel of 2000 | -40 to 125 | HB08
SN74AHCT08DR | Reel of 2500 | -40 to 125 | AHCT08
SN74AHCT08N | Tube of 25 | -40 to 125 | SN74AHCT08N | NRND
SN74AHCT08NSR | Reel of 2000 | -40 to 125 | AHCT08 | NRND
SN74AHCT08PW | | -40 to 125 | HB08 | Obsolete
SN74AHCT08PWR | Reel of 2000 | -40 to 125 | HB08
SN74AHCT08PWRG4 | Reel of 2000 | -40 to 125 | HB08
SN74AHCT08RGYR | Reel of 3000 | -40 to 125 | HB08
5962-9682101QCA | Tube of 25 | -55 to 125 | 5962-9682101QCA SNJ54AHCT08J
SNJ54AHCT08J | Tube of 25 | -55 to 125 | 5962-9682101QCA SNJ54AHCT08J
5962-9682101QDA | Tube of 25 | -55 to 125 | 5962-9682101QDA SNJ54AHCT08W
SNJ54AHCT08W | Tube of 25 | -55 to 125 | 5962-9682101QDA SNJ54AHCT08W
5962-9682101VDA | Tube of 25 | -55 to 125 | 5962-9682101VDA SNV54AHCT08W
5962-9682101Q2A | Tube of 55 | -55 to 125 | 5962-9682101Q2A SNJ54AHCT08FK
SNJ54AHCT08FK | Tube of 55 | -55 to 125 | 5962-9682101Q2A SNJ54AHCT08FK
```
[[13::table #1]] [[14::table #1]]

## Conversion notes

- Source pages 1–12 converted; revision history, support pages, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, package information table,
  simplified schematic and functional block diagram (one AND gate). The
  layout example (Figure 9-3) is described in text.
- Dropped: Figure 9-2 (switching characteristics comparison, an
  oscilloscope-style plot of V~I~ and the ACT08, HCT08 and AHCT08 outputs
  at V~CC~ = 5 V, load 50 Ω/50 pF); it is not a characteristic of this
  part alone.
- The ordering block follows the package option addendum (pp.13–14).
- The TPD curve is digitized from Figure 6-1 to about 2 % of full scale;
  the source states no load or V~CC~ for it.
- The typical application circuit (Figure 9-1) is described in text.
- From images: 25, 75 (temperature points read from Figure 6-1); 0.1 µF,
  5 V, 3.3-V, 5-V (Figure 9-1, p.10); 50 Ω, 50 pF, ACT08, HCT08 (Figure
  9-2, p.11); 0 V, 3 V input levels (Figure 7-1, p.8); overbars of
  Y = !{!{A} + !{B}} (p.1).
- Source issues:
  - Source conflict: Overview "low drive ... slow rise and fall times" vs
    Design Requirements "high drive ... fast edges" (Functional
    description note).
