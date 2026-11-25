# plan-60-C: Bill of Materials Validate (DigiKey stock and price)

Last updated: 2026-11-24
Effort: medium
Depends on: plan-60-B

plan-60-C adds a **Validate** button to the Bill of Materials window. With a verified DigiKey connection it looks
up each orderable line at DigiKey and fills new **Stock**, **Unit Price**, **Total** and **DigiKey** columns; the
Export CSV and Copy outputs include them. Without a verified connection it shows a dialog that sends the user to
Settings ▸ Connections.

A correct implementation: on a verified connection, **Validate** on a BOM containing `SN74HC00N` shows its stock
(292 at planning) and the unit price at the quantity's price break; a Generic line shows "Generic"; an unknown
part number shows "Not found"; on an unverified connection a dialog explains and **Open Settings** opens the
Connections page.

References:

- `schematical/schematical/BillOfMaterials/BillOfMaterialsWindow.swift` (`BillOfMaterialsLine`,
  `BillOfMaterials.csv`, `BillOfMaterialsTable`)
- `docs/export.md` § Bill of materials
- `planning/plan-60-data/productdetails-SN74HC00N.json`: the measured API response the tests use
- plan-60-B (`DigiKeyConnection`)

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-60-B is complete | `rg -l "func accessToken" schematical/schematical/Connections` → `DigiKeyConnection.swift`; `rg -n "Commit: —" planning/plan-60-B-digikey-sign-in.md` → no matches | NOT MET at planning |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue. If you stop, report the status of all prerequisites.

## 1. Goal

- **Validate** (toolbar, `checkmark.seal`) in the BOM window.
- Unverified: an alert "DigiKey connection needed" with **Open Settings** (opens Settings on the Connections
  page) and **Cancel**.
- Verified: one DigiKey lookup per orderable line, sequential, with progress ("Validating 3 of 12") and a
  **Cancel** button; results fill the new columns.
- **Export CSV…** and Copy include the four new columns when results exist.

### Non-goals (explicit constraints)

- Results are never saved in the document and never cached to disk: stock and price are live.
- **Export JLCPCB / PCBWay BOM…** is unchanged (a fixed fab layout; `BillOfMaterials.jlcpcbCSV`).
- Without results, `BillOfMaterials.csv` output is byte-identical to today's (existing tests keep passing).
- No other DigiKey endpoint, no ordering, no cart.

## 2. Current State

- `BillOfMaterialsLine` carries `partNumber`, `manufacturer` (empty for a Generic part, which has no orderable
  number), `quantity`, `references` (`BillOfMaterialsWindow.swift:23-40`).
- `BillOfMaterials.csv(_:)` writes `Quantity,References,Part Number,Package,Value,Color,Description`
  (`BillOfMaterialsWindow.swift:142-152`); `onCopyCommand` and the Export CSV button use it.
- The toolbar has Select All, Export CSV…, Export JLCPCB / PCBWay BOM… (`BillOfMaterialsWindow.swift:285-311`).
- The table shows Value and Color columns only when some line has one (`hasValue`, `hasColor`,
  `BillOfMaterialsWindow.swift:239-240, 264-271`); the new columns follow that pattern.
- `docs/export.md` § Bill of materials (line 162 onward) documents the CSV columns.

### Measured populations

| What | Count | Command |
|---|---|---|
| BOM tests today | 7 | `rg -c "func test" schematical/schematicalTests/BillOfMaterialsTests.swift` → 7 |
| DigiKey daily quota | 1000 calls per key, 23 left when probed (shared with other sessions) | `x-ratelimit-limit`/`x-ratelimit-remaining` on `GET /products/v4/search/SN74HC00N/productdetails` |

### Verified properties (probed 2026-11-24 through `scripts/digikey.js`'s Safari transport)

- `GET /products/v4/search/{partNumber}/productdetails?includes=Product(ManufacturerProductNumber,Manufacturer,
  QuantityAvailable,ProductStatus,ProductUrl,ProductVariations)` for `SN74HC00N` → `200`, 1445 bytes, body
  `{"Product": {...}}` with keys `Manufacturer, ManufacturerProductNumber, ProductUrl, ProductVariations,
  QuantityAvailable, ProductStatus` (saved: `planning/plan-60-data/productdetails-SN74HC00N.json`).
- Same call for `NOT-A-REAL-PART-123` → `404`, body `{}`.
- The keyword route `POST /products/v4/search/keyword` with `Keywords=SN74HC00N, Limit=3` returned
  `SN74HC00NSR` first, so it does not give an exact match; the product-details route does.
- For `SN74HC00N`: `QuantityAvailable` 292; `ProductStatus` `{"Id":0,"Status":"Active"}`; two variations:
  `2156-SN74HC00N-ND` (`MarketPlace: true`, `StandardPricing: []`, stock 0, no `MinimumOrderQuantity` key) and
  `296-1563-5-ND` (Tube, `MarketPlace: false`, stock 292, `MinimumOrderQuantity` 1, 9 price breaks from
  `1 → 0.78` to `5000 → 0.35313`).
