# plan-38-B: Net rules — per-net layer rules and net groups: data model, file format, DRC check

Last updated: 2026-11-13
Effort: medium (1h–2h)
Depends on: plan-38-A

Prerequisites: see plan-38-A § Prerequisites (re-run its commands). Also every earlier letter (A) is complete: for each letter X, `grep -c "Commit: —" planning/plan-38-X-*.md planning/completed/plan-38-X-*.md 2>/dev/null` prints 0 (a letter is archived to `planning/completed/` when it finishes, so either path may exist).

Adds to the net classes of plan-38-A the other two net-level rule sets the
router reads: a **layer rule** (a net must be only on, or prefers, a set of
copper layers) and **net groups** (a set of nets whose routed lengths must
match, or a pair routed side by side). This letter is data only: model, edit
functions, file format, and the DRC lines that report a net that breaks its own
layer rule. Nothing routes; the group's meander and pair engines are plan-38-I
and J.

References:

- plan-38-A (§ 3 class model, § Decisions); `Canvas/BoardSettings.swift`
  (`PlaneNet`, `BoardSettings.traceLayers`); `Canvas/NetClass.swift` (from A);
  `DesignRuleCheck/DesignRuleCheck.swift:checkLayers`; `docs/file-format.md`.

## 1. Goal

- `LayerRule { mode: only | prefer, layers: [CopperLayer] }` can be set on a net
  class (`NetClass.layerRule`) and on a single net (`BoardSettings.netOverrides`);
  `SchematicDocument.effectiveLayers(ofNet:in:)` returns the layers a net may use
  and which of them are preferred.
- `BoardSettings.netGroups: [NetGroup]` holds equal-length groups and differential
  pairs, with edit functions; a net is in at most one group.
- Both round-trip through the file, written only when present; a file with
  neither is byte-identical to today's.
- The DRC reports copper of a net on a layer its **Only** rule forbids
  (`DesignRuleIssue.Kind.layerRule`, one line per net, listing the traces).

### Non-goals (explicit constraints)

- No routing behavior (E, F) and no UI (C, K). No length measuring or meanders (I),
  no coupled routing (J). No new DRC *length* check (I adds `lengthMatch`).
- File `version` unchanged (optional fields only), as A.
- `NetClass`'s existing fields and file entries do not change meaning.

## 2. Current State

- `BoardSettings.traceLayers` is the board's non-plane copper layers
  (`BoardSettings.swift:traceLayers`); `traceLayerRefusal` refuses a trace on a
  plane or an absent layer (`SchematicModel.swift:2932`).
- `checkLayers()` reports a trace on a layer the board lacks or on a Plane, and
  Bottom parts on a 1-layer board (`DesignRuleCheck.swift:343`).
- A net's pads have a layer: a surface-mount pad sits on its part's side
  (`BoardConnectivity.Pad.side.copperLayer`), a through-hole pad (`drill > 0`) is
  on every layer (`DesignRuleCheck.swift:padLayers`).
- A net's identity (rail or pin) is `PlaneNet`; `BoardConnectivity.net(of:)` and
  `netClass(ofNet:in:)` come from plan-38-A.
- `DesignRuleIssue.Kind` is `short, unrouted, unconnected, placement, layer,
  clearance, edge, drill, annularRing, holeToHole, via, traceWidth, silkscreen,
  solderMask`; nothing persists or switches on its raw values
  (`grep -rn "DesignRuleIssue.Kind" schematical schematicalTests` → only the
  declaration and `DesignRuleCheckTests.swift:30`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Existing DRC tests (must stay green) | 11 | `grep -c "func test" schematical/schematicalTests/DesignRuleCheckTests.swift` → 11 |
| Nets with exactly one connection (candidates for pairs / length groups) in AFU, LFU | 41 of 53, 38 of 59 | `planning/plan-38-data/spike/results.txt` § Connections per net |

### Verified properties

- A net restricted to one layer cannot also reach a pad on another side: SMD pads
  are on `side.copperLayer` only (read `DesignRuleCheck.swift:padLayers`), so the
  router (plan-38-E) can decide "impossible" from the pads alone.

## 3. Design Overview

Layer rule. `LayerRule { mode: LayerRuleMode (.only | .prefer), layers:
[CopperLayer] }`, `layers` non-empty and each on the board. Resolution for a
net: a `netOverrides` entry for the net (`PlaneNet`, first match) → the net's
class rule (plan-38-A resolution) → none (any routable layer, no preference).
**Effective layers** = the rule's layers ∩ `board.traceLayers`. For **Only**, that
set is the net's hard allowed set (empty = the net can't be routed, reported by
the DRC line and by the router). For **Prefer**, it is the cheap set; any other
trace layer costs extra in the router (plan-38-E); the allowed set stays every
trace layer.

