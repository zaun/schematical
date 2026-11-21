---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup1g14",
  "title": "SN74AUP1G14 low-power single Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup1g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES578J",
    "date": "2017-09",
    "pages": 50
  },
  "devices": ["gate:1G14", "gate:1G14_SON"],
  "parts": ["SN74AUP1G14DBVR", "SN74AUP1G14DBVT", "SN74AUP1G14DCKR", "SN74AUP1G14DCKT", "SN74AUP1G14DPWR", "SN74AUP1G14DRLR", "SN74AUP1G14YFPR", "SN74AUP1G14DRYR", "SN74AUP1G14DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga4": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP1G14 low-power single Schmitt-trigger inverter

## Description

The AUP family is TI's premier solution to the industry's low power
needs in battery-powered portable applications. This family assures a
very low static and dynamic power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in an increased battery life. This
product also maintains excellent signal integrity.

This device functions as an independent gate with Schmitt-trigger
inputs, which allows for slow input transition and better
switching-noise immunity at the input.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when the device is
powered down. This inhibits current backflow into the device which
prevents damage to the device.

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
A: Logic Input
GND: Ground
NC: No internal connection
VCC: Positive Supply
Y: Output
```

## Functional description

This device functions as an independent gate with Schmitt-trigger
inputs, which allows for slow input transition and better
switching-noise immunity at the input.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when the device is
powered down. This inhibits current backflow into the device which
prevents damage to the device.

Conversion note: the logic diagrams show one Schmitt-trigger inverter, with input A on pin 2 and output Y on pin 4 (Figure 4, the DBV, DCK, DRL, DRY, DSF, DPW, and YZP packages), and with input A marked 1 and output Y marked 3 (Figure 5, the YFP package).

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar currents.
The drive capability of this device creates fast edges into light loads
so routing and load conditions should be considered to prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to over-current. The electrical and
thermal limits defined the in the Absolute Maximum Ratings table must be
followed at all times.

### Schmitt-trigger inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance given in the Electrical
Characteristics table. The worst case resistance is calculated with the
maximum input voltage, given in the Absolute Maximum Ratings table, and
the maximum input leakage current, given in the Electrical
Characteristics table, using ohm's law (R = V ÷ I).

The Schmitt-trigger input architecture provides hysteresis as defined by
ΔV~T~ in the Electrical Characteristics table, which makes this device
extremely tolerant to slow or noisy inputs. While the inputs can be
driven much slower than standard CMOS inputs, it is still recommended to
properly terminate unused inputs. Driving the inputs slowly will also
increase dynamic current consumption of the device. For additional
information regarding Schmitt-trigger inputs, see Understanding Schmitt
Triggers.

### Clamp diodes

The inputs and outputs to this device have negative clamping diodes.

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed.

Conversion note: Figure 6 (electrical placement of clamping diodes for each input and output) shows one diode from GND to the input and one from GND to the output, carrying -I~IK~ and -I~OK~.

### Partial power down (I~off~)

The inputs and outputs for this device enter a high impedance state when
the supply voltage is 0 V. The maximum leakage into or out of any input
or output pin on the device is specified by I~off~ in the Electrical
Characteristics table.

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value specified in
the Absolute Maximum Ratings table.

### Device functional modes

Table 1 lists the functional modes of the SN74AUP1G14.

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 4.6 | V |
| V~I~ | Input voltage [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="See [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | -1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -4 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 4 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to assure proper device operation. See Implications of Slow or Floating CMOS Inputs.
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AUP1G14 | < | < | < | < | < | < | Unit |
| ^ | ^ | DRY (SON) 6 pins | DSF (SON) 6 pins | YFP (DSBGA) 4 pins | DPW (X2SON) 5 pins | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT) 5 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 347.8 | 386.2 | 179.3 | 489.2 | 289.1 | 325.1 | 324.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 237.7 | 192.9 | 2.8 | 226.3 | 213.7 | 229.1 | 156.7 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 210.6 | 242.2 | 58.3 | 352.9 | 123.0 | 123.2 | 172.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 64.4 | 28.9 | 1.1 | 38.2 | 104.5 | 96.3 | 21.6 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 210.6 | 241.9 | 58.6 | 352.1 | 122.3 | 122.4 | 173.6 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | 150.8 | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 0.8 V | 0.3 | | 0.6 | V |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.1 V | 0.53 | | 0.9 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.4 V | 0.74 | | 1.11 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.65 V | 0.91 | | 1.29 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 2.3 V | 1.37 | | 1.77 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 3 V | 1.88 | | 2.29 | ^ |
| V~T–~ | Negative-going input threshold voltage | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 0.8 V | 0.1 | | 0.6 | V |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.1 V | 0.26 | | 0.65 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.4 V | 0.39 | | 0.75 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.65 V | 0.47 | | 0.84 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 2.3 V | 0.69 | | 1.04 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 3 V | 0.88 | | 1.24 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 0.8 V | 0.07 | | 0.5 | V |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.1 V | 0.08 | | 0.46 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.4 V | 0.18 | | 0.56 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 1.65 V | 0.27 | | 0.66 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 2.3 V | 0.53 | | 0.92 | ^ |
| ^ | ^ | T~A~ = 25°C, T~A~ = –40°C to +85°C | < | 3 V | 0.79 | | 1.31 | ^ |
| V~OH~ | | I~OH~ = –20 µA | T~A~ = 25°C, T~A~ = –40°C to +85°C | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –1.1 mA | T~A~ = 25°C | 1.1 V | 0.75 × V~CC~ | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 0.7 × V~CC~ | | | ^ |
| ^ | ^ | I~OH~ = –1.7 mA | T~A~ = 25°C | 1.4 V | 1.11 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 1.03 | | | ^ |
| ^ | ^ | I~OH~ = –1.9 mA | T~A~ = 25°C | 1.65 V | 1.32 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 1.3 | | | ^ |
| ^ | ^ | I~OH~ = –2.3 mA | T~A~ = 25°C | 2.3 V | 2.05 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 1.97 | | | ^ |
| ^ | ^ | I~OH~ = –3.1 mA | T~A~ = 25°C | ^ | 1.9 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 1.85 | | | ^ |
| ^ | ^ | I~OH~ = –2.7 mA | T~A~ = 25°C | 3 V | 2.72 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 2.67 | | | ^ |
| ^ | ^ | I~OH~ = –4 mA | T~A~ = 25°C | ^ | 2.6 | | | ^ |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | 2.55 | | | ^ |
| V~OL~ | | I~OL~ = 20 μA | T~A~ = 25°C, T~A~ = –40°C to +85°C | 0.8 V to 3.6 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | T~A~ = 25°C, T~A~ = –40°C to +85°C | 1.1 V | | | 0.3 × V~CC~ | ^ |
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
| I~I~ | A input | V~I~ = GND to 3.6 V | T~A~ = 25°C | 0 V to 3.6 V | | | 0.1 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.5 | ^ |
| I~OFF~ |  | V~I~ or V~O~ = 0 V to 3.6 V | T~A~ = 25°C | 0 V | | | 0.2 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.6 | ^ |
| ΔI~OFF~ |  | V~I~ or V~O~ = 0 V to 3.6 V | T~A~ = 25°C | 0 V to 0.2 V | | | 0.2 |  |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.6 |  |
| I~CC~ |  | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | T~A~ = 25°C | 0.8 V to 3.6 V | | | 0.5 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 0.9 | ^ |
| ΔI~CC~ |  | V~I~ = V~CC~ – 0.6 V, I~O~ = 0 | T~A~ = 25°C | 3.3 V | | | 40 | µA |
| ^ | ^ | ^ | T~A~ = –40°C to +85°C | ^ | | | 50 | ^ |
| C~I~ | | V~I~ = V~CC~ or GND | T~A~ = –40°C to +85°C | 0 V, 3.6 V | 1.5 [1] | | | pF |
| C~O~ | | V~O~ = GND | T~A~ = –40°C to +85°C | 0 V | 2.5 [1] | | | pF |

[1] Unconfirmed: the source prints this value at the MIN position of the MIN TYP MAX header; the features (p.1) give C~I~ = 1.5 pF Typical.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 4 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4.1 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4.1 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 4.3 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.4 | ^ |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-5pf caption="Switching characteristics: C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 5 pF (unless otherwise noted) (see Figure 2 and Figure 3)"
| Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 16.3 | | ns |
| ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 4.2 | 6.9 | 11.7 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 0.9 | | 15 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 3.7 | 5.2 | 8.4 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.7 | | 10.7 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 3.3 | 4.4 | 6.9 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.9 | | 8.5 | ^ |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 2.8 | 3.5 | 4.8 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.8 | | 6.1 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 2.5 | 3 | 4 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.7 | | 4.9 | ^ |
```

```table id=sw-10pf caption="Switching characteristics: C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 2 and Figure 3)"
| Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 18.4 | | ns |
| ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 4.6 | 7.9 | 13.4 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 1.3 | | 16.7 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 4 | 6 | 9.6 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.2 | | 11.8 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 3.6 | 5 | 7.9 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.4 | | 9.5 | ^ |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 3.2 | 4 | 5.5 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.3 | | 6.8 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 2.9 | 3.5 | 4.6 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.1 | | 5.6 | ^ |
```

```table id=sw-15pf caption="Switching characteristics: C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 2 and Figure 3)"
| Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 20.1 | | ns |
| ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 5.5 | 8.7 | 14 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.5 | | 17.3 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 4.7 | 6.7 | 10 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 3 | | 12.5 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 4.2 | 5.6 | 8.3 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 3 | | 10.1 | ^ |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 3.6 | 4.5 | 5.9 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.7 | | 7.4 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 3.3 | 3.9 | 5 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 2.5 | | 6.1 | ^ |
```

```table id=sw-30pf caption="Switching characteristics: C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 2 and Figure 3)"
| Parameter | From (input) | To (output) | Test conditions | < | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~pd~ | A | Y | V~CC~ = 0.8 V | T~A~ = 25°C | | 25.7 | | ns |
| ^ | ^ | ^ | V~CC~ = 1.2 V ± 0.1 V | T~A~ = 25°C | 7.4 | 11.2 | 17.1 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 4.5 | | 20.5 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.5 V ± 0.1 V | T~A~ = 25°C | 6.1 | 8.5 | 12.3 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 4.6 | | 14.7 | ^ |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | T~A~ = 25°C | 5.4 | 7.2 | 10.3 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 4.1 | | 12 | ^ |
| ^ | ^ | ^ | V~CC~ = 2.5 V ± 0.2 V | T~A~ = 25°C | 4.7 | 5.7 | 7.4 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 3.7 | | 8.8 | ^ |
| ^ | ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | T~A~ = 25°C | 4.2 | 5 | 6.2 | ^ |
| ^ | ^ | ^ | ^ | T~A~ = –40°C to +85°C | 3.5 | | 7.3 | ^ |
```

## Typical characteristics

```curve id=tpd-vcc caption="Typical Propagation Delay vs. Supply Voltage (C~L~ = 15 pF), T~A~ = 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0.75, 3.5]},
 "y": {"label": "tpd (TYP)", "unit": "ns", "range": [2, 22]},
 "series": [
  {"label": "TA = 25°C", "points": [[0.8, 20], [1.2, 8.7], [1.5, 6.7], [1.8, 5.6], [2.5, 4.5], [3.3, 3.9]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, t~r~/t~f~ = 3 ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices.

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | V~CC~ = 1.5 V ± 0.1 V | V~CC~ = 1.8 V ± 0.15 V | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|---|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~M~ | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 |
| V~I~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ |
```

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

Mechanical input elements, such as push buttons or rotary knobs, offer
simple ways to interact with electronic systems. Typically, these
elements have recoil or bouncing, where the mechanical element makes and
breaks contact multiple times during human interaction. This bouncing
can cause one or more repeated signals to be passed, triggering multiple
actions when only a single input was intended. One potential solution to
mitigating these multiple inputs is by utilizing a Schmitt-trigger to
create a debounce circuit. Figure 7 shows an example of this solution.

The input due to the push button switches multiple times, causing the
output of a non Schmitt-trigger device to trigger multiple times, while
the Schmitt-trigger input device with RC delay limits the output pulse
to a single pulse desired by the user. The separated positive and
negative input voltage threshold values prevent multiple triggers from
occurring, see the Electrical Characteristics table for V~T+~, V~T-~,
and V~hys~ values.

Conversion note: Figure 7 (push button debounce circuit schematic) shows a push button to ground with a resistor R1 to V~CC~, a resistor R2 from the button to input A with a capacitor C1 to ground, and output Y as the OUTPUT.

Source conflict: the typical application (p.14) points to the Electrical Characteristics table for V~hys~; that table (p.6) names the hysteresis ΔV~T~ and has no V~hys~ row.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The drive strength also creates fast edges into
light loads so routing and load conditions should be considered to
prevent ringing.

### Detailed design procedure

1. Recommended Input Conditions:
   - For specified high and low levels, see (V~T+~ and V~T-~) in the Electrical Characteristics table.
   - Inputs are overvoltage tolerant allowing them to go as high as (V~I~ max) in the Recommended Operating Conditions table at any valid V~CC~.
2. Recommended Output Conditions:
   - Load currents should not exceed (I~O~ max) per output and should not exceed (Continuous current through V~CC~ or GND) total current for the part. These limits are located in the Absolute Maximum Ratings table.

### Power supply recommendations

The power supply can be any voltage between the min and max supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. It is ok to parallel multiple bypass caps to reject
different frequencies of noise. 0.1-μF and 1-μF capacitors are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

Even low data rate digital signals can contain high-frequency signal
components due to fast edge rates. When a printed-circuit board (PCB)
trace turns a corner at a 90° angle, a reflection can occur. A
reflection occurs primarily because of the change of width of the trace.
At the apex of the turn, the trace width increases to 1.414 times the
width. This increase upsets the transmission-line characteristics,
especially the distributed capacitance and self–inductance of the trace
which results in the reflection. Not all PCB traces can be straight and
therefore some traces must turn corners. Figure 10 shows progressively
better techniques of rounding corners. Only the last example (BEST)
maintains constant trace width and minimizes reflections.

An example layout is given in Figure 11 for the DPW (X2SON-5) package.
This example layout includes a 0402 (metric) capacitor and uses the
measurements found in the example board layout appended to this end of
this datasheet. A via of diameter 0.1 mm (3.973 mil) is placed directly
in the center of the device. This via can be used to trace out the
center pin connection through another board layer, or it can be left out
of the layout

Conversion note: Figure 10 (trace example: WORST, BETTER, BEST corners) and Figure 11 (example layout with the DPW package and a 0402 0.1 µF bypass capacitor, p.16) are not drawn.

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
SN74AUP1G14DBVR | Reel of 3000 | -40 to 85 | H14R
SN74AUP1G14DBVT | Reel of 250 | -40 to 85 | H14R
SN74AUP1G14DCKR | Reel of 3000 | -40 to 85 | HF5, HFF, HFK, HFR
SN74AUP1G14DCKT | Reel of 250 | -40 to 85 | HF5, HFK, HFR
SN74AUP1G14DPWR | Reel of 3000 | -40 to 85 | B, BZ
SN74AUP1G14DRLR | Reel of 4000 | -40 to 85 | HF7, HFR
SN74AUP1G14DRYR | Reel of 5000 | -40 to 85 | HF
SN74AUP1G14DSFR | Reel of 5000 | -40 to 85 | HF
SN74AUP1G14YFPR | Reel of 3000 | | HFN
```

## Conversion notes

- Source pages 1–16 converted; the documentation support and
  mechanical information page (17), tape and reel and mechanical pages
  (20–50) dropped, replaced by the generated Package and Ordering
  sections. The ordering lines are from the package option addendum
  (pp.18–19). The table of contents and revision history on page 2 are
  dropped too.
- Dropped: the features and applications lists, the Device Information
  table, Figure 3 (enable and disable times load circuit and waveforms,
  p.11) and the pulse-duration and setup/hold waveforms of Figure 2,
  which the source notes are not applicable to all devices (this
  inverter has no output enable and no clock), Figure 8 (family power
  comparison) and Figure 9 (25 MHz switching waveform of the AUP1G08).
  The logic diagrams, the clamping diodes (Figure 6), the debounce
  circuit (Figure 7) and the layout figures are described in text.
- In the electrical characteristics the source prints one value cell
  across the two T~A~ rows of each V~T+~, V~T–~ and ΔV~T~ supply
  voltage and of the first V~OH~ and the first two V~OL~ conditions,
  and one across the 0 V and 3.6 V rows of C~I~; each such pair is one
  row here, naming both conditions.
- The addendum also lists SN74AUP1G14DBVRG4, SN74AUP1G14DCKR1G4,
  SN74AUP1G14DCKRG4, SN74AUP1G14DRYRG4 and SN74AUP1G14DSFRG4, which the
  catalog doesn't carry.
- Curve digitized from Figure 1 (p.9) to about 2 % of full scale; its
  T~A~ = 25°C is the legend printed inside the figure, and the source
  caption has no closing parenthesis.
- From images: curve points 20, 8.7, 6.7, 5.6, 4.5, 3.9 and the legend
  T~A~ = 25°C (Figure 1, p.9; the text layer has other glyphs).
- Source issues:
  - Source conflict: V~hys~ referenced in the Electrical
    Characteristics table, which names it ΔV~T~ (typical application
    note).
  - Unconfirmed: the C~I~ and C~O~ values (p.7) are printed at the MIN
    position (electrical characteristics note 1).
  - Unconfirmed: the ΔI~OFF~ row of the electrical characteristics
    (p.6) has an empty unit cell.
  - Unconfirmed: the logic diagram for the YFP package (p.1, Figure 5
    p.12) marks A and Y with 1 and 3, and the pin functions (p.3) give
    the YFP balls as A1 and B2; the other logic diagram names a YZP
    package that the pin functions don't have.
  - Unconfirmed: the last sentence of the layout guidelines (p.16)
    ends at "left out of the layout" with no period.
  - Unconfirmed: "defined the in the Absolute Maximum Ratings table"
    (p.12) is printed so.
  - Unconfirmed: the package option addendum (p.19) prints no operating
    temperature for SN74AUP1G14YFPR and -40 to 85 for
    SN74AUP1G14YFPR.B.
