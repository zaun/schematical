---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "ECS-2520MV",
  "title": "ECS-2520MV SMD MultiVolt crystal oscillator",
  "class": "oscillator",
  "source": {
    "pdf": "osc/ECS-2520MV.pdf",
    "vendor": "ECS Inc. International",
    "document": "",
    "date": "2026",
    "pages": 2
  },
  "devices": ["oscillator:ecs-2520mv-1", "oscillator:ecs-2520mv-2", "oscillator:ecs-2520mv-3"],
  "parts": ["ECS-2520MV-*"],
  "packages": {"smd2520": "2.5 × 2.0 × 0.8 mm ceramic SMD"},
  "converted": "2026-11-07"
}
---

# ECS-2520MV SMD MultiVolt crystal oscillator

## Description

A crystal oscillator with an HCMOS output in a 2.5 × 2.0 × 0.8 mm
ceramic package, running from any supply between 1.6 V and 3.6 V
(1.8 V, 2.5 V and 3.3 V systems). Frequencies from 0.75 MHz to
160 MHz; pad 1 puts the output in high impedance.

## Pinout

```pinout pinout=smd2520
```

## Pin functions

```pins
Tri-state: Output enable. High (≥ 0.7 × V~dd~) or open: output active. Low (≤ 0.3 × V~dd~): output high impedance.
Gnd: Ground
Output: HCMOS clock output
Vdd: Supply
```

## Function tables

```table id=tristate caption="Tri-state function"
| Pad 1 | Output |
|---|---|
| ≥ 0.7 × V~dd~ or open | Active |
| ≤ 0.3 × V~dd~ | High impedance |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings (the source gives storage temperature only)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| T~stg~ | Storage temperature | -55 | 125 | °C |
```

## Recommended operating conditions

```table id=roc caption="Operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~dd~ | Supply voltage | | 1.6 | 3.6 | V |
| T~A~ | Operating temperature [1] | Temperature code N | -40 | 85 | °C |
| C~L~ | Output load | CMOS | | 15 | pF |

[1] Set by the part number's temperature code: L -10 to 70 °C, M -20 to 70 °C, N -40 to 85 °C, P -40 to 105 °C.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| f | Frequency range | | 0.750 | | 160.000 | MHz |
| Δf/f | Frequency stability [1] | -40 to 85 °C, stability code B | | | ±50 | ppm |
| V~OL~ | Output low voltage | | | | 0.1 × V~dd~ | V |
| V~OH~ | Output high voltage | | 0.9 × V~dd~ | | | V |
| t~r~, t~f~ | Rise and fall time | 10 % to 90 % V~dd~ | | | 7 | ns |
| t~su~ | Start-up time | to 90 % V~dd~ | | | 5 | ms |
| | Phase jitter | 12 kHz to 5 MHz | | | 1 | ps |
| | Duty cycle | at ½ V~dd~ | | | 45/55 | % |
| I~stby~ | Standby current | Pad 1 low | | | 10 | µA |

[1] Includes initial tolerance, temperature, supply voltage and load change, reflow frequency shift and aging. Set by the part number's stability code (see Ordering information).
```

```table id=idd caption="Current consumption, maximum" values=Max
| V~dd~ | ≤ 19 MHz | ≤ 39.9 MHz | ≤ 50 MHz | ≤ 60 MHz | ≤ 160 MHz | Unit |
|---|--:|--:|--:|--:|--:|---|
| 1.8 V | 2.5 | 3.0 | 3.5 | 4.0 | 40 [1] | mA |
| 2.5 V | 4.5 | 5.5 | 6.5 | 7.0 | 40 [1] | mA |
| 3.0 V | 5.5 | 6.5 | 7.5 | 8.0 | 40 [1] | mA |
| 3.3 V | 6.0 | 7.0 | 8.0 | 8.5 | 40 [1] | mA |

[1] As printed; ten times the 60 MHz column, possibly 4.0 mA. Not confirmed with ECS.
```

## Package

```package footprint=OSC-ECS-2520-4-MV
```

```table id=pkg-dims caption="Package"
| Dimension | Value | Unit |
|---|--:|---|
| Body | 2.5 ± 0.1 × 2.0 ± 0.1 | mm |
| Height | 0.8 ± 0.1 | mm |
| Pad 1 mark | 0.3 corner chamfer | mm |
| Moisture sensitivity level | MSL 1 | |
| Lead finish | Au | |
```

```table id=reflow caption="Suggested reflow profile"
| Process | Peak T1 | Time at peak t1 | Preheat T2 | Time above T2, t2 | Cycles |
|---|---|---|---|---|---|
| Lead-free | 260 ± 5 °C | 10 ± 5 s | 217 °C min | 60–150 s | 2 max |
| Tin-lead | 260 ± 5 °C | 10 ± 5 s | 183 °C min | 60–150 s | 2 max |
```

## Ordering information

```ordering
```

```table id=part-key caption="Part number key: ECS-2520MV-[frequency]-[stability][temperature]-TR"
| Field | Code | Meaning |
|---|---|---|
| Frequency | 040, 080, 120, 130, 160, 240, 250, 260, 320, 480, 500, 1000, 1250 | MHz × 10: 4, 8, 12, 13, 16, 24, 25, 26, 32, 48, 50, 100, 125 MHz |
| ^ | 333.3 | 33.3333 MHz |
| Stability | A | ±100 ppm |
| ^ | B | ±50 ppm |
| ^ | C | ±25 ppm |
| ^ | D | ±20 ppm |
| Temperature | L | -10 to 70 °C |
| ^ | M | -20 to 70 °C |
| ^ | N | -40 to 85 °C |
| ^ | P | -40 to 105 °C |
| Packaging | -TR | Tape and reel, 3,000 per reel |
```

## Conversion notes

- Both pages converted. Dropped: photo, feature bullets, "Request a
  Sample", address block, pocket tape drawing.
- Package views and the suggested land pattern (Figures 1 and 2) are
  replaced by the generated package drawing; the catalog footprint was
  made from Figure 2.
- The developed-frequencies list (page 2) is folded into the part
  number key. The catalog also carries codes 200 and 491.52, which the
  source's list doesn't show.
- The source gives only one stability value (B, ±50 ppm) in its table;
  the other codes come from the part numbering guide.
- Reflow profile graph (Figure 3) reduced to its parameter table.
