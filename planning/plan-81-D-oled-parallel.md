# plan-81-D: 3-wire SPI, 8080 and 6800 interfaces, parallel reads, timing checks

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-81-C

D completes the SSD1306-class model's interfaces: the 9-bit 3-wire SPI and the two 8-bit parallel
buses, with the parallel status and data reads and the datasheets' bus-timing checks. After D every
strap code of § 3.5 of C runs; C's `interface "…" is not modeled yet` fault is gone. Many of the
plan's modules (Newhaven `NHD-2.23-12832UC*3`, `NHD-1.27/1.5-…`, Display Visions `EA W…`, Orient
`AOM12864A0-0.96WW`) bring their BS straps and D0–D7 to their headers, so a schematic can strap any
of the five interfaces.

References:

- `SSD1306.pdf` § 8.1.1 (6800), § 8.1.2 (8080), § 8.1.4 (3-wire: "9-bits … D/C# bit, D7 to D0 …
  Under serial mode, only write operations are allowed"), Table 13-2 (6800 timing), Table 13-3
  (8080 timing), Table 13-5 (3-wire timing); `SH1106.pdf` pp. 8–9 (6800/8080, dummy read,
  status read table: "Reads display data" / "Reads status"), the SH1107 equivalents.
- `backend/src/models/charlcd.c:878` and its bus-timing rules (`tc`, `tpw`, `tas`, `tah`, `tdsw`,
  `th`, `tddr`, `tdhr`: the precedent for timing keys and violations).
- plan-81-C § 3 (profiles, registers, LOST, image).

## Prerequisites

plan-81-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-81-C complete | `ls planning/completed/plan-81-C-*.md` → exists | NOT MET (2026-11-25) |
| C's not-yet fault is present (D removes it) | `grep -c 'is not modeled yet' backend/src/models/oled*.c` → ≥ 1 | NOT MET (C adds it) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- 3-wire SPI, 8080 and 6800 run the same command and data stream as C's interfaces: the C
  init-and-pattern test, replayed on each, gives the same `OledState`.
- Parallel reads: a status read (D/C# = 0) returns bit 6 = display off (SSD13xx: bits 7 and 5–0
  read 0; SH110x: bit 7 BUSY = 0, bit 5 = 0); a data read after setting the address returns the
  dummy byte first, then GDDRAM bytes, the column advancing per read except in SH110x
  read-modify-write (`E0h` … `EEh`).
- A parallel access that violates the device's `timing` stores X (data) or makes the controller
  LOST (command), exactly as charlcd's rules do.
- `ctest` passes T1 + 1 programs.

### Non-goals (explicit constraints)

- No serial timing checks: `serial_target.c`'s front ends check none for any of their users
  (`serialmem`, the MAX310x, SC18IS604 and converter models), and 3-wire SPI follows them. A
  serial clock faster than the datasheet minimum (SSD1306 Table 13-4) is not reported.
- No change to the image, profiles or commands of C.

## 2. Current State

C's `oled.c` decodes all five strap codes; for 3-wire, 8080 and 6800 it publishes all-X with
`fault` `interface "<name>" is not modeled yet`. The 4-wire SPI and I2C front ends are
`serial_target.c`'s; nothing in `serial_target.c` frames 9-bit words or parallel strobes.

## 3. Design

### 3.1 `oled_bus.c`

A new file with the three interfaces (keeps `oled.c` and `oled_ctl.c` under 1000 lines):

- **3-wire SPI**: CS# (active low; `null` = tied low), D0 = SCLK, D1 = SDIN. A transaction starts at
  CS# falling; on each SCLK rising edge one bit is shifted in; every 9th bit completes a word: bit 8
  (the first) = D/C#, bits 7–0 the byte, fed to `oled_ctl_byte`. A word cut short by CS# rising is
  dropped. SCLK or SDIN reading X at a sample makes that word's bits X (a command word with X bits
  is LOST, as in C). No reads.
