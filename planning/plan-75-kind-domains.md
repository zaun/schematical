# plan-75: Shared supply-domain loader (`kind_domains.c`)

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-68 (§ Prerequisites)

`buffer.c` holds the only loader for a part with two supply domains, which is
the `"domains"`, `"vccB"` and `"inputMaxB"` keys of the level translators
(plan-20-F). plan-70 (the `isolator` kind) and plan-71 (the `transceiver` kind)
both need the same loader for digital isolators and VIO / isolated
transceivers. plan-75 moves that code, unchanged, into
`backend/src/library/kind_domains.c` behind one API that `buffer.c`, plan-70-G
and plan-71-G all call. Doing the move as its own plan means neither kind plan
does it, and neither depends on the other.

The single behavioral outcome: nothing changes. Every buffer part loads, binds
and runs exactly as before, with the same error messages. The goldens are
byte-identical, and `buffer.c` has no domain code of its own left.

References:

- `docs/parts-format.md` § Two supply domains (`domains`, `vccB`, `inputMaxB`),
  § `buffer` (model `translator`).
- `docs/backend.md` § Layout, § Adding a kind.
- `backend/src/models/buffer.c`: `DomainNames`, `BufDevice.has_domains` /
  `domains[2]` / `vcc_b`, `BufBound`, `load_domains`, `voltage_b`,
  `buffer_load_device`, `bind_domain`, `buffer_bind_pinout`, `buffer_load_part`.
- `backend/src/library/kind.h` (§ KindContext helpers, `kind_util.c`).
- `planning/plan-68-A-rules-and-gates.md` § 3.3–3.4, § 4, § 6 (the size and
  coverage checks `SZ` and `COV`).
- `planning/plan-70-G-isolator-kind.md`, `planning/plan-71-G-transceiver-kind.md`
  (the two callers that follow; both list plan-75 as a prerequisite).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-68 complete (the size and coverage rules and their checks exist, the ratchets are gone) | `ls planning/completed/plan-68-AK-*.md` → exists | NOT MET (2026-11-25: plan-68 A–AK are in `planning/`, not started) |
| The size check runs | `node scripts/source-size.js --check` → exit 0 | NOT MET (2026-11-25: `scripts/source-size.js` does not exist; plan-68-A adds it) |
| The backend coverage check runs | `node scripts/coverage.js --check --backend` → exit 0 | NOT MET (2026-11-25: `scripts/coverage.js` does not exist; plan-68-A adds it) |
| Backend tests green | `cmake -S backend -B /tmp/logic-p75 && cmake --build /tmp/logic-p75 -j8 && ctest --test-dir /tmp/logic-p75 -j8` → `100% tests passed` | MET (2026-11-25 at 63e2f7d2: `100% tests passed, 0 tests failed out of 77`) |
| The domain code is only in `buffer.c` | `grep -rln 'load_domains\|voltage_b\|bind_domain' backend/src` → `backend/src/models/buffer.c` | MET (2026-11-25) |
| `kind_domains.c` does not exist | `ls backend/src/library/kind_domains.c` → no such file; `grep -c 'kind_domains' backend/src/library/kind.h` → 0 | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `backend/src/library/kind_domains.c` exists, with the API of § 3, declared in
  `kind.h`. `buffer.c` calls it, and `grep -c 'load_domains\|voltage_b\|bind_domain\|DomainNames' backend/src/models/buffer.c` → 0.
- The goldens are byte-identical, and `buffer_test`, `listing_check`,
  `trace_check` and `erc_pins_check` pass.
- `SZ` and `COV --backend` pass with the new file in scope: no file over 1000
  code lines, and no uncovered line in `kind_domains.c` or `buffer.c`.

### Non-goals (explicit constraints)

- No change to the part-file format, to any message text, or to
  `Part.domains` / `domain_count` / `PartSupplyDomain` (`parts.h`).
- No new caller: plan-70-G and plan-71-G add theirs.
- No behaviour change in the translator or transceiver models of `buffer.c`.
- The `"vccB" without "domains"` check and the `missing "vccB"` timing check
  stay in `buffer.c`. They are about which keys a buffer model accepts, not the
  domain format.

## 2. Current State

All in `backend/src/models/buffer.c` (read 2026-11-25 at 63e2f7d2; line numbers
from that tree, which plan-68 may move, so the implementer goes by function
name):

- `DomainNames { power; pins; pin_count; }` (`:675-679`). `BufDevice` keeps
  `has_domains`, `domains[2]` and `vcc_b[]` (one per timing entry) (`:681-689`).
  `BufBound { model; power[2], pins[2]; power_count[2], pin_count[2]; }`
  (`:691-696`).
