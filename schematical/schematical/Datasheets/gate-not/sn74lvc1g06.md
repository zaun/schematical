---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc1g06",
  "title": "SN74LVC1G06 single inverter buffer or driver with open-drain output",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc1g06.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES295AB",
    "date": "2025-10",
    "pages": 48
  },
  "devices": ["gate:1G06", "gate:1G06_SON"],
  "parts": ["SN74LVC1G06DBVR", "SN74LVC1G06DBVT", "SN74LVC1G06DBVTG4", "SN74LVC1G06DCKR", "SN74LVC1G06DCKT", "SN74LVC1G06DPWR", "SN74LVC1G06YZPR", "SN74LVC1G06DRYR", "SN74LVC1G06DSFR"],
  "packages": {"p5": "DBV, DCK, DRL, DPW", "p6": "DRY, DSF", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC1G06 single inverter buffer or driver with open-drain output

## Description

Supports 5V V~CC~ operation. Input and open-drain output accept voltages
up to 5.5V. Latch-up performance exceeds 100mA per JESD 78, class II.
Can be used for up or down translation. [[1::3]]

This single inverter buffer and driver is designed for 1.65V to 5.5V
V~CC~ operation. [[1::2]]

The output of the SN74LVC1G06 device is open-drain and can be connected
to other open-drain outputs to implement active-low wired-OR or
active-high wired-AND functions. The maximum sink current is 32mA. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs when the device is
powered down. This inhibits current backflow into the device which
prevents damage to the device. [[1::6]]

## Pinout

```pinout pinout=p5
```

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
A: Input
DNU: Do not use
GND: Ground
NC: Not connected
VCC: Power pin
Y: Output
```
[[3::Table 4-1]]

NC – No internal connection [[3::1]]

See mechanical drawings for dimensions. [[3::1]]

## Functional description

The SN74LVC1G06 device contains one open-drain inverter with a maximum
sink current of 32mA. This device is fully specified for
partial-power-down applications using I~off~. The I~off~ circuitry
disables the outputs when the device is powered down. This inhibits
current backflow into the device which prevents damage to the device. [[9::1]]

Conversion note: the logic diagram (positive logic) shows one inverter with an open-drain output, A (pin 2) to Y (pin 4).

### CMOS open-drain outputs

The open-drain output allows the device to sink current to GND but not
to source current from V~CC~. When the output is not actively pulling
the line low, it will go into a high impedance state (tri-state). This
allows the device to be used for a wide variety of applications,
including up-translation and down-translation, as the output voltage can
be determined by an external pullup. [[9::2]]

Consider the drive capability of this device creates fast edges into
light loads so routing and load conditions prevent ringing.
Additionally, the outputs of this device are capable of driving larger
currents than the device can sustain without being damaged. It is
important for the power output of the device to be limited to avoid
thermal runaway and damage due to over-current. The electrical and
thermal limits defined in the Absolute Maximum Ratings must be followed
at all times. [[9::3]]

### Standard CMOS inputs

The impendence for standard CMOS inputs is high. Typically, a CMOS input
is modeled as a resistor in parallel with the input capacitance as shown
in the Electrical Characteristics. The worst case resistance is
calculated with the maximum input voltage, given in the Absolute Maximum
Ratings, and the maximum input leakage current, given in the Electrical
Characteristics, using ohm's law (R = V ÷ I). [[9::4]]

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv in the Recommended Operating Conditions to avoid excessive
current consumption and oscillations. If a slow or noisy input signal is
required, a device with a Schmitt-trigger input should be used to
condition the input signal before the standard CMOS input. [[9::5]]

### Negative clamping diodes

The inputs and outputs to this device have negative clamping diodes as
depicted in Figure 6-2. [[10::1]]

Voltages beyond the values specified in the Absolute Maximum Ratings
table can cause damage to the device. The input negative-voltage and the
output voltage ratings may be exceeded if the input and output
clamp-current ratings are observed. [[10::2]]

```figure ref=clamp-diodes caption="Figure 6-2. Electrical placement of clamping diodes for each input and output"
```
[[10::Figure 6-2]]

### Partial power down (I~off~)

Each input and output enter a high impedance state when the supply
voltage is 0V. The maximum leakage into or out of any input or output
pin on the device is specified by I~off~ in the Electrical
Characteristics. [[10::4]]

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage as
long as the input signals remain below the maximum input voltage value
specified in the Recommended Operating Conditions. [[10::5]]

### Device functional modes

Table 6-1 lists the functional modes of the SN74LVC1G06. [[10::6]]

## Function tables

```table id=function caption="Table 6-1. Function table"
| Input A | Output Y |
|:-:|:-:|
| L | Hi-Z |
| H | L |
```
[[10::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6.5 | V |
| V~I~ | Input voltage [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage applied to any output in the high or low state [2] [3] | | -0.5 | 6.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~j~ | Junction temperature | | -65 | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Operation outside the Absolute Maximum Ratings may cause permanent device damage. Absolute Maximum Ratings do not imply functional operation of the device at these or any other conditions beyond those listed under Recommended Operating Conditions. If used outside the Recommended Operating Conditions but within the Absolute Maximum Ratings, the device may not be fully functional, and this may affect device reliability, functionality, performance, and shorten the device lifetime.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the Recommended Operating Conditions table.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | ^ |
| ^ | ^ | Machine Model (MM), per A115-A | 200 | ^ |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~IH~ | High-level input voltage | V~CC~ = 1.65V to 1.95V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | 1.7 | | ^ |
| ^ | ^ | V~CC~ = 3V to 3.6V | 2 | | ^ |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | 0.7 × V~CC~ | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65V to 1.95V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3V to 2.7V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3V to 3.6V | | 0.8 | ^ |
| ^ | ^ | V~CC~ = 4.5V to 5.5V | | 0.3 × V~CC~ | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | 5.5 | V |
| I~OL~ | Low-level output current | V~CC~ = 1.65V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5V | | 32 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8V ± 0.15V, 2.5V ± 0.2V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3V ± 0.3V | | 10 | ^ |
| ^ | ^ | V~CC~ = 5V ± 0.5V | | 5 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to establish proper device operation. See Implications of Slow or Floating CMOS Inputs application report.
```
[[5::table #1]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | SN74LVC1G06 | < | < | < | < | < | < | Unit |
| ^ | ^ | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT-5X3) 5 pins | DRY (SON) 5 pins [2] | DPW (X2SON) 5 pins | YZV (DSBGA) 4 pins | YZP (DSBGA) 5 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 357.1 | 371.0 | 296.2 | 369.6 | 511 | 168.2 | 144.4 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 263.7 | 297.5 | 137.3 | 257.6 | 241.9 | 2.1 | 1.3 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 264.4 | 258.6 | 145.3 | 230.8 | 374.2 | 55.9 | 39.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 195.6 | 195.6 | 14.7 | 77.2 | 45 | 1.1 | 0.5 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 262.2 | 256.2 | 145.9 | 231 | 373.3 | 56.3 | 39.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | 168 | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application note.
[2] Source error: the table prints "5 PINS" under DRY; the package information (p.1) and the pin drawing (p.3) give the DRY package 6 pins. Kept as printed.
```
[[5::table #2]]

Conversion note: the source gives no thermal data for the DSF package.

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OL~ | High-level output voltage [2] | I~OL~ = 100μA | < | 1.65V to 5.5V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4mA | < | 1.65V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8mA | < | 2.3V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16mA | < | 3V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24mA | < | ^ | | | 0.55 | ^ |
| ^ | ^ | I~OL~ = 32mA | < | 4.5V | | | 0.55 | ^ |
| I~I~ | Inflection-point current | V~I~ = 5.5V or GND | A input | 0 to 5.5V | | | ±1 | µA |
| I~off~ | Off-state current | V~I~ or V~O~ = 5.5V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5V or GND, I~O~ = 0 | < | 1.65V to 5.5V | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6V, other inputs at V~CC~ or GND | < | 3V to 5.5V | | | 500 | µA |
| C~I~ | Input capacitance | V~I~ = V~CC~ or GND | < | 3.3V | | 4 | | pF |
| C~O~ | Off-state capacitance | V~O~ = V~CC~ or GND | < | 3.3V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3V, T~A~ = 25°C.
[2] Source error: V~OL~, with I~OL~ test conditions and maximum values, is the low-level output voltage; the source prints "High-level output voltage". Kept as printed.
```
[[6::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10MHz | 1.8V | 3 | pF |
| ^ | ^ | ^ | 2.5V | 3 | ^ |
| ^ | ^ | ^ | 3.3V | 4 | ^ |
| ^ | ^ | ^ | 5V | 6 | ^ |
```
[[6::table #2]]

## Switching characteristics

Conversion note: Figure 6-1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics: –40°C to +85°C" conditions="Over recommended operating free-air temperature range, T~A~ = –40°C to +85°C (unless otherwise noted) (see Figure 6-1)"
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Max | Unit |
|---|---|---|---|---|--:|--:|---|
| t~pd~ | Propagation delay | A | Y | 1.8V ± 0.15V | 2.2 | 6.5 | ns |
| ^ | ^ | ^ | ^ | 2.5V ± 0.2V | 1.1 | 4 | ^ |
| ^ | ^ | ^ | ^ | 3.3V ± 0.3V | 1.2 | 4 | ^ |
| ^ | ^ | ^ | ^ | 5V ± 0.5V | 1 | 3 | ^ |
```
[[6::table #3]]

```table id=sw-125 caption="Switching characteristics: –40°C to +125°C" conditions="Over recommended operating free-air temperature range, T~A~ = –40°C to +125°C (unless otherwise noted) (see Figure 6-1)"
| Symbol | Parameter | From (input) | To (output) | V~CC~ | Min | Max | Unit |
|---|---|---|---|---|--:|--:|---|
| t~pd~ | Propagation delay | A | Y | 1.8V ± 0.15V | 2.2 | 7 | ns |
| ^ | ^ | ^ | ^ | 2.5V ± 0.2V | 1.1 | 4.5 | ^ |
| ^ | ^ | ^ | ^ | 3.3V ± 0.3V | 1.2 | 4.5 | ^ |
| ^ | ^ | ^ | ^ | 5V ± 0.5V | 1 | 3.5 | ^ |
```
[[6::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="Figure 5-1. TPD across temperature at 3.3V V~CC~"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 2.5]},
 "series": [
  {"label": "TPD", "points": [[-50, 1.5], [125, 2.0]]}
 ]}
```
[[7::Figure 5-1]]

```curve id=tpd-vcc caption="Figure 5-2. TPD across V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD", "points": [[1.8, 5.5], [2.5, 3.05], [3.3, 1.7], [5.0, 2.05]]}
 ]}
```
[[7::Figure 5-2]]

```curve id=icc-freq caption="Figure 7-2. I~CC~ vs frequency"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0, 80]},
 "y": {"label": "Icc", "unit": "µA", "range": [0, 1600]},
 "series": [
  {"label": "Icc 1.8V", "points": [[0, 0], [20, 80], [40, 140], [60, 190], [75, 220]]},
  {"label": "Icc 2.5V", "points": [[0, 0], [20, 130], [40, 210], [60, 280], [75, 350]]},
  {"label": "Icc 3.3V", "points": [[0, 0], [20, 180], [40, 330], [45, 380], [60, 480], [75, 590]]},
  {"label": "Icc 5V", "points": [[0, 0], [20, 400], [35, 680], [45, 920], [50, 980], [60, 1200], [75, 1500]]}
 ]}