- **8080**: CS#, D/C#, WR# (`bus.rw`), RD# (`bus.e`), D0–D7. A write is latched at WR#'s rising edge
  while CS# = 0; a read drives D0–D7 from RD#'s falling edge + `tacc` to its rising edge + `toh`
  (X during `tacc`, as charlcd's `tddr`).
- **6800**: CS#, D/C#, R/W#, E, D0–D7. An access is E high while CS# = 0; R/W# = 0 latches the data
  at E's falling edge; R/W# = 1 drives D0–D7 from E rising + `tacc` to E falling + `toh`.
- **Reads** (both parallel buses): D/C# = 0 → the status byte (§ 1). D/C# = 1 → a data read: the first
  read after an address-setting command returns the dummy byte (X, the datasheets' "dummy read"),
  later reads return the GDDRAM byte at the pointer and advance it (not in read-modify-write mode,
  where writes advance it, SH1106).
- **Data bus drive**: D0–D7 are `inout`; the model drives them only during a read (Z otherwise),
  as charlcd.

### 3.2 `"timing"` key

Required on a simulated device whose `bus` has `rw` and `e` and all 8 `data` pins (a module that can
be strapped parallel): `{"6800": {...}, "8080": {...}}`, ns, from the controller's Tables 13-2 and
13-3 (SSD1306: `tcycle 300, tas 0/10, tah 0, tdsw 40, tdhw 7, tacc 140, toh 70, tpwl 60, tpwh 60`;
6800 `tas` 0, 8080 `tas` 10; the module datasheet's table when it prints one, else the controller's).
Keys per bus: `tcycle tas tah tdsw tdhw tacc toh tpwl tpwh`, each ≥ 0; `tcycle`, `tpwl`, `tpwh` > 0
(`"timing" "8080" needs "tcycle" above 0`). Violations, per access: less than `tas` from D/C# or
R/W# to the strobe's active edge; less than `tcycle` between active edges; strobe low (8080) or E
high (6800) less than `tpwl`, high/low less than `tpwh`; a data pin changing less than `tdsw` before
the latching edge or less than `tdhw` after it; D/C# or R/W# changing less than `tah` after it.
A violated data write stores X bits; a violated command write is LOST.

### Rejected alternatives

- **3-wire SPI on `SerialSpi`.** Its framing is 8-bit bytes (`received` at the 8th sample edge);
  forcing 9-bit words through its `bit` callback would split every word across two bytes. Twenty
  lines of edge sampling in `oled_bus.c` are simpler and testable on their own.
- **No parallel timing checks.** charlcd checks its bus timing, and an AVR bit-banging a parallel
  bus at 20 MHz can violate `tcycle` (300 ns = 6 cycles); the check is what tells the user.

## 4. Phases

### Phase 1 — 3-wire SPI

- [ ] `backend/src/models/oled_bus.c` / `oled_bus.h`: § 3.1's 3-wire SPI; `oled.c` routes the strap code to it.
- [ ] `backend/tests/oled_test.c`: C's init-and-pattern replayed over 3-wire (a static `spi9_word` helper); a short word; X on SDIN.

**Acceptance:** `ctest --test-dir /tmp/logic-p81 -R oled_test --output-on-failure` → passed. (≈ 30 s.)

Commit:

### Phase 2 — 8080 and 6800, reads

- [ ] `oled_bus.c`: 8080 and 6800 writes and reads (§ 3.1); remove C's not-yet fault.
- [ ] Tests: the replay on each bus; status read (display on and off); dummy read then data reads;
      SH1106 read-modify-write; D0–D7 Z outside reads.

**Acceptance:** `ctest … -R oled_test` → passed; `grep -c 'is not modeled yet' backend/src/models/oled*.c` → 0. (≈ 30 s.)

Commit:

### Phase 3 — timing

- [ ] `oled.c`: the `"timing"` key and errors (§ 3.2); `oled_bus.c`: the checks.
- [ ] Tests: one violation per rule on each bus (data → X bits, command → LOST); an access exactly at
      each minimum passes.

**Acceptance:** `ctest … -R oled_test` → passed. (≈ 30 s.)

Commit:

### Phase 4 — docs

- [ ] `docs/parts-format.md` § `oled`: the three interfaces, reads, `"timing"`.

**Acceptance:** `awk '/^### `oled`/{f=1;next} /^### /{f=0} f' docs/parts-format.md | grep -c -E '"timing"|8080|6800|3-wire'` → ≥ 4. (≈ 1 s.)

Commit:

## 5. Validation Plan

End of D: `ctest --test-dir /tmp/logic-p81 -j8` → 100% of T1 + 1; `node scripts/source-size.js
--check` and `node scripts/coverage.js --check --backend` → exit 0 (`oled_bus.c` 100% lines);
goldens unchanged.