- `load_domains` (`:710-736`): an absent `"domains"` is fine. Otherwise it must
  be an array of exactly 2 `{"power", "pins"}` objects (`must be an array of 2
  domains`, `must be {"power", "pins"}`).
- `voltage_b` (`:743-780`): the B side's `PartVoltage` from `"vccB"` /
  `"inputMaxB"` over a base (`"inputMaxB" without "vccB"`, `"inputMaxB" below
  the "vccB" max`). It has two callers: the per-timing-entry loop in
  `buffer_load_device` (`:823`) and `buffer_load_part` (`:924`).
- `bind_domain` (`:833-873`) is called twice from `buffer_bind_pinout`
  (`:889-890`), followed by the "in no domain" pass over the pins (`:893-900`)
  (`"domains": "%s" is not a power pin`, `"domains": pin "%s" is in two
  domains`, `"domains": pin "%s" is in no domain`).
- `buffer_load_part`'s domain block (`:927-940`) fills `PartSupplyDomain[2]`
  from the bound indices, with `vcc` for domain 0 and the B voltage for domain 1.
- `BufBound.model` is read by the pin hooks and `buffer_create` (`:947`, `:963`,
  `:1029`). Those keep working on `b->model`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Callers of the three helpers | `load_domains` 1, `voltage_b` 2, `bind_domain` 2 | `grep -n 'load_domains(\|voltage_b(\|bind_domain(' backend/src/models/buffer.c` → 797, 823, 889, 890, 924 (plus the 3 definitions) |
| Buffer devices with `"domains"` | 26 | `grep -c '"domains"' schematical/schematical/Parts/buffer.json` → 26 |
| `buffer_test` lines naming domains | 28 in `backend/tests/*.c` | `grep -n domains backend/tests/*.c \| wc -l` → 28 |
| Error cases for domains in `buffer_test` | 6 (no domain, two domains, not a power pin, vccB on part, vccB in timing, domains on a model without) | `backend/tests/buffer_test.c:1006-1022` |
| `buffer.c` size | 1038 lines | `wc -l backend/src/models/buffer.c` |

### Verified properties

- **The move is neutral.** The bodies use only `KindContext` helpers already
  exported by `kind.h` (`kind_push`, `kind_pop`, `kind_error`,
  `kind_check_keys`, `kind_string`, `kind_names`, `kind_alloc`, `kind_pins`,
  `kind_volt_range`, `kind_volts`). They read nothing of `BufDevice` except the
  fields they fill. Read 2026-11-25, `buffer.c:710-940` against
  `kind.h:85-171`.
- **The error cases are already tested**, so after plan-68's 100% coverage of
  `buffer.c` the moved lines are covered by the same tests
  (`buffer_test.c:1006-1022`, plus the 26 shipped devices through
  `listing_check`).

## 3. Design

`backend/src/library/kind_domains.c`, declared in `kind.h`. The API is the one
plan-70-G and plan-71-G use:

```c
/* Two supply domains (plan-20-F; moved from buffer.c by plan-75). */
typedef struct { const char *power; const char *const *pins; size_t pin_count; } KindDomainNames;
typedef struct { int present; KindDomainNames d[2]; } KindDomains;
int kind_domains_load(KindContext *ctx, const cJSON *json, KindDomains *out);   /* "domains": 2 objects, or absent (present = 0) */
int kind_voltage_b(KindContext *ctx, const cJSON *json, const PartVoltage *base,
                   const PartVoltage **out);                                    /* "vccB" / "inputMaxB" */
/* Per pinout: the power pins and other pins of each domain, as indices; every
   pin except ground, nc and absent in exactly one domain. */
typedef struct { size_t *power[2], power_count[2], *pins[2], pin_count[2]; } KindDomainPins;
int kind_domains_bind(KindContext *ctx, const PartPin *pins, size_t count,
                      const KindDomains *names, KindDomainPins *out);
/* Part.domains / domain_count from the bound pins, vcc (domain 0) and vcc_b. */
int kind_domains_part(KindContext *ctx, const KindDomainPins *b, const PartVoltage *vcc,
                      const PartVoltage *vcc_b, Part *out);
```

| New | Body taken from |
|---|---|
| `kind_domains_load` | `load_domains`, writing `out->present` and `out->d[]` in place of `dev->has_domains` / `dev->domains[]` |
| `kind_voltage_b` | `voltage_b`, verbatim |
| `kind_domains_bind` | `bind_domain` (a static helper inside `kind_domains.c`) called for both domains, then `buffer_bind_pinout`'s "in no domain" pass, including the `owner` scratch allocation |
| `kind_domains_part` | `buffer_load_part`'s block from `kind_alloc(ctx, 2 * sizeof(*domains))` to `out->domain_count = 2` |