- `SN74HC00NSR`'s variations include a Tape & Reel variation with `MinimumOrderQuantity` 2000 and breaks starting
  at `2000`, and a Cut Tape variation with breaks from `1`. A reel is not orderable at a quantity of 5.

## 3. Design Overview

Three pieces:

1. `Connections/DigiKeyLookup.swift` (pure): `PartAvailability` (stock, unit price, line total, status, lifecycle
   text, product URL) and `DigiKeyLookup.summarize(_ body: Data?, status: Int, quantity: Int) -> PartAvailability`.
   All the price and status rules live here and are unit tested against the saved JSON.
2. `BillOfMaterials/BOMValidator.swift`: `@Observable @MainActor final class BOMValidator`: runs the lookups for a
   list of lines through `DigiKeyConnection`, keeps `results: [BillOfMaterialsLine.ID: PartAvailability]`,
   `progress`, and a per-session cache keyed by part number (the product body, not the price, so a changed
   quantity needs no new call).
3. The window changes: the **Validate** toolbar button and alert, the four columns, the CSV columns.

Risk concentrates in the rules of piece 1 (a wrong price is worse than no price) and in the rate limit; both are
covered by the unit tests. The network path was built and passed in plan-60-B.

### Rules (all decided)

**Which lines are looked up:** a line with a non-empty `manufacturer`. A line with an empty one is shown as
status "Generic" with no call.

**One call per distinct part number**, `GET /products/v4/search/{percent-encoded partNumber}/productdetails` with
the `includes` of § 2 and the headers of plan-60-B. Sequential. A part number seen twice in one run is one call.

**Status** (`status` and `stock` from the response):

| Condition | Status text |
|---|---|
| `404` | "Not found" |
| `QuantityAvailable == 0` | "Out of stock" |
| `0 < QuantityAvailable < quantity` | "Only N" (N = stock) |
| `QuantityAvailable >= quantity` | "In stock" |
| any other HTTP error | "Error" (the message goes to the tooltip) |

When `ProductStatus.Status` is not "Active", " · " and that text follow (for example "In stock · Obsolete").

**Stock column**: `QuantityAvailable` (blank for Generic, Not found, Error).

