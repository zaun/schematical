# plan-37 data (registers)

Written at planning time, 2026-11-12, at cf40403. Each letter adds its own
files and updates this map.

| File | What | Written by |
|---|---|---|
| `raw/<label>.jsonl.gz`, `raw/<label>.err` | the planning listing: one JSON line per DigiKey product (category 706 Flip Flops unless noted, in stock, Active / Last Time Buy) and the piece summary per run. Labels: `nexperia` (maker 1727, every package), `onsemi` (maker 488), `ti-p<package id>` (TI, maker 296, one "Package / Case" value each: the 24-, 48-, 56- and 64-pin packages), `ti-k<number>` (TI, keyword = the device number: 173 377 378 379 396 534 564 576 4076 40174), `ti-k670` (TI, keyword `SN74LS670`, no category: DigiKey files the '670 under Signal Switches, Multiplexers, Decoders) | planning |
| `raw/ti-facets.tsv` | the "Package / Case" facet of TI's piece (value id, count, name): 1271 parts | planning |
| `candidates.tsv` | one row per listed part (929; 472 in scope: 272 `register`, 198 `dff`, 2 `mux`). Columns: `mpn maker device family kind scope stock status grade dk_package dk_supplier_package bits elements output function datasheet_url pdf ordering verdict footprint`. Planning fills `mpn`…`datasheet_url` and `footprint`; A fills `pdf`, `ordering`, `verdict` | planning; A |
| `footprints-needed.txt` | in-scope parts per (footprint id, maker, DigiKey package); 4 ids marked `NEW` | planning |
| `census/list.js` | DigiKey KeywordSearch lister: one (category, maker[, package]) piece, split by Package / Case above 300 | planning |
| `census/facets.js` | one call: the Package / Case facet of a (category, maker) piece | planning |
| `census/run.sh` | wrapper (credentials from `~/.zshrc`; skips a piece with a `.done`) | planning |
| `census/candidates.py` | builds `candidates.tsv` from `raw/` and the Parts directory: device, family, kind, scope | planning |
| `census/footprints.py` | fills `footprint` and writes `footprints-needed.txt` | planning |

Re-running the planning census (64 DigiKey calls on 2026-11-12: nexperia 29, onsemi 6, ti-p 14, ti-k 13, ti-k670 1, facets 1):

```
census/run.sh /tmp/p37/raw 706 1727 - nexperia
census/run.sh /tmp/p37/raw 706 488 - onsemi
node census/facets.js 706 296 > raw/ti-facets.tsv
for p in 154526 154586 155096 155118 155120 155141 155158 225779 226086 226087 244896 245094 245095 261808; do census/run.sh /tmp/p37/raw 706 296 $p ti-p$p; done
for k in 173 377 378 379 396 534 564 576 4076 40174; do census/run.sh /tmp/p37/raw 706 296 - ti-k$k $k; done
census/run.sh /tmp/p37/raw - 296 - ti-k670 SN74LS670
python3 -I census/candidates.py /tmp/p37/raw ../../schematical/schematical/Parts candidates.tsv
python3 -I census/footprints.py candidates.tsv ../../schematical/schematical/Parts/footprints.json footprints-needed.txt
```

Known limits of the planning listing:

- TI's 16- and 20-pin packages were listed by device number (the
  `ti-k<number>` keyword pieces), not in full: the day's DigiKey quota was
  down to 3 calls. plan-37-A Phase 1 lists them in full.
- A "Number of Bits per Element" (parameter 2207) filter returns no products
  from the v4 API (`node census/list.js 706 1727 …` with that filter → `0
  products`), so pieces are cut by package, never by width.
