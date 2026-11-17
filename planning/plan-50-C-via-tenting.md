# plan-50-C: Per-Via Tenting (Top and Bottom)

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-50-B (complete)

Every via is tented today: the solder mask covers it on both sides, hard-coded in `GerberExport.drawMask` ("Vias
are tented") and in the preview. A test point, a via to probe, or a via to solder a jumper to needs an open
side. This sub-plan gives each via two flags, **Tented on Top** and **Tented on Bottom**, both on by default.

The single outcome: a via can be left open on either side; the choice is saved, undoable, shown in the Via
sidebar and in Board Preview, written as a mask opening in the Gerber of that side, and written to the KiCad
board as a per-via `(tenting …)` that KiCad 10 reads back as the same two modes.

References:

- plan-50-A, plan-50-B
- `schematical/schematical/Canvas/BoardTrace.swift:246` (`BoardVia`), `Canvas/PCBSidebarView.swift:667` (`ViaPanel`),
  `Document/GerberExport.swift` (`drawMask`), `Document/KiCadBoardWriter.swift` (vias at `:498`),
  `BoardPreview/BoardRenderer.swift` (`drawSurface`, the tented-via ring at `:369`)
- `docs/file-format.md` § Vias (line 207), `docs/export.md` § Gerber and § KiCad
- `planning/plan-50-data/README.md` (KiCad per-via tenting facts) and `probe_kicad_via_tenting.py`

## Prerequisites

The shared gate is in plan-50-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-50-B is complete | `ls planning/completed \| grep -c '^plan-50-B'` → 1 | NOT MET until B is archived |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue, and again before you decide to stop. If you stop, report the current status
> of *all* prerequisites.

## 1. Goal

- `BoardVia` has `tentedTop` and `tentedBottom`, both `true` by default; every place that makes a via (the Via
  tool, the router) gets both.
- File: a via entry may carry `"openTop": true` and/or `"openBottom": true`, written only when true; missing
  means tented. A file saved before this plan opens with every via tented.
- The Via sidebar has two toggles, **Tented on Top** and **Tented on Bottom**; each is one undo step.
- Board Preview draws an open side's via ring as bare finish metal, a tented side's under the mask as today.
- Gerber: the solder-mask file of a side has, for each via open on that side, an opening the via's metal diameter
  grown by the Mask expansion rule. A tented side has nothing, as today.
- KiCad: every via is written with `(tenting (front yes|no) (back yes|no))` (`front` is Top).

### Non-goals (explicit constraints)

- No via plugging, filling or capping (IPC-4761 types II–VII), and no `ViaDrill` protection field: a tented or
  open via is described only by the mask file (and, in KiCad, the `tenting` key).
- The PCB canvas (the editing view) does not show the mask, so it draws open and tented vias the same.
- The EasyEDA Pro export is unchanged: its via mask fields cannot be checked here (no file of it has been opened
  in EasyEDA Pro, `docs/export.md` § EasyEDA Pro).
- The DRC is unchanged (it has no via-mask rule), and the automation tools (`add_via`) are unchanged.
- No other file version change; older builds ignore the two keys and show the via tented.

## 2. Current State

- `BoardVia` (`Canvas/BoardTrace.swift:246`): `id`, `centre`, `drill`, `diameter`, `locked`, `auto`. Undo diffs
  `vias` as whole values (`Canvas/UndoHistory.swift:128`), so a new field is undoable with no other work.
- File: `SchematicFile.Board.ViaEntry` (`Document/SchematicFile.swift:307`) has `locked` and `auto` as optional
  booleans written only when true (`:786`); the reader builds `BoardVia` at `:1103`–`:1115`.
- Via is built in 3 places in the app (`grep -rn --include='*.swift' "BoardVia(" schematical/schematical | wc -l` → 3: `SchematicModel.swift:3443`, `Routing/RoutingApply.swift:49`,
  `SchematicFile.swift:1109`) and 1 in tests.
- Sidebar: `ViaPanel` (`Canvas/PCBSidebarView.swift:667`) — Lock Position, X, Y, Hole, Metal, Ring, Layers, net rows.
  Setters follow `setViaDrill` (`SchematicModel.swift:3470`): `recordEdit(.pcb, "Change Via Hole") { … }`.
- Gerber mask: `GerberExport.drawMask(on:copper:expansion:into:)` writes pads and bare holes grown by the
  expansion; its comment says "Vias are tented (covered)". `BoardConnectivity.vias` is the document's vias.
- Preview: `BoardRenderer.drawSurface` fills every via's ring with `underMask` ("Tented vias: a ring under the mask
  and the hole", `:369`), then the hole.
- KiCad: `KiCadBoardWriter.tracks()` writes each via as `(via (at) (size) (drill) (layers "F.Cu" "B.Cu") net uuid)`
  (`:498`–`:509`) with no tenting key.

### Verified properties

- **KiCad 10.0.8 reads a per-via `(tenting …)` in the file version this app writes.** In a `(version 20240108)`
  board a via with `(tenting (front no) (back yes))` loads as front NOT_TENTED (2), back TENTED (1); a via with no
  `tenting` key loads as FROM_BOARD (0) on both (`planning/plan-50-data/probe_kicad_via_tenting.py`, output in its
  docstring and the README). So the key is written on every via, explicitly.
- **The test host can hand files to the shell.** `KiCadExportTests.testPrintSamplesWhenAsked`
  (`schematicalTests/KiCadExportTests.swift:247`) prints `KICAD_FILE <name> <base64>` lines when `KICAD_OUT` is
  set (the sandboxed host cannot write where the shell reads), and `ExportSamples` builds the boards.

## 3. Design Overview

`BoardVia.tentedTop: Bool = true`, `tentedBottom: Bool = true`; a helper `func isTented(on side: BoardSide) -> Bool`.
The file keys are `openTop` / `openBottom` (true only when open), the same shape as `locked`. `setViaTenting(_ id:
UUID, _ side: BoardSide, _ tented: Bool)` is `recordEdit(.pcb, "Change Via Tenting")`.

Each consumer asks `via.isTented(on: side)`:

- Preview: in `drawSurface(side:)`, a via open on `side` is filled as `metal(ring path, finish, rect)` instead of
  the `underMask` ring; the hole is drawn as before.
- Gerber `drawMask(on: side, …)`: for each via with `!isTented(on: side)`, `file.fill(.circle(centre: via.centre,
  diameter: via.diameter), grownBy: expansion)`. The copper files are unchanged.
- KiCad: `(tenting (front \(top)) (back \(bottom)))` with `yes`/`no`, after `(layers …)`.

**Risk.** Low: a Boolean pair with defaults that reproduce every existing output. Where the output must stay
identical (a board with no open via), the check is that the Gerber zip, KiCad board and saved file are
byte-identical to before for such a board, except the KiCad via gaining the new `tenting` lines (which is the
expected diff) and the saved file's via entries staying unchanged (the keys are written only when open).

**Rejected.** *One toggle for both sides*: the owner chose Top and Bottom separately. *Writing `tentedTop: true`
always*: it would change every saved file with a via. *A global "vias tented" board setting*: per-via is what the
owner asked for.

## Compatibility / Format Impact

- New optional via keys `openTop`, `openBottom` (§ 3), documented in `docs/file-format.md` § Vias. Unchanged:
  every saved file with no open via.
- Gerber: an open via adds one flash to a mask file; KiCad: every via gains a `tenting` key.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. `- [~]` for
> partial, with one line on what remains. A moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's
> `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — The via flag, the file and the sidebar

- [ ] `Canvas/BoardTrace.swift`: `BoardVia.tentedTop`, `tentedBottom`, `isTented(on:)`.
- [ ] `Document/SchematicFile.swift`: `ViaEntry.openTop` / `openBottom` (optional Booleans, written only when
      true; `Board(…)` at `:786`, reader at `:1109`).
- [ ] `Canvas/SchematicModel.swift`: `setViaTenting(_:_:_:)` beside `setViaDiameter`.
- [ ] `Canvas/PCBSidebarView.swift` `ViaPanel`: two `Toggle`s, "Tented on Top" and "Tented on Bottom" (help:
      "Covered by solder mask. Off leaves the via open on that side, for probing or soldering"), after Ring.
- [ ] `docs/file-format.md` § Vias: the table rows for `openTop` / `openBottom`.
- [ ] Tests, in `schematicalTests/ViaTests.swift`: a new via is tented on both sides; `setViaTenting` flips one
      side, is one undo step named "Change Via Tenting" (`doc.undoName(on: .pcb)`), and `doc.undo(on: .pcb)`
      restores it; a file round-trips `openTop` and `openBottom` independently; a saved file with every via
      tented has neither key; a file with neither key opens tented.

Acceptance: the flag is stored, saved, undoable and editable.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/ViaTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: —

### Phase 2 — Preview, Gerber and KiCad

- [ ] `BoardPreview/BoardRenderer.swift` `drawSurface`: an open via is drawn in the finish (§ 3).
- [ ] `Document/GerberExport.swift` `drawMask`: the openings (§ 3); update its doc comment ("Vias are tented unless
      opened").
- [ ] `Document/KiCadBoardWriter.swift` `tracks()`: the `(tenting …)` lines.
- [ ] `docs/export.md`: Gerber table's `.GTS` / `.GBS` row now ends "Vias are tented unless opened (Via ▸ Tented
      on Top / Bottom); an open side gets an opening the via's metal grown by the Mask expansion"; § KiCad: the board's
      via line says each via carries its two tenting flags.
- [ ] `KiCadExportTests.swift`: `ExportSamples.vias()` — the 2-layer board of an `SN74HC00N` and a resistor wired
      together with four vias on its traces, the vias set (Top, Bottom) tented = (yes, yes), (no, yes), (yes, no),
      (no, no) — and `testPrintSamplesWhenAsked` prints it (`KICAD_FILE vias/…` and `GERBER_FILE vias/<name> <base64>`
      for every Gerber file).
- [ ] Tests: `GerberExportTests`: with the four vias, `.GTS` has exactly 2 via openings (the (no, yes) and (no, no)
      vias) and `.GBS` exactly 2 (the (yes, no) and (no, no)), each `C,<diameter + 2 × expansion>` at the via's
      centre; with every via tented the mask files gain nothing (`.GTS` and `.GBS` of the vias board
      with all four tented are byte-identical to those of the same board with its four vias deleted). `KiCadExportTests`: the board text has exactly 4 `(tenting` lines in the
      vias, with the four (front, back) pairs in via order. `BoardPreviewTests`: an open via changes the render
      (pixel buffers of the same board with the via tented and open differ).

Acceptance: Gerber and KiCad carry the choice, and KiCad reads it back as the same modes.
  Check: `xcodebuild … test -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/BoardPreviewTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
  And: run `KICAD_OUT=1 xcodebuild … test -only-testing:schematicalTests/KiCadExportTests/testPrintSamplesWhenAsked`, decode the `KICAD_FILE vias/…kicad_pcb` line into `$OUT/vias.kicad_pcb`, then `PYTHONPATH=~/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/Current/lib/python3.9/site-packages ~/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/Current/bin/python3 -I planning/plan-50-data/probe_kicad_via_tenting.py $OUT/vias.kicad_pcb` → four via lines `1 1`, `2 1`, `1 2`, `2 2` in via order (TENTED 1, NOT_TENTED 2) (est. 3 min).
Commit: —

## Validation Plan

- Tests: `ViaTests`, `GerberExportTests`, `KiCadExportTests`, `BoardPreviewTests`, `SchematicFileV5Tests`,
  `PCBUndoTests` (all named in the phases).
- Coverage check: the all-tented byte comparison in Phase 2 is the proof that existing outputs did not change;
  the open-via counts prove the new path runs.
- Runtime proof: the KiCad python probe above (the real KiCad 10 reading the app's own file).
- Doc sync: `docs/file-format.md` § Vias, `docs/export.md` § Gerber and § KiCad.
- Final gate (once, after Phase 2): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/ViaTests -only-testing:schematicalTests/GerberExportTests -only-testing:schematicalTests/KiCadExportTests -only-testing:schematicalTests/BoardPreviewTests -only-testing:schematicalTests/SchematicFileV5Tests -only-testing:schematicalTests/PCBUndoTests -only-testing:schematicalTests/EasyEDAProExportTests -only-testing:schematicalTests/RoutingProblemTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 8 min).

## Decisions

- Tenting is per via, Top and Bottom separately, both on by default — decided by the owner, 2026-11-16. An open side gets a mask opening; test points and one-side probing work. (§ 1, § 3)
- The mask opening is the via's metal grown by the Mask expansion — decided by the author: it is how a pad's opening is made in the same file.
- No `ViaDrill` IPC-4761 field — decided by the author: the Gerber spec lists the values but not their meanings, and the mask file already says what is open.
- KiCad gets the key on every via, not only open ones — decided by the author: a via without it is "from the board", which depends on the board's own setting, not the app's.

## Corrections

(Filled in during execution.)

## Summary

The risk is in not changing any output for a board with no open via; the byte comparison is the guard. Untouched:
the router and its vias' sizes, drill files, the DRC, the PCB canvas, EasyEDA, automation.
