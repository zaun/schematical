# plan-27-data — tactile switches

Data and scripts of plan-27 (`planning/plan-27-*.md`, archived to
`planning/completed/` as each letter lands).

| File | What |
|---|---|
| `candidates.tsv` | Every in-stock Active / Last Time Buy tactile switch of the ten makers (DigiKey category 197), one row per part, with DigiKey's parameters and the plan's `decision` (`add`, `pending`, `skip`, `out`) and `reason`; `pdf`, `hit`, `dead` from the datasheet scan. Rebuilt by the census (below); B/C turn `pending` into `add` / `skip`. |
| `footprints-needed.txt` | Planning-time proxy: one line per (maker, series, mounting) with a PDF. `census/footprint-lines.py` regenerates it. |
| `specs.tsv` | One or more rows per (maker, series, mounting) of the kept rows: `mpn_regex`, `terminals`, `pins` (`A=…;B=…`), `footprint`, `note` (plan-27-B § 4.3). A created the skeleton; B/C fill it. |
| `schemes.tsv` | Per (maker, series): the regular expression of the datasheet's part-number key that decides `pending` rows (plan-27-B § 4.3). |
| `maker-urls.tsv` | For datasheets DigiKey links on its own or Waldom's server: the same document on the maker's site, or empty (then the DigiKey URL stays; owner, 2026-11-08). |
| `footprints-B.json`, `footprints-C.json` | Ready footprint objects until their parts land (E–I move them into `Parts/footprints.json`). |
| `work/` | The datasheet readers' files, one folder per maker (`BRIEF.md` is their instructions); `census/merge-work.py` merges them into the files above. |
| `gen-parts.py` | Writes the tactile devices and parts of `Parts/switch.json` and their `Parts/datasheets.json` entries (plan-27-E § 4.1). `--selftest`. |
| `append-footprints.py` | Moves the footprints the generated parts use into `Parts/footprints.json` (plan-27-E § 4.2). |

`census/` (run from the repo root, in this order):

| Script | Step |
|---|---|
| `list.sh <raw-dir>` | DigiKey listing per maker (229 calls; needs `DIGIKEY_*` from `~/.zshrc`). |
| `build.py <raw-dir> <all.tsv>` | Raw pages → one row per part. |
| `select.py <all.tsv> <cand.tsv>` | DigiKey rules (circuit, lighting, mounting, stock) → `candidate` / `check` / `out`. |
| `fetch.py <cand.tsv> <pdf-dir>` | Downloads every datasheet URL (Safari headers, `Claude-User` for we-online.com; te.com one at a time); keeps a file only when `pdfinfo` opens it. |
| `textscan.py <cand.tsv> <pdf-dir> <out.tsv>` | `pdf`, `hit` (the part number printed and standing on its own), `dead`. |
| `footprint-lines.py <candidates.tsv>` | The proxy lines of `footprints-needed.txt`. |
| `place.py <candidates.tsv> <pdf-dir>` / `--verify` / `--path` | Copies the needed PDFs to `datasheets/switch/tactile/<maker>/` (one file per distinct content) and writes `datasheets.tsv`. |
| `decide.py <candidates.tsv> <pdf-dir>` / `--skipped` / `--verify-skipped` | plan-27-A § 3 rules → `add` / `pending` / `skip` / `out`; the "Tactile switches (plan-27)" section of `datasheets/skipped-parts.md`. |
| `maker-urls.py --list` / `--verify` / `--url URL` | The non-maker datasheet URLs and their maker URLs. |
| `specs-skeleton.py <candidates.tsv> <specs.tsv>` | The empty `specs.tsv` rows. |
| `scheme-check.py --maker M` | Applies `schemes.tsv` to the `pending` rows. |
| `specs-check.py --maker M` | Every `add` row has exactly one complete spec row whose footprint has its pads. |
| `fp-load.sh <footprints-X.json>…` (+ `fp-merge.py`) | Loads ready footprints through `footprint_check` in a copy of the tree; also rejects overlapping pads. |
| `merge-work.py <B\|C> <slug>…` | Merges `work/<slug>/` into `schemes.tsv`, `specs.tsv`, `footprints-<letter>.json`, `candidates.tsv`. |
| `uncovered.py [--maker M] [--rows]` | Kept rows no reader has covered yet. |
