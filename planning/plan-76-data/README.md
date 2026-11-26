# Planning census for plans 76–82 (2026-11-25)

Seven plans, one per hobby part type the owner asked for on 2026-11-25:

| Plan | Part type | Kind |
|---|---|---|
| 76 | sensors (board-mount ICs, 7 families) | new `sensor` |
| 77 | STM32 microcontrollers | existing `mcu`, facts only |
| 78 | potentiometers (rotary, trimmer, slide, thumbwheel) | new `potentiometer` |
| 79 | battery holders, clips and contacts | existing `connector`, new `"battery"` mode |
| 80 | addressable LEDs (WS2812-class) | new `smartled`, simulated |
| 81 | OLED displays (graphic and character) | new `oled` (simulated, SSD1306 class) + `charlcd` |
| 82 | indicator LEDs | existing `led` |

All counts: DigiKey through Safari (`scripts/safari-digikey.sh`, AGENTS.md), with the plan-69
census tools (`planning/plan-69-data/census/`: `url.js`, `facets.sh`, `show.py`, `kw.js`; they are
used as they are). Every page was filtered **In Stock** (`-2` = 5), **Active** (`1989` = 0) and
**Exclude Marketplace Products** (`-9` = 1). The Marketplace exclusion is new in this census: it
drops the Marketplace-only rows that the In Stock facet otherwise counts (plan-58-A, plan-74-A
§ 2). Checked on `m105.json`: of its first 100 rows, 98 show only an `In Stock` line, 1 `In Stock`
+ `Alternate Stock`, 1 `In Stock` + `Factory`, none Marketplace. The A letter of each plan still
checks each part's own `In Stock` line.

Raw pages: `raw/<file>.json` (facets and `productCount`; files marked † keep their product rows).
Show one: `python3 -I planning/plan-69-data/census/show.py planning/plan-76-data/raw/<file>.json Manufacturer`.

"Board mount" below = Mounting Type `69` ∈ {Surface Mount 409393, Through Hole 411897,
Surface Mount Wettable Flank 409430}. A category without a Mounting Type facet (Gyroscopes 555) is
taken whole.

## Plan 76: sensors

| Family | Category (id) | Filter | Total | Top makers (board mount, no Marketplace) | file |
|---|---|---|---|---|---|
| temperature | Temperature Sensors - Analog and Digital Output (518) | board mount | 907 | ADI 324 (Maxim 215 + ADI 109); TI 258; Microchip 181; NXP 27 / Melexis 27 (tie) | b518 |
| — tie-break | 518, maker NXP (568) / Melexis (413) | board mount | 27 / 27 | summed In Stock: NXP 244,014; Melexis 30,016 → **NXP** | t568†, t413† |
| humidity | Humidity, Moisture Sensors (529) | board mount | 104 | Sensirion 49; Amphenol Telaire 21; TE Measurement Specialties 13; Innovative Sensor Technology 9 | b529 |
| pressure | Pressure Sensors, Transducers (512) | board mount | 1,230 | Amphenol All Sensors 411; Honeywell 406; ST 104; Amphenol NovaSensor 93; TE 73 | b512 |
| motion | Accelerometers (515) + Gyroscopes (555) + IMUs (567) | board mount (555 whole) | 133 + 18 + 42 | ADI 90 (68 + 13 + 9); ST 28 (15 + 2 + 11); TE 22 (22 + 0 + 0); Memsic 10; Bosch 8 (3 + 0 + 5); TDK InvenSense 6 (0 + 2 + 4) | b515, m555, b567 |
| magnetic | Linear, Compass ICs (554) + Switches, Solid State (565) | board mount | 372 + 1,012 | Allegro 291 (83 + 208); TI 282 (121 + 161); Diodes 225 (30 + 195); Infineon 127 (37 + 90); ABLIC 112; Melexis 112 (36 + 76) | b554, b565 |
| light | Ambient Light, IR, UV Sensors (536) | board mount | 94 | ams OSRAM 26; Vishay 22; Broadcom 12; TI 11 | b536 |
| current | Current Sensors (525) | board mount | 901 | Allegro 236; LEM 166; TI 108; Melexis 69; MPS 65 | b525 |

Not in the board-mount pools: Proximity Sensors (524) has no board-mount row (`b524`: 0; its 764
are housed industrial sensors, `m524`). Gas (530), color (539), flow, force, level and the
industrial categories are not families of plan-76 (owner decision, § Decisions of plan-76-A).

Unfiltered for contrast (`m*.json`, no Mounting Type filter): 518's top maker before the
Marketplace exclusion was Automation Components 852 (`c518` at planning, not kept), all of them
Marketplace or housed probes.

## Plan 77: STM32

| What | Category | Filter | Total | file |
|---|---|---|---|---|
| ST microcontrollers | Embedded - Microcontrollers (685) | maker ST (497) | 1,623 | st |
| …of them STM32 (Series starting `STM32`) | | | **1,421**: F4 157, F0 146, F1 144, L4 143, L0 117, U5 107, G0 92, F3 86, C0 76, H7 64, F7 60, F2 56, L1 45, U3 32, U0 28, G4 24, L5 17, H5 14, C5 8, N6 5 | st |
| the rest (out) | | | STM8 132, SPC5 35, `-` 35 | st |

