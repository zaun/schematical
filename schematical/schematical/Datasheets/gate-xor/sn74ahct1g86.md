---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g86",
  "title": "SN74AHCT1G86 single 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ahct1g86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS324P",
    "date": "2024-02",
    "pages": 24
  },
  "devices": ["gate:1G86"],
  "parts": ["74AHCT1G86DBVRG4", "74AHCT1G86DBVTG4", "SN74AHCT1G86DBVR", "SN74AHCT1G86DBVT", "SN74AHCT1G86DCKR", "SN74AHCT1G86DCKT"],
  "packages": {"p5": "DBV, DCK"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G86 single 2-input exclusive-OR gate

## Description

The SN74AHCT1G86 is a single 2-input exclusive-OR gate. The device
performs the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic.

Operating range of 4.5V to 5.5V. Max t~pd~ of 8ns at 5V. 8mA output
drive at 5V. Inputs are TTL-voltage compatible. Latch-up performance
exceeds 250mA per JESD 17.

Source error: the Features (p.1) also print "Low power consumption, 10A maximum I~CC~"; the electrical characteristics give an I~CC~ of at most 10 µA. Kept out.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: A input
B: B input
GND: Ground pin
Y: Output
VCC: Power pin
```

## Functional description

Source error: the Overview (p.8) says "To ensure the high-impedance state during power up or power down, OE should be tied to GND through a pulldown resistor"; the SN74AHCT1G86 has no OE pin and no high-impedance state (pin table, p.3). Kept out.

Conversion note: the functional block diagram (Figure 7-1, a picture) shows five equivalent exclusive-OR symbols and the logic-identity, even-parity and odd-parity elements; it is not drawn here.

The device is ideal for operating in a 5-V logic system. The low
propagation delay allows fast switching and higher speeds of operation.
In addition, the low power consumption makes this device a good choice
for portable and battery power-sensitive applications.

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | -20 | 20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | -25 | 25 | mA |
| | Continuous current through V~CC~ or GND | | -50 | 50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| T~J~ | Junction Temperature | | | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | | V |
| V~IL~ | Low-level input voltage | | | 0.8 | V |
| V~I~ | Input voltage | 0 | | 5.5 | V |
| V~O~ | Output voltage | 0 | | V~CC~ | V |
| I~OH~ | High-level output current | | | -8 | mA |
| I~OL~ | Low-level output current | | | 8 | mA |
| Δt/Δv [1] | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | | 125 | °C |

[1] Source error: the source prints this symbol as "t/v" (p.4); its design procedure (§ 8.2.2) refers to it as Δt/ΔV.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT1G86: thermal metric [1]"
| Symbol | Parameter | DBV (SOT-23) 5 pins | DCK (SC-70) 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| ΔI~CC~ [1] | Supply-current change | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 5 | 6.2 | 1 | 8 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | 5 | 6.2 | 1 | 8 | 1 | 9 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 | 7.9 | 1 | 9 | 1 | 10 | ns |
```

## Typical characteristics

```curve id=tpd-temp caption="TPD vs. Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[-55, 4.2], [-40, 4.4], [25, 5], [85, 5.4], [125, 5.6]]}
 ]}
```

## Parameter measurement information

Conversion note: Figure 6-1 (p.7) is a picture; its notes are quoted in the Conversion notes, and it gives no value for C~L~, which is 15 pF or 50 pF in the [switching characteristics](#sw).

Conversion note: in the figure the inputs swing 0 V to 3 V and are measured at 1.5 V, and the outputs are measured at 50% V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```

Conversion note: the source figure also shows the load circuit for
3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The SN74AHCT1G86 is a Low drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The input switching levels have been lowered
to accommodate TTL inputs of 0.8-V V~IL~ and 2-V V~IH~. This feature
makes it Ideal for translating up from 3.3 V to 5 V.

Conversion note: the typical application schematic (Figure 8-1) shows the gate, supplied from a 5V regulated rail with a 0.1uf capacitor to ground, between 3.3V system logic and a uC or system logic.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads so consider routing and load conditions to prevent ringing.

Source conflict: Application Information (§ 8.1) says "low drive and slow edge rates", Design Requirements (§ 8.2.1) says "the high drive will also create fast edges"; both are kept as printed.

### Detailed design procedure

- Recommended input conditions:
  - Rise time and fall time specs. See (Δt/ΔV) in
    [Section 5.3](#recommended-operating-conditions).
  - Specified high and low levels. See (V~IH~ and V~IL~) in
    Section 5.3.
- Recommended output conditions:
  - Load currents should not exceed 25 mA per output and 50 mA total for the part.
  - Outputs should not be pulled above V~CC~.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Section 5.3.

Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. For devices with a single supply, TI recommends a
0.1-μF capacitor and if there are multiple V~CC~ terminals then TI
recommends a 0.01-μF or 0.022-μF capacitor for each power terminal.
Multiple bypass capacitors can be paralleled to reject different
frequencies of noise. Frequencies of 0.1 μF and 1 μF are commonly used
in parallel. The bypass capacitor should be installed as close as
possible to the power terminal for best results.

### Layout guidelines

When using multiple bit logic devices inputs should not ever float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND
gate are used or only three of the four buffer gates are used. Such
input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states. Specified below are the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~ whichever make more sense or is more convenient. Floating
outputs is generally acceptable, unless the part is a transceiver. If
the transceiver has an output enable pin it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I.O’s so they also cannot float when disabled.

Conversion note: the layout example (Figure 8-3) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, and avoiding 90° corners for signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

## Ordering information

```ordering
74AHCT1G86DBVRG4 | Reel of 3000 | -40 to 125 | B86G
74AHCT1G86DBVTG4 | | -40 to 125 | B86G | Obsolete
SN74AHCT1G86DBVR | Reel of 3000 | -40 to 125 | 39EH, 3CDF, B863, B86G, B86J, B86S
SN74AHCT1G86DBVT | | -40 to 125 | B863, B86G, B86J, B86S | Obsolete
SN74AHCT1G86DCKR | Reel of 3000 | -40 to 125 | 1QV, BH3, BHG, BHJ, BHL, BHS
SN74AHCT1G86DCKT | | -40 to 125 | BH3, BHG, BHJ, BHS | Obsolete
```

## Conversion notes

- Source pages 1–11 converted. The ordering lines are from the package
  option addendum (pp.14–15). The support and revision history page
  (12), the mechanical information page (13), tape and reel and
  mechanical pages (16–24) are left out; the generated Package section
  replaces them.
- Dropped: features and applications lists (features kept as sentences
  in the Description), package information table, the exclusive-OR
  logic picture (p.1 and Figure 7-1, described in one sentence); the
  typical application schematic (Figure 8-1) and the layout example
  (Figure 8-3) are described in text.
- Dropped: Figure 8-2 (p.10, an oscilloscope picture of an input edge
  and an output edge with no scales, captioned "Translation From 3.3 V
  to 5.5 V" and labelled "Tranlsation from 3.3V to 5V" inside).
- The pin table's note "Signal Types: I = Input, O = Output, I/O =
  Input or Output" is on its TYPE column, which the generated pin table
  draws from the catalog.
- The curve is digitized from Figure 5-1 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for it and no unit on the
  temperature axis.
- Figure 6-1 (p.7) is a picture. Its notes read: "A. C~L~ includes probe
  and jig capacitance. C. All input pulses are supplied by generators
  having the following characteristics: PRR 1 MHz, Z~O~ = 50, t~r~
  3 ns, t~f~ 3 ns. D. The outputs are measured one at a time with one
  input transition per measurement. E. All parameters and waveforms are
  not applicable to all devices."
- From images: the overbars of Y = !{A}B + A!{B} (p.1 and p.8); the
  curve points -55, 4.2, 4.4, 85, 5.4, 5.6 (Figure 5-1, p.6); 0 V, 3 V,
  1.5 V input levels and 50% V~CC~ (Figure 6-1, p.7); 3.3V, 5V
  regulated, 0.1uf, uC (Figure 8-1, p.9); 0.1 µF (Figure 8-3, p.11; the
  text layer prints "0.1 F").
- Source issues:
  - Source error: Features print "10A maximum I~CC~" (Description
    note).
  - Source error: the Overview's sentence on OE and the high-impedance
    state (Functional description note).
  - Source error: Δt/Δv printed as "t/v" (recommended operating
    conditions note 1).
  - Unconfirmed: Note C of Figure 6-1 (p.7) is printed with gaps where
    signs would stand, "PRR 1 MHz, Z~O~ = 50, t~r~ 3 ns, t~f~ 3 ns";
    kept as printed.
  - Source conflict: "low drive and slow edge rates" (§ 8.1) vs "high
    drive ... fast edges" (§ 8.2.1) (Application information note).
  - Source conflict: Figure 8-2 is captioned "Translation From 3.3 V to
    5.5 V"; the picture's own label and § 8.1 say 3.3 V to 5 V.
  - Source error: "Frequencies of 0.1 μF and 1 μF are commonly used in
    parallel" (§ 8.3) calls capacitor values frequencies; kept as
    printed.
