---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup2g08",
  "title": "SN74AUP2G08 low-power dual 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74aup2g08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES681F",
    "date": "2025-05",
    "pages": 38
  },
  "devices": ["gate:2G08", "gate:2G08_RSE"],
  "parts": ["SN74AUP2G08DCUR", "SN74AUP2G08DCUR1G4", "SN74AUP2G08DQER", "SN74AUP2G08YFPR", "SN74AUP2G08YZPR", "SN74AUP2G08RSER"],
  "packages": {"p8": "DCU, DQE, RSE", "dsbga": "YFP, YZP"},
  "converted": "2026-11-08"
}
---

# SN74AUP2G08 low-power dual 2-input positive-AND gate

## Description

This dual 2-input positive-AND gate is designed for 0.8-V to 3.6-V V~CC~
operation and performs the Boolean function Y = A ● B in positive logic.
The AUP family ensures a very low static- and dynamic-power consumption
across the entire V~CC~ range of 0.8 V to 3.6 V. Inputs have
Schmitt-trigger action; inputs and outputs are 3.6-V tolerant.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when V~CC~ = 0 V,
preventing damaging current backflow through the device when it is
powered down.

## Pinout

DCU and DQE share one pin order; the RSE (UQFN) package has its own
(1Y on pin 1, 1A on pin 7). YFP and YZP share one ball map.

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A: Channel 1 logic input A
1B: Channel 1 logic input B
1Y: Channel 1 output
2A: Channel 2 logic input A
2B: Channel 2 logic input B
2Y: Channel 2 output
GND: Ground
VCC: Power supply
```

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | L |
| H | L | L |
| H | H | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute maximum ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If briefly operating outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not sustain damage, but it may not be fully functional. Operating the device in this manner may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002 [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | μA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | -1.9 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | -4 | mA |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | μA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | 4 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs.
```

## Thermal information

```table id=thermal caption="Thermal resistance characteristics [1]"
| Symbol | Parameter | DCU (VSSOP), 8 pins | DQE (X2SON), 8 pins | RSE (UQFN), 8 pins | YFP (DSBGA), 8 pins [2] | YZP (DSBGA), 8 pins | Unit |
|---|---|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 227 | 261 | 253 | 98.8 | 102 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC package thermal metrics application report.
[2] Source error: the source heads this column "YPF (DSBGA)"; the package information table and the pinout figure name the package YFP.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -20 μA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = -1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | V |
| ^ | ^ | I~OH~ = -1.7 mA | 1.4 V | 1.11 | | | 1.03 | | V |
| ^ | ^ | I~OH~ = -1.9 mA | 1.65 V | 1.32 | | | 1.3 | | V |
| ^ | ^ | I~OH~ = -2.3 mA | 2.3 V | 2.05 | | | 1.97 | | V |
| ^ | ^ | I~OH~ = -3.1 mA | 2.3 V | 1.9 | | | 1.85 | | V |
| ^ | ^ | I~OH~ = -2.7 mA | 3 V | 2.72 | | | 2.67 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.6 | | | 2.55 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 20 μA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | V |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | V |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | V |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | V |
| ^ | ^ | I~OL~ = 3.1 mA | 2.3 V | | | 0.44 | | 0.45 | V |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.44 | | 0.45 | V |
| I~I~ | A or B input | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | μA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 1.3 | μA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 2 | μA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 1.7 | μA |
| ΔI~CC~ | | V~I~ = V~CC~ - 0.6 V [1], I~O~ = 0 | 3.3 V | | | 40 | | 50 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | | ^ | 3.6 V | | 1.5 | | | | pF |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | pF |

[1] Source error: the source prints a broken note marker ("#none#") here and gives no note text; kept out.
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 19.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.8 | 18.8 | 2.1 | 20.9 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.4 | 11.8 | 0.9 | 12.7 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.3 | 9 | 0.5 | 9.5 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 5.9 | 0.5 | 6.4 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 5.2 | 0.5 | 5.7 | ns |
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 23.1 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.9 | 21.1 | 1 | 22.1 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.3 | 13.2 | 0.5 | 13.7 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 10.1 | 0.5 | 10.6 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 7.4 | 0.5 | 7.9 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 5.5 | 0.5 | 6 | ns |
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 24.7 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.8 | 21.7 | 3.1 | 24.8 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 4.6 [1] | 14 | 1.8 | 15.8 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.5 | 10.6 | 1.1 | 11.7 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4 | 7 | 0.5 | 7.5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.3 | 5.9 | 0.5 | 6.4 | ns |

[1] Unconfirmed: this typical value is lower than the typical at 1.8 V in the same table and than the 10 pF typical at 1.5 V (6.3 ns).
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From | To | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 31.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 12.6 | 26.3 | 4.4 | 29 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9 | 16.6 | 2.9 | 20 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.3 | 12.9 | 2 | 15.7 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.4 | 8.8 | 1.3 | 11.4 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.5 | 7 | 1 | 9.5 | ns |
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 4 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4 | pF |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4 | pF |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4 | pF |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 4.1 | pF |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.3 | pF |
```

