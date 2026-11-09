# Converting a datasheet PDF for the app

How to turn one vendor PDF into the app's Markdown datasheet. The format
is [format.md](format.md); this page is the procedure.

## Where things go

- Source: `datasheets/<group>/<id>.pdf`.
- Output: `schematical/schematical/Datasheets/<group>/<id>.md`. The id
  is the PDF's base name, and it is the file's front-matter `id`.
- Shared figures: `schematical/schematical/Datasheets/figures/<ref>.svg`.
  The app bundles both folders.
- `schematical/schematical/Parts/datasheets.json`: the entry with that
  id gets `"md": "<group>/<id>.md"` once the file passes the checks
  below.

## Steps

1. **Claims.** `python3 -I planning/spike-datasheet-format/claims.py <id>`
   prints the `devices` and `parts` the file must claim (every catalog
   part whose `"datasheet"` is this id), the pinouts and the footprints.
   Copy `devices` and `parts` into the front matter as printed; `packages`
   maps each pinout name to the vendor's package letters. A part the PDF
   lists but the catalog doesn't carry is mentioned in Conversion notes,
   not claimed.
2. **Read the PDF** (`pdftotext -layout`, and page images with
   `pdftoppm -r 100 -png` for figures, scans and anything the text layer
   garbles).
3. **Write the file**, following format.md: front matter, the `##`
   sections in order, spec tables, `pins`, generated `pinout` / `package`
   / `ordering` blocks (one `package` block per footprint the claimed
   parts use), and § Source fidelity. Model it on
   `planning/spike-datasheet-format/samples/sn74hc00.md`.
4. **Figures.** Use only the shared figures in the figures folder, with
   their parameters as `key: value` body lines:

   | ref | Shows | Parameters |
   |---|---|---|
   | `load-cmos-50pf` | output to C~L~ to ground | `CL` |
   | `load-rl-cl` | output to R~L~ and C~L~ in parallel to ground | `RL`, `CL` |
   | `load-ttl` | R~L~ from V~CC~ to the output, C~L~ to ground | `RL`, `CL` |
   | `waveforms-tpd` | input, in-phase and out-of-phase outputs; t~PLH~/t~PHL~ at V~M~ | `VM` |
   | `waveforms-cmos-tpd-tt` | the same plus 10 %/90 % transition times | `VM` |
   | `clamp-diodes` | input and output clamp diodes to V~CC~ and GND | none |

   A figure that matches none of these is described in one sentence
   (values in a table), never drawn as a sidecar SVG for these parts. A
   load figure whose values differ by test (C~L~ 15 pF and 50 pF) gets
   the values in a small table or a sentence next to the figure.
5. **Check.** Both must pass:
   - `python3 -I planning/spike-datasheet-format/check.py --quiet <file>`:
     `ok` (warnings about size or repeated anchors are acceptable).
   - `python3 -I planning/spike-datasheet-format/validate.py --max 0 <file>`:
     no errors. Read every warning of kind numbers, rows, registers,
     waveforms, notes, and prose "negation differs" / "obligation
     differs", and fix what is wrong. Values the text layer doesn't hold
     go under `Derived:` or `From images:` in Conversion notes, checked on
     the page image.
6. **Register** the file: add `"md"` to the id's entry in
   `datasheets.json`.

Variants of one family (`sn74ac08`, `sn74ac08-ep`, `sn74ac08-q1`) are
separate PDFs with different values and parts: convert each from its own
PDF. Reusing a sibling's structure is fine; every value comes from the
file's own PDF.
