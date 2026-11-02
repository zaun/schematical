# plan-10 data

Research outputs that plan-10 (counters, dividers, latches, shift registers)
transcribes into part files. Read 2026-11-01 from the datasheets in
`datasheets/{counters,divider,latch,shifters}/`. Treat them as the per-device
fact source the sub-plans cite; the design (kinds, models, keys) is in the
plan files, not here.

| File | Contents |
|---|---|
| `parts.tsv` | Every orderable part plan-10 adds (2097 rows): name, device id, family, package name, TI package code, pin count, footprint id, flags, ordering status, datasheet. Built from each datasheet's current PACKAGING INFORMATION table (`.A`/`.B` rows dropped, older addendum tables ignored). |
| `latch-a.md` | 74_373, 74_374 (dff), 74_573, 74_533, 74_563, 74_580, 74_841, 74_843, 74_1G373, 74_16373, 74_162373, 74_32373, 74_16841, 74_162841 |
| `latch-b.md` | 74_75, 74_375, 74_279, 74_259, 4099B, 4724B, 4042B, 4043B, 4044B, 4508B, 74_4511, 74_16260, TPIC6259, TPIC6A259, TPIC6B259, TPIC6273 (dff), TPIC6B273 (dff) |
| `counter-a.md` | 74_161, 74_163, 74_169, 74_190, 74_191, 74_192, 74_193, 74_561, 74_569, 74_867, 74_869 |
| `counter-b.md` | 74_90, 74_92, 74_93, 74_293, 74_390, 74_393, 74_590, 74_592, 74_593, 74_697, 74_8154, 4020B, 4024B, 4040B, 74_4020, 74_4024, 74_4040 |
| `counter-c.md` | 4029B, 4510B/4516B, 4518B/4520B, 74_4518/74_4520, 40102B/40103B, 74_40103, 40160B/40161B, 40192B/40193B, 4026B/4033B, 40110B |
| `divider.md` | 4017B, 4022B, 74_4017, 4018B, 4059A, 74_4059, 4522B, 4089B, 4527B, 74_97 |
| `shift-a.md` | 74_164, 74_264, 74_165, 74_166, 74_194, 74_195, 74_299, 74_323, 4014B, 4021B, 4015B, 74_4015, 40194B, 4035B |
| `shift-b.md` | 74_595, 74_594, 74_596, 74_597, 74_598, 74_673, 74_674, TPIC6595/6A595/6B595/6C595, TPIC6A596/6B596/6C596, 4094B, 74_4094, 4031B, 4517B, 4034B, 40105B, 74_40105, 74_8153 |
| `footprints.md` | The 18 new footprints as ready JSON objects, with drawings and derivations (the PDIP-20-NE section is not added: NE parts use `PDIP-20`) |

Each device section gives: datasheets and pages, families and part-number
prefixes, military-number mapping, every pinout (pin number, name, role),
exact behavior (function tables, equations, priorities), per-family timing and
VCC, and the obsolete rows.

Where a section says "census" or cites `/tmp/p10/census2.json`, the same rows
are in `parts.tsv`. Military numbers the datasheets do not map were resolved
during planning: JM38510/32502 → LS373, /32503 → LS374, /05754 → CD4021B,
/31508 → LS193, /32701 → LS390, /32702 → LS393; the agents' inferred
mappings (/31501 LS90, /31502 LS93, /31504 LS161, /31512 LS163, /38001
ALS161B, /38002 ALS163B, /05653 4020B, /05655 4024B, /05651 4017B, /07601
S194) are used as given. Families for slash numbers follow the slash sheet
(076 S; 306, 315, 316, 325, 327 LS; 346 F; 372, 380, 382 ALS; 654 HC or HCT
by datasheet; 663 HC).
