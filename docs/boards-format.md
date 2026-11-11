# Board file format

Every standard board the PCB's **Type** menu offers — add-in cards (ISA, PCI, PCI Express), modules (M.2, Mini
PCIe, SO-DIMM, DIMM), cartridges, and boards that plug onto a header (Raspberry Pi HAT, Arduino shield, …) — is
described in `schematical/schematical/Boards/boards.json`. Adding a board is an edit to that file: no code knows
any one standard. Only **Custom** (a free rectangle) and **Custom Card** (an edge the user builds) are built
in.

`BoardCatalog` (`Boards/BoardCatalog.swift`) reads the file when the app starts. The tests build every board
with every combination of its options (`BoardCatalogTests`), so a mistake shows up as a failing test that names
the board, the options and the field.

## The file

```json
{ "boards": [ { …board… }, … ] }
```

The boards are listed in menu order, each under its `group`'s heading.

## A board

| Field | Type | Meaning |
|---|---|---|
| `id` | string | The file value (a document's board `type`, a bus's `type`). Unique; not `custom` or `customCard`. Never change it once documents use it. |
| `title` | string | The menu name. |
| `group` | string | The menu section: `"PC Cards"`, `"Modules"`, `"Cartridges"`, `"Add-on Boards"`, or a new one. Default `"Other"`. |
| `notes` | string | Optional. Where the numbers come from and anything a reader needs (pin layout, which side is which). Not used by the app. |
| `connector` | string | `"edge"` (default; gold fingers, pins labelled `Edge.A1`) or `"header"` (through-hole pads, `Header.40`). |
| `optionsTitle` | string | The sidebar heading over the options. Default `"Card"`. |
| `options` | array | Optional. The board's settings; see [Options](#options). |
| `vars` | object | Optional. Named values, each a [formula](#formulas), worked out when first used. |
| `height` | formula | The board's height (y), in mm. |
| `width` | formula | The board's length (x) when it's fixed. |
| `length` | object | Instead of `width`, when the user sets the length: `default`, `min` and `max`, each a formula (they can't use `width`). |
| `yUp` | bool | Optional. `true` when y is measured up from the bottom edge (as many datasheets draw it), for the outline, pins and holes. Default `false`: y down from the top edge. |
| `outline` | array | The board's edge; see [Outline](#outline). |
| `pins` | array | The connector's pins; see [Pins](#pins). |
| `holes` | array | Optional. Non-plated mounting holes: `x`, `y`, `drill` (formulas) and an optional `when`. |
| `pinOrder` | string or array | Optional. The order a bus of the type carries the pins in: natural order of the names when missing (A1, A2, … A10, B1; numbers before letters), `"listed"` for the order the pins are listed in, or an array of every pin name. |

Coordinates are in mm on the board, seen from the component side (Top), with x = 0 at the left. A card's
edge tab is along the bottom and its bracket end at x = 0.

## Formulas

Every number in a board can be a formula instead: a JSON number, `true` / `false`, or a string holding an
expression.

- Numbers in mm, or with a unit: `0.1in`, `100mil`, `2mm`.
- Words in single quotes: `'ddr5'`.
- Names: `width` (the board's actual length) and `height`; the board's option values by id; the `vars` of the
  chosen option choices; the board's `vars`; and inside a pin group, `n`, `i` and its `let` values.
- Operators, loosest first: `c ? a : b` (nests to the right), `||`, `&&`, `==` `!=`, `<` `<=` `>` `>=` and
  `x in [a, b, …]`, `+` `-`, `*` `/` `%`, then unary `-` and `!`, and `( )`.
- Functions: `min`, `max`, `abs`, `floor`, `ceil`, `round`, `sqrt`, `sin` and `cos` (degrees), and
  `interp(x, x0, y0, x1, y1, …)` (straight between the points; the end values beyond them).

```json
"height": "lowProfile ? 68.90 : 111.15",
"x": "45.65 + (n - 1) + (n >= 12 ? 2 : 0)"
```

## Options

Each option is a picker or toggle in the sidebar, and its value is stored in the document's `cardOptions` under
its `id`.

| Field | Type | Meaning |
|---|---|---|
| `id` | string | The name formulas use, and the `cardOptions` key. |
| `title` | string | The sidebar label. |
| `style` | string | `"segmented"` (default), `"menu"` or `"toggle"` (a check box; its choices default to `false` and `true`). |
| `choices` | array | Each `{ "value": …, "title": "…", "vars": { … } }`: the stored value (a number, a word or a flag), the label (default: the value), and named formulas the choice sets. |
| `default` | value | Optional. One of the choices' values; default the first. |
| `onBus` | bool | Whether it changes the pins, so a bus of the type stores it and offers it. Default `true`; set `false` for a setting that only changes the board (a length, a bracket, extra holes). |
| `when` | formula | Optional. Show it only while this holds (`"memory == 'ddr5'"`). |
| `help` | string | Optional. The control's tooltip. |

```json
{ "id": "lanes", "title": "Lanes", "default": 1, "choices": [
  { "value": 1, "title": "x1", "vars": { "pins": 18, "tabEnd": 65.30 } },
  { "value": 4, "title": "x4", "vars": { "pins": 32, "tabEnd": 79.30 } } ] }
```

## Outline

The outline is the board's edge, clockwise from the top-left, as a list of steps:

- `[x, y]`: a point.
- `{ "arc": [cx, cy], "radius": r, "from": a, "to": b, "steps": 6, "skipFirst": false }`: points round a
  centre from angle `a` to `b` in degrees, `steps` segments. With y down, 90° points down, so a corner
  rounded clockwise runs 180 → 270 at the top-left, 270 → 360 at the top-right, 0 → 90 at the bottom-right
  and 90 → 180 at the bottom-left. `skipFirst` leaves out the arc's first point.
- `{ "when": formula, "then": [ …steps… ] }`: the steps, only while the formula holds (a key slot, a
  generation's notches).

A board whose length is free uses `width` for its far end:

```json
"outline": [[0, 0], ["width", 0], ["width", "height"], [0, "height"]]
```

## Pins

Each entry is a run of pins:

| Field | Type | Meaning |
|---|---|---|
| `from`, `to`, `step` | formula | `n` runs from `from` to `to` (inclusive) by `step` (default 1). |
| `each` | array | Instead, `n` takes each value in turn (`["A", "B", …]`). `i` counts from 0 either way. |
| `where` | formula | Optional. Keep only the pins where it holds (`"!(n in [12, 13])"`). |
| `when` | formula | Optional. The whole run, only while it holds. |
| `name` | string | The pin name, with expressions in braces: `"A{n}"`, `"{n + 36}"`, `"8.{n}"`. Default `"{n}"`. Bus wire names are 1 to 4 characters. |
| `let` | object | Optional. Per-pin named formulas the others can use (`"x"`, `"bottom"`). |
| `finger` | object | An edge contact: `x` (its centre), `width`, `top` and `bottom` (its two ends, the edge at the bottom), and `side`, `"top"` (component side) or `"bottom"` (solder side). |
| `pad` | object | Instead, a plated through-hole header pad on every layer: `x`, `y` (its centre), `size` (default 1.8), `drill` (default 1.1) and `square` (default `false`, round; pin 1 is usually square). |
| `onBus` | bool | Default `true`. `false` for pads on the board only, which no bus carries and no net joins (an Arduino shield's ICSP header). |

A pin on each side with the same name is one pin of the bus (a card whose two sides carry the same signal).

```json
{ "name": "B{n}", "from": 1, "to": 62, "where": "!(n in [12, 13, 50, 51])",
  "finger": { "x": "41.605 + (n - 1) * 1.27", "width": 1.016,
              "top": "height - 6.89", "bottom": "height - 2.413", "side": "top" } }
```

## Adding a board

1. Find the standard's mechanical drawing (or a maintained CAD template), and note its source in `notes`.
2. Add the board to `boards.json` under its group, with a new `id`.
3. Run the app tests: `BoardCatalogTests` builds it with every option, checks that every pin and hole is on the
   board and every bus name fits, and names the field when something is wrong.
4. Add a test for the standard's own numbers (a few pin centres, the size), as `AddOnBoardTests` does.
