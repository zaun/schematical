# plan-51-A: Backend — the `socket` kind and the `"fits"` key

Last updated: 2026-11-21
Overall Effort: huge (> 3d) — plan-51 is A → B → C → D → E → F → G → H → I → J
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

plan-51 adds **IC sockets** to the parts library and a **socket mode** to every
placed DIP or PLCC chip. A chip has three modes: **IC** (today), **IC + Socket**
(the chip is soldered into a socket: the PCB footprint is the socket's, the Bill
of Materials lists the chip and the socket) and **Socket only** (the socket is on
the board, the chip is not inserted: the footprint is the socket's, the Bill of
Materials lists only the socket). The socket is a second part saved with the
chip, picked in a small dialog that lists the sockets that fit the chip. The
sockets are ordinary catalog parts: they are listed in the parts drawer under
**Connector → IC Socket**, with their datasheets, like any other part.

| Letter | Content | Effort |
|---|---|---|
| **A** | backend: the `socket` kind, the `"fits"` part key, tests, docs | medium |
| B | census: finish the Preci-Dip listing, stock re-check, datasheet downloads | large |
| C | the drawings: per-series land-pattern facts, `series.tsv`, skipped parts | large |
| D | footprints: DIP sockets (through hole and surface mount), the generic DIP sockets | large |
| E | footprints: PLCC sockets (through hole and surface mount), the generic PLCC sockets | medium |
| F | parts: `Parts/socket.json`, datasheets, goldens, the drawer lists them | large |
| G | the Part Editor: the kind's form and wizard steps | medium |
| H | app: a placed chip's socket mode (model, file, undo, Bill of Materials, facts, automation) | large |
| I | app: the mode control, the compatible-sockets dialog, the harness | large |
| J | landing tier | small |

The behavioral outcome of plan-51: Parts → Connector → IC Socket lists every
socket; a placed `SN74HC00N` shows **IC / IC + Socket / Socket only** in its
sidebar; choosing **IC + Socket** picks the generic 14-pin 0.3 in socket, puts
its footprint on the PCB (same pad numbers, so wires and traces stay), and the
Bill of Materials gets a second line `XU1`; **Choose Socket…** opens a dialog of
the sockets whose `"fits"` names the chip's footprint; the file, undo, copy and
paste, the KiCad and EasyEDA exports and the design-rule check follow the
socket footprint; **Run** behaves as if the chip were in the socket in all three
modes.

Outcome of A: `socket` is a part kind. A part file with `"kind": "socket"`
loads, with the device keys `"family"` and `"rowSpacing"`, the part keys
`"mounting"`, `"finish"` and `"fits"`; the loader rejects every malformed case
of § 3; a socket is a passive part in a circuit (its pins join nothing);
`ctest` passes T0 + 1 programs and every golden is byte-identical (no part
uses the kind yet).

References:

- `AGENTS.md` (§ Datasheets); `docs/backend.md` (§ Adding a kind, § Tests);
  `docs/parts-format.md` (§ Footprints, § `header`, § `generic`).
- `backend/src/models/header.c` (the passive-kind shape: 68 lines),
  `backend/src/library/kind.h`, `kinds.c`, `library.c` (`PART_KEYS`,
  `load_footprint_choices`: the precedent for `"fits"`), `backend/include/parts.h`.
- `planning/plan-51-data/` (README, `fits.tsv`).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.

