---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc86",
  "title": "SNx4AHC86 quadruple 2-input exclusive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ahc86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS249L",
    "date": "2023-10",
    "pages": 42
  },
  "devices": ["gate:74_86"],
  "parts": ["5962-9681601QCA", "5962-9681601QDA", "SN74AHC86BQAR", "SN74AHC86D", "SN74AHC86DBR", "SN74AHC86DGVR", "SN74AHC86DR", "SN74AHC86DRG4", "SN74AHC86N", "SN74AHC86NSR", "SN74AHC86PW", "SN74AHC86PWR", "SN74AHC86RGYR", "SNJ54AHC86J", "SNJ54AHC86W", "5962-9681601Q2A", "SNJ54AHC86FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHC86 quadruple 2-input exclusive-OR gates

## Description

The SNx4AHC86 devices are quadruple 2-input exclusive-OR gates. These
devices perform the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in
positive logic.

A common application is as a true or complement element. If one of the
inputs is low, the other input is reproduced in true form at the
output. If one of the inputs is high, the signal on the other input is
reproduced inverted at the output.

Operating range 2-V to 5.5-V V~CC~. Latch-up performance exceeds 250 mA
per JESD 17. ESD protection exceeds JESD 22: 2000-V human-body model
(A114-A), 200-V machine model (A115-A), 1000-V charged-device model
(C101).

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A: Channel 1, Input A
1B: Channel 1, Input B
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2B: Channel 2, Input B
2Y: Channel 2, Output Y
GND: Ground
3Y: Channel 3, Output Y
3A: Channel 3, Input A
3B: Channel 3, Input B
4Y: Channel 4, Output Y
4A: Channel 4, Input A
4B: Channel 4, Input B
VCC: Positive Supply
NC: Not internally connected
```

The thermal pad can be connected to GND or left floating. Do not
connect to any other signal or supply. Conversion note: the source has
this for the thermal pad, BQA package only.

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. These are five
equivalent exclusive-OR symbols valid for an SN74AHC86 gate in positive
logic; negation may be shown at any two ports. Conversion note: the
five symbols of Figure 8-1 are not drawn here.

The output is active (low) if all inputs stand at the same logic level
(i.e., A = B). Conversion note: this is the logic-identity element
(symbol =).

The output is active (low) if an even number of inputs (i.e., 0 or 2)
are active. Conversion note: this is the even-parity element (symbol
2k).

The output is active (high) if an odd number of inputs (i.e., only 1 of
the 2) are active. Conversion note: this is the odd-parity element
(symbol 2k + 1).

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

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Value | < | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | SN54AHC86 | < | SN74AHC86 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | 1.5 | | V |
| ^ | ^ | V~CC~ = 3V | 2.1 | | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | 3.85 | | V |
| V~IL~ | Low-level Input voltage | V~CC~ = 2 V | | 0.5 | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V [1] | | -50 | | -50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V [1] | | 50 | | 50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | | 8 | mA |
| Δt/Δv | Input Transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 125 | °C |

[1] Unconfirmed: the source prints the unit mA for the V~CC~ = 2 V rows; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = –50 µA and I~OL~ = 50 µA.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D [2] SOIC 14 | DB [2] SSOP 14 | DGV [2] TVSOP 14 | N [2] PDIP 14 | NS [2] SOP 14 | PW [2] TSSOP 14 | RGY [3] VQFN 14 | BQA WQFN 14 | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ [4] | Junction-to-ambient thermal resistance | 124.5 | 96 | 127 | 80 | 76 | 147.7 | 47 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
[2] The package thermal impedance is calculated in accordance with JESD 51-7.
[3] The package thermal impedance is calculated in accordance with JESD 51-5
[4] Conversion note: the text layer prints this symbol as "R θJA", in two pieces.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –55°C to 125°C, SN54AHC86 | < | T~A~ = –40°C to 85°C, SN74AHC86 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC86 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA [2] | 3 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA [2] | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | 10 | | | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at VCC = 0 V.
[2] Source error: printed as I~OH~ = 4 mA and I~OH~ = 8 mA; the rows give V~OL~, and the recommended operating conditions give I~OL~ = 4 mA and 8 mA, so the condition is I~OL~.
```

```table id=noise caption="Noise characteristics, SN74AHC86" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.3 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.3 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC86 | < | T~A~ = –40°C to 85°C, SN74AHC86 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC86 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 7 [1] | 11 [1] | 1 [1] | 13 [1] | 1 | 13 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | 7 [1] | 11 [1] | 1 [1] | 13 [1] | 1 | 13 | 1 | 13 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 9.5 | 14.5 | 1 | 16.5 | 1 | 16.5 | 1 | 16.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 9.5 | 14.5 | 1 | 16.5 | 1 | 16.5 | 1 | 16.5 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC86 | < | T~A~ = –40°C to 85°C, SN74AHC86 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC86 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 4.8 [1] | 6.8 [1] | 1 [1] | 8 [1] | 1 | 8 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | 4.8 [1] | 6.8 [1] | 1 [1] | 8 [1] | 1 | 8 | 1 | 8 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 6.3 | 8.8 | 1 | 10 | 1 | 10 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | 6.3 | 8.8 | 1 | 10 | 1 | 10 | 1 | 10 | ns |

[1] Source conflict: the 5-V table (p.6) marks these values with (1) and prints no note under the table; the 3.3-V table (p.5) prints note (1): On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. Conversion note: C~L~ is
15 pF or 50 pF, as given in the
[switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50 % V~CC~
```

Conversion note: the source figure also shows the load circuit for
3-state and open-drain outputs and the pulse-duration, setup-and-hold
and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

Source error: the Application Information text (§ 9.1), the typical application block diagram (Figure 9-1) and the application timing diagram (Figure 9-2) describe three 2-input AND gates combined into a 4-input AND gate and name the SN74LV4T08-Q1; this data sheet's device is an exclusive-OR gate (title and function table). Kept out.

### Power considerations

Ensure the desired supply voltage is within the range specified in the
Recommended Operating Conditions. The supply voltage sets the
electrical characteristics of the device as described in the Electrical
Characteristics section.

The positive voltage supply must be capable of sourcing current equal
to the maximum static supply current, I~CC~, listed in the Electrical
Characteristics, and any transient current required for switching.

The ground must be capable of sinking current equal to the total
current to be sunk by all outputs of the SNx4AHC86 plus the maximum
supply current, I~CC~, listed in the Electrical Characteristics, and
any transient current required for switching. The logic device can only
sink as much current that can be sunk into its ground connection. Be
sure to not exceed the maximum total current through GND listed in the
Absolute Maximum Ratings.

The SNx4AHC86 can drive a load with a total capacitance less than or
equal to 50 pF while still meeting all of the data sheet
specifications. Larger capacitive loads can be applied; however, it is
not recommended to exceed 50 pF.

The SNx4AHC86 can drive a load with total resistance described by
R~L~ ≥ V~O~ / I~O~, with the output voltage and current defined in the
Electrical Characteristics table with V~OL~. When outputting in the
HIGH state, the output voltage in the equation is defined as the
difference between the measured output voltage and the supply voltage
at the V~CC~ pin.

The maximum junction temperature, T~J(max)~ listed in the Absolute
Maximum Ratings, is an additional limitation to prevent damage to the
device. Do not violate any values listed in the Absolute Maximum
Ratings. These limits are provided to prevent damage to the device.
Source conflict: the Absolute Maximum Ratings table (p.4) lists no
junction temperature.

### Input considerations

Input signals must cross to be considered a logic LOW, and to be
considered a logic HIGH. Source error: the source prints this sentence
without the two levels the signals must cross (p.10); kept as printed.
Do not exceed the maximum input voltage range found in the Absolute
Maximum Ratings.

Unused inputs must be terminated to either V~CC~ or ground. The unused
inputs can be directly terminated if the input is completely unused, or
they can be connected with a pull-up or pull-down resistor if the input
will be used sometimes, but not always. A pull-up resistor is used for
a default state of HIGH, and a pull-down resistor is used for a default
state of LOW. The drive current of the controller, leakage current into
the SNx4AHC86 (as specified in the Electrical Characteristics), and the
desired input transition rate limits the resistor size. A 10-kΩ
resistor value is often used due to these factors.

### Output considerations

The ground voltage is used to produce the output LOW voltage. Sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification in the Electrical Characteristics.

Unused outputs can be left floating. Do not connect outputs directly to
V~CC~ or ground.

### Detailed design procedure

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Ensure the capacitive load at the output is ≤ 50 pF. This is not a
   hard limit; it will, however, ensure optimal performance. Source
   error: the next sentence of the source names the traces of another
   device, the SN74AHC1G04-Q1. Kept out.
3. Ensure the resistive load at the output is larger than (V~CC~ /
   I~O(max)~) Ω, so that the maximum output current from the Absolute
   Maximum Ratings is not violated. Most CMOS inputs have a resistive
   load measured in MΩ; much larger than the minimum calculated
   previously.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions.
Each V~CC~ terminal should have a good bypass capacitor to prevent
power disturbance. A 0.1-μF capacitor is recommended for this device.
It is acceptable to parallel multiple bypass capacitors to reject
different frequencies of noise. The 0.1-μF and 1-μF capacitors are
commonly used in parallel. The bypass capacitor should be installed as
close to the power terminal as possible for best results.

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4
buffer gates are used). Such unused input pins must not be left
unconnected because the undefined voltages at the outside connections
result in undefined operational states. All unused inputs of digital
logic devices must be connected to a logic high or logic low voltage,
as defined by the input voltage specifications, to prevent them from
floating. The logic level that must be applied to any particular unused
input depends on the function of the device. Generally, the inputs are
tied to GND or V~CC~, whichever makes more sense for the logic function
or is more convenient. Conversion note: Figure 9-3, the example layout
(p.12), recommends a GND flood fill, a bypass capacitor placed close to
the device, unused inputs tied to V~CC~, an unused output left
floating, and no 90° corners for signal lines.

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
5962-9681601QCA | Tube of 25 | -55 to 125 | 5962-9681601QCA SNJ54AHC86J
5962-9681601QDA | Tube of 25 | -55 to 125 | 5962-9681601QDA SNJ54AHC86W
SN74AHC86BQAR | Reel of 3000 | -40 to 125 | AHC86
SN74AHC86D | | -40 to 125 | AHC86 | Obsolete
SN74AHC86DBR | Reel of 2000 | -40 to 125 | HA86
SN74AHC86DGVR | Reel of 2000 | -40 to 125 | HA86
SN74AHC86DR | Reel of 2500 | -40 to 125 | AHC86
SN74AHC86DRG4 | Reel of 2500 | -40 to 125 | AHC86
SN74AHC86N | Tube of 25 | -40 to 125 | SN74AHC86N
SN74AHC86NSR | Reel of 2000 | -40 to 125 | AHC86
SN74AHC86PW | | -40 to 125 | HA86 | Obsolete
SN74AHC86PWR | Reel of 2000 | -40 to 125 | HA86
SN74AHC86RGYR | Reel of 3000 | -40 to 125 | HA86 | NRND
SNJ54AHC86J | Tube of 25 | -55 to 125 | 5962-9681601QCA SNJ54AHC86J
SNJ54AHC86W | Tube of 25 | -55 to 125 | 5962-9681601QDA SNJ54AHC86W
5962-9681601Q2A | Tube of 55 | -55 to 125 | 5962-9681601Q2A SNJ54AHC86FK
SNJ54AHC86FK | Tube of 55 | -55 to 125 | 5962-9681601Q2A SNJ54AHC86FK
```

## Conversion notes

- Source pages 1–12 converted; the support page (13), the package
  option addendum, tape and reel and mechanical pages (14–42) dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, device information table,
  revision history, the exclusive-OR symbol drawings (Figure 8-1 and
  p.1), the layout example drawing (Figure 9-3, summarized in one
  sentence), the power-consumption and thermal references of 9.2.1, the
  references to a Feature Description section (the source has none) and
  step 4 of the design procedure.
- The absolute maximum ratings are a list under one VALUE header in the
  source, with ranges printed as "–0.5 to 7"; here the Value header
  spans two columns, the lower limit in the first and the upper limit
  or the single value in the second. The symbol, parameter and
  condition of each line ("Input clamp current, I~IK~ (V~I~ < 0)") are
  put in separate cells.
- The thermal table's symbol is printed "R θJA" in two pieces in the
  text layer (thermal note 4).
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- Figure 7-1 (p.7) is a picture; only its notes are in the text layer.
- From images: 50 % V~CC~ (V~M~, Figure 7-1, p.7); the overbars of
  Y = !{A}B + A!{B} (Description, p.1).
- The ordering block follows the package option addendum (pp.14–15).
- Source issues:
  - Source error: § 9.1 and Figures 9-1 and 9-2 describe a 4-input AND
    function built from AND gates and name the SN74LV4T08-Q1. Kept out.
  - Source error: step 2 of the design procedure names the
    SN74AHC1G04-Q1. That sentence is kept out.
  - Source error: § 9.2.1.1 "Input signals must cross to be considered a
    logic LOW, and to be considered a logic HIGH" names no levels; kept
    as printed.
  - Source error: V~OL~ rows printed with I~OH~ = 4 mA / 8 mA; corrected
    to I~OL~ (electrical characteristics note 2).
  - Source conflict: the caution of § 9.2.1 says T~J(max)~ is listed in
    the Absolute Maximum Ratings; the table lists no junction
    temperature.
  - Source conflict: the 5-V switching table marks values with (1) and
    prints no note; the 3.3-V table prints it (switching table note).
  - Unconfirmed: I~OH~/I~OL~ at V~CC~ = 2 V printed as ±50 mA (ROC table
    note).
