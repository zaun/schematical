# plan-25-B: Space / Esc / `/` while drawing a Line or Bus (canvas)

Last updated: 2026-11-07
Effort: medium (1h–2h)
Depends on: plan-25-A

This wires plan-25-A's `RouteDraft` into the Schematic canvas. Line and Bus
work the same way:

- **Space** locks the live leg as the preview shows it, and drawing carries
  on from its end.
- **Esc** unlocks the last lock. With nothing locked, it cancels the drag
  (today's Esc).
- **`/`** flips the live leg's L when neither of its ends is a pin.
- **Release** places the wire or bus along the whole route.

Outcome: in the app, start a Line on an LED's right pin and drag right and
down. Press Space, then drag back left onto a second LED's right pin and
release. You get a U wire with exactly those two bends. With the Bus tool,
drag, Space, drag, Space, drag, release: you get one bus whose lines follow
the three legs. Press Esc once mid-drag and the last leg comes loose. Press
it again with no locks and nothing is placed.

References:

- `planning/plan-25-A-route-locks-model.md`: `RouteDraft`, `RouteAxis`,
  `anchorAxis`, `connect(_:_:path:)`, `busDraft(candidates:)`,
  `addBus(candidates:)`
- `schematical/schematical/Canvas/SchematicCanvasView.swift`: `Interaction`
  (line 69), `connectionPreview` (316), `busPreview` (330), the Escape
  monitor (358–383), `handleChanged` `.line`/`.bus` (474–508), `handleEnded`
  (602–612)
- `schematical/schematical/ContentView.swift:232,257` (tool button `.help`)
- `scripts/harness/build.sh`, `scripts/harness/bus-lines.swift` (harness
  precedent)

## Prerequisites

See plan-25-A for the shared gate.

| Must be true | Command | Status |
|---|---|---|
| plan-25-A complete | `ls planning/completed/plan-25-A-*` → 1 file | MET (2026-11-07, 8db6988) |
| `RouteDraft` exists | `grep -c 'struct RouteDraft' schematical/schematical/Canvas/RouteDraft.swift` → 1 | MET (2026-11-07) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- Space, Esc and `/` work during a Line or Bus drag as above, and are
  swallowed only while one is in progress. Otherwise they pass through
  untouched.
- The preview and the placed result are the same route.
- The Line and Bus tool tooltips name the three keys.

### Non-goals (explicit constraints)

- No change to what starts or ends a Line: it still starts and ends on a
  pin, a bus pin or a point on a wire, and a release over nothing places
  nothing (locks included).
- No change to Bus refusals or their messages. A refused route previews
  dashed-refused, as today.
- No click-to-click drawing mode. Drawing stays one drag.
- The PCB Trace tool and every other tool are unchanged.

## 2. Current State

(As of plan-25-A complete.)

- `Interaction.connecting(from: LineAnchor, cursor:)` and
  `.drawingBus(from: CGPoint, cursor:)` (`SchematicCanvasView.swift:75-77`)
  hold only the start.
- An `NSEvent` local keyDown monitor is installed `onAppear`
  (`:125,358`). It handles keyCode 53 (Esc) by calling `cancelDrawing()`,
  which sets `interaction = .idle` so the rest of the drag does nothing,
  and swallows the event only when that returned true. Commit b7c95b5 added
  it for use during a held drag. That shows keyDown reaches the monitor
  while the mouse button is down, which Space and `/` rely on too.
- `connectionPreview` draws `OrthogonalRoute.route(start, end)`, where `end`
  is the hovered anchor's tip or the snapped cursor (`:316-326`).
  `busPreview` draws `busDraft(from:to:)`, or a refused H-first L
  (`:330-342`).
- `handleEnded` calls `connect(from, target)` (`:603-607`) or
  `addBus(from:to:)` (`:608-610`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Lines matching `connecting(` / `drawingBus(` in the canvas | 6 / 6 | `grep -c 'connecting(' schematical/schematical/Canvas/SchematicCanvasView.swift` → 6; same for `drawingBus(` → 6 |
| Key handlers in the app | 1 (the Esc monitor) | `grep -rn 'keyCode\|addLocalMonitor\|onKeyPress' schematical/schematical` → 2 lines, both in it |

### Verified properties

- **No existing binding for Space or `/`.** `grep -rn 'keyboardShortcut' schematical/schematical`
  shows none on space or "/". The single monitor (above) handles only 53.
- **Every placed route is orthogonal and ends at the tips.** `connect` uses
  the anchors' tips for the ends (plan-25-A §4.3), so a preview that ended
  on the snapped cursor can't misplace a wire end.

## 3. Design Overview

One piece: the canvas holds a `RouteDraft` in each drawing interaction and
maps three keys onto it. The risk is small and sits in the key monitor:
swallowing a key when not drawing would break typing in sidebar fields.
That case is handled the way Esc already is: swallow only on a handled
key with a live drawing.

The gate is behavioral: a harness run for the model and preview path, plus
a real-app check of the keys.

Rejected: SwiftUI `.onKeyPress`. It needs focus on the canvas, and the
canvas doesn't take focus during a drag. The existing NSEvent monitor
already works mid-drag.

## 4. Detailed Design

### 4.1 State

```swift
case connecting(from: LineAnchor, draft: RouteDraft, cursor: CGPoint)
case drawingBus(draft: RouteDraft, cursor: CGPoint)
```

- Line press: `RouteDraft(start: anchorTip(from), startAxis: document.anchorAxis(from))`.
- Bus press: `RouteDraft(start: snap(start), startAxis: nil)`.
- `handleChanged` replaces only `cursor`, keeping `draft`.

### 4.2 What the preview shows (one helper each, used by preview, Space and release)

- Line: `lineEnd(cursor) -> (point, axis, target)`. `target` is the anchor
  under the cursor other than `from`, `point` is its tip (else
  `snap(cursor)`), and `axis` is `anchorAxis(target)` (else nil). The
  preview is `draft.path(to: point, endAxis: axis)`, with `target` for the
  highlight as today.
- Bus: `candidates = draft.busCandidates(to: snap(cursor))`. The preview is
  `busDraft(candidates:)`'s `draft.path` on success. On failure it is
  `candidates[0]`, refused.

### 4.3 Keys (`drawingKeyMonitor`, replacing `escapeMonitor`)

Handled only while `interaction` is `.connecting` or `.drawingBus`.
Otherwise the event is returned untouched.

- keyCode 53 (Esc): `draft.unlock()`. When it returns false, `interaction = .idle`
  (today's cancel).
- keyCode 49 (Space), ignoring `event.isARepeat`:
  - Line: `draft.lock(to: point, endAxis: axis)` from §4.2.
  - Bus: when the successful draft's path equals `candidates[1]` (the
    fallback posture is what's shown), `draft.flip()` first. Then
    `draft.lock(to: snap(cursor), endAxis: nil)`. A refused preview locks
    `candidates[0]`.
- `event.charactersIgnoringModifiers == "/"` with no ⌘/⌃/⌥: `draft.flip()`
  when `canFlip(endAxis:)` is true for the current end (Line: §4.2 axis;
  Bus: nil).

A handled key returns nil (swallowed). `cancelDrawing()` is folded into
this.

### 4.4 Release

- Line: with a target, call `connect(from, target, path: draft.path(to: tip, endAxis: axis))`.
  With none, nothing happens (today).
- Bus: `addBus(candidates: draft.busCandidates(to: snap(release)))`.

### 4.5 Tooltips (`ContentView.swift:232,257`)

`.help` for `.line` and `.bus` becomes
`"\(tool.title) — Space: lock a corner · /: flip the bend · Esc: undo a corner"`.
That comes from a new `SchematicTool.help` (`SchematicModel.swift`), which
is `title` for every other tool. It's the author's call: the keys have no
other on-screen mention.

## Phases

> Tick each box in the same commit as its work.

### Phase 1 — Canvas state, preview and release

- [x] `SchematicCanvasView.swift`: §4.1 state, the §4.2 helpers,
      `connectionPreview`/`busPreview` over them, and §4.4 release.
- [x] `SchematicCanvasView.swift`: `WireLayer` (line 768) drops `private`,
      as `BusLayer` did, so the harness can render it. It is `private
      struct WireLayer` today (`grep -n 'struct WireLayer'` → `768:private struct WireLayer: View`).
- [x] `scripts/harness/route-locks.swift` (new): through the same calls
      the canvas makes (`RouteDraft`, `anchorAxis`, `connect(path:)`,
      `busDraft/addBus(candidates:)`), draw the Outcome's Line U and a
      3-leg bus. Render `WireLayer`/`BusLayer` previews and the placed
      result to `/tmp/p25img/*.png`. `expect` covers:
      - the wire's interior is the two U bends;
      - the bus has 3 lines and 2 L joints;
      - an unlock returns to the 1-leg preview.

Acceptance: the harness prints no `FAIL`, and the PNGs show the U wire and
the 3-leg bus.
  Check: `scripts/harness/build.sh scripts/harness/route-locks.swift && /tmp/schematical-harness/H.app/Contents/MacOS/harness /tmp/p25img` → only `ok:` lines (est. 4 min, mostly the first app build).
  Observed 2026-11-07, run as `APP=/tmp/p25-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/route-locks.swift /tmp/p25-harness && /tmp/p25-harness/H.app/Contents/MacOS/harness /tmp/p25img`
  (reusing the worktree's Debug build): 16 `ok:` lines, no `FAIL`, `PASS`.
  `line-placed.png` shows the U from cathode to cathode, and `bus-placed.png`
  shows the 3-line bus.
Commit: e6d8d50

### Phase 2 — Keys and tooltips

- [x] (landed with Phase 1 in e6d8d50) `SchematicCanvasView.swift`: `drawingKeyMonitor` per §4.3, installed
      and removed where the Escape monitor is now.
- [x] `SchematicModel.swift`: `SchematicTool.tooltip` (renamed from `help`, see
      Corrections). `ContentView.swift:233` uses it. Line 258 is the PCB picker and keeps `title`.

Acceptance: in the running app (Debug build), check each of these:
- the Outcome's Line U and 3-leg bus;
- Esc once drops only the last lock;
- Esc with no locks places nothing;
- `/` flips a free leg and does nothing on a pin-forced leg;
- Space typed into the sidebar's bus-name field still types a space.

Drive the real mouse (AGENTS.md: put the pointer at (0, 0) after).
  Check: build with `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **`. Then run each step above in the app and record what happened here (est. 10 min; key events during a held drag exist only in the real app, so no harness run can catch a broken monitor).
  Observed 2026-11-07: `** BUILD SUCCEEDED **`. Run in the Debug build, with
  the mouse driven by a CGEvent script (`/tmp/p25drive/drive`, keys posted to
  the app's pid; see Corrections). The test file had two LEDs (K tips at
  world (230,150) and (230,310)). Every check passed:
  - **Line U:** cathode → out and down → Space → into the other cathode.
    Placed a U with bends at x = 290.
  - **Bus:** right, Space, down, Space, left. Placed 3 lines following the
    legs.
  - **Esc once:** after 2 locks, one Esc dropped the second. The corner fell
    at the one-lock route's point (1050,350), not the two-lock (950,300).
  - **Esc with no locks:** nothing placed.
  - **`/` on a free bus leg:** the L flipped to vertical-first.
  - **`/` on a leg leaving the LED anode:** the preview still left sideways
    (unchanged).
  - **Space in the bus wire-name field:** typed a space ("B0 x").

  Pointer left at (0, 0), and the test app quit.
Commit: —

## Validation Plan

- Tests: plan-25-A's unit tests cover the rules. The harness (Phase 1)
  covers the canvas's call sequence. The real-app run (Phase 2) covers the
  keys.
- Runtime proof: the Phase 2 app run.
- Doc sync: doc comments on `Interaction.connecting/.drawingBus`, the key
  monitor, `connectionPreview`, `busPreview`, `BusPreview`.
- Final gate (once, after Phase 2): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical test CODE_SIGNING_ALLOWED=NO`
  → "0 failures". `git log --format= --name-only --grep='^plan-25' | grep '^backend/'`
  → no output (est. 6 min).

## Decisions

See plan-25-A § Decisions (owner, 2026-11-07). Also:

- Space locks what the preview shows. For a Bus showing its fallback
  posture, that posture is what gets locked. Author's call: what you see
  is what you get.
- Tooltips name the keys. Author's call (§4.5).

## Corrections

- **`SchematicTool.help` → `tooltip`.** §4.5 named the property `help`.
  Building with it, `.help(tool.help)` failed (`the compiler is unable to
  type-check this expression in reasonable time`, ContentView.swift:158),
  and only the tool picker at line 233 is the schematic one. Line 258 is
  the PCB picker (`PCBTool`), which keeps `.help(tool.title)`. The property
  is now `tooltip`.
- **The key monitor landed with Phase 1**, because the new
  `Interaction` cases replace `cancelDrawing()` and could not build without
  it. It is still the design in §4.3.
- **App test harness: Space, `/`, Esc and Return posted through the HID tap
  never reached the app on this machine.** Letters did. The keys were
  confirmed with a temporary stderr log in `handleDrawingKey`, since
  removed. Posting with `CGEvent.postToPid` reaches it. A Command flag left
  by earlier Cmd-key steps also stuck to later events: `/` arrived with
  `mods 0x100000` and was rightly ignored. It was cleared with a
  flagsChanged event, and the Phase 2 results above are from the clean
  run. Neither problem is in the app.
- Each open window's canvas installs its own monitor, so a key is seen
  once per window (seen in the log). Only the window that is drawing has
  a drawing interaction, so the others pass the key on. That is the same
  as the Esc monitor before.

## Summary

The only new risk is the key monitor swallowing keys. It is limited to a
live drawing, the same way Esc is today.