| Must be true | Command | Status |
|---|---|---|
| The backend builds and passes | `cmake -S backend -B /tmp/logic-p51 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p51 -j8 && ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed, 0 tests failed out of 60` | MET (2026-11-24 at 2865af91: 60 of 60, build dir /tmp/logic-p51-wt) |
| No `socket` kind yet | `grep -c 'PART_SOCKET' backend/include/parts.h` → 0; `ls backend/src/models/socket.c` → no such file | MET (2026-11-21: 0, no file) |
| Baselines recorded at A's start | `grep -c '^logic_test' backend/CMakeLists.txt` (T0 = 60); `wc -l backend/tests/golden/{parts,traces,footprints}.txt` (36515 / 36515 / 2775); `awk '/PART_GATE/,/} PartKind;/' backend/include/parts.h \| grep -o 'PART_[A-Z0-9_]*' \| wc -l` (42 kinds) | MET (2026-11-24 at 2865af91: 60, 36515 / 36515 / 2775, 42 kinds) |
| The DigiKey daily quota is available to B | `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)"; node scripts/digikey.js --limit 1 "74HC00"` → a table, not HTTP 429 (the quota resets about 00:00 UTC; other sessions share it) | MET on the second key (2026-11-24 18:01 UTC, `node scripts/digikey.js --opt2 --limit 1 "74HC00"` → a table, 399 calls left today; the standard key is at 0 until 2026-11-25 00:00 UTC; B only: run B's listing on `--opt2`) |

Everything below is written against the world where these hold. The last row
gates sub-plan B alone.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all* rows.

## 1. Goal

- `ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed` with 61 programs
  (T0 + `socket_test`).
- `socket_test` passes the § 6 cases (loads, every rejection, a socket in a circuit).
- `parts.txt`, `traces.txt` and `footprints.txt` under `backend/tests/golden/`
  are byte-identical (`git diff --stat backend/tests/golden` → empty).

### Non-goals (explicit constraints)

- No part file ships in A: `Parts/socket.json` is F's. No app change.
- No simulation: a socket is a passive part, the `header` shape; its pins are
  `inout` terminals that join nothing.
- No change to any existing kind, to `footprints.json`, to the part key table's
  existing keys or to any loader message of an existing kind.
- No geometry rule for fitting. A socket names the chip footprints it holds, by
  id (§ 3.2); nothing derives fit from pad positions.

## 2. Current State

- `PartKind` ends with `PART_PLD` (`backend/include/parts.h:33`);
  `TRACE_LAST_KIND` is `PART_PLD` (`tests/trace_check.c:22`); two arrays in
  `tests/listing_check.c:50–51` are sized `PART_PLD + 1`;
  `tests/support/listing.c:30–42` names each kind and `:119–:194` prints its facts.
- `HEADER_KIND` (`backend/src/models/header.c`, 68 lines) is the passive kind:
  `load_device` reads `"rows"` and `"genders"`, `bind_pinout` checks the pin
  count, `load_part` sets `out->header`, the `SimComponentType` has no hooks.
  `KINDS[]` lists it at `backend/src/library/kinds.c:64`.
- `KindContext` carries no library pointer (`library_internal.h:17–24`), so a
  kind cannot look a footprint up. The footprint keys are common part keys
  handled in `library.c`: `PART_KEYS` (`:261–264`) lists `footprint`,
  `connectorFootprint`, `footprintChoices`; `load_footprint_choices`
  (`:597–660`) validates an array of footprint ids with
  `part_library_find_footprint` and `check_pads` (`:544–560`: every pin of the
  part has a pad of its designator in the footprint).
- Per-kind restrictions of common part keys have a precedent: `"vcc"` and
  `"inputMax"` are allowed on gate, dff, jkff, sram, eeprom, serialmem and
  oscillator parts only (`docs/parts-format.md` § Parts).

### Measured populations

| What | Count | Command |
|---|---|---|
| Test programs T0 | 60 | `grep -c '^logic_test' backend/CMakeLists.txt` → 60; `ctest` → `100% tests passed, 0 tests failed out of 60` (2026-11-24 at 2865af91) |
| Kinds | 42 | `awk '/PART_GATE/,/} PartKind;/' backend/include/parts.h \| grep -o 'PART_[A-Z0-9_]*' \| wc -l` → 42 |
| Golden lines parts / traces / footprints | 36515 / 36515 / 2775 | `wc -l backend/tests/golden/{parts,traces,footprints}.txt` |
| Chip footprints a socket holds | 24, used by 2405 parts, 0 with an absent pin | `python3 -I planning/plan-51-data/census/chips.py` |

### Verified properties

- **The part's pinout is bound and the kind's `load_part` has run before any
  footprint key is read** (`library.c:739–748`: `part->pins`, `part->pin_count`
  are set, then `ops->load_part`; the footprint keys follow at `:758–780`), so
  `"fits"` is read in that same sequence, after `"footprint"`, with the part's
  pin count and kind known (`ops->kind`).
- **`check_pads(ctx, part, footprint, key, id)` is the right test for a chip
  footprint**: it needs a pad of every pin's designator and nothing else, and a
  chip footprint has the socket's designators (`"1"` … `"<pins>"`), checked by
  `chips.py`'s assertion that every `FITS` footprint has `pins` equal to its
  shape's pin count.

## 3. Design

### 3.1 The kind

`parts.h`: `PART_SOCKET` appended after the last kind present; an info struct and
three `Part` members:

```c
typedef struct {
    const char *family;       /* "DIP" or "PLCC" */
    uint32_t row_um;          /* DIP: row spacing in micrometres (5080, 7620, 10160, 15240); PLCC: 0 */
    const char *mounting;     /* "through-hole" or "surface-mount" */
    const char *finish;       /* the contact finish as the datasheet prints it ("gold", "tin", "tin-lead"); NULL = not given */
} PartSocket;
/* Part: const PartSocket *socket;
         const PartFootprint *const *fits; size_t fit_count;   (the chips it holds) */
```

`socket.c` (`SOCKET_KIND`, file `Parts/socket.json`): device keys `"family"`
(`"DIP"` or `"PLCC"`; `unknown socket family "X"`) and `"rowSpacing"` (mm:
required for DIP and one of 5.08, 7.62, 10.16, 15.24 — `"rowSpacing" must be
5.08, 7.62, 10.16 or 15.24`; not allowed for PLCC — `a PLCC socket has no
"rowSpacing"`); `row_um` = llround(mm × 1000). Part keys: `"mounting"` (required;
`unknown socket mounting "X"`) and `"finish"` (optional non-empty string).
`bind_pinout`: every pin is `inout` (`socket pin "X" must be inout`); a DIP
socket's pin count is even and 4 to 64 (`a DIP socket needs an even pin count of
4 to 64`); a PLCC socket's is 20, 28, 32, 44, 52, 68 or 84 (`a PLCC socket needs
20, 28, 32, 44, 52, 68 or 84 pins`). The model is the `header` shape: no hooks.
`"simulated"` stays the device default (`true`): a placed socket does not turn
Run off.

### 3.2 `"fits"`

A common part key in `library.c`, next to `footprintChoices`: `PART_KEYS` gains
`"fits"`. On a `socket` part it is required (`missing "fits"`): a non-empty array of footprint
ids (`"fits" must be a non-empty array of footprint ids`). Each id must exist (`unknown fits footprint "X"`), be listed once
(`fits footprint "X" listed twice`), and have a pad for every pin
(`check_pads` with the key text `fits footprint`); its `pins` must equal the
part's pin count (`fits footprint "X" has 12 pins; the part has 14`). On any
other kind it is rejected (`"fits" is only for a socket`). Loaded into
`Part.fits` / `Part.fit_count` in file order.

Why by id: the chip footprints are the catalog's own ids, a socket's fit is
then data a test checks (`chips.py`'s `FITS`, F's generator and `library_check`
read the same list) and no geometry code decides it. Rejected: a derived rule
(family, pin count, row spacing against the chip footprint's pad x-extent):
oscillators and 7-segment displays carry the package `DIP` and have 4-pin and
10-pin footprints that a 4-pin socket would pass; and `kind` code cannot see the
chip footprint without a second library pointer in `KindContext`.

### 3.3 Where the risk is

Small. The change adds a kind and one optional common key. The only existing
code changed is `PART_KEYS` (one entry) and the key loop that calls
`load_footprint_choices`; the goldens being byte-identical is the check that no
existing part changed. **Byte-identity is a verification method here, not the
premise**: a diff means the key loop changed an existing part; root-cause it
with `listing_check --print | diff`.

## 4. Phases

### Phase 1 — the kind

- [x] `backend/include/parts.h`: `PART_SOCKET` after the last kind; `PartSocket`; `Part.socket`, `Part.fits`, `Part.fit_count` (doc comments). — `PART_SOCKET` follows `PART_PLD`; `PartSocket` after `PartPld`; the three `Part` members after `pld`.
- [x] `backend/src/models/socket.c`: `SOCKET_KIND` per § 3.1 (copy `header.c`'s shape; `KindContext` helpers `kind_string`, `kind_number`). — device keys `family` / `rowSpacing`, part keys `mounting` / `finish`; `socket_bind_pinout` checks `inout` and the pin counts; the per-part `PartSocket` is allocated in `socket_load_part`.
- [x] `backend/src/library/kinds.c`: `extern const PartKindOps SOCKET_KIND;` and one line in `KINDS[]`; `backend/CMakeLists.txt`: add `src/models/socket.c` where `header.c` is listed.
- [x] `tests/trace_check.c:22` `TRACE_LAST_KIND`, `tests/listing_check.c:50–51`, `tests/support/listing.c` (name `socket`; print `family`, `row_um`, `mounting`, `finish`, `fit_count`) — the three lines `docs/backend.md` § Adding a kind names. — `TRACE_LAST_KIND` → `PART_SOCKET`, the two arrays → `PART_SOCKET + 1`, `listing.c` names `socket` and prints `family=… row=… mounting=… finish=… fits=…`.
- [x] Discovered: the two Swift switches over `PartKind` (`PartFacts.kindName`, `BundledPartFile.name(of:)`) both end in `default:`, so a kind the app does not know yet does not break the app build (the kind's own entries are G's and H's).

Acceptance: the backend builds; the existing 60 programs pass; goldens byte-identical.
  Check: `cmake --build /tmp/logic-p51 -j8 && ctest --test-dir /tmp/logic-p51 -j8 && git diff --stat backend/tests/golden` → `100% tests passed, 0 tests failed out of 60`, empty diff (est. 3 min). Measured 2026-11-24 in `/tmp/logic-p51-wt`: `100% tests passed, 0 tests failed out of 60`, `git diff --stat backend/tests/golden` empty. VERIFIED.
Commit: d3a4cf25

### Phase 2 — `"fits"`, tests, docs

- [x] `backend/src/library/library.c`: `"fits"` in `PART_KEYS`; `load_fits` beside `load_footprint_choices` per § 3.2 (required on a socket, rejected elsewhere), called from the same key loop. — `load_fits` runs after the footprint keys (`library.c`, after the `missing "footprint"` check).
- [x] `backend/tests/socket_test.c` + `logic_test(socket_test)` in `backend/CMakeLists.txt`: § 6 cases (inline JSON, a tiny `footprints.json` with `PDIP-14`, `CDIP-14`, `PLCC-28`). — 31 assertions, `/tmp/logic-p51-wt/socket_test | grep -c '^PASS'` → 31, `socket_test: all passed`. Deviations in § Corrections.
- [x] `docs/parts-format.md`: the file table gains `socket.json`; the intro list of kinds gains "IC socket"; a `### socket` section (device and part keys, the pin rules, `"fits"`); `"fits"` in the Parts key table; `docs/backend.md`: none (no new step). — done; `docs/backend.md` also lists `socket.c` in its model-file index.

Acceptance: `socket_test` passes every § 6 case; 61 of 61.
  Check: `cmake --build /tmp/logic-p51 -j8 && ctest --test-dir /tmp/logic-p51 -j8` → `100% tests passed, 0 tests failed out of 61`; `git diff --stat backend/tests/golden` → empty (est. 3 min). Measured 2026-11-24 in `/tmp/logic-p51-wt`: `100% tests passed, 0 tests failed out of 61`; `git diff --stat backend/tests/golden` empty. VERIFIED.
Commit: (next commit)

## 6. Test cases (`socket_test`)

| Case | Result |
|---|---|
| a 14-pin DIP socket (`"family":"DIP"`, `"rowSpacing":7.62`, `"mounting":"through-hole"`, `"fits":["PDIP-14","CDIP-14"]`) | loads; `socket->family` `DIP`, `row_um` 7620, `fit_count` 2, `fits[0]->id` `PDIP-14` |
| a 28-pin PLCC socket (no `rowSpacing`, `"surface-mount"`, `"finish":"tin"`) | loads; `row_um` 0, `finish` `tin` |
| `"family":"SIP"`; DIP without `rowSpacing`; `rowSpacing` 6.0; PLCC with `rowSpacing` | each rejected with its § 3.1 message |
| 13 pins DIP; 30 pins PLCC; a pin with role `input` | rejected (`even pin count`, `20, 28, …`, `must be inout`) |
| `"mounting":"clip"`; missing `mounting`; `"finish":""` | rejected |
| missing `fits`; `fits` `[]`; unknown id; an id twice; a 12-pin footprint on a 14-pin socket; a footprint with no pad `"7"` | each rejected with its § 3.2 message |
| `"fits"` on a gate part | rejected: `"fits" is only for a socket` |
| `circuit_add_part` of the socket, two pins on one net, the other pins on another | succeeds; the two nets stay separate; `circuit_start` succeeds |

## Validation Plan

- Tests: `socket_test` (above); the 60 existing programs unchanged.
- Coverage check: `socket_test` exercises `socket.c` and `load_fits`; `listing_check` prints the kind once F adds parts.
- Runtime proof: the circuit case of `socket_test`.
- Doc sync: `docs/parts-format.md` (§ socket, § Parts key table, file table).
- Final gate: plan-51-J. Per-phase checks stay scoped.

## Decisions

Owner decisions (asked once, 2026-11-21):

- **The socket is a second part saved with the chip**, not a separate placed symbol.
- **Socket parts are listed in the Parts drawer like any other part.**
- **Three modes: IC, IC + Socket, Socket only**; the Bill of Materials follows the mode; **Run ignores the mode** (the chip is simulated in all three).
- **Switching to a socket mode picks the generic socket for the chip**; the dialog swaps it for a maker part.
- **Catalog scope: through-hole and surface-mount DIP and PLCC sockets**, the three makers Mill-Max, Preci-Dip and Assmann (Samtec has one more in-scope part than Assmann but no PLCC), plus generic sockets.

Author decisions:

- **`"fits"` by footprint id on the socket part** (§ 3.2, rejected alternative there).
- **A new `socket` kind**, not the `header` or `connector` kind: a socket's keys (family, row spacing, mounting, fits) belong to no existing kind.
- **A socket is simulated (passive)**, so a standalone placed socket does not disable Run.
- **Only chip shapes the library has**: DIP 0.3 in at 8, 14, 16, 20, 24, 28 pins; DIP 0.6 in at 24, 28, 32, 40, 48; PLCC 28, 32, 44, 68 (`fits.tsv`). A socket that fits no catalog chip is not added.

## Corrections

- **2026-11-24, baselines at the start of the run (main at 2865af91).** The plan was written at f3256ba1; other plans landed since. Measured: `grep -c '^logic_test' backend/CMakeLists.txt` → 60 (was 39), so A's goal is 61 programs (was 40); `wc -l backend/tests/golden/{parts,traces,footprints}.txt` → 36515 / 36515 / 2775 (was 33042 / 33042 / 1685); kinds 42 (was 34); the last kind is `PART_PLD` (`parts.h:33`), not `PART_NIXIE` (`TRACE_LAST_KIND` `trace_check.c:22`, `listing_check.c:50–51`). Every count of A, F and J was updated in place; no sub-plan's scope rested on the old numbers. Datasheets: 9356 (F's baseline said 8010).
- **2026-11-24, the build directory is `/tmp/logic-p51-wt`.** `/tmp/logic-p51` is a CMake cache of the main checkout's source (another session's), which CMake refuses to reuse for the worktree. Read `/tmp/logic-p51` in every check below as `/tmp/logic-p51-wt`.
- **2026-11-24, `socket_test` differs from the § 6 plan in two places.** (1) It loads the bundled library (`load_parts_dir`: the `5V` supply the circuit case needs, and the real `PDIP-14`, `CDIP-14`, `PLCC-28`) plus two inline footprints (`T-DIP-12`, a 12-pin dual; `T-BALL-14`, 14 ball-named pads) through `load_footprints`, instead of a tiny `footprints.json`. (2) "a footprint with no pad `"7"`" is `T-BALL-14`: a footprint whose designators are `A1` … `B7`, so the numbered pin `"1"` has no pad (`pin "1" has no pad in fits footprint "T-BALL-14"`). A numbered explicit footprint cannot lack one pad and keep its pin count, since the loader requires pads `1` … `pins`. Added cases: a DIP with 2 and with 66 pins, `fits` as a string and as an array of a number.
- **2026-11-24, the DigiKey row is checked on the second key.** The owner started the run with "using opt2": the standard key's daily quota was spent (HTTP 429, resets 2026-11-25 00:00 UTC), the second key had 399 calls left. B runs `node` and `list.js` with `--opt2`-equivalent credentials (B § Corrections).

## Summary

A's risk is the one common-key change in `library.c`; the byte-identical goldens
bound it. Untouched: every existing kind, the engine, the app and every part file.