A net limited to one layer gets no vias (a via joins layers). A net whose Only
set holds ≥ 2 layers may use vias between those layers only. A through-hole pad
is reachable on every layer.

Net group. `NetGroup { id: UUID, name: String, mode: .lengthMatch | .diffPair,
members: [PlaneNet], tolerance: Double (mm, 0.01…50, default 0.25), gap: Double
(mm, diff pair only, 0.05…10, default 0.2: the edge-to-edge gap between the
pair's two traces), maxUncoupled: Double (mm, diff pair only, 0…50, default 5:
how much of a pair may run apart near its pads), maxMeander: Double (mm,
0.5…50, default 4: the largest sideways size one tuning bump may reach) }`. A
`.diffPair` group has exactly 2 members; a `.lengthMatch` group has 2 or more. A
net is in at most one group (`assignNet` moves it). Every member of a pair uses
the class width of the first member, so a pair's traces are one width.

Length is measured as the sum of a net's trace segment lengths (plan-38-I); a
group's eligibility (every member a two-terminal net) is decided when routing,
not stored.

Where correctness risk concentrates: net resolution when a member's pin was
deleted (the member stays, resolves to nothing, the group reports it) and the
file round trip. Byte-identity gate: a document with no rules/groups is written
as before (Phase 2).

Rejected: storing the rule on traces (they follow the net when it is rewired);
Prefer as a single layer only (a pair of layers is a normal ask: "Top or
Bottom, not Layer 2"); group members as net integers (renumbering).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` partial. Moot: `- [x] ~~text~~ — moot: <evidence>`.
> Fill the `Commit:` line when the phase lands. Unticked means NOT DONE.

> **How to read the Check lines.** `xcodebuild` below means `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test CODE_SIGNING_ALLOWED=NO`; a Check that starts with `-only-testing:…` is that command with those arguments. Tests are classes in `schematical/schematicalTests/` (file-system-synchronized: a new file there is in the test target with no project edit).

### Phase 1 — types, edit functions, resolution

- [x] `Canvas/NetRules.swift` (new): `LayerRuleMode`, `LayerRule`, `NetOverride { net: PlaneNet, layerRule: LayerRule? }`, `NetGroupMode`, `NetGroup` with the ranges and defaults of § 3, and `NetGroup.problem(...)` (member count by mode, tolerance/gap/maxUncoupled/maxMeander in range, name 1…40). (Member count: a pair has at most 2; see Corrections.)
- [x] `Canvas/NetClass.swift`: add `var layerRule: LayerRule?` (default nil).
- [x] `Canvas/BoardSettings.swift`: `netOverrides: [NetOverride] = []`, `netGroups: [NetGroup] = []`.
- [x] `Canvas/SchematicModel.swift` (each `recordEdit(.pcb, …)`): `setNetClassLayerRule(_:_:)`, `setNetLayerRule(_ net: PlaneNet, _ rule: LayerRule?)` (nil removes the override), `addNetGroup(named:mode:) -> UUID`, `removeNetGroup(_:)`, `renameNetGroup`, `setNetGroupMode` (a pair with > 2 members is refused with `editorMessage`), `setNetGroupTolerance/Gap/MaxUncoupled/MaxMeander` (clamped), `assignNet(_:toGroup:)` (moves the member out of any other group; refuses a 3rd member of a pair), `unassignNet(_:)`.
- [x] `SchematicModel.swift`: `effectiveLayers(ofNet:in:) -> EffectiveLayers` (`allowed`, `preferred`, and the `rule` they came from) and `group(ofNet:in:) -> NetGroup?`, both by `BoardConnectivity.net(of:)`; plus `layerRule(ofNet:in:)`, the resolution both read.
- [x] Tests: `schematicalTests/NetRulesTests.swift` — override beats class rule beats none; Only {Top} on a 2-layer board gives allowed [Top]; Only naming a Plane layer drops it; Prefer leaves allowed = all trace layers; a pair refuses a third member and a mode switch with 3 members; assigning a net to group 2 removes it from group 1; each edit is one undo step. — `-only-testing:NetRulesTests` → 6 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: the cases above pass.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p38-dd test -only-testing:schematicalTests/NetRulesTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 1–2 min).
Commit: c56aa28b

### Phase 2 — file format

