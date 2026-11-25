---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup2g14",
  "title": "SN74AUP2G14 low-power dual Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup2g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES749D",
    "date": "2025-06",
    "pages": 36
  },
  "devices": ["gate:2G14"],
  "parts": ["SN74AUP2G14DCKR", "SN74AUP2G14DRYR", "SN74AUP2G14DSF2", "SN74AUP2G14DSFR", "SN74AUP2G14YFPR"],
  "packages": {"p6": "DCK, DRY, DSF", "dsbga": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP2G14 low-power dual Schmitt-trigger inverter

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static- and dynamic-power consumption across the entire V~CC~
range of 0.8V to 3.6V, resulting in increased battery life. This
product also maintains excellent signal integrity. [[1::3]]

The SN74AUP2G14 contains two inverters and performs the Boolean
function Y = !{A}. The device functions as two independent inverters,
but because of Schmitt action, it may have different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals. [[1::4]]

NanoStar™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::5]]

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down. [[1::6]]

Optimized for 3.3V operation. 3.6V I/O tolerant to support mixed-mode
signal Operation. Suitable for point-to-point applications. Latch-up
performance exceeds 100mA Per JESD 78, Class II. [[1::1]]

## Pinout

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A: Gate 1 logic signal
GND: Ground
2A: Gate 2 logic signal
1Y: Gate 1 inverted signal
VCC: Supply/Power Pin
2Y: Gate 2 inverted signal
```
[[3::Table 4-1]]

## Functional description

The SN74AUP2G14 contains two inverters and performs the Boolean
function Y = !{A}. The device functions as two independent inverters,
but because of Schmitt action, it may have different input threshold
levels for positivegoing (V~T+~) and negative-going (V~T–~) signals. [[1::4]]

Source error: the overview (p.13) prints the function as Y = A, without the overbar that p.1 and p.14 print, and "positivegoing" as one word (kept as printed).

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down. [[1::6]]

Conversion note: the logic diagram (Figure 7-1) shows two Schmitt-trigger inverters, 1A on pin 1 to 1Y on pin 6 and 2A on pin 3 to 2Y on pin 4.

### Feature description

As the inputs are 5.5V tolerant, the device can be used as a down
translator. When the input voltage exceeds V~T+~ (Max), the output will
follow V~CC~, performing down-translation if the input voltage exceeds
V~CC~. [[13::3]]

Source conflict: "the inputs are 5.5V tolerant" (p.13) vs the absolute maximum input voltage of 4.6 V and the recommended maximum of 3.6 V (p.4–5); both are kept as printed.

### Device functional modes

Table 7-1 lists the functional modes of SN74AUP2G14. [[13::4]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[13::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 4.6 | V |
| V~I~ | Input voltage [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4V | | -1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65V | | -1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3V | | -3.1 | ^ |
| ^ | ^ | V~CC~ = 3V | | -4 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4V | | 1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65V | | 1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3V | | 3.1 | ^ |
| ^ | ^ | V~CC~ = 3V | | 4 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AUP2G14"
| Symbol | Thermal metric [1] | DRY (SON) | DSF (SON) | YFP (DSBGA) | DCK (SC70) | Unit |
|---|---|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 234 | 300 | 132 | 252 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 0.8V | 0.3 | | 0.6 | 0.3 | 0.6 | V |
| ^ | ^ | | 1.1V | 0.53 | | 0.9 | 0.53 | 0.9 | ^ |
| ^ | ^ | | 1.4V | 0.74 | | 1.11 | 0.74 | 1.11 | ^ |
| ^ | ^ | | 1.65V | 0.91 | | 1.29 | 0.91 | 1.29 | ^ |
| ^ | ^ | | 2.3V | 1.37 | | 1.77 | 1.37 | 1.77 | ^ |
| ^ | ^ | | 3V | 1.88 | | 2.29 | 1.88 | 2.29 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 0.8V | 0.1 | | 0.6 | 0.1 | 0.6 | V |
| ^ | ^ | | 1.1V | 0.26 | | 0.65 | 0.26 | 0.65 | ^ |
| ^ | ^ | | 1.4V | 0.39 | | 0.75 | 0.39 | 0.75 | ^ |
| ^ | ^ | | 1.65V | 0.47 | | 0.84 | 0.47 | 0.84 | ^ |
| ^ | ^ | | 2.3V | 0.69 | | 1.04 | 0.69 | 1.04 | ^ |
| ^ | ^ | | 3V | 0.88 | | 1.24 | 0.88 | 1.24 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 0.8V | 0.07 | | 0.5 | 0.07 | 0.5 | V |
| ^ | ^ | | 1.1V | 0.08 | | 0.46 | 0.08 | 0.46 | ^ |
| ^ | ^ | | 1.4V | 0.18 | | 0.56 | 0.18 | 0.56 | ^ |
| ^ | ^ | | 1.65V | 0.27 | | 0.66 | 0.27 | 0.66 | ^ |
| ^ | ^ | | 2.3V | 0.53 | | 0.92 | 0.53 | 0.92 | ^ |
| ^ | ^ | | 3V | 0.79 | | 1.31 | 0.79 | 1.31 | ^ |
| V~OH~ | | I~OH~ = –20µA | 0.8V to 3.6V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = –1.1mA | 1.1V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | ^ |
| ^ | ^ | I~OH~ = –1.7mA | 1.4V | 1.11 | | | 1.03 | | ^ |
| ^ | ^ | I~OH~ = –1.9mA | 1.65V | 1.32 | | | 1.3 | | ^ |
| ^ | ^ | I~OH~ = –2.3mA | 2.3V | 2.05 | | | 1.97 | | ^ |
| ^ | ^ | I~OH~ = –3.1mA | ^ | 1.9 | | | 1.85 | | ^ |
| ^ | ^ | I~OH~ = –2.7mA | 3V | 2.72 | | | 2.67 | | ^ |
| ^ | ^ | I~OH~ = –4mA | ^ | 2.6 | | | 2.55 | | ^ |
| V~OL~ | | I~OL~ = 20µA | 0.8 V to 3.6V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1mA | 1.1V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | ^ |
| ^ | ^ | I~OL~ = 1.7mA | 1.4V | | | 0.31 | | 0.37 | ^ |
| ^ | ^ | I~OL~ = 1.9mA | 1.65V | | | 0.31 | | 0.35 | ^ |
| ^ | ^ | I~OL~ = 2.3mA | 2.3V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1mA | ^ | | | 0.44 | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7mA | 3V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4mA | ^ | | | 0.44 | | 0.45 | ^ |
| I~I~ | A or B input [1] | V~I~ = GND to 3.6V | 0V to 3.6V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0V to 3.6V | 0V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0V to 3.6V | 0V to 0.2V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6V), I~O~ = 0 | 0.8V to 3.6V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6V [2], I~O~ = 0 | 3.3V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 0V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6V | | 1.5 | | | | ^ |
| C~o~ | | V~O~ = GND | 0V | | 3 | | | | pF |

[1] Source error: the source prints "A or B input", kept as printed; each inverter has one input, A (pin functions, p.3).
[2] One input at V~CC~ – 0.6V, other input at V~CC~ or GND.
```
[[6::table #1]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 0.8V | 4 | pF |
| ^ | ^ | ^ | 1.2V ± 0.1V | 4 | ^ |
| ^ | ^ | ^ | 1.5V ± 0.1V | 4 | ^ |
| ^ | ^ | ^ | 1.8V ± 0.15V | 4 | ^ |
| ^ | ^ | ^ | 2.5V ± 0.2V | 4.1 | ^ |
| ^ | ^ | ^ | 3.3V ± 0.3V | 4.3 | ^ |
```
[[8::table #1]]

## Switching characteristics

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5pF" conditions="Over recommended operating free-air temperature range, C~L~ = 5pF (unless otherwise noted) (see Figure 6-3 and Figure 6-4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8V | | 18 | | | | ns |
| ^ | ^ | ^ | 1.2V ± 0.1V | 2.6 | 7.3 | 12.8 | 2.1 | 15.6 | ^ |
| ^ | ^ | ^ | 1.5V ± 0.1V | 1.4 | 5.2 | 8.7 | 0.9 | 10.3 | ^ |
| ^ | ^ | ^ | 1.8V ± 0.15V | 1 | 4.2 | 6.6 | 0.5 | 8.2 | ^ |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1 | 3 | 4.4 | 0.5 | 5.5 | ^ |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 2.4 | 3.5 | 0.5 | 4.3 | ^ |
```
[[7::table #1]]

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10pF (unless otherwise noted) (see Figure 6-3 and Figure 6-4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8V | | 18.4 | | | | ns |
| ^ | ^ | ^ | 1.2V ± 0.1V [1] | 4.6 | 7.9 | 13.4 | 1.3 | 16.7 | ^ |
| ^ | ^ | ^ | 1.5V ± 0.1V | 4 | 6 | 9.6 | 2.2 | 11.8 | ^ |
| ^ | ^ | ^ | 1.8V ± 0.15V | 3.6 | 5 | 7.9 | 2.4 | 9.5 | ^ |
| ^ | ^ | ^ | 2.5V ± 0.2V | 3.2 | 4 | 5.5 | 2.3 | 6.8 | ^ |
| ^ | ^ | ^ | 3.3V ± 0.3V | 2.9 | 3.5 | 4.6 | 2.1 | 5.6 | ^ |

[1] Unconfirmed: the source prints a –40°C to 85°C minimum of 1.3 ns at 1.2V, below the minimums it prints on the rows under it.
```
[[7::table #2]]

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15pF (unless otherwise noted) (see Figure 6-3 and Figure 6-4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8V | | 24 | | | | ns |
| ^ | ^ | ^ | 1.2V ± 0.1V | 3.6 | 9.9 | 16.3 | 3.1 | 19.9 | ^ |
| ^ | ^ | ^ | 1.5V ± 0.1V | 2.3 | 7.2 | 11.1 | 1.8 | 13.2 | ^ |
| ^ | ^ | ^ | 1.8V ± 0.15V | 1.6 | 5.8 | 8.7 | 1.1 | 10.6 | ^ |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1 | 4.3 | 5.9 | 0.5 | 7.3 | ^ |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1 | 3.4 | 4.8 | 0.5 | 5.9 | ^ |
```
[[7::table #3]]

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30pF (unless otherwise noted) (see Figure 6-3 and Figure 6-4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8V | | 32.8 | | | | ns |
| ^ | ^ | ^ | 1.2V ± 0.1V | 4.9 | 13.1 | 20.9 | 4.4 | 25.5 | ^ |
| ^ | ^ | ^ | 1.5V ± 0.1V | 3.4 | 9.5 | 14.2 | 2.9 | 16.9 | ^ |
| ^ | ^ | ^ | 1.8V ± 0.15V | 2.5 | 7.7 | 11 | 2 | 13.5 | ^ |
| ^ | ^ | ^ | 2.5V ± 0.2V | 1.8 | 5.7 | 7.6 | 1.3 | 9.4 | ^ |
| ^ | ^ | ^ | 3.3V ± 0.3V | 1.5 | 4.7 | 6.2 | 1 | 7.5 | ^ |
```
[[8::table #2]]

## Typical characteristics

Conversion note: Figure 5-1 (AUP – The Lowest-Power Family) compares the static- and dynamic-power consumption of AUP with 3.3-V logic as bars, and Figure 5-2 (Excellent Signal Integrity) shows input and output voltage against time, 0 to 45 ns, at 25 MHz.

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, for propagation delays t~r~/t~f~ = 3 ns, for setup and
hold times and pulse width t~r~/t~f~ = 1.2 ns. The outputs are measured
one at a time, with one transition per measurement. t~PLH~ and t~PHL~
are the same as t~pd~. All parameters and waveforms are not applicable
to all devices. [[11::2]]

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | V~CC~ = 1.5 V ± 0.1 V | V~CC~ = 1.8 V ± 0.15 V | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|---|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~M~ | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 |
| V~I~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ |
```
[[1::table #1]]

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```
[[11::Figure 6-3]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```
[[11::Figure 6-3]]

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[14::1]]

The SN74AUP2G14 contains two inverters and performs the Boolean
function Y = !{A}. The device functions as two independent inverters,
but because of Schmitt action, it may have different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals. [[1::4]]

Conversion note: the typical application schematic (Figure 8-1) is not drawn.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads so routing and load conditions should be
considered to prevent ringing. [[14::3]]

### Detailed design procedure

1. Recommended Input Conditions [[14::4]]
   - Rise time and fall time specs. See (Δt/ΔV) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[14::4]]
   - Specified high and low levels. See (V~IH~ and V~IL~) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[14::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table at any valid V~CC~. [[14::4]]
2. Recommend Output Conditions [[14::4]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed (continuous current through V~CC~ or GND) total current
     for the part. These limits are located in the
     [Absolute Maximum Ratings](#absolute-maximum-ratings) table. [[14::4]]
   - Outputs should not be pulled above V~CC~. [[14::4]]

Source conflict: the Recommended Operating Conditions table (p.5) has no Δt/ΔV, V~IH~ or V~IL~ row; the input thresholds are V~T+~ and V~T–~ in the Electrical Characteristics (p.6).

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the table. Each V~CC~ pin should have
a good bypass capacitor to prevent power disturbance. For devices with
a single supply, TI recommends a 0.1μF capacitor. If there are multiple
V~CC~ pins, then TI recommends a 0.01μF or 0.022μF capacitor for each
power pin. It is ok to parallel multiple bypass capacitors to reject
different frequencies of noise. 0.1μF and 1μF capacitors are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[15::1]]

### Layout guidelines

- Bypass capacitor placement [[16::2]]
  - Place near the positive supply terminal of the device [[15::2]]
  - Provide an electrically short ground return path [[15::2]]
  - Use wide traces to minimize impedance [[15::2]]
  - Keep the device, capacitors, and traces on the same side of the
    board whenever possible [[15::2]]
- Signal trace geometry [[15::2]]
  - 8mil to 12mil trace width [[15::2]]
  - Lengths less than 12cm to minimize transmission line effects [[15::2]]
  - Avoid 90° corners for signal traces [[15::2]]
  - Use an unbroken ground plane below signal traces [[15::2]]
  - Flood fill areas around signal traces with ground [[15::2]]
  - Parallel traces must be separated by at least 3x dielectric
    thickness [[15::2]]
  - For traces longer than 12cm [[15::2]]
    - Use impedance controlled traces [[15::2]]
    - Source-terminate using a series damping resistor near the output [[15::2]]
    - Avoid branches; buffer each signal that must branch separately [[15::2]]

Conversion note: the layout examples (Figures 8-2 to 8-6: trace corners, bypass capacitor placement for three package styles, and damping resistor placement) are not drawn.

## Package

```package footprint=SC70-6
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-6-YFP
```

## Ordering information

```ordering
SN74AUP2G14DCKR | Reel of 3000 | -40 to 125 | H65, H6F
SN74AUP2G14DRYR | Reel of 5000 | -40 to 85 | H6
SN74AUP2G14DSF2 | Reel of 5000 | -40 to 85 | H6
SN74AUP2G14DSFR | Reel of 5000 | -40 to 85 | H6
SN74AUP2G14YFPR | Reel of 3000 | -40 to 85 | HFN
```
[[19::table #1]]

## Conversion notes

- Source pages 1–8, 11 and 13–16 converted; p.2 holds only the table of
  contents, which is dropped. Pages 9–10 (Figures 6-1 and 6-2, which
  print Figures 6-3 and 6-4 a first time) and page 12 (Figure 6-4, the
  load circuit and voltage waveforms for enable and disable times, which
  this gate does not have) are dropped. The ordering lines are from the
  package option addendum (p.19–20). The support, revision history and
  mechanical information pages (17–18), tape and reel and mechanical
  pages (21–36) are left out; the generated Package section replaces
  them.
- Dropped: features and applications lists (some features are kept as
  sentences in the Description), the device information table, the
  logic diagram, Figures 5-1 and 5-2, the typical application schematic
  and the layout examples (described or named in text), and the
  pulse-duration and setup-and-hold waveforms of Figure 6-3 with its
  note B on output-control waveforms.
- Section 5.3, named in the detailed design procedure, is the source's
  Recommended Operating Conditions; Section 5.1 is its Absolute Maximum
  Ratings.
- The thermal table's package columns are headed "PINS" without a pin
  count in the source.
- The addendum also lists SN74AUP2G14DCKRG4, SN74AUP2G14DRYRG4 and
  SN74AUP2G14DSFRG4, which the catalog doesn't carry.
- From images: the overbar of Y = !{A} (p.1 and Application
  Information, p.14).
- Source issues:
  - Source error: the overview (p.13) prints Y = A without the overbar,
    and "positivegoing" as one word.
  - Source error: the I~I~ row (p.6) prints "A or B input"; each
    inverter has one input, A (kept as printed).
  - Source conflict: "the inputs are 5.5V tolerant" (p.13) vs V~I~
    maximums of 4.6 V and 3.6 V (p.4–5).
  - Source conflict: Δt/ΔV, V~IH~ and V~IL~ referenced in the
    Recommended Operating Conditions but not in that table (Detailed
    design procedure note).
  - Source conflict: the addendum (p.19) gives SN74AUP2G14DCKR –40 to
    125 °C and the revision history says the operating temperature was
    updated to 125°C, but the recommended operating conditions and the
    tables give –40°C to 85°C only.
  - Unconfirmed: the C~L~ = 10pF switching table (p.7) prints a –40°C
    to 85°C minimum of 1.3 ns at 1.2V, below the 2.2 ns it prints at
    1.5V.
  - Unconfirmed: the power supply recommendations say "located in the
    table" without naming the table (p.15).
