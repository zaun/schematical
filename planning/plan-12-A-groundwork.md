# plan-12-A: Serial controllers — groundwork (datasheets, pull-up and pull-down parts)

Last updated: 2026-11-02
Overall Effort: huge (> 3d) — plan-12 is A → B → C → D → E → F → G → H → I → J → K
Effort: medium (1h–2h)
Depends on: plan-09 and plan-11 complete (§ Prerequisites)

plan-12 adds every bus-controller IC the owner collected under
`datasheets/controller-uart/` and `datasheets/controller-i2c/` that DigiKey
had in stock: 219 orderable parts in 79 devices — parallel-bus UARTs, USARTs
and ACIAs from TI, MaxLinear/Exar, NXP, Intel, Harris/Intersil, SMSC, GI and
National; I2C/SPI-to-UART bridges from NXP, Diodes, MaxLinear and Maxim; and
the NXP SC18IS604 SPI-to-I2C bridge. They form one new part kind, `serial`,
whose devices pick a behavioral model (§3.2). Every model is a full model of
the chip's digital behavior: registers, FIFOs, baud generator, serial
framing, modem lines, interrupts, flow control, and each chip's extras. The
app gets a terminal window per serial part (double-click, like the memory
window) that shows what a channel transmits and types into its RX pin.

plan-12 also adds generic **pull-up and pull-down parts**, placed from the
toolbar like VCC and GND: I2C needs them, and so do open-drain outputs.

