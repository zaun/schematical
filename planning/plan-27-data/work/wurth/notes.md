# plan-27 C — Würth Elektronik datasheet reading (slug wurth)

Rows: the pre-census copy (/tmp/p27/pre-cand.tsv): 136 Würth rows, all `add`
(every article number printed verbatim in its own PDF), none `pending`, none
`check`. So `schemes.tsv`, `checks.tsv` and `cannot-draw.tsv` are header-only.

## Method

Würth publishes one PDF per article number (`<article>.pdf`, drawing on p.1:
Dimensions, Recommended Land Pattern / Hole Pattern, Schematic). The DigiKey
series (WS-TASV, WS-TATV, WS-TATU, WS-TASU) are families of many sizes, so
"one land pattern per series" does not hold: the land pattern follows the
article's size/type. All 136 PDFs' p.1 text was clustered by (description,
size/type, land-pattern text, dimension text): 38 clusters. One drawing per
cluster (its first article number) was viewed as an image; every other member
of a cluster has the same land-pattern dimensions and numbering in its text, so
the whole of every group was checked, not only spot checks. Clusters whose
land-pattern text matched but whose dimension text differed (heights) were
viewed too where the land pattern could differ (430162/430162085826/430172073816;
434331045822/436331045822; 434333045822/436333045822; 431151008826/431151015826
top views) and share one footprint. 30 footprints, 30 specs rows (one per
footprint; `mpn_regex` = the article prefixes of the cluster).

Ids: `SW-TACT-<series>-<first article prefix>`, with `-H25`/`-H43`/`-H13`/`-H45`
where one 6-digit prefix has two land patterns (434111, 434113, 434331).

Pin numbering: as the land pattern prints it; where it prints none, from the
top-view dimension drawing (431151/431153: 2 TL 1 BL 4 TR 3 BR; 431471: GND tab
on the left in both views, 4 TL 2 BL 3 TR 1 BR); for 2-terminal parts with no
numbers, left = 1. Pairs from each Schematic.

## WS-TASV (Surface Mount, 105 rows)

| footprint | parts | land pattern (p.1) | pins |
|---|---|---|---|
| -430133 | 430133050816 | 2 pads 2.0×2.0, centres 6.0 | 1–2 |
| -430151 | 430151005826, 430181005816 | 1.0×0.7, 3.85 × 3.5; 1 TL 2 TR | 1,2 / 3,4 |
| -430152 | 430152* (12), 430182* (6) | 1.55×1.3, 7.95 × 4.5; 1 TL 2 BL 3 TR 4 BR | 1,3 / 2,4 |
| -430162 | 430162* (2), 430172* (3) | 2.0×1.8, 14.0 × 5.0 | 1,2 / 3,4 |
| -430173 | 430173003816 | 0.6×0.65, 3.1 × 2.75 | 1,3 / 2,4 |
| -430181 | 430181038816 | 2.0×1.4, 7.0 × 3.0 | 1,2 / 3,4 |
| -430451 | 4304[578]1* (9) | 2.0×1.4, 7.0 × 4.0 | 1,2 / 3,4 |
| -430453 | 4304[578]3* (9) | 1.7×1.4, 6.7 × 4.0 | 1,2 / 3,4 |
| -430753 | 4307[578]3* (5) | 2.75×1.0, 5.75 × 4.0 | 1,2 / 3,4 |
| -431151 | 4311[58]1* (4) | 1.2×1.1, 6.0 × 3.7, GND pads left out | 1,3 / 2,4 |
| -431153 | 4311[58]3* (4) | 0.8×0.6, 4.6 × 3.7, GND pads left out | 1,3 / 2,4 |
| -431471 | 4314[78]1* (3) | 2.0×1.3, 7.0 × 4.5, GND pad left out | 1,2 / 3,4 |
| -434111-H25 | 434111025826, 434121025816 | 2 pads 1.5×1.4, 6.5 | 1–2 |
| -434111-H43 | 4341[12]1 043/050 (6) | 2 pads 2.3×1.5, 6.7 (no numbers) | 1–2 |
| -434113-H25 | 434113025826, 434123025816 | 2 pads 1.35×1.4, 6.35 | 1–2 |
| -434113-H43 | 4341[12]3 043/050 (6) | 2 pads 2.0×1.5, 5.4 (no numbers) | 1–2 |
| -434123 | 434123025826, 434133025816 | 1.05×0.65, 4.15 × 2.15 | 1,2 / 3,4 |
| -434133 | 434133025826, 434163025816 | 1.0×1.0, 5.0 × 1.7; 1 TL 2 BL | 1,3 / 2,4 |
| -434153 | 434153017835 | 1.15×1.05, 3.35 × 1.45 (mounting plan ±0.1) | 1,3 / 2,4 |
| -434771 | 434771025826, 434781025816 | 2 pads 1.75×1.2, 6.25 (no numbers) | 1–2 |
| -435151 | 4351[57]1014* (3) | 0.65×0.6, 3.15 × 1.7; ground 5, 6 left out | 1,2 / 3,4 |
| -435431 | 4354[345]1019* (3) | 0.9×1.0, 4.1 × 1.6; 4 TL 1 TR 3 BL 2 BR; GND left out | 1,4 / 2,3 |