## Typical characteristics

```curve id=tpd-vcc caption="t~PD~ vs V~CC~, 15 pF load"
{"x": {"label": "VCC", "unit": "V", "range": [0, 4]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 30]},
 "series": [
  {"label": "TPD", "points": [[0.8, 24], [1.2, 10], [1.5, 7.2], [1.8, 5.8], [2.5, 4.3], [3.3, 3.4]]}
 ]}
```

```curve id=tpd-temp caption="t~PD~ vs temperature, V~CC~ = 1.8 V, 15 pF load"
{"x": {"label": "Temperature", "unit": "°C", "range": [-50, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD", "points": [[-40, 5.2], [0, 5.6], [25, 5.8], [85, 6.0], [125, 6.6]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. V~I~ = V~CC~ and V~M~ = V~CC~/2 at every V~CC~ (0.8 V to
3.3 V ± 0.3 V).

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```

## Application information

### Inputs and outputs

Balanced CMOS push-pull outputs: the device can sink and source similar
currents. The drive capability of this device may create fast edges into
light loads, so routing and load conditions should be considered to
prevent ringing. The outputs are capable of driving larger currents than
the device can sustain without being damaged; the electrical and
thermal limits defined in the Absolute Maximum Ratings must be followed
at all times. Unused push-pull CMOS outputs must be left disconnected.

