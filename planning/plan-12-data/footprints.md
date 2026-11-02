# plan-12 footprints

Rules: plan-03-B §3 R1–R6 + docs/parts-format.md mid-value / no-overlap rules. Pages are PDF page numbers. Repo paths in notes are relative to `datasheets/` (repo convention). JSON objects are paste-ready for `footprints.json`. Nothing in the repo was edited. Fetched outside documents are in `/tmp/p11/fp/dl/` (Maxim/ADI, Renesas) and `/tmp/p11/fp/dl/m/` (TI GQS/ZQS); page images in `/tmp/p11/fp/img/`.

| id | rule | source |
|---|---|---|
| `TQFP-48-7X7` | R1 | repo PDF |
| `PLCC-68` | R1 | repo PDF |
| `LQFP-64-10X10` | R1 | repo PDF |
| `QFN-32-5X5` | R1 (span read from scale) | repo PDF |
| `VQFN-32-5X5` | R1 | repo PDF |
| `LQFP-80-12X12` | R1 | repo PDF |
| `PLCC-28` | R3 | fetched |
| `QFP-100-14X20` | R2 | repo PDF |
| `HVQFN-32-5X5` | R4 | repo PDF |
| `QFN-24-4X4` | R1 | repo PDF |
| `TSSOP-28` | R2 | repo PDF |
| `QSOP-16` | R1 | fetched |
| `QFN-48-7X7` | R4 | repo PDF |
| `TQFN-32-5X5-T3255` | R1 | fetched |
| `TQFN-24-3.5X3.5` | R1 | fetched |
| `TQFN-48-7X7` | R1 | fetched |
| `QSOP-28` | R1 | fetched |
| `PDIP-48-600` | R5 | repo PDF |
| `CDIP-40-600` | R5 | fetched |
| `BGA-24-MICROSTAR` | R6 | fetched |
| `TQFN-32-5X5-ZH` | R1 | repo PDF |
| `TQFN-24-4X4-ZD` | R1 | repo PDF |
| `TQFN-24-4X4-T2444` | R1 | fetched |
| `WLP-25` | R6 | fetched |
| `SOP-24-24P2W` | — | not added: M66230 left out (no maker drawing found) |

## `TQFP-48-7X7`

```json
{"id": "TQFP-48-7X7", "description": "48-pin TQFP/LQFP, 7 x 7 mm body, 0.5 mm pitch, 9.0 mm tip span", "note": "TI PFB0048A board layout (R1; 48X (1.35) x 48X (0.25), (8.5) pad-centre rows), controller-uart/tl16c550d.pdf p.55; body 7.0 mid (6.8-7.2) from outline 4215157/A p.54. Same leads (b 0.17-0.27, L 0.45-0.75, tip span 8.8-9.2) in TI PT0048A (tl16c550d.pdf p.57), Exar 48-TQFP (st16c550.pdf p.34, xr16l2552.pdf p.43) and NXP SOT313-2 (sc16c752b.pdf p.37)", "style": "quad", "pins": 48, "body": {"width": 7.0, "height": 7.0}, "sides": [12, 12, 12, 12], "pitch": 0.5, "spanX": 8.5, "spanY": 8.5, "pad": {"width": 1.35, "height": 0.25, "shape": "rect"}}
```

**Derivation (R1).** TI PFB0048A land: 48X (1.35) x 48X (0.25), (8.5) between pad centres on both axes (the (8.5) is drawn between pad centres; as an outer extent the inner ends would sit 1.45 inside a 7.0 body). Body 7.0 mid (6.8-7.2). TI PT0048A (p.58) prints a different land (1.6 x 0.3, 8.2) for the same leads; the brief asks for one land for all 48-TQFP/LQFP parts, PFB used. Exar 48-TQFP and NXP SOT313-2 have the same lead dims (b 0.17-0.27, L 0.45-0.75, tip span 8.8-9.2) and no land of their own.

## `PLCC-68`

```json
{"id": "PLCC-68", "description": "68-pin PLCC, 24.23 x 24.23 mm body, 1.27 mm pitch, pin 1 top centre", "note": "TI FN0068A board layout (R1; 68X (2.39) x 68X (0.65), (23.6) pad-centre rows), controller-uart/tl16c554a.pdf p.48; body 24.23 mid (24.13-24.33) from outline 4215155/A p.47. Same package: TI tl16c754b.pdf p.35, Exar 68-PLCC xr16v554.pdf p.41", "style": "quad", "pins": 68, "body": {"width": 24.23, "height": 24.23}, "sides": [17, 17, 17, 17], "pitch": 1.27, "spanX": 23.6, "spanY": 23.6, "pad": {"width": 2.39, "height": 0.65, "shape": "rect"}, "slots": [60, 61, 62, 63, 64, 65, 66, 67, 68, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59]}
```