- [x] `Document/SchematicFile.swift`: class entries gain `layerMode`, `layers` (written only when the rule is set); `Board.netLayerRules: [{net, mode, layers}]` and `Board.netGroups: [{id, name, mode, members, tolerance, gap, maxUncoupled, maxMeander}]`, each written only when non-empty; decode errors name the class/group ("net group <id> pair has 3 members; it can have at most 2" — see Corrections; unknown layer string, a layer not on the board is **kept** — it is the user's data, as a layer style is kept after the layer leaves, and ignored by `effectiveLayers`). Also refused: a net listed twice in `netLayerRules`, a group id twice, a net in two groups.
- [x] `docs/file-format.md`: § Net classes rows `layerMode`/`layers`; new § Net layer rules and § Net groups.
- [x] Tests (`NetRulesTests`): round trip of a class rule, an override and both group modes; a document with none writes none of the new keys and its encoded bytes equal the literal from plan-38-A Phase 2 (same fixed document); a pair with 3 members, a tolerance of 0 and an unknown layer name each throw a message naming the group. (A group has no layers, so the unknown layer name is tested on a class rule and a net rule, each naming its owner.) — `-only-testing:NetRulesTests -only-testing:NetClassTests -only-testing:SchematicFileV5Tests` → 35 tests, 0 failures, `** TEST SUCCEEDED **`.

Acceptance: round trip equal; no-rules document byte-identical.
  Check: `-only-testing:schematicalTests/NetRulesTests -only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/SchematicFileV5Tests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

### Phase 3 — DRC: layer rule

- [ ] `DesignRuleCheck/DesignRuleCheck.swift`: `DesignRuleIssue.Kind.layerRule` after `layer`; `DesignRuleChecker.checkLayerRules()` (called after `checkLayers()` in `run()`): for each net whose effective rule is **Only**, every trace of the net on a layer outside the allowed set, and every via on a net whose allowed set is one layer, yields one line per net — reference the net's name (`netNames`), issue "Copper on Layer 2; this net is limited to Top", items the offending traces/vias. A net with an Only rule that resolves to no routable layer yields "Layer rule names no routable layer".
- [ ] `DesignRuleCheckWindow.swift:DesignRuleCheckInput`: no change needed (`board` already holds the rules); the file records that reading.
- [ ] Tests (`DesignRuleCheckTests`): a trace on Bottom of a net limited to Top → one `.layerRule` line; the same net routed on Top → none; a via on a Top-only net → a line; the existing 11 tests unchanged.

Acceptance: the new cases pass; no existing DRC test changes.
  Check: `-only-testing:schematicalTests/DesignRuleCheckTests -only-testing:schematicalTests/NetRulesTests` → `** TEST SUCCEEDED **` (est. 2 min).
Commit: —

## Validation Plan

- Tests: `NetRulesTests` (new), `DesignRuleCheckTests`, `NetClassTests`, `SchematicFileV5Tests`.
- Coverage check: `NetRules.swift`, the new file keys and `checkLayerRules` are each hit by a test that fails if it is removed.
- Runtime proof: none (data only).
- Doc sync: `docs/file-format.md` (Phase 2).
- Final gate: plan-38-M.

## Decisions

See plan-38-A § Decisions (layer rules = Only and Prefer; groups = length match and coupled pair; both decided by the owner, 2026-11-13). Author decisions: the defaults in § 3 (tolerance 0.25 mm; pair gap 0.2 mm, twice the 0.1 mm clearance of the Custom DRC defaults, `results.txt`), override-before-class resolution, one group per net.

## Corrections

- § 3 said a `.diffPair` group "has exactly 2 members" and a `.lengthMatch`
  group "2 or more", and Phase 1/2 had the mode switch and the file refuse a
  pair with ≠ 2. That can't hold while editing: `addNetGroup` makes an empty
  group, so a pair being filled in (0 or 1 member) would be refused by
  `setNetGroupMode` and fail to reopen from its own file. Corrected to: a pair
  has **at most** 2 members (more is refused by `assignNet(_:toGroup:)`,
  `setNetGroupMode` and the file: "net group <id> pair has 3 members; it can
  have at most 2"); having exactly 2 (and a length group 2 or more) is checked
  when routing, with each member's eligibility, as § 3 already says. Not a
  weakening: every check that can be made on stored data still is.
- `effectiveLayers` returns a struct `EffectiveLayers { allowed, preferred,
  rule }` instead of a tuple: same `allowed`/`preferred` reads, plus the rule
  the DRC line (Phase 3) and the router name in their messages. For **Only**,
  `preferred` is empty (every allowed layer is equal).

## Summary

Data and one DRC line. The risk is small: an optional-field round trip and
resolution through `PlaneNet`. Untouched: routing, UI, length math.
