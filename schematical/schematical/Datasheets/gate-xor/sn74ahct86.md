---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct86",
  "title": "SNx4AHCT86 quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ahct86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS250P",
    "date": "2023-10",
    "pages": 39
  },
  "devices": ["gate:74_86"],
  "parts": ["5962-9681701QCA", "SN74AHCT86BQAR", "SN74AHCT86D", "SN74AHCT86DBR", "SN74AHCT86DGVR", "SN74AHCT86DR", "SN74AHCT86N", "SN74AHCT86NSR", "SN74AHCT86PW", "SN74AHCT86PWR", "SN74AHCT86RGYR", "SN74AHCT86RGYRG4", "SNJ54AHCT86J", "5962-9681701Q2A", "SNJ54AHCT86FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT86 quadruple 2-input exclusive-OR gates

## Description

The SNx4AHCT86 devices are quadruple 2-input exclusive-OR gates. These
devices perform the Boolean function Y = A × B or Y = !{A}B + A!{B} in
positive logic. Source error: the source prints the exclusive-OR
operator of the first form as "×"; the function table gives the
exclusive-OR function. [[1::1]]

Inputs are TTL-voltage compatible. Latch-up performance exceeds 250 mA
per JESD 17. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters. [[1::2]]

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
1Y: 1Y Output
2A: 2A Input
2B: 2B Input
2Y: 2Y Output
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
4Y: 4Y Output
4A: 4A Input
4B: 4B Input
GND: Ground Pin
NC: No Connection
VCC: Power Pin
```
[[3::Table 5-1]]

Conversion note: the RGY and BQA packages also have a thermal pad.

## Functional description

The SNx4AHCT86 devices are quadruple 2-input exclusive-OR gates. The
inputs are TTL compatible allowing 3.3 V to 5 V translation. [[9::1]] [[9::2]]

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an SN74AHCT86 gate in positive
logic; negation may be shown at any two ports. Conversion note: the
five symbols of Figure 8-1 are not drawn here. [[1::5]] [[1::6]]

The output is active (low) if all inputs stand at the same logic level
(that is, A = B). Conversion note: this is the logic-identity element
(symbol =). [[1::7]]

The output is active (low) if an even number of inputs (that is, 0 or 2)
are active. Conversion note: this is the even-parity element (symbol
2k). [[1::8]]

The output is active (high) if an odd number of inputs (that is, only 1
of the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1). [[1::9]]

- TTL inputs [[1::2]]
  - Lowered switching threshold allows up translation 3.3 V to 5 V [[9::8]]
- Slow edges reduce output ringing [[9::8]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
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

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[5::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 0 | 2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002, all pins [2] | 0 | 1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[5::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | SN54AHCT86 | < | SN74AHCT86 | < | Unit |
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

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```
[[5::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SNx4AHCT86, 14 pins"
| Symbol | Parameter [1] | D | DB | DGV | N | NS | PW | RGY | BQA | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 109.5 | 133.3 | 59.7 | 92.2 | 147.7 | 59.0 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.8 | 62.1 | 55.6 | 47.3 | 49.8 | 77.4 | 72.5 | 90.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 81 | 56.9 | 66.3 | 39.5 | 51.0 | 90.9 | 35.0 | 56.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 37 | 22.6 | 7.8 | 32.4 | 15.7 | 27.2 | 3.9 | 9.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 80.6 | 56.3 | 56.6 | 39.4 | 50.6 | 90.2 | 35.1 | 56.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | N/A | 15.4 | 33.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[6::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT86 | < | –40°C to 85°C, SN74AHCT86 | < | –40°C to 125°C, SN74AHCT86 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | 10 | | | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[6::table #2]]

```table id=noise caption="Noise characteristics, SN74AHCT86" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
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

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```
[[7::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | –55°C to 125°C, SN54AHCT86 | < | –40°C to 85°C, SN74AHCT86 | < | –40°C to 125°C, SN74AHCT86 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | 1 | 9 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 8.8 | 1 | 10 | 1 | 9 | 1 | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 8.8 | 1 | 10 | 1 | 9 | 1 | 11 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.7], [-40, 4.9], [0, 4.97], [25, 5], [85, 5.4], [125, 5.8]]}
 ]}
```
[[7::Figure 6-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. Conversion note: C~L~ is
15 pF or 50 pF, as given in the [switching characteristics](#sw), and
in the source figure inputs swing 0 V to 3 V and are measured at 1.5 V,
outputs at 50% V~CC~. [[8::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[8::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[8::Figure 7-1]]

Conversion note: the source figure also shows the load circuit for
3-state and open-drain outputs and the pulse-duration, setup-and-hold
and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

The SNx4AHCT86 is a low-drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The input switching levels have been lowered
to accommodate TTL inputs of 0.8-V V~IL~ and 2-V V~IH~. This feature
makes the device ideal for translating up from 3.3 V to 5 V. Figure 9-2
shows this type of translation. Conversion note: Figure 9-2 (p.11) is an
oscilloscope picture of an input and an output edge and is not drawn
here. Conversion note: in the typical application schematic (Figure 9-1,
p.10) a 3.3-V bus driver drives the gate inputs, the gate runs from a
5 V regulated V~CC~ bypassed with 0.1 µF, and its output drives a 5-V
accessory. [[10::2]]

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads; therefore, routing and load conditions should be
considered to prevent ringing. [[10::3]]

### Detailed design procedure

1. Recommended input conditions: [[10::4]]
   - Rise time and fall time specs: see (Δt/ΔV) in the Recommended Operating Conditions table. [[10::4]]
   - Specified High and low levels: see (V~IH~ and V~IL~) in the Recommended Operating Conditions table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at any valid V~CC~ [[10::4]]
2. Recommend output conditions: [[10::4]]
   - Load currents should not exceed 25 mA per output and 75 mA total for the part [[10::4]]
   - Outputs should not be pulled above V~CC~ [[10::4]]

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[11::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[11::2]]

### Layout guidelines

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
outputs unless the part is a transceiver. If the transceiver has an
output enable pin, it will disable the outputs section of the part when
asserted. This will not disable the input section of the IOs so they
cannot float when disabled. Conversion note: Figure 9-3, the layout
diagram (p.11), shows one gate with its unused input tied to V~CC~ and
one with its unused input tied to ground. [[11::4]]

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

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9681701Q2A | Tube of 55 | -55 to 125 | 5962-9681701Q2A SNJ54AHCT86FK | Last Time Buy
5962-9681701QCA | Tube of 25 | -55 to 125 | 5962-9681701QCA SNJ54AHCT86J
SN74AHCT86BQAR | Reel of 3000 | -40 to 125 | AHCT86
SN74AHCT86D | | -40 to 85 | AHCT86 | Obsolete
SN74AHCT86DBR | Reel of 2000 | -40 to 125 | HB86
SN74AHCT86DGVR | Reel of 2000 | -40 to 125 | HB86
SN74AHCT86DR | Reel of 2500 | -40 to 125 | AHCT86
SN74AHCT86N | Tube of 25 | -40 to 125 | SN74AHCT86N
SN74AHCT86NSR | Reel of 2000 | -40 to 125 | AHCT86
SN74AHCT86PW | | -40 to 125 | HB86 | Obsolete
SN74AHCT86PWR | Reel of 2000 | -40 to 125 | HB86
SN74AHCT86RGYR | Reel of 3000 | -40 to 125 | HB86 | NRND
SN74AHCT86RGYRG4 | Reel of 3000 | -40 to 125 | HB86 | NRND
SNJ54AHCT86FK | Tube of 55 | -55 to 125 | 5962-9681701Q2A SNJ54AHCT86FK | Last Time Buy
SNJ54AHCT86J | Tube of 25 | -55 to 125 | 5962-9681701QCA SNJ54AHCT86J
```
[[13::table #1]]

## Conversion notes

- Source pages 1–11 converted; the support page (12), the package
  option addendum, tape and reel and mechanical pages (13–39) dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, package information table,
  revision history, the exclusive-OR symbol drawings (Figure 8-1 and
  p.1), the typical application schematic (Figure 9-1), the up
  translation picture (Figure 9-2) and the layout diagram (Figure 9-3),
  each described in one sentence.
- The source prints the storage temperature row in the ESD ratings
  table, under MIN and MAX headers; it is kept there.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The TPD curve is digitized from Figure 6-1 (p.7) to about 2 % of full
  scale; the source states no load or V~CC~ for it.
- From images: 4.7, 4.9, 4.97, 5.4, 5.8 and the temperatures -55, -40,
  25, 85, 125 (curve points read from Figure 6-1, p.7); the overbars of
  Y = !{A}B + A!{B} (Description, p.1).
- The ordering block follows the package option addendum (pp.13–14).
- Source issues:
  - Source error: the Boolean function is printed "Y = A × B" (pp.1, 9);
    the function table (p.9) gives exclusive-OR. Kept as printed.
  - Source conflict: the Application Information (p.10) says low drive
    and slow edge rates, the Design Requirements (p.10) say balanced
    output drive whose high drive will create fast edges into light
    loads; both are kept.
  - Source conflict: the detailed design procedure (p.10) allows 75 mA
    total for the part; the absolute maximum ratings (p.5) give ±50 mA
    continuous current through V~CC~ or GND.