The source describes the inputs both as Schmitt-trigger inputs and as
standard CMOS inputs (see [Conversion notes](#conversion-notes)).
Schmitt-trigger inputs: high impedance, typically modeled as a resistor
in parallel with the input capacitance; the hysteresis makes this
device extremely tolerant to slow or noisy inputs. It is still
recommended to properly terminate unused inputs; driving the inputs with
slow transitioning signals will increase dynamic current consumption.
Standard CMOS inputs require that input signals transition between valid
logic states quickly, as defined by the input transition rate in the
[recommended operating conditions](#recommended-operating-conditions);
failing to meet this specification will result in excessive power
consumption and could cause oscillations. Do not leave standard CMOS
inputs floating at any time during operation. Unused inputs must be
terminated at V~CC~ or GND. If a system will not be actively driving an
input at all times, then a pull-up or pull-down resistor can be added to
provide a valid input voltage during these times; a 10kΩ resistor is
recommended.

Partial power down (I~off~): circuitry disables all outputs when the
supply pin is held at 0 V; the outputs will neither source nor sink
current, regardless of the input voltages applied.

Clamp diodes (source Figure 7-1): the inputs and outputs have negative
clamping diodes only, from GND to each input and each output.
Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device.

### Typical application

Three 2-input AND gates are combined to produce a 4-input AND gate
function that directly controls the RESET pin of a motor controller. The
controller requires four input signals (!{OC}, PG, ON/!{OFF}, !{OT}) to
all be HIGH before being enabled, and should be disabled in the event
that any one signal goes LOW; the 4-input AND combines the four
individual reset signals into a single active-low reset signal.

```wavedrom id=app-timing caption="Application timing diagram"
{"signal": [
  {"name": "OC#", "wave": "10101010"},
  {"name": "PG", "wave": "1.0.1..."},
  {"name": "ON/OFF#", "wave": "1..0.1.."},
  {"name": "OT#", "wave": "1...0..."},
  {"name": "RESET#", "wave": "10......"}
]}
```

Design requirements:

- The positive supply must be capable of sourcing current equal to the
  total current sourced by all outputs plus the maximum static supply
  current I~CC~ and any transient current required for switching; the
  ground must sink the matching total. Ensure the maximum total current
  through V~CC~ or GND in the Absolute Maximum Ratings is not exceeded.
- The device can drive a load with a total capacitance less than or
  equal to 50pF while still meeting all of the data sheet
  specifications. Larger capacitive loads can be applied; however, it
  is not recommended to exceed 50pF.
- Resistive load: R~L~ ≥ V~O~ / I~O~, with V~O~ and I~O~ from V~OH~ and
  V~OL~ in the Electrical Characteristics; in the HIGH state, V~O~ is
  the difference between the output voltage and V~CC~.
- Input signals must cross V~IL~(max) to be considered a logic LOW, and
  V~IH~(min) to be considered a logic HIGH. Do not exceed the maximum
  input voltage range found in the Absolute Maximum Ratings.
- Push-pull outputs that could be in opposite states, even for a very
  short time period, should never be connected directly together. Two
  channels with the same input signals can be connected in parallel for
  additional output drive strength. Unused outputs can be left
  floating. Do not connect outputs directly to V~CC~ or ground.
- The maximum junction temperature, T~J~(max), is an additional
  limitation to prevent damage to the device.

Design procedure: add a decoupling capacitor from V~CC~ to GND, placed
physically close to the device and electrically close to both the
V~CC~ and GND pins. Ensure the capacitive load at the output is
≤ 50pF. This is not a hard limit; by design, however, it will optimize
performance. Ensure the resistive load at the
output is larger than (V~CC~ / I~O(max)~) Ω.

### Power and layout

The power supply can be any voltage between the minimum and maximum
supply voltage rating in the recommended operating conditions. Each
V~CC~ terminal should have a good bypass capacitor to prevent power
disturbance; a 0.1μF capacitor is recommended. The 0.1μF and 1μF
capacitors are commonly used in parallel. The bypass capacitor should
be installed as close to the power terminal as possible.

Inputs must never be left floating. All unused inputs of digital logic
devices must be connected to a logic high or logic low voltage, as
defined by the input voltage specifications, to prevent them from
floating; generally, the inputs are tied to GND or V~CC~.

## Package

```package footprint=VSSOP-8
```

```package footprint=X2SON-8
```

```package footprint=UQFN-8
```

```package footprint=DSBGA-8-YFP
```

```package footprint=DSBGA-8-YZP
```

## Ordering information

```ordering
SN74AUP2G08DCUR | Reel of 3000 | -40 to 125 | H08R
SN74AUP2G08DCUR1G4 | Reel of 3000 | -40 to 125 | H08R
SN74AUP2G08DQER | Reel of 5000 | -40 to 125 | PR
SN74AUP2G08RSER | Reel of 5000 | -40 to 125 | PR
SN74AUP2G08YFPR | Reel of 3000 | -40 to 125 | HEN
SN74AUP2G08YZPR | Reel of 3000 | -40 to 125 | HEN
```

## Conversion notes

- Source pages 1–17 converted; device and documentation support,
  revision history, package option addendum, tape and reel and
  mechanical pages (18–38) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: Figure 6-2 (enable and disable times load circuit and
  waveforms) and the pulse-duration and setup/hold waveforms of
  Figure 6-1, which the source notes are not applicable to all devices;
  Figure 8-3 (example layout, which shows a 5-pin single-gate part);
  the CMOS power and thermal calculation references.
- The application block diagram (Figure 8-1) is described in text; the
  application timing diagram is redrawn from Figure 8-2.
- Curves digitized from Figures 5-1 and 5-2 to about 2 % of full scale.
- From images: curve points 24, 10, 7.2, 5.8, 4.3, 3.4, 5.2, 5.6, 5.8,
  6.0, 6.6, 125 (Figures 5-1 and 5-2, p.8); the overbars on !{OC},
  ON/!{OFF}, !{OT} and !{RESET} (Figure 8-2, p.16).
- Source issues:
  - Source error: thermal table column headed "YPF (DSBGA)"; the
    package is YFP (p.5).
  - Source error: ΔI~CC~ test condition carries a broken note marker
    "#none#" with no note text (p.6); kept out.
  - Unconfirmed: t~pd~ typical 4.6 ns at C~L~ = 15 pF, V~CC~ = 1.5 V
    (p.7), lower than the 1.8 V typical and the 10 pF typical; Figure
    5-1 shows about 7 ns at 1.5 V.
  - Source conflict: §7.3.2 and §8.2.1.2 describe Schmitt-trigger inputs
    with "no input signal transition rate requirements", while §7.3.4
    describes standard CMOS inputs, §8.2.1.2 says the inputs "require
    fast input transitions", and the recommended operating conditions
    give Δt/Δv ≤ 200 ns/V (p.5, 11–12, 15).
  - Source conflict: §7.3.2 and §8.2.1.2 refer to ΔV~T~ in the
    Electrical Characteristics table and to a supply-current plot in
    Typical Characteristics; neither is in the PDF (p.6, 8, 11, 15).
