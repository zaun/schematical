# plan-58-data — slide switches

Data and scripts of plan-58 (`planning/plan-58-*.md`, archived to
`planning/completed/` as each letter lands). Written at planning time,
2026-11-24; letter A re-runs the census and rewrites `candidates.tsv`.

| File | What |
|---|---|
| `candidates.tsv` | One row per (maker, manufacturer part number) of the four makers' SPST / SPDT / DPST / DPDT slide switches that DigiKey lists as Active and In Stock (category 213), with DigiKey's parameters and the plan's `decision` (`add`, `out`, `check`) and `reason`. Later columns (`pdf`, `hit`, the final `skip`) are added by letter A. |
| `census/urls.tsv` | Sixteen rows (maker, circuit, URL): the DigiKey category page filtered to In Stock + Active + one maker + one circuit. Written by `census/make-urls.js`. |
| `census/make-urls.js` | Builds `urls.tsv`. DigiKey's `s=` query value is lz-string (`compressToEncodedURIComponent`) of `{"1": {<filter key>: {"c": false, "vn": false, "o": [<value ids>], "pt": <"String" for a parametric filter, "" otherwise>}}}`. Filter keys: `-2` Stocking Options (`5` = In Stock), `1989` Product Status (`0` = Active), `-1` Manufacturer (C&K 108, NKK 360, CIT Relay and Switch 2449, E-Switch 141), `130` Circuit (SPST 406576, SPDT 405967, DPST 340489, DPDT 340464). |
| `census/list.sh <raw-dir> [maker [circuit]]` | Loads each URL in a temporary tab of the Safari window that has a DigiKey page open (`scripts/safari-digikey.sh` explains the setup) and saves the page's own product data (`__NEXT_DATA__`, up to 100 products) to `<raw-dir>/<maker>_<circuit>.json`. Safari sometimes returns another tab's page or nothing; `check.py` validates every page and `list.sh` retries up to 5 times. |
| `census/check.py` | Validates one raw page: every row has the asked circuit and maker, and the row count is DigiKey's count. |
| `census/build.py <raw-dir> <candidates.tsv>` | Raw pages → `candidates.tsv` (merges DigiKey's per-packaging rows of one part number; applies the `decision` rules, see its docstring). |

Run order, from the repo root:

```
census/list.sh /tmp/p58/raw
python3 -I census/build.py /tmp/p58/raw planning/plan-58-data/candidates.tsv
```

Letter A then runs (each script has a `--selftest` or `--verify` mode and a docstring):

```
python3 -I planning/plan-58-data/census/fetch.py planning/plan-58-data/candidates.tsv /tmp/p58/pdf
python3 -I planning/plan-58-data/census/textscan.py planning/plan-58-data/candidates.tsv /tmp/p58/pdf /tmp/p58/scan.tsv
cp /tmp/p58/scan.tsv planning/plan-58-data/candidates.tsv        # adds pdf, hit, dead
python3 -I planning/plan-58-data/census/decide.py planning/plan-58-data/candidates.tsv /tmp/p58/pdf
python3 -I planning/plan-58-data/census/decide.py --skipped
python3 -I planning/plan-58-data/census/place.py planning/plan-58-data/candidates.tsv /tmp/p58/pdf
python3 -I planning/plan-58-data/census/specs-skeleton.py planning/plan-58-data/candidates.tsv planning/plan-58-data/specs.tsv
```

| File | What |
|---|---|
| `census/fetch.py <candidates.tsv> <pdf-dir>` | Downloads every distinct `datasheet` URL of the `add` rows to `<pdf-dir>/<md5 of URL>.pdf` and writes `index.tsv`. Safari's header set for littelfuse.com (a bare curl gets 403), `scripts/safari-digikey-pdf.sh` for `mm.digikey.com`; a body that is not a whole PDF (`%PDF-` and `pdfinfo` opens it) is not kept. |
| `census/textscan.py <candidates.tsv> <pdf-dir> <out.tsv>` | Adds `pdf`, `hit` (the part number is in the PDF text on its own), `dead` (obsolete/discontinued/NRND/end of life within 300 characters). |
| `census/decide.py` | `add` (hit), `pending` (PDF, number not printed: C–F read the ordering key), `skip` (no PDF), `out` (every row of a PDF obsolete); `--skipped` writes the "Slide switches (plan-58)" section of `datasheets/skipped-parts.md`. |
| `census/place.py` | Copies the PDFs the `add` / `pending` rows need to `datasheets/switch/slide/<ck|nkk|cit|eswitch>/` and writes `datasheets/switch/slide/datasheets.tsv`; `--verify` recomputes the hashes; `--path` prints the file of each part. |
| `census/maker-urls.py` | `--list` the datasheet URLs that are not the maker's own host (only `mm.digikey.com`: E-Switch's S3 bucket is the maker's own); `maker-urls.tsv` maps each to the maker's copy (or empty); `--verify` fetches them. |
| `census/specs-skeleton.py` | `specs.tsv`: one row per (maker, series, mounting, circuit) of the `add` / `pending` rows; C–F fill `mpn_regex`, `pins`, `footprint`, `note`. |

Later letters add `gen-parts.py` (plan-58-C).
