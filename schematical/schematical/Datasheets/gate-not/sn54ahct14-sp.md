---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn54ahct14-sp",
  "title": "SNx4AHCT14 hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn54ahct14-sp.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS246T",
    "date": "2023-10",
    "pages": 42
  },
  "devices": ["gate:74_14"],
  "parts": ["5962-9680101QCA"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT14 hex Schmitt-trigger inverters

## Description

The SNx4AHCT14 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}. [[9::1]]

Inputs are TTL-voltage compatible. Latch-up performance exceeds 250 mA
per JESD 17. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A1
1Y: 1Y1
2A: 2A1
2Y: 2Y1
3A: 3A1
3Y: 3Y1
GND: Ground pin
4Y: 4Y1
4A: 4A1
5Y: 5Y1
5A: 5A1
6Y: 6Y1
6A: 6A1
VCC: Power pin
```
[[3::Table 5-1]]

Conversion note: the pin table (Table 5-1) describes the signal pins only as 1A1, 1Y1 and so on, and its type column gives I = input for the A pins and O = output for the Y pins.

## Functional description

The SNx4AHCT14 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}. Each circuit functions as an
independent inverter, but because of the Schmitt action, the inverters
have different input threshold levels for positive-going (V~T+~) and
for negative-going (V~T−~) signals. [[9::1]]

Conversion note: the functional block diagram shows one inverter with a Schmitt-trigger symbol, input A and output Y.

- Inputs are TTL-Voltage compatible [[1::2]]
- Inputs accept very slow or noisy inputs [[9::2]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input A | Output Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[9::Table 8-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=handling caption="Handling ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 0 | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 0 | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | SN54AHCT14 | < | SN74AHCT14 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Application Report, Implications of Slow or Floating CMOS Inputs, (SCBA004).
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT14 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | D | DGV | DB | N | NS | PW | RGY | BQA | ^ |
| ^ | ^ | 14 pins | < | < | < | < | < | < | < | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 138.7 | 113.1 | 61.1 | 98.6 | 147.7 | 63.7 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.8 | 60.6 | 65.6 | 48.0 | 54.1 | 77.4 | 77.6 | 90.9 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 81 | 71.8 | 60.4 | 41.0 | 57.4 | 90.9 | 39.7 | 56.8 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 37 | 10.6 | 25.5 | 32.4 | 19.6 | 27.2 | 5.7 | 9.9 | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 80.6 | 71.1 | 59.9 | 40.9 | 57.0 | 90.2 | 39.9 | 56.7 | ^ |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | N/A | 19.9 | 33.4 | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, (SPRA953).
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT14 | < | SN74AHCT14 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 4.5 V | 0.9 | | 1.9 | 0.9 | 1.9 | 0.9 | 1.9 | V |
| ^ | ^ | | 5.5 V | 1 | | 2.1 | 1 | 2.1 | 1 | 2.1 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 4.5 V | 0.5 | | 1.5 | 0.5 | 1.5 | 0.5 | 1.5 | V |
| ^ | ^ | | 5.5 V | 0.6 | | 1.7 | 0.6 | 1.7 | 0.6 | 1.7 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 4.5 V | 0.4 | | 1.4 | 0.4 | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.4 | | 1.5 | 0.4 | 1.5 | 0.4 | 1.5 | ^ |
| V~OH~ | | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #2]]

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74AHCT14 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.9 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.7 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.1 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.5 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHCT14 | < | SN74AHCT14 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4 [1] | 7 [1] | 1 [1] | 8 [1] | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4 [1] | 7 [1] | 1 [1] | 8 [1] | 1 | 8 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.5 | 8 | 1 | 9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 8 | 1 | 9 | 1 | 9 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[6::table #3]]

## Typical characteristics

One common misconception is that the current consumption will be less
when switching a slow signal into a Schmitt trigger. This is partly true
because the Schmitt trigger prevents oscillation which can draw a lot of
current; however, you will see higher I~CC~ current due to the amount of
time the input is not at the rail. This is Delta I~CC~. Delta I~CC~ is
where the inputs are not at the rails and upper or lower drive
transistors are partially on. Figure 6-1 shows I~CC~ across the input
voltage sweep. [[7::1]]

Conversion note: Figure 6-1 (supply current vs input voltage, p.7) is a picture that plots I~CC~ in mA against V~I~ in V as a hysteresis loop with two peaks, which is not digitized here.

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[8::1]]

Conversion note: in Figure 7-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[8::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[8::Figure 7-1]]

Conversion note: the source figure (Figure 7-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms with their note B, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[10::1]]

Schmitt triggers should be used anytime you need to translate a sign
wave into a square wave, or used where a slow or noisy input needs to be
sped up or cleaned up as in the switch de-bouncer circuit. [[10::2]]

Source error: "sign wave" is the source's spelling of "sine wave" (Application Information, p.10); kept as printed.

Conversion note: Figure 9-1 (switch de-bouncer using Schmitt trigger inverter) shows a 10 kΩ, 1/6 W resistor from 5 V to the input of an SN74AHCT14 inverter, and a switch and a 10 nf capacitor from that input to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads; therefore, routing and load conditions should be
considered to prevent ringing. [[10::5]]

### Detailed design procedure

1. Recommended input conditions: [[10::6]]
   - Specified High and low levels: See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table. [[10::6]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~ [[10::6]]
2. Recommended output conditions: [[10::6]]
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part [[10::6]]
   - Outputs should not be pulled above V~CC~ [[10::6]]

Source conflict: the design procedure (p.10) refers to V~IH~ and V~IL~ in the Recommended Operating Conditions table, and that table (p.4) has no V~IH~ or V~IL~ row.

Conversion note: Figure 9-2 (typical application curves, p.11) is a picture of three IN and OUT waveform pairs, a sine wave, a noisy signal and a signal with slow edges, each giving a square wave at OUT.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[11::1]]

Each VCC pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μf is recommended.
If there are multiple VCC pins, 0.01 μf or 0.022 μf is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μf and 1 μf are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[11::2]]

### Layout guidelines

When using multiple bit logic devices inputs should not ever float. [[11::3]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Figure 9-3
shows the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or VCC; whichever makes more
sense or is more convenient. It is generally acceptable to float outputs
unless the part is a transceiver. If the transceiver has an output
enable pin, it will disable the outputs section of the part when
asserted. This will not disable the input section of the IO’s so they
cannot float when disabled. [[11::4]]

Conversion note: the layout diagram (Figure 9-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=CDIP-14
```