Pinout source: STMicroelectronics/STM32_open_pin_data on GitHub, BSD-3-Clause
(`gh api repos/STMicroelectronics/STM32_open_pin_data --jq .license.spdx_id` → `BSD-3-Clause`;
`mcu/` holds one XML per die and package, e.g. `STM32C011F(4-6)Px.xml`; listing capped at 1000
entries by the contents API, so the A letter clones the repository).

## Plan 78: potentiometers

| Category (id) | Total | Bourns | Amphenol Piher | Vishay Sfernice | Same Sky | TT Electronics/BI | Nidec | file |
|---|---|---|---|---|---|---|---|---|
| Rotary Potentiometers, Rheostats (84) | 2,744 | 1,014 | 211 | 436 | 221 | 138 | — | m84 |
| Trimmer Potentiometers (80) | 4,053 | 2,073 | 633 | 369 | 222 | 261 | 318 | m80 |
| Slide Potentiometers (78) | 136 | 108 | — | — | 14 | 9 | — | m78 |
| Thumbwheel Potentiometers (77) | 74 | 57 | — | — | — | 12 | — | m77 |
| **all four** | 7,007 | **3,252** | **844** | **805** | **457** | 420 | 318 | |

Top 4: Bourns, Amphenol Piher, Vishay Sfernice, Same Sky.

## Plan 79: battery holders

| Category (id) | Total | Top makers | file |
|---|---|---|---|
| Battery Holders, Clips, Contacts (86) | 1,360 | Keystone 622; MPD 425; Adam Tech 74; Assmann WSW 60; Bulgin 28 | m86 |
| …PCB mount only: Mounting Type `69` ∈ {PCB, Surface Mount 385069; PCB, Through Hole 385070; Surface Mount 409393} | **780** | **Keystone 361; MPD 249; Adam Tech 74; Assmann WSW 53**; TE Linx 15; Würth 8 | p86 |

PCB-mount rows by function (`p86`): coin cell holder 255, coin cell retainer 222, cylindrical
holder 176, cylindrical contact (single) 41, coin cell contact 36, cylindrical clip 31, 9 V holder
8, 9 V snap contact 6, 9 V snap connector 2, cylindrical contact (dual) 2. The other 580 rows of 86
are chassis, panel, free-hanging (wire-lead) or custom mounts: no footprint, out of plan-79.

## Plan 80: addressable LEDs

DigiKey files them under **LED Addressable, Specialty (126)** (found with the API:
`node scripts/digikey.js --json --limit 10 IN-PI55TATPRPGPB` → category "LED Addressable,
Specialty:126"; one API call, 2026-11-25).

| What | Filter | Total | Makers | file |
|---|---|---|---|---|
| all of 126 | — | 292 | Adafruit 172; Inolux 29; SparkFun 15; Harvatek 11; Dialight 10; M5Stack 10 | m126 |
| discrete (one LED) | Configuration `207` = Discrete 339432 | 146 | Adafruit 75; Inolux 28; Dialight 10; Würth 6; ams OSRAM 5; Lite-On 5; SparkFun 5 | d126† |

The 100 rows of `d126` show no Worldsemi maker: the only WS2812B / SK6812 silicon in stock comes as
Adafruit or SparkFun packs (e.g. Adafruit `1655`, "1=10"), and XINGLIGHT's `XL-5050RGBC-WS2812B` is
Marketplace-only (`kw.js` on `x/105?keywords=WS2812`: 33 rows, every one `Marketplace:`).
Owner decision: real makers only, packs out: **Inolux, Dialight, Würth, Lite-On**.

## Plan 81: OLED displays

| What | Category | Filter | Total | Makers | file |
|---|---|---|---|---|---|
| graphic OLED | Display Modules - LCD, OLED, Graphic (107) | Display Type `38` ∈ {OLED 381743, OLED - Passive Matrix 381744, OLED Monochrome 381768} | 96 | Newhaven 28; Display Visions 17; Orient 13; DLC 11; Adafruit 6; DFRobot 6 | o107† |
| character OLED | Display Modules - LCD, OLED Character and Numeric (99) | Display Type = OLED 381743 | 27 | Newhaven 19; Display Visions 7; Vishay Dale 1 | o99† |
| **both** | | | 123 | **Newhaven 47; Display Visions 24; Orient 13; DLC 11** | |

## Plan 82: indicator LEDs

| Category (id) | Filter | Total | Top makers | file |
|---|---|---|---|---|
| LED Indication - Discrete (105) | — | 6,479 | Kingbright 959; Broadcom 768; SunLED 562; QT Brightek 497; Lite-On 412; Dialight 366 | m105 |

Before the Marketplace exclusion XINGLIGHT (608) ranked fourth; none of its rows has DigiKey's own
stock.