Odd: 434123* and 434133* each hold two different switches (434123025816 is the
6.0×3.8 J-bend, 434123025826 the 4.2×3.2; 434133025816 the 4.2×3.2,
434133025826 the 4.7×3.5): regexes list the article numbers exactly.
SW-TACT-WS-TASV-434133 and SW-TACT-WS-TASU-434333 have the same pads and body
but different numbering (1 TL 2 BL vs 3 TL 1 TR), so they are not shared.

## WS-TASU (Surface Mount, Right Angle 10; Edge Mount, Right Angle 1)

- -434331-H13 (434331013822, 3.5×2.9 side push): 1.6×1.0 pads, 3.8 × 1.4; the two
  0.65 peg holes not drawn; body 3.5×2.9 centred on the pads (top view).
- -434331-H45 (434331045822, 434351045816, 436331045822, 436351045816): 1.2×1.0,
  5.6 × 1.7, 3 TL 1 TR 4 BL 2 BR; 4363* add two 0.75 peg holes (not drawn).
- -434333 (434333045822, 434353045816, 436333045822, 436353045816): 1.0×1.0,
  5.0 × 1.7, same numbering; 4363* add peg holes.
- -436333 (436333033816, 4.45×2.55): three terminals 1, 2, 3 in a row (1 and 3
  common per schematic): explicit, pin 1 on terminals 1 and 3, pin 2 = terminal 2.
  Hold-down pads, leg holes and the prohibited area not drawn. Body placed from
  the side view (legs 1.2 behind the front face) against the land pattern's leg
  holes (1.5 from its front edge): pads at y −0.825. Doubt: the 0.3 mm body offset
  rests on reading that side view; the pads themselves are printed.
- -434381 (434381035816, edge mount in a 5.1×1.65 board notch): explicit; terminal
  pads 1.45×1.1 at ±2.275; hold-down pads beside the notch not drawn; body 6.3×3.55
  (mid-values) with its back on the pads' back edge (flange depth scaled, ~1.45,
  matches 3.10 − 1.65): pads at y −1.225. Notch itself not modelled.
  Doubt: the body placement is from scaled drawing positions.

## WS-TATV (Through Hole, 23 rows)

- -430156 (4301[58]6*, 15): Recommended Hole Pattern 4 × 1.0 dia. at 6.5 × 4.5; pad
  1.7. No terminal numbers anywhere on the THT drawing: numbered as the same-size
  SMD sibling 430152043826 (1 TL 2 BL 3 TR 4 BR), giving the schematic's 1–3 / 2–4
  pairs across the 6.5 span. Doubt: worth a look; the pairing is the usual 6×6 one.
- -430456 (4304[567]6*, 8): 4 × 1.2 dia. at 12.5 × 5.0, numbered 1 TL 2 TR 3 BL 4 BR
  in the hole pattern; pad 2.05.

## WS-TATU (Through Hole, Right Angle, 11 rows)

- -431256 (431256*): hole pattern: switch terminals 2, 3 at 4.5, frame terminals 1, 4
  (ground per schematic) at 7.0, 2.5 behind; frame holes not drawn; datasheet 2, 3
  = pins 1, 2. Body 7.4×6.55; side view: terminal row 2.55 behind the front face,
  so pads at y +0.725 (plunger toward +y).

## Cannot draw

None.

## Counts

- Kept rows: 136 (all verbatim `add`); regex add 0 / skip 0 (no pending rows); out 0.
- scheme-check: `0 add, 0 skip, 0 still pending`.
- Footprints: 30 (fp-load: `30 new footprints: load ok`). Specs rows: 30
  (`specs ok (136 add rows)`).