```
[[12::Figure 7-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. Since this device has open-drain outputs, t~PLZ~ and
t~PZL~ are the same as t~pd~. t~PZL~ is measured at V~M~. t~PLZ~ is
measured at V~OL~ + V~∆~. All parameters and waveforms are not
applicable to all devices. [[8::1]]

Conversion note: the load circuit of Figure 6-1 has C~L~ and one R~L~
from the output under test to ground, and a second R~L~ from the output
to switch S1, which goes to V~LOAD~, open or GND.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PZL~ (see Notes E and F) | V~LOAD~ |
| t~PLZ~ (see Notes E and G) | V~LOAD~ |
| t~PHZ~/t~PZH~ | V~LOAD~ |
```
[[8::table #1]]

```table id=test-conditions caption="Figure 6-1. Load circuit and voltage waveforms (open drain)"
| V~CC~ | Input | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~∆~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤ 2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤ 2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤ 2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[8::table #2]]

Conversion note: the enable and disable waveforms of Figure 6-1 show
t~PZL~ from V~M~ of the input to V~M~ of the falling output and t~PLZ~
from V~M~ of the input to V~OL~ + V~∆~ of the rising output, which ends
at V~LOAD~/2, and the pulse-duration, setup-and-hold and
propagation-delay waveforms of the figure are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[11::1]]

The SN74LVC1G06 is a high-drive CMOS device that can be used to
implement a high output drive buffer, such as an LED application. It can
sink 32mA of current at 4.5V making it appropriate for high-drive
applications. It is good for high-speed applications up to 100MHz. The
inputs are 5.5V tolerant allowing it to translate up or down to V~CC~.
Below shows a simple LED driver application for a single channel of the
device. [[11::2]]

Conversion note: in the typical application diagram (Figure 7-1) the
input is driven from an MCU, and the output is connected to the cathode
of an LED whose anode goes through a resistor to VPU.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads so routing and load conditions should be considered to
prevent ringing. [[11::3]]

### Detailed design procedure

1. Recommended Input Conditions [[11::4]]
   - Rise time and fall time specs. See (Δt/ΔV) in the Recommended
     Operating Conditions table. [[11::4]]
   - Specified high and low levels. See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table. [[11::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     (V~I~ max) in the Recommended Operating Conditions table at any
     valid V~CC~. [[11::4]]
2. Recommended Output Conditions [[11::4]]
   - Load currents should not exceed (I~O~ max) per output and should
     not exceed (Continuous current through V~CC~ or GND) total current
     for the part. These limits are located in the Absolute Maximum
     Ratings table. [[11::4]]
   - Outputs should not be pulled above 5.5V. [[11::4]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Recommended Operating Conditions
table. [[12::1]]

The V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. A 0.1µF capacitor is recommended, and it is acceptable to
parallel multiple bypass caps to reject different frequencies of noise.
0.1µF and 1µF capacitors are commonly used in parallel. For the best
results, install the bypass capacitor as close to the power pin as
possible. [[12::2]]

### Layout guidelines

Even low data rate digital signals can contain high-frequency signal
components due to fast edge rates. When a printed-circuit board (PCB)
trace turns a corner at a 90° angle, a reflection can occur. A
reflection occurs primarily because of the change of width of the trace.
At the apex of the turn, the trace width increases to 1.414 times the
width. This increase upsets the transmission-line characteristics,
especially the distributed capacitance and self–inductance of the trace
which results in the reflection. Not all PCB traces can be straight and
therefore some traces must turn corners. Figure 7-3 shows progressively
better techniques of rounding corners. Only the last example (BEST)
maintains constant trace width and minimizes reflections. [[12::3]]

An example layout is given in Figure 7-4 for the DPW (X2SON-5) package.
This example layout includes a 0402 (metric) capacitor and uses the
measurements found in the example board layout appended to this end of
this data sheet. A via of diameter 0.1mm (3.973mil) is placed directly
in the center of the device. This via can be used to trace out the
center pin connection through another board layer, or it can be left out
of the layout. [[12::4]]

Conversion note: the layout figures (Figure 7-3 and Figure 7-4) are not
drawn here.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=X2SON-5-DPW
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-5-YZP
```

## Ordering information

```ordering
SN74LVC1G06DBVR | Reel of 3000 | -40 to 125 | 3VSF, C065, C06F, C06J, C06R, C06T, C06H, C06P, C06S
SN74LVC1G06DBVT | Reel of 250 | -40 to 125 | C065, C06F, C06J, C06R, C06H, C06P, C06S
SN74LVC1G06DBVTG4 | Reel of 250 | -40 to 125 | C06F
SN74LVC1G06DCKR | Reel of 3000 | -40 to 125 | CT5, CTF, CTJ, CTK, CTR, CTT, CTH, CTS
SN74LVC1G06DCKT | Reel of 250 | -40 to 125 | CT5, CTF, CTJ, CTK, CTR, CTH, CTS
SN74LVC1G06DPWR | Reel of 3000 | -40 to 85 | CO
SN74LVC1G06YZPR | Reel of 3000 | -40 to 85 | CTN
SN74LVC1G06DRYR | Reel of 5000 | -40 to 125 | CT
SN74LVC1G06DSFR | Reel of 5000 | -40 to 125 | CT
```
[[16::table #1]] [[21::table #1]]

## Conversion notes

- Source pages 1 and 3–13 converted. The ordering lines are from the
  package option addendum (pp.16–18). The table of contents (p.2), the
  support and revision history pages (14–15), the rest of the addendum
  (p.19), tape and reel (pp.20–23) and mechanical pages (24–48) are left
  out; the generated Package section replaces them.
- Dropped: the features list except the lines quoted, the NanoFree
  package sentence of the description, the applications list, the
  package information table and the vendor pinout drawings. The logic
  diagram, the load circuit and waveforms (Figure 6-1, p.8), the typical
  application diagram (Figure 7-1) and the layout figures (Figures 7-3
  and 7-4) are described in text. Of the lettered notes of Figure 6-1,
  A and C to H are kept as sentences; B is for the two output waveforms,
  which are not drawn.
- The pin table's two notes are kept as lines under the pin
  descriptions.
- The CAUTION box of Negative Clamping Diodes is kept as a paragraph.
- The catalog doesn't carry the DRL (SOT-5X3) and YZV (4-pin DSBGA)
  parts; the YZV pinout (A at A1, V~CC~ at A2, GND at B1, Y at B2) is
  not drawn. The addendum also lists SN74LVC1G06DBVRE4,
  SN74LVC1G06DBVRG4, SN74LVC1G06DCKRE4, SN74LVC1G06DCKRG4,
  SN74LVC1G06DCKTE4, SN74LVC1G06DCKTG4, SN74LVC1G06DRLR,
  SN74LVC1G06DRYRG4, SN74LVC1G06DSFRG4 and SN74LVC1G06YZVR, which the
  catalog doesn't carry.
- Curves digitized from Figures 5-1, 5-2 (p.7) and 7-2 (p.12) to about
  2 % of full scale.
- From images: the curve points of Figures 5-1, 5-2 and 7-2; the
  arrangement of Figure 6-1 (p.8) and Figure 7-1 (p.11); V~I~ and V~O~
  in the test conditions of C~I~ and C~O~ (p.6; the text layer prints
  the subscripts on the next line).
- Source issues:
  - Source error: the electrical characteristics (p.6) name V~OL~
    "High-level output voltage"; its I~OL~ conditions and maximum values
    are those of a low-level output voltage. Kept as printed.
  - Source error: the Thermal Information table (p.5) prints "5 PINS"
    under DRY; the package information (p.1) and the pin drawing (p.3)
    give 6 pins. Kept as printed.
  - Source conflict: "Figure 6-1" is both the load circuit and voltage
    waveforms (p.8) and the logic diagram (p.9); the Parameter
    Measurement Information heading (p.8) has no section number.
  - Source conflict: the features (p.1) give ±24mA output drive at 3.3V
    for open-drain devices; an open-drain output only sinks current
    (p.9).
  - Unconfirmed: the electrical characteristics (p.6) name I~I~
    "Inflection-point current".
  - Unconfirmed: the Absolute Maximum Ratings (p.4) print a minimum
    junction temperature of –65 °C.
  - Unconfirmed: ψ~JT~ is 195.6 °C/W for both DBV and DCK (p.5).
  - Unconfirmed: Design Requirements (p.11) speaks of balanced output
    drive for this open-drain device.
  - Unconfirmed: the load table of Figure 6-1 (p.8) gives S1 at V~LOAD~
    for t~PHZ~/t~PZH~.
  - Source error: Standard CMOS Inputs (p.9) prints "impendence";
    impedance is meant. Kept as printed.