**Unit price**: among the variations with `MarketPlace == false`, a non-empty `StandardPricing`, and
`MinimumOrderQuantity` (a missing key is 1) at most the line's quantity, take for each the price at the largest
`BreakQuantity` that is at most the quantity; the unit price is the lowest of those. No such variation → blank
price and the DigiKey column gets " · Min order above quantity" appended when stock is above 0.
**Total** = unit price × quantity, rounded to cents. Prices are USD (the request's locale headers fix it).

**Text**: price shows a `$` and 2 to 5 decimals with trailing zeros past 2 removed (`$0.78`, `$0.41028`); total
shows `$` and exactly 2 decimals.

**Results go stale**: a result stores the quantity it was computed for; the table shows blank cells for a line
whose quantity changed since, and Validate again fills them (from the cache: no new call).

**Rate limit and errors**: `429` (`DigiKeyError.rateLimited`) stops the run and shows an alert: "DigiKey's daily
limit was reached" plus the reset time from `x-ratelimit-resettime` or `retry-after` when present; lines already
looked up keep their results. `DigiKeyError.notVerified` mid-run stops the run and shows the "needs
connection" alert.

### CSV

`BillOfMaterials.csv(_ lines:, availability: [BillOfMaterialsLine.ID: PartAvailability] = [:])`: when
`availability` is empty the header and rows are today's; when not empty four columns follow Description:
`Stock,Unit Price,Total,DigiKey` (stock as an integer, prices as plain decimals with no `$`, so a spreadsheet
reads them as numbers; the DigiKey column is the status text). Lines with no result have empty cells.

Rejected alternatives:

- The keyword search: inexact match first (§ 2).
- A bulk call: no DigiKey route for a list of manufacturer part numbers was probed or documented in the pages
  read; one call per part is the measured route.
- Saving results in the document: stale the next day, and a new file-format field for live data.
- A separate results sheet: the owner chose columns.

## Compatibility / Format Impact

`BillOfMaterials.csv` gains four columns only when results exist; its no-results output is unchanged. The JLCPCB
CSV, the document file and the catalog are unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work it describes.
> `- [~]` is partial; a moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's `Commit:` line when it
> lands. An unticked box means NOT DONE.

### Phase 1 — Summarizing a DigiKey response

Lands alone: pure code with no caller.

- [ ] `Connections/DigiKeyLookup.swift`: `PartAvailability`, `DigiKeyLookup.summarize`, the price and status
      rules and the two text formatters of § 3.
- [ ] Tests `schematicalTests/DigiKeyLookupTests.swift`, using `planning/plan-60-data/productdetails-SN74HC00N.json`
      pasted as a string literal: quantity 5 → unit `0.78`, status "In stock", stock 292, the marketplace
      variation ignored; quantity 30 → unit `0.5016` (the 25 break); quantity 5000 → unit `0.35313` and
      "Only 292"; the `404` body `{}` → "Not found"; a body with `QuantityAvailable: 0` → "Out of stock"; a
      `ProductStatus.Status` of "Obsolete" appends " · Obsolete"; a variation with `MinimumOrderQuantity` 2000 is
      skipped at quantity 5 and its cheaper price is used at quantity 2000; price text `0.78` → `$0.78`,
      `0.41028` → `$0.41028`, `0.5` → `$0.50`; total of `0.5016 × 30` → `$15.05`.
- [ ] Add files to `project.pbxproj`.

Acceptance: every case above passes.
  Check: `XC test -only-testing:schematicalTests/DigiKeyLookupTests` → `** TEST SUCCEEDED **` (est. 5 min;
  `XC` = `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p60-dd
  CODE_SIGNING_ALLOWED=NO`).
Commit: —

### Phase 2 — The validator and the CSV columns

- [ ] `BillOfMaterials/BOMValidator.swift` as § 3.2, the transport reached through `DigiKeyConnection.send`.
- [ ] `BillOfMaterials/BillOfMaterialsWindow.swift`: `BillOfMaterials.csv(_:availability:)` per § 3; the
      `onCopyCommand` and `export(_:)` pass the validator's results.
- [ ] Tests `schematicalTests/BOMValidatorTests.swift` with a fake `DigiKeyTransport`: a Generic line makes no
      request; two lines with one part number make one request; results fill by line id; a `429` stops the run,
      keeps earlier results and reports the reset time; a second run with the same part numbers makes no
      request; a changed quantity recomputes from the cache with no request.
- [ ] Tests in `BillOfMaterialsTests`: `csv(lines)` with no availability equals the pre-change output for the
      same lines (the 7 existing tests keep passing); with availability the header gains the four columns and an
      unvalidated line has four empty cells; `jlcpcbCSV` output is unchanged.

Acceptance: the cases above pass; the existing seven BOM tests pass unchanged.
  Check: `XC test -only-testing:schematicalTests/BOMValidatorTests
  -only-testing:schematicalTests/BillOfMaterialsTests` → `** TEST SUCCEEDED **` (est. 8 min).
Commit: —

### Phase 3 — The Validate button and the columns (largest blast radius last)

- [ ] `BillOfMaterialsWindow.swift`: the **Validate** toolbar item (`checkmark.seal`) before Export CSV…; while a
      run is going it shows a `ProgressView` with "Validating n of m" and a **Cancel** button; the alert of § 1
      (`openSettings` environment action after setting `@AppStorage("preferencesPage")` to
      `PreferencesPage.connections.rawValue`).
- [ ] The four columns (**Stock**, **Unit Price**, **Total**, **DigiKey**), shown only when the validator has
      results; the DigiKey cell's tooltip is the product URL or error message; numeric cells right-aligned,
      `.monospacedDigit()`.
- [ ] Readiness: the button's action checks `DigiKeyCredentials.status`: `.verified` runs; anything else shows
      the alert.
- [ ] Owner-driven real-window pass: with the connection unverified, Validate shows the alert and **Open
      Settings** lands on Connections; after Verify (plan-60-B), Validate on a schematic with an `SN74HC00D` or
      similar line fills the columns; Export CSV… contains them. Move the pointer to (0, 0) afterwards.

Acceptance: the owner's pass above succeeds on a Debug build.
  Check: owner-visible pass (est. 15 min; no unit test drives a toolbar, an alert and a Settings window).
Commit: —

## Validation Plan

- Tests: `DigiKeyLookupTests`, `BOMValidatorTests`, `BillOfMaterialsTests`.
- Coverage check: the three classes `@testable import schematical`; the new files are in the test target.
- Runtime proof: Phase 3's owner pass.
- Doc sync: `docs/export.md` § Bill of materials: the four optional columns and that the JLCPCB BOM has none;
  `docs/connections.md`: the Validate rules of § 3 (which lines, status text, price rule, rate limit).
- Final gate (run once): `XC test -only-testing:schematicalTests/DigiKeyLookupTests
  -only-testing:schematicalTests/BOMValidatorTests -only-testing:schematicalTests/BillOfMaterialsTests
  -only-testing:schematicalTests/PickAndPlaceExportTests -only-testing:schematicalTests/DigiKeyConnectionTests` →
  `** TEST SUCCEEDED **` (est. 12 min; `PickAndPlaceExportTests` is the other class that reads
  `BillOfMaterialsLine`).

## Decisions

- Results are new columns in the BOM table — decided by the owner, 2026-11-24. (§ 1)
- Export CSV includes stock and price — decided by the owner, 2026-11-24. (§ 3, CSV)
- Validate on an unverified connection shows a dialog that sends the user to Settings ▸ Connections — decided by
  the owner, 2026-11-24. (§ 1)
- The JLCPCB / PCBWay BOM is unchanged — author's call: it is a fixed fab layout. (Non-goals)
- Unit price is the lowest orderable price at the line's quantity; one call per distinct part number via the
  exact product-details route — author's calls from the probes in § 2. (§ 3)

## Corrections

(none yet)

## Summary

The risk is a wrong price or status, held down by pure rules tested on a measured response, and the daily
quota (23 of 1000 left when probed), handled by caching and a clear `429` alert. Untouched: the document format,
the JLCPCB BOM and every other DigiKey endpoint.