`buffer.c` after the move:

- `BufDevice` keeps `KindDomains domains;` in place of `has_domains` and
  `domains[2]`. Its tests read `dev->domains.present`.
- `BufBound` is `{ const void *model; KindDomainPins domains; }`.
- `buffer_load_device` calls `kind_domains_load` and `kind_voltage_b`.
  `buffer_bind_pinout` calls `kind_domains_bind`. `buffer_load_part` keeps its
  `"vccB" without "domains"` check, calls `kind_voltage_b`, then
  `kind_domains_part`.

The gate is **byte-identity**. This is code motion: every golden
(`parts.txt`, `traces.txt`, `footprints.txt`, `erc_pins.txt`) must stay as it
is, and every message must stay as it is (the six `expect_fail` cases check
the text). A diff means the move changed something; find it and fix it.

### Rejected alternatives

- **Each kind plan moves the code when it gets there:** this braids plan-70 and
  plan-71 together. The second of them to run would find its task already
  done, or conflicting.
- **Copy the code into each new kind:** three copies of the same rules and
  messages.
- **A generic "domains" pass in `library.c` for every kind:** that changes
  every kind's key set. The explicit call keeps `"domains"` an error on a kind
  that doesn't use it.

### Where the risk is

Low. The only risk is a silent behaviour change in the move. The byte-identical
goldens and the six message tests catch it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The move

The whole change: one new file, `buffer.c` calling it, and docs.

- [ ] `backend/src/library/kind_domains.c` with the four functions of § 3 (and
      the static per-domain bind helper); `kind.h` declares the types and
      functions after the § KindContext helpers block.
- [ ] `backend/CMakeLists.txt`: `src/library/kind_domains.c` after
      `src/library/kind_util.c`.
- [ ] `backend/src/models/buffer.c`: `DomainNames`, `load_domains`,
      `voltage_b`, `bind_domain`, the inline "no domain" pass and the
      `PartSupplyDomain` fill are deleted. `BufDevice` and `BufBound` use
      `KindDomains` / `KindDomainPins` and the three loaders call the new
      functions, per § 3.
- [ ] The Xcode project compiles `backend/src` as a synchronized folder, so
      the new file needs no project edit (as `kind_util.c`). Check:
      `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p75-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **`.
- [ ] `docs/backend.md` § Layout lists `kind_domains.c`. § Adding a kind gets
      one sentence: "a kind with two supply domains loads them with
      `kind_domains_*` (`kind.h`)".

Acceptance: buffers load and run exactly as before, and the new file meets
plan-68's rules.
  Check: `cmake --build /tmp/logic-p75 -j8 && ctest --test-dir /tmp/logic-p75 -R 'buffer_test|listing_check|trace_check|erc_pins_check|source_size_check' -j8`
  → `100% tests passed`; `git diff --stat backend/tests/golden/` → empty;
  `grep -c 'load_domains\|voltage_b\|bind_domain\|DomainNames' backend/src/models/buffer.c`
  → 0; `node scripts/source-size.js --check` → exit 0;
  `node scripts/coverage.js --check --backend` → exit 0 (est. 6 min: a
  rebuild, the scoped tests, and one instrumented backend build with `ctest`
  for coverage, which is the only tool that shows a moved line is still run).
  Result:
Commit: —

## Validation Plan

- Tests: no new test. The six `buffer_test` domain error cases and the 26
  shipped translator devices (`listing_check`, `trace_check`) cover every
  moved branch.
- Coverage check: `node scripts/coverage.js --check --backend` covers the new
  file because it is in `backend/src` (plan-68-A § 3.3).
- Runtime proof: the app build above, plus the goldens unchanged.
- Doc sync: `docs/backend.md` § Layout, § Adding a kind.
- Final gate (once, after Phase 1): `ctest --test-dir /tmp/logic-p75 -j8` →
  `100% tests passed`; `node scripts/source-size.js --check` → exit 0;
  `node scripts/coverage.js --check --backend` → exit 0 (est. 6 min).

## Decisions

- The shared domain loader is its own plan, and plan-70-G and plan-71-G list it
  as a prerequisite. Decided by the author (2026-11-25), so the two kind plans
  don't both do the move.
- Backend-code letters wait for plan-68 to be complete and meet its size and
  coverage gates. Decided by the owner, 2026-11-25. This plan is all backend
  code, so plan-68 is a whole-plan prerequisite here.
- The API is plan-71-G § 3.0's, adopted by plan-70-G as well (the author,
  2026-11-25).

## Corrections

## Summary

A neutral code move. It unblocks two kind plans without tying them together.
The risk is low and fully gated by byte-identical goldens and the existing
message tests. The translator and transceiver models and the file format are
untouched.