## Ordering information

```ordering
5962-9680101QCA | Tube of 25 | -55 to 125 | 5962-9680101QCA SNJ54AHCT14J
```
[[14::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering line is from the package
  option addendum (pp.14–17). The support page (13), tape and reel and
  mechanical pages (18–42) are left out; the generated Package section
  replaces them.
- The PDF filed as sn54ahct14-sp is the SN54AHCT14, SN74AHCT14 data
  sheet; it names the SN54AHCT14-SP only among the other qualified
  versions of the package option addendum (p.16).
- Dropped: applications list, device information table, the simplified
  schematic of page 1 (the same as the functional block diagram,
  described in text), the table of contents and revision history of
  page 2, the vendor's pin drawings; the application figures and the
  layout diagram are described in text.
- The catalog carries only the 5962-9680101QCA of this PDF's parts under
  this datasheet; the other orderables of the package option addendum
  (SN74AHCT14 parts, SNJ54AHCT14 parts and the other 5962-9680101
  numbers) are not claimed here. The FK (LCCC) drawing (Figure 5-3) and
  its NC pins are left out with them.
- The electrical characteristics table of the source names only the
  V~T+~, V~T–~ and ΔV~T~ rows, so the other Parameter cells are empty.
- Figures 6-1 (p.7) and 9-2 (p.11) are pictures; they are described in
  text.
- From images: overbars of Y = !{A} (pp.1 and 9).
- Source issues:
  - Source error: "sign wave" for "sine wave" (Application information),
    kept as printed.
  - Source conflict: the design procedure (p.10) refers to V~IH~ and
    V~IL~ rows that the Recommended Operating Conditions table (p.4)
    does not have.
