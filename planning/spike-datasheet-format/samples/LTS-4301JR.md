---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "LTS-4301JR",
  "title": "LTS-4301JR 0.4 inch single-digit 7-segment LED display",
  "class": "display",
  "source": {
    "pdf": "led-7/LTS-4301JR.pdf",
    "vendor": "Lite-On Technology",
    "document": "DS30-2001-029 rev. A",
    "date": "2010-06",
    "pages": 6
  },
  "devices": ["7seg:LTS-4301JR"],
  "parts": ["LTS-4301JR"],
  "packages": {"p10": "10-pin DIP, 7.62 mm rows"},
  "converted": "2026-11-07"
}
---

# LTS-4301JR 0.4 inch single-digit 7-segment LED display

## Description

A single-digit seven-segment display, 0.4 inch (10.0 mm) digit height,
with a right-hand decimal point. AlInGaP super red LED chips (AlInGaP
on a transparent GaAs substrate), gray face, white segments, common
cathode. Parts are binned for luminous intensity.

## Pinout

The drawing shows the segment letters and the internal circuit (every
segment anode on its own pin, all cathodes on pins 3 and 8), both
generated from the catalog.

```pinout pinout=p10
```

## Pin functions

```pins
A, B, C, D, E, F, G: Segment anode
DP: Decimal point anode
CC: Common cathode (pins 3 and 8, connected inside)
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings, T~A~ = 25 °C"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| P~D~ | Power dissipation per segment | | | 70 | mW |
| I~FP~ | Peak forward current per segment | 1/10 duty cycle, 0.1 ms pulse | | 90 | mA |
| I~F~ | Continuous forward current per segment | | | 25 | mA |
| | Derating, linear from 25 °C, per segment | | | 0.33 | mA/°C |
| V~R~ | Reverse voltage per segment | | | 5 | V |
| T~opr~ | Operating temperature | | -35 | 85 | °C |
| T~stg~ | Storage temperature | | -35 | 85 | °C |
| | Solder temperature | 3 s max, 1.6 mm below seating plane | | 260 | °C |
```

## Electrical characteristics

```table id=eo caption="Electrical and optical characteristics, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| I~V~ | Average luminous intensity [1] | I~F~ = 1 mA | 200 | 520 | | µcd |
| λ~p~ | Peak emission wavelength | I~F~ = 20 mA | | 639 | | nm |
| Δλ | Spectral line half-width | I~F~ = 20 mA | | 20 | | nm |
| λ~d~ | Dominant wavelength | I~F~ = 20 mA | | 631 | | nm |
| V~F~ | Forward voltage per segment | I~F~ = 20 mA | | 2.0 | 2.6 | V |
| I~R~ | Reverse current per segment | V~R~ = 5 V | | | 100 | µA |
| I~V-m~ | Luminous intensity matching ratio | I~F~ = 1 mA | | | 2:1 | |

[1] Measured with a light sensor and filter combination that approximates the CIE eye-response curve.
```

## Typical characteristics

T~A~ = 25 °C unless noted.

```curve id=spectrum caption="Relative intensity vs wavelength"
{"x": {"label": "λ", "unit": "nm", "range": [500, 800]},
 "y": {"label": "Relative intensity", "unit": "%", "range": [0, 100]},
 "series": [{"label": "JR", "points": [[619, 0], [623, 6], [626, 14], [630, 45], [634, 85], [638, 100], [640, 99], [644, 72], [648, 45], [650, 30], [652, 14], [655, 6], [660, 0]]}]}
```

```curve id=efficiency caption="Relative luminous efficiency vs peak current (average ≤ 10 mA), normalized to 1 at 10 mA"
{"x": {"label": "IP", "unit": "mA", "range": [0, 100]},
 "y": {"label": "Relative efficiency", "unit": "", "range": [0, 1.6]},
 "series": [{"label": "JR", "points": [[1, 0.1], [4, 0.5], [7, 0.9], [10, 1.0], [20, 1.17], [30, 1.23], [50, 1.25], [90, 1.26]]}]}
```

```curve id=if-vf caption="Forward current vs forward voltage"
{"x": {"label": "VF", "unit": "V", "range": [1.2, 3.2]},
 "y": {"label": "IF", "unit": "mA", "range": [0, 50]},
 "series": [{"label": "JR", "points": [[1.6, 0], [1.85, 10], [1.98, 20], [2.1, 30], [2.25, 40], [2.4, 50]]}]}
```

```curve id=iv-if caption="Relative luminous intensity vs forward current, normalized to 1 at 10 mA"
{"x": {"label": "IF", "unit": "mA", "range": [0, 30]},
 "y": {"label": "Relative intensity", "unit": "", "range": [0, 4]},
 "series": [{"label": "JR", "points": [[0, 0], [10, 1.0], [20, 2.0], [30, 3.0]]}]}
```

```curve id=derating caption="Maximum DC current vs ambient temperature"
{"x": {"label": "TA", "unit": "°C", "range": [0, 90]},
 "y": {"label": "IF max", "unit": "mA", "range": [0, 40]},
 "series": [{"label": "JR", "points": [[0, 25], [25, 25], [85, 5], [85, 0]]}]}
```

```curve id=peak-duty caption="Maximum peak current vs duty cycle (refresh rate 1 kHz)"
{"x": {"label": "Duty cycle", "unit": "%", "range": [1, 100], "log": true},
 "y": {"label": "IP max", "unit": "mA", "range": [10, 1000], "log": true},
 "series": [{"label": "JR", "points": [[1, 90], [10, 90], [20, 65], [50, 38], [100, 25]]}]}
```

## Package

```package footprint=7SEG-LTS-4301JR
```

```table id=pkg-dims caption="Package dimensions (tolerance ± 0.25 mm unless noted)"
| Dimension | Value | Unit |
|---|--:|---|
| Body width × height | 9.7 × 12.9 | mm |
| Body depth | 6.4 | mm |
| Seated height | 7.0 | mm |
| Lead length below body | 3.9 ± 0.5 | mm |
| Lead width, front view | 0.5 | mm |
| Lead width, side view | 0.3 | mm |
| Lead pitch | 2.54 | mm |
| Row spacing | 7.62 | mm |
| Digit height | 10.0 | mm |
| Segment slant | 10 | ° |
| Decimal point diameter | 1.2 | mm |
```

## Ordering information

```ordering
```

## Conversion notes

- Pages 2–6 converted; cover page dropped. Dropped: feature list.
- Text extraction loses µ, λ and Δ (it reads "cd", "p", "A"); the units
  and symbols here are from the page images: I~V~ in µcd, I~R~ in µA.
- The internal circuit diagram and the segment identification drawing
  are generated from the catalog's segment and common-pin keys, so they
  are not transcribed.
- Curves digitized from Figures 1–6 (about 2–3 % of full scale; log
  scales on Figure 6).
- From images: 9.7, 12.9, 6.4, 7.0, 3.9, 0.5, 0.3, 2.54, 7.62, 10.0, 10
  and 1.2 (package drawing, p.3; the drawing has no text layer).
- The package drawing is replaced by the generated footprint plus the
  dimension table; the segment-face dimensions (6.0 and 0.85 mm) are
  left out.
