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

Later letters add `census/fetch.py`, `census/textscan.py`, `census/decide.py`,
`specs.tsv`, `maker-urls.tsv` and `gen-parts.py` (plan-58-A, plan-58-C).