**Derivation (R1).** TI FN0068A land (pdf p.48; outline p.47 — the brief's p.46/47 is one page early): 68X (2.39) x 68X (0.65), (23.6) row centres. Body .950-.958 in [24.13-24.33] -> 24.23. Pin 1 top centre: sides 17, top side slots 52-68 run right->left, its middle slot 60 = pin 1, pins 1-9 on slots 60-68, pins 10-68 on slots 1-59 (pin 10 at the top of the left side, as the drawing). Corner pads: left x -12.995..-10.605, top corner pad x -10.485..-9.835: clear.

## `LQFP-64-10X10`

```json
{"id": "LQFP-64-10X10", "description": "64-pin LQFP, 10 x 10 mm body, 0.5 mm pitch, 12.0 mm tip span", "note": "TI PM0064A board layout (R1; 64X (1.5) x 64X (0.3), (11.4) pad-centre rows), controller-uart/tl16c754c.pdf p.54; body 10.0 mid (9.8-10.2) from outline 4215162/A p.53. Same land in MaxLinear POD-00000092 (st16c554.pdf p.36); same outline Exar 64-LQFP xr16v554.pdf p.40, xr16c854.pdf p.50, xr16l784.pdf p.48", "style": "quad", "pins": 64, "body": {"width": 10.0, "height": 10.0}, "sides": [16, 16, 16, 16], "pitch": 0.5, "spanX": 11.4, "spanY": 11.4, "pad": {"width": 1.5, "height": 0.3, "shape": "rect"}}
```

**Derivation (R1).** TI PM0064A land: 64X (1.5) x 64X (0.3), (11.4) centres. Body 10.0 mid (9.8-10.2). MaxLinear POD-00000092 prints the same land.

## `QFN-32-5X5`

```json
{"id": "QFN-32-5X5", "description": "32-pin QFN, 5 x 5 x 0.9 mm body, 0.5 mm pitch, exposed pad 3.8 mm land (Exar/MaxLinear)", "note": "MaxLinear POD-00000037 rev B recommended land pattern (R1; 0.625 x 0.275 pads, 3.80 thermal land), controller-uart/xr20m1280.pdf p.62; pad-centre span 4.8 not dimensioned, read from the drawing's scale (pads reach 0.2 beyond the 5.0 body edge, inner ends at the terminal's inner end, L 0.40 nom); body 5.00 BSC, D2 3.65 mid (3.50-3.80) from outline p.61. Same outline Exar 32-QFN xr16l580.pdf p.47, xr16m2550.pdf p.44, xr16v2550.pdf p.45 (D2 3.50-3.80, b 0.18-0.30, L 0.35-0.45)", "style": "quad", "pins": 32, "body": {"width": 5.0, "height": 5.0}, "sides": [8, 8, 8, 8], "pitch": 0.5, "spanX": 4.8, "spanY": 4.8, "pad": {"width": 0.625, "height": 0.275, "shape": "rect"}, "centre": {"width": 3.8, "height": 3.8, "shape": "rect"}}
```

**Derivation (R1 (span read from scale)).** MaxLinear POD-00000037 land page gives pad 0.625 x 0.275 (32X) and the 3.800 x 3.800 thermal land but no pad-centre dimension. Read from the 150 dpi page (body 5.0 = 293 px): the pads reach ~0.2 beyond the body edge and their inner ends sit ~0.41 inside it (= L 0.40 nom), so centre at ~2.40 -> span 4.8 (outer 2.7125, inner 2.0875; 0.19 clear of the 3.8 land). Thermal 3.80 is the land value (D2 3.65 mid (3.50-3.80) in the outline). Exar 32-QFN outlines in xr16l580/xr16m2550/xr16v2550 match (D 4.90-5.10, D2 3.50-3.80, b 0.18-0.30, L 0.35-0.45). Kept apart from VQFN-32-5X5 / HVQFN-32-5X5 / the TQFN-32s: different pads and exposed pad.

## `VQFN-32-5X5`

```json
{"id": "VQFN-32-5X5", "description": "32-pin VQFN (RHB), 5 x 5 mm body, 0.5 mm pitch, exposed pad 3.45 mm", "note": "TI RHB0032E board layout (R1; 32X (0.6) x 32X (0.25), (4.8) pad-centre rows, ( 3.45) thermal pad), controller-uart/tl16c550d.pdf p.52; body 5.0 mid (4.9-5.1) from outline 4223442/B p.51 (same in tl16c2550.pdf p.46-47, tl16c752c.pdf p.58-59)", "style": "quad", "pins": 32, "body": {"width": 5.0, "height": 5.0}, "sides": [8, 8, 8, 8], "pitch": 0.5, "spanX": 4.8, "spanY": 4.8, "pad": {"width": 0.6, "height": 0.25, "shape": "rect"}, "centre": {"width": 3.45, "height": 3.45, "shape": "rect"}}
```

**Derivation (R1).** TI RHB0032E land: 32X (0.6) x 32X (0.25), (4.8) centres, ( 3.45) thermal. Body 5.0 mid (4.9-5.1). The outline labels the thermal pad 33; per the brief it is the unnumbered `centre`.

## `LQFP-80-12X12`

```json
{"id": "LQFP-80-12X12", "description": "80-pin LQFP, 12 x 12 mm body, 0.5 mm pitch, 14.0 mm tip span", "note": "TI PN0080A board layout (R1; 80X (1.5) x 80X (0.3), (13.4) pad-centre rows), controller-uart/tl16c554a.pdf p.44; body 12.0 mid (11.8-12.2) from outline 4215166/A p.43", "style": "quad", "pins": 80, "body": {"width": 12.0, "height": 12.0}, "sides": [20, 20, 20, 20], "pitch": 0.5, "spanX": 13.4, "spanY": 13.4, "pad": {"width": 1.5, "height": 0.3, "shape": "rect"}}
```

**Derivation (R1).** TI PN0080A land: 80X (1.5) x 80X (0.3), (13.4) centres. Body 12.0 mid (11.8-12.2).

## `PLCC-28`

```json
{"id": "PLCC-28", "description": "28-pin PLCC, 11.51 x 11.51 mm body, 1.27 mm pitch, pin 1 top centre", "note": "Intersil/Renesas N28.45 (JEDEC MS-018AB) outline (R3; lead b 0.43 mid (0.33-0.53), D/E tip span 12.445 mid (12.32-12.57), body D1/E1 11.505 mid (11.43-11.58)), renesas.com/en-us/package-image/pdf/outdrawing/n28.45.pdf (-> renesas.com/en/document/psc/package-drawing-plcc-28pin-n2845) p.1; named by controller-uart/82c52.pdf p.2", "style": "quad", "pins": 28, "body": {"width": 11.51, "height": 11.51}, "sides": [7, 7, 7, 7], "pitch": 1.27, "spanX": 11.445, "spanY": 11.445, "pad": {"width": 2.0, "height": 0.53, "shape": "rect"}, "slots": [25, 26, 27, 28, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24]}
```

**Derivation (R3).** No drawing in the repo PDFs; fetched Renesas/Intersil N28.45 (JEDEC MS-018AB). R3: pad length 2.0; pad width = b 0.43 mid (0.013-0.021 in = 0.33-0.53) + 0.1 = 0.53; span = tip-to-tip D/E 12.445 mid (12.32-12.57) - 1.0 = 11.445 (three decimals like PLCC-44's 16.525). Body D1/E1 11.505 mid (11.43-11.58) -> 11.51. Pin 1 top centre: top slots 22-28, middle slot 25; pins 1-4 on 25-28, pins 5-28 on 1-24. Corner pads clear (left x -6.72..-4.72; top corner pad x -4.075..-3.545). 82510 (Intel spec 231369) is assumed to be the same JEDEC MS-018AB 28-PLCC (only 28-PLCC outline that exists at 0.050 pitch / 0.450 body).

## `QFP-100-14X20`

```json
{"id": "QFP-100-14X20", "description": "100-pin QFP, 14 x 20 mm body, 0.65 mm pitch, 17.9 x 23.9 mm tip span (MS-022)", "note": "Exar 100 Pin QFP 14x20 mm 1.95 mm form (MS-022) outline (R2; lead b 0.31 mid (0.22-0.40), L 0.88 mid (0.73-1.03), D 17.90 BSC, E 23.90 BSC; body D1 14.00 x E1 20.00 BSC), controller-uart/xr16c854.pdf p.52 (same in xr16c864.pdf p.50); pins 1-30 on a long side, so the 20 mm side is vertical. xr16v798.pdf p.53 prints b 0.22-0.38, L 0.65-0.95 (R2 would give 1.60 x 0.35 pads, span 17.3/23.3, same outer extent); xr16c854 values used for all", "style": "quad", "pins": 100, "body": {"width": 14.0, "height": 20.0}, "sides": [30, 20, 30, 20], "pitch": 0.65, "spanX": 17.22, "spanY": 23.22, "pad": {"width": 1.68, "height": 0.36, "shape": "rect"}}
```

**Derivation (R2).** Exar 100-QFP table (rotated on the page): D 17.90 BSC, D1 14.00 BSC, E 23.90 BSC, E1 20.00 BSC, b 0.22-0.40, L 0.73-1.03, e 0.65, ND 20, NE 30. R2: pad width 0.31 + 0.05 = 0.36; pad length 0.88 + 0.8 = 1.68; spanX = 17.90 + 1.0 - 1.68 = 17.22 (across the 14 mm side), spanY = 23.90 + 1.0 - 1.68 = 23.22. xr16v798 top view: pins 1-30 along a 20 mm side, 31-50 on a 14 mm side, CCW; turned 90 deg clockwise so pins 1-30 run down the left: sides [30,20,30,20], body 14 wide x 20 high, pin 1 top-left. The xr16v798 table (b 0.22-0.38, L 0.65-0.95) would give 1.60 x 0.35 pads at 17.30/23.30 (same 18.9 x 24.9 outer extent): within 0.08 mm, one id kept, xr16c854 values (5 of 6 parts).

## `HVQFN-32-5X5`

```json
{"id": "HVQFN-32-5X5", "description": "32-pin HVQFN (SOT617-1), 5 x 5 x 0.85 mm body, 0.5 mm pitch, exposed pad 3.1 mm", "note": "NXP SOT617-1 outline (R4; terminal b 0.24 mid (0.18-0.30), L 0.4 mid (0.3-0.5), terminal-centre span 5.0 - 0.4 = 4.6; exposed pad Dh 3.1 mid (2.95-3.25); body 5.0 mid (4.9-5.1)), controller-uart/sc16is752.pdf p.48 (same in sc16c752b.pdf p.38, sc16c650b.pdf p.41)", "style": "quad", "pins": 32, "body": {"width": 5.0, "height": 5.0}, "sides": [8, 8, 8, 8], "pitch": 0.5, "spanX": 4.9, "spanY": 4.9, "pad": {"width": 0.7, "height": 0.29, "shape": "rect"}, "centre": {"width": 3.1, "height": 3.1, "shape": "rect"}}
```

**Derivation (R4).** NXP SOT617-1 (no land printed): pad length L 0.4 mid (0.3-0.5) + 0.3 = 0.7; pad width b 0.24 mid (0.18-0.30) + 0.05 = 0.29; terminal-centre span = D 5.0 - L 0.4 = 4.6, + 0.3 = 4.9; centre Dh 3.1 mid (2.95-3.25). Pad inner end 2.1 vs thermal edge 1.55.

## `QFN-24-4X4`

```json
{"id": "QFN-24-4X4", "description": "24-pin QFN/HVQFN, 4 x 4 mm body, 0.5 mm pitch, exposed pad 2.7 mm land", "note": "MaxLinear POD-00000130 rev B typical recommended land pattern (R1; 0.65 x 0.30 pads, 2.70 thermal land, 0.20 gap thermal-to-pad, so pad centres at 1.35 + 0.20 + 0.325 = 1.875), controller-uart/xr20m1170.pdf p.54; body 4.00 BSC, D2 2.65 mid (2.50-2.80) from outline p.53. NXP SOT616-3 (sc16is750.pdf p.50: D 3.9-4.1, Dh 2.45-2.75, b 0.18-0.30, L 0.3-0.5) is the same outline", "style": "quad", "pins": 24, "body": {"width": 4.0, "height": 4.0}, "sides": [6, 6, 6, 6], "pitch": 0.5, "spanX": 3.75, "spanY": 3.75, "pad": {"width": 0.65, "height": 0.3, "shape": "rect"}, "centre": {"width": 2.7, "height": 2.7, "shape": "rect"}}
```

**Derivation (R1).** MaxLinear POD-00000130 land: pads 0.65 x 0.30 at 0.50 pitch, thermal 2.70, 0.20 between thermal edge and pad inner ends -> pad centre 1.35 + 0.20 + 0.325 = 1.875, span 3.75 (outer end 2.20, 0.20 beyond the body, as the drawing's other 0.20). The NXP SOT616-3 outline (sc16is750 p.50) is the same 4x4/0.5 package (R4 would give 0.7 x 0.29 at 3.9, centre 2.6); one id per the brief, MaxLinear land used. Not merged with TQFN-24-4X4-ZD (2.30 land, 3.86 span) or TQFN-24-4X4-T2444 (2.60 land, 0.80 pads, 3.91): different land patterns.

## `TSSOP-28`

```json
{"id": "TSSOP-28", "description": "28-pin TSSOP (SOT361-1), 4.4 x 9.7 mm body, 0.65 mm pitch", "note": "NXP SOT361-1 outline (R2; lead bp 0.245 mid (0.19-0.30), Lp 0.625 mid (0.50-0.75), HE 6.4 mid (6.2-6.6); pad length 1.425 -> 1.43 and span 5.975 -> 5.97 keep the 7.4 outer extent, as TVSOP-14; body E 4.4 mid (4.3-4.5) x D 9.7 mid (9.6-9.8)), controller-uart/sc16is752.pdf p.47", "style": "dual", "pins": 28, "body": {"width": 4.4, "height": 9.7}, "pitch": 0.65, "span": 5.97, "pad": {"width": 1.43, "height": 0.3, "shape": "rect"}}
```

**Derivation (R2).** NXP SOT361-1 (no land): pad width bp 0.245 mid + 0.05 = 0.295 -> 0.30; pad length Lp 0.625 mid + 0.8 = 1.425; span = HE 6.4 mid + 1.0 - 1.425 = 5.975. Rounded as TVSOP-14/16/20 (same 6.2-6.6 / 0.50-0.75 leads): 1.43 / 5.97, keeping the 7.4 outer extent. Body 4.4 x 9.7 mids.

## `QSOP-16`

```json
{"id": "QSOP-16", "description": "16-pin QSOP, 3.9 x 4.89 mm body, 0.635 mm pitch", "note": "Maxim/ADI 90-0167 rev H land pattern [E16] (R1; 1.60 x 0.36 pads, 5.31 pad-centre span), analog.com/media/en/package-pcb-resources/land-pattern/qsop/90-0167.pdf p.1; body E 3.90 mid (3.81-3.99) x D 4.89 mid (4.80-4.98) from outline 21-0055 rev S, analog.com/media/en/package-pcb-resources/package/pkg_pdf/qsop/21-0055.pdf p.1; MAX3100 code E16-1 named in controller-uart/max3100.pdf p.23", "style": "dual", "pins": 16, "body": {"width": 3.9, "height": 4.89}, "pitch": 0.635, "span": 5.31, "pad": {"width": 1.6, "height": 0.36, "shape": "rect"}}
```

**Derivation (R1).** Maxim/ADI 90-0167 rev H (codes include E16+1, the Pb-free form of MAX3100's E16-1): 1.60 x 0.36 pads, 0.635 pitch, 5.31 pad-centre span. Body from 21-0055 rev S: E 0.150-0.157 in (3.81-3.99) -> 3.90, D 16-pin 4.80-4.98 -> 4.89.

## `QFN-48-7X7`

```json
{"id": "QFN-48-7X7", "description": "48-pin QFN, 7 x 7 x 0.9 mm body, 0.5 mm pitch, exposed pad 5.2 mm (Exar)", "note": "Exar 48 lead QFN 7x7x0.9 outline (R4; terminal b 0.24 mid (0.18-0.30), L 0.40 mid (0.30-0.50), D 7.00 mid (6.85-7.15), terminal-centre span 6.6; exposed pad D2 5.20 mid (5.10-5.30)), controller-uart/xr16v554.pdf p.39 (same in xr16v564.pdf p.50, xr16v654.pdf p.51)", "style": "quad", "pins": 48, "body": {"width": 7.0, "height": 7.0}, "sides": [12, 12, 12, 12], "pitch": 0.5, "spanX": 6.9, "spanY": 6.9, "pad": {"width": 0.7, "height": 0.29, "shape": "rect"}, "centre": {"width": 5.2, "height": 5.2, "shape": "rect"}}
```

**Derivation (R4).** Exar 48-QFN table (no land): L 0.30-0.50 -> 0.40 + 0.3 = 0.7 pad length; b 0.18-0.30 -> 0.24 + 0.05 = 0.29; terminal-centre span D 7.00 mid (6.85-7.15) - 0.40 = 6.6, + 0.3 = 6.9; centre D2 5.20 mid (5.10-5.30). Pad inner end 3.1 vs thermal edge 2.6.

## `TQFN-32-5X5-T3255`

```json
{"id": "TQFN-32-5X5-T3255", "description": "32-pin TQFN-EP (Maxim T3255+4), 5 x 5 x 0.75 mm body, 0.5 mm pitch, exposed pad 3.15 mm land", "note": "Maxim/ADI 90-0012 rev E land pattern [T3255-4/T3255+4] (R1; 0.80 x 0.30 pads, 4.83 pad-centre span, 3.15 thermal land), analog.com/media/en/package-pcb-resources/land-pattern/tqfn/90-0012.pdf p.1; body 5.00 BSC from outline 21-0140 rev AM p.2, analog.com/media/en/package-pcb-resources/package/pkg_pdf/tqfn/21-0140.pdf; code named in controller-uart/max3109.pdf p.7", "style": "quad", "pins": 32, "body": {"width": 5.0, "height": 5.0}, "sides": [8, 8, 8, 8], "pitch": 0.5, "spanX": 4.83, "spanY": 4.83, "pad": {"width": 0.8, "height": 0.3, "shape": "rect"}, "centre": {"width": 3.15, "height": 3.15, "shape": "rect"}}
```

**Derivation (R1).** Maxim 90-0012 rev E (codes T3255-4/T3255+4): pads 0.80 x 0.30, 0.50 pitch, 4.83 pad-centre span both axes (dimension lines at the pad centres, checked on the 170 dpi render), 3.15 thermal. Outline 21-0140: D/E 5.00 BSC, 32L b 0.20-0.30, L 0.30-0.50. Not merged with TQFN-32-5X5-ZH (Diodes: 0.70 x 0.27, 4.80, 3.25).

## `TQFN-24-3.5X3.5`

```json
{"id": "TQFN-24-3.5X3.5", "description": "24-pin TQFN-EP (Maxim T243A3+1), 3.5 x 3.5 x 0.75 mm body, 0.4 mm pitch, exposed pad 2.4 mm land", "note": "Maxim/ADI 90-0122 rev C land pattern [T243A3-1/T243A3+1] (R1; 0.70 x 0.20 pads, 0.40 pitch, 3.50 pad-centre span, 2.40 thermal land), analog.com/media/en/package-pcb-resources/land-pattern/tqfn/90-0122.pdf p.1; body 3.50 mid (3.40-3.60), e 0.40 BSC from outline 21-0188 rev E p.2, analog.com/media/en/package-pcb-resources/package/pkg_pdf/tqfn/21-0188.pdf; code named in controller-uart/max3107.pdf p.51", "style": "quad", "pins": 24, "body": {"width": 3.5, "height": 3.5}, "sides": [6, 6, 6, 6], "pitch": 0.4, "spanX": 3.5, "spanY": 3.5, "pad": {"width": 0.7, "height": 0.2, "shape": "rect"}, "centre": {"width": 2.4, "height": 2.4, "shape": "rect"}}
```

**Derivation (R1).** Maxim 90-0122 rev C (T243A3-1/+1): pads 0.70 x 0.20, pitch 0.40 (the brief had pitch unknown; 21-0188 e 0.40 BSC confirms), 3.50 pad-centre span, 2.40 thermal. Outline 21-0188 rev E: D/E 3.40-3.60 -> 3.50, b 0.15-0.25, L 0.30-0.40, D2 2.20-2.40.

## `TQFN-48-7X7`

```json
{"id": "TQFN-48-7X7", "description": "48-pin TQFN-EP (Maxim T4877+3), 7 x 7 x 0.75 mm body, 0.5 mm pitch, exposed pad 5.18 mm land", "note": "Maxim/ADI 90-0129 rev C land pattern [T4877-3/T4877+3] (R1; 0.85 x 0.30 pads, 6.75 pad-centre span, 5.18 thermal land; the drawing's 0.09 corner cut on the end pads is left out, the pads do not touch without it), analog.com/media/en/package-pcb-resources/land-pattern/tqfn/90-0129.pdf p.1; body 7x7 from outline 21-0144 rev Y p.2, analog.com/media/en/package-pcb-resources/package/pkg_pdf/tqfn/21-0144.pdf; code named in controller-uart/max14830.pdf p.67", "style": "quad", "pins": 48, "body": {"width": 7.0, "height": 7.0}, "sides": [12, 12, 12, 12], "pitch": 0.5, "spanX": 6.75, "spanY": 6.75, "pad": {"width": 0.85, "height": 0.3, "shape": "rect"}, "centre": {"width": 5.18, "height": 5.18, "shape": "rect"}}
```

**Derivation (R1).** Maxim 90-0129 rev C (T4877-3/+3): pads 0.85 x 0.30, 0.50 pitch, 6.75 pad-centre span, 5.18 thermal. The end pads of each row have a 0.09 corner cut; a quad `pad` can only cut every pad alike, so it is left out (left corner pad x -3.80..-2.95, top corner pad x -2.90..-2.60: 0.05 clear without the cut). Body 7x7 (21-0144 title, 7x7x0.75).

## `QSOP-28`

```json
{"id": "QSOP-28", "description": "28-pin QSOP, 3.9 x 9.89 mm body, 0.635 mm pitch", "note": "Maxim 90-0173 rev A land pattern [E28/E28M] (R1; 0.063 in (1.600) x 0.014 in (0.356) pads, 0.209 in (5.309) pad-centre span), analog.com/media/en/package-pcb-resources/land-pattern/qsop/90-0173.pdf p.1; body E 3.90 mid (3.81-3.99) x D 9.89 mid (9.80-9.98) from outline 21-0055 rev S, analog.com/media/en/package-pcb-resources/package/pkg_pdf/qsop/21-0055.pdf p.1; code E28M+1 named in controller-uart/max3140.pdf p.35", "style": "dual", "pins": 28, "body": {"width": 3.9, "height": 9.89}, "pitch": 0.635, "span": 5.31, "pad": {"width": 1.6, "height": 0.36, "shape": "rect"}}
```

**Derivation (R1).** Maxim 90-0173 rev A (E28-1/E28M-1, +): 0.063 in [1.600] x 0.014 in [0.356] pads, 0.025 in [0.635] pitch, 0.209 in [5.309] pad-centre span; written 1.6 x 0.36 at 5.31 (same as the newer 90-0167 for 16 pins). Body from 21-0055: E 3.90 mid, D 28-pin 9.80-9.98 -> 9.89.

## `PDIP-48-600`

```json
{"id": "PDIP-48-600", "description": "48-pin 600 mil PDIP, 13.59 x 62.15 mm body, 2.54 mm pitch", "note": "SMC 48 LEAD PLASTIC DIP PACKAGE OUTLINE (R5; lead D 0.47 mid (0.016-0.021 in); drill 0.47 + 0.25 = 0.72 -> 0.75; span 15.24 = 0.600 in rows (not printed; L 0.610-0.675 over the splayed leads); body B 0.535 in (13.59) mid (0.520-0.550) x A 2.447 in (62.15) mid (2.437-2.457)), controller-uart/com78804.pdf p.15", "style": "dual", "pins": 48, "body": {"width": 13.59, "height": 62.15}, "pitch": 2.54, "span": 15.24, "pad": {"width": 1.5, "height": 1.5, "shape": "round"}, "drill": 0.75, "pin1Shape": "rect"}
```

**Derivation (R5).** SMC outline, inches: D (lead) 0.016-0.021 -> 0.0185 in = 0.47 -> drill 0.72 -> 0.75 (rounded up to 0.05); pad 0.75 + 0.7 = 1.45 -> 1.5 (minimum); pitch G 0.100 nom (0.090-0.110); span 15.24 (600 mil, the heading's row; the drawing prints E 0.520-0.630 and L 0.610-0.675 only); body B 0.535 in -> 13.59, A 2.447 in -> 62.15.

## `CDIP-40-600`

```json
{"id": "CDIP-40-600", "description": "40-pin 600 mil ceramic DIP, 14.35 x 53.24 mm body, 2.54 mm pitch", "note": "Intersil/Renesas D40.6 (MIL-STD-1835 CDIP2-T40) 40 lead ceramic DIP outline (R5; lead b 0.51 mid (0.36-0.66), drill 0.76 -> 0.8; eA 15.24 BSC; body E 14.35 mid (12.95-15.75) x D 53.24 (max only)), renesas.com/en-us/package-image/pdf/outdrawing/d40.6.pdf (-> renesas.com/en/document/psc/d406-40-lead-ceramic-dual-line-metal-seal-package) p.1; CDP1854A D suffix = hermetic ceramic DIP (controller-uart/cdp1854a.pdf p.1). The CERDIP F40.6 (same site, f40.6.pdf) has the same leads and body", "style": "dual", "pins": 40, "body": {"width": 14.35, "height": 53.24}, "pitch": 2.54, "span": 15.24, "pad": {"width": 1.5, "height": 1.5, "shape": "round"}, "drill": 0.8, "pin1Shape": "rect"}
```

**Derivation (R5).** No drawing in the repo PDF (cdp1854a.pdf p.1: "40-lead hermetic ceramic DIP", D suffix). Fetched Renesas/Intersil D40.6 (CDIP2-T40 side-brazed, the Harris/RCA D-suffix package): b 0.36-0.66 -> 0.51 -> drill 0.76 -> 0.8; pad 1.5; eA 15.24 BSC; body E 12.95-15.75 -> 14.35, D 53.24 max only (used as printed). The CERDIP F40.6 drawing has identical numbers, so either reading of "hermetic ceramic DIP" gives this footprint. Not merged with PDIP-40-600 (same pads, different body), as CDIP-28-600 vs PDIP-28-600.

## `BGA-24-MICROSTAR`

```json
{"id": "BGA-24-MICROSTAR", "description": "24-ball MicroStar Junior BGA (ZQS/GQS), 5 x 5 grid less B2, 3 x 3 mm, 0.5 mm pitch", "note": "TI GQS (S-PBGA-N24) MECHANICAL DATA MPBG127B, drawing 4201012/D (R6; ball 0.30 mid (0.25-0.35) -> 0.24 -> 0.25; body 3.00 mid (2.90-3.10) SQ; 0.50 pitch), in TI SN74AVCA406L datasheet SCES634B p.16 (ZQS = Pb-free GQS, same drawing; copy fetched from s1.dtsheet.com/store/data/000042887.pdf); ball map (B2 empty) controller-uart/tl16c550d.pdf p.2 and p.12", "style": "grid", "pins": 24, "body": {"width": 3.0, "height": 3.0}, "rows": 5, "columns": 5, "pitch": 0.5, "pad": {"width": 0.25, "height": 0.25, "shape": "round"}, "balls": ["A1", "A2", "A3", "A4", "A5", "B1", "B3", "B4", "B5", "C1", "C2", "C3", "C4", "C5", "D1", "D2", "D3", "D4", "D5", "E1", "E2", "E3", "E4", "E5"]}
```

**Derivation (R6).** No ZQS drawing in tl16c550d.pdf; TI's ZQS is the Pb-free GQS (same TI drawing; SN74AVCA406L orders LGQSR/LZQSR on one MPBG127B outline). GQS (S-PBGA-N24) 4201012/D: body 2.90-3.10 SQ -> 3.00, ball 0.25-0.35 -> 0.30, 0.50 pitch, 5 x 5 grid (A-E, 1-5). R6: 0.8 x 0.30 = 0.24 -> 0.25. Balls: all 25 less B2 (tl16c550d p.12 terminal table: B1 D7, B3 D3, B4 D1, B5 MR; no B2). Pitch 0.5 now confirmed by the drawing (the brief had it assumed). Source copy is TI's own datasheet SCES634B served by dtsheet.com (ti.com no longer hosts SN74AVCA406L); same kind of external TI copy as VQFN-20-RGY.

## `TQFN-32-5X5-ZH`

```json
{"id": "TQFN-32-5X5-ZH", "description": "32-pin TQFN (Diodes ZH, W-QFN5050-32), 5 x 5 x 0.75 mm body, 0.5 mm pitch, exposed pad 3.25 mm", "note": "Diodes/Pericom PD-2070 rev E recommended land pattern (R1; 0.70 x 0.27 pads, 4.10 between the inner pad ends so 4.80 pad-centre span, 3.25 thermal land), controller-uart/pi7c9x762.pdf p.48; body 5.00 mid (4.90-5.10), D2 3.25 mid (3.15-3.35) from the outline on the same page", "style": "quad", "pins": 32, "body": {"width": 5.0, "height": 5.0}, "sides": [8, 8, 8, 8], "pitch": 0.5, "spanX": 4.8, "spanY": 4.8, "pad": {"width": 0.7, "height": 0.27, "shape": "rect"}, "centre": {"width": 3.25, "height": 3.25, "shape": "rect"}}
```

**Derivation (R1).** Diodes PD-2070 rev E land: 0.70(32X) x 0.27(32X), 0.50 pitch, 3.25 x 3.25 thermal, and 4.10 on both axes. On the 200 dpi render the 4.10 dimension runs between the inner ends of opposite pads (left pads x 1008-1047 px, line from 1047 to 1268 px; 3.25 land = 175 px, so 4.10 = 221 px), so span = 4.10 + 0.70 = 4.80 (outer 2.75, 0.25 beyond the body). Body 5.00 mid (4.90-5.10).

## `TQFN-24-4X4-ZD`

```json
{"id": "TQFN-24-4X4-ZD", "description": "24-pin TQFN (Diodes ZD), 4 x 4 x 0.85 mm body, 0.5 mm pitch, exposed pad 2.3 mm land", "note": "Diodes/Pericom PD-2100 rev C recommended land pattern (R1; 24X 0.70 x 0.28 pads, 3.86 pad-centre span, 2.30 x 2.30 thermal land), controller-uart/pi7c9x760.pdf p.47; body 4.00 mid (3.90-4.10) from the outline on the same page", "style": "quad", "pins": 24, "body": {"width": 4.0, "height": 4.0}, "sides": [6, 6, 6, 6], "pitch": 0.5, "spanX": 3.86, "spanY": 3.86, "pad": {"width": 0.7, "height": 0.28, "shape": "rect"}, "centre": {"width": 2.3, "height": 2.3, "shape": "rect"}}
```

**Derivation (R1).** Diodes PD-2100 rev C land: 24X 0.70 x 0.28, 0.50 BSC, 2.30 x 2.30 thermal, 3.86 both axes. Here the 3.86 lines end at the pad centres (left pads 1013-1052 px, line from 1033 px; right 1232-1271 px, line to 1250 px; 2.30 = 130 px -> 3.86 = 218 px), so span = 3.86 (outer 2.28, 0.28 beyond the body; the ZH reading gives the same overhang). Body 4.00 mid (3.90-4.10).

## `TQFN-24-4X4-T2444`

```json
{"id": "TQFN-24-4X4-T2444", "description": "24-pin TQFN-EP (Maxim T2444-4), 4 x 4 x 0.75 mm body, 0.5 mm pitch, exposed pad 2.6 mm land", "note": "Maxim/ADI 90-0022 rev E land pattern [T2444-4] (R1; 0.80 x 0.30 pads, 3.91 pad-centre span, 2.60 thermal land), analog.com/media/en/package-pcb-resources/land-pattern/tqfn/90-0022.pdf p.1; body 4.00 mid (3.90-4.10), D2 2.54 mid (2.45-2.63) from outline 21-0139 rev V p.2-3, analog.com/media/en/package-pcb-resources/package/pkg_pdf/tqfn/21-0139.pdf; code named in controller-uart/max3100.pdf p.23", "style": "quad", "pins": 24, "body": {"width": 4.0, "height": 4.0}, "sides": [6, 6, 6, 6], "pitch": 0.5, "spanX": 3.91, "spanY": 3.91, "pad": {"width": 0.8, "height": 0.3, "shape": "rect"}, "centre": {"width": 2.6, "height": 2.6, "shape": "rect"}}
```

**Derivation (R1).** No drawing in max3100.pdf (outline 21-0139 named, no land). Fetched 21-0139 rev V (T2444-4: D 3.90-4.10, e 0.50, b 0.18-0.30, L 0.30-0.50, D2 2.45-2.63) and land 90-0022 rev E, whose code box lists [T2444-4]: pads 0.80 x 0.30, 3.91 pad-centre span (lines at the pad centres), 2.60 thermal. (90-0021 is for T2444-3.)

## `WLP-25`

```json
{"id": "WLP-25", "description": "25-bump WLP (Maxim W252B2+1), 5 x 5 grid, 2.07 x 2.07 mm, 0.4 mm pitch", "note": "Maxim/ADI 21-0180 rev G 25-bump WLP outline (R6; bump b 0.26 nom (+/-0.03) -> 0.208 -> 0.20; body W252B2+1 2.065 +/-0.075 -> 2.07; e 0.40 BASIC), analog.com/media/en/package-pcb-resources/package/pkg_pdf/wlp/21-0180.pdf p.1 (land pattern: \"Refer to Application Note 1891\", none drawn); ball map controller-uart/max3108.pdf p.13-14", "style": "grid", "pins": 25, "body": {"width": 2.07, "height": 2.07}, "rows": 5, "columns": 5, "pitch": 0.4, "pad": {"width": 0.2, "height": 0.2, "shape": "round"}, "balls": ["A1", "A2", "A3", "A4", "A5", "B1", "B2", "B3", "B4", "B5", "C1", "C2", "C3", "C4", "C5", "D1", "D2", "D3", "D4", "D5", "E1", "E2", "E3", "E4", "E5"]}
```

**Derivation (R6).** 21-0180 rev G: bump b 0.26 +/-0.03 (nom 0.26) -> 0.8 x 0.26 = 0.208 -> 0.20; W252B2+1 D/E 2.065 +/-0.075 -> 2.07; e 0.40 BASIC; 5 x 5 bumps A1-E5, none depopulated. No land drawing (Application Note 1891 is general guidance), so R6.

## `SOP-24-24P2W` — unresolved, no JSON (NOT ADDED: M66230 is left out, plan-12-A §4.1)

M66230FP (Renesas, ex-Mitsubishi) names outline 24P2W-A only (m66230.pdf p.1; the repo PDF is a scan with no outline page). Tried: renesas.com package-image `outdrawing/24p2w-a.pdf` (404), Renesas package pages (pkg8706 = PRSP0024DA-A / 24P2N-B; no page lists 24P2W-A), web searches for "24P2W-A". No maker drawing found, so no footprint is given: the body width (the "300 mil" in the brief is the row class, not a measured value) and lead length are unknown, and SOIC-24W (TI DW, 7.5 mm body) cannot be assumed. Options for the plan: (a) leave M66230FP#TB0Q out until a 24P2W-A drawing is found (one part), or (b) find the Renesas PRSP0024xx-A code whose "previous code" is 24P2W-A and use its psc drawing.

## Probe

Build (once):

```
cmake -S /Users/justinzaun/Development/logic/backend -B /tmp/p11/fp/build -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/p11/fp/build -j8
```

Copy of the repo `footprints.json` (195 footprints) with the 24 objects above appended (`/tmp/p11/fp/new.json`, generated by `/tmp/p11/fp/gen/fps.py`), loaded with the real `load_footprints()`; `/tmp/p11/fp/probe/probe.c` prints each new footprint (pins, expanded pad count, body, pad extent, pin-1 pad position) and tests every pad pair for overlap (strict, 1e-9 tolerance):

```
cd /tmp/p11/fp/probe
python3 -I -c "import json; d=json.load(open('/Users/justinzaun/Development/logic/schematical/schematical/Parts/footprints.json')); d['footprints']+=json.load(open('/tmp/p11/fp/new.json')); json.dump(d,open('fp.json','w'),indent=1)"
cc -O1 -I/Users/justinzaun/Development/logic/backend/include probe.c /tmp/p11/fp/build/liblogic.a -lm -o probe
./probe fp.json 195
```

Output (exit 0):

```
ok 219 footprints loaded
TQFP-48-7X7          pins=48  pads=48  body=7x7 extent=9.850x9.850 overlaps=0 pad1=(-4.25,-2.75)
PLCC-68              pins=68  pads=68  body=24.23x24.23 extent=25.990x25.990 overlaps=0 pad1=(0,-11.8)
LQFP-64-10X10        pins=64  pads=64  body=10x10 extent=12.900x12.900 overlaps=0 pad1=(-5.7,-3.75)
QFN-32-5X5           pins=32  pads=33  body=5x5 extent=5.425x5.425 overlaps=0 pad1=(-2.4,-1.75)
VQFN-32-5X5          pins=32  pads=33  body=5x5 extent=5.400x5.400 overlaps=0 pad1=(-2.4,-1.75)
LQFP-80-12X12        pins=80  pads=80  body=12x12 extent=14.900x14.900 overlaps=0 pad1=(-6.7,-4.75)
PLCC-28              pins=28  pads=28  body=11.51x11.51 extent=13.445x13.445 overlaps=0 pad1=(0,-5.7225)
QFP-100-14X20        pins=100 pads=100 body=14x20 extent=18.900x24.900 overlaps=0 pad1=(-8.61,-9.425)
HVQFN-32-5X5         pins=32  pads=33  body=5x5 extent=5.600x5.600 overlaps=0 pad1=(-2.45,-1.75)
QFN-24-4X4           pins=24  pads=25  body=4x4 extent=4.400x4.400 overlaps=0 pad1=(-1.875,-1.25)
TSSOP-28             pins=28  pads=28  body=4.4x9.7 extent=7.400x8.750 overlaps=0 pad1=(-2.985,-4.225)
QSOP-16              pins=16  pads=16  body=3.9x4.89 extent=6.910x4.805 overlaps=0 pad1=(-2.655,-2.2225)
QFN-48-7X7           pins=48  pads=49  body=7x7 extent=7.600x7.600 overlaps=0 pad1=(-3.45,-2.75)
TQFN-32-5X5-T3255    pins=32  pads=33  body=5x5 extent=5.630x5.630 overlaps=0 pad1=(-2.415,-1.75)
TQFN-24-3.5X3.5      pins=24  pads=25  body=3.5x3.5 extent=4.200x4.200 overlaps=0 pad1=(-1.75,-1)
TQFN-48-7X7          pins=48  pads=49  body=7x7 extent=7.600x7.600 overlaps=0 pad1=(-3.375,-2.75)
QSOP-28              pins=28  pads=28  body=3.9x9.89 extent=6.910x8.615 overlaps=0 pad1=(-2.655,-4.1275)
PDIP-48-600          pins=48  pads=48  body=13.59x62.15 extent=16.740x59.920 overlaps=0 pad1=(-7.62,-29.21)
CDIP-40-600          pins=40  pads=40  body=14.35x53.24 extent=16.740x49.760 overlaps=0 pad1=(-7.62,-24.13)
BGA-24-MICROSTAR     pins=24  pads=24  body=3x3 extent=2.250x2.250 overlaps=0 pad1=(-1,-1)
TQFN-32-5X5-ZH       pins=32  pads=33  body=5x5 extent=5.500x5.500 overlaps=0 pad1=(-2.4,-1.75)
TQFN-24-4X4-ZD       pins=24  pads=25  body=4x4 extent=4.560x4.560 overlaps=0 pad1=(-1.93,-1.25)
TQFN-24-4X4-T2444    pins=24  pads=25  body=4x4 extent=4.710x4.710 overlaps=0 pad1=(-1.955,-1.25)
WLP-25               pins=25  pads=25  body=2.07x2.07 extent=1.800x1.800 overlaps=0 pad1=(-0.8,-0.8)
total overlaps 0
```

Pad counts: quads with a `centre` show pins + 1 (the unnumbered exposed pad). PLCC-68 / PLCC-28 pin 1 is at x = 0 on the top row (top centre). QFP-100 pin 1 is top-left with the 20 mm side vertical.

## id changes

- **No merges.** The candidate pairs were checked and are different land patterns: QFN-24-4X4 (MaxLinear, 0.65 x 0.30 @ 3.75, 2.70 land) vs TQFN-24-4X4-ZD (Diodes, 0.70 x 0.28 @ 3.86, 2.30) vs TQFN-24-4X4-T2444 (Maxim, 0.80 x 0.30 @ 3.91, 2.60); the five 5x5 32-pad QFNs (QFN-32-5X5 3.8 land, VQFN-32-5X5 3.45, HVQFN-32-5X5 3.1, TQFN-32-5X5-ZH 3.25 @ 0.70 x 0.27, TQFN-32-5X5-T3255 3.15 @ 0.80 x 0.30) all differ in pads or exposed pad; CDIP-40-600 has PDIP-40-600's pads but its own body (precedent CDIP-28-600 / PDIP-28-600).
- **No splits.** QFP-100-14X20 stays one id (xr16v798's leads give pads within 0.08 mm, same outer extent; see its note). QFN-24-4X4 keeps both the MaxLinear and NXP SOT616-3 parts (same outline).
- `SOP-24-24P2W` is not added: M66230 is left out (no maker drawing), plan-12-A §4.1.
- Corrections to `footprints-needed.md`: PLCC-68 drawings are tl16c554a.pdf p.47 (outline) / p.48 (land), not p.46/47. TQFN-24-3.5X3.5 pitch is 0.40 (21-0188), exposed land 2.40. BGA-24-MICROSTAR pitch 0.5 is now from a TI drawing (GQS 4201012/D), ball 0.25-0.35. TQFN-32-5X5-T3255 exposed land is 3.15 (90-0012).