The single behavioral outcome of plan-12: loading the part files gives
P0 + 226 parts (219 serial parts and 7 pull parts), every serial part runs
its datasheet behavior in a model test or a golden trace, a terminal attached
to a UART shows the bytes a program sends through it, and `ctest` passes
T0 + 1 programs (the new `serial_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | datasheet clean-up (27 PDFs out, recorded); pull-up and pull-down parts in the backend (supply `pull`, weak drive, voltage check) | medium |
| B | app: Pull-up and Pull-down toolbar tools and symbols | medium |
| C | `serial` kind, the serial line engine, the `ns16550` core (PC16550D behavior), the terminal API, `serial_test` | large |
| D | `ns16550` TI, NXP and National extensions; the printer port | large |
| E | `ns16550` Exar extensions; Motorola bus; the XR16V798/XR16L784 flat map | large |
| F | 81 parts: TI, National, Intersil and SMSC 16550-family devices; 7 footprints | medium |
| G | 69 parts: Exar and NXP 16550-family devices; 4 footprints | medium |
| H | classic models (`ay1013`, `i8251`, `i82510`, `acia6551`, `hs82c52`, `com78c80x`) and their 29 parts; 2 footprints | large |
| I | I2C and SPI target front ends; the SC16IS7xx, PI7C9X76x and XR20M bridges; the `sc18is604` I2C controller; 22 parts; 4 footprints | large |
| J | `max3100` and `max310x` models and 18 parts; 7 footprints | large |
| K | app: the serial kind, the terminal window; plan-12 landing tier | large |

Outcome of A: `datasheets/controller-*` holds the 80 PDFs plan-12 uses (27
removed and recorded), and the library has 7 pull parts (`PULLUP-5V`,
`PULLUP-3V3`, `PULLUP-2V5`, `PULLUP-1V8`, `PULLUP-1V2`, `PULLUP-12V`,
`PULLDOWN`) that drive their net weakly and feed the voltage check.

References:

- `AGENTS.md` (datasheets: in-stock rule, obsolete rule, part files,
  goldens).
- `docs/parts-format.md` (whole file; § `supply`, § Parts, § Footprints),
  `docs/backend.md` (voltages, adding a kind, tests).
- `planning/plan-12-data/README.md` and every file it lists — the facts each
  sub-plan cites: `parts.tsv`, `devices.tsv`, `specs/*.md`, `matrix.md`,
  `decisions.md`, `footprints.md`.
- `planning/completed/plan-10-A-groundwork.md` … `plan-10-J-app.md` (the
  last multi-kind plan: kind with model dispatch, data conventions, goldens)
  and `planning/completed/plan-09-B-cpu-kind-6502.md` (the `cpu_state` hook
  and `circuit_cpu_state`, the pattern for the terminal API).
- `planning/completed/plan-07-B-voltage-check.md` (the voltage check).

## Prerequisites

This table is the gate for all of plan-12. B–K point here and add their own
rows.

| Must be true | Command | Status |
|---|---|---|
| plan-09 complete (owner, 2026-11-02: plan-12 follows plan-09) | `ls planning/plan-09-* 2>/dev/null` → no output; `ls planning/completed/plan-09-E-*.md` → exists | MET (2026-11-01 at 403651a) |
| plan-11 complete (character LCD modules; plans land in creation order, §Decisions) | `ls planning/plan-11-*.md 2>/dev/null` → no output; `ls planning/completed/plan-11-D-*.md` → exists | **NOT MET** (2026-11-01 at 403651a: `planning/plan-11-A…D-*.md` open on main; plan-11 in progress on `worktree-P-11` at bffe730 — A done, B/C/D 42 boxes open) |
| plan-10 complete | `ls planning/plan-10-*.md 2>/dev/null` → no output; `ls planning/completed/plan-10-J-app.md` → exists | MET (2026-11-01 at 403651a) |
| Backend tests green | `cmake -S backend -B /tmp/logic-p12 && cmake --build /tmp/logic-p12 -j8 && ctest --test-dir /tmp/logic-p12` → `100% tests passed` | MET (2026-11-01 at 403651a: 17 programs; T0 is measured after plan-11 lands, which adds programs) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt` (= `traces.txt`) and `footprints.txt` | measured 2026-11-01 at 403651a: parts/traces 7157, footprints 195 — not yet P0/F0 (plan-11 changes them) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p12-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01 at 403651a) |
| No `serial` kind or pull parts yet | `ls schematical/schematical/Parts/serial.json backend/src/models/serial*.c 2>/dev/null` → no output; `grep -c PULLUP schematical/schematical/Parts/supply.json` → 0 | MET (2026-11-01 at 403651a) |
| The datasheets are present | `ls datasheets/controller-*/*.pdf \| wc -l` → 107 | MET (2026-11-01 at 403651a) |
| The plan data is present | `awk -F'\t' '$16=="add"' planning/plan-12-data/parts.tsv \| wc -l` → 219; `tail -n +2 planning/plan-12-data/devices.tsv \| wc -l` → 79 | MET (2026-11-01 at 403651a) |
| DigiKey script works (F–J re-check stock) | `node scripts/digikey.js --in-stock --list "TL16C550D" \| head -1` → a part line | MET (2026-11-01 at 403651a: `TL16C550DIPFB Active`) |

Everything below is written against the world where these hold.

> **Owner override (2026-11-01):** the owner said to start plan-12 while
> plan-11 is still running (plan-11 thread 742c2f59…, `worktree-P-11`) and
> to work around it. plan-12 runs in `worktree-P-12`. The plan-11 row stays
> NOT MET until plan-11 lands on main. Then main is merged into
> `worktree-P-12`, conflicts are resolved, and the goldens regenerated.
> T0/P0/F0 are taken from main **with** plan-11, and every count check is
> re-run against them. plan-12 does not merge to main before plan-11 has.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `ls datasheets/controller-*/*.pdf | wc -l` → 80; `datasheets/not-included.md`
  gains 24 rows and `datasheets/obsolete-not-included.md` 3 (§4.1).
- `ctest --test-dir /tmp/logic-p12` → `100% tests passed`, T0 programs.
- `wc -l < backend/tests/golden/parts.txt` → P0 + 7, `traces.txt` → P0 + 7,
  `footprints.txt` → F0; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- A net with only `PULLUP-5V` on it reads 1; with `PULLUP-5V` and a strong 0
  it reads 0 and nothing shorts; `voltage_check` takes a pull-up's voltage on a
  net no strong driver sets (§4.2).

### Non-goals (explicit constraints)

- No existing part's listing line or trace hash changes (every existing
  `parts.txt`/`traces.txt` line byte-identical).
- The voltage check's result for every circuit without a pull part is
  unchanged: pull levels are consulted only on groups where a pull part is
  connected.
- Existing supplies (`VCC`, `5V`, … `GND`) behave exactly as today.
- No app change (B), no `serial` kind (C).

## 2. Current State

- `supply` kind (`backend/src/models/supply.c`): device key `volts`; one pin;
  drives it `DRIVE_STRONG` at start (`supply_start`). `PartSupply` is
  `{ int level; uint32_t mv; }` (`backend/include/parts.h:63`).
- Net resolution (`backend/src/engine/net.c:90-125`): the strongest drivers
  win; equal-strength drivers that disagree give X; `shorted` only for strong
  1 against strong 0. `DRIVE_WEAK` exists (`sim.h:17`) but no model uses it
  (`grep -rn DRIVE_WEAK backend/src/models` → no output).
- Voltage check (`backend/src/voltage.c`): groups (closed switch contacts),
  supply level S per group (every `PART_SUPPLY`; two different non-zero
  supplies clash), driven D (powered outputs, open-drain skipped), carried C
  (LEDs); effective E = S, else D, else C.
- Datasheets (`ls datasheets/controller-*/*.pdf | wc -l` → 107): 104 in
  `controller-uart/`, 2 in `controller-i2c/` (`sc18is604.pdf`, `cp2120.pdf`), 1 in
  `controller-spi/` (`sc18is606.pdf`).

### Measured populations

| What | Count | Command |
|---|---|---|
| In-stock part names checked (DigiKey, 2026-11-02: 249 rows, 5 names listed twice under two makers and merged) | 244 | `tail -n +2 planning/plan-12-data/parts.tsv \| wc -l` |
| … added | 219 | `awk -F'\t' '$16=="add"' planning/plan-12-data/parts.tsv \| wc -l` |
| … left out (reason in the row) | 25 | `awk -F'\t' '$16=="leave-out"' planning/plan-12-data/parts.tsv \| wc -l` |
| Devices | 79 | `tail -n +2 planning/plan-12-data/devices.tsv \| wc -l` |
| … `ns16550` (parallel bus / bridge) | 52 / 7 | `awk -F'\t' '$2=="ns16550"{print $3}' planning/plan-12-data/devices.tsv \| sort \| uniq -c` |
| … other models | 20 | `devices.tsv` (ay1013 5, i8251 1, i82510 1, acia6551 1, hs82c52 2, com78c80x 3, max3100 2, max310x 4, sc18is604 1) |
| Parts flagged obsolete | 29 | `awk -F'\t' '$16=="add" && $11~/obsolete/' planning/plan-12-data/parts.tsv \| wc -l` |
| New footprints | 24 | `grep -c '^## `' planning/plan-12-data/footprints.md` → 25 sections, one (`SOP-24-24P2W`) not added |
| Parts per data sub-plan | F 81, G 69, H 29, I 22, J 18 | grouping of `devices.tsv` (script in §3.4) |
| Contradictions resolved | 405 | `planning/plan-12-data/decisions.md` header |
| PDFs removed (§4.1) | 27 | §4.1 table |

### Verified properties

| Claim | How verified |
|---|---|
| No new part name collides with an existing one | the census names against every `name` in `Parts/*.json` (python set intersection, 2026-11-02) → 0 |
| Every added part has a pinout and a footprint (existing or new) | `parts.tsv`: 219 `add` rows, `pinout` and `footprint` non-empty in all |
| The 24 new footprints load and do not overlap | throwaway probe (`plan-12-data/footprints.md` § Probe: a copy of `footprints.json` + 24 objects → `load_footprints` 219 footprints, overlaps 0) |
| A model can drive a pin whose role is `input` | `sim_schedule_drive` takes any pin index of the instance (`sim.h:193`); the net resolves every pin's drive (`net.c:97`) — the terminal's RX drive (§3.7) relies on it |
| A new `.c` under `backend/src/models/` needs one line in `backend/CMakeLists.txt`; a new `Parts/*.json` needs none | plan-10-A §2 (unchanged: `backend/CMakeLists.txt` lists sources; tests and the app load `Parts/*.json` by directory/bundle) |

## 3. Design Overview (all of plan-12)

### 3.1 Where each part goes

| Model | Devices | Parts | Sub-plans |
|---|---|---|---|
| `supply` with `pull` (existing kind) | 7 pull devices | 7 | A (backend), B (app) |
| `serial` / `ns16550`, parallel bus | 52 | 150 | C (core), D (TI/NXP/NS), E (Exar); parts F (22 devices, 81 parts), G (30, 69) |
| `serial` / `ay1013`, `i8251`, `i82510`, `acia6551`, `hs82c52`, `com78c80x` | 13 | 29 | H |
| `serial` / `ns16550` with an I2C/SPI host; `sc18is604` | 8 | 22 | I |
| `serial` / `max3100`, `max310x` | 6 | 18 | J |
| app | — | — | B, K |

### 3.2 One kind, models inside it

`serial` is one `PartKindOps` and one `SimComponentType` (the plan-10 and
plan-09-B pattern: "one `SimComponentType` per kind; a kind with several
behaviours dispatches inside its model"). A device names its model with
`"model"`; the model supplies its device keys, binding and engine callbacks.
Part file `schematical/schematical/Parts/serial.json`; `PartKind`
`PART_SERIAL`; info struct `PartSerial { const char *model; unsigned channels;
const char *grade; }` on `Part.serial`. `Part.logic` stays NULL (no logic
family).

| Model | Devices (sub-plan) |
|---|---|
| `ns16550` | the 16450/16550 register family and its extensions — 59 devices (C core; D, E extensions; I I2C/SPI host) |
| `ay1013` | AY-5-1013A, COM2017, COM1863, HD-6402, CDP1854A — pin-parallel UARTs (H) |
| `i8251` | 8251A, asynchronous and synchronous (H) |
| `i82510` | 82510 (H) |
| `acia6551` | CDP65C51 (H) |
| `hs82c52` | 82C52, HD-6406 (H) |
| `com78c80x` | COM78C804, COM78C802, COM78804 (H) |
| `sc18is604` | SC18IS604 SPI-to-I2C bridge (I) |
| `max3100` | MAX3100, MAX3140 (J) |
| `max310x` | MAX3107, MAX3108, MAX3109, MAX14830 (J) |

Rejected alternatives:

- **One kind per function** (`uart`, `usart`, `bridge`, `i2c`): the app, the
  terminal API and the loader would branch four ways for parts that share the
  serial line engine and the terminal; the drawer separates them by
  `"function"` instead.
- **A model per device** (79 models): 52 of the 59 16550-family devices are
  the same machine with different FIFO sizes, register banks and pins;
  `matrix.md` §3 lists the parameters that separate them.
- **Bridges as a CPU-bus part plus a separate I2C/SPI front-end part** wired
  together: the chips are single parts with one pinout; the front end is a
  host-bus option of the `ns16550` model (I).

### 3.3 Shared pieces

- `backend/src/models/serial.h` + `serial.c` (C): the kind, the model table,
  grades, the terminal hooks.
- `backend/src/models/serial_line.h` + `serial_line.c` (C): the line engine
  every model uses — a baud generator that counts edges of its clock pin
  (divisor, 16×/8×/4× sampling, fractional divisor in D/E), the transmitter
  (start, 5–9 data bits LSB first, parity none/odd/even/mark/space, 1/1.5/2
  stop bits, break), the receiver (start-bit detection on the falling edge,
  start validated at mid-bit, data and stop sampled at mid-bit from the 16×
  clock, framing/parity/break detection), and the sent-frame log and RX drive
  for the terminal (§3.7).
- `backend/src/models/serial_bus.h` + `serial_bus.c` (C): host-bus helpers —
  Intel strobes (RD/RD-high, WR/WR-high, chip selects, ADS address latch),
  data bus 3-state drive with read access and float delays, DDIS; I adds the
  I2C target and SPI target front ends and E the Motorola bus.
- `model_pins.h`: `part_pin_open_drain` answers for serial parts (open-drain
  INT/IRQ pins, I2C SDA/SCL) from C on.

### 3.4 Data conventions (every data sub-plan: F, G, H, I, J)

- **Parts** come from `planning/plan-12-data/parts.tsv` rows with `status`
  `add`: `name`, `package`, `pinout`, `footprint`, `flags` (omit when empty),
  `grade` (§ below) and a part-level `vcc`/`inputMax` where the row's range
  differs from its grade's. The `parts` arrays are generated from the TSV by a
  script under `/tmp/p11/` (not committed; the goldens and each sub-plan's
  name check verify it). The grouping into F–J is `devices.tsv`: model
  `ns16550` function `UART` with a device id starting `TL16`, `NS16`, `PC16`,
  `82C50` or `COM92` → F; other `ns16550` `UART` → G; `ay1013`, `i8251`,
  `i82510`, `acia6551`, `hs82c52`, `com78c80x` → H; `max3100`, `max310x` → J;
  the rest → I.
- **Stock re-check.** Before adding its parts each data sub-plan re-runs
  `node scripts/digikey.js --in-stock --list "<family>"` for its families; a
  part no longer listed is left out and named in that sub-plan's Corrections
  (AGENTS.md: add only in-stock parts). The data does not change otherwise.
- **Device ids** are the base part number as the maker prints it
  (`TL16C550C`, `XR16V554`, `8251A`, `AY-5-1013A`, `SC16IS752`), from
  `devices.tsv`.
- **Pinouts** are named as in `parts.tsv` (`plcc44`, `tqfp48`, `qfn32`,
  `dip40`, …); pin names and roles come from the device's spec section (no
  overbar marks; `BAR` only for a true/complement pair). Pins whose function a
  mode pin selects (Intel/Motorola) take the Intel-mode name; the model's
  keys name the Motorola meaning. Exposed pads are not pins (the footprint's
  unnumbered `centre`). Pins a package does not bond out are absent from that
  pinout; their register bits follow `decisions.md` (absent modem inputs read
  X, their delta bits stay 0).
- **Grades**: `"grades"` maps a grade name (the part-number letters that pick
  the supply/temperature/speed, e.g. `"C"`, `"I"`, `"5V"`, `"-5"`) to timing
  in ns (the model's timing keys; a missing name is 0, as `sram`) plus `vcc`,
  `inputMax` and `note`. Timing values: the datasheet's typical, else its max
  (bus delays) or min (widths), at the main supply row (P5), with a `note`
  naming the page.
- **Flags**: `parts.tsv` already applies them (`obsolete` from DigiKey status;
  `automotive`, `military`, `ceramic`).
- **Drawer**: `"drawer": ["Interface"]`; `"function"` from `devices.tsv`:
  `"UART"`, `"Bridge UART"` (I2C/SPI-hosted UARTs), `"I2C controller"`.
  `"description"`: the datasheet title in sentence case. `"note"`: the
  datasheet(s) and the spec section, e.g. `"tl16c550c.pdf p.1-40;
  plan-12-data/specs/ti-a.md § TL16C550C"`.
- **Footprints**: each new footprint lands with the first sub-plan whose parts
  use it (`footprint_check` fails on an unused footprint): F 7, G 4, H 2, I 4,
  J 7, the JSON objects verbatim from `footprints.md`.

### 3.5 Signal rules (every serial model)

- **Power-on**: the device's reset values (the spec section's reset table;
  P3 where silent), then the reset pin applies at start.
- **Inputs**: Z reads as X, except reset pins and chip selects, where Z reads
  inactive (an unconnected reset or select is the common hookup). An X on a
  strobe or chip select during a cycle makes that cycle's effect X: a write
  stores X into the addressed register, a read drives X.
- **Bus outputs**: the data bus is driven only during a selected read
  (3-state otherwise), with the grade's access and float delays; DDIS-style
  pins mirror it.
- **Serial outputs** (TX, RTS, DTR, OUT1/2, BAUDOUT, TXRDY/RXRDY): driven
  strong, inertially (`sim_schedule_inertial_drive`), changed only on value
  change. INT/IRQ pins per the device's mode: driven, 3-state (OUT2/MCR[3]
  gated) or open-drain (0 strong, else Z).
- **Clock**: the clock pin (XTAL1/XIN/CLK/RCP/TCP) is an input the model
  counts edges on (owner: an oscillator part drives it); XTAL2/XOUT is never
  driven. Divisor 0 stops the generator (P2).
- **Undefined behavior** is X (P2); clock and baud limits are not checked
  (P5).

### 3.6 Pull-up and pull-down parts (A, B)

A pull part is a `supply` device with `"pull": true`: it drives its one pin
`DRIVE_WEAK` (1 for `volts` > 0, 0 for 0 V). A strong driver on the net wins
over it, so open-drain outputs and I2C lines work; a pull-up and a pull-down
on one net give X (equal strength, `net.c`). In the voltage check a pull adds
a level P per group, used only when no strong supply and no powered output
sets the group: E = S, else D, else P, else C. A pull never clashes with a
supply or another pull (the highest pull voltage is P). §4.2 has the details.

### 3.7 The terminal (C backend, K app)

Owner decisions: the terminal is a window opened by double-clicking a serial
part, like the memory window; what a channel transmits is shown; typed bytes
are driven onto the channel's RX pin as a real serial waveform at the
channel's current baud and format, and only while no other part drives that
RX net.

- Hooks on `SimComponentType` (the `cpu_state` precedent):
  `serial_channels(state)`, `serial_sent(state, channel, start, frames, cap)`
  (frames a channel has sent since start; each `uint16_t` is the data bits)
  with `serial_sent_count`, and `serial_send(sim, instance, state, channel,
  bytes, count)` (queue bytes for that channel's RX; 0 when the channel is not
  in an asynchronous mode).
- Circuit API (`circuit.h`): `circuit_serial_channels`,
  `circuit_serial_sent_count`, `circuit_serial_sent`, `circuit_serial_send`.
  `circuit_serial_send` refuses (-1, `circuit_error`) when another part's
  output, in/out or strong supply pin is on the RX pin's net (pull parts are
  allowed), when the part is not a serial part, or when the channel is out of
  range or not asynchronous.
- The model drives the queued bytes onto its own RX pin, strong, one bit per
  16 ticks of that channel's receive clock with the channel's current LCR
  format (or control-pin format, `ay1013`); after the last stop bit it
  releases the pin (Z). While it sends, the receiver reads that level even if
  the RX pin is unconnected.

### 3.8 Where correctness risk concentrates

- The `ns16550` core (C): 150 parallel parts and 21 bridge parts run through
  it. Its interrupt priorities, FIFO trigger and timeout rules and the
  RX sampling point are where a wrong rule mislabels every part. C's
  `serial_test` has one case per PC16550D feature before any part lands.
- The extension sets (D, E): each style (EFR banks, TCR/TLR, Exar FCTR tables,
  5G flat map) gets its own `serial_test` cases from its spec section before
  F/G add parts that use it.
- Data sub-plans (F–J) change the goldens by 2 × parts lines each; they come
  after their models' tests so a golden recorded over a model bug cannot hide
  it. Byte-identity is the gate for existing parts only (every existing golden
  line unchanged, in every sub-plan).

## 4. Detailed Design (this sub-plan)

### 4.1 Datasheets

Delete these 27 PDFs and record each (columns PDF, family, category / group,
notes; the notes give the reason and the source URL):

| PDF | Record in | Reason |
|---|---|---|
| `controller-uart/ch432.pdf`, `ch438.pdf`, `ch9432.pdf`, `ch9434.pdf`, `ch9437.pdf`, `ch9438.pdf` (WCH) | `not-included.md` | no part in stock at DigiKey (2026-11-02; `--in-stock` searches `CH432`, `CH438`, `CH943` find none of them) |
| `controller-uart/wk2124.pdf`, `wk2132.pdf`, `wk2168.pdf`, `wk2204.pdf`, `wk2212.pdf` (WeiKai) | `not-included.md` | no part in stock at DigiKey (search `WK2` finds none) |
| `controller-uart/pi7c9x1170.pdf`, `pi7c9x1172.pdf` (Diodes) | `not-included.md` | no part in stock (search `PI7C9X11` finds only PI7C9X112/113, other chips) |
| `controller-uart/sc16c550b.pdf`, `sc28l202.pdf` (NXP) | `not-included.md` | no part in stock (searches `SC16C550`, `SC28L202` → 0) |
| `controller-uart/xr16l2550.pdf`, `xr16m2750.pdf`, `xr16v598.pdf`, `xr16v698.pdf` (MaxLinear) | `not-included.md` | no part in stock (absent from the in-stock UART category list) |
| `controller-i2c/cp2120.pdf` (Silicon Labs) | `not-included.md` | no part in stock (search `CP2120` finds only MCP2120) |
| `controller-spi/sc18is606.pdf` (NXP) | `not-included.md` | no part in stock (search `SC18IS` finds only SC18IS604); `controller-spi/` is then empty and goes |
| `controller-uart/com8017.pdf` (COM8017, COM8502) | `not-included.md` | no in-stock part has a known package with a pinout in the PDF (`parts.tsv` reasons) |
| `controller-uart/m66230.pdf` (M66230) | `not-included.md` | no package drawing for Renesas 24P2W-A can be found (`footprints.md` § SOP-24-24P2W) |
| `controller-uart/tl16c2752.pdf` (TL16C2752) | `not-included.md` | the datasheet names the extended registers but does not describe their bits; it cannot be modeled fully (`specs/ti-c.md` § TL16C2752) |
| `controller-uart/cdp6402.pdf` (CDP6402) | `obsolete-not-included.md` | its only in-stock part CDP6402D is Obsolete at DigiKey |
| `controller-uart/sc16c652b.pdf` (SC16C652B) | `obsolete-not-included.md` | every in-stock part Obsolete at DigiKey (3) |
| `controller-uart/st16c650a.pdf` (ST16C650A) | `obsolete-not-included.md` | the only datasheet-covered in-stock part ST16C650ACJ44 is Obsolete; ST16C650CJ44/IJ44 are the non-A chip with no datasheet |

Family column: `serial`; category / group: `Interface → UART` (or `Bridge
UART`, `I2C controller` for cp2120 and sc18is606). The remaining 80 PDFs (79 in
`controller-uart/`, `controller-i2c/sc18is604.pdf`) stay as they are.

### 4.2 Pull parts

- **Format.** `supply` device key `"pull"`: optional boolean, default
  `false`. With `true` the device is a pull: `volts` > 0 makes a pull-up, 0 a
  pull-down. Its one pin has role `output` (it drives a signal net, not a power
  net), named `PU` (pull-up) or `PD` (pull-down). `PartSupply` gains `int
  pull;`. `docs/parts-format.md` § `supply` gets the key, the 7 parts in its
  table, and one paragraph on weak drive.
- **Model.** `supply_start` drives `DRIVE_WEAK` when `pull`, else
  `DRIVE_STRONG` as today.
- **Parts** (`supply.json`, `"drawer": []` like the supplies, one part per
  device named as its id):

  | Part | `volts` | Pin |
  |---|---|---|
  | `PULLUP-5V` | 5 | `PU` |
  | `PULLUP-3V3` | 3.3 | `PU` |
  | `PULLUP-2V5` | 2.5 | `PU` |
  | `PULLUP-1V8` | 1.8 | `PU` |
  | `PULLUP-1V2` | 1.2 | `PU` |
  | `PULLUP-12V` | 12 | `PU` |
  | `PULLDOWN` | 0 | `PD` |

  The pull-up rails are the VCC tool's (`SupplyChoice.names`, B).
- **Voltage check** (`voltage.c`): step 2 skips pull supplies for S and for
  clashes; a new array `pulled` takes `level_raise` from each pull-up on its
  group; `effective()` becomes S, else D, else P, else C. A pull-down adds
  nothing (0 V is the absence of a level, as GND is never a voltage problem).
  The header comment's step list gains P.
- **Listing.** `listing_check` prints `|pull` after a supply's voltage for pull
  parts only (existing supply lines unchanged).
- **Tests.**
  - `library_check`: `"pull": "yes"` → rejected (`"pull" must be true or
    false`); `"pull": true, "volts": 0` loads as a pull-down (`supply->pull ==
    1`, `level == 0`).
  - `circuit_check`: a net with `PULLUP-5V` only reads `CIRCUIT_1`; the same
    net with `GND` added reads `CIRCUIT_0` and `circuit_shorted_net` → 0;
    `PULLUP-3V3` + `PULLDOWN` reads `CIRCUIT_X`.
  - `voltage_test`: the output of the open-drain fixture gate `VFX-OD1G00`
    (`voltage_fixture.h`) with `PULLUP-5V` on its net into
    input A of `VFX-AUC1G00` (input max 3.6 V) → the input-max problem at
    5000 mV; with `PULLUP-3V3` instead → no problem; `PULLUP-5V` and `3V3` on
    one net → no clash (E = 3.3 V); a push-pull 3.3 V output with
    `PULLUP-5V` on its net → no problem (D wins).

## Compatibility / Format Impact

- Part-file format: one optional `supply` key. Existing files load unchanged.
- C API: `PartSupply` gains `pull` (public `parts.h`; the app reads it in B).
- Datasheets: 27 PDFs leave the repo, each recorded.

## Phases

> **Status discipline (applies to every phase of every plan-12 sub-plan).**
> - A task checkbox is ticked only when its result is observed, with the
>   command and output recorded on the task line.
> - Fill the phase's `Commit:` line with the hash(es) the moment it lands.
> - Anything that differs from the plan goes in Corrections, not silently.
> - Commits are made on the current branch (AGENTS.md: never branch unless the
>   owner names the branch). Itemized commits; touch only this plan's files.

### Phase 1 — Datasheets

- [x] Delete the 27 PDFs of §4.1; add 24 rows to `datasheets/not-included.md`
  and 3 to `datasheets/obsolete-not-included.md`. Check: `ls
  datasheets/controller-*/*.pdf | wc -l` → 80; `ls -d datasheets/controller-spi
  2>/dev/null` → no output; `git diff --stat datasheets/*.md` → `+24`, `+3`.
  Observed: `80`; no output; `not-included.md | 24 +++…`,
  `obsolete-not-included.md | 3 +++`.
- [x] Commit (one commit: the deletions and both records).

Acceptance: the checks above (seconds).
Commit: ece2a38

### Phase 2 — Pull parts

- [x] `parts.h` `PartSupply.pull`; `supply.c` key, weak drive; docs § `supply`.
  Check: the `library_check`/`circuit_check` cases below pass (weak drive:
  "a strong 0 beats a pull-up").
- [x] `voltage.c` pull level P (§4.2); `backend.md` § Using it, the voltage
  note gains one sentence on pulls. Check: the 4 `voltage_test` pull cases
  PASS; the earlier cases are unchanged and still pass.
- [x] `supply.json`: the 7 devices of §4.2. Check: the device ids load as
  `[… '12V', 'PULLUP-5V', …, 'PULLDOWN']`, and the listing shows 7 new lines.
- [x] `listing_check` `|pull` (printed as ` pull` inside the facts field,
  see Corrections); regenerate `parts.txt` and `traces.txt`. Check:
  `git diff backend/tests/golden | grep -c '^-[^-]'` → 0; `git diff
  backend/tests/golden/parts.txt | grep -c '^+[^+]'` → 7, same for
  `traces.txt`. Observed: `0`; 7 (`PULLDOWN|supply|||Pull-down|||level=0
  mv=0 pull|1:PD:output:default`, …); 7.
- [x] The `library_check`, `circuit_check` and `voltage_test` cases of §4.2. Observed: `ok   valid pull part`,
  `ok   supply pull not a boolean`, 6 `circuit_check` PASS lines (net reads 1,
  0 with no short, X), 4 `voltage_test` PASS lines.

Acceptance: `ctest --test-dir /tmp/logic-p12` → `100% tests passed`, T0
programs (about 15 s); the golden checks above. Observed (pre-plan-11 main):
`100% tests passed, 0 tests failed out of 17`. Re-run after merging plan-11.
Commit: —

## Validation Plan

The landing tier for plan-12 runs once, at the end of K (plan-12-K
§ Validation Plan). For A: Phase 2's acceptance (`ctest`, the golden diff) is
the gate, since A changes the supply model, the voltage check and the part
files — what `library_check`, `circuit_check`, `voltage_test` and the golden
programs cover.

## Decisions

- **Scope** (owner, 2026-11-01/02): bus-controller ICs only (chips that drive
  a bus from another bus, never slaves); every parallel UART from every maker
  plus every I2C/SPI-to-UART bridge; every in-stock classic UART the census
  found (the owner said yes to downloading their datasheets); SPI-hosted
  I2C controllers count as I2C controllers, I2C-hosted SPI controllers as SPI
  controllers (none is in stock).
- **In stock only** (AGENTS.md, 2026-11-02): parts come from DigiKey's
  in-stock list; DigiKey's status sets `obsolete` (the ECS precedent).
- **Full models** (owner): every digital behavior the datasheet describes,
  including synchronous modes, IrDA, RS-485 direction control and printer
  ports.
- **Clock** (owner): XTAL1/XIN is a clock input driven by an oscillator part;
  no crystal part.
- **Terminal** (owner): a window per serial part, opened by double-click;
  typed bytes drive the RX pin, only while nothing else drives it (§3.7).
- **Pull-ups follow the VCC menu** (owner): one pull-up per rail and one
  pull-down; the Pull-up tool uses the supply menu's voltage (B).
- **plan-12 after plan-09** (owner: plan-09 was active in its own thread) **and
  after plan-11** (character LCD modules, written while this plan was
  researched): plans land in creation order unless the owner says otherwise,
  and plan-11 also changes the goldens, the kind table and the app's symbol
  code, so T0/P0/F0 are measured after it.
- **One kind `serial`** with model dispatch (§3.2).
- **Datasheet contradictions** are resolved by P1–P11 and 16 planner
  decisions (`plan-12-data/decisions.md`).
- **Left-out parts and PDFs** (§4.1, `parts.tsv` reasons): unknown package,
  no pinout for the package, a device not fully modelable from its datasheet
  (TL16C2752), no package drawing (M66230), all-obsolete PDFs.
- **Pull parts are `supply` devices**, not a new kind: the one-pin, one-level
  shape, the drawer rule and the app's supply tools all apply; only the drive
  strength and the voltage-check role differ.

## Corrections

- **Gate overridden by the owner (2026-11-01).** The plan-11 prerequisite was
  NOT MET, measured at 403651a: plan-11 A done, B/C/D 42 boxes open on
  `worktree-P-11`. The owner said to start anyway and work around plan-11
  (see the note under Prerequisites).
- **Source URLs in the §4.1 records.** `plan-12-data` holds no source URLs,
  so each row names the maker's product page or site (the format the
  existing rows use).
- **Listing marker.** §4.2 says to print `|pull` after the voltage. The
  supply facts field is space-separated `key=value` inside one pipe column,
  so the marker is ` pull` inside that field (`level=1 mv=5000 pull`). A
  `|` would give these 7 lines one more column than every other line.

## Summary

A clears the ground: the datasheet folders down to the 80 PDFs plan-12 uses,
with 27 recorded, and 7 pull parts that drive weakly and give the voltage
check a pull level. No serial kind yet; the goldens grow by 7 lines and
nothing existing changes.
