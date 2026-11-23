---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1g06",
  "title": "SN74AUP1G06 low-power single inverter with open-drain outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup1g06.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES590E",
    "date": "2018-03",
    "pages": 52
  },
  "devices": ["gate:1G06", "gate:1G06_SON"],
  "parts": ["SN74AUP1G06DBVR", "SN74AUP1G06DBVT", "SN74AUP1G06DCKR", "SN74AUP1G06DCKT", "SN74AUP1G06DPWR", "SN74AUP1G06DRLR", "SN74AUP1G06YFPR", "SN74AUP1G06DRYR", "SN74AUP1G06DSF2", "SN74AUP1G06DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga4": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP1G06 low-power single inverter with open-drain outputs

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static and dynamic power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in an increased battery life. This
product also maintains excellent signal integrity. [[1::4]]

## Pinout

Conversion note: the source draws the YFP package in bottom view, and the drawing here is generated from the catalog.

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga4
```

## Pin functions

```pins
A: Input
GND: Ground
NC: Not connected
VCC: Positive supply
Y: Output
```
[[3::table #1]]

NC – No internal connection [[3::2]]

## Functional description

The output of this single inverter buffer/driver is open drain, and can
be connected to other open-drain outputs to implement active-low
wired-OR or active-high wired-AND functions. [[10::1]]

NanoStar™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[10::2]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when the device is
powered down. This inhibits current backflow into the device which
prevents damage to the device. [[10::3]]

Conversion note: the logic diagram (Figure 5) shows one inverter with an open-drain output, input A on pin 2 and output Y on pin 4.

### CMOS open-drain outputs

The open-drain output allows the device to sink current to GND but not
to source current from V~CC~. When the output is not actively pulling
the line low, it will go into a high impedance state (3-state). This
allows the device to be used for a wide variety of applications,
including up-translation and down-translation, as the output voltage can
be determined by an external pullup. [[10::4]]

The drive capability of this device creates fast edges into light loads
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to over-current. The electrical and
thermal limits defined the in the Absolute Maximum Ratings must be
followed at all times. [[10::5]]

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modelled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings, and the
maximum input leakage current, given in the Electrical Characteristics,
using ohm's law (R = V ÷ I). [[10::6]]

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in Recommended Operating Conditions to avoid excessive currents
and oscillations. If a slow or noisy input signal is required, a device
with a Schmitt-trigger input should be utilized to condition the input
signal prior to the standard CMOS input. [[10::7]]

### Clamp diodes

The inputs and outputs to this device have negative clamping diodes. [[11::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[11::2]]

Conversion note: Figure 6 (electrical placement of clamping diodes for each input and output) shows one diode from GND to the input and one from GND to the output, carrying -I~IK~ and -I~OK~.

### Partial power down (I~off~)

The inputs and outputs for this device enter a high impedance state when
the supply voltage is 0 V. The maximum leakage into or out of any input
or output pin on the device is specified by I~off~ in the Electrical
Characteristics. [[11::4]]

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Absolute Maximum Ratings. [[11::5]]

### Device functional modes

Table 1 lists the functional modes of the SN74AUP1G06 device. [[11::6]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | Hi-Z |
```
[[11::Table 1]]

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
| T~j~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="See [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | ^ |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | 3.6 | V |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 4 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AUP1G06 | < | < | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT-5X3) 5 pins | DRY (SON) 6 pins | DPW (X2SON) 5 pins | DSF (SON) 6 pins | YFP (DSBGA) 4 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 230.5 | 303.6 | 295.1 | 342.1 | 504.3 | 377.1 | 179.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 172.7 | 203.8 | 131.0 | 233.1 | 234.9 | 187.7 | 2.8 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 62.2 | 100.9 | 143.9 | 206.7 | 370.3 | 236.6 | 58.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 49.3 | 76.1 | 14.7 | 63.4 | 44.5 | 29.0 | 1.1 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 61.6 | 99.3 | 144.4 | 206.7 | 369.7 | 236.3 | 58.6 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | 165.2 | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OL~ | Low-level output voltage | I~OL~ = 20 µA | T~A~ = 25°C | 0.8 V to 3.6 V | | | 0.1 | V |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 1.1 mA | T~A~ = 25°C | 1.1 V | | | 0.3 × V~CC~ | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.3 × V~CC~ | ^ |
| ^ | ^ | I~OL~ = 1.7 mA | T~A~ = 25°C | 1.4 V | | | 0.31 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.37 | ^ |
| ^ | ^ | I~OL~ = 1.9 mA | T~A~ = 25°C | 1.65 V | | | 0.31 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.35 | ^ |
| ^ | ^ | I~OL~ = 2.3 mA | T~A~ = 25°C | 2.3 V | | | 0.31 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1 mA | T~A~ = 25°C | ^ | | | 0.44 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7 mA | T~A~ = 25°C | 3 V | | | 0.31 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4 mA | T~A~ = 25°C | ^ | | | 0.44 | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.45 | ^ |
| I~I~ | Inflection point current [1] | A input: V~I~ = GND to 3.6 V | T~A~ = 25°C | 0 V to 3.6 V | | | 0.1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.5 | ^ |
| I~off~ | Off-state current | V~I~ or V~O~ = 0 V to 3.6 V | T~A~ = 25°C | 0 V | | | 0.2 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.6 | ^ |
| ΔI~off~ | Off-state current change | V~I~ or V~O~ = 0 V to 3.6 V | T~A~ = 25°C | 0 V to 0.2 V | | | 0.2 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.6 | ^ |
| I~CC~ | Supply current | V~I~ = GND or V~CC~ to 3.6 V, I~O~ = 0 | T~A~ = 25°C | 0.8 V to 3.6 V | | | 0.5 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.9 | ^ |
| ΔI~CC~ | Supply current change | V~I~ = V~CC~ – 0.6 V, I~O~ = 0 | T~A~ = 25°C | 3.3 V | | | 40 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 50 | ^ |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND, T~A~ = 25°C | < | 0 V | | 1.5 | | pF |
| ^ | ^ | ^ | ^ | 3.6 V | | 1.7 | | ^ |
| C~o~ | Output capacitance | V~O~ = GND, T~A~ = 25°C | < | 0 V | | 1.7 | | pF |

[1] Unconfirmed: the source names the I~I~ row "Inflection point current"; its condition is the A input at V~I~ = GND to 3.6 V.
```
[[5::table #2]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 1 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | ^ |
```
[[6::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 5 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay time | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 12.4 | | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 2.7 | 12 [1] | 9.9 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2 | | 12.8 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 2.1 | 3.5 | 6.2 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.5 | | 7.6 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 2.1 | 3.1 | 4.7 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.2 | | 5.9 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 1.4 | 2.2 | 3.2 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1 | | 3.9 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 1.3 | 2.2 | 3.3 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 0.8 | | 3.6 | ^ |

[1] Unconfirmed: the source prints a typical of 12 ns at V~CC~ = 1.2 V ± 0.1 V, above the 9.9 ns maximum on the same row.
```
[[6::table #2]]

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay time | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 15.1 | | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 3.6 | 12 [1] | 11.2 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.7 | | 14.1 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 2.9 | 4.3 | 7 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.2 | | 8.6 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 2.7 | 3.9 | 5.4 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.8 | | 6.7 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 2.1 | 2.9 | 3.8 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.4 | | 4.5 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 1.7 | 3 | 4.5 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.2 | | 4.9 | ^ |

[1] Unconfirmed: the source prints a typical of 12 ns at V~CC~ = 1.2 V ± 0.1 V, above the 11.2 ns maximum on the same row.
```
[[6::table #3]]

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay time | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 17.4 | | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 4.9 | 12 | 12.2 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 3.4 | | 15.2 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 3.5 | 5 | 7.7 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.7 | | 9.4 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 3.2 | 4.8 | 6.6 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.2 | | 7.3 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 2.5 | 3.5 | 4.5 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.7 | | 5.1 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 2 | 3.8 | 6 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.5 | | 6.5 | ^ |
```
[[6::table #4]]

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | Propagation delay time | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 25.3 | | ns |
| ^ | ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 7.6 | 12 | 16 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 5.6 | | 19.3 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 5.9 | 7.6 | 10.1 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 4.3 | | 12 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 4.8 | 7.4 | 10.7 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 3.6 | | 11 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 3.7 | 5.4 | 7.1 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.8 | | 7.8 | ^ |
| ^ | ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 3.2 | 6.5 | 10.5 | ^ |
| ^ | ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.5 | | 10.8 | ^ |
```
[[7::table #1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, t~r~/t~f~ = 3 ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices. [[8::1]]

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
[[8::figure #1]]

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```
[[8::figure #2]]

Conversion note: Figure 4 (enable and disable times, p.9) shows a second load circuit with two 5 kΩ resistors, a switch S1 to 2 × V~CC~ for t~PLZ~/t~PZL~ and to GND for t~PHZ~/t~PZH~, the same C~L~, V~M~ and V~I~ rows with V~Δ~ of 0.1 V, 0.1 V, 0.1 V, 0.15 V, 0.15 V and 0.3 V, and the enable and disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[12::1]]

Open-drain devices are very commonly used for voltage level translation.
In this application, the SN74AUP1G06 is used to translate a 1.8-V output
from device A to a 3.3-V input on device B. [[12::2]]

The application schematic shown in Figure 7 includes two generic
devices, labeled as "Device A" and "Device B." [[12::3]]

Conversion note: Figure 7 (application schematic for voltage translation with SN74AUP1G06) shows Device A on supply V~A~ driving input A of the AUP1G06, and output Y going to Device B on supply V~B~ with a pull-up resistor R~PU~ to V~B~, a capacitance C~P~ to ground, and the input current I~I~ and input capacitance C~I~ of Device B.

### Design requirements

This device has a standard CMOS input, so be careful to avoid slow or
floating inputs that might cause oscillation or excessive current.
Please see the Implications of Slow or FLoating CMOS Inputs Application
Report. [[12::5]]

This device has an open-drain output, which means that the output enters
a high-impedance state when a normal CMOS device would drive the output
high. A pull-up resistor must be added to the output for an open-drain
device to have a high output. The selection of this pull-up resistor is
detailed in the next section. [[12::6]]

### Detailed design procedure

1. Recommended Input Conditions [[12::7]]
   - For specified high and low levels, see V~IH~ and V~IL~ in the Electrical Characteristics table. [[12::7]]
   - Inputs are overvoltage tolerant allowing them to go as high as V~I(max)~ in the Absolute Maximum Ratings table at any valid V~CC~. [[12::7]]
2. Recommended Output Conditions [[12::7]]
   - Output voltage must not exceed V~O(max)~ as specified in the Absolute Maximum Ratings table. [[12::7]]
   - Pull-up resistor (R) selection depends on three primary factors: desired output high voltage (V~OH~), which is directly related to total leakage current into the SN74AUP1G240 and the peripheral device's input (I~L~), desired 0 to 90% rising edge time (t~r~), which is directly related to the parasitic line capacitance (C~P~), and the maximum current during low output (I~OL~), which is directly related to the supply value. These three equations govern pull-up resistor selection: [[12::7]]
     - R ≤ ( V~CC~ – V~OH~ ) / I~L~ [[12::7]]
     - R ≤ t~r~ / ( 2.3 * C~P~) [[12::7]]
     - R ≥ V~CC~ / I~OL(max)~ [[12::7]]

Source conflict: the design procedure (p.12) points to the Electrical Characteristics table for V~IH~ and V~IL~; the Electrical Characteristics (p.5) have no such rows, and V~IH~ and V~IL~ are in the Recommended Operating Conditions (p.4).

Source error: the design procedure (p.12) names the SN74AUP1G240 where this datasheet's device is the SN74AUP1G06 (title, p.1); kept as printed.

### Application curve

Conversion note: Figure 8 (simulated up-translation with the SN74AUP1G06, p.13) plots voltage against time over 0 to 200 µs, with Vin a pulse between 0 and 1.8 V and Vout the inverted pulse between 0 and 3.3 V.

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table. [[13::2]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1-µF capacitor is
recommended and if there are multiple V~CC~ pins then a 0.01-µF or
0.022-µF capacitor is recommended for each power pin. It is ok to
parallel multiple bypass caps to reject different frequencies of noise.
0.1-µF and 1-µF capacitors are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results. [[13::3]]

### Layout guidelines

Even low data rate digital signals can contain high-frequency signal
components due to fast edge rates. When a printed-circuit board (PCB)
trace turns a corner at a 90° angle, a reflection can occur. A
reflection occurs primarily because of the change of width of the trace.
At the apex of the turn, the trace width increases to 1.414 times the
width. This increase upsets the transmission-line characteristics,
especially the distributed capacitance and self–inductance of the trace
which results in the reflection. Not all PCB traces can be straight and
therefore some traces must turn corners. Figure 9 shows progressively
better techniques of rounding corners. Only the last example (BEST)
maintains constant trace width and minimizes reflections. [[13::4]]

An example layout is given in Figure 10 for the DPW (X2SON-5) package.
This example layout includes a 0402 (metric) capacitor and uses the
measurements found in the example board layout appended to this end of
this datasheet. A via of diameter 0.1 mm (3.973 mil) is placed directly
in the center of the device. This via can be used to trace out the
center pin connection through another board layer, or it can be left out
of the layout [[13::5]]

Conversion note: Figure 9 (trace example: WORST, BETTER, BEST corners) and Figure 10 (example layout with the DPW package and a 0402 0.1 µF bypass capacitor, p.14) are not drawn.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

```package footprint=X2SON-5-DPW
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-4-YFP
```

## Ordering information

```ordering
SN74AUP1G06DBVR | Reel of 3000 | -40 to 85 | H06R
SN74AUP1G06DBVT | Reel of 250 | -40 to 85 | H06R
SN74AUP1G06DCKR | Reel of 3000 | -40 to 85 | HT5, HTF, HTR
SN74AUP1G06DCKT | Reel of 250 | -40 to 85 | HT5, HTR
SN74AUP1G06DPWR | Reel of 3000 | -40 to 85 | CH
SN74AUP1G06DRLR | Reel of 4000 | -40 to 85 | HT7, HTR
SN74AUP1G06DRYR | Reel of 5000 | -40 to 85 | HT
SN74AUP1G06DSF2 | Reel of 5000 | -40 to 85 | HT
SN74AUP1G06DSFR | Reel of 5000 | -40 to 85 | HT
SN74AUP1G06YFPR | Reel of 3000 | | HTN
```
[[21::table #1]]

## Conversion notes

- Source pages 1–14 converted; the documentation support and
  mechanical information pages (15–17), the DPW drawings (18–20), tape
  and reel and mechanical pages (23–52) dropped, replaced by the
  generated Package and Ordering sections. The ordering lines are from
  the package option addendum (pp.21–22). The table of contents and
  revision history on page 2 are dropped too.
- Dropped: the features and applications lists, the Device Information
  table, Figure 1 (family power comparison) and Figure 2 (25 MHz
  switching waveform of the AUP1G08) of the typical characteristics
  (p.7), and the pulse-duration and setup/hold waveforms of Figure 3,
  which the source notes are not applicable to all devices. The logic
  diagram, the clamping diodes (Figure 6), Figure 4, the application
  schematic (Figure 7), the application curve (Figure 8) and the layout
  figures are described in text.
- The addendum also lists SN74AUP1G06DBVRE4, SN74AUP1G06DBVRG4,
  SN74AUP1G06DCKRG4 and SN74AUP1G06DSFRG4, which the catalog doesn't
  carry.
- From images: µs (time axis of Figure 8, p.13; the text layer has
  other glyphs); V~Δ~ (Figure 4, p.9; the text layer reads VD).
- From images: ΔI~off~, ΔI~CC~ (electrical characteristics, p.5; the
  text layer breaks each symbol over two lines).
- Source issues:
  - Unconfirmed: the electrical characteristics (p.5) name the I~I~ row
    "Inflection point current" (electrical characteristics note 1).
  - Unconfirmed: t~pd~ typical at V~CC~ = 1.2 V ± 0.1 V is printed as
    12 ns in all four switching tables, above the 9.9 ns and 11.2 ns
    maximums of the 5 pF and 10 pF tables (switching characteristics
    notes).
  - Source conflict: V~IH~ and V~IL~ referenced in the Electrical
    Characteristics table but printed in the Recommended Operating
    Conditions (Detailed design procedure note).
  - Source error: the design procedure names the SN74AUP1G240; kept as
    printed (Detailed design procedure note).
  - Unconfirmed: the last sentence of the layout guidelines (p.13)
    ends at "left out of the layout" with no period, and the next page
    does not continue it.
  - Unconfirmed: "defined the in the Absolute Maximum Ratings" (p.10)
    and "FLoating" (p.12) are printed so.
  - Unconfirmed: the title says open-drain outputs, and the device has
    one output, Y.
  - Unconfirmed: the package option addendum (p.21) prints no operating
    temperature for SN74AUP1G06YFPR and -40 to 85 for
    SN74AUP1G06YFPR.B.
